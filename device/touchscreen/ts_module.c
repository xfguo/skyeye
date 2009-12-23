#include "skyeye_module.h"

const char* skyeye_module = "touchscreen";

void module_init(){
	touchscreen_register();
}

void module_fini(){
}
