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
#include <bits/stdint-intn.h>
#include <fstream>
#include <iostream>
#include "../thirdparty/trs/trs.h"
#include "TraceReaderTrs.h"
#include "PImpl.h"
#include "Util.h"
#include "Log.h"
#include "DataFile.h"
enum datatype_t
{
    dt_double,
    dt_float
};
struct tracereader_trs_priv_t
{
    size_t sz;
    double* samples;
    float* fsamples;
    datatype_t dtype;
    traceinfo_t traceinfo;
    trs_set trsset;
    trs_trace trace;
    size_t trace_id;
    pwdatastream_t datastream;
    std::string tracepath;
};
tracereader_trs_t::tracereader_trs_t()
{
    pimpl.init();
}
void tracereader_trs_t::open_file(const char* tracepath, traceinfo_t* info)
{
    auto strpath = std::string(tracepath);
    int failed = 0;
    trs_open_file(&pimpl.get()->trsset, tracepath, "rb");
    pimpl.get()->tracepath = strpath;
    trs_parse_header(&pimpl.get()->trsset, &failed);
    PWASSERT(failed == 0, "parse trs header failed!");
    auto trsset = pimpl.get()->trsset;

    pimpl.get()->traceinfo.ndata = trsset.DS;
    pimpl.get()->traceinfo.ntraces = trsset.NT;
    pimpl.get()->traceinfo.nsamples = trsset.NS;
    pimpl.get()->traceinfo.nterms = 1;
 
    std::string stracepath(tracepath);                                                                                                        
    auto dotpos = stracepath.find_last_of("."); 
    auto slashpos = stracepath.find_last_of("/");
    if (slashpos == std::string::npos)  
    {                                                                                                                                                                   
        slashpos = 0;                                                                                                                                                   
    }
    else                                                                                                                                                                
    {                                                                                                                                                                   
        slashpos +=1;                                                                                                                                                   
    }                                                                                             

    pimpl.get()->traceinfo.title = stracepath.substr(slashpos, dotpos - slashpos); 
    pimpl.get()->trace = trs_make_trace(&trsset);
    pimpl.get()->samples = new double[trsset.NS];
    pimpl.get()->trace_id = 0;
    *info = pimpl.get()->traceinfo;
}
void tracereader_trs_t::seek(size_t traceid)
{
    trs_seekto_trace(&pimpl.get()->trsset, (unsigned)traceid);
    pimpl.get()->trace_id = traceid;
}
void tracereader_trs_t::rewind()
{
    pimpl.get()->trace_id = 0;
    trs_rewind(&pimpl.get()->trsset);
}
int tracereader_trs_t::read_trace(trace_t* trace)
{
    auto p = pimpl.get();
    size_t nsamples = p->traceinfo.nsamples * p->traceinfo.nterms;
    if (p->trace_id >= p->traceinfo.ntraces)
    {
        return 0;
    }
    trs_read_trace(&p->trsset, &p->trace);
    switch (p->trsset.SC)
    {
        case trs_sc_float:
            trace->init_copy((float*)p->trace.samples, p->trace.data, &p->traceinfo);
            break;
        case trs_sc_int:
            for (int i=0;i<p->trsset.NS;i++)
            {
                p->samples[i] = (double) ((int32_t*)p->trace.samples)[i];
            }
            trace->init_copy(&p->samples[0], p->trace.data, &p->traceinfo);
            break;
        case trs_sc_short:
            for (int i=0;i<p->trsset.NS;i++)
            {
                p->samples[i] = (double) ((int16_t*)p->trace.samples)[i];
            }
            trace->init_copy(&p->samples[0], p->trace.data, &p->traceinfo);
            break;
        case trs_sc_byte:
            for (int i=0;i<p->trsset.NS;i++)
            {
                p->samples[i] = (double) ((int8_t*)p->trace.samples)[i];
            }
            trace->init_copy(&p->samples[0], p->trace.data, &p->traceinfo);
            break;
        default:
            LogErrDie("impossible\n");
    }
    p->trace_id++;
    return 1;
}
tracereader_trs_t::~tracereader_trs_t()
{
    trs_close(&pimpl.get()->trsset);
    delete [] pimpl.get()->samples;
}
