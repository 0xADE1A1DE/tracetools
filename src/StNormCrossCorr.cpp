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
#include "StNormCrossCorr.h"
#include "Log.h"
#include "Trace.h"
#include "WorkContext.h"
#include "AnalysisOuput.h"
#include "Passes.h"
#include <algorithm>
#include <functional>
#include <vector>
#include <numeric>
#include <math.h>
#include <fftw3.h>
struct st_norm_cross_corr_priv_t
{
    int fft_len;
    int ref_begin;
    int refwind_len;
    int npeaks;
    fftw_complex *in, *refin, *out, *refout, *revin, *revout;
    traceinfo_t inti;
    fftw_plan sigpl;
    fftw_plan refpl;
    fftw_plan revpl;
    analysis_output_t ao;
    double stat_max_xcorr;
    int stat_max_xcorr_at;
    double meanH, stdH;
    trace_t trcout;
    bool reftrace_done;
};

static unsigned int next_power_of_2(unsigned int x)
{
    unsigned int npow = 1U;
    while (x > 0)
    {
        x >>= 1;
        npow <<= 1;
    }
    return npow;
}
static double stat_sum(const double* buf, uint32_t n)
{
    double sum = 0;
    for (auto i=0u;i<n;i++)
    {
        sum += buf[i];
    }
    return sum;
}
static double stat_mean(const double* buf, uint32_t n)
{
    return stat_sum(buf, n) /n;
}
static double stat_std(const double* buf, uint32_t n)
{
    double m = stat_mean(buf, n);
    double sd = 0;
    for (auto i=0u;i<n;i++)
    {
        double a = buf[i] - m;
        sd += a*a;
    }
    return sqrt(sd/n);
}
void st_norm_cross_corr_t::init(const traceinfo_t* inti, traceinfo_t* outti)
{
    _pimpl.init();
    
    int w = wc_gopts_get("static_align_ref_width").get_int();
    _pimpl.get()->refwind_len = w;
    _pimpl.get()->ref_begin = wc_gopts_get("static_align_ref_begin").get_int();
    int n = inti->nsamples;
    _pimpl.get()->npeaks = 1;

    if (wc_gopts_has("static_align_npeaks"))
    {
        _pimpl.get()->npeaks = wc_gopts_get("static_align_npeaks").get_int();
    }
    _pimpl.get()->inti = *inti;
    _pimpl.get()->fft_len = next_power_of_2(w + n -1);
    LogInfo("FFT length: %d\n", _pimpl.get()->fft_len);
    _pimpl.get()->in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*_pimpl.get()->fft_len);
    _pimpl.get()->out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*_pimpl.get()->fft_len);
    _pimpl.get()->refin = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*_pimpl.get()->fft_len);
    _pimpl.get()->revin = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*_pimpl.get()->fft_len);
    _pimpl.get()->revout = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*_pimpl.get()->fft_len);
    _pimpl.get()->refout = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*_pimpl.get()->fft_len);
    
    _pimpl.get()->sigpl = fftw_plan_dft_1d(_pimpl.get()->fft_len, _pimpl.get()->in, _pimpl.get()->out, FFTW_FORWARD, FFTW_ESTIMATE);
    _pimpl.get()->refpl = fftw_plan_dft_1d(_pimpl.get()->fft_len, _pimpl.get()->refin, _pimpl.get()->refout, FFTW_FORWARD, FFTW_ESTIMATE);
    _pimpl.get()->revpl = fftw_plan_dft_1d(_pimpl.get()->fft_len, _pimpl.get()->revin, _pimpl.get()->revout, FFTW_BACKWARD, FFTW_ESTIMATE);

    _pimpl.get()->reftrace_done = false;
    _pimpl.get()->stat_max_xcorr = 0;
    *outti = *inti;

    _pimpl.get()->trcout.init(inti);
    std::string outfile = "xcorr-";
    outfile += inti->title;
    outfile += ".npy";
    //_pimpl.get()->ao.init(outfile.c_str(), "npy");
    //_pimpl.get()->ao.init(inti);
}
const trace_t* st_norm_cross_corr_t::process(const trace_t* trace)
{
    auto p = _pimpl.get();
    const double* H = &trace->_samples[p->ref_begin];

    if (!p->reftrace_done)
    {
        p->meanH = stat_mean(H, p->refwind_len);
        p->stdH = stat_std(H, p->refwind_len);
        int i = 0;
        for (;i<p->refwind_len;i++)
        {
            p->refin[i][0] = (H[p->refwind_len -1 -i] - p->meanH)/p->stdH;
            p->refin[i][1] = 0.0;
        }
        for (;i<p->fft_len;i++)
        {
            p->refin[i][0] = 0.0;
            p->refin[i][1] = 0.0;
        }

        fftw_execute(p->refpl);
        p->reftrace_done = true;
    }

    double meanX = stat_mean(trace->_samples.getptr(), p->inti.nsamples);
    double stdX = stat_std(trace->_samples.getptr(), p->inti.nsamples);
    int i = 0;
    for (;i<p->inti.nsamples;i++)
    {
        p->in[i][0] = (trace->_samples[i] - meanX)/stdX;
        p->in[i][1] = 0.0;
    }
    for (;i<p->fft_len;i++)
    {
        p->in[i][0] = 0.0;
        p->in[i][1] = 0.0;
    }
    fftw_execute(p->sigpl);
    double mulx = 1.0/p->fft_len;
    for (i=0;i<p->fft_len;i++)
    {
        double a = p->out[i][0];
        double b = p->out[i][1];
        double c = p->refout[i][0];
        double d = p->refout[i][1];
        p->revin[i][0] = (a*c - b*d) * mulx;
        p->revin[i][1] = (b*c + a*d) * mulx;
    }
    
    fftw_execute(p->revpl);
    for (i=0;i<p->inti.nsamples;i++)
    {
        double Xcorr = p->revout[i][0]/p->refwind_len;
        p->trcout._samples[i] = Xcorr;
        if (p->stat_max_xcorr < Xcorr)
        {
            p->stat_max_xcorr = Xcorr;
            p->stat_max_xcorr_at = i;
        }
    }
    p->trcout.copydatafrom(trace);
    //p->ao.on_result_trace(&p->trcout);
    const_cast<trace_t*>(trace)->_opaque = (void*) &p->trcout;
    return trace;
}
void st_norm_cross_corr_t::finit()
{
    auto p = _pimpl.get();

    LogStat(LOG_STAT_INTL, "Max Normalised Cross Correlation (NCC): %lf, at %d\n ", 
            p->stat_max_xcorr, p->stat_max_xcorr_at);
    fftw_free(p->in);
    fftw_free(p->out);
    fftw_free(p->refin);
    fftw_free(p->refout);
    fftw_free(p->revin);
    fftw_free(p->revout);

}
st_norm_cross_corr_t::~st_norm_cross_corr_t() 
{
}
