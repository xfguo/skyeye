#ifndef __COMMON_SKY_CONTROL_H__
#define __COMMON_SKY_CONTROL_H__
#include "skyeye_arch.h"

#ifdef __cplusplus
 extern "C" {
#endif 
//void SIM_init_command_line(void);
//void SIM_init_environment(char **argv, bool handle_signals);
//void SIM_init_simulators(init_prefs_t *init_prefs);

/*
 * initialization of simulator.
 */
void SIM_init();

/*
 * read and parse the specific config file and initilization the data.
 */
void SIM_start();

/*
 * Launch the simulator.
 */
void SIM_run();

/*
 * Enters the main loop of SkyEye and never returns. It should only be called
 * When embedding SkyEye in another application that wishes to transfer full 
 * control of the simulations to SkyEye.
 */
void SIM_main_loop(void);

/*
 * Ask SkyEye to stop the simulation as soon as possible, displaying the supplied 
 * messages.
 */
void SIM_break_simulation(const char *msg);

/*
 * Run the simulation. if step is zero, the simulation will run forward until it
 * is stoppped.
 */
void SIM_continue(generic_arch_t* arch_instance);
/* 
 * stop a processor 
 */
void SIM_stop(generic_arch_t* arch_instance);

void skyeye_loop();
#ifdef __cplusplus
}
#endif 

#endif
