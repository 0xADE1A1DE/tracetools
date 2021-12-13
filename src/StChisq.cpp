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
#include "StChisq.h"
#include "FirstOrderTTestAnalysis.h"
#include "AnalysisOuput.h"
#include "PWFactory.h"
#include "TraceUtils.h"
#include "Passes.h"

struct st_chisq_priv_t
{
    analysis_output_t aos;
    analysis_t* chisqests;
    traceinfo_t traceinfo;
    pwtermset_mode_t mode;
};

st_chisq_t::st_chisq_t()
{
    pimpl.init();
}
void st_chisq_t::init(const traceinfo_t* inti, traceinfo_t* outti)
{
    auto p = pimpl.get();
    p->traceinfo = *inti;
    traceinfo_print("dddd", inti);
    outti->nsamples = 0;

    std::vector<char> outfilepath(255);                                                                         
    snprintf(outfilepath.data(), 254, "%s-st-chisq.npy", inti->title.c_str());
    p->chisqests = pwfactory_t::get_instance()->new_analysis("chisquare");
    p->aos.init(outfilepath.data(), "numpy");

    p->chisqests->init(inti, &p->aos);
}
const trace_t* st_chisq_t::process(const trace_t* trace)
{
    auto p = pimpl.get();
    //LogInfo("dd%x\n",trace->_data[0]);
    pwpass_moving_average(const_cast<trace_t*>(trace)); 
    p->chisqests->trace_submit(trace);
    return nullptr;
}
void st_chisq_t::finit()
{
    auto p = pimpl.get();
    
    p->chisqests->finit();
}
st_chisq_t::~st_chisq_t()
{
}
