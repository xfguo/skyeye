/*
 * =====================================================================================
 *
 *       Filename:  i_mulscc.c
 *
 *    Description:  Implementation of the SUBCC instruction where the carry bit
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

op              = 10
rd              = 0000
op3             = 010100

31-30  29-25   24-19     18-14    13        12-5          4-0
+----+-------+--------+------------------------------------------+
| 10 | 00000 | 010100 |   rs1   | 1/0 |      simm12/rs2          |
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

#define SUBCC_CYCLES    1
#define SUBCC_CODE_MASK 0x8120000
#define PRIVILEDGE  0

#define OP_OFF_first     30
#define OP_OFF_last      31
#define OP         0x2

#define OP3_OFF_first     19
#define OP3_OFF_last      24
#define OP3         0x24

#define RD_OFF_first      25
#define RD_OFF_last       29

#define RS1_OFF_first     14
#define RS1_OFF_last      18

#define RS2_OFF_first     0
#define RS2_OFF_last      4

#define I_OFF       13

#define SIMM13_OFF_first  0
#define SIMM13_OFF_last   12


sparc_instruction_t i_mulscc = {
    execute,
    disassemble,
    SUBCC_CODE_MASK,
    PRIVILEDGE,
    OP,
};



static int execute(void *state)
{
    /*
     * Sm: source operand (MSB)
     * Rm: Result operand (MSB)
     * Dm: Destination operand (MSB)
     */
    uint8 Sm, Dm, Rm;
    uint32 op1, op2;
    uint32 rrs1, rrs2;

    /*  We operate with this values so the calculation is good even when the
     *  source and destination register is the same */
    rrs1 = REG(rs1);
    rrs2 = REG(rs2);

    op1 = bits(rrs1, 31, 1);
    clear_bit(op1, 31);
    int N = psr_get_neg();
    int V = psr_get_overflow();
    if( (N ^ V) )
        set_bit(op1, 31);
    
    if( bit(YREG, 0) == 0 )
    {
        op2 = 0;
//        DBG("mulscc reg[%d], reg[%d], reg[%d]\n", rs1, rs2, rd);
        print_inst_RS_RS_RD("mulscc", rs1, rs2, rd);
    }
    else 
    {
        if( imm < 0 )
        {
            op2 = rrs2;
            print_inst_RS_RS_RD("mulscc", rs1, rs2, rd);
//            DBG("mulscc reg[%d], reg[%d], reg[%d]\n", rs1, rs2, rd);
        }
        else
        {
            op2 = sign_ext13(imm);
            print_inst_RS_IM13_RD("mulscc", rs1, sign_ext13(imm), rd);
//            DBG("mulscc reg[%d], %d, reg[%d]\n", rs1, (int)sign_ext13(imm), rd);
        }
    }

    /*  operand2 is added to operand1 and stored in RD (partial product)    */
    REG(rd) = op1 + op2;
    YREG = bits(YREG, 31, 1);
    ( bit(rrs1, 0) == 1 ) ? set_bit(YREG, 31) : clear_bit(YREG, 31);

    clear_icc();

    /*  NEGATIVE condition  */
    if( bit(REG(rd), 31) )
        psr_set_neg();

    /*  ZERO condition  */
    if( REG(rd) == 0 )
        psr_set_zero(); 

    Sm = bit( op1, 31 );
    Dm = bit( op2, 31 );
    Rm = bit( REG(rd), 31 );

    /*  OVERFLOW condition  */
    if ( (Sm && Dm && !Rm) || (!Sm && !Dm && Rm)  )
        psr_set_overflow();

    /*  CARRY condition */
    if( (Sm && Dm) || (!Rm && (Sm || Dm)) )
    {
//        DBG("Carry\n");
        psr_set_carry();
    }


    PCREG = NPCREG;
    NPCREG += 4;

    // Everyting takes some time
    return(SUBCC_CYCLES);
}

static int disassemble(uint32 instr, void *state)
{
    int op3 = 0, op;

    op = bits(instr, OP_OFF_last, OP_OFF_first);
    op3 = bits(instr, OP3_OFF_last, OP3_OFF_first);

    if( (instr & SUBCC_CODE_MASK) && (op == OP) && (op3 == OP3) )
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

