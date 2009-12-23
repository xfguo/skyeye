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
#ifndef __COMMON_MEMORY_SPACE_H__
#define __COMMON_MEMORY_SPACE_H__
class memory_space_intf{
public:
	virtual exception_type_t read(conf_object_t *obj, physical_address_t addr, int length, attr_value_t* result);
	virtual exception_type_t write(conf_object_t *obj, physical_address_t addr, int length, attr_value_t data);
}

device_remap(device_t* dev, uint32 start, uint32 end);
#endif

