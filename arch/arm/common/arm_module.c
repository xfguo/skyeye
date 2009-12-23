#include "skyeye_module.h"
#include "skyeye_mach.h"
#include "skyeye_options.h"
#include "armdefs.h"
#include <stdlib.h>
const char* skyeye_module = "arm";
extern void init_arm_arch();
extern machine_config_t arm_machines[];
extern ARMul_State* state;

extern int
do_cpu_option (skyeye_option_t * this_option, int num_params,
                const char *params[]);

void module_init(){
	/* register the arm core to the common library */
	init_arm_arch ();

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
