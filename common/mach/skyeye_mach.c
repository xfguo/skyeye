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
* @file skyeye_mach.c
* @brief the machine module of SkyEye
* @author Michael.Kang blackfin.kang@gmail.com
* @version 
* @date 2011-04-30
*/

#include <stdlib.h>
#include "skyeye_mm.h"
#include "skyeye_mach.h"
#include "skyeye_options.h"
#include "skyeye_config.h"
#include "skyeye_log.h"

/**
* @brief the supported machine list
*/
static machine_config_t* mach_list;

/**
* @brief the handler of machine option
*
* @param this_option
* @param num_params
* @param params[]
*
* @return 
*/
int
do_mach_option (skyeye_option_t * this_option, int num_params,
                const char *params[])
{
        int ret;
        machine_config_t *mach = get_mach(params[0]);
	skyeye_config_t* config = get_current_config();
	if(mach != NULL){
		config->mach = mach;
		skyeye_log(Info_log,__FUNCTION__,"mach info: name %s, mach_init addr %p\n", config->mach->machine_name, config->mach->mach_init);
		ret = 0;
	}
        else{
                SKYEYE_ERR ("Error: Unknown mach name \"%s\"\n", params[0]);
		ret = -1;
        }
        return ret;
}

/**
* @brief initialization of machine module
*/
void init_mach(){
	mach_list = NULL;
	register_option("mach", do_mach_option, "machine option");
}

/**
* @brief register a machine to the simulator
*
* @param mach_name
* @param mach_init
*/
void register_mach(const char* mach_name, mach_init_t mach_init){
	machine_config_t * mach;
	mach = skyeye_mm_zero(sizeof(machine_config_t));
	mach->machine_name =  skyeye_strdup(mach_name);
	mach->mach_init = mach_init;
	mach->next = mach_list;
	mach_list = mach;
	//skyeye_log(Debug_log, __FUNCTION__, "regiser mach %s successfully.\n", mach->machine_name);
}

/**
* @brief get a machine struct by its name
*
* @param mach_name
*
* @return 
*/
machine_config_t * get_mach(const char* mach_name){
	machine_config_t* node;
	node = mach_list;
	while(node){
		//if(!strncmp(node->machine_name, mach_name, strlen(node->machine_name))){
		if(!strcmp(node->machine_name, mach_name)){
			return node;
		}
		node = node->next;
	}
	return NULL;
}

#if 0
machine_config_t* get_mach(skyeye_config_t* config){
	return config->mach;
}
#endif

/**
* @brief get the whole machine list
*
* @return 
*/
machine_config_t * get_mach_list(){
	return mach_list;
}

/**
* @brief get the current running machine 
*
* @return 
*/
machine_config_t *get_current_mach(){
	skyeye_config_t* config = get_current_config();
	return config->mach;
}
