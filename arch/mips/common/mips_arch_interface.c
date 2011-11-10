/* Copyright (C)
* 2011 - SkyEye Team
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

/**
* @file mips_arch_interface.c
* @brief initial and register mips architecture interface.
* @author SkyEye Team
* @version
* @date 2011-05-05
*/

#include "skyeye_config.h"
#include "skyeye_exec.h"
#include "skyeye_cell.h"
#include "skyeye_arch.h"
#include "skyeye_options.h"
#include "skyeye_pref.h"
#include "skyeye_mm.h"
#include "skyeye_callback.h"
#include "bank_defs.h"

#include "emul.h"
#include <stdlib.h>
#include "mipsdef.h"
#include <stdio.h>
#include <stdbool.h>
#include "mips_regformat.h"
#include "mips_cpu.h"
//MIPS_State* mstate;
static char *arch_name = "mips";
mips_mem_config_t mips_mem_config;
extern FILE *skyeye_logfd;

/**
* @brief  Trigger a irq and set
*
* @param mstate
*/
void 
mips_trigger_irq(MIPS_State* mstate)
{
	VA epc;

	//Get the content of the cause register
	UInt32 cause = mstate->cp0[Cause];

	//When the instruction is in the delay slot, we have to delay an instruction
	if (!branch_delay_slot(mstate))
		epc = mstate->pc;
	else {
		epc = mstate->pc - 4;
		cause = set_bit(cause, Cause_BD);
	}
	mstate->cp0[Cause] = cause;
	mstate->cp0[EPC] = epc;

	//Change the pointer pc to deal with the interrupt handler
	if(bit(mstate->cp0[SR], SR_BEV) )
	{
		mstate->pc = 0xbfc00380;
	} else {
		mstate->pc = 0x80000180;
	}

	mstate->pipeline = nothing_special;
}


/**
* @brief mips read mem interface
*
* @param pa	physic addr
* @param data  data pointer
* @param len	data length
*/
void
mips_mem_read(UInt32 pa, UInt32 *data, int len)
{
	sky_pref_t* pref = get_skyeye_pref();
	if(!pref->user_mode_sim){
		/* if pa is located at kseg0 */
		if(pa >= 0x80000000 && pa < 0xA0000000)
			pa = pa & ~0x80000000;
		/* if pa is located at kseg1 */
		if(pa >= 0xA0000000 && pa < 0xC0000000)
			pa = pa & ~0xE0000000;
	}
	//if(pa >= 0x14a1a0 && pa <= 0x14c000)
	//	printf("###############read addr pa=0x%x,pc=0x%x\n", pa, mstate->pc);
	bus_read(len * 8, pa, data);
}

/**
* @brief mips write memory interface
*
* @param pa	physic addr
* @param data	data pointer
* @param len	data length
*/
void 
mips_mem_write(UInt32 pa, const UInt32* data, int len)
{
	sky_pref_t* pref = get_skyeye_pref();
	if(!pref->user_mode_sim){
		/* if pa is located at kseg0 */
		if(pa >= 0x80000000 && pa < 0xA0000000)
			pa = pa & ~0x80000000;
		/* if pa is located at kseg1 */
		if(pa >= 0xA0000000 && pa < 0xC0000000)
			pa = pa & ~0xE0000000;
	}

	UInt32 addr = bits(pa, 31, 0);
	bus_write(len * 8, pa, *data);
	return;
}

/**
* @brief cpu stop interface.
*
* @param running_core	core state
*/
static void
per_cpu_stop(conf_object_t *running_core)
{
	mips_core_t* core = (mips_core_t *)get_cast_conf_obj(running_core, "mips_core_t");
}

