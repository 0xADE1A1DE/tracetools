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

#include "Stackable.h"
#include "StMean.h"
#include "PImpl.h"
struct st_norm_priv_t;

class st_norm_t : public stackable_t
{
public:
    st_norm_t(st_mean_t* prerunmeans);
    void init(const traceinfo_t* inti, traceinfo_t* outti) override;
    const trace_t* process(const trace_t* trace) override;
    ~st_norm_t() override;
private:
    PImpl<st_norm_priv_t> pimpl;
};
