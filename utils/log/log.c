#include <stdlib.h>
#include <stdio.h>
#include "skyeye_arch.h"
#include "skyeye_callback.h"
//#include "arm_regformat.h"

/* flag to enable log function. */
static int enable_log_flag;

/* the log file for record. */
static const char* log_filename = "./pc.log";

/* fd of log_filename */
static FILE* log_fd;

/* callback function for step exeuction. Will record pc here. */
static void log_pc_callback(generic_arch_t* arch_instance){
	if(enable_log_flag){
		fprintf(log_fd, "pc=0x%x\n", arch_instance->get_pc());
#if 0
		//fprintf(skyeye_logfd, "pc=0x%x,r3=0x%x\n", pc, state->Reg[3]);
		 if(arch_instance->get_regval_by_id){
                        /* only for arm */
                        int i = 0;
                        uint32 reg_value = 0;
                        while(arch_instance->get_regname_by_id){
                                reg_value = arch_instance->get_regval_by_id(i);
                                printf("%s\t0x%x\n", arch_instance->get_regname_by_id, reg_value);
                                i++;
                        }
                }
		else{
		}
#endif
	}
}

/* enable log functionality */
static void com_log_pc(char* arg){
	enable_log_flag = 1;
	/* open file for record pc */
	log_fd = fopen(log_filename, "w");	
	if(log_fd == NULL){
		fprintf(stderr, "Can not open the file %s for log-pc module.\n", log_filename);
		return;
	}

}

/* some initialization for log functionality */
int log_init(){
	exception_t exp;
	/* register callback function */
	register_callback(log_pc_callback, Step_callback);
	/* add correspinding command */
	add_command("log-pc", com_log_pc, "record the every pc to log file.\n");
}

/* destruction function for log functionality */
int log_fini(){
	if(log_fd != NULL){
		fclose(log_fd);
	}
}
