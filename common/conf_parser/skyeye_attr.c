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
* @file skyeye_attr.c
* @brief The attribute system for skyeye object
* @author Michael.Kang blackfin.kang@gmail.com
* @version 0.1
* @date 2011-09-22
*/

#include <skyeye_attr.h>
#include <skyeye_obj.h>
#include <skyeye_mm.h>

#include <skyeye_log.h>

/**
* @brief Register an attribute to the system
*
* @param obj
* @param attr_name
* @param lcd_ctrl
* @param type
*/
void SKY_register_attr(conf_object_t* obj, const char* attr_name, attr_value_t* attr){
	char attr_objname[MAX_OBJNAME];
	get_strcat_objname(attr_objname, obj->objname, attr_name);
	if(new_conf_object(attr_objname, attr) != NULL){
		return No_exp;
	}
	else{
		return Invarg_exp;
	}
}

/**
* @brief Get an attribute by its name
*
* @param conf_obj
* @param attr_name
* @param attr
*
* @return 
*/
attr_value_t* SKY_get_attr(conf_object_t* conf_obj, const char* attr_name){
	char attr_objname[MAX_OBJNAME];
	if(strlen(conf_obj->objname) + strlen(attr_name) + 1 > MAX_OBJNAME){
		return NULL;
	}
	get_strcat_objname(&attr_objname[0], conf_obj->objname, attr_name);
	conf_object_t* attr_obj = get_conf_obj(attr_objname);
	return attr_obj->obj;
}

/**
* @brief Set the value for the given attribute
*
* @param conf_obj
* @param attr_name
* @param attr
*
* @return 
*/
set_attr_error_t SKY_set_attr(conf_object_t* conf_obj, const char* attr_name, attr_value_t* attr){
	char attr_objname[MAX_OBJNAME];
	if(strlen(conf_obj->objname) + strlen(attr_name) + 1 > MAX_OBJNAME){
		return Set_error;
	}
	get_strcat_objname(&attr_objname[0], conf_obj->objname, attr_name);
	conf_object_t* attr_obj = get_conf_obj(attr_objname);
	if(attr_obj == NULL){
		skyeye_log(Error_log, __FUNCTION__, "The object %s not exist.\n", attr_objname);
		return Set_error;
		
	}
	attr_value_t* current_attr = attr_obj->obj;
	if(current_attr->type != attr->type){
		return Set_invalid_type;
	}
	if(current_attr->type == Val_ptr){
		current_attr->u.ptr = attr->u.ptr;
		return Set_ok;
	}
	else{
		skyeye_log(Error_log, __FUNCTION__, "No such type\n");
		return Set_error;
	}
}

attr_value_t* make_new_attr(value_type_t type){
	attr_value_t* attr = skyeye_mm(sizeof(attr_value_t));
	attr->type = type;
	if(type == Val_ptr){
		return attr;	
	}
	else{
		return NULL;
	}
}
