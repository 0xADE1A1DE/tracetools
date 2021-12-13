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
#include "PWLeakageParams.h"
#include "Log.h"

#include <vector>
#include <stdint.h>
#include <stdlib.h>

typedef struct _pwaccessor_i pwaccessor_i;

struct _pwleakage_model_i
{
    virtual void pwleakage_init(int nototaltraces) = 0;
    virtual void pwleakage_set_param(pwleakage_model_param param, int value) {}
    virtual void pwleakage_set_param(pwleakage_model_param param, const char* value) {}
    virtual void pwleakage_set_leaky_indexes(int* leakyindexes, int leakyindexescount) { LogErrDie("set leaky indexes not implemented\n"); }
    virtual void pwleakage_on_trace_done(double* samples, uint32_t samplecount) = 0;
    virtual _pwleakage_model_i* pwleakage_merge(_pwleakage_model_i* dest) { LogErrDie("merge not implemented\n"); return nullptr; } 
    virtual uint32_t pwleakage_get_output_size() = 0; 
    virtual double pwleakage_get_output_at(uint32_t index) = 0;
    virtual void pwleakage_get_internals_size(uint32_t* nrows, uint32_t* ncols) { *nrows = 0; *ncols = 0; }
    virtual void pwleakage_get_internals_at(uint32_t row_idx, double* row) {}
    virtual void pwleakage_finit() = 0;
    virtual ~_pwleakage_model_i() {}
};
