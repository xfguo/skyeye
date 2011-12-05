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

//#include "common/bus/io.h"
#include "armemu.h"
#include "arm_regformat.h"
#include "armdefs.h"
#include "armcpu.h"
#include "skyeye_dyncom.h"
#include "arm_dyncom_run.h"
#include "arm_arch_interface.h"
#include "arm_dyncom_translate.h"
#include "arm_dyncom_mmu.h"
#include "arm_dyncom_parallel.h"
#include "arm_dyncom_run.h"

static void
arm_reset_state ()
{
	int i;
	ARMul_State *state;

	ARM_CPU_State *cpu = get_current_cpu();
	for(i = 0; i < cpu->core_num; i++){
		state = &cpu->core[i];
		ARMul_Reset (state);
		state->NextInstr = PRIMEPIPE;
		state->Emulate = 3;
	}
}

#if 1
void
arm_dyncom_abort(arm_core_t * state, ARMword vector)
{
    uint32_t eebit;
	switch (vector) {
	case ARMul_ResetV:	/* RESET */
		break;
	case ARMul_UndefinedInstrV:	/* Undefined Instruction */
		printf("in %s, undefined instruction\n", __FUNCTION__);
		state->Reg_undef[1] = state->Reg[15] + 4;
		state->Spsr[UNDEFBANK] = state->Cpsr;
		state->Cpsr = state->Cpsr & 0xfffffc40;
		state->Cpsr |= 0x9b;
		eebit = (state->CP15[CP15(CP15_CONTROL)] >> 25) & 1;
		state->Cpsr |= (eebit << 9);

		switch_mode(state, state->Cpsr & 0x1f);
		state->Aborted = 0;
		state->abortSig = 0;
		break;
	case ARMul_SWIV:	/* Software Interrupt */
		state->Reg_svc[1] = state->Reg[15] + 4;
		state->Spsr[SVCBANK] = state->Cpsr;
		state->Cpsr = state->Cpsr & 0xfffffc40;
		state->Cpsr |= 0x93;
		eebit = (state->CP15[CP15(CP15_CONTROL)] >> 25) & 1;
		state->Cpsr |= (eebit << 9);

		switch_mode(state, state->Cpsr & 0x1f);
		break;
	case ARMul_PrefetchAbortV:	/* Prefetch Abort */
		state->Reg_abort[1] = state->Reg[15] + 4;
		state->Spsr[ABORTBANK] = state->Cpsr;
		state->Cpsr = state->Cpsr & 0xfffffc40;
		state->Cpsr |= 0x97;
		eebit = (state->CP15[CP15(CP15_CONTROL)] >> 25) & 1;
		state->Cpsr |= (eebit << 9);

		switch_mode(state, state->Cpsr & 0x1f);
		state->Aborted = 0;
		state->abortSig = 0;
		break;
	case ARMul_DataAbortV:	/* Data Abort */
		state->Reg_abort[1] = state->Reg[15] + 8;
		//printf("### Dyncom Data Abort pc %x Cpsr %x SpsrC %x Spsr %x ", state->Reg[15], state->Cpsr, state->Spsr_copy, state->Spsr[ABORTBANK]);
		state->Spsr[ABORTBANK] = state->Cpsr;
		state->Cpsr = state->Cpsr & 0xfffffc40;
		state->Cpsr |= 0x97;
		eebit = (state->CP15[CP15(CP15_CONTROL)] >> 25) & 1;
		state->Cpsr |= (eebit << 9);

		switch_mode(state, state->Cpsr & 0x1f);
		state->Aborted = 0;
		state->abortSig = 0;
		ASSIGNINT(state->Cpsr & INTBITS);
		//printf("  ---> Cpsr %x SpsrC %x Spsr %x\n", state->Cpsr, state->Spsr_copy, state->Spsr[ABORTBANK]);
		break;
	case ARMul_AddrExceptnV:	/* Address Exception */
		exit(-1);
		break;
	case ARMul_IRQV:	/* IRQ */
		//chy 2003-09-02 the if sentence seems no use
		{
			state->Reg_irq[1] = state->Reg[15] + 4;
			//printf("in %s R15 is %x at %x\n", __FUNCTION__, state->Reg[15], state->NumInstrs);
			state->Spsr[IRQBANK] = state->Cpsr;
			state->Cpsr = state->Cpsr & 0xfffffc40;
			state->Cpsr |= 0x92;
			eebit = (state->CP15[CP15(CP15_CONTROL)] >> 25) & 1;
			state->Cpsr |= (eebit << 9);
			switch_mode(state, state->Cpsr & 0x1f);
			//state->NirqSig = HIGH;
			state->Aborted = 0;
			ASSIGNINT(state->Cpsr & INTBITS);
			//printf("Cpsr is %x Flags %x IRQ Spsr is %x \n", state->Cpsr, state->IFFlags, state->Spsr[IRQBANK]);
		}
	break;
	case ARMul_FIQV:	/* FIQ */
		//chy 2003-09-02 the if sentence seems no use
            printf("FIQV\n");
            exit(-1);

		break;
	}

	if (ARMul_MODE32BIT) {
		if (state->CP15[CP15(CP15_CONTROL)] & CONTROL_VECTOR)
			vector += 0xffff0000;	//for v4 high exception  address
		if (state->vector_remap_flag)
			vector += state->vector_remap_addr; /* support some remap function in LPC processor */
		//ARMul_SetR15 (state, vector);
        state->Reg[15] = vector;
	}
	else
            ;
}
#endif
static void per_cpu_step(conf_object_t * running_core){

        arm_core_t *core = (arm_core_t *)running_core->obj;
        machine_config_t* mach = get_current_mach();
        ARM_CPU_State* cpu = get_current_cpu();
        cpu_t *cpu_dyncom = (cpu_t*)get_cast_conf_obj(core->dyncom_cpu, "cpu_t");
	
	if (running_mode != PURE_DYNCOM)
	{
		uint32_t ret = 0;
		if(running_mode == FAST_INTERPRET){
			extern void InterpreterMainLoop(cpu_t *core);
			InterpreterMainLoop((cpu_t*)(core->dyncom_cpu->obj));
		}
		else{
			ret = launch_compiled_queue((cpu_t*)(core->dyncom_cpu->obj), core->Reg[15]);
		}
		/* in any case, user mode traps and exception are all handled within launc compiled queue */
		if(is_user_mode(cpu_dyncom))
			return;
		/* if next instruction will be interpreted, ret value is 0. Exceptions are handled inside
		   the interpreter, so we return immediatly */
		else if ((ret == 0) || (running_mode == PURE_INTERPRET))
			return;
	} 
	else 
	{	
		uint32_t ret = 0;
		ret = launch_compiled_queue_dyncom((cpu_t*)(core->dyncom_cpu->obj), core->Reg[15]);
		//arm_dyncom_run((cpu_t*)get_cast_conf_obj(core->dyncom_cpu, "cpu_t"));
	}

	/* the next part concerns only dyncom */
	if (core->syscallSig) {
		printf("In %s, syscallSig %x\n", __FUNCTION__, core->Reg[15]);
		core->syscallSig = 0;
		arm_dyncom_abort(core, ARMul_SWIV);
	}
	#if SYNC_WITH_INTERPRET
	if (cpu_dyncom->icounter > 1951000 && is_int_in_interpret(cpu_dyncom))
	{
		while(core->NirqSig) {
			mach->mach_io_do_cycle(cpu);
		}
	}
	#endif
	if (core->abortSig) {
		printf("In %s, abortSig occured at pc %x with signal %x\n", __FUNCTION__, core->Reg[15], core->Aborted);
		arm_dyncom_abort(core, core->Aborted);
	}
	if (!core->NirqSig) {
		//printf("Trying abort \n");
		if (!(core->Cpsr & 0x80)) {
			//printf("In %s, irqSig %x\n", __FUNCTION__, core->Reg[15]);
			//printf("Cpsr not NirqSig %x\n", core->Reg[15]);
	#if SYNC_HYBRID
			if (!is_int_in_interpret(cpu_dyncom))
				return;
	#endif
	
	#if SYNC_WITH_INTERPRET
			if (cpu_dyncom->icounter > 1951000 && !is_int_in_interpret(cpu_dyncom)) {
				return;
			}
	#endif
			arm_dyncom_abort(core, ARMul_IRQV);
		}
	}
	mach->mach_io_do_cycle(cpu);
}

