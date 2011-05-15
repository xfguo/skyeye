/*
 *	PearPC
 *	ppc_opc.cc
 *
 *	Copyright (C) 2003 Sebastian Biallas (sb@biallas.net)
 *	Copyright (C) 2004 Dainel Foesch (dfoesch@cs.nmsu.edu)
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
#include "debug/tracers.h"
#include "cpu/debug.h"
#include "io/pic/pic.h"
#include "info.h"
*/

#include <stdio.h>
#include <skyeye_pref.h>
#include <skyeye.h>

#include "ppc_cpu.h"
#include "ppc_exc.h"
#include "ppc_e500_exc.h"
#include "ppc_mmu.h"
#include "ppc_opc.h"
#include "ppc_dec.h"
#include "tracers.h"
#include "debug.h"
#include "ppc_syscall.h"

void ppc_set_msr(uint32 newmsr)
{
	e500_core_t *current_core = get_current_core();
/*	if ((newmsr & MSR_EE) && !(current_core->msr & MSR_EE)) {
		if (pic_check_interrupt()) {
			current_core->exception_pending = true;
			current_core->ext_exception = true;
		}
	}*/
	ppc_mmu_tlb_invalidate(current_core);
#ifndef PPC_CPU_ENABLE_SINGLESTEP
	if (newmsr & MSR_SE) {
		PPC_CPU_WARN
		    ("MSR[SE] (singlestep enable) set, but compiled w/o SE support.\n");
	}
#else
	current_core->singlestep_ignore = True;
#endif
	if (newmsr & PPC_CPU_UNSUPPORTED_MSR_BITS) {
		PPC_CPU_ERR("unsupported bits in MSR set: %08x @%08x\n",
			    newmsr & PPC_CPU_UNSUPPORTED_MSR_BITS,
			    current_core->pc);
	}
	if (newmsr & MSR_POW) {
		// doze();
		newmsr &= ~MSR_POW;
	}
	current_core->msr = newmsr;

}

/*
 *	bx		Branch
 *	.435
 */
void ppc_opc_bx()
{
	e500_core_t *current_core = get_current_core();
	uint32 li;
	PPC_OPC_TEMPL_I(current_core->current_opc, li);
	if (!(current_core->current_opc & PPC_OPC_AA)) {
		li += current_core->pc;
	}
	if (current_core->current_opc & PPC_OPC_LK) {
		current_core->lr = current_core->pc + 4;
	}
	current_core->npc = li;
}

/*
 *	bcx		Branch Conditional
 *	.436
 */
void ppc_opc_bcx()
{
	e500_core_t *current_core = get_current_core();
	uint32 BO, BI, BD;
	PPC_OPC_TEMPL_B(current_core->current_opc, BO, BI, BD);
	if (!(BO & 4)) {
		current_core->ctr--;
	}
	bool_t bo2 = ((BO & 2) ? 1 : 0);
	bool_t bo8 = ((BO & 8) ? 1 : 0);	// branch condition true
	bool_t cr = ((current_core->cr & (1 << (31 - BI))) ? 1 : 0);
	if (((BO & 4) || ((current_core->ctr != 0) ^ bo2))
	    && ((BO & 16) || (!(cr ^ bo8)))) {
		if (!(current_core->current_opc & PPC_OPC_AA)) {
			BD += current_core->pc;
		}
		if (current_core->current_opc & PPC_OPC_LK) {
			current_core->lr = current_core->pc + 4;
		}
		current_core->npc = BD;
	}
	//fprintf(stderr,"in %s,BO=0x%x,BI=0x%x,BD=0x%x,cr=0x%x,cr^bo8=0x%x,ctr=0x%x,pc=0x%x\n",__FUNCTION__,BO,BI,BD,current_core->cr,(cr ^ bo8),current_core->ctr, current_core->pc);
}

/*
 *	bcctrx		Branch Conditional to Count Register
 *	.438
 */
void ppc_opc_bcctrx()
{
	e500_core_t *current_core = get_current_core();
	uint32 BO, BI, BD;
	PPC_OPC_TEMPL_XL(current_core->current_opc, BO, BI, BD);
	PPC_OPC_ASSERT(BD == 0);
	PPC_OPC_ASSERT(!(BO & 2));
	bool_t bo8 = ((BO & 8) ? 1 : 0);
	bool_t cr = ((current_core->cr & (1 << (31 - BI))) ? 1 : 0);
	if ((BO & 16) || (!(cr ^ bo8))) {
		if (current_core->current_opc & PPC_OPC_LK) {
			current_core->lr = current_core->pc + 4;
		}
		current_core->npc = current_core->ctr & 0xfffffffc;
	}
	//fprintf(stderr,"in %s,BO=0x%x,BI=0x%x,BD=0x%x,cr=0x%x,cr^bo8=0x%x,ctr=0x%x,pc=0x%x,npc=0x%x\n",__FUNCTION__,BO,BI,BD,current_core->cr,(cr ^ bo8),current_core->ctr, current_core->pc,current_core->npc);
}

/*
 *	bclrx		Branch Conditional to Link Register
 *	.440
 */
void ppc_opc_bclrx()
{
	e500_core_t *current_core = get_current_core();
	uint32 BO, BI, BD;
	PPC_OPC_TEMPL_XL(current_core->current_opc, BO, BI, BD);
	PPC_OPC_ASSERT(BD == 0);
	if (!(BO & 4)) {
		current_core->ctr--;
	}
	bool_t bo2 = ((BO & 2) ? 1 : 0);
	bool_t bo8 = ((BO & 8) ? 1 : 0);
	bool_t cr = ((current_core->cr & (1 << (31 - BI))) ? 1 : 0);
	if (((BO & 4) || ((current_core->ctr != 0) ^ bo2))
	    && ((BO & 16) || (!(cr ^ bo8)))) {
		BD = current_core->lr & 0xfffffffc;
		if (current_core->current_opc & PPC_OPC_LK) {
			current_core->lr = current_core->pc + 4;
		}
		current_core->npc = BD;
	}
}

/*
 *	dcbf		Data Cache Block Flush
 *	.458
 */
