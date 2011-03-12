/*
 * The interface of dynamic compiled mode for ppc simulation
 *
 * 08/22/2010 Michael.Kang (blackfin.kang@gmail.com)
 */
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>


#include <llvm/LLVMContext.h>
#include <llvm/Type.h>
#include <llvm/Function.h>
#include <llvm/Module.h>
#include <llvm/Constant.h>

#include <skyeye_dyncom.h>
#include <skyeye_types.h>
#include <skyeye_obj.h>
#include <skyeye.h>
#include <dyncom/dyncom_llvm.h>
#include <skyeye_pref.h>
#include "dyncom/defines.h"

#include "ppc_cpu.h"
#include "ppc_mmu.h"
#include "ppc_dyncom.h"
#include "ppc_dyncom_run.h"
#include "dyncom/memory.h"
#include "bank_defs.h"
#include "skyeye_ram.h"
#include "ppc_dyncom_debug.h"

#include <pthread.h>
#include <sys/utsname.h>

using namespace std;

e500_core_t* get_core_from_dyncom_cpu(cpu_t* cpu){
	//e500_core_t* core = (e500_core_t*)get_cast_conf_obj(cpu->cpu_data, "e500_core_t");
	e500_core_t* core = (e500_core_t*)(cpu->cpu_data->obj);
	return core;
}

static uint32_t ppc_read_memory(cpu_t *cpu, addr_t addr, uint32_t size){
	uint32_t result;
	bus_read(size, addr, &result);
	return result;
}
static void ppc_write_memory(cpu_t *cpu, addr_t addr, uint32_t value, uint32_t size){
	bus_write(size, addr, value);
}

/* physical register for powerpc archtecture */
static void arch_powerpc_init(cpu_t *cpu, cpu_archinfo_t *info, cpu_archrf_t *rf)
{
	// Basic Information
	info->name = "powerpc";
	info->full_name = "powerpc_common";

	// This architecture is biendian, accept whatever the
	// client wants, override other flags.
	info->common_flags &= CPU_FLAG_ENDIAN_MASK;

	info->delay_slots = 0;
	// The byte size is 8bits.
	// The word size is 32bits.
	// The float size is 64bits.
	// The address size is 32bits.
	info->byte_size = 8;
	info->word_size = 32;
	info->float_size = 64;
	info->address_size = 32;
	// There are 16 32-bit GPRs
	info->register_count[CPU_REG_GPR] = PPC_DYNCOM_GPR_SIZE;
	info->register_size[CPU_REG_GPR] = info->word_size;
	info->register_count[CPU_REG_SPR] = PPC_DYNCOM_MAX_SPR_REGNUM;
	info->register_size[CPU_REG_SPR] = info->word_size;
	// There is also 1 extra register to handle PSR.
	//info->register_count[CPU_REG_XR] = PPC_XR_SIZE;
	info->register_count[CPU_REG_XR] = 0;
	info->register_size[CPU_REG_XR] = 32;
	info->register_count[CPU_REG_FPR] = 0;
	info->register_size[CPU_REG_FPR] = 64;
	cpu->redirection = false;
}

static void
arch_powerpc_done(cpu_t *cpu)
{
	//free(cpu->rf.grf);
}

static addr_t
arch_powerpc_get_pc(cpu_t *cpu, void *reg)
{
	//e500_core_t* core = (e500_core_t*)get_cast_conf_obj(cpu->cpu_data, "e500_core_t");
	e500_core_t* core = (e500_core_t*)(cpu->cpu_data->obj);

	return core->phys_pc;
	//return ((reg_powerpc_t *)reg)->pc;
}

static uint64_t
arch_powerpc_get_psr(cpu_t *, void *reg)
{
	return 0;
}

static int
arch_powerpc_get_reg(cpu_t *cpu, void *reg, unsigned reg_no, uint64_t *value)
{
	return (0);
}

static int arch_powerpc_disasm_instr(cpu_t *cpu, addr_t pc, char* line, unsigned int max_line){
	return 0;
}
static int arch_powerpc_translate_loop_helper(cpu_t *cpu, addr_t pc, BasicBlock *bb_ret, BasicBlock *bb_next, BasicBlock *bb, BasicBlock *bb_zol_cond){
	return 0;
}
static arch_func_t powerpc_arch_func = {
	arch_powerpc_init,
	arch_powerpc_done,
	arch_powerpc_get_pc,
	NULL,
	NULL,
	arch_powerpc_tag_instr,
	arch_powerpc_disasm_instr,
	arch_powerpc_translate_cond,
	arch_powerpc_translate_instr,
        arch_powerpc_translate_loop_helper,
	// idbg support
	arch_powerpc_get_psr,
	arch_powerpc_get_reg,
	NULL
};

