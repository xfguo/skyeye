#include <stdlib.h>
#include <skyeye_module.h>
#include <skyeye_mach.h>
#include "ep7312.h"

const char* skyeye_module = "ep7312";

void module_init(){
        /*
         * register the soc to the common library.
         */
	register_mach("ep7312", ep7312_mach_init);
}

void module_fini(){
}
