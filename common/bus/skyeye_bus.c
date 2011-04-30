/*
        skyeye_bus.c - parse memory map from skyeye.conf
        Copyright (C) 2003-2007 Skyeye Develop Group
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
#include <stdlib.h>

#include "skyeye_config.h"
#include "skyeye_options.h"
#include "skyeye_log.h"
#include "bank_defs.h"
#include "io.h"
#include "skyeye_ram.h"
#include "flash.h"
#include "skyeye_module.h"

/**
* @brief parse the memory related option
*
* @param num_params
* @param params[]
*
* @return 
*/
static int 
parse_mem(int num_params, const char* params[]);

/**
* @brief the handler of memory bank option
*
* @param this_option the option
* @param num_params
* @param params[]
*
* @return 
*/
static int
do_mem_bank_option (skyeye_option_t * this_option, int num_params,
		    const char *params[]);
static int
do_bus_bank_option (skyeye_option_t * this_option, int num_params,
		    const char *params[]);

static void insert_bank(mem_bank_t* bank);

/**
* @brief initiliztion the memory bank
*/
void init_bus(){
	register_option("mem_bank", do_mem_bank_option, "");
	mem_config_t *mc = get_global_memmap();
	memset(mc, 0, sizeof(mem_config_t));
	//register_option("mem_bank", do_bus_bank_option, "");
}

/**
* @brief insert a bank to the memory bank array
*
* @param bank the insert bank
*
* @return the possible exception
*/
exception_t addr_mapping(mem_bank_t* bank){
	insert_bank(bank);
	return No_exp;
}

/**
* @brief insert a bank to the memory bank array
*
* @param bank
*/
static void insert_bank(mem_bank_t* bank){
	int num;
	mem_config_t *mc = get_global_memmap();
	
	mem_bank_t *mb = mc->mem_banks;

	/* TODO, should check the MAX_BANK range.*/
	mc->bank_num = mc->current_num++;
	num = mc->current_num - 1;	/*mem_banks should begin from 0. */
	//skyeye_log(Info_log, __FUNCTION__, "insert bank at %d\n", num);
	if(num >= MAX_BANK){
		skyeye_log(Error_log, __FUNCTION__, "Out of range of valid bank\n");
		return;
	}
	/* TODO, should check the validity of every field in bank */
	/* insert a bank to the last blank band in mem_banks array */
	strncpy(mb[num].filename, bank->filename, MAX_STR);
	//strncpy(mb[num].objname, bank->objname, MAX_STR);
	mb[num].objname = bank->objname;
	mb[num].addr = bank->addr; 
	mb[num].len = bank->len; 
	mb[num].bank_write = bank->bank_write; 
	mb[num].bank_read = bank->bank_read; 
	mb[num].type = bank->type;
}

/**
 * parse_mem - parse the option from skyeye.conf to get mem info
 * @num_params: number of parameters
 * @params: parameter array
 */
static int 
parse_mem(int num_params, const char* params[])
{
	char name[MAX_PARAM_NAME], value[MAX_PARAM_NAME];
	mem_bank_t mb;
	memset(&mb, 0, sizeof(mem_bank_t));
	skyeye_config_t* config = get_current_config();

	int i;
	for (i = 0; i < num_params; i++) {
		if (split_param (params[i], name, value) < 0)
			SKYEYE_ERR
				("Error: mem_bank %d has wrong parameter \"%s\".\n",
				 i, name);

		if (!strncmp ("map", name, strlen (name))) {
			if (!strncmp ("M", value, strlen (value))) {
				mb.bank_read = mem_read;
				mb.bank_write = mem_write;
				mb.type = MEMTYPE_RAM;
			}
			else if (!strncmp ("I", value, strlen (value))) {
				mb.bank_read = io_read;
				mb.bank_write = io_write;
				mb.type = MEMTYPE_IO;
			}
			else if (!strncmp ("F", value, strlen (value))) {
				mb.bank_read = flash_read;
				mb.bank_write = flash_write;
				mb.type = MEMTYPE_FLASH;
			}
			else {
				SKYEYE_ERR
					("Error: mem_bank %d \"%s\" parameter has wrong value \"%s\"\n",
					 i, name, value);
			}
		}
		else if (!strncmp ("type", name, strlen (name))) {
			//chy 2003-09-21: process type
			if (!strncmp ("R", value, strlen (value))) {
				if (mb.type == MEMTYPE_RAM)
					mb.type = MEMTYPE_ROM;
				mb.bank_write = warn_write;
			}
		}
		else if (!strncmp ("addr", name, strlen (name))) {

			if (value[0] == '0' && value[1] == 'x')
				mb.addr = strtoul (value, NULL, 16);
			else
				mb.addr = strtoul (value, NULL, 10);

		}
		else if (!strncmp ("size", name, strlen (name))) {

			if (value[0] == '0' && value[1] == 'x')
				mb.len = strtoul (value, NULL, 16);
			else
				mb.len = strtoul (value, NULL, 10);

		}
		else if (!strncmp ("file", name, strlen (name))) {
			strncpy (mb.filename, value, strlen (value) + 1);
		}
		else if (!strncmp ("boot", name, strlen (name))) {
			/*this must be the last parameter. */
			if (!strncmp ("yes", value, strlen (value)))
				config->start_address = mb.addr;
		}
		else {
			SKYEYE_ERR
				("Error: mem_bank %d has unknow parameter \"%s\".\n",
				 i, name);
		}
	}//for (i = 0; i < num_params; i++)
	insert_bank(&mb);
	return 0;
}

static int
do_mem_bank_option (skyeye_option_t * this_option, int num_params,
		    const char *params[])
{
	#if 0
	int ret;
	ret = skyeye_config.arch->parse_mem (num_params, params);
	if (ret < 0) {
		SKYEYE_ERR ("Error: Unknown mem bank name \"%s\"\n",
			    params[0]);
	}
	return ret;
	#endif
	return parse_mem(num_params, params);
}
static int
do_bus_bank_option (skyeye_option_t * this_option, int num_params,
		    const char *params[])
{
	#if 0
	int ret;
	ret = skyeye_config.arch->parse_mem (num_params, params);
	if (ret < 0) {
		SKYEYE_ERR ("Error: Unknown mem bank name \"%s\"\n",
			    params[0]);
	}
	return ret;
	#endif
	return parse_mem(num_params, params);
}