void ppc_opc_dcbf()
{
	// NO-OP
}

/*
 *	dcbi		Data Cache Block Invalidate
 *	.460
 */
void ppc_opc_dcbi()
{
	e500_core_t *current_core = get_current_core();
	if (current_core->msr & MSR_PR) {
		ppc_exception(current_core, PPC_EXC_PROGRAM,
			      PPC_EXC_PROGRAM_PRIV, 0);
		return;
	}
	// FIXME: check addr
}

/*
 *	dcbst		Data Cache Block Store
 *	.461
 */
void ppc_opc_dcbst()
{
	// NO-OP
}

/*
 *	dcbt		Data Cache Block Touch
 *	.462
 */
void ppc_opc_dcbt()
{
	// NO-OP
}

/*
 *	dcbtst		Data Cache Block Touch for Store
 *	.463
 */
void ppc_opc_dcbtst()
{
	// NO-OP
}

/*
 *	eciwx		External Control In Word Indexed
 *	.474
 */
void ppc_opc_eciwx()
{
	PPC_OPC_ERR("eciwx unimplemented.\n");
}

/*
 *	ecowx		External Control Out Word Indexed
 *	.476
 */
void ppc_opc_ecowx()
{
	PPC_OPC_ERR("ecowx unimplemented.\n");
}

/*
 *	eieio		Enforce In-Order Execution of I/O
 *	.478
 */
void ppc_opc_eieio()
{
	// NO-OP
}

/*
 *	icbi		Instruction Cache Block Invalidate
 *	.519
 */
void ppc_opc_icbi()
{
	// NO-OP
}

/*
 *	isync		Instruction Synchronize
 *	.520
 */
void ppc_opc_isync()
{
	// NO-OP
}

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

/*
 *	mcrf		Move Condition Register Field
 *	.561
 */
void ppc_opc_mcrf()
{
	e500_core_t *current_core = get_current_core();
	uint32 crD, crS, bla;
	PPC_OPC_TEMPL_X(current_core->current_opc, crD, crS, bla);
	// FIXME: bla == 0
	crD >>= 2;
	crS >>= 2;
	crD = 7 - crD;
	crS = 7 - crS;
	uint32 c = (current_core->cr >> (crS * 4)) & 0xf;
	current_core->cr &= ppc_cmp_and_mask[crD];
	current_core->cr |= c << (crD * 4);
}

/*
 *	mcrfs		Move to Condition Register from FPSCR
 *	.562
 */
void ppc_opc_mcrfs()
{
	PPC_OPC_ERR("mcrfs unimplemented.\n");
}

/*
 *	mcrxr		Move to Condition Register from XER
 *	.563
 */
void ppc_opc_mcrxr()
{
	PPC_OPC_ERR("mcrxr unimplemented.\n");
}

/*
 *	mfcr		Move from Condition Register
 *	.564
 */
void ppc_opc_mfcr()
{
	e500_core_t *current_core = get_current_core();
	int rD, rA, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rD, rA, rB);
	PPC_OPC_ASSERT(rA == 0 && rB == 0);
	current_core->gpr[rD] = current_core->cr;
}

/*
 *	mffs		Move from FPSCR
 *	.565
 */
void ppc_opc_mffsx()
{
	e500_core_t *current_core = get_current_core();
	int frD, rA, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, frD, rA, rB);
	PPC_OPC_ASSERT(rA == 0 && rB == 0);
	current_core->fpr[frD] = current_core->fpscr;
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr1 flags
		PPC_OPC_ERR("mffs. unimplemented.\n");
	}
}

/*
 *	mfmsr		Move from Machine State Register
 *	.566
 */
void ppc_opc_mfmsr()
{
	e500_core_t *current_core = get_current_core();
	if (current_core->msr & MSR_PR) {
		ppc_exception(current_core, PPC_EXC_PROGRAM,
			      PPC_EXC_PROGRAM_PRIV, 0);
		return;
	}
	int rD, rA, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rD, rA, rB);
	PPC_OPC_ASSERT((rA == 0) && (rB == 0));
	current_core->gpr[rD] = current_core->msr;
}

/*
 *	mfspr		Move from Special-Purpose Register
 *	.567
 */
