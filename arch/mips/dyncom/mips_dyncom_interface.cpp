/* Simulator for MIPS R3000 architecture.

		THIS SOFTWARE IS NOT COPYRIGHTED

   Cygnus offers the following for use in the public domain.  Cygnus
   makes no warranty with regard to the software or it's performance
   and the user accepts the software "AS IS" with all faults.

   CYGNUS DISCLAIMS ANY WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD TO
   THIS SOFTWARE INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

*/

#include <skyeye_dyncom.h>
#include <skyeye_obj.h>
#include "skyeye_config.h"
#include "skyeye_options.h"
extern "C" {
#include "../common/emul.h"
}
#include <stdlib.h>
#include "mipsdef.h"
#include <stdio.h>
#include "mips_regformat.h"
#include "bank_defs.h"
#include "../common/emul.h"
#include "mips_dyncom_run.h"
#include "mipscpu.h"
#include "skyeye_mm.h"
#include "skyeye_cell.h"
#include "skyeye_arch.h"

MIPS_State* mstate;
mips_mem_config_t mips_mem_config;
extern FILE *skyeye_logfd;
extern int trace_level;

static void
init_icache()
{
	int i;
	for(i = 0; i < Icache_log2_sets; i++)
	{
		Icache_lru_init(mstate->icache.set[i].Icache_lru);
	}

}

static void
init_dcache()
{
	int i;
	for(i = 0; i < Dcache_log2_sets; i++)
	{
		Dcache_lru_init(mstate->dcache.set[i].Dcache_lru);
	}
}

static void
init_tlb()
{
	int i;
	for(i = 0;i < tlb_map_size + 1; i++)
	{
		mstate->tlb_map[i] = NULL;
	}
}

static void
mips_core_init(MIPS_State *state, int i)
{
	set_bit(mstate->mode, 2);
	mstate = (MIPS_State* )malloc(sizeof(MIPS_State));
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

#if 0
	init_icache();
	init_dcache();
	init_tlb();
#endif
}

static void per_cpu_step(mips_core_t * core){
	mips_dyncom_run((cpu_t*)get_cast_conf_obj(core->dyncom_cpu, "cpu_t"));
}

static void per_cpu_stop(mips_core_t * core){
}

static void
mips_cpu_init()
{
	MIPS_CPU_State *cpu = (MIPS_CPU_State *)skyeye_mm(sizeof(MIPS_CPU_State));
	machine_config_t *mach = get_current_mach();
	mach->cpu_data = get_conf_obj_by_cast(cpu, "MIPS_CPU_State");

	cpu->core_num = 1;
	if(!cpu->core_num){
		fprintf(stderr, "ERROR:you need to set numbers of core in mach_init.\n");
		skyeye_exit(-1);
	}
	else
		cpu->core = (MIPS_State *)skyeye_mm(sizeof(MIPS_State) * cpu->core_num);
	/* TODO: zero the memory by malloc */

	if(!cpu->core){
		fprintf(stderr, "Can not allocate memory for arm core.\n");
		skyeye_exit(-1);
	}
	else
		printf("%d core is initialized.\n", cpu->core_num);

	int i;
	for(i = 0; i < cpu->core_num; i++){
		MIPS_State* core = &cpu->core[i];
		mips_core_init(core, i);
		mips_dyncom_init(core);
		skyeye_exec_t* exec = create_exec();
		exec->priv_data = get_conf_obj_by_cast(core, "MIPS_State");
		exec->run = (void (*)(conf_object_t*))per_cpu_step;
		exec->stop = (void (*)(conf_object_t*))per_cpu_stop;
		add_to_default_cell(exec);
	}

	cpu->boot_core_id = 0;
}

static void
mips_init_state()
{
	mips_cpu_init();
}

static void
mips_reset_state()
{
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

static void
mips_step_once()
{
	int i;
	machine_config_t* mach = get_current_mach();
	MIPS_CPU_State* cpu = get_current_cpu();
	/* workaround boot sequence for dual core, we need the first core initialize some variable for second core. */
	mips_core_t* core;
	for(i = 0; i < cpu->core_num; i++ ){
		core = &cpu->core[i];
		/* if CPU1_EN is set? */
			per_cpu_step(core);
	}

	/* for peripheral */
//	mach->mach_io_do_cycle(cpu);

#if 0
	mstate->gpr[0] = 0;

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
	if(translate_vaddr(mstate, va, instr_fetch, &pa) == TLB_SUCC){
		mips_mem_read(pa, &instr, 4);
		next_state = decode(mstate, instr);
		//skyeye_exit(-1);
	}
	else{
		//fprintf(stderr, "Exception when get instruction!\n");
	}

	/* NOTE: mstate->pipeline is also possibely set in decode function */
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
			/* Set ExcCode to zero in Cause register */
				process_exception(mstate, EXC_Int, common_vector);
			}
		}
	}
#endif
}

static void
mips_set_pc(generic_address_t pc)
{
	//mstate->pc = addr;
	MIPS_CPU_State* cpu = get_current_cpu();
	cpu->core[0].pc = pc;
}

static UInt32
mips_get_pc()
{
	//return  mstate->pc;
	MIPS_CPU_State* cpu = get_current_cpu();
	return cpu->core[0].pc;
}

static void
mips_write_byte (generic_address_t addr, uint8_t v)
{
}

extern void nedved_mach_init(void * state, machine_config_t * mach);
extern void au1100_mach_init(void * state, machine_config_t * mach);
extern void fulong_mach_init(void * state, machine_config_t * mach);
extern void gs32eb1_mach_init(void * state, machine_config_t * mach);

#if 0
machine_config_t mips_machines[] = {
	{"nedved", nedved_mach_init, NULL, NULL, NULL},
	{"au1100", au1100_mach_init, NULL, NULL, NULL},
	{"fulong", fulong_mach_init, NULL, NULL, NULL},
	{"gs32eb1", gs32eb1_mach_init, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL},
};
#endif

static int
mips_parse_cpu(const char* param[])
{
	return 1;
}

static int mips_ICE_read_byte(generic_address_t addr, uint8_t *data){
	return 0;
}
static int mips_ICE_write_byte(generic_address_t addr, uint8_t data){
	return 0;
}
static uint32 mips_get_step(){
#if 0
	uint32 step = mstate->cycle;
	return step;
#endif
	MIPS_CPU_State* cpu = get_current_cpu();
	return cpu->core[0].step;
}
static char* mips_get_regname_by_id(int id){
	return (char *)mips_regstr[id];
}
static uint32 mips_get_regval_by_id(int id){
#if 0
	if(id == PC)
		return mstate->pc;
	return mstate->gpr[id];
#endif
	int core_id = 0;
	MIPS_CPU_State* cpu = get_current_cpu();

	if(id == PC)
		return cpu->core[core_id].pc;
	//return cpu->core[core_id].Reg[id];
	return cpu->core[core_id].gpr[id];
}
static exception_t mips_set_register_by_id(int id, uint32 value){
#if 0
	mstate->gpr[id] = value;
	return No_exp;
#endif
	MIPS_CPU_State* cpu = get_current_cpu();

	int core_id = 0;
	if(id == PC)
		cpu->core[core_id].pc = value;
	//cpu->core[core_id].Reg[id] = value;
	cpu->core[core_id].gpr[id] = value;

	return No_exp;
}

void
init_mips_arch ()
{
	static arch_config_t mips_arch;
	mips_arch.arch_name = "mips_dyncom";
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
	register_arch (&mips_arch);
}
