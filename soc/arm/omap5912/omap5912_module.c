#include <stdlib.h>
#include <skyeye_module.h>
#include <skyeye_mach.h>
#include "omap5912.h"

const char* skyeye_module = "omap5912";

void module_init(){
        /*
         * register the soc to the common library.
         */
	register_mach("omap5912", omap5912_mach_init);
}

void module_fini(){
}
