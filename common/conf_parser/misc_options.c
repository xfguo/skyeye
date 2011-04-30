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
* @file misc_options.c
* @brief some misc skyeye option handler
* @author Michael.Kang blackfin.kang@gmail.com
* @version 
* @date 2011-04-30
*/

#include "skyeye_options.h"
#include "skyeye_config.h"
#include "skyeye_pref.h"
#include "skyeye_log.h"
#include "skyeye_types.h"
#include "skyeye_callback.h"
#include "skyeye_loader.h"
#ifdef DBCT_TEST_SPEED
int
do_dbct_test_speed_sec(struct skyeye_option_t *this_opion, int num_params, const char *params[])
{
	if (num_params != 1) {
		goto error_out;
	}
	errno = 0;
	skyeye_config.dbct_test_speed_sec = strtol(params[0], NULL, 10);
	if (errno == ERANGE) {
		goto error_out;
	}
	printf("dbct_test_speed_sec %ld\n", skyeye_config.dbct_test_speed_sec);

error_out:
	SKYEYE_ERR("Error :usage: step_disassemble: dbct_test_speed_sec\n");
	return(-1);
}
#endif	//DBCT_TEST_SPEED
//AJ2D--------------------------------------------------------------------------

/* set values for some register */
int
do_regfile_option (skyeye_option_t * this_option, int num_params,
		 const char *params[])
{
	skyeye_config_t* config = get_current_config();
	if(config->arch && config->arch->parse_regfile)
		config->arch->parse_regfile(num_params, params);
	else
		SKYEYE_WARNING("regfile option is not implemented by current arch\n");
	return 0;
}

/**
* @brief set load address for elf image
*
* @param this_option
* @param num_params
* @param params[]
*
* @return 
*/
int
do_load_addr_option (skyeye_option_t * this_option, int num_params,
		 const char *params[])
{
	int i;
	char name[MAX_PARAM_NAME], value[MAX_PARAM_NAME];
	unsigned long load_base;
	unsigned long load_mask;
	for (i = 0; i < num_params; i++) {
		if (split_param (params[i], name, value) < 0)
			SKYEYE_ERR
				("Error: sound has wrong parameter \"%s\".\n",
				 name);
		if (!strncmp ("base", name, strlen (name))) {
			sscanf (value, "%x", &load_base);
		}
		else if (!strncmp ("mask", name, strlen (name))) {
			sscanf (value, "%x", &load_mask);
		}
		else
                        SKYEYE_ERR ("Error: Unknown load_addr option  \"%s\"\n", params[i]);
	}
	sky_pref_t *pref;
	/* get the current preference for simulator */
	pref = get_skyeye_pref();
	pref->exec_load_base = load_base;
	pref->exec_load_mask = load_mask;
	/* FIXME, we should update load_base and load_mask to preference of SkyEye */
	fprintf(stderr, "%s not finished.\n", __FUNCTION__);
	//printf("Your elf file will be load to: base address=0x%x,mask=0x%x\n", load_base, load_mask);
	return 0;
}

/**
* @brief the handler for deprecated option
*
* @param this_option
* @param num_params
* @param params[]
*
* @return 
*/
int
do_deprecated_option (skyeye_option_t * this_option, int num_params,
		 const char *params[])
{
	skyeye_log(Warnning_log, __FUNCTION__, "Deprecated option. Do not use any more.\n");
}

/**
* @brief the filename of linux initrd image
*/
static char filename_initrd[MAX_PARAM_NAME];

/**
* @brief the start address of initrd
*/
static uint32_t initrd_start;

/**
* @brief load an initrd image file to the given address
*/
static void load_initrd(void)
{
	load_file(filename_initrd, initrd_start);
}

/**
* @brief set load address for initrd image
*
* @param this_option
* @param num_params
* @param params[]
*
* @return 
*/
int
do_load_file_option (skyeye_option_t * this_option, int num_params,
		 const char *params[])
{
	int i;
	char name[MAX_PARAM_NAME], value[MAX_PARAM_NAME];
	unsigned long load_mask;
	for (i = 0; i < num_params; i++) {
		if (split_param (params[i], name, value) < 0)
			SKYEYE_ERR
				("Error: sound has wrong parameter \"%s\".\n",
				 name);
		if (!strncmp ("filename", name, strlen (name))) {
			strcpy(filename_initrd, value);
		}
		else if (!strncmp ("initrd_start", name, strlen (name))) {
			sscanf (value, "%x", &initrd_start);
		}
		else
                        SKYEYE_ERR ("Error: Unknown load_file option  \"%s\"\n", params[i]);
	}
	register_callback(load_initrd, Bootmach_callback);
	/* FIXME, we should update load_base and load_mask to preference of SkyEye */
	fprintf(stderr, "%s not finished.\n", __FUNCTION__);
	//printf("Your elf file will be load to: base address=0x%x,mask=0x%x\n", load_base, load_mask);
	return 0;
}
