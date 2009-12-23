#include <stdlib.h>
#include "skyeye_module.h"
#include "skyeye_mach.h"

char* skyeye_module = "x86";
extern void init_x86_arch();
extern machine_config_t x86_machines[];
void module_init(){
	//skyeye_module = strdup("x86");
	init_x86_arch ();
	/*
         * register all the supported mach to the common library.
         */
        int i = 0;
#if 1
        while(x86_machines[i].machine_name != NULL){
                register_mach(x86_machines[i].machine_name, x86_machines[i].mach_init);
                i++;
        }
#endif
}

void module_fini(){
}
