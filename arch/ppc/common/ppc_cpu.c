/*
 *	PearPC
 *	ppc_cpu.cc
 *
 *	Copyright (C) 2003, 2004 Sebastian Biallas (sb@biallas.net)
 *      Portions Copyright (C) 2004 Apple Computer, Inc.
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

#include "ppc_cpu.h"
#include "ppc_dec.h"
#include "ppc_fpu.h"
#include "ppc_exc.h"
#include "ppc_mmu.h"
#include "ppc_tools.h"
#include "tracers.h"
#include "sysendian.h"
#include "portable/portable.h"

#include <stdio.h>
#include <stdarg.h>

//#include "io/graphic/gcard.h"

static bool_t gSinglestep = False;

//uint32 gBreakpoint2 = 0x11b3acf4;
uint32 gBreakpoint3 = 0xc016ee74 & 0;
uint32 gBreakpoint = 0x11b3acf4 & 0;
uint32 gBreakpoint2 = 0xc017a4f4 & 0;

bool_t activate = False;
static inline void ppc_debug_hook()
{
	e500_core_t *current_core = get_current_core();
	if (current_core->pc == gBreakpoint) {
		gSinglestep = True;
//              SINGLESTEP("breakpoint 1");
	}
	if (current_core->pc == gBreakpoint2) {
		//SINGLESTEP("breakpoint 2");
	}
//      if (current_core->pc == gBreakpoint3 && current_core->gpr[5]==0x100004ec) {
/*	if (current_core->pc == gBreakpoint3) {
		activate = True;
		SINGLESTEP("breakpoint 3");
	}*/
	/*
	   if (gSinglestep) {
	   gDebugger->enter();
	   } */
}

/*sys_mutex exception_mutex;*/

void ppc_cpu_atomic_raise_ext_exception()
{
	e500_core_t *current_core = get_current_core();
	/*sys_lock_mutex(exception_mutex); */
	current_core->ext_exception = True;
	current_core->exception_pending = True;
	/*sys_unlock_mutex(exception_mutex); */
}

void ppc_cpu_atomic_cancel_ext_exception()
{
	e500_core_t *current_core = get_current_core();
	/* sys_lock_mutex(exception_mutex); */
	current_core->ext_exception = False;
	if (!current_core->dec_exception)
		current_core->exception_pending = False;
	/*sys_unlock_mutex(exception_mutex); */
}

void ppc_cpu_atomic_raise_dec_exception()
{
	e500_core_t *current_core = get_current_core();
	/*sys_lock_mutex(exception_mutex); */
	current_core->dec_exception = True;
	current_core->exception_pending = True;
	/*sys_unlock_mutex(exception_mutex); */
}

void ppc_cpu_wakeup()
{
}

void ppc_cpu_run()
{
	e500_core_t *current_core = get_current_core();
	/*gDebugger = new Debugger();
	   gDebugger->mAlwaysShowRegs = True; */
	PPC_CPU_TRACE("execution started at %08x\n", current_core->pc);
	uint ops = 0;
	current_core->effective_code_page = 0xffffffff;
//      ppc_fpu_test();
//      return;
	while (True) {
		current_core->npc = current_core->pc + 4;
		if ((current_core->pc & ~0xfff) ==
		    current_core->effective_code_page) {
			current_core->current_opc =
			    ppc_word_from_BE(*
					     ((uint32 *) (&current_core->
							  physical_code_page
							  [current_core->
							   pc & 0xfff])));
			ppc_debug_hook();
		} else {
			int ret;
			if ((ret =
			     ppc_direct_effective_memory_handle_code
			     (current_core->pc & ~0xfff,
			      current_core->physical_code_page))) {
				if (ret == PPC_MMU_EXC) {
					current_core->pc = current_core->npc;
					continue;
				} else {
					PPC_CPU_ERR("?\n");
				}
			}
			current_core->effective_code_page =
			    current_core->pc & ~0xfff;
			continue;
		}
		//ppc_exec_opc();
		ops++;
		current_core->ptb++;
		if (current_core->pdec == 0) {
			current_core->exception_pending = True;
			current_core->dec_exception = True;
			current_core->pdec = 0xffffffff * TB_TO_PTB_FACTOR;
		} else {
			current_core->pdec--;
		}
		if ((ops & 0x3ffff) == 0) {
/*			if (pic_check_interrupt()) {
				current_core->exception_pending = True;
				current_core->ext_exception = True;
			}*/
			if ((ops & 0x0fffff) == 0) {
//                              uint32 j=0;
//                              ppc_read_effective_word(0xc046b2f8, j);

				ht_printf
				    ("@%08x (%u ops) pdec: %08x lr: %08x\r",
				     current_core->pc, ops, current_core->pdec,
				     current_core->lr);
#if 0
				extern uint32 PIC_enable_low;
				extern uint32 PIC_enable_high;
				ht_printf("enable ");
				int x = 1;
				for (int i = 0; i < 31; i++) {
					if (PIC_enable_low & x) {
						ht_printf("%d ", i);
					}
					x <<= 1;
				}
				x = 1;
				for (int i = 0; i < 31; i++) {
					if (PIC_enable_high & x) {
						ht_printf("%d ", 32 + i);
					}
					x <<= 1;
				}
				ht_printf("\n");
#endif
			}
		}

		current_core->pc = current_core->npc;

		if (current_core->exception_pending) {
			if (current_core->stop_exception) {
				current_core->stop_exception = False;
				if (!current_core->dec_exception
				    && !current_core->ext_exception)
					current_core->exception_pending = False;
				break;
			}
			if (current_core->msr & MSR_EE) {
				/*sys_lock_mutex(exception_mutex); */
				if (current_core->ext_exception) {
					ppc_exception(current_core,
						      PPC_EXC_EXT_INT, 0, 0);
					current_core->ext_exception = False;
					current_core->pc = current_core->npc;
					if (!current_core->dec_exception)
						current_core->
						    exception_pending = False;
					/*sys_unlock_mutex(exception_mutex); */
					continue;
				}
				if (current_core->dec_exception) {
					ppc_exception(current_core, PPC_EXC_DEC,
						      0, 0);
					current_core->dec_exception = False;
					current_core->pc = current_core->npc;
					current_core->exception_pending = False;
					/*sys_unlock_mutex(exception_mutex); */
					continue;
				}
				/*sys_unlock_mutex(exception_mutex); */
				PPC_CPU_ERR("no interrupt, but signaled?!\n");
			}
		}
#ifdef PPC_CPU_ENABLE_SINGLESTEP
		if (current_core->msr & MSR_SE) {
			if (current_core->singlestep_ignore) {
				current_core->singlestep_ignore = False;
			} else {
				ppc_exception(current_core, PPC_EXC_TRACE2);
				current_core->pc = current_core->npc;
				continue;
			}
		}
#endif
	}
}

