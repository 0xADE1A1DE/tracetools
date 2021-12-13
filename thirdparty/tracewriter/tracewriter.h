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
#include <stdio.h>
#include <stdint.h>

#include "tracewriter_config.h"

struct TraceWriter
{
    FILE* file;
    void* impl;
    int fd;
    ssize_t size;
};

//#define TW_FMT_F32 0
//#define TW_FMT_I16 1

#define TW_P_FMT 0
#define TW_P_SAMPLES_PER_CAP 1
#define TW_P_CAPS_PER_ROUND 2
#define TW_P_ROUNDS 3
#define TW_P_FRAC_NUM 4 
#define TW_P_FRAC_DENO 5

#define TW_M_NEW 0
#define TW_M_CONT 1
#define TW_M_READ 2

#if defined(TW_FMT_F32)
#define TW_FMT 0
typedef float tw_t;
#elif defined(TW_FMT_I16)
#define TW_FMT 1
typedef int16_t tw_t;
#else
#error "unsupported format"
#endif
#ifdef __cplusplus
extern "C" {
#endif
void tw_open(struct TraceWriter* tw, const char* file, int mode);
void tw_get_property(struct TraceWriter* tw, int prop, uint32_t* value);
void tw_set_property(struct TraceWriter* tw, int prop, uint32_t value);
ssize_t tw_read_samples(struct TraceWriter* tw, tw_t** samples);
ssize_t tw_write_samples(struct TraceWriter* tw, tw_t* samples, int count);
void tw_close(struct TraceWriter* tw);
#ifdef __cplusplus
}
#endif
