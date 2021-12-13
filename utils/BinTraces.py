# Copyright 2021 University of Adelaide
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import mmap
import struct
import os
import numpy


class BinTraces:
    ROUNDS = 0
    SAMPLES_PER_CAPTURE = 0
    CAPTURES_PER_ROUND = 0
    FMT = 0
    HEADER = 4096
    FMT_STR_LIST = ['f', 'h']
    FMT_SZ_LIST = [4, 2]
    NP_DT_LIST = [numpy.float32, numpy.int16]
    def __init__(self, file, mode="rb", fmt=0, sampl_per_cap=0, cap_per_round=0, rounds=0):
        if mode == "wb":
            fD = open(file, "wb")
            self.FMT = fmt
            self.SAMPLES_PER_CAPTURE = sampl_per_cap
            self.CAPTURES_PER_ROUND = cap_per_round
            self.ROUNDS = rounds
            fD.seek(self.HEADER + self.size() - 1)
            fD.write(b'\0')
            fD.flush()
            fD.close()

            fD = open(file, "r+b")
            self.fD = fD
            self.mmD = mmap.mmap(fD.fileno(), self.HEADER + self.size(), access=mmap.ACCESS_WRITE)
            A = struct.pack('4I', self.FMT, self.SAMPLES_PER_CAPTURE, self.CAPTURES_PER_ROUND, self.ROUNDS)
            self.mmD.write(A)

            self.mmD.seek(self.HEADER, os.SEEK_SET)
        else:
            fD = open(file, "rb")
            self.fD = fD
            self.mmD = mmap.mmap(fD.fileno(), 0, prot=mmap.PROT_READ)
            bytes = self.mmD.read(6 * 4);
            A = struct.unpack('6I', bytes)
            self.FMT = A[0]
            self.SAMPLES_PER_CAPTURE = A[1]
            self.CAPTURES_PER_ROUND = A[2]
            self.ROUNDS = A[3]
            self.FRAC_NUM = A[4]
            self.FRAC_DENO = A[5]
    def size(self):
        return self.ROUNDS * self.CAPTURES_PER_ROUND * self.SAMPLES_PER_CAPTURE * self.FMT_SZ_LIST[self.FMT];

    def get_sample_count(self):
        return self.SAMPLES_PER_CAPTURE

    def get_trace_count(self):
        return self.ROUNDS * self.CAPTURES_PER_ROUND;

    def read_traces(self, trace_start, trace_end):
        sample_count = self.SAMPLES_PER_CAPTURE
        
        self.mmD.seek(self.HEADER + trace_start * self.SAMPLES_PER_CAPTURE * self.FMT_SZ_LIST[self.FMT], os.SEEK_SET)
        
        bytes = self.mmD.read((self.FMT_SZ_LIST[self.FMT] * self.SAMPLES_PER_CAPTURE) * (trace_end - trace_start))
        
        mat = numpy.asarray(
            struct.unpack(str(sample_count * (trace_end - trace_start)) + self.FMT_STR_LIST[self.FMT], bytes), dtype=numpy.float32)
        mat = mat * (self.FRAC_NUM * 1.0 / self.FRAC_DENO)
        mat = mat.reshape((trace_end - trace_start, sample_count))
        return mat

    def read_trace(self, trace_id):
        self.mmD.seek(self.HEADER + trace_id * self.SAMPLES_PER_CAPTURE * self.FMT_SZ_LIST[self.FMT], os.SEEK_SET)

        bytes = self.mmD.read(self.FMT_SZ_LIST[self.FMT] * self.SAMPLES_PER_CAPTURE);

        mat = numpy.asarray(
            struct.unpack(str(self.SAMPLES_PER_CAPTURE) + self.FMT_STR_LIST[self.FMT], bytes), dtype=numpy.float32)
        mat = mat * (self.FRAC_NUM * 1.0 / self.FRAC_DENO)
        return mat

    def read(self, trace_id, sample_start_idx, sample_count):

        self.mmD.seek(
            self.HEADER + (sample_start_idx + trace_id * self.SAMPLES_PER_CAPTURE) * self.FMT_SZ_LIST[self.FMT],
            os.SEEK_SET)
        
        bytes = self.mmD.read(self.FMT_SZ_LIST[self.FMT] * sample_count);

        floats = struct.unpack(str(sample_count) + self.FMT_STR_LIST[self.FMT], bytes)

        mat = numpy.asarray(floats, dtype=numpy.float32)

        mat = mat * (self.FRAC_NUM * 1.0 / self.FRAC_DENO)

        return mat
        
    def write(self, trace_id, sample_start_idx, samples: numpy.ndarray):
        assert samples.dtype == NP_DT_LIST[self.FMT], "Data type needs to match the format in file"

        sample_count = numpy.size(samples)[0]

        self.mmD.seek(
            self.HEADER + (sample_start_idx + trace_id * self.SAMPLES_PER_CAPTURE) * self.FMT_SZ_LIST[self.FMT],
            os.SEEK_SET)

        self.mmD.write(samples.tobytes())

    def write_trace(self, trace):
        assert trace.size == self.SAMPLES_PER_CAPTURE, trace.size

        A = trace.tobytes()
        self.mmD.write(A)

    def close(self):
        self.mmD.close()
        self.fD.close()



class MergedBinTraces():
    def __init__(self, bintraces):
        self.bintraces = bintraces
        self.nt = bintraces[0].get_trace_count()
        self.ns = bintraces[0].get_sample_count()

    def read(self, trace_id, sample_start_idx, sample_count):
        return self.bintraces[trace_id // self.nt].read(trace_id % self.nt, sample_start_idx,
                                                        sample_count)

    def read_trace(self, trace_id):
        return self.bintraces[trace_id // self.nt].read_trace(trace_id % self.nt)

    def get_sample_count(self):
        return self.ns

    def get_trace_count(self):
        return len(self.bintraces) * self.nt

    def close(self):
        for tr in self.bintraces:
            tr.close()

# concatenate multiple sets of traces in to one
def merge( basepath, file):
    part = 0
    while os.path.exists(basepath +"/run_"+ str(part)):
        part +=1
    merged= []
    for p in range(0, part):
        filepath = basepath+"/run_"+str(p)+"/"+file
        print("loading %s" %(filepath))
        merged += [BinTraces(filepath)]
    return MergedBinTraces(merged)
