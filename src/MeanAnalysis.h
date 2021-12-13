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

#include "Analysis.h"
#include "PImpl.h"

struct mean_analysis_priv_t;
class mean_analysis_t : public analysis_t
{
public:
    mean_analysis_t();
    void init(const traceinfo_t *traceinfo, 
            analysis_output_t* output, 
            const analysis_opts_t& opts = {}) override;
    void trace_submit(const trace_t *trace) override;
    void finit() override;
        
    double getmean(int groupid, int sampleid);
    ~mean_analysis_t();
private:
    PImpl<mean_analysis_priv_t> pimpl;
};
