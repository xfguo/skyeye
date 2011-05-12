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


static inline void ppc_dyncom_update_cr0(cpu_t* cpu, BasicBlock *bb, uint32 r)
{
	LETS(CR_REGNUM, AND(RS(CR_REGNUM), CONST(0x0fffffff)));
	LETS(CR_REGNUM, 
		SELECT(ICMP_EQ(R(r), CONST(0)), OR(RS(CR_REGNUM), CONST(CR_CR0_EQ)), 
			SELECT(ICMP_NE(AND(R(r), CONST(0x80000000)), CONST(0)), 
				OR(RS(CR_REGNUM), CONST(CR_CR0_LT)), 
				OR(RS(CR_REGNUM), CONST(CR_CR0_GT)))));

	LETS(CR_REGNUM, SELECT(ICMP_NE(AND(RS(XER_REGNUM), CONST(XER_SO)), CONST(0)), 
		OR(RS(CR_REGNUM), CONST(CR_CR0_SO)), RS(CR_REGNUM)));
}
static int inline ppc_dyncom_mask(int MB, int ME){
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
	Value* tmp1 = ICMP_ULT(ADD(a, b), a);
	Value* tmp2 = ICMP_ULT(ADD(ADD(a, b), c), c);
	return SELECT(OR(tmp1, tmp2), TRUE, FALSE);
}
static void ppc_dyncom_set_msr(cpu_t *cpu, BasicBlock *bb, Value *newmsr, Value *cond)
{
	LETS(EFFECTIVE_CODE_PAGE_REGNUM, SELECT(cond, RS(EFFECTIVE_CODE_PAGE_REGNUM), CONST(0xffffffff)));
	newmsr = SELECT(ICMP_NE(AND(newmsr, CONST(MSR_POW)), CONST(0)), AND(newmsr,CONST(~MSR_POW)), newmsr);
	LETS(MSR_REGNUM, SELECT(cond, RS(MSR_REGNUM), newmsr));
}
#endif
