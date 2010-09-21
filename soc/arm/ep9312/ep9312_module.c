#include <stdlib.h>
#include <skyeye_module.h>
#include <skyeye_mach.h>
#include "ep9312.h"

const char* skyeye_module = "ep9312";

void module_init() {
        /*
         * register the soc to the common library.
         */
	register_mach("ep9312", ep9312_mach_init);
}

void module_fini() {
}
