#include <stdio.h>
#include <errno.h>
#include "skyeye_types.h"
#include "skyeye_arch.h"
#include "skyeye_module.h"

int bus_log_init();
int bus_log_fini();
/* module name */
const char* skyeye_module = "bus-log";

/* module initialization and will be executed automatically when loading. */
void module_init(){
	bus_log_init();
}

/* module destruction and will be executed automatically when unloading */
void module_fini(){
	bus_log_fini();
}
