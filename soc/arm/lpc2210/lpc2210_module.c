#include <stdlib.h>
#include <skyeye_module.h>
#include <skyeye_mach.h>
#include "lpc.h"

const char* skyeye_module = "lpc2210";

void module_init(){
        /*
         * register the soc to the common library.
         */
	register_mach("lpc2210", lpc2210_mach_init);
}

void module_fini(){
}
