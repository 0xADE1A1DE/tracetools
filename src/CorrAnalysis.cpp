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
#include "LeakageModel.h"
#include "LeakageModelFactory.h"
#include <string>
#include <vector>
#include <cmath>

#include <eigen3/Eigen/Dense>

typedef float real_t;

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

using Mat = Eigen::MatrixXf;
struct corr_analysis_priv_t
{
    // Model value distributions, each element 
    // represents the statistics for each distribution 
    // of model generated values for input values (plaintext, key, ciphertext) 
    // for each trace
    std::vector<cs_t> cs_model_dists;
    // Sample distributions hold the stats for
    // each distribution at each sample index
    // (each sample index in a trace)
    std::vector<cs_t> cs_sample_dists;
    Mat c_modelsamples;
    uint32_t model_dist_count;
    leakage_model_t* model;
    leakage_t* leakage;
    std::vector<float> test;

    traceinfo_t traceinfo;
    double sample_data;
    uint32_t n;
    uint32_t totalproctraces;
    uint32_t output_at;
    
    Mat model_batch_mat;
    Mat sample_batch_mat;
    uint32_t batch_mat_fill;

    traceinfo_t mainoutti;
    analysis_output_t* mainoutput;
    std::vector<float> modelvalues;

    std::vector<Mat> batch_mats;
    
};

#define BATCH_LEN 5000

