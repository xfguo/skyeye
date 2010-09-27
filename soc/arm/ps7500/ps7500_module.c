#include <stdlib.h>
#include <skyeye_module.h>
#include <skyeye_mach.h>
#include "ps7500.h"

const char* skyeye_module = "ps7500";

void module_init(){
        /*
         * register the soc to the common library.
         */
	register_mach("ps7500", ps7500_mach_init);
}

void module_fini(){
}
