/*
 *	ppc_dyncom_alu.cpp - Implementation of some translation function for 
 *	powerpc dyncom. 
 *
 *	Copyright (C) 2010 Michael.kang (blackfin.kang@gmail.com)
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/*
 * 05/16/2010   Michael.Kang  <blackfin.kang@gmail.com>
 */

#include "debug.h"
#include "tracers.h"
#include "ppc_dyncom_alu.h"
#include "ppc_dyncom_dec.h"
#include "ppc_exc.h"
#include "ppc_cpu.h"
#include "ppc_dyncom_opc.h"
#include "ppc_dyncom_alu.h"
#include "ppc_tools.h"
#include "ppc_mmu.h"

#include "llvm/Instructions.h"
#include <dyncom/dyncom_llvm.h>
#include <dyncom/frontend.h>
#include "dyncom/basicblock.h"
#include "skyeye.h"

/*
 *	addx		Add
 *	.422
 */
void ppc_opc_addx(cpu_t* cpu, BasicBlock *bb)
{
	int rD, rA, rB;
	e500_core_t* core = (e500_core_t *)cpu->cpu_data;
	PPC_OPC_TEMPL_XO(core->current_opc, rD, rA, rB);
	LET(rD, ADD(R(rA), R(rB)));

	//core->gpr[rD] = core->gpr[rA] + core->gpr[rB];
#if 0 /* FIXME, not implemented */
	if (core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(cpu, bb, current_core->gpr[rD]);
	}
#endif
}

/*
 *      addis           Add Immediate Shifted
 *      .428
 */
int opc_addis_translate(cpu_t *cpu, addr_t real_addr, BasicBlock *bb)
{
        int rD, rA;
        uint32 imm;
        e500_core_t* current_core = get_current_core();
        PPC_OPC_TEMPL_D_Shift16(current_core->current_opc, rD, rA, imm);
	//current_core->gpr[rD] = (rA ? current_core->gpr[rA] : 0) + imm;
	if(rA)
		LET(rD, ADD(R(rA), CONST(imm)));
	else
		LET(rD, CONST(imm));
}
ppc_opc_func_t ppc_opc_addis_func = {
	opc_default_tag,
	opc_addis_translate,
	opc_invalid_translate_cond,
};

/*
 *	addi		Add Immediate
 *	.425
 */
int opc_addi_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rD, rA;
	uint32 imm;
	e500_core_t* current_core = get_current_core();
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rD, rA, imm);
	if(rA)
		LET(rD, ADD(R(rA), CONST(imm)));
	else
		LET(rD, CONST(imm));
}
ppc_opc_func_t ppc_opc_addi_func = {
	opc_default_tag,
	opc_addi_translate,
	opc_invalid_translate_cond,
};

/*
 *	orx		OR
 *	.603
 */
static int opc_orx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(instr, rS, rA, rB);
	LET(rA, OR(R(rS), R(rB)));
	//current_core->gpr[rA] = current_core->gpr[rS] | current_core->gpr[rB];
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		fprintf(stderr, "##### In %s,cr0 is not updated.\n", __FUNCTION__);
		//ppc_update_cr0(current_core->gpr[rA]);
	}
}
ppc_opc_func_t ppc_opc_orx_func = {
	opc_default_tag,
	opc_orx_translate,
	opc_invalid_translate_cond,
};

/*
 *	rlwinmx		Rotate Left Word Immediate then AND with Mask
 *	.618
 */
