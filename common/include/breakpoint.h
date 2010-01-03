/*
        breakpoint.h - definition for breakpoint related operation
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
#ifndef __COMMON_BREAKPOINT_H__
#define __COMMON_BREAKPOINT_H__

#include "skyeye_types.h"

typedef enum{
	SIM_access_read = 1,
	SIM_access_write = 2,
	SIM_access_execute = 4
}access_t;

typedef enum{
	/* the breakpoint on physical address */
	SIM_Break_Physical = 0,
	/* the breakpoint on virtual address */
	SIM_Break_Virtual = 1,
	SIM_Break_Linear = 2 /* x86 only */
}breakpoint_kind_t;

typedef int breakpoint_id_t;

/*
 * insert one breakpoint with indicated attribute
 */
exception_t skyeye_insert_bp(access_t access_type, breakpoint_kind_t address_type, generic_address_t addr);

/*
 * delete one breakpoint with id
 */
//exception_t skyeye_remove_bp(breakpoint_id_t id);

/*
 * Delete one breakoint by its address. Maybe match multiple
 * addresses.
 */
exception_t skyeye_remove_bp_by_addr(generic_address_t addr);

/*
 * Get a breakpoint by its address.
 */
//breakpoint_t* get_bp_by_addr(generic_address_t addr);


/*
 * delete the breakpoint in a memory range
 */
void SKY_breakpoint_remove(int id, access_t access, generic_address_t address, generic_address_t length);
/*
 * interface for breakpoint management
 */
#if 0
class breakpoint_interface{
	virtual void insert_bp(conf_object_t *object, breakpoint_t bp, generic_address_t start, generic_address_t end);
	virtual void remove_bp(conf_object_t *object, breakpoint_id_t bp_id, access_t access, generic_address_t start, generic_address_t end);
	virtual void remove_bp_range(conf_object_t *object, breakpoint_id_t bp_id, access_t access, generic_address_t start, generic_address_t end);
	virtual breakpoint_range_t get_bp_range(conf_object_t *object, breapoint_t *bp);
}
#endif
#endif
