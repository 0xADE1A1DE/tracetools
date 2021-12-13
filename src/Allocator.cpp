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
#include "Allocator.h"
#include "Trace.h"
thread_local allocator_t<double>* __g_thread_instance_dbl = nullptr;
thread_local allocator_t<size_t>* __g_thread_instance_st = nullptr;
thread_local allocator_t<uint8_t>* __g_thread_instance_u8 = nullptr;
thread_local allocator_t<trace_t>* __g_thread_instance_trc = nullptr;

template<>
allocator_t<double>* allocator_t<double>::get_instance()
{
    if (__g_thread_instance_dbl == nullptr)
    {
        __g_thread_instance_dbl = new allocator_t<double>();
    }
    return __g_thread_instance_dbl;
}
template<>
allocator_t<size_t>* allocator_t<size_t>::get_instance()
{
    if (__g_thread_instance_st == nullptr)
    {
        __g_thread_instance_st = new allocator_t<size_t>();
    }
    return __g_thread_instance_st;
}
template<>
allocator_t<uint8_t>* allocator_t<uint8_t>::get_instance()
{
    if (__g_thread_instance_u8 == nullptr)
    {
        __g_thread_instance_u8 = new allocator_t<uint8_t>();
    }
    return __g_thread_instance_u8;
}
template<>
allocator_t<trace_t>* allocator_t<trace_t>::get_instance()
{
    if (__g_thread_instance_trc == nullptr)
    {
        __g_thread_instance_trc = new allocator_t<trace_t>();
    }
    return __g_thread_instance_trc;
}
