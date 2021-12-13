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
#include "StStaticAlign.h"
#include "Trace.h"
#include "WorkContext.h"
#include "AnalysisOuput.h"
#include "Passes.h"
#include <algorithm>
#include <functional>
#include <vector>
#include <numeric>
#include <cmath>
#include <math.h>
#include <fftw3.h>
struct st_static_align_priv_t
{
    int fft_len;
    int ref_begin;
    int refwind_len;
    int npeaks;
    int maxshift;
    double mincorr;
    int search_begin;
    int search_end;
    std::vector<int> idxvec;
    std::vector<int> sigpeaks;
    traceinfo_t inti;
    analysis_output_t ao;
    trace_t trcout;
};
struct peak_pair_t
{
    int peakstart;
    int peakat;
    int peakend;
    double peakval;
};
static void util_shift(trace_t* dst, const trace_t* src, int n, int shift)
{
    if (shift < 0)
    {
        for (int i=0;i<n+shift;i++)
        {
            dst->_samples[i] = src->_samples[i-shift-1];
        }
        for (int i=n+shift;i<n;i++)
        {
            dst->_samples[i] = 0.0;
        }
    }
    else
    {
        for (int i=0;i<shift;i++)
        {
            dst->_samples[i] = 0.0;
        }
        for (int i=shift;i<n;i++)
        {
            dst->_samples[i] = src->_samples[i-shift];
        }
    }
}

void st_static_align_t::init(const traceinfo_t* inti, traceinfo_t* outti)
{
    _pimpl.init();
    
    outti->nsamples = 0;

    int w = wc_gopts_get("static_align_ref_width").get_int();
    _pimpl.get()->refwind_len = w;
    _pimpl.get()->ref_begin = wc_gopts_get("static_align_ref_begin").get_int();
    int n = inti->nsamples;
    _pimpl.get()->npeaks = 1;
    _pimpl.get()->maxshift = -1;
    _pimpl.get()->mincorr = 0;
    _pimpl.get()->search_begin = -1;
    _pimpl.get()->search_end = -1;
    _pimpl.get()->sigpeaks.resize(n);
    if (wc_gopts_has("static_align_npeaks"))
    {
        _pimpl.get()->npeaks = wc_gopts_get("static_align_npeaks").get_int();
    }
    if (wc_gopts_has("static_align_maxshift"))
    {
        _pimpl.get()->maxshift = wc_gopts_get("static_align_maxshift").get_int();
    }
    if (wc_gopts_has("static_align_mincorr"))
    {
        _pimpl.get()->mincorr = wc_gopts_get("static_align_mincorr").get_double();
    }
    if (wc_gopts_has("static_align_begin"))
    {
        _pimpl.get()->search_begin = wc_gopts_get("static_align_begin").get_int();
        _pimpl.get()->search_end = wc_gopts_get("static_align_end").get_int();
        _pimpl.get()->idxvec.resize(_pimpl.get()->search_end - _pimpl.get()->search_begin);
    }
    else
    {
        _pimpl.get()->search_begin = 0;
        _pimpl.get()->idxvec.resize(inti->nsamples);
    }
    _pimpl.get()->inti = *inti;

    std::string outname = "static-aligned-";
    outname += inti->title;
    outname += ".npy";
        
    _pimpl.get()->ao.init(outname.c_str(), "npy");
    _pimpl.get()->ao.init(inti);
}
const trace_t* st_static_align_t::process(const trace_t* trace)
{
    PWASSERT(trace->_opaque != nullptr, "incompatible usage\n");
    auto trccorr = (trace_t*) trace->_opaque;
    auto p = _pimpl.get();
    auto& xcorrvals = trccorr->_samples;
    auto& idxvec = p->idxvec;
    trace_t trcshifted;

    trcshifted.init(&p->inti);

    bool inpeak = false;
    int lastinpeak = -1;
    int lastpeakfoundat = -1;
    int minpeakwidth = 1;
    double peakthres = 0.5;
    int minnextpeak = 50;
    std::vector<peak_pair_t> peaks;
    peak_pair_t lastpeak = {};
    for (int i=0;i<p->inti.nsamples;i++)
    {
        if (!inpeak && (xcorrvals[i] > peakthres) &&
            (lastpeakfoundat + minnextpeak < i))
        {
            inpeak = true;
            lastpeak.peakstart = i;
            lastpeak.peakat = i;
            lastpeak.peakend = -1;
            lastpeak.peakval = xcorrvals[i];
        }
        else if (inpeak && (xcorrvals[i] > peakthres) &&
            (lastpeakfoundat + minnextpeak < i))
        {
            if (lastpeak.peakval < xcorrvals[i])
            {
                lastpeak.peakend = -1;
                lastpeak.peakat = i;
                lastpeak.peakval = xcorrvals[i];
            }
        }
        else if (inpeak && (xcorrvals[i] <= peakthres))
        {
            if (lastinpeak + minpeakwidth < i)
            {
                lastpeak.peakend = i;
                peaks.push_back(lastpeak);
                lastpeakfoundat = i;
            }
            inpeak = false;
        }
    }

    if (peaks.size() > 0)
    {

        std::sort(peaks.begin(), peaks.end(), [&](const peak_pair_t& a, const peak_pair_t& b)
            {
                return a.peakval > b.peakval;
            } );
        trace_t trcshifted;
        trcshifted.init(&p->inti);
        LogInfo("%lf\n", peaks[0].peakval);
        /// positive means right shift
        int shift = (p->ref_begin + p->refwind_len) - peaks[0].peakat;
        util_shift(&trcshifted, trace, p->inti.nsamples, shift);
        if (p->maxshift == -1)
        {
            p->ao.on_result_trace(&trcshifted);
        }
        else if (abs(shift) < p->maxshift)
        {
            p->ao.on_result_trace(&trcshifted);
        }
    }
    return nullptr;
}
void st_static_align_t::finit()
{
    auto p = _pimpl.get();

    p->ao.finit();
}
st_static_align_t::~st_static_align_t() 
{
}
