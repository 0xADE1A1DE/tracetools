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
#include "PWFactory.h"
#include "PWArgs.h"
#include "FileProcessor.h"
#include "CmdPipeline.h"
#include "Log.h"
#include "AnalysisOuput.h"
#include "Stackable.h"
#include "WorkContext.h"
#include "Trace.h"
#include "Passes.h"

#include <cstdlib>
#include <iostream>
#include <functional>
#include <chrono>
enum Arguments
{
    ARG_AT_EACH_NTRACES=0,
    ARG_FSPACE,
    ARG_RANGE,
    ARG_SAMPLE_RANGE,
    ARG_FILT_GROUP,
    ARG_COMBS,
    ARG_RCOMBS,
    ARG_MVG_AVG_WIND,
    ARG_DATA_RANGE,
    ARG_CONV,
    ARG_LIBNAME,
    ARG_GUESS_COUNT,
    ARG_GROUPS,
    ARG_FILT_TYPE,
    ARG_FILT_SMPLR,
    ARG_FILT_CUTOFF,
    ARG_FILT_CEN,
    ARG_FILT_WIDTH,
    ARG_SA_REF,
    ARG_SA_MINCORR,
    ARG_SA_RANGE,
    ARG_SA_SHIFT,
    ARG_PM_PEAKS,
    ARG_PM_PAD,
    ARG_CSQ_MIN,
    ARG_CSQ_DIV
};
pwargs_t args[] = 
{
    { "--at-each-ntraces", "-e", -1, 1 },
    { "--force-space", "-F", -1, 0 },
    { "--trace-range", "-r", -1, 2 },
    { "--sample-range", "-s", -1, 2 },
    { "--filter-group", "-f", -1, 1 },
    { "--sample-combinations", "-x", -1, 1 },
    { "--sample-window", "-y", -1, 0 },
    { "--moving-avg-wind", "-w", -1, 1 },
    { "--data-range", "-d", -1, 2 },
    { "--convert-to", "-c", -1, 1 },
    { "--libname", "-l", -1, 1 },
    { "--guess-count", "-u", -1, 1 },
    { "--groups", "-g", -1, 1 },
    { "--filter-type", "", -1, 1 },
    { "--filter-samplerate", "", -1, 1 },
    { "--filter-cutoff", "", -1, 1 },
    { "--filter-center", "", -1, 1 },
    { "--filter-width", "", -1, 1 },
    { "--align-ref-range", "", -1, 2 },
    { "--align-min-corr", "", -1, 1 },
    { "--align-search-in", "", -1, 2 },
    { "--align-max-shift", "", -1, 1 },
    { "--pattern-matches", "", -1, 1 },
    { "--pattern-padding", "", -1, 2 },
    { "--chisq-min", "", -1, 1 },
    { "--chisq-div", "", -1, 1 }
};
const char* longdescs[] = 
{
    "\%Common",
    "Output to file at each \'n\' number of traces",
    "Force operations when there's not enough space",
    "Run only for given range of traces (start, length)",
    "Run only on samples window (start, length)",
    "Filter by group (byte 1 of data), [0x00, 0xff]",
    "Combine samples of each trace a given number of times, [1, 3]",
    "Combine sample windows of each trace (use with -x)",
    "Set value of moving average window size",
    "Range of data indexes that are used for correlation,\n"
        "\tstdin needs to be fed with (end - start + 1) correlation\n"
        "\tcoefficient values (i.e. Hamming weight) for each trace",
    "Convertion target",
    "Module library name",
    "Guess count, [1, 256]",
    "Number of trace groups (default is 2), when larger than 0\n"
        "\tthe group count is the first byte of data, [1, 256]",
    "\%'filter' Command",
    "Filter type [lowpass, highpass, bandstop]",
    "Filter samplerate in Hz",
    "Filter cutoff frequency in Hz (highpass,lowpass)",
    "Filter center frequency (bandpass only)",
    "Filter width (bandpass only)",
    "\%'staticalign' & 'patternmatch' Commands",
    "Range of reference window for static alignment",
    "Minimum level of correlation for NCC (Normalised Cross Correlation) (default: 0.8)",
    "\%'staticalign' Command",
    "Search range, [start, end)",
    "Maximum allowed shift (default: inf)",
    "\%'patternmatch' Command",
    "Match N number of NCC (Normalised Cross Correlation) peaks (default: 1)",
    "Start and end padding for each matched pattern (default: 0, 0)",
    "\%'chisq' Command",
    "Mininum value for all samples",
    "Divisor that normalizes all samples"
};
#define ARGS_IDX(__X) \
    args[__X].arg_start_idx
