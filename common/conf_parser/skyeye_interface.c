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
* @file skyeye_interface.c
* @brief The implementation of interface
* @author Michael.Kang blackfin.kang@gmail.com
* @version 1.3.4
* @date 2011-08-15
*/

#include <string.h>
#include "skyeye_types.h"
#include "skyeye_obj.h"

/**
* @brief compose an interface object name for hash table
*
* @param iface_objname
* @param objname
* @param iface_name
*/
static inline void get_iface_objname(char* iface_objname, const char* objname, const char* iface_name){
	memcpy(&iface_objname[0], objname, strlen(objname));
	iface_objname[strlen(objname)] = '\0';
	strcat(iface_objname, iface_name);
	return;
}
exception_t SKY_register_interface(conf_object_t* obj, const char* iface_name, void* iface){
	char iface_objname[MAX_OBJNAME];
	if(strlen(obj->objname) + strlen(iface_name) + 1 > MAX_OBJNAME){
		return Invarg_exp;
	}
	get_iface_objname(iface_objname, obj->objname, iface);
	if(new_conf_object(iface_objname, iface) != NULL)	
		return No_exp;
	else
		return Invarg_exp;
}

void* SKY_get_interface(conf_object_t* obj, const char* iface_name){
	char iface_objname[MAX_OBJNAME];
	if(strlen(obj->objname) + strlen(iface_name) + 1 > MAX_OBJNAME){
		return Invarg_exp;
	}
	get_iface_objname(iface_objname, obj->objname, iface_name);
	return get_conf_obj(iface_objname);
}
