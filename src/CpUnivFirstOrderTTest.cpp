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
#include "Trace.h"
#include "FirstOrderTTestAnalysis.h"
#include "AnalysisOuput.h"
#include "PWFactory.h"
#include "TraceUtils.h"
#include "WorkContext.h"
#include "CpUnivFirstOrderTTest.h"

struct cp_univ_first_order_ttest_priv_t
{
    std::vector<analysis_output_t> aos;
    std::vector<first_order_ttest_analysis_t> fottests;
    traceinfo_t traceinfo;
    pwtermset_mode_t mode;
};

cp_univ_first_order_ttest_t::~cp_univ_first_order_ttest_t()
{
}
cp_univ_first_order_ttest_t::cp_univ_first_order_ttest_t()
{
    pimpl.init();
}
void cp_univ_first_order_ttest_t::init(const traceinfo_t *traceinfo, pwtermset_mode_t mode)
{
    
    auto p = pimpl.get();
    p->aos.resize(traceinfo->nterms);
    p->fottests.resize(traceinfo->nterms);
    p->mode = mode;
    p->traceinfo = *traceinfo;
    if (p->mode == TS_RECORD && traceinfo->title == "power")
    {
        p->mode = TS_ONLINE;
    }
    
    if (p->mode == TS_RECORD)
    {
        return;
    }
    for (auto i=0u;i<traceinfo->nterms;i++)
    {
        std::vector<char> outsuffix(255);
        snprintf(outsuffix.data(), 254, "fottest-%d.npy", i);
        
        std::string filename = wc_generate_path(traceinfo->title.c_str(), outsuffix.data());
        p->aos[i].init(filename.data(), "numpy");
        
        p->fottests[i].init(traceinfo, &p->aos[i]);
    }
}
void cp_univ_first_order_ttest_t::process(const trace_t* trace)
{
    auto p = pimpl.get();
    if (p->mode == TS_RECORD)
        return;
    for (auto i=0u;i<p->traceinfo.nterms;i++)
    {
        traceinfo_t term0traceinfo = p->traceinfo;
        term0traceinfo.nterms = 1;
        
        trace_t term0trace(&term0traceinfo);
        traceutils_extract_term(trace, &term0trace, i, &p->traceinfo , &term0traceinfo);

        p->fottests[i].trace_submit(&term0trace);
    }
}
void cp_univ_first_order_ttest_t::finit()
{
    auto p = pimpl.get();
    if (p->mode == TS_RECORD)
    {
        return;
    }
    
    
    for (auto i=0u;i<p->traceinfo.nterms;i++)
    {
        p->fottests[i].finit();
    }
}

