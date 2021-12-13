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
#include "Util.h"

#include <string.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <linux/limits.h>

int pwutil_get_tracecount(const char* execfile)
{
    char* exec_file_cp = strdup(execfile);
    char* exec_dir = dirname(exec_file_cp);
    char tracecount_buf[256];
    snprintf(tracecount_buf, 255, "%s/tracecount.txt", exec_dir);
    free(exec_file_cp);
    FILE *ft = fopen(tracecount_buf, "r");
    if (!ft)
    {
        printf("place tracecount.txt with traces count in exec dir\n");
        exit(-1);
    }
    int tracecount; 
    fscanf(ft,"%d",&tracecount);
    printf("%d\n",tracecount);
    fclose(ft);
    return tracecount;
}

char* pwutil_dirname(char *path)
{
    return dirname(path);
}

int pwutil_abspath(const char* relpath, char* abspath, int abspathlen)
{
    char actualpath[PATH_MAX+1];
    char *resolvpath = realpath(relpath, actualpath);
    if (resolvpath)
    {
        strncpy(abspath, resolvpath, abspathlen);
        return 0;
    }
    return -1;
}
int hexchar_to_int(char hex)
{
    if ((hex >= 'a') && (hex <= 'z'))
    {
        return 10 + hex - 'a';
    }
    else
    {
        return hex - '0';
    }
}
char int_to_hexchar(uint8_t ival)
{
    if (ival <10)
    {
        return '0' + ival;
    }
    else
    {
        return 'a' + (ival - 10);
    }
}
void byte_to_hex(uint8_t byte, char* hex)
{
    hex[0] = int_to_hexchar((byte & 0xF0) >> 4);
    hex[1] = int_to_hexchar((byte & 0x0F));
}
uint8_t hex_to_byte(const char hex[2])
{
    char a = tolower(hex[0]);
    char b = tolower(hex[1]);
    
    return hexchar_to_int(a) << 4 | hexchar_to_int(b);
}
void hexstr_to_bytebuf(const char* hexstr, uint8_t* bytebuf)
{
    while (*hexstr != '\0')
    {
        *bytebuf = hex_to_byte(hexstr);
        hexstr += 2;
        bytebuf += 1;
    }
}

void bytebuf_to_hexstr(const uint8_t* bytebuf, int bytebuflen, char* hexstr)
{
    for (int i=0;i<bytebuflen;i++)
    {
        byte_to_hex(bytebuf[i], hexstr);
        hexstr += 2;
    }
    *hexstr = '\0';
}
