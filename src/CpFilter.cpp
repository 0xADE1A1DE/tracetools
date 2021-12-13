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
#include "WorkContext.h"
#include "PWFactory.h"
#include "TraceReader.h"
#include "CpFilter.h"
#include "Passes.h"
#include "Iir.h"


int OPT_SAMPL=80000000;
int OPT_CUTOF=400000;
int OPT_CENT=(8000000-500);
int OPT_WIDTH=500;

uint16_t PAD_AMOUNT=100;
class FiltBase
{
public:
    virtual void reset() = 0;
    virtual void apply(double* samples, long samplescount) = 0;
};
class BandstopFilt : public FiltBase
{
public:
    Iir::Butterworth::BandStop<4> _filt;
    BandstopFilt()
    {
        _filt.setup(wc_gopts_get("filter_samplerate").get_int(),
                wc_gopts_get("filter_centre_freq").get_int(), 
                wc_gopts_get("filter_width").get_int());
    }
    void reset()
    {
        _filt.reset();
    }
    // apply forward and backward filtering
    void apply(double* samples, long samplescount)
    {
        for (long i=0;i<samplescount;i++)
        {
            samples[i] = _filt.filter(samples[i]);
        }
        for (long i=samplescount-1;i>=0;i--)
        {
            samples[i] = _filt.filter(samples[i]);
        }
    }
};

class HighpassFilt : public FiltBase
{
public:
    Iir::Butterworth::HighPass<4> _filt;
    HighpassFilt()
    {
        _filt.setup(wc_gopts_get("filter_samplerate").get_int(), 
                wc_gopts_get("filter_cutoff").get_int());
    }
    void reset()
    {
        _filt.reset();
    }
    void apply(double* samples, long samplescount)
    {
        for (long i=0;i<samplescount;i++)
        {
            samples[i] = _filt.filter(samples[i]);
        }
        for (long i=samplescount-1;i>=0;i--)
        {
            samples[i] = _filt.filter(samples[i]);
        }

    }
};
class LowpassFilt : public FiltBase
{
public:
    Iir::Butterworth::LowPass<4> _filt;
    LowpassFilt()
    {
        _filt.setup(wc_gopts_get("filter_samplerate").get_int(), 
                wc_gopts_get("filter_cutoff").get_int());
    }
    void reset()
    {
        _filt.reset();
    }
    void apply(double* samples, long samplescount)
    {
        for (long i=0;i<samplescount;i++)
        {
            samples[i] = _filt.filter(samples[i]);
        }
        for (long i=samplescount-1;i>=0;i--)
        {
            samples[i] = _filt.filter(samples[i]);
        }

    }
};
struct cp_filter_priv_t
{
    std::string ext;
    std::string filename;
    traceinfo_t traceinfo;
    traceinfo_t winti;
    tracereader_t* reader;
    tracewriter_t* writer;
    FiltBase* filt;
    std::vector<double> tempsamples;
};
cp_filter_t::cp_filter_t()
{
    pimpl.init();
}
void cp_filter_t::init(const traceinfo_t *traceinfo, pwtermset_mode_t mode)
{
    std::string outfilepath = wc_generate_path(traceinfo->title.c_str(), "filtered.npy");
    pimpl.get()->writer = pwfactory_t::get_instance()->new_tracewriter("npy");
    pimpl.get()->writer->open_file(outfilepath.c_str(), traceinfo, {{"numpy",""}});
    pimpl.get()->traceinfo = *traceinfo;
    pimpl.get()->tempsamples.resize(2*PAD_AMOUNT + traceinfo->nsamples);
    auto filtername = wc_gopts_get("filter_select").get_string();
    if (filtername == "bandstop")
    {
        pimpl.get()->filt = new BandstopFilt();
    }
    else if (filtername == "highpass")
    {
        pimpl.get()->filt = new HighpassFilt();
    }
    else if (filtername == "lowpass")    
    {
        pimpl.get()->filt = new LowpassFilt();
    }
    else
    {
        LogErrDie("no filter selected\n");
    }
}

void cp_filter_t::init(const char* filename)
{
}
void cp_filter_t::process()
{ 

}
void cp_filter_t::process(const trace_t* trace)
{
    auto p = pimpl.get();
    trace_t newtrace(*trace);
    pwpass_add_padding(trace->_samples.getptr(), trace->_traceinfo.nsamples,
            p->tempsamples.data(), p->tempsamples.size(), PAD_AMOUNT, 0);
    pimpl.get()->filt->reset();
    pimpl.get()->filt->apply(p->tempsamples.data(), p->tempsamples.size());
    memcpy(newtrace._samples.getptr(), &p->tempsamples.data()[PAD_AMOUNT], newtrace._traceinfo.nsamples * sizeof(double));
    pwpass_moving_average(&newtrace);
    pimpl.get()->writer->write_trace(&newtrace);
}
void cp_filter_t::finit()
{
    pimpl.get()->writer->finit(&pimpl.get()->traceinfo);
}
cp_filter_t::~cp_filter_t()
{
}
