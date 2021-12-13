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
#include "PWFactory.h"
#include "TraceReaderNpy.h"
#include "TraceReaderXrc.h"
#include "TraceReaderTrs.h"
#include "TraceWriterWrapper.h"
#include "TraceWriterNpy.h"
#include "TraceWriterTrs.h"
#include "BivAnalysisMt.h"
#include "FirstOrderTTestAnalysis.h"
#include "SecondOrderTTestAnalysis.h"
#include "ChisquareTest.h"
#include "CorrAnalysis.h"
#include "CpUnivFirstOrderTTest.h"
#include "CpUnivSecondOrderTTest.h"
#include "CpBivFirstOrderTTest.h"
#include "CpTrivFirstOrderTTest.h"
#include "CpFileUnivFirstOrderTTest.h"
#include "CpFileBivFirstOrderTTest.h"
#include "CpFileBivFirstOrderTTestMt.h"
#include "CpFileTrivFirstOrderTTest.h"
#include "CpFilter.h"
#include "CpConvert.h"
#include "CpCompress.h"
#include "CpStack.h"
#include "CpDynLib.h"
#include "Log.h"

pwfactory_t* pwfactory_t::get_instance()
{
    static pwfactory_t fac;
    return &fac;
}
tracereader_t* pwfactory_t::new_tracereader(const std::string& kind)
{
    if (kind == "numpy" || kind == "npy")
        return new tracereader_npy_t();
    else if (kind == "xrc")
        return new tracereader_xrc_t();
    else if (kind == "trs")
        return new tracereader_trs_t();
    LogErrDie("unknown trace reader '%s'\n", kind.c_str());
    return nullptr;
}
tracewriter_t* pwfactory_t::new_tracewriter(const std::string& kind)
{
    if (kind == "numpy" || kind == "npy")
        return new tracewriter_wrapper_t(new tracewriter_npy_t());
    if (kind == "trs")
        return new tracewriter_wrapper_t(new tracewriter_trs_t());
    LogErrDie("unknown trace writer '%s'\n", kind.c_str());
    return nullptr;
}
analysis_t* pwfactory_t::new_analysis(const std::string& kind)
{
    
    if (kind == "first-order-ttest")
        return new first_order_ttest_analysis_t();
    else if (kind == "second-order-ttest")
        return new second_order_ttest_analysis_t();
    else if (kind == "mt,first-order-ttest")
    {
        biv_analysis_mt_t *bt = new biv_analysis_mt_t();
        bt->init_threaded<first_order_ttest_analysis_t, analysis_output_t>(4);
        
        return bt;
    }
    else if (kind == "chisquare")
        return new chisquare_test_t();
    else if (kind == "corr")
        return new corr_analysis_t();
    //else if (kind == "second-order-ttest")
    //    return new second_order_ttest_analysis_t();
    return nullptr;
}
cmd_pipeline_t* pwfactory_t::new_cmdpipeline(const std::string& kind)
{
    if (kind == "uni-variate,first-order-ttest" || kind == "rosita,univ,first-order-ttest")
        return new cp_univ_first_order_ttest_t();
    else if (kind == "uni-variate,second-order-ttest" || kind == "rosita,univ,second-order-ttest")
        return new cp_univ_second_order_ttest_t();
    else if (kind == "bi-variate,first-order-ttest" || kind == "rosita,biv,first-order-ttest")
        return new cp_biv_first_order_ttest_t();
    else if (kind == "tri-variate,first-order-ttest" || kind == "rosita,triv,first-order-ttest")
        return new cp_triv_first_order_ttest_t();
    else if (kind == "file,uni-variate,first-order-ttest" || kind == "univfott")
        return new cp_file_univ_first_order_ttest_t();
    else if (kind == "file,bi-variate,first-order-ttest" || kind == "bivfott")
        return new cp_file_biv_first_order_ttest_t();
    else if (kind == "file,mt,bi-variate,first-order-ttest" || kind == "bivfott,mt")
        return new cp_file_biv_first_order_ttest_mt_t();
    else if (kind == "file,tri-variate,first-order-ttest" || kind == "trivfott")
        return new cp_file_triv_first_order_ttest_t();
    else if (kind == "filter")
        return new cp_filter_t();
    else if (kind == "convert")
        return new cp_convert_t();
    else if (kind == "compress")
        return new cp_compress_t();
    else if (kind == "library")
        return new cp_dynlib_t();
    else 
        return cp_stack_t::make_instance(kind);
    return nullptr;
}
