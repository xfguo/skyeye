/*
 * =====================================================================================
 *
 *       Filename:  i_wry.c
 *
 *    Description:  Implementst the WRY SPARC instruction
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

31-30  29-25   24-19     18-14    13        12-5          4-0
+----+-------+--------+------------------------------------------+
| op |   rd  |  op3   |   rs1   | i=0 |   non-used   |    rs2    |
+----+-------+--------+------------------------------------------+

op              = 10
rd              = 0000
op3             = 110000

31-30  29-25   24-19     18-14    13        12-5          4-0
+----+-------+--------+------------------------------------------+
| 10 | 00000 | 000010 |   rs1   | i=0 |   non-used   |    rs2    |
+----+-------+--------+------------------------------------------+

*/
#include "../common/bits.h"
#include "../common/sparc.h"
#include "../common/traps.h"
#include "../common/iu.h"
#include "../common/debug.h"
#include "i_utils.h"
#include "skyeye.h"

#include <stdio.h>

static int execute(void *);
static int disassemble(uint32 instr, void *state);
static int rd, rs1, rs2, imm;

#define WRY_CYCLES    1
#define WRY_CODE_MASK 0x81800000
#define PRIVILEDGE  1

#define OP_OFF_first     30
#define OP_OFF_last      31
#define OP         0x2

#define OP3_OFF_first     19
#define OP3_OFF_last      24
#define OP3         0x30

#define RD_OFF_first      25
#define RD_OFF_last       29

#define RS1_OFF_first     14
#define RS1_OFF_last      18

#define RS2_OFF_first     0
#define RS2_OFF_last      4

#define I_OFF       13

#define SIMM13_OFF_first  0
#define SIMM13_OFF_last   12

sparc_instruction_t i_wry = {
    execute,
    disassemble,
    WRY_CODE_MASK,
    PRIVILEDGE,
    OP,
};



static int execute(void *state)
{
    uint32 value;

    if( imm < 0 )
    {
        value = REG(rs1) ^ REG(rs2);
        DBG("wr reg[%d], reg[%d], %s\n", rs1, rs2, "%y");
    }
    else
    {
        value = REG(rs1) ^ sign_ext13(imm);
        DBG("wr reg[%d], %d, %s\n", rs1, sign_ext13(imm), "%y");
    }

    /*  WRY is distinguished from WRASR only by the rd field. The rd must be
     *  zero and OP3 = 0x30 to write the Y register */
//    if( rd == 0 )
        YREG = value;
//    else
//    {
//        SKYEYE_ERR("ASR register not implemented\n");
//        skyeye_exit(1);
//    }

    PCREG = NPCREG;
    NPCREG += 4;

    // Everyting takes some time
    return WRY_CYCLES;
}

static int disassemble(uint32 instr, void *state)
{
    int op3 = 0, op;

    op = bits(instr, OP_OFF_last, OP_OFF_first);
    op3 = bits(instr, OP3_OFF_last, OP3_OFF_first);

    if( (instr & WRY_CODE_MASK) && (op == OP) && (op3 == OP3) )
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

