/*
        memory_space.h - definition for memory space
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
#ifndef __MEMORY_SPACE_H__
#define __MEMORY_SPACE_H__
#include "skyeye_types.h"

typedef exception_t(*read_byte_t)(conf_object_t* target, generic_address_t addr, void *buf, size_t count);
typedef exception_t(*write_byte_t)(conf_object_t* target, generic_address_t addr, const void *buf, size_t count);

typedef struct memory_space{
	conf_object_t* conf_obj;
	read_byte_t read;
	write_byte_t write;
}memory_space_intf;
#define MEMORY_SPACE_INTF_NAME "memory_space"

#endif

