/*
 *	ppc_dyncom_alu.h - Definition of some translation function for 
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

#ifndef __PPC_DYNCOM_ALU_H__
#define __PPC_DYNCOM_ALU_H__
#include "skyeye_types.h"
#include <dyncom/tag.h>
#include "llvm/Instructions.h"
#include <dyncom/dyncom_llvm.h>
#include <dyncom/frontend.h>

#include "ppc_tools.h"
#include "ppc_dyncom_run.h"
#include "ppc_dyncom_dec.h"
#include "ppc_e500_core.h"
#include "ppc_cpu.h"
#include "ppc_regformat.h"


static inline void ppc_dyncom_update_cr0(cpu_t* cpu, BasicBlock *bb, uint32 r)
{
	LETS(CR_REG, AND(RS(CR_REG), CONST(0x0fffffff)));
	LETS(CR_REG, 
		SELECT(NOT(R(r)), OR(RS(CR_REG), CONST(CR_CR0_EQ)), 
			SELECT(AND(R(r), CONST(0x80000000)), 
				OR(RS(CR_REG), CONST(CR_CR0_LT)), 
				OR(RS(CR_REG), CONST(CR_CR0_GT)))));

	LETS(CR_REG, SELECT(AND(RS(XER_REG), CONST(XER_SO)), 
		RS(CR_REG), OR(RS(CR_REG), CONST(CR_CR0_SO))));
}
static int inline ppc_mask(int MB, int ME){
	uint32 mask;
	if (MB <= ME) {
		if (ME-MB == 31) {
			mask = 0xffffffff;
		} else {
			mask = ((1<<(ME-MB+1))-1)<<(31-ME);
		}
	} else {
		mask = ppc_word_rotl((1<<(32-MB+ME+1))-1, 31-ME);
	}
	return mask;
}

static inline Value* ppc_dyncom_carry_3(cpu_t* cpu, BasicBlock *bb, Value* a, Value* b, Value* c)
{
	return SELECT(LOG_OR(ICMP_ULT(ADD(a, b), c), ICMP_ULT(ADD(ADD(a, b), c), c)), CONST(1), CONST(0));
}

#endif
