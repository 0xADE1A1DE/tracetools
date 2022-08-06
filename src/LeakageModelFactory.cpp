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

#include "LeakageModelFactory.h"

#include "leakage_models/AESModel.h"

leakage_model_factory_t* leakage_model_factory_t::get_instance()
{
    static leakage_model_factory_t model_fac;
    return &model_fac;
}
leakage_model_t* leakage_model_factory_t::get_model(const std::string& name)
{
    if (name == "aes128" || name == "aes") {
        return new aes_model_t();
    } 
    else 
    {
        LogErrDie("No matching model found\n");
    }
}