#include <stdlib.h>
#include <skyeye_module.h>
#include <skyeye_mach.h>
#include <skyeye_arch.h>
#include "at91.h"

const char* skyeye_module = "at91";

void module_init(){
        /*
         * register the soc to the common library.
         */
	register_mach("at91", at91_mach_init);
}

void module_fini(){
}
