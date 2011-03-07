#include "llvm/Instructions.h"
#include <skyeye_dyncom.h>
#include "dyncom/dyncom_llvm.h"
#include "dyncom/frontend.h"
#include "mips_internal.h"
#include "bank_defs.h"
#include "mips_dyncom_dec.h"

int opc_bltz_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *off = CONST(OFFSET);
	LET_PC(SELECT(ICMP_ULT(op1, CONST(0)), ADD(ADD(PC, CONST(4)), SHL(off, CONST(2))), ADD(PC, CONST(4))));
	//check overflow

	return nothing_special;
}

int opc_bgez_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *off = CONST(OFFSET);
	LET_PC(SELECT(ICMP_UGE(op1, CONST(0)), ADD(ADD(PC, CONST(4)), SHL(off, CONST(2))), ADD(PC, CONST(4))));
	//check overflow

	return nothing_special;
}

int opc_bltzl_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *op2 = R(RT);
	Value *off = CONST(OFFSET);
	LET_PC(SELECT(ICMP_ULT(op2, CONST(0)), ADD(ADD(PC, CONST(4)), SHL(off, CONST(2))), ADD(PC, CONST(4))));
	//check overflow

	return nothing_special;
}

int opc_bgezl_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *op2 = R(RT);
	Value *off = CONST(OFFSET);
	LET_PC(SELECT(ICMP_UGE(op2, CONST(0)), ADD(ADD(PC, CONST(4)), SHL(off, CONST(2))), ADD(PC, CONST(4))));
	//check overflow

	return nothing_special;
}

int opc_tgei_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;

	return nothing_special;
}

int opc_tgeiu_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;

	return nothing_special;
}

int opc_tlti_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;

	return nothing_special;
}

int opc_tltiu_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;

	return nothing_special;
}

int opc_teqi_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;

	return nothing_special;
}

int opc_tnei_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;

	return nothing_special;
}

int opc_bltzal_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *off = CONST(OFFSET);
	LET(31, ADD(PC, CONST(8)));
	LET_PC(SELECT(ICMP_ULT(op1, CONST(0)), ADD(ADD(PC, CONST(4)), SHL(off, CONST(2))), ADD(PC, CONST(4))));
	//check overflow

	return nothing_special;
}

int opc_bgezal_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *op1 = R(RS);
	Value *off = CONST(OFFSET);
	LET(31, ADD(PC, CONST(8)));
	LET_PC(SELECT(ICMP_UGE(op1, CONST(0)), ADD(ADD(PC, CONST(4)), SHL(off, CONST(2))), ADD(PC, CONST(4))));
	//check overflow

	return nothing_special;
}

int opc_bltzall_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;
	return nothing_special;
}

int opc_bgezall_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;
	return nothing_special;
}

