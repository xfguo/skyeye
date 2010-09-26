#include <stdlib.h>
#include <skyeye_module.h>
#include <skyeye_mach.h>
#include "pxa.h"

const char *skyeye_module = "pxa250";

void module_init(){
        /*
         * register the soc to the common library.
         */
	register_mach("pxa_lubbock", pxa250_mach_init);
}

void module_fini(){
}
