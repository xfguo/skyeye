/*
	skyeye_options.c - skyeye config file options' functions
	Copyright (C) 2003 Skyeye Develop Group
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

/* 08/20/2003   add log option function
				chenyu
   4/02/2003	add net option function
 * 				walimis <walimi@peoplemail.com.cn>
 * 3/22/2003 	add cpu, mem_num, mem_bank, arch, dummy option function
 *				walimis <walimi@peoplemail.com.cn> 		
 * 10/24/2005	add dbct test speed function
 *				teawater <c7code-uc@yahoo.com.cn>
 * */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "skyeye_options.h"
//#include "skyeye_arch.h"
#include "skyeye_config.h"
#include "skyeye_pref.h"

#include "skyeye_types.h"

/* 2007-01-18 added by Anthony Lee: for new uart device frame */
/*#include "skyeye_uart.h"
#include "skyeye_net.h"
#include "skyeye_lcd.h"
*/
int
do_load_addr_option (skyeye_option_t * this_option, int num_params,
		 const char *params[]);
extern FILE *skyeye_logfd;
int
split_param (const char *param, char *name, char *value)
{
	const char *src = param;
	char *dst = name;

	while (*src && (*src != '='))
		*dst++ = *src++;
	*dst = '\0';

	if (*src == '\0') {
		value = '\0';
		return -1;
	}

	strcpy (value, src + 1);
	return 0;
}


/* we need init some options before read the option file.
 * now put them here.
 * */

/* 2007-01-22 : SKYEYE4ECLIPSE moved to skyeye_config.c */
static skyeye_option_t* skyeye_option_list;
int
skyeye_option_init (skyeye_config_t * config)
{
	/* 2007-01-18 added by Anthony Lee: for new uart device frame */
	config->uart.count = 0;
	/* should move to uart module loading */
	//atexit(skyeye_uart_cleanup);

	/*ywc 2005-04-01 */
	config->no_dbct = 1;	/*default, dbct is off */
	//teawater add for new tb manage function 2005.07.10----------------------------
	config->tb_tbt_size = 0;
#if DBCT
	config->tb_tbp_size = TB_TBP_DEFAULT;
#else
	config->tb_tbp_size = 0;
#endif
	skyeye_option_list = NULL;
	register_option("cpu", do_deprecated_option, "Do not need to provide cpu option any more.\n");
	register_option("load_addr", do_load_addr_option, "Set load base address and mask value for elf file loading.\n");
}

exception_t register_option(char* option_name, do_option_t do_option_func, char* helper){
	if(option_name == NULL || !do_option_func)
		return Invarg_exp;
	skyeye_option_t* node = malloc(sizeof(skyeye_option_t));
	if(node == NULL)
		return Malloc_exp; 
	node->option_name = skyeye_strdup(option_name);
	if(node->option_name == NULL){
		skyeye_free(node);
		return Malloc_exp;
	}
	node->do_option = do_option_func;
	/* maybe we should use skyeye_mm to replace all the strdup */
	node->helper = skyeye_strdup(helper);
	if(node->helper == NULL){
		skyeye_free(node->option_name);
		skyeye_free(node);
		return Malloc_exp;
	}
	node->next = skyeye_option_list;
	skyeye_option_list = node;
	//skyeye_log(Info_log, __FUNCTION__, "register option %s successfully.", option_name);
	return No_exp;
}

skyeye_option_t* get_option_list(){
	return skyeye_option_list;
}

#if 0
int com_list_options(char* arg){
	char* format = "%-20s\t%s\n";
        printf(format, "Option Name", "Description");
        skyeye_option_t* list = get_option_list();
        while(list != NULL){
                printf(format, list->option_name, list->helper);
                list = list->next;
        }
        return 0;
}
#endif

int
do_dummy_option (skyeye_option_t * this_option, int num_params,
		 const char *params[])
{
	return 0;
};

