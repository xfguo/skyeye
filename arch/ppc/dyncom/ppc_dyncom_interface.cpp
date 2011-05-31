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
#include "ppc_dec.h"
#include "ppc_e500_exc.h"
#include "ppc_e500_core.h"
#include "ppc_memory.h"
#include "ppc_io.h"
//#include "types.h"
#include "tracers.h"
#include "sysendian.h"
#include "ppc_irq.h"
#include "ppc_regformat.h"
#include "bank_defs.h"
#include "ppc_dyncom_run.h"
#include "ppc_dyncom_dec.h"

#include "skyeye_types.h"
#include "skyeye_config.h"
#include "skyeye_arch.h"
#include "skyeye_mm.h"
#include <skyeye_log.h>
#include <skyeye_exec.h>
#include <skyeye_cell.h>
#include <skyeye_pref.h>
#include "ppc_dyncom_debug.h"
#include "ppc_dyncom_parallel.h"

#ifdef __CYGWIN__
#include <sys/time.h>
#endif
extern "C"{
#include "ppc_exc.h"
};
#include <pthread.h>
#include <unistd.h>
#include "dyncom/defines.h"

static void per_cpu_step(conf_object_t * core);
static void per_cpu_stop(conf_object_t * core);

static void
ppc_reset_state ()
{
}