void ppc_opc_mfspr()
{
	e500_core_t *current_core = get_current_core();
	int rD, spr1, spr2;
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, spr1, spr2);
	if (current_core->msr & MSR_PR) {
		//ppc_exception(current_core, PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV, 0);
		if (!(spr2 == 0 && spr1 == 8))	/* read lr */
			printf("Warning, execute mfspr in user mode, pc=0x%x\n",
			       current_core->pc);
		//return;
	}
	//fprintf(stderr, "spr2=0x%x,spr1=0x%x,pc=0x%x,no such spr\n", spr2,spr1,current_core->pc);
	switch (spr2) {
	case 0:
		switch (spr1) {
		case 1:
			current_core->gpr[rD] = current_core->xer;
			return;
		case 8:
			current_core->gpr[rD] = current_core->lr;
			return;
		case 9:
			current_core->gpr[rD] = current_core->ctr;
			return;

		case 18:
			current_core->gpr[rD] = current_core->dsisr;
			return;
		case 19:
			current_core->gpr[rD] = current_core->dar;
			return;
		case 22:{
				current_core->dec =
				    current_core->pdec / TB_TO_PTB_FACTOR;
				current_core->gpr[rD] = current_core->dec;
				return;
			}
		case 25:
			current_core->gpr[rD] = current_core->sdr1;
			return;
		case 26:
			current_core->gpr[rD] = current_core->srr[0];
			return;
		case 27:
			current_core->gpr[rD] = current_core->srr[1];
			return;
		}
		break;
	case 1:
		switch (spr1) {
		case 16:
			current_core->gpr[rD] = current_core->mmu.pid[0];
			//printf("read pid0 0x%x,pc=0x%x\n", current_core->gpr[rD],current_core->pc);
			return;
		case 29:
			current_core->gpr[rD] = current_core->dear;
			return;
		case 30:
			current_core->gpr[rD] = current_core->esr;
			return;
		default:
			fprintf(stderr,
				"spr2=0x%x,spr1=0x%x,pc=0x%x,no such spr\n",
				spr2, spr1, current_core->pc);
			break;

		}
		break;
	case 8:
		switch (spr1) {
		case 12:{
				/*
				   current_core->tb = current_core->ptb / TB_TO_PTB_FACTOR;
				   current_core->gpr[rD] = current_core->tb;
				 */
				current_core->gpr[rD] = current_core->tbl;
				return;
			}
		case 13:{
				/*
				   current_core->tb = current_core->ptb / TB_TO_PTB_FACTOR;
				   current_core->gpr[rD] = current_core->tb >> 32;
				 */
				current_core->gpr[rD] = current_core->tbu;
				return;
			}
		case 0:
			current_core->gpr[rD] = current_core->vrsave;
			return;
		case 16:
			current_core->gpr[rD] = current_core->sprg[0];
			return;
		case 1:
		case 17:
			current_core->gpr[rD] = current_core->sprg[1];
			return;
		case 2:
		case 18:
			current_core->gpr[rD] = current_core->sprg[2];
			return;
		case 3:
		case 19:
			current_core->gpr[rD] = current_core->sprg[3];
			return;
		case 4:
		case 20:
			current_core->gpr[rD] = current_core->sprg[4];
			return;
		case 5:
		case 21:
			current_core->gpr[rD] = current_core->sprg[5];
			return;
		case 6:
		case 22:
			current_core->gpr[rD] = current_core->sprg[6];
			return;
		case 23:
		case 7:
			current_core->gpr[rD] = current_core->sprg[7];
			return;

		case 26:
			current_core->gpr[rD] = current_core->ear;
			return;
		case 30:
			//printf("In %s, read pir=0x%x,pc=0x%x\n", __FUNCTION__, current_core->pir, current_core->pc);
			current_core->gpr[rD] = current_core->pir;
			return;
		case 31:
			current_core->gpr[rD] = current_core->pvr;
			return;
		default:
			fprintf(stderr, "unknown mfspr: %i:%i\n", spr1, spr2);
			fprintf(stderr, "pc=0x%x\n", current_core->pc);
			skyeye_exit(-1);

		}
		break;
	case 9:
		switch (spr1) {
		case 16:
			current_core->gpr[rD] = current_core->dbsr;
			return;
		case 20:
			current_core->gpr[rD] = current_core->dbcr[0];
			return;
		case 21:
			current_core->gpr[rD] = current_core->dbcr[1];
			return;
		case 22:
			current_core->gpr[rD] = current_core->dbcr[2];
			return;
		case 28:
			current_core->gpr[rD] = current_core->dac[0];
			return;
		case 29:
			current_core->gpr[rD] = current_core->dac[1];
			return;
		}
		break;
	case 10:
		switch (spr1) {
		case 20:
			current_core->gpr[rD] = current_core->tcr;
			return;
		default:
			break;
		}
		break;
	case 16:
		switch (spr1) {
		case 0:
			current_core->gpr[rD] = current_core->spefscr;
			return;
		case 16:
			current_core->gpr[rD] = current_core->ibatu[0];
			return;
		case 17:
			current_core->gpr[rD] = current_core->ibatl[0];
			return;
		case 18:
			current_core->gpr[rD] = current_core->ibatu[1];
			return;
		case 19:
			current_core->gpr[rD] = current_core->ibatl[1];
			return;
		case 20:
			current_core->gpr[rD] = current_core->ibatu[2];
			return;
		case 21:
			current_core->gpr[rD] = current_core->ibatl[2];
			return;
		case 22:
			current_core->gpr[rD] = current_core->ibatu[3];
			return;
		case 23:
			current_core->gpr[rD] = current_core->ibatl[3];
			return;
		case 24:
			current_core->gpr[rD] = current_core->dbatu[0];
			return;
		case 25:
			current_core->gpr[rD] = current_core->dbatl[0];
			return;
		case 26:
			current_core->gpr[rD] = current_core->dbatu[1];
			return;
		case 27:
			current_core->gpr[rD] = current_core->dbatl[1];
			return;
		case 28:
			current_core->gpr[rD] = current_core->dbatu[2];
			return;
		case 29:
			current_core->gpr[rD] = current_core->dbatl[2];
			return;
		case 30:
			current_core->gpr[rD] = current_core->dbatu[3];
			return;
		case 31:
			current_core->gpr[rD] = current_core->dbatl[3];
			return;
		}
		break;
	case 17:		//LCH
		switch (spr1) {
		case 16:
			current_core->gpr[rD] = current_core->e600_ibatu[0];
			return;
		case 17:
			current_core->gpr[rD] = current_core->e600_ibatl[0];
			return;
		case 18:
			current_core->gpr[rD] = current_core->e600_ibatu[1];
			return;
		case 19:
			current_core->gpr[rD] = current_core->e600_ibatl[1];
			return;
		case 20:
			current_core->gpr[rD] = current_core->e600_ibatu[2];
			return;
		case 21:
			current_core->gpr[rD] = current_core->e600_ibatl[2];
			return;
		case 22:
			current_core->gpr[rD] = current_core->e600_ibatu[3];
			return;
		case 23:
			current_core->gpr[rD] = current_core->e600_ibatl[3];
			return;
		case 24:
			current_core->gpr[rD] = current_core->e600_dbatu[0];
			return;
		case 25:
			current_core->gpr[rD] = current_core->e600_dbatl[0];
			return;
		case 26:
			current_core->gpr[rD] = current_core->e600_dbatu[1];
			return;
		case 27:
			current_core->gpr[rD] = current_core->e600_dbatl[1];
			return;
		case 28:
			current_core->gpr[rD] = current_core->e600_dbatu[2];
			return;
		case 29:
			current_core->gpr[rD] = current_core->e600_dbatl[2];
			return;
		case 30:
			current_core->gpr[rD] = current_core->e600_dbatu[3];
			return;
		case 31:
			current_core->gpr[rD] = current_core->e600_dbatl[3];
			return;

		}
		break;
	case 19:
		switch (spr1) {
		case 16:
			current_core->gpr[rD] = current_core->mmu.mas[0];
			return;
		case 17:
			current_core->gpr[rD] = current_core->mmu.mas[1];
			return;
		case 18:
			current_core->gpr[rD] = current_core->mmu.mas[2];
			return;
		case 19:
			current_core->gpr[rD] = current_core->mmu.mas[3];
			return;
		case 20:
			current_core->gpr[rD] = current_core->mmu.mas[4];
			return;
		case 22:
			current_core->gpr[rD] = current_core->mmu.mas[6];
			return;
		case 25:
			current_core->gpr[rD] = current_core->mmu.pid[1];
			//printf("read pid 1 0x%x\n", current_core->gpr[rD]);
			return;
		case 26:
			current_core->gpr[rD] = current_core->mmu.pid[2];
			//printf("read pid 2 0x%x\n", current_core->gpr[rD]);
			return;

		}
		break;
	case 21:
		switch (spr1) {
		case 17:
			current_core->gpr[rD] = current_core->mmu.tlbcfg[1];
			return;
		}
		break;
	case 29:
		switch (spr1) {
		case 16:
			current_core->gpr[rD] = 0;
			return;
		case 17:
			current_core->gpr[rD] = 0;
			return;
		case 18:
			current_core->gpr[rD] = 0;
			return;
		case 24:
			current_core->gpr[rD] = 0;
			return;
		case 25:
			current_core->gpr[rD] = 0;
			return;
		case 26:
			current_core->gpr[rD] = 0;
			return;
		case 28:
			current_core->gpr[rD] = 0;
			return;
		case 29:
			current_core->gpr[rD] = 0;
			return;
		case 30:
			current_core->gpr[rD] = 0;
			return;
		}
		break;
	case 31:
		switch (spr1) {
		case 16:
//                      PPC_OPC_WARN("read from spr %d:%d (HID0) not supported!\n", spr1, spr2);
			current_core->gpr[rD] = current_core->hid[0];
			return;
		case 17:
			PPC_OPC_WARN
			    ("read from spr %d:%d (HID1) not supported!\n",
			     spr1, spr2);
			current_core->gpr[rD] = current_core->hid[1];
			return;
		case 18:
			current_core->gpr[rD] = 0;
			return;
		case 19:
			current_core->gpr[rD] = current_core->e600_ictrl;
			return;
		case 20:
			current_core->gpr[rD] = current_core->e600_ldstdb;
			return;
		case 21:
			current_core->gpr[rD] = 0;
			return;
		case 22:
			current_core->gpr[rD] = 0;
			return;
		case 23:
			current_core->gpr[rD] = 0;
			return;
		case 24:
			current_core->gpr[rD] = 0;
			return;
		case 25:
			PPC_OPC_WARN
			    ("read from spr %d:%d (L2CR) not supported! (from %08x)\n",
			     spr1, spr2, current_core->pc);
			current_core->gpr[rD] = 0;
			return;
		case 27:
			PPC_OPC_WARN
			    ("read from spr %d:%d (ICTC) not supported!\n",
			     spr1, spr2);
			current_core->gpr[rD] = 0;
			return;
		case 28:
//                      PPC_OPC_WARN("read from spr %d:%d (THRM1) not supported!\n", spr1, spr2);
			current_core->gpr[rD] = 0;
			return;
		case 29:
//                      PPC_OPC_WARN("read from spr %d:%d (THRM2) not supported!\n", spr1, spr2);
			current_core->gpr[rD] = 0;
			return;
		case 30:
//                      PPC_OPC_WARN("read from spr %d:%d (THRM3) not supported!\n", spr1, spr2);
			current_core->gpr[rD] = 0;
			return;
		case 31:
//                      PPC_OPC_WARN("read from spr %d:%d (???) not supported!\n", spr1, spr2);
			current_core->gpr[rD] = 0;
			return;
		}
		break;
	}
	fprintf(stderr, "unknown mfspr: %i:%i\n", spr1, spr2);
	fprintf(stderr, "pc=0x%x\n", current_core->pc);
	skyeye_exit(-1);
	//SINGLESTEP("invalid mfspr\n");
}

