/*
 * =====================================================================================
 *
 *       Filename:  i_rdtbr.c
 *
 *    Description:  Implementst the RDTBR SPARC instruction
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

op              = 10
rd              = 0000
op3             = 101011

31-30  29-25   24-19     18-14    13        12-5          4-0
+----+-------+--------+------------------------------------------+
| 10 | 00000 | 101011 |   rs1   | i=0 |   non-used   |    rs2    |
+----+-------+--------+------------------------------------------+

*/
#include "../common/bits.h"
#include "../common/sparc.h"
#include "../common/traps.h"
#include "../common/iu.h"

#include <stdio.h>

static int execute(void *);
static int disassemble(uint32 instr, void *state);
static int rd;

#define RDTBR_CYCLES    1
#define RDTBR_CODE_MASK 0x81580000
#define PRIVILEDGE  1

#define OP_MASK    0xc0000000
#define OP_OFF     30
#define OP         0x2

#define OP3_MASK    0x01f80000
#define OP3_OFF     19
#define OP3         0x2B

#define RD_MASK     0x3e000000
#define RD_OFF      25
#define RS1_MASK    0x0007c000
#define RS1_OFF     14
#define RS2_MASK    0x0000001f
#define RS2_OFF     0
#define I_MASK      0x00002000
#define I_OFF       13
#define SIMM13_MASK 0x00001fff
#define SIMM13_OFF  0

sparc_instruction_t i_rdtbr = {
    execute,
    disassemble,
    RDTBR_CODE_MASK,
    PRIVILEDGE,
    OP,
};



static int execute(void *state)
{
    uint32 value;

    if( bit(PSRREG, PSR_S) != PRIVILEDGE )
    {
        traps->signal(PRIVILEGED_INSTR);
        return(RDTBR_CYCLES);
    }

    value = TBRREG;
    REG(rd) = value;
//    DBG("rd tbr, reg[%d]\n", rd);
    print_inst_RD("rd %tbr", rd);

    PCREG = NPCREG;
    NPCREG += 4;

    // Everyting takes some time
    return RDTBR_CYCLES;
}

static int disassemble(uint32 instr, void *state)
{
    int op3 = 0, op;

    op = (instr & OP_MASK) >> OP_OFF;

    if( (instr & RDTBR_CODE_MASK) && (op == OP) )
    {
        op3 =  (instr & OP3_MASK) >> OP3_OFF;
        if( op3 != OP3 )
            return 0;

        rd = (instr & RD_MASK) >> RD_OFF;

        return 1;
    }
    return 0;
}

