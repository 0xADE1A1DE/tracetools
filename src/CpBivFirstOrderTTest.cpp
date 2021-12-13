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
#include "CpBivFirstOrderTTest.h"
#include "FirstOrderTTestAnalysis.h"
#include "MeanAnalysis.h"
#include "AnalysisOuput.h"
#include "TraceUtils.h"
#include "PWFactory.h"
#include "TraceWriter.h"
#include "TraceReader.h"
#include "Log.h"
#include "Trace.h"
#include "WorkContext.h"
struct cp_biv_first_order_ttest_priv_t
{
    int ncombsamples;
    traceinfo_t traceinfo;
    first_order_ttest_analysis_t fottest;
    analysis_output_t ao;
    mean_analysis_t meanana;
    tracewriter_t* writer;
    pwtermset_mode_t mode;
};

cp_biv_first_order_ttest_t::cp_biv_first_order_ttest_t()
{
    pimpl.init();
}
void cp_biv_first_order_ttest_t::init(const traceinfo_t *traceinfo, pwtermset_mode_t mode)
{ 
    pimpl.get()->traceinfo = *traceinfo;
    pimpl.get()->meanana.init(traceinfo, nullptr);
    auto p = pimpl.get();
    
    //std::vector<char> dumpfilepath(255);
    //snprintf(dumpfilepath.data(), 254, "%s-dump.npy", traceinfo->title.c_str());
    std::string dumpfilepath = wc_generate_path(traceinfo->title.c_str(), "dump.npy");
    pimpl.get()->writer = pwfactory_t::get_instance()->new_tracewriter("numpy");
    pimpl.get()->writer->open_file(dumpfilepath.c_str(), traceinfo, {{ "termsnpy", ""}} );
}
void cp_biv_first_order_ttest_t::process(const trace_t* trace)
{
    auto p = pimpl.get();
    
    // write
    p->writer->write_trace(trace);
    // submit to mean calc
    p->meanana.trace_submit(trace);
    
}
void cp_biv_first_order_ttest_t::finit()
{
    auto p = pimpl.get();
    
    p->writer->finit(nullptr);
    delete p->writer;
    p->meanana.finit();
    
    //std::vector<char> dumpfilepath(255);
    //snprintf(dumpfilepath.data(), 254, "%s-dump.npy", p->traceinfo.title.c_str());
    std::string dumpfilepath = wc_generate_path(p->traceinfo.title.c_str(), "dump.npy");
    tracereader_t* reader = pwfactory_t::get_instance()->new_tracereader("numpy");
    traceinfo_t readtraceinfo;
    reader->open_file(dumpfilepath.c_str(), &readtraceinfo);
    readtraceinfo.title = p->traceinfo.title;
    
    traceinfo_t fottesttraceinfo = readtraceinfo;
    fottesttraceinfo.title = p->traceinfo.title;
    fottesttraceinfo.nsamples = (readtraceinfo.nsamples - 1) * readtraceinfo.nsamples / 2;
    fottesttraceinfo.nterms = 1;
    
    //std::vector<char> outfilepath(255);
    //snprintf(outfilepath.data(), 254, "%s-biv-fottest.npy", readtraceinfo.title.c_str());
    std::string outfilepath = wc_generate_path(p->traceinfo.title.c_str(), "biv-fottest.npy");
    
    pimpl.get()->ao.init(outfilepath.c_str(),"numpy");
    pimpl.get()->fottest.init(&fottesttraceinfo, &pimpl.get()->ao);
    traceinfo_print("out",&fottesttraceinfo);

    while (true)
    {
        trace_t trace;
        if (reader->read_trace(&trace) == false)
        {
            break;
        }
        
        // submit to first order 
        traceinfo_t term0traceinfo = readtraceinfo;
        
        term0traceinfo.nterms = 1;
        trace_t term0trace(&term0traceinfo);
        traceutils_extract_term(&trace, &term0trace, readtraceinfo.nterms -1, 
                                &readtraceinfo, &term0traceinfo);
        
        // mean centered
        int g = traceutils_group(&term0trace);
        
        for (auto i=0u;i<term0traceinfo.nsamples;i++)
        {
            term0trace._samples[i] -= pimpl.get()->meanana.getmean(g, i);
        }
                
        trace_t combinedtr(&fottesttraceinfo);
        auto hh = traceutils_biv_combine(&term0trace, &combinedtr, 
                               &term0traceinfo, &fottesttraceinfo);
        assert( fottesttraceinfo.nsamples == hh );                 
        
        p->fottest.trace_submit(&combinedtr);
        
    }
    delete reader;
    p->fottest.finit();
    
}

cp_biv_first_order_ttest_t::~cp_biv_first_order_ttest_t()
{
    
}
