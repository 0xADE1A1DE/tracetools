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

#include <stdint.h>
#include <stdlib.h>

#define TS_DUMP_FILE "./termsetdump.npy"

struct _pwtsp_i
{
    virtual void pwtsp_init(int nterms, int nsamples, int ntraces) = 0;
    virtual void pwtsp_bulk_process(double* termsamples) = 0;
    virtual void pwtsp_set_leakyindexes(int* leakyinds, int leakyindscount) { LogErrDie("leaky indexes not implemented\n"); }
    virtual void pwtsp_trace_submit(double* termsamples) = 0;
    virtual void pwtsp_dump_output(const char* outputfile) = 0;
    virtual void pwtsp_finit() = 0;
    virtual ~_pwtsp_i() {}
};
