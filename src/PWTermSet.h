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

#include <stdint.h>
#include "Trace.h"

struct _pwtsp_i;
struct _pwtermset_impl_t;
struct trace_t;
typedef struct _pwtermset_t
{
    _pwtsp_i* tsp;
    _pwtermset_impl_t* impl;
} pwtermset_t;

void pwtermset_init(pwtermset_t**, const traceinfo_t *traceinfo, const char* tsdumpfile);
void pwtermset_init_from_file(pwtermset_t**, const char* termsetfile);
void pwtermset_process_file(pwtermset_t*);
void pwtermset_bulk_process(pwtermset_t*, double* termsamples);
void pwtermset_set_leakyindexes(pwtermset_t*, int* leakyinds, int leakyindscount);
void pwtermset_trace_submit(pwtermset_t*, trace_t* trace);
void pwtermset_dump_output(pwtermset_t*, const char* outputfile);
void pwtermset_finit(pwtermset_t*);

