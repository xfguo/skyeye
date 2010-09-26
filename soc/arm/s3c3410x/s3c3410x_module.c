#include <stdlib.h>
#include <skyeye_module.h>
#include <skyeye_mach.h>
#include "s3c3410x.h"

const char* skyeye_module = "s3c3410x";

void module_init(){
        /*
         * register the soc to the common library.
         */
	register_mach("s3c3410x", s3c3410x_mach_init);
}

void module_fini(){
}