/*
 *	mfsr		Move from Segment Register
 *	.570
 */
void ppc_opc_mfsr()
{
	e500_core_t *current_core = get_current_core();
	if (current_core->msr & MSR_PR) {
		ppc_exception(current_core, PPC_EXC_PROGRAM,
			      PPC_EXC_PROGRAM_PRIV, 0);
		return;
	}
	int rD, SR, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rD, SR, rB);
	// FIXME: check insn
	current_core->gpr[rD] = current_core->sr[SR & 0xf];
}

/*
 *	mfsrin		Move from Segment Register Indirect
 *	.572
 */
void ppc_opc_mfsrin()
{
	e500_core_t *current_core = get_current_core();
	if (current_core->msr & MSR_PR) {
		ppc_exception(current_core, PPC_EXC_PROGRAM,
			      PPC_EXC_PROGRAM_PRIV, 0);
		return;
	}
	int rD, rA, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rD, rA, rB);
	// FIXME: check insn
	current_core->gpr[rD] = current_core->sr[current_core->gpr[rB] >> 28];
}

/*
 *	mftb		Move from Time Base
 *	.574
 */
void ppc_opc_mftb()
{
	e500_core_t *current_core = get_current_core();
	int rD, spr1, spr2;
	PPC_OPC_TEMPL_X(current_core->current_opc, rD, spr1, spr2);
	switch (spr2) {
	case 8:
		switch (spr1) {
		case 12:{
				current_core->tb =
				    current_core->ptb / TB_TO_PTB_FACTOR;
				current_core->gpr[rD] = current_core->tb;
				return;
			}
		case 13:{
				current_core->tb =
				    current_core->ptb / TB_TO_PTB_FACTOR;
				current_core->gpr[rD] = current_core->tb >> 32;
				return;
			}
		}
		break;
	}
}

/*
 *	mtcrf		Move to Condition Register Fields
 *	.576
 */
