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
#include "Util.h"

#include <stdint.h>

typedef struct _pwmodel_i pwmodel_i;

typedef struct _pwmodel_input
{
    uint32_t op1;
    uint32_t op2;
    uint16_t reg_op1;
    uint16_t reg_op2;
    uint32_t inst_id;
    uint32_t result;
    uint64_t pc;
    void* privateptr;
    struct _pwmodel_input* next;
} pwmodel_input_t;

typedef struct _pwmodel
{
    pwmodel_input_t* codesegment_start;
    pwmodel_input_t* codesegment_current;
    pwmodel_i* model;
    uint64_t startpc;
    uint64_t endpc;
    uint32_t currenttrace;
    uint32_t nsamples;
    uint32_t nsamples_first;
    uint32_t ndata_first;
    uint8_t* inputdata;
    uint32_t inputdatalength;
} pwmodel_t;

CAPI pwmodel_i* pwmodel_get_model(const char* modelname);
CAPI void pwmodel_init(pwmodel_t**, const char* execfile);
CAPI void pwmodel_update(pwmodel_t*, pwmodel_input_t*);
CAPI void pwmodel_begin(pwmodel_t*, uint64_t pc);
CAPI void pwmodel_end(pwmodel_t*, uint64_t pc);
CAPI void pwmodel_dump(pwmodel_t*);
CAPI void pwmodel_update_inputdata(pwmodel_t*, uint8_t* inputdata, uint32_t length);
// read only after pwmodel_end is called for first trace and after
// i.e. pwodel_i::on_trace_done
CAPI uint32_t pwmodel_get_sample_count(pwmodel_t*);
CAPI void pwmodel_finit(pwmodel_t*);
