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
#include "llvm/Intrinsics.h"
#include <dyncom/dyncom_llvm.h>
#include <dyncom/frontend.h>
#include "dyncom/basicblock.h"
#include "skyeye.h"

#include "ppc_dyncom_debug.h"
#include "ppc_dyncom_run.h"

#define NOT_TESTED() do { debug(DEBUG_NOT_TEST, "INSTRUCTION NOT TESTED:%s", __FUNCTION__); } while(0)
/*
 *	cmp		Compare
 *	.442
 */
static uint32 ppc_cmp_and_mask[8] = {
	0xfffffff0,
	0xffffff0f,
	0xfffff0ff,
	0xffff0fff,
	0xfff0ffff,
	0xff0fffff,
	0xf0ffffff,
	0x0fffffff,
};

static int opc_cmp_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	NOT_TESTED();
	uint32 cr;
	int rA, rB;
	PPC_OPC_TEMPL_X(instr, cr, rA, rB);
	cr >>= 2;
	cr = 7 - cr;
	Value * tmp1 = ICMP_SLT(R(rA), R(rB));
	Value * tmp2 = ICMP_SGT(R(rA), R(rB));
	Value * c = SELECT(tmp1, CONST(8),SELECT(tmp2, CONST(4), CONST(2)));
	c = SELECT(ICMP_NE(AND(RS(XER_REGNUM), CONST(XER_SO)), CONST(0)), OR(c, CONST(1)), c);
	LETS(CR_REGNUM, AND(RS(CR_REGNUM), CONST(ppc_cmp_and_mask[cr])));
	LETS(CR_REGNUM, OR(RS(CR_REGNUM), SHL(c, CONST(cr * 4))));
	return 0;
}

/*
 *	orx		OR
 *	.603
 */
static int opc_orx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	LET(rA, OR(R(rS), R(rB)));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rA);
	}
	return 0;
}
/*
 *	addx		Add
 *	.422
 */
int opc_addx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rD, rA, rB;
	PPC_OPC_TEMPL_XO(instr, rD, rA, rB);
	LET(rD, ADD(R(rA), R(rB)));
	if (instr & PPC_OPC_Rc) {
		ppc_dyncom_update_cr0(cpu, bb, rD);
	}
	return 0;
}
/*
 *	lwzx		Load Word and Zero Indexed
 *	.560
 */
int opc_lwzx_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	*tag = TAG_CONTINUE;
	*tag |= TAG_EXCEPTION;
	*next_pc = phys_pc + PPC_INSN_SIZE;
	*new_pc = NEW_PC_NONE;
	return PPC_INSN_SIZE;
}
static int opc_lwzx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rA, rD, rB;
	PPC_OPC_TEMPL_X(instr, rD, rA, rB);
	Value* addr = rA ? ADD(R(rB), R(rA)) : R(rB);
	Value* result = arch_read_memory(cpu, bb, addr, 0, 32);
	/**
	 * If occur exception, rD should not to be written.
	 **/
	if(is_user_mode(cpu)){
		LET(rD, result);
	}else{
		Value *current_pc = RS(PHYS_PC_REGNUM);
		Value *exc_occur = ICMP_EQ(current_pc, CONST(PPC_EXC_DSI_ADDR));
		LET(rD, SELECT(exc_occur, R(rD), result));
	}
	return 0;
}
/*
 *	mtspr		Move to Special-Purpose Register
 *	.584
 */
#define dyncom_BATU_BL(v)	LSHR(AND(v, CONST(0x1ffc)), CONST(2))
static int opc_mtspr_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rS, spr1, spr2;
	PPC_OPC_TEMPL_X(instr, rS, spr1, spr2);
	//e500_core_t* current_core = get_current_core();
