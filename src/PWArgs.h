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
#include "Util.h"

typedef struct _pwargs_t 
{
    char long_sw[32];
    char short_sw[8];
    int arg_start_idx;
    int jump_length;
} pwargs_t;
CAPI void pwargs_print_options(pwargs_t *args, const char* longdescs[], int args_count);
CAPI void pwargs_process(int argc, char** argv, pwargs_t *args, int args_count, int* nosw_args_start);
