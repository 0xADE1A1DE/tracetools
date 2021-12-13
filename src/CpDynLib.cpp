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
#include "CpDynLib.h"
#include "AnalysisOuput.h"
#include "Log.h"
#include "Variant.h"
#include "WorkContext.h"
#include "PWFactory.h"
#include "TraceReader.h"
#include "TraceWriter.h"

#include <dlfcn.h>
#include <cstdio>
#include <sstream>

struct cp_dynlib_priv_t
{
    analysis_output_t* ao;
    void* libhandle;
    std::string ext;
    std::string filename;
    tracereader_t* reader;
    tracewriter_t* writer;
    traceinfo_t traceinfo;
    traceinfo_t winti;
    wc_gopts_t opts_map;

    void (*pwdl_on_init)(uint32_t ntraces, uint32_t nsamples, uint32_t ndata);
    void (*pwdl_on_trace)(const double* tracein, const uint8_t* tracedatain, double** traceout, uint8_t** tracedataout);
    void (*pwdl_on_finit)();
};

cp_dynlib_t::cp_dynlib_t()
{
    pimpl.init();
}
void cp_dynlib_t::init(const traceinfo_t *traceinfo, pwtermset_mode_t mode)
{   
    pimpl.get()->ao = new analysis_output_t[1];
    std::string libname = wc_gopts_get("library_name").get_string();
    auto& lh = pimpl.get()->libhandle;
    lh = dlopen(libname.c_str(), RTLD_NOW);
    dlerror();
    char *error=nullptr;
    pimpl.get()->pwdl_on_init = (decltype(pimpl.get()->pwdl_on_init))dlsym(lh, "pwdl_on_init");
    pimpl.get()->pwdl_on_trace = (decltype(pimpl.get()->pwdl_on_trace))dlsym(lh, "pwdl_on_trace");
    pimpl.get()->pwdl_on_finit = (decltype(pimpl.get()->pwdl_on_finit))dlsym(lh, "pwdl_on_finit");
    error = dlerror();
    if (error)
    {
        LogErrDie("%s\n", error);
    }
    pimpl.get()->pwdl_on_init(traceinfo->ntraces, 
            traceinfo->nsamples * traceinfo->nterms,
            traceinfo->ndata);
    pimpl.get()->writer = pwfactory_t::get_instance()->new_tracewriter("npy");
    /*
    for (int i=r_begin;i<=r_end;i++)
    {
        int normidx = i;
        std::stringstream ssaofile;
        ssaofile<<"dynlibout-"<<normidx<<".npy";

        std::string outfilepath = wc_generate_path(traceinfo->title.c_str(), ssaofile.str().c_str());
        pimpl.get()->ao[normidx].init(outfilepath.c_str(),"numpy");
        pimpl.get()->corr[normidx].init(traceinfo, &pimpl.get()->ao[normidx], {{"corr_data_index", i}});
    }*/
    traceinfo_print("", traceinfo);
}

void cp_dynlib_t::init(const char* filename)
{
}
void cp_dynlib_t::process()
{ 
}
void cp_dynlib_t::process(const trace_t* trace)
{
    double* outsamples=nullptr;
    uint8_t* outdata=nullptr;
    pimpl.get()->pwdl_on_trace(trace->_samples.getptr(), trace->_data.getptr(),
            &outsamples,
            &outdata);
    if (outsamples)
    {
        
    }
}
void cp_dynlib_t::finit()
{
    pimpl.get()->pwdl_on_finit();
    dlclose(pimpl.get()->libhandle);
    pimpl.get()->writer->finit(nullptr);
}
cp_dynlib_t::~cp_dynlib_t()
{
}
