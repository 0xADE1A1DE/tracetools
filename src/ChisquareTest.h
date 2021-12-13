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

struct chisquare_test_priv_t;

class chisquare_test_t : public analysis_t
{
public:
    struct is_sample_independent {};
    
    chisquare_test_t();
    chisquare_test_t(const chisquare_test_t&);
    chisquare_test_t(chisquare_test_t&&);
    void init(const traceinfo_t *traceinfo, 
              analysis_output_t* output, const analysis_opts_t& opts = {});
    void trace_submit(const trace_t *trace);
    void finit();
    ~chisquare_test_t();
private:
    double calc_chisq_at(int index);
    
    PImpl<chisquare_test_priv_t> _pimpl;
};
