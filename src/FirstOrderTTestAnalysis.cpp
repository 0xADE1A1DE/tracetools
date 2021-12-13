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
#include <vector>
#include <string>
#include <cmath>
#include <deque>
#include <omp.h>

#include "Log.h"
#include "AnalysisOuput.h"
#include "WorkContext.h"
#include "FirstOrderTTestAnalysis.h"

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
struct copyinfo_t
{
    uint32_t n;
    std::vector<cs_t> copycs;
};
struct first_order_ttest_analysis_priv_t
{
    traceinfo_t traceinfo;
    analysis_output_t* output;
    analysis_output_t* varoutput0;
    analysis_output_t* varoutput1;
    uint32_t output_at;
    std::vector<cs_t> cs[2];
    std::vector<uint32_t> n;
    std::deque<copyinfo_t *> copyqueue[2];
    uint32_t totalproctraces;
    bool liveoutput;
};

static int group(const trace_t* trace)
{
    return (int)trace->_data[0];
}

first_order_ttest_analysis_t::first_order_ttest_analysis_t(const first_order_ttest_analysis_t&) = default;
first_order_ttest_analysis_t::first_order_ttest_analysis_t(first_order_ttest_analysis_t&&) = default;
first_order_ttest_analysis_t::first_order_ttest_analysis_t()
{
    pimpl.init();
}
void first_order_ttest_analysis_t::init(const traceinfo_t *traceinfo, 
                                        analysis_output_t* output, 
                                        const analysis_opts_t& opts)
{
    pimpl.get()->traceinfo = *traceinfo;
    pimpl.get()->cs[0].resize(traceinfo->nsamples);
    pimpl.get()->cs[1].resize(traceinfo->nsamples);
    pimpl.get()->output = output;
    
    pimpl.get()->varoutput0 = output->add_sub_output("var-0", "numpy");
    pimpl.get()->varoutput1 = output->add_sub_output("var-1", "numpy");
    
    pimpl.get()->output_at = wc_gopts_get("at_each_ntraces").get_int();
    pimpl.get()->liveoutput = wc_gopts_has("live_output");
   
    traceinfo_t tout = *traceinfo;
    tout.ntraces = tout.ntraces/pimpl.get()->output_at;
    tout.ndata = 0;
    tout.nterms = 1;
    output->init(&tout);
    pimpl.get()->varoutput0->init(&tout);
    pimpl.get()->varoutput1->init(&tout);
    
    pimpl.get()->totalproctraces = 0;
    pimpl.get()->n.resize(2);
    
}

void first_order_ttest_analysis_t::trace_submit(const trace_t *trace)
{
    auto& ti = pimpl.get()->traceinfo;
    auto& cs = pimpl.get()->cs[group(trace)];
    auto p = pimpl.get();
    int g = group(trace);
    uint32_t n = p->n[g]+1;
    
    real_t nx = 1.0/n;
    real_t nn = (n-1.0)/n;
    if (n == 1)
    {
        for (auto i=0u;i<ti.nsamples;i++)
        {
            cs[i].mean = trace->_samples[i];
            cs[i].cs2 = 0;
        }
    }
    else
    {

#ifdef PARALLEL
        omp_set_dynamic(0);
        omp_set_num_threads(8);
        #pragma omp parallel for
#endif
        for (auto i=0u;i<ti.nsamples;i++)
        {
            real_t delta= trace->_samples[i] - cs[i].mean;
            cs[i].mean = cs[i].mean + delta * nx;
            // Schneider et al. Leakage Assessment Methodology Eq. (3), d=2
            // $s^2 = CM_2; CM_d = CS_d/n; s^2 = CS_2/n$
            cs[i].cs2 = cs[i].cs2 + delta * delta * nn;
        }
    }
    
    uint32_t m = p->output_at/2;
    if (n % m == 0)
    {
        auto ci = new copyinfo_t();
        ci->n = n;
        /* ?? crash on direct assignment */
        ci->copycs.resize(cs.size());
        ci->copycs = cs;
        p->copyqueue[g].push_back(ci);
    }
    // calc t-test values at each interval and submit to analysis_output_t
    if ( p->copyqueue[0].size() > 0 && 
         p->copyqueue[1].size() > 0 &&
        p->copyqueue[0].front()->n == p->copyqueue[1].front()->n )
    {
        LogInfo("push output %lu %lu %lu\n",
                (unsigned long) p->copyqueue[0].front()->n, 
                (unsigned long) p->copyqueue[0].size(),
                (unsigned long) p->copyqueue[1].size());
        traceinfo_t oi{ti.nsamples, ti.ntraces, 1, 0};
        trace_t outputtrace(&oi);
        uint32_t tracecount = p->copyqueue[0].front()->n;
        
        calc_output(tracecount, &outputtrace);
        
        trace_t vartrace0(&oi);
        var_output(tracecount, &vartrace0, 0);
        p->varoutput0->on_result_trace(&vartrace0);
     
        trace_t vartrace1(&oi);
        var_output(tracecount, &vartrace1, 1);
        p->varoutput1->on_result_trace(&vartrace1);
        
        p->output->on_result_trace(&outputtrace);
    
        if (p->liveoutput)
        {
            p->varoutput0->update_written_count();
            p->varoutput1->update_written_count();
            p->output->update_written_count();
        }

        delete p->copyqueue[0].front();
        delete p->copyqueue[1].front();
        
        p->copyqueue[0].pop_front();
        p->copyqueue[1].pop_front();
    }
    
    p->n[g]++;
}
void first_order_ttest_analysis_t::var_output(uint32_t outputtraces, trace_t *trace, int group)
{
    auto& cs = pimpl.get()->copyqueue[group].front()->copycs;
    for (auto index=0u;index<pimpl.get()->traceinfo.nsamples;index++)
    {
        trace->_samples[index] = cs[index].cs2 / outputtraces;
    }
}
void first_order_ttest_analysis_t::calc_output(uint32_t outputtraces, trace_t *trace)
{
    auto& cs0 = pimpl.get()->copyqueue[0].front()->copycs;
    auto& cs1 = pimpl.get()->copyqueue[1].front()->copycs;
    
    for (auto index=0u;index<pimpl.get()->traceinfo.nsamples;index++)
    {
        // $ t-test = (m_1 - m_2)/sqrt( s_1^2/n + s_2^2/n ) $
        // $ s^2 = CS_2/n $
        double a = outputtraces * ((cs0)[index].mean - (cs1)[index].mean);
        double b = sqrt((cs0)[index].cs2 + (cs1)[index].cs2);
        if (b != 0.0)
        {
            a /= b;
        }   
        else if (a != 0.0)
        {
            // means are different, while the respective variances are 0
            a = 987654321; 
        }
        if (std::isnan(a))
        {
            a = 0;
        }
        trace->_samples[index] = a;
    }
}
void first_order_ttest_analysis_t::finit()
{
    auto p = pimpl.get();
    LogInfo("groups %lu %lu\n", (unsigned long)p->n[0], (unsigned long)p->n[1]);
    // finish output
    p->output->finit();
}
first_order_ttest_analysis_t::~first_order_ttest_analysis_t()
{
}
