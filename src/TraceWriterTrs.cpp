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
#include <bits/stdint-uintn.h>
#include <iostream>
#include <fstream>
#include "../thirdparty/trs/trs.h"
#include "TraceWriterTrs.h"
#include "PImpl.h"
#include "WorkContext.h"
#include "Util.h"
#include "Log.h"

struct tracewriter_trs_priv_t
{
    size_t sz;
    double* samples;
    traceinfo_t traceinfo;
    trs_set trsset;
    int trace_id;
    std::ofstream ofdata;
    bool hasdata;
    std::vector<float> fsamples;
    std::vector<traceopt_t> opts;
    std::string tracepath;
    long written;
};

tracewriter_trs_t::tracewriter_trs_t()
{
    pimpl.init();
}
void tracewriter_trs_t::open_file(const char* tracepath, const traceinfo_t* info, const std::vector<traceopt_t>& opts)
{
    auto p = pimpl.get();
    p->opts = opts;
    p->tracepath = tracepath;
    p->hasdata = false;
    if (info->ndata > 0)
    {
        p->hasdata = true;
    }
    int failed=0;
    auto& trsset = pimpl.get()->trsset;
    trs_open_file(&trsset, tracepath, "wb");
    trs_add_header_int(&trsset, NUMBER_TRACES, (int) info->ntraces);
    trs_add_header_int(&trsset, NUMBER_SAMPLES, (int) (info->nsamples * info->nterms));
    trs_add_header_int(&trsset, LENGTH_DATA, (int) info->ndata);
    trs_add_header_int(&trsset, SAMPLE_CODING, (int) trs_sc_float);
    trs_write_headers(&trsset, &failed);
    PWASSERT(failed == 0, "writing headers failed!");
    p->fsamples.resize(info->nsamples*info->nterms);
    p->traceinfo = *info;
    p->written = 0;
}

void tracewriter_trs_t::finit(const traceinfo_t* finaltraceinfo)
{
    if (finaltraceinfo)
    {
        trs_clear_header(&pimpl.get()->trsset);
        auto& trsset = pimpl.get()->trsset;
        int failed=0;
        auto info = finaltraceinfo;
        trs_add_header_int(&trsset, NUMBER_TRACES, (int) info->ntraces);
        trs_add_header_int(&trsset, NUMBER_SAMPLES, (int) (info->nsamples * info->nterms));
        trs_add_header_int(&trsset, LENGTH_DATA, (int) info->ndata);
        trs_add_header_int(&trsset, SAMPLE_CODING, (int) trs_sc_float);
        trs_write_headers(&trsset, &failed);
        PWASSERT(failed == 0, "writing headers failed!");
    }
    LogInfo("aa %ld\n", pimpl.get()->written);
    trs_close(&pimpl.get()->trsset);
}    
int tracewriter_trs_t::write_trace(const trace_t* trace)
{
    auto p = pimpl.get();

    for (auto i=0u; i<p->traceinfo.nsamples*p->traceinfo.nterms;i++)
    {
        p->fsamples[i] = (float) trace->_samples[i];
    }

    trs_trace trstrace;
    trstrace.data = const_cast<uint8_t*> (trace->_data.getptr());
    trstrace.samples = p->fsamples.data();
    trs_write_trace(&p->trsset, &trstrace); 
    p->written++;
    return 0;
}
void tracewriter_trs_t::update_traceinfo(const traceinfo_t* newtraceinfo)
{
    trs_clear_header(&pimpl.get()->trsset);
    auto& trsset = pimpl.get()->trsset;
    int failed=0;
    auto info = newtraceinfo;
    trs_add_header_int(&trsset, NUMBER_TRACES, (int) info->ntraces);
    trs_add_header_int(&trsset, NUMBER_SAMPLES, (int) (info->nsamples * info->nterms));
    trs_add_header_int(&trsset, LENGTH_DATA, (int) info->ndata);
    trs_add_header_int(&trsset, SAMPLE_CODING, (int) trs_sc_float);
    trs_write_headers(&trsset, &failed);
    PWASSERT(failed == 0, "writing headers failed!");
}
tracewriter_trs_t::~tracewriter_trs_t()
{
    finit(nullptr);
}
