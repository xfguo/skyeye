/*
 * =====================================================================================
 *
 *       Filename:  i_restore.c
 *
 *    Description:  Implementation of the RESTORE instruction
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
rd              = 00000
op3             = 111101

31-30  29-25   24-19     18-14    13        12-5          4-0
+----+-------+--------+------------------------------------------+
| 10 | 00000 | 111101 |   rs1   | 1/0 |      simm13/rs2          |
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

#define RESTORE_CYCLES    1
#define RESTORE_CODE_MASK 0x81e80000
#define PRIVILEDGE  0

#define OP_OFF_first     30
#define OP_OFF_last      31
#define OP         0x2

#define OP3_OFF_first     19
#define OP3_OFF_last      24
#define OP3         0x3d

#define RD_OFF_first      25
#define RD_OFF_last       29

#define RS1_OFF_first     14
#define RS1_OFF_last      18

#define RS2_OFF_first     0
#define RS2_OFF_last      4

#define I_OFF       13

#define SIMM13_OFF_first  0
#define SIMM13_OFF_last   12

sparc_instruction_t i_restore = {
    execute,
    disassemble,
    RESTORE_CODE_MASK,
    PRIVILEDGE,
    OP,
};



static int execute(void *state)
{

    uint32 wim, ncwp, cwp;
    int sum;

    if( imm < 0 )
    {
        sum = REG(rs1) + REG(rs2);
        print_inst_RS_RS_RD("restore", rs1, rs2, rd);
    }
    else
    {
        sum = REG(rs1) + sign_ext13(imm);
        print_inst_RS_IM13_RD("restore", rs1, sign_ext13(imm), rd);
    }

    /*  Value of the WIM register   */
    wim = WIMREG;
    cwp = CWP;

    /*  Try to advanced the window  */
    ncwp = (cwp + 1) & (N_WINDOWS - 1);

    /*  Check for underflow  */
    if( (1 << ncwp) & wim )
    {
        /*  Overflow, we do not perform the add */
        traps->signal(WUF);
        return RESTORE_CYCLES;
    }
    else
    {
        /*  Advance the window  */
        ncwp = iu_add_cwp();
        REG(rd) = sum;
    }

    PCREG = NPCREG;
    NPCREG += 4;

    // Everyting takes some time
    return RESTORE_CYCLES;

}

static int disassemble(uint32 instr, void *state)
{
    int op3 = 0, op;

    op = bits(instr, OP_OFF_last, OP_OFF_first);

    if( (instr & RESTORE_CODE_MASK) && (op == OP) )
    {
        op3 = bits(instr, OP3_OFF_last, OP3_OFF_first);
        if( op3 != OP3 )
            return 0;


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

