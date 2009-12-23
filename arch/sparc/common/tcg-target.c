/*
 * =====================================================================================
 *
 *       Filename:  tcg-target.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/10/08 17:48:54
 *       Modified:  02/10/08 17:48:54
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

#include "../common/sparc.h"
#include "../common/debug.h"

static const char * const sparc_reg_names[32] = {
    "%g0",
    "%g1",
    "%g2",
    "%g3",
    "%g4",
    "%g5",
    "%g6",
    "%g7",
    "%o0",
    "%o1",
    "%o2",
    "%o3",
    "%o4",
    "%o5",
    "%o6",
    "%o7",
    "%l0",
    "%l1",
    "%l2",
    "%l3",
    "%l4",
    "%l5",
    "%l6",
    "%l7",
    "%i0",
    "%i1",
    "%i2",
    "%i3",
    "%i4",
    "%i5",
    "%i6",
    "%i7",
};

#define RNAME(x)    sparc_reg_names[x]

inline void print_inst_RS_RS_RD(char *mnemonic, int rs1, int rs2, int rd)
{
    DBG("%s %s, %s, %s\n", mnemonic, RNAME(rs1), RNAME(rs2), RNAME(rd));
}

inline void print_inst_RS_IM13_RD(char *mnemonic, int rs1, int im13, int rd)
{
    DBG("%s %s, 0x%x, %s\n", mnemonic, RNAME(rs1), im13, RNAME(rd));
}

inline void print_inst_BICC(char *mnemonic, unsigned int addr)
{
    DBG("%s 0x%x\n", mnemonic, addr);
}

void print_inst_IMM_RD(char *mnemonic, int im, int rd)
{
    DBG("%s 0x%x, %s\n", mnemonic, im, RNAME(rd));
}

void print_inst_RD_IMM(char *mnemonic, int rd, int im)
{
    DBG("%s %s, [0x%x]\n", mnemonic, RNAME(rd), im);
}

inline void print_inst_LS_RS_IM13_RD(char *mnemonic, int rs1, int im13, int rd)
{
    DBG("%s [%s + 0x%x], %s\n", mnemonic, RNAME(rs1), im13, RNAME(rd));
}

inline void print_inst_LS_RS_RS_RD(char *mnemonic, int rd, int rs1, int rs2)
{
    DBG("%s %s, [%s + %s]\n", mnemonic, RNAME(rd), RNAME(rs1), RNAME(rs2));
}

inline void print_inst_ST_RS_RS_RD(char *mnemonic, int rd,  int rs1, int rs2)
{
    DBG("%s [%s + %s], %s\n", mnemonic, RNAME(rd), RNAME(rs1), RNAME(rs2));
}

inline void print_inst_ST_RS_IM13_RD(char *mnemonic, int rd, int rs1, int im13)
{
    DBG("%s %s [%s + 0x%x]\n", mnemonic, RNAME(rd), RNAME(rs1), im13);
}

inline void print_inst_RD(char *mnemonic, int rd)
{
    DBG("%s %s\n", mnemonic, RNAME(rd));
}


