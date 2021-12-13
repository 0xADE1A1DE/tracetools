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
#include <functional>

#include "CmdPipeline.h"
#include "Trace.h"
#include "PImpl.h"

struct file_processor_priv_t;
class file_processor_t 
{
public:
    typedef std::function<void(trace_t*, const trace_t*)> preprocess_func_t;
    struct range_t
    {
        size_t samplewinstart;
        size_t samplewinlen;
        size_t tracerangestart;
        size_t tracerangelen;
        
        range_t():
            samplewinstart(0)
            ,samplewinlen(0)
            ,tracerangestart(0)
            ,tracerangelen(0)
        {}
    };
    file_processor_t();
    void init(cmd_pipeline_t* cp, const std::string& filename, range_t* range=nullptr);
    const traceinfo_t* get_file_traceinfo();
    void process_window(size_t samplewinstart, size_t samplewinlen);
    void process_range(const range_t& range);
    void process_window(size_t samplewinstart, size_t samplewinlen, preprocess_func_t func);
    void process_all();
    void process_all(preprocess_func_t func);
    void finit();
    
    ~file_processor_t();
private:
    PImpl<file_processor_priv_t> pimpl;
};
