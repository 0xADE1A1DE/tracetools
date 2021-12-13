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
#include "ChisquareTest.h"
#include "Analysis.h"
#include "Trace.h"
#include "Variant.h"
#include "WorkContext.h"
#include "TraceUtils.h"
#include "AnalysisOuput.h"
#include <vector>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <boost/math/distributions/chi_squared.hpp>

typedef double real_t;

struct lv_chisquare_hist_t
{
    std::vector<int> bins;
};

struct chisquare_test_priv_t
{
    std::vector<lv_chisquare_hist_t> hists[2];
    std::vector<lv_chisquare_hist_t> copyhists[2];
    int copydone[2];
    int traces[2];
    double min;
    double normdiv;
    int nbins;
    traceinfo_t ti;
    traceinfo_t tout;
    int m;
    analysis_output_t *ao;
};
chisquare_test_t::chisquare_test_t(const chisquare_test_t&) = default;
chisquare_test_t::chisquare_test_t(chisquare_test_t&&) = default;
chisquare_test_t::chisquare_test_t()
{
    _pimpl.init();
}

void chisquare_test_t::init(const traceinfo_t *traceinfo, 
        analysis_output_t* output, const analysis_opts_t& opts)
{
    _pimpl.init();
    // make init size non zero
    int ns = (int) traceinfo->nsamples;
    _pimpl.get()->hists[0].resize(ns);
    _pimpl.get()->hists[1].resize(ns);
    _pimpl.get()->nbins = wc_gopts_get("chisq_bin_count", variant_t(256)).get_int();
    _pimpl.get()->min = wc_gopts_get("chisq_trace_min").get_double();
    _pimpl.get()->normdiv = wc_gopts_get("chisq_trace_div").get_double();
    _pimpl.get()->ti = *traceinfo;
    _pimpl.get()->traces[0] = 0;
    _pimpl.get()->traces[1] = 0;
    _pimpl.get()->copydone[0] = 0;
    _pimpl.get()->copydone[1] = 0;
    _pimpl.get()->m = wc_gopts_get("at_each_ntraces").get_int();
    _pimpl.get()->ao = output;

    traceinfo_t tout = *traceinfo;
    tout.ntraces = tout.ntraces/_pimpl.get()->m;
    tout.ndata = 0;
    tout.nterms = 1;
    output->init(&tout);

    _pimpl.get()->tout = tout;
    // init bins with 0
    for(int i=0;i<2;i++)
    {
        for(int j=0;j<ns;j++)
        {
            _pimpl.get()->hists[i][j].bins.resize(_pimpl.get()->nbins);
            for(int k=0;k<_pimpl.get()->nbins;k++) 
            {
                _pimpl.get()->hists[i][j].bins[k] = 0;
            }
        }
    }
}
void chisquare_test_t::trace_submit(const trace_t* trace)
{
    uint32_t g = traceutils_group(trace);
    std::vector<lv_chisquare_hist_t>& hists = _pimpl.get()->hists[g];
    auto ns = (int) trace->_traceinfo.nsamples;
    auto nbins = _pimpl.get()->nbins;
    _pimpl.get()->traces[g]++;
    // Add values to bins
    for(int i=0;i<ns;i++) 
    {
        int bin = (int)round((trace->_samples[i] - _pimpl.get()->min)* nbins / _pimpl.get()->normdiv);
        //int bin = round(trace->_samples[i] * nbins);
        //LogInfo("xx%d\n", bin);
        if(bin < 0)
            bin = 0;
        if(bin >= nbins)
            bin = nbins-1;
        hists[i].bins[bin]++;
    }
    auto m = _pimpl.get()->m;
    if ((_pimpl.get()->traces[g] % m) == 0)
    {
        _pimpl.get()->copyhists[g] = hists;
        _pimpl.get()->copydone[g] = _pimpl.get()->traces[g];
        LogInfo("ff %d %d\n", g,_pimpl.get()->traces[g]);
    }

    if ((_pimpl.get()->copydone[0] == _pimpl.get()->copydone[1]) && 
            (_pimpl.get()->copydone[0] > 0))
    {
        trace_t restrace;
        restrace.init(&_pimpl.get()->tout);

        for (int i=0;i<ns;i++)
        {
            restrace._samples[i] = calc_chisq_at(i);
        }
        _pimpl.get()->ao->on_result_trace(&restrace);

        _pimpl.get()->copydone[0] = 0;
        _pimpl.get()->copydone[1] = 0;
    }

}
double chisquare_test_t::calc_chisq_at(int index)
{
    auto nbins = _pimpl.get()->nbins;
  real_t F[2][nbins];
  real_t E[2][nbins];

  for(int i=0;i<nbins;i++) 
  {
      F[0][i] = _pimpl.get()->copyhists[0][index].bins[i];
      F[1][i] = _pimpl.get()->copyhists[1][index].bins[i];
  }

  //Calculate expected frequency
  for(int i=0;i<2;i++)
  {
    for(int j=0;j<nbins;j++) 
    {
      real_t val = 0;
      for(int k=0;k<nbins;k++)
      {
        val += F[i][k];
      }
      val *= F[0][j] + F[1][j];
      E[i][j] = val / (2 *_pimpl.get()->copydone[i]);
    }
  }
  // Calculate x and v
  real_t x = 0;
  for(int i=0;i<2;i++)
  {
    for(int j=0;j<nbins;j++)
    {
      real_t temp;
      temp = F[i][j] - E[i][j];
      if (E[i][j] != 0.0)
          x += (temp*temp) / E[i][j];
    }
  }
  int v = 1 * (nbins-1);
  boost::math::chi_squared chi_dist(v);
  return -1 * log10( 1 - boost::math::cdf(chi_dist, x));
}
void chisquare_test_t::finit()
{
    _pimpl.get()->ao->finit();
}

chisquare_test_t::~chisquare_test_t()
{
}
