#include <stdio.h>
#include "skyeye_module.h"
#include "skyeye_mach.h"

const char* skyeye_module = "coldfire";
extern machine_config_t coldfire_machines[];

void module_init(){
	init_coldfire_arch ();

	/*
	 * register all the supported mach to the common library.
	 */
	int i = 0;
	while(coldfire_machines[i].machine_name != NULL){
		register_mach(coldfire_machines[i].machine_name, coldfire_machines[i].mach_init);
		i++;
	}
}

void module_fini(){
}
