#include <stdlib.h>
#include <skyeye_module.h>
#include <skyeye_mach.h>
#include "sharp.h"

const char* skyeye_module = "sharp";

void module_init(){
        /*
         * register the soc to the common library.
         */
	register_mach("sharp_lh7a400", shp_mach_init);
}

void module_fini(){
}
