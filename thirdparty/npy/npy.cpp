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
#include "npy/npy.hpp"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace npy {
constexpr dtype_t has_typestring<float>::dtype;
constexpr dtype_t has_typestring<double>::dtype;
constexpr dtype_t has_typestring<long double>::dtype;
constexpr dtype_t has_typestring<char>::dtype;
constexpr dtype_t has_typestring<short>::dtype;
constexpr dtype_t has_typestring<int>::dtype;
constexpr dtype_t has_typestring<long>::dtype;
constexpr dtype_t has_typestring<long long>::dtype;
constexpr dtype_t has_typestring<unsigned char>::dtype;
constexpr dtype_t has_typestring<unsigned short>::dtype;
constexpr dtype_t has_typestring<unsigned int>::dtype;
constexpr dtype_t has_typestring<unsigned long>::dtype;
constexpr dtype_t has_typestring<unsigned long long>::dtype;
constexpr dtype_t has_typestring<std::complex<float>>::dtype;
constexpr dtype_t has_typestring<std::complex<double>>::dtype;
constexpr dtype_t has_typestring<std::complex<long double>>::dtype;

void CloseNumpy(npy_reader_t* reader)
{
    reader->stream.close();
    delete reader;
}
void CloseNumpy(npy_writer_t* writer)                                           
{                                                                               
    writer->stream.close();                                                     
    delete writer;                                                              
}

void* OpenNumpyMmap( const std::string& filename, size_t offset, size_t* size)
{
  int fd = open(filename.c_str(), O_RDONLY);
  struct stat sb;
  if (fstat(fd,&sb) == -1)
  {
    return nullptr;
  }
  void* ptr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE | MAP_NORESERVE, fd, 0);
  *size = sb.st_size;
  return (void*)((uint8_t*)ptr+offset);
}

void CloseNumpyMmap(void *ptr, size_t size)
{
  munmap(ptr, size);
}
void Seek(npy_reader_t* reader, size_t offset)
{
    reader->stream.seekg(offset, std::ios::beg);
}
}
