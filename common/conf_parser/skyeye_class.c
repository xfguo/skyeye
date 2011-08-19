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
* @file skyeye_class.c
* @brief the class register.
* @author Michael.Kang blackfin.kang@gmail.com
* @version 0.1
* @date 2011-08-12
*/
#include "skyeye_obj.h"
#include "skyeye_class.h"
void SKY_register_class(const char* name, skyeye_class_t* skyeye_class){
	new_conf_object(name, skyeye_class);	
	return;
}

conf_object_t* pre_conf_obj(const char* objname, const char* class_name){
	skyeye_class_t* class_data = (skyeye_class_t*)get_conf_obj(class_name);
	conf_object_t* obj = class_data->new_instance(objname);
	return obj;
}
