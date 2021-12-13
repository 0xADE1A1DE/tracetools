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
#include "HWModel.h"
#include "PWModel.h"

#include "PWLeakageParams.h"
#include "Util.h"
#include "Log.h"

#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define STATE_CLASS_COUNT 4

#define ST_CLS_EORS 2
#define ST_CLS_STR 0
#define ST_CLS_LDR 1
#define ST_CLS_MOVS 3
// convert between order in coeff and classorder in elmo.c
static uint8_t stateconv[4] = {2, 0, 1, 3}; 

// immediate value
#define ST_REG_IMM ((1U<<16) -2)
#define ST_REG_NONE ((1U<<16) -1)
#define GET_BIT(_b, _x) \
    ((_x >> _b) & 1U)

#define TRACEFOLDER "."
#define TRACEFILE "/trace-pwmodel.txt"
#define PRINTALLNONPROFILEDTRACES 0
#define NONPROFILEDFOLDER "."
#define NONPROFILEDFILE "/nonprof-pwmodel.txt"
#define CYCLEACCURATE 0
// Coeffiecients for power model

typedef struct _PowerModel {
    double constant[5], PrvInstr[4][5], SubInstr[4][5], Operand1[32][5], Operand2[32][5], 
    	BitFlip1[32][5], BitFlip2[32][5],  HWOp1PrvInstr[4][5], HWOp2PrvInstr[4][5], 
    	HDOp1PrvInstr[4][5], HDOp2PrvInstr[4][5], HWOp1SubInstr[4][5], HWOp2SubInstr[4][5],
    	HDOp1SubInstr[4][5], HDOp2SubInstr[4][5], Operand1_bitinteractions[496][5], 
    	Operand2_bitinteractions[496][5], BitFlip1_bitinteractions[496][5], BitFlip2_bitinteractions[496][5];
    double HDState[128][5]; // state vars
    double HDOverwrite[1][5]; // movs overwrite
} PowerModel;

static PowerModel powermodel;

static void readcoeffs(double varaddress[][5], FILE *fp, int number){

    int i, j;
    char line[128];
    const char s[2] = " ";
    char *token;

    for (i=0;i<number;i++){

        fgets(line, sizeof line, fp);

        token = strtok(line, s);

        for(j=0; j<5; j++){

            sscanf(token, "%lf", &varaddress[i][j]);
            token = strtok(NULL, s);

        }
    }
}
/*void load_default(PowerModel* pwModel)
{
    int i=0;
    for (;i<5;i++)
    {
        pwModel->HDDestSrc[0][i] =1.0;
        pwModel->HDMBState[0][i] = 1.0;
    }
}*/

