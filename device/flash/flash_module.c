#include "skyeye_module.h"

const char* skyeye_module = "flash";
void module_init(){
	flash_register();
}

void module_fini(){
}
