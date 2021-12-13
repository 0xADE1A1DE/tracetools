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

#include "Log.h"
#include "AnalysisOuput.h"
#include "WorkContext.h"
#include "SecondOrderTTestAnalysis.h"

typedef double real_t;

struct cs_t
{
    real_t mean;
    real_t cs2;
    real_t cs3;
    real_t cs4;
    cs_t() :
        mean(0.0),
        cs2(0.0),
        cs3(0.0),
        cs4(0.0)
    {}
};
struct copyinfo_t
{
    uint32_t n;
    std::vector<cs_t> copycs;
};
struct second_order_ttest_analysis_priv_t
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
};

static int group(const trace_t* trace)
{
    return (int)trace->_data[0];
}
#define POW2(_x) \
    ((_x )* (_x))
    
second_order_ttest_analysis_t::second_order_ttest_analysis_t(const second_order_ttest_analysis_t&) = default;
second_order_ttest_analysis_t::second_order_ttest_analysis_t(second_order_ttest_analysis_t&&) = default;
second_order_ttest_analysis_t::second_order_ttest_analysis_t()
{
    pimpl.init();
}
void second_order_ttest_analysis_t::init(const traceinfo_t *traceinfo, 
                                        analysis_output_t* output, 
                                        const analysis_opts_t& opts)
{
    pimpl.get()->traceinfo = *traceinfo;
    pimpl.get()->cs[0].resize(traceinfo->nsamples);
    pimpl.get()->cs[1].resize(traceinfo->nsamples);
    pimpl.get()->output = output;
    
    pimpl.get()->varoutput0 = output->add_sub_output("var-0", "numpy");
    pimpl.get()->varoutput1 = output->add_sub_output("var-1", "numpy");
    
    traceinfo_t tout = *traceinfo;
    tout.ndata = 0;
    tout.nterms = 1;
    output->init(&tout);
    pimpl.get()->varoutput0->init(&tout);
    pimpl.get()->varoutput1->init(&tout);
    
    pimpl.get()->totalproctraces = 0;
    pimpl.get()->n.resize(2);
    
    pimpl.get()->output_at = wc_gopts_get("at_each_ntraces").get_int();
}

void second_order_ttest_analysis_t::trace_submit(const trace_t *trace)
{
    auto& ti = pimpl.get()->traceinfo;
    auto& cs = pimpl.get()->cs[group(trace)];
    auto p = pimpl.get();
    int g = group(trace);
    uint32_t n = p->n[g]+1;
    real_t r_n = (real_t) n;
    
    real_t nx = 1.0/r_n;
    real_t nn = (r_n-1.0)/r_n;
    real_t n3 = r_n*r_n*r_n;
    real_t nn3 = nn*nn*nn;
    real_t nn4 = nn3*nn;
    real_t nx3 = (r_n-1.0)/n3;
    real_t nx4 = (nx3)/r_n;
    
    if (n == 1)
    {
        for (auto i=0u;i<ti.nsamples;i++)
        {
            cs[i].mean = trace->_samples[i];
            cs[i].cs2 = 0.0;
            cs[i].cs3 = 0.0;
            cs[i].cs4 = 0.0;
        }
    }
    else
    {
        for (auto i=0u;i<ti.nsamples;i++)
        {
            real_t delta= trace->_samples[i] - cs[i].mean;
            real_t delta2 = delta*delta;
            real_t delta3 = delta2*delta;
            real_t delta4 = delta3*delta;
            cs[i].mean = cs[i].mean + delta * nx;
            cs[i].cs2 = cs[i].cs2 + delta * delta * nn;
            cs[i].cs3 = cs[i].cs3 - 3.0*delta*cs[i].cs2/n + delta3*(nn3-nx3);
            cs[i].cs4 = cs[i].cs4 - 4.0*delta*cs[i].cs3/n + 6.0*delta2*cs[i].cs2/(n*n) + delta4*(nn4+nx4);
            
            if (std::isnan(cs[i].cs2) || std::isnan(cs[i].cs3) || std::isnan(cs[i].cs4))
            {
                printf("aa %f %f %f\n", cs[i].cs2 , cs[i].cs3, cs[i].cs4 );
                std::abort();
            }
            //printf("aa %f %f %f\n", cs[i].cs2 , cs[i].cs3, cs[i].cs4 );
        }
    }
    
    uint32_t m = p->output_at/2;
    if (n % m == 0)
    {
        auto ci = new copyinfo_t();
        ci->n = n;
        ci->copycs = cs;
        
        p->copyqueue[g].push_back(ci);
        LogInfo("xx %d %d\n",g, n);
    }
    // calc t-test values at each interval and submit to analysis_output_t
    if ( p->copyqueue[0].size() > 0 && 
         p->copyqueue[1].size() > 0 &&
        p->copyqueue[0].front()->n == p->copyqueue[1].front()->n )
    {
        LogInfo("push output %u %lu %lu\n",p->copyqueue[0].front()->n, 
                p->copyqueue[0].size(), p->copyqueue[1].size());
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
        
        delete p->copyqueue[0].front();
        delete p->copyqueue[1].front();
        
        p->copyqueue[0].pop_front();
        p->copyqueue[1].pop_front();
    }
    
    p->n[g]++;
}
void second_order_ttest_analysis_t::var_output(uint32_t outputtraces, trace_t *trace, int group)
{
    auto& cs = pimpl.get()->copyqueue[group].front()->copycs;
    for (auto index=0u;index<pimpl.get()->traceinfo.nsamples;index++)
    {
        trace->_samples[index] = cs[index].cs2 / outputtraces;
    }
}
void second_order_ttest_analysis_t::calc_output(uint32_t outputtraces, trace_t *trace)
{
    auto& cs0 = pimpl.get()->copyqueue[0].front()->copycs;
    auto& cs1 = pimpl.get()->copyqueue[1].front()->copycs;
    
    for (auto index=0u;index<pimpl.get()->traceinfo.nsamples;index++)
    {
        double cm4_0 = (cs0)[index].cs4/ (outputtraces);
        double cm4_1 = (cs1)[index].cs4/ (outputtraces);
        double cm2_0 = (cs0)[index].cs2/ (outputtraces);
        double cm2_1 = (cs1)[index].cs2/ (outputtraces);
        // Schneider et al. Leakage Assessment Methodology Eq. (5)
        double s2_0 = (cm4_0 - POW2( cm2_0 ));
        double s2_1 = (cm4_1 - POW2( cm2_1 ));
        
        double a = sqrt(outputtraces)*(cm2_0 - cm2_1);
        double b = sqrt(s2_0 + s2_1);
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
            a = 5.0;
            std::abort();
        }
        trace->_samples[index] = a;
    }
}
void second_order_ttest_analysis_t::finit()
{
    auto p = pimpl.get();
    LogInfo("groups %u %u\n",p->n[0],p->n[1]);
    // finish output
    p->output->finit();
}
second_order_ttest_analysis_t::~second_order_ttest_analysis_t()
{
}
