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
#include "SafeQueue.h"
#include "Allocator.h"
#include "Perf.h"
#include "Vec2.h"
#include "AnalysisOuput.h"
#include "BivMergeOutput.h"
#include "BivAnalysisMt.h"
#include "WorkContext.h"

#include <unistd.h>
#include <thread>
#include <sstream>

struct thread_data_t
{
    SafeQueue<trace_t*>* _qin;
    SafeQueue<trace_t*>* _qout;
    analysis_t* _a;
    analysis_output_t* _ao;
    trace_t _temptrace;
    std::thread _thread;
    thread_data_t() :
         _qin(nullptr)
        ,_qout(nullptr)
    {
    }
};
struct biv_analysis_mt_priv_t
{
    traceinfo_t _traceinfo;
    traceinfo_t _subtrace;
    traceinfo_t _subtraceinfoS;
    traceinfo_t _subtraceinfoT;
    Vec2<thread_data_t>* _tdata;
    int _ns;
    size_t _sent;
    size_t _received;
};

void thread_run_S(biv_analysis_mt_priv_t* priv, int row, int col)
{
    auto& qin = *priv->_tdata->at(row, col)._qin;
    auto& qout = *priv->_tdata->at(row, col)._qout;
    auto& a = priv->_tdata->at(row, col)._a;
    auto& _temptrace = priv->_tdata->at(row, col)._temptrace;
    int ns = priv->_ns;
    while (true)
    {
        trace_t* trace = nullptr;
        trace = qin.pop();
        if (trace == nullptr)
        {
            a->finit();
            break;
        }
        for (int i=0;i<ns;i++)
        {
            for (int j=0;j<ns;j++)
            {
                _temptrace._samples[i*ns + j] = trace->_samples[i+row*ns] * trace->_samples[j+col*ns];
            }
        }
        
        memcpy(&_temptrace._data[0], &trace->_data[0], trace->_traceinfo.ndata * sizeof(uint8_t));
        // work
        a->trace_submit(&_temptrace);
        qout.push(trace);
    }
}
void thread_run_T(biv_analysis_mt_priv_t* priv, int row, int col)
{
    auto& qin = *priv->_tdata->at(row, col)._qin;
    auto& qout = *priv->_tdata->at(row, col)._qout;
    auto& a = priv->_tdata->at(row, col)._a;
    auto& _temptrace = priv->_tdata->at(row, col)._temptrace;
    int ns = priv->_ns;
    while (true)
    {
        trace_t* trace = nullptr;
        trace = qin.pop();
        if (trace == nullptr)
        {
            a->finit();
            break;
        }
        int cc = 0;
        for (int i=0;i<ns;i++)
        {
            for (int j=i+1;j<ns;j++)
            {
                _temptrace._samples[cc] = trace->_samples[i+row*ns] * trace->_samples[j+col*ns];
                cc ++;
            }
        }
        memcpy(&_temptrace._data[0], &trace->_data[0], trace->_traceinfo.ndata * sizeof(uint8_t));
        // work
        a->trace_submit(&_temptrace);
        qout.push(trace);
    }
}
int nsum(int n)
{
    return n * (n + 1) / 2;
}
biv_analysis_mt_t::biv_analysis_mt_t() = default;
biv_analysis_mt_t::biv_analysis_mt_t(const biv_analysis_mt_t&) = default;
biv_analysis_mt_t::biv_analysis_mt_t(biv_analysis_mt_t&&) = default;
void biv_analysis_mt_t::init(const traceinfo_t *traceinfo, 
                      analysis_output_t* output, 
                      const analysis_opts_t& opts)
{
    pimpl.init();
    pimpl.get()->_traceinfo = *traceinfo;
    pimpl.get()->_subtrace = *traceinfo;
    pimpl.get()->_subtrace.nsamples = pimpl.get()->_subtrace.nsamples * pimpl.get()->_subtrace.nterms / _nsplits;
    pimpl.get()->_subtrace.nterms = 1;
    
    _nt = nsum(_nsplits);
    pimpl.get()->_sent = 0;
    pimpl.get()->_received = 0;
    pimpl.get()->_ns = traceinfo->nsamples / _nsplits;
    
    PWASSERT( traceinfo->nsamples % _nsplits == 0, "number of samples must divide by split count");
    pimpl.get()->_tdata = new Vec2<thread_data_t>();
    pimpl.get()->_tdata->init(_nsplits, _nsplits);
    
    // for _nsplits = 4, there will be 4 triangle shape threads, and 6 (nsum(4 - 1))
    //  i -->
    //  j \/
    //
    //    0 1 2 3
    //  0 T
    //  1 S T 
    //  2 S S T
    //  3 S S S T
    
    
    // all blocks
    for (int i=0;i<_nsplits;i++) // -->
    {
        for (int j=i;j<_nsplits;j++) // \/
        {
            auto& cell = pimpl.get()->_tdata->at(i, j);
            cell._qin = new SafeQueue<trace_t*>();
            cell._qout = new SafeQueue<trace_t*>();
    
            cell._ao = _make_analysis_output();
            cell._a = _make_analysis();
        }
    }
    pimpl.get()->_subtraceinfoS = *traceinfo;   
    pimpl.get()->_subtraceinfoS.nsamples = pimpl.get()->_subtraceinfoS.nsamples / _nsplits;
    pimpl.get()->_subtraceinfoS.nsamples *= pimpl.get()->_subtraceinfoS.nsamples;
    
    pimpl.get()->_subtraceinfoT = *traceinfo;   
    pimpl.get()->_subtraceinfoT.nsamples = pimpl.get()->_subtraceinfoT.nsamples / _nsplits;
    pimpl.get()->_subtraceinfoT.nsamples = pimpl.get()->_subtraceinfoT.nsamples * (pimpl.get()->_subtraceinfoT.nsamples - 1)/2;
    // S
    for (int i=0;i<_nsplits;i++)
    {
        for (int j=i+1;j<_nsplits;j++)
        {
            auto& cell = pimpl.get()->_tdata->at(i, j);
            wc_path_gen_t pgen(traceinfo->title);
            pgen.add("biv-output");
            pgen.add(i);
            pgen.add(j);
            cell._ao->init(pgen.getpath(".npy").c_str(), "numpy");
            cell._a->init(&pimpl.get()->_subtraceinfoS, cell._ao);
            cell._temptrace.init(&pimpl.get()->_subtraceinfoS);
            
            cell._thread = std::thread(thread_run_S, pimpl.get(), i, j);
        }
    }
    
    for (int i=0;i<_nsplits;i++)
    {
        int j = i;
        auto& cell = pimpl.get()->_tdata->at(j, i);
        
        wc_path_gen_t pgen(traceinfo->title);
        pgen.add("biv-output");
        pgen.add(i);
        pgen.add(j);
        cell._ao->init(pgen.getpath(".npy").c_str(), "numpy");
        cell._a->init(&pimpl.get()->_subtraceinfoT, cell._ao);
        cell._temptrace.init(&pimpl.get()->_subtraceinfoT);
        
        cell._thread = std::thread(thread_run_T, pimpl.get(), i, j);
    }
}
void biv_analysis_mt_t::trace_submit(const trace_t *trace) 
{
    auto p = pimpl.get();
    for (int i=0;i<_nsplits;i++)
    {
        for (int j=i;j<_nsplits;j++)
        {
            auto& cell = pimpl.get()->_tdata->at(i, j);
            trace_t* copytrace = allocator_t<trace_t>::get_instance()->allocate(1);
            
            new (copytrace) trace_t(*trace);
            //trace->copyto(copytrace,0);
            p->_sent ++;
            cell._qin->push(copytrace);
        }
    }
    
    
    for (int i=0;i<_nsplits;i++)
    {
        for (int j=i;j<_nsplits;j++)
        {
            auto& cell = pimpl.get()->_tdata->at(i, j);
            while (cell._qout->isempty() == false)
            {
                auto trace = cell._qout->pop();
                trace->~trace_t();
                p->_received++;
                allocator_t<trace_t>::get_instance()->deallocate(trace);
            }
        }
    }
    
    if (p->_sent > p->_received + 2000000)
    {
        LogInfo("long backlog, pausing reads\n");
        sleep(1);
    }
    //LogInfo("ss%lf\n", 1.0/d.get_elapsed());
}
void biv_analysis_mt_t::finit() 
{
    auto p = pimpl.get();
    // S
    for (int i=0;i<_nsplits;i++)
    {
        for (int j=i+1;j<_nsplits;j++)
        {
            p->_tdata->at(i, j)._qin->push(nullptr);
            p->_tdata->at(i, j)._thread.join();
        }
    }
    // T
    for (int i=0;i<_nsplits;i++)
    {
        int j = i;
        p->_tdata->at(i, j)._qin->push(nullptr);
        p->_tdata->at(i, j)._thread.join();
    }
    wc_path_gen_t pgen0(p->_traceinfo.title);
    wc_path_gen_t pgen0out(p->_traceinfo.title);
    
    wc_path_gen_t pgen1(p->_traceinfo.title);
    wc_path_gen_t pgen1out(p->_traceinfo.title);
    
    wc_path_gen_t pgen2(p->_traceinfo.title);
    wc_path_gen_t pgen2out(p->_traceinfo.title);
    
    biv_merge_output(pgen0.getpath("-biv-output-%d-%d.npy"), pgen0out.getpath("-biv-output.npy"),
                     _nsplits, p->_traceinfo.nsamples, 
                     p->_tdata->at(0,0)._ao->get_written_count());
    /** ugly but works until we'd need to use mt-biv some other analysis **/
    biv_merge_output(pgen1.getpath("-biv-output-%d-%d-var-0.npy"), pgen1out.getpath("-biv-output-var-0.npy"),
                     _nsplits, p->_traceinfo.nsamples, 
                     p->_tdata->at(0,0)._ao->get_written_count());
    biv_merge_output(pgen2.getpath("-biv-output-%d-%d-var-1.npy"), pgen2out.getpath("-biv-output-var-1.npy"),
                     _nsplits, p->_traceinfo.nsamples, 
                     p->_tdata->at(0,0)._ao->get_written_count());
    
}

biv_analysis_mt_t::~biv_analysis_mt_t()
{
    
}
