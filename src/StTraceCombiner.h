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
#include "Trace.h"
#include "Stackable.h"

class trace_combiner_base_t
{
public:
    virtual traceinfo_t get_outtraceinfo() = 0;
    virtual void init_combined_trace(trace_t* trace) = 0;
    virtual void combine(const trace_t* intrace, trace_t* outtrace) = 0;
    virtual ~trace_combiner_base_t() {}
};

template<unsigned int N>
class trace_combiner_t : public trace_combiner_base_t
{
public:
    trace_combiner_t(const traceinfo_t* intraceinfo);
    traceinfo_t get_outtraceinfo() override;
    void init_combined_trace(trace_t* trace) override;
    void combine(const trace_t* intrace, trace_t* outtrace) override;
private:
    traceinfo_t outti;
    traceinfo_t inti;
};

template<unsigned int N>
class trace_combiner_wind_t : public trace_combiner_base_t
{
public:
    trace_combiner_wind_t(const traceinfo_t* intraceinfo);
    traceinfo_t get_outtraceinfo() override;
    void init_combined_trace(trace_t* trace) override;
    void combine(const trace_t* intrace, trace_t* outtrace) override;
private:
    traceinfo_t outti;
    traceinfo_t inti;
};


template<>
trace_combiner_t<1>::trace_combiner_t(const traceinfo_t* intraceinfo);
template<>
trace_combiner_t<2>::trace_combiner_t(const traceinfo_t* intraceinfo);
template<>
trace_combiner_t<3>::trace_combiner_t(const traceinfo_t* intraceinfo);

template<>
trace_combiner_wind_t<3>::trace_combiner_wind_t(const traceinfo_t* intraceinfo);


class st_trace_combiner_t : public stackable_t
{
public:
    void init(const traceinfo_t* inti, traceinfo_t* outti);
    trace_t* process(const trace_t* trace);
private:
    trace_t _outtrace;
    traceinfo_t _outti;
    traceinfo_t _inti;
    trace_combiner_base_t* _comb;
};
