#include <stdlib.h>
#include "skyeye_module.h"
#include "skyeye_mach.h"

const char* skyeye_module = "bfin";

extern machine_config_t bfin_machines[];
void module_init(){
	//skyeye_module = strdup("bfin");
	init_bfin_arch ();
	/*
         * register all the supported mach to the common library.
         */
        int i = 0;
        while(bfin_machines[i].machine_name != NULL){
                register_mach(bfin_machines[i].machine_name, bfin_machines[i].mach_init);
                i++;
        }

}

void module_fini(){
}
