/* Copyright (C) 
* 2011 - Michael.Kang blackfin.kang@gmail.com
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
* 
*/
/**
* @file skyeye_interface.h
* @brief the interface definition
* @author Michael.Kang blackfin.kang@gmail.com
* @version 1.3.4
* @date 2011-08-15
*/
#ifndef __SKYEYE_INTERFACE_H__
#define __SKYEYE_INTERFACE_H__
#include <skyeye_types.h>
exception_t SKY_register_interface(conf_object_t* obj, const char* name, const char* iface);
void* SKY_get_interface(conf_object_t* obj, const char* iface_name);
#endif
