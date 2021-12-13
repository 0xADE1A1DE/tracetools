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

class tracereader_t
{
public:
    virtual void open_file(const char* tracepath, traceinfo_t* info) = 0;
    virtual void seek(size_t traceid) = 0;
    virtual void rewind() = 0;
    virtual int read_trace(trace_t* trace) = 0;
    virtual ~tracereader_t() {};   
};
