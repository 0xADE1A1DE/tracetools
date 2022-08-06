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
#include "Log.h"
#include "Util.h"
#include "Allocator.h"
#include <cstring>
#ifdef DEBUG
#include <thread>
#endif
template <class T>
class refcountedptr_t
{
public:
    refcountedptr_t() :
        _ptr(nullptr)
        ,_refcountptr(nullptr)
#ifdef DEBUG
        ,_length(0)
        ,_tid(std::this_thread::get_id())
#endif
    {
    }
    void init(size_t length)
    {
        PWASSERT(_refcountptr == nullptr, "double init");
        _ptr = allocator_t<T>::get_instance()->allocate(length);
        LogInfoV(2, "init %p\n", _ptr);
        _refcountptr = allocator_t<size_t>::get_instance()->allocate(1);
        *_refcountptr = 1;
#ifdef DEBUG
        _length = length;
        PWASSERT(_tid == std::this_thread::get_id(), "tid mismatch!");
        _tid = std::this_thread::get_id();
#endif
    }
    refcountedptr_t(const refcountedptr_t& other)
    {
        _refcountptr = other._refcountptr;
#ifdef DEBUG
        _length = other._length;
        _tid = std::this_thread::get_id();
	if ((*_refcountptr % 100 == 0) && (*_refcountptr > 100))
	{
	    LogInfo("rr %p %lu\n", other._ptr, *_refcountptr);
	}
#endif
        LogInfoV(2, "copy %p\n", other._ptr);
        if (_refcountptr)
        {        
            (*_refcountptr) ++;
        }
        else
        {
            LogErr("copying empty ref\n");
        }
        _ptr = other._ptr;
    }
    refcountedptr_t(refcountedptr_t&& other) noexcept
    {
        _refcountptr = other._refcountptr;
#ifdef DEBUG
        _length = other._length;
        _tid = std::this_thread::get_id();
#endif
        if (_refcountptr == nullptr)
        {
            LogErr("copying empty ref\n");
        }
        
        LogInfoV(1, "move %p\n", other._ptr);
        other._refcountptr = nullptr;
        _ptr = other._ptr;
    }
    T* operator->() 
    {
        PWASSERT(_refcountptr != nullptr, "");
        return _ptr;
    }
    const T* operator->() const
    {
        PWASSERT(_refcountptr != nullptr, "");
        return _ptr;
    }
    inline T& operator[](size_t index) 
    {
        PWASSERT(_refcountptr != nullptr, "");
#ifdef DEBUG
        PWASSERT(index < _length, "out of bound access");
#endif
        return _ptr[index];
    }
    inline const T& operator[](size_t index) const
    {
        PWASSERT(_refcountptr != nullptr, "");
#ifdef DEBUG
        PWASSERT(index < _length, "out of bound access");
#endif
        return _ptr[index];
    }
    T* getptr()
    {
        PWASSERT(_refcountptr != nullptr, "");
        return _ptr;
    }
    const T* getptr() const
    {
        PWASSERT(_refcountptr != nullptr, "");
        return _ptr;
    }
    ~refcountedptr_t()
    {
        if (_refcountptr == nullptr)
        {
            LogInfoV(1,"delete empty ref %s:%u\n", __FILE__, __LINE__);
            return;
        }
        
        if (*_refcountptr == 1)
        {
            LogInfoV(2, "release %p\n", _ptr);
#ifdef DEBUG
            PWASSERT(_tid == std::this_thread::get_id(), "tid mismatch!");
#endif
            allocator_t<T>::get_instance()->deallocate(_ptr);
            allocator_t<size_t>::get_instance()->deallocate(_refcountptr);
        }
        else
        {
            (*_refcountptr) --;
        }
    }
    
private:
    T* _ptr;
    size_t *_refcountptr;
#ifdef DEBUG
    size_t _length;
    std::thread::id _tid;
#endif
};
