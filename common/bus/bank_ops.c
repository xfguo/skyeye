/*
        bank_ops.c - bank operation implementation
        Copyright (C) 2003-2007 Skyeye Develop Group
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
 * 12/26/2009   Michael.Kang  <blackfin.kang@gmail.com>
 */

#include "bank_defs.h"
#include "skyeye.h"
#include "skyeye_types.h"
#include "skyeye_arch.h"
#include "skyeye_callback.h"
#include "skyeye_bus.h"

/* Here is the global memory map */
static mem_config_t global_memmap;

mem_bank_t *
bank_ptr (uint32_t addr)
{
	/* Try to reduce the time of find the right bank */
	static mem_bank_t *mbp = NULL;
	if (mbp) {
		if (mbp->addr <= addr && (addr - mbp->addr) < mbp->len)
			return (mbp);
	}
	for (mbp = global_memmap.mem_banks; mbp->len; mbp++)
		if (mbp->addr <= addr && (addr - mbp->addr) < mbp->len)
			return (mbp);
	return (NULL);
}

/* called by dbct/tb.c tb_find FUNCTION */
mem_bank_t *
insn_bank_ptr (uint32_t addr)
{
	static mem_bank_t *mbp = NULL;
	if (mbp) {
		if (mbp->addr <= addr && (addr - mbp->addr) < mbp->len)
			return (mbp);
	}
	for (mbp = global_memmap.mem_banks; mbp->len; mbp++)
		if (mbp->addr <= addr && (addr - mbp->addr) < mbp->len)
			return (mbp);
	return (NULL);
}

/**
 *  The interface of read data from bus
 */
int bus_read(short size, int addr, uint32_t * value){
	mem_bank_t * bank;
	generic_arch_t* arch_instance = get_arch_instance();

	if((bank = bank_ptr(addr)) && (bank->bank_read))
		bank->bank_read(size, addr, value);
	else{
		SKYEYE_ERR( "Bus read error, can not find corresponding bank for addr 0x%x,pc=0x%x\n", addr, arch_instance->get_pc());
		return -1;
		//skyeye_exit(-1);
	}
	bus_snoop(SIM_access_read, size ,addr, *value, After_act);
	exec_callback(Bus_read_callback, arch_instance);
	return 0;	
}

/**
 * The interface of write data from bus
 */
int bus_write(short size, int addr, uint32_t value){
	mem_bank_t * bank;
	generic_arch_t* arch_instance = get_arch_instance();

	bus_snoop(SIM_access_write, size ,addr, value, Before_act);
	exec_callback(Bus_write_callback, arch_instance);
        if(bank = bank_ptr(addr))
                bank->bank_write(size, addr, value);
        else{
		SKYEYE_ERR( "Bus write error, can not find corresponding bank for addr 0x%x,pc=0x%x\n", addr, arch_instance->get_pc());
		//skyeye_exit(-1);
	}
       return 0; 
}
void reset_global_memmap(){
	memset(&global_memmap, 0, sizeof(mem_config_t));
}
mem_config_t * get_global_memmap(){
	return &global_memmap;
}
