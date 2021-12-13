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
#include "CpConvert.h"

#include "Passes.h"
#include "AnalysisOuput.h"
#include "WorkContext.h"
#include "PWFactory.h"
#include "TraceWriter.h"
#include "Passes.h"
struct cp_convert_priv_t
{
    tracewriter_t* writer;
    traceinfo_t traceinfo;
    traceinfo_t wti;
};
cp_convert_t::cp_convert_t()
{
    pimpl.init();
}
void cp_convert_t::init(const traceinfo_t *traceinfo, pwtermset_mode_t mode)
{
    wc_path_gen_t pathgen(traceinfo->title);
    pathgen.add("conv");
    std::string target = wc_gopts_get("convert_to").get_string();
    pimpl.get()->writer = pwfactory_t::get_instance()->new_tracewriter(target);
    std::string targetext(".");
    targetext += target;
    
    pimpl.get()->writer->open_file(pathgen.getpath(targetext).c_str(), traceinfo, {});
    pimpl.get()->traceinfo = *traceinfo;
    pimpl.get()->wti = *traceinfo;
    pimpl.get()->wti.ntraces = 0;

    traceinfo_print("", traceinfo);
}

void cp_convert_t::init(const char* filename)
{
}
void cp_convert_t::process()
{ 

}
void cp_convert_t::process(const trace_t* trace)
{    
    pwpass_moving_average(const_cast<trace_t*>(trace)); 
    pimpl.get()->writer->write_trace(trace);
    pimpl.get()->wti.ntraces++;
}
void cp_convert_t::finit()
{
    pimpl.get()->writer->finit(&pimpl.get()->wti);
}
cp_convert_t::~cp_convert_t()
{}
