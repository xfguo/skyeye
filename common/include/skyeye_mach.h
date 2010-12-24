/*
 *	skyeye_mach.h - The mach related definition  
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
 * 05/16/2009   Michael.Kang  <blackfin.kang@gmail.com>
 */
#ifndef __SKYEYE_MACH_H__
#define __SKYEYE_MACH_H__
#include "skyeye_types.h"
#include "skyeye_signal.h"

#ifdef __cplusplus
 extern "C" {
#endif

typedef struct machine_config
{
	const char *machine_name;	/*e.g.at91,ep7312,clps711x */
	void (*mach_init) (void * state, struct machine_config * this_mach);	/*should be called when the machine initilization */
	uint32 (*mach_io_read_byte) (void * state, uint32 addr);
	void (*mach_io_write_byte) (void * state, uint32 addr,
				    uint32 data);
	uint32 (*mach_io_read_halfword) (void * state,
					    uint32 addr);
	void (*mach_io_write_halfword) (void * state, uint32 addr,
					uint32 data);
	  uint32 (*mach_io_read_word) (void * state, uint32 addr);
	void (*mach_io_write_word) (void * state, uint32 addr,
				    uint32 data);

	/*ywc 2005-03-30 */
	  uint32 (*mach_flash_read_byte) (void * state, uint32 addr);
	void (*mach_flash_write_byte) (void * state, uint32 addr,
				       uint32 data);
	  uint32 (*mach_flash_read_halfword) (void * state,
					       uint32 addr);
	void (*mach_flash_write_halfword) (void * state, uint32 addr,
					   uint32 data);
	  uint32 (*mach_flash_read_word) (void * state, uint32 addr);
	void (*mach_flash_write_word) (void * state, uint32 addr,
				       uint32 data);

	/* for I/O device
	 * */
	/* We put this here so prescaling can be done at a higher level for speed */
	int  io_cycle_divisor;

	void (*mach_io_do_cycle) (void * state);
	void (*mach_io_reset) (void * state);	/* io reset when init io */
	void (*mach_update_int) (void * state);	/* update interrupt pend bit */

	void (*mach_set_intr) (uint32 interrupt);	/*set interrupt pending bit */
	int (*mach_pending_intr) (uint32 interrupt);	/*test if interrupt is pending. 1: pending */
	void (*mach_update_intr) (void *mach);	/*update interrupt pending bit */
	void (*mach_intr_signal)(int irq_line, signal_t signal);

	int (*mach_mem_read_byte) (void *mach, uint32 addr, uint32 * data);
	int (*mach_mem_write_byte) (void *mach, uint32 addr, uint32 data);
	/* FIXME! point to the current void.
	 * */
	void *state;
	/* FIXME!
	 * we just temporarily put devices here
	 * */
	struct device_desc **devices;
	unsigned int dev_count;
	struct machine_config* next;

	conf_object_t* cpu_data;			/* The pointer to cpu data */
} machine_config_t;
 
typedef	void (*mach_init_t) (void * state, struct machine_config * this_mach);	/*should be called when machine initialization */
void register_mach(const char* mach_name, mach_init_t mach_init);
machine_config_t * get_mach(const char* mach_name);
machine_config_t * get_current_mach();

machine_config_t * get_mach_list();
#ifdef __cplusplus
}
#endif

#endif
