/*
        bus_log.c - record the bus access
        Copyright (C) 2003-2007 Skyeye Develop Group
        for help please send mail to <skyeye-developer@lists.sf.linuxforum.net>

        This program is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2 of the License, or
        (at your option) any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program; if not, write to the Free Software
        Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
/*
 * 01/02/2010   Michael.Kang  <blackfin.kang@gmail.com>
 */
#include <stdlib.h>
#include <stdio.h>
#include "skyeye_arch.h"
#include "skyeye_callback.h"
#include "skyeye_command.h"
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
	unsigned int current_pc = arch_instance->get_pc();
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

		if(!strncmp("arm", arch_instance->arch_name, strlen("arm"))){
			current_pc -=8;
		}
		fprintf(log_fd, "Bus %s %s access @0x%x, size=%d, addr=0x%x, value=0x%x\n", 
		when, access, current_pc, recorder_buffer->size, recorder_buffer->addr, recorder_buffer->value);
	}
}

/* enable log functionality */
void com_log_bus(char *arg) {
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
