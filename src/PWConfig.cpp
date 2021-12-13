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
#include "PWConfig.h"
#include "Log.h"
#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#define CFG_FILE "./pwconfig.conf"
static std::map<std::string, std::string> confmap;
static bool loaded=false;
void pwconfig_init()
{
    std::ifstream conf(CFG_FILE,std::ifstream::in);
    LogInfo("config from file\n");
    while (!conf.eof())
    {
        std::string key, val;
        conf >> key >> std::ws>> val >>std::ws;
        LogInfo("%s=%s\n", key.c_str(), val.c_str());
        confmap.insert({key, val});
    }
    loaded = true;
}
void pwconfig_set_value(const char* key, const char* value)
{
    std::string strkey(key);
    std::string strval(value);

    confmap.insert( {strkey, strval} );
    LogInfo("config %s=%s\n", key, value);
    loaded = true;
}
const char* pwconfig_get_value(const char* key)
{
    if (!loaded)
    {
        pwconfig_init();
    }
    std::string strkey(key);
    if (confmap.find(strkey) != confmap.end())
    {   
        return confmap[strkey].c_str();
    }
    return nullptr;
}

int pwconfig_get_value_int(const char* key)
{
    const char* val = pwconfig_get_value(key);
    if (val == nullptr)
    {
        return -1;
    }
    return atoi(val);
}

void pwconfig_finit() 
{
}
