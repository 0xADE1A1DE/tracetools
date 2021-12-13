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
#include <iostream>
#include <fstream>
#include "../thirdparty/npy/npy.hpp"
#include "TraceWriterNpy.h"
#include "PImpl.h"
#include "WorkContext.h"
#include "Util.h"
#include "Log.h"

class npy_writer_wrapper_t
{
public:
    virtual npy::npy_writer_t* write_numpy_header( const std::string& filename, 
            bool fortran_order, unsigned int n_dims, 
            const unsigned long shape[], const std::string& other = "") = 0;
    virtual void update_numpy_header( const std::string& filename, bool fortran_order, 
            unsigned int n_dims, const unsigned long shape[], const std::string& other = "") = 0;
    virtual void update_numpy_header( npy::npy_writer_t* writer, bool fortran_order, 
            unsigned int n_dims, const unsigned long shape[], const std::string& other = "") = 0;
    virtual void write_numpy_data(npy::npy_writer_t* writer, const double* data, size_t count) = 0;
};
class npy_flt_writer : public npy_writer_wrapper_t
{
public:
    npy_flt_writer(size_t sz)
    {
        _buf = new float[sz];
    }
    npy::npy_writer_t* write_numpy_header( const std::string& filename, 
            bool fortran_order, unsigned int n_dims, 
            const unsigned long shape[], const std::string& other = "")
    {
        return npy::WriteNumpyHeader<float>(filename, fortran_order, n_dims, shape, other);
    }
    void update_numpy_header( const std::string& filename, bool fortran_order, 
            unsigned int n_dims, const unsigned long shape[], const std::string& other = "")
    {
        return npy::UpdateNumpyHeader<float>(filename, fortran_order, n_dims, shape, other);
    }
    void update_numpy_header( npy::npy_writer_t* writer, bool fortran_order,
            unsigned int n_dims, const unsigned long shape[], const std::string& other = "")
    {
        return npy::UpdateNumpyHeader<float>(writer, fortran_order, n_dims, shape, other);
    }
    void write_numpy_data(npy::npy_writer_t* writer, const double* data, size_t count)
    {
        for (size_t i=0;i<count;i++)
        {
            _buf[i] = (float)data[i];
        }
        return npy::WriteNumpyData(writer, _buf, count);
    }
private:
    float* _buf;
};
class npy_dbl_writer : public npy_writer_wrapper_t
{
public:
    npy::npy_writer_t* write_numpy_header( const std::string& filename, 
            bool fortran_order, unsigned int n_dims, 
            const unsigned long shape[], const std::string& other = "")
    {
        return npy::WriteNumpyHeader<double>(filename, fortran_order, n_dims, shape, other);
    }
    void update_numpy_header( const std::string& filename, bool fortran_order, 
            unsigned int n_dims, const unsigned long shape[], const std::string& other = "")
    {
        return npy::UpdateNumpyHeader<double>(filename, fortran_order, n_dims, shape, other);
    }
    void update_numpy_header( npy::npy_writer_t* writer, bool fortran_order,
            unsigned int n_dims, const unsigned long shape[], const std::string& other = "")
    {
        return npy::UpdateNumpyHeader<double>(writer, fortran_order, n_dims, shape, other);
    }
    void write_numpy_data(npy::npy_writer_t* writer, const double* data, size_t count)
    {
        return npy::WriteNumpyData(writer, data, count);
    }
};
struct tracewriter_npy_priv_t
{
    size_t sz;
    double* samples;
    traceinfo_t traceinfo;
    npy_writer_wrapper_t *wrapper;
    npy::npy_writer_t* writer;
    int trace_id;
    std::ofstream ofdata;
    bool hasdata;
    std::vector<traceopt_t> opts;
    std::string tracepath;
};

