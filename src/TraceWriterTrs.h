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
#include "TraceWriter.h"
#include "PImpl.h"

struct tracewriter_trs_priv_t;

class tracewriter_trs_t : public tracewriter_t
{
public:
    tracewriter_trs_t();
    void open_file(const char* tracepath, const traceinfo_t* info,
                   const std::vector<traceopt_t>& opts = {"termcomb",""}) override;
    void update_traceinfo(const traceinfo_t* newtraceinfo) override;
    int write_trace(const trace_t* trace) override;
    void finit(const traceinfo_t* finaltraceinfo) override;
    ~tracewriter_trs_t() override;
private:
    PImpl<tracewriter_trs_priv_t> pimpl;
};