//	printf("In %s, spr2 = %d, spr1 = %d, rS = %d\n", __func__, spr2, spr1, rS);
	switch (spr2) {
	case 0:
		switch (spr1) {
			case 1: LETS(XER_REGNUM, R(rS)); return 0;
			case 8:	LETS(LR_REGNUM, R(rS)); return 0;
			case 9:	LETS(CTR_REGNUM,R(rS)); return 0;
			case 22: {
				Value *rS_v = R(rS);
				LETS(DEC_REGNUM, rS_v);
				LETS(PDEC_REGNUM, MUL(rS_v, CONST(TB_TO_PTB_FACTOR)));
				return 0;
			}
			case 25: arch_ppc_dyncom_mmu_set_sdr1(cpu, bb, rS); return 0;
			case 26: LETS(SRR_REGNUM, R(rS)); return 0;
			case 27: LETS(SRR_REGNUM + 1, R(rS)); return 0;
			default:fprintf(stderr, "spr2=0x%x,spr1=0x%x,no such spr\n", spr2,spr1);break;
		}
		break;
	case 1:
		printf("NOT IMPLEMENT... IN %s, line %d\n", __func__, __LINE__);
		switch (spr1) {//TODO...
		#if 0
			case 16:current_core->mmu.pid[0] = current_core->gpr[rS];return 0;
			case 26:current_core->csrr[0] = current_core->gpr[rS];return 0;
			case 27:current_core->csrr[1] = current_core->gpr[rS];return 0;
			case 29:current_core->dear = current_core->gpr[rS];return 0;
			case 30:current_core->esr = current_core->gpr[rS];return 0;
			case 31:current_core->ivpr = current_core->gpr[rS];return 0;
		#endif
			default:fprintf(stderr, "spr2=0x%x,spr1=0x%x,,no such spr\n", spr2,spr1);break;
		}
	case 8:
		switch (spr1) {
			case 0:	LETS(VRSAVE_REGNUM, R(rS)); return 0;
			case 16: LETS(SPRG_REGNUM, R(rS)); return 0;
			case 17: LETS(SPRG_REGNUM + 1, R(rS)); return 0;
			case 18: LETS(SPRG_REGNUM + 2, R(rS)); return 0;
			case 19: LETS(SPRG_REGNUM + 3, R(rS)); return 0;
			case 20: LETS(SPRG_REGNUM + 4, R(rS)); return 0;
			case 21: LETS(SPRG_REGNUM + 5, R(rS)); return 0;
			case 22: LETS(SPRG_REGNUM + 6, R(rS)); return 0;
			case 23: LETS(SPRG_REGNUM + 7, R(rS)); return 0;
			case 28: LETS(TBL_REGNUM, R(rS)); return 0;
			case 29: LETS(TBU_REGNUM, R(rS)); return 0;
			default:fprintf(stderr, "spr2=0x%x,spr1=0x%x,\n", spr2,spr1);break;
		}
		break;
	case 9:
		switch (spr1) {
			case 16:LETS(DBSR_REGNUM, R(rS)); return 0;
			case 20:LETS(DBCR_REGNUM, R(rS)); return 0;
			case 21:LETS(DBCR_REGNUM + 1, R(rS)); return 0;
			case 22:LETS(DBCR_REGNUM + 2, R(rS)); return 0;
			case 28:LETS(DAC_REGNUM, R(rS)); return 0;
			case 29:LETS(DAC_REGNUM + 1, R(rS)); return 0;
			default:fprintf(stderr, "spr2=0x%x,spr1=0x%x,,no such spr\n", spr2,spr1);break;
		}
		break;
	case 10:
		printf("NOT IMPLEMENT... IN %s, line %d\n", __func__, __LINE__);
		switch (spr1){
			case 16:
			#if 0
				/* W1C, write one to clear */
				current_core->tsr &= ~(current_core->tsr & current_core->gpr[rS]) ;
				return 0;
			case 20:current_core->tcr = current_core->gpr[rS];return 0;
			#endif
			default:fprintf(stderr, "spr2=0x%x,spr1=0x%x,no such spr\n", spr2,spr1);break;
		}
		break;
	case 12:
		printf("NOT IMPLEMENT... IN %s, line %d\n", __func__, __LINE__);
		if(spr1 >= 16 && spr1 < 32){
			//current_core->ivor[spr1 - 16] = current_core->gpr[rS];
			return 0;
		}
		switch (spr1){
			default:fprintf(stderr, "spr2=0x%x,spr1=0x%x,no such spr\n", spr2,spr1);break;
		}
		break;
	case 16:
		switch (spr1) {
		case 0: 
			LETS(SPEFSCR_REGNUM, R(rS));
			return 0;
		case 16:
			LETS(IBATU_REGNUM, R(rS));
			LETS(IBAT_BL17_REGNUM, XOR((SHL(dyncom_BATU_BL(RS(IBATU_REGNUM)), CONST(17))), CONST(-1)));
			return 0;
		case 17:
			LETS(IBATL_REGNUM, R(rS));
			return 0;
		case 18:
			LETS(IBATU_REGNUM + 1, R(rS));
			LETS(IBAT_BL17_REGNUM + 1, XOR((SHL(dyncom_BATU_BL(RS(IBATU_REGNUM + 1)), CONST(17))), CONST(-1)));
			return 0;
		case 19:
			LETS(IBATL_REGNUM + 1, R(rS));
			return 0;
		case 20:
			LETS(IBATU_REGNUM + 2, R(rS));
			LETS(IBAT_BL17_REGNUM + 2, XOR((SHL(dyncom_BATU_BL(RS(IBATU_REGNUM + 2)), CONST(17))), CONST(-1)));
			return 0;
		case 21:
			LETS(IBATL_REGNUM + 2, R(rS));
			return 0;
		case 22:
			LETS(IBATU_REGNUM + 3, R(rS));
			LETS(IBAT_BL17_REGNUM + 3, XOR((SHL(dyncom_BATU_BL(RS(IBATU_REGNUM + 3)), CONST(17))), CONST(-1)));
			return 0;
		case 23:
			LETS(IBATL_REGNUM + 3, R(rS));
			return 0;
		case 24:
			LETS(DBATU_REGNUM, R(rS));
			LETS(DBAT_BL17_REGNUM, XOR((SHL(dyncom_BATU_BL(RS(DBATU_REGNUM)), CONST(17))), CONST(-1)));
			return 0;
		case 25:
			LETS(DBATL_REGNUM, R(rS));
			return 0;
		case 26:
			LETS(DBATU_REGNUM + 1, R(rS));
			LETS(DBAT_BL17_REGNUM + 1, XOR((SHL(dyncom_BATU_BL(RS(DBATU_REGNUM + 1)), CONST(17))), CONST(-1)));
			return 0;
		case 27:
			LETS(DBATL_REGNUM + 1, R(rS));
			return 0;
		case 28:
			LETS(DBATU_REGNUM + 2, R(rS));
			LETS(DBAT_BL17_REGNUM + 2, XOR((SHL(dyncom_BATU_BL(RS(DBATU_REGNUM + 2)), CONST(17))), CONST(-1)));
			return 0;
		case 29:
			LETS(DBATL_REGNUM + 2, R(rS));
			return 0;
		case 30:
			LETS(DBATU_REGNUM + 3, R(rS));
			LETS(DBAT_BL17_REGNUM + 3, XOR((SHL(dyncom_BATU_BL(RS(DBATU_REGNUM + 3)), CONST(17))), CONST(-1)));
			return 0;
		case 31:
			LETS(DBATL_REGNUM + 3, R(rS));
			return 0;
		default:fprintf(stderr, "spr2=0x%x,spr1=0x%x,,no such spr\n", spr2,spr1);break;
		}
		break;
	case 17:
		switch(spr1){
		case 16://LCH
			LETS(E600_IBATU_REGNUM, R(rS));
			return 0;
		case 17://LCH
			LETS(E600_IBATL_REGNUM, R(rS));
			return 0;
		case 18://LCH
			LETS(E600_IBATU_REGNUM + 1, R(rS));
			return 0;
		case 19://LCH
			LETS(E600_IBATL_REGNUM + 1, R(rS));
			return 0;
		case 20://LCH
			LETS(E600_IBATU_REGNUM + 2, R(rS));
			return 0;
		case 21://LCH
			LETS(E600_IBATL_REGNUM + 2, R(rS));
			return 0;
		case 22://LCH
			LETS(E600_IBATU_REGNUM + 3, R(rS));
			return 0;
		case 23://LCH
			LETS(E600_IBATL_REGNUM + 3, R(rS));
			return 0;
		case 24://LCH
			LETS(E600_DBATU_REGNUM, R(rS));
			return 0;
		case 25://LCH
			LETS(E600_DBATL_REGNUM, R(rS));
			return 0;
		case 26://LCH
			LETS(E600_DBATU_REGNUM + 1, R(rS));
			return 0;
		case 27://LCH
			LETS(E600_DBATL_REGNUM + 1, R(rS));
			return 0;
		case 28://LCH
			LETS(E600_DBATU_REGNUM + 2, R(rS));
			return 0;
		case 29://LCH
			LETS(E600_DBATL_REGNUM + 2, R(rS));
			return 0;
		case 30://LCH
			LETS(E600_DBATU_REGNUM + 3, R(rS));
			return 0;
		case 31://LCH
			LETS(E600_DBATL_REGNUM + 3, R(rS));
			return 0;
		default:fprintf(stderr, "spr2=0x%x,spr1=0x%x,no such spr\n", spr2,spr1);break;
		}
		break;
	case 19:
		printf("NOT IMPLEMENT... IN %s, line %d\n", __func__, __LINE__);
		switch(spr1){
		#if 0
			case 16:
				current_core->mmu.mas[0] = current_core->gpr[rS];
				return 0;
			case 17:
				current_core->mmu.mas[1] = current_core->gpr[rS];
				return 0;
			case 18:
				current_core->mmu.mas[2] = current_core->gpr[rS];
				return 0;
			case 19:
				current_core->mmu.mas[3] = current_core->gpr[rS];
				return 0;
			case 20:
				current_core->mmu.mas[4] = current_core->gpr[rS];
				return 0;
			case 22:
				current_core->mmu.mas[6] = current_core->gpr[rS];
				return 0;
			case 25:
				current_core->mmu.pid[1] = current_core->gpr[rS];
				//printf("write pid 1 0x%x\n", current_core->gpr[rS]);
				return 0;
			case 26:
				current_core->mmu.pid[2] = current_core->gpr[rS];
				//printf("write pid 2 0x%x\n", current_core->gpr[rS]);
				return 0;
		#endif
			default:fprintf(stderr, "spr2=0x%x,spr1=0x%x,no such spr\n", spr2,spr1);break;
		}
		break;
	case 29:
		switch(spr1) {
			case 17: return 0;
			case 24: return 0;
			case 25: return 0;
			case 26: return 0;
		}
	case 30://LCH
		printf("NOT IMPLEMENT... IN %s, line %d\n", __func__, __LINE__);
		switch(spr1) {
		#if 0
		case 20: 
			current_core->e600_tlbmiss = current_core->gpr[rS];
			return 0;
		case 21: 
			current_core->e600_pte[0] = current_core->gpr[rS];
			return 0;
		case 22: 
			current_core->e600_pte[1] = current_core->gpr[rS];
			return 0;
		#endif
		}
		return 0;
	case 31:
		switch (spr1) {
		case 16:
			LETS(HID_REGNUM, R(rS));
			return 0;
		case 17: return 0;
		case 18:
			PPC_OPC_ERR("write(%08x) to spr %d:%d (IABR) not supported!\n", current_core->gpr[rS], spr1, spr2);
			return 0;
		case 19:
			LETS(L1CSR_REGNUM + 1, R(rS));return 0;
		case 20:
			LETS(IAC_REGNUM, R(rS));return 0;
		case 21:
			PPC_OPC_ERR("write(%08x) to spr %d:%d (DABR) not supported!\n", current_core->gpr[rS], spr1, spr2);
			return 0;
		case 22:
			LETS(E600_MSSCR0_REGNUM, R(rS));
			return 0;
		case 23:
			LETS(E600_MSSSR0_REGNUM, R(rS));
			return 0;
		case 24:
			LETS(E600_LDSTCR_REGNUM, R(rS));
			return 0;	
		case 27:
		case 28:
		case 29:
		case 30:
		case 31: return 0;
		default:fprintf(stderr, "spr2=0x%x,spr1=0x%x,no such spr\n", spr2,spr1);break;
		}
	}
	fprintf(stderr, "unknown mtspr: %i:%i\n", spr1, spr2);
	//fprintf(stderr, "pc=0x%x\n",current_core->pc);
	skyeye_exit(-1);
}
/*
 *	mfspr		Move from Special-Purpose Register
 *	.567
 */
static int opc_mfspr_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	//e500_core_t* current_core = get_current_core();
	int rD, spr1, spr2;
	PPC_OPC_TEMPL_XO(instr, rD, spr1, spr2);
	#if 0
	if (current_core->msr & MSR_PR) {
		//ppc_exception(current_core, PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV, 0);
		if(!(spr2 == 0 && spr1 == 8)) /* read lr*/
			printf("Warning, execute mfspr in user mode, pc=0x%x\n", current_core->pc);
		//return;
	}
	#endif
	debug(DEBUG_TRANSLATE, "In %s, spr2=%d, spr1=%d\n", __func__, spr2, spr1);
