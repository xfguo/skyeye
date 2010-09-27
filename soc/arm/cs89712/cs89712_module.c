#include <stdlib.h>
#include <skyeye_module.h>
#include <skyeye_mach.h>
#include "cs89712.h"

const char* skyeye_module = "cs89712";

void module_init(){
        /*
         * register the soc to the common library.
         */
	register_mach("cs89712", cs89712_mach_init);
}

void module_fini(){
}
