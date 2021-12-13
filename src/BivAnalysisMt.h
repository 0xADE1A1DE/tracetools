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
#include <functional>

#include "Analysis.h"
#include "Trace.h"
#include "PImpl.h"

struct biv_analysis_mt_priv_t ;

class biv_analysis_mt_t : public analysis_t
{
public:
    
    template<class AnalysisT, class AnalysisOutputT>
    void init_threaded(int nsplits)
    {
        typename AnalysisT::is_sample_independent{};
        _nsplits = nsplits;
        
        _make_analysis = []() { return new AnalysisT(); };
        _make_analysis_output = []() { return new AnalysisOutputT(); };
        
    }
    biv_analysis_mt_t();
    biv_analysis_mt_t(const biv_analysis_mt_t&);
    biv_analysis_mt_t(biv_analysis_mt_t&&);
    void init(const traceinfo_t* traceinfo, 
                      analysis_output_t* output, 
                      const analysis_opts_t& opts = {}) override;
    void trace_submit(const trace_t *trace) override;
    void finit() override;
    ~biv_analysis_mt_t();
private:
    std::function<analysis_t*()> _make_analysis;
    std::function<analysis_output_t*()> _make_analysis_output;
    int _nsplits;
    int _nt;
    
    PImpl<biv_analysis_mt_priv_t> pimpl;
};
