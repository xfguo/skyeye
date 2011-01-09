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

#include "ppc_dyncom_debug.h"

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
	e500_core_t* current_core = get_current_core();
	PPC_OPC_TEMPL_X(instr, cr, rA, rB);
	cr >>= 2;
	cr = 7 - cr;
	Value * tmp1 = ICMP_SLT(R(rA), R(rB));
	Value * tmp2 = ICMP_SGT(R(rA), R(rB));
	Value * tmp3 = ICMP_EQ(R(rA), R(rB));
	Value * c = SELECT(tmp1, CONST(8),SELECT(tmp2, CONST(4), SELECT(tmp3, CONST(2), CONST(0))));
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
static int opc_lwzx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rA, rD, rB;
	PPC_OPC_TEMPL_X(instr, rD, rA, rB);
	Value* addr = rA ? ADD(R(rB), R(rA)) : R(rB);
	Value* result = arch_read_memory(cpu, bb, addr, 0, 32);
	LET(rD, result);
	return 0;
}
/*
 *	mtspr		Move to Special-Purpose Register
 *	.584
 */
static int opc_mtspr_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rS, spr1, spr2;
	PPC_OPC_TEMPL_X(instr, rS, spr1, spr2);
	e500_core_t* current_core = get_current_core();
	switch (spr2) {
	case 0:
		switch (spr1) {
		case 1: LETS(XER_REGNUM, R(rS)); return 0;
		case 8:	LETS(LR_REGNUM, R(rS)); return 0;
		case 9:	LETS(CTR_REGNUM,R(rS)); return 0;
		}
		break;
	
	case 8:	//altivec makes this register unpriviledged
		if (spr1 == 0) {
			LET32_BY_PTR(&current_core->vrsave, R(rS)); 
			return 0;
		}
		switch(spr1){
			case 28:
				LET32_BY_PTR(&current_core->tbl, R(rS)); 
				return 0;
			case 29:
				current_core->tbu = current_core->gpr[rS];
				return 0;
		}
		break;
	}
	if (current_core->msr & MSR_PR) {
		//	ppc_exception(current_core, PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV, 0);
		//printf("Warning: execute mtspr in user mode\n");
		//return;
	}
	switch (spr2) {
	case 0:
		switch (spr1) {
/*		case 18: current_core->gpr[rD] = current_core->dsisr; return;
		case 19: current_core->gpr[rD] = current_core->dar; return;*/
		case 22: {
			//printf("In %s, write DEC=0x%x\n", __FUNCTION__, current_core->gpr[rS]);
			current_core->dec = current_core->gpr[rS];
			current_core->pdec = current_core->dec;
			current_core->pdec *= TB_TO_PTB_FACTOR;
			return 0;
		}
		case 25: 
			if (!ppc_mmu_set_sdr1(current_core->gpr[rS], True)) {
				PPC_OPC_ERR("cannot set sdr1\n");
			}
			return 0;
		case 26: current_core->srr[0] = current_core->gpr[rS]; return 0;
		case 27: current_core->srr[1] = current_core->gpr[rS]; return 0;
		default:fprintf(stderr, "spr2=0x%x,spr1=0x%x,pc=0x%x,no such spr\n", spr2,spr1,current_core->pc);break;
		}
		break;
	case 1:
		switch (spr1) {
			case 16:
				current_core->mmu.pid[0] = current_core->gpr[rS]; 
				//printf("write pid0=0x%x\n", current_core->gpr[rS]);
				return 0;
			case 26:current_core->csrr[0] = current_core->gpr[rS];return 0;
			case 27:current_core->csrr[1] = current_core->gpr[rS];return 0;
			case 29:current_core->dear = current_core->gpr[rS];return 0;
			case 30:current_core->esr = current_core->gpr[rS];return 0;
			case 31:current_core->ivpr = current_core->gpr[rS];return 0;
			default:fprintf(stderr, "spr2=0x%x,spr1=0x%x,pc=0x%x,no such spr\n", spr2,spr1,current_core->pc);break;
		}
	case 8:
		switch (spr1) {
		case 16: current_core->sprg[0] = current_core->gpr[rS]; return 0;
		case 17: current_core->sprg[1] = current_core->gpr[rS]; return 0;
		case 18: current_core->sprg[2] = current_core->gpr[rS]; return 0;
		case 19: current_core->sprg[3] = current_core->gpr[rS]; return 0;
		case 20: current_core->sprg[4] = current_core->gpr[rS]; return 0;
		case 21: current_core->sprg[5] = current_core->gpr[rS]; return 0;
		case 22: current_core->sprg[6] = current_core->gpr[rS]; return 0;
		case 23: current_core->sprg[7] = current_core->gpr[rS]; return 0;
/*		case 26: current_core->gpr[rD] = current_core->ear; return;
		case 31: current_core->gpr[rD] = current_core->pvr; return;*/
		default:fprintf(stderr, "spr2=0x%x,spr1=0x%x,pc=0x%x,no such spr\n", spr2,spr1,current_core->pc);break;
		}
		break;
	case 9:
		switch (spr1) {
			case 16:current_core->dbsr = current_core->gpr[rS]; return 0;
			case 20:current_core->dbcr[0] = current_core->gpr[rS]; return 0;
			case 21:current_core->dbcr[1] = current_core->gpr[rS]; return 0;
			case 22:current_core->dbcr[2] = current_core->gpr[rS]; return 0;
			case 28:current_core->dac[0] = current_core->gpr[rS]; return 0;
			case 29:current_core->dac[1] = current_core->gpr[rS]; return 0;
			default:fprintf(stderr, "spr2=0x%x,spr1=0x%x,pc=0x%x,no such spr\n", spr2,spr1,current_core->pc);break;
		}
		break;
	case 10:
		switch (spr1){
			
			case 16:
				/* W1C, write one to clear */
				current_core->tsr &= ~(current_core->tsr & current_core->gpr[rS]) ;
				return 0;
			case 20:current_core->tcr = current_core->gpr[rS];return 0;
			default:fprintf(stderr, "spr2=0x%x,spr1=0x%x,pc=0x%x,no such spr\n", spr2,spr1,current_core->pc);break;

		}
		break;
	case 12:
		if(spr1 >= 16 && spr1 < 32){
			current_core->ivor[spr1 - 16] = current_core->gpr[rS];
			return 0;
		}
		switch (spr1){
			default:fprintf(stderr, "spr2=0x%x,spr1=0x%x,pc=0x%x,no such spr\n", spr2,spr1,current_core->pc);break;
		}
		break;
	case 16:
		switch (spr1) {
		case 0: 
			current_core->spefscr = current_core->gpr[rS]; 
			return 0;
		case 16:
			current_core->ibatu[0] = current_core->gpr[rS];
			current_core->ibat_bl17[0] = ~(BATU_BL(current_core->ibatu[0])<<17);
			return 0;
		case 17:
			current_core->ibatl[0] = current_core->gpr[rS];
			return 0;
		case 18:
			current_core->ibatu[1] = current_core->gpr[rS];
			current_core->ibat_bl17[1] = ~(BATU_BL(current_core->ibatu[1])<<17);
			return 0;
		case 19:
			current_core->ibatl[1] = current_core->gpr[rS];
			return 0;
		case 20:
			current_core->ibatu[2] = current_core->gpr[rS];
			current_core->ibat_bl17[2] = ~(BATU_BL(current_core->ibatu[2])<<17);
			return 0;
		case 21:
			current_core->ibatl[2] = current_core->gpr[rS];
			return 0;
		case 22:
			current_core->ibatu[3] = current_core->gpr[rS];
			current_core->ibat_bl17[3] = ~(BATU_BL(current_core->ibatu[3])<<17);
			return 0;
		case 23:
			current_core->ibatl[3] = current_core->gpr[rS];
			return 0;
		case 24:
			current_core->dbatu[0] = current_core->gpr[rS];
			current_core->dbat_bl17[0] = ~(BATU_BL(current_core->dbatu[0])<<17);
			return 0;
		case 25:
			current_core->dbatl[0] = current_core->gpr[rS];
			return 0;
		case 26:
			current_core->dbatu[1] = current_core->gpr[rS];
			current_core->dbat_bl17[1] = ~(BATU_BL(current_core->dbatu[1])<<17);
			return 0;
		case 27:
			current_core->dbatl[1] = current_core->gpr[rS];
			return 0;
		case 28:
			current_core->dbatu[2] = current_core->gpr[rS];
			current_core->dbat_bl17[2] = ~(BATU_BL(current_core->dbatu[2])<<17);
			return 0;
		case 29:
			current_core->dbatl[2] = current_core->gpr[rS];
			return 0;
		case 30:
			current_core->dbatu[3] = current_core->gpr[rS];
			current_core->dbat_bl17[3] = ~(BATU_BL(current_core->dbatu[3])<<17);
			return 0;
		case 31:
			current_core->dbatl[3] = current_core->gpr[rS];
			return 0;
		default:fprintf(stderr, "spr2=0x%x,spr1=0x%x,pc=0x%x,no such spr\n", spr2,spr1,current_core->pc);break;
		}
		break;
	case 17:
		switch(spr1){
		printf("YUAN:func=%s,line=%d, write_e600_BAT", __func__, __LINE__);
		case 16://LCH
			current_core->e600_ibatu[0] = current_core->gpr[rS];
			return 0;
		case 17://LCH
			current_core->e600_ibatl[0] = current_core->gpr[rS];
			return 0;
		case 18://LCH
			current_core->e600_ibatu[1] = current_core->gpr[rS];
			return 0;
		case 19://LCH
			current_core->e600_ibatl[1] = current_core->gpr[rS];
			return 0;
		case 20://LCH
			current_core->e600_ibatu[2] = current_core->gpr[rS];
			return 0;
		case 21://LCH
			current_core->e600_ibatl[2] = current_core->gpr[rS];
			return 0;
		case 22://LCH
			current_core->e600_ibatu[3] = current_core->gpr[rS];
			return 0;
		case 23://LCH
			current_core->e600_ibatl[3] = current_core->gpr[rS];
			return 0;
		case 24://LCH
			current_core->e600_dbatu[0] = current_core->gpr[rS];
			return 0;
		case 25://LCH
			current_core->e600_dbatl[0] = current_core->gpr[rS];
			return 0;
		case 26://LCH
			current_core->e600_dbatu[1] = current_core->gpr[rS];
			return 0;
		case 27://LCH
			current_core->e600_dbatl[1] = current_core->gpr[rS];
			return 0;
		case 28://LCH
			current_core->e600_dbatu[2] = current_core->gpr[rS];
			return 0;
		case 29://LCH
			current_core->e600_dbatl[2] = current_core->gpr[rS];
			return 0;
		case 30://LCH
			current_core->e600_dbatu[3] = current_core->gpr[rS];
			return 0;
		case 31://LCH
			current_core->e600_dbatl[3] = current_core->gpr[rS];
			return 0;

			/*
		case 26:
			current_core->mcsrr[0] = current_core->gpr[rS];
			return;
		case 27:
                        current_core->mcsrr[1] = current_core->gpr[rS];
                        return;
		case 28:
			current_core->mcsr = current_core->gpr[rS];
			return;
			*/
		default:fprintf(stderr, "spr2=0x%x,spr1=0x%x,pc=0x%x,no such spr\n", spr2,spr1,current_core->pc);break;
		}

		break;
	case 19:
		switch(spr1){
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
			default:fprintf(stderr, "spr2=0x%x,spr1=0x%x,pc=0x%x,no such spr\n", spr2,spr1,current_core->pc);break;
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
		switch(spr1) {
		case 20: 
			current_core->e600_tlbmiss = current_core->gpr[rS];
			return 0;
		case 21: 
			current_core->e600_pte[0] = current_core->gpr[rS];
			return 0;
		case 22: 
			current_core->e600_pte[1] = current_core->gpr[rS];
			return 0;
		}
		return 0;
	case 31:
		switch (spr1) {
		case 16:
//			PPC_OPC_WARN("write(%08x) to spr %d:%d (HID0) not supported! @%08x\n", current_core->gpr[rS], spr1, spr2, current_core->pc);
			current_core->hid[0] = current_core->gpr[rS];
			//printf("YUAN:func=%s, line=%d, current_core->hid[0]=0x%x\n", __func__, __LINE__, current_core->hid[0]);
			return 0;
		case 17: return 0;
		case 18:
			PPC_OPC_ERR("write(%08x) to spr %d:%d (IABR) not supported!\n", current_core->gpr[rS], spr1, spr2);
			return 0;
		case 19:
                        current_core->l1csr[1] = current_core->gpr[rS];
                        return 0;
		case 20:
                        current_core->iac[0] = current_core->gpr[rS];
                        return 0;

		case 21:
			PPC_OPC_ERR("write(%08x) to spr %d:%d (DABR) not supported!\n", current_core->gpr[rS], spr1, spr2);
			return 0;
		case 22:
			current_core->e600_msscr0 = current_core->gpr[rS];
			return 0;
		case 23:
			current_core->e600_msssr0 = current_core->gpr[rS];
			return 0;
		case 24:
			current_core->e600_ldstcr = current_core->gpr[rS];
			return 0;	
		case 27:
			PPC_OPC_WARN("write(%08x) to spr %d:%d (ICTC) not supported!\n", current_core->gpr[rS], spr1, spr2);
			return 0;
		case 28:
//			PPC_OPC_WARN("write(%08x) to spr %d:%d (THRM1) not supported!\n", current_core->gpr[rS], spr1, spr2);
			return 0;
		case 29:
//			PPC_OPC_WARN("write(%08x) to spr %d:%d (THRM2) not supported!\n", current_core->gpr[rS], spr1, spr2);
			return 0;
		case 30:
//			PPC_OPC_WARN("write(%08x) to spr %d:%d (THRM3) not supported!\n", current_core->gpr[rS], spr1, spr2);
			return 0;
		case 31: return 0;
		default:fprintf(stderr, "spr2=0x%x,spr1=0x%x,pc=0x%x,no such spr\n", spr2,spr1,current_core->pc);break;
		}
	}
	fprintf(stderr, "unknown mtspr: %i:%i\n", spr1, spr2);
	fprintf(stderr, "pc=0x%x\n",current_core->pc);
	skyeye_exit(-1);
}
/*
 *	mfspr		Move from Special-Purpose Register
 *	.567
 */
