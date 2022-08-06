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
#include "TraceUtils.h"
#include "Log.h"
#include "Trace.h"
#include "WorkContext.h"

int __pwtraceutils_data_groups = -1;


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

int traceutils_groups_count()
{
    if (__pwtraceutils_data_groups == -1)
    {
        if (wc_gopts_has("data_groups_count"))
        {
            __pwtraceutils_data_groups = wc_gopts_get("data_groups_count").get_int();
            /*if (__pwtraceutils_data_groups == 0)
            {
                LogErrDie("Group count must be non zero\n");
            }*/
        }
        else
        {
            __pwtraceutils_data_groups = 2;
        }
    }
    return __pwtraceutils_data_groups;
}
void traceutils_extract_term(const trace_t* trace, trace_t *termtrace, uint32_t term_id,
                             const traceinfo_t* traceinfo, const traceinfo_t* termtraceinfo)
{
    auto nsamples = traceinfo->nsamples;
    if (term_id >= traceinfo->nterms)
    {
        LogErr("invalid term id\n");
    }
    
    for (auto i=0u;i<nsamples;i++)
    {
        termtrace->_samples[i] = trace->_samples[nsamples*term_id + i];
    }

    copydata(trace, traceinfo, termtrace, termtraceinfo);
}
uint32_t traceutils_biv_combine(const trace_t *trace, trace_t *combinedtrace, 
                           const traceinfo_t* traceinfo, const traceinfo_t* combinedtraceinfo)
{
    uint32_t nsamples = traceinfo->nsamples;
    uint32_t cc=0;

    for (auto i=0u;i<nsamples;i++)
    {
        for (auto j=i+1;j<nsamples;j++)
        {
            combinedtrace->_samples[cc] = trace->_samples[i]*trace->_samples[j]; 
            cc ++;
        }
    }

    copydata(trace, traceinfo, combinedtrace, combinedtraceinfo);
    return cc;
}
uint32_t traceutils_triv_combine(const trace_t *trace, trace_t *combinedtrace, 
                           const traceinfo_t* traceinfo, const traceinfo_t* combinedtraceinfo)
{
    uint32_t nsamples = traceinfo->nsamples;
    uint32_t cc=0;
    
    for (auto i=0u;i<nsamples;i++)
    {
        for (auto j=i+1u;j<nsamples;j++)
        {
            for (auto k=j+1u;k<nsamples;k++)
            {
                combinedtrace->_samples[cc] = trace->_samples[i]*trace->_samples[j]*trace->_samples[k]; 
                cc ++;
            }
        }
    }
    copydata(trace, traceinfo, combinedtrace, combinedtraceinfo);
    return cc;
}

uint32_t traceutils_biv_combine2h(const trace_t *trace, trace_t *combinedtrace, 
                           const traceinfo_t* traceinfo, const traceinfo_t* combinedtraceinfo)
{
    uint32_t nsamples = traceinfo->nsamples;
    uint32_t nsamples_mid = nsamples/2;
    uint32_t cc=0;
    for (auto i=0u;i<nsamples_mid;i++)
    {
        for (auto j=0u;j<nsamples_mid;j++)
        {
            combinedtrace->_samples[cc] = trace->_samples[i]*trace->_samples[nsamples_mid+j]; 
            cc ++;
        }
    }
    copydata(trace, traceinfo, combinedtrace, combinedtraceinfo);
    return cc;
}
void traceutils_diff(trace_t* a, trace_t* b)
{
    uint32_t nsamples = a->_traceinfo.nsamples;
    for (auto i=0u;i<nsamples;i++)
    {
        a->_samples[i] = a->_samples[i] - b->_samples[i];
    }
}
