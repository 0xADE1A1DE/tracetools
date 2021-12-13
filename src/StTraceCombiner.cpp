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
#include "StTraceCombiner.h"
#include "Passes.h"
#include "Trace.h"
#include "TraceUtils.h"
#include "WorkContext.h"
void st_trace_combiner_t::init(const traceinfo_t* inti, traceinfo_t* outti)
{
    if (!wc_gopts_has("combinations"))
    {
        LogErrDie("'combinations' option is not set\n");
    }
    bool wind = wc_gopts_has("combinations_window");
    switch (wc_gopts_get("combinations").get_int())
    {
    case 1:
        _comb = new trace_combiner_t<1>(inti);
        break;
    case 2:
        _comb = new trace_combiner_t<2>(inti);
        break;
    case 3:
        if (!wind)
        {
            _comb = new trace_combiner_t<3>(inti);
        }
        else
        {
            _comb = new trace_combiner_wind_t<3>(inti);
        }
        break;
    default:
        LogErrDie("unsupported combination count\n");
    }
    *outti = _comb->get_outtraceinfo();
    _outti = *outti;
    _inti = *inti;
}

trace_t* st_trace_combiner_t::process(const trace_t* trace)
{
    _outtrace.~trace_t();
    new (&_outtrace) trace_t();
    _outtrace.init(&_outti);
    _comb->combine(trace, &_outtrace);
    return &_outtrace;
}

template<>
trace_combiner_t<1>::trace_combiner_t(const traceinfo_t* intraceinfo)
{
    inti = *intraceinfo;
    outti = *intraceinfo;
}

template<>
traceinfo_t trace_combiner_t<1>::get_outtraceinfo()
{
    return outti;
}

template<>
void trace_combiner_t<1>::init_combined_trace(trace_t* trace)
{
    trace->init(&outti);
}

template<>
void trace_combiner_t<1>::combine(const trace_t* intrace, trace_t* outtrace)
{
    intrace->copyto(outtrace, 0);
}

template<>
trace_combiner_t<2>::trace_combiner_t(const traceinfo_t* intraceinfo)
{
    inti = *intraceinfo;
    outti = *intraceinfo;
    outti.nsamples = outti.nsamples * (outti.nsamples -1)/2;
}

template<>
traceinfo_t trace_combiner_t<2>::get_outtraceinfo()
{
    return outti;
}

template<>
void trace_combiner_t<2>::init_combined_trace(trace_t* trace)
{
    trace->init(&outti);
}

template<>
void trace_combiner_t<2>::combine(const trace_t* intrace, trace_t* outtrace)
{
    traceutils_biv_combine(intrace, outtrace, &inti, &outti);
}


template<>
trace_combiner_t<3>::trace_combiner_t(const traceinfo_t* intraceinfo)
{
    inti = *intraceinfo;
    outti = *intraceinfo;
    outti.nsamples = outti.nsamples * (outti.nsamples -1) * (outti.nsamples -2) / 6;
}

template<>
traceinfo_t trace_combiner_t<3>::get_outtraceinfo()
{
    return outti;
}

template<>
void trace_combiner_t<3>::init_combined_trace(trace_t* trace)
{
    trace->init(&outti);
}

template<>
void trace_combiner_t<3>::combine(const trace_t* intrace, trace_t* outtrace)
{
    traceutils_triv_combine(intrace, outtrace, &inti, &outti);
}

template<>
trace_combiner_wind_t<3>::trace_combiner_wind_t(const traceinfo_t* intraceinfo)
{
    inti = *intraceinfo;
    outti = *intraceinfo;
    uint32_t wind = outti.nsamples /3;
    outti.nsamples = wind * wind * wind;
}

template<>
traceinfo_t trace_combiner_wind_t<3>::get_outtraceinfo()
{
    return outti;
}

template<>
void trace_combiner_wind_t<3>::init_combined_trace(trace_t* trace)
{
    trace->init(&outti);
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

template<>
void trace_combiner_wind_t<3>::combine(const trace_t* intrace, trace_t* outtrace)
{
    uint32_t nsamples = inti.nsamples/3;
    uint32_t cc=0;
    
    for (auto i=0u;i<nsamples;i++)
    {
        for (auto j=1*nsamples;j<2*nsamples;j++)
        {
            for (auto k=2*nsamples;k<3*nsamples;k++)
            {
                outtrace->_samples[cc] = intrace->_samples[i]*intrace->_samples[j]*intrace->_samples[k]; 
                cc ++;
            }
        }
    }
    copydata(intrace, &inti, outtrace, &outti);
}