void ppc_opc_mtcrf()
{
	e500_core_t *current_core = get_current_core();

	int rS;
	uint32 crm;
	uint32 CRM;
	PPC_OPC_TEMPL_XFX(current_core->current_opc, rS, crm);
	CRM =
	    ((crm & 0x80) ? 0xf0000000 : 0) | ((crm & 0x40) ? 0x0f000000 : 0) |
	    ((crm & 0x20) ? 0x00f00000 : 0) | ((crm & 0x10) ? 0x000f0000 : 0) |
	    ((crm & 0x08) ? 0x0000f000 : 0) | ((crm & 0x04) ? 0x00000f00 : 0) |
	    ((crm & 0x02) ? 0x000000f0 : 0) | ((crm & 0x01) ? 0x0000000f : 0);
	current_core->cr =
	    (current_core->gpr[rS] & CRM) | (current_core->cr & ~CRM);
}

/*
 *	mtfsb0x		Move to FPSCR Bit 0
 *	.577
 */
void ppc_opc_mtfsb0x()
{
	e500_core_t *current_core = get_current_core();
	int crbD, n1, n2;
	PPC_OPC_TEMPL_X(current_core->current_opc, crbD, n1, n2);
	if (crbD != 1 && crbD != 2) {
		current_core->fpscr &= ~(1 << (31 - crbD));
	}
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr1 flags
		PPC_OPC_ERR("mtfsb0. unimplemented.\n");
	}
}

/*
 *	mtfsb1x		Move to FPSCR Bit 1
 *	.578
 */
void ppc_opc_mtfsb1x()
{
	e500_core_t *current_core = get_current_core();
	int crbD, n1, n2;
	PPC_OPC_TEMPL_X(current_core->current_opc, crbD, n1, n2);
	if (crbD != 1 && crbD != 2) {
		current_core->fpscr |= 1 << (31 - crbD);
	}
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr1 flags
		PPC_OPC_ERR("mtfsb1. unimplemented.\n");
	}
}

/*
 *	mtfsfx		Move to FPSCR Fields
 *	.579
 */
void ppc_opc_mtfsfx()
{
	e500_core_t *current_core = get_current_core();
	int frB;
	uint32 fm, FM;
	PPC_OPC_TEMPL_XFL(current_core->current_opc, frB, fm);
	FM = ((fm & 0x80) ? 0xf0000000 : 0) | ((fm & 0x40) ? 0x0f000000 : 0) |
	    ((fm & 0x20) ? 0x00f00000 : 0) | ((fm & 0x10) ? 0x000f0000 : 0) |
	    ((fm & 0x08) ? 0x0000f000 : 0) | ((fm & 0x04) ? 0x00000f00 : 0) |
	    ((fm & 0x02) ? 0x000000f0 : 0) | ((fm & 0x01) ? 0x0000000f : 0);
	current_core->fpscr =
	    (current_core->fpr[frB] & FM) | (current_core->fpscr & ~FM);
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr1 flags
		PPC_OPC_ERR("mtfsf. unimplemented.\n");
	}
}

/*
 *	mtfsfix		Move to FPSCR Field Immediate
 *	.580
 */
void ppc_opc_mtfsfix()
{
	e500_core_t *current_core = get_current_core();
	int crfD, n1;
	uint32 imm;
	PPC_OPC_TEMPL_X(current_core->current_opc, crfD, n1, imm);
	crfD >>= 2;
	imm >>= 1;
	crfD = 7 - crfD;
	current_core->fpscr &= ppc_cmp_and_mask[crfD];
	current_core->fpscr |= imm << (crfD * 4);
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr1 flags
		PPC_OPC_ERR("mtfsfi. unimplemented.\n");
	}
}

/*
 *	mtmsr		Move to Machine State Register
 *	.581
 */
void ppc_opc_mtmsr()
{
	e500_core_t *current_core = get_current_core();
	if (current_core->msr & MSR_PR) {
		ppc_exception(current_core, PPC_EXC_PROGRAM,
			      PPC_EXC_PROGRAM_PRIV, 0);
		return;
	}
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	PPC_OPC_ASSERT((rA == 0) && (rB == 0));
	ppc_set_msr(current_core->gpr[rS]);
}

/*
 *	mtspr		Move to Special-Purpose Register
 *	.584
 */
