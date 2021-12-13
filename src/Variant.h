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

#include <functional>

class funcbase_t
{
public:
    virtual funcbase_t* copy() = 0;
    virtual ~funcbase_t() {}
};
template<typename T, typename... Args>
class func_t : public funcbase_t
{
public:
    typedef std::function<T(Args...)> fn_t;
    func_t(const func_t& oth) = default;
    func_t(func_t&& func) = default;
    func_t(fn_t& fn)
    {        
        _fn=fn;
    }
    funcbase_t* copy() override
    {
        return new func_t<T,Args...>(_fn);
    }
    T call(Args&... args)
    {
        return _fn(args...);
    }
    T call(Args&&... args)
    {
        return _fn(args...);
    }
    fn_t& get_func()
    {
        return _fn;
    }
private:
    fn_t _fn;
};

struct variant_t
{
    std::string strval;
    funcbase_t* funcval;
    double dblval;
    int intval;
    void* ptrval;
    
    template<class T, class... Args>
    variant_t(std::function<T(Args...)> func)
    {
        init();
        funcval = new func_t<T,Args...>(func);
    }
    variant_t(void* ptr)
    {
        init();
        ptrval = ptr;
    }
    variant_t(const char* str)
    {
        init();
        strval = std::string(str);
    }
    variant_t(const std::string& str)
    {
        init();
        strval = str;
    }
    variant_t(double dbl)
    {
        init();
        dblval = dbl;
    }
    variant_t(int i)
    {
        init();
        intval = i;
    }
    void init()
    {
        strval = "";
        funcval = nullptr;
        dblval = 0.0;
        intval = 0;
        ptrval = nullptr;
    }
    void* get_void_ptr() const { return ptrval; }
    int get_int() const { return intval; }
    double get_double() const { return dblval; }
    const std::string& get_string() const { return strval; }
    template<class T, class... Args>
    std::function<T(Args...)>& get_func() const 
    {
        PWASSERT(funcval != nullptr, "no func!");
        auto ptr = dynamic_cast<func_t<T,Args...>*>(funcval);
        PWASSERT(ptr != nullptr, "dynamic_cast failed");
        return ptr->get_func();
    }
    variant_t() = default;
    variant_t(const variant_t& oth)
    {
        strval = oth.strval;
        dblval = oth.dblval;
        intval = oth.intval;
        ptrval = oth.ptrval;
        funcval = nullptr;
        if (oth.funcval != nullptr)
        {
            funcval = oth.funcval->copy();
        }
    }
    variant_t(variant_t&& oth)
    {
        strval = std::move(oth.strval);
        dblval = oth.dblval;
        intval = oth.intval;
        funcval = oth.funcval;
        oth.funcval = nullptr;
    }
    template<class T, class... Args>
    T call(Args&&... args)
    {
         auto ptr = dynamic_cast<func_t<T,Args...>*>(funcval);
         PWASSERT(ptr != nullptr, "dynamic_cast failed");
         return ptr->call(args...);
    }
    
    template<class T, class... Args>
    T call(Args&... args)
    {
         auto ptr = dynamic_cast<func_t<T,Args...>*>(funcval);
         PWASSERT(ptr != nullptr, "dynamic_cast failed");
         return ptr->call(args...);
    }
    void operator=(const variant_t& oth)
    { 
         dblval = oth.dblval;
         intval = oth.intval;
         funcval = oth.funcval;
         strval = oth.strval;
    } 
    ~variant_t()
    {
        if (funcval)
        {
            delete funcval;
        }
    }
};
