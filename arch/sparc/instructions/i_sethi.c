/*
 * =====================================================================================
 *
 *       Filename:  i_sethi.c
 *
 *    Description:  Implementation of the SETHI instruction
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

31-30  29-25   24-22                21-0
+----+-------+--------+------------------------------------------+
| op |   rd  |  op2   |             imm22                        |
+----+-------+--------+------------------------------------------+

op              = 00
rd              = 0000
op2             = 100

*/
#include "../common/bits.h"
#include "../common/sparc.h"
#include "../common/traps.h"
#include "../common/iu.h"
#include "i_utils.h"

#include <stdio.h>

static int execute(void *);
static int disassemble(uint32 instr, void *state);
static int rd, imm22;

#define SETHI_CYCLES    1
#define SETHI_CODE_MASK 0x01000000
#define PRIVILEDGE  0

#define IMM22_OFF_first  0
#define IMM22_OFF_last   21

#define OP2_OFF_first     22
#define OP2_OFF_last      24
#define OP2         0x4

#define RD_OFF_first      25
#define RD_OFF_last       29

#define OP_OFF_first    30
#define OP_OFF_last     31
#define OP  0x0

sparc_instruction_t i_sethi = {
    execute,
    disassemble,
    SETHI_CODE_MASK,
    PRIVILEDGE,
    OP,
};

static int execute(void *state)
{
//    sparc_state_t *pstate = (sparc_state_t *)state;
//    int cwp = pstate->cwp;

    /*  zeroes the least significant 10 bits of r[rd], and replaces its
     *  high-order 22 bits with the value from its im22 field   */
    REG(rd) = (imm22 << 10) & 0xFFFFFC00;
//    pstate->regs[cwp].r[rd] = (imm22 << 10) & 0xFFFFFC00;


//    DBG("sethi hi(0x%x), reg[%d]\n", imm22, rd);
    print_inst_IMM_RD("sethi", imm22, rd);

    PCREG = NPCREG;
    NPCREG += 4;
//    pstate->pc = pstate->npc;
//    pstate->npc +=4;

    // Everyting takes some time
    return(SETHI_CYCLES);
}

static int disassemble(uint32 instr, void *state)
{
    int op2 = 0, op = 0;

    op = bits(instr, OP_OFF_last, OP_OFF_first);

    if( (instr & SETHI_CODE_MASK) && (op == 0x0) )
    {
        op2 = bits(instr, OP2_OFF_last, OP2_OFF_first);
        if( op2 != OP2 )
            return 0;


        rd = bits(instr, RD_OFF_last, RD_OFF_first);
        imm22 = bits(instr, IMM22_OFF_last, IMM22_OFF_first);

        return 1;
    }
    return 0;
}

