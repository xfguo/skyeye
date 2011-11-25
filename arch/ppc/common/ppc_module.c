#include <stdlib.h>
#include <stdio.h>

#include "skyeye_mach.h"
#include "skyeye_module.h"
#include "skyeye_log.h"

const char *skyeye_module = "ppc";

extern machine_config_t ppc_machines[];
extern exception_t init_ppc_dyncom();
extern void init_ppc_arch();

void module_init()
{
	init_ppc_arch();
#ifdef LLVM_EXIST
	skyeye_log(Debug_log, __FUNCTION__, "LLVM exist and dyncom registered.\n");
	init_ppc_dyncom();
#else
	skyeye_log(Debug_log, __FUNCTION__, "LLVM not exist and dyncom not available.\n");
#endif
	/*
	 * register all the supported mach to the common library.
	 */
	int i = 0;
	while (ppc_machines[i].machine_name != NULL) {
		register_mach(ppc_machines[i].machine_name,
			      ppc_machines[i].mach_init);
		i++;
	}
}

void module_fini()
{
}
