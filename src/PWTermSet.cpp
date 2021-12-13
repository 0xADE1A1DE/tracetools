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
#include "IPWTermSetProc.h"
#include "PWTermSet.h"
#include "PWConfig.h"
#include "TraceWriter.h"
#include "PWFactory.h"
#include "CmdPipeline.h"
#include "WorkContext.h"
#include "Log.h"
#include "Util.h"

struct _pwtermset_impl_t
{
    std::string _termsetfile;
    pwtermset_mode_t _mode;
    tracewriter_t* _writer;
    traceinfo_t _traceinfo;
    int _forcecalc;
    cmd_pipeline_t* _pipeline;
    
    _pwtermset_impl_t():
        _writer(nullptr),
        _pipeline(nullptr)
    {}
};

void pwtermset_init(pwtermset_t** pt, const traceinfo_t* traceinfo, const char* tsdumpfile)
{
    *pt = new pwtermset_t;
    auto ptptr = *pt;
    ptptr->impl = new _pwtermset_impl_t();
    auto pti = ptptr->impl;

    pti->_traceinfo = *traceinfo;
    pti->_writer = nullptr;
    
    wc_gopts_t opts = {{"at_each_ntraces", 5000}};
    wc_gopts_merge(opts);
    std::string analysis = pwconfig_get_value("ANALYSIS");
    pti->_pipeline = pwfactory_t::get_instance()->new_cmdpipeline(analysis);
    if (analysis.find("rosita,univ") != std::string::npos)
    {    
        pti->_mode = TS_ONLINE;
    }
    else if (analysis.find("rosita,biv") != std::string::npos)
    {
        pti->_mode = TS_RECORD;
    }
    else if (analysis.find("rosita,triv") != std::string::npos)
    {
        pti->_mode = TS_RECORD;
    }
    else
    {
        LogErrDie("unsupported analysis\n");
    }
    pti->_pipeline->init(traceinfo, pti->_mode);

}
void pwtermset_init_from_file(pwtermset_t **pt, const char* termsetfile)
{
}
void pwtermset_process_file(pwtermset_t* pt)
{
}
void pwtermset_set_leakyindexes(pwtermset_t* pt, int* leakyinds, int leakyindscount)
{
}
void pwtermset_bulk_process(pwtermset_t* pt, double* alltermsamples)
{
}
void pwtermset_trace_submit(pwtermset_t* pt, trace_t* trace)
{
    auto pti = pt->impl;
    PWASSERT(trace->_traceinfo.ndata > 0, "data required");
    PWASSERT(trace->_data.getptr() != nullptr, "data not init");
    // data required due to its usage in differentiation of traces to groups

    pti->_pipeline->process(trace);
}
void pwtermset_dump_output(pwtermset_t* pt, const char* outputfile)
{
}
void pwtermset_finit(pwtermset_t* pt)
{
    if (pt->impl->_pipeline)
    {
        pt->impl->_pipeline->finit();
    }
}
