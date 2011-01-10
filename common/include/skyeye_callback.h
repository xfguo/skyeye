/*
        skyeye_callback.h - callback function definition for skyeye
        Copyright (C) 2003 Skyeye Develop Group
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
 * 09/16/2009   Michael.Kang  <blackfin.kang@gmail.com>
 */

#ifndef __CALLBACK_H__
#define __CALLBACK_H__
#include "skyeye_arch.h"
typedef enum{
	Step_callback = 0, /* called when step running of core */
	Mem_read_callback, /* called when memory write */
	Mem_write_callback, /* called when memory read */
	Bus_read_callback, /* called when memory write */
	Bus_write_callback, /* called when memory read */
	Exception_callback, /* called when some exceptions are triggered. */
	Bootmach_callback, /* called when hard reset of machine */
	SIM_exit_callback, /* called when simulator exit */
	Max_callback
}callback_kind_t;
typedef void(*callback_func_t)(generic_arch_t* arch_instance);
void register_callback(callback_func_t func, callback_kind_t kind);
int exec_callback(callback_kind_t kind, generic_arch_t* arch_instance);
#endif