//	printf("In %s, spr2=%d, spr1=%d\n", __func__, spr2, spr1);
	switch(spr2) {
	case 0:
		switch (spr1) {
		case 1: LET(rD, RS(XER_REGNUM)); return 0;
		case 8: LET(rD, RS(LR_REGNUM)); return 0;
		case 9: LET(rD, RS(CTR_REGNUM)); return 0;
		case 18: LET(rD, RS(DSISR_REGNUM)); return 0;
		case 19: LET(rD, RS(DAR_REGNUM)); return 0;
		case 22: {
			LETS(DEC_REGNUM, UDIV(RS(PDEC_REGNUM), CONST(TB_TO_PTB_FACTOR)));
			LET(rD, RS(DEC_REGNUM));
			return 0;
		}
		case 25: LET(rD, RS(SDR1_REGNUM)); return 0;
		case 26: LET(rD, RS(SRR_REGNUM)); return 0;
		case 27: LET(rD, RS(SRR_REGNUM + 1)); return 0;
		}
		break;
	case 1:
		switch(spr1) {
			case 16:
				printf("NOT IMPLEMENT... IN %s, line %d\n", __func__, __LINE__);
				//current_core->gpr[rD] = current_core->mmu.pid[0];
				return 0;
			case 29: LET(rD, RS(DEAR_REGNUM)); return 0;
			case 30: LET(rD, RS(ESR_REGNUM)); return 0;
			default:fprintf(stderr, "spr2=0x%x,spr1=0x%x,no such spr\n", spr2,spr1);break;
		}
		break;
	case 8:
		switch (spr1) {
		case 12: LET(rD, RS(TBL_REGNUM)); return 0;
		case 13: LET(rD, RS(TBU_REGNUM)); return 0;
		case 0: LET(rD, RS(VRSAVE_REGNUM)); return 0;
		case 16: LET(rD, RS(SPRG_REGNUM)); return 0;
		case 1:
		case 17: LET(rD, RS(SPRG_REGNUM + 1)); return 0;
		case 2:
		case 18: LET(rD, RS(SPRG_REGNUM + 2)); return 0;
		case 3:
		case 19: LET(rD, RS(SPRG_REGNUM + 3)); return 0;
		case 4:
		case 20: LET(rD, RS(SPRG_REGNUM + 4)); return 0;
		case 5:
		case 21: LET(rD, RS(SPRG_REGNUM + 5)); return 0;
		case 6:
		case 22: LET(rD, RS(SPRG_REGNUM + 6)); return 0;
		case 7:
		case 23: LET(rD, RS(SPRG_REGNUM + 7)); return 0;
		case 26: LET(rD, RS(EAR_REGNUM)); return 0;
		case 30: LET(rD, RS(PIR_REGNUM)); return 0;
		case 31: LET(rD, RS(PVR_REGNUM)); return 0;
		default:
			fprintf(stderr, "[warning:mfspr]line = %d, instr = 0x%x, spr1:spr2 = %i:%i\n",
					__LINE__, instr, spr1, spr2);
		}
		break;
	case 9:
		switch(spr1) {
			case 16:LET(rD, RS(DBSR_REGNUM)); return 0;
			case 20:LET(rD, RS(DBCR_REGNUM)); return 0;
			case 21:LET(rD, RS(DBCR_REGNUM + 1)); return 0;
			case 22:LET(rD, RS(DBCR_REGNUM + 2)); return 0;
			case 28:LET(rD, RS(DAC_REGNUM)); return 0;
			case 29:LET(rD, RS(DAC_REGNUM + 1)); return 0;
		}
		break;
	case 10:
		switch(spr1){
			case 20:LET(rD, RS(TCR_REGNUM)); return 0;
			default:break;
		}
		break;
	case 16:
		switch (spr1) {
		case 0: LET(rD, RS(SPEFSCR_REGNUM)); return 0;
		case 16: LET(rD, RS(IBATU_REGNUM)); return 0;
		case 17: LET(rD, RS(IBATL_REGNUM)); return 0;
		case 18: LET(rD, RS(IBATU_REGNUM + 1)); return 0;
		case 19: LET(rD, RS(IBATL_REGNUM + 1)); return 0;
		case 20: LET(rD, RS(IBATU_REGNUM + 2)); return 0;
		case 21: LET(rD, RS(IBATL_REGNUM + 2)); return 0;
		case 22: LET(rD, RS(IBATU_REGNUM + 3)); return 0;
		case 23: LET(rD, RS(IBATL_REGNUM + 3)); return 0;
		case 24: LET(rD, RS(DBATU_REGNUM)); return 0;
		case 25: LET(rD, RS(DBATL_REGNUM)); return 0;
		case 26: LET(rD, RS(DBATU_REGNUM + 1)); return 0;
		case 27: LET(rD, RS(DBATL_REGNUM + 1)); return 0;
		case 28: LET(rD, RS(DBATU_REGNUM + 2)); return 0;
		case 29: LET(rD, RS(DBATL_REGNUM + 2)); return 0;
		case 30: LET(rD, RS(DBATU_REGNUM + 3)); return 0;
		case 31: LET(rD, RS(DBATL_REGNUM + 3)); return 0;
		}
		break;
	case 17://LCH
		printf("NOT IMPLEMENT... IN %s, line %d\n", __func__, __LINE__);
		switch (spr1) {
		#if 0
		case 16: current_core->gpr[rD] = current_core->e600_ibatu[0]; return 0;
		case 17: current_core->gpr[rD] = current_core->e600_ibatl[0]; return 0;
		case 18: current_core->gpr[rD] = current_core->e600_ibatu[1]; return 0;
		case 19: current_core->gpr[rD] = current_core->e600_ibatl[1]; return 0;
		case 20: current_core->gpr[rD] = current_core->e600_ibatu[2]; return 0;
		case 21: current_core->gpr[rD] = current_core->e600_ibatl[2]; return 0;
		case 22: current_core->gpr[rD] = current_core->e600_ibatu[3]; return 0;
		case 23: current_core->gpr[rD] = current_core->e600_ibatl[3]; return 0;
		case 24: current_core->gpr[rD] = current_core->e600_dbatu[0]; return 0;
		case 25: current_core->gpr[rD] = current_core->e600_dbatl[0]; return 0;
		case 26: current_core->gpr[rD] = current_core->e600_dbatu[1]; return 0;
		case 27: current_core->gpr[rD] = current_core->e600_dbatl[1]; return 0;
		case 28: current_core->gpr[rD] = current_core->e600_dbatu[2]; return 0;
		case 29: current_core->gpr[rD] = current_core->e600_dbatl[2]; return 0;
		case 30: current_core->gpr[rD] = current_core->e600_dbatu[3]; return 0;
		case 31: current_core->gpr[rD] = current_core->e600_dbatl[3]; return 0;
		#endif
		}
		break;
	case 19:
		printf("NOT IMPLEMENT... IN %s, line %d\n", __func__, __LINE__);
                switch(spr1) {
		#if 0
			case 16:
                                current_core->gpr[rD] = current_core->mmu.mas[0];
                                return 0;
                        case 17:
                                current_core->gpr[rD] = current_core->mmu.mas[1];
                                return 0;
                        case 18:
                                current_core->gpr[rD] = current_core->mmu.mas[2];
                                return 0;
                        case 19:
                                current_core->gpr[rD] = current_core->mmu.mas[3];
                                return 0;
                        case 20:
                                current_core->gpr[rD] = current_core->mmu.mas[4];
                                return 0;
                        case 22:
                                current_core->gpr[rD] = current_core->mmu.mas[6];
                                return 0;
                        case 25:
                                current_core->gpr[rD] = current_core->mmu.pid[1];
				//printf("read pid 1 0x%x\n", current_core->gpr[rD]);
                                return 0;
                        case 26:
                                current_core->gpr[rD] = current_core->mmu.pid[2];
				//printf("read pid 2 0x%x\n", current_core->gpr[rD]);
                                return 0;
		#endif

                }
                break;
	case 21:
		printf("NOT IMPLEMENT... IN %s, line %d\n", __func__, __LINE__);
                switch(spr1) {
                        //case 17:current_core->gpr[rD] = current_core->mmu.tlbcfg[1]; return 0;
                }
                break;
	case 29:
		printf("NOT IMPLEMENT... IN %s, line %d\n", __func__, __LINE__);
		#if 0
		switch (spr1) {
		case 16:
			current_core->gpr[rD] = 0;
			return 0;
		case 17:
			current_core->gpr[rD] = 0;
			return 0;
		case 18:
			current_core->gpr[rD] = 0;
			return 0;
		case 24:
			current_core->gpr[rD] = 0;
			return 0;
		case 25:
			current_core->gpr[rD] = 0;
			return 0;
		case 26:
			current_core->gpr[rD] = 0;
			return 0;
		case 28:
			current_core->gpr[rD] = 0;
			return 0;
		case 29:
			current_core->gpr[rD] = 0;
			return 0;
		case 30:
			current_core->gpr[rD] = 0;
			return 0;
		}
		#endif
		break;
	case 31:
		switch (spr1) {
		case 16:
//			PPC_OPC_WARN("read from spr %d:%d (HID0) not supported!\n", spr1, spr2);
			LET(rD, RS(HID_REGNUM));
			return 0;
		case 17:
			PPC_OPC_WARN("read from spr %d:%d (HID1) not supported!\n", spr1, spr2);
			LET(rD, RS(HID_REGNUM + 1));
			return 0;
		case 18:
			LET(rD, CONST(0));
			return 0;
		case 19:
			LET(rD, RS(E600_ICTRL_REGNUM));
			return 0;
		case 20:
			LET(rD, RS(E600_LDSTDB_REGNUM));
			return 0;
		case 21:
			LET(rD, CONST(0));
			return 0;
		case 22:
			LET(rD, CONST(0));
			return 0;
		case 23:
			LET(rD, CONST(0));
			return 0;
		case 24:
			LET(rD, CONST(0));
			return 0;
		case 25:
			PPC_OPC_WARN("read from spr %d:%d (L2CR) not supported! (from %08x)\n", spr1, spr2, current_core->pc);
			LET(rD, CONST(0));
			return 0;
		case 27:
			PPC_OPC_WARN("read from spr %d:%d (ICTC) not supported!\n", spr1, spr2);
			LET(rD, CONST(0));
			return 0;
		case 28:
//			PPC_OPC_WARN("read from spr %d:%d (THRM1) not supported!\n", spr1, spr2);
			LET(rD, CONST(0));
			return 0;
		case 29:
//			PPC_OPC_WARN("read from spr %d:%d (THRM2) not supported!\n", spr1, spr2);
			LET(rD, CONST(0));
			return 0;
		case 30:
//			PPC_OPC_WARN("read from spr %d:%d (THRM3) not supported!\n", spr1, spr2);
			LET(rD, CONST(0));
			return 0;
		case 31:
//			PPC_OPC_WARN("read from spr %d:%d (???) not supported!\n", spr1, spr2);
			LET(rD, CONST(0));
			return 0;
		}
		break;
	}
	fprintf(stderr, "[warning:mfspr]line = %d, instr = 0x%x, spr1:spr2 = %i:%i\n",
			__LINE__, instr, spr1, spr2);
}
/*
 *	cntlzwx		Count Leading Zeros Word
 *	.447
 */
