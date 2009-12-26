#include <stdlib.h>
#include <stdio.h>
#include "skyeye_arch.h"
#include "skyeye_callback.h"
#include "skyeye_bus.h"

/* flag to enable log function. */
static int enable_log_flag;

/* the log file for record. */
static const char* log_filename = "./bus.log";

/* fd of log_filename */
static FILE* log_fd;

const char* read_str = "read";
const char* write_str = "write";
const char* before_str = "before";
const char* after_str = "after";


/* callback function for bus access. Will record pc here. */
static void log_bus_callback(generic_arch_t* arch_instance){
	if(enable_log_flag){
		bus_recorder_t* recorder_buffer;
		recorder_buffer = get_last_bus_access();
		char* access;
		char* when;
		if(recorder_buffer->rw == SIM_access_read)
			access = read_str;
		else
			access = write_str;
		if(recorder_buffer->when == Before_act)
			when = before_str;
		else
			when = after_str;

		fprintf(log_fd, "Bus %s %s access @0x%x, size=%d, addr=0x%x, value=0x%x\n", 
		when, access, arch_instance->get_pc(), recorder_buffer->size, recorder_buffer->addr, recorder_buffer->value);
	}
}

/* enable log functionality */
static void com_log_bus(char* arg){
	enable_log_flag = 1;
	/* open file for record pc */
	log_fd = fopen(log_filename, "w");	
	if(log_fd == NULL){
		fprintf(stderr, "Can not open the file %s for log-bus module.\n", log_filename);
		return;
	}

}

/* some initialization for log functionality */
int bus_log_init(){
	exception_t exp;
	/* register callback function */
	register_callback(log_bus_callback, Bus_read_callback);
	register_callback(log_bus_callback, Bus_write_callback);
	/* add corresponding command */
	add_command("log-bus", com_log_bus, "record every bus access to log file.\n");
}

/* destruction function for log functionality */
int bus_log_fini(){
	if(log_fd != NULL){
		fclose(log_fd);
	}
}
