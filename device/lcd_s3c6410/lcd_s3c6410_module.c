#include "skyeye_module.h"

const char* skyeye_module = "lcd_s3c6410";

extern void init_lcd_s3c6410();

void module_init(){
	init_lcd_s3c6410();
}

void module_fini(){
}
