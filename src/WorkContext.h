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
#include <string>
#include <sstream>
#include <map>

#include "Log.h"
#include "Variant.h"

typedef std::map<std::string, variant_t> wc_gopts_t;

void wc_gopts_merge(const wc_gopts_t& opts);

const variant_t& wc_gopts_get(const std::string& key);

bool wc_gopts_has(const std::string& key);

const variant_t& wc_gopts_get(const std::string& key, const variant_t& defaultvar);

std::string wc_output_dir();

class wc_path_gen_t
{
public:
    wc_path_gen_t(const std::string& title)
    {
        std::string outdir = wc_output_dir();
        if (outdir != "")
        {
            _ss << outdir << "/" << title;
        }
        else
        {
            _ss << title;
        }
    }
    template <typename T>
    void add(T t) 
    {
        _ss << "-" <<t;
    }

    template<typename T, typename... Args>
    void add(T t, Args... args) 
    {
        _ss << "-" <<t;

        add(args...);
    }
    std::string getpath(std::string suffix)
    {
        _ss << suffix;
        return _ss.str();
    }
private:
    std::stringstream _ss;
};
std::string wc_generate_path(const char* title, const char* filesuffix);
