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
#include "CmdPipeline.h"
#include "Stackable.h"

#include <string>
#include <vector>
class cp_stack_t : public cmd_pipeline_t
{
public:
    static cp_stack_t* make_instance(const std::string& name);
    void get_info(cmd_pipeline_info_t& info);
    
    void init(const traceinfo_t *traceinfo, pwtermset_mode_t mode);
    void stack_add(stackable_t* cp, int pass);

    void pass_begin(int pass);
    void process(const trace_t* trace);
    void pass_end(int pass);
    void finit();
private:
    std::vector<std::vector<stackable_t*>> _multistack;
    uint32_t _passes;
    uint32_t _current_pass;
};