/* parse "int" parameters. e.g. int=16:17*/
int
get_interrupts (char value[], uint32 * interrupts)
{
	char *cur = value;
	char *next = value;
	int i = 0, end = 0;

	while ((*next != ':') && (*next != 0))
		next++;

	while (*cur != 0) {
		if (*next != 0) {
			*next = '\0';
		}
		else
			end = 1;
		interrupts[i] = strtoul (cur, NULL, 0);
		//printf("%s:%s\n", __FUNCTION__, cur);
		i++;
		if ((i > 4) || end == 1)
			return 0;
		cur = ++next;
		while ((*next != ':') && (*next != 0))
			next++;
	}
	return 0;
}


#if 0
/* defined in skyeye_arch.c */
extern arch_config_t *skyeye_archs[];


int
do_cpu_option (skyeye_option_t * this_option, int num_params,
	       const char *params[])
{
	int ret;

	if (skyeye_config.arch == NULL) {
		/* If we don't set arch, we use "arm" as default. */
		char *default_arch = "arm";
		int i;
		for (i = 0; i < MAX_SUPP_ARCH; i++) {
			if (skyeye_archs[i] == NULL)
				continue;
			if (!strncmp
			    (default_arch, skyeye_archs[i]->arch_name,
			     MAX_PARAM_NAME)) {
				skyeye_config.arch = skyeye_archs[i];
				SKYEYE_INFO ("arch: %s\n",
					     skyeye_archs[i]->arch_name);
			}
		}
		if (skyeye_config.arch == NULL) {
			SKYEYE_ERR
				("ERROR: No arch option found! Maybe you use low version of skyeye?\n");
			skyeye_exit (-1);
		}
	}
	ret = skyeye_config.arch->parse_cpu (params);
	if (ret < 0)
		SKYEYE_ERR ("Error: Unknown cpu name \"%s\"\n", params[0]);
	return ret;
}
#endif
#if 0
int
do_mach_option (skyeye_option_t * this_option, int num_params,
		const char *params[])
{
	int ret;
	machine_config_t *mach = skyeye_config.mach;
	ret = skyeye_config.arch->parse_mach (mach, params);
	if (ret < 0) {
		SKYEYE_ERR ("Error: Unknown mach name \"%s\"\n", params[0]);
	}
	return ret;
}
#endif




/*
 * we can add this option using:
 * step_disassemble:on[ON|1] for open this option
 * step_disassemble:off[OFF|0] for disable this option 
 * oyangjian add here
 */
#if 0
int
do_step_disassemble_option (skyeye_option_t * this_option, int num_params,
			    const char *params[])
{
	int i;

	for (i = 0; i < num_params; i++) {
		printf ("step_disassemble state:%s\n", params[0]);
		if (!params[0]) {
			SKYEYE_ERR ("Error :usage: step_disassemble:on[off]");
			return -1;
		}
		if (!strncmp (params[0], "on", 2)
		    || !strncmp (params[0], "ON", 2)) {
			skyeye_config.can_step_disassemble = 1;
			return 0;
		}
		else if (!strncmp (params[0], "off", 3)
			 || !strncmp (params[0], "OFF", 3)) {

			skyeye_config.can_step_disassemble = 0;
			return 0;
		}

	}
	SKYEYE_ERR ("Error: Unknown cpu name \"%s\"\n", params[0]);
	return -1;
}
#endif


int
parse_line_formatted (int num_params, const char *params[])
{
	skyeye_option_t *sop;
	//int len = sizeof (skyeye_options) / sizeof (skyeye_option_t);
	int retval = 0;
	if(skyeye_option_list == NULL)
		return 0;
	sop = skyeye_option_list;
	if (num_params < 1)
		return 0;

	while(sop != NULL){
		if (!strncmp (sop->option_name, params[0], MAX_OPTION_NAME)) {
			if (retval = sop->do_option (sop, num_params - 1,
							  &params[1]) < 0) {
				fprintf (stderr,
					 "\"%s\" option parameter error!\n",
					 params[0]);
				return retval;
			}
			else
				return retval;

		}
		sop = sop->next;
	}
	fprintf (stderr, "Unknown option: %s\n", params[0]);
	return -1;		/* unknow option specified */
}
