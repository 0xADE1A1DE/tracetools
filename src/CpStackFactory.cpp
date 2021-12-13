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
#include "CpStack.h"
#include "StMean.h"
#include "StNorm.h"
#include "StCorr.h"
#include "StFott.h"
#include "StChisq.h"
#include "StTraceCombiner.h"
#include "StNormCrossCorr.h"
#include "StPattern.h"
#include "StStaticAlign.h"
static cp_stack_t* make_normcombcorr()
{
    auto stmean = new st_mean_t();
    auto stnorm = new st_norm_t(stmean);
    auto stcorr = new st_corr_t();
    auto stcomb = new st_trace_combiner_t();
    
    auto cps = new cp_stack_t();
    cps->stack_add(stmean, 0);
    cps->stack_add(stnorm, 1);
    cps->stack_add(stcomb, 1);
    cps->stack_add(stcorr, 1);
    return cps;
}
static cp_stack_t* make_corr()
{
    auto stcorr = new st_corr_t();
    
    auto cps = new cp_stack_t();
    cps->stack_add(stcorr, 0);
    return cps;
}
static cp_stack_t* make_normcombfott()
{
    auto stfott = new st_fott_t();
    auto stmean = new st_mean_t();
    auto stnorm = new st_norm_t(stmean);
    auto stcomb = new st_trace_combiner_t();
    auto cps = new cp_stack_t();

    cps->stack_add(stmean, 0);
    cps->stack_add(stnorm, 1);
    cps->stack_add(stcomb, 1);
    cps->stack_add(stfott, 1);
    return cps;
}
static cp_stack_t* make_chisq()
{
    auto stchisq = new st_chisq_t();
    auto cps = new cp_stack_t();

    cps->stack_add(stchisq, 0);
    return cps;
}
static cp_stack_t* make_patternmatch()
{
    auto stcorr = new st_norm_cross_corr_t();
    auto stpat = new st_pattern_t();
    auto cps = new cp_stack_t();
    cps->stack_add(stcorr, 0);
    cps->stack_add(stpat, 0);
    return cps;
}
static cp_stack_t* make_staticalign()
{
    auto stcorr = new st_norm_cross_corr_t();
    auto stalign = new st_static_align_t();
    auto cps = new cp_stack_t();
    cps->stack_add(stcorr, 0);
    cps->stack_add(stalign, 0);
    return cps;
}
cp_stack_t* cp_stack_t::make_instance(const std::string& name)
{
    if (name == "normcombcorr")
    {
        return make_normcombcorr();
    }
    else if (name == "corr")
    {   
        return make_corr();
    }
    else if (name == "normcombfott")
    {   
        return make_normcombfott();
    }
    else if (name == "chisq")
    {
        return make_chisq();
    }
    else if (name == "staticalign")
    {
        return make_staticalign();
    }
    else if (name == "patternmatch")
    {
        return make_patternmatch();
    }
    return nullptr;
}

