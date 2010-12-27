#include <stdio.h>
#include <errno.h>
#include "skyeye_types.h"
#include "skyeye_arch.h"
#include "skyeye_module.h"

/* module name */
const char* skyeye_module = "log-pc";

int log_init();
int log_fini();
/* module initialization and will be executed automatically when loading. */
void module_init(){
	log_init();
}

/* module destruction and will be executed automatically when unloading */
void module_fini(){
	log_fini();
}
