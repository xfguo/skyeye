/* 
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/*
 * 30/10/2007   Michael.Kang  <blackfin.kang@gmail.com>
 */


/*
 * A simple boot function for linux happy
 */

#include <stdio.h>
#include <string.h>

#include "ppc_cpu.h"
#include "ppc_boot.h"
#include "ppc_mmu.h"
#include "sysendian.h"
#include "skyeye_loader.h" 

static void set_bootcmd(){
	const int bd_start = 8 * 1024 * 1024;
	const int initrd_start  = 32 * 1024 * 1024, initrd_size = 2 * 1024 * 1024;
	e500_core_t * core = get_boot_core();
	bd_t t;
	memset(&t, '\0', sizeof(t));
        t.bi_immr_base = ppc_word_to_BE(0xe0000000);
        t.bi_busfreq = ppc_word_to_BE(100 * 1024 * 1024);
        t.bi_intfreq = ppc_word_to_BE(500 * 1024 * 1024);
        t.bi_baudrate = ppc_word_to_BE(9600);
        t.bi_memsize = ppc_word_to_BE(64 * 1024 * 1024);
	load_data(&t, sizeof(t), bd_start);

	core->gpr[3] = bd_start;

/*
 *   r4 - Starting address of the init RAM disk
 *   r5 - Ending address of the init RAM disk
 */
	core->gpr[4] = initrd_start;
	core->gpr[5] = initrd_start + initrd_size;


	char * bootcmd = "root=/dev/ram0 console=ttyCPM0 mem=64M";
	const int bootcmd_start= 9 * 1024 * 1024;
	/* load bootcmd string to bootcmd_start address */
	load_data(bootcmd, (strlen(bootcmd) + 1), bootcmd_start);

	core->gpr[6] = bootcmd_start;
	core->gpr[7] = bootcmd_start + strlen(bootcmd) + 1;
}

static void setup_boot_map(){
	e500_core_t * core = get_boot_core();
	/* setup initial tlb map for linux, that should be done by bootloader */
        ppc_tlb_entry_t * entry = &core->mmu.l2_tlb1_vsp[0];

        entry->v = 1; /* entry is valid */
        entry->ts = 0; /* address space 0 */
        entry->tid = 0; /* TID value for shared(global) page */
        entry->epn = 0xC0000; /* Virtual address of DDR ram in address space*/
        entry->rpn = 0x0; /* Physical address of DDR ram in address space*/
        entry->size = 0x7; /* 16M byte page size */
        /* usxrw should be initialized to 010101 */
        entry->usxrw |= 0x15; /* Full supervisor mode access allowed */
        entry->usxrw &= 0x15; /* No user mode access allowed */
        entry->wimge = 0x8; /* Caching-inhibited, non-coherent,big-endian*/
        entry->x = 0; /* Reserved system attributes */
        entry->u = 0; /* User attribute bits */
        entry->iprot = 1; /* Page is protected from invalidation */
}
void mpc8560_boot_linux(){
	/* Fixme, will move it to skyeye.conf */
	set_bootcmd();
	/* just for linux boot, so we need to do some map */
	setup_boot_map();
	PPC_CPU_State* cpu = get_current_cpu();
	cpu->ccsr = 0xE0000; /* Just for boot linux */
}
