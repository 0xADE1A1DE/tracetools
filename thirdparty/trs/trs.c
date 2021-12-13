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
#include <bits/stdint-uintn.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "trs.h"

trs_sc_def scdefs[] = {
	{0x01, 1},
	{0x02, 2},
	{0x04, 4},
	{0x14, 4}
};

trs_tag_def tagdefs[] = { 
	{0x41, "NT", 1,    trs_int,          4,     "Number of traces"},
	{0x42, "NS", 1,    trs_int,          4,     "Number of samples per trace"},
	{0x43, "SC", 1,    trs_samplecoding, 1,     "Sample Coding {see SampleCoding class}"},
	{0x44, "DS", 0,    trs_short,        2,     "Length of cryptographic data included in trace"},
	{0x45, "TS", 0,    trs_int,          1,     "Title space reserved per trace"},
	{0x46, "GT", 0,    trs_str,          0,     "Global trace title"},
	{0x47, "DC", 0,    trs_str,          0,     "Description"},
	{0x48, "XO", 0,    trs_int,          4,     "Offset in X-axis for trace representation"},
	{0x49, "XL", 0,    trs_str,          0,     "Label of X-axis"},
	{0x4A, "YL", 0,    trs_str,          0,     "Label of Y-axis"},
	{0x4B, "XS", 0,    trs_float,        4,     "Scale value for X-axis"},
	{0x4C, "YS", 0,    trs_float,        4,     "Scale value for Y-axis"},
	{0x4D, "TO", 0,    trs_int,          4,     "Trace offset for displaying trace numbers"},
	{0x4E, "LS", 0,    trs_int,          1,     "Logarithmic scale"},
	{0x55, "RG", 0,    trs_float,        4,     "Range of the scope used to perform acquisition"},
	{0x56, "CL", 0,    trs_int,          4,     "Coupling of the scope used to perform acquisition"},
	{0x57, "OS", 0,    trs_float,        4,     "Offset of the scope used to perform acquisition"},
	{0x58, "II", 0,    trs_float,        4,     "Input impedance of the scope used to perform acquisition"},
	{0x59, "AI", 0,    trs_byte,         0,     "Device ID of the scope used to perform acquisition"},
	{0x5A, "FT", 0,    trs_int,          4,     "The type of filter used during acquisition"},
	{0x5B, "FF", 0,    trs_float,        4,     "Frequency of the filter used during acquisition"},
	{0x5C, "FR", 0,    trs_float,        4,     "Range of the filter used during acquisition"},
	{0x5F, "TB", 1,    trs_none,         0,     "Trace block marker: an empty TLV that marks the end of the header"},
	{0x60, "EU", 0,    trs_bool,         1,     "External clock used"},
	{0x61, "ET", 0,    trs_float,        4,     "External clock threshold"},
	{0x62, "EM", 0,    trs_int,          4,     "External clock multiplier"},
	{0x63, "EP", 0,    trs_int,          4,     "External clock phase shift"},
	{0x64, "ER", 0,    trs_int,          4,     "External clock resampler mask"},
	{0x65, "RE", 0,    trs_bool,         1,     "External clock resampler enabled"},
	{0x66, "EF", 0,    trs_float,        4,     "External clock frequency"},
	{0x67, "EB", 0,    trs_int,          4,     "External clock time base"},
	{0x68, "VT", 0,    trs_int,          4,     "View number of traces: number of traces to show on opening"},
	{0x69, "OV", 0,    trs_bool,         1,     "Overlap: whether to overlap traces in case of multi trace view"},
	{0x6A, "GL", 0,    trs_bool,         1,     "Go to last trace on opening"},
	{0x6B, "IO", 0,    trs_int,          4,     "Input data offset in trace data"},
	{0x6C, "OO", 0,    trs_int,          4,     "Output data offset in trace data"},
	{0x6D, "KO", 0,    trs_int,          4,     "Key data offset in trace data"},
	{0x6E, "IL", 0,    trs_int,          4,     "Input data length in trace data"},
	{0x6F, "OL", 0,    trs_int,          4,     "Output data length in trace data"},
	{0x70, "KL", 0,    trs_int,          4,     "Key data length in trace data"},
	{0x71, "CH", 0,    trs_int,          4,     "Number of oscilloscope channels used for measurement"},
	{0x72, "NO", 0,    trs_int,          4,     "Number of oscilloscopes used for measurement"},
	{0x73, "WI", 0,    trs_int,          4,     "Number of steps in the \"x\" direction during XY scan"},
	{0x74, "HE", 0,    trs_int,          4,     "Number of steps in the \"y\" direction during XY scan"},
	{0x75, "ME", 0,    trs_int,          4,     "Number of consecutive measurements done per spot during"}
};

