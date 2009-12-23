/*
 * =====================================================================================
 *
 *       Filename:  i_st.c
 *
 *    Description:  Implementation of the ST instruction
 *
 *        Version:  1.0
 *        Created:  16/04/08 18:09:22
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

/*  Format (3)

31-30  29-25   24-19     18-14    13            12-0
+----+-------+--------+------------------------------------------+
| op |   rd  |  op3   |   rs1   | i=1 |         simm13           |
+----+-------+--------+------------------------------------------+

op              = 11
rd              = 0000
op3             = 000100

31-30  29-25   24-19     18-14    13        12-5          4-0
+----+-------+--------+------------------------------------------+
| 11 | 00000 | 000100 |   rs1   | 1/0 |      simm12/rs2          |
+----+-------+--------+------------------------------------------+

*/
#include "../common/bits.h"
#include "../common/sparc.h"
#include "../common/traps.h"
#include "../common/iu.h"
#include "i_utils.h"
#include "../common/memory.h"

#include <stdio.h>

static int execute(void *);
static int disassemble(uint32 instr, void *state);
static int rd, rs1, rs2, imm;

#define ST_CYCLES    1
#define ST_CODE_MASK 0xc0200000
#define PRIVILEDGE  0

#define OP_OFF_first     30
#define OP_OFF_last      31
#define OP         0x3

#define OP3_OFF_first     19
#define OP3_OFF_last      24
#define OP3         0x04

#define RD_OFF_first      25
#define RD_OFF_last       29

#define RS1_OFF_first     14
#define RS1_OFF_last      18

#define RS2_OFF_first     0
#define RS2_OFF_last      8

#define I_OFF       13

#define SIMM13_OFF_first  0
#define SIMM13_OFF_last   12

sparc_instruction_t i_st = {
    execute,
    disassemble,
    ST_CODE_MASK,
    PRIVILEDGE,
    OP,
};



static int execute(void *state)
{
    int status;
    uint32 addr;

    if( imm < 0 )
        addr = REG(rs1) + REG(rs2);
    else
        addr = REG(rs1) + (int)sign_ext13(imm);

    /*  Check the word alignment. The address MUST be WORD aligned. That means
     *  that the last 2 bits of the address must be zero    */
    if( addr & 0x3 )
    {
        traps->signal(MEM_ADDR_NOT_ALIGNED);
        return(ST_CYCLES);
    }

//    DBG("st reg[%d], [ 0x%x ]\n", rd, addr);
    print_inst_RD_IMM("st", rd, addr);
    status = sparc_memory_store_word32(addr, REG(rd) & 0xffffffff);

    if( status < 0 )
    {
        traps->signal(DATA_STORE_ERROR);
        return(ST_CYCLES);
    }


    PCREG = NPCREG;
    NPCREG += 4;

    // Everyting takes some time
    return(ST_CYCLES);
}

static int disassemble(uint32 instr, void *state)
{
    int op3 = 0, op, i;

    op = bits(instr, OP_OFF_last, OP_OFF_first);

    if( (instr & ST_CODE_MASK) && (op == OP) )
    {
        op3 = bits(instr, OP3_OFF_last, OP3_OFF_first);  
        if( op3 != OP3 )
            return 0;


        rd = bits(instr, RD_OFF_last, RD_OFF_first);
        rs1 = bits(instr, RS1_OFF_last, RS1_OFF_first);
        i = bit(instr, I_OFF);

        if( i )
        {
            imm = bits(instr, SIMM13_OFF_last, SIMM13_OFF_first);
            rs2 = -1;
        }
        else
        {
            rs2 = bits(instr, RS2_OFF_last, RS2_OFF_first);
            imm = -1;
        }

        return 1;
    }
    return 0;
}