int ppc_dyncom_start_debug_flag = 0;
/**
* @brief Place all the powerpc debug output here.
*
* @param cpu the instance of cpu_t
*/
static void ppc_debug_func(cpu_t* cpu){
	e500_core_t* core = (e500_core_t*)get_cast_conf_obj(cpu->cpu_data, "e500_core_t");
	if(ppc_dyncom_start_debug_flag){
		printf("In %s, phys_pc=0x%x\n", __FUNCTION__, *(addr_t*)cpu->rf.phys_pc);
		printf("gprs:\n ");
#define PRINT_COLUMN 8
		int i;
		for(i = 0; i < 32; i ++){
			printf("gpr[%d]=0x%x ", i, *(uint32_t*)((uint8_t*)cpu->rf.grf + 4*i));
			if((i % PRINT_COLUMN) == (PRINT_COLUMN - 1))
				printf("\n ");
		}
		printf("\nsprs:\n ");
		printf("PC = 0x%x\n ", core->phys_pc);
		printf("LR = 0x%x\n ", core->lr);
		printf("CR = 0x%x\n ", core->cr);
		printf("CTR= 0x%x\n ", core->ctr);
		printf("ICOUNT = %d\n", core->icount);
	}
	if(core->icount == START_DEBUG_ICOUNT){
		ppc_dyncom_start_debug_flag = 1;
		cpu_set_flags_debug(cpu, CPU_DEBUG_LOG);
	}
	core->icount ++;
#if 0
	extern void ppc_dyncom_diff_log(const unsigned int pc, const unsigned int lr, const unsigned int cr, const unsigned int ctr, const unsigned int reg[]);
	ppc_dyncom_diff_log(*(addr_t*)cpu->rf.phys_pc, ((uint32_t*)cpu->rf.srf)[LR_REGNUM], ((uint32_t*)cpu->rf.srf)[CR_REGNUM], ((uint32_t*)cpu->rf.srf)[CTR_REGNUM], core->gpr);
#endif
	return;
}
/**
 * @brief for llvm ir invoking
 *
 * @param cpu
 */
extern "C" int ppc_syscall(e500_core_t* core);
extern "C" bool_t ppc_exception(e500_core_t *core, uint32 type, uint32 flags, uint32 a);
static void ppc_dyncom_syscall(cpu_t* cpu, uint32_t num){
	e500_core_t* core = (e500_core_t*)get_cast_conf_obj(cpu->cpu_data, "e500_core_t");
	sky_pref_t* pref = get_skyeye_pref();
	if(pref->user_mode_sim)
		ppc_syscall(core);
	else
		ppc_exception(core, core->gpr[0], 0, 0);
}

void ppc_dyncom_init(e500_core_t* core){
	cpu_t* cpu = cpu_new(0, 0, powerpc_arch_func);
	cpu->dyncom_engine->code_start = 0x100000f4;
	cpu->dyncom_engine->code_entry = 0x10000140;
	cpu->dyncom_engine->code_end = 0x11000000;
	cpu->cpu_data = get_conf_obj_by_cast(core, "e500_core_t");
	/* Initilize different register set for different core */
	cpu->rf.pc = &core->pc;
	//cpu->rf.pc = &core->phys_pc;
	cpu->rf.phys_pc = &core->phys_pc;
	cpu->rf.grf = core->gpr;
	cpu->rf.srf = &core->cr;
	cpu_set_flags_codegen(cpu, 0
					| CPU_CODEGEN_TAG_LIMIT
					| CPU_CODEGEN_OPTIMIZE
				//	| CPU_CODEGEN_VERIFY
				);
	cpu_set_flags_debug(cpu, 0
				//	| CPU_DEBUG_PRINT_IR
				//	| CPU_DEBUG_LOG
					| CPU_DEBUG_PROFILE
                );
	/* set endian */
	cpu->info.common_flags |= CPU_FLAG_ENDIAN_BIG;
	cpu->info.psr_size = 0;
 
	cpu->debug_func = ppc_debug_func;
	sky_pref_t *pref = get_skyeye_pref();
	if(pref->user_mode_sim){
		extern void ppc_dyncom_syscall(cpu_t* cpu, uint32_t num);
		cpu->syscall_func = ppc_dyncom_syscall;
	}
	else
		cpu->syscall_func = NULL;
	core->dyncom_cpu = get_conf_obj_by_cast(cpu, "cpu_t");
#ifdef FAST_MEMORY
	cpu->dyncom_engine->RAM = (uint8_t*)get_dma_addr(0);
#else
	set_memory_operator(ppc_read_memory, ppc_write_memory);
#endif
	/* init thread clock for profile */
#if THREAD_CLOCK
	extern void *clock_thread(void*);
	pthread_t thread;
	int ret = pthread_create(&thread, NULL, clock_thread, NULL);
	if(ret){
		fprintf(stderr, "failed create timing thread\n");
		exit(0);
	}
#endif
	return;
}

