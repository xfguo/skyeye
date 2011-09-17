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
#include "skyeye_exec_info.h"

/**
* @brief The global memory map
*/
static mem_config_t global_memmap;

/**
* @brief Get a memory bank for given address
*
* @param addr
*
* @return 
*/
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

/**
* @brief generic data read interface
*
* @param size the width of data
* @param addr the address for the data
* @param value the value of data
*
* @return 
*/
int bus_read(short size, generic_address_t addr, uint32_t * value){
	mem_bank_t * bank;
	generic_arch_t* arch_instance = get_arch_instance("");

	/* bypass the bank check */
	/* in case of error, a seg fault will happen, I guess */
	if (get_skyeye_exec_info()->mmap_access) {
		if (size == 8) {
			*value = *(uint8_t *)(addr) & 0xff;
		} else if (size == 16) {
			*value = *(uint16_t *)(addr) & 0xffff;
		} else if (size == 32) {
			*value = *(uint32_t *)(addr);
		}
		return 0;
	} 
		
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

/**
* @brief the generic data write interface
*
* @param size the width of data
* @param addr the address of the data
* @param value the value of the data
*
* @return 
*/
int bus_write(short size, generic_address_t addr, uint32_t value){
	mem_bank_t * bank;
	generic_arch_t* arch_instance = get_arch_instance("");

	/* bypass the bank check */
	/* in case of error, a seg fault will happen, I guess */
	if (get_skyeye_exec_info()->mmap_access) {
		if (size == 8) {
			*(uint8_t *)(addr) = (value & 0xff);
		} else if (size == 16) {
			*(uint16_t *)(addr) = (value & 0xffff);
		} else if (size == 32) {
			*(uint32_t *)(addr) = value;
		}
		return 0;
	}
	
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

/**
* @brief initialization of global memory map
*/
void reset_global_memmap(){
	memset(&global_memmap, 0, sizeof(mem_config_t));
}

/**
* @brief get globalo memory map
*
* @return the pointer of global memory map
*/
mem_config_t * get_global_memmap(){
	return &global_memmap;
}
