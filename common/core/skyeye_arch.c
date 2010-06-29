/*
        skyeye_arch.c -  all architecture definition for skyeye
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
/*
 * 12/16/2006   Michael.Kang  <blackfin.kang@gmail.com>
 */

#include "skyeye_arch.h"
#include "skyeye_config.h"
#include "skyeye_options.h"
#include <stdlib.h>

/* the number of supported architecture */
#define MAX_SUPP_ARCH 8
static arch_config_t *skyeye_archs[MAX_SUPP_ARCH];

/*
 * register a supported arch to skyeye_archs
 */
static generic_arch_t* running_arch_list;

static char *default_arch_name = "arm";

void
register_arch (arch_config_t * arch)
{
	int i;
	for (i = 0; i < MAX_SUPP_ARCH; i++) {
		if (skyeye_archs[i] == NULL) {
			skyeye_archs[i] = arch;
			return;
		}
	}
}
/*
 * get arch instance in running by its name.
 */
generic_arch_t * get_arch_instance(const char* arch_name){
	if(running_arch_list == NULL){
		running_arch_list = skyeye_mm_zero(sizeof(generic_arch_t));
	}
	skyeye_config_t* config = get_current_config();
	if(config->arch == NULL){
		printf("No valid arch option is provided.\n");
		return NULL;
	}
	running_arch_list->arch_name = config->arch->arch_name;
	running_arch_list->init = config->arch->init;
        running_arch_list->reset = config->arch->reset;
        running_arch_list->step_once = config->arch->step_once;
        running_arch_list->set_pc = config->arch->set_pc;
        running_arch_list->get_pc = config->arch->get_pc;
        running_arch_list->get_step = config->arch->get_step;
        //running_arch_list->ICE_write_byte = config->arch->ICE_write_byte;
        //running_arch_list->ICE_read_byte = config->arch->ICE_read_byte;
	running_arch_list->get_regval_by_id = config->arch->get_regval_by_id;	
	running_arch_list->get_regname_by_id = config->arch->get_regname_by_id;	
	running_arch_list->mmu_read = config->arch->mmu_read;	
	running_arch_list->mmu_write = config->arch->mmu_write;	
	return running_arch_list;
}
static int
do_arch_option (skyeye_option_t * this_option, int num_params,
		const char *params[])
{
	int i;
	//arch_config_t *arch = skyeye_config.arch;
	skyeye_config_t* config = get_current_config();

	for (i = 0; i < MAX_SUPP_ARCH; i++) {
		if (skyeye_archs[i] == NULL)
			continue;
		if (!strncmp
		    (params[0], skyeye_archs[i]->arch_name, MAX_PARAM_NAME)) {
			config->arch = skyeye_archs[i];
			SKYEYE_INFO ("arch: %s\n",
				     skyeye_archs[i]->arch_name);
			return 0;
		}
	}
	SKYEYE_ERR
		("Error: Unknowm architecture name \"%s\" or you use low version of skyeye?\n",
		 params[0]);
	return -1;
}

generic_arch_t* get_default_arch(){
	int i;
	for (i = 0; i < MAX_SUPP_ARCH; i++) {
        	if (skyeye_archs[i] == NULL)
                	continue;
		if (!strncmp
                    (default_arch_name, skyeye_archs[i]->arch_name, MAX_PARAM_NAME)) {
                        return skyeye_archs[i];
                }
        }
	skyeye_log(Warnning_log, __FUNCTION__, "No default arch is found.\n");
	return NULL;
}

void init_arch(){
	register_option("arch", do_arch_option, "support different architectures.\n");
}

#if 0 /* we will dynamiclly load all the arch module */
extern void init_arm_arch ();
extern void init_bfin_arch ();
extern void init_coldfire_arch ();
extern void init_mips_arch();

extern void init_ppc_arch();
extern void init_sparc_arch();

void
initialize_all_arch ()
{
	int i;
	for (i = 0; i < MAX_SUPP_ARCH; i++) {
		skyeye_archs[i] = NULL;
	}
	/* register arm_arch */
	init_arm_arch ();

	/* register bfin_arch */
	init_bfin_arch ();

	/* register mips_arch */
	init_mips_arch ();

	/* register coldfire_arch */
	init_coldfire_arch ();

	/* register ppc_arch */
	init_ppc_arch();

	/* register sparc_arch */
	init_sparc_arch();
}
#endif
