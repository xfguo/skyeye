#include "skyeye_module.h"

const char* skyeye_module = "uart_16550";

extern void init_uart_16550();

void module_init(){
	init_uart_16550();
}

void module_fini(){
}
