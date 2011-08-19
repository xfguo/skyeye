#include "skyeye_module.h"

const char* skyeye_module = "touchscreen";

extern void touchscreen_register();

void module_init(){
	touchscreen_register();
}

void module_fini(){
}
