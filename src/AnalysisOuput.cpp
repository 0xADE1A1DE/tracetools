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
#include <string>
#include <sstream>
#include "AnalysisOuput.h"
#include "PWFactory.h"
#include "Log.h"

struct analysis_output_priv_t
{ 
    std::string outputfile;
    tracewriter_t* writer;
    traceinfo_t traceinfo;
    size_t finaltracecount;
    bool issub;
    std::vector<analysis_output_t*> children;
    analysis_output_priv_t() :
         writer(nullptr)
        ,finaltracecount(0)
        ,issub(false)
    {}
    
};

analysis_output_t::analysis_output_t(const analysis_output_t&) = default;
analysis_output_t::analysis_output_t(analysis_output_t&&) = default;
analysis_output_t::analysis_output_t()
{
    pimpl.init();
}
void analysis_output_t::init(const char* outputfile, 
                             const std::string& kind)
{
    pimpl.get()->writer = pwfactory_t::get_instance()->new_tracewriter(kind);
    pimpl.get()->outputfile = outputfile;
    pimpl.get()->finaltracecount = 0;
}
void analysis_output_t::init(const traceinfo_t* traceinfo)
{
    PWASSERT(pimpl.get()->writer != nullptr, "writer not init");
    pimpl.get()->writer->open_file(pimpl.get()->outputfile.c_str(), 
                                   traceinfo, {{"numpy",""}});
    pimpl.get()->traceinfo = *traceinfo;
}
analysis_output_t* analysis_output_t::add_sub_output(const std::string& id, const std::string& kind)
{
    auto dotpos = pimpl.get()->outputfile.find_last_of('.');
    auto afterdot = pimpl.get()->outputfile.substr(dotpos+1);
    auto beforedot = pimpl.get()->outputfile.substr(0, dotpos);
    std::stringstream ss;
    ss << beforedot;
    ss << "-" << id << ".";
    ss << afterdot;
    auto ao = new analysis_output_t();
    ao->init(ss.str().c_str(), kind); 
    pimpl.get()->children.push_back(ao);
    return ao;
}
analysis_output_t* analysis_output_t::get_sub_output(size_t index)
{
    return pimpl.get()->children[index];
}
void analysis_output_t::on_result_trace(const trace_t* trace)
{
    pimpl.get()->writer->write_trace(trace);
    pimpl.get()->finaltracecount++;
}
void analysis_output_t::update_written_count()
{
    auto p = pimpl.get();
    auto ti = p->traceinfo;
    ti.ntraces = p->finaltracecount;
    p->writer->update_traceinfo(&ti);
}
size_t analysis_output_t::get_written_count()
{
    return pimpl.get()->finaltracecount;
}
size_t analysis_output_t::finit()
{
    auto p = pimpl.get();
    
    for (auto& child : p->children)
    {
        child->finit();
        delete child;
    }
    p->children.clear();
    
    p->traceinfo.ntraces = p->finaltracecount;
    p->writer->finit(&p->traceinfo);
    return p->finaltracecount;
}
analysis_output_t::~analysis_output_t()
{
}
