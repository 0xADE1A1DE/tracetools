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

#include "Util.h"
#include <stdio.h>
#include <unistd.h>
extern int __pw_log_verbosity;
extern int __pw_log_fd;
#define LOG_STAT_TIME 
#define LOG_STAT_TIME_STR "time" 
#define LOG_STAT_MEM
#define LOG_STAT_MEM_STR "mem"
#define LOG_STAT_INTL
#define LOG_STAT_INTL_STR "intl"

#define LogStat(__stat, ...) \
    { \
        dprintf(__pw_log_fd, __stat##_STR ": " __VA_ARGS__); \
    }
#define LogInfo(...) \
    dprintf(__pw_log_fd, "info: " __VA_ARGS__)
#define LogInfoV(__verb, ...) \
    { \
        if (__pw_log_verbosity > __verb) \
            dprintf(__pw_log_fd, "info: " __VA_ARGS__); \
    }
#define LogErr(...) \
    dprintf(__pw_log_fd, "err: " __VA_ARGS__)
#define LogErrDie(...) \
{ \
    dprintf(__pw_log_fd, "err: " __VA_ARGS__);\
    fsync(__pw_log_fd); \
    abort(); \
}

CAPI void pwlog_set_verbosity(int verbosity);
CAPI void pwlog_set_fd(int fd);
