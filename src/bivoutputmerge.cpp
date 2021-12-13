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
#include "BivMergeOutput.h"
#include "Log.h"
#include <cstdlib>

void showhelp()
{
    LogInfo("Usage: bivoutputmerge <in-file-template> <out-file-name> \
            <nsamples-in-input-trace> <ntraces-in-part-files>\n");
}
int main(int argc, char **argv)
{
    if (argc != 5)
    {
        showhelp();
        return 0;
    }

    biv_merge_output(argv[1], argv[2], 4, 
            (uint64_t)atol(argv[3]), (uint64_t)atol(argv[4]));
    return 0;
}
