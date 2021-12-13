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
#include "Vec2.h"
#include "TraceWriterNpy.h"

#include "../thirdparty/npy/npy.hpp"

#include <cstdio>

struct block_t
{
    float* _data;
    size_t _readhead;
    block_t() :
        _data(nullptr)
        ,_readhead(0)
    {}
};
void biv_merge_output(const std::string& inppathtempl, const std::string& outpath, int splits, uint64_t nsamples, uint64_t ntraces)
{
    Vec2<block_t> blocks;
    blocks.init(splits, splits);
    // all blocks
    for (int i=0;i<splits;i++)
    {
        for (int j=i;j<splits;j++)
        {
            std::vector<char> outfilepath(255);
            snprintf(outfilepath.data(), 254, inppathtempl.c_str(), i, j);
            auto reader = npy::LoadHeaderFromNumpy(outfilepath.data());
            size_t offset = reader->offset;
            npy::CloseNumpy(reader);
            size_t sz = 0;
            blocks.at(i, j)._data = (float*)npy::OpenNumpyMmap(outfilepath.data(), offset, &sz);
        }
    }

    int ns = nsamples / splits;
    tracewriter_npy_t writer;
    traceinfo_t traceinfo;
    traceinfo.ndata = 0;
    traceinfo.nsamples = nsamples * (nsamples - 1) /2;
    traceinfo.nterms = 1;
    traceinfo.ntraces = ntraces;
    
    writer.open_file(outpath.c_str(), &traceinfo, {{"numpy",""}});
    for (auto t=0u;t<ntraces;t++)
    {
        trace_t trace(&traceinfo);
        int cc=0;
        for (auto i=0u;i<nsamples;i++)
        {
            for (auto j=i+1;j<nsamples;j++)
            {
                auto& block = blocks.at(i /ns, j /ns);
                
                trace._samples[cc] = block._data[block._readhead];
                block._readhead++;
                cc++;
            }
        }
        writer.write_trace(&trace);
    }
    writer.finit(nullptr);
    // delete temp files
    for (int i=0;i<splits;i++)
    {
        for (int j=i;j<splits;j++)
        {
            std::vector<char> outfilepath(255);
            snprintf(outfilepath.data(), 254, inppathtempl.c_str(), i, j);
            if (std::remove(outfilepath.data()) != 0)
            {
                LogErr("failed to delete file %s\n", outfilepath.data());
            }
        }
    }
}
