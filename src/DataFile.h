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

#include <fstream>
#include <string>
struct pwdatastream_t
{
    std::ofstream ofstream;
    std::ifstream ifstream;
    int length;
    bool hasdata;
};
bool pwdata_write_open(pwdatastream_t& pwd, const std::string& filename);
bool pwdata_read_open(pwdatastream_t& pwd, const std::string& filename, uint32_t& length);
int pwdata_get_data_length(pwdatastream_t& pwd);
void pwdata_read_data(pwdatastream_t& pwd, uint8_t* data, uint32_t length);
void pwdata_write_data(pwdatastream_t& pwd, uint8_t* data, uint32_t length);
void pwdata_write_close(pwdatastream_t& pwd);
void pwdata_read_close(pwdatastream_t& pwd);
void pwdata_read_rewind(pwdatastream_t& pwd);
void pwdata_read_seek(pwdatastream_t& pwd, size_t traceid);
