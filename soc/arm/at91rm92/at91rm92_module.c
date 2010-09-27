#include <stdlib.h>
#include <skyeye_module.h>
#include <skyeye_mach.h>
#include <skyeye_arch.h>
#include "at91rm92.h"

const char* skyeye_module = "at91rm92";

void module_init(){
        /*
         * register the soc to the common library.
         */
	register_mach("at91rm92", at91rm92_mach_init);
}

void module_fini(){
}
