/*
 * =====================================================================================
 *
 *       Filename:  i_rett.c
 *
 *    Description:  Implementation of the RETT instruction
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
op3             = 111001

31-30  29-25   24-19     18-14    13        12-5          4-0
+----+-------+--------+------------------------------------------+
| 10 | 00000 | 111001 |   rs1   | 1/0 |      simm12/rs2          |
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

#define RETT_CYCLES    2
#define RETT_CODE_MASK 0x81c80000
#define PRIVILEDGE  1

#define OP_OFF_first     30
#define OP_OFF_last      31
#define OP         0x2

#define OP3_OFF_first     19
#define OP3_OFF_last      24
#define OP3         0x39

#define RD_OFF_first      25
#define RD_OFF_last       29

#define RS1_OFF_first     14
#define RS1_OFF_last      18

#define RS2_OFF_first     0
#define RS2_OFF_last      4

#define I_OFF       13

#define SIMM13_OFF_first  0
#define SIMM13_OFF_last   12


sparc_instruction_t i_rett = {
    execute,
    disassemble,
    RETT_CODE_MASK,
    PRIVILEDGE,
    OP,
};

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  RETT
 *  Description:  This instructions is used to return from a trap handler. Under
 *  some circumstances, it may itself cause a trap. If a RETT does not cause a
 *  trap, it:
 *  1. adds 1 to the CWP
 *  2. causes a delayed control transfer to the target address
 *  3. restores the S field of the PSR from the PS field and,
 *  4. sets the ET field of the PSR to 1
 * =====================================================================================
 */
static int execute(void *state)
{
    uint32 daddr = 0x0;
    int S, ET;

    /*  Sanity check    */
    S = bit(PSRREG, PSR_S);
    ET = bit(PSRREG, PSR_ET);

    /*  Calculate the destination address   */
    if( imm < 0 )
        daddr = REG(rs1) + REG(rs2);
    else
        daddr = REG(rs1) + (int)sign_ext13(imm);

    /*  Check whether the instructions causes itself a trap */
    if( (ET == 1) && (S == 0) )
    {
        traps->signal(PRIVILEGED_INSTR);
        return(RETT_CYCLES);
    }
    else if( (ET == 1) && (S == 1) )
    {
        traps->signal(ILLEGAL_INSTR);
        return(RETT_CYCLES);
    }
    else if( (ET == 0) )
    {
        int ncwp = (CWP + 1) % N_WINDOWS;
        int wim = WIMREG;

        if( S == 0 )
        {
            traps->signal(PRIVILEGED_INSTR);
            return(RETT_CYCLES);
        }
        else if( (1 << ncwp) & wim )
        {
            traps->signal(WUF);
            return(RETT_CYCLES);
        }
        else if(daddr & 0x00000003)
        {
            traps->signal(MEM_ADDR_NOT_ALIGNED);
            return(RETT_CYCLES);
        }

    }

//    DBG("rett 0x%x\n", daddr);
    print_inst_BICC("rett", daddr);

    /*  Add 1 to the CWP    */
    iu_add_cwp();

    /*  Cause a delayed control transfer    */
    PCREG = NPCREG;
    NPCREG = daddr;

    /*  restores the S field with the value from PS */
    int PS = bit(PSRREG, PSR_PS);
    if( PS == 1 ) set_bit(PSRREG, PSR_S);
    else clear_bit(PSRREG, PSR_S);

    /*  Enable the traps back   */
    set_bit(PSRREG, PSR_ET);

    // Everyting takes some time
    return RETT_CYCLES;

}

static int disassemble(uint32 instr, void *state)
{
    int op3 = 0, op;

    op = bits(instr, OP_OFF_last, OP_OFF_first);
    op3 = bits(instr, OP3_OFF_last, OP3_OFF_first);

    if( (instr & RETT_CODE_MASK) && (op == OP) && (op3 == OP3) )
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

