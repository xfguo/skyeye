#include <stdlib.h>
#include <skyeye_module.h>
#include <skyeye_arch.h>
#include <skyeye_mach.h>

void imx_mach_init (generic_arch_t *arch_instance, machine_config_t *this_mach);
const char* skyeye_module = "imx9238";

void module_init(){
        /*
         * register the soc to the common library.
         */
	register_mach("imx9238", imx_mach_init);
}

void module_fini(){
}
