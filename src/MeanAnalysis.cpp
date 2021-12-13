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
#include <vector>

#include "Trace.h"
#include "TraceUtils.h"
#include "Log.h"
#include "WorkContext.h"
#include "AnalysisOuput.h"
#include "MeanAnalysis.h"
struct mean_analysis_priv_t
{
    traceinfo_t traceinfo;
    traceinfo_t meantrinfo;
    std::vector<trace_t> meantraces;
    std::vector<trace_t> copymeantraces;
    std::vector<size_t> n;
    bool hasdata;
    size_t m;
};

void mean_analysis_t::init(const traceinfo_t *traceinfo, 
            analysis_output_t* output, 
            const analysis_opts_t& opts) 
{
    pimpl.get()->traceinfo = *traceinfo;
    pimpl.get()->hasdata = traceinfo->ndata != 0;
    traceinfo_print("mean::init", traceinfo);
    pimpl.get()->meantrinfo = *traceinfo;
    pimpl.get()->meantrinfo.ntraces = 1;
   
    int groups = traceutils_groups_count();
    pimpl.get()->meantraces.resize(groups);
    pimpl.get()->copymeantraces.resize(groups);
    for (int i=0;i<groups;i++)
    {
        pimpl.get()->meantraces[i].init(&pimpl.get()->meantrinfo);
        pimpl.get()->copymeantraces[i].init(&pimpl.get()->meantrinfo);
    }
    
    pimpl.get()->n.resize(groups);
    LogInfo("mean %p %s\n", this, traceinfo->title.c_str());
    pimpl.get()->m = wc_gopts_get("at_each_ntraces").get_int();
    
}
void mean_analysis_t::trace_submit(const trace_t *trace) 
{
    auto p = pimpl.get();
    uint8_t gid = 0;
    if (p->hasdata)
    {
        gid = traceutils_group(trace);
        auto oldsize = p->meantraces.size();
        if (gid >= p->meantraces.size())
        {
            p->meantraces.resize(gid+1);
            p->n.resize(gid+1);
            p->copymeantraces.resize(gid+1);
            for (auto i=oldsize;i<p->meantraces.size();i++)
            {
                p->meantraces[gid].init(&p->meantrinfo);
                p->copymeantraces[gid].init(&p->meantrinfo);
            }
        }
    }
    if ((p->n[gid] % p->m)  == 0)
    {
        p->copymeantraces[gid].copyfrom(&p->meantraces[gid]);
    }
    size_t n = p->n[gid];
    for (auto i=0u;i<p->traceinfo.nsamples*p->traceinfo.nterms;i++)
    {
        p->meantraces[gid]._samples[i] = (p->meantraces[gid]._samples[i] * n + trace->_samples[i])/(n+1);
    }
    p->n[gid] ++;
}

double mean_analysis_t::getmean(int groupid, int sampleid)
{
    auto p = pimpl.get();
    return p->meantraces[groupid]._samples[sampleid];
}
void mean_analysis_t::finit()
{
    auto p = pimpl.get();
    for (auto g=0u;g<p->n.size();g++)
    {
        analysis_output_t ao;
        for (auto i=0u;i<p->meantrinfo.nsamples*p->meantrinfo.nterms;i++)
        {
            p->copymeantraces[g]._samples[i] = p->meantraces[g]._samples[i];
        }
        LogInfo("mean group %lu count %lu\n", (unsigned long) g, (unsigned long) p->n[g]);
        std::vector<char> fsuffix(32);
        snprintf(fsuffix.data(), 31, "means-%lu.npy", (unsigned long) g);
        
        std::string outputfilepath = wc_generate_path(p->meantrinfo.title.c_str(), fsuffix.data());
        
        LogInfo("%s\n", outputfilepath.c_str()); 
        ao.init(outputfilepath.c_str(),"numpy");
        ao.init(&p->meantrinfo);
        ao.on_result_trace(&p->copymeantraces[g]);
        p->copymeantraces[g].print();
        ao.finit();
    }
}

mean_analysis_t::mean_analysis_t()
{
    pimpl.init();
}
mean_analysis_t::~mean_analysis_t()
{
    
}