void ppc_cpu_stop()
{
	e500_core_t *current_core = get_current_core();
	/*sys_lock_mutex(exception_mutex); */
	current_core->stop_exception = True;
	current_core->exception_pending = True;
	/*sys_unlock_mutex(exception_mutex); */
}

uint64 ppc_get_clock_frequency(int cpu)
{
	return PPC_CLOCK_FREQUENCY;
}

uint64 ppc_get_bus_frequency(int cpu)
{
	return PPC_BUS_FREQUENCY;
}

uint64 ppc_get_timebase_frequency(int cpu)
{
	return PPC_TIMEBASE_FREQUENCY;
}

void ppc_machine_check_exception()
{
	PPC_CPU_ERR("machine check exception\n");
}

uint32 ppc_cpu_get_gpr(int cpu, int i)
{
	e500_core_t *current_core = get_current_core();
	return current_core->gpr[i];
}

void ppc_cpu_set_gpr(int cpu, int i, uint32 newvalue)
{
	e500_core_t *current_core = get_current_core();
	current_core->gpr[i] = newvalue;
}

void ppc_cpu_set_msr(int cpu, uint32 newvalue)
{
	e500_core_t *current_core = get_current_core();
	current_core->msr = newvalue;
}

void ppc_cpu_set_pc(int cpu, uint32 newvalue)
{
	e500_core_t *current_core = get_current_core();
	current_core->pc = newvalue;
}

uint32 ppc_cpu_get_pc(int cpu)
{
	e500_core_t *current_core = get_current_core();
	return current_core->pc;
}

uint32 ppc_cpu_get_pvr(e500_core_t * core)
{
	return core->pvr;
}

void ppc_cpu_map_framebuffer(uint32 pa, uint32 ea)
{
	e500_core_t *current_core = get_current_core();
	// use BAT for framebuffer
	current_core->dbatu[0] = ea | (7 << 2) | 0x3;
	current_core->dbat_bl17[0] = ~(BATU_BL(current_core->dbatu[0]) << 17);
	current_core->dbatl[0] = pa;
}

void ppc_set_singlestep_v(bool_t v, const char *file, int line,
			  const char *format, ...)
{
	va_list arg;
	va_start(arg, format);
	ht_fprintf(stdout, "singlestep %s from %s:%d, info: ",
		   v ? "set" : "cleared", file, line);
	ht_vfprintf(stdout, format, arg);
	ht_fprintf(stdout, "\n");
	va_end(arg);
	gSinglestep = v;
}

void ppc_set_singlestep_nonverbose(bool_t v)
{
	gSinglestep = v;
}

#define CPU_KEY_PVR	"cpu_pvr"

//#include "configparser.h"
#define MPC8560_DPRAM_SIZE 0xC000
#define MPC8560_IRAM_SIZE 0x8000
#if 0
bool ppc_cpu_init()
{
	//memset(&current_core-> 0, sizeof current_core->;
	memset(&gCPU[0], 0, sizeof gCPU[0]);
	memset(&gCPU[1], 0, sizeof gCPU[1]);
	//current_core->pvr = gConfig->getConfigInt(CPU_KEY_PVR);

	current_core->cpm_reg.dpram = (void *)malloc(MPC8560_DPRAM_SIZE);
	if (!current_core->cpm_reg.dpram) {
		printf("malloc failed for dpram\n");
		skyeye_exit(-1);
	} else
		printf("malloc succ for dpram, dpram=0x%x\n",
		       current_core->cpm_reg.dpram);

	current_core->cpm_reg.iram = (void *)malloc(MPC8560_IRAM_SIZE);
	if (!current_core->cpm_reg.iram) {
		printf("malloc failed for iram\n");
		skyeye_exit(-1);
	} else
		printf("malloc succ for dpram, dpram=0x%x\n",
		       current_core->cpm_reg.iram);

	ppc_dec_init();
	// initialize srs (mostly for prom)
	int i;
	for (i = 0; i < 16; i++) {
		current_core->sr[i] = 0x2aa * i;
	}
	/*sys_create_mutex(&exception_mutex); */

	PPC_CPU_WARN("You are using the generic CPU!\n");
	PPC_CPU_WARN
	    ("This is much slower than the just-in-time compiler and\n");
	PPC_CPU_WARN
	    ("should only be used for debugging purposes or if there's\n");
	PPC_CPU_WARN("no just-in-time compiler for your platform.\n");

	return True;
}
#endif
void ppc_cpu_init_config()
{
//      gConfig->acceptConfigEntryIntDef("cpu_pvr", 0x000c0201);
}
