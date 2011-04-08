#include <stdlib.h>
#include "skyeye_mach.h"
#include "skyeye_module.h"

const char* skyeye_module = "mips";

extern void init_mips_arch ();
extern machine_config_t mips_machines[];
void module_init(){
	init_mips_arch ();
	    /*
         * register all the supported mach to the common library.
         */
        int i = 0;
        while(mips_machines[i].machine_name != NULL){
                register_mach(mips_machines[i].machine_name, mips_machines[i].mach_init);
                i++;
        }
}

void module_fini(){
}
