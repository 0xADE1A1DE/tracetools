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
#include "IPWModel.h"

typedef struct _pwprops_t pwprops_t;
typedef struct _pwanalyzer_t pwanalyzer_t;
typedef struct _pwtermset_t pwtermset_t;
typedef struct _termcombs_t termcombs_t;

struct elmoextmodel_t : public pwmodel_i
{
public:
    void init(pwmodel_t*, const char*);
    void finit(pwmodel_t*);
    void on_first_trace_done(pwmodel_t*);
    void on_trace_done(pwmodel_t*);
private:
    uint8_t* _data;
    pwprops_t* _props;
    pwanalyzer_t* _power;
    termcombs_t* _tc;
    pwtermset_t* _ts;
    pwtermset_t* _tpower;
    double* _powersamples;
    double* _termsamples;
    int _nsamples;
    int _totaltraces;
};