static int opc_rlwinmx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rS, rA, SH, MB, ME;
	PPC_OPC_TEMPL_M(instr, rS, rA, SH, MB, ME);
	//uint32 v = ppc_word_rotl(current_core->gpr[rS], SH);
	Value* v = ROTL(R(rS), SH);
	uint32 mask = ppc_mask(MB, ME);
	//current_core->gpr[rA] = v & mask;
	LET(rA, AND(v, CONST(mask)));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		fprintf(stderr, "Not implemented update cr\n");
		ppc_dyncom_update_cr0(cpu, bb, rA);
	}
}
ppc_opc_func_t ppc_opc_rlwinmx_func = {
	opc_default_tag,
	opc_rlwinmx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_twi_func;		//  3
 ppc_opc_func_t ppc_opc_mulli_func;		//  7
 ppc_opc_func_t ppc_opc_subfic_func;	//  8
 ppc_opc_func_t ppc_opc_cmpli_func;
/*
 *	cmpi		Compare Immediate
 *	.443
 */
static int opc_cmpi_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	uint32 cr;
	int rA;
	e500_core_t* current_core = get_current_core();
	printf("In %s not implemented\n", __FUNCTION__);
	return 0;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, cr, rA, imm);
	cr >>= 2;
	sint32 a = current_core->gpr[rA];
	sint32 b = imm;
	uint32 c;
/*	if (!VALGRIND_CHECK_READABLE(a, sizeof a)) {
		ht_printf("%08x <--i\n", current_core->pc);
//		SINGLESTEP("");
	}*/
	if (a < b) {
		c = 8;
	} else if (a > b) {
		c = 4;
	} else {
		c = 2;
	}
	if (current_core->xer & XER_SO) c |= 1;
	cr = 7-cr;
//	current_core->cr &= ppc_cmp_and_mask[cr];
	current_core->cr |= c<<(cr*4);
	//fprintf(stderr,"in %s,rA=%d,gpr[rA]=0x%d,im=%d,c=%d\n",__FUNCTION__,rA,current_core->gpr[rA],imm,c);
}
 ppc_opc_func_t ppc_opc_cmpi_func = {
	opc_default_tag,
	opc_cmpi_translate,
	opc_invalid_translate_cond,
};

 ppc_opc_func_t ppc_opc_addic_func;		
 ppc_opc_func_t ppc_opc_addic__func;		
 ppc_opc_func_t ppc_opc_sc_func;
 ppc_opc_func_t ppc_opc_rlwimix_func;
 ppc_opc_func_t ppc_opc_rlwnmx_func;
 ppc_opc_func_t ppc_opc_ori_func;
 ppc_opc_func_t ppc_opc_oris_func;
 ppc_opc_func_t ppc_opc_xori_func;
 ppc_opc_func_t ppc_opc_xoris_func;
 ppc_opc_func_t ppc_opc_andi__func;
 ppc_opc_func_t ppc_opc_andis__func;

/*
 *	lwz		Load Word and Zero
 *	.557
 */
static int opc_lwz_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	e500_core_t* current_core = get_current_core();
	int rA, rD;
	uint32 imm;
	printf("In %s not implemented\n", __FUNCTION__);
	return 0;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rD, rA, imm);
	uint32 r;

	int ret = ppc_read_effective_word((rA?current_core->gpr[rA]:0)+imm, &r);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rD] = r;
	}	

}
 ppc_opc_func_t ppc_opc_lwz_func = {
	opc_default_tag,
	opc_lwz_translate,
	opc_invalid_translate_cond,
};


/*
 *	lwzu		Load Word and Zero with Update
 *	.558
 */
static int opc_lwzu_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	e500_core_t* current_core = get_current_core();
	int rA, rD;
	uint32 imm;
	printf("In %s not implemented\n", __FUNCTION__);
	return 0;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rD, rA, imm);
	// FIXME: check rA!=0 && rA!=rD
	uint32 r;
	int ret = ppc_read_effective_word(current_core->gpr[rA]+imm, &r);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rA] += imm;
		current_core->gpr[rD] = r;
	}	
}

ppc_opc_func_t ppc_opc_lwzu_func = {
	opc_default_tag,
	opc_lwzu_translate,
	opc_invalid_translate_cond,

};
 ppc_opc_func_t ppc_opc_lbz_func;
 ppc_opc_func_t ppc_opc_lbzu_func;

/*
 *	stw		Store Word
 *	.659
 */
