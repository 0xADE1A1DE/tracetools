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
#include <fstream>
#include <iostream>
#include "../thirdparty/npy/npy.hpp"
#include "TraceReaderNpy.h"
#include "PImpl.h"
#include "Util.h"
#include "Log.h"
#include "DataFile.h"
enum datatype_t
{
    dt_double,
    dt_float
};
struct tracereader_npy_priv_t
{
    size_t sz;
    double* samples;
    float* fsamples;
    datatype_t dtype;
    traceinfo_t traceinfo;
    npy::npy_reader_t* reader;
    size_t trace_id;
    pwdatastream_t datastream;
    std::string tracepath;
};
tracereader_npy_t::tracereader_npy_t()
{
    pimpl.init();
}
void tracereader_npy_t::open_file(const char* tracepath, traceinfo_t* info)
{
    auto strpath = std::string(tracepath);
    pimpl.get()->reader = npy::LoadHeaderFromNumpy(strpath);
    pimpl.get()->tracepath = strpath;
    
    auto rdr = pimpl.get()->reader;
    pimpl.get()->traceinfo.ndata = 0;
    if (rdr->header.other.length() > 0)
    {
        std::stringstream ss(rdr->header.other);
        std::string magic;
        ss>>magic
            >>pimpl.get()->traceinfo.nterms
            >>pimpl.get()->traceinfo.nsamples 
            >>pimpl.get()->traceinfo.ntraces;
    }
    else
    {
        pimpl.get()->traceinfo.ntraces = rdr->header.shape[0];
        pimpl.get()->traceinfo.nsamples = rdr->header.shape[1];
        pimpl.get()->traceinfo.nterms = 1;
    }
 
    std::vector<char> datapath(256);
    snprintf(datapath.data(), datapath.size(), "%s.data", tracepath);
    LogInfo("%s\n", datapath.data()); 
    pwdata_read_open(pimpl.get()->datastream, datapath.data(), pimpl.get()->traceinfo.ndata);
        
    PWASSERT(pimpl.get()->reader->header.dtype.str() == "<f8" || 
            pimpl.get()->reader->header.dtype.str() == "<f4", "not supported");
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

    size_t offset = pimpl.get()->reader->offset; 
    std::string dtypestr = pimpl.get()->reader->header.dtype.str();
    npy::CloseNumpy(pimpl.get()->reader);
    LogInfo("npy header offset %lu\n", offset);
   
    if (dtypestr == "<f8")
    {
        pimpl.get()->dtype = dt_double;
        pimpl.get()->samples = (double*)npy::OpenNumpyMmap(tracepath, offset, &pimpl.get()->sz);
    }
    else if (dtypestr == "<f4")
    {
        pimpl.get()->dtype = dt_float;
        pimpl.get()->fsamples = (float*)npy::OpenNumpyMmap(tracepath, offset, &pimpl.get()->sz);
    }
    pimpl.get()->trace_id = 0;
    *info = pimpl.get()->traceinfo;
}
void tracereader_npy_t::seek(size_t traceid)
{
    pimpl.get()->trace_id = traceid;
    pwdata_read_seek(pimpl.get()->datastream, traceid);
}
void tracereader_npy_t::rewind()
{
    pimpl.get()->trace_id = 0;
    pwdata_read_rewind(pimpl.get()->datastream);
}
int tracereader_npy_t::read_trace(trace_t* trace)
{
    auto p = pimpl.get();
    size_t nsamples = p->traceinfo.nsamples * p->traceinfo.nterms;
    if (p->trace_id >= p->traceinfo.ntraces)
    {
        return 0;
    }
    
    if (p->datastream.hasdata)
    {
        std::vector<uint8_t> bytedata(p->traceinfo.ndata);
        pwdata_read_data(p->datastream, bytedata.data(), p->traceinfo.ndata);
        
        switch (p->dtype)
        {
            case dt_double:
                trace->init_copy(&p->samples[nsamples * p->trace_id], bytedata.data(), &p->traceinfo);
                break;
            case dt_float:
                trace->init_copy(&p->fsamples[nsamples * p->trace_id], bytedata.data(), &p->traceinfo);
                break;
            default:
                LogErrDie("impossible\n");
        }
    }
    else
    {
        switch (p->dtype)
        {
            case dt_double:
                trace->init_view(&p->samples[nsamples * p->trace_id], nullptr, &p->traceinfo);
                break;
            case dt_float:
                trace->init_view(&p->fsamples[nsamples * p->trace_id], nullptr, &p->traceinfo);
                break;
            default:
                LogErrDie("impossible\n");
        }
    }
    p->trace_id++;
    return 1;
}
tracereader_npy_t::~tracereader_npy_t()
{
    pwdata_read_close(pimpl.get()->datastream);
    
    npy::CloseNumpyMmap(pimpl.get()->samples, pimpl.get()->sz);
}
