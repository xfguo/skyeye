/*
	skyeye_mach_goldfish.c - define machine goldfish for skyeye
	Copyright (C) 2005 Skyeye Develop Group
	for help please send mail to <skyeye-developer@lists.gro.clinux.org>
	
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

//#define DEBUG
#include <skyeye_config.h>
#include <skyeye_arch.h>
#include <skyeye_sched.h>
#include <skyeye_lock.h>
#include <skyeye_obj.h>
#include <skyeye_log.h>
#include <bank_defs.h>
#ifdef __CYGWIN__
#include <time.h>
#endif

#include "skyeye_mach_goldfish.h"


static uint32
goldfish_io_read_word (void *arch_instance, uint32 addr)
{
	conf_object_t* conf_obj = get_conf_obj("goldfish_mach_space");
	addr_space_t* phys_mem = (addr_space_t*)conf_obj->obj;
	uint32 data;
	DBG("In %s, read addr is 0x%x\n", __FUNCTION__, addr);
	exception_t ret = phys_mem->memory_space->read(conf_obj, addr, &data, 4);
	if(ret == Not_found_exp){
		fprintf(stderr, "Can not find the address 0x%x\n", addr);
		return -1;
	}
	else{
		return data;
	}
}

static uint32
goldfish_io_read_byte (void *arch_instance, uint32 addr)
{
        goldfish_io_read_word (arch_instance, addr);
}

static uint32
goldfish_io_read_halfword (void *arch_instance, uint32 addr)
{
        goldfish_io_read_word (arch_instance, addr);
}


static void
goldfish_io_write_word (generic_arch_t *state, uint32 addr, uint32 data)
{
	conf_object_t* conf_obj = get_conf_obj("goldfish_mach_space");
	addr_space_t* phys_mem = (addr_space_t*)conf_obj->obj;
	DBG("In %s, write addr is 0x%x\n", __FUNCTION__, addr);
	exception_t ret = phys_mem->memory_space->write(conf_obj, addr, &data, 4);
	if(ret == Not_found_exp){
		fprintf(stderr, "Can not find the address 0x%x\n", addr);
		return;
	}
	else{
		return;
	}
}

static void
goldfish_io_write_byte (generic_arch_t * state, uint32 addr, uint32 data)
{
        SKYEYE_DBG ("SKYEYE: goldfish_io_write_byte error\n");
        goldfish_io_write_word (state, addr, data);
}

static void
goldfish_io_write_halfword (generic_arch_t * state, uint32 addr, uint32 data)
{
        SKYEYE_DBG ("SKYEYE: goldfish_io_write_halfword error\n");
        goldfish_io_write_word (state, addr, data);
}

static void
goldfish_io_do_cycle (generic_arch_t *state)
{
}

/* borrowed from qemu */
#ifndef KERNEL_ARGS_ADDR
#define KERNEL_ARGS_ADDR 0x100
#endif
#define INITRD_LOAD_ADDR 0x00800000
#ifdef WRITE_WORD
#undef WRITE_WORD
#endif
#define WRITE_WORD(p, value) do { \
    bus_write(32, p, value);  \
    p += 4;                       \
} while (0)

static void set_kernel_args(generic_arch_t* arch_instance)
{
	generic_address_t base = 0, p = 0;
	size_t ram_size = 0x10000000;
	generic_address_t loader_start = 0;
	size_t initrd_size = 0x200000;
	const char* kernel_cmdline = "qemu=1 console=ttyS0 android.qemud=ttyS1 android.checkjni=1 ndns=3";
	p = base + KERNEL_ARGS_ADDR;
	/* ATAG_CORE */
	WRITE_WORD(p, 5);
	WRITE_WORD(p, 0x54410001);
	WRITE_WORD(p, 1);
	WRITE_WORD(p, 0x1000);
	WRITE_WORD(p, 0);
    /* ATAG_MEM */
    /* TODO: handle multiple chips on one ATAG list */
	WRITE_WORD(p, 4);
	WRITE_WORD(p, 0x54410002);
	WRITE_WORD(p, ram_size);
	WRITE_WORD(p, loader_start);
	if (initrd_size) {
        /* ATAG_INITRD2 */
		WRITE_WORD(p, 4);
		WRITE_WORD(p, 0x54420005);
		WRITE_WORD(p, loader_start + INITRD_LOAD_ADDR);
		WRITE_WORD(p, initrd_size);
	}
	if (kernel_cmdline && *kernel_cmdline) {
        /* ATAG_CMDLINE */
		int cmdline_size;

		cmdline_size = strlen(kernel_cmdline);
		int i = 0;
		for(; i < cmdline_size + 1; i++)
			bus_write(8, p + 8 + i, kernel_cmdline[i]);
		/*
		bus_write(p + 8, (void *)kernel_cmdline,
                                  cmdline_size + 1);
		*/
		cmdline_size = (cmdline_size >> 2) + 1;
		WRITE_WORD(p, cmdline_size + 2);
		WRITE_WORD(p, 0x54410009);
		p += cmdline_size * 4;
	}
#if 0
	if (info->atag_board) {
        /* ATAG_BOARD */
        int atag_board_len;
        uint8_t atag_board_buf[0x1000];

        atag_board_len = (info->atag_board(info, atag_board_buf) + 3) & ~3;
        WRITE_WORD(p, (atag_board_len + 8) >> 2);
        WRITE_WORD(p, 0x414f4d50);
        cpu_physical_memory_write(p, atag_board_buf, atag_board_len);
        p += atag_board_len;
    }
#endif
    /* ATAG_END */
	WRITE_WORD(p, 0);
	WRITE_WORD(p, 0);
	arch_instance->set_regval_by_id(2, KERNEL_ARGS_ADDR);
}

static void
goldfish_io_reset (generic_arch_t* arch_instance)
{
	/* set mach id of goldfish for linux boot */
	arch_instance->set_regval_by_id(1, 1441);
	/**/
	set_kernel_args(arch_instance);
}
void
goldfish_mach_init (void *arch_instance, machine_config_t *this_mach)
{
	goldfish_pic_device* goldfish_pic = new_goldfish_pic_device("goldfish_pic0");

	goldfish_timer_device* goldfish_timer = new_goldfish_timer_device("goldfish_timer0");
	goldfish_timer->line_no = TIMER0_IRQ;

	goldfish_timer->master = goldfish_pic->slave;
	goldfish_timer->signal_target = goldfish_pic->obj;

	addr_space_t* phys_mem = new_addr_space("goldfish_mach_space");

	exception_t ret;
	ret = add_map(phys_mem, 0xff000000, 0x1000, 0x0, goldfish_pic->obj, goldfish_pic->io_memory, 1, 1);
	if(ret != No_exp)
		printf("Warnning, pic can not be mapped\n");
	ret = add_map(phys_mem, 0xff003000, 0x1000, 0x0, goldfish_timer->obj, goldfish_timer->io_memory, 1, 1);
	
	if(ret != No_exp)
		printf("Warnning, timer can not be mapped\n");
	/* @Deprecated */
	this_mach->mach_io_do_cycle = goldfish_io_do_cycle;
        this_mach->mach_io_reset = goldfish_io_reset;
        this_mach->mach_io_read_byte = goldfish_io_read_byte;
        this_mach->mach_io_write_byte = goldfish_io_write_byte;
        this_mach->mach_io_read_halfword = goldfish_io_read_halfword;
        this_mach->mach_io_write_halfword = goldfish_io_write_halfword;
        this_mach->mach_io_read_word = goldfish_io_read_word;
        this_mach->mach_io_write_word = goldfish_io_write_word;
	this_mach->state = (void *) arch_instance;
}