void corr_analysis_t::read_stdin(std::vector<float>& modelvals)
{
    float stddval=0;
    for (auto g=0u;g<modelvals.size();g++)
    {
        int ret = scanf("%f\n", &stddval);
        if (ret != 1)
        {
            LogErrDie("failed read from stdin!\n");
        }
        modelvals[g] = (float)stddval;
    }
}
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
    p->cs_sample_dists.resize(traceinfo->nsamples);
    p->totalproctraces = 0;
    p->output_at = wc_gopts_get("at_each_ntraces").get_int();

    p->n = 0;
    p->leakage = nullptr;
    if (wc_gopts_has("corr_model")) 
    {
        std::string model = wc_gopts_get("corr_model").get_string();
        p->model = leakage_model_factory_t::get_instance()->get_model(model);
        p->model->init(traceinfo, &p->leakage);
        p->model_dist_count = p->leakage->get_dist_count();
    }
    else
    {
        p->model = nullptr;
        p->model_dist_count = 1;
        if (wc_gopts_has("corr_guess_count"))
        {
            p->model_dist_count = wc_gopts_get("corr_guess_count").get_int();
        }
        else
        {
            LogInfo("guess count has been set to 1\n");
        }
        p->modelvalues.resize(p->model_dist_count);
        
    }
    
    p->model_batch_mat.resize(BATCH_LEN, p->model_dist_count);
    p->sample_batch_mat.resize(BATCH_LEN, traceinfo->nsamples);
    p->c_modelsamples.resize(p->model_dist_count, traceinfo->nsamples);
    
    p->cs_model_dists.resize(p->model_dist_count);
    p->test.resize(traceinfo->nsamples);

    auto& mainoutti = p->mainoutti;
    mainoutti = *traceinfo;
    mainoutti.nsamples = p->model_dist_count;
    mainoutti.ntraces = mainoutti.ntraces/p->output_at;
    mainoutti.nterms = 1;
    mainoutti.ndata = 0;
    
    if (!p->model)
    {
        p->mainoutput = output;
        p->mainoutput->init(&mainoutti);
    }
    
}
void corr_analysis_t::trace_submit(const trace_t *trace)
{
    auto& ti = pimpl.get()->traceinfo;

    auto& cs_model_dists = pimpl.get()->cs_model_dists;
    auto& cs_sample_dists = pimpl.get()->cs_sample_dists;

    auto& c_modelsamples = pimpl.get()->c_modelsamples;
    auto p = pimpl.get();
    uint32_t n = ++p->n;
    
    real_t nx = 1.0/n;
    real_t nn = (n-1.0)/n;
    
    float* leakage = nullptr;
    if (p->model) 
    {
        p->model->model_leakage(trace->_data.getptr(), p->leakage);
        leakage = p->leakage->get_dist_values();
    }
    else 
    {
        read_stdin(p->modelvalues);
        leakage = p->modelvalues.data();
    }
    if (n == 1)
    {
        for (auto i=0u;i<ti.nsamples;i++)
        {
            cs_sample_dists[i].mean = trace->_samples[i];
            cs_sample_dists[i].cs2 = 0;
        }
        for (auto j=0u;j<p->model_dist_count;j++)
        {
            cs_model_dists[j].cs2 = 0;
            cs_model_dists[j].mean = leakage[j];
            for (auto i=0u;i<ti.nsamples;i++)
            {
                c_modelsamples(j,i) = 0;
            }
        }
    }
    else
    {
        for (auto j=0u;j<p->model_dist_count;j++)
        {
            real_t delta = (real_t) leakage[j] - cs_model_dists[j].mean;
            cs_model_dists[j].mean = cs_model_dists[j].mean + delta * nx;
            // Schneider et al. Leakage Assessment Methodology Eq. (3), d=2
            // $s^2 = CM_2; CM_d = CS_d/n; s^2 = CS_2/n$
            cs_model_dists[j].cs2 = cs_model_dists[j].cs2 + delta * delta * nn;
        }

        for (auto i=0u;i<ti.nsamples;i++)
        {
            real_t delta= trace->_samples[i] - cs_sample_dists[i].mean;
            cs_sample_dists[i].mean = cs_sample_dists[i].mean + delta * nx;
            // Schneider et al. Leakage Assessment Methodology Eq. (3), d=2
            // $s^2 = CM_2; CM_d = CS_d/n; s^2 = CS_2/n$
            cs_sample_dists[i].cs2 = cs_sample_dists[i].cs2 + delta * delta * nn;
        }

    }
    
    if (p->batch_mat_fill < BATCH_LEN)
    {
        for (auto i=0u;i<ti.nsamples;i++)
        {
            p->sample_batch_mat(p->batch_mat_fill, i) = (trace->_samples[i]-cs_sample_dists[i].mean);
        }
        for (auto j=0u;j<p->model_dist_count;j++)
        {
            p->model_batch_mat(p->batch_mat_fill, j) = (leakage[j] - cs_model_dists[j].mean)*nn;
        }
        p->batch_mat_fill++;
    }
    else
    {
        // Philippe Pebay, Formulas for Robust, One-Pass 
        // Parallel Computation of Covariances and 
        // Arbitrary-Order Statistical Moments 
        // Eq. (3.12)

        Mat temp = p->model_batch_mat;
        temp.transposeInPlace();
        Mat result = temp * p->sample_batch_mat;
        p->c_modelsamples += result;
        p->batch_mat_fill = 0;
    }
    
    uint32_t m = p->output_at;
    if ( n % m == 0 )
    {
        trace_t mainout(&p->mainoutti);
        for (auto j=0u;j<p->model_dist_count;j++)
        {
            traceinfo_t oi{ti.nsamples, ti.ntraces, 1, 0};
            trace_t outputtrace(&oi);
            mainout._samples[j] = 0;
            calc_output(p->n, &outputtrace, j, &mainout._samples[j]);
        }
        if (p->model) 
        {
            std::vector<winner_t> winners;
            p->model->pick_best(&mainout, winners);
            std::stringstream ss;
            for (auto& winner : winners)
            {
                ss << winner.winner_name << "(" << winner.best_corr << "), ";
            }
            LogInfo("winners %s\n", ss.str().c_str());
        }
        else
        {
            p->mainoutput->on_result_trace(&mainout);    
        }
    }
    
}
void corr_analysis_t::calc_output(uint32_t outputtraces, trace_t* traces, 
        uint32_t model_dist_idx, double* maxcorrelation)
{
    auto& cs_samples = pimpl.get()->cs_sample_dists;
    auto& cs_model_samples = pimpl.get()->cs_model_dists;
    auto& c_modelsamples = pimpl.get()->c_modelsamples;
    
    for (auto i=0u;i<pimpl.get()->traceinfo.nsamples;i++)
    {
        double b = 0;
        double a = sqrt(cs_samples[i].cs2 * cs_model_samples[model_dist_idx].cs2);
        
        if (a == 0.0)
        {
            b = 0;
        }
        else
        {
            b = c_modelsamples(model_dist_idx, i) / a;

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
    if (!p->model)
    {
        p->mainoutput->finit();
    }

}
corr_analysis_t::~corr_analysis_t() 
{
    
}
