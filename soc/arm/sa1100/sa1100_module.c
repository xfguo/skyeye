#include <stdlib.h>
#include <skyeye_module.h>
#include <skyeye_mach.h>
#include "sa1100.h"

const char* skyeye_module = "sa1100";

void module_init(){
        /*
         * register the soc to the common library.
         */
	register_mach("sa1100", sa1100_mach_init);
}

void module_fini(){
}
