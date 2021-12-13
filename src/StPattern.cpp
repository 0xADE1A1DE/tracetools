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
#include "StPattern.h"
#include "Trace.h"
#include "WorkContext.h"
#include "AnalysisOuput.h"
#include <cmath>
#include <functional>
#include <vector>
#include <algorithm>
#include <numeric>
struct st_pattern_priv_t
{
    int fft_len;
    int ref_begin;
    int refwind_len;
    int npeaks;
    int maxshift;
    double mincorr;
    int search_begin;
    int search_end;
    int pad_start;
    int pad_end;
    std::vector<int> idxvec;
    traceinfo_t inti;
    traceinfo_t patternti;
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
void st_pattern_t::init(const traceinfo_t* inti, traceinfo_t* outti)
{
    _pimpl.init();
    auto p = _pimpl.get(); 
    outti->nsamples = 0;

    int w = wc_gopts_get("static_align_ref_width").get_int();
    p->refwind_len = w;
    p->ref_begin = wc_gopts_get("static_align_ref_begin").get_int();
    p->npeaks = 1;
    p->maxshift = -1;
    p->mincorr = 0.8;
    p->pad_start = 0;
    p->pad_end = 0;
    if (wc_gopts_has("static_align_npeaks"))
    {
        p->npeaks = wc_gopts_get("static_align_npeaks").get_int();
    }
    if (wc_gopts_has("static_align_maxshift"))
    {
        p->maxshift = wc_gopts_get("static_align_maxshift").get_int();
    }
    if (wc_gopts_has("static_align_mincorr"))
    {
        p->mincorr = wc_gopts_get("static_align_mincorr").get_double();
    }
    if (wc_gopts_has("pattern_pad_start"))
    {
        p->pad_start = wc_gopts_get("pattern_pad_start").get_int();
        p->pad_end = wc_gopts_get("pattern_pad_end").get_int();
    }
    p->idxvec.resize(inti->nsamples);
    p->inti = *inti;
    LogInfo("peaks: %d\n", p->npeaks);
    std::string outname = "patternmatch-";
    outname += inti->title;
    outname += ".npy";
        
    p->ao.init(outname.c_str(), "npy");
    p->patternti = *inti;
    p->patternti.nsamples = w + p->pad_start + p->pad_end;
    p->patternti.ntraces *= p->npeaks;
    p->ao.init(&p->patternti);
    traceinfo_print("ss", &p->patternti);
}
const trace_t* st_pattern_t::process(const trace_t* trace)
{
    PWASSERT(trace->_opaque != nullptr, "incompatible usage\n");
    auto trccorr = (trace_t*) trace->_opaque;
    auto p = _pimpl.get();
    auto& idxvec = p->idxvec;
    auto& xcorrvals = trccorr->_samples;

    bool inpeak = false;
    int lastinpeak = -1;
    int lastpeakfoundat = -1;
    int minpeakwidth = 1;
    int minnextpeak = 100;
    double peakthres = 0.8;
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
        auto pp = peaks.begin();

        std::sort(peaks.begin(), peaks.end(), [&](const peak_pair_t& a, const peak_pair_t& b)
            {
                return a.peakval > b.peakval;
            } );
        trace_t trcshifted;
        trcshifted.init(&p->inti);
        int total_matches = 0;
        // count NCC peaks that are above 
        // correlation threshold
        for (auto& peak : peaks)
        {
            if ((peak.peakval < 1.0) && peak.peakval > p->mincorr &&
                    peak.peakat >= p->refwind_len + p->pad_start)
            {
                total_matches++;
            }
        }
        if (total_matches >= p->npeaks)
        {
            for (int i=0;i<p->npeaks;i++)
            {
                if ((peaks[i].peakval < 1.0) && peaks[i].peakval > p->mincorr &&
                    peaks[i].peakat >= p->refwind_len + p->pad_start)
                {
                    int matchat = peaks[i].peakat - p->refwind_len;
                    trace_t trcpattern;
                    trcpattern.init(&p->patternti);
                    
                    int start = matchat - p->pad_start;
                    int end = std::min(matchat + p->refwind_len + p->pad_end, (int)p->inti.nsamples);
                    int writehead =0;
                    for (auto j=start;j<end;j++)
                    {
                        trcpattern._samples[writehead] = trace->_samples[j];
                        writehead++;
                    }
                    trcpattern.copydatafrom(trace);
                    p->ao.on_result_trace(&trcpattern);
                }
            }
        }
    }

    return nullptr;
}
void st_pattern_t::finit()
{
    _pimpl.get()->ao.finit();
}
st_pattern_t::~st_pattern_t()
{
}