static int opc_cntlzwx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	NOT_TESTED();
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	PPC_OPC_ASSERT(rB==0);
	Type const *ty = getIntegerType(32);
	Value* intrinsic_ctlz = (Value*)Intrinsic::getDeclaration(cpu->dyncom_engine->mod, Intrinsic::ctlz, &ty, 1);
	Value* result = CallInst::Create(intrinsic_ctlz, R(rS), "", bb);
	LET(rA, result);
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rA);
	}
}
/*
 *	cmpl		Compare Logical
 *	.444
 */
static int opc_cmpl_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	uint32 cr;
	int rA, rB;
	PPC_OPC_TEMPL_X(instr, cr, rA, rB);
	cr >>= 2;
	cr = 7-cr;
	Value* c;
	c = SELECT(ICMP_ULT(R(rA), R(rB)), CONST(8), SELECT(ICMP_UGT(R(rA), R(rB)), CONST(4), CONST(2)));
	c = SELECT(ICMP_NE(AND(RS(XER_REGNUM), CONST(XER_SO)), CONST(0)), OR(c, CONST(1)), c);
	LETS(CR_REGNUM, AND(RS(CR_REGNUM), CONST(ppc_cmp_and_mask[cr])));
	LETS(CR_REGNUM, OR(RS(CR_REGNUM), SHL(c, CONST(cr * 4))));
	return 0;
#if 0
	Value * tmp1 = ICMP_SLT(R(rA), R(rB));
	Value * tmp2 = ICMP_SGT(R(rA), R(rB));
	Value * tmp3 = ICMP_EQ(R(rA), R(rB));
	Value * c = SELECT(tmp1, CONST(8),SELECT(tmp2, CONST(4), SELECT(tmp3, CONST(2), CONST(0))));
	c = SELECT(ICMP_NE(AND(RS(XER_REGNUM), CONST(XER_SO)), CONST(0)), OR(c, CONST(1)), c);
	LETS(CR_REGNUM, AND(RS(CR_REGNUM), CONST(ppc_cmp_and_mask[cr])));
	LETS(CR_REGNUM, OR(RS(CR_REGNUM), SHL(c, CONST(cr * 4))));
	return 0;
#endif
}
/*
 *	dcbz		Data Cache Clear to Zero
 *	.464
 */
int opc_dcbz_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	*tag = TAG_CONTINUE;
	*tag |= TAG_EXCEPTION;
	*next_pc = phys_pc + PPC_INSN_SIZE;
	*new_pc = NEW_PC_NONE;
	return PPC_INSN_SIZE;
}
static int opc_dcbz_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb){
        int rA, rD, rB;
        PPC_OPC_TEMPL_X(instr, rD, rA, rB);
	Value* base;
	if(rA)
		base = ADD(R(rA), R(rB));
	else
		base = R(rB);
	int i = 0;
	for(; i < 32; i += 4)
		arch_write_memory(cpu, bb, ADD(base, CONST(i)), CONST(0), 32);
}

/*
 *	mullwx		Multiply Low Word
 *	.599
 */
static int opc_mullwx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rD, rA, rB;
	PPC_OPC_TEMPL_XO(instr, rD, rA, rB);
	LET(rD, MUL(R(rA), R(rB)));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rD);
	}
	if (instr & PPC_OPC_OE) {
		// update XER flags
		// FIXME
		PPC_ALU_ERR("mullwx unimplemented\n");
	}
}
/*
 *	negx		Negate
 *	.601
 */
static int opc_negx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rD, rA, rB;
	PPC_OPC_TEMPL_XO(instr, rD, rA, rB);
	PPC_OPC_ASSERT(rB == 0);
	LET(rD, NEG(R(rA)));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rD);
	}
}
/*
 *	andx		AND
 *	.431
 */
static int opc_andx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	LET(rA, AND(R(rS), R(rB)));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rA);
	}
}
/*
 *	srwx		Shift Right Word
 *	.631
 */
static int opc_srwx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	Value* v = AND(R(rB), CONST(0x3f));
	LET(rA, SELECT(ICMP_UGT(v, CONST(31)), CONST(0), LSHR(R(rS), v)));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rA);
	}
}
/*
 *	norx		NOR
 *	.602
 */
static int opc_norx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	Value* v = OR(R(rS), R(rB));
	LET(rA, XOR(v, CONST(-1)));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rA);
	}
}
/*
 *	mtcrf		Move to Condition Register Fields
 *	.576
 */
static int opc_mtcrf_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rS;
	uint32 crm;
	PPC_OPC_TEMPL_XFX(instr, rS, crm);
	Value* tmp1 = SELECT(ICMP_NE(AND(CONST(crm), CONST(0x80)), CONST(0)), CONST(0xf0000000), CONST(0));
	Value* tmp2 = SELECT(ICMP_NE(AND(CONST(crm), CONST(0x40)), CONST(0)), CONST(0x0f000000), CONST(0));
	Value* tmp3 = SELECT(ICMP_NE(AND(CONST(crm), CONST(0x20)), CONST(0)), CONST(0x00f00000), CONST(0));
	Value* tmp4 = SELECT(ICMP_NE(AND(CONST(crm), CONST(0x10)), CONST(0)), CONST(0x000f0000), CONST(0));
	Value* tmp5 = SELECT(ICMP_NE(AND(CONST(crm), CONST(0x08)), CONST(0)), CONST(0x0000f000), CONST(0));
	Value* tmp6 = SELECT(ICMP_NE(AND(CONST(crm), CONST(0x04)), CONST(0)), CONST(0x00000f00), CONST(0));
	Value* tmp7 = SELECT(ICMP_NE(AND(CONST(crm), CONST(0x02)), CONST(0)), CONST(0x000000f0), CONST(0));
	Value* tmp8 = SELECT(ICMP_NE(AND(CONST(crm), CONST(0x01)), CONST(0)), CONST(0x0000000f), CONST(0));
	Value* CRMv = OR(OR(OR(tmp1, tmp2), OR(tmp3, tmp4)), OR(OR(tmp5, tmp6), OR(tmp7, tmp8)));
	LETS(CR_REGNUM, OR(AND(R(rS), CRMv), AND(RS(CR_REGNUM), XOR(CRMv, CONST(-1)))));
	return 0;
}
/*
 *	subfx		Subtract From
 *	.666
 */
static int opc_subfx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rD, rA, rB;
	PPC_OPC_TEMPL_XO(instr, rD, rA, rB);
	LET(rD, ADD(XOR(R(rA), CONST(-1)), ADD(R(rB), CONST(1))));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rD);
	}
	return 0;
}
/*
 *	extshx		Extend Sign Half Word
 *	.482
 */
static int opc_extshx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	PPC_OPC_ASSERT(rB==0);
	Value *v_rs = R(rS);
	LET(rA, SELECT(ICMP_NE(AND(v_rs, CONST(0x8000)), CONST(0)), OR(v_rs, CONST(0xffff0000)), AND(v_rs, CONST(~0xffff0000))));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rA);
	}
	return 0;
}
/*
 *	andcx		AND with Complement
 *	.432
 */
static int opc_andcx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	LET(rA, AND(R(rS), XOR(R(rB), CONST(-1))));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rA);
	}
	return 0;
}
/*
 *	mfcr		Move from Condition Register
 *	.564
 */
static int opc_mfcr_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rD, rA, rB;
	PPC_OPC_TEMPL_X(instr, rD, rA, rB);
	PPC_OPC_ASSERT(rA==0 && rB==0);
	LET(rD, RS(CR_REGNUM));
	return 0;
}
/*
 *	lwarx		Load Word and Reserve Indexed
 *	.553
 */
