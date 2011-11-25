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
 * 03/27/2011   Michael.Kang  <blackfin.kang@gmail.com> 
 * 10/2011 rewritten by Alexis He <ahe.krosk@gmail.com> 
 */
#include "armdefs.h"
//#include "armemu.h"
#include "arm_dyncom_parallel.h"
#include "arm_dyncom_mmu.h"

#include <pthread.h>

#include "skyeye_dyncom.h"
#include "skyeye_thread.h"
#include "stat.h"
#include "dyncom/tag.h"
#include "dyncom/basicblock.h"
#include "bank_defs.h"

#include <stack>
using namespace std;
#ifndef __ARMEMU_H__
#define __ARMEMU_H__
/* Different ways to start the next instruction.  */
#define SEQ           0
#define NONSEQ        1
#define PCINCEDSEQ    2
#define PCINCEDNONSEQ 3
#define PRIMEPIPE     4
#define RESUME        8

#ifdef __cplusplus
extern "C" {
#endif
extern ARMword ARMul_Emulate32 (ARMul_State *);
#ifdef __cplusplus
}
#endif

#define INTBITS (0xc0L)
#endif

#define QUEUE_LENGTH 0x6000
/* Monothread: threshold compilation only
   Multithread: threshold compilation or Polling compilation (cpu intensive) */
#define MULTI_THREAD 1
#define LIFO 0
static uint32_t compiled_queue[QUEUE_LENGTH]; /* list of tagged addresses. Note: is not a shared resource */
static stack<uint32_t> compile_stack; /* stack of untranslated addresses. Note: is a shared resource */
static pthread_rwlock_t compile_stack_rwlock;
static pthread_rwlock_t translation_rwlock;
static uint32_t translated_block = 0; /* translated block count, for block threshold */
static void* compiled_worker(void* cpu);
static void push_compiled_work(cpu_t* cpu, uint32_t pc);
/*
 * Three running mode: PURE_INTERPRET, PURE_DYNCOM, HYBRID
 */
//running_mode_t running_mode = PURE_INTERPRET;
running_mode_t running_mode = PURE_DYNCOM;
//running_mode_t running_mode = HYBRID;
static char* running_mode_str[] = {
	"pure interpret running",
	"pure dyncom running",
	"hybrid running",
	NULL
};

#if L3_HASHMAP
#define PFUNC(pc)					\
	if(hash_map[HASH_MAP_INDEX_L1(pc)] == NULL)	\
		pfunc = NULL;				\
	else if(hash_map[HASH_MAP_INDEX_L1(pc)][HASH_MAP_INDEX_L2(pc)] == NULL)				\
		pfunc = NULL;										\
	else if(hash_map[HASH_MAP_INDEX_L1(pc)][HASH_MAP_INDEX_L2(pc)][HASH_MAP_INDEX_L3(pc)] == NULL)	\
		pfunc = NULL;										\
	else												\
		pfunc = (void *)hash_map[HASH_MAP_INDEX_L1(pc)][HASH_MAP_INDEX_L2(pc)][HASH_MAP_INDEX_L3(pc)];
#else
#define PFUNC(pc)					\
	pfunc = (void*) hash_map[pc & 0x1fffff];
#endif

void init_compiled_queue(cpu_t* cpu){
	memset(&compiled_queue[0], 0xff, sizeof(uint32_t) * QUEUE_LENGTH);
	if (get_skyeye_pref()->interpret_mode) {
		running_mode = PURE_INTERPRET;
	}
	skyeye_log(Info_log, __FUNCTION__, "Current running mode: %s\n", running_mode_str[running_mode]);
	if (running_mode == HYBRID){
		if(pthread_rwlock_init(&compile_stack_rwlock, NULL)){
			fprintf(stderr, "can not initilize the rwlock\n");
		}
		if(pthread_rwlock_init(&translation_rwlock, NULL)){
			fprintf(stderr, "can not initilize the rwlock\n");
		}
		/* Create a thread to compile IR to native code */
#if MULTI_THREAD
		pthread_t id;
		create_thread(compiled_worker, (void*)cpu, &id);
#endif
	}
	cpu->dyncom_engine->cur_tagging_pos = 0;
}

