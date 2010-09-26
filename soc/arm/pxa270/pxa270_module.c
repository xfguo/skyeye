#include <stdlib.h>
#include <skyeye_module.h>
#include <skyeye_mach.h>
#include "pxa.h"

const char* skyeye_module = "pxa270";

void module_init(){
        /*
         * register the soc to the common library.
         */
	register_mach("pxa_mainstone", pxa270_mach_init);
}

void module_fini(){
}