int opc_lwarx_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	*tag = TAG_CONTINUE;
	*tag |= TAG_EXCEPTION;
	*next_pc = phys_pc + PPC_INSN_SIZE;
	*new_pc = NEW_PC_NONE;
	return PPC_INSN_SIZE;
}
static int opc_lwarx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rA, rD, rB;
	PPC_OPC_TEMPL_X(instr, rD, rA, rB);
	uint32 r;
	Value* addr = rA? ADD(R(rA), R(rB)): R(rB);
	Value* result = arch_read_memory(cpu, bb, addr, 0, 32);
	/**
	 * If occur exception, rD should not to be written.
	 **/
	if(is_user_mode(cpu)){
		LET(rD, result);
		LETS(RESERVE_REGNUM, result);
		LETS(HAVE_RESERVATION_REGNUM, CONST(1));
	}else{
		Value *current_pc = RS(PHYS_PC_REGNUM);
		Value *exc_occur = ICMP_EQ(current_pc, CONST(PPC_EXC_DSI_ADDR));
		LET(rD, SELECT(exc_occur, R(rD), result));
		LETS(RESERVE_REGNUM, SELECT(exc_occur, RS(RESERVE_REGNUM), result));
		LETS(HAVE_RESERVATION_REGNUM, SELECT(exc_occur, RS(HAVE_RESERVATION_REGNUM), CONST(1)));
	}
	return 0;
}
/*
 *	stwcx.		Store Word Conditional Indexed
 *	.661
 */
int opc_stwcx__tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	*tag = TAG_CONDITIONAL;
	*next_pc = phys_pc + PPC_INSN_SIZE;
	*new_pc = *next_pc;
	return PPC_INSN_SIZE;
}
Value* opc_stwcx__translate_cond(cpu_t *cpu, uint32_t instr, BasicBlock *bb){
	int rA, rS, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	LETS(CR_REGNUM, AND(RS(CR_REGNUM), CONST(0x0fffffff)));
	return ICMP_NE(RS(HAVE_RESERVATION_REGNUM), CONST(0));
}
static int opc_stwcx__translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rA, rS, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	LETS(HAVE_RESERVATION_REGNUM, CONST(0));
	Value* addr = rA? ADD(R(rA), R(rB)): R(rB);
	Value* v = arch_read_memory(cpu, bb, addr, 0, 32);
	Value* tmp = SELECT(ICMP_EQ(v, RS(RESERVE_REGNUM)), R(rS), v);
	arch_write_memory(cpu, bb, addr, tmp, 32);
	LETS(CR_REGNUM, SELECT(ICMP_EQ(v, RS(RESERVE_REGNUM)), OR(RS(CR_REGNUM), CONST(CR_CR0_EQ)), RS(CR_REGNUM)));
	LETS(CR_REGNUM, SELECT(ICMP_NE(AND(RS(XER_REGNUM), CONST(XER_SO)), CONST(0)), OR(RS(CR_REGNUM), CONST(CR_CR0_SO)), RS(CR_REGNUM)));
}
/*
 *	slwx		Shift Left Word
 *	.625
 */
static int opc_slwx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	Value* s = AND(R(rB), CONST(0x3f));
	LET(rA, SELECT(ICMP_UGT(s, CONST(31)), CONST(0), SHL(R(rS), s)));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rA);
	}
}
/*
 *	srawix		Shift Right Algebraic Word Immediate
 *	.629
 */
int opc_srawix_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	*tag = TAG_CONDITIONAL;
	*next_pc = phys_pc + PPC_INSN_SIZE;
	//*new_pc = NEW_PC_NONE;
	*new_pc = *next_pc;
	return PPC_INSN_SIZE;
}
Value* opc_srawix_translate_cond(cpu_t *cpu, uint32_t instr, BasicBlock *bb){
	int rS, rA;
	uint32 SH;
	PPC_OPC_TEMPL_X(instr, rS, rA, SH);
	LET(rA, R(rS));
	LETS(XER_REGNUM, AND(RS(XER_REGNUM), CONST(~XER_CA)));
	Value* tmp = SELECT(ICMP_UGT(CONST(SH), CONST(31)), CONST(0), LSHR(R(rA), UREM(CONST(SH), CONST(32))));
	Value* result = AND(R(rA), CONST(0x80000000));
	Value* cond = ICMP_NE(result, CONST(0));
	LET(rA, SELECT(cond, R(rA), tmp));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rA);
	}
	return cond;
}
static int opc_srawix_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rS, rA;
	uint32 SH;
	PPC_OPC_TEMPL_X(instr, rS, rA, SH);
	Value* ca = CONST(0);
	Value* mask = LSHR(CONST(0xffffffff), CONST((32 - SH) & 0x1f));
	LETS(XER_REGNUM, SELECT(ICMP_NE(AND(R(rA), mask), CONST(0)), OR(RS(XER_REGNUM), CONST(XER_CA)), RS(XER_REGNUM)));
	LET(rA, OR(LSHR(R(rA), UREM(CONST(SH),CONST(32))), SHL(CONST(0xffffffff), UREM(SUB(CONST(32), CONST(SH)), CONST(32)))));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rA);
	}
	return 0;
}
/*
 *	xorx		XOR
 *	.680
 */
static int opc_xorx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	LET(rA, XOR(R(rS), R(rB)));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rA);
	}
}
/*
 *	subfex		Subtract From Extended
 *	.668
 */
static int opc_subfex_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rD, rA, rB;
	PPC_OPC_TEMPL_XO(instr, rD, rA, rB);
	Value* ca = SELECT(ICMP_NE(AND(RS(XER_REGNUM), CONST(XER_CA)), CONST(0)), CONST(1), CONST(0));
	LET(rD, ADD(ADD(XOR(R(rA), CONST(-1)), R(rB)), ca));
	// update xer
	SELECT(ppc_dyncom_carry_3(cpu, bb, XOR(R(rA), CONST(-1)), R(rB), ca), OR(RS(XER_REGNUM), CONST(XER_CA)), AND(RS(XER_REGNUM), CONST(~XER_CA)));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rD);
	}
}
/*
 *	sync		Synchronize
 *	.672
 */
static int opc_sync_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	// NO-OP
}
/*
 *	mulhwux		Multiply High Word Unsigned
 *	.596
 */
static int opc_mulhwux_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rD, rA, rB;
	PPC_OPC_TEMPL_XO(instr, rD, rA, rB);
	Value* result64 = MUL(ZEXT64(R(rA)), ZEXT64(R(rB)));
	LET(rD, TRUNC32(LSHR(result64, CONST64(32))));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rD);
	}
}
/*
 *	stwx		Store Word Indexed
 *	.665
 */
static int opc_stwx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rA, rS, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	Value *addr;
	if(rA)
		addr = ADD(R(rA), R(rB));
	else
		addr = R(rB);
	arch_write_memory(cpu, bb, addr, R(rS), 32);
	return 0;
}
/*
 *	addzex		Add to Zero Extended
 *	.430
 */
static int opc_addzex_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rD, rA, rB;
	PPC_OPC_TEMPL_XO(instr, rD, rA, rB);
	PPC_OPC_ASSERT(rB == 0);
	Value* ca = SELECT(ICMP_NE(AND(RS(XER_REGNUM), CONST(XER_CA)), CONST(0)), CONST(1), CONST(0));
	LET(rD, ADD(ca, R(rA)));
	LETS(XER_REGNUM, SELECT(AND(ICMP_EQ(R(rA), CONST(0xffffffff)), ICMP_NE(ca, CONST(0))), OR(RS(XER_REGNUM), CONST(XER_CA)), AND(RS(XER_REGNUM), CONST(~XER_CA))));
	// update xer
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rD);
	}
}
/*
 *	eieio		Enforce In-Order Execution of I/O
 *	.478
 */
static int opc_eieio_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	// NO-OP
}
/*
 *	divwux		Divide Word Unsigned
 *	.472
 */
static int opc_divwux_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rD, rA, rB;
	PPC_OPC_TEMPL_XO(instr, rD, rA, rB);
	/* FIXME:checking division by zeor */
//	if (!current_core->gpr[rB]) {
//		PPC_ALU_WARN("division by zero @%08x\n", current_core->pc);
//	}
	LET(rD, UDIV(R(rA), R(rB)));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rD);
	}
}
/*
 *	orcx		OR with Complement
 *	.604
 */
static int opc_orcx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	LET(rA, OR(R(rS), XOR(R(rB), CONST(-1))));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rA);
	}
}
/*
 *	extsbx		Extend Sign Byte
 *	.481
 */
static int opc_extsbx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	PPC_OPC_ASSERT(rB==0);
	LET(rA, R(rS));
	LET(rA, SELECT(ICMP_NE(AND(R(rA), CONST(0x80)), CONST(0)), OR(R(rA), CONST(0xffffff00)), AND(R(rA), CONST(~0xffffff00))));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rA);
	}
}
/*
 *	lbzx		Load Byte and Zero Indexed
 *	.524
 */
