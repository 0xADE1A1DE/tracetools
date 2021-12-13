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
#include "TraceWriterWrapper.h"
#include "Log.h"
#include "Trace.h"
#include "WorkContext.h"

#include <atomic>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <mutex>
namespace fs = boost::filesystem;
class fsuse_t
{
public:
    static fsuse_t *get_instance()
    {
        static fsuse_t singleton;
        return &singleton;
    }
    fsuse_t()
    {
        LogInfo("fsuse init\n");
        _total = 0;
        _checked = false;
    }
    void add(size_t filesize)
    {
        std::lock_guard<std::mutex> guard(_lock);
        _total += filesize;
    }
    inline void test_fsusage()
    {
        std::lock_guard<std::mutex> guard(_lock);
        if (_checked)
            return;
        fs::space_info si = fs::space(fs::current_path());
        LogInfo("disk available: %lu, required: %lu\n", si.available, _total);
        if ( si.available < _total)
        {
            LogErrDie("Not enough space\n");
        }
        _checked = true;
    }
private:
    std::mutex _lock;
    size_t _total;
    std::atomic<bool> _checked;
};
tracewriter_wrapper_t::tracewriter_wrapper_t(tracewriter_t* base)
{
    _base = base;
};

void tracewriter_wrapper_t::open_file(const char* tracepath, 
        const traceinfo_t* info, 
        const std::vector<traceopt_t>& opts)
{
    _base->open_file(tracepath, info, opts);
    _active = true;
    if (wc_gopts_has("force_space"))
    {
        _active = false;
        return;
    }
    size_t unitsize = 4;
    if (traceopt_has_key(opts, "datatype"))
    {
        auto dtype = traceopt_get_value(opts, "datatype");
        if (dtype == "double")
        {
            unitsize = 8;
        }
    }
    auto filesize = ((size_t) info->nsamples )* info->ntraces * info->nterms * unitsize;
    fsuse_t::get_instance()->add(filesize);
}
void tracewriter_wrapper_t::update_traceinfo(const traceinfo_t* newtraceinfo)
{
    _base->update_traceinfo(newtraceinfo);
}
int tracewriter_wrapper_t::write_trace(const trace_t* trace)
{
    if (_active)
        fsuse_t::get_instance()->test_fsusage();

    return _base->write_trace(trace);
}
void tracewriter_wrapper_t::finit(const traceinfo_t* finaltraceinfo)
{
    _base->finit(finaltraceinfo);
}
tracewriter_wrapper_t::~tracewriter_wrapper_t()
{
    delete _base;
}
