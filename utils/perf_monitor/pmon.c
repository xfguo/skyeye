#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include "skyeye_arch.h"
#include "skyeye_callback.h"
#include "sim_control.h"
#include "skyeye_command.h"
#include "skyeye_thread.h"

/* flag to enable performence monitor function. */
static int enable_pmon_flag;

/* the log file for record. */
static const char* pmon_filename = "./pmon.log";

/* fd of log_filename */
static FILE* pmon_fd;

static void pmon_count(generic_arch_t* arch_instance){
	int seconds = 0;
	uint32 steps = 0;
	uint32 last_steps = 0;
	/* Test if skyeye is in running state. */
	while(!SIM_is_running())
		;
	while(enable_pmon_flag){
		last_steps = arch_instance->get_step();
		sleep(1);
		seconds++;
		steps = arch_instance->get_step();
		fprintf(pmon_fd,"In %d seconds, MIPS=%d\n", seconds, (steps - last_steps));
	}
}

/* enable log functionality */
static void com_pmon(char* arg){
	enable_pmon_flag = 1;
	/* open file for record performance data */
	pmon_fd = fopen(pmon_filename, "w");	
	if(pmon_fd == NULL){
		fprintf(stderr, "Can not open the file %s for pmon module.\n", pmon_filename);
		return;
	}
	pthread_t id;
	generic_arch_t* arch_instance = get_arch_instance("");
	create_thread(pmon_count, arch_instance, &id);
}

/* some initialization for log functionality */
int pmon_init(){
	exception_t exp;
	/* register callback function */
	//register_callback(log_pc_callback, Step_callback);
	/* add correspinding command */
	add_command("pmon", com_pmon, "enable the performance monitor.\n");
}

/* destruction function for log functionality */
int pmon_fini(){
	if(pmon_fd != NULL){
		fclose(pmon_fd);
	}
}
