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
#include "Trace.h"
#include "Log.h"

std::string __traceopt_empty="";

const std::string& traceopt_empty()
{
    return __traceopt_empty;
}
bool traceopt_has_key(const std::vector<traceopt_t>& opts, const std::string& key)
{
    for (auto opt: opts)
    {
        if (opt.key == key)
        {
            return true;
        }
    }
    return false;
}
const std::string& traceopt_get_value(const std::vector<traceopt_t>& opts, const std::string& key)
{
    for (auto& opt: opts)
    {
        if (opt.key == key)
        {
            return opt.value;
        }
    }
    return traceopt_empty();
}

void traceinfo_print(const char* msg, const traceinfo_t* traceinfo)
{
    LogInfo("%s t:%s,ns:%d,nt:%d,terms:%d,nd:%d\n", msg, traceinfo->title.c_str(),
        traceinfo->nsamples,
        traceinfo->ntraces,
        traceinfo->nterms,
        traceinfo->ndata
    );
}
