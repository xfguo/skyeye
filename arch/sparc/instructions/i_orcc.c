/*
 * =====================================================================================
 *
 *       Filename:  i_orcc.c
 *
 *    Description:  
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

op              = 00
rd              = 0000
op3             = 010010

31-30  29-25   24-19     18-14    13        12-5          4-0
+----+-------+--------+------------------------------------------+
| 10 | 00000 | 010010 |   rs1   | i=0 |   non-used   |    rs2    |
+----+-------+--------+------------------------------------------+

*/
#include "../common/bits.h"
#include "../common/sparc.h"
#include "../common/traps.h"
#include "../common/iu.h"
#include "i_utils.h"

#include <stdio.h>

static int execute(void *);
static int disassemble(uint32 instr, void *state);
static int rd, rs1, rs2, imm;

#define ORCC_CYCLES    1
#define ORCC_CODE_MASK 0x80900000
#define PRIVILEDGE  0

#define OP_OFF_first     30
#define OP_OFF_last      31
#define OP         0x2

#define OP3_OFF_first     19
#define OP3_OFF_last      24
#define OP3         0x12

#define RD_OFF_first      25
#define RD_OFF_last       29

#define RS1_OFF_first     14
#define RS1_OFF_last      18

#define RS2_OFF_first     0
#define RS2_OFF_last      4

#define I_OFF       13

#define SIMM13_OFF_first  0
#define SIMM13_OFF_last   12

sparc_instruction_t i_orcc = {
    execute,
    disassemble,
    ORCC_CODE_MASK,
    PRIVILEDGE,
    OP,
};



static int execute(void *state)
{
    int x, y, z;

    x = REG(rs1);
    if( imm < 0 )
    {
        y = REG(rs2);
        z = REG(rd) = (REG(rs1) | REG(rs2));
//        DBG("orcc reg[%d], reg[%d], reg[%d]\n", rs1, rs2, rd);
        print_inst_RS_RS_RD("orcc", rs1, rs2, rd);
    }
    else
    {
        y = sign_ext13(imm);
        z = REG(rd) = (REG(rs1) | sign_ext13(imm));
//        DBG("orcc reg[%d], 0x%x, reg[%d]\n", rs1, imm, rd);
        print_inst_RS_IM13_RD("orcc", rs1, sign_ext13(imm), rd);
    }


    clear_icc();

    /*  Zero condition  */
    if( z == 0 )
        psr_set_zero(); 

    /*  Negative condition  */
    if( bit(z, 31) )
        psr_set_neg();

    PCREG = NPCREG;
    NPCREG += 4;

    // Everyting takes some time
    return ORCC_CYCLES;
}

static int disassemble(uint32 instr, void *state)
{
    int op3 = 0, op;

    op = bits(instr, OP_OFF_last, OP_OFF_first);
    op3 = bits(instr, OP3_OFF_last, OP3_OFF_first);

    if( (instr & ORCC_CODE_MASK) && (op == OP) && (op3 == OP3) )
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

