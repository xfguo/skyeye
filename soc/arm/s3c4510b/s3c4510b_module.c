#include <stdlib.h>
#include <skyeye_module.h>
#include <skyeye_mach.h>
#include "s3c4510b.h"

const char* skyeye_module = "s3c4510b";

void 
module_init()
{
	/*
	 * register the soc to the common library.
	 */
 	register_mach("s3c4510b", s3c4510b_mach_init);
}

void 
module_fini()
{
}