static int opc_lbzx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rA, rD, rB;
	PPC_OPC_TEMPL_X(instr, rD, rA, rB);
	uint8 r;
	Value* addr;
	if(rA)
		addr = ADD(R(rA), R(rB));
	else
		addr = R(rB);
	Value* result = arch_read_memory(cpu, bb, addr, 0, 8);
	LET(rD, result);
	return 0;
}
/*
 *	stbx		Store Byte Indexed
 *	.635
 */
int opc_stbx_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	*tag = TAG_CONTINUE;
	*tag |= TAG_EXCEPTION;
	*next_pc = phys_pc + PPC_INSN_SIZE;
	*new_pc = NEW_PC_NONE;
	return PPC_INSN_SIZE;
}
static int opc_stbx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rA, rS, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	Value* addr;
	if(rA)
		addr = ADD(R(rA), R(rB));
	else
		addr = R(rB);
	arch_write_memory(cpu, bb, addr, R(rS), 8);
	return 0;
}
/*
 *	lhzx		Load Half Word and Zero Indexed
 *	.546
 */
static int opc_lhzx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rA, rD, rB;
	PPC_OPC_TEMPL_X(instr, rD, rA, rB);
	Value* addr;
	if(rA)
		addr = ADD(R(rA), R(rB));
	else
		addr = R(rB);
	Value* result = arch_read_memory(cpu, bb, addr, 0, 16);
	LET(rD, result);
	return 0;
}
/*
 *	divwx		Divide Word
 *	.470
 */
static int opc_divwx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rD, rA, rB;
	PPC_OPC_TEMPL_XO(instr, rD, rA, rB);
	/* FIXME check division by zero */
//	if (!current_core->gpr[rB]) {
//		PPC_ALU_WARN("division by zero @%08x\n", current_core->pc);
//	}
	LET(rD, SDIV(R(rA), R(rB)));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rD);
	}
}
/*
 *	mulhwx		Multiply High Word
 *	.595
 */
static int opc_mulhwx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rD, rA, rB;
	PPC_OPC_TEMPL_XO(instr, rD, rA, rB);
	Value* result64 = MUL(SEXT64(R(rA)), SEXT64(R(rB)));
	LET(rD, TRUNC32(LSHR(result64, CONST64(32))));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rD);
	}
}
/*
 *	sthx		Store Half Word Indexed
 *	.655
 */
int opc_sthx_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	*tag = TAG_CONTINUE;
	*tag |= TAG_EXCEPTION;
	*next_pc = phys_pc + PPC_INSN_SIZE;
	*new_pc = NEW_PC_NONE;
	return PPC_INSN_SIZE;
}
static int opc_sthx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rA, rS, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	Value* addr = rA ? ADD(R(rB), R(rA)) : R(rB);
	arch_write_memory(cpu, bb, addr, R(rS), 16);
	return 0;
}
/*
 *	lbzux		Load Byte and Zero with Update Indexed
 *	.523
 */
static int opc_lbzux_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rA, rD, rB;
	PPC_OPC_TEMPL_X(instr, rD, rA, rB);
	// FIXME: check rA!=0 && rA!=rD
	Value* addr = ADD(R(rA), R(rB));
	Value* result = arch_read_memory(cpu, bb, addr, 0, 8);
	LET(rA, addr);
	LET(rD, AND(result, CONST(0x000000ff)));
	return 0;
}
/*
 *	lhax		Load Half Word Algebraic Indexed
 *	.541
 */
static int opc_lhax_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rA, rD, rB;
	PPC_OPC_TEMPL_X(instr, rD, rA, rB);
	uint16 r;
	Value* addr = rA ? ADD(R(rB), R(rA)) : R(rB);
	Value* result = arch_read_memory(cpu, bb, addr, 1, 16);
	LET(rD,result);
	return 0;
}
/*
 *	subfcx		Subtract From Carrying
 *	.667
 */
static int opc_subfcx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rD, rA, rB;
	PPC_OPC_TEMPL_XO(instr, rD, rA, rB);
	Value* a = R(rA);
	Value* b = R(rB);
	LET(rD, ADD(XOR(a, CONST(-1)), ADD(b, CONST(1))));
	Value* cond = ppc_dyncom_carry_3(cpu, bb, XOR(a, CONST(-1)), b, CONST(1));
	LETS(XER_REGNUM, SELECT(cond, OR(RS(XER_REGNUM), CONST(XER_CA)), AND(RS(XER_REGNUM), CONST(~XER_CA))));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rD);
	}
	return 0;
}
/*
 *	subfzex		Subtract From Zero Extended
 *	.671
 */
static int opc_subfzex_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rD, rA, rB;
	PPC_OPC_TEMPL_XO(instr, rD, rA, rB);
	PPC_OPC_ASSERT(rB == 0);
	Value* ca = SELECT(ICMP_NE(AND(R(XER_REGNUM), CONST(XER_CA)), CONST(0)), CONST(1), CONST(0));
	LET(rD, ADD(XOR(R(rA), CONST(-1)), ca));
	LET(XER_REGNUM, SELECT(AND(LOG_NOT(R(rA)), ICMP_NE(ca, CONST(0))), OR(R(XER_REGNUM), CONST(XER_CA)), AND(R(XER_REGNUM), CONST(~XER_CA))));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rD);
	}
}

/*
 *	stwux		Store Word with Update Indexed
 *	.664
 */
static int opc_stwux_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rA, rS, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	Value* addr = ADD(R(rA), R(rB));
	arch_write_memory(cpu, bb, addr, R(rS), 32);
	LET(rA, addr);
	NOT_TESTED();
	return 0;
}
/*
 *	addex		Add Extended
 *	.424
 */
static int opc_addex_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rD, rA, rB;
	PPC_OPC_TEMPL_XO(instr, rD, rA, rB);
	Value* ca = SELECT(ICMP_NE(AND(RS(XER_REGNUM), CONST(XER_CA)), CONST(0)), CONST(1), CONST(0));
	Value* ra = R(rA);
	Value* rb = R(rB);
	LET(rD, ADD(ra, ADD(rb, ca)));
	Value* cond = ppc_dyncom_carry_3(cpu, bb, ra, rb, ca);
	LETS(XER_REGNUM, SELECT(cond, OR(RS(XER_REGNUM), CONST(XER_CA)), AND(RS(XER_REGNUM), CONST(~XER_CA))));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rD);
	}
	NOT_TESTED();
	return 0;
}
/*
 *	nandx		NAND
 *	.600
 */
static int opc_nandx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	LET(rA, XOR(AND(R(rS), R(rB)), CONST(-1)));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rA);
	}
	NOT_TESTED();
	return 0;
}
/*
 *	addcx		Add Carrying
 *	.423
 */
static int opc_addcx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rD, rA, rB;
	PPC_OPC_TEMPL_XO(instr, rD, rA, rB);
	Value* ra = R(rA);
	LET(rD, ADD(ra, R(rB)));
	Value* cond = ICMP_ULT(R(rD), ra);
	LETS(XER_REGNUM, SELECT(cond, OR(RS(XER_REGNUM), CONST(XER_CA)), AND(RS(XER_REGNUM), CONST(~XER_CA))));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rD);
	}
	NOT_TESTED();
	return 0;
}
/*
 *	addmex		Add to Minus One Extended
 *	.429
 */
static int opc_addmex_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rD, rA, rB;
	PPC_OPC_TEMPL_XO(instr, rD, rA, rB);
	PPC_OPC_ASSERT(rB == 0);
	Value* ca = SELECT(ICMP_NE(AND(RS(XER_REGNUM), CONST(XER_CA)), CONST(0)), CONST(1), CONST(0));
	LET(rD, ADD(R(rA), ADD(ca, CONST(0xffffffff))));
	LETS(XER_REGNUM, SELECT(OR(ICMP_NE(R(rA), CONST(0)), ICMP_NE(ca, CONST(0))), OR(RS(XER_REGNUM), CONST(XER_CA)), AND(RS(XER_REGNUM), CONST(~XER_CA))));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rD);
	}
	NOT_TESTED();
	return 0;
}
/*
 *	srawx		Shift Right Algebraic Word
 *	.628
 */
static int opc_srawx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	Value *sh = AND(R(rB), CONST(0x1f));
	Value *rS_v = R(rS);
	LET(rA, ASHR(rS_v, sh));
	Value *ca_flag = ICMP_NE(AND(rS_v, SUB(LSHR(CONST(0x80000000),SUB(sh, CONST(1))), CONST(1))), CONST(0));
	Value *is_negtive = ICMP_EQ(AND(rS_v, CONST(0x80000000)), CONST(1));
	Value *xer_v = RS(XER_REGNUM);
	LETS(XER_REGNUM, SELECT(ICMP_EQ(AND(ca_flag, is_negtive), CONST1(1)), OR(xer_v, CONST(XER_CA)), AND(xer_v, CONST(~XER_CA))));
	if (instr & PPC_OPC_Rc) {
		ppc_dyncom_update_cr0(cpu, bb, rA);
	}
}
/*
 *	dcbst		Data Cache Block Store
 *	.461
 */
