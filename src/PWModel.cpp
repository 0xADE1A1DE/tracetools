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
#include "PWModel.h"
#include "IPWModel.h"
#include "PWConfig.h"
#include "ElmoExtModel.h" 
#include "ElmoModel.h"
#include "HWModel.h"
#include "Log.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static pwmodel_input_t* make_input(pwmodel_t* pm, uint64_t pc)
{
    pwmodel_input_t* inp = (pwmodel_input_t*)malloc(sizeof(pwmodel_input_t));
    inp->inst_id = 0;
    inp->pc = pc;
    inp->op1 = 0;
    inp->op2 = 0;
    inp->reg_op1 = 0;
    inp->reg_op2 = 0;
    inp->result = 0;
    inp->next = NULL;
    return inp;
}
pwmodel_i* pwmodel_get_model(const char* modelname)
{
    if (strcmp(modelname, "elmo*") == 0)
    {
        return new elmoextmodel_t;
    }
    else if (strcmp(modelname, "elmo") == 0)
    {
        return new elmomodel_t;
    }
    else if (strcmp(modelname, "hw") == 0)
    {
        return new hwmodel_t;
    }
    return nullptr;
}
void pwmodel_begin(pwmodel_t* pm, uint64_t pc)
{
    if (pm->startpc == 0)
    {
        pm->codesegment_start = make_input(pm, pc);
        pm->startpc = pc;
    }
    else if (pc != pm->startpc)
    {
        LogErr("invalid start pc address\n");
        exit(-1);
    }
    pm->codesegment_current = pm->codesegment_start;
    pm->nsamples = 0;
}   
// copy preserving next ptr (only data)
void pwmodel_copy_input(pwmodel_input_t* dest, pwmodel_input_t* src)
{
    pwmodel_input_t* next= dest->next; 
    memcpy(dest, src, sizeof(pwmodel_input_t));
    dest->next = next;
}
void pwmodel_update(pwmodel_t* pm, pwmodel_input_t* pi)
{
    pwmodel_copy_input(pm->codesegment_current, pi);
    if (pm->codesegment_current->next == NULL)
        pm->codesegment_current->next = make_input(pm, (uint64_t)-1);
    pm->codesegment_current = pm->codesegment_current->next;
    pm->nsamples += 1;
}
void pwmodel_update_inputdata(pwmodel_t* pm, uint8_t* inputdata, uint32_t length)
{
    if (pm->ndata_first != 0 && pm->ndata_first != length)
    {
        LogErr("trace data counts differ\n");
        exit(-1);
    }
    
    if (pm->currenttrace == 0)
    {
        pm->ndata_first = length;
        pm->inputdata = (uint8_t*) malloc(length * sizeof(uint8_t));
        pm->inputdatalength = length;
    }
    
    memcpy(pm->inputdata, inputdata, pm->inputdatalength * sizeof(uint8_t));
}
void pwmodel_end(pwmodel_t* pm, uint64_t pc)
{
    if (pm->nsamples_first != 0 && pm->nsamples_first != pm->nsamples)
    {
        LogErr("trace sample counts differ\n");
        exit(-1);
    }
    if (pm->endpc == 0)
    {
        pm->endpc = pc;
    }
    else if (pc != pm->endpc)
    {
        LogErr("invalid end pc address\n");
        exit(-1);
    }
    pm->codesegment_current->pc = pc;
    pm->codesegment_current->next = NULL;
    if (pm->currenttrace == 0)
    {
        pm->nsamples_first = pm->nsamples;
        pm->model->on_first_trace_done(pm);
    }
    pm->model->on_trace_done(pm);
    pm->currenttrace ++;
}
void pwmodel_init(pwmodel_t** pm, const char* execfile)
{
    pwconfig_init();
    *pm = (pwmodel_t*)malloc(sizeof(pwmodel_t));
    auto pmptr = *pm;
    pmptr->currenttrace = 0;
    pmptr->nsamples = 0;
    pmptr->nsamples_first = 0;
    pmptr->ndata_first = 0;
    pmptr->startpc = 0;
    pmptr->endpc = 0;
    pmptr->inputdata = NULL;
    pmptr->inputdatalength = 0;
    const char* model = pwconfig_get_value("PWM_MODEL");
    pmptr->model = pwmodel_get_model(model);
    pmptr->model->init(pmptr, execfile);
    
    pwlog_set_verbosity(0);
}
void pwmodel_dump(pwmodel_t* pm)
{
    pwmodel_input_t* g=pm->codesegment_start;
    while (g != NULL)
    {
        printf("%lx %x\n", g->pc, g->inst_id);
        g = g->next;
    }
}
uint32_t pwmodel_get_sample_count(pwmodel_t* pm)
{
    return pm->nsamples_first;
}
void pwmodel_finit(pwmodel_t* pm)
{
    pm->model->finit(pm);
    free(pm->inputdata);
    free(pm);
}
