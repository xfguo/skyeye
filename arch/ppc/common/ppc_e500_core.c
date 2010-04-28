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

#define TCR_DIE (1 << 26)
#define TSR_DIS (1 << 27)
void dec_io_do_cycle(e500_core_t * core){
	core->tbl++;
	/**
	 *  test DIE bit of TCR if timer is enabled
	 */
	if(!(core->tsr & 0x8000000)){
		if((core->tcr & 0x4000000) && (core->msr & 0x8000)) {
		
			if(core->dec > 0)
				core->dec--;
			/* if decrementer eqauls zero */
			if(core->dec == 0){

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
#if 0
void ppc_e500_ipi_int(int core_id, int ipi_id){

	e500_core_t* core = &gCPU.core[core_id];
	core->ipr |= IPI0;
	skyeye_config.mach->mach_set_intr(ipi_id);
	ppc_exception(core, EXT_INT, 0, 0);
	core->ipi_flag = 1; /* we need to inform the core that npc is changed to exception vector */
	//printf("In %s, npc=0x%x, pir=0x%x\n", __FUNCTION__, core->npc, core->pir);
}
#endif
/*
 * Initialization for e500 core
 */
void ppc_core_init(e500_core_t * core, int core_id){
	// initialize srs (mostly for prom)
	int j;
	for (j = 0; j < 16; j++) {
		core->sr[j] = 0x2aa*j;
	}
	//core->pvr = 0x8020000; /* PVR for mpc8560 */
	//core->pvr = 0x80210030;	/* PVR for mpc8572 */
	core->pvr = 0x80040010; /* PVR for mpc8641D */
	core->pir = core_id;

	e500_mmu_init(&core->mmu);
}
