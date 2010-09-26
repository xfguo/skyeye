#include <stdlib.h>
#include <skyeye_module.h>
#include <skyeye_mach.h>
#include "lh79520.h"

const char* skyeye_module = "lh79520";

void module_init(){
        /*
         * register the soc to the common library.
         */
	register_mach("lh79520", lh79520_mach_init);
}

void module_fini(){
}
