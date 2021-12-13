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
#include <string>
#include <cstddef>
#include "PImpl.h"
#include "Trace.h"
struct analysis_output_priv_t;

class analysis_output_t
{
public:
    analysis_output_t();
    analysis_output_t(const analysis_output_t& a);
    analysis_output_t(analysis_output_t&&);
    void init(const char* outputfile, const std::string& kind);
    analysis_output_t* add_sub_output(const std::string& id, const std::string& kind);
    analysis_output_t* get_sub_output(size_t index);
    void init(const traceinfo_t* traceinfo);
    void on_result_trace(const trace_t* trace);
    void update_written_count();
    size_t get_written_count();
    size_t finit();
    ~analysis_output_t();
private:
    PImpl<analysis_output_priv_t> pimpl;
};
