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
#include "Trace.h"
void cp_stack_t::get_info(cmd_pipeline_info_t& info)
{
    info.passes_count = _passes;
}

void cp_stack_t::init(const traceinfo_t *traceinfo, pwtermset_mode_t mode)
{
    for (auto i=0u;i<_passes;i++)
    {
        auto& stacks = _multistack[i];
        traceinfo_t inti, outti;
        inti = *traceinfo;
        for (auto& st : stacks)
        {
            st->init(&inti, &outti);
            inti = outti;
        }
    }
}
void cp_stack_t::stack_add(stackable_t* cp, int pass)
{
    if (pass >= (int)_multistack.size())
    {
    	_multistack.resize(pass*2+32);
    }
    _passes = std::max<uint32_t>(_passes, pass+1);
    _multistack[pass].push_back(cp);
}

void cp_stack_t::pass_begin(int pass)
{
    _current_pass = pass;
}
void cp_stack_t::process(const trace_t* trace)
{
    auto& stacks = _multistack[_current_pass];
    for (auto& st : stacks)
    {
        trace = st->process(trace);
    }
}
void cp_stack_t::pass_end(int pass)
{}
void cp_stack_t::finit()
{
    for (auto i=0u;i<_passes;i++)
    {
        auto& stacks = _multistack[i];
        for (auto& st : stacks)
        {
            st->finit();
        }
    }

}
