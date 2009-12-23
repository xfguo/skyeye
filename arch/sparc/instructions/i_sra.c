/*
 * =====================================================================================
 *
 *       Filename:  i_srl.c
 *
 *    Description:  Implementation of the arithmetic right shift.
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

31-30  29-25   24-19     18-14    13        12-5          4-0
+----+-------+--------+------------------------------------------+
| op |   rd  |  op3   |   rs1   | i=1 |   non-used   |  shcnt    |
+----+-------+--------+------------------------------------------+

op              = 00
rd              = 0000
op3             = 100111

31-30  29-25   24-19     18-14    13        12-5          4-0
+----+-------+--------+------------------------------------------+
| 10 | 00000 | 100111 |   rs1   |  i  |   non-used   | rs2/shcnt |
+----+-------+--------+------------------------------------------+

*/
#include "../common/bits.h"
#include "../common/sparc.h"
#include "../common/traps.h"
#include "../common/iu.h"

#include <stdio.h>

static int execute(void *);
static int disassemble(uint32 instr, void *state);
static int rd, rs1, rs2;
char imm;

#define SRA_CYCLES    1
#define SRA_CODE_MASK   0x81380000 
#define PRIVILEDGE  0

#define OP_OFF_first     30
#define OP_OFF_last      31
#define OP         0x2

#define OP3_OFF_first     19
#define OP3_OFF_last      24
#define OP3         0x27

#define RD_OFF_first      25
#define RD_OFF_last       29

#define RS1_OFF_first     14
#define RS1_OFF_last      18

#define RS2_OFF_first     0
#define RS2_OFF_last      4

#define I_OFF       13

#define SHCNT_OFF_last    4
#define SHCNT_OFF_first   0

sparc_instruction_t i_sra = {
    execute,
    disassemble,
    SRA_CODE_MASK,
    PRIVILEDGE,
    OP,
};


static uint32 sign_extend_uint32(uint32 x, int n)
{
	if (((uint32)-1 >> 1) < 0) {
		// Host platform does arithmetic right shifts.
		uint32 y = x;
		return y << (8 * sizeof(uint32) - n) >> (8 * sizeof(uint32) - n);
    	} else if (n < 8 * sizeof(uint32)) {
		// We have to manually patch the high-order bits.
		if (bit(x, (n - 1)))
	    		return set_bits(x, (8 * sizeof(uint32) - 1), n);
		else
	    		return clear_bits(x, (8 * sizeof(uint32) - 1), n);

    	}

    return 0;
}

static int execute(void *state)
{
    int s = 0;

    if( imm < 0 )
    {
        REG(rd) = REG(rs1) >> REG(rs2);
        s = REG(rs2);
        print_inst_RS_RS_RD("sra", rs1, rs2, rd);
    }
    else
    {
        REG(rd) = REG(rs1) >> imm;
        s = imm;
        print_inst_RS_IM13_RD("sra", rs1, imm, rd);
    }

    /*  The host platform does logical shift, so we need to manually patch the
     *  higer bits  */
    uint32 x = REG(rd);
    REG(rd) = sign_extend_uint32(x, 32 - s);
//    uint32 x = pstate->regs[cwp].r[rd];
//    pstate->regs[cwp].r[rd] = sign_extend_uint32(x, 32 - s);


    PCREG = NPCREG;
    NPCREG += 4;

    // Everyting takes some time
    return(SRA_CYCLES);
}

static int disassemble(uint32 instr, void *state)
{
    int op3 = 0, op;

    op = bits(instr, OP_OFF_last, OP_OFF_first);
    op3 = bits(instr, OP3_OFF_last, OP3_OFF_first);

    if( (instr & SRA_CODE_MASK) && (op == OP) && (op3 == OP3) )
    {
        rd = bits(instr, RD_OFF_last, RD_OFF_first);
        rs1 = bits(instr, RS1_OFF_last, RS1_OFF_first);
        int i = bit(instr, I_OFF);

        if( i )
        {
            imm = bits(instr, SHCNT_OFF_last, SHCNT_OFF_first);
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

