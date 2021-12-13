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
#include "Trace.h"
void pwpass_update_properties();
void pwpass_moving_average(trace_t *trace);
void pwpass_add_padding(const double* insamples, uint32_t nin, double* outsamples, 
        uint32_t nout, uint16_t pad_amount, double pad_value);
void pwpass_compress(const double* insamples, uint32_t nin, double* outsamples,
        uint32_t nout, uint32_t window, uint32_t overlap);