uint8_t read_byte(FILE *fp) 
{
    uint8_t byte;
    size_t ret = fread(&byte, 1, 1, fp);
    assert(ret == 1);
    return byte;
}

void write_byte(uint8_t byte, FILE* fp)
{
    size_t ret = fwrite(&byte, 1, 1, fp);
    assert(ret == 1);
}
  
void read_bytes(uint8_t *buffer, int len, FILE *fp) 
{
    size_t ret = fread(buffer, 1, len, fp);
    assert(ret == len);
}

void write_bytes(uint8_t *buffer, int len, FILE *fp)
{
    size_t ret = fwrite(buffer, 1, len, fp);
    assert(ret == len);
}
int bytes_to_int(uint8_t *buffer) {
    return buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
}
void int_to_bytes(uint8_t *buffer, int val)
{
    buffer[0] = val & 0xFF;
    buffer[1] = (val & 0xFF00) >> 8;
    buffer[2] = (val & 0xFF0000) >> 16;
    buffer[3] = (val & 0xFF000000) >> 24;
}
int bytes_to_short(uint8_t *buffer) {
    return buffer[0] | (buffer[1] << 8);
}
void short_to_bytes(uint8_t *buffer, int val)
{
    buffer[0] = val & 0xFF;
    buffer[1] = (val & 0xFF00) >> 8;
}
int iscode(trs_tag tag, const char* checkcode)
{
    return (tagdefs[tag].tagcode[0] == checkcode[0]) &&
        (tagdefs[tag].tagcode[1] == checkcode[1]);
}
void assign_fields(trs_set* trs, trs_tag tag, int val)
{
    if (iscode(tag, "NT"))
    {
        trs->NT = val;
        trs->write_ok++;
    }
    else if (iscode(tag, "NS"))
    {
        trs->NS = val;
        trs->write_ok++;
    }
    else if (iscode(tag, "SC"))
    {
        trs->SC = (trs_sc)val;
        trs->write_ok++;
    }
    else if (iscode(tag, "TS"))
    {
        trs->TS = val;
    }
    else if (iscode(tag, "DS"))
    {
        trs->DS = val;
    }
}
trs_sc make_sc(uint8_t tagval, int* failed)
{
    if (tagval == 0x01)
        return trs_sc_byte;
    if (tagval == 0x02)
        return trs_sc_short;
    if (tagval == 0x04)
        return trs_sc_int;
    if (tagval == 0x14)
        return trs_sc_float;
    *failed = 1;
    return trs_sc_byte;
}

trs_header_data* makeheaderdata(trs_set* trs)
{
    trs_header_data* newdata = (trs_header_data*) malloc(sizeof(trs_header_data));
    newdata->next = NULL;

    if (trs->headerdata_current == NULL)
    {
        trs->headerdata_start = newdata;
        trs->headerdata_current = newdata;
    }
    else
    {
        trs->headerdata_current->next = newdata;
        trs->headerdata_current = newdata;
    }
    return newdata;
}