static int opc_mfspr_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	e500_core_t* current_core = get_current_core();
	int rD, spr1, spr2;
	PPC_OPC_TEMPL_XO(instr, rD, spr1, spr2);
	if (current_core->msr & MSR_PR) {
		//ppc_exception(current_core, PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV, 0);
		if(!(spr2 == 0 && spr1 == 8)) /* read lr*/
			printf("Warning, execute mfspr in user mode, pc=0x%x\n", current_core->pc);
		//return;
	}
	debug(DEBUG_TRANSLATE, "In %s, spr2=%d, spr1=%d\n", __func__, spr2, spr1);
	switch(spr2) {
	case 0:
		switch (spr1) {
		case 1: LET(rD, RS(XER_REGNUM)); return 0;
		case 8: LET(rD, RS(LR_REGNUM)); return 0;
		case 9: LET(rD, RS(CTR_REGNUM)); return 0;

		case 18: current_core->gpr[rD] = current_core->dsisr; return 0;
		case 19: current_core->gpr[rD] = current_core->dar; return 0;
		case 22: {
			current_core->dec = current_core->pdec / TB_TO_PTB_FACTOR;
			current_core->gpr[rD] = current_core->dec;
			return 0;
		}
		case 25: current_core->gpr[rD] = current_core->sdr1; return 0;
		case 26: current_core->gpr[rD] = current_core->srr[0]; return 0;
		case 27: current_core->gpr[rD] = current_core->srr[1]; return 0;
		}
		break;
	case 1:
		switch(spr1) {
			case 16:
				current_core->gpr[rD] = current_core->mmu.pid[0];
                                //printf("read pid0 0x%x,pc=0x%x\n", current_core->gpr[rD],current_core->pc);
                                return 0;
			case 29: current_core->gpr[rD] = current_core->dear;return 0;
			case 30: current_core->gpr[rD] = current_core->esr; return 0;
			default:fprintf(stderr, "spr2=0x%x,spr1=0x%x,pc=0x%x,no such spr\n", spr2,spr1,current_core->pc);break;

		}
		break;
	case 8:
		switch (spr1) {
		case 12: {
			/*
			current_core->tb = current_core->ptb / TB_TO_PTB_FACTOR;
			current_core->gpr[rD] = current_core->tb;
			*/
			current_core->gpr[rD] = current_core->tbl;
			return 0;
		}
		case 13: {
			/*
			current_core->tb = current_core->ptb / TB_TO_PTB_FACTOR;
			current_core->gpr[rD] = current_core->tb >> 32;
			*/
			current_core->gpr[rD] = current_core->tbu;
			return 0;
		}
		case 0: current_core->gpr[rD] = current_core->vrsave; return 0;
		case 16: current_core->gpr[rD] = current_core->sprg[0]; return 0;
		case 1:
		case 17: current_core->gpr[rD] = current_core->sprg[1]; return 0;
		case 2:
		case 18: current_core->gpr[rD] = current_core->sprg[2]; return 0;
		case 3:
		case 19: current_core->gpr[rD] = current_core->sprg[3]; return 0;
		case 4:
		case 20: current_core->gpr[rD] = current_core->sprg[4]; return 0;
		case 5:
                case 21: current_core->gpr[rD] = current_core->sprg[5]; return 0;
		case 6:
                case 22: current_core->gpr[rD] = current_core->sprg[6]; return 0;
                case 23:
		case 7: 
			current_core->gpr[rD] = current_core->sprg[7]; return 0;

		case 26: 
			current_core->gpr[rD] = current_core->ear; return 0;
		case 30:
			//printf("In %s, read pir=0x%x,pc=0x%x\n", __FUNCTION__, current_core->pir, current_core->pc);
			current_core->gpr[rD] = current_core->pir; 
			return 0;
		case 31: LET(rD, RS(PVR_REGNUM)); return 0;
		default:
			fprintf(stderr, "unknown mfspr: %i:%i\n", spr1, spr2);
		        fprintf(stderr, "pc=0x%x\n", current_core->pc);
        		skyeye_exit(-1);

		}
		break;
	case 9:
		switch(spr1) {
			case 16:current_core->gpr[rD] = current_core->dbsr; return 0;
			case 20:current_core->gpr[rD] = current_core->dbcr[0]; return 0;
                        case 21:current_core->gpr[rD] = current_core->dbcr[1]; return 0;
                        case 22:current_core->gpr[rD] = current_core->dbcr[2]; return 0;
                        case 28:current_core->gpr[rD] = current_core->dac[0]; return 0;
                        case 29:current_core->gpr[rD] = current_core->dac[1]; return 0;
		}
		break;
	case 10:
		switch(spr1){
			case 20:current_core->gpr[rD] = current_core->tcr; return 0;
			default:break;
		}
		break;
	case 16:
		switch (spr1) {
		case 0: current_core->gpr[rD] = current_core->spefscr; return 0;
		case 16: current_core->gpr[rD] = current_core->ibatu[0]; return 0;
		case 17: current_core->gpr[rD] = current_core->ibatl[0]; return 0;
		case 18: current_core->gpr[rD] = current_core->ibatu[1]; return 0;
		case 19: current_core->gpr[rD] = current_core->ibatl[1]; return 0;
		case 20: current_core->gpr[rD] = current_core->ibatu[2]; return 0;
		case 21: current_core->gpr[rD] = current_core->ibatl[2]; return 0;
		case 22: current_core->gpr[rD] = current_core->ibatu[3]; return 0;
		case 23: current_core->gpr[rD] = current_core->ibatl[3]; return 0;
		case 24: current_core->gpr[rD] = current_core->dbatu[0]; return 0;
		case 25: current_core->gpr[rD] = current_core->dbatl[0]; return 0;
		case 26: current_core->gpr[rD] = current_core->dbatu[1]; return 0;
		case 27: current_core->gpr[rD] = current_core->dbatl[1]; return 0;
		case 28: current_core->gpr[rD] = current_core->dbatu[2]; return 0;
		case 29: current_core->gpr[rD] = current_core->dbatl[2]; return 0;
		case 30: current_core->gpr[rD] = current_core->dbatu[3]; return 0;
		case 31: current_core->gpr[rD] = current_core->dbatl[3]; return 0;
		}
		break;
	case 17://LCH
		switch (spr1) {
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
		}
		break;
	case 19:
                switch(spr1) {
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

                }
                break;
	case 21:
                switch(spr1) {
                        case 17:current_core->gpr[rD] = current_core->mmu.tlbcfg[1]; return 0;
                }
                break;
	case 29:
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
		break;
	case 31:
		switch (spr1) {
		case 16:
//			PPC_OPC_WARN("read from spr %d:%d (HID0) not supported!\n", spr1, spr2);
			current_core->gpr[rD] = current_core->hid[0];
			return 0;
		case 17:
			PPC_OPC_WARN("read from spr %d:%d (HID1) not supported!\n", spr1, spr2);
			current_core->gpr[rD] = current_core->hid[1];
			return 0;
		case 18:
			current_core->gpr[rD] = 0;
			return 0;
		case 19:
			current_core->gpr[rD] = current_core->e600_ictrl;
			return 0;
		case 20:
			current_core->gpr[rD] = current_core->e600_ldstdb;
			return 0;
		case 21:
			current_core->gpr[rD] = 0;
			return 0;
		case 22:
			current_core->gpr[rD] = 0;
			return 0;
		case 23:
			current_core->gpr[rD] = 0;
			return 0;
		case 24:
			current_core->gpr[rD] = 0;
			return 0;
		case 25:
			PPC_OPC_WARN("read from spr %d:%d (L2CR) not supported! (from %08x)\n", spr1, spr2, current_core->pc);
			current_core->gpr[rD] = 0;
			return 0;
		case 27:
			PPC_OPC_WARN("read from spr %d:%d (ICTC) not supported!\n", spr1, spr2);
			current_core->gpr[rD] = 0;
			return 0;
		case 28:
//			PPC_OPC_WARN("read from spr %d:%d (THRM1) not supported!\n", spr1, spr2);
			current_core->gpr[rD] = 0;
			return 0;
		case 29:
//			PPC_OPC_WARN("read from spr %d:%d (THRM2) not supported!\n", spr1, spr2);
			current_core->gpr[rD] = 0;
			return 0;
		case 30:
//			PPC_OPC_WARN("read from spr %d:%d (THRM3) not supported!\n", spr1, spr2);
			current_core->gpr[rD] = 0;
			return 0;
		case 31:
//			PPC_OPC_WARN("read from spr %d:%d (???) not supported!\n", spr1, spr2);
			current_core->gpr[rD] = 0;
			return 0;
		}
		break;
	}
	fprintf(stderr, "unknown mfspr: %i:%i\n", spr1, spr2);
	fprintf(stderr, "pc=0x%x\n", current_core->pc);
	skyeye_exit(-1);
	//SINGLESTEP("invalid mfspr\n");
	return -1;
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
	int i;
	Value * n_value = CONST(0);
	Value * old_n_value = CONST(0);
	Value * v_value = R(rS);
	for(i = 0; i < 32; i++){
		n_value = SELECT(ICMP_EQ(AND(LSHR(v_value, CONST(i)), CONST(1)), CONST(1)), CONST(i), old_n_value);
		old_n_value = n_value;
	}
	old_n_value = SELECT(ICMP_EQ(old_n_value, CONST(0)), CONST(-1), old_n_value);
	LET(rA, SUB(CONST(31), old_n_value));
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
	c = SELECT(ICMP_EQ(R(rA), R(rB)), CONST(2), c);
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
	LET(rA, R(rS));
	SELECT(ICMP_NE(AND(R(rA), CONST(0x8000)), CONST(0)), OR(R(rA), CONST(0xffff0000)), AND(R(rA), CONST(~0xffff0000)));
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
static int opc_lwarx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rA, rD, rB;
	PPC_OPC_TEMPL_X(instr, rD, rA, rB);
	uint32 r;
	Value* addr = ADD(SELECT(ICMP_NE(CONST(rA), CONST(0)), R(rA), CONST(0)), R(rB));
	Value* result = arch_read_memory(cpu, bb, addr, 0, 32);
	LET(rD, result);
	LETS(RESERVE_REGNUM, result);
	LETS(HAVE_RESERVATION_REGNUM, CONST(1));
	return 0;
}
/*
 *	stwcx.		Store Word Conditional Indexed
 *	.661
 */
