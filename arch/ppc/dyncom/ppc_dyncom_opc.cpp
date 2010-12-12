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

#include "ppc_cpu.h"
#include "ppc_exc.h"
#include "ppc_e500_exc.h"
#include "ppc_mmu.h"
#include "ppc_opc.h"
#include "ppc_dec.h"
#include "tracers.h"
#include "debug.h"
#include "ppc_dyncom.h"
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
void ppc_opc_bx(cpu_t* cpu, BasicBlock* bb)
{
	e500_core_t* current_core = get_current_core();
	uint32 li;
	PPC_OPC_TEMPL_I(current_core->current_opc, li);
	if (!(current_core->current_opc & PPC_OPC_AA)) {
		//li += current_core->pc;
		arch_store(CONST(li + current_core->pc), cpu->ptr_PC, bb);
	}
	if (current_core->current_opc & PPC_OPC_LK) {
		current_core->lr = current_core->pc + 4;
		LET(LR_REGNUM, CONST(current_core->pc + 4));
	}
	//current_core->npc = li;
}
