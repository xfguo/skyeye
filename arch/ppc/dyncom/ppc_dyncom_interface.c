/* 
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/*
 * 12/06/2007   Michael.Kang  <blackfin.kang@gmail.com>
 */

#include "ppc_cpu.h"
#include "ppc_mmu.h"
#include "ppc_exc.h"
#include "ppc_e500_exc.h"
#include "ppc_memory.h"
#include "ppc_io.h"
#include "types.h"
#include "tracers.h"
#include "sysendian.h"
#include "ppc_irq.h"
//#include "ppc_regformat.h"
#include "bank_defs.h"

#include "skyeye_types.h"
#include "skyeye_config.h"
#include "skyeye_arch.h"

#ifdef __CYGWIN__
#include <sys/time.h>
#endif

//PPC_CPU_State gCPU;

static void
ppc_reset_state ()
{
	//skyeye_config_t* config = get_current_config();
	//config->mach->mach_io_reset(&gCPU);
	//skyeye_config.mach->mach_io_reset(&gCPU);/* set all the default value for register */	
}

static bool ppc_cpu_init()
{
	skyeye_config_t* config = get_current_config();
	machine_config_t *mach = config->mach;
	PPC_CPU_State* cpu = (PPC_CPU_State*)skyeye_mm_zero(sizeof(PPC_CPU_State));
	if(!cpu){
		skyeye_log(Critical_log, __FUNCTION__, "Can not allocate the enough memory for cpu.\n");
		skyeye_exit(-1);
	}
	mach->cpu_data = cpu;
	if(!strcmp(mach->machine_name, "mpc8560")){
		cpu->core_num = 1;
	}
	else if(!strcmp(mach->machine_name, "mpc8572"))
		cpu->core_num = 2;
	else if(!strcmp(mach->machine_name, "mpc8641d"))
                cpu->core_num = 2;
        else
		cpu->core_num = 0;

	if(!cpu->core_num){
		skyeye_log(Critical_log, __FUNCTION__, "Can not get the core number or set wrong mach name?\n");
		skyeye_exit(-1);
	}
	else
		cpu->core = malloc(sizeof(e500_core_t) * cpu->core_num);
	/* TODO: zero the memory by malloc */

	if(!cpu->core){
		skyeye_log(Critical_log, __FUNCTION__, "Can not allocate memory for ppc core.\n");
		skyeye_exit(-1);
	}
	else
		skyeye_log(Info_log, __FUNCTION__, "Initilization for %d core\n", cpu->core_num);
	
	int i;
	for(i = 0; i < gCPU.core_num; i++){
		ppc_core_init(cpu->core[i], i);
		ppc_dyncom_init(cpu->core[i]);
	}

	current_core = &cpu->core[0];
	/* initialize decoder */
	ppc_dec_init();
	return true;
}

/**
* @brief Initalization for powerpc architecture
*/
static void
ppc_init_state ()
{
	ppc_cpu_init();
	
	/* initialize the alignment and endianess for powerpc */
	generic_arch_t* arch_instance = get_arch_instance(NULL);
	arch_instance->alignment = UnAlign;
	arch_instance->endianess = Big_endian;
}

static void per_cpu_step(e500_core_t * core){
	ppc_dyncom_run(core->dyncom_cpu);
}

/* Fixme later */
e500_core_t * current_core;

static void
ppc_step_once ()
{
	int i;
	machine_config_t* mach = get_current_mach();
	PPC_CPU_State* cpu = mach->cpu_data;	
	/* workaround boot sequence for dual core, we need the first core initialize some variable for second core. */
	e500_core_t* core;
	for(i = 0; i < cpu->core_num; i++ ){
		core = &cpu->core[i];
		/* if CPU1_EN is set? */
		if(!i || cpu->eebpcr & 0x2000000)
			per_cpu_step(core);
	}
	/* for peripheral */
	mach->mach_io_do_cycle(cpu);
}

static void ppc_stop(uint32_t id){
	return;
}

static void
ppc_set_pc (generic_address_t pc)
{
	int i;
	machine_config_t* mach = get_current_mach();
	PPC_CPU_State* cpu = mach->cpu_data;	
	cpu->core[0].pc = pc;
	/* Fixme, for e500 core, the first instruction should be executed at 0xFFFFFFFC */
	//gCPU.pc = 0xFFFFFFFC;
}
static generic_address_t
ppc_get_pc(int core_id){
	machine_config_t* mach = get_current_mach();
	PPC_CPU_State* cpu = mach->cpu_data;	
	return cpu->core[0].pc;
}
/*
 * Since mmu of ppc always enabled, so we can write virtual address here
 */
static int
ppc_ICE_write_byte (generic_address_t addr, uint8_t v)
{
	ppc_write_effective_byte(addr, v);

	/* if failed, return -1*/
	return 0;
}

