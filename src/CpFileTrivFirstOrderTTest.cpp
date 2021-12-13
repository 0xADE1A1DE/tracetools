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
#include "CpFileTrivFirstOrderTTest.h"
#include "FirstOrderTTestAnalysis.h"
#include "MeanAnalysis.h"
#include "AnalysisOuput.h"
#include "TraceUtils.h"
#include "PWFactory.h"
#include "TraceWriter.h"
#include "TraceReader.h"
#include "Log.h"
#include "Passes.h"
#include "Trace.h"
#include "WorkContext.h"
#include "Perf.h"

/***
 * This class is the same as CpBivFirstOrderTTest but a windowing function 
 * run over all samples (window size 8) as it is geared towards real trace_submit
 * rather than simulated traces.
 ***/

struct cp_file_triv_first_order_ttest_priv_t
{
    traceinfo_t traceinfo;
    traceinfo_t windowtrinfo;
    traceinfo_t fottesttraceinfo;
    analysis_t* fottest;
    analysis_output_t ao;
    mean_analysis_t meanana;
    pwtermset_mode_t mode;
    trace_t temptrace;
    trace_t combinedtr;
    int pass;
    uint32_t trid;
};
cp_file_triv_first_order_ttest_t::cp_file_triv_first_order_ttest_t()
{
    pimpl.init();
}
void cp_file_triv_first_order_ttest_t::get_info(cmd_pipeline_info_t& info) 
{ 
    info = cmd_pipeline_info_t{2}; 
}
void cp_file_triv_first_order_ttest_t::init(const traceinfo_t *traceinfo, pwtermset_mode_t mode)
{
    pimpl.get()->traceinfo = *traceinfo;
    pimpl.get()->windowtrinfo = *traceinfo;
    pimpl.get()->meanana.init(traceinfo, nullptr);
    pimpl.get()->temptrace.init(traceinfo);
}
void cp_file_triv_first_order_ttest_t::init(const char* filename)
{ 
}
void cp_file_triv_first_order_ttest_t::pass_begin(int pass)
{
    pimpl.get()->pass = pass;
}
void cp_file_triv_first_order_ttest_t::process()
{
}
void cp_file_triv_first_order_ttest_t::process(const trace_t* trace)
{
    auto p = pimpl.get();
    
    if (p->pass == 0)
    {
        // submit to mean calc
        //trace_t meantr(&p->windowtrinfo);
        //preprocess_trace(&meantr, trace);
        //meantr.print();
        trace_t temptrace(&p->windowtrinfo);
        trace->copyto(&temptrace, 0);
        pwpass_moving_average(&temptrace);
        p->meanana.trace_submit(&temptrace);
    }
    else if (p->pass == 1)
    {
        int g = traceutils_group(trace);
        auto& temptrace = p->temptrace;
        auto& combinedtr = p->combinedtr;
        trace->copyto(&temptrace, 0);
        pwpass_moving_average(&temptrace);
        for (auto i=0u;i<p->windowtrinfo.nsamples;i++)
        {
            temptrace._samples[i] -= p->meanana.getmean(g, i);
        }
        
        auto hh = traceutils_triv_combine(&temptrace, &combinedtr, 
                               &p->windowtrinfo, &p->fottesttraceinfo);
        assert( p->fottesttraceinfo.nsamples == hh );
        
        if (p->trid % 65536 == 0)
            combinedtr.print();

        p->fottest->trace_submit(&combinedtr);
        p->trid ++;
    }
    
}
void cp_file_triv_first_order_ttest_t::pass_end(int pass)
{
    auto p = pimpl.get();
     
    p->trid = 0;
    if (p->pass == 0)
    {
        
        p->meanana.finit();
        
        p->fottesttraceinfo = p->traceinfo;
        p->fottesttraceinfo.nsamples = (p->traceinfo.nsamples - 2) * (p->traceinfo.nsamples - 1) * p->traceinfo.nsamples / 6;
        p->fottesttraceinfo.nterms = 1;

        p->combinedtr.init(&p->fottesttraceinfo);
        
        //std::vector<char> outfilepath(255);
        //snprintf(outfilepath.data(), 254, "%s-triv-fottest.npy", p->traceinfo.title.c_str());
        std::string outfilepath = wc_generate_path(p->traceinfo.title.c_str(), "triv-fottest.npy");
        
        p->ao.init(outfilepath.c_str(),"numpy");
        p->fottest = pwfactory_t::get_instance()->new_analysis("first-order-ttest");
        p->fottest->init(&p->fottesttraceinfo, &p->ao);
        traceinfo_print("out",&p->fottesttraceinfo);
        traceinfo_print("in",&p->traceinfo);
        traceinfo_print("in",&p->windowtrinfo);
    }
    else if (p->pass == 1)
    {
        p->fottest->finit();
    }
}
void cp_file_triv_first_order_ttest_t::finit()
{
    
}

cp_file_triv_first_order_ttest_t::~cp_file_triv_first_order_ttest_t()
{
    
}