/**
* @brief cpu exec one step
*
* @param running_core core state
*/
static void
per_cpu_step(conf_object_t *running_core)
{
	mips_core_t* mstate = (mips_core_t *)get_cast_conf_obj(running_core, "mips_core_t");
	MIPS_CPU_State* cpu = get_current_cpu();
	mstate->gpr[0] = 0;

	/* if active is 0, the core is suspend */
	if(!mstate->active)
		return;
	/* Check for interrupts. In real hardware, these have a priority lower
	 * than all exceptions, but simulating this effect is too hard to be
	 * worth the effort (interrupts and resets are not meant to be
	 * delivered accurately anyway.)
         */
	if(mstate->irq_pending)
	{
		mips_trigger_irq(mstate);
	}

	/* Look up the ITLB. It's not clear from the manuals whether the ITLB
	 * stores the ASIDs or not. I assume it does. ITLB has the same size
	 * as in the real hardware, mapping two 4KB pages.  Because decoding a
	 * MIPS64 virtual address is far from trivial, ITLB and DTLB actually
	 * improve the simulator's performance: something I cannot say about
	 * caches and JTLB.
	*/

	PA pa; //Shi yang 2006-08-18
	VA va;
	Instr instr;
	int next_state;
	va = mstate->pc;
	mstate->cycle++;

	generic_arch_t *arch_instance = get_arch_instance("");
	exec_callback(Step_callback, arch_instance);

	if(translate_vaddr(mstate, va, instr_fetch, &pa) == TLB_SUCC){
		mips_mem_read(pa, &instr, 4);
		next_state = decode(mstate, instr);
		//skyeye_exit(-1);
	}
	else{
		//fprintf(stderr, "Exception when get instruction!\n");
	}

	/* NOTE: mstate->pipeline is also possibely set in decode function */
	if(skyeye_logfd)
		//fprintf(skyeye_logfd, "KSDBG:instr=0x%x,pa=0x%x, va=0x%x, sp=0x%x, ra=0x%x,s1=0x%x, v0=0x%x\n", instr, pa, va, mstate->gpr[29], mstate->gpr[31],mstate->gpr[17], mstate->gpr[2]);
		fprintf(skyeye_logfd, "KSDBG:instr=0x%x,pa=0x%x, va=0x%x, a0=0x%x, k1=0x%x, t0=0x%x, ra=0x%x, s4=0x%x, gp=0x%x\n", instr, pa, va, mstate->gpr[4], mstate->gpr[27], mstate->gpr[8], mstate->gpr[31], mstate->gpr[20], mstate->gpr[28]);
		//fprintf(skyeye_logfd, "KSDBG:instr=0x%x,pa=0x%x, va=0x%x,v0=0x%x,t0=0x%x\n", instr, pa, va, mstate->gpr[2], mstate->gpr[8]);

	switch (mstate->pipeline) {
		case nothing_special:
			mstate->pc += 4;
			break;
		case branch_delay:
			mstate->pc = mstate->branch_target;
			break;
		case instr_addr_error:
			process_address_error(mstate, instr_fetch, mstate->branch_target);
		case branch_nodelay: /* For syscall and TLB exp, we donot like to add pc */
			mstate->pipeline = nothing_special;
			return; /* do nothing */
	}
	mstate->pipeline = next_state;
	/* if timer int is not mask and counter value is equal to compare value */
	if(mstate->cp0[Count]++ >= mstate->cp0[Compare]){
			/* update counter value in cp0 */
			mstate->cp0[Count] = 0;

		/* if interrupt is enabled? */
		if((mstate->cp0[SR] & (1 << SR_IEC)) && (mstate-> cp0[SR] & 1 << SR_IM7)){
			if(!(mstate->cp0[Cause] & 1 << Cause_IP7) && (!(mstate->cp0[SR] & 0x2)))
			{
				//fprintf(stderr, "counter=0x%x,pc=0x%x\n", mstate->cp0[Count], mstate->pc);
				/* Set counter interrupt bit in IP section of Cause register */
				mstate->cp0[Cause] |= 1 << Cause_IP7;
				mstate->cp0[Cause] |= 1 << Cause_TI;	/* for release 2 */
			/* Set ExcCode to zero in Cause register */
				process_exception(mstate, EXC_Int, common_vector);
			}
		}
	}

	if(mstate->mt_flag){
		int TcBindVPE = mstate->cp0_TCBind & 0xf;			/* get current VPE */
		mips_core_t* curVPE = &cpu->core[TcBindVPE + mstate->tc_num];
		/* if has a interrupt */
		if((curVPE->cp0[SR] & 0x1 ) && curVPE->int_flag){
			curVPE->int_flag = 0;
			//mstate->int_flag = 0;
			process_exception(mstate, EXC_Int, common_vector);
		}
	}

	//skyeye_config.mach->mach_io_do_cycle (mstate);
	//exec_callback();
}


/**
* @brief inital icache
*
* @param mstate core state
*/
static void 
init_icache(mips_core_t* mstate)
{
	int i;
	for(i = 0; i < Icache_log2_sets; i++)
	{  
		Icache_lru_init(mstate->icache.set[i].Icache_lru);
	}
}

