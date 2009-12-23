/*
	skyeye_device.h - definitions of the device framework for skyeye
	Copyright (C) 2004 Skyeye Develop Group
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
/*
 * 05/04/2004  	initial version
 * 		desgin a generic device framework for skyeye to providing good exetension of device simulations
 *
 *		walimis <wlm@student.dlut.edu.cn> 
 * */



/* Important notice: 
 * 	We just want to design an "independent" device framework that hasn't any
 * relationships with any specific device simulation. So don't put any specific 
 * device simulation data here.
 * */

#ifndef __SKYEYE_DEVICE_H_
#define __SKYEYE_DEVICE_H_
#include "skyeye.h"
#include "skyeye_types.h"

#define MAX_STR_NAME	32

#define MAX_INTERRUPT   4
struct device_interrupt
{

	/* interrupts of device.
	 * some devices may have multi interrupts, so we define a array of interrupts.
	 * The meanings of every interrupt must be exactly known by device simulations.*/
	uint32 interrupts[MAX_INTERRUPT];

	/* set interrupt bit.
	 * */
	void (*set_interrupt) (uint32 interrupt);

	/* see if interrupt bit is pending
	 * return value:
	 * 1: pending.
	 * 0: not pending
	 * */
	int (*pending_interrupt) (uint32 interrupt);

	/* update interrupt state.
	 * */
	void (*update_interrupt) (void *mach);

};
struct device_mem_op
{
	int (*read_byte) (void *mach, uint32 addr, uint32 * data);
	int (*write_byte) (void *mach, uint32 addr, uint32 data);
};
/* many devices have default value.
 * */
struct device_default_value
{
	char *name;
	/*I/O or memory base address and size */
	uint32 base;
	uint32 size;
	uint32 interrupts[MAX_INTERRUPT];
};

typedef struct device_desc
{
	/* device type name.
	 * if inexistance, can be gotten from "mach name" 
	 * e.g. ep7312, at91.
	 * */
	char type[MAX_STR_NAME];

	/* device instance name.
	 * The same type of device may have two or more instances, but they
	 * have different name. "name" can identify different instances.
	 * e.g. the "s3c4510b" uart has two instances: uart1 and uart2. 
	 * */
	char name[MAX_STR_NAME];

	/*I/O or memory base address and size */
	uint32 base;
	uint32 size;

	/* interrupt of device.
	 * */
	struct device_interrupt intr;

	/* mem operation 
	 * */
	struct device_mem_op mem_op;

	void (*fini) (struct device_desc * dev);	/*finish routine */
	void (*reset) (struct device_desc * dev);	/*reset device. */
	void (*update) (struct device_desc * dev);	/*called by io_do_cycle */

	int (*filter_read) (struct device_desc *dev, uint32 addr, uint32 *data, size_t count);
	int (*filter_write) (struct device_desc *dev, uint32 addr, uint32 data, size_t count);

	int (*read_byte) (struct device_desc * dev, uint32 addr, uint8 * result);
	int (*read_halfword) (struct device_desc * dev, uint32 addr,
			      uint16 * result);
	int (*read_word) (struct device_desc * dev, uint32 addr, uint32 * result);

	int (*write_byte) (struct device_desc * dev, uint32 addr, uint8 data);
	int (*write_halfword) (struct device_desc * dev, uint32 addr, uint16 data);
	int (*write_word) (struct device_desc * dev, uint32 addr, uint32 data);

	/* refer the "mach" that the device belongs to.
	 * */
	void *mach;

	/* specific common data for a type of device
	 * */
	void *dev;

	/* device specific data
	 * usually be an "io" struct.
	 * */
	void *data;
} device_desc_t;


struct device_module
{
	char *type_name;
	int (*setup) (struct device_desc * dev);
};
struct device_module_set
{
	char *name;		/*module set name. e.g. uart, net, lcd */
	struct device_module **mod;	/*pointer of array of device_module */
	int count;		/*current count of device_module */
	int count_max;		/*max count of device_module */

	/* initialize this module set. */
	void (*init) (struct device_module_set * mod_set);
	int initialized;

	/*
	 * setup "device module" data
	 * */
	int (*setup_module) (struct device_desc * dev, void *option);

};

int register_device_module (char *name,
				   struct device_module_set *mod_set,
				   int (*setup) (struct device_desc * dev));
int register_device_module_set (struct device_module_set *mod_set);

void initialize_all_devices ();
void set_device_default (struct device_desc *dev,
				struct device_default_value *def);
/* register funtions of all kinds of device module set. 
 * */
//extern void uart_register();
//extern void timer_register();
//extern void net_register();
//extern void lcd_register();
//extern void flash_register();
//extern void touchscreen_register();
//extern void sound_register();
void register_pen_buffer(int* pen_buffer);
int* get_pen_buffer();

#define ADDR_NOHIT		0	/* address no hit */
#define ADDR_HIT		1	/* address hit */
#define ADDR_HITNOAVAIL		2	/* address hit but no available */


#define ACTION_EQUAL		0
#define ACTION_AND		1
#define ACTION_AND_NOT		2
#define ACTION_OR		3
#define ACTION_OR_NOT		4

#endif /*__SKYEYE_DEVICE_H_ */