//-------------------------------------------------------------------
static void load_coeffs(char* coeffsfile, PowerModel* pwModel)
{
    FILE* fpcoeffs = fopen(coeffsfile, "r");
    unsigned int ra, j, fvr_only;
    char line[128];
    const char s[2] = " ";
    char *token;
    
    if (!fpcoeffs)
    {
        printf("can't load coeffs file\n");
        exit(-1);
    }
    fgets(line, sizeof line, fpcoeffs);
    token = strtok(line, s);

    for(j=0; j<5; j++){
	    if(token == NULL) {
	    	printf("Error: \"token\" at idx %d is NULL. Aborting.\n", j);
	    	exit(666);
	    }
        sscanf(token, "%lf", &pwModel->constant[j]);
        token = strtok(NULL, s);
    }

    readcoeffs(pwModel->PrvInstr,fpcoeffs, 4);
    readcoeffs(pwModel->SubInstr,fpcoeffs, 4);
    readcoeffs(pwModel->Operand1,fpcoeffs, 32);
    readcoeffs(pwModel->Operand2,fpcoeffs, 32);
    readcoeffs(pwModel->BitFlip1,fpcoeffs, 32);
    readcoeffs(pwModel->BitFlip2,fpcoeffs, 32);
    readcoeffs(pwModel->HWOp1PrvInstr,fpcoeffs, 4);
    readcoeffs(pwModel->HWOp2PrvInstr,fpcoeffs, 4);
    readcoeffs(pwModel->HDOp1PrvInstr,fpcoeffs, 4);
    readcoeffs(pwModel->HDOp2PrvInstr,fpcoeffs, 4);
    readcoeffs(pwModel->HWOp1SubInstr,fpcoeffs, 4);
    readcoeffs(pwModel->HWOp2SubInstr,fpcoeffs, 4);
    readcoeffs(pwModel->HDOp1SubInstr,fpcoeffs, 4);
    readcoeffs(pwModel->HDOp2SubInstr,fpcoeffs, 4);
    readcoeffs(pwModel->Operand1_bitinteractions,fpcoeffs, 496);
    readcoeffs(pwModel->Operand2_bitinteractions,fpcoeffs, 496);
    readcoeffs(pwModel->BitFlip1_bitinteractions,fpcoeffs, 496);
    readcoeffs(pwModel->BitFlip2_bitinteractions,fpcoeffs, 496);
    readcoeffs(pwModel->HDState,fpcoeffs, 4*32);
    readcoeffs(pwModel->HDOverwrite,fpcoeffs, 1);
    
    //load_default(pwModel);
    fclose(fpcoeffs);
}

static int hweight(unsigned int n)
{ 
    unsigned int c; // the total bits set in n
    for (c = 0; n; n >>= 1)
    {
        c += n & 1;
    }
    return c;
} 
  
//-------------------------------------------------------------------
  
static int hdistance(unsigned int x, unsigned int y)
{ 
    int dist = 0;
    unsigned  val = x ^ y;
    // Count the number of bits set
    while (val != 0)
    {
        // A bit is set, so increment the count and clear the bit    
        dist++;
        val &= val - 1;
    }
  
    // Return the number of differing bits
    return dist;
} 
static inline uint32_t get_instruction_type(uint32_t inst_id)
{
    return inst_id & 0xff;
}
static inline uint32_t get_state_class(uint32_t inst_id)
{
    uint32_t cc= (inst_id & 0xff00) >> 8;
    if (cc < STATE_CLASS_COUNT)
    {
        return stateconv[cc];
    }
    else
    {
        return cc;
    }
}
static inline uint32_t get_bit(uint32_t val, uint8_t bit)
{
    return (val >> bit) & 0x1U;
}
static inline uint32_t get_bitflip(uint32_t a, uint32_t b, uint8_t bit)
{
    return ((a ^ b) >> bit) & 0x1U;
}
void hwmodel_t::init(pwmodel_t* pm, const char* execfile)
{
 /*   load_coeffs("../coeffs.txt", &powermodel);
    //_analyzers = (pwanalyzer_t**) malloc(sizeof(pwanalyzer_t*) * N_PROPS);

    _totaltraces = pwutil_get_tracecount(execfile);
    int ntotaltraces = pwutil_get_tracecount(execfile);
    pwanalyzer_init(&_pwa, 
        ntotaltraces);
    char *dump_traces_file = getenv("PWL_PARAM_DUMP_TRACES_FILE");
    if (dump_traces_file)
    {
        pwanalyzer_set_param_str(_pwa, PWL_PARAM_DUMP_TRACES_FILE, dump_traces_file);
    }*/
}
void hwmodel_t::finit(pwmodel_t* pm)
{
    /*FILE* fpw=fopen("./output/fixedvsrandompropttest.txt","w");
    uint32_t sz = pwanalyzer_get_output_size(_pwa);
    for (int j=0;j<sz;j++)
    {
        double val = pwanalyzer_get_output_at(_pwa, j); 
        fprintf(fpw,"%lf\n",val);
    }
    fclose(fpw);
    fpw=fopen("./output/fixedvsrandomtstatistics.txt","w");
    for (int j=0;j<sz;j++)
    {
        double val = pwanalyzer_get_output_at(_pwa, j);
        fprintf(fpw,"%lf\n",val);
    }
    fclose(fpw);*/
}
static FILE *fxdumpfile;

