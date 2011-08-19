#include "skyeye_module.h"

const char* skyeye_module = "uart";

extern void uart_register();
extern skyeye_uart_cleanup();

void module_init(){
	//init_bfin_arch ();
	uart_register();
}

void module_fini(){
	skyeye_uart_cleanup();
}