void ppc_dyncom_run(cpu_t* cpu){
	//e500_core_t* core = (e500_core_t*)get_cast_conf_obj(cpu->cpu_data, "e500_core_t");
	e500_core_t* core = (e500_core_t*)(cpu->cpu_data->obj);
	addr_t phys_pc = *(addr_t*)cpu->rf.phys_pc;
	#if 0	
	if(ppc_effective_to_physical(core, *(addr_t*)cpu->rf.phys_pc, PPC_MMU_CODE, &phys_pc) != PPC_MMU_OK){
		/* we donot allow mmu exception in tagging state */
		fprintf(stderr, "In %s, can not translate the pc 0x%x\n", __FUNCTION__, core->pc);
		exit(-1);
	}
	#endif
	debug(DEBUG_RUN, "In %s,pc=0x%x,phys_pc=0x%x\n", __FUNCTION__, core->pc, phys_pc);
	core->phys_pc = phys_pc;

	int rc = cpu_run(cpu);
	switch (rc) {   
		case JIT_RETURN_NOERR: /* JIT code wants us to end execution */
			break;  
		case JIT_RETURN_SINGLESTEP:
		case JIT_RETURN_FUNCNOTFOUND:

			debug(DEBUG_RUN, "In %s, function not found at 0x%x\n", __FUNCTION__, core->phys_pc);
			cpu_tag(cpu, core->phys_pc);
		//	cpu->dyncom_engine->functions = 0;
			cpu_translate(cpu);
			/*
			**If singlestep,we run it here,otherwise,break.
			*/
			if (cpu->dyncom_engine->flags_debug & CPU_DEBUG_SINGLESTEP){
				rc = cpu_run(cpu);
				if(rc != JIT_RETURN_TRAP)
					break;
			}else
				break;
		case JIT_RETURN_TRAP:
#ifdef OPT_LOCAL_REGISTERS
			ppc_syscall(core);
			core->phys_pc += 4;
#endif
			break;
		default:
			fprintf(stderr, "unknown return code: %d\n", rc);
		skyeye_exit(-1);
	}// switch (rc)
}

////**profile**////
#define JIT_FUNCTION_PROFILE 1
struct ppc_dyncom_profile{
	unsigned int func_num;
	unsigned int average_func_size;
	unsigned int size_lt_10_func;
	unsigned int size_lt_50_func;
	unsigned int size_ge_50_func;
};
static void print_cpuinfo() {
	string line;
	std::ifstream finfo("/proc/cpuinfo");
	cout << "Machine:" << endl;
	while(getline(finfo,line)) {
		stringstream str(line);
		string itype;
		string info;
		getline( str, itype, ':' );
		getline(str,info);
		if (itype.substr(0,10) == "model name" ) {
			cout << info;
		}else if (itype.substr(0,10) == "cache size" ) {
			cout << ". Cache size:" << info << endl;
		}
	}
}
static void ppc_dyncom_profile(e500_core_t* core){
	cpu_t* cpu = (cpu_t*)get_cast_conf_obj(core->dyncom_cpu, "cpu_t");
	struct ppc_dyncom_profile profile;
	memset(&profile, 0, sizeof(struct ppc_dyncom_profile));
	printf("DFS : %d \n", LIMIT_TAGGING_DFS);
	printf("LLVM OPTIMIZE %s\n",
			cpu->dyncom_engine->flags_codegen & CPU_CODEGEN_OPTIMIZE ? "on" :"off");
	printf("Verify Function %s\n",
			cpu->dyncom_engine->flags_codegen & CPU_CODEGEN_VERIFY ? "on" :"off");
	profile.func_num = cpu->dyncom_engine->mod->size();
	Module::iterator it = cpu->dyncom_engine->mod->begin();
	unsigned int tmp_size = 0;
	unsigned int tmp_total_size = 0;
	for(; it != cpu->dyncom_engine->mod->end(); it++){
		Function* f = cast<Function>(it);
		tmp_size = f->size();
		if(tmp_size < 10)
			profile.size_lt_10_func ++;
		else if(tmp_size < 50)
			profile.size_lt_50_func ++;
		else
			profile.size_ge_50_func ++;
		tmp_total_size += tmp_size;
	}
	profile.average_func_size = tmp_total_size / profile.func_num;
	printf("JIT Fuctions in Module: %d\n",profile.func_num);
	printf("Average size of Function: %d\n",profile.average_func_size);
	printf("Function size (0 , 10): %d %f\n", profile.size_lt_10_func,
			((float)profile.size_lt_10_func)/((float)profile.func_num));
	printf("Function size [10, 50): %d %f\n", profile.size_lt_50_func,
			((float)profile.size_lt_50_func)/((float)profile.func_num));
	printf("Function size [50, +~): %d %f\n", profile.size_ge_50_func,
			((float)profile.size_ge_50_func)/((float)profile.func_num));
	printf("\n");
}
void ppc_dyncom_stop(e500_core_t* core){
	cpu_t* cpu = (cpu_t*)get_cast_conf_obj(core->dyncom_cpu, "cpu_t");
#if JIT_FUNCTION_PROFILE
	printf("PROFILING >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	print_cpuinfo();
	ppc_dyncom_profile(core);
#endif
#if ENABLE_ICOUNTER
	printf("Icounter: %lld\n", cpu->icounter);
#endif
	if (cpu->dyncom_engine->flags_debug & CPU_DEBUG_PROFILE)
		cpu_print_statistics(cpu);
}

void ppc_dyncom_fini(){
}
