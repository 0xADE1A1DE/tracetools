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

#include "LeakageModel.h"

class aes_model_t : public leakage_model_t {
public:
    void init(const traceinfo_t* ti, leakage_t** leakage);
    void model_leakage(const uint8_t* data, leakage_t* leakage);
    void pick_best(const trace_t* corr, std::vector<winner_t>& winners);
private:
    const uint8_t* get_key(const uint8_t* data);
    const uint8_t* get_plain(const uint8_t* data);
    const uint8_t* get_cipher(const uint8_t* data);

    std::vector<uint8_t> cipher;
    traceinfo_t _ti;
};