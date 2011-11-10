/*
 * The interface of dynamic compiled mode for ppc simulation
 *
 * 08/22/2010 Michael.Kang (blackfin.kang@gmail.com)
 */

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
#include <bank_defs.h>
#include <skyeye_pref.h>
#include <skyeye_symbol.h>
#include <dyncom/dyncom_llvm.h>
#include <skyeye_log.h>
#include <dyncom/tag.h>

#include <vector>

#include "arm_regformat.h"
#include <skyeye_ram.h>

#include "armdefs.h"
#include "memory.h"
#include "dyncom/memory.h"
#include "dyncom/frontend.h"
#include "arm_dyncom_translate.h"
#include "arm_dyncom_parallel.h"
#include "dyncom/defines.h"
#include "common/mmu/arm1176jzf_s_mmu.h"
#include "armmmu.h"

#include "dyncom/arm_dyncom_mmu.h"

#define LOG_IN_CLR	skyeye_printf_in_color

void arm_switch_mode(cpu_t *cpu);
//#define MAX_REGNUM 16
extern const char* arm_regstr[MAX_REG_NUM];

enum{
	ARM_DYNCOM_CALLOUT_UNDEF = 2,
	ARM_DYNCOM_MAX_CALLOUT
};

extern "C" {
extern void io_do_cycle (void * state);
}

uint32_t get_end_of_page(uint32 phys_addr){
	const uint32 page_size = 4 * 1024;
	return (phys_addr + page_size) & (~(page_size - 1));
}

void cpu_set_flags_codegen(cpu_t *cpu, uint32_t f)
{
        cpu->dyncom_engine->flags_codegen = f;
}
static uint32_t int_icounter[] = {1953097, 2082332, 2211567, 2340802, 2470037, 2599272, 2728507, 2857742, 2986977, 3116212, 
				3245447, 3374682, 3503917, 3633152, 3762387, 3891622, 4020857, 4027664, 4033277, 4063047,
				4063047, 4149547, 4150163, 4157113, 4279332, 4408567, 4537802, 4667037, 4796272, 4925507,
				5054755, 5183977, 5313448, 5442447, 5571682, 5622630, 5683942, 5691077, 5698212, 5705347,
				5707073, 5718725, 5725860, 5732995, 5740130, 5744864, 5781941, 5789076, 5796320, 5803564};

static int flush_current_page(cpu_t *cpu);
static int last_idx = 0;

#if __FOLLOW_MODE__
static uint32_t arm_get_id_from_string(char *reg_name, bool print_regname)
{
	if (print_regname) {
	//	printf("in %s\n", __FUNCTION__);
		printf("reg_name is %s\n", reg_name);
	}
        int i = 0;
        for (i = 0; i < MAX_REG_NUM; i ++) {
                if (0 == strcmp(arm_regstr[i], reg_name))
                        return i;
        }
}

void update_int_array(cpu_t *cpu, uint32_t icounter)
{
	if (icounter > int_icounter[last_idx]) {
//		last_idx ++;
		int_icounter[last_idx] = icounter;
	}
}

#define PRINT_LOG 0
uint32_t follow_mode(cpu_t *cpu)
{
        static uint32_t wait_one_step = 1;
	static int adjust_pc = 0;
        char reg_name[20];
        static char string[100] = {0};
        int i = 0;
        uint32_t val = 0;
        uint32_t idx = 0;
        static uint32_t last_pc = 0;
        bool sw = true;
	static bool print_regname = false;
        arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	if (adjust_pc) {
		adjust_pc = 0;
		goto begin;
	}
        if (wait_one_step == 0) {
                wait_one_step = 1;
                last_pc = cpu->f.get_pc(cpu, cpu->rf.grf);
                return 0;
        }
        if (string[0] == 0) {
                fscanf(cpu->src_log, "%s", string);
        }
#if PRINT_LOG
        printf("%s\n", string);
#endif
        if (string[0] != 'P' && string[1] != 'C') {
                printf("log format is not wrong!\n");
#if PRINT_LOG
                printf("%s\n", string);
#endif
                exit(-1);
        }
        val = strtol(&string[3], NULL, 16);
        if (val == 0) {
                fscanf(cpu->src_log, "%s", string);
                val = strtol(&string[3], NULL, 16);
//                return;
        }
        last_pc = cpu->f.get_pc(cpu, cpu->rf.grf);
        if (val != last_pc) {
		if (val == 0xffff0018) {
			cpu->check_int_flag = 1;
			update_int_array(cpu, cpu->icounter);
			adjust_pc = 1;
			return 1;
		} else if (last_pc == 0xffff000c) {
			do {
				fscanf(cpu->src_log, "%s", string);
			} while (string[0] != 'P' && string[1] != 'C');
			val = strtol(&string[3], NULL, 16);
			if (val != last_pc) {
				printf("try again, but pc is still wrong.\n");
				exit(-1);
			}
		} else {
	//                printf("pc is wrong.\n");
			LOG_IN_CLR(RED, "pc is wrong\n");
	//                printf("dyncom mode pc is %x\n", core->Reg[15]);
			LOG_IN_CLR(BLUE, "dyncom mode pc is %x\n", core->Reg[15]);
			LOG_IN_CLR(CYAN, "dyncom mode phys_pc is %x\n", core->phys_pc);
			LOG_IN_CLR(LIGHT_RED, "interpreter mode is %x\n", val);
			LOG_IN_CLR(PURPLE, "icounter is %d\n", cpu->icounter);
			LOG_IN_CLR(RED, "adjust pc...\n");
//		core->Reg[15] = val;
//		flush_current_page(cpu);
//		return 1;
			exit(-1);
		}
        }
begin:
        fscanf(cpu->src_log, "%s", string);
#if PRINT_LOG
        printf("%s\n", string);
#endif
//	if (cpu->icounter >= 163630) {
//		print_regname = true;
//	}
        while(string[0] != 'P' && string[1] != 'C') {
                while(string[i] != ':') i++;
                string[i] = '\0';
                idx = arm_get_id_from_string(string, print_regname);
#if PRINT_LOG
                printf("idx is %d\n", idx);
#endif
                string[i] = ':';
                val = strtol(&string[i + 1], NULL, 16);
#if PRINT_LOG
                printf("val is %x\n", val);
#endif
                if (idx < 15 && core->Reg[idx] != val) { //&& ((val & 0xf) != 3)) {
			if (core->Reg[15] == 0xc0020ab0 && cpu->icounter > 237538020) {
				core->Reg[idx] = val;
			}
                        if (sw) {
                                printf("addr : %x\n", last_pc);
                                sw = false;
                        }
			if (idx != 16) {
				printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
			}
                        printf("R%d's value is wrong.\n", idx);
                        printf("R%d wrong value : %x right value: %x\n", idx, core->Reg[idx], val);
                        if(idx != 15) {
                                fprintf(cpu->fm_log, "%x instr implementation is wrong\n", last_pc);
                        }
                        //core->Reg[idx] = val;
			printf("icounter is %lld\n", cpu->icounter);
                }
		i = 0;
                fscanf(cpu->src_log, "%s", string);
#if PRINT_LOG
                printf("%s\n", string);
#endif
        }
	return 0;
}
#endif
struct symbolInfo *symbol_info_head = NULL;