void ppc_opc_mtspr()
{
	e500_core_t *current_core = get_current_core();
	int rS, spr1, spr2;
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, spr1, spr2);
	/*
	   if(!(spr1 == 8 && spr2 == 0)) 
	   if( !(spr1 == 9 && spr2 == 0))
	   fprintf(stderr, "In %s, opc=0x%x,spr1=%d,spr2=%d\n", __FUNCTION__, current_core->current_opc, spr1, spr2);
	 */
	switch (spr2) {
	case 0:
		switch (spr1) {
		case 1:
			current_core->xer = current_core->gpr[rS];
			return;
		case 8:
			current_core->lr = current_core->gpr[rS];
			return;
		case 9:
			current_core->ctr = current_core->gpr[rS];
			return;
		}
		break;

	case 8:		//altivec makes this register unpriviledged
		if (spr1 == 0) {
			current_core->vrsave = current_core->gpr[rS];
			return;
		}
		switch (spr1) {
		case 28:
			current_core->tbl = current_core->gpr[rS];
			return;
		case 29:
			current_core->tbu = current_core->gpr[rS];
			return;
		}
		break;
	}
	if (current_core->msr & MSR_PR) {
		//      ppc_exception(current_core, PPC_EXC_PROGRAM, PPC_EXC_PROGRAM_PRIV, 0);
		//printf("Warning: execute mtspr in user mode\n");
		//return;
	}
	switch (spr2) {
	case 0:
		switch (spr1) {
/*		case 18: current_core->gpr[rD] = current_core->dsisr; return;
		case 19: current_core->gpr[rD] = current_core->dar; return;*/
		case 22:{
				//printf("In %s, write DEC=0x%x\n", __FUNCTION__, current_core->gpr[rS]);
				current_core->dec = current_core->gpr[rS];
				current_core->pdec = current_core->dec;
				current_core->pdec *= TB_TO_PTB_FACTOR;
				return;
			}
		case 25:
			if (!ppc_mmu_set_sdr1(current_core->gpr[rS], True)) {
				PPC_OPC_ERR("cannot set sdr1\n");
			}
			return;
		case 26:
			current_core->srr[0] = current_core->gpr[rS];
			return;
		case 27:
			current_core->srr[1] = current_core->gpr[rS];
			return;
		default:
			fprintf(stderr,
				"spr2=0x%x,spr1=0x%x,pc=0x%x,no such spr\n",
				spr2, spr1, current_core->pc);
			break;
		}
		break;
	case 1:
		switch (spr1) {
		case 16:
			current_core->mmu.pid[0] = current_core->gpr[rS];
			//printf("write pid0=0x%x\n", current_core->gpr[rS]);
			return;
		case 26:
			current_core->csrr[0] = current_core->gpr[rS];
			return;
		case 27:
			current_core->csrr[1] = current_core->gpr[rS];
			return;
		case 29:
			current_core->dear = current_core->gpr[rS];
			return;
		case 30:
			current_core->esr = current_core->gpr[rS];
			return;
		case 31:
			current_core->ivpr = current_core->gpr[rS];
			return;
		default:
			fprintf(stderr,
				"spr2=0x%x,spr1=0x%x,pc=0x%x,no such spr\n",
				spr2, spr1, current_core->pc);
			break;
		}
	case 8:
		switch (spr1) {
		case 16:
			current_core->sprg[0] = current_core->gpr[rS];
			return;
		case 17:
			current_core->sprg[1] = current_core->gpr[rS];
			return;
		case 18:
			current_core->sprg[2] = current_core->gpr[rS];
			return;
		case 19:
			current_core->sprg[3] = current_core->gpr[rS];
			return;
		case 20:
			current_core->sprg[4] = current_core->gpr[rS];
			return;
		case 21:
			current_core->sprg[5] = current_core->gpr[rS];
			return;
		case 22:
			current_core->sprg[6] = current_core->gpr[rS];
			return;
		case 23:
			current_core->sprg[7] = current_core->gpr[rS];
			return;
/*		case 26: current_core->gpr[rD] = current_core->ear; return;
		case 31: current_core->gpr[rD] = current_core->pvr; return;*/
		default:
			fprintf(stderr,
				"spr2=0x%x,spr1=0x%x,pc=0x%x,no such spr\n",
				spr2, spr1, current_core->pc);
			break;
		}
		break;
	case 9:
		switch (spr1) {
		case 16:
			current_core->dbsr = current_core->gpr[rS];
			return;
		case 20:
			current_core->dbcr[0] = current_core->gpr[rS];
			return;
		case 21:
			current_core->dbcr[1] = current_core->gpr[rS];
			return;
		case 22:
			current_core->dbcr[2] = current_core->gpr[rS];
			return;
		case 28:
			current_core->dac[0] = current_core->gpr[rS];
			return;
		case 29:
			current_core->dac[1] = current_core->gpr[rS];
			return;
		default:
			fprintf(stderr,
				"spr2=0x%x,spr1=0x%x,pc=0x%x,no such spr\n",
				spr2, spr1, current_core->pc);
			break;
		}
		break;
	case 10:
		switch (spr1) {

		case 16:
			/* W1C, write one to clear */
			current_core->tsr &=
			    ~(current_core->tsr & current_core->gpr[rS]);
			return;
		case 20:
			current_core->tcr = current_core->gpr[rS];
			return;
		default:
			fprintf(stderr,
				"spr2=0x%x,spr1=0x%x,pc=0x%x,no such spr\n",
				spr2, spr1, current_core->pc);
			break;

		}
		break;
	case 12:
		if (spr1 >= 16 && spr1 < 32) {
			current_core->ivor[spr1 - 16] = current_core->gpr[rS];
			return;
		}
		switch (spr1) {
		default:
			fprintf(stderr,
				"spr2=0x%x,spr1=0x%x,pc=0x%x,no such spr\n",
				spr2, spr1, current_core->pc);
			break;
		}
		break;
	case 16:
		switch (spr1) {
		case 0:
			current_core->spefscr = current_core->gpr[rS];
			return;
		case 16:
			current_core->ibatu[0] = current_core->gpr[rS];
			current_core->ibat_bl17[0] =
			    ~(BATU_BL(current_core->ibatu[0]) << 17);
			return;
		case 17:
			current_core->ibatl[0] = current_core->gpr[rS];
			return;
		case 18:
			current_core->ibatu[1] = current_core->gpr[rS];
			current_core->ibat_bl17[1] =
			    ~(BATU_BL(current_core->ibatu[1]) << 17);
			return;
		case 19:
			current_core->ibatl[1] = current_core->gpr[rS];
			return;
		case 20:
			current_core->ibatu[2] = current_core->gpr[rS];
			current_core->ibat_bl17[2] =
			    ~(BATU_BL(current_core->ibatu[2]) << 17);
			return;
		case 21:
			current_core->ibatl[2] = current_core->gpr[rS];
			return;
		case 22:
			current_core->ibatu[3] = current_core->gpr[rS];
			current_core->ibat_bl17[3] =
			    ~(BATU_BL(current_core->ibatu[3]) << 17);
			return;
		case 23:
			current_core->ibatl[3] = current_core->gpr[rS];
			return;
		case 24:
			current_core->dbatu[0] = current_core->gpr[rS];
			current_core->dbat_bl17[0] =
			    ~(BATU_BL(current_core->dbatu[0]) << 17);
			return;
		case 25:
			current_core->dbatl[0] = current_core->gpr[rS];
			return;
		case 26:
			current_core->dbatu[1] = current_core->gpr[rS];
			current_core->dbat_bl17[1] =
			    ~(BATU_BL(current_core->dbatu[1]) << 17);
			return;
		case 27:
			current_core->dbatl[1] = current_core->gpr[rS];
			return;
		case 28:
			current_core->dbatu[2] = current_core->gpr[rS];
			current_core->dbat_bl17[2] =
			    ~(BATU_BL(current_core->dbatu[2]) << 17);
			return;
		case 29:
			current_core->dbatl[2] = current_core->gpr[rS];
			return;
		case 30:
			current_core->dbatu[3] = current_core->gpr[rS];
			current_core->dbat_bl17[3] =
			    ~(BATU_BL(current_core->dbatu[3]) << 17);
			return;
		case 31:
			current_core->dbatl[3] = current_core->gpr[rS];
			return;
		default:
			fprintf(stderr,
				"spr2=0x%x,spr1=0x%x,pc=0x%x,no such spr\n",
				spr2, spr1, current_core->pc);
			break;
		}
		break;
	case 17:
		switch (spr1) {
			printf("YUAN:func=%s,line=%d, write_e600_BAT", __func__,
			       __LINE__);
		case 16:	//LCH
			current_core->e600_ibatu[0] = current_core->gpr[rS];
			return;
		case 17:	//LCH
			current_core->e600_ibatl[0] = current_core->gpr[rS];
			return;
		case 18:	//LCH
			current_core->e600_ibatu[1] = current_core->gpr[rS];
			return;
		case 19:	//LCH
			current_core->e600_ibatl[1] = current_core->gpr[rS];
			return;
		case 20:	//LCH
			current_core->e600_ibatu[2] = current_core->gpr[rS];
			return;
		case 21:	//LCH
			current_core->e600_ibatl[2] = current_core->gpr[rS];
			return;
		case 22:	//LCH
			current_core->e600_ibatu[3] = current_core->gpr[rS];
			return;
		case 23:	//LCH
			current_core->e600_ibatl[3] = current_core->gpr[rS];
			return;
		case 24:	//LCH
			current_core->e600_dbatu[0] = current_core->gpr[rS];
			return;
		case 25:	//LCH
			current_core->e600_dbatl[0] = current_core->gpr[rS];
			return;
		case 26:	//LCH
			current_core->e600_dbatu[1] = current_core->gpr[rS];
			return;
		case 27:	//LCH
			current_core->e600_dbatl[1] = current_core->gpr[rS];
			return;
		case 28:	//LCH
			current_core->e600_dbatu[2] = current_core->gpr[rS];
			return;
		case 29:	//LCH
			current_core->e600_dbatl[2] = current_core->gpr[rS];
			return;
		case 30:	//LCH
			current_core->e600_dbatu[3] = current_core->gpr[rS];
			return;
		case 31:	//LCH
			current_core->e600_dbatl[3] = current_core->gpr[rS];
			return;

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
		default:
			fprintf(stderr,
				"spr2=0x%x,spr1=0x%x,pc=0x%x,no such spr\n",
				spr2, spr1, current_core->pc);
			break;
		}

		break;
	case 19:
		switch (spr1) {
		case 16:
			current_core->mmu.mas[0] = current_core->gpr[rS];
			return;
		case 17:
			current_core->mmu.mas[1] = current_core->gpr[rS];
			return;
		case 18:
			current_core->mmu.mas[2] = current_core->gpr[rS];
			return;
		case 19:
			current_core->mmu.mas[3] = current_core->gpr[rS];
			return;
		case 20:
			current_core->mmu.mas[4] = current_core->gpr[rS];
			return;
		case 22:
			current_core->mmu.mas[6] = current_core->gpr[rS];
			return;
		case 25:
			current_core->mmu.pid[1] = current_core->gpr[rS];
			//printf("write pid 1 0x%x\n", current_core->gpr[rS]);
			return;
		case 26:
			current_core->mmu.pid[2] = current_core->gpr[rS];
			//printf("write pid 2 0x%x\n", current_core->gpr[rS]);
			return;
		default:
			fprintf(stderr,
				"spr2=0x%x,spr1=0x%x,pc=0x%x,no such spr\n",
				spr2, spr1, current_core->pc);
			break;
		}
		break;
	case 29:
		switch (spr1) {
		case 17:
			return;
		case 24:
			return;
		case 25:
			return;
		case 26:
			return;
		}
	case 30:		//LCH
		switch (spr1) {
		case 20:
			current_core->e600_tlbmiss = current_core->gpr[rS];
			return;
		case 21:
			current_core->e600_pte[0] = current_core->gpr[rS];
			return;
		case 22:
			current_core->e600_pte[1] = current_core->gpr[rS];
			return;
		}
		return;
	case 31:
		switch (spr1) {
		case 16:
//                      PPC_OPC_WARN("write(%08x) to spr %d:%d (HID0) not supported! @%08x\n", current_core->gpr[rS], spr1, spr2, current_core->pc);
			current_core->hid[0] = current_core->gpr[rS];
			//printf("YUAN:func=%s, line=%d, current_core->hid[0]=0x%x\n", __func__, __LINE__, current_core->hid[0]);
			return;
		case 17:
			return;
		case 18:
			PPC_OPC_ERR
			    ("write(%08x) to spr %d:%d (IABR) not supported!\n",
			     current_core->gpr[rS], spr1, spr2);
			return;
		case 19:
			current_core->l1csr[1] = current_core->gpr[rS];
			return;
		case 20:
			current_core->iac[0] = current_core->gpr[rS];
			return;

		case 21:
			PPC_OPC_ERR
			    ("write(%08x) to spr %d:%d (DABR) not supported!\n",
			     current_core->gpr[rS], spr1, spr2);
			return;
		case 22:
			current_core->e600_msscr0 = current_core->gpr[rS];
			return;
		case 23:
			current_core->e600_msssr0 = current_core->gpr[rS];
			return;
		case 24:
			current_core->e600_ldstcr = current_core->gpr[rS];
			return;
		case 25:
			current_core->e600_l2cr = current_core->gpr[rS];
			return;
		case 27:
			PPC_OPC_WARN
			    ("write(%08x) to spr %d:%d (ICTC) not supported!\n",
			     current_core->gpr[rS], spr1, spr2);
			return;
		case 28:
//                      PPC_OPC_WARN("write(%08x) to spr %d:%d (THRM1) not supported!\n", current_core->gpr[rS], spr1, spr2);
			return;
		case 29:
//                      PPC_OPC_WARN("write(%08x) to spr %d:%d (THRM2) not supported!\n", current_core->gpr[rS], spr1, spr2);
			return;
		case 30:
//                      PPC_OPC_WARN("write(%08x) to spr %d:%d (THRM3) not supported!\n", current_core->gpr[rS], spr1, spr2);
			return;
		case 31:
			return;
		default:
			fprintf(stderr,
				"spr2=0x%x,spr1=0x%x,pc=0x%x,no such spr\n",
				spr2, spr1, current_core->pc);
			break;
		}
	}
	fprintf(stderr, "unknown mtspr: %i:%i\n", spr1, spr2);
	fprintf(stderr, "pc=0x%x\n", current_core->pc);
	skyeye_exit(-1);
}

