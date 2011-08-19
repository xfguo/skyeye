#include "skyeye_module.h"

extern flash_register();

const char* skyeye_module = "flash";
void module_init(){
	flash_register();
}

void module_fini(){
}
