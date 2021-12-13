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
#pragma once
#include <map>
#include <deque>
#include <stdlib.h>
#include "Log.h"


template <class T>
class allocator_t
{
public:
    static allocator_t<T>* get_instance();
    
    T* allocate(int arraycount)
    {
        auto mm = freelist.find(arraycount);
        if (mm != freelist.end() && mm->second.size() > 0)
        {
            alloc_header_t* val = mm->second.back();
            T* ret = (T*) (&val[1]);
            mm->second.pop_back();
            return ret;
        }
        else
        {
            alloc_header_t* headerptr = (alloc_header_t*) malloc(sizeof(alloc_header_t) + sizeof(T)*arraycount);
            headerptr->length = arraycount;
            return (T*) (&headerptr[1]);
        }
    }
    void deallocate(T* t)
    {       
        alloc_header_t* headerptr = (alloc_header_t*) t;
        headerptr = headerptr - 1;

        freelist[headerptr->length].push_front( headerptr );
    }
private:
    struct alloc_header_t
    {
        int length;
    };
    std::map<int, std::deque<alloc_header_t*>> freelist;
};
