#include "llvm/Instructions.h"
#include <skyeye_dyncom.h>
#include "dyncom/dyncom_llvm.h"
#include "dyncom/frontend.h"
#include "mips_internal.h"
#include "bank_defs.h"
#include "mips_dyncom_dec.h"

#define SA	((instr >> 6) & 0x1F)
int opc_sll_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	LET(RD, SHL(R(RT),R(SA)));
	return nothing_special;
}

int opc_srl_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	LET(RD, LSHR(R(RT), R(SA)));
	return nothing_special;
}

int opc_sra_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	LET(RD, SEXT32((LSHR(R(RT), R(SA)))));
	return nothing_special;
}

int opc_sllv_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	LET(RD, SHL(R(RT),R(SA)));
	return nothing_special;
}

int opc_srlv_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	LET(RD, LSHR(R(RT), R(SA)));
	return nothing_special;
}

int opc_srav_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	LET(RD, SEXT32(LSHR(R(RT), R(SA))));
	return nothing_special;
}

int opc_movz_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	LET(RD, SELECT(R(RT), R(RD), R(RS)));
	return nothing_special;
}

int opc_movn_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	LET(RD, SELECT(R(RT), R(RS), R(RD)));
	return nothing_special;
}

int opc_jr_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	LET_PC(R(RS));
	//if(pipeline == branch_delay)
	return branch_delay;
}

int opc_jalr_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	LET(RD, ADD(PC, CONST(8)));
	LET_PC(R(RS));
	//if(pipeline == branch_delay)
	return branch_delay;
}

int opc_syscall_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;
	return nothing_special;
}

int opc_break_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	fprintf(stderr, "workaround for break instruction\n");
	return nothing_special;
}

int opc_sync_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;
	return nothing_special;
}

int opc_mfhi_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	LET(RD, R(HI));
	return nothing_special;
}

int opc_mthi_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	LET(RD, R(HI));
	return nothing_special;
}

int opc_mflo_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	LET(RD, R(LO));
	return nothing_special;
}

int opc_mtlo_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	LET(RD, R(LO));
	return nothing_special;
}

int opc_mult_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;
	return nothing_special;
}

int opc_multu_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;
	return nothing_special;
}

int opc_div_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;
	return nothing_special;
}

int opc_divu_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;
	return nothing_special;
}

int opc_add_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *op2 = R(RT);
	LET(RD, ADD(op1, op2));
	//if(overflow)

	return nothing_special;
}

int opc_sub_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *op2 = R(RT);
	LET(RD, SUB(op1, op2));

	//if(overflow)
	return nothing_special;
}

int opc_subu_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *op2 = R(RT);
	LET(RD, SUB(op1, op2));

	return nothing_special;
}

int opc_and_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *op2 = R(RT);
	LET(RD, AND(op1, op2));

	return nothing_special;
}

int opc_or_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *op2 = R(RT);
	LET(RD, OR(op1, op2));

	return nothing_special;
}

int opc_xor_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *op2 = R(RT);
	LET(RD, XOR(op1, op2));

	return nothing_special;
}

int opc_nor_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *op2 = R(RT);
	LET(RD, NOT(OR(op1, op2)));

	return nothing_special;
}

int opc_slt_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *op2 = R(RT);

	LET(RD, ICMP_ULT(op1, op2));
	return nothing_special;
}

int opc_tge_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *op2 = R(RT);

	BAD;
}

int opc_tgeu_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;
}

int opc_tlt_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;
}

int opc_tltu_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;
}

int opc_teq_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;
}

int opc_tne_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;
}


