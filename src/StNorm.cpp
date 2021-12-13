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
#include "StNorm.h"
#include "MeanAnalysis.h"
#include "AnalysisOuput.h"
#include "StMean.h"
#include "Trace.h"
#include "TraceUtils.h"
#include "WorkContext.h"
#include "PWFactory.h"
#include "TraceReader.h"
struct st_norm_priv_t
{
    st_mean_t* prerunmeans;
    traceinfo_t traceinfo;
    traceinfo_t winti;
    trace_t outtrace;
};
st_norm_t::st_norm_t(st_mean_t* prerunmeans)
{
    pimpl.init();
    pimpl.get()->prerunmeans = prerunmeans;
}


static void copydata(const trace_t *fromtrace, const traceinfo_t *fromti,
        trace_t *totrace, const traceinfo_t *toti)
{
    if (fromti->ndata > 0)
    {
        if (fromti->ndata == toti->ndata)
        {
            memcpy(&totrace->_data[0], &fromtrace->_data[0], toti->ndata * sizeof(uint8_t));
        }
        else
        {
            LogErrDie("Can't copy data due to data buffer size mismatch!\n");
        }
    }
}

void st_norm_t::init(const traceinfo_t* inti, traceinfo_t* outti)
{
    pimpl.get()->traceinfo = *inti;
    *outti = *inti;
}
const trace_t* st_norm_t::process(const trace_t* trace)
{
    auto p = pimpl.get();
    p->outtrace.~trace_t();
    new (&p->outtrace) trace_t();
    p->outtrace.init(&p->traceinfo);
    int g = traceutils_group(trace);
    for (auto i=0u;i<p->traceinfo.nsamples;i++)
    {
        p->outtrace._samples[i] = trace->_samples[i] - p->prerunmeans->get_mean(g, i);
    }
    copydata(trace, &pimpl.get()->traceinfo, &p->outtrace, &pimpl.get()->traceinfo);
    return &p->outtrace;
}
st_norm_t::~st_norm_t()
{}
