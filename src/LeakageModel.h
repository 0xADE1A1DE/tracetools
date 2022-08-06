// Copyright 2022 University of Adelaide
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
#include <stdint.h>
#include <vector>

#include "Trace.h"

class leakage_t {
public:
    void init(uint32_t dist_count) {
        _values.resize(dist_count);
    }
    float *get_dist_values() {
        return _values.data();
    }
    uint32_t get_dist_count() {
        return _values.size();
    }
protected:
    std::vector<float> _values;
};

struct winner_t {
    float best_corr;
    std::string winner_name;
    uint32_t model_dist_idx;
};

class leakage_model_t {
public:
    virtual void init(const traceinfo_t* ti, leakage_t** leakage) = 0;
    virtual void model_leakage(const uint8_t* data, leakage_t* leakage) = 0;
    virtual void pick_best(const trace_t* corr, std::vector<winner_t>& winners) = 0;
};