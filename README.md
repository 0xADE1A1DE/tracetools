# README

TraceTools is a set of tools for power trace analysis.


## Features

+ PlotTraces (`utils/plot_traces`)  is a graphical viewer that supports viewing power traces stored as NumPy, Inspector TRS or plain text.
+ Static alignment of signals.
+ Use of pattern matching to find similar regions within a signal.
+ Zero phase filter application on signals
+ Welch's T-tests in unvariate, bivariate and trivariate configurations (with or without normalisation)
+ Pearson's Chi Square test.
+ Conversion between Numpy and Inspector TRS power trace formats.
+ Correlation Power Analysis (CPA) attacks.
+ Compress power traces by calculating overlapping moving average.

## Install

##### TraceTools
Before building TraceTools, install the dependencies by running the following command (only in Debian like systems, 
you may have to find correct pacakage names for other distributions). 
TraceTools have only been tested on Ubuntu but will work on 
other systems with minimal changes.

	sudo apt install libboost-filesystem-dev libboost-math-dev libfftw3-dev cmake

Build the thirdparty `iir1` library by running the following commands,

	cd thirdparty/iir1
	mkdir base 
	mkdir build
	cmake .. -DCMAKE_INSTALL_PREFIX=<path_to_trace_tools>/thirdparty/iir1/base

Build TraceTools by running `make` in `./src` directory. You may optionally
use `PARALLEL=1` to enable OpenMP based parallelism. 

	make -e 'PARALLEL=1'

Then use `make install` to install TraceTools to a directory, 
you may optionally use `PREFIX` to set installation directory.

	make -e 'PREFIX=/home/user/tracetools' install

##### PlotTraces

The following Python 3 dependencies are required to run PlotTraces. Once these are met, it can be run by executing `./utils/plot_traces`.

+ `numpy`
+ `pyqtgraph`
+ `pandas`
+ `trsfile`

## Usage

TraceTools operates on files with power traces. These files and be either NumPy or Inspector TRS files. 
Output files will be written as NumPy trace files.

	traceprocessor [options] <command> <trace-file>


### File formats

Power traces in the NumPy format may have an additional text file accompanying data for each power traces. 
The filename must be the original NumPy file's name suffixed with `.data`. Each line holds the byte data in
a hex string and each line represents the data for the corresponding power trace in the NumPy file.

Inspector TRS file format is described [here](https://github.com/Riscure/python-trsfile).

### Commands

List of all commands

+ `univfott` - univariate first order t-test
+ `bivfott` - bivariate first order t-test
+ `bivfott,mt` - bivariate first order t-test multi-threaded 
+ `trivfott` - trivariate first order t-test
+ `filter` - apply zero phase filtering on power traces (highpass or lowpass)
+ `convert` - convert between NumPy and TRS files
+ `compress` - compress power traces by calculating overlapping moving average
+ `corr` - CPA attack on power traces
+ `normcombfott` - first order t-test on normalised and combined power traces
+ `normcombcorr` - CPA attack on normalised and combined power traces
+ `chisq` - chi square test on power traces
+ `staticalign` - statically align power traces
+ `patternmatch` - find similar looking regions in power traces and output all matching regions only

#### Switches for commands

##### Common to all commands

`-e, --at-each-ntraces` Output to file at each 'n' number of traces  
`-F, --force-space` Force operations when there's not enough space  
`-r, --trace-range` Run only for given range of traces (start, length)  
`-s, --sample-range` Run only on samples window (start, length)  
`-f, --filter-group` Filter by group (byte 1 of data), [0x00, 0xff]  
`-x, --sample-combinations` Combine samples of each trace a given number of times, [1, 3]  
`-y, --sample-window` Combine sample windows of each trace (use with -x)  
`-w, --moving-avg-wind` Set value of moving average window size  
`-d, --data-range` Range of data indexes that are used for correlation, stdin needs to be fed with (end - start + 1) model values (i.e. Hamming weight) for each trace  
`-c, --convert-to` Conversion target  
`-l, --libname` Module library name  
`-u, --guess-count` Guess count, [1, 256]  
`-g, --groups` Number of trace groups (default is 2), when larger than 0 the group count is the first byte of data, [1, 256]  

##### `filter` Command

`--filter-type` Filter type [lowpass, highpass, bandstop]  
`--filter-samplerate` Filter samplerate in Hz  
`--filter-cutoff` Filter cutoff frequency in Hz (only in highpass, lowpass)  
`--filter-center` Filter center frequency (only in bandstop)  
`--filter-width` Filter width (only in bandstop)  

##### `staticalign` & `patternmatch` Commands

`--align-ref-range` Range of reference window for static alignment  
`--align-min-corr` Minimum level of correlation for NCC (Normalised Cross Correlation) (default: 0.8)  

##### `staticalign` Command

`--align-search-in` Search range, [start, end)  
`--align-max-shift` Maximum allowed shift (default: inf)  

##### `patternmatch` Command
`--pattern-matches` Match N number of NCC (Normalised Cross Correlation) peaks (default: 1)  
`--pattern-padding` Start and end padding for each matched pattern (default: 0, 0)  

##### `chisq` Command
`--chisq-min` Mininum value for all samples  
`--chisq-div` Divisor that normalizes all samples  

#### Examples

Run a univariate t-test, a resulting t-test trace is output for each 20000 input traces. 

	traceprocessor -e 20000 univfott traces.npy

Run CPA attack on samples in `traces.npy`. The data used for each trace is in `traces.npy.data`. 
Since the data is not grouped, `-g 1` so that the mean calculation considers the whole trace file as one group. 

	python3 hw.py traces.npy.data | traceprocessor -x 1 -g 1 -u 256 -d 0 0 normcombcorr traces.npy

Run Chi Sqaure test for traces 0 thorugh 500000 in `traces.npy`. The minimum value in `traces.npy` is -80000 and 
the normilizing factor is 160000. In other words, reducing -80000 and then dividing each sample by 160000 will 
produce a value in the range [0, 1].

	traceprocessor -r 0 500000 --chisq-min -80000 --chisq-div 160000 -e 20000 chisq traces.npy




