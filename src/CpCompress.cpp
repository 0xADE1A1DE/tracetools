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
#include "CpCompress.h"
#include "Variant.h"
#include "WorkContext.h"
#include "PWFactory.h"
#include "TraceReader.h"
#include "TraceWriter.h"
#include "Passes.h"
#include <cstdio>
#include <sstream>

struct cp_compress_priv_t
{
    std::string ext;
    std::string filename;
    tracewriter_t* writer;
    traceinfo_t rti;
    traceinfo_t wti;
    wc_gopts_t opts_map;
    int datarange_begin;
    int datarange_end;
};

cp_compress_t::cp_compress_t()
{
    pimpl.init();
}
void cp_compress_t::init(const traceinfo_t *traceinfo, pwtermset_mode_t mode)
{   
    std::stringstream ssaofile;
    ssaofile<<"compressed.npy";

    std::string outfilepath = wc_generate_path(traceinfo->title.c_str(), ssaofile.str().c_str());
    pimpl.get()->writer = pwfactory_t::get_instance()->new_tracewriter("npy");
    pimpl.get()->rti = *traceinfo;
    pimpl.get()->wti = *traceinfo;
    pimpl.get()->wti.nsamples = traceinfo->nsamples * traceinfo->nterms / 10;
    
    pimpl.get()->writer->open_file(outfilepath.data(), &pimpl.get()->wti, {{"numpy",""}});
    traceinfo_print("", traceinfo);
}

void cp_compress_t::init(const char* filename)
{
}
void cp_compress_t::process()
{ 
}
void cp_compress_t::process(const trace_t* trace)
{
    auto p = pimpl.get();
    trace_t wtrace;
    wtrace.init_dataonly(trace->_data.getptr(), &p->wti); 
    pwpass_compress(trace->_samples.getptr(), p->rti.nsamples * p->rti.nterms,
            wtrace._samples.getptr(), p->wti.nsamples , 15, 5);
    //wtrace.print();
    p->writer->write_trace(&wtrace);
}
void cp_compress_t::finit()
{
    pimpl.get()->writer->finit(nullptr);
}
cp_compress_t::~cp_compress_t()
{
}
