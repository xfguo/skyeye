/*
 * =====================================================================================
 *
 *       Filename:  i_bn.c
 *
 *    Description:  Implementation of the BN instruction
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

31-30  29   28-25    24-22       21-0
+----+----+--------+------------------------------------------+
| op | a  |  cond  |   010   |          disp22                |
+----+----+--------+------------------------------------------+

op              = 00
cond            = 0000

31-30  29   28-25    24-22       21-0
+----+----+--------+------------------------------------------+
| op | a  |  0000  |   010   |          disp22                |
+----+----+--------+------------------------------------------+

*/
#include "../common/bits.h"
#include "../common/sparc.h"
#include "../common/traps.h"
#include "../common/iu.h"
#include "i_utils.h"

#include <stdio.h>

static int execute(void *);
static int disassemble(uint32 instr, void *state);
static int cond, disp22, annul, op;

#define BN_CYCLES    1
#define BN_CODE_MASK 0x0080000
#define PRIVILEDGE  0

#define OP_MASK    0xc0000000
#define OP_OFF     30
#define OP         0x0

#define COND_MASK    0x01f80000
#define COND_OFF     25
#define COND         0x8

#define A_MASK      0x20000000
#define A_OFF       29
#define DISP22_MASK 0x003fffff
#define DISP22_OFF  0

sparc_instruction_t i_bn = {
    execute,
    disassemble,
    BN_CODE_MASK,
    PRIVILEDGE,
    OP,
};

static int execute(void *state)
{
    int32 addr = (disp22 << 10) >> 8;   /*  sign extend */

    if( annul )
    {
        /*  the delay slot is annulled  */
        PCREG = NPCREG + 8;
        NPCREG += 8;

//        DBG("bn,a 0x%x\n", addr);
        print_inst_BICC("bn,a", addr);

    }
    else
    {
//        DBG("bn 0x%x\n", addr);
        print_inst_BICC("bn", addr);
        PCREG = NPCREG;
        NPCREG += 4;
    }

    // Everyting takes some time
    return(BN_CYCLES);

}

static int disassemble(uint32 instr, void *state)
{

    op = (instr & OP_MASK) >> OP_OFF;
    annul = bit(instr, A_OFF);
    cond = (instr & COND_MASK) >> COND_OFF;

    if( (instr & BN_CODE_MASK) && (op == OP) && (cond == COND) )
    {
        disp22 = (instr & DISP22_MASK) >> DISP22_OFF;
        return 1;
    }
    return 0;
}

