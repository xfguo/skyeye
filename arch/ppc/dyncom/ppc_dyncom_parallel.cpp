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
#include <pthread.h>
#include "ppc_cpu.h"
#include "ppc_mmu.h"
#include "ppc_exc.h"
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
#include "ppc_dec.h"
#include "ppc_dyncom_dec.h"
#include "ppc_dyncom_parallel.h"
#include "ppc_syscall.h"

#include "skyeye_dyncom.h"
#include "dyncom/tag.h"
#include "dyncom/basicblock.h"
#include "portable/usleep.h"

#define QUEUE_LENGTH 1024
static uint32_t compiled_queue[QUEUE_LENGTH];
static pthread_rwlock_t compiled_queue_rwlock;
static void* compiled_worker(void* cpu);
static void push_compiled_work(cpu_t* cpu, uint32_t pc);
/*
 * Three running mode: PURE_INTERPRET, PURE_DYNCOM, HYBRID
 */
//static running_mode_t mode = PURE_INTERPRET;
static running_mode_t mode = PURE_DYNCOM;
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
	uint32 real_addr;
	e500_core_t *core = (e500_core_t *)running_core->obj;
	
	core->step++;
	core->npc = core->pc + 4;
	/* do not need to mmu translation for user program */
	real_addr = core->pc;	
	uint32 instr;
	if(bus_read(32, real_addr, &instr) != 0){
		/* some error handler */
	}
	//core->current_opc = ppc_word_from_BE(instr);
	core->current_opc = instr;

	ppc_exec_opc(core);
	//debug_log(core);	
exec_npc:
	core->pc = core->npc;
}

void launch_compiled_queue(cpu_t* cpu, uint32_t pc){
	if(mode != PURE_INTERPRET){
		if(get_tag(cpu, pc) == TAG_UNKNOWN){
			//printf("In %s, TAG_UNKNOWN, push compiled pc = 0x%x\n", __FUNCTION__, pc);
			push_compiled_work(cpu, pc);
		}
		/* 
		 * in the entry of jit, we want to know if JIT is compiled 
	 	*/
		if(is_start_of_basicblock(cpu, pc)){
			int rc;
			rc = JIT_RETURN_NOERR;
			e500_core_t* core = (e500_core_t*)(cpu->cpu_data->obj);
			core->phys_pc = core->pc;
			//printf("In %s, found compiled pc = 0x%x\n", __FUNCTION__, pc);

			//rc = cpu_run(cpu);
			rc = um_cpu_run(cpu);
			core->pc = core->phys_pc;
			if(rc == JIT_RETURN_TRAP){
#ifdef OPT_LOCAL_REGISTERS
				ppc_syscall(core);
				core->phys_pc += 4;
#endif
			}	
			else if(rc == JIT_RETURN_FUNCNOTFOUND){
				//printf("In %s, FUNCNOTFOUND, push compiled pc = 0x%x\n", __FUNCTION__, core->pc);
				/* cpu_run have already run some JIT functions */
				if(core->pc != pc)
					push_compiled_work(cpu, core->pc);
			}	
			else{
				fprintf(stderr, "In %s, wrong return value, rc=0x%x\n", __FUNCTION__, rc);
			}
		}
	}//if(mode != PURE_INTERPRET)
	if(mode != PURE_DYNCOM)
		interpret_cpu_step(cpu->cpu_data);
}
static int cur_queue_pos = 0;
static void push_compiled_work(cpu_t* cpu, uint32_t pc){
	int i = 0;
	int cur_pos;
	cur_pos = cpu->dyncom_engine->cur_tagging_pos;
	/* check if the pc already exist in the queue */
	for(i = 0; i < cur_pos; i++)
		if(compiled_queue[i] == pc){
			//printf("pc 0x%x is also exist at %d\n", pc, cur_pos);
			return;
		}

	if(cur_pos >= 0 && cur_pos < QUEUE_LENGTH){
		cpu_tag(cpu, pc);
		if(mode == PURE_DYNCOM){
			cpu_translate(cpu, pc);
		}
		else{
			pthread_rwlock_wrlock(&compiled_queue_rwlock);
			//printf("In %s,place the pc=0x%x to the pos %d\n",__FUNCTION__, pc, cur_pos);
			compiled_queue[cur_pos] = pc;
			pthread_rwlock_unlock(&compiled_queue_rwlock);
		}
		cpu->dyncom_engine->cur_tagging_pos ++;
	}
	else{
		printf("In %s, compiled queue overflowed\n", __FUNCTION__);
	}
}
/* Compiled the target to the host address */
static int compiling_pos = 0;
static void* compiled_worker(void* argp){
	cpu_t* cpu = (cpu_t*)argp;
	//int cur_pos = cpu->dyncom_engine->functions;
	for(;;){
try_compile:
		while(1){
			pthread_rwlock_rdlock(&compiled_queue_rwlock);
			uint32_t compiled_addr = compiled_queue[compiling_pos];
			pthread_rwlock_unlock(&compiled_queue_rwlock);
			if(compiled_addr == 0xFFFFFFFF)
				break;
			//printf("In %s, pc=0x%x\n", __FUNCTION__, compiled_queue[compiling_pos]);
			/* Just for get basicblock */
			//cpu_tag(cpu, compiled_queue[cur_pos]);
			/* functions will increasement in translate procedure */
			pthread_rwlock_rdlock(&(cpu->dyncom_engine->rwlock));
			fast_map hash_map = cpu->dyncom_engine->fmap;
			void* pfunc = (void*)hash_map[compiled_addr & 0x1fffff];
			if(pthread_rwlock_unlock(&(cpu->dyncom_engine->rwlock))){
				fprintf(stderr, "In %s, unlock error\n", __FUNCTION__);
			}
			if(pfunc == NULL){
				cpu_translate(cpu, compiled_addr);
				//printf("In %s, finish translate addr 0x%x, functions=%d, compiling_pos=%d\n", __FUNCTION__, compiled_addr, cpu->dyncom_engine->functions, compiling_pos);
			}
			compiling_pos++;
			//cur_pos = cpu->dyncom_engine->functions;
		}
		usleep(1);
	}
}
