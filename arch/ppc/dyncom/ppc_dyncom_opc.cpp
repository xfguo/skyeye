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
#include "llvm/Instructions.h"
#include <skyeye_pref.h>
#include <skyeye_dyncom.h>
#include <dyncom/dyncom_llvm.h>
#include <dyncom/frontend.h>
#include <dyncom/tag.h>

#include "ppc_cpu.h"
#include "ppc_exc.h"
#include "ppc_e500_exc.h"
#include "ppc_mmu.h"
#include "ppc_opc.h"
#include "ppc_dec.h"
#include "tracers.h"
#include "debug.h"
#include "ppc_dyncom.h"
#include "ppc_dyncom_run.h"
#include "ppc_dyncom_dec.h"
#include "ppc_dyncom_debug.h"
#if 0
void ppc_set_msr(uint32 newmsr)
{
/*	if ((newmsr & MSR_EE) && !(current_core->msr & MSR_EE)) {
		if (pic_check_interrupt()) {
			current_core->exception_pending = true;
			current_core->ext_exception = true;
		}
	}*/
	ppc_mmu_tlb_invalidate();
#ifndef PPC_CPU_ENABLE_SINGLESTEP
	if (newmsr & MSR_SE) {
		PPC_CPU_WARN("MSR[SE] (singlestep enable) set, but compiled w/o SE support.\n");
	}
#else 
	current_core->singlestep_ignore = true;
#endif
	if (newmsr & PPC_CPU_UNSUPPORTED_MSR_BITS) {
		PPC_CPU_ERR("unsupported bits in MSR set: %08x @%08x\n", newmsr & PPC_CPU_UNSUPPORTED_MSR_BITS, current_core->pc);
	}
	if (newmsr & MSR_POW) {
		// doze();
		newmsr &= ~MSR_POW;
	}
	current_core->msr = newmsr;
	
}
#endif
/*
 *	bx		Branch
 *	.435
 */
int opc_bx_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	e500_core_t* current_core = get_core_from_dyncom_cpu(cpu);
	uint32 li;
	PPC_OPC_TEMPL_I(instr, li);
	*tag = TAG_BRANCH;
	/*if the branch target is out of the page, stop the tagging */
	if(li > get_end_of_page(current_core->pc)){
		*tag |= TAG_STOP;
	}
	*new_pc = li + phys_pc;
	debug(DEBUG_TAG, "In %s, new_pc=0x%x\n", __FUNCTION__, *new_pc);
	return PPC_INSN_SIZE;
}
static int opc_bx_translate(cpu_t* cpu, uint32_t instr, BasicBlock* bb)
{
	e500_core_t* current_core = get_core_from_dyncom_cpu(cpu);
	uint32 li;
	PPC_OPC_TEMPL_I(instr, li);
	if (!(instr & PPC_OPC_AA)) {
		//li += current_core->pc;
		arch_store(CONST(li + current_core->phys_pc), cpu->ptr_PHYS_PC, bb);
	}
	if (instr & PPC_OPC_LK) {
		//current_core->lr = current_core->pc + 4;
		LET(LR_REGNUM, CONST(4 + current_core->phys_pc));
	}
	//current_core->npc = li;
	return PPC_INSN_SIZE;
}
ppc_opc_func_t ppc_opc_bx_func = {
	opc_bx_tag,
	opc_bx_translate,
	opc_invalid_translate_cond,
};
/*
 *	bcx		Branch Conditional
 *	.436
 */
int opc_bcx_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	e500_core_t* current_core = get_core_from_dyncom_cpu(cpu);
	*tag = TAG_COND_BRANCH;
	*new_pc = NEW_PC_NONE;
	debug(DEBUG_TAG, "In %s, new_pc=0x%x\n", __FUNCTION__, *new_pc);
	return PPC_INSN_SIZE;
}
static int opc_bcx_translate(cpu_t* cpu, uint32_t instr, BasicBlock* bb)
{
	return 0;
#if 0
	e500_core_t* current_core = get_current_core();
	uint32 BO, BI, BD;
	PPC_OPC_TEMPL_B(current_core->current_opc, BO, BI, BD);
	if (!(BO & 4)) {
		current_core->ctr--;
	}
	bool_t bo2 = ((BO & 2)?1:0);
	bool_t bo8 = ((BO & 8)?1:0); // branch condition true
	bool_t cr = ((current_core->cr & (1<<(31-BI)))?1:0) ;
	if (((BO & 4) || ((current_core->ctr!=0) ^ bo2))
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
#endif
}
ppc_opc_func_t ppc_opc_bcx_func = {
	opc_bcx_tag,
	opc_bcx_translate,
	opc_invalid_translate_cond,
};

