/*
 * =====================================================================================
 *
 *       Filename:  i_ta.c
 *
 *    Description:  Implementation of the TA instruction
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

/*  Format (2)

31-30  29   28-25    24-19     18-14    13   12-5       4-0
+----+----+--------+------------------------------------------+
| op | na |  cond  | 111010  |   rs1   |i=0| reserved  | rs2  |
+----+----+--------+------------------------------------------+

31-30  29   28-25    24-19     18-14    13   12-7       6-0
+----+----+--------+------------------------------------------+
| op | na |  cond  | 111010  |   rs1   |i=1| reserved | imm7  |
+----+----+--------+------------------------------------------+

op              = 00
cond            = 1000

+----+----+--------+------------------------------------------+
| op | na |  1000  | 111010  |   rs1   |i=1| reserved | imm7  |
+----+----+--------+------------------------------------------+

*/
#include "../common/bits.h"
#include "../common/sparc.h"
#include "../common/traps.h"
#include "../common/iu.h"
#include "../common/debug.h"
#include "i_utils.h"

#include <stdio.h>

static int execute(void *);
static int disassemble(uint32 instr, void *state);
static int cond, imm7, fixed, op, rs2, rs1;

#define TA_CYCLES    1
#define TA_CODE_MASK 0x91d00000
#define PRIVILEDGE  0

#define OP_OFF_first        30
#define OP_OFF_last         31
#define OP         0x2

#define NA_OFF              29

#define COND_OFF_last       28
#define COND_OFF_first      25
#define COND         0x8

#define FIXED_OFF_last      24
#define FIXED_OFF_first     19
#define FIXED   0x3a

#define RS1_OFF_last        18
#define RS1_OFF_first       14

#define I_OFF               13

#define RS2_OFF_last        4
#define RS2_OFF_first       0

#define IMM7_OFF_last   6
#define IMM7_OFF_first  0

sparc_instruction_t i_ta = {
    execute,
    disassemble,
    TA_CODE_MASK,
    PRIVILEDGE,
    OP,
};

static int execute(void *state)
{
    int tt;

    /*  The tt field is writen with 128 + the leas significant seven bits of
     *  r[rs1] + r[rs2] if 'i' field is zero, or 128 + least significant bits of
     *  r[rs1] + sign_ext(imm7) if the 'i' field is one */ 
    if( imm7 < 0 )
        tt = 128 + bits(REG(rs1), 6, 0) + bits(REG(rs2), 6, 0);
    else if(rs2 < 0)
        tt = 128 + bits(REG(rs1), 6, 0) + imm7;

    traps->signal(tt);

    DBG("ta %d\n", tt - 128);

    // Everyting takes some time
    return(TA_CYCLES);

}

static int disassemble(uint32 instr, void *state)
{

    int op;

    op = bits(instr, OP_OFF_last, OP_OFF_first);
    cond = bits(instr, COND_OFF_last, COND_OFF_first);
    fixed = bits(instr, FIXED_OFF_last, FIXED_OFF_first);

    if( (instr & TA_CODE_MASK) && (op == OP) && (fixed == FIXED) )
    {
        rs1 = bits(instr, RS1_OFF_last, RS1_OFF_first);

        int i = bit(instr, I_OFF);

        if( i )
        {
            imm7 = bits(instr, IMM7_OFF_last, IMM7_OFF_first);
            rs2 = -1;
        }
        else
        {
            rs2 = bits(instr, RS2_OFF_last, RS2_OFF_first);
            imm7 = -1;
        }

        return 1;
    }

    return 0;
}