/*
 *	mtsr		Move to Segment Register
 *	.587
 */
void ppc_opc_mtsr()
{
	e500_core_t *current_core = get_current_core();
	if (current_core->msr & MSR_PR) {
		ppc_exception(current_core, PPC_EXC_PROGRAM,
			      PPC_EXC_PROGRAM_PRIV, 0);
		return;
	}
	int rS, SR, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, SR, rB);
	// FIXME: check insn
	current_core->sr[SR & 0xf] = current_core->gpr[rS];
}

/*
 *	mtsrin		Move to Segment Register Indirect
 *	.591
 */
void ppc_opc_mtsrin()
{
	e500_core_t *current_core = get_current_core();
	if (current_core->msr & MSR_PR) {
		ppc_exception(current_core, PPC_EXC_PROGRAM,
			      PPC_EXC_PROGRAM_PRIV, 0);
		return;
	}
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	// FIXME: check insn
	current_core->sr[current_core->gpr[rB] >> 28] = current_core->gpr[rS];
}

/*
 *	rfi		Return from Interrupt
 *	.607
 */
void ppc_opc_rfi()
{
	e500_core_t *current_core = get_current_core();
	if (current_core->msr & MSR_PR) {
		ppc_exception(current_core, PPC_EXC_PROGRAM,
			      PPC_EXC_PROGRAM_PRIV, 0);
		return;
	}
	ppc_set_msr(current_core->srr[1] & MSR_RFI_SAVE_MASK);
	current_core->npc = current_core->srr[0] & 0xfffffffc;
	//printf("In  %s, r7=0x%x, srr0=0x%x, srr1=0x%x\n", __FUNCTION__, current_core->gpr[7], current_core->srr[0], current_core->srr[1]);
}

