/*
   cov_prof.c - used to record the WRX action for memory address
   Copyright (C) 2008 Skyeye Develop Group
   for help please send mail to <skyeye-developer@lists.gro.clinux.org>

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
 * 03/08/2008   Michael.Kang  <blackfin.kang@gmail.com>
 */

#include <stdio.h>
#include <strings.h>
#include <stdint.h>
#include <stdlib.h>
#include "skyeye_options.h"
#include "skyeye_config.h"
#include "skyeye_types.h"
#include "skyeye_arch.h"
#include "skyeye_command.h"
#include "skyeye_callback.h"
#include "portable/portable.h"
#define COV_ON 1
#define COV_OFF 0
#define COV_START 2

static int
do_code_cov_option (skyeye_option_t * this_option, int num_params,
		const char *params[]);
/**
 *  pointer of memory allocated for code coverage
 */
static uint8_t * prof_mem;
static size_t prof_start;
static size_t prof_end;

static char cov_state = COV_START;	//0: off, 1: on, 2: off(never on before) 
static char cov_filename[30];
void cov_init(size_t start_addr, size_t end_addr){

	size_t prof_size = end_addr - start_addr;

	/* we use four bits to record the WRX action for a 32 bit word */
	size_t mem_alloc = (prof_size / 4) / 2;
	prof_mem = malloc(prof_size);
	if(!prof_mem)
		fprintf(stderr, "Can not alloc memory for code coverage, profiling is disabled.\n");
	else{
		printf("Begin do code coverage between 0x%x and 0x%x .\n", prof_start, prof_end);
		bzero(prof_mem, prof_size);
	}
//	register_option("code_coverage", do_code_cov_option, "");
}
/**
 * flags: 4 means read, 2 means write, 1 means eXecute
 *
 */
	void cov_prof(int flags,generic_address_t addr){
		if(addr < prof_start || addr >= prof_end)
			return;	
		int offset = (addr - prof_start) / 8;
		unsigned int * prof_addr  =(unsigned int *)&prof_mem[offset] ;
		*prof_addr |= flags << ((addr - prof_start) % 8);
		//printf("addr=0x%x, flags=0x%x, offset=0x%x,(addr - prof_start)%8=0x%x\n", addr, flags, offset, (addr - prof_start)%8);
		return;
	}

/**
 * deinitialization function
 */
void cov_fini(char * filename){
	if (prof_mem == NULL)
		return;
	FILE * fp = fopen(filename, "w+");
	if(!fp){
		fprintf(stderr, "Warning: can not open file %s for code coverage\n", filename);
		return;
	}
	int count = fwrite(prof_mem, (prof_end - prof_start), 1, fp);
	if(count < (prof_end - prof_start))
		printf("Write %d bytes for code coverage .\n", (prof_end - prof_start));	
	fclose(fp);
	if(prof_mem){
		free(prof_mem);
		prof_mem = NULL;
	}
	return;
}
/**
 *parse the configuration file
 */

int cov_conf_parse(struct skyeye_option_t *option, int num_params, const char *params[]) 
{
	int i;
	char *p;
	char name[MAX_PARAM_NAME], value[MAX_PARAM_NAME];

	for (i = 0; i < num_params; i++) {
		if (split_param (params[i], name, value) < 0)
			SKYEYE_ERR
				("Error: code_coverage option has wrong parameter \"%s\".\n",
				 name);
		if (!strncmp ("start", name, strlen (name))) {
			sscanf (value, "%x", &prof_start);
		}
		else if (!strncmp ("end", name, strlen (name))) {
			sscanf (value, "%x", &prof_end);
		}
		else if (!strncmp ("filename", name, strlen (name))) {
			strcpy(cov_filename, value);
		}
		else
			SKYEYE_ERR ("Cov_conf Error: Unkonw load_addr option  \"%s\"\n", params[i]);
	}

	
	return 0;
}
static void cov_execmem_callback(generic_arch_t* arch_instance)
{
	if (cov_state == COV_ON)
		cov_prof(1,arch_instance->get_pc());
}
static void cov_readmem_callback(generic_arch_t* arch_instance)
{
	if (cov_state == COV_ON)
		cov_prof(4,arch_instance->get_pc());
}
static void cov_writemem_callback(generic_arch_t* arch_instance)
{
	if (cov_state == COV_ON)
		cov_prof(2,arch_instance->get_pc());
}

void cov_state_on(char *arg)
{

	if (cov_state == COV_START) { //run this function first time
		cov_init(prof_start, prof_end);
		register_callback(cov_readmem_callback, Mem_read_callback);
		register_callback(cov_writemem_callback, Mem_write_callback);
		register_callback(cov_execmem_callback, Step_callback);
	}else
		printf("code coverage state: on\n");

	cov_state = COV_ON;
}

void cov_state_off(char *arg)
{
	printf("code coverage state: off\n");
	if (cov_state == COV_OFF || cov_state == COV_START)
		return;
	cov_state = COV_OFF;
	return;
}

void cov_state_show(char *arg)
{
	if (cov_state == COV_OFF || cov_state == COV_START)
		printf("code coverage state: off\n");
	else
		printf("code coverage state: on\n");
	return;
}

int cov_module_init()
{
	register_option("code_coverage", cov_conf_parse, "code coverage module");
	add_command("cov-on", cov_state_on, "turn on code coverage switch.\n");
	add_command("cov-off", cov_state_off, "turn off code coverage switch.\n");
	add_command("cov-state", cov_state_show, "show code coverage state.\n");
	return 0;
}

int cov_module_exit()
{
	cov_fini(cov_filename);
	return 0;
}

