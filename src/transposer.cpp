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
#include "../thirdparty/npy/npy.hpp"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sstream>
#include <omp.h>
void transpose(float* dest, float* src, long cols, long rows)
{
#ifdef PARALLEL
    omp_set_dynamic(0);
    omp_set_num_threads(8);
#endif
    long block_size = 128;
    for (long a=0;a<rows;a+=block_size)
    {
        for (long b=0;b<cols;b+=block_size)
        {
#ifdef PARALLEL
            #pragma omp parallel for
#endif
            for (long i=a;i<std::min(a+block_size,rows);i++)
            {
                for (long j=b;j<std::min(b+block_size,cols);j++)
                {
                    dest[j*rows+i] = src[i*cols+j];
                }
            }
        }
    }
    
}
int main(int argc, const char **argv)
{
    std::string infile = std::string(argv[1]);
    std::string outfile = std::string(argv[2]);
    auto reader = npy::LoadHeaderFromNumpy(infile);
    size_t offset = reader->offset;
    size_t size = 0;
    std::stringstream ss(reader->header.other);
    std::string magic;
    long nd,ns,nt; 
    ss>>magic>>nd>>ns>>nt;
    unsigned long shape[] = { (unsigned long) ns*nd, (unsigned long) nt };
    auto writer = npy::WriteNumpyHeader<float>(outfile, false, 2, shape, reader->header.other);
    size_t header_offset = writer->offset;
    npy::CloseNumpy(writer);

    float* src = (float*) npy::OpenNumpyMmap(infile, offset, &size);
    npy::CloseNumpy(reader);
    int fd = open(outfile.c_str(), O_RDWR, (mode_t) 0777);
    printf("%lu\n", size);
    if (lseek (fd, size, SEEK_SET) == -1)
    {
        printf ("lseek error");
        return -1;
    }

    if (write (fd, "", 1) != 1)
    {    
        printf ("write error");
        return -1;
    }

    sync();
    void* mmaped = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    float* dest = (float*)((uint8_t*)mmaped +header_offset);
    if (dest == (void*)-1)
    {
        perror(strerror(errno));
        return -1;
    }
    //dest[0]=0;
    transpose(dest, src, ns*nd, nt);
    munmap(mmaped, size);
    npy::CloseNumpyMmap(src, size);
    return 0;
}