static void interpret_cpu_step(conf_object_t * running_core){
	arm_core_t *state = (arm_core_t *)running_core->obj;

#if 0
	static uint32_t flag = 0;
	if (flag)
	{
		state->NextInstr = PRIMEPIPE;
		flag = 0;
	}
#endif	
	/* Initialize interpreter step */
	state->step++;
	state->cycle++;
	state->EndCondition = 0;
	if (running_mode == HYBRID)
		state->stop_simulator = 1;
	else
		state->stop_simulator = 1; /* 1 if debugging */
	
	/* There might be an issue with Emulate26 */
	state->pc = ARMul_Emulate32(state);
	
	return;
}


static int flush_current_page(cpu_t *cpu){
	//arm_core_t* core = (arm_core_t*)(cpu->cpu_data);
	arm_core_t* core = (arm_core_t*)(cpu->cpu_data->obj);
	addr_t effec_pc = *(addr_t*)cpu->rf.pc;
	//printf("effec_pc is %x\n", effec_pc);
	//printf("in %s\n", __FUNCTION__);
	int ret; //= cpu->mem_ops.effective_to_physical(cpu, effec_pc, (uint32_t*)cpu->rf.phys_pc);
	ret = cpu->mem_ops.effective_to_physical(cpu, effec_pc, &core->phys_pc);
	if (ret != 0)
	{
		printf("##### in %s, effec_pc %x phys_pc %x ret %d\n", __FUNCTION__, effec_pc, *((uint32_t*)cpu->rf.phys_pc), ret);
	}
	cpu->current_page_phys = core->phys_pc & 0xfffff000;
	cpu->current_page_effec = core->pc & 0xfffff000;
	return ret;
}

/* This function clears the whole basic block cache (translation and tags), and frees the memory */
inline int clear_cache(cpu_t *cpu, fast_map hash_map)
{
	uint32_t index;
	void* pfunc = NULL;
	for(index = 0; index <= cpu->dyncom_engine->cur_tagging_pos; index++) {
		compiled_queue[index] = -1;
		cpu->dyncom_engine->startbb[index].clear();
	}
	clear_fmap(hash_map);
	clear_tag_table(cpu);
	cpu_flush(cpu);
	cpu->dyncom_engine->cur_tagging_pos = 0;
	translated_block = 0;
	//cpu->dyncom_engine->exec_engine = ExecutionEngine::create(cpu->dyncom_engine->mod);
}

/* For HYBRID or PURE_INTERPRETER.
   In HYBRID mode, when encountering a new (untagged) pc, we recursive-tag it so all newly tagged
   instructions belongs to this basic block. A translated address, if it is an entry point,
   will be dyncom-executed. Else, it will be interpreted. */
