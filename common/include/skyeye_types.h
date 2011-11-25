/*
        skyeye_types.h - some data types definition for skyeye debugger
        Copyright (C) 2003 Skyeye Develop Group
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

#ifndef __SKYEYE_TYPES_H
#define __SKYEYE_TYPES_H

#include <stdint.h>
#include <unistd.h>

/*default machine word length */

#define WORD uint32_t

#ifndef __BEOS__
/* To avoid the type conflict with the qemu */
#ifndef QEMU
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t sint8;
typedef int16_t sint16;
typedef int32_t sint32;
typedef int64_t sint64;
#endif

typedef uint32_t address_t;
typedef uint32_t uinteger_t;
typedef int32_t integer_t;

typedef uint32_t physical_address_t;
typedef uint32_t generic_address_t; 

typedef struct conf_object_s{
	char* objname;
	void* obj;
	char* class_name;
}conf_object_t;

typedef enum {
        False = 0,
        True
}bool_t;
#else
#include <be/support/SupportDefs.h>
#endif

typedef enum {
	Align = 0,
	UnAlign	
}align_t;

typedef enum {
	Little_endian = 0,
	Big_endian
}endian_t;
//typedef int exception_t;

typedef enum{
	Phys_addr = 0,
	Virt_addr
}addr_type_t;

typedef enum{
	/* No exception */
	No_exp = 0,
	/* Memory allocation exception */
	Malloc_exp,
	/* File open exception */
	File_open_exp,
	/* DLL open exception */
	Dll_open_exp,
	/* Invalid argument exception */
	Invarg_exp,
	/* Invalid module exception */
	Invmod_exp,
	/* wrong format exception for config file parsing */
	Conf_format_exp,
	/* some reference excess the predefiend range. Such as the index out of array range */
	Excess_range_exp,
	/* Can not find the desirable result */
	Not_found_exp,

	/* Unknown exception */
	Unknown_exp
}exception_t;


typedef enum{
	Generic_exp = 0,
	Core,
	ModuleLoading,
	Conf
}exception_class_t;

#endif
