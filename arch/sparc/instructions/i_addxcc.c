/*
 * =====================================================================================
 *
 *       Filename:  i_addxcc.c
 *
 *    Description:  Implementation of the ADDXCC instruction where the carry bit
 *    is placed in the PSR's register
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

op              = 00
rd              = 0000
op3             = 011000

31-30  29-25   24-19     18-14    13        12-5          4-0
+----+-------+--------+------------------------------------------+
| 10 | 00000 | 011000 |   rs1   | 1/0 |      simm12/rs2          |
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

#define ADDXCC_CYCLES    1
#define ADDXCC_CODE_MASK 0x80c00000
#define PRIVILEDGE  0

#define OP_OFF_first     30
#define OP_OFF_last      31
#define OP         0x2

#define OP3_OFF_first     19
#define OP3_OFF_last      24
#define OP3         0x18

#define RD_OFF_first      25
#define RD_OFF_last       29

#define RS1_OFF_first     14
#define RS1_OFF_last      18

#define RS2_OFF_first     0
#define RS2_OFF_last      4

#define I_OFF       13

#define SIMM13_OFF_first  0
#define SIMM13_OFF_last   12


sparc_instruction_t i_addxcc = {
    execute,
    disassemble,
    ADDXCC_CODE_MASK,
    PRIVILEDGE,
    OP,
};



static int execute(void *state)
{
    uint32 x, y, z;
    /*
     * Sm: source operand (MSB)
     * Rm: Result operand (MSB)
     * Dm: Destination operand (MSB)
     */
    uint8 Sm, Dm, Rm;
    int c = psr_get_carry();

    x = REG(rs1);
    int src1 = REG(rd);
    if( imm < 0 )
    {
        y = REG(rs2);
        z = REG(rd) = (REG(rs1) + REG(rs2) + c);
        print_inst_RS_RS_RD("addxcc", rs1, rs2, rd);
//        DBG("addxcc reg[%d], reg[%d], reg[%d]\n", rs1, rs2, rd);
    }
    else
    {
        y = sign_ext13(imm);
        z = REG(rd) = (REG(rs1) + sign_ext13(imm) + c);
        print_inst_RS_IM13_RD("addxcc", rs1, sign_ext13(imm), rd);
//        DBG("addxcc reg[%d], %d, reg[%d]\n", rs1, sign_ext13(imm), rd);
    }

    clear_icc();

    /*  Negative condition  */
    if( bit(z, 31) )
        psr_set_neg();

    /*  Zero condition  */
    if( z == 0 )
        psr_set_zero(); 

    Sm = bit( x, 31 );  // RS1
    Dm = bit( y, 31 );  // RS2
    Rm = bit( z, 31 );  // RD

    /*  Set the OVERFLOW condition  */
    if ( (Sm && Dm && !Rm) || (!Sm && !Dm && Rm)  )
        psr_set_overflow();

    /*  Set the CARRY condition */
//    if( (Sm && Dm) || (!Rm && (Sm || Dm)) )
    if( z <= src1 )
        psr_set_carry();

    PCREG = NPCREG;
    NPCREG += 4;

    // Everyting takes some time
    return ADDXCC_CYCLES;
}

static int disassemble(uint32 instr, void *state)
{
    int op3 = 0, op;

    op = bits(instr, OP_OFF_last, OP_OFF_first);
    op3 = bits(instr, OP3_OFF_last, OP3_OFF_first);

    if( (instr & ADDXCC_CODE_MASK) && (op == OP) && (op3 == OP3) )
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

