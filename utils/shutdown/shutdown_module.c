/*
        shutdown_module.c - quit the simulator when write the  special io address
	or execute max number of instructions.
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
 * port the patch from xi.yang
 * 01/02/2010   Michael.Kang  <blackfin.kang@gmail.com>
 */
#include <stdlib.h>
#include <stdio.h>
#include "skyeye_arch.h"
#include "skyeye_callback.h"
#include "skyeye_bus.h"
#include "skyeye_options.h"
#include "skyeye_command.h"
#include "skyeye_module.h"

/*
 * If status of shutdown device is 1, when "shutdown_addr"
 * (align on 8bytes) is written, SKYEYE simulator exits.
 * If the number of executed instructions by skyeye simulator
 * is bigger than "max_instructions", then skyeye return with
 * value 2
*/
typedef struct shutdown_config
{
	uint32 shutdown_addr;
	unsigned long long  max_ins;
}shutdown_config_t;
static shutdown_config_t* shutdown = NULL;
static max_insn_shutdown_enable = 0;
static addr_access_shutdown_enable = 0;

static int
do_shutdown_option (skyeye_option_t * this_option, int num_params,
		const char *params[])
{
	int ret;
	char *value;
	if(ret = strncmp(params[0],"addr=",5)){
		SKYEYE_ERR ("Error, Wrong parameter for shutdown_device\n");
		return -1;
	}
	value = &(params[0][5]);
	if(value[0]=='0' && (value[1] == 'x' || value[1] == 'X'))
		shutdown->shutdown_addr = (unsigned int)strtoul(value,NULL,16);
	else
		shutdown->shutdown_addr = (unsigned int)strtoul(value,NULL,10);
	if(shutdown->shutdown_addr & 0x7){
		SKYEYE_ERR ("Error, shutdown address needs align on 8 bytes\n");
		return -1;
	}
	addr_access_shutdown_enable = 1;

	if(ret = strncmp(params[1],"max_ins=",8)){
		SKYEYE_ERR ("Error, Wrong parameter for shutdown_device\n");
		return -1;
	}
	value = &(params[1][8]);
	if(value[0]=='0' && (value[1] == 'x' || value[1] == 'X'))
		shutdown->max_ins = strtoull(value,NULL,16);
	else
		shutdown->max_ins = strtoull(value,NULL,10);
	max_insn_shutdown_enable = 1;

	printf("Shutdown addr=%x, max_ins=%x\n",shutdown->shutdown_addr,shutdown->max_ins);
	return 1;
}


/* callback function for step exeuction. */
static void max_insn_callback(generic_arch_t* arch_instance){
	uint32 step = arch_instance->get_step();
	if(!max_insn_shutdown_enable)
		return;
	if(step == shutdown->max_ins)
		run_command("quit");
}

/* callback function for bus write. Will record pc here. */
static void write_shutdown_callback(generic_arch_t* arch_instance){
	bus_recorder_t* buffer = get_last_bus_access(SIM_access_write);
	if(!addr_access_shutdown_enable)
		return;
	if(buffer->addr == shutdown->shutdown_addr)
		run_command("quit");
}

/* module name */
const char* skyeye_module = "shutdown";

/* module initialization and will be executed automatically when loading. */
void module_init(){
	shutdown = malloc(sizeof(shutdown_config_t));
	register_callback(max_insn_callback, Step_callback);
	register_callback(write_shutdown_callback, Bus_write_callback);
	if(register_option("shutdown_device", do_shutdown_option, "Used to stop machine by writing a special address or set the max executed instruction number.") != No_exp)
		fprintf(stderr,"Can not register shutdown_device option\n");
}

/* module destruction and will be executed automatically when unloading */
void module_fini(){
	if(shutdown != NULL){
		free(shutdown);
		shutdown = NULL;
	}
}
