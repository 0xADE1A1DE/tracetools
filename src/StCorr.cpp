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
#include "StCorr.h"
#include "CorrAnalysis.h"
#include "AnalysisOuput.h"
#include "Log.h"
#include "Trace.h"
#include "Variant.h"
#include "WorkContext.h"
#include "PWFactory.h"
#include "TraceReader.h"
#include "MeanAnalysis.h"
#include "Passes.h"

#include <cstdio>
#include <sstream>

#define MAX_CORR_TESTS 32

struct st_corr_priv_t
{
    corr_analysis_t* corr;
    analysis_output_t* ao;
    std::string ext;
    std::string filename;
    tracereader_t* reader;
    traceinfo_t traceinfo;
    traceinfo_t winti;
    wc_gopts_t opts_map;
    int datarange_begin;
    int datarange_end;
    int current_pass;
};

st_corr_t::st_corr_t()
{
    pimpl.init();
}
static std::vector<double> s_datavals;
void st_corr_t::init(const traceinfo_t* inti, traceinfo_t* outti) 
{
    pimpl.get()->opts_map = { 
        {"corr_model", (std::function<void(int,const trace_t*,double**)>) 
            [](int data_index, const trace_t* trace, double** datavals) 
                {
                    int stdinval=0;
                    double stddval=0;
                    for (auto g=0u;g<s_datavals.size();g++)
                    {
                        int ret = scanf("%lf\n", &stddval);
                        if (ret != 1)
                        {
                            LogErrDie("failed read from stdin!\n");
                        }
                        s_datavals[g] = (double)stddval;
                    }
                    *datavals = s_datavals.data();
                }}
        };

    wc_gopts_merge(pimpl.get()->opts_map);
    variant_t defbeg(1), defend(16);

    int r_begin = wc_gopts_get("data_range_begin", defbeg).get_int();
    int r_end = wc_gopts_get("data_range_end", defend).get_int();
    PWASSERT((MAX_CORR_TESTS > r_begin) && (MAX_CORR_TESTS > r_end), "correlation test count too large");
    PWASSERT(r_end >= r_begin, "data range end must be larger than data range begin");
    pimpl.get()->datarange_begin = r_begin;
    pimpl.get()->datarange_end = r_end;
    pimpl.get()->traceinfo = *inti;
    pimpl.get()->ao = new analysis_output_t[MAX_CORR_TESTS];
    pimpl.get()->corr = new corr_analysis_t[MAX_CORR_TESTS];
    pimpl.get()->current_pass = -1;
    for (int i=r_begin;i<=r_end;i++)
    {
        int normidx = i;
        std::stringstream ssaofile;
        ssaofile<<"corr-"<<normidx<<".npy";

        std::string outfilepath = wc_generate_path(inti->title.c_str(), ssaofile.str().c_str());
        pimpl.get()->ao[normidx].init(outfilepath.c_str(),"numpy");
        pimpl.get()->corr[normidx].init(inti, &pimpl.get()->ao[normidx], {{"corr_data_index", i}});
    }
    if (wc_gopts_has("corr_guess_count"))
    {
        LogInfo("detected guess count\n");
        s_datavals.resize(wc_gopts_get("corr_guess_count").get_int());
    }
    else
    {
        s_datavals.resize(1);
    }

    traceinfo_print("", inti);
}

const trace_t* st_corr_t::process(const trace_t* trace)
{
    auto p = pimpl.get();
    trace_t tempout(&trace->_traceinfo);
    trace->copyto(&tempout, 0);
    pwpass_moving_average(&tempout);
    for (int i=p->datarange_begin;i<=p->datarange_end;i++)
    {
        pimpl.get()->corr[i].trace_submit(&tempout);
    }
    return trace;
}
void st_corr_t::finit()
{
    auto p = pimpl.get();
    for (int i=p->datarange_begin;i<=p->datarange_end;i++)
    {
        pimpl.get()->corr[i].finit();
    }
    if (p->ao)
        delete[] p->ao;
    if (p->corr)
        delete[] p->corr;
}
st_corr_t::~st_corr_t()
{
}
