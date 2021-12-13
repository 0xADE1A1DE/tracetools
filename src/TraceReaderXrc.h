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
#include "TraceReader.h"
#include "PImpl.h"

struct tracereader_xrc_priv_t;

class tracereader_xrc_t : public tracereader_t
{
public:
    tracereader_xrc_t();
    void open_file(const char* tracepath, traceinfo_t* info) override;
    void seek(size_t traceid) override;
    void rewind() override;
    int read_trace(trace_t* trace) override;
    ~tracereader_xrc_t() override;
private:
    PImpl<tracereader_xrc_priv_t> pimpl;
};
