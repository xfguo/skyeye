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
* @file skyeye_pref.c
* @brief the preference module of skyeye
* @author Michael.Kang blackfin.kang@gmail.com
* @version 
* @date 2011-04-30
*/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "skyeye_pref.h"
#include "skyeye_log.h"
#include "skyeye_mm.h"

/**
* @brief the preference struct 
*/
static sky_pref_t *skyeye_pref;

/**
* @brief the initiialization of preference module
*
* @param pref
*
* @return 
*/
static exception_t init_skyeye_pref(sky_pref_t** pref){
	*pref = skyeye_mm_zero(sizeof(sky_pref_t));
	//skyeye_log(Debug_log, __FUNCTION__, "pref = 0x%x\n", *pref);
	if(*pref == NULL){
		skyeye_log(Error_log, __FUNCTION__, get_exp_str(Malloc_exp));
		return Malloc_exp;
	}
	memset(*pref, 0, sizeof(sky_pref_t));
	
	return No_exp;
}

/* a set of apis for operating some boot parameters*/
void set_exec_file(const char *filename)
{
	if (filename == NULL) {
		skyeye_log(Error_log, __func__,
					"kernel filename is NULL\n");
		exit(0);
	}
	skyeye_pref->exec_file = strdup(filename);
}

/**
* @brief get the executable file
*
* @return 
*/
char *get_exec_file()
{
	return skyeye_pref->exec_file;
}

/**
* @brief set the load base for an executable file
*
* @param addr
*/
void set_exec_load_base(const generic_address_t addr)
{
/*
	if(addr == NULL)
	{
		skyeye_log(Error_log, __func__, "addr is NULL\n");
		exit(0);
	}
*/
	skyeye_pref->exec_load_base = addr;
}

/**
* @brief get the load base of an executable base
*
* @return 
*/
generic_address_t get_exec_load_base()
{
	return skyeye_pref->exec_load_base;
}

/**
* @brief set the load mask of an executable file
*
* @param addr
*/
void set_exec_load_mask(const uint32_t addr)
{
/*
	if(addr == NULL)
		exit(0);
*/
	skyeye_pref->exec_load_mask = addr;
}

/**
* @brief get the load mask of an executable file
*
* @return 
*/
uint32_t get_exec_load_mask()
{
	return skyeye_pref->exec_load_mask;
}

/**
* @brief set the config file for simulator
*
* @param filename
*/
void set_conf_filename(const char *filename)
{
	if (filename == NULL) {
		skyeye_log(Error_log, __func__,
					"The path of skyeye.conf is NULL\n");
		exit(0);
	}
	skyeye_pref->conf_filename = strdup(filename);
}

/*
char *get_conf_filename()
{
	return skyeye_pref->conf_filename;
}
*/

/**
* @brief set the interactice mode
*
* @param mode
*/
void set_interactive_mode(const bool_t mode)
{
	skyeye_pref->interactive_mode = mode;
}

/**
* @brief get the current interactive mode
*
* @return 
*/
bool_t get_interactive_mode()
{
	return skyeye_pref->interactive_mode;
}

/**
* @brief set the current endian
*
* @param endian
*/
void set_endian(const endian_t endian)
{
	skyeye_pref->endian = endian;
}

/**
* @brief get the current endian
*
* @return 
*/
bool_t get_endian()
{
	return skyeye_pref->endian;
}

/**
* @brief set the autoboot mode
*
* @param value
*/
void set_autoboot(const bool_t value)
{
	skyeye_pref->autoboot = value;
}

/**
* @brief get the autoboot mode
*
* @return 
*/
bool_t get_autoboot()
{
	return skyeye_pref->autoboot;
}

/**
* @brief set the user mode for the simulator
*
* @param value
*/
void set_user_mode(const uint32_t value){
	skyeye_pref->user_mode_sim = value;
}

/**
* @brief get the user mode
*
* @return 
*/
bool_t get_user_mode(){
	return skyeye_pref->user_mode_sim;
}

/**
* @brief set the uart port
*
* @param value
*/
void set_uart_port(const uint32_t value){
	skyeye_pref->uart_port = value;
}

/**
* @brief get the uart port
*
* @return 
*/
uint32_t get_uart_port(){
	return skyeye_pref->uart_port;
}

/**
* @brief update current preference
*
* @param pref
*/
void update_skyeye_pref(sky_pref_t *pref){
	/*
	if(skyeye_pref->module_search_dir)
		free(skyeye_pref->module_search_dir)
	skyeye_pref->module_search_dir = 
	*/
}

/* FIXME, that is maybe not thread-safe */

/**
* @brief get the current preference
*
* @return 
*/
sky_pref_t * get_skyeye_pref(){
	if(skyeye_pref == NULL){
		init_skyeye_pref(&skyeye_pref);/* set the load base */
		skyeye_pref->exec_load_base = 0x0;
        	skyeye_pref->exec_load_mask = 0xFFFFFFFF;

	}
	//skyeye_log(Debug_log, __FUNCTION__, "skyeye_pref = 0x%x\n", skyeye_pref);
	return skyeye_pref;
}

/**
* @brief get the current config file
*
* @return 
*/
char* get_conf_filename(){
	sky_pref_t* pref = get_skyeye_pref();
	assert(pref != NULL);
	return pref->conf_filename;
}
