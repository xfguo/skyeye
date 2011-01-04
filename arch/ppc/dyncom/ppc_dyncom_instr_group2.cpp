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

#define NOT_TESTED() do { printf("INSTRUCTION NOT TESTED:%s", __FUNCTION__); } while(0)
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
	e500_core_t* core = (e500_core_t *)cpu->cpu_data;
	PPC_OPC_TEMPL_XO(core->current_opc, rD, rA, rB);
	LET(rD, ADD(R(rA), R(rB)));
	//FIXME:
	return 0;
}
/*
 *	lwzx		Load Word and Zero Indexed
 *	.560
 */
static int opc_lwzx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	e500_core_t* current_core = get_current_core();
	int rA, rD, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rD, rA, rB);
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
		case 31: current_core->gpr[rD] = current_core->pvr; return 0;
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
	e500_core_t* current_core = get_current_core();
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
	LET(rA, SUB(SUB(CONST(32), n_value), CONST(1)));
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_dyncom_update_cr0(cpu, bb, rA);
	}
}

/* Interfaces */
ppc_opc_func_t ppc_opc_cmp_func = {
	opc_default_tag,
	opc_cmp_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_tw_func;
ppc_opc_func_t ppc_opc_subfcx_func;//+
ppc_opc_func_t ppc_opc_addcx_func;//+
ppc_opc_func_t ppc_opc_mulhwux_func;
ppc_opc_func_t ppc_opc_mfcr_func;
ppc_opc_func_t ppc_opc_lwarx_func;
ppc_opc_func_t ppc_opc_lwzx_func = {
	opc_default_tag,
	opc_lwzx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_slwx_func;
ppc_opc_func_t ppc_opc_cntlzwx_func = {
	opc_default_tag,
	opc_cntlzwx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_andx_func;
ppc_opc_func_t ppc_opc_cmpl_func;
ppc_opc_func_t ppc_opc_subfx_func;
ppc_opc_func_t ppc_opc_iselgt_func;
ppc_opc_func_t ppc_opc_dcbst_func;
ppc_opc_func_t ppc_opc_lwzux_func;
ppc_opc_func_t ppc_opc_andcx_func;
ppc_opc_func_t ppc_opc_mulhwx_func;
ppc_opc_func_t ppc_opc_iseleq_func;
ppc_opc_func_t ppc_opc_mfmsr_func;
ppc_opc_func_t ppc_opc_dcbf_func;
ppc_opc_func_t ppc_opc_lbzx_func;
ppc_opc_func_t ppc_opc_negx_func;
ppc_opc_func_t ppc_opc_lbzux_func;
ppc_opc_func_t ppc_opc_norx_func;
ppc_opc_func_t ppc_opc_wrtee_func;
ppc_opc_func_t ppc_opc_subfex_func;//+
ppc_opc_func_t ppc_opc_addex_func;//+
ppc_opc_func_t ppc_opc_mtcrf_func;
ppc_opc_func_t ppc_opc_mtmsr_func;
ppc_opc_func_t ppc_opc_stwcx__func;
ppc_opc_func_t ppc_opc_stwx_func;
ppc_opc_func_t ppc_opc_wrteei_func;
ppc_opc_func_t ppc_opc_dcbtls_func;
ppc_opc_func_t ppc_opc_stwux_func;
ppc_opc_func_t ppc_opc_subfzex_func;//+
ppc_opc_func_t ppc_opc_addzex_func;//+
ppc_opc_func_t ppc_opc_mtsr_func;
ppc_opc_func_t ppc_opc_stbx_func;
ppc_opc_func_t ppc_opc_subfmex_func;//+
ppc_opc_func_t ppc_opc_addmex_func;
ppc_opc_func_t ppc_opc_mullwx_func;//+
ppc_opc_func_t ppc_opc_mtsrin_func;
ppc_opc_func_t ppc_opc_dcbtst_func;
ppc_opc_func_t ppc_opc_stbux_func;
ppc_opc_func_t ppc_opc_addx_func = {
	opc_default_tag,
	opc_addx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_dcbt_func;
ppc_opc_func_t ppc_opc_lhzx_func;
ppc_opc_func_t ppc_opc_eqvx_func;
ppc_opc_func_t ppc_opc_tlbie_func;
ppc_opc_func_t ppc_opc_eciwx_func;
ppc_opc_func_t ppc_opc_lhzux_func;
ppc_opc_func_t ppc_opc_xorx_func;
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
ppc_opc_func_t ppc_opc_lhax_func;
ppc_opc_func_t ppc_opc_isel_func;
ppc_opc_func_t ppc_opc_tlbia_func;
ppc_opc_func_t ppc_opc_lhaux_func;
ppc_opc_func_t ppc_opc_sthx_func;
ppc_opc_func_t ppc_opc_orcx_func;
ppc_opc_func_t ppc_opc_ecowx_func;
ppc_opc_func_t ppc_opc_sthux_func;
ppc_opc_func_t ppc_opc_orx_func = {
	opc_default_tag,
	opc_orx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_divwux_func;//+
ppc_opc_func_t ppc_opc_dcbi_func;
ppc_opc_func_t ppc_opc_nandx_func;
ppc_opc_func_t ppc_opc_divwx_func;//+
ppc_opc_func_t ppc_opc_mcrxr_func;
ppc_opc_func_t ppc_opc_lswx_func;
ppc_opc_func_t ppc_opc_lwbrx_func;
ppc_opc_func_t ppc_opc_lfsx_func;
ppc_opc_func_t ppc_opc_srwx_func;
ppc_opc_func_t ppc_opc_tlbsync_func;
ppc_opc_func_t ppc_opc_lfsux_func;
ppc_opc_func_t ppc_opc_mfsr_func;
ppc_opc_func_t ppc_opc_lswi_func;
ppc_opc_func_t ppc_opc_sync_func;
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
ppc_opc_func_t ppc_opc_srawix_func;
ppc_opc_func_t ppc_opc_eieio_func;
ppc_opc_func_t ppc_opc_tlbsx_func;
ppc_opc_func_t ppc_opc_sthbrx_func;
ppc_opc_func_t ppc_opc_extshx_func;
ppc_opc_func_t ppc_opc_tlbrehi_func;
ppc_opc_func_t ppc_opc_extsbx_func;
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
