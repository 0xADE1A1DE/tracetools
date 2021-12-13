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
#include "PWArgs.h"
#include "Log.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
static int pwargs_check(pwargs_t* arg, char* arg_val)
{
    if (strcmp(arg->long_sw, arg_val) == 0)
        return 1;
    if (strcmp(arg->short_sw, arg_val) == 0)
        return 2;
    return 0;
}

void pwargs_print_options(pwargs_t *args, const char* longdescs[], int args_count)
{
    int i=0;
    int j=0;
    while (i<args_count)
    {
        if (longdescs[j][0] == '%')
        {
            printf("%s\n",&(longdescs[j])[1]);
            j++;
            continue;
        }
        if (args[i].short_sw[0] == '\0')
        {
            printf("  %s: %s\n", args[i].long_sw, longdescs[j]);
        }
        else
        {
            printf("%s, %s: %s\n", args[i].short_sw, args[i].long_sw, longdescs[j]);
        }
        i++;
        j++;
    }
}
void pwargs_process(int argc, char** argv, pwargs_t* args, int args_count, int* nosw_arg_start)
{
    int i=1;
    *nosw_arg_start = -1;
    int last_swarg = -1;
    for (;i<argc-1;i++)
    {
        int ret = 0;
        for (int j=0;j<args_count;j++)
        {
            ret = pwargs_check(&args[j], argv[i]);
            if (ret)
            {
                args[j].arg_start_idx = i+1;
                last_swarg = i+args[j].jump_length;
                break;
            }
        }
        if ((ret == 0) && (argv[i][0] == '-') && (!isdigit(argv[i][1])))
        {
            LogErrDie("unknown switch %s\n", argv[i]);
        }
    }

    if (last_swarg != -1 && (last_swarg + 1) < argc)
    {
        *nosw_arg_start = last_swarg+1;
    }

    if (last_swarg == -1 && argc > 1)
    {
        if (argv[1][0] != '-')
            *nosw_arg_start = 1;
    }
}

