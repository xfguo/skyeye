/*
        ppc_e500_core.c - definition for powerpc e500 core
        Copyright (C) 2003 Skyeye Develop Group
        for help please send mail to <skyeye-developer@lists.sf.linuxforum.net>

        This program is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2 of the License, or
        (at your option) any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program; if not, write to the Free Software
        Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
/*
 * 07/21/2008   Michael.Kang  <blackfin.kang@gmail.com>
 */

#include "ppc_cpu.h"
#include "ppc_e500_exc.h"
#include "ppc_exc.h"
#include "ppc_mmu.h"
#include "skyeye_config.h"

#define TCR_DIE (1 << 26)
#define TSR_DIS (1 << 27)
static void e500_dec_io_do_cycle(e500_core_t * core)
{
	core->tbl++;
	/**
	 *  test DIE bit of TCR if timer is enabled
	 */
	if (!(core->tsr & 0x8000000)) {
		if ((core->tcr & 0x4000000) && (core->msr & 0x8000)) {

			if (core->dec > 0)
				core->dec--;
			/* if decrementer eqauls zero */
			if (core->dec == 0) {

				/* if auto-load mode is set */
#if 0
				if (gCPU.tcr & 0x400000)
					gCPU.dec = gCPU.decar;
#endif
				/* trigger timer interrupt */
				ppc_exception(core, DEC, 0, core->pc);
			}
		}
	}
	return;
}

static void e600_dec_io_do_cycle(e500_core_t * core)
{
	core->tbl++;
	/* if tbl overflow, we increase tbh */
	if (core->tbl == 0)
		core->tbu++;
	/* trigger interrupt when dec value from 1 to 0 */
	if (core->dec > 0)
		core->dec--;
	/* if decrementer eqauls zero */
	if ((core->dec == 0) && (core->msr & MSR_EE)) {
		/* trigger timer interrupt */
		ppc_exception(core, PPC_EXC_DEC, 0, core->pc);
		core->dec--;
	}
	return;
}

#if 0
void ppc_e500_ipi_int(int core_id, int ipi_id)
{

	e500_core_t *core = &gCPU.core[core_id];
	core->ipr |= IPI0;
	skyeye_config.mach->mach_set_intr(ipi_id);
	ppc_exception(core, EXT_INT, 0, 0);
	core->ipi_flag = 1;	/* we need to inform the core that npc is changed to exception vector */
	//printf("In %s, npc=0x%x, pir=0x%x\n", __FUNCTION__, core->npc, core->pir);
}
#endif
static uint32 e500_get_ccsr_base(uint32 reg)
{
	return ((reg >> 8) & 0xFFF) << 20;
}

/*
 * get ccsr base address according to the register value
 */
static uint32 e600_get_ccsr_base(uint32 reg)
{
	return ((reg >> 8) & 0xFFFF) << 16;
}

/*
 * Initialization for e500 core
 */
void ppc_core_init(e500_core_t * core, int core_id)
{
	// initialize srs (mostly for prom)
	int j;
	for (j = 0; j < 16; j++) {
		core->sr[j] = 0x2aa * j;
	}
	skyeye_config_t *config = get_current_config();
	machine_config_t *mach = config->mach;
	if (!strcmp(mach->machine_name, "mpc8560")) {
		core->pvr = 0x8020000;	/* PVR for mpc8560 */
		/* E500 core initialization */
		e500_mmu_init(&core->mmu);
		core->effective_to_physical = e500_effective_to_physical;
		core->ppc_exception = e500_ppc_exception;
		core->syscall_number = SYSCALL;
		core->dec_io_do_cycle = e500_dec_io_do_cycle;
		core->get_ccsr_base = e500_get_ccsr_base;
		core->ccsr_size = 0x100000;
		core->pc = 0xFFFFFFFC;
	} else if (!strcmp(mach->machine_name, "mpc8572")) {
		core->pvr = 0x80210030;	/* PVR for mpc8572 */
		/* E500 core initialization */
		e500_mmu_init(&core->mmu);
		core->effective_to_physical = e500_effective_to_physical;
		core->ppc_exception = e500_ppc_exception;
		core->syscall_number = SYSCALL;
		core->dec_io_do_cycle = e500_dec_io_do_cycle;
		core->get_ccsr_base = e500_get_ccsr_base;
		core->ccsr_size = 0x100000;
		core->pc = 0xFFFFFFFC;
	} else if (!strcmp(mach->machine_name, "mpc8641d")) {
		core->pvr = 0x80040010;	/* PVR for mpc8641D */
		/* E600 core initialization */
		core->effective_to_physical = e600_effective_to_physical;
		core->ppc_exception = e600_ppc_exception;
		core->syscall_number = PPC_EXC_SC;
		core->dec_io_do_cycle = e600_dec_io_do_cycle;
		core->get_ccsr_base = e600_get_ccsr_base;
		core->ccsr_size = 0x100000;
		/* FIXME, we should give the default value to all the register 
		 * according to the reset settings section of e600 manual 
		 */
		core->msr = 0x00000040;
	}

	core->pir = core_id;
	pthread_spin_init(&(core->ipr_spinlock), PTHREAD_PROCESS_SHARED);
	core->ipi_flag = 0;
}