static int opc_dcbst_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	// NO-OP
}
/*
 *	icbi		Instruction Cache Block Invalidate
 *	.519
 */
static int opc_icbi_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	// NO-OP
}
/*
 *	mfmsr		Move from Machine State Register
 *	.566
 */
int opc_mfmsr_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	*tag = TAG_EXCEPTION;
	return PPC_INSN_SIZE;
}
static int opc_mfmsr_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb){
	Value *cond = ICMP_NE(AND(RS(MSR_REGNUM), CONST(MSR_PR)), CONST(0));
	arch_ppc_dyncom_exception(cpu, bb, cond, PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV, 0);
	int rD, rA, rB;
	PPC_OPC_TEMPL_X(instr, rD, rA, rB);
	PPC_OPC_ASSERT((rA == 0) && (rB == 0));
	Value *rd = R(rD);
	LET(rD, SELECT(cond, rd, RS(MSR_REGNUM)));
}
/*
 *	tlbie		Translation Lookaside Buffer Invalidate All
 *	.676
 */
int opc_tlbie_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	*tag = TAG_EXCEPTION;
	return PPC_INSN_SIZE;
}
static int opc_tlbie_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb){
	Value *cond = ICMP_NE(AND(RS(MSR_REGNUM), CONST(MSR_PR)), CONST(0));
	arch_ppc_dyncom_exception(cpu, bb, cond, PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV, 0);
	LETS(EFFECTIVE_CODE_PAGE_REGNUM, SELECT(cond, RS(EFFECTIVE_CODE_PAGE_REGNUM), CONST(0xffffffff)));
}
/*
 *	mtmsr		Move to Machine State Register
 *	.581
 */
int opc_mtmsr_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	*tag = TAG_EXCEPTION;
	return PPC_INSN_SIZE;
}
static int opc_mtmsr_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	Value *cond = ICMP_NE(AND(RS(MSR_REGNUM), CONST(MSR_PR)), CONST(0));
	arch_ppc_dyncom_exception(cpu, bb, cond, PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV, 0);
	ppc_dyncom_set_msr(cpu, bb, R(rS), cond);
}
/*
 *	tlbsync		Translation Lookaside Buffer Syncronize
 *	.677
 */
int opc_tlbsync_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	*tag = TAG_EXCEPTION;
	return PPC_INSN_SIZE;
}
static int opc_tlbsync_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *cond = ICMP_NE(AND(RS(MSR_REGNUM), CONST(MSR_PR)), CONST(0));
	arch_ppc_dyncom_exception(cpu, bb, cond, PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV, 0);
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	// FIXME: check rS.. for 0     
	LETS(EFFECTIVE_CODE_PAGE_REGNUM, SELECT(cond, RS(EFFECTIVE_CODE_PAGE_REGNUM), CONST(0xffffffff)));
}
/*
 *	mtsrin		Move to Segment Register Indirect
 *	.591
 */
int opc_mtsrin_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	*tag = TAG_EXCEPTION;
	return PPC_INSN_SIZE;
}
static int opc_mtsrin_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *cond = ICMP_NE(AND(RS(MSR_REGNUM), CONST(MSR_PR)), CONST(0));
	arch_ppc_dyncom_exception(cpu, bb, cond, PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV, 0);
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	// FIXME: check insn
	Value *index = LSHR(R(rB), CONST(28));
	Value *base = cpu->ptr_spr[SR_REGNUM];
	Value *pointer = GetElementPtrInst::Create(base, index, "", bb);
	Value *sr_v_orig = new LoadInst(pointer, "", false, bb);
	new StoreInst(SELECT(cond, sr_v_orig, R(rS)), pointer, bb);
}
/*
 *	mfsrin		Move from Segment Register Indirect
 *	.572
 */
int opc_mfsrin_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	*tag = TAG_EXCEPTION;
	return PPC_INSN_SIZE;
}
static int opc_mfsrin_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *cond = ICMP_NE(AND(RS(MSR_REGNUM), CONST(MSR_PR)), CONST(0));
	arch_ppc_dyncom_exception(cpu, bb, cond, PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV, 0);
	int rD, rA, rB;
	PPC_OPC_TEMPL_X(instr, rD, rA, rB);
	// FIXME: check insn
	Value *index = LSHR(R(rB), CONST(28));
	Value *base = cpu->ptr_spr[SR_REGNUM];
	Value *pointer = GetElementPtrInst::Create(base, index, "", bb);
	Value *sr_v = new LoadInst(pointer, "", false, bb);
	LET(rD, SELECT(cond, R(rD), sr_v));
}
/*
 *	sthbrx		Store Half Word Byte-Reverse Indexed
 *	.652
 */
static int opc_sthbrx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rA, rS, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	Value *addr = rA ? ADD(R(rA), R(rB)) : R(rB);
	Value *data = AND(OR(SHL(R(rS), CONST(8)), LSHR(R(rS), CONST(8))), CONST(0x0000ffff));
	arch_write_memory(cpu, bb, addr, data, 16);
}
/*
 *	lhbrx		Load Half Word Byte-Reverse Indexed
 *	.542
 */
static int opc_lhbrx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rA, rD, rB;
	PPC_OPC_TEMPL_X(instr, rD, rA, rB);
	Value *addr = rA ? ADD(R(rA), R(rB)) : R(rB);
	Value* result = arch_read_memory(cpu, bb, addr, 0, 16);
	result = AND(OR(SHL(result, CONST(8)), LSHR(result, CONST(8))), CONST(0x0000ffff));
	LET(rD, result);
}
/*
 *	stwbrx		Store Word Byte-Reverse Indexed
 *	.660
 */
int opc_stwbrx_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	*tag = TAG_CONTINUE;
	*tag |= TAG_EXCEPTION;
	*next_pc = phys_pc + PPC_INSN_SIZE;
	*new_pc = NEW_PC_NONE;
	return PPC_INSN_SIZE;
}
static int opc_stwbrx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rA, rS, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	Value *addr = rA ? ADD(R(rA), R(rB)) : R(rB);
	Value *rs = R(rS);
	rs = OR(OR(LSHR(rs, CONST(24)), AND(LSHR(rs, CONST(8)), CONST(0xff00))),
			OR(SHL(rs, CONST(24)), AND(SHL(rs, CONST(8)), CONST(0xff0000)))
			);
	// FIXME: doppelt gemoppelt
	arch_write_memory(cpu, bb, addr, rs, 32);
}
/*
 *	lwzux		Load Word and Zero with Update Indexed
 *	.559
 */
