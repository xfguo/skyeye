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

#include "skyeye_config.h"
#include "skyeye_options.h"
#include "bank_defs.h"
#include "io.h"
#include "skyeye_ram.h"
#include "flash.h"
#include "skyeye_module.h"

static int 
parse_mem(int num_params, const char* params[]);

static int
do_mem_bank_option (skyeye_option_t * this_option, int num_params,
		    const char *params[]);
static int
do_bus_bank_option (skyeye_option_t * this_option, int num_params,
		    const char *params[]);

void init_bus(){
	register_option("mem_bank", do_mem_bank_option, "");
	//register_option("mem_bank", do_bus_bank_option, "");
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
	int i, num;
	mem_config_t *mc = get_global_memmap();

	mem_bank_t *mb = mc->mem_banks;
	skyeye_config_t* config = get_current_config();
	mc->bank_num = mc->current_num++;

	num = mc->current_num - 1;	/*mem_banks should begin from 0. */
	mb[num].filename[0] = '\0';
	for (i = 0; i < num_params; i++) {
		if (split_param (params[i], name, value) < 0)
			SKYEYE_ERR
				("Error: mem_bank %d has wrong parameter \"%s\".\n",
				 num, name);

		if (!strncmp ("map", name, strlen (name))) {
			if (!strncmp ("M", value, strlen (value))) {
				mb[num].bank_read = mem_read;
				mb[num].bank_write = mem_write;
				mb[num].type = MEMTYPE_RAM;
			}
			else if (!strncmp ("I", value, strlen (value))) {
				mb[num].bank_read = io_read;
				mb[num].bank_write = io_write;
				mb[num].type = MEMTYPE_IO;
			}
			else if (!strncmp ("F", value, strlen (value))) {
				mb[num].bank_read = flash_read;
				mb[num].bank_write = flash_write;
				mb[num].type = MEMTYPE_FLASH;
			}
			else {
				SKYEYE_ERR
					("Error: mem_bank %d \"%s\" parameter has wrong value \"%s\"\n",
					 num, name, value);
			}
		}
		else if (!strncmp ("type", name, strlen (name))) {
			//chy 2003-09-21: process type
			if (!strncmp ("R", value, strlen (value))) {
				if (mb[num].type == MEMTYPE_RAM)
					mb[num].type = MEMTYPE_ROM;
				mb[num].bank_write = warn_write;
			}
		}
		else if (!strncmp ("addr", name, strlen (name))) {

			if (value[0] == '0' && value[1] == 'x')
				mb[num].addr = strtoul (value, NULL, 16);
			else
				mb[num].addr = strtoul (value, NULL, 10);

		}
		else if (!strncmp ("size", name, strlen (name))) {

			if (value[0] == '0' && value[1] == 'x')
				mb[num].len = strtoul (value, NULL, 16);
			else
				mb[num].len = strtoul (value, NULL, 10);

		}
		else if (!strncmp ("file", name, strlen (name))) {
			strncpy (mb[num].filename, value, strlen (value) + 1);
		}
		else if (!strncmp ("boot", name, strlen (name))) {
			/*this must be the last parameter. */
			if (!strncmp ("yes", value, strlen (value)))
				config->start_address = mb[num].addr;
		}
		else {
			SKYEYE_ERR
				("Error: mem_bank %d has unknow parameter \"%s\".\n",
				 num, name);
		}
	}
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
