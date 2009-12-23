#include <stdlib.h>
#include "skyeye_module.h"
#include "skyeye_mach.h"

const char* skyeye_module = "sparc";

extern machine_config_t sparc_machines[];
void module_init(){
	init_sparc_arch ();
	/*
	 * register all the supported mach to the common library.
         */
        int i = 0;
        while(sparc_machines[i].machine_name != NULL){
                register_mach(sparc_machines[i].machine_name, sparc_machines[i].mach_init);
                i++;
        }

}

void module_fini(){
}