int opc_lwzux_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	*tag = TAG_CONTINUE;
	*tag |= TAG_EXCEPTION;
	*next_pc = phys_pc + PPC_INSN_SIZE;
	*new_pc = NEW_PC_NONE;
	return PPC_INSN_SIZE;
}
static int opc_lwzux_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rA, rD, rB;
	PPC_OPC_TEMPL_X(instr, rD, rA, rB);
	// FIXME: check rA!=0 && rA!=rD
	Value *addr = ADD(R(rA), R(rB));
	Value* result = arch_read_memory(cpu, bb, addr, 0, 32);
	/**
	 * If occur exception, rD should not to be written.
	 **/
	if(is_user_mode(cpu)){
		LET(rA, addr);
		LET(rD, result);
	}else{
		Value *current_pc = RS(PHYS_PC_REGNUM);
		Value *exc_occur = ICMP_EQ(current_pc, CONST(PPC_EXC_DSI_ADDR));
		LET(rA, SELECT(exc_occur, R(rA), addr));
		LET(rD, SELECT(exc_occur, R(rD), result));
	}
}
static int opc_isel_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rD, rA, rB;
	PPC_OPC_TEMPL_XO(instr, rD, rA, rB);
	int crb = (instr >> 6) & 0x1f;
	Value *cond = ICMP_NE(AND(RS(CR_REGNUM), CONST(1 << (31 - crb))), CONST(0));
	LET(rD, SELECT(cond, R(rA), R(rB)));
}
static int opc_iseleq_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rD, rA, rB;
	PPC_OPC_TEMPL_XO(instr, rD, rA, rB);
	Value *cond = ICMP_NE(AND(RS(CR_REGNUM), CONST(CR_CR0_EQ)), CONST(0));
	LET(rD, SELECT(cond, R(rA), R(rB)));
}
/* Interfaces */
ppc_opc_func_t ppc_opc_cmp_func = {
	opc_default_tag,
	opc_cmp_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_tw_func;
ppc_opc_func_t ppc_opc_subfcx_func = {
	opc_default_tag,
	opc_subfcx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_addcx_func = {
	opc_default_tag,
	opc_addcx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_mulhwux_func = {
	opc_default_tag,
	opc_mulhwux_translate,
	opc_invalid_translate_cond,
};

ppc_opc_func_t ppc_opc_mfcr_func = {
	opc_default_tag,
	opc_mfcr_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_lwarx_func = {
	opc_lwarx_tag,
	opc_lwarx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_lwzx_func = {
	opc_lwzx_tag,
	opc_lwzx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_slwx_func = {
	opc_default_tag,
	opc_slwx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_cntlzwx_func = {
	opc_default_tag,
	opc_cntlzwx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_andx_func = {
	opc_default_tag,
	opc_andx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_cmpl_func = {
	opc_default_tag,
	opc_cmpl_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_subfx_func = {
	opc_default_tag,
	opc_subfx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_iselgt_func;
ppc_opc_func_t ppc_opc_dcbst_func = {
	opc_default_tag,
	opc_dcbst_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_lwzux_func = {
	opc_lwzux_tag,
	opc_lwzux_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_andcx_func = {
	opc_default_tag,
	opc_andcx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_mulhwx_func = {
	opc_default_tag,
	opc_mulhwx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_iseleq_func = {
	opc_default_tag,
	opc_iseleq_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_mfmsr_func = {
	opc_mfmsr_tag,
	opc_mfmsr_translate,
	opc_invalid_translate_cond,
};

ppc_opc_func_t ppc_opc_dcbf_func;
ppc_opc_func_t ppc_opc_lbzx_func = {
	opc_default_tag,
	opc_lbzx_translate,
	opc_invalid_translate_cond,
};

ppc_opc_func_t ppc_opc_negx_func = {
	opc_default_tag,
	opc_negx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_lbzux_func = {
	opc_default_tag,
	opc_lbzux_translate,
	opc_invalid_translate_cond,
};

ppc_opc_func_t ppc_opc_norx_func = {
	opc_default_tag,
	opc_norx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_wrtee_func;
ppc_opc_func_t ppc_opc_subfex_func = {
	opc_default_tag,
	opc_subfex_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_addex_func = {
	opc_default_tag,
	opc_addex_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_mtcrf_func = {
	opc_default_tag,
	opc_mtcrf_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_mtmsr_func = {
	opc_mtmsr_tag,
	opc_mtmsr_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_stwcx__func = {
	opc_stwcx__tag,
	opc_stwcx__translate,
	opc_stwcx__translate_cond,
};
ppc_opc_func_t ppc_opc_stwx_func = {
	opc_default_tag,
	opc_stwx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_wrteei_func;
ppc_opc_func_t ppc_opc_dcbtls_func;
ppc_opc_func_t ppc_opc_stwux_func = {
	opc_default_tag,
	opc_stwux_translate,
	opc_invalid_translate_cond,
};

ppc_opc_func_t ppc_opc_subfzex_func = {
	opc_default_tag,
	opc_subfzex_translate,
	opc_invalid_translate_cond,

};//+
ppc_opc_func_t ppc_opc_addzex_func = {
	opc_default_tag,
	opc_addzex_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_mtsr_func;
ppc_opc_func_t ppc_opc_stbx_func = {
	opc_stbx_tag,
	opc_stbx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_subfmex_func;//+
ppc_opc_func_t ppc_opc_addmex_func = {
	opc_default_tag,
	opc_addmex_translate,
	opc_invalid_translate_cond,
};

ppc_opc_func_t ppc_opc_mullwx_func = {
	opc_default_tag,
	opc_mullwx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_mtsrin_func = {
	opc_mtsrin_tag,
	opc_mtsrin_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_dcbtst_func = {
	opc_default_tag,
	opc_default_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_stbux_func;
ppc_opc_func_t ppc_opc_addx_func = {
	opc_default_tag,
	opc_addx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_dcbt_func = {
	opc_default_tag,
	opc_default_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_lhzx_func = {
	opc_default_tag,
	opc_lhzx_translate,
	opc_invalid_translate_cond,
};

ppc_opc_func_t ppc_opc_eqvx_func;
ppc_opc_func_t ppc_opc_tlbie_func = {
	opc_tlbie_tag,
	opc_tlbie_translate,
	opc_invalid_translate_cond,
};

ppc_opc_func_t ppc_opc_eciwx_func;
ppc_opc_func_t ppc_opc_lhzux_func;
ppc_opc_func_t ppc_opc_xorx_func = {
	opc_default_tag,
	opc_xorx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_mfspr_func = {
	opc_default_tag,
	opc_mfspr_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_mtspr_func = {
	opc_default_tag,
	opc_mtspr_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_lhax_func = {
	opc_default_tag,
	opc_lhax_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_isel_func = {
	opc_default_tag,
	opc_isel_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_tlbia_func;
ppc_opc_func_t ppc_opc_lhaux_func;
ppc_opc_func_t ppc_opc_sthx_func = {
	opc_sthx_tag,
	opc_sthx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_orcx_func = {
	opc_default_tag,
	opc_orcx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_ecowx_func;
ppc_opc_func_t ppc_opc_sthux_func;
ppc_opc_func_t ppc_opc_orx_func = {
	opc_default_tag,
	opc_orx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_divwux_func = {
	opc_default_tag,
	opc_divwux_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_dcbi_func;
ppc_opc_func_t ppc_opc_nandx_func = {
	opc_default_tag,
	opc_nandx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_divwx_func = {
	opc_default_tag,
	opc_divwx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_mcrxr_func;
ppc_opc_func_t ppc_opc_lswx_func;
ppc_opc_func_t ppc_opc_lwbrx_func;
ppc_opc_func_t ppc_opc_lfsx_func;
ppc_opc_func_t ppc_opc_srwx_func = {
	opc_default_tag,
	opc_srwx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_tlbsync_func = {
	opc_tlbsync_tag,
	opc_tlbsync_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_lfsux_func;
ppc_opc_func_t ppc_opc_mfsr_func;
ppc_opc_func_t ppc_opc_lswi_func;
ppc_opc_func_t ppc_opc_sync_func = {
	opc_default_tag,
	opc_sync_translate,
	opc_invalid_translate_cond,
};

ppc_opc_func_t ppc_opc_lfdx_func;
ppc_opc_func_t ppc_opc_lfdux_func;
ppc_opc_func_t ppc_opc_mfsrin_func = {
	opc_mfsrin_tag,
	opc_mfsrin_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_stswx_func;
ppc_opc_func_t ppc_opc_stwbrx_func = {
	opc_stwbrx_tag,
	opc_stwbrx_translate,
	opc_invalid_translate_cond,
};

ppc_opc_func_t ppc_opc_stfsx_func;
ppc_opc_func_t ppc_opc_stfsux_func;
ppc_opc_func_t ppc_opc_stswi_func;
ppc_opc_func_t ppc_opc_stfdx_func;
ppc_opc_func_t ppc_opc_dcba_func;
ppc_opc_func_t ppc_opc_stfdux_func;
ppc_opc_func_t ppc_opc_tlbivax_func; /* TLB invalidated virtual address indexed */
ppc_opc_func_t ppc_opc_lhbrx_func = {
	opc_default_tag,
	opc_lhbrx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_srawx_func = {
	opc_default_tag,
	opc_srawx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_srawix_func = {
	opc_srawix_tag,
	opc_srawix_translate,
	opc_srawix_translate_cond,
};
ppc_opc_func_t ppc_opc_eieio_func = {
	opc_default_tag,
	opc_eieio_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_tlbsx_func;
ppc_opc_func_t ppc_opc_sthbrx_func = {
	opc_default_tag,
	opc_sthbrx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_extshx_func = {
	opc_default_tag,
	opc_extshx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_tlbrehi_func;
ppc_opc_func_t ppc_opc_extsbx_func = {
	opc_default_tag,
	opc_extsbx_translate,
	opc_invalid_translate_cond,
};

ppc_opc_func_t ppc_opc_tlbwe_func; /* TLB write entry */
ppc_opc_func_t ppc_opc_icbi_func = {
	opc_default_tag,
	opc_icbi_translate,
	opc_invalid_translate_cond,
};

ppc_opc_func_t ppc_opc_stfiwx_func;
ppc_opc_func_t ppc_opc_dcbz_func = {
	opc_dcbz_tag,
	opc_dcbz_translate,
	opc_invalid_translate_cond,
};

ppc_opc_func_t ppc_opc_dss_func;      /*Temporarily modify*/
ppc_opc_func_t ppc_opc_lvsl_func;
ppc_opc_func_t ppc_opc_lvebx_func;
ppc_opc_func_t ppc_opc_lvsr_func;
ppc_opc_func_t ppc_opc_lvehx_func;
ppc_opc_func_t ppc_opc_lvewx_func;
ppc_opc_func_t ppc_opc_lvx_func;
ppc_opc_func_t ppc_opc_stvebx_func;
ppc_opc_func_t ppc_opc_stvehx_func;
ppc_opc_func_t ppc_opc_stvewx_func;
ppc_opc_func_t ppc_opc_stvx_func;
ppc_opc_func_t ppc_opc_dst_func;
ppc_opc_func_t ppc_opc_lvxl_func;
ppc_opc_func_t ppc_opc_dstst_func;
ppc_opc_func_t ppc_opc_stvxl_func;
