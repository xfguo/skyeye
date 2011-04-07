#include <stdlib.h>
#include <skyeye_module.h>
#include <skyeye_options.h>
#include "skyeye_os.h"

const char *skyeye_module = "os_supp";

void module_init() {

    /* register the os to the common library. */
	init_os_option();
	register_option("os", do_os_option, "Set os you want to run on simulator.\n");
}

void module_fini() {
}
