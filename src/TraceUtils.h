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
#pragma once
#include "Log.h"
#include "Trace.h"
extern int __pwtraceutils_data_groups;

/* get trace group: only on standard traces that 
 * have set the first data byte to the group */
inline int traceutils_group(const trace_t* trace)
{
    if (__pwtraceutils_data_groups > 1)
    {
        return (int) trace->_data[0];
    }
    else if (__pwtraceutils_data_groups == -1)
    {
        LogErrDie("data group count not initialized\n");
    }
    return 0;
}

int traceutils_groups_count();

/* extract a given term from a trace */
void traceutils_extract_term(const trace_t* trace, trace_t *termtrace, uint32_t term_id, 
                             const traceinfo_t* traceinfo, const traceinfo_t* termtraceinfo);
/* combine bivariate */
uint32_t traceutils_biv_combine(const trace_t *trace, trace_t *combinedtrace, 
                           const traceinfo_t* traceinfo, const traceinfo_t* combinedtraceinfo);
/* combine bivariate two halves */
uint32_t traceutils_biv_combine2h(const trace_t *trace, trace_t *combinedtrace, 
                           const traceinfo_t* traceinfo, const traceinfo_t* combinedtraceinfo);
/* combine trivariate */
uint32_t traceutils_triv_combine(const trace_t *trace, trace_t *combinedtrace, 
                           const traceinfo_t* traceinfo, const traceinfo_t* combinedtraceinfo);

/* a = a - b */
void traceutils_diff(trace_t* a, trace_t* b);
