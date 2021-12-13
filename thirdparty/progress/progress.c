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
#include "progress.h"
#include <stdio.h>

void progress_init(progress_t* pg, unsigned long max)
{
    pg->perc = 0;
    pg->perc_incr = 1;
    if (max < 10000)
    {
        pg->last = 1;
        pg->mod = 1;
        pg->perc_incr = 10000 / max;
    }
    else
    {
        pg->last = max / 10000;
        pg->mod = max / 10000;
    }
    printf("[  0.00%%]");
    fflush(stdout);
}
void progress_update(progress_t* pg, unsigned long current)
{
    if (current >= pg->last)
    {
        pg->perc += pg->perc_incr;

        printf("\r[%3d.%2d%%]", pg->perc / 100, pg->perc % 100);
        
        fflush(stdout);
        pg->last += pg->mod;
    }
}
void progress_finit(progress_t* pg)
{
    printf("\r[100.00%%]\n");
    fflush(stdout);
}
