#include "skyeye_module.h"

const char* skyeye_module = "sound";

extern void sound_register();

void module_init(){
	//init_bfin_arch ();
	sound_register();
}

void module_fini(){
}
