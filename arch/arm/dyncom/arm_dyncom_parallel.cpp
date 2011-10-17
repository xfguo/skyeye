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
 */
#include "armdefs.h"
#include "armemu.h"
#include "arm_dyncom_parallel.h"

#include <pthread.h>

#include "skyeye_dyncom.h"
#include "skyeye_thread.h"
#include "dyncom/tag.h"
#include "dyncom/basicblock.h"
#include "portable/usleep.h"
#include "bank_defs.h"

#include <stack>
using namespace std;

#define QUEUE_LENGTH 1024
static uint32_t compiled_queue[QUEUE_LENGTH]; /* list of tagged addresses */
static stack<uint32_t> compile_stack; /* stack of untranslated addresses */
static pthread_rwlock_t compiled_queue_rwlock;
static void* compiled_worker(void* cpu);
static void push_compiled_work(cpu_t* cpu, uint32_t pc);
/*
 * Three running mode: PURE_INTERPRET, PURE_DYNCOM, HYBRID
 */
//static running_mode_t mode = PURE_INTERPRET;
//static running_mode_t mode = PURE_DYNCOM;
static running_mode_t mode = HYBRID;
static char* running_mode_str[] = {
	"pure interpret running",
	"pure dyncom running",
	"hybrid running",
	NULL
};

