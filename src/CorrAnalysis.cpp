// Copyright 2021 University of Adelaide
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "CorrAnalysis.h"
#include "AnalysisOuput.h"
#include "WorkContext.h"
#include <string>
#include <vector>
#include <cmath>
#include <omp.h>
typedef double real_t;

struct cs_t
{
    real_t mean;
    real_t cs2;
    cs_t() :
        mean(0.0),
        cs2(0.0)
    {}
};

struct c_t
{
    real_t c2;
};

struct corr_analysis_priv_t
{
    traceinfo_t traceinfo;
    traceinfo_t mainoutti;
    analysis_output_t* mainoutput;
    std::vector<analysis_output_t*> outputs;
    std::vector<cs_t> cs_data;
    std::vector<cs_t> cs_samples;
    std::vector<std::vector<c_t>> c_datasamples;
    double sample_data;
    uint32_t n;
    uint32_t totalproctraces;
    uint32_t output_at;
    uint32_t guess_count;
    std::function<void(int,const trace_t*,double**)> model;
    int data_index;
};

corr_analysis_t::corr_analysis_t()
{
    pimpl.init(); 
}
void corr_analysis_t::init(const traceinfo_t *traceinfo, 
                      analysis_output_t* output, 
                      const analysis_opts_t& opts)
{
    auto p = pimpl.get();
    p->traceinfo = *traceinfo;
    p->cs_samples.resize(traceinfo->nsamples);
    p->totalproctraces = 0;
    p->output_at = wc_gopts_get("at_each_ntraces").get_int();

    auto it = opts.find("corr_data_index");
    PWASSERT(it != opts.end(), "'corr_data_index not' set");
    p->data_index = it->second.get_int();
    p->n = 0;
    p->model = wc_gopts_get("corr_model").get_func<void,int,const trace_t*,double**>();
    p->guess_count = 1;
    if (wc_gopts_has("corr_guess_count"))
    {
        p->guess_count = wc_gopts_get("corr_guess_count").get_int();
    }
    p->cs_data.resize(p->guess_count);
    p->c_datasamples.resize(p->guess_count);
    p->outputs.resize(p->guess_count);

    auto& mainoutti = p->mainoutti;
    mainoutti = *traceinfo;
    mainoutti.nsamples = p->guess_count;
    mainoutti.ntraces = mainoutti.ntraces/p->output_at;
    mainoutti.nterms = 1;
    mainoutti.ndata = 0;
    p->mainoutput = output;
    p->mainoutput->init(&mainoutti);
    for (auto i=0u;i<p->guess_count;i++)
    {
        p->outputs[i] = output->add_sub_output(std::to_string(i), "numpy");
        
        traceinfo_t tout = *traceinfo;
        tout.ntraces = tout.ntraces/p->output_at;
        tout.ndata = 0;
        tout.nterms = 1;
        p->outputs[i]->init(&tout);
        
        p->c_datasamples[i].resize(traceinfo->nsamples);
    }
}
void corr_analysis_t::trace_submit(const trace_t *trace)
{
    auto& ti = pimpl.get()->traceinfo;

    auto& cs_data = pimpl.get()->cs_data;
    auto& cs_samples = pimpl.get()->cs_samples;

    auto& c_datasamples = pimpl.get()->c_datasamples;
    auto p = pimpl.get();
    uint32_t n = ++p->n;
    
    real_t nx = 1.0/n;
    real_t nn = (n-1.0)/n;
    double *datavals = nullptr;
    
    p->model(p->data_index, trace, &datavals);
    if (n == 1)
    {
        for (auto i=0u;i<ti.nsamples;i++)
        {
            cs_samples[i].mean = trace->_samples[i];
            cs_samples[i].cs2 = 0;
        }
        for (auto j=0u;j<p->guess_count;j++)
        {
            cs_data[j].cs2 = 0;
            cs_data[j].mean = (real_t) datavals[j];
            for (auto i=0u;i<ti.nsamples;i++)
            {
                c_datasamples[j][i].c2 = 0;
            }
        }
    }
    else
    {
        for (auto j=0u;j<p->guess_count;j++)
        {
            real_t delta = (real_t) datavals[j] - cs_data[j].mean;
            cs_data[j].mean = cs_data[j].mean + delta * nx;
            // Schneider et al. Leakage Assessment Methodology Eq. (3), d=2
            // $s^2 = CM_2; CM_d = CS_d/n; s^2 = CS_2/n$
            cs_data[j].cs2 = cs_data[j].cs2 + delta * delta * nn;
        }

        for (auto i=0u;i<ti.nsamples;i++)
        {
            real_t delta= trace->_samples[i] - cs_samples[i].mean;
            cs_samples[i].mean = cs_samples[i].mean + delta * nx;
            // Schneider et al. Leakage Assessment Methodology Eq. (3), d=2
            // $s^2 = CM_2; CM_d = CS_d/n; s^2 = CS_2/n$
            cs_samples[i].cs2 = cs_samples[i].cs2 + delta * delta * nn;
        }
#ifdef PARALLEL
        omp_set_dynamic(0);
        omp_set_num_threads(8);
        #pragma omp parallel for
#endif
        for (auto i=0u;i<ti.nsamples;i++)
        {
            for (auto j=0u;j<p->guess_count;j++)
            {
                real_t deltadata = datavals[j] - cs_data[j].mean;
                // Philippe Pebay, Formulas for Robust, One-Pass 
                // Parallel Computation of Covariances and 
                // Arbitrary-Order Statistical Moments 
                // Eq. (3.12)
                c_datasamples[j][i].c2 += nn*(trace->_samples[i]-cs_samples[i].mean)*deltadata;
            }
        }
    }
    
    uint32_t m = p->output_at;
    if ( n % m == 0 )
    {
        trace_t mainout(&p->mainoutti);
        for (auto j=0u;j<p->guess_count;j++)
        {
            traceinfo_t oi{ti.nsamples, ti.ntraces, 1, 0};
            trace_t outputtrace(&oi);
            mainout._samples[j] = 0;
            calc_output(p->n, &outputtrace, j, &mainout._samples[j]);
            p->outputs[j]->on_result_trace(&outputtrace);
        }
        p->mainoutput->on_result_trace(&mainout);
    }
    
}
void corr_analysis_t::calc_output(uint32_t outputtraces, trace_t* traces, 
        uint32_t guessidx, double* maxcorrelation)
{
    auto& cs_samples = pimpl.get()->cs_samples;
    auto& cs_data = pimpl.get()->cs_data;
    auto& c_datasamples = pimpl.get()->c_datasamples;
    
    for (auto i=0u;i<pimpl.get()->traceinfo.nsamples;i++)
    {
        double b = 0;
        double a = sqrt(cs_samples[i].cs2 * cs_data[guessidx].cs2);
        
        if (a == 0.0)
        {
            b = 0;
        }
        else
        {
            b = c_datasamples[guessidx][i].c2 / a;
        }
        if (std::isnan(b))
        {
            std::abort();
        }
        traces->_samples[i] = b;
        *maxcorrelation = std::max(*maxcorrelation, b);
    }
    
}
void corr_analysis_t::finit()
{
    auto p = pimpl.get();
    LogInfo("groups %u\n",p->n);
    // finish output
    p->mainoutput->finit();

}
corr_analysis_t::~corr_analysis_t() 
{
    
}
