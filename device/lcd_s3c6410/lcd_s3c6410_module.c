#include "skyeye_module.h"

const char* skyeye_module = "lcd_s3c6410";

extern void init_s3c6410_lcd();

void module_init(){
	init_s3c6410_lcd();
}

void module_fini(){
}