int launch_compiled_queue(cpu_t* cpu, uint32_t pc){
	arm_core_t* core = (arm_core_t*)(cpu->cpu_data->obj);
	void * pfunc = NULL;
	
	//printf("\t\t### Launching %p Type %d\n", pc, core->NextInstr);
	
	if (running_mode == PURE_INTERPRET)
	{
		interpret_cpu_step(cpu->cpu_data);
		return 0;
	}
	
	/* pc correction due to the pipeline  */
	if (core->NextInstr == PRIMEPIPE) {
		/* If PRIMEPIPE, then the executed pc is the value in the pc register */
		/* In Pure Dyncom mode, NextInstr is always PRIMEPIPE */
	} else if (core->NextInstr == PCINCEDSEQ) {
		/* If PCINCEDSEQ, the executed pc is -8 */
		pc = pc - 8 ;
	} else {
		/* Else, the executed pc is pc register - 4 */
		pc = pc - 4 ;
	}
	
	
	/* Attempt to retrieve physical address if mmu, only in kernel mode */
	if (is_user_mode(cpu))
	{
		core->phys_pc = pc;
	}
	else 
	{
		uint32_t dummy, ret = 0;
		//ret = core->mmu.ops.load_instr(core,pc,&core->phys_pc); // for interpreter only
		
		/* effective_to_physical function uses dyncom registers.
		   Therefore, they need to be imported from interpreter registers.
		   Hopefully, the new interpreter will use unified registers. */
		CP15REG(CP15_TRANSLATION_BASE_CONTROL) = core->mmu.translation_table_ctrl;
		CP15REG(CP15_TRANSLATION_BASE_TABLE_0) = core->mmu.translation_table_base0;
		CP15REG(CP15_TRANSLATION_BASE_TABLE_1) = core->mmu.translation_table_base1;
		CP15REG(CP15_DOMAIN_ACCESS_CONTROL) = core->mmu.domain_access_control;
		CP15REG(CP15_CONTROL) = core->mmu.control;
		
		ret = cpu->mem_ops.effective_to_physical(cpu, pc, &core->phys_pc);
		
		/* targeted instruction is not in memory, let the interpreter prefetch abort do the job */
		if (ret != 0)
		{
			//printf("#### Issues in mmu %d at pc %x, phys_pc? %x\n", ret, pc, core->phys_pc);
			interpret_cpu_step(cpu->cpu_data);
			return 0;
		}
	}
	
	/* Check if the executed pc already got tagged */
	uint32_t counter = 0, entry = 0;
	uint32_t tag = check_tag_execution(cpu, core->phys_pc, &counter, &entry);
	if ((tag & TAG_CODE) == 0)
	{
		push_compiled_work(cpu, core->phys_pc);
	}
	
	/* Check if the wanted instruction is in the dyncom engine */
	fast_map hash_map = cpu->dyncom_engine->fmap;
	PFUNC(core->phys_pc);

	/* If not, compile it if it has reached execution threshold
	   Note: in pure dyncom, push_compiled_work should have translated the block,
	   so this code will not be executed in pure dyncom. It is ok since
	   we don't care much about the compiled queue in pure dyncom. */
#if !MULTI_THREAD
	if ((!pfunc) && (tag & TAG_ENTRY) && (counter > 0x1000))
	{
		uint32_t index;
		if (translated_block > 0x200)
		{
			clear_cache(cpu, hash_map);
			printf(" (Monothread) Cleaning translated blocks ----- \n");
			return 0;
		}

		for (index = 0; index < cpu->dyncom_engine->cur_tagging_pos; index++)
		{
			if(compiled_queue[index] == core->phys_pc) {
				//printf("### Compiling %x for %x idx %x\n", entry, core->phys_pc, index);
				cpu->dyncom_engine->functions = index;
				//printf("Planning to translate block n*%x at pc %x phys %x\n", cpu->dyncom_engine->functions, pc, core->phys_pc);
				cpu_translate(cpu, compiled_queue[index]);
				//PFUNC(core->phys_pc);
				translated_block += 1;
				break;
			}
		}
	}
#else
#if !LIFO
	if ((!pfunc) && (tag & TAG_ENTRY) && (counter > 0x1000))
	{
		uint32_t index;
		if (translated_block > 0x200)
		{
			pthread_rwlock_wrlock(&translation_rwlock);
			clear_cache(cpu, hash_map);
			printf(" (Multithread) Cleaning translated blocks ----- \n");
			while (!compile_stack.empty())
			{
				compile_stack.pop();
			}
			pthread_rwlock_unlock(&translation_rwlock);
			return 0;
		}

		for (index = 0; index < cpu->dyncom_engine->cur_tagging_pos; index++)
		{
			if(compiled_queue[index] == core->phys_pc) {
				pthread_rwlock_wrlock(&compile_stack_rwlock);
				//if (compile_stack.empty() || compile_stack.top() != index)
				{
				//	printf("### Pushing to compile %x for %x idx %x\n", entry, core->phys_pc, index);
					compile_stack.push(core->phys_pc);
					compile_stack.push(index);
				}
				pthread_rwlock_unlock(&compile_stack_rwlock);
				compiled_queue[index] = -1;
				//printf("Planning to translate block n*%x at pc %x phys %x\n", cpu->dyncom_engine->functions, pc, core->phys_pc);
				break;
			}
		}
	}
#else
	if ((!pfunc) && (tag & TAG_ENTRY) && (counter > 0x5))
	{
		uint32_t index;
		if (translated_block > 0x400)
		{
			pthread_rwlock_wrlock(&translation_rwlock);
			clear_cache(cpu, hash_map);
			printf(" (LIFO) Cleaning translated blocks ----- \n");
			while (!compile_stack.empty())
			{
				compile_stack.pop();
			}
			pthread_rwlock_unlock(&translation_rwlock);
			return 0;
		}

		for (index = 0; index < cpu->dyncom_engine->cur_tagging_pos; index++)
		{
			if(compiled_queue[index] == core->phys_pc) {
				pthread_rwlock_wrlock(&compile_stack_rwlock);
				if (compile_stack.empty() || compile_stack.top() != index)
				{
					//printf("### Pushing to compile %x for %x idx %x\n", entry, core->phys_pc, index);
					compile_stack.push(core->phys_pc);
					compile_stack.push(index);
				}
				pthread_rwlock_unlock(&compile_stack_rwlock);
				//compiled_queue[index] = -1;
				//printf("Planning to translate block n*%x at pc %x phys %x\n", cpu->dyncom_engine->functions, pc, core->phys_pc);
				break;
			}
		}
	}
#endif
#endif

	//printf("#### counter is %x for pc %x\n", counter, pc);
	
	/* The instruction is not is the engine, we interpret it */
	if (!pfunc)
	{
		//printf("Interpreting %p-%p with MMU %x\n", core->phys_pc, pc, (core->mmu.control));
		interpret_cpu_step(cpu->cpu_data);
		return 0;
	}
	else
	{
		//if (get_skyeye_pref()->start_logging)
			//printf("Ready! In %p %p Type %d \n", pc, core->phys_pc, core->NextInstr);

		/* synchronize flags between dyncom and interpreter */
		//UPDATE_TIMING(cpu, TIMER_SWITCH, true);
		//printf("In Cpsr %x N %x Z %x C %x V %x -> ", core->Cpsr, core->NFlag, core->ZFlag, core->CFlag, core->VFlag);
		core->Cpsr = core->Cpsr & 0xfffffff;
		core->Cpsr |= core->NFlag << 31;
		core->Cpsr |= core->ZFlag << 30;
		core->Cpsr |= core->CFlag << 29;
		core->Cpsr |= core->VFlag << 28;
		//core->Cpsr |= IFFlags = (((core->Cpsr & INTBITS) >> 6) & 3);
		//printf("-> Cpsr %x\n", core->Cpsr);

		if (!is_user_mode(cpu)) {
			CP15REG(CP15_TRANSLATION_BASE_CONTROL) = core->mmu.translation_table_ctrl;
			CP15REG(CP15_TRANSLATION_BASE_TABLE_0) = core->mmu.translation_table_base0;
			CP15REG(CP15_TRANSLATION_BASE_TABLE_1) = core->mmu.translation_table_base1;
			CP15REG(CP15_DOMAIN_ACCESS_CONTROL) = core->mmu.domain_access_control;
			//CP15REG(CP15_CONTROL) &= ~CONTROL_MMU;
			//CP15REG(CP15_CONTROL) &= ~CONTROL_VECTOR;
			//CP15REG(CP15_CONTROL) |= core->mmu.control & CONTROL_MMU;
			//CP15REG(CP15_CONTROL) |= core->mmu.control & CONTROL_VECTOR;
			CP15REG(CP15_CONTROL) = core->mmu.control;
			CP15REG(CP15_INSTR_FAULT_STATUS) = core->mmu.fault_statusi;
			CP15REG(CP15_FAULT_STATUS) = core->mmu.fault_status;
			CP15REG(CP15_FAULT_ADDRESS) = core->mmu.fault_address;
			
			switch (core->Mode)
			{
			case USER32MODE:
				core->Spsr_copy = core->Spsr[USERBANK];
				core->RegBank[USERBANK][13] = core->Reg[13];
				core->RegBank[USERBANK][14] = core->Reg[14];
				break;
			case IRQ32MODE:
				core->Spsr_copy = core->Spsr[IRQBANK];
				core->RegBank[IRQBANK][13] = core->Reg[13];
				core->RegBank[IRQBANK][14] = core->Reg[14];
				break;
			case SVC32MODE:
				core->Spsr_copy = core->Spsr[SVCBANK];
				core->RegBank[SVCBANK][13] = core->Reg[13];
				core->RegBank[SVCBANK][14] = core->Reg[14];
				break;
			case ABORT32MODE:
				core->Spsr_copy = core->Spsr[ABORTBANK];
				core->RegBank[ABORTBANK][13] = core->Reg[13];
				core->RegBank[ABORTBANK][14] = core->Reg[14];
				break;
			case UNDEF32MODE:
				core->Spsr_copy = core->Spsr[UNDEFBANK];
				core->RegBank[UNDEFBANK][13] = core->Reg[13];
				core->RegBank[UNDEFBANK][14] = core->Reg[14];
				break;
			case FIQ32MODE:
				core->Spsr_copy = core->Spsr[FIQBANK];
				core->RegBank[FIQBANK][13] = core->Reg[13];
				core->RegBank[FIQBANK][14] = core->Reg[14];
				break;
			}
	
			#if 0 // performance ?
			core->Spsr_copy = core->Spsr[core->Bank];
			core->RegBank[core->Bank][13] = core->Reg[13];
			core->RegBank[core->Bank][14] = core->Reg[14];
			#endif
			
			core->Reg_usr[0] = core->RegBank[USERBANK][13];
			core->Reg_usr[1] = core->RegBank[USERBANK][14];
			core->Reg_irq[0] = core->RegBank[IRQBANK][13];
			core->Reg_irq[1] = core->RegBank[IRQBANK][14];
			core->Reg_svc[0] = core->RegBank[SVCBANK][13];
			core->Reg_svc[1] = core->RegBank[SVCBANK][14];
			core->Reg_abort[0] = core->RegBank[ABORTBANK][13];
			core->Reg_abort[1] = core->RegBank[ABORTBANK][14];
			core->Reg_undef[0] = core->RegBank[UNDEFBANK][13];
			core->Reg_undef[1] = core->RegBank[UNDEFBANK][14];
			core->Reg_firq[0] = core->RegBank[FIQBANK][13];
			core->Reg_firq[1] = core->RegBank[FIQBANK][14];
		}
		
		if (core->NextInstr == PRIMEPIPE)
		{
			
		} else if (core->NextInstr == SEQ){
			core->Reg[15] -= 4;
		} else if (core->NextInstr == NONSEQ){
			core->Reg[15] -= 4;
		}
		//UPDATE_TIMING(cpu, TIMER_SWITCH, false);
		int rc = JIT_RETURN_NOERR;
		if (is_user_mode(cpu))
			rc = um_cpu_run(cpu);
		else
			rc = cpu_run(cpu);
		
		//UPDATE_TIMING(cpu, TIMER_SWITCH, true);
		core->NFlag = ((core->Cpsr & 0x80000000) != 0);
		core->ZFlag = ((core->Cpsr & 0x40000000) != 0);
		core->CFlag = ((core->Cpsr & 0x20000000) != 0);
		core->VFlag = ((core->Cpsr & 0x10000000) != 0);
		core->IFFlags = (((core->Cpsr & INTBITS) >> 6) & 3);
		//printf("Out Cpsr %x N %x Z %x C %x V %x\n", core->Cpsr, core->NFlag, core->ZFlag, core->CFlag, core->VFlag);

		if (!is_user_mode(cpu)) {
			core->mmu.translation_table_ctrl = CP15REG(CP15_TRANSLATION_BASE_CONTROL);
			core->mmu.translation_table_base0 = CP15REG(CP15_TRANSLATION_BASE_TABLE_0);
			core->mmu.translation_table_base1 = CP15REG(CP15_TRANSLATION_BASE_TABLE_1);
			core->mmu.domain_access_control = CP15REG(CP15_DOMAIN_ACCESS_CONTROL);
			core->mmu.control = CP15REG(CP15_CONTROL);
			core->mmu.fault_statusi = CP15REG(CP15_INSTR_FAULT_STATUS);
			core->mmu.fault_status = CP15REG(CP15_FAULT_STATUS);
			core->mmu.fault_address = CP15REG(CP15_FAULT_ADDRESS);
			
			switch (core->Mode)
			{
			case USER32MODE:
				core->Bank = USERBANK;
				core->Spsr[USERBANK] = core->Spsr_copy;
				core->Reg_usr[0] = core->Reg[13];
				core->Reg_usr[1] = core->Reg[14];
				break;
			case IRQ32MODE:
				core->Bank = IRQBANK;
				core->Spsr[IRQBANK] = core->Spsr_copy;
				core->Reg_irq[0] = core->Reg[13];
				core->Reg_irq[1] = core->Reg[14];
				break;
			case SVC32MODE:
				core->Bank = SVCBANK;
				core->Spsr[SVCBANK] = core->Spsr_copy;
				core->Reg_svc[0] = core->Reg[13];
				core->Reg_svc[1] = core->Reg[14];
				break;
			case ABORT32MODE:
				core->Bank = ABORTBANK;
				core->Spsr[ABORTBANK] = core->Spsr_copy;
				core->Reg_abort[0] = core->Reg[13];
				core->Reg_abort[1] = core->Reg[14];
				break;
			case UNDEF32MODE:
				core->Bank = UNDEFBANK;
				core->Spsr[UNDEFBANK] = core->Spsr_copy;
				core->Reg_undef[0] = core->Reg[13];
				core->Reg_undef[1] = core->Reg[14];
				break;
			case FIQ32MODE:
				core->Bank = FIQBANK;
				core->Spsr[FIQBANK] = core->Spsr_copy;
				core->Reg_firq[0] = core->Reg[13];
				core->Reg_firq[1] = core->Reg[14];
				break;
			}

			core->RegBank[USERBANK][13] = core->Reg_usr[0];
			core->RegBank[USERBANK][14] = core->Reg_usr[1];
			core->RegBank[IRQBANK][13] = core->Reg_irq[0];
			core->RegBank[IRQBANK][14] = core->Reg_irq[1];
			core->RegBank[SVCBANK][13] = core->Reg_svc[0];
			core->RegBank[SVCBANK][14] = core->Reg_svc[1];
			core->RegBank[ABORTBANK][13] = core->Reg_abort[0];
			core->RegBank[ABORTBANK][14] = core->Reg_abort[1];
			core->RegBank[UNDEFBANK][13] = core->Reg_undef[0];
			core->RegBank[UNDEFBANK][14] = core->Reg_undef[1];
			core->RegBank[FIQBANK][13] = core->Reg_firq[0];
			core->RegBank[FIQBANK][14] = core->Reg_firq[1];
		}

		//UPDATE_TIMING(cpu, TIMER_SWITCH, false);
		
		if (is_user_mode(cpu)) {
			core->phys_pc = core->Reg[15];
		} else {
			uint32_t ret;
			ret = cpu->mem_ops.effective_to_physical(cpu, core->Reg[15], &core->phys_pc);
		}
		
		//if (get_skyeye_pref()->start_logging)
		//	printf("### Out of jit return %d - %p %p\n", rc, core->Reg[15], core->phys_pc);

		/* In all cases, the next instruction should be considered first in pipeline */
		core->NextInstr = PRIMEPIPE;
		
		/* General rule: return 1 if next block should be handled by Dyncom */
		switch (rc) {
		case JIT_RETURN_NOERR: /* JIT code wants us to end execution */
		case JIT_RETURN_TIMEOUT:
			PFUNC(core->phys_pc);
			if (pfunc) {
				//printf("Timeout - Next handling by DYNCOM %x\n", core->Reg[15]);
				return 1;
			} else {
				//printf("Timeout - Next handling by INTERP %x\n", core->Reg[15]);
				return 0;
			}
		case JIT_RETURN_SINGLESTEP:
			/* TODO */
		case JIT_RETURN_FUNCNOTFOUND:
			//printf("pc %x is not found, phys_pc is %p\n", core->Reg[15], core->phys_pc);
			if (!is_user_mode(cpu))
			{
				switch_mode(core, core->Cpsr & 0x1f);
				if (flush_current_page(cpu)) {
					return 0;
				}
			}
			//clear_tag_page(cpu, core->phys_pc); /* do it or not ? */
			push_compiled_work(cpu, core->phys_pc);
			return 0;
		case JIT_RETURN_TRAP:
			{
				/* user mode handling */
				if (is_user_mode(cpu))
				{
				#ifdef OPT_LOCAL_REGISTERS
					uint32_t instr;
					bus_read(32, core->Reg[15], &instr);
					ARMul_OSHandleSWI(core, BITS(0,19));
				#endif
					core->Reg[15] += 4;
					core->phys_pc = core->Reg[15];
					//if (get_skyeye_pref()->start_logging)
					//	printf("Trap - Handled %x\n", core->phys_pc);
					return 0;
				}
				
				/* kernel mode handling */
				PFUNC(core->phys_pc);
				if (pfunc) {
					//if (get_skyeye_pref()->start_logging)
					//	printf("Trap - Next handling by DYNCOM %x %x\n", core->Reg[15], core->phys_pc);
				} else {
					core->Reg[15] += 4;
					//if (get_skyeye_pref()->start_logging)
					//	printf("Trap - Next handling by INTERP %x %x\n", core->Reg[15], core->phys_pc);
					return 0;
				}
				
				//printf("###### Trap %x\n", core->Reg[15]);
				if (core->syscallSig) {
					return 1;
				}
				if (cpu->check_int_flag == 1) {
					cpu->check_int_flag = 0;
					return 1;
				}
				if (core->abortSig) {
					return 1;
				}
				
				/* if regular trap */
				uint32_t mode = core->Cpsr & 0x1f;
				if ( (mode != core->Mode) && (!is_user_mode(cpu)) ) {
					switch_mode(core, mode);
				}
				
				core->Reg[15] += 4;
			}
			return 1;
		default:
			fprintf(stderr, "unknown return code: %d\n", rc);
			skyeye_exit(-1);
		}
	}
	return 0;
}

