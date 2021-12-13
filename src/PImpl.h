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

template<class T>
class PImpl
{
public:
    PImpl() : _t(nullptr) {}
    PImpl(const PImpl& other)
    {
        _t = new T(*other._t);
    }
    PImpl(PImpl&& other)
    {
        _t = other._t;
        other._t = nullptr;
    }
    void init()
    {
        _t = new T();
    }
    T* get() 
    {
        return _t;
    }
    ~PImpl()
    {
        delete _t;
    }
private:
    T* _t;
};
