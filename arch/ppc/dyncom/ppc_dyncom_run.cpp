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
#include <llvm/Constants.h>
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Instructions.h"
 
#include <skyeye_dyncom.h>
#include <skyeye_types.h>
#include <skyeye_obj.h>
#include <skyeye.h>
#include <dyncom/dyncom_llvm.h>
#include <skyeye_pref.h>
#include "dyncom/defines.h"
#include "dyncom/frontend.h"
 
#include "ppc_e500_core.h"
#include "ppc_cpu.h"
#include "ppc_dyncom.h"
#include "ppc_dyncom_run.h"
#include "bank_defs.h"
#include "skyeye_ram.h"
#include "ppc_dyncom_debug.h"
#include "ppc_dyncom_parallel.h"
extern "C" {
#include "ppc_mmu.h"
#include "ppc_exc.h"
};
#include <sys/utsname.h>

using namespace std;
/*
 * PowerPC dyncom callout function index
 * 0 --debug function
 * 1 --syscall for user mode simulation
 * */
enum{
	PPC_DYNCOM_CALLOUT_EXC = 2,
	PPC_DYNCOM_CALLOUT_MMU_SET_SDR1,
	PPC_DYNCOM_MAX_CALLOUT
};

e500_core_t* get_core_from_dyncom_cpu(cpu_t* cpu){
	//e500_core_t* core = (e500_core_t*)get_cast_conf_obj(cpu->cpu_data, "e500_core_t");
	e500_core_t* core = (e500_core_t*)(cpu->cpu_data->obj);
	return core;
}
/* FIXME: Only 4k page is supported. */
static bool_t is_inside_page(cpu_t *cpu, addr_t a)
{
	return ((a & 0xfffff000) == cpu->current_page_phys) ? True : False;
}
static bool_t is_page_start(addr_t a)
{
	return ((a & 0x00000fff) == 0x0) ? True : False;
}
static bool_t is_page_end(addr_t a)
{
	return ((a & 0x00000fff) == 0xffc) ? True : False;
}
static uint32_t ppc_read_memory(cpu_t *cpu, addr_t addr, uint32_t size){
	uint32_t result, ret;
	uint8_t result_8;
	uint16_t result_16;
	if(is_user_mode(cpu)){
		bus_read(size, addr, &result);
	}else{
		if(size == 8){
			ret = ppc_read_effective_byte(addr, &result_8);
			result = result_8;
		}
		else if(size == 16){
			ret = ppc_read_effective_half(addr, &result_16);
			result = result_16;
		}
		else if(size == 32)
			ret = ppc_read_effective_word(addr, &result);
		else
			fprintf(stderr, "in %s, ppc read memory error.\n", __func__);
	}
	return result;
}
static void ppc_write_memory(cpu_t *cpu, addr_t addr, uint32_t value, uint32_t size){
	if(is_user_mode(cpu)){
		bus_write(size, addr, value);
	}else{
		if(size == 8)
			ppc_write_effective_byte(addr, value);
		else if(size == 16)
			ppc_write_effective_half(addr, value);
		else if(size == 32)
			ppc_write_effective_word(addr, value);
		else
			fprintf(stderr, "in %s, ppc write memory error.\n", __func__);
	}
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
	if(is_user_mode(cpu))
		info->register_count[CPU_REG_SPR] = IBATU_REGNUM;
	else
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
	e500_core_t* core = (e500_core_t*)(cpu->cpu_data->obj);
	if(is_user_mode(cpu))
		return core->phys_pc;
	else
		return core->pc;
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
static int arch_powerpc_effective_to_physical(struct cpu *cpu, uint32_t addr, uint32_t *result){
	e500_core_t* core = (e500_core_t*)get_cast_conf_obj(cpu->cpu_data, "e500_core_t");
	if(is_user_mode(cpu)) {
		*result = addr;
		return 0;
	} else {
		/* only support PPC_MMU_CODE */
		int ret;
		ret = core->effective_to_physical(core, addr, PPC_MMU_CODE, result);
		if(core->pc == PPC_EXC_ISI)
			*result = PPC_EXC_ISI;
		return ret;
	}
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
	NULL,
};

/* Check address load/store instruction access.*/
static uint32_t ppc_check_mm(cpu_t *cpu, uint32_t instr){
	return 0;
}
static arch_mem_ops_t ppc_dyncom_mem_ops = {
	is_inside_page,
	is_page_start,
	is_page_end,
	ppc_read_memory,
	ppc_write_memory,
	ppc_check_mm,
	arch_powerpc_effective_to_physical
};

int ppc_dyncom_start_debug_flag = 0;
/**
* @brief Place all the powerpc debug output here.
*
* @param cpu the instance of cpu_t
*/
static uint32_t ppc_debug_func(cpu_t* cpu){
	e500_core_t* core = (e500_core_t*)get_cast_conf_obj(cpu->cpu_data, "e500_core_t");
	if(ppc_dyncom_start_debug_flag
			&& core->pir == DEBUG_CORE
			){
#if 1
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
		printf("PAGE_BASE(effec) = 0x%x\n ", cpu->current_page_effec);
		printf("PAGE_BASE(phys) = 0x%x\n ", cpu->current_page_phys);
		printf("EFFEC_PC = 0x%x\n ", core->pc);
		printf("PHYS_PC = 0x%x\n ", core->phys_pc);
		printf("LR = 0x%x\n ", core->lr);
		printf("CR = 0x%x\n ", core->cr);
		printf("CTR= 0x%x\n ", core->ctr);
		printf("XER= 0x%x\n ", core->xer);
		printf("MSR= 0x%x\n ", core->msr);
		printf("ICOUNT = %d\n ", core->icount);
		printf("HID[0] = %x\n ", core->hid[0]);
		printf("SRR[0] = %x\n ", core->srr[0]);
		printf("TBL = %x\n ", core->tbl);
		printf("TBU = %x\n ", core->tbu);
#else
		printf("Core %d, pc = 0x%x, icount = %d\n", core->pir, core->pc, core->icount);
#endif
	}
	if((core->icount == START_DEBUG_ICOUNT || core->phys_pc == START_DEBUG_PC)
			&& core->pir == DEBUG_CORE
			){
		ppc_dyncom_start_debug_flag = 1;
		cpu_set_flags_debug(cpu, CPU_DEBUG_LOG
//			| CPU_DEBUG_PRINT_IR
			);
	}
	if((core->icount == STOP_DEBUG_ICOUNT || core->phys_pc == STOP_DEBUG_PC)
			&& core->pir == DEBUG_CORE
			){
		ppc_dyncom_start_debug_flag = 0;
		cpu->dyncom_engine->flags_debug &= ~CPU_DEBUG_LOG;
	}
#if 0
#define START_DIFF 0 
extern void ppc_dyncom_diff_log(const unsigned long long icount,
		const unsigned int pc,
		const unsigned int gpr[],
		const unsigned int spr[]);
	if(core->icount > START_DIFF)
		ppc_dyncom_diff_log(core->icount, *(addr_t*)cpu->rf.phys_pc, core->gpr, &core->cr);
#endif
	core->icount ++;
	return 0;
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
	if(is_user_mode(cpu))
		ppc_syscall(core);
	else
		ppc_exception(core, core->gpr[0], 0, 0);
}
/**
 * llvm invoke ppc exception
 * bool_t (*ppc_exception)(struct e500_core_s *core, uint32 type, uint32 flags, uint32 a);
 */
void _ppc_dyncom_exception(cpu_t *cpu, bool cond, uint32 type, uint32 flags, uint32 a){
	e500_core_t* core = (e500_core_t*)get_cast_conf_obj(cpu->cpu_data, "e500_core_t");
	if (cond)
		ppc_exception(core, type, flags, a);
}
static void ppc_dyncom_exception_init(cpu_t *cpu){
	//types
	std::vector<const Type*> type_func_exception_args;
	PointerType *type_intptr = PointerType::get(cpu->dyncom_engine->exec_engine->getTargetData()->getIntPtrType(_CTX()), 0);
	const IntegerType *type_i32 = IntegerType::get(_CTX(), 32);
	const IntegerType *type_i1 = IntegerType::get(_CTX(), 1);
	type_func_exception_args.push_back(type_intptr);	/* intptr *cpu */
	type_func_exception_args.push_back(type_i32);	/* unsinged int */
	type_func_exception_args.push_back(type_i1);	/* cond */
	type_func_exception_args.push_back(type_i32);	/* unsinged int */
	type_func_exception_args.push_back(type_i32);	/* unsinged int */
	type_func_exception_args.push_back(type_i32);	/* unsinged int */
	FunctionType *type_func_exception_callout = FunctionType::get(
		Type::getVoidTy(cpu->dyncom_engine->mod->getContext()),	//return
		type_func_exception_args,	/* Params */
		false);		      	/* isVarArg */
	Constant *exception_const = cpu->dyncom_engine->mod->getOrInsertFunction("dyncom_callout4",	//function name
		type_func_exception_callout);	//return
	if(exception_const == NULL)
		fprintf(stderr, "Error:cannot insert function:exception.\n");
	Function *exception_func = cast<Function>(exception_const);
	exception_func->setCallingConv(CallingConv::C);
	cpu->dyncom_engine->ptr_arch_func[PPC_DYNCOM_CALLOUT_EXC] = exception_func;
	cpu->dyncom_engine->arch_func[PPC_DYNCOM_CALLOUT_EXC] = (void*)_ppc_dyncom_exception;
}
void
arch_ppc_dyncom_exception(cpu_t *cpu, BasicBlock *bb, Value *cond, uint32 type, uint32 flags, uint32 a)
{
	if (cpu->dyncom_engine->ptr_arch_func[PPC_DYNCOM_CALLOUT_EXC] == NULL)
		return;
	Type const *intptr_type = cpu->dyncom_engine->exec_engine->getTargetData()->getIntPtrType(_CTX());
	Constant *v_cpu = ConstantInt::get(intptr_type, (uintptr_t)cpu);
	Value *v_cpu_ptr = ConstantExpr::getIntToPtr(v_cpu, PointerType::getUnqual(intptr_type));
	std::vector<Value *> params;
	params.push_back(v_cpu_ptr);
	params.push_back(CONST(PPC_DYNCOM_CALLOUT_EXC));
	params.push_back(cond);
	params.push_back(CONST(type));
	params.push_back(CONST(flags));
	params.push_back(CONST(a));
	CallInst *ret = CallInst::Create(cpu->dyncom_engine->ptr_arch_func[PPC_DYNCOM_CALLOUT_EXC], params.begin(), params.end(), "", bb);
}

void _ppc_dyncom_mmu_set_sdr1(cpu_t *cpu, uint32_t rS){
	if (!ppc_mmu_set_sdr1(((uint32_t*)cpu->rf.grf)[rS], True)) {
		printf("cannot set sdr1\n");
	}
}
static void ppc_dyncom_mmu_set_sdr1_init(cpu_t *cpu){
	//types
	std::vector<const Type*> type_func_args;
	PointerType *type_intptr = PointerType::get(cpu->dyncom_engine->exec_engine->getTargetData()->getIntPtrType(_CTX()), 0);
	const IntegerType *type_i32 = IntegerType::get(_CTX(), 32);
	type_func_args.push_back(type_intptr);	/* intptr *cpu */
	type_func_args.push_back(type_i32);	/* unsinged int */
	type_func_args.push_back(type_i32);	/* unsinged int */
	FunctionType *type_func_callout = FunctionType::get(
		Type::getVoidTy(cpu->dyncom_engine->mod->getContext()),	//return
		type_func_args,	/* Params */
		false);		      	/* isVarArg */
	Constant *func_const = cpu->dyncom_engine->mod->getOrInsertFunction("dyncom_callout1",	//function name
		type_func_callout);	//return
	if(func_const == NULL)
		fprintf(stderr, "Error:cannot insert function:func.\n");
	Function *func = cast<Function>(func_const);
	func->setCallingConv(CallingConv::C);
	cpu->dyncom_engine->ptr_arch_func[PPC_DYNCOM_CALLOUT_MMU_SET_SDR1] = func;
	cpu->dyncom_engine->arch_func[PPC_DYNCOM_CALLOUT_MMU_SET_SDR1] = (void*)_ppc_dyncom_mmu_set_sdr1;
}
void
arch_ppc_dyncom_mmu_set_sdr1(cpu_t *cpu, BasicBlock *bb, uint32_t rS)
{
	if (cpu->dyncom_engine->ptr_arch_func[PPC_DYNCOM_CALLOUT_MMU_SET_SDR1] == NULL)
		return;
	Type const *intptr_type = cpu->dyncom_engine->exec_engine->getTargetData()->getIntPtrType(_CTX());
	Constant *v_cpu = ConstantInt::get(intptr_type, (uintptr_t)cpu);
	Value *v_cpu_ptr = ConstantExpr::getIntToPtr(v_cpu, PointerType::getUnqual(intptr_type));
	std::vector<Value *> params;
	params.push_back(v_cpu_ptr);
	params.push_back(CONST(PPC_DYNCOM_CALLOUT_MMU_SET_SDR1));
	params.push_back(CONST(rS));
	CallInst *ret = CallInst::Create(cpu->dyncom_engine->ptr_arch_func[PPC_DYNCOM_CALLOUT_MMU_SET_SDR1], params.begin(), params.end(), "", bb);
}
void ppc_switch_mode(cpu_t *cpu)
{
	return;
}

void ppc_dyncom_init(e500_core_t* core){
	cpu_t* cpu = cpu_new(0, 0, powerpc_arch_func);
	/* set user mode or not */
	sky_pref_t *pref = get_skyeye_pref();
	if(pref->user_mode_sim)
		cpu->is_user_mode = 1;
	else
		cpu->is_user_mode = 0;
	if(is_user_mode(cpu)){
		cpu->dyncom_engine->code_start = 0x100000f4;
		cpu->dyncom_engine->code_entry = 0x10000140;
		cpu->dyncom_engine->code_end = 0x11000000;
	}else{
		cpu->dyncom_engine->code_start = 0x00000000;
		cpu->dyncom_engine->code_entry = 0x00000000;
		/* should set code_end to the end of physical
		 * memory skyeye simulated */
		cpu->dyncom_engine->code_end = 0x20000000;
	}
	cpu->mem_ops = ppc_dyncom_mem_ops;
	cpu->switch_mode = ppc_switch_mode;
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
	ppc_dyncom_exception_init(cpu);
	ppc_dyncom_mmu_set_sdr1_init(cpu);
	if(is_user_mode(cpu)){
		extern void ppc_dyncom_syscall(cpu_t* cpu, uint32_t num);
		cpu->syscall_func = ppc_dyncom_syscall;
	}
	else
		cpu->syscall_func = NULL;
	core->dyncom_cpu = get_conf_obj_by_cast(cpu, "cpu_t");
#ifdef FAST_MEMORY
	cpu->dyncom_engine->RAM = (uint8_t*)get_dma_addr(0);
#endif
	init_compiled_queue(cpu);
	return;
}
static void flush_current_page(cpu_t *cpu){
	e500_core_t* core = (e500_core_t*)(cpu->cpu_data->obj);
	addr_t effec_pc = *(addr_t*)cpu->rf.pc;
	ppc_effective_to_physical(core, effec_pc, PPC_MMU_CODE, (uint32_t*)cpu->rf.phys_pc);
	cpu->current_page_phys = core->phys_pc & 0xfffff000;
	cpu->current_page_effec = core->pc & 0xfffff000;
}

void ppc_dyncom_run(cpu_t* cpu){
	e500_core_t* core = (e500_core_t*)(cpu->cpu_data->obj);
	debug(DEBUG_RUN, "In %s,pc=0x%x,phys_pc=0x%x\n", __FUNCTION__, core->pc, core->phys_pc);
	int rc = cpu_run(cpu);
	switch (rc) {   
		case JIT_RETURN_NOERR: /* JIT code wants us to end execution */
		case JIT_RETURN_TIMEOUT:
			break;  
		case JIT_RETURN_SINGLESTEP:
		case JIT_RETURN_FUNCNOTFOUND:
			debug(DEBUG_RUN, "In %s, function not found at pc=0x%x, phys_pc=0x%x\n",
					__FUNCTION__, core->pc, core->phys_pc);
			/* flush the current page before translate the next instruction.
			 * When running here, return from JIT Function, ONLY effective pc is
			 * valid, we need to update physics pc and page bases. Tagging and
			 * translate need them. */
			if(!is_user_mode(cpu))
				flush_current_page(cpu);
			cpu_tag(cpu, core->phys_pc);
			cpu->dyncom_engine->cur_tagging_pos ++;
			cpu_translate(cpu, core->phys_pc);
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
