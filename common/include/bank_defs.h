/*
        bank_defs.h - necessary definition for skyeye bank
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
 * 12/16/2006   Michael.Kang  <blackfin.kang@gmail.com>
 */
#ifndef __BANK_DEF_H__
#define __BANK_DEF_H__

#include <stdint.h>
#include "skyeye_types.h"
#define MAX_BANK 8
#define MAX_STR  1024
typedef struct mem_bank
{
	unsigned int addr, len;
	char (*bank_write)(short size, int offset, unsigned int value);
	char (*bank_read)(short size, int offset, unsigned int *result);
	char filename[MAX_STR];
	/* the name of object mapping to the bank */
	char* objname;
	unsigned type;
} mem_bank_t;

typedef struct
{
	int bank_num;
	int current_num;	/* current num of bank */
	mem_bank_t mem_banks[MAX_BANK];
} mem_config_t;

#ifdef __cplusplus
 extern "C" {
#endif
/**
 *  The interface of read data from bus
 */
int bus_read(short size, int addr, uint32_t * value);

/**
 * The interface of write data from bus
 */
int bus_write(short size, int addr, uint32_t value);

mem_bank_t * bank_ptr(uint32_t addr);

/* fill the zero for global_memmap */
void reset_global_memmap();

/* Get the global memmap */
mem_config_t * get_global_memmap();

/* Mapping a range of address to the address space */
exception_t addr_mapping(mem_bank_t* bank);

#ifdef __cplusplus
}
#endif
#endif