int at_each_ntraces = 500;
int main(int argc, char *argv[])
{
    if ( ((argc == 2) && (strcmp(argv[1], "-h") == 0)) || (argc == 1) )
    {
        printf("Usage: traceprocessor [options] <command> <trace-file>\n");
        pwargs_print_options(&args[0], longdescs, sizeof(args)/sizeof(args[0]));
        return 0;
    }
    int noswargsstart=0;
    pwargs_process(argc, argv, &args[0], sizeof(args)/sizeof(args[0]), &noswargsstart);
    if (noswargsstart == -1)
    {
        printf("Invalid arguments\n");
        return -1;
    }
    if (args[ARG_AT_EACH_NTRACES].arg_start_idx != -1)
    {
        at_each_ntraces = atoi(argv[args[ARG_AT_EACH_NTRACES].arg_start_idx]);
    }
    
    char **xargs = &argv[noswargsstart];
    
    wc_gopts_t opts_map = { 
        {"at_each_ntraces", at_each_ntraces}
    };
    if (ARGS_IDX(ARG_FSPACE) != -1)
    {
        opts_map.insert({"force_space", 1});
    }
    if (ARGS_IDX(ARG_GROUPS) != -1)
    {
        opts_map.insert({"data_groups_count", atoi(argv[ARGS_IDX(ARG_GROUPS)])});
    }
    if (ARGS_IDX(ARG_MVG_AVG_WIND) != -1)
    {
        opts_map.insert({"moving_avg_wind", atoi(argv[args[ARG_MVG_AVG_WIND].arg_start_idx])});
    }
    if (args[ARG_DATA_RANGE].arg_start_idx != -1)
    {
        opts_map.insert({"data_range_begin", atoi(argv[args[ARG_DATA_RANGE].arg_start_idx])});
        opts_map.insert({"data_range_end", atoi(argv[args[ARG_DATA_RANGE].arg_start_idx+1])});
    }
    if (args[ARG_CONV].arg_start_idx != -1)
    {
        opts_map.insert({"convert_to", argv[args[ARG_CONV].arg_start_idx]});
    }
    if (ARGS_IDX(ARG_FILT_GROUP) != -1)
    {
        char *endptr=nullptr;
        opts_map.insert({"filter_by_group", (int) strtol(argv[ARGS_IDX(ARG_FILT_GROUP)], &endptr, 16)});
    }
    if (ARGS_IDX(ARG_COMBS) != -1)
    {
        opts_map.insert({"combinations", atoi(argv[ARGS_IDX(ARG_COMBS)])});
    }
    if (ARGS_IDX(ARG_RCOMBS) != -1)
    {
        opts_map.insert({"combinations_window", argv[ARGS_IDX(ARG_RCOMBS)]});
    }
    if (ARGS_IDX(ARG_LIBNAME) != -1)
    {
        opts_map.insert({"library_name", argv[ARGS_IDX(ARG_LIBNAME)]});
    }
    if (ARGS_IDX(ARG_GUESS_COUNT) != -1)
    {
        opts_map.insert({"corr_guess_count", atoi(argv[ARGS_IDX(ARG_GUESS_COUNT)])});
    }
    if (ARGS_IDX(ARG_FILT_TYPE) != -1)
    {
        opts_map.insert({"filter_select", argv[ARGS_IDX(ARG_FILT_TYPE)]});
    }
    if (ARGS_IDX(ARG_FILT_CUTOFF) != -1)
    {
        opts_map.insert({"filter_cutoff", atoi(argv[ARGS_IDX(ARG_FILT_CUTOFF)])});
    }
    if (ARGS_IDX(ARG_FILT_SMPLR) != -1)
    {
        opts_map.insert({"filter_samplerate", atoi(argv[ARGS_IDX(ARG_FILT_SMPLR)])});
    }
    if (ARGS_IDX(ARG_FILT_CEN) != -1)
    {
        opts_map.insert({"filter_centre_freq", atoi(argv[ARGS_IDX(ARG_FILT_CEN)])});
    }
    if (ARGS_IDX(ARG_FILT_WIDTH) != -1)
    {
        opts_map.insert({"filter_width", atoi(argv[ARGS_IDX(ARG_FILT_WIDTH)])});
    }
    if (ARGS_IDX(ARG_SA_REF) != -1)
    {
        int a = atoi(argv[ARGS_IDX(ARG_SA_REF)]);
        int b = atoi(argv[ARGS_IDX(ARG_SA_REF)+1]);
        
        opts_map.insert({"static_align_ref_width", b - a});
        opts_map.insert({"static_align_ref_begin", a});
    }
    if (ARGS_IDX(ARG_SA_MINCORR) != -1)
    {
        double c = atof(argv[ARGS_IDX(ARG_SA_MINCORR)]);
        opts_map.insert({"static_align_mincorr", c});
    }
    if (ARGS_IDX(ARG_SA_RANGE) != -1)
    {
        int a = atoi(argv[ARGS_IDX(ARG_SA_RANGE)]);
        int b = atoi(argv[ARGS_IDX(ARG_SA_RANGE)+1]);
        
        opts_map.insert({"static_align_begin", a});
        opts_map.insert({"static_align_end", b});
    }
    if (ARGS_IDX(ARG_SA_SHIFT) != -1)
    {
        int m = atoi(argv[ARGS_IDX(ARG_SA_SHIFT)]);
        opts_map.insert({"static_align_maxshift", m});
    }
    if (ARGS_IDX(ARG_PM_PEAKS) != -1)
    {
        int m = atoi(argv[ARGS_IDX(ARG_PM_PEAKS)]);
        opts_map.insert({"static_align_npeaks", m});
    }
    if (ARGS_IDX(ARG_PM_PAD) != -1)
    {
        int a = atoi(argv[ARGS_IDX(ARG_PM_PAD)]);
        int b = atoi(argv[ARGS_IDX(ARG_PM_PAD)+1]);
        
        opts_map.insert({"pattern_pad_start", a});
        opts_map.insert({"pattern_pad_end", b});
    }
    if (ARGS_IDX(ARG_CSQ_MIN) != -1)
    {
        opts_map.insert({"chisq_trace_min", (double) atof(argv[ARGS_IDX(ARG_CSQ_MIN)])});
    }
    if (ARGS_IDX(ARG_CSQ_DIV) != -1)
    {
        opts_map.insert({"chisq_trace_div", (double) atof(argv[ARGS_IDX(ARG_CSQ_DIV)])});
    }
    wc_gopts_merge(opts_map);
    pwpass_update_properties();
    pwlog_set_verbosity(0);
    file_processor_t fileproc;
    auto cp = pwfactory_t::get_instance()->new_cmdpipeline(xargs[0]);
    
    file_processor_t::range_t r; 
    if (ARGS_IDX(ARG_RANGE) != -1 )
    {
        r.tracerangestart = atoi(argv[ARGS_IDX(ARG_RANGE)]);
        r.tracerangelen = atoi(argv[ARGS_IDX(ARG_RANGE)+1]);
    }
    if (ARGS_IDX(ARG_SAMPLE_RANGE) != -1 )
    {
        r.samplewinstart = atoi(argv[ARGS_IDX(ARG_SAMPLE_RANGE)]);
        r.samplewinlen = atoi(argv[ARGS_IDX(ARG_SAMPLE_RANGE)+1]);
    }
    fileproc.init(cp, xargs[1], &r);
    auto t_start = std::chrono::high_resolution_clock::now();
    fileproc.process_range(r);
    auto t_end = std::chrono::high_resolution_clock::now();   
    double elapsed_time_ms = std::chrono::duration<double, std::milli>(t_end-t_start).count();
    LogStat(LOG_STAT_TIME, "%lfs\n", elapsed_time_ms/1000.0);
    fileproc.finit();
    return 0;
}