static void set_exception_stage2(e500_core_t *core, const uint32 type){
	switch(type){
		case PPC_EXC_DEC:
			core->asyn_exc_flag |= DYNCOM_ASYN_EXC_DEC;
			break;
		case PPC_EXC_EXT_INT:
			core->asyn_exc_flag |= DYNCOM_ASYN_EXC_EXT_INT;
			break;
		default:
			fprintf(stderr, "Core %d, EXC %x not support.\n", core->pir, type);
	}
}
void e600_dyncom_dec_io_do_cycles(e500_core_t * core){
	cpu_t* cpu = (cpu_t*)get_cast_conf_obj(core->dyncom_cpu, "cpu_t");
	uint32_t cycles = cpu->icounter - cpu->old_icounter;
	uint32_t old_tbl = core->tbl;
	uint32_t old_dec = core->dec;
	core->tbl += cycles;
	/* if tbl overflow, we increase tbh */
	if(core->tbl < old_tbl)
		core->tbu++;
	/* if decrementer eqauls zero */
	if(cycles >= old_dec){
		if(core->msr & MSR_EE){
			/* trigger timer interrupt */
			ppc_exception(core, PPC_EXC_DEC, 0, core->pc);
			core->dec = 0;
		}
	}else{
		core->dec -= cycles;
	}
	return;
}
static bool_t e600_dyncom_ppc_exception(e500_core_t *core, uint32 type, uint32 flags, uint32 a)
{
	core->pc -= 4;
	core->phys_pc -= 4;
	switch (type) {
	case PPC_EXC_DSI: {	// .271
		core->srr[0] = core->pc;
		core->srr[1] = core->msr & 0x87c0ffff;
		core->dar = a;
		core->dsisr = flags;
		//printf("In %s, addr=0x%x, pc=0x%x, icount = %d DSI exception.\n", __FUNCTION__, a, core->pc, core->icount);
		break;
	}
	case PPC_EXC_ISI: { // .274
		core->srr[0] = core->pc + 4;
		core->srr[1] = (core->msr & 0x87c0ffff) | flags;
		//printf("In %s, addr=0x%x, pc=0x%x, icount = %d ISI exception.\n", __FUNCTION__, a, core->pc, core->icount);
		break;
	}
	case PPC_EXC_EXT_INT: {
		set_exception_stage2(core, PPC_EXC_EXT_INT);
		core->pc += 4;
		core->phys_pc += 4;
		return True;
	}
	case PPC_EXC_SC: {	// .285
		core->srr[0] = core->pc + 4;
		core->srr[1] = core->msr & 0x87c0ffff;
		break;
	}
	case PPC_EXC_NO_FPU: { // .284
		core->srr[0] = core->pc;
		core->srr[1] = core->msr & 0x87c0ffff;
		break;
	}
	case PPC_EXC_NO_VEC: {	// v.41
		core->srr[0] = core->pc;
		core->srr[1] = core->msr & 0x87c0ffff;
		break;
	}
	case PPC_EXC_PROGRAM: { // .283
		core->srr[0] = core->pc;
		core->srr[1] = (core->msr & 0x87c0ffff) | flags;
		break;
	}
	case PPC_EXC_FLOAT_ASSIST: { // .288
		core->srr[0] = core->pc;
		core->srr[1] = core->msr & 0x87c0ffff;
		break;
	}
	case PPC_EXC_MACHINE_CHECK: { // .270
		if (!(core->msr & MSR_ME)) {
			PPC_EXC_ERR("machine check exception and MSR[ME]=0.\n");
		}
		core->srr[0] = core->pc;
		core->srr[1] = (core->msr & 0x87c0ffff) | MSR_RI;
		break;
	}
	case PPC_EXC_TRACE2: { // .286
		core->srr[0] = core->pc;
		core->srr[1] = core->msr & 0x87c0ffff;
		break;
	}
	case PPC_EXC_DEC: { // .284
		set_exception_stage2(core, PPC_EXC_DEC);
		core->pc += 4;
		core->phys_pc += 4;
		return True;
	}
	default:
		PPC_EXC_ERR("unknown\n");
		core->pc += 4;
		core->phys_pc += 4;
		return False;
	}
	ppc_mmu_tlb_invalidate(core);
	core->msr = 0;
	core->pc = type;
	core->phys_pc = type;
	return True;
}
static int detect_exception(cpu_t *cpu, e500_core_t *core)
{
	if(core->asyn_exc_flag & DYNCOM_ASYN_EXC_DEC){
		if(core->msr & MSR_EE){
			core->srr[0] = core->pc;
			core->srr[1] = core->msr & 0x87c0ffff;

			ppc_mmu_tlb_invalidate(core);
			core->msr = 0;
			core->pc = PPC_EXC_DEC;
			core->phys_pc = PPC_EXC_DEC;
			core->asyn_exc_flag &= ~DYNCOM_ASYN_EXC_DEC;
//			skyeye_log(Debug_log, __func__, "core %d PPC_EXC_DEC happened. icount = %d\n", core->pir, core->icount);
		}else{
			return 0;
		}
	}
	if(core->asyn_exc_flag & DYNCOM_ASYN_EXC_EXT_INT){
		if(core->msr & MSR_EE){
			core->srr[0] = core->pc;
			core->srr[1] = core->msr & 0x87c0ffff;

			ppc_mmu_tlb_invalidate(core);
			core->msr = 0;
			core->pc = PPC_EXC_EXT_INT;
			core->phys_pc = PPC_EXC_EXT_INT;
			core->asyn_exc_flag &= ~DYNCOM_ASYN_EXC_EXT_INT;
//			skyeye_log(Debug_log, __func__, "core %d PPC_EXC_EXT_INT happened.icount = %d\n", core->pir, core->icount);
		}else{
			return 0;
		}
	}
	return 0;
}
/* init thread clock for profile */
static void ppc_dyncom_thread_clock_init()
{
	extern void *clock_thread(void*);
	pthread_t thread;
	int ret = pthread_create(&thread, NULL, clock_thread, NULL);
	if(ret){
		fprintf(stderr, "failed create timing thread\n");
		exit(0);
	}
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
	mach->cpu_data = get_conf_obj_by_cast(cpu, "PPC_CPU_State");
	if(!strcmp(mach->machine_name, "mpc8560")){
		cpu->core_num = 1;
	}
	else if(!strcmp(mach->machine_name, "mpc8572"))
		cpu->core_num = 2;
	else if(!strcmp(mach->machine_name, "mpc8641d"))
                cpu->core_num = 2;
        else
		cpu->core_num = 0;
	/* We only use one core for user mode running */
	if(get_user_mode() == True)
		cpu->core_num = 1;
	if(!cpu->core_num){
		skyeye_log(Critical_log, __FUNCTION__, "Can not get the core number or set wrong mach name?\n");
		skyeye_exit(-1);
	}
	else
		cpu->core = (e500_core_t*)skyeye_mm_zero(sizeof(e500_core_t) * cpu->core_num);
	/* TODO: zero the memory by malloc */

	if(!cpu->core){
		skyeye_log(Critical_log, __FUNCTION__, "Can not allocate memory for ppc core.\n");
		skyeye_exit(-1);
	}
	else
		skyeye_log(Info_log, __FUNCTION__, "Initilization for %d core\n", cpu->core_num);
		
	int i;
	for(i = 0; i < cpu->core_num; i++){
		e500_core_t* core = &cpu->core[i];
		ppc_core_init(core, i);
		/* set exception funcion for dyncom */
		core->ppc_exception = e600_dyncom_ppc_exception;
		/* set dec_io_do_cycle funcion for dyncom */
		core->dec_io_do_cycle = e600_dyncom_dec_io_do_cycles;

		ppc_dyncom_init(core);

		skyeye_exec_t* exec = create_exec();
		exec->priv_data = get_conf_obj_by_cast(core, "e500_core_t");
		exec->run = per_cpu_step;
		exec->stop = per_cpu_stop;
		add_to_default_cell(exec);
	}

	cpu->boot_core_id = 0;
#if THREAD_CLOCK
	if(get_user_mode())
		ppc_dyncom_thread_clock_init();
#endif
	/* initialize decoder */
	ppc_dyncom_dec_init();
	/* initialize decoder */
	ppc_dec_init(&cpu->core[cpu->boot_core_id]);
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

static void per_cpu_step(conf_object_t * running_core){
	/* Use typecast directly for performance issue */
	e500_core_t *core = (e500_core_t *)running_core->obj;
	PPC_CPU_State* cpu = get_current_cpu();
	/* Check the second core and boot flags */
	if(core->pir){
		if(!(cpu->eebpcr & 0x2000000))
			return;
	}
	debug(DEBUG_INTERFACE, "In %s, core[%d].pc=0x%x\n", __FUNCTION__, core->pir, core->pc);
	ppc_dyncom_run((cpu_t*)get_cast_conf_obj(core->dyncom_cpu, "cpu_t"));
//	launch_compiled_queue((cpu_t*)(core->dyncom_cpu->obj), core->pc);	

	if(!is_user_mode((cpu_t*)(core->dyncom_cpu->obj))){
		core->dec_io_do_cycle(core);
		detect_exception((cpu_t*)(core->dyncom_cpu->obj), core);
	}
}

static void per_cpu_stop(conf_object_t * core){
}

static void ppc_step_once ()
{
	/* FIXME: not used in dyncom */
	return;
}

static void ppc_stop(uint32_t id){
	return;
}

static void ppc_set_pc (generic_address_t pc)
{
	int i;
	PPC_CPU_State* cpu = get_current_cpu();
	cpu->core[0].pc = pc;
	/* Fixme, for e500 core, the first instruction should be executed at 0xFFFFFFFC */
	//gCPU.pc = 0xFFFFFFFC;
}
static uint32_t ppc_get_pc()
{
	PPC_CPU_State* cpu = get_current_cpu();
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
	PPC_CPU_State* cpu = get_current_cpu();
	return cpu->core[0].step;
}

extern char* ppc_regstr[];
static char* ppc_get_regname_by_id(int id){
        return ppc_regstr[id];
}
static uint32 ppc_get_regval_by_id(int id){
	/* we return the reg value of core 0 by default */
	int core_id = 0;
	PPC_CPU_State* cpu = get_current_cpu();
	if(id >= 0 && id < 32)
        	return cpu->core[core_id].gpr[id];

	switch(id){
		case PC_REG:
			return cpu->core[core_id].pc;
		case MSR:
			return cpu->core[core_id].msr;
		case CR_REG:
			return cpu->core[core_id].cr;
		case LR_REG:
			return cpu->core[core_id].lr;
		case CTR_REG:
			return cpu->core[core_id].ctr;
		case XER_REG:
			return cpu->core[core_id].xer;
		case FPSCR:
			return cpu->core[core_id].fpscr;
		default:
			/* can not find any corrsponding register */
			return 0;
	}
}
static uint32 ppc_get_regnum(){
       return PPC_MAX_REGNUM;
}

static exception_t ppc_mmu_read(short size, generic_address_t addr, uint32_t * value){
	uint32 result;

	/**
         *  work around for ppc gdb remote debugger
         */
        if ((addr & 0xFFFFF000) == 0xBFFFF000)
                return No_exp;

	switch(size){
                case 8:
                        ppc_read_effective_byte (addr, (uint8_t *)&result);
			*(uint8_t *)value = (uint8_t)result;
                        break;
                case 16:
			ppc_read_effective_half(addr, (uint16_t *)&result);
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
	ppc_arch->get_regnum = ppc_get_regnum;
	ppc_arch->mmu_read = ppc_mmu_read;
	ppc_arch->mmu_write = ppc_mmu_write;

	register_arch (ppc_arch);
	return No_exp;
}
