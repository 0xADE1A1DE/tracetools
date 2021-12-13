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
#include "FileProcessor.h"
#include "Log.h"
#include "TraceReader.h"
#include "PWFactory.h"
#include "Trace.h"
#include "TraceUtils.h"
#include "Perf.h"
#include "Util.h"
#include "WorkContext.h"
#include "../thirdparty/progress/progress.h"
struct file_processor_priv_t
{
    std::string ext;
    std::string filename;
    tracereader_t* reader;
    traceinfo_t readinfo;
    traceinfo_t wininfo;
    cmd_pipeline_t *cp;
    bool combine;
    int data_group_filter;
    int passes;
};
file_processor_t::file_processor_t()
{
    pimpl.init();
}
const traceinfo_t* file_processor_t::get_file_traceinfo()
{
    return &pimpl.get()->readinfo;
}
void file_processor_t::init(cmd_pipeline_t* cp, const std::string& filename, range_t* range)
{
    auto dotpos = filename.find_last_of(".");
    std::string ext;
    if (dotpos != std::string::npos)
    {
        ext = filename.substr(dotpos+1);
        pimpl.get()->ext = ext;
        pimpl.get()->filename = filename;
    }
    else
    {
        LogErrDie("invalid filename\n");
    }
    pimpl.get()->reader = pwfactory_t::get_instance()->new_tracereader(ext);
    pimpl.get()->reader->open_file(filename.c_str(), &pimpl.get()->readinfo);
    pimpl.get()->cp = cp;
    pimpl.get()->data_group_filter = -1;
    traceinfo_print("file opened", &pimpl.get()->readinfo);
    if (range == nullptr)
    {
        pimpl.get()->wininfo = pimpl.get()->readinfo;
    }
    else
    {
        traceinfo_t wti = pimpl.get()->readinfo;
        if (range->samplewinlen > 0)
        {
            wti.nsamples = range->samplewinlen;
        }
        if (range->tracerangelen > 0)
        {
            wti.ntraces = range->tracerangelen;
        }
        pimpl.get()->wininfo = wti;
    }
     
    if (wc_gopts_has("filter_by_group"))
    {
        pimpl.get()->data_group_filter = wc_gopts_get("filter_by_group").get_int();
    }
    
    cp->init(&pimpl.get()->wininfo, TS_IGNORE);
    cmd_pipeline_info_t cpinfo;
    cp->get_info(cpinfo);
    pimpl.get()->passes = cpinfo.passes_count;
}

void file_processor_t::process_window(size_t samplewinstart, size_t samplewinlen)
{
    auto p = pimpl.get();
    traceinfo_t wti = p->readinfo;
    wti.nsamples = samplewinlen;
    for (int i=0;i<p->passes;i++)
    {
        while (true)
        {
            trace_t trace;
            trace_t wintrace(&wti);
            if (p->reader->read_trace(&trace) == false)
            {
                p->reader->rewind();
                break;
            }
            
            trace.copyto(&wintrace, samplewinstart, samplewinlen);
            p->cp->process(&wintrace);
        }
    }
}
void file_processor_t::process_window(size_t samplewinstart, size_t samplewinlen, preprocess_func_t func)
{
    
}
void file_processor_t::process_range(const range_t& range)
{
    auto p = pimpl.get();
    traceutils_groups_count();
    range_t r = range;
    if (r.tracerangelen == 0)
    {
        r.tracerangelen = p->readinfo.ntraces;
    }
    if (r.samplewinlen == 0)
    {
        r.samplewinlen = p->readinfo.nsamples;
    }
    
    progress_t prg;
    for (int i=0;i<p->passes;i++)
    {
        LogInfo("running pass %d of %d\n", i, p->passes); 
        size_t tid = 0;
        progress_init(&prg, r.tracerangelen);
//        p->reader->seek(r.tracerangestart);
        p->cp->pass_begin(i);
        p->reader->seek(r.tracerangestart);
        while (tid < r.tracerangelen)
        {
            trace_t trace;
            trace_t combined;
            if (p->reader->read_trace(&trace) == false)
            {
                LogInfo("rewinding %d\n", i);
                p->reader->rewind();
                break;
            }
            if (p->data_group_filter != -1)
            {
                if (traceutils_group(&trace) != p->data_group_filter)
                {
                    progress_update(&prg, (unsigned long)tid);
                    tid++;
                    continue;
                }
            }
            if (r.samplewinlen == p->readinfo.nsamples)
            {
                p->cp->process(&trace);
            }
            else
            {
                trace_t wintrace(&p->wininfo);
                trace.copyto(&wintrace, r.samplewinstart, r.samplewinlen, 0);
                p->cp->process(&wintrace);
            }

            progress_update(&prg, (unsigned long)tid);
            tid ++;
        }
        p->cp->pass_end(i);
        progress_finit(&prg);
    }
    
}
void file_processor_t::process_all()
{
    auto p = pimpl.get();
    traceinfo_t wti = p->readinfo;
    progress_t prg;
    
    for (int i=0;i<p->passes;i++)
    {
        unsigned long tid = 0;
        progress_init(&prg, wti.ntraces);
        p->cp->pass_begin(i);
        while (true)
        {
            trace_t trace;
            if (p->reader->read_trace(&trace) == false)
            {
                LogInfo("rewinding %d\n", i);
                p->reader->rewind();
                break;
            }
            p->cp->process(&trace);
            progress_update(&prg, tid);
            tid ++;
        }
        p->cp->pass_end(i);
        progress_finit(&prg);
    }
    
}

void file_processor_t::process_all(std::function<void(trace_t*, const trace_t*)> preproc)
{
    
    
}

void file_processor_t::finit()
{
    auto p = pimpl.get();
    p->cp->finit();
}

file_processor_t::~file_processor_t()
{
}