static void per_cpu_stop(conf_object_t * core){
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
		exec->priv_data = get_conf_obj_by_cast(core, "arm_core_t");
        //exec->priv_data = get_conf_obj_by_cast(core, "ARMul_State");
		//exec->priv_data = (conf_object_t*)core;
		//exec->run =  (void (*)(conf_object_t*))per_cpu_step;
        exec->run =  per_cpu_step;
		//exec->stop = (void (*)(conf_object_t*))per_cpu_stop;
        exec->stop = per_cpu_stop;
		add_to_default_cell(exec);
	}

	cpu->boot_core_id = 0;

	ARMul_EmulateInit(); /* Needed by the interpreter */
	
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

static uint32 arm_get_regnum(){
	return MAX_REG_NUM;
}

static exception_t arm_set_register_by_id(int id, uint32 value){
	ARM_CPU_State* cpu = get_current_cpu();
	cpu->core[0].Reg[id] = value;
        return No_exp;
}

static exception_t arm_signal(interrupt_signal_t *signal){
	ARMul_State *state = get_current_core();
	arm_signal_t *arm_signal = &signal->arm_signal;
	if (arm_signal->irq != Prev_level) {
		state->NirqSig = arm_signal->irq;
    }
	if (arm_signal->firq != Prev_level)
		state->NfiqSig = arm_signal->firq;

	/* reset signal in arm dyf add when move sa1100 to soc dir  2010.9.21*/
	if (arm_signal->reset != Prev_level)
		state->NresetSig = arm_signal->reset;
	return No_exp;
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
	arm_arch->set_regval_by_id = arm_set_register_by_id;
	arm_arch->get_regnum = arm_get_regnum;
    arm_arch->signal = arm_signal;

	register_arch (arm_arch);
	return No_exp;
}
#ifdef __cplusplus
	}
#endif