/*
 * Since mmu of ppc always enabled, so we can read virtual address here
 */
static int ppc_ICE_read_byte (generic_address_t addr, uint8_t *pv){
	/**
	 *  work around for ppc debugger
	 */
	if ((addr & 0xFFFFF000) == 0xBFFFF000)
		return 0;

	ppc_read_effective_byte(addr, pv);
	return 0;
}

static int
ppc_parse_cpu (const char *params[])
{
	return 0;
}
#if 0
extern void mpc8560_mach_init();
extern void mpc8572_mach_init();
extern void mpc8641d_mach_init();
machine_config_t ppc_machines[] = {
        /* machine define for MPC8560 */
        {"mpc8560", mpc8560_mach_init, NULL, NULL, NULL},
	{"mpc8572", mpc8572_mach_init, NULL, NULL, NULL},
	{"mpc8641d", mpc8641d_mach_init, NULL, NULL, NULL},
	{NULL,	NULL,			NULL,NULL, NULL},
};
#endif
/**
* @brief get the current pc for a specified core
*
* @return the current step count for the core
*/
static uint32 ppc_get_step(){
	machine_config_t* mach = get_current_mach();
	PPC_CPU_State* cpu = mach->cpu_data;	
	return cpu->core[0].step;
}
static char* ppc_get_regname_by_id(int id){
        return NULL;
}
static uint32 ppc_get_regval_by_id(int id){
	/* we return the reg value of core 0 by default */
	int core_id = 0;
	machine_config_t* mach = get_current_mach();
	PPC_CPU_State* cpu = mach->cpu_data;	

	if(id >= 0 && id < 32)
        	return cpu->core[core_id].gpr[id];

	switch(id){
#if 0
		case PC:
			return cpu->core[core_id].pc;
		case MSR:
			return cpu->core[core_id].msr;
		case CR:
			return cpu->core[core_id].cr;
		case LR:
			return cpu->core[core_id].lr;
		case CTR:
			return cpu->core[core_id].ctr;
		case XER:
			return cpu->core[core_id].xer;
		case FPSCR:
			return cpu->core[core_id].fpscr;
#endif
		default:
			/* can not find any corrsponding register */
			return 0;
	}
}

static exception_t ppc_mmu_read(short size, generic_address_t addr, uint32_t * value){
	uint32 result;

	/**
         *  work around for ppc gdb remote debugger
         */
        if ((addr & 0xFFFFF000) == 0xBFFFF000)
                return 0;

	switch(size){
                case 8:
                        ppc_read_effective_byte (addr, &result);
			*(uint8_t *)value = (uint8_t)result;
                        break;
                case 16:
			ppc_read_effective_half(addr, &result);
                        *(uint16_t *)value = (uint16_t)result;
                        break;
                case 32:
			ppc_read_effective_word(addr, &result);
                        *value = result;
                        break;
                default:
                        fprintf(stderr, "In %s, invalid data length %d\n", __FUNCTION__, size);
                        return Invarg_exp;
        }
	return No_exp;
}

static exception_t ppc_mmu_write(short size, generic_address_t addr, uint32_t value){
	switch(size){
                case 8:
                        //mem_write_byte (offset, value);
			ppc_write_effective_byte(addr, value);
                        break;
                case 16:
                        //mem_write_halfword(offset, value);
			ppc_write_effective_half(addr, value);
                        break;
                case 32:
			ppc_write_effective_word(addr, value);
                        //mem_write_word(offset, value);
                        break;
                default:
			fprintf(stderr, "In %s, invalid data length %d\n", __FUNCTION__, size);
			return Invarg_exp;
	}
	return No_exp;
}
exception_t
init_ppc_dyncom ()
{

	arch_config_t* ppc_arch = (arch_config_t*)skyeye_mm_zero(sizeof(arch_config_t));

	ppc_arch->arch_name = "powerpc_dyncom";
	ppc_arch->init = ppc_init_state;
	ppc_arch->reset = ppc_reset_state;
	ppc_arch->set_pc = ppc_set_pc;
	ppc_arch->get_pc = ppc_get_pc;
	ppc_arch->get_step = ppc_get_step;
	ppc_arch->step_once = ppc_step_once;
	ppc_arch->stop = ppc_stop;
	ppc_arch->ICE_write_byte = ppc_ICE_write_byte;
	ppc_arch->ICE_read_byte = ppc_ICE_read_byte;
	ppc_arch->parse_cpu = ppc_parse_cpu;
	ppc_arch->get_regval_by_id = ppc_get_regval_by_id;
        ppc_arch->get_regname_by_id = ppc_get_regname_by_id;
	ppc_arch->mmu_read = ppc_mmu_read;
	ppc_arch->mmu_write = ppc_mmu_write;

	register_arch (ppc_arch);
	return No_exp;
}
