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
#include "WorkContext.h"
#include "PWConfig.h"
#include "Log.h"

#include <fcntl.h>
#include <sys/stat.h>

#include <vector>

static wc_gopts_t g_opts_map;

void wc_gopts_merge(const wc_gopts_t& opts)
{
    g_opts_map.insert(opts.begin(), opts.end());
}
const variant_t& wc_gopts_get(const std::string& key, const variant_t& defaultvar)
{
    auto it = g_opts_map.find(key);
    if (it != g_opts_map.end())
    {
        return it->second;
    }
    else
    {
        return defaultvar;
    }
}
bool wc_gopts_has(const std::string& key)
{
    return g_opts_map.end() != g_opts_map.find(key);
}
const variant_t& wc_gopts_get(const std::string& key)
{
    auto it = g_opts_map.find(key);
    if (it != g_opts_map.end())
    {
        return it->second;
    }
    else
    {
        LogErrDie("option not found '%s'\n", key.c_str());
    }
}
std::string wc_generate_path(const char* title, const char* filesuffix)
{
    std::vector<char> path(255);
    const char* outputdir = pwconfig_get_value("OUTPUTDIR");
    if (outputdir)
    {
        struct stat stats;

        stat(outputdir, &stats);
        
        if ((stats.st_mode & F_OK) == 0)
        {
            mkdir(outputdir, 0777);
        }
        
        snprintf(path.data(), 254, "%s/%s-%s", outputdir, title, filesuffix);
    }
    else
    {
        snprintf(path.data(), 254, "%s-%s", title, filesuffix);
    }
    return std::string(path.data());
}

std::string wc_output_dir()
{
    std::vector<char> path(255);
    const char* outputdir = pwconfig_get_value("OUTPUTDIR");
    if (outputdir)
    {
        struct stat stats;

        stat(outputdir, &stats);
        
        if ((stats.st_mode & F_OK) == 0)
        {
            mkdir(outputdir, 0777);
        }
        
        return std::string(outputdir);
    }
    else
    {
        return std::string("");
    }
}

