#include "skyeye_module.h"

const char* skyeye_module = "net";

extern void net_register();

void module_init(){
	net_register();
}

void module_fini(){
}
