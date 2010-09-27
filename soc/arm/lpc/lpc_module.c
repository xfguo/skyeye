#include <stdlib.h>
#include <skyeye_module.h>
#include <skyeye_mach.h>
#include "lpc.h"

const char* skyeye_module = "lpc";

void module_init(){
        /*
         * register the soc to the common library.
         */
	register_mach("lpc", lpc_mach_init);
}

void module_fini(){
}