void add_list_tail(struct symbolInfo *list)
{
        static struct symbolInfo *symbol_info_tail = NULL;
        if(!symbol_info_head) {
                symbol_info_head = symbol_info_tail = list;
                return;
        }
        symbol_info_tail->next = list;
        symbol_info_tail = list;
}

uint8_t *store_string_info(char *info, size_t size)
{
        static uint32_t max_vol = 0x1000;
        static uint32_t offset = 0;
        static uint32_t remain = 0x1000;
        static uint8_t *repo = NULL;

        uint8_t *str = NULL;
        uint8_t *new_repo = NULL;
        struct symbolInfo *item = NULL;

        //printf("%s, %d, %d\n", info, size, remain);
        if (repo == NULL) {
                repo = (uint8_t *)malloc(max_vol);
                printf("allocate %d bytes.\n", max_vol);
        }
        if (remain < size) {
                new_repo = (uint8_t *)malloc(max_vol * 2);
                printf("allocate %d bytes.\n", max_vol * 2);
                memcpy(new_repo, repo, offset);
                for (item = symbol_info_head; item; item = item->next) {
                        //printf("symbol : %s\taddress : %x\n", item->name, item->address);
                        item->name = new_repo + ((uint8_t *)item->name - (uint8_t *)repo);
                }
                free(repo);
                repo = new_repo;
                new_repo = NULL;
                remain += max_vol;
                max_vol *= 2;
        }
        str = repo + offset;
        memcpy(repo + offset, info, size);
        repo[offset + size] = '\0';
        offset += size;
        remain -= size;
        return str;
}
struct symbolInfo *alloc_symbol_info(uint8_t *str, uint32_t address)
{
        struct symbolInfo *item = (struct symbolInfo *)malloc(sizeof(struct symbolInfo));
        if (item == NULL) {
                printf("Can't allocate more memory in %s\n", __FUNCTION__);
                exit(-1);
        }
        item->next = NULL;
        item->name = str;
        item->address = address;
        return item;
}

struct symbolInfo *search_symbol_info_by_addr(uint32_t address)
{
        struct symbolInfo *prev = NULL, *item = NULL;
        for (item = symbol_info_head; item; item = item->next) {
                if(address == item->address) {
                        return item;
                } else if(address > item->address){
                        prev = item;
                        continue;
                } else {
                        return prev;
                }
        }
        printf("Can not found the address 0x%x in System.map.\n", address);
        //exit(-1);
        return NULL;
}

void print_func_name(uint32_t address)
{
        static struct symbolInfo *last_found = NULL;
        static uint32_t last_address = 0;
        struct symbolInfo *new_found = NULL;
        new_found = search_symbol_info_by_addr(address);
        if (new_found == NULL) {
                return;
        }
        if (last_found != new_found) {
                if (last_found) {
                        LOG_IN_CLR(LIGHT_RED, "exit function %s 0x%x\n", last_found->name, last_address);
                }
                printf("%s\n", new_found->name);
                last_found = new_found;
                last_address = address;
        } else {
		last_address = address;
	}
}

void load_symbol_from_sysmap()
{
        char symbol_address[100];
        char symbol_name[100];
        char type = 0;
        char *str = NULL;
        struct symbolInfo *item = NULL;
        int i = 0;

        uint32_t address = 0;
        FILE *sysmap = fopen("/home/myesis/linux-2.6.35.y/System.map", "r");

        do {
                    if (3 != fscanf(sysmap, "%s %c %s", symbol_address, &type, symbol_name)) break;
                    address = strtol(symbol_address, NULL, 16);
                    while (symbol_name[i] != '\0') {
                            //printf("%c\n", symbol_name[i]);
                            i++;
                    }
                    //printf("symbol:%s\taddress:%x\tsize:%d\n", symbol_name, address, i);
                    str = (char *)store_string_info(symbol_name, i + 1);
                    item = alloc_symbol_info((uint8_t *)str, address);
                    add_list_tail(item);
        } while (1);
        for (item = symbol_info_head; item; item = item->next) {
                printf("symbol : %s\taddress : %x\n", item->name, item->address);
        }
}


