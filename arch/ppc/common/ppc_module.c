#include <stdlib.h>

#include "skyeye_mach.h"
#include "skyeye_module.h"

const char* skyeye_module = "ppc";

extern machine_config_t ppc_machines[];
void module_init(){
	init_ppc_arch ();
	 /*
         * register all the supported mach to the common library.
         */
        int i = 0;
        while(ppc_machines[i].machine_name != NULL){
                register_mach(ppc_machines[i].machine_name, ppc_machines[i].mach_init);
                i++;
        }
}

void module_fini(){
}
