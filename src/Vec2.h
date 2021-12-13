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
#include <cstddef>
template <class T>
class Vec2
{
public:
    Vec2() :
        _data(nullptr)
    {}
    Vec2(const Vec2& other)
    {
        _data = new T[other._nrows*other._ncols];
        for (size_t i=0;i<other._ncols*other._nrows;i++)
        {
            _data[i] = other._data[i];
        }
        _ncols = other._ncols;
        _nrows = other._nrows;
    }
    Vec2(Vec2&& other)
    {
        _data = other._data;
        other._data = nullptr;
        _ncols = other._ncols;
        _nrows = other._nrows;
    }
    void init(size_t rows, size_t cols)
    {
        _data = new T[rows*cols];
        _ncols = cols;
        _nrows = rows;
    }
    inline T& at(size_t row, size_t col) 
    {
        return _data[row * _ncols + col];
    }
    inline const T& at(size_t row, size_t col) const
    {
        return _data[row * _ncols + col];
    }
    ~Vec2()
    {
        delete [] _data;
    }
private:
    T* _data;
    size_t _nrows;
    size_t _ncols;
};
