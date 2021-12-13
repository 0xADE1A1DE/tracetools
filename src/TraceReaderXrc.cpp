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
#include "TraceReaderXrc.h"
#include "DataFile.h"
#include "../thirdparty/tracewriter/tracewriter.h"

struct tracereader_xrc_priv_t
{
    pwdatastream_t datastream;
    traceinfo_t traceinfo;
    TraceWriter tw;
    double frac;
    uint64_t trace_id;
    std::vector<double> dsamples;
};

tracereader_xrc_t::tracereader_xrc_t()
{
    pimpl.init();
}
void tracereader_xrc_t::open_file(const char* tracepath, traceinfo_t* info)
{
    auto p = pimpl.get();
    tw_open(&p->tw, tracepath, TW_M_READ);
    
    uint32_t tp_fmt, tp_samples, tp_rounds, tp_caps, tp_num, tp_den;
    tw_get_property(&p->tw, TW_P_SAMPLES_PER_CAP, &tp_samples);
    tw_get_property(&p->tw, TW_P_FMT, &tp_fmt);
    tw_get_property(&p->tw, TW_P_ROUNDS, &tp_rounds);
    tw_get_property(&p->tw, TW_P_CAPS_PER_ROUND, &tp_caps);

    tw_get_property(&p->tw, TW_P_FRAC_NUM, &tp_num);
    tw_get_property(&p->tw, TW_P_FRAC_DENO, &tp_den);
    
    p->frac = ((float)tp_num)/((float)tp_den);
    uint32_t ntraces = tp_caps * tp_rounds;
    
    p->traceinfo.ntraces = (int) ntraces;
    p->traceinfo.nsamples = (int) tp_samples;
    p->traceinfo.nterms = 1;
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
    
    p->traceinfo.title = stracepath.substr(slashpos, dotpos - slashpos);
    
    std::vector<char> datapath(256);
    snprintf(datapath.data(), datapath.size(), "%s.data", tracepath);
    
    pwdata_read_open(p->datastream, datapath.data(), p->traceinfo.ndata); 
    
    p->dsamples.resize(tp_samples);
    p->trace_id = 0;
    *info = p->traceinfo;
}
void tracereader_xrc_t::seek(size_t traceid)
{
    pimpl.get()->trace_id = traceid;
    pwdata_read_seek(pimpl.get()->datastream, traceid);
}
void tracereader_xrc_t::rewind()
{
    pimpl.get()->trace_id = 0;
    pwdata_read_rewind(pimpl.get()->datastream);
}
int tracereader_xrc_t::read_trace(trace_t* trace)
{
    auto p = pimpl.get();
    if (p->trace_id >= (uint64_t) p->traceinfo.ntraces)
    {
        return 0;
    }
    
    tw_t *samples;
    tw_read_samples(&p->tw, &samples);
    
    for (auto i=0u;i<p->traceinfo.nsamples;i++)
    {
        p->dsamples[i] = p->frac * samples[p->trace_id * p->traceinfo.nsamples + i];
    }
    
    if (p->datastream.hasdata)
    {
        std::vector<uint8_t> bytedata(p->traceinfo.ndata);
        pwdata_read_data(p->datastream, bytedata.data(), p->traceinfo.ndata);
        
        trace->init_copy(p->dsamples.data(), 
                     bytedata.data(), &p->traceinfo);
    }
    else
    {
        trace->init_view(p->dsamples.data(), nullptr, &p->traceinfo);
    }
    
    p->trace_id++;
    return 1;
}
tracereader_xrc_t::~tracereader_xrc_t()
{
    auto p = pimpl.get();
    pwdata_read_close(p->datastream);
    tw_close(&p->tw);
}