int opc_stwcx__tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	*tag = TAG_CONDITIONAL;
	*next_pc = phys_pc + PPC_INSN_SIZE;
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
	Value* addr = ADD(SELECT(ICMP_NE(CONST(rA), CONST(0)), R(rA), CONST(0)), R(rB));
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
	*new_pc = NEW_PC_NONE;
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
	Value* shift = SHL(CONST(1), UREM(CONST(SH),CONST(32)));
	LETS(XER_REGNUM, SELECT(ICMP_NE(AND(R(rA), shift), CONST(0)), OR(RS(XER_REGNUM), CONST(XER_CA)), RS(XER_REGNUM)));
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
	Value* addr = ADD(SELECT(ICMP_NE(CONST(rA), CONST(0)), R(rA), CONST(0)), R(rB));
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
	LET(rD, AND(result, CONST(0x000000ff)));
	return 0;
}
/*
 *	stbx		Store Byte Indexed
 *	.635
 */
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
	LET(rD, AND(result, CONST(0x0000ffff)));
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
 *	dcbtst		Data Cache Block Touch for Store
 *	.463
 */
static int opc_dcbtst_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	// NO-OP
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
	Value* result = arch_read_memory(cpu, bb, addr, 0, 16);
	result = AND(result, CONST(0x0000ffff));
	LET(rD, SELECT(ICMP_NE(AND(result, CONST(0x8000)), CONST(0)), OR(result, CONST(0xffff0000)), result));
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
	LET(rD, ADD(XOR(R(rA), CONST(-1)), ADD(R(rB), CONST(1))));
	Value* cond = ppc_dyncom_carry_3(cpu, bb, XOR(R(rA), CONST(-1)), R(rB), CONST(1));
	LETS(XER_REGNUM, SELECT(cond, OR(RS(XER_REGNUM), CONST(XER_CA)), AND(RS(XER_REGNUM), CONST(~XER_CA))));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rD);
	}
	NOT_TESTED();
	return 0;
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
	LET(rD, ADD(R(rA), ADD(R(rB), ca)));
	Value* cond = ppc_dyncom_carry_3(cpu, bb, R(rA), R(rB), ca);
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
	LET(rD, ADD(R(rA), R(rB)));
	Value* cond = ICMP_ULT(R(rD), R(rA));
	LETS(XER_REGNUM, SELECT(cond, OR(RS(XER_REGNUM), CONST(XER_CA)), AND(RS(XER_REGNUM), CONST(~XER_CA))));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rD);
	}
	NOT_TESTED();
	return 0;
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
	opc_default_tag,
	opc_lwarx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_lwzx_func = {
	opc_default_tag,
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
ppc_opc_func_t ppc_opc_dcbst_func;
ppc_opc_func_t ppc_opc_lwzux_func;
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
ppc_opc_func_t ppc_opc_iseleq_func;
ppc_opc_func_t ppc_opc_mfmsr_func;
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
ppc_opc_func_t ppc_opc_mtmsr_func;
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

ppc_opc_func_t ppc_opc_subfzex_func;//+
ppc_opc_func_t ppc_opc_addzex_func = {
	opc_default_tag,
	opc_addzex_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_mtsr_func;
ppc_opc_func_t ppc_opc_stbx_func = {
	opc_default_tag,
	opc_stbx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_subfmex_func;//+
ppc_opc_func_t ppc_opc_addmex_func;
ppc_opc_func_t ppc_opc_mullwx_func = {
	opc_default_tag,
	opc_mullwx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_mtsrin_func;
ppc_opc_func_t ppc_opc_dcbtst_func = {
	opc_default_tag,
	opc_dcbtst_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_stbux_func;
ppc_opc_func_t ppc_opc_addx_func = {
	opc_default_tag,
	opc_addx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_dcbt_func;
ppc_opc_func_t ppc_opc_lhzx_func = {
	opc_default_tag,
	opc_lhzx_translate,
	opc_invalid_translate_cond,
};

ppc_opc_func_t ppc_opc_eqvx_func;
ppc_opc_func_t ppc_opc_tlbie_func;
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
ppc_opc_func_t ppc_opc_isel_func;
ppc_opc_func_t ppc_opc_tlbia_func;
ppc_opc_func_t ppc_opc_lhaux_func;
ppc_opc_func_t ppc_opc_sthx_func = {
	opc_default_tag,
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
ppc_opc_func_t ppc_opc_tlbsync_func;
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
ppc_opc_func_t ppc_opc_mfsrin_func;
ppc_opc_func_t ppc_opc_stswx_func;
ppc_opc_func_t ppc_opc_stwbrx_func;
ppc_opc_func_t ppc_opc_stfsx_func;
ppc_opc_func_t ppc_opc_stfsux_func;
ppc_opc_func_t ppc_opc_stswi_func;
ppc_opc_func_t ppc_opc_stfdx_func;
ppc_opc_func_t ppc_opc_dcba_func;
ppc_opc_func_t ppc_opc_stfdux_func;
ppc_opc_func_t ppc_opc_tlbivax_func; /* TLB invalidated virtual address indexed */
ppc_opc_func_t ppc_opc_lhbrx_func;
ppc_opc_func_t ppc_opc_srawx_func;
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
ppc_opc_func_t ppc_opc_sthbrx_func;
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
ppc_opc_func_t ppc_opc_icbi_func;
ppc_opc_func_t ppc_opc_stfiwx_func;
ppc_opc_func_t ppc_opc_dcbz_func;
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
