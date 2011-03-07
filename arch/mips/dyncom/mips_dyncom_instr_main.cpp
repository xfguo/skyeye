#include "llvm/Instructions.h"
#include <skyeye_dyncom.h>
#include "dyncom/dyncom_llvm.h"
#include "dyncom/frontend.h"
#include "mips_internal.h"
#include "bank_defs.h"
#include "mips_dyncom_dec.h"

#define SA	((instr >> 6) & 0x1F)
int opc_j_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RT);
	Value *op2 = R(SA);

	LET(RD, SHL(op1,op2));
	return branch_delay;
}

int opc_jal_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;
	return branch_delay;
}

int opc_beq_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *op2 = R(RT);
	Value *off = CONST(OFFSET);
	Value *value = SELECT(ICMP_EQ(op1, op2), ADD(ADD(PC, CONST(4)),SHL(off, CONST(2))), ADD(PC, CONST(8)));
	LET_PC(value);
	//check pipeline

	return branch_delay;
}

int opc_bne_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *op2 = R(RT);
	Value *off = CONST(OFFSET);

	LET_PC(SELECT(ICMP_ULE(op1, op2), ADD(ADD(PC, CONST(4)),SHL(off, CONST(2))), ADD(PC, CONST(8))));
	//check pipeline

	return branch_delay;
}

int opc_blez_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *off = CONST(OFFSET);

	LET_PC(SELECT(ICMP_ULE(op1, CONST(0)), ADD(ADD(PC, CONST(4)),SHL(off, CONST(2))), ADD(PC, CONST(8))));
	//check pipeline

	return branch_delay;
}

int opc_bgtz_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *off = CONST(OFFSET);

	LET_PC(SELECT(ICMP_UGT(op1, CONST(0)), ADD(ADD(PC, CONST(4)),SHL(off, CONST(2))), ADD(PC, CONST(8))));
	//check pipeline

	return branch_delay;
}

int opc_addi_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *op2 = IMM;
	LET(RT, ADD(op1, op2));
	//if(overflow);

	return nothing_special;
}

int opc_addiu_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *op2 = IMMU;
	LET(RT, ADD(op1, op2));

	return nothing_special;
}

int opc_slti_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *op2 = IMM;
	LET(RT, ICMP_ULT(op1, op2));

	return nothing_special;
}

int opc_sltiu_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *op2 = IMMU;
	LET(RT, ICMP_ULT(op1, op2));

	return nothing_special;
}

int opc_andi_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *op2 = IMMU;
	LET(RT, AND(op1, op2));

	return nothing_special;
}

int opc_ori_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *op2 = IMMU;
	LET(RT, OR(op1, op2));

	return nothing_special;
}

int opc_xori_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *op2 = IMMU;
	LET(RT, XOR(op1, op2));

	return nothing_special;
}

int opc_lui_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *imm = IMM;
	imm = SHL(imm, CONST(16));
	LET(RT, imm);

	return nothing_special;
}

int opc_pref_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	printf("Waring, PREF instruction not implemented \n");
	return nothing_special;
}

int opc_beql_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RT);
	Value *op2 = R(RS);
	Value *off = CONST(OFFSET);

	LET_PC(SELECT(ICMP_EQ(op1, op2), ADD(ADD(PC, CONST(4)), SHL(off, CONST(2))), ADD(PC, CONST(8))));
	//check pipeline;

	return branch_delay;
}

int opc_bnel_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RT);
	Value *op2 = R(RS);
	Value *off = CONST(OFFSET);

	LET_PC(SELECT(ICMP_NE(op1, op2), ADD(ADD(PC, CONST(4)), SHL(off, CONST(2))), ADD(PC, CONST(8))));
	//check pipeline;

	return branch_delay;
}

int opc_blezl_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RT);
	Value *op2 = R(RS);
	Value *off = CONST(OFFSET);

	LET_PC(SELECT(ICMP_ULE(op2, CONST(0)), ADD(ADD(PC, CONST(4)), SHL(off, CONST(2))), ADD(PC, CONST(8))));
	//check pipeline;

	return branch_delay;
}

int opc_bgtzl_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RT);
	Value *op2 = R(RS);
	Value *off = CONST(OFFSET);

	LET_PC(SELECT(ICMP_UGT(op2, CONST(0)), ADD(ADD(PC, CONST(4)), SHL(off, CONST(2))), ADD(PC, CONST(8))));
	//check pipeline;

	return branch_delay;
}

int opc_lb_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *va = ADD(R(BASE), SEXT16(CONST(OFFSET)));
	Value *data =SEXT32(arch_read_memory(cpu, bb, va, 1, 8));
	LET(RT, data);

	return nothing_special;
}

int opc_lh_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *va = ADD(R(BASE), SEXT16(CONST(OFFSET)));
	Value *data = SEXT16(arch_read_memory(cpu, bb, va, 1, 16));
	LET(RT, data);

	return nothing_special;
}

int opc_lwl_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;
	return nothing_special;
}

int opc_lw_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *va = ADD(R(BASE), SEXT16(CONST(OFFSET)));
	Value *data = arch_read_memory(cpu, bb, va, 1, 32);
	LET(RT, data);

	return nothing_special;
}

int opc_lbu_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *va = ADD(R(BASE), SEXT16(CONST(OFFSET)));
	Value *data = arch_read_memory(cpu, bb, va, 0, 8);
	LET(RT, data);

	return nothing_special;
}

int opc_lhu_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *va = ADD(R(BASE), SEXT16(CONST(OFFSET)));
	Value *data = arch_read_memory(cpu, bb, va, 0, 16);
	LET(RT, data);

	return nothing_special;
}

int opc_lwr_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;
	return nothing_special;
}

int opc_lwu_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *va = ADD(R(BASE), SEXT16(CONST(OFFSET)));
	Value *data = arch_read_memory(cpu, bb, va, 0, 32);
	LET(RT, data);

	return nothing_special;
}

int opc_sb_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *va = ADD(R(BASE), SEXT16(CONST(OFFSET)));
	Value *data = R(RT);
	arch_write_memory(cpu, bb, va, data, 8);
	return nothing_special;
}

int opc_sh_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *va = ADD(R(BASE), SEXT16(CONST(OFFSET)));
	Value *data = R(RT);
	arch_write_memory(cpu, bb, va, data, 16);
	return nothing_special;
}

int opc_swl_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;
	return nothing_special;
}

int opc_sw_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *va = ADD(R(BASE), SEXT16(CONST(OFFSET)));
	Value *data = R(RT);
	arch_write_memory(cpu, bb, va, data, 32);
	return nothing_special;
}

int opc_cache_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;
	return nothing_special;
}

int opc_ll_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *va = ADD(R(BASE), SEXT16(CONST(OFFSET)));
	Value *data = arch_read_memory(cpu, bb, va, 0, 32);
	LET(RT, data);

	return nothing_special;
}

int opc_lwc1_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;
	return nothing_special;
}

int opc_lld_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;
	return nothing_special;
}

int opc_sc_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *va = ADD(R(BASE), SEXT16(CONST(OFFSET)));
	Value *data = R(RT);
	arch_write_memory(cpu, bb, va, data, 32);
	LET(RT, CONST(1));

	return nothing_special;
}


