/*
 *	PearPC
 *	ppc_exc.cc
 *
 *	Copyright (C) 2003 Sebastian Biallas (sb@biallas.net)
 *	Copyright (C) 2004 Daniel Foesch (dfoesch@cs.nmsu.edu)
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

/*	Pages marked: v.???
 *	From: IBM PowerPC MicroProcessor Family: Altivec(tm) Technology...
 *		Programming Environments Manual
 */

/*
 *  Modified by Michael.Kang to implement E500 core
 */

//#include "tools/snprintf.h"
//#include "debug/tracers.h"
//#include "cpu/debug.h"
//#include "info.h"
#include "ppc_cpu.h"
#include "ppc_exc.h"
#include "ppc_mmu.h"
#include "tracers.h"

bool_t ppc_exception(e500_core_t * core, uint32 type, uint32 flags, uint32 a)
{
	return core->ppc_exception(core, type, flags, a);
}

/*
IVOR0  Critical Input
IVOR1  Machine Check
IVOR2  Data Storage
IVOR3  Instruction Storage
IVOR4  External Input
IVOR5  Alignment
IVOR6  Program
IVOR7  Floating-Point Unavailable
IVOR8  System Call
IVOR9  Auxiliary Processor Unavailable
IVOR10 Decrementer
IVOR11 Fixed-Interval Timer Interrupt
IVOR12 Watchdog Timer Interrupt
IVOR13 Data TLB Error
IVOR14 Instruction TLB Error
IVOR15 Debug
*/
/*
 *	.247
 */
#define ST (1 << 24)

#if 1
bool_t e600_ppc_exception(e500_core_t * core, uint32 type, uint32 flags,
			  uint32 a)
{
	if (type != PPC_EXC_DEC)
		PPC_EXC_TRACE("@%08x: type = %08x (%08x, %08x)\n", core->pc,
			      type, flags, a);
	switch (type) {
	case PPC_EXC_DSI:{	// .271
			core->srr[0] = core->pc;
			core->srr[1] = core->msr & 0x87c0ffff;
			core->dar = a;
			core->dsisr = flags;
			printf("In %s, addr=0x%x, pc=0x%x DSI exception.\n",
			       __FUNCTION__, a, core->pc);
			break;
		}
	case PPC_EXC_ISI:{	// .274
			if (core->pc == 0) {
				PPC_EXC_WARN("pc == 0 in ISI\n");
				//SINGLESTEP("");
			}
			core->srr[0] = core->pc;
			core->srr[1] = (core->msr & 0x87c0ffff) | flags;
			break;
		}
	case PPC_EXC_DEC:{	// .284
			core->srr[0] = core->npc;
			core->srr[1] = core->msr & 0x87c0ffff;
			break;
		}
	case PPC_EXC_EXT_INT:{
			core->srr[0] = core->pc;
			core->srr[1] = core->msr & 0x87c0ffff;
			break;
		}
	case PPC_EXC_SC:{	// .285
			core->srr[0] = core->npc;
			core->srr[1] = core->msr & 0x87c0ffff;
			break;
		}
	case PPC_EXC_NO_FPU:{	// .284
			core->srr[0] = core->pc;
			core->srr[1] = core->msr & 0x87c0ffff;
			break;
		}
	case PPC_EXC_NO_VEC:{	// v.41
			core->srr[0] = core->pc;
			core->srr[1] = core->msr & 0x87c0ffff;
			break;
		}
	case PPC_EXC_PROGRAM:{	// .283
			if (flags & PPC_EXC_PROGRAM_NEXT) {
				core->srr[0] = core->npc;
			} else {
				core->srr[0] = core->pc;
			}
			core->srr[1] = (core->msr & 0x87c0ffff) | flags;
			break;
		}
	case PPC_EXC_FLOAT_ASSIST:{
				// .288
			core->srr[0] = core->pc;
			core->srr[1] = core->msr & 0x87c0ffff;
			break;
		}
	case PPC_EXC_MACHINE_CHECK:{
				// .270
			if (!(core->msr & MSR_ME)) {
				PPC_EXC_ERR
				    ("machine check exception and MSR[ME]=0.\n");
			}
			core->srr[0] = core->pc;
			core->srr[1] = (core->msr & 0x87c0ffff) | MSR_RI;
			break;
		}
	case PPC_EXC_TRACE2:{	// .286
			core->srr[0] = core->pc;
			core->srr[1] = core->msr & 0x87c0ffff;
			break;
		}
	default:
		PPC_EXC_ERR("unknown\n");
		return False;
	}
	ppc_mmu_tlb_invalidate(core);
	core->msr = 0;
	core->npc = type;
	return True;
}
#endif
void ppc_cpu_raise_ext_exception()
{
	ppc_cpu_atomic_raise_ext_exception();
}

void ppc_cpu_cancel_ext_exception()
{
	ppc_cpu_atomic_cancel_ext_exception();
}
