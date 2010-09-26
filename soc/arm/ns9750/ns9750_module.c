#include <stdlib.h>
#include <skyeye_module.h>
#include <skyeye_mach.h>
#include "ns9750.h"

const char* skyeye_module = "ns9750";

void module_init(){
        /*
         * register the soc to the common library.
         */
	register_mach("ns9750", ns9750_mach_init);
}

void module_fini(){
}
