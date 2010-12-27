/*
        skyeye2gdb.c - necessary definition for skyeye debugger
        Copyright (C) 2003 Skyeye Develop Group
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
 * 12/04/2005   ksh  <blackfin.kang@gmail.com>
 * */

//#include "armdefs.h"

#include "skyeye_types.h"
#include "skyeye_ram.h"
#include "sim_control.h"
#include "skyeye_arch.h"
#include "breakpoint.h"
//#include "memory.h"
//#include "armemu.h"
//#include "arch_regdefs.h"
//#include "skyeye_defs.h"
#include <stdlib.h>
#include <stdint.h>
#if 0
extern struct ARMul_State * state;
extern struct _memory_core memory_core;
extern generic_arch_t * arch_instance;
#endif
#include "skyeye_pref.h"
int
frommem (unsigned char *memory)
{
	sky_pref_t* pref = get_skyeye_pref();
	//pref->endian = Big_endian;
	if (pref->endian == Big_endian) {
		return (memory[0] << 24)
			| (memory[1] << 16) | (memory[2] << 8) | (memory[3] <<
								  0);
	}
	else {
		return (memory[3] << 24)
			| (memory[2] << 16) | (memory[1] << 8) | (memory[0] <<
								  0);
	}
}


void
tomem (unsigned char *memory, int val)
{
	sky_pref_t* pref = get_skyeye_pref();
	//pref->endian = Big_endian;
	if (pref->endian == Big_endian) {
		memory[0] = val >> 24;
		memory[1] = val >> 16;
		memory[2] = val >> 8;
		memory[3] = val >> 0;
	}
	else {
		memory[3] = val >> 24;
		memory[2] = val >> 16;
		memory[1] = val >> 8;
		memory[0] = val >> 0;
	}
}
#if 0
ARMword
ARMul_Debug (ARMul_State * state, ARMword pc ATTRIBUTE_UNUSED,
	     ARMword instr ATTRIBUTE_UNUSED)
{
	state->Emulate = STOP;
	stop_simulator = 1;
	return 1;
}
#endif

int
sim_write (generic_address_t addr, unsigned char *buffer, int size)
{
	int i;
	int fault=0;
	skyeye_config_t* config = get_current_config();
	generic_arch_t *arch_instance = get_arch_instance(config->arch->arch_name);
	for (i = 0; i < size; i++) {
		if(arch_instance->mmu_write != NULL)
			fault = arch_instance->mmu_write(8, addr + i, buffer[i]);
		else
			mem_write(8, addr + i, buffer[i]);
		if(fault) return -1; 
	}
	return size;
}

int
sim_read (generic_address_t addr, unsigned char *buffer, int size)
{
	int i;
	int fault = 0;
	unsigned char v;
	skyeye_config_t* config = get_current_config();
	generic_arch_t *arch_instance = get_arch_instance(config->arch->arch_name);
	for (i = 0; i < size; i++) {
		if(arch_instance->mmu_read != NULL)
			fault = arch_instance->mmu_read(8, addr+i, &v);
		else
			fault = mem_read(8, addr + i, &v);
		if(fault) 
			return -1; 
		buffer[i]=v;
	}
	return size;
}

void gdbserver_cont(){
#if 0
         if(!strcmp(skyeye_config.arch->arch_name,"arm")){
                //chy 2006-04-12
                state->NextInstr = RESUME;      /* treat as PC change */
                state->Reg[15]=ARMul_DoProg (state);
        }
	else
		sim_resume(0);
#endif
	SIM_continue(0);
	while(SIM_is_running())
		;
	return;
}
void gdbserver_step(){
	skyeye_stepi(1);
	while(SIM_is_running())
		;
#if 0
         if(!strcmp(skyeye_config.arch->arch_name,"arm")){
                //chy 2006004-12
                state->NextInstr = RESUME;
                state->Reg[15]=ARMul_DoInstr (state);
        }
	else
		sim_resume(1);
#endif
}

int sim_ice_breakpoint_remove(generic_address_t addr){
	if(skyeye_remove_bp_by_addr(addr) != No_exp)
		return -1;
	else
		return 0;
}

int sim_ice_breakpoint_insert(generic_address_t addr){
	int ret = skyeye_insert_bp(SIM_access_execute, SIM_Break_Physical, addr);
	if(ret != 0)
		return -1;
	else
		return 0;
}
