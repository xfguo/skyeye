#include "skyeye_module.h"

const char* skyeye_module = "pci_bus";

void module_init(){
	init_pcie();
}

void module_fini(){
}
