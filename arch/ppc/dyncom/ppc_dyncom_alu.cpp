/*
 *	PearPC
 *	ppc_alu.cc
 *
 *	Copyright (C) 2003, 2004 Sebastian Biallas (sb@biallas.net)
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
*/
#include "debug.h"
#include "tracers.h"
#include "ppc_dyncom_alu.h"
#include "ppc_dyncom_dec.h"
#include "ppc_exc.h"
#include "ppc_cpu.h"
#include "ppc_dyncom_opc.h"
#include "ppc_tools.h"
#include "ppc_mmu.h"

#include "llvm/Instructions.h"
#include <dyncom/frontend.h>
#include "dyncom/basicblock.h"

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
