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
#include <stdint.h>
#include <memory.h>
#include <string>
#include <vector>
#include "RefCountedPtr.h"
#include "Log.h"
#include "Util.h"
struct traceinfo_t
{
    uint32_t nsamples;
    uint32_t ntraces;
    uint32_t nterms;
    uint32_t ndata;
    std::string title;
    
    traceinfo_t() = default;
};

struct traceopt_t
{
    std::string key;
    std::string value;
};

struct trace_t
{    
    refcountedptr_t<double> _samples;
    refcountedptr_t<uint8_t> _data;
    traceinfo_t _traceinfo;
    void *_opaque;
    trace_t() :
        _opaque(nullptr)
    {
    }
    trace_t(const traceinfo_t* traceinfo)
    {
        init(traceinfo);
    }
    trace_t(const trace_t& other, uint32_t nsamples) :
        _samples(),
        _data(other._data),
        _traceinfo(other._traceinfo)
    {
        _samples.init(nsamples);
        _traceinfo.nsamples = nsamples;
    }
    trace_t(const trace_t& other) :
        _samples(other._samples),
        _data(other._data),
        _traceinfo(other._traceinfo)
    {
    }
    trace_t(trace_t&& other) noexcept:
        _samples(std::move(other._samples)),
        _data(std::move(other._data)),
        _traceinfo(other._traceinfo)
    {
    }
    void copyfrom(const trace_t* trace)
    {
        size_t nsamples = trace->_traceinfo.nsamples * trace->_traceinfo.nterms;
        size_t ndata = trace->_traceinfo.ndata;

        PWASSERT(trace->_samples.getptr() != nullptr && _samples.getptr() != nullptr, "not init");  
        PWASSERT(nsamples <= _traceinfo.nsamples * _traceinfo.nterms, "invalid start sample index");

        /*for (size_t i=0;i<nsamples;i++)
        {
            _samples[i] = trace->_samples[i];
        }*/
        memcpy(&_samples[0], &trace->_samples[0], nsamples * sizeof(double));
        if (ndata > 0)
        {
            if (_data.getptr() != nullptr && trace->_data.getptr() != nullptr)
            {
                memcpy(_data.getptr(), trace->_data.getptr(), ndata * sizeof(uint8_t));
            }
            else
            {
                LogErrDie("data required\n");
            }
        }
    }
    void copydatafrom(const trace_t* trace)
    {
        size_t ndata = trace->_traceinfo.ndata;

        PWASSERT(trace->_samples.getptr() != nullptr && _samples.getptr() != nullptr, "not init");  
        PWASSERT(ndata == _traceinfo.ndata, "unequal data lengths!");
        if (_data.getptr() != nullptr && trace->_data.getptr() != nullptr)
        {
            memcpy(_data.getptr(), trace->_data.getptr(), ndata * sizeof(uint8_t));
        }
        else
        {
            LogErrDie("data required\n");
        }
    }

    void print() const
    {
        size_t nsamples = _traceinfo.nsamples * _traceinfo.nterms;
        printf("trace %s: [", _traceinfo.title.c_str());
        for (size_t i=0;i<std::min(nsamples, (size_t)5);i++)
        {
            printf("%lf ", _samples[i]);
        }
        printf("... ");
        for (size_t i=nsamples-5;i<nsamples;i++)
        {
            printf("%lf ", _samples[i]);
        }
        printf("]\n");
    }
    inline void samples_copyto(trace_t* trace, size_t from, size_t count) const
    {   
        memcpy(&trace->_samples[0], &_samples[from], count * sizeof(double));
    }
    void copyto(trace_t* trace, size_t from, size_t count=0, size_t to=0) const
    {
        size_t nsamples = (count == 0) ? (trace->_traceinfo.nsamples * trace->_traceinfo.nterms) : count;
        size_t ndata = trace->_traceinfo.ndata;
        
        PWASSERT(trace->_samples.getptr() != nullptr && _samples.getptr() != nullptr, "not init"); 
        PWASSERT(from + nsamples <= _traceinfo.nsamples * _traceinfo.nterms, "invalid start sample index");
        
        memcpy(&trace->_samples[to], &_samples[from], nsamples * sizeof(double));
        if (ndata > 0)
        {
            if (_data.getptr() != nullptr && trace->_data.getptr() != nullptr)
            {
                memcpy(trace->_data.getptr(), _data.getptr(), ndata * sizeof(uint8_t));
            }
            else
            {
                LogErrDie("data required\n");
            }
        }

    }
    void init(const traceinfo_t* traceinfo, bool clear=true)
    {   
        _samples.init(traceinfo->nsamples * traceinfo->nterms);
        if (clear)
        {
            for (size_t i=0;i<traceinfo->nsamples * traceinfo->nterms;i++)
            {
                _samples[i] = 0.0;
            }
        }
        if (traceinfo->ndata > 0)
        {
            _data.init(traceinfo->ndata);
            if (clear)
            {
                for (size_t i=0;i<traceinfo->ndata;i++)
                {
                    _data[i] = 0;
                }
            }
        }
        _traceinfo = *traceinfo;
        
    }
    void init_view(double *samples, uint8_t *data, const traceinfo_t* traceinfo)
    {
        init_copy(samples, data, traceinfo);
    }
    void init_view(float *samples, uint8_t *data, const traceinfo_t* traceinfo)
    {
        init_copy(samples, data, traceinfo);
    }
    void init_copy(float *samples, uint8_t *data, const traceinfo_t* traceinfo)
    {
        size_t nsamples = traceinfo->nsamples * traceinfo->nterms;
        size_t ndata = traceinfo->ndata;
        
        init(traceinfo, false);
        for (size_t i=0;i<nsamples;i++)
        {
            _samples[i] = (double) samples[i];
        }
        if (ndata > 0)
        {
            if (data)
            {
                memcpy(_data.getptr(), data, ndata * sizeof(uint8_t));
            }
            else
            {
                LogErrDie("data required\n");
            }
        }

    }
    void init_dataonly(const uint8_t *data, const traceinfo_t* traceinfo)
    {
        size_t ndata = traceinfo->ndata;
        
        init(traceinfo, false);
        if (ndata > 0)
        {
            if (data)
            {
                memcpy(_data.getptr(), data, ndata * sizeof(uint8_t));
            }
            else
            {
                LogErrDie("data required\n");
            }
        }
    }

    void init_copy(double *samples, uint8_t *data, const traceinfo_t* traceinfo)
    {
        size_t nsamples = traceinfo->nsamples * traceinfo->nterms;
        size_t ndata = traceinfo->ndata;

        init(traceinfo, false);
        memcpy(_samples.getptr(), samples, nsamples * sizeof(double));
        
        if (ndata > 0)
        {
            if (data)
            {
                memcpy(_data.getptr(), data, ndata * sizeof(uint8_t));
            }
            else
            {
                LogErrDie("data required\n");
            }
        }
    }
};
void traceinfo_print(const char* msg, const traceinfo_t* traceinfo);
const std::string& traceopt_empty();
bool traceopt_has_key(const std::vector<traceopt_t>& opts, const std::string& key);
const std::string& traceopt_get_value(const std::vector<traceopt_t>& opts, const std::string& key);