void trs_open_file(trs_set *trs, const char *filename, const char* mode) {
    trs->fp = fopen(filename, mode);
    trs->headerdata_start = NULL;
    trs->headerdata_current = NULL;	
    trs->write_ok = 0;
    trs->NT = 0;
    trs->NS = 0;
    trs->DS = 0;
    trs->TS = 0;
    trs->SC = (trs_sc)0;
    trs->last_write = 0;
}

void trs_clear_header(trs_set* trs)
{
    trs_header_data* curr = trs->headerdata_start;
    while (curr)
    {
        trs_header_data* tmp = curr;
        curr = curr->next;
        free(tmp);
    }
    trs->headerdata_start = NULL;
    trs->headerdata_current = NULL;	
    trs->write_ok = 0;
    trs->NT = 0;
    trs->NS = 0;
    trs->DS = 0;
    trs->TS = 0;
    trs->SC = (trs_sc)0;

    trs->last_write = ftell(trs->fp);
    fseek(trs->fp, 0, SEEK_SET);
}

void trs_add_header_int(trs_set* trs, trs_tag tag, int val)
{
    trs_header_data* newdata = makeheaderdata(trs);
    assign_fields(trs, tag, val);
    newdata->intdata = val;
    newdata->tag = tag;
}
void trs_add_header_str(trs_set* trs, trs_tag tag, char* val)
{
    trs_header_data* newdata = makeheaderdata(trs);
    newdata->chrdata = val;
    newdata->tag = tag;
}
void trs_add_header_float(trs_set* trs, trs_tag tag, float val)
{
    trs_header_data* newdata = makeheaderdata(trs);
    newdata->fltdata = val;
    newdata->tag = tag;
}

void write_tag(trs_set* trs, trs_header_data* headerdata)
{
    uint8_t temp[32];
    trs_tag tag = headerdata->tag;
    
    write_byte(tagdefs[tag].tagid, trs->fp);
    if (tagdefs[tag].type == trs_str)
    {
        int len = strlen(headerdata->chrdata);
        write_byte(len, trs->fp);
        write_bytes((uint8_t*)headerdata->chrdata, len, trs->fp);
    }
    else if (tagdefs[tag].type == trs_none)
    {
        // no length is written
    }
    else
    {
        write_byte(tagdefs[tag].length, trs->fp);
        if (tagdefs[tag].type == trs_byte)
        {
            write_byte(headerdata->intdata, trs->fp);
        }
        else if (tagdefs[tag].type == trs_int)
        {
            int_to_bytes(temp, headerdata->intdata);
            write_bytes(temp, tagdefs[tag].length, trs->fp);
        }
        else if (tagdefs[tag].type == trs_short)
        {
            short_to_bytes(temp, headerdata->intdata);
            write_bytes(temp, tagdefs[tag].length, trs->fp);
        }
        else if (tagdefs[tag].type == trs_samplecoding)
        {
            write_byte(scdefs[headerdata->intdata].scid, trs->fp);
        }
        else if (tagdefs[tag].type == trs_float)
        {
            write_bytes((uint8_t*)&headerdata->fltdata, tagdefs[tag].length, trs->fp);
        }
    }
}
void trs_write_headers(trs_set* trs, int *failed)
{
    fseek(trs->fp, 0, SEEK_SET);
    if (trs->write_ok != 3)
    {
        *failed = 1;
        return;
    }

    trs_header_data* curr = trs->headerdata_start;
    while (curr)
    {
        write_tag(trs, curr);
        curr = curr->next;
    }
    
    write_byte(tagdefs[TRACE_BLOCK].tagid, trs->fp);
    write_byte(0, trs->fp);
}
unsigned read_tag_len(FILE* fp)
{
    uint8_t buffer[128];
    unsigned taglen = (unsigned) read_byte(fp);
    if ((taglen & 0x80) != 0)
    {
        read_bytes(buffer, (taglen & 0x7F), fp);
        taglen = bytes_to_int(buffer);
    }
    return taglen;
}
void trs_parse_header(trs_set *trs, int* failed) {
    uint8_t buffer[1024];
    uint8_t tag;
    unsigned len=0;
    tag = read_byte(trs->fp);
    len = read_tag_len(trs->fp);
    while (tag != tagdefs[TRACE_BLOCK].tagid)
    {
        if (len > 1024)
        {
            fseek(trs->fp, len, SEEK_CUR);
            printf("warn: skipping buffer read\n");
        }
        else
        {
            read_bytes(buffer, len, trs->fp);
        }
        if (tag == tagdefs[LENGTH_DATA].tagid)
        {
            assert(len == tagdefs[LENGTH_DATA].length);
            trs->DS = bytes_to_short(buffer);
        }
        else if (tag == tagdefs[NUMBER_TRACES].tagid)
        {
            assert(len == tagdefs[NUMBER_TRACES].length);
            trs->NT = bytes_to_int(buffer);
        }
        else if (tag == tagdefs[NUMBER_SAMPLES].tagid)
        {
            assert(len == tagdefs[NUMBER_SAMPLES].length);
            trs->NS = bytes_to_int(buffer);
        }
        else if (tag == tagdefs[SAMPLE_CODING].tagid)
        {
            assert(len == tagdefs[SAMPLE_CODING].length);
            trs->SC = make_sc(buffer[0], failed);
        }
        else if (tag == tagdefs[TITLE_SPACE].tagid)
        {
            assert(len == tagdefs[TITLE_SPACE].length);
            trs->TS = buffer[0];
        }
        if (*failed == 1)
            break;
        tag = read_byte(trs->fp);
        len = read_tag_len(trs->fp);
        printf("len=%d\n", len);
    }
    trs->data_start = ftell(trs->fp);
    printf("NT: %i\n", trs->NT);
    printf("NS: %i\n", trs->NS);
    printf("SC: %i\n", trs->SC);
    printf("DS: %i\n", trs->DS);
    printf("TS: %i\n", trs->TS);
}

