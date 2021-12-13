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
#include "Stackable.h"
#include "PImpl.h"
struct st_static_align_priv_t;
class st_static_align_t : public stackable_t 
{
public:
    void init(const traceinfo_t* inti, traceinfo_t* outti);
    const trace_t* process(const trace_t* trace);
    void finit(); 
    ~st_static_align_t();
private:
    PImpl<st_static_align_priv_t> _pimpl;
};