/* For PURE_DYNCON mode. This one
   is a direct copy of the old one,
   and is far less complicated */
int launch_compiled_queue_dyncom(cpu_t* cpu, uint32_t pc) {
	arm_core_t* core = (arm_core_t*)(cpu->cpu_data->obj);
	void * pfunc = NULL;
	
	/* set correct pc */
	if (is_user_mode(cpu))
	{
		core->phys_pc = pc;
	}
	
	/* if cpu_run doesn't find the address of the block, it will return asap */
	int rc = JIT_RETURN_NOERR;
	if (is_user_mode(cpu))
			rc = um_cpu_run(cpu);
		else
			rc = cpu_run(cpu);
	
	/* General rule: return 1 if next block should be handled by Dyncom */
	switch (rc) {
	case JIT_RETURN_NOERR: /* JIT code wants us to end execution */
	case JIT_RETURN_TIMEOUT:
		//printf("Timeout - Next handling by DYNCOM %x\n", core->Reg[15]);
		return 1;
	case JIT_RETURN_SINGLESTEP:
		/* TODO */
	case JIT_RETURN_FUNCNOTFOUND:
		//printf("pc %x is not found, phys_pc is %p\n", core->Reg[15], core->phys_pc);
		if (!is_user_mode(cpu))
		{
			switch_mode(core, core->Cpsr & 0x1f);
			if (flush_current_page(cpu)) {
				return 1;
			}
		}
		//clear_tag_page(cpu, core->phys_pc); /* do it or not ? */
		push_compiled_work(cpu, core->phys_pc); // in usermode, it might be more accurate to translate reg[15] instead
		return 0;
	case JIT_RETURN_TRAP:
	{
		/* user mode handling */
		if (is_user_mode(cpu))
		{
		#ifdef OPT_LOCAL_REGISTERS
			uint32_t instr;
			bus_read(32, core->Reg[15], &instr);
			ARMul_OSHandleSWI(core, BITS(0,19));
		#endif
			core->Reg[15] += 4;
			core->phys_pc = core->Reg[15];
			//if (get_skyeye_pref()->start_logging)
			//	printf("Trap - Handled %x\n", core->phys_pc);
			return 0;
		}
		
		if (core->syscallSig) {
			return 1;
		}
		if (cpu->check_int_flag == 1) {
			cpu->check_int_flag = 0;
			return 1;
		}
		if (core->abortSig) {
			return 1;
		}
			
		/* if regular trap */
		uint32_t mode = core->Cpsr & 0x1f;
		if ( (mode != core->Mode) && (!is_user_mode(cpu)) ) {
			switch_mode(core, mode);
		}
		
		core->Reg[15] += 4;
		return 1;
	}
	default: 
	{
		fprintf(stderr, "unknown return code: %d\n", rc);
		skyeye_exit(-1);
	}
	}
	return 1;
}