tracewriter_npy_t::tracewriter_npy_t()
{
    pimpl.init();
}
void tracewriter_npy_t::open_file(const char* tracepath, const traceinfo_t* info, const std::vector<traceopt_t>& opts)
{
    auto p = pimpl.get();
    p->opts = opts;
    p->tracepath = tracepath;
    p->hasdata = false;
    if (info->ndata > 0)
    {
        p->hasdata = true;
    }
    if (traceopt_has_key(opts, "datatype") && (traceopt_get_value(opts, "datatype") == "double"))
    {
        p->wrapper = new npy_dbl_writer();
    }
    else
    {
        p->wrapper = new npy_flt_writer(info->nsamples * info->nterms);
    }

    if (traceopt_has_key(opts, "termsnpy"))
    {
        unsigned long dims[] = { (unsigned long)info->ntraces, (unsigned long)info->nsamples*info->nterms };
        std::stringstream ss;
        ss <<"termcombplayback "
            <<info->nterms<<" "
            //<<rterms<<" "
            <<info->nsamples<<" "
            <<info->ntraces;
        
        if (p->hasdata)
        {
            std::vector<char> datapath(256);
            snprintf(datapath.data(), datapath.size(), "%s.data", tracepath);
            p->ofdata.open(datapath.data(),std::ofstream::binary);
            if (p->ofdata.fail())
            {
                LogErrDie("data file open failed %s\n", datapath.data());
            }
        }
        //p->writer = npy::WriteNumpyHeader<double>(tracepath, false, 2, dims, ss.str());
        p->writer = p->wrapper->write_numpy_header(tracepath, false, 2, dims, ss.str());
    }
    else
    {
        unsigned long dims[] = { (unsigned long)info->ntraces, (unsigned long)info->nsamples*info->nterms };
        
        if (p->hasdata)
        {
            std::vector<char> datapath(256);
            snprintf(datapath.data(), datapath.size(), "%s.data", tracepath);
            p->ofdata.open(datapath.data(),std::ofstream::binary);
            if (p->ofdata.fail())
            {
                LogErrDie("data file open failed %s\n", datapath.data());
            }
        }
        //p->writer = npy::WriteNumpyHeader<double>(tracepath, false, 2, dims);
        p->writer = p->wrapper->write_numpy_header(tracepath, false, 2, dims);
    }
    
    PWASSERT(p->writer != nullptr, "writer failed");
    
    p->traceinfo = *info;
}

void tracewriter_npy_t::finit(const traceinfo_t* finaltraceinfo)
{
    auto p = pimpl.get();
    auto& opts = p->opts;
    auto tracepath = p->tracepath.c_str();
    auto info = finaltraceinfo;
    
    if (p->writer)
    {
        npy::CloseNumpy(p->writer);
        p->writer = nullptr;
    }
    else
    {
        return;
    }
    
    if (pimpl.get()->ofdata.is_open())
    {
        pimpl.get()->ofdata.close();
    }
    
    // there's nothing to update, end here
    if (info == nullptr)
    {
        return;
    }
    
    if (traceopt_has_key(opts, "termsnpy"))
    {
        unsigned long dims[] = { (unsigned long)info->ntraces, (unsigned long)info->nsamples*info->nterms };
        std::stringstream ss;
        ss <<"termcombplayback "
            <<info->nterms<<" "
            //<<rterms<<" "
            <<info->nsamples<<" "
            <<info->ntraces;
        
        //npy::UpdateNumpyHeader<double>(tracepath, false, 2, dims, ss.str());
        p->wrapper->update_numpy_header(tracepath, false, 2, dims, ss.str());
    }
    else 
    {
        unsigned long dims[] = { (unsigned long)info->ntraces, (unsigned long)info->nsamples*info->nterms };
        
        //npy::UpdateNumpyHeader<double>(tracepath, false, 2, dims);
        p->wrapper->update_numpy_header(tracepath, false, 2, dims);
    }
    
}
int tracewriter_npy_t::write_trace(const trace_t* trace)
{
    auto p = pimpl.get();
    std::vector<char> dataline(p->traceinfo.ndata*2+2);
    if (p->hasdata)
    {
        bytebuf_to_hexstr(trace->_data.getptr(), p->traceinfo.ndata, dataline.data());
        dataline[dataline.size()-2]='\n';
        dataline[dataline.size()-1]='\0';
        p->ofdata.write(dataline.data(), dataline.size()-1);
    }
        
    //npy::WriteNumpyData(p->writer, trace->_samples.getptr(), 
    //                    p->traceinfo.nsamples*p->traceinfo.nterms);
    p->wrapper->write_numpy_data(p->writer, trace->_samples.getptr(), 
                        p->traceinfo.nsamples*p->traceinfo.nterms);

    return 0;
}
void tracewriter_npy_t::update_traceinfo(const traceinfo_t* newtraceinfo)
{
    auto p = pimpl.get();
    unsigned long dims[] = { (unsigned long) newtraceinfo->ntraces, 
        (unsigned long) newtraceinfo->nsamples*newtraceinfo->nterms };
    p->wrapper->update_numpy_header(p->writer, false, 2, dims);
}
tracewriter_npy_t::~tracewriter_npy_t()
{
    finit(nullptr);
}

