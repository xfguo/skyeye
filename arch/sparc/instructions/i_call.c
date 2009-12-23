/*
 * =====================================================================================
 *
 *       Filename:  i_call.c
 *
 *    Description:  Implementation of the CALL instruction
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

31-30                         29-0
+----+-----------------------------------------------------------------+
| op |                    disp30                                       |
+----+-----------------------------------------------------------------+

op              = 01
disp30

31-30                         29-0
+----+-----------------------------------------------------------------+
| 01 |                         ----                                    |
+----+-----------------------------------------------------------------+

*/
#include "../common/bits.h"
#include "../common/sparc.h"
#include "../common/traps.h"
#include "../common/iu.h"

#include <stdio.h>

static int execute(void *);
static int disassemble(uint32 instr, void *state);

#define CALL_CYCLES    1
#define CALL_CODE_MASK 0x1000000
#define PRIVILEDGE  0

#define OP_OFF_first    30
#define OP_OFF_last     31
#define OP              0x01

#define DISP30_OFF_first    0
#define DISP30_OFF_last     29


static int op = 0, disp30 = 0;

sparc_instruction_t i_call = {
    execute,
    disassemble,
    CALL_CODE_MASK,
    PRIVILEDGE,
    OP,
};



static int execute(void *state)
{
    uint32 addr = (disp30 << 2);

    REG(O7) = PCREG;
    addr += PCREG;

    /*  We have to take into account the delay slot */
    PCREG = NPCREG;
    NPCREG = addr;

//    DBG("call 0x%x\n", addr);
    print_inst_BICC("call", addr);


    // Everyting takes some time
    return CALL_CYCLES;
}

static int disassemble(uint32 instr, void *state)
{
    op = bits(instr, OP_OFF_last, OP_OFF_first);
    disp30 = bits(instr, DISP30_OFF_last, DISP30_OFF_first);

    if( op == OP )
        return 1;
    else 
        return 0;
}

