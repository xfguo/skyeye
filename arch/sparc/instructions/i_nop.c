/*
 * =====================================================================================
 *
 *       Filename:  i_nop.c
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

/*  Format (2)

31-30  29-25   24-22
+----+-------+-------+---------------------------------------------------+
| op |   rd  |  op2  |              imm22/disp22                         |
+----+-------+-------+---------------------------------------------------+

op              = 00
rd              = 0000
op2             = 100
imm22/disp22    = 0

+----+-------+-------+---------------------------------------------------+
| 00 | 0000  |  100  |           --------- 0 --------                    |
+----+-------+-------+---------------------------------------------------+

*/
#include "../common/sparc.h"
#include "../common/traps.h"
#include "../common/debug.h"
#include "../common/iu.h"
#include "i_utils.h"

#include <stdio.h>

static int execute(void *);
static int disassemble(uint32 instr, void *state);

#define NOP_CYCLES    1
#define NOP_CODE_MASK 0x1000000
#define PRIVILEDGE  0

#define OP  0x0

sparc_instruction_t i_nop = {
    execute,
    disassemble,
    NOP_CODE_MASK,
    PRIVILEDGE,
    OP,
};



static int execute(void *state)
{

    DBG("nop\n");


    PCREG = NPCREG;
    NPCREG += 4;

    // Everyting takes some time
    return NOP_CYCLES;
}

static int disassemble(uint32 instr, void *state)
{
    if( (instr ^ NOP_CODE_MASK) == 0x0 )
        return 1;
    else
        return 0;
}