void trs_read_trace(trs_set *trs, trs_trace *tr) {
    // Read titlespace if present
    fseek(trs->fp, trs->TS, SEEK_CUR);

    // Read data if present
    read_bytes(tr->data, trs->DS, trs->fp);
    
    // Read samples
    read_bytes((uint8_t*)tr->samples, trs->NS * scdefs[trs->SC].length, trs->fp);
}
void trs_write_trace(trs_set *trs, trs_trace *tr)
{
    if (trs->last_write != 0)
    {
        // Restore write pointer to last write location after header update
        fseek(trs->fp, trs->last_write, SEEK_SET);
        trs->last_write = 0;
    }
    if (trs->write_ok != 3)
    {
        printf("err: mandatory headers not written\n");
        return ;
    }
    if (tr->data)
    {
        write_bytes(tr->data, trs->DS, trs->fp);
    }
    else
    {
        int i=0;
        for (;i<trs->DS;i++)
        {
            write_byte(0, trs->fp);
        }
    }
    
    // write samples
    write_bytes((uint8_t*)tr->samples, trs->NS * scdefs[trs->SC].length, trs->fp);
}
trs_trace trs_make_trace(trs_set* trs) {
    trs_trace tr;
    tr.samples = malloc(trs->NS * scdefs[trs->SC].length);
    tr.data = (uint8_t*)malloc(trs->DS);
    return tr;
}

void trs_del_trace(trs_trace tr) {
    free(tr.samples);
    free(tr.data);
}

void trs_seekto_trace(trs_set *trs, unsigned traceid)
{
    unsigned long seekto =  (unsigned long) trs->data_start +
                traceid * ( (unsigned long) trs->DS + (unsigned long) trs->TS + (unsigned long) trs->NS * scdefs[trs->SC].length);
    fseek(trs->fp, seekto, SEEK_SET);
}
void trs_rewind(trs_set* trs) 
{
    fseek(trs->fp, trs->data_start, SEEK_SET);
}
void trs_close(trs_set* trs)
{
    fclose(trs->fp); 
}