static int opc_stw_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rA, rS;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(instr, rS, rA, imm);
	printf("In %s not implemented\n", __FUNCTION__);
	return 0;
	e500_core_t* current_core = get_current_core();
	ppc_write_effective_word((rA?current_core->gpr[rA]:0)+imm, current_core->gpr[rS]) != PPC_MMU_FATAL;
}

ppc_opc_func_t ppc_opc_stw_func = {
	opc_default_tag,
	opc_stw_translate,
	opc_invalid_translate_cond,
};

/*
 *	stwu		Store Word with Update
 *	.663
 */
static int opc_stwu_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rA, rS;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(instr, rS, rA, imm);
	// FIXME: check rA!=0
	/* FIXME: no MMU now */
	arch_write_memory(cpu, bb, ADD(R(rA), CONST(imm)), R(rS), 32);
	return 0;
}

 ppc_opc_func_t ppc_opc_stwu_func = {
	opc_default_tag,
	opc_stwu_translate,
	opc_invalid_translate_cond,
};
 ppc_opc_func_t ppc_opc_stb_func;
 ppc_opc_func_t ppc_opc_stbu_func;
 ppc_opc_func_t ppc_opc_lhz_func;
 ppc_opc_func_t ppc_opc_lhzu_func;
 ppc_opc_func_t ppc_opc_lha_func;
 ppc_opc_func_t ppc_opc_lhau_func;
 ppc_opc_func_t ppc_opc_sth_func;
 ppc_opc_func_t ppc_opc_sthu_func;
 ppc_opc_func_t ppc_opc_lmw_func;
 ppc_opc_func_t ppc_opc_stmw_func;
 ppc_opc_func_t ppc_opc_lfs_func;
 ppc_opc_func_t ppc_opc_lfsu_func;
 ppc_opc_func_t ppc_opc_lfd_func;
 ppc_opc_func_t ppc_opc_lfdu_func;
 ppc_opc_func_t ppc_opc_stfs_func;
 ppc_opc_func_t ppc_opc_stfsu_func;
 ppc_opc_func_t ppc_opc_stfd_func;
 ppc_opc_func_t ppc_opc_stfdu_func;

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
		case 1: LET(SR(XER_REGNUM), R(rS)); return 0;
		case 8:	LET(SR(LR_REGNUM), R(rS)); return 0;
		case 9:	LET(SR(CTR_REGNUM), R(rS)); return 0;
		}
		break;
	
	case 8:	//altivec makes this register unpriviledged
		if (spr1 == 0) {
			current_core->vrsave = current_core->gpr[rS]; 
			return 0;
		}
		switch(spr1){
			case 28:
				current_core->tbl = current_core->gpr[rS];
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

ppc_opc_func_t ppc_opc_mtspr_func = {
	opc_default_tag,
	opc_mtspr_translate,
	opc_invalid_translate_cond,
};
/*
 *	mfspr		Move from Special-Purpose Register
 *	.567
 */
static int opc_mfspr_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	e500_core_t* current_core = get_current_core();
	int rD, spr1, spr2;
	printf("In %s not implemented\n", __FUNCTION__);
	return 0;
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, spr1, spr2);
	if (current_core->msr & MSR_PR) {
		//ppc_exception(current_core, PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV, 0);
		if(!(spr2 == 0 && spr1 == 8)) /* read lr*/
			printf("Warning, execute mfspr in user mode, pc=0x%x\n", current_core->pc);
		//return;
	}
	//fprintf(stderr, "spr2=0x%x,spr1=0x%x,pc=0x%x,no such spr\n", spr2,spr1,current_core->pc);
	switch(spr2) {
	case 0:
		switch (spr1) {
		case 1: current_core->gpr[rD] = current_core->xer; return 0;
		case 8: current_core->gpr[rD] = current_core->lr; return 0;
		case 9: current_core->gpr[rD] = current_core->ctr; return 0;

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

ppc_opc_func_t ppc_opc_mfspr_func = {
	opc_default_tag,
	opc_mfspr_translate,
	opc_invalid_translate_cond,
};