/* This function handles tagging */
static void push_compiled_work(cpu_t* cpu, uint32_t pc){
	int i = 0;
	int cur_pos = 0;
	
	#if 1
	if (running_mode == HYBRID) {
		cur_pos = cpu->dyncom_engine->cur_tagging_pos;
		/* check if the pc already exist in the queue */
		for(i = 0; i < cur_pos; i++) {
			if(compiled_queue[i] == pc) {
				//printf("pc 0x%x is also exist at %d\n", pc, cur_pos);
				return;
			}
		}

		if(cur_pos >= 0 && cur_pos < QUEUE_LENGTH)
		{
			//printf("\t##### Tagging %p\n", pc);
			cpu_tag(cpu, pc);
			/* Locked data includes compiled_queue, stack, and function pointer */
			//pthread_rwlock_wrlock(&compiled_queue_rwlock);
			//printf("In %s, place the pc=0x%x to the pos %d\n",__FUNCTION__, pc, cur_pos);
			compiled_queue[cur_pos] = pc;
			//pthread_rwlock_unlock(&compiled_queue_rwlock);
			//printf("##### Waiting to compile %p, %d\n", pc, cur_pos);
			cpu->dyncom_engine->cur_tagging_pos ++;
		}
		else
			printf("In %s, compiled queue overflowed\n", __FUNCTION__);
	}
	else if (running_mode == PURE_DYNCOM) {
		if (translated_block > 0x1800)
		{
			printf("\t(Dyncom) Cleaning %x translated blocks ----- \n", cpu->dyncom_engine->cur_tagging_pos);
			clear_cache(cpu, cpu->dyncom_engine->fmap);
		}
		cpu_tag(cpu, pc);
		cpu->dyncom_engine->cur_tagging_pos ++;
		cpu_translate(cpu, pc);
		translated_block += 1;
		#if 0
		void* pfunc = NULL;
		fast_map hash_map = cpu->dyncom_engine->fmap;
		PFUNC(pc);
		//printf("######### (Dyncom) Translated %x %p rank\n", pc, pfunc, cpu->dyncom_engine->cur_tagging_pos);
		#endif
	}
	else
		printf("In %s, this function should not be called in interpreter mode\n", __FUNCTION__);
	
	
	#else
	if (running_mode == HYBRID)
		cur_pos = cpu->dyncom_engine->cur_tagging_pos;
	
	/* check if the pc already exist in the queue */
	for(i = 0; i < cur_pos; i++)
		if(compiled_queue[i] == pc) {
			printf("pc 0x%x is also exist at %d\n", pc, cur_pos);
			return;
		}

	if(cur_pos >= 0 && cur_pos < QUEUE_LENGTH){
		//printf("\t##### Tagging %p\n", pc);
		
		if( running_mode == PURE_DYNCOM ){
			cpu_tag(cpu, pc);
			cpu_translate(cpu, pc);
			translated_block += 1;
			#if 0
			void* pfunc = NULL;
			fast_map hash_map = cpu->dyncom_engine->fmap;
			PFUNC(pc);
			//printf("######### (Dyncom) Translated %x %p rank\n", pc, pfunc, cpu->dyncom_engine->cur_tagging_pos);
			#endif
		}
		else{
			cpu_tag(cpu, pc);
			/* Locked data includes compiled_queue, stack, and function pointer */
			//pthread_rwlock_wrlock(&compiled_queue_rwlock);
			//printf("In %s, place the pc=0x%x to the pos %d\n",__FUNCTION__, pc, cur_pos);
			compiled_queue[cur_pos] = pc;
			//pthread_rwlock_unlock(&compiled_queue_rwlock);
			//printf("##### Waiting to compile %p, %d\n", pc, cur_pos);
		}
		cpu->dyncom_engine->cur_tagging_pos ++;
	}
	else{
		printf("In %s, compiled queue overflowed\n", __FUNCTION__);
	}
	#endif
}
/* Compiled the target to the host address */
static void* compiled_worker(void* argp){
	cpu_t* cpu = (cpu_t*) argp;
	uint32_t pos_to_translate;
	for(;;){
try_compile:
		while(1){
			pthread_rwlock_wrlock(&translation_rwlock);
			uint32_t compiled_addr = 0xFFFFFFFF;
			pthread_rwlock_rdlock(&compile_stack_rwlock);
			if (!compile_stack.empty()) {
				pos_to_translate = compile_stack.top();
				compile_stack.pop();
				compiled_addr = compile_stack.top();
				compile_stack.pop();
			}
			pthread_rwlock_unlock(&compile_stack_rwlock);
			if(compiled_addr == 0xFFFFFFFF) {
				pthread_rwlock_unlock(&translation_rwlock);
				break;
			}
			//printf("In %s, pc=0x%x\n", __FUNCTION__, compiled_queue[compiling_pos]);
			/* Just for get basicblock */
			//cpu_tag(cpu, compiled_queue[cur_pos]);
			/* functions will increasement in translate procedure */
			//printf("#### %x GET \n", compiled_addr);
			fast_map hash_map = cpu->dyncom_engine->fmap;
			void* pfunc;

			PFUNC(compiled_addr);
			if(pfunc == NULL){
				cpu->dyncom_engine->functions = pos_to_translate;
				cpu_translate(cpu, compiled_addr);
				translated_block += 1;
				//printf("##### (Hybrid) Translated %p %x\n", compiled_addr, pos_to_translate);
			}
			pthread_rwlock_unlock(&translation_rwlock);
		}
		usleep(2);
	}
}
