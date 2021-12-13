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
#include <stdint.h>
#include <assert.h>

#ifdef __cplusplus
#define CAPI \
    extern "C"
#else
#define CAPI
#endif 
    
#define PWASSERT(_exp, _msg) \
    { \
        if ( (_exp) == false ) \
        { \
            printf("assert failed: '%s' at %s:%u\n", _msg, __FILE__, __LINE__); \
            std::abort(); \
        } \
    }

CAPI int pwutil_get_tracecount(const char* execfile);
CAPI char* pwutil_dirname(char *path);
CAPI int pwutil_abspath(const char* relpath, char* abspath, int abspathlen);
CAPI uint8_t hex_to_byte(const char hex[2]);
CAPI void hexstr_to_bytebuf(const char* hexstr, uint8_t* bytebuf);
CAPI void bytebuf_to_hexstr(const uint8_t* bytebuf, int bytebuflen, char* hexstr);
