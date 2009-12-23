#include "skyeye_module.h"

const char* skyeye_module = "nandflash";

void module_init(){
	nandflash_register();
}

void module_fini(){
}
