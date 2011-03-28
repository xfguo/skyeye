/*
        arm_module.c - the module function for arm simulation on SkyEye.
        Copyright (C) 2003-2010 Skyeye Develop Group
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
 * 01/16/2010   Michael.Kang  <blackfin.kang@gmail.com>
 */

#include "skyeye_module.h"
#include "skyeye_mach.h"
#include "skyeye_options.h"
#include "armdefs.h"
#include <stdlib.h>
const char* skyeye_module = "arm";
extern void init_arm_arch();
extern void init_arm_dyncom ();
extern machine_config_t arm_machines[];
extern ARMul_State* state;

extern int
do_cpu_option (skyeye_option_t * this_option, int num_params,
                const char *params[]);

void module_init(){
	/* register the arm core to the common library */
	init_arm_arch ();
#ifdef LLVM_EXIST
	printf("arm LLVM EXIST \n");
	init_arm_dyncom ();
#else
	printf("arm Don't have LLVM\n");
#endif
	/*
	 * register all the supported mach to the common library.
	 */
	int i = 0;
	while(arm_machines[i].machine_name != NULL){
		register_mach(arm_machines[i].machine_name, arm_machines[i].mach_init);
		i++;
	}

	if(register_option("cpu", do_cpu_option, "Processor option for arm architecture.") != No_exp)
                fprintf(stderr,"Can not register cpu option\n");
}
void module_fini(){
	//ARMul_DeleteState(state);
}