uint32_t is_int_in_interpret(cpu_t *cpu)
{
	static int hit = 0;
	int curr_idx = last_idx;
	int length = sizeof(int_icounter) / sizeof(uint32_t);
	for (; curr_idx < length; curr_idx ++) {
		if (cpu->icounter < int_icounter[curr_idx]) {
			return 0;
		}
		if (int_icounter[curr_idx] == cpu->icounter) {
			last_idx = curr_idx;
			return 1;
		}
	}
	if (last_idx == length) {
		last_idx --;
	}
	return 0;
}

#ifdef TIMER_PROFILE
static cpu_t* gcpu;
#include <signal.h>
#include "skyeye_thread.h"
void printinfo(int signum)
{
	cpu_print_statistics(gcpu);
}
#endif

static cpu_flags_layout_t arm_flags_layout[4] ={{3,'N',"NFLAG"},{2,'Z',"ZFLAG"},{1,'C',"CFLAG"},{0,'V',"VFLAG"}} ;
/* physical register for arm archtecture */
static void arch_arm_init(cpu_t *cpu, cpu_archinfo_t *info, cpu_archrf_t *rf)
{
	arm_opc_func_init();
	// Basic Information
	info->name = "arm"; info->full_name = "arm_dyncom";

	// This architecture is biendian, accept whatever the
	// client wants, override other flags.
	info->common_flags &= CPU_FLAG_ENDIAN_MASK;
	/* set the flag of save pc */
	cpu->info.common_flags |= CPU_FLAG_SAVE_PC;

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
	info->register_count[CPU_REG_GPR] = 19;
	info->register_size[CPU_REG_GPR] = info->word_size;
	// There is also 1 extra register to handle PSR.
	//info->register_count[CPU_REG_XR] = PPC_XR_SIZE;
	info->register_count[CPU_REG_XR] = MAX_REG_NUM - 19;
	//info->register_count[CPU_REG_XR] = 0;
	info->register_size[CPU_REG_XR] = 32;
	//info->register_count[CPU_REG_SPR] = MAX_REG_NUM - PHYS_PC;
	info->register_count[CPU_REG_SPR] = 0;
	info->register_size[CPU_REG_SPR] = 32;
	info->psr_size = 32;
	info->flags_count = 4;
	info->flags_layout = arm_flags_layout;
	/* Indicate the pc index for OPT_LOCAL_REGISTERS */
	info->pc_index_in_gpr = 15;

	cpu->redirection = false;
	
#ifdef TIMER_PROFILE
	gcpu = cpu;
	signal(SIGUSR1, printinfo);
	extern void *clock_thread(void*);
	pthread_t thread;
	int ret = pthread_create(&thread, NULL, clock_thread, NULL);
#endif

	//debug
	cpu_set_flags_debug(cpu, 0
	//	| CPU_DEBUG_PRINT_IR
	//	| CPU_DEBUG_LOG
	//	| CPU_DEBUG_PROFILE
               );
        cpu_set_flags_codegen(cpu, CPU_CODEGEN_TAG_LIMIT 
				| CPU_CODEGEN_OPTIMIZE
				| CPU_CODEGEN_VERIFY
			      );
	/* Initilize different register set for different core */

//	set_memory_operator(arch_arm_read_memory, arch_arm_write_memory);
//	arm_dyncom_mcr_init(cpu);
}

static void
arch_arm_done(cpu_t *cpu)
{
	//free(cpu->rf.grf);
}

static addr_t
arch_arm_get_pc(cpu_t *, void *reg)
{
	unsigned int *grf =(unsigned int *) reg;
	return grf[15];
}

static uint64_t
arch_arm_get_psr(cpu_t *, void *reg)
{
	return 0;
}

static int
arch_arm_get_reg(cpu_t *cpu, void *reg, unsigned reg_no, uint64_t *value)
{
	return (0);
}

static int arch_arm_disasm_instr(cpu_t *cpu, addr_t pc, char* line, unsigned int max_line){
	return 0;
}
static int arch_arm_translate_loop_helper(cpu_t *cpu, addr_t pc, BasicBlock *bb_ret, BasicBlock *bb_next, BasicBlock *bb, BasicBlock *bb_zol_cond){
	return 0;
}

static void arch_arm_emit_decode_reg(cpu_t *cpu, BasicBlock *bb)
{
	Value *nzcv = LSHR(AND(LOAD(cpu->ptr_gpr[16]), CONST(0xf0000000)), CONST(28));
	Value *n = TRUNC1(AND(LSHR(nzcv, CONST(3)), CONST(1)));
	Value *z = TRUNC1(AND(LSHR(nzcv, CONST(2)), CONST(1)));
	Value *c = TRUNC1(AND(LSHR(nzcv, CONST(1)), CONST(1)));
	Value *v = TRUNC1(AND(LSHR(nzcv, CONST(0)), CONST(1)));
	new StoreInst(n, cpu->ptr_N, false, bb);
	new StoreInst(z, cpu->ptr_Z, false, bb);
	new StoreInst(c, cpu->ptr_C, false, bb);
	new StoreInst(v, cpu->ptr_V, false, bb);
}

static void arch_arm_spill_reg_state(cpu_t *cpu, BasicBlock *bb)
{
		/* Save N Z C V */
	Value *z = SHL(ZEXT32(LOAD(cpu->ptr_Z)), CONST(30));
	Value *n = SHL(ZEXT32(LOAD(cpu->ptr_N)), CONST(31));
	Value *c = SHL(ZEXT32(LOAD(cpu->ptr_C)), CONST(29));
	Value *v = SHL(ZEXT32(LOAD(cpu->ptr_V)), CONST(28));
	Value *nzcv = OR(OR(OR(z, n), c), v);
	Value *cpsr = OR(AND(LOAD(cpu->ptr_gpr[16]), CONST(0xfffffff)), nzcv);
	new StoreInst(cpsr, cpu->ptr_gpr[16], false, bb);
}