/*
 *	sc		System Call
 *	.621
 */
//#include "io/graphic/gcard.h"
void ppc_opc_sc()
{
	e500_core_t *current_core = get_current_core();
	if (current_core->gpr[3] == 0x113724fa
	    && current_core->gpr[4] == 0x77810f9b) {
		//gcard_osi(0);
		return;
	}
	//ppc_exception(current_core, SYSCALL ,0 ,0);
	//e600_ppc_exception(current_core, PPC_EXC_SC ,0 ,0);
	sky_pref_t *pref = get_skyeye_pref();
	if (pref->user_mode_sim)
		ppc_syscall(current_core);
	else
		ppc_exception(current_core, current_core->syscall_number, 0, 0);
}

/*
 *	sync		Synchronize
 *	.672
 */
void ppc_opc_sync()
{
	// NO-OP
}

/*
 *	tlbie		Translation Lookaside Buffer Invalidate Entry
 *	.676
 */
void ppc_opc_tlbia()
{
	e500_core_t *current_core = get_current_core();
	if (current_core->msr & MSR_PR) {
		ppc_exception(current_core, PPC_EXC_PROGRAM,
			      PPC_EXC_PROGRAM_PRIV, 0);
		return;
	}
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	// FIXME: check rS.. for 0
	ppc_mmu_tlb_invalidate(current_core);
}

/*
 *	tlbie		Translation Lookaside Buffer Invalidate All
 *	.676
 */
void ppc_opc_tlbie()
{
	e500_core_t *current_core = get_current_core();
	if (current_core->msr & MSR_PR) {
		ppc_exception(current_core, PPC_EXC_PROGRAM,
			      PPC_EXC_PROGRAM_PRIV, 0);
		return;
	}
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	// FIXME: check rS.. for 0     
	ppc_mmu_tlb_invalidate(current_core);
}

/*
 *	tlbsync		Translation Lookaside Buffer Syncronize
 *	.677
 */
void ppc_opc_tlbsync()
{
	e500_core_t *current_core = get_current_core();
	if (current_core->msr & MSR_PR) {
		ppc_exception(current_core, PPC_EXC_PROGRAM,
			      PPC_EXC_PROGRAM_PRIV, 0);
		return;
	}
	int rS, rA, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	// FIXME: check rS.. for 0     
	ppc_mmu_tlb_invalidate(current_core);
}

/*
 *	tw		Trap Word
 *	.678
 */
void ppc_opc_tw()
{
	e500_core_t *current_core = get_current_core();
	int TO, rA, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, TO, rA, rB);
	uint32 a = current_core->gpr[rA];
	uint32 b = current_core->gpr[rB];
	if (((TO & 16) && ((sint32) a < (sint32) b))
	    || ((TO & 8) && ((sint32) a > (sint32) b))
	    || ((TO & 4) && (a == b))
	    || ((TO & 2) && (a < b))
	    || ((TO & 1) && (a > b))) {
		ppc_exception(current_core, PPC_EXC_PROGRAM,
			      PPC_EXC_PROGRAM_TRAP, 0);
	}
}

/*
 *	twi		Trap Word Immediate
 *	.679
 */
void ppc_opc_twi()
{
	e500_core_t *current_core = get_current_core();
	int TO, rA;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, TO, rA, imm);
	uint32 a = current_core->gpr[rA];
	if (((TO & 16) && ((sint32) a < (sint32) imm))
	    || ((TO & 8) && ((sint32) a > (sint32) imm))
	    || ((TO & 4) && (a == imm))
	    || ((TO & 2) && (a < imm))
	    || ((TO & 1) && (a > imm))) {
		ppc_exception(current_core, PROG, PPC_EXC_PROGRAM_TRAP, 0);
	}
}

/*	dcba		Data Cache Block Allocate
 *	.???
 */
void ppc_opc_dcba()
{
	/* NO-OP */
}
