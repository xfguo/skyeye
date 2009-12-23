/*
        memory.h - definition for memory related operation
        Copyright (C) 2009 Michael.Kang
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
 * 05/16/2009   Michael.Kang  <blackfin.kang@gmail.com>
 */

#ifndef __COMMON_MEMORY_H__
#define __COMMON_MEMORY_H__
#include "skyeye_types.h"
uinteger_t SIM_read_phys_memory(conf_object_t *cpu, physical_address_t paddr, int length);
uinteger_t SIM_write_phys_memory(conf_object_t *cpu, physical_address_t paddr, uinteger_t value, int length);

exception_t SIM_read_byte(conf_object_t *obj, generic_address_t paddr, uint8 &value);
exception_t SIM_write_byte(conf_object_t *obj, generic_address_t paddr, uint8 value);
#endif
