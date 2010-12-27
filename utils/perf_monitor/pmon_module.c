#include <stdio.h>
#include <errno.h>
#include "skyeye_types.h"
#include "skyeye_arch.h"
#include "skyeye_module.h"

int pmon_init();
int pmon_fini();
/* module name */
const char* skyeye_module = "perf-monitor";

/* module initialization and will be executed automatically when loading. */
void module_init(){
	pmon_init();
}

/* module destruction and will be executed automatically when unloading */
void module_fini(){
	pmon_fini();
}
