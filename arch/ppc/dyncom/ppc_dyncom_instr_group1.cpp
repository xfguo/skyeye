#include "debug.h"
#include "tracers.h"
#include "ppc_dyncom_dec.h"
#include "ppc_exc.h"
#include "ppc_cpu.h"
#include "ppc_dyncom_alu.h"
#include "ppc_dyncom_run.h"
#include "ppc_tools.h"
#include "ppc_mmu.h"

#include "llvm/Instructions.h"
#include <dyncom/dyncom_llvm.h>
#include <dyncom/frontend.h>
#include "dyncom/basicblock.h"
#include "skyeye.h"
#include "skyeye_types.h"

#include "ppc_dyncom_debug.h"
/*
 *	bcctrx		Branch Conditional to Count Register
 *	.438
 */
int opc_bcctrx_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	*tag = TAG_COND_BRANCH;
	*new_pc = NEW_PC_NONE;
	*tag |= TAG_STOP;
	return PPC_INSN_SIZE;
}
Value* opc_bcctrx_translate_cond(cpu_t *cpu, uint32_t instr, BasicBlock *bb){
	uint32 BO, BI, BD;
	PPC_OPC_TEMPL_XL(instr, BO, BI, BD);
	PPC_OPC_ASSERT(BD==0);
	PPC_OPC_ASSERT(!(BO & 2));     
	bool_t bo8 = (bool_t)((BO & 8)?1:0);
	Value* cr_value = SELECT(ICMP_NE(AND(RS(CR_REGNUM), CONST(1<<(31-BI))), CONST(0)), CONST(1), CONST(0));
	return OR(ICMP_NE(AND(CONST(BO), CONST(16)), CONST(0)), LOG_NOT(XOR(cr_value, CONST(bo8))));
}
static int opc_bcctrx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb){
	if (instr & PPC_OPC_LK) {
		LETS(LR_REGNUM, ADD(RS(PHYS_PC_REGNUM), CONST(4)));
	}
	arch_store(AND(RS(CTR_REGNUM), CONST(0xfffffffc)), cpu->ptr_PHYS_PC, bb);
}
/*
 *	bclrx		Branch Conditional to Link Register
 *	.440
 */
int opc_bclrx_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	*tag = TAG_COND_BRANCH;
	*new_pc = NEW_PC_NONE;
	*tag |= TAG_STOP;
	return PPC_INSN_SIZE;
}
Value* opc_bclrx_translate_cond(cpu_t *cpu, uint32_t instr, BasicBlock *bb){
	uint32 BO, BI, BD;
	PPC_OPC_TEMPL_XL(instr, BO, BI, BD);
	PPC_OPC_ASSERT(BD==0);
	if (!(BO & 4)) {
		LETS(CTR_REGNUM, SUB(RS(CTR_REGNUM), CONST(1)));
	}
	bool_t bo2 = ((BO & 2)?True:False);
	bool_t bo8 = ((BO & 8)?True:False);
	Value* cr_value = SELECT(ICMP_NE(AND(RS(CR_REGNUM), CONST(1<<(31-BI))), CONST(0)), CONST(1), CONST(0));
	Value * tmp1 = ICMP_NE(AND(CONST(BO), CONST(4)), CONST(0));
	Value * tmp2 = XOR(ICMP_NE(RS(CTR_REGNUM), CONST(0)), ICMP_NE(CONST(bo2), CONST(0)));
	Value * tmp3 = ICMP_NE(AND(CONST(BO), CONST(16)), CONST(0));
	Value * tmp4 = ICMP_EQ(XOR(cr_value, CONST(bo8)), CONST(0));
	return AND(OR(tmp1, tmp2), OR(tmp3, tmp4));
}
static int opc_bclrx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb){
	Value *tmp = AND(RS(LR_REGNUM), CONST(0xfffffffc));
	if (instr & PPC_OPC_LK) {
		LETS(LR_REGNUM, ADD(RS(PHYS_PC_REGNUM), CONST(4)));
	}
	arch_store(tmp, cpu->ptr_PHYS_PC, bb);
}
/*
 *	isync		Instruction Synchronize
 *	.520
 */
static int opc_isync_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb){
	// NO-OP
}
/* Interfaces */
ppc_opc_func_t ppc_opc_crnor_func;
ppc_opc_func_t ppc_opc_crandc_func;
ppc_opc_func_t ppc_opc_crxor_func;
ppc_opc_func_t ppc_opc_crnand_func;
ppc_opc_func_t ppc_opc_crand_func;
ppc_opc_func_t ppc_opc_creqv_func;
ppc_opc_func_t ppc_opc_crorc_func;
ppc_opc_func_t ppc_opc_cror_func;
ppc_opc_func_t ppc_opc_bcctrx_func = {
	opc_bcctrx_tag,
	opc_bcctrx_translate,
	opc_bcctrx_translate_cond,
};
ppc_opc_func_t ppc_opc_bclrx_func = {
	opc_bclrx_tag,
	opc_bclrx_translate,
	opc_bclrx_translate_cond,
};
ppc_opc_func_t ppc_opc_mcrf_func;
ppc_opc_func_t ppc_opc_rfi_func;
ppc_opc_func_t ppc_opc_isync_func = {
	opc_default_tag,
	opc_isync_translate,
	opc_invalid_translate_cond,
};
