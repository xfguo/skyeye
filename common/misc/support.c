#include "skyeye_config.h"
static void display_arch_support(
	const char * arch,
	machine_config_t machines[]
){
	int i;
        printf ( "-------- %s architectures ---------\n", arch );

	for ( i=0 ; machines[i].machine_name ; i++ )
		printf("%s \n",machines[i].machine_name);
}
void display_all_support(){
#if 0
	extern machine_config_t arm_machines[];
	extern machine_config_t bfin_machines[];
	extern machine_config_t coldfire_machines[];
	extern machine_config_t mips_machines[];
	extern machine_config_t ppc_machines[];

 	printf (
                  "----------- Architectures and CPUs simulated by SkyEye-------------\n");
	display_arch_support( "ARM", arm_machines );
	display_arch_support( "BlackFin", bfin_machines );
	display_arch_support( "Coldfire", coldfire_machines );
	display_arch_support( "MIPS", mips_machines );
	display_arch_support( "PowerPC", ppc_machines );
#endif
}

