#include <stdlib.h>

#include "skyeye_arch.h"
#include "skyeye_callback.h"
/*
 * That is an absolute value for step. Simulator should stop when hit this value 
 */
static int stopped_step = 0;
static void check_steps(generic_arch_t* arch_instance){
	int current_step;
	if(arch_instance->get_step){
		current_step = arch_instance->get_step();
	}
	else
		return;
	if(!stopped_step)
		return;
	if(current_step == stopped_step){
		SIM_stop();
		stopped_step = 0;
	}
	return;
}

static int com_show_step(char* arg){
	generic_arch_t* arch_instance = get_arch_instance();
	if(!arch_instance)
		return 1;
	uint32 step = arch_instance->get_step();
	printf("steps: %d\n", step);
}

void init_stepi(){
	register_callback(check_steps, Step_callback);
	add_command("show-step", com_show_step, "Show the steps of current processor.\n");
	//add_command("show-step", com_show_step, "Show the steps of current processor.\n");
}

/*
 * 
 */
void skyeye_stepi(int steps){
	/* FIXME, that is not true for variable length of ISA, so get_next_pc should implemented for every core  */
	generic_arch_t* arch_instance = get_arch_instance();
	/* we do not have valid arch_instance now */
	if(arch_instance == NULL){
		return;
	}
	if(arch_instance->get_step == NULL){
		printf(Warnning_log, __FUNCTION__, "The arch have not implemented get_step.\n");
		return;
	}
	stopped_step  = arch_instance->get_step() + steps;
	skyeye_log(Debug_log, __FUNCTION__, "stopped_step=%d\n", stopped_step);
	SIM_continue();
	while(arch_instance->get_step() < stopped_step)
	{
			usleep(10);
	}
}
