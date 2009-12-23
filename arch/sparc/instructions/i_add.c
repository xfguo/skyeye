/*
 * =====================================================================================
 *
 *       Filename:  i_add.c
 *
 *    Description:  Implementation of the ADD instruction
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

31-30  29-25   24-19     18-14    13            12-0
+----+-------+--------+------------------------------------------+
| op |   rd  |  op3   |   rs1   | i=1 |         simm13           |
+----+-------+--------+------------------------------------------+

op              = 10
rd              = 0000
op3             = 000000

31-30  29-25   24-19     18-14    13        12-5          4-0
+----+-------+--------+------------------------------------------+
| 10 | 00000 | 000000 |   rs1   | 1/0 |      simm13/rs2          |
+----+-------+--------+------------------------------------------+

*/

#include "../common/types.h"
#include "../common/sparc.h"
#include "../common/traps.h"
#include "../common/iu.h"
#include "../common/bits.h"
#include "i_utils.h"

#include <stdio.h>

static int execute(void *);
static int disassemble(uint32 instr, void *state);
static int rd, rs1, rs2, imm;

#define ADD_CYCLES    1
#define ADD_CODE_MASK 0x80000000
#define PRIVILEDGE  0

#define OP_OFF_first     30
#define OP_OFF_last      31
#define OP         0x2

#define OP3_OFF_first     19
#define OP3_OFF_last      24
#define OP3         0x00

#define RD_OFF_first      25
#define RD_OFF_last       29

#define RS1_OFF_first     14
#define RS1_OFF_last      18

#define RS2_OFF_first     0
#define RS2_OFF_last      4

#define I_OFF       13

#define SIMM13_OFF_first  0
#define SIMM13_OFF_last   12

sparc_instruction_t i_add = {
    execute,
    disassemble,
    ADD_CODE_MASK,
    PRIVILEDGE,
    OP,
};



static int execute(void *state)
{

    if( imm < 0 )
    {
        REG(rd) = REG(rs1) + REG(rs2);
        print_inst_RS_RS_RD("add", rs1, rs2, rd);
//        DBG("add %s, %s, %s", RNAME(rs1), RNAME(rs2), RNAME(rd));
    }
    else
    {
        REG(rd) = REG(rs1) + sign_ext13(imm);
        print_inst_RS_IM13_RD("add", rs1, sign_ext13(imm), rd);
//        DBG("add %s, %d, %s", RNAME(rs1), sign_ext13(imm), RNAME(rd));
    }

    PCREG = NPCREG;
    NPCREG += 4;

    // Everyting takes some time
    return ADD_CYCLES;

}

static int disassemble(uint32 instr, void *state)
{
    int op3 = 0, op;

    op = bits(instr, OP_OFF_last, OP_OFF_first);
    op3 = bits(instr, OP3_OFF_last, OP3_OFF_first);

    if( (instr & ADD_CODE_MASK) && (op == OP) && (op3 == OP3) )
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