/**
* @brief inital dcache
*
* @param mstate core state
*/
static void 
init_dcache(mips_core_t* mstate)
{
	int i;
	for(i = 0; i < Dcache_log2_sets; i++)
	{  
	      Dcache_lru_init(mstate->dcache.set[i].Dcache_lru);
	}
}

/**
* @brief inital tlb
*
* @param mstate	core state
*/
static void 
init_tlb(mips_core_t* mstate)
{
	int i; 
	for(i = 0;i < tlb_map_size + 1; i++)
	{
		mstate->tlb_map[i] = NULL;
	}
}

/**
* @brief  initial a cpu core
*
* @param mstate core state
* @param core_id core id
*/
static void 
mips_core_init(mips_core_t* mstate,int core_id)
{
	set_bit(mstate->mode, 2);
/*	mstate = (MIPS_State* )malloc(sizeof(MIPS_State)); */
	if (!mstate) {
		fprintf (stderr, "malloc error!\n");
		skyeye_exit (-1);
	}

	mstate->warm = 0;
	mstate->conf.ec = 4; //I don't know what should it be.

	// set the little endian as the default
	mstate->bigendSig = 0; //Shi yang 2006-08-18
	
	//No interrupt
	mstate->irq_pending = 0;

	mstate->cp0[SR] = 0x40004;
	
	init_icache(mstate);
	init_dcache(mstate);
	init_tlb(mstate);

	return true;
}

/**
* @brief cpu state initalization
*/
static void
mips_cpu_init()
{
	MIPS_CPU_State* cpu = skyeye_mm_zero(sizeof(MIPS_CPU_State));
	machine_config_t* mach = get_current_mach();
	mach->cpu_data = get_conf_obj_by_cast(cpu, "MIPS_CPU_State");
	if(!mach->cpu_data)
		return false;

	cpu->core_num = 6;	/* for mt tmep */
	if(!cpu->core_num){
		fprintf(stderr, "ERROR:you need to set numbers of core in mach_init.\n");
		skyeye_exit(-1);
	}
	else
		cpu->core = skyeye_mm(sizeof(mips_core_t) * cpu->core_num);

	if(!cpu->core){
		fprintf(stderr, "Can not allocate memory for mips core.\n");
		skyeye_exit(-1);
	}
	else
		printf("%d core is initialized.\n", cpu->core_num);

	int i;
	cpu->boot_core_id = 0;
	for(i = 0; i < cpu->core_num; i++){
		mips_core_t* core = &cpu->core[i];
		mips_core_init(core, i);
		core->active = 0;	/* set cpu suspend */
		skyeye_exec_t* exec = create_exec();
		exec->priv_data = get_conf_obj_by_cast(core, "mips_core_t");
		exec->run = per_cpu_step;
		exec->stop = per_cpu_stop;
		add_to_default_cell(exec);
	}
	cpu->core[cpu->boot_core_id].active = 1; /* set boot cpu working */

	return true;
}

/**
* @brief inital mips architecture
*/
static void
mips_init_state()
{
	mips_cpu_init();
	sky_pref_t* pref = get_skyeye_pref();
	if(!pref->user_mode_sim){
		mips_core_t* mstate = get_current_core();
		mstate->gpr[30] = 0x10000000;
	}
}

/**
* @brief reset mips architecture
*/
static void 
mips_reset_state()
{
	mips_core_t* mstate = get_current_core();

    	if (!mstate->warm) {
		memset(mstate->cp1, 0, sizeof(mstate->cp1[32]));
		memset(mstate->fpr, 0, sizeof(mstate->fpr[32]));
		mstate->count_seed = mstate->now;
		mstate->nop_count = 0;
    	}
    	mstate->ll_bit = 0;
    	mstate->sync_bit = 0;

    	// Deliver the reset exception.
    	if (mstate->warm)
		deliver_soft_reset(mstate);
    	else
		deliver_cold_reset(mstate);

    	process_reset(mstate);
}

/**
* @brief every core in current cpu step once
*/
static void 
mips_step_once()
{
	int i;
	MIPS_CPU_State* cpu = get_current_cpu();
	for( i = 0; i < cpu->core_num; i ++ ){
		per_cpu_step(&cpu->core[i]);
	}
	
}

