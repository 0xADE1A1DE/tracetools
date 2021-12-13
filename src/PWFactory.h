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
#include "TraceReader.h"
#include "TraceWriter.h"
#include "Analysis.h"
#include "CmdPipeline.h"
class pwfactory_t
{
public:
    static pwfactory_t* get_instance();
    tracereader_t* new_tracereader(const std::string& kind);
    tracewriter_t* new_tracewriter(const std::string& kind);
    analysis_t* new_analysis(const std::string& kind);
    cmd_pipeline_t* new_cmdpipeline(const std::string& kind);
};
