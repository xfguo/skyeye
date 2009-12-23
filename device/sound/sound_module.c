#include "skyeye_module.h"

const char* skyeye_module = "sound";

void module_init(){
	//init_bfin_arch ();
	sound_register();
}

void module_fini(){
}
