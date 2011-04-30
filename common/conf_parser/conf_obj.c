/*
        conf_obj.c - Manage the named object 

        Copyright (C) 2009 - 2010 Michael.Kang
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
 * 09/12/2010   Michael.Kang  <blackfin.kang@gmail.com>
 */

#include "skyeye_types.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#ifndef _GUN_HASH_LIST
#define _GNU_HASH_LIST
#endif
#include <string.h>
#define __USE_GNU
#include <search.h>
#include "skyeye_mm.h"
#include "skyeye_log.h"

/**
* @brief the data saved to hash table
*/
struct hsearch_data *conf_tab = NULL;

/**
* @brief The max object number in hash table
*/
const static max_conf_obj = 1024;

/**
* @brief initialization of config object management
*/
void init_conf_obj(){
	conf_tab = skyeye_mm(sizeof(struct hsearch_data));
	memset(conf_tab, 0, sizeof(struct hsearch_data));
	hcreate_r(max_conf_obj, conf_tab);
}

/**
* @brief place an object to the hash table
*
* @param objname
* @param obj
*
* @return 
*/
exception_t put_conf_obj(char* objname, void* obj){
	ENTRY item, *retval;
	//printf("In %s, objname=%s\n", __FUNCTION__, objname);
	item.key = objname;
	item.data = obj;
	int ret = hsearch_r(item, ENTER, &retval, conf_tab);
	if(ret != 0)
		return No_exp;
	else
		/* TODO: more handler for different exceptions */
		return Invarg_exp;
}

/*
 *  Get a named object from hash table
 */
void* get_conf_obj(char* objname){
	ENTRY item;
	ENTRY* retval;
	item.key = objname;
	assert(objname != NULL);
	int ret = hsearch_r(item, FIND, &retval, conf_tab);
	//printf("In %s, ret= 0x%x, objname=%s, data=0x%x\n", __FUNCTION__, ret, objname, retval->data);
	/* Non-zero on success */
	if(ret == 0)
		return NULL;
	else
		return retval->data;
}
#define TYPE_CASTING(conf_obj, type_string) (##type_string##)get_cast_conf_obj(conf_obj, type_string)

/**
* @brief For type casting safely
*
* @param conf_obj
* @param type_string
*
* @return 
*/
void* get_cast_conf_obj(conf_object_t* conf_obj, const char* type_string){
	if(!strncmp(conf_obj->objname, type_string, strlen(type_string))){
		return conf_obj->obj;
	}
	else{
		printf("In %s,conf_obj=0x%x\n, conf_obj->objname=0x%x\n", __FUNCTION__, conf_obj, conf_obj->objname);
		printf("In %s, conf_obj->objname=%s\n", __FUNCTION__, conf_obj->objname);
		skyeye_log(Warnning_log, __FUNCTION__, "Type %s cast is failed!\n", type_string);
		return NULL;
	}
}

/**
* @brief get its conf_obj from an object and its name
*
* @param obj
* @param type_string
*
* @return 
*/
conf_object_t* get_conf_obj_by_cast(void* obj, const char* type_string){
	//printf("In %s,sizeof(conf_object_t)=0x%x\n", __FUNCTION__, sizeof(conf_object_t));
	conf_object_t* conf_obj = skyeye_mm(sizeof(struct conf_object_s));
	/* Memory allocation failed. */
	if(conf_obj == NULL)
		return NULL;
	conf_obj->obj = obj;
	//printf("In %s, type_string=%s\n", __FUNCTION__, type_string);
	conf_obj->objname = skyeye_strdup(type_string);
	//printf("In %s, conf_obj->objname=%s\n", __FUNCTION__, conf_obj->objname);
	//printf("In %s, conf_obj=0x%x, conf_obj->objname=0x%x\n", __FUNCTION__, conf_obj, conf_obj->obj);
	//put_conf_obj(obj, type_string);
	return conf_obj;
}

/**
* @brief List all the object in the hash
*
* @return 
*/
void* list_all_obj(){
}

/**
* @brief destruction of the hash
*/
void fini_conf_obj(){
	hdestroy_r(conf_tab);
}
