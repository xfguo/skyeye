/*
 * =====================================================================================
 *
 *       Filename:  i_sth.c
 *
 *    Description:  Implementation of the STH instruction
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
op3             = 000110

31-30  29-25   24-19     18-14    13        12-5          4-0
+----+-------+--------+------------------------------------------+
| 11 | 00000 | 000110 |   rs1   | 1/0 |      simm12/rs2          |
+----+-------+--------+------------------------------------------+

*/
#include "../common/bits.h"
#include "../common/sparc.h"
#include "../common/traps.h"
#include "../common/iu.h"
#include "../common/debug.h"
#include "../common/memory.h"
#include "i_utils.h"

#include <stdio.h>

static int execute(void *);
static int disassemble(uint32 instr, void *state);
static int rd, rs1, rs2, imm;


#define STH_CYCLES    1
#define STH_CODE_MASK 0xc0300000
#define PRIVILEDGE  0

#define OP_OFF_first     30
#define OP_OFF_last      31
#define OP         0x3

#define OP3_OFF_first     19
#define OP3_OFF_last      24
#define OP3         0x06

#define RD_OFF_first      25
#define RD_OFF_last       29

#define RS1_OFF_first     14
#define RS1_OFF_last      18

#define RS2_OFF_first     0
#define RS2_OFF_last      4

#define I_OFF       13

#define SIMM13_OFF_first  0
#define SIMM13_OFF_last   12

sparc_instruction_t i_sth = {
    execute,
    disassemble,
    STH_CODE_MASK,
    PRIVILEDGE,
    OP,
};



static int execute(void *state)
{
    int status;
    uint32 addr = 0;

    if( imm < 0 )
    {
        addr = REG(rs1) + REG(rs2);

        if( addr & 0x1 )
        {
            traps->signal(MEM_ADDR_NOT_ALIGNED);
            return(STH_CYCLES);
        }

        status = sparc_memory_store_word16(addr, REG(rd) & 0x0000ffff);
//        DBG("sth reg[%d], [ reg[%d] + reg[%d] ]\n", rd, rs1, rs2);
        print_inst_ST_RS_RS_RD("sth", rd, rs1, rs2);
    }
    else
    {
        addr = REG(rs1) + (int)sign_ext13(imm);

        if( addr & 0x1 )
        {
            traps->signal(MEM_ADDR_NOT_ALIGNED);
            return(STH_CYCLES);
        }

        status = sparc_memory_store_word16(addr, REG(rd) & 0x0000ffff);
//        DBG("sth reg[%d], [ reg[%d] + %d ]\n", rd, rs1, sign_ext13(imm));
        print_inst_ST_RS_IM13_RD("sth", rd, rs1, sign_ext13(imm));
    }

    if( status < 0 )
    {
        traps->signal(DATA_STORE_ERROR);
        return(STH_CYCLES);
    }


    PCREG = NPCREG;
    NPCREG += 4;

    // Everyting takes some time
    return(STH_CYCLES);
}

static int disassemble(uint32 instr, void *state)
{

    int op3 = 0, op;

    op = bits(instr, OP_OFF_last, OP_OFF_first);
    op3 = bits(instr, OP3_OFF_last, OP3_OFF_first);

    if( (instr & STH_CODE_MASK) && (op == OP) &&(op3 == OP3) )
    {
        rd = bits(instr, RD_OFF_last, RD_OFF_first);
        rs1 = bits(instr, RS1_OFF_last, RS1_OFF_first);
        int i = bit(instr, I_OFF);

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

