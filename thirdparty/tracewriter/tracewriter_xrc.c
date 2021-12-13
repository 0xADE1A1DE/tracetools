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
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tracewriter.h"
#define XRC_HEADER_LIMIT 4096
void tw_open(struct TraceWriter* tw, const char* file, int mode)
{
    tw->impl = NULL;
    if (mode == TW_M_CONT)
    {
        tw->file = fopen(file, "a");
    }
    else if (mode == TW_M_NEW)
    {
        tw->file = fopen(file, "w");
    }
    else if (mode == TW_M_READ)
    {
        tw->fd = open(file, O_RDONLY);
        /* Get the size of the file. */
        struct stat s;
        int status = fstat (tw->fd, &s);
       
        tw->impl = mmap(0, s.st_size, PROT_READ, MAP_PRIVATE, tw->fd, 0);
        tw->size = s.st_size;
    }
}

void tw_set_property(struct TraceWriter* tw, int prop, uint32_t value)
{
    if (!tw->impl)
    {
        fseek(tw->file, prop * sizeof(uint32_t), SEEK_SET);
        fwrite(&value, sizeof(uint32_t), 1, tw->file);
    }
    else
    {
        ((uint32_t*)tw->impl)[ prop ] = value;
    }
}
void tw_get_property(struct TraceWriter* tw, int prop, uint32_t* value)
{
    if (!tw->impl)
    {
        fseek(tw->file, prop * sizeof(uint32_t), SEEK_SET);
        fread(value, sizeof(uint32_t), 1, tw->file);
    }
    else
    {
        *value = ((uint32_t*)tw->impl)[ prop ];
    }
}
ssize_t tw_read_samples(struct TraceWriter* tw, tw_t** samples)
{
    if (tw->impl)
    {
        *samples = (tw_t*) &((char*)tw->impl)[XRC_HEADER_LIMIT];
        return tw->size - XRC_HEADER_LIMIT;
    }
    else 
    {
        return (ssize_t)-1;
    }
}
ssize_t tw_write_samples(struct TraceWriter* tw, tw_t* samples, int count)
{
    if (ftell(tw->file) < XRC_HEADER_LIMIT)
    {
        fseek(tw->file, 0, SEEK_END);
        if (ftell(tw->file) < XRC_HEADER_LIMIT)
        {
            fseek(tw->file, XRC_HEADER_LIMIT, SEEK_SET); 
        }
    }

    return fwrite(samples, sizeof(tw_t), count, tw->file);
}
void tw_close(struct TraceWriter* tw)
{
    if (tw->impl)
    {
        struct stat s;
        int status = fstat (tw->fd, &s);

        munmap(tw->impl, s.st_size);
    }
    else
    {
        fclose(tw->file);
    }
}
