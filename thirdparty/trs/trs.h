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

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef enum _trs_datatype_t
{
    trs_int,
    trs_float,
    trs_short,
    trs_byte,
    trs_bool,
    trs_str,
    trs_none,
    trs_samplecoding
} trs_datatype;

typedef struct _trs_tag_def
{
    uint8_t tagid;
    char tagcode[3];
    int is_mandatory;
    trs_datatype type;
    int length;
    const char *desc;
} trs_tag_def;

typedef struct _trs_sc_def
{
	uint8_t scid;
	uint8_t length;
} trs_sc_def;

typedef enum _trs_sc
{
    trs_sc_byte = 0,
    trs_sc_short,
    trs_sc_int,
    trs_sc_float
} trs_sc;

extern trs_sc_def scdefs[];
extern trs_tag_def tagdefs[];

typedef enum _trs_tag {
    NUMBER_TRACES = 0, 
    NUMBER_SAMPLES, 
    SAMPLE_CODING, 
    LENGTH_DATA, 
    TITLE_SPACE, 
    TRACE_TITLE, 
    DESCRIPTION, 
    OFFSET_X, 
    LABEL_X, 
    LABEL_Y, 
    SCALE_X, 
    SCALE_Y, 
    TRACE_OFFSET, 
    LOGARITHMIC_SCALE, 
    ACQUISITION_RANGE_OF_SCOPE, 
    ACQUISITION_COUPLING_OF_SCOPE, 
    ACQUISITION_OFFSET_OF_SCOPE, 
    ACQUISITION_INPUT_IMPEDANCE, 
    ACQUISITION_DEVICE_ID, 
    ACQUISITION_TYPE_FILTER, 
    ACQUISITION_FREQUENCY_FILTER, 
    ACQUISITION_RANGE_FILTER, 
    TRACE_BLOCK, 
    EXTERNAL_CLOCK_USED, 
    EXTERNAL_CLOCK_THRESHOLD, 
    EXTERNAL_CLOCK_MULTIPLIER, 
    EXTERNAL_CLOCK_PHASE_SHIFT, 
    EXTERNAL_CLOCK_RESAMPLER_MASK, 
    EXTERNAL_CLOCK_RESAMPLER_ENABLED, 
    EXTERNAL_CLOCK_FREQUENCY, 
    EXTERNAL_CLOCK_BASE, 
    NUMBER_VIEW, 
    TRACE_OVERLAP, 
    GO_LAST_TRACE, 
    INPUT_OFFSET, 
    OUTPUT_OFFSET, 
    KEY_OFFSET, 
    INPUT_LENGTH, 
    OUTPUT_LENGTH, 
    KEY_LENGTH, 
    NUMBER_OF_ENABLED_CHANNELS, 
    NUMBER_OF_USED_OSCILLOSCOPES, 
    XY_SCAN_WIDTH, 
    XY_SCAN_HEIGHT, 
    XY_MEASUREMENTS_PER_SPOT
} trs_tag;


typedef struct _trace {
  void *samples;
  uint8_t *data;
} trs_trace;

typedef struct _trs_header_data {
  trs_tag tag;
  char *chrdata;
  int intdata;
  float fltdata;
  struct _trs_header_data *next;
} trs_header_data;

typedef struct _trs_set {
  FILE *fp;
  int NT;
  int NS;
  trs_sc SC;
  int DS;
  uint8_t TS;
  trs_trace *traces;
  // only allow write when value is 3 (all mandatory fields are filled in)
  int write_ok;
  trs_header_data *headerdata_start;
  trs_header_data *headerdata_current;
  long data_start;
  long last_write;
} trs_set;

void trs_open_file(trs_set *trs, const char *filename, const char* mode);
void trs_add_header_int(trs_set* trs, trs_tag tag, int val);
void trs_add_header_str(trs_set* trs, trs_tag tag, char* val);
void trs_add_header_float(trs_set* trs, trs_tag tag, float val);
void trs_clear_header(trs_set* trs);
void trs_write_headers(trs_set* trs, int *failed);
void trs_parse_header(trs_set *trs, int *failed);
void trs_write_trace(trs_set *trs, trs_trace *trace);
void trs_read_trace(trs_set *trs, trs_trace* trace);
void trs_seekto_trace(trs_set *trs, unsigned traceid);
trs_trace trs_make_trace(trs_set* trs);
void trs_del_trace(trs_trace trace);
void trs_close(trs_set* trs);
void trs_rewind(trs_set* trs);
#ifdef __cplusplus
}
#endif