void init_compiled_queue(cpu_t* cpu){
	memset(&compiled_queue[0], 0xff, sizeof(uint32_t) * QUEUE_LENGTH);
	printf("Current running mode: %s\n", running_mode_str[mode]);
	if(mode == HYBRID){
		if(pthread_rwlock_init(&compiled_queue_rwlock, NULL)){
			fprintf(stderr, "can not initilize the rwlock\n");
		}

		/* Create a thread to compile IR to native code */
		pthread_t id;
		create_thread(compiled_worker, (void*)cpu, &id);
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
	if (mode == HYBRID)
		state->stop_simulator = 1;
	else
		state->stop_simulator = 0;
	
	/* There might be an issue with Emulate26 */
	state->pc = ARMul_Emulate32(state);
	
	return;
}

/* In HYBRID mode, When encountering a new (untagged) pc, we recursive-tag it so all newly tagged
   instructions belongs to this basic block. A translated address, if it is an entry point,
   will be dyncom-executed. Else, it will be interpreted. */
void launch_compiled_queue(cpu_t* cpu, uint32_t pc){
	arm_core_t* core = (arm_core_t*)(cpu->cpu_data->obj);
	void * pfunc = NULL;
	//printf("\t\t### Launching %p %p %p Type %d\n", pc, core_->Reg[15], cpu->f.get_pc(cpu, cpu->rf.grf), core_->NextInstr);
	
	if (mode == PURE_INTERPRET)
	{
		interpret_cpu_step(cpu->cpu_data);
		return ;
	}
	
	/* pc correction due to the pipeline  */
	if (core->NextInstr == PRIMEPIPE) {
		/* If PRIMEPIPE, then the executed pc is the value in the pc register */
		/* No worries, in Dyncom mode, NextInstr is always PRIMEPIPE */
	} else {
		/* Else, the executed pc is pc register - 4 */
		pc = pc - 4 ;
	}
	
	/* Check if the executed pc already got tagged */
	if ( (get_tag(cpu, pc) & TAG_CODE) == 0)
	{
		push_compiled_work(cpu, pc);
	} 
	
	/* Check if the wanted instruction is in the dyncom engine */
	fast_map hash_map = cpu->dyncom_engine->fmap;
	pfunc = (void*) hash_map[pc & 0x1fffff];
		
	/* The instruction is not is the engine, we interpret it */
	if (!pfunc)
	{
		interpret_cpu_step(cpu->cpu_data);
	}
	else
	{
		//printf("Ready! In %p %d ", pc, core_->NextInstr);
		core->Reg[15] = pc;
		//printf("CPSR b %x ", core_->Cpsr);
		/* synchronize flags between dyncom and interpreter */
		if(mode == HYBRID) {
			core->Cpsr = core->Cpsr & 0xfffffff;
			core->Cpsr |= core->NFlag << 31;
			core->Cpsr |= core->ZFlag << 30;
			core->Cpsr |= core->CFlag << 29;
			core->Cpsr |= core->VFlag << 28;
		}

		int rc;
		rc = JIT_RETURN_NOERR;
		rc = um_cpu_run(cpu);

		if(mode == HYBRID) {
			core->NFlag = core->Cpsr & 0x80000000;
			core->ZFlag = core->Cpsr & 0x40000000;
			core->CFlag = core->Cpsr & 0x20000000;
			core->VFlag = core->Cpsr & 0x10000000;
		}
			
		if(rc == JIT_RETURN_TRAP){
			pfunc = (void*) hash_map[core->Reg[15] & 0x1fffff];
			//printf("| Out %d %p ",core_->NextInstr, core_->Reg[15]); 
			//printf("| pfunc %p | TRAP\n", pfunc);
			core->NextInstr = PRIMEPIPE;
#ifdef OPT_LOCAL_REGISTERS
			uint32_t instr;
			bus_read(32, core->Reg[15], &instr);
			//printf("In %s, OPT_LOCAL, pc=0x%x, instr=0x%x, number=0x%x\n", __FUNCTION__, core->Reg[15], instr, BITS(0,19));
	
			ARMul_OSHandleSWI(core, BITS(0,19));
#endif
			core->Reg[15] += 4;
			return;
		}	

		pfunc = (void *)hash_map[core->Reg[15] & 0x1fffff];
		//printf("| Out %d %p ",core_->NextInstr, core_->Reg[15]); 
		if (rc == JIT_RETURN_FUNCNOTFOUND)
		{
			//printf("| pushing %p ", core_->Reg[15]);
			//cpu_tag(cpu, core_->Reg[15]);
			//cpu_translate(cpu, core_->Reg[15]);
			//pfunc = (um_fp_t)hash_map[core_->Reg[15] & 0x1fffff];
			push_compiled_work(cpu, core->Reg[15]);
		}
		else
		{
			fprintf(stderr, "In %s, wrong return value %d\n", __FUNCTION__,  rc);
		}
		//printf("| pfunc %p\n", pfunc);

		/* Next executed function must be considered as a new function in the pipeline */
		core->NextInstr = PRIMEPIPE;
	}
}

static int cur_queue_pos = 0;
static void push_compiled_work(cpu_t* cpu, uint32_t pc){
	int i = 0;
	int cur_pos;
	cur_pos = cpu->dyncom_engine->cur_tagging_pos;
	/* check if the pc already exist in the queue */
	for(i = 0; i < cur_pos; i++)
		if(compiled_queue[i] == pc) {
			//printf("| in queue %d/%d %p ", i, cur_pos, pc);
			//printf("\t\t\t###### already in list %p\n", pc);
			//printf("pc 0x%x is also exist at %d\n", pc, cur_pos);
			return;
		}

	if(cur_pos >= 0 && cur_pos < QUEUE_LENGTH){
		//printf("\t##### Tagging %p\n", pc);
		
		if( mode == PURE_DYNCOM ){
			cpu_tag(cpu, pc);
			cpu_translate(cpu, pc);
			#if 0
			typedef int (*um_fp_t)(uint8_t *RAM, void *grf, void *srf, void *frf);
			um_fp_t pfunc = NULL;
			fast_map hash_map = cpu->dyncom_engine->fmap;
			pfunc = (um_fp_t)hash_map[pc & 0x1fffff];
			printf("######### (Dyncom) Translated %x %p\n", pc, pfunc);
			#endif
		}
		else{
			cpu_tag(cpu, pc);
			/* Locked data includes compiled_queue, stack, and function pointer */
			pthread_rwlock_wrlock(&compiled_queue_rwlock);
			//printf("In %s,place the pc=0x%x to the pos %d\n",__FUNCTION__, pc, cur_pos);
			compiled_queue[cur_pos] = pc;
			compile_stack.push(pc);
			compile_stack.push(cpu->dyncom_engine->cur_tagging_pos);
			pthread_rwlock_unlock(&compiled_queue_rwlock);
			//printf("##### Waiting to compile %p\n", pc);
		}
		cpu->dyncom_engine->cur_tagging_pos ++;
	}
	else{
		printf("In %s, compiled queue overflowed\n", __FUNCTION__);
	}
}
/* Compiled the target to the host address */
static void* compiled_worker(void* argp){
	cpu_t* cpu = (cpu_t*) argp;
	uint32_t pos_to_translate;
	for(;;){
try_compile:
		while(1){
			uint32_t compiled_addr = 0xFFFFFFFF;
			pthread_rwlock_rdlock(&compiled_queue_rwlock);
			if (!compile_stack.empty()) {
				pos_to_translate = compile_stack.top();
				compile_stack.pop();
				compiled_addr = compile_stack.top();
				compile_stack.pop();
			}
			pthread_rwlock_unlock(&compiled_queue_rwlock);
			if(compiled_addr == 0xFFFFFFFF)
				break;
			//printf("In %s, pc=0x%x\n", __FUNCTION__, compiled_queue[compiling_pos]);
			/* Just for get basicblock */
			//cpu_tag(cpu, compiled_queue[cur_pos]);
			/* functions will increasement in translate procedure */
			//printf("#### %x GET ", compiled_addr);
			fast_map hash_map = cpu->dyncom_engine->fmap;

			pthread_rwlock_rdlock(&(cpu->dyncom_engine->rwlock));
			void* pfunc = (void *)hash_map[compiled_addr & 0x1fffff];
			//void* pfunc = (void*)hash_map[compiled_addr & 0x1fffff];
			//printf("   pfunc exist for %x? %p ", compiled_addr, pfunc);
			if(pfunc == NULL){
				cpu->dyncom_engine->functions = pos_to_translate;
				cpu_translate(cpu, compiled_addr);
				//pfunc = (void *)hash_map[compiled_addr & 0x1fffff];
				//printf("####### (Hybrid) Translated %x %p\n", compiled_addr, pfunc);
				//printf("In %s, finish translate addr 0x%x, functions=%d, compiling_pos=%d\n", __FUNCTION__, compiled_addr, cpu->dyncom_engine->functions, compiling_pos);
			}
			if(pthread_rwlock_unlock(&(cpu->dyncom_engine->rwlock))){
				fprintf(stderr, "In %s, unlock error\n", __FUNCTION__);
			}
			//cur_pos = cpu->dyncom_engine->functions;
		}
		usleep(1);
	}
}
