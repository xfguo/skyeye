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


#include "skyeye_types.h"
#include "skyeye_config.h"
#include "skyeye_arch.h"
#include "skyeye_mm.h"
#include "skyeye_options.h"
#include "skyeye_signal.h"
#include "skyeye_cell.h"
#include <skyeye_log.h>
#ifdef __CYGWIN__
#include <sys/time.h>
#endif

#include "armdefs.h"
#include "armcpu.h"
#include "skyeye_dyncom.h"
#include "arm_dyncom_run.h"
#include "arm_arch_interface.h"
#include "arm_dyncom_translate.h"

static void
arm_reset_state ()
{
}

static void per_cpu_step(arm_core_t * core){
	arm_dyncom_run((cpu_t*)get_cast_conf_obj(core->dyncom_cpu, "cpu_t"));
}

static void per_cpu_stop(arm_core_t * core){
}

static bool arm_cpu_init()
{
	skyeye_config_t* config = get_current_config();
	machine_config_t *mach = config->mach;
	ARM_CPU_State* cpu = (ARM_CPU_State*)skyeye_mm_zero(sizeof(ARM_CPU_State));
	if(!cpu){
		skyeye_log(Critical_log, __FUNCTION__, "Can not allocate the enough memory for cpu.\n");
		skyeye_exit(-1);
	}
	mach->cpu_data = get_conf_obj_by_cast(cpu, "ARM_CPU_State");

	cpu->core_num = 1;

	if(!cpu->core_num){
		skyeye_log(Critical_log, __FUNCTION__, "Can not get the core number or set wrong mach name?\n");
		skyeye_exit(-1);
	}
	else
		cpu->core = (arm_core_t*)skyeye_mm(sizeof(arm_core_t) * cpu->core_num);
	/* TODO: zero the memory by malloc */

	if(!cpu->core){
		skyeye_log(Critical_log, __FUNCTION__, "Can not allocate memory for ppc core.\n");
		skyeye_exit(-1);
	}
	else
		skyeye_log(Info_log, __FUNCTION__, "Initilization for %d core\n", cpu->core_num);

	int i;
	for(i = 0; i < cpu->core_num; i++){
		arm_core_t* core = &cpu->core[i];
		arm_core_init(core, i);
		arm_dyncom_init(core);

		skyeye_exec_t* exec = create_exec();
		//exec->priv_data = get_conf_obj_by_cast(core, "arm_core_t");
		exec->priv_data = (conf_object_t*)core;
		exec->run =  (void (*)(conf_object_t*))per_cpu_step;
		exec->stop = (void (*)(conf_object_t*))per_cpu_stop;
		add_to_default_cell(exec);
	}

	cpu->boot_core_id = 0;

	return true;
}

/**
* @brief Initalization for powerpc architecture
*/
static void
arm_init_state ()
{
	arm_cpu_init();
}


static void
arm_step_once ()
{
	int i;
	machine_config_t* mach = get_current_mach();
	ARM_CPU_State* cpu = get_current_cpu();
	/* workaround boot sequence for dual core, we need the first core initialize some variable for second core. */
	arm_core_t* core;
	for(i = 0; i < cpu->core_num; i++ ){
		core = &cpu->core[i];
		/* if CPU1_EN is set? */
			per_cpu_step(core);
	}
	/* for peripheral */
	mach->mach_io_do_cycle(cpu);
}

static void arm_stop(uint32_t id){
	return;
}

static void
arm_set_pc (generic_address_t pc)
{
	int i;
	ARM_CPU_State* cpu = get_current_cpu();
	cpu->core[0].Reg[15] = pc;
}

static uint32_t
arm_get_pc(){
	ARM_CPU_State* cpu = get_current_cpu();
	return cpu->core[0].Reg[15];
}
/*
 * Since mmu of arm always enabled, so we can write virtual address here
 */
static int
arm_ICE_write_byte (generic_address_t addr, uint8_t v)
{
	//return bus_write(8, addr, v);

	/* if failed, return -1*/
	return 0;
}

/*
 * Since mmu of arm always enabled, so we can read virtual address here
 */
static int arm_ICE_read_byte (generic_address_t addr, uint8_t *pv){
	/**
	 *  work around for ppc debugger
	 */
	int ret;
	//return bus_read(8, addr, pv);
	return 0;
}

static int
arm_parse_cpu (const char *params[])
{
	return 0;
}

/**
* @brief get the current pc for a specified core
*
* @return the current step count for the core
*/
static uint32 arm_get_step(){
	ARM_CPU_State* cpu = get_current_cpu();
	return cpu->core[0].step;
}
static char* arm_get_regname_by_id(int id){
        return NULL;
}
static uint32 arm_get_regval_by_id(int id){
	/* we return the reg value of core 0 by default */
	int core_id = 0;
	ARM_CPU_State* cpu = get_current_cpu();
}

#ifdef __cplusplus
	extern "C" {
#endif
exception_t
init_arm_dyncom ()
{

	arch_config_t* arm_arch = (arch_config_t*)skyeye_mm_zero(sizeof(arch_config_t));

	arm_arch->arch_name = "arm_dyncom";
	arm_arch->init = arm_init_state;
	arm_arch->reset = arm_reset_state;
	arm_arch->set_pc = arm_set_pc;
	arm_arch->get_pc = arm_get_pc;
	arm_arch->get_step = arm_get_step;
	arm_arch->step_once = arm_step_once;
	arm_arch->stop = arm_stop;
	arm_arch->ICE_write_byte = arm_ICE_write_byte;
	arm_arch->ICE_read_byte = arm_ICE_read_byte;
	arm_arch->parse_cpu = arm_parse_cpu;
	arm_arch->get_regval_by_id = arm_get_regval_by_id;
	arm_arch->get_regname_by_id = arm_get_regname_by_id;

	register_arch (arm_arch);
	return No_exp;
}
#ifdef __cplusplus
	}
#endif
