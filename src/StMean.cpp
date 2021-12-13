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
#include "StMean.h"
#include "MeanAnalysis.h"
#include "AnalysisOuput.h"
#include "WorkContext.h"
#include "PWFactory.h"
#include "TraceReader.h"
struct st_mean_priv_t
{
    mean_analysis_t mean;
    traceinfo_t traceinfo;
    traceinfo_t winti;
};
st_mean_t::st_mean_t()
{
    pimpl.init();
}

void st_mean_t::init(const traceinfo_t* inti, traceinfo_t* outti)
{
    pimpl.get()->mean.init(inti, nullptr);
    outti->nsamples = 0; // return nothing   
}
const trace_t* st_mean_t::process(const trace_t* trace)
{
    pimpl.get()->mean.trace_submit(trace);
    return nullptr;
}
double st_mean_t::get_mean(int group, int sample)
{
    return pimpl.get()->mean.getmean(group, sample);
}
st_mean_t::~st_mean_t()
{}