static arch_func_t arm_arch_func = {
	arch_arm_init,
	arch_arm_done,
	arch_arm_get_pc,
	arch_arm_emit_decode_reg,
	arch_arm_spill_reg_state,
	arch_arm_tag_instr,
	arch_arm_disasm_instr,
	arch_arm_translate_cond,
	arch_arm_translate_instr,
        arch_arm_translate_loop_helper,
	// idbg support
	arch_arm_get_psr,
	arch_arm_get_reg,
	NULL
};

static uint32_t arm_debug_func(cpu_t* cpu){
	int idx = 0;
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
#if 0	
	for (idx = 0;idx < 16; idx ++) {
		LOG_IN_CLR(RED, "R%d:0x%x\t", idx, core->Reg[idx]);
	}
	printf("\n");

	if (cpu->icounter > 248306790) {
		if ((core->Reg[15] & 0xf0000000) == 0x50000000) {
			print_func_name(core->Reg[15] + 0x70000000);
		} else
			print_func_name(core->Reg[15]);
	}
#endif
	//printf("run at %x\n", core->Reg[15]);
#if 0
	if (cpu->icounter == 170965) {
		printf("at 790000\n");
		exit(-1);
	}
#endif
#if 0
//	if (cpu->icounter == 687418 || cpu->icounter == 687417 || cpu->icounter == 687416) {
//	if (cpu->icounter >= 0) {
	if (cpu->icounter > 254940700) {
// 	if (cpu->icounter > 1694550) {
//	if (cpu->icounter > 1287900) {
//	if (cpu->icounter > 1696000) {
//	if (cpu->icounter > 779800) {
		printf("icounter is %lld\n", cpu->icounter);
		for (idx = 0;idx < 16; idx ++) {
			LOG_IN_CLR(RED, "R%d:0x%x\t", idx, core->Reg[idx]);
		}
		LOG_IN_CLR(BLUE, "CPSR:0x%x\n", core->Cpsr);
		LOG_IN_CLR(LIGHT_BLUE, "SPSR:0x%x\n", core->Spsr_copy);
		printf("int is %d\n", core->NirqSig);
//		printf("\n");
//		printf("phys base addr is %x\n", cpu->current_page_phys);
//		printf("effec base addr is %x\n", cpu->current_page_effec);
	}
	if (cpu->icounter == 1696000) {
//		exit(1);
	}
	#if 0
	if (core->Reg[15] == 0xc0122570) {
		printf("hit it\n");
	}
	#endif
	#if 0
	if (core->Reg[15] == 0x500083b0) {
		for (idx = 0;idx < 16; idx ++) {
			printf("R%d:0x%x\t", idx, core->Reg[idx]);
		}
		printf("\n");
	}
	#endif
#endif

#if 0
#if DIFF_LOG
#if SAVE_LOG
	fprintf(core->state_log, "PC:0x%x\n", cpu->f.get_pc(cpu, cpu->rf.grf));
	for (idx = 0;idx < 16; idx ++) {
		fprintf(core->state_log, "R%d:0x%x\n", idx, core->Reg[idx]);
	}
#else
	uint32_t val;
	fscanf(core->state_log, "PC:0x%x\n", &val);
        uint32_t pc = cpu->f.get_pc(cpu, cpu->rf.grf);
        if (val != pc) {
                printf("pc is wrong.\n");
                printf("dyncom mode pc is %x\n", pc);
                printf("adu mode is %x\n", val);
		printf("icounter is %x\n", cpu->icounter);
                exit(-1);
        }
	uint32_t dummy;
	bool flags = 0;
	for (idx = 0; idx < 16; idx ++) {
		fscanf(core->state_log, "R%d:0x%x\n", &dummy, &val);
		//printf("R%d:0x%x\n", dummy, val);
		if (dummy == idx) {
			if (core->Reg[idx] != val) {
				printf("dummy is %d R%d : \t[R]%x \t[W]%x\n", dummy, idx, val, core->Reg[idx]);
				flags = 1;
				//core->Reg[idx] = val;
			}
		} else {
			printf("wrong dummy\n");
			exit(-1);
		}
	}
	if (flags) {
		printf("pc is %x\n", pc);
		printf("icounter is %x\n", cpu->icounter);
		flags = 0;
		exit(-1);
	}
#endif
#endif
#endif
#if 0
#if __FOLLOW_MODE__
#if SYNC_WITH_INTERPRET
	if (is_int_in_interpret(cpu)) {
		cpu->check_int_flag = 1;
		return 1;
	}
	return follow_mode(cpu);
#endif
#else
	return 0;
#endif
#endif

#define BOT_LOG 0
#define TOP_LOG 0xffff0000
#if 0
#if DIFF_LOG
#if SAVE_LOG

const char* arm_instr[] = {
"adc", "add","and","b,bl","bic","bkpt","blx(1)","blx(2)","bx","bxj","cdp","clrex","clz","cmn","cmp","cps","cpy","eor","ldc","ldm(1)","ldm(2)","ldm(3)","sxth","ldr","uxth","uxtah","ldrb","ldrbt","ldrd","ldrex","ldrexb","ldrh","ldrsb","ldrsh","ldrt","mcr","mcrr","mla","mov","mrc","mrrc","mrs","msr","msr","mul","mvn","orr","pkhbt","pkhtb","pld","qadd","qadd16","qadd8","qaddsubx","qdadd","qdsub","qsub","qsub16","qsub8","qsubaddx","rev","revsh","rfe","rsb","rsc","sadd16","sadd8","saddsubx","sbc","sel","setend","shadd16","shadd8","shaddsubx","shsub16","shsub8","shsubaddx","smla<x><y>","smlad","smlal","smlal<x><y>","smlald","smlaw<y>","smlsd","smlsld","smmla","smmls","smmul","smuad","smul<x><y>","smull","smulw<y>","smusd","srs","ssat","ssat16","ssub16","ssub8","ssubaddx","stc","stm(1)","stm(2)","sxtb","str","uxtb","uxtab","strb","strbt","strd","strex",	"strexb","strh","strt","sub","swi","swp","swpb","sxtab","sxtab16","sxtah","sxtb16","teq","tst","uadd16","uadd8","uaddsubx","uhadd16","uhadd8","uhaddsubx","uhsub16","uhsub8","uhsubaddx","umaal","umlal","umull","uqadd16","uqadd8","uqaddsubx","uqsub16","uqsub8","uqsubaddx","usad8","usada8","usat","usat16","usub16","usub8","usubaddx","uxtab16","uxtb16"
};
	
	static int32_t hash_to_instr [0x150000];
	static uint32_t instr_count [150];
	static uint32_t icounter = 0;

	static uint32_t instr_flag = 1;
	if (instr_flag)
	{
		printf("Reset instr table\n");
		for (instr_flag = 0; instr_flag < 0x150000; instr_flag++)
			hash_to_instr[instr_flag] = -1;
		for (instr_flag = 0; instr_flag < 0x150; instr_flag++)
			instr_count[instr_flag] = 0;
		instr_flag = 0;
	}
	uint32_t pc = cpu->f.get_pc(cpu, cpu->rf.grf);
	uint32_t instr = *((uint32_t *) pc);
	if (hash_to_instr[pc] == -1)
	{
		decode_arm_instr(instr, &idx);
		hash_to_instr[pc] = idx;
	}
	else
	{
		instr_count[hash_to_instr[pc]] += 1;
	}
	
	icounter++;
	
	if ( (icounter%0x100000) == 0)
	{
		setbuf(core->state_log, NULL);
		printf("Instr table updated %x\n", icounter);
		for(idx = 0; idx < 150; idx++)
		{
			fprintf( core->state_log, "%s\t%d\t% 12ud\n", arm_instr[idx], idx, instr_count[idx]);
			//printf("%d\t% 12ud\n", idx, instr_count[idx]);
		}
	}
#endif
#endif
#endif

#if 0
	static int flagged = 0;
	static uint32_t bot_log = -1;
	static uint32_t top_log = -1;
	if (bot_log == -1)
	{
		sky_pref_t* pref = get_skyeye_pref();
		bot_log = pref->bot_log;
		top_log = pref->top_log;
	}

#if 0
	uint32_t f6 = 0;
	if (bus_read(32, 0x71200000, &f6) != -1)
	{
		if (f6 != 0)
		{
			//asm("int $3");
			printf("#### f6 is %x at %x\n", f6, core->NumInstrs);
		}
	}
#endif
	
#if 1
	//if (cpu->f.get_pc(cpu, cpu->rf.grf) == 0xc0009788)
	if ((cpu->f.get_pc(cpu, cpu->rf.grf) == 0xc0009810) && (core->Reg[5] == 0x481))
	{
		if (!(get_skyeye_pref()->start_logging))
			core->NumInstrs = 0xf0000000;
		printf("JUMPED!\n");
		//get_skyeye_pref()->start_logging = 1;
	}
#endif
		
	//if (core->NumInstrs < 0x10000)
	if ((flagged == 0) && (top_log > 0)) {
		if (
		    
		    //(cpu->f.get_pc(cpu, cpu->rf.grf) == 0xc00255a0) ||
		    (core->NumInstrs > bot_log) ||
		    0
		    )
		{
			printf("##### Hit %x at %x\n", cpu->f.get_pc(cpu, cpu->rf.grf), core->NumInstrs);
			get_skyeye_pref()->start_logging = 1;
			//core->NumInstrs = bot_log;
		}
	}
	
#if 0
	if ( ((flagged == 0) && 
	    (top_log > 0) && 
	    (core->NumInstrs > bot_log) &&
	    1) {
		flagged = 1;
		//core->NumInstrs = bot_log;
	}
#endif

#if 0
	if (cpu->f.get_pc(cpu, cpu->rf.grf) == 0x8210)
	{
		uint32_t dummy = 0;
		uint32_t pa = 0;
		core->mmu.ops.load_instr(core,cpu->f.get_pc(cpu, cpu->rf.grf),&dummy,&pa);
		uint32_t dummy_tag = get_tag(cpu, pa);
		printf("~~~~ instruction at %x-%x is %x and get tagged %x\n", cpu->f.get_pc(cpu, cpu->rf.grf), pa, dummy, dummy_tag);
	}
#endif
	if ((flagged == 0) && (get_skyeye_pref()->start_logging == 1))
		flagged = 1;

	if (flagged)
	{
		if (core->NumInstrs % 0x100 == 0)
			printf("||||| %x |||||\n", core->NumInstrs);
		
		//if (cpu->f.get_pc(cpu, cpu->rf.grf) == 0xc0009810)
		{
		printf("---|%p|---\n", cpu->f.get_pc(cpu, cpu->rf.grf));
		
		for (idx = 0;idx < 16; idx ++) {
			printf("R%02d % 8x\n", idx, core->Reg[idx]);
		}
		printf("CPS %08x\n", core->Cpsr);
		}
		
		if ( (top_log > 0) && (core->NumInstrs > top_log))
		{
			printf("Exiting\n");
			sleep(1);
			exit(-1);
		}
	}
	core->NumInstrs++;
	
#endif

#if 0
#if DIFF_LOG
#if SAVE_LOG
	static uint32_t icounter = 0; //cpu->icounter;
	icounter++;
	/*if ( (icounter % 0x1000000) == 0)
		printf("COUNTING: %x\n", icounter);
	return;*/

	if ( (cpu->icounter >= BOT_LOG) && ((cpu->icounter <= TOP_LOG) || (TOP_LOG <= BOT_LOG) ))
	{
		//fprintf(core->state_log, "---|%05x|---IN: \n", cpu->icounter);
		
		
		fprintf(core->state_log, "---|%08x|---PC:%08x\n", icounter, cpu->f.get_pc(cpu, cpu->rf.grf));
		for (idx = 0;idx < 16; idx ++) {
			//idx = 13;
			fprintf(core->state_log, "R%02d:% 8x\n", idx, core->Reg[idx]);
			//break;
		}
		//fprintf(core->state_log, "PSR:%08x\n", core->Cpsr);
		setbuf(core->state_log, NULL);
	}
	if ( (TOP_LOG > BOT_LOG) && (cpu->icounter > TOP_LOG))
	{
		printf("End logging\n");
		exit(-1);
	}

#else
	static uint32_t val;
	static uint32_t icounter = 0;
	icounter++;
	static uint32_t readcount = 0;
        uint32_t pc = cpu->f.get_pc(cpu, cpu->rf.grf);

	static int flag = 0, readflag = 0;
	if (flag == 0) {
		flag = 1;
		char buf [100];
		int i;
		for (i = 0; i < 16; i++) {
			fgets(buf, 100, core->state_log);
			printf("%s", buf);
		}
	}
	
	if ( (TOP_LOG > BOT_LOG) && (icounter > TOP_LOG))
	{
		printf("End logging\n");
		exit(-1);
	}
	if (readflag == 0)
	{
		readflag = 1;
		fscanf(core->state_log, "---|%08x|---PC:%08x\n", &readcount, &val);
	}
	if (readflag == 1)
	{
		if (icounter < readcount)
		{
			return;
		}
	}
	readflag = 0;
	
	//printf("found icounter %d with %x\n", icounter, val);

        if (val != pc) {
		printf("---|%08x| PC: [Q] %08x - [S] %08x ### PC ERROR\n", readcount, val, pc);
                exit(-1);
        }

	uint32_t dummy = 0xff;
	uint32_t v = 0xff;
	bool flags = 0;
	for (idx = 0; idx < 16; idx ++) {
		fscanf(core->state_log, "R%d: %x\n", &dummy, &v);
		//printf("found r0 %d with %x\n", dummy, v);
		if (dummy == idx) {
			if ((core->Reg[idx] != v))
			{
				printf("---|%08x|---PC:%08x --- R%02d: [Q] % 8x - [S] % 8x\n", icounter, pc, idx, v, core->Reg[idx]);
				//flags = 1;
			}
		} else {
			printf("wrong dummy\n");
			exit(-1);
		}
	}
	fscanf(core->state_log, "PSR:%0x\n", &dummy, &v);
	if (flags) {
		flags = 0;
		exit(-1);
	}
#endif
#endif
#endif
	return 0;
}

extern "C" unsigned arm_dyncom_SWI (ARMul_State * state, ARMword number);
extern "C" void arm_dyncom_Abort(ARMul_State * state, ARMword vector);

static void arm_dyncom_syscall(cpu_t* cpu, uint32_t num){

	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	sky_pref_t* pref = get_skyeye_pref();
	//printf("in %s user_mode_sim %d", __FUNCTION__, pref->user_mode_sim);
	if(pref->user_mode_sim)
		arm_dyncom_SWI(core, num);
	else
		//ARMul_Abort(core,ARMul_SWIV);
		core->syscallSig = 1;
}

/* Undefined instruction handler, set necessary flags */
void 
arm_undef_instr(cpu_t *cpu){
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	printf("\t\tLet us set a flag, signaling an undefined instruction!\n");
	core->Aborted = ARMul_UndefinedInstrV;
	core->abortSig = HIGH;
}

/**
 * @brief Generate the invoke undef instr exception llvm IR
 *
 * @param cpu CPU core structure
 * @param bb basic block to store llvm IR 
 * @param instr undefined instruction (unused)
 */
void
arch_arm_undef(cpu_t *cpu, BasicBlock *bb, uint32_t instr)
{
	if (cpu->dyncom_engine->ptr_arch_func[ARM_DYNCOM_CALLOUT_UNDEF] == NULL) {
		printf("in %s Could not find callout\n", __FUNCTION__);
		return;
	}
	Type const *intptr_type = cpu->dyncom_engine->exec_engine->getTargetData()->getIntPtrType(_CTX());
	Constant *v_cpu = ConstantInt::get(intptr_type, (uintptr_t)cpu);
	Value *v_cpu_ptr = ConstantExpr::getIntToPtr(v_cpu, PointerType::getUnqual(intptr_type));
	std::vector<Value *> params;
	params.push_back(v_cpu_ptr);
	/* When using a custom callout, must put the callout index as argument for dyncom_callout */
	params.push_back(CONST(ARM_DYNCOM_CALLOUT_UNDEF));
	//params.push_back(CONST(instr)); // no need for now, the callout func takes no argument
	CallInst *ret = CallInst::Create(cpu->dyncom_engine->ptr_arch_func[ARM_DYNCOM_CALLOUT_UNDEF], params.begin(), params.end(), "", bb);
}

/* Undefined instruction handler initialization. Should be called once at init */
static void 
arch_arm_undef_init(cpu_t *cpu){
	//types
	std::vector<const Type*> type_func_undef_args;
	PointerType *type_intptr = PointerType::get(cpu->dyncom_engine->exec_engine->getTargetData()->getIntPtrType(_CTX()), 0);
	const IntegerType *type_i32 = IntegerType::get(_CTX(), 32);
	type_func_undef_args.push_back(type_intptr);	/* intptr *cpu */
	type_func_undef_args.push_back(type_i32);	/* unsinged int */
	FunctionType *type_func_undef_callout = FunctionType::get(
		Type::getInt32Ty(cpu->dyncom_engine->mod->getContext()),	//return
		type_func_undef_args,	/* Params */
		false);		      	/* isVarArg */
	/* For a custom callout, the dyncom_calloutX functions should be used */
	Constant *undef_const = cpu->dyncom_engine->mod->getOrInsertFunction("dyncom_callout",	//function name
		type_func_undef_callout);	//return
	if(undef_const == NULL)
		fprintf(stderr, "Error:cannot insert function:undefined_instr_callout.\n");
	Function *undef_func = cast<Function>(undef_const);
	undef_func->setCallingConv(CallingConv::C);
	cpu->dyncom_engine->ptr_arch_func[ARM_DYNCOM_CALLOUT_UNDEF] = undef_func;
	cpu->dyncom_engine->arch_func[ARM_DYNCOM_CALLOUT_UNDEF] = (void*)arm_undef_instr;
}

void arm_dyncom_init(arm_core_t* core){
	cpu_t* cpu = cpu_new(0, 0, arm_arch_func);
#if __FOLLOW_MODE__
//	cpu->src_log = fopen("/data/state.log", "r");
	cpu->src_log = fopen("/diff/state.log", "r");
	if (cpu->src_log == NULL) {
		printf("Load log file failed.\n");
		//exit(-1);
	}
	printf("Load source log file successfully.\n");
	cpu->fm_log = fopen("/data/fm.log", "w");
	printf("Create follow mode log file.\n");
#endif

	/* set user mode or not */
	sky_pref_t *pref = get_skyeye_pref();
	if(pref->user_mode_sim)
                cpu->is_user_mode = 1;
        else
                cpu->is_user_mode = 0;
	
	core->NirqSig = HIGH;
	cpu->dyncom_engine->code_entry = 0x80d0;
	if (!pref->user_mode_sim) {
		cpu->dyncom_engine->code_start = 0;
		cpu->dyncom_engine->code_end = 0xffffffff;
	} else {
		cpu->dyncom_engine->code_end = 0x100000;
		cpu->dyncom_engine->code_entry = 0x80d0;
	}

	cpu->switch_mode = arm_switch_mode;
	/* for sync with interpret mode */
	cpu->check_int_flag = 0;
	
	cpu->mem_ops = arm_dyncom_mem_ops;
	//cpu->cpu_data = (conf_object_t*)core;
	cpu->cpu_data = get_conf_obj_by_cast(core, "arm_core_t");
	
	/* init the reg structure */
	cpu->rf.pc = &core->Reg[15];
	/* Under user mode sim, both phys_pc and pc are pointed to Reg 15 */
	if(is_user_mode(cpu))
		cpu->rf.phys_pc = &core->Reg[15];
	else
		cpu->rf.phys_pc = &core->phys_pc;
	cpu->rf.grf = core->Reg;
	//cpu->rf.srf = core->Spsr;
	//cpu->rf.srf = &core->phys_pc;
	cpu->rf.srf = core->Reg_usr;

	
	cpu->debug_func = arm_debug_func;
	
	if(pref->user_mode_sim){
		cpu->syscall_func = arm_dyncom_syscall;
	}
	else
//		cpu->syscall_func = NULL;
		cpu->syscall_func = arm_dyncom_syscall;
	core->dyncom_cpu = get_conf_obj_by_cast(cpu, "cpu_t");
	
	cpu->dyncom_engine->flags &= ~CPU_FLAG_SWAPMEM;

	if (pref->user_mode_sim){
#ifdef FAST_MEMORY
		cpu->dyncom_engine->RAM = (uint8_t*)get_dma_addr(0);
#endif
	}

	//core->CP15[CP15(CP15_MAIN_ID)] = 0x410FB760;
	core->CP15[CP15(CP15_MAIN_ID)] = 0x7b000;
	//core->CP15[CP15_MAIN_ID + 1] = 0x410FB760;
	//core->CP15[CP15_MAIN_ID - 1] = 0x410FB760;
	core->CP15[CP15(CP15_CONTROL)] = 0x00050078;
//	core->CP15[CP15(CP15_CONTROL)] = 0x00000078;
	core->CP15[CP15(CP15_CACHE_TYPE)] = 0xd172172;
	core->Cpsr = 0xd3;
	core->Mode = SVC32MODE;

//	load_symbol_from_sysmap();

	/* undefined instr handler init */
	arch_arm_undef_init(cpu);
	
	init_compiled_queue(cpu);
	return;
}

void switch_mode(arm_core_t *core, uint32_t mode)
{
	uint32_t tmp1, tmp2;
	if (core->Mode == mode) {
		//Mode not changed.
		//printf("mode not changed\n");
		return;
	}
	//printf("%d --->>> %d\n", core->Mode, mode);
	if (mode != USERBANK) {
		switch (core->Mode) {
		case USER32MODE:
			core->Reg_usr[0] = core->Reg[13];
			core->Reg_usr[1] = core->Reg[14];
			break;
		case IRQ32MODE:
			core->Reg_irq[0] = core->Reg[13];
			core->Reg_irq[1] = core->Reg[14];
			core->Spsr[IRQBANK] = core->Spsr_copy;
			break;
		case SVC32MODE:
			core->Reg_svc[0] = core->Reg[13];
			core->Reg_svc[1] = core->Reg[14];
			core->Spsr[SVCBANK] = core->Spsr_copy;
			break;
		case ABORT32MODE:
			core->Reg_abort[0] = core->Reg[13];
			core->Reg_abort[1] = core->Reg[14];
			core->Spsr[ABORTBANK] = core->Spsr_copy;
			break;
		case UNDEF32MODE:
			core->Reg_undef[0] = core->Reg[13];
			core->Reg_undef[1] = core->Reg[14];
			core->Spsr[UNDEFBANK] = core->Spsr_copy;
			break;
		case FIQ32MODE:
			core->Reg_firq[0] = core->Reg[13];
			core->Reg_firq[1] = core->Reg[14];
			core->Spsr[FIQBANK] = core->Spsr_copy;
			break;

		}

		switch (mode) {
		case USER32MODE:
			core->Reg[13] = core->Reg_usr[0];
			core->Reg[14] = core->Reg_usr[1];
			core->Bank = USERBANK;
			break;
		case IRQ32MODE:
			core->Reg[13] = core->Reg_irq[0];
			core->Reg[14] = core->Reg_irq[1];
			core->Spsr_copy = core->Spsr[IRQBANK];
			core->Bank = IRQBANK;
			break;
		case SVC32MODE:
			core->Reg[13] = core->Reg_svc[0];
			core->Reg[14] = core->Reg_svc[1];
			core->Spsr_copy = core->Spsr[SVCBANK];
			core->Bank = SVCBANK;
			break;
		case ABORT32MODE:
			core->Reg[13] = core->Reg_abort[0];
			core->Reg[14] = core->Reg_abort[1];
			core->Spsr_copy = core->Spsr[ABORTBANK];
			core->Bank = ABORTBANK;
			break;
		case UNDEF32MODE:
			core->Reg[13] = core->Reg_undef[0];
			core->Reg[14] = core->Reg_undef[1];
			core->Spsr_copy = core->Spsr[UNDEFBANK];
			core->Bank = UNDEFBANK;
			break;
		case FIQ32MODE:
			core->Reg[13] = core->Reg_firq[0];
			core->Reg[14] = core->Reg_firq[1];
			core->Spsr_copy = core->Spsr[FIQBANK];
			core->Bank = FIQBANK;
			break;

		}
		core->Mode = mode;
	} else {
		printf("user mode\n");
		exit(-2);
	}
}

void arm_switch_mode(cpu_t *cpu)
{
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	switch_mode(core, core->Cpsr & 0x1f);
}

static int flush_current_page(cpu_t *cpu){
	//arm_core_t* core = (arm_core_t*)(cpu->cpu_data);
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	addr_t effec_pc = *(addr_t*)cpu->rf.pc;
//	printf("effec_pc is %x\n", effec_pc);
//	printf("in %s\n", __FUNCTION__);
	int ret = cpu->mem_ops.effective_to_physical(cpu, effec_pc, (uint32_t*)cpu->rf.phys_pc);
	cpu->current_page_phys = core->phys_pc & 0xfffff000;
	cpu->current_page_effec = core->pc & 0xfffff000;
	return ret;
}

void arm_dyncom_run(cpu_t* cpu){
	//arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	arm_core_t* core = (arm_core_t*)(cpu->cpu_data->obj);
	uint32_t mode;

	addr_t phys_pc;
	if(is_user_mode(cpu)){
		addr_t phys_pc = core->Reg[15];
	}
#if 0
	if(mmu_read_(core, core->pc, PPC_MMU_CODE, &phys_pc) != PPC_MMU_OK){
		/* we donot allow mmu exception in tagging state */
		fprintf(stderr, "In %s, can not translate the pc 0x%x\n", __FUNCTION__, core->pc);
		exit(-1);
	}
#endif

#if 0
	cpu->dyncom_engine->code_start = phys_pc;
        cpu->dyncom_engine->code_end = get_end_of_page(phys_pc);
        cpu->dyncom_engine->code_entry = phys_pc;
#endif

	int rc = cpu_run(cpu);
	//printf("### Out of jit return %d - %p %p\n", rc, core->Reg[15], core->Reg[15]);
	
//	printf("pc %x is not found\n", core->Reg[15]);
	switch (rc) {
	case JIT_RETURN_NOERR: /* JIT code wants us to end execution */
	case JIT_RETURN_TIMEOUT:
                        break;
                case JIT_RETURN_SINGLESTEP:
	case JIT_RETURN_FUNCNOTFOUND:
//			printf("pc %x is not found\n", core->Reg[15]);
//			printf("phys_pc is %x\n", core->phys_pc);
//			printf("out of jit\n");
			if(!is_user_mode(cpu)){
				switch_mode(core, core->Cpsr & 0x1f);
				if (flush_current_page(cpu)) {
					return;
				}
				clear_tag_page(cpu, core->phys_pc);
				cpu_tag(cpu, core->phys_pc);
				cpu->dyncom_engine->cur_tagging_pos ++;
				//cpu_translate(cpu, core->Reg[15]);
				cpu_translate(cpu, core->phys_pc);
			}
			else{
				cpu_tag(cpu, core->Reg[15]);
				cpu->dyncom_engine->cur_tagging_pos ++;
				cpu_translate(cpu, core->Reg[15]);
			}

		 /*
                  *If singlestep,we run it here,otherwise,break.
                  */
                        if (cpu->dyncom_engine->flags_debug & CPU_DEBUG_SINGLESTEP){
                                rc = cpu_run(cpu);
                                if(rc != JIT_RETURN_TRAP)
                                        break;
                        }
                        else
                                break;
	case JIT_RETURN_TRAP:
		if (core->syscallSig) {
			return;
		}
		if (cpu->check_int_flag == 1) {
			cpu->check_int_flag = 0;
			return;
		}
		if (core->abortSig) {
			return;
		}
//		printf("cpu maybe changed mode.\n");
//		printf("pc is %x\n", core->Reg[15]);
		//printf("icounter is %lld\n", cpu->icounter);
		//exit(-1);
		//core->Reg[15] += 4;
		mode = core->Cpsr & 0x1f;
		if ( (mode != core->Mode) && (!is_user_mode(cpu)) ) {
			switch_mode(core, mode);
			//exit(-1);
		}
		core->Reg[15] += 4;
		return;
			break;
		default:
                        fprintf(stderr, "unknown return code: %d\n", rc);
			skyeye_exit(-1);
        }// switch (rc)
	return;
}
/**
* @brief Debug function that will be called in every instruction execution, print the cpu state
*
* @param cpu the cpu_t instance
*/

void arm_dyncom_stop(){
}

void arm_dyncom_fini(){
}
