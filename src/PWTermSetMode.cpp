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
#include "PWTermSetMode.h"
#include "PWConfig.h"
#include <string.h>
pwtermset_mode_t pwtermset_mode_from_config()
{
    const char* mode = pwconfig_get_value("TS_MODE");
    pwtermset_mode_t ret = TS_ONLINE;
    
    if (mode == nullptr)
    {
        return ret;
    }

    if (strcmp("online", mode) == 0)
    {
	ret = TS_ONLINE;
    }
    else if (strcmp("record", mode) == 0)
    {
        ret = TS_RECORD;
    }
    else if (strcmp("ignore", mode) == 0)
    {
        ret = TS_IGNORE; 
    }
    return ret;
}
