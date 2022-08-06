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
#include "StCorr.h"
#include "CorrAnalysis.h"
#include "AnalysisOuput.h"
#include "Log.h"
#include "Trace.h"
#include "Variant.h"
#include "WorkContext.h"
#include "PWFactory.h"
#include "TraceReader.h"
#include "MeanAnalysis.h"
#include "Passes.h"

#include <cstdio>
#include <sstream>

struct st_corr_priv_t
{
    corr_analysis_t corr;
    analysis_output_t ao;
    traceinfo_t traceinfo;
};

st_corr_t::st_corr_t()
{
    pimpl.init();
}
void st_corr_t::init(const traceinfo_t* inti, traceinfo_t* outti) 
{
    pimpl.get()->traceinfo = *inti;
    std::string outfilepath = wc_generate_path(inti->title.c_str(), "corr.npy");
    pimpl.get()->ao.init(outfilepath.c_str(), "numpy");
    pimpl.get()->corr.init(inti, &pimpl.get()->ao);
        
    traceinfo_print("", inti);
}

const trace_t* st_corr_t::process(const trace_t* trace)
{
    auto p = pimpl.get();
    trace_t tempout(&trace->_traceinfo);
    trace->copyto(&tempout, 0);
    pwpass_moving_average(&tempout);
    
    pimpl.get()->corr.trace_submit(&tempout);
    
    return trace;
}
void st_corr_t::finit()
{
    auto p = pimpl.get();
    
    pimpl.get()->corr.finit();
}
st_corr_t::~st_corr_t()
{
}