void hwmodel_t::on_first_trace_done(pwmodel_t* pm)
{
}
void hwmodel_t::on_trace_done(pwmodel_t* pm)
{
    
    // NM_ prefixed variables are not included in model because they are not used for leakage detection
    // they used to detect the kind of leakage (model already includes these effects)
    char str[256], filepath[256];
    FILE *fp, *fp_nonprofiled;
    double differentialvoltage, supplycurrent, power;
    int last_ldrstr_state=-1;
    int hw_op1, hw_op2, hd_op1, hd_op2, instructiontype, i, j, count, index = 1;
    double PrvInstr_data = 0, SubInstr_data = 0, Operand1_data = 0, Operand2_data = 0, BitFlip1_data = 0, BitFlip2_data = 0, HWOp1PrvInstr_data = 0, HWOp2PrvInstr_data = 0, HDOp1PrvInstr_data = 0, HDOp2PrvInstr_data = 0, HWOp1SubInstr_data = 0, HWOp2SubInstr_data = 0, HDOp1SubInstr_data = 0, HDOp2SubInstr_data = 0, Operand1_bitinteractions_data = 0, Operand2_bitinteractions_data = 0, BitFlip1_bitinteractions_data = 0, BitFlip2_bitinteractions_data = 0;
    double Op1Op2_Overwrite_data = 0;
    double NM_Op1Op2_Overwrite_data = 0;
    double NM_Op1_Overwrite_data = 0;
    double NM_Op2_Overwrite_data = 0;
    double State_data = 0;
    double state_cum_data[STATE_CLASS_COUNT] = {0};
    uint32_t states[STATE_CLASS_COUNT] = {0};
    uint16_t states_op1_reg[STATE_CLASS_COUNT] = {ST_REG_NONE};
    uint16_t states_op2_reg[STATE_CLASS_COUNT] = {ST_REG_NONE};
    uint32_t state_class;
    uint32_t t = pm->currenttrace;
    pwmodel_input_t* previous;
    pwmodel_input_t* current;
    pwmodel_input_t* subsequent;
    pwmodel_input_t* start = pm->codesegment_start;
    pwmodel_input_t dummy;
    dummy.op1 = 0;
    dummy.op2 = 0;
    dummy.inst_id = 0x0100U;
    dummy.pc = 0;
    dummy.reg_op1 = ST_REG_NONE;
    dummy.reg_op2 = ST_REG_NONE;
    dummy.next = start;

    previous = &dummy;
    current = start;
    subsequent = start->next;

    strcpy(str, TRACEFOLDER);
    strcat(str, TRACEFILE);
    sprintf(filepath, str, t);
    if (t==0)
    {
        fp = fopen(filepath, "w+");
        fxdumpfile = fopen("./fdump-pwmodel.txt", "w");
    }

    if(t==0 || PRINTALLNONPROFILEDTRACES){
        strcpy(str, NONPROFILEDFOLDER);
        strcat(str, NONPROFILEDFILE);
        sprintf(filepath, str, t);
        fp_nonprofiled = fopen(filepath, "w+");
    }
    int inst_index = 0;
    double LDR_state_data = 0;
    double STR_state_data = 0;
    double EORS_state_data = 0;

    while(subsequent->next != NULL){
        differentialvoltage = 0;
        PrvInstr_data = 0; SubInstr_data = 0; Operand1_data = 0; Operand2_data = 0;
        BitFlip1_data = 0; BitFlip2_data = 0; HWOp1PrvInstr_data = 0; HWOp2PrvInstr_data = 0;
        HDOp1PrvInstr_data = 0; HDOp2PrvInstr_data = 0; HWOp1SubInstr_data = 0; 
        HWOp2SubInstr_data = 0; HDOp1SubInstr_data = 0; HDOp2SubInstr_data = 0; 
        Operand1_bitinteractions_data = 0; Operand2_bitinteractions_data = 0; 
        BitFlip1_bitinteractions_data = 0; BitFlip2_bitinteractions_data = 0;
        
        Op1Op2_Overwrite_data = 0;
        NM_Op1_Overwrite_data = 0;
        NM_Op2_Overwrite_data = 0;
        NM_Op1Op2_Overwrite_data = 0;
            
        State_data = 0;
        LDR_state_data = 0;
        STR_state_data = 0;
        uint8_t instruction_type_prev[6] = {0};
        uint8_t instruction_type_sub[6] = {0};
        PowerModel* powermodelptr = &powermodel;
        
        instructiontype =  get_instruction_type(current->inst_id);// current->instruction_typedec;

        //printf("instt %u\n",instructiontype);
        for (i=0;i<6;i++)
        {
            instruction_type_prev[i] = 0;
            instruction_type_sub[i] = 0;
        }

        instruction_type_prev[get_instruction_type(previous->inst_id)] = 1;
        instruction_type_sub[get_instruction_type(subsequent->inst_id)] = 1;
        // Test for key guessing space
        // if(t == 1)
        //      keyflowfailtest(current);
        //
        uint32_t current_state = 0;
        uint32_t current_state_class = get_state_class(current->inst_id);
        if (current_state_class == 10 || current_state_class == 11) /* PUSH POP*/
        {
            current_state = current->result;
        }
        else
        {
            current_state = current->op2;
        }
       

        // Modelled differential voltage is total of different factors (for debugging)
        if (current->pc == 0x0800041E)
        {
            //fprintf(fdumpfile, "%x %x %x\n", current->pc, current->op1, current->op2);

           /* for (i=0;i<STATE_CLASS_COUNT;i++)
            {
                double v = pwModel->HDState[i][instructiontype]*hdistance(states[i], current_state);
                State_data += v;

                fprintf(fdumpfile, "%x %lf %d\n", current->pc, pwModel->HDState[i][instructiontype], hdistance(states[i], current_state) );
            }*/

        }
        

        if(instructiontype == 5){

            //current->instruction_type[0] = 1;
            instructiontype = 0;

            //if(t==1 || PRINTALLNONPROFILEDTRACES)
            //    fprintf(fp_nonprofiled,"%d %x\n",index,current->pc);
        }

        else{

            for (i=0;i<STATE_CLASS_COUNT;i++)
            {
                int bit;
                double v=0;
                uint32_t statebitflip = states[i] ^ current_state;

                //fprintf(fxdumpfile, "%u %u\n", states[i], current_state);
                for (bit=0;bit<32;bit++)
                {
                    v += powermodelptr->HDState[i*STATE_CLASS_COUNT + bit][instructiontype] * GET_BIT(bit, statebitflip);
                }
                //double v = pwModel->HDState[i][instructiontype]*hdistance(states[i], current_state);
                state_cum_data[i] = v;
                
                State_data += v; 
            }

            Op1Op2_Overwrite_data = powermodelptr->HDOverwrite[0][instructiontype]*hdistance(current->op1 ,current->op2);
            NM_Op1Op2_Overwrite_data = hdistance(current->op1 ,current->op2);
            NM_Op1_Overwrite_data = hdistance(current->op1 ,previous->op1);
            NM_Op2_Overwrite_data = hdistance(current->op2 ,previous->op2);


            hw_op1 = hweight(current->op1);
            hw_op2 = hweight(current->op2);

            hd_op1 = hdistance(previous->op1, current->op1);
            hd_op2 = hdistance(previous->op2, current->op2);
/*
            for(i=0;i<32;i++){

                if(current->op1_binary[i] == previous->op1_binary[i])
                    current->op1_bitflip[i] = 0;
                else
                    current->op1_bitflip[i] = 1;

                if(current->op2_binary[i] == previous->op2_binary[i])
                    current->op2_bitflip[i] = 0;
                else
                    current->op2_bitflip[i] = 1;
            }
*/
            // For each bit of two inputs

            for(i=0;i<32;i++){

                // Input hamming weights
                Operand1_data = Operand1_data + powermodelptr->Operand1[i][instructiontype]*get_bit(current->op1, i);
                Operand2_data = Operand2_data + powermodelptr->Operand2[i][instructiontype]*get_bit(current->op2, i);

                // Input hamming distance
                BitFlip1_data = BitFlip1_data + powermodelptr->BitFlip1[i][instructiontype]*get_bitflip(current->op1, previous->op1, i);
                BitFlip2_data = BitFlip2_data + powermodelptr->BitFlip2[i][instructiontype]*get_bitflip(current->op2, previous->op2, i);

            }

            // For each instruction type

            for(i=0;i<4;i++){

                // Previous and subsequent factors
                PrvInstr_data = PrvInstr_data + powermodelptr->PrvInstr[i][instructiontype]*instruction_type_prev[i+1];
                SubInstr_data = SubInstr_data + powermodelptr->SubInstr[i][instructiontype]*instruction_type_sub[i+1];

                // Hamming weight of previous
                HWOp1PrvInstr_data = HWOp1PrvInstr_data + powermodelptr->HWOp1PrvInstr[i][instructiontype]*instruction_type_prev[i+1]*hw_op1;
                HWOp2PrvInstr_data = HWOp2PrvInstr_data + powermodelptr->HWOp2PrvInstr[i][instructiontype]*instruction_type_prev[i+1]*hw_op2;

                // Hamming distance of previous
                HDOp1PrvInstr_data = HDOp1PrvInstr_data + powermodelptr->HDOp1PrvInstr[i][instructiontype]*instruction_type_prev[i+1]*hd_op1;
                HDOp2PrvInstr_data = HDOp2PrvInstr_data + powermodelptr->HDOp2PrvInstr[i][instructiontype]*instruction_type_prev[i+1]*hd_op2;

                // Hamming weight of subsequence
                HWOp1SubInstr_data = HWOp1SubInstr_data + powermodelptr->HWOp1SubInstr[i][instructiontype]*instruction_type_sub[i+1]*hw_op1;
                HWOp2SubInstr_data = HWOp2SubInstr_data + powermodelptr->HWOp2SubInstr[i][instructiontype]*instruction_type_sub[i+1]*hw_op2;
 
                // Hamming distance of subsequent
                HDOp1SubInstr_data = HDOp1SubInstr_data + powermodelptr->HDOp1SubInstr[i][instructiontype]*instruction_type_sub[i+1]*hd_op1;
                HDOp2SubInstr_data = HDOp2SubInstr_data + powermodelptr->HDOp2SubInstr[i][instructiontype]*instruction_type_sub[i+1]*hd_op2;

            }
            // Higher order bit interactions

            count = 0;
            
            for(i=0;i<32;i++){
                for(j=i+1;j<32;j++){
                    
                    // Input hamming weights
                    Operand1_bitinteractions_data = Operand1_bitinteractions_data + 
                        powermodelptr->Operand1_bitinteractions[count][instructiontype]*get_bit(current->op1, i)*get_bit(current->op1, j);//current->op1_binary[i]*current->op1_binary[j];
                    Operand2_bitinteractions_data = Operand2_bitinteractions_data + 
                        powermodelptr->Operand2_bitinteractions[count][instructiontype]*get_bit(current->op2, i)*get_bit(current->op2, j);

                    // Input hamming distance
                    BitFlip1_bitinteractions_data = BitFlip1_bitinteractions_data + 
                        powermodelptr->BitFlip1_bitinteractions[count][instructiontype]*get_bitflip(current->op1, previous->op1, i)*get_bitflip(current->op1, previous->op1, j);//*current->op1_bitflip[i]*current->op1_bitflip[j];
                    BitFlip2_bitinteractions_data = BitFlip2_bitinteractions_data + 
                        powermodelptr->BitFlip2_bitinteractions[count][instructiontype]*get_bitflip(current->op2, previous->op2, i)*get_bitflip(current->op2, previous->op2, j);

                    count++;

                }
            }
            
            count = 0;
    
            differentialvoltage = powermodelptr->constant[instructiontype] + PrvInstr_data + SubInstr_data + Operand1_data + Operand2_data +
                BitFlip1_data + BitFlip2_data + HWOp1PrvInstr_data + HWOp2PrvInstr_data + HDOp1PrvInstr_data + HDOp2PrvInstr_data +
                HWOp1SubInstr_data + HWOp2SubInstr_data + HDOp1SubInstr_data + HDOp2SubInstr_data + Operand1_bitinteractions_data +
                Operand2_bitinteractions_data + BitFlip1_bitinteractions_data + BitFlip2_bitinteractions_data +
                State_data + Op1Op2_Overwrite_data;
            //EORS_state_data = State_data; 
        }

               
        if (current_state_class == 10 || current_state_class == 11)
        {
            // as leakage matrix(2) shows that push/pop can stop any leakage...
            states[ST_CLS_STR] = current->result;
            states[ST_CLS_EORS] = current->result;
            states[ST_CLS_LDR] = current->result;
            states[ST_CLS_MOVS] = current->result;
            
            states_op1_reg[ST_CLS_STR] = ST_REG_NONE;
            states_op1_reg[ST_CLS_EORS] = ST_REG_NONE;
            states_op1_reg[ST_CLS_LDR] = ST_REG_NONE;
            states_op1_reg[ST_CLS_MOVS] = ST_REG_NONE;

            states_op2_reg[ST_CLS_STR] = ST_REG_NONE;
            states_op2_reg[ST_CLS_EORS] = ST_REG_NONE;
            states_op2_reg[ST_CLS_LDR] = ST_REG_NONE;
            states_op2_reg[ST_CLS_MOVS] = ST_REG_NONE;

        }
        else
        {
            states[current_state_class] = current->op2;
            // when str instruction is run the register at op2
            // is set as the connection to the memory bus and therefore
            // leakage is present when the value at that register is 
            // changed due to a leaky value from a former instruction
            // it is effectively resetting str state through another
            // instruction
            // * reg_op1 is used because it is the dest register
            
            pwmodel_input_t* newinst=previous;
            //fprintf(fxdumpfile, "%u %u\n", get_state_class(newinst->inst_id), newinst->reg_op1);
            if ((get_state_class(newinst->inst_id) != ST_CLS_STR) && (states_op2_reg[ST_CLS_STR] == newinst->reg_op1))
            {
                states[ST_CLS_STR] = 0;//newinst->result;
            }
            //fflush(fxdumpfile);
            // similar effects also happen for ALU instructions
            if (get_state_class(newinst->inst_id) != ST_CLS_EORS && states_op2_reg[ST_CLS_EORS] == newinst->reg_op1)
            {
                states[ST_CLS_EORS] = 0;//newinst->result;
            }

            states_op1_reg[current_state_class] = current->reg_op1;
            states_op2_reg[current_state_class] = current->reg_op2;

        }
        /*pwanalyzer_trace_update(_analyzers[0], pm->currenttrace, inst_index, powermodelptr->constant[instructiontype]);
        pwanalyzer_trace_update(_analyzers[1], pm->currenttrace, inst_index, PrvInstr_data);
        pwanalyzer_trace_update(_analyzers[2], pm->currenttrace, inst_index, SubInstr_data);
        pwanalyzer_trace_update(_analyzers[3], pm->currenttrace, inst_index, Operand1_data);
        pwanalyzer_trace_update(_analyzers[4], pm->currenttrace, inst_index, Operand2_data);
        pwanalyzer_trace_update(_analyzers[5], pm->currenttrace, inst_index, BitFlip1_data);
        pwanalyzer_trace_update(_analyzers[6], pm->currenttrace, inst_index, BitFlip2_data);
        pwanalyzer_trace_update(_analyzers[7], pm->currenttrace, inst_index, HWOp1PrvInstr_data);
        pwanalyzer_trace_update(_analyzers[8], pm->currenttrace, inst_index, HWOp2PrvInstr_data);
        pwanalyzer_trace_update(_analyzers[9], pm->currenttrace, inst_index, HDOp1PrvInstr_data);
        pwanalyzer_trace_update(_analyzers[10], pm->currenttrace, inst_index, HDOp2PrvInstr_data);
        pwanalyzer_trace_update(_analyzers[11], pm->currenttrace, inst_index, HWOp1SubInstr_data);
        pwanalyzer_trace_update(_analyzers[12], pm->currenttrace, inst_index, HWOp2SubInstr_data);
        pwanalyzer_trace_update(_analyzers[13], pm->currenttrace, inst_index, HDOp1SubInstr_data);
        pwanalyzer_trace_update(_analyzers[14], pm->currenttrace, inst_index, HDOp2SubInstr_data);
        pwanalyzer_trace_update(_analyzers[15], pm->currenttrace, inst_index, Operand1_bitinteractions_data);
        pwanalyzer_trace_update(_analyzers[16], pm->currenttrace, inst_index, Operand2_bitinteractions_data);
        pwanalyzer_trace_update(_analyzers[17], pm->currenttrace, inst_index, BitFlip1_bitinteractions_data);
        pwanalyzer_trace_update(_analyzers[18], pm->currenttrace, inst_index, BitFlip2_bitinteractions_data);
        pwanalyzer_trace_update(_analyzers[19], pm->currenttrace, inst_index, state_cum_data[ST_CLS_LDR]);
        pwanalyzer_trace_update(_analyzers[20], pm->currenttrace, inst_index, state_cum_data[ST_CLS_STR]);
        pwanalyzer_trace_update(_analyzers[21], pm->currenttrace, inst_index, NM_Op1Op2_Overwrite_data);
        pwanalyzer_trace_update(_analyzers[22], pm->currenttrace, inst_index, NM_Op1_Overwrite_data);
        pwanalyzer_trace_update(_analyzers[23], pm->currenttrace, inst_index, NM_Op2_Overwrite_data);
        pwanalyzer_trace_update(_analyzers[24], pm->currenttrace, inst_index, state_cum_data[ST_CLS_EORS]);
        pwanalyzer_trace_update(_pwa, pm->currenttrace, inst_index, differentialvoltage);*/

#ifdef PWPROPS_ENABLE
        // Convert from differential voltage to power
        pwprops_update(_props ,pm->currenttrace, powermodelptr->constant[instructiontype], 0, inst_index); //A
        pwprops_update(_props ,pm->currenttrace, PrvInstr_data, 1, inst_index); //B
        pwprops_update(_props ,pm->currenttrace, SubInstr_data, 2, inst_index); //C
        pwprops_update(_props ,pm->currenttrace, Operand1_data, 3, inst_index); //D
        pwprops_update(_props ,pm->currenttrace, Operand2_data, 4, inst_index); //E
        pwprops_update(_props ,pm->currenttrace, BitFlip1_data, 5, inst_index); //F
        pwprops_update(_props ,pm->currenttrace, BitFlip2_data, 6, inst_index); //G
        pwprops_update(_props ,pm->currenttrace, HWOp1PrvInstr_data, 7, inst_index); //H
        pwprops_update(_props ,pm->currenttrace, HWOp2PrvInstr_data, 8, inst_index); //I
        pwprops_update(_props ,pm->currenttrace, HDOp1PrvInstr_data, 9, inst_index); //J
        pwprops_update(_props ,pm->currenttrace, HDOp2PrvInstr_data, 10, inst_index); //k
        pwprops_update(_props ,pm->currenttrace, HWOp1SubInstr_data, 11, inst_index); //l
        pwprops_update(_props ,pm->currenttrace, HWOp2SubInstr_data, 12, inst_index); //m
        pwprops_update(_props ,pm->currenttrace, HDOp1SubInstr_data, 13, inst_index); //n
        pwprops_update(_props ,pm->currenttrace, HDOp2SubInstr_data, 14, inst_index); //o
        pwprops_update(_props ,pm->currenttrace, Operand1_bitinteractions_data, 15, inst_index); //p
        pwprops_update(_props ,pm->currenttrace, Operand2_bitinteractions_data, 16, inst_index); //q
        pwprops_update(_props ,pm->currenttrace, BitFlip1_bitinteractions_data, 17, inst_index); //r
        pwprops_update(_props ,pm->currenttrace, BitFlip2_bitinteractions_data, 18, inst_index); //s
        pwprops_update(_props ,pm->currenttrace, state_cum_data[ST_CLS_LDR], 19, inst_index); //t
        pwprops_update(_props ,pm->currenttrace, state_cum_data[ST_CLS_STR], 20, inst_index); //u
        pwprops_update(_props ,pm->currenttrace, NM_Op1Op2_Overwrite_data, 21, inst_index); //v
        pwprops_update(_props ,pm->currenttrace, NM_Op1_Overwrite_data, 22, inst_index); //w
        pwprops_update(_props ,pm->currenttrace, NM_Op2_Overwrite_data, 23, inst_index); //x
        pwprops_update(_props ,pm->currenttrace, state_cum_data[ST_CLS_EORS], 24, inst_index); //y
        pwprops_update(_props ,pm->currenttrace, differentialvoltage, 25, inst_index); //z
#endif
        //set_inst_type(instructiontype, inst_index);
        inst_index ++;
#ifdef POWERTRACES
        supplycurrent = differentialvoltage/RESISTANCE;
        power = supplycurrent*SUPPLYVOLTAGE;
#else
        power = differentialvoltage;
#endif
#ifdef PWPROPS_ENABLE
        pwprops_set_cycles(_props, inst_index, 1);
#endif

        if(instructiontype == 2 | instructiontype == 3){
            if(CYCLEACCURATE){
#ifdef PWPROPS_ENABLE
                pwprops_set_cycles(_props, inst_index, 2);
#endif
                if (t == 0)
                {
#ifdef BINARYTRACES
                    fwrite(&power, sizeof(power), 1, fp);
                    fwrite(&power, sizeof(power), 1, fp);
#else
                    fprintf(fp,"%0.40f\n",power);
                    fprintf(fp,"%0.40f\n",power);
#endif
                }
                index += 2;
            }
            else{
                if (t == 0)
                {
#ifdef BINARYTRACES
                    fwrite(&power, sizeof(power), 1, fp);
#else
                    fprintf(fp,"%0.40f\n",power);
#endif
                }
                index += 1;
            }
        }
        else{
        	
            if (t == 0)
            {
#ifdef BINARYTRACES
                fwrite(&power, sizeof(power), 1, fp);
#else
                fprintf(fp,"%0.40f\n",power);
#endif
            }
            index += 1;
        }

        previous = previous->next;
        current = current->next;
        subsequent = subsequent->next;

    }

   // pwanalyzer_trace_done(_pwa);

//    calc_props_fin();

    if (t == 0)
    {
        fclose(fp);
    }

    if(t==0 || PRINTALLNONPROFILEDTRACES) fclose(fp_nonprofiled);

}
