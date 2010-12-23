/*
        log.c - log pc for the instruction execution flow on SkyEye.
        Copyright (C) 2003-2010 Skyeye Develop Group
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
 * 01/16/2010   Michael.Kang  <blackfin.kang@gmail.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include "skyeye_arch.h"
#include "skyeye_callback.h"
#include "skyeye_options.h"
//#include "arm_regformat.h"

/* flag to enable log function. */
static bool_t enable_log_flag;

/* the log file for record. */
//static const char* log_filename = "./pc.log";

static const char* log_option_name = "instr_log";
static const char log_filename[30];

static uint32 range_begin = 0, range_end = ~0, trigger_start = ~0, trigger_stop = ~0;

/* fd of log_filename */
static FILE* log_fd;

/**
 *parse the configuration file
 */
int instr_log_parse(struct skyeye_option_t *option, int num_params, const char *params[]) 
{
	int i;
	char *p;
	char name[MAX_PARAM_NAME], value[MAX_PARAM_NAME];

	for (i = 0; i < num_params; i++) {
		if (split_param (params[i], name, value) < 0)
			SKYEYE_ERR
				("Error: %s option has wrong parameter \"%s\".\n", log_option_name,
				 name);
		if (!strncmp ("range_begin", name, strlen (name))) {
			sscanf (value, "%x", &range_begin);
		}
		else if (!strncmp ("trigger_start", name, strlen (name))) {
			sscanf (value, "%x", &trigger_start);
		}
		else if (!strncmp ("range_end", name, strlen (name))) {
			sscanf (value, "%x", &range_end);
		}
		else if (!strncmp ("trigger_stop", name, strlen (name))) {
			sscanf (value, "%x", &trigger_stop);
		}

		else if (!strncmp ("filename", name, strlen (name))) {
			strcpy(log_filename, value);
			 log_fd = fopen(log_filename, "w");
			if(log_fd == NULL){
				fprintf(stderr, "Can not open the file %s for log-pc module.\n", log_filename);
				return;
			}
		}
		else
			SKYEYE_ERR ("%s Error: Unkonw load_addr option  \"%s\"\n", log_option_name, params[i]);
	}
	return 0;
}


/* callback function for step exeuction. Will record pc here. */
static void log_pc_callback(generic_arch_t* arch_instance){
	generic_address_t pc = arch_instance->get_pc();
	if(trigger_start == pc || (trigger_start == trigger_stop)){
		enable_log_flag =True;
	}
	if(trigger_stop == pc)
		enable_log_flag = False;
	if(enable_log_flag == True){
		if((pc >= range_begin) && (pc < range_end)){
			fprintf(log_fd, "pc=0x%x\n", arch_instance->get_pc());
		}
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
	register_option(log_option_name, instr_log_parse, "Log every executed instruction");
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
