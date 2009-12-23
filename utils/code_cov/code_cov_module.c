#include "skyeye_module.h"
const char* skyeye_module = "code_cov";
void module_init(){
cov_module_init();
}
void module_fini(){
cov_module_exit();
}

