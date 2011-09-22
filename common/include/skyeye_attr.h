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
* @file skyeye_attr.h
* @brief The definition of attribute system
* @author Michael.Kang blackfin.kang@gmail.com
* @version 0.1
* @date 2011-09-22
*/

#ifndef __SKYEYE_ATTR_H__
#define __SKYEYE_ATTR_H__
#include <skyeye_types.h>
typedef enum {
        Val_Invalid = -1,
        Val_String,
        Val_Integer,
        Val_Floating,
        Val_List,
        Val_Data,
        Val_Nil,
        Val_Object,
        Val_Dict,
        Val_Boolean,
	Val_ptr,

        Sim_Val_Unresolved_Object
} value_type_t;

typedef struct attr_value {
        value_type_t type;
        union {
                const char *string;          /* Sim_Val_String */
                integer_t integer;           /* Sim_Val_Integer */
                bool_t boolean;                /* Sim_Val_Boolean */
                double floating;             /* Sim_Val_Floating */

                /* Sim_Val_List */
                struct {
                        size_t size;
                        struct attr_value *vector;  /* [size] */
                } list;

                /* data array */
                struct {
                        size_t size;
                        uint8 *data;         /* [size] */
                } data;

                struct conf_object* object;       /* Sim_Val_Object */
		void* ptr;
        } u;
} attr_value_t;

typedef enum{
	Set_ok = 0,
	Set_invalid_type,
	Set_error,
}set_attr_error_t;

void SKY_register_attr(conf_object_t* obj, const char*  attr_name, attr_value_t* attr);

set_attr_error_t SKY_set_attr(conf_object_t* obj, const char* attr_name, attr_value_t* attr);

attr_value_t* SKY_get_attr(conf_object_t* conf_obj, const char* attr_name);
attr_value_t* make_new_attr(value_type_t type);
#endif
