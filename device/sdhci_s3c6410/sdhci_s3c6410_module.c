#include "skyeye_module.h"

const char* skyeye_module = "sdhci_s3c6410";

extern void init_s3c6410_sdhci();

void module_init(){
	init_s3c6410_sdhci();
}

void module_fini(){
}
