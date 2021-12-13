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
#include "PWTermSetMode.h"
#include "Trace.h"
#include "Log.h"

struct cmd_pipeline_info_t
{
    int passes_count;
};
class cmd_pipeline_t
{
public:
    virtual void get_info(cmd_pipeline_info_t& info) { info = cmd_pipeline_info_t{1}; }
    
    virtual void init(const traceinfo_t *traceinfo, pwtermset_mode_t mode) { LogErrDie("not implemented\n"); }
    virtual void init(const char* filename) { LogErrDie("not implemented\n"); }
    virtual void pass_begin(int pass) {}
    virtual void process() { LogErrDie("not implemented\n"); }
    virtual void process(const trace_t* trace) = 0;
    virtual void pass_end(int pass) {}
    virtual void finit() = 0;
    virtual ~cmd_pipeline_t() {}

};
