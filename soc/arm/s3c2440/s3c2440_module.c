#include <stdlib.h>
#include <skyeye_module.h>
#include <skyeye_mach.h>
#include "s3c2440.h"

const char *skyeye_module = "s3c2440";

void module_init(){
        /*
         * register the soc to the common library.
         */
	register_mach("s3c2440", s3c2440_mach_init);
}

void module_fini(){
}
