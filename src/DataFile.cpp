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
#include "DataFile.h"
#include "Log.h"
#include "Util.h"

/***
 * Strict text format
 * Each line should be:
 *    <data-in-hex>\n 
 * only
 ***/ 

bool pwdata_write_open(pwdatastream_t& pwd, const std::string& filename)
{
    pwd.ofstream.open(filename);
    return pwd.ofstream.good();
}
bool pwdata_read_open(pwdatastream_t& pwd, const std::string& filename, uint32_t& length)
{
    pwd.ifstream.open(filename);
    if (pwd.ifstream.is_open() == false)
    {
        pwd.hasdata = false;
        return false;
    }
    std::string dataline;
    if (std::getline(pwd.ifstream, dataline).good())
    {
        length = dataline.length()/2;
        pwd.length = length;
        pwd.ifstream.seekg(0, pwd.ifstream.beg);
        pwd.hasdata = true;
        return true;
    }
    else
    {
        pwd.hasdata = false;
        return false;
    }
}
void pwdata_read_data(pwdatastream_t& pwd, uint8_t* data, uint32_t length)
{
    if (pwd.hasdata == false)
        return ;
    
    std::string dataline;
    
    if (std::getline(pwd.ifstream, dataline).good() == false)
    {
        LogErrDie("can not read data\n");
    }
    
    PWASSERT(length * 2 == dataline.length(), "data file length mismatch");
    
    hexstr_to_bytebuf(dataline.data(), data);
    
}
void pwdata_write_data(pwdatastream_t& pwd, uint8_t* data, uint32_t length)
{
    
}

void pwdata_write_close(pwdatastream_t& pwd)
{
    pwd.ofstream.close();
}
void pwdata_read_close(pwdatastream_t& pwd)
{
    pwd.ifstream.close();
}
void pwdata_read_rewind(pwdatastream_t& pwd)
{
    pwd.ifstream.seekg(std::ios::beg);
}
void pwdata_read_seek(pwdatastream_t& pwd, size_t traceid)
{
    pwd.ifstream.seekg((pwd.length*2+1) * traceid, std::ios::beg);
}
