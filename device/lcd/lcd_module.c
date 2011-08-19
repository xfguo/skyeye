#include "skyeye_module.h"

const char* skyeye_module = "lcd";

extern void lcd_register ();

void module_init(){
	//init_bfin_arch ();
	lcd_register();
}

void module_fini(){
}