/**
* @brief Set core pc value
*
* @param addr Address set to pc
*/
static void 
mips_set_pc(UInt32 addr)
{
	MIPS_CPU_State* cpu = get_current_cpu();
	int i;
#if 0
	for( i = 0; i < cpu->core_num; i ++ )
		cpu->core[i].pc = addr;
#endif
	cpu->core[0].pc = addr;
}

/**
* @brief Get current pc value
*
* @return The pc.
*/
static UInt32 
mips_get_pc()
{
	MIPS_CPU_State* cpu = get_current_cpu();
	return  cpu->core[0].pc;
}

static void
mips_write_byte (generic_address_t addr, uint8_t v)
{
	/* mips_mem_write_byte (addr, v); */
}

static void 
mips_write_byte64(UInt64 addr, UInt8 data)
{

}

static unsigned char 
mips_read_byte64(UInt64 addr)
{

}

extern void nedved_mach_init(void * state, machine_config_t * mach);
extern void au1100_mach_init(void * state, machine_config_t * mach);
extern void fulong_mach_init(void * state, machine_config_t * mach);
extern void gs32eb1_mach_init(void * state, machine_config_t * mach);
extern void malta_mach_init(void * state, machine_config_t * mach);

/* machines register arrary */
machine_config_t mips_machines[] = {
	{"nedved", nedved_mach_init, NULL, NULL, NULL},
	{"au1100", au1100_mach_init, NULL, NULL, NULL},
	{"fulong", fulong_mach_init, NULL, NULL, NULL},
	{"gs32eb1", gs32eb1_mach_init, NULL, NULL, NULL},
	{"malta", malta_mach_init, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL},
};

static int 
mips_parse_cpu(const char* param[])
{
	return 1;
}

/**
* @brief mips read byte
*
* @param addr read address
* @param data	data pointer
*
* @return
*/
static int mips_ICE_read_byte(generic_address_t addr, uint8_t *data){
	mips_mem_read(addr, (UInt32 *)data, 1);
	return 0;
}

/**
* @brief mips write byte
*
* @param addr write address
* @param data write value
*
* @return
*/
static int mips_ICE_write_byte(generic_address_t addr, uint8_t data){
      	mips_mem_write(addr, &data, 1);  
	return 0;
}

/**
* @brief get current step counter
*
* @return  step counter
*/
static uint32 mips_get_step(){
	MIPS_CPU_State* cpu = get_current_cpu();
	uint32 step = cpu->core[0].cycle;
        return step;
}

/**
* @brief get register name
*
* @param id register id
*
* @return  register value
*/
static char* mips_get_regname_by_id(int id){
        return mips_regstr[id];
}

/**
* @brief Set a register value
*
* @param id register id
*
* @return register value
*/
static uint32 mips_get_regval_by_id(int id){
	MIPS_CPU_State* cpu = get_current_cpu();

	if(id == PC)
		return cpu->core[0].pc;
	return cpu->core[0].gpr[id];
}

/**
* @brief Set a register value
*
* @param id register id
* @param value	register value
*
* @return func state
*/
static exception_t mips_set_register_by_id(int id, uint32 value){
	MIPS_CPU_State* cpu = get_current_cpu();
	cpu->core[0].gpr[id] = value;

        return No_exp;
}

/**
* @brief get mips arch register number
*
* @return register number
*/
static int mips_get_regnum()
{
	return 32;
}

/**
* @brief register mips architecture.
*/
void 
init_mips_arch ()
{
	static arch_config_t mips_arch;
	mips_arch.arch_name = arch_name;
	mips_arch.init = mips_init_state;
	mips_arch.reset = mips_reset_state;
	mips_arch.step_once = mips_step_once;
	mips_arch.set_pc = mips_set_pc;
	mips_arch.get_pc = mips_get_pc;
	mips_arch.ICE_read_byte = mips_ICE_read_byte;
	mips_arch.ICE_write_byte = mips_ICE_write_byte;
	mips_arch.parse_cpu = mips_parse_cpu;
	mips_arch.get_step = mips_get_step;
	//mips_arch.parse_mach = mips_parse_mach;
	//mips_arch.parse_mem = mips_parse_mem;
	mips_arch.get_regval_by_id = mips_get_regval_by_id;
        mips_arch.get_regname_by_id = mips_get_regname_by_id;
	mips_arch.get_regnum  = mips_get_regnum;
	register_arch (&mips_arch);
}
