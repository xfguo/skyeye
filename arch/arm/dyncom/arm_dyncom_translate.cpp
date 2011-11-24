/*
 * libcpu: arm_translate.cpp
 *
 * main translation code
 */

#include "llvm/Instructions.h"
#include "llvm/Intrinsics.h"

#include "dyncom/defines.h"
#include "skyeye_dyncom.h"
#include "skyeye_obj.h"
#include "dyncom/dyncom_llvm.h"
#include "dyncom/frontend.h"
#include "arm_internal.h"
#include "arm_types.h"
#include "dyncom/tag.h"
#include "bank_defs.h"
#include "armdefs.h"
#include "arm_dyncom_dec.h"
#include "arm_dyncom_translate.h"
#include "arm_dyncom_run.h"
//#include "armemu.h"
#include "arm_dyncom_thumb.h"
#include "skyeye_instr_length.h"

using namespace llvm;

int arm_tag_continue(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc);
int arm_tag_branch(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc);
typedef int (*tag_func_t)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc);
typedef int (*translate_func_t)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc);
typedef Value* (*translate_cond_func_t)(cpu_t *cpu, addr_t pc, BasicBlock *bb);
typedef struct arm_opc_func_s{
        tag_func_t tag;
        translate_func_t translate;
        translate_cond_func_t translate_cond;
}arm_opc_func_t;
arm_opc_func_t* arm_get_opc_func(uint32_t opc);
Value *arm_translate_cond(cpu_t *cpu, uint32_t instr, BasicBlock *bb);

arm_opc_func_t  arm_opc_table_main[0xff+1];

#if 0
int init_arm_opc_group0(arm_opc_func_t* arm_opc_table);
int init_arm_opc_group1(arm_opc_func_t* arm_opc_table);
int init_arm_opc_group2(arm_opc_func_t* arm_opc_table);
int init_arm_opc_group3(arm_opc_func_t* arm_opc_table);
int init_arm_opc_group4(arm_opc_func_t* arm_opc_table);
int init_arm_opc_group5(arm_opc_func_t* arm_opc_table);
int init_arm_opc_group6(arm_opc_func_t* arm_opc_table);
int init_arm_opc_group7(arm_opc_func_t* arm_opc_table);
int init_arm_opc_group8(arm_opc_func_t* arm_opc_table);
int init_arm_opc_group9(arm_opc_func_t* arm_opc_table);
int init_arm_opc_group10(arm_opc_func_t* arm_opc_table);
int init_arm_opc_group11(arm_opc_func_t* arm_opc_table);
int init_arm_opc_group12(arm_opc_func_t* arm_opc_table);
int init_arm_opc_group13(arm_opc_func_t* arm_opc_table);
int init_arm_opc_group14(arm_opc_func_t* arm_opc_table);
int init_arm_opc_group15(arm_opc_func_t* arm_opc_table);
#endif

#ifndef INSTR_SIZE
#define INSTR_SIZE 4
#endif

#define BAD_INSTR {fprintf(stderr, "In %s, cannot parse instruction 0x%x\n", __FUNCTION__, instr);exit(-1);}
int opc_invalid_tag(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	BAD_INSTR;
	return -1;
}
int opc_invalid_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){
	BAD_INSTR;
	return -1;
}
Value* opc_invalid_translate_cond(cpu_t *cpu, uint32_t instr, BasicBlock *bb){
	BAD_INSTR;
	return NULL;
}
static arm_opc_func_t ppc_opc_invalid = {
	opc_invalid_tag,
	opc_invalid_translate,
	opc_invalid_translate_cond,
};

inline bool InAPrivilegedMode(arm_core_t *core)
{
	return (core->Mode != USER32MODE);
}

//typedef std::map<addr_t, int> decoder_cache;
decoder_cache dc_map;

int32_t arch_arm_get_instr_category(addr_t pc)
{
	//FIXME
	//return dc_map[pc];
	decoder_cache::const_iterator it = dc_map.find(pc);
		if (it != dc_map.end()) {
			return (int32_t)it->second;
		} else{
			printf("Can not find %x in decode cache\n", pc);
			exit(-1);
		}
}

void arch_arm_insert_instr_category(addr_t pc, int32_t index)
{
	dc_map[pc] = index;
}

void arch_arm_flus_instr_category()
{
	dc_map.clear();
}

typedef int (*trans_fp_t)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc);
typedef int (*tag_fp_t)(cpu_t *cpu, addr_t pc, uint32_t addr, tag_t *tag, addr_t *new_pc, addr_t *next_pc);
typedef Value *(*cond_fp_t)(cpu_t *cpu, uint32_t instr, BasicBlock *bb);

struct instruction_action {
	const char *name;
	trans_fp_t translate_func;
	tag_fp_t tag_func;
	cond_fp_t cond_func;
};
typedef struct instruction_action INSTRACT;
static tdstate decode_dyncom_thumb_instr(arm_core_t *core, uint32_t inst, uint32_t *arm_inst, addr_t pc, int* index);

#define ADU_VER 0

extern const INSTRACT arm_instruction_action[];
extern const ISEITEM arm_instruction[];


Value * arch_arm_translate_cond(cpu_t *cpu, addr_t pc, BasicBlock *bb)
{
	uint32_t instr;
	bus_read(32, pc, &instr);
	int index = arch_arm_get_instr_category(pc);
	return arm_instruction_action[index].cond_func(cpu, instr, bb);
}

/**
* @brief Distinguish 32bit thumb instruction from arm 32 bit instruction
*
* @param inst an instruction
*
* @return true or false
*/
static inline int is_thumb32_inst(uint32 inst){
	int op;
	op = 0xf8000000 & inst;
	return ((op == 0x1d) || (op = 0x1e) || (op = 0x1f));
}

#define BIT(n) ((instr >> (n)) & 1)
#define BITS(a,b) ((instr >> (a)) & ((1 << (1+(b)-(a)))-1))
int arch_arm_tag_instr(cpu_t *cpu, addr_t pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc) {
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
        int instr_size = INSTR_SIZE;
        uint32_t instr;
	bus_read(32, pc, &instr);
	int index = -1;
	int ret = DECODE_FAILURE;
	tdstate current_state = t_undefined;
	if(core->Cpsr & (1 << THUMB_BIT)){
		/* Get the corresponding arm instruction for thumb instruction */
		uint32 arm_inst;

		current_state = decode_dyncom_thumb_instr(core, instr, &arm_inst, pc, &index);
		printf("In %s, thumb instruction , index=%d\n", __FUNCTION__, index);
		instr = arm_inst;
		instr_size = 2;
		/* for branch and state change instruction, we have already get the index from thumb decoder */
		if(current_state != t_branch){
			ret = decode_arm_instr(instr, &index);
		}
	}
	else{
		ret = decode_arm_instr(instr, &index);
	}

	if (ret == DECODE_SUCCESS) {
		arch_arm_insert_instr_category(pc, index);
		arm_instruction_action[index].tag_func(cpu, pc, instr, tag, new_pc, next_pc);
	} else {
		printf("in %s unknown instruction %x at %x.\n", __FUNCTION__, instr, pc);
		exit(-1);
	}

        return instr_size;
}
#undef BITS
#undef BIT

#define DEBUG_FLAGS 0

int arch_arm_translate_instr(cpu_t *cpu, addr_t pc, BasicBlock *bb) {
	int instr_size = INSTR_SIZE;
	uint32_t instr = 0xFFFFFFFF;
	if(bus_read(32, pc, &instr) != 0){
		/* instruction read error handler here */
 	}

	int index = arch_arm_get_instr_category(pc);
	arm_instruction_action[index].translate_func(cpu, instr, bb, pc);
#if DEBUG_FLAGS	
	if(!cpu->is_user_mode) {						\
		Value *z = SHL(ZEXT32(LOAD(ptr_Z)), CONST(30)); 		\
		Value *n = SHL(ZEXT32(LOAD(ptr_N)), CONST(31));			\
		Value *c = SHL(ZEXT32(LOAD(ptr_C)), CONST(29));			\
		Value *v = SHL(ZEXT32(LOAD(ptr_V)), CONST(28));			\
		Value *nzcv = OR(OR(OR(z, n), c), v);				\
		Value *cpsr = OR(AND(R(CPSR_REG), CONST(0xfffffff)), nzcv);	\
		LET(CPSR_REG, cpsr);						\
	}									
#endif
	return instr_size;
}

#define BITS(a,b) ((opc >> (a)) & ((1 << (1+(b)-(a)))-1))
#define BIT(n) ((opc>>(n))&1)
#define ARM_OPC_MAIN(opc)		(((opc)>>20)&0xff)
arm_opc_func_t* arm_get_opc_func(uint32_t opc)
{
	uint32 mainopc = ARM_OPC_MAIN(opc);
	return &arm_opc_table_main[mainopc];
}
#undef BITS
#undef BIT

#define BIT(n) ((instr >> (n)) & 1)
#define BITS(a,b) ((instr >> (a)) & ((1 << (1+(b)-(a)))-1))

void arm_opc_func_init()
{
	//*arm_opc_table =(arm_opc_func_t *) malloc(sizeof(arm_opc_func_t) * 0xff);
	arm_opc_func_t *arm_opc_table = arm_opc_table_main;
	int i;
	for(i = 0; i < 0x100; i ++){
		arm_opc_table[i].translate_cond = arm_translate_cond;
	}

#if 0
	init_arm_opc_group0(&arm_opc_table[0x0]);
	init_arm_opc_group1(&arm_opc_table[0x10]);
	init_arm_opc_group2(&arm_opc_table[0x20]);
	init_arm_opc_group3(&arm_opc_table[0x30]);
	init_arm_opc_group4(&arm_opc_table[0x40]);
	init_arm_opc_group5(&arm_opc_table[0x50]);
	init_arm_opc_group6(&arm_opc_table[0x60]);
	init_arm_opc_group7(&arm_opc_table[0x70]);
	init_arm_opc_group8(&arm_opc_table[0x80]);
	init_arm_opc_group9(&arm_opc_table[0x90]);
	init_arm_opc_group10(&arm_opc_table[0xa0]);
	init_arm_opc_group11(&arm_opc_table[0xb0]);
	init_arm_opc_group12(&arm_opc_table[0xc0]);
	init_arm_opc_group13(&arm_opc_table[0xd0]);
	init_arm_opc_group14(&arm_opc_table[0xe0]);
	init_arm_opc_group15(&arm_opc_table[0xf0]);
#endif
}


Value *
arm_translate_cond(cpu_t *cpu, uint32_t instr, BasicBlock *bb) {
	switch ((instr) >> 28) {
		case 0x0: /* EQ */
			return LOAD(ptr_Z);
		case 0x1: /* NE */
			return NOT(LOAD(ptr_Z));
		case 0x2: /* CS */
			return LOAD(ptr_C);
		case 0x3: /* CC */
			return NOT(LOAD(ptr_C));
		case 0x4: /* MI */
			return LOAD(ptr_N);
		case 0x5: /* PL */
			return NOT(LOAD(ptr_N));
		case 0x6: /* VS */
			return LOAD(ptr_V);
		case 0x7: /* VC */
			return NOT(LOAD(ptr_V));
		case 0x8: /* HI */
			return AND(LOAD(ptr_C),NOT(LOAD(ptr_Z)));
		case 0x9: /* LS */
			return NOT(AND(LOAD(ptr_C),NOT(LOAD(ptr_Z))));
		case 0xA: /* GE */
			return ICMP_EQ(LOAD(ptr_N),LOAD(ptr_V));
		case 0xB: /* LT */
			return NOT(ICMP_EQ(LOAD(ptr_N),LOAD(ptr_V)));
		case 0xC: /* GT */
			return AND(NOT(LOAD(ptr_Z)),ICMP_EQ(LOAD(ptr_N),LOAD(ptr_V)));
		case 0xD: /* LE */
			return NOT(AND(NOT(LOAD(ptr_Z)),ICMP_EQ(LOAD(ptr_N),LOAD(ptr_V))));
		case 0xE: /* AL */
			return NULL; /* no condition; this should never happen */
		case 0xF: /* NV */
			return FALSE;
		default:
			assert(0 && "Cannot happen");
      return FALSE;
	}
}

int arm_tag_trap(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	*tag = TAG_TRAP;
	*next_pc = pc + INSTR_SIZE;
	if((instr >> 28 != 0xe) && (instr >> 28 != 0xf))
		*tag |= TAG_CONDITIONAL;
}
//#define instr_size 4
int arm_tag_continue(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	*tag = TAG_CONTINUE;
	*next_pc = pc + INSTR_SIZE;
	if((instr >> 28 != 0xe) && (instr >> 28 != 0xf))
		*tag |= TAG_CONDITIONAL;
}

int arm_tag_stop(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	*tag = TAG_STOP;
	*next_pc = pc + INSTR_SIZE;
	if((instr >> 28 != 0xe) && (instr >> 28 != 0xf))
		*tag |= TAG_CONDITIONAL;
}
int arm_tag_ret(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	*tag = TAG_RET;
	*next_pc = pc + INSTR_SIZE;
	if((instr >> 28 != 0xe) && (instr >> 28 != 0xf))
		*tag |= TAG_CONDITIONAL;
}

#define ARM_BRANCH_TARGET (BIT(23)?(pc - ((~(BITS(0,23) << 8) >> 6) + 1) + 8):(((BITS(0,23) << 8) >> 6) + pc + 8))
int arm_tag_call(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	uint32_t opc = instr;
	*tag = TAG_CALL;
	*new_pc = ARM_BRANCH_TARGET;
	*next_pc = pc + INSTR_SIZE;
	if((instr >> 28 != 0xe) && (instr >> 28 != 0xf))
		*tag |= TAG_CONDITIONAL;
}

int arm_tag_branch(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	uint32_t opc = instr;
	*tag = TAG_BRANCH;
	if(is_user_mode(cpu))
		/* For R15 update , that should be NEW_PC_NONE */
		*new_pc = ARM_BRANCH_TARGET;
	else
		*new_pc = NEW_PC_NONE;
	//printf("in %s pc is %x new pc is %x\n", __FUNCTION__, pc, ARM_BRANCH_TARGET);
//	sleep(1);
	*next_pc = pc + INSTR_SIZE;
	if((instr >> 28 != 0xe) && (instr >> 28 != 0xf))
	{
		*tag |= TAG_CONDITIONAL;
	}
}


#define glue(x, y)		x ## y
#define DYNCOM_TRANS(s) 	glue(arm_translate_, s)
#define DYNCOM_TAG(s)		glue(arm_tag_, s)
//#define INSTR_SIZE 4

#define DYNCOM_FILL_ACTION(s) 	{#s, DYNCOM_TRANS(s), DYNCOM_TAG(s), arm_translate_cond}

#define GETSIGN(ret,ptr)			(new StoreInst(ICMP_SLT(ret, CONST(0)), ptr, bb))
#define EQZERO(ret,ptr)				(new StoreInst(ICMP_EQ(ret, CONST(0)), ptr, bb))
#define CARRYFROMADD(op1,ret,ptr)		(new StoreInst(ICMP_ULT(ret, op1), ptr, false, bb))
#define NOTBORROWFROMSUB(op1,op2,ptr)		(new StoreInst(ICMP_UGE(op1, op2), ptr, false, bb))
#define OVERFLOWFROMADD(op1,op2,ret,ptr)	(new StoreInst(ICMP_SLT(AND(COM(XOR(op1, op2)), XOR(op1,ret)), CONST(0)), ptr, bb))
#define OVERFLOWFROMSUB(op1,op2,ret,ptr)	(new StoreInst(ICMP_SLT(AND((XOR(op1, op2)), XOR(op1,ret)), CONST(0)), ptr, bb))
#define INCOMPLETE printf("in %s:%d, incomplete implementation\n", __FUNCTION__, __LINE__); asm("int $3"); exit(-1);
#define UNPREDICTABLE(pc) printf("in %s:%d, pc %x is unpredictable\n", __FUNCTION__, __LINE__, pc); asm("int $3"); exit(-1);

#define SET_CPSR if(!cpu->is_user_mode) {						\
			Value *z = SHL(ZEXT32(LOAD(ptr_Z)), CONST(30)); 		\
			Value *n = SHL(ZEXT32(LOAD(ptr_N)), CONST(31));			\
			Value *c = SHL(ZEXT32(LOAD(ptr_C)), CONST(29));			\
			Value *v = SHL(ZEXT32(LOAD(ptr_V)), CONST(28));			\
			Value *nzcv = OR(OR(OR(z, n), c), v);				\
			Value *cpsr = OR(AND(R(CPSR_REG), CONST(0xfffffff)), nzcv);	\
			LET(CPSR_REG, cpsr);						\
		}
		
#define SET_NEW_PAGE if (!cpu->is_user_mode) { 				\
	Value *new_page_effec = AND(R(15), CONST(0xfffff000));		\
	new StoreInst(new_page_effec, cpu->ptr_CURRENT_PAGE_EFFEC, bb);	\
	}; LET(PHYS_PC, R(15));
	
#define SET_NEW_PAGE_PC(pc) if (!cpu->is_user_mode) { 				\
	Value *new_page_effec = AND(CONST(pc), CONST(0xfffff000));		\
	new StoreInst(new_page_effec, cpu->ptr_CURRENT_PAGE_EFFEC, bb);	\
	}; LET(PHYS_PC, CONST(pc));
	

int DYNCOM_TRANS(adc)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	/* for 0x0a 0x0b 0x1a 0x1b */
	Value *op1 = R(RN);
	Value *op2 = OPERAND;
	Value *ret = SELECT(LOAD(ptr_C), ADD(ADD(op1, op2), CONST(1)), ADD(op1, op2));

	LET(RD, ret);
	if(SBIT)
	{
		if (RD == 15) {
			INCOMPLETE;
			/* Shouldn't we have CPSR = SPSR ? */
		} else {
			GETSIGN(ret,ptr_N);
			EQZERO(ret, ptr_Z);
			CARRYFROMADD(op1,ret,ptr_C);
			OVERFLOWFROMADD(op1,op2,ret,ptr_V);
		}
	}
}
int DYNCOM_TRANS(add)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	/* for 0x08 0x09 0x28 0x29 */
	Value *op1 = CHECK_RN_PC;
	Value *op2 = OPERAND;
	Value *ret = ADD(op1, op2);
	LET(RD, ret);
	if (RD == 15) {
		SET_NEW_PAGE;
		if (SBIT)
		{
			INCOMPLETE;
			/* Shouldn't we have CPSR = SPSR ? */
		}
	} else if(SBIT) {
		GETSIGN(ret,ptr_N);
		EQZERO(ret,ptr_Z);
		CARRYFROMADD(op1,ret,ptr_C);
		OVERFLOWFROMADD(op1,op2,ret,ptr_V);
	}
}
int DYNCOM_TRANS(and)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	/* for 0x00, 0x01, 0x20, 0x21 */
	Value *op1 = R(RN);
	Value *op2 = SCO_OPERAND(SBIT ? ptr_C : NULL);
	Value *ret = AND(op1,op2);
	LET(RD, ret);
	if(SBIT)
	{
		if (RD == 15) {
			INCOMPLETE;
			/* Shouldn't we have CPSR = SPSR ? */
		} else {
			GETSIGN(ret,ptr_N);
			EQZERO(ret,ptr_Z);
			/* C in SCO_OPERAND */
		}
	}
}
int DYNCOM_TRANS(bbl)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	if (cpu->is_user_mode) {
		if (BIT(24)) {
			if(BITS(20, 27) >= 0xb8 && BITS(20, 27) <=0xbf) {
				LET(14, CONST(pc + 4));
				LET(15, CONST(pc + 8 - BOPERAND));
				SET_NEW_PAGE_PC(pc + 8 - BOPERAND);
			} else if (BITS(20, 27) >= 0xb0 && BITS(20, 27) <=0xb7) {
				LET(14, CONST(pc + 4));
				LET(15, CONST(pc + 8 + BOPERAND));
				SET_NEW_PAGE_PC(pc + 8 + BOPERAND);
			}
		} else if(BIT(23)) {
			LET(15, CONST(pc + 8 - BOPERAND));
			SET_NEW_PAGE_PC(pc + 8 - BOPERAND);
		} else {
			LET(15, CONST(pc + 8 + BOPERAND));
			SET_NEW_PAGE_PC(pc + 8 + BOPERAND);
		}
	} else {
		if (BIT(24)) {
			if(BITS(20, 27) >= 0xb8 && BITS(20, 27) <=0xbf) {
				LET(14, ADD(R(15),CONST(4)));
				LET(15, SUB(ADD(R(15), CONST(8)), CONST(BOPERAND)));
			} else if (BITS(20, 27) >= 0xb0 && BITS(20, 27) <=0xb7) {
				LET(14, ADD(R(15),CONST(4)));
				LET(15, ADD(ADD(R(15), CONST(8)),CONST(BOPERAND))); 
			}
		} else {
			if(BIT(23)) {
				LET(15, SUB(ADD(R(15), CONST(8)),CONST(BOPERAND))); 
			} else {
				LET(15, ADD(ADD(R(15), CONST(8)),CONST(BOPERAND))); 
			}
		}
		SET_NEW_PAGE;
	}
}
int DYNCOM_TRANS(bic)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	/* for 0x1c 0x2d */
	Value *op1 = R(RN);
	Value *op2 = SCO_OPERAND(SBIT ? ptr_C : NULL);
	Value *ret = AND(op1,COM(op2));
	LET(RD, ret);

	if(SBIT) {
		if (RD == 15) {
			INCOMPLETE;
			/* Shouldn't we have CPSR = SPSR ? */
		} else {
			GETSIGN(ret,ptr_N);
			EQZERO(ret,ptr_Z);
			/* C in SCO_OPERAND */
		}
	}
}
int DYNCOM_TRANS(bkpt)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(blx)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	if (BITS(20, 27) == 0x12 && BITS(4, 7) == 0x3) {
		if (cpu->is_user_mode)
			LET(14, CONST((pc + INSTR_SIZE) | 0x1));
		else
			LET(14, OR(ADD(R(15),CONST(INSTR_SIZE)), CONST(0x1)));
		LET(15, AND(R(RM), CONST(0xFFFFFFFE)));
		/* Set thumb bit*/	
		STORE(TRUNC1(AND(R(RM), CONST(0x1))), ptr_T);	

		SET_NEW_PAGE;
	} else {
		printf("in %s\n", __FUNCTION__);
		exit(-1);
	}
}
int DYNCOM_TRANS(bx)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	/* ykl FIXME missing thumb switching */
	Value *op1 = R(RM);
	Value *ret = AND(op1, CONST(0xfffffffe));

	LET(15, ret);
	SET_NEW_PAGE;
}
int DYNCOM_TRANS(bxj)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(cdp)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(clrex)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(clz)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	Type const *ty = getIntegerType(32);
	Value* intrinsic_ctlz = (Value*)Intrinsic::getDeclaration(cpu->dyncom_engine->mod, Intrinsic::ctlz, &ty, 1);
	Value* result = CallInst::Create(intrinsic_ctlz, R(RM), "", bb);
	LET(RD, result);
}
int DYNCOM_TRANS(cmn)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	/* for 0x17 0x37 */
	Value *op1 = R(RN);
	Value *op2 = OPERAND;
	Value *ret = ADD(op1, op2);
	
	GETSIGN(ret,ptr_N);
	EQZERO(ret,ptr_Z);
	CARRYFROMADD(op1,ret,ptr_C);
	OVERFLOWFROMADD(op1,op2,ret,ptr_V);
}
#define ISNEG(VAL) TRUNC1(LSHR(VAL, CONST(31)))
#define ISPOS(VAL) TRUNC1(LSHR(COM(VAL), CONST(31)))
int DYNCOM_TRANS(cmp)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	/* for 0x15 0x35 */
	Value *op1 = R(RN);
	Value *op2 = OPERAND;

	Value *ret = SUB(op1, op2);

	GETSIGN(ret,ptr_N);
	EQZERO(ret,ptr_Z);
	NOTBORROWFROMSUB(op1,op2,ptr_C);
	OVERFLOWFROMSUB(op1,op2,ret,ptr_V);
}
int DYNCOM_TRANS(cps)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	//uint32_t cpsr_val = 0xfffffe3f;
	uint32_t cpsr_val = 0;
	uint32_t aif = 0;

	SET_CPSR;

	if (BIT(19) == 1) {
		if (BIT(8) == 1) {
			cpsr_val |= (BIT(18) << 8);
			aif |= 1 << 8;
		}
		if (BIT(7) == 1) {
			cpsr_val |= (BIT(18) << 7);
			aif |= 1 << 7;
		}
		if (BIT(6) == 1) {
			cpsr_val |= (BIT(18) << 6);
			aif |= 1 << 6;
		}
		aif = ~aif;

		Value *cpsr_old = R(CPSR_REG);
		Value *user_mode = ICMP_EQ(AND(cpsr_old, CONST(0xffffffe0)), CONST(0));
		LET(CPSR_REG, SELECT(user_mode,
				      cpsr_old,
				      OR(AND(cpsr_old, CONST(aif)), CONST(cpsr_val))
				      ));
		//LET(CPSR_REG, CONST(1));
	}
	#if 1
	if (BIT(17) == 1) {
		Value *cpsr_new = OR(AND(R(CPSR_REG), CONST(0xffffffe0)), CONST(BITS(0, 4)));
		LET(CPSR_REG, cpsr_new);
	}
	#endif
}
int DYNCOM_TRANS(cpy)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	if(RM == 15) {
		if (cpu->is_user_mode) {
			LET(15, CONST(pc + 8));
			LET(PHYS_PC, CONST(pc + 8));
		} else {
			LET(15, ADD(R(15), CONST(8)));
			LET(PHYS_PC, R(15));
		}
	}
	Value *op1 = R(RM);

	LET(RD, op1);
	if(RD == 15) {
		LET(PHYS_PC, op1);
	}
}
int DYNCOM_TRANS(eor)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	/* for 0x02, 0x03, 0x22, 0x23 */
	Value *op1 = R(RN);
	Value *op2 = SCO_OPERAND(SBIT ? ptr_C : NULL);
	Value *ret = XOR(op1,op2);

	LET(RD,ret);
	if(SBIT)
	{
		if (RD == 15) {
			INCOMPLETE;
			/* Shouldn't we have CPSR = SPSR ? */
		} else {
			GETSIGN(ret,ptr_N);
			EQZERO(ret,ptr_Z);
			/* C in SCO_OPERAND */
		}
	}
}
int DYNCOM_TRANS(ldc)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(ldm)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
	if (BIT(15)) {
		SET_NEW_PAGE;
		if (BITS(25, 27) == 4 && BIT(22) == 1 && BIT(20) == 1) {
			LET(CPSR_REG, R(SPSR_REG));
			cpu->f.emit_decode_reg(cpu, bb);
		}
	}
}
int DYNCOM_TRANS(ldr)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
//	printf("in %s %x\n", __FUNCTION__, cpu->f.get_pc(cpu, cpu->rf.grf));
//	sleep(2);
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
	if (RD == 15) {
		SET_NEW_PAGE;
	}
	return 0;
}
int DYNCOM_TRANS(ldrb)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	/*LDRB, No WriteBack, Pre Inc, Regist - Immed */
	/*I = 0, P = 1, U = 0, W = 0, B = 1 */

	/* LDRB, No WriteBack, Pre Inc, Regist + Immed || Regist */
	/* I = 0, P = 1, U = 1, B = 1, W = 0 */

	/* LDRB, WriteBack, Pre Inc, Immed. */
	/* I = 0, P = 1, U = 1, B = 1, W = 1 */
	
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);

	return 0;
}
int DYNCOM_TRANS(ldrbt)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
	printf("in %s instruction is not fully implementated.\n", __FUNCTION__);
	return 0;
}
int DYNCOM_TRANS(ldrd)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{

	/* LDRD P = 0, U = 0, I = 0, W = 0 */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
	printf("in %s instruction is not fully implementated.\n", __FUNCTION__);
	return 0;
}
int DYNCOM_TRANS(ldrex)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
//	Value *addr = GetAddr(cpu, instr, bb);
	Value *addr = R(RN);
	LoadStore(cpu,instr,bb,addr);
}
int DYNCOM_TRANS(ldrexb)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	//Value *addr = GetAddr(cpu, instr, bb);
	Value *addr = R(RN);
	LoadStore(cpu,instr,bb,addr);
}
int DYNCOM_TRANS(ldrh)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	/* LDRH immediate offset, no write-back, up, pre indexed.  */
	/* P = 1, U = 1, I = 1, W = 0 */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
	return 0;
}
int DYNCOM_TRANS(ldrsb)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
	return 0;
}
int DYNCOM_TRANS(ldrsh)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	/* LDRSH P=1 U=1 W=0 */
	Value *addr = GetAddr(cpu, instr, bb);
	//LoadStore(cpu,instr,bb,addr);
	Value *ret = arch_read_memory(cpu, bb, addr, 0, 16);
	LET(RD, SELECT(ICMP_EQ(AND(ret, CONST(0x8000)), CONST(0)), ret, OR(CONST(0xffff0000), ret)));

	return 0;
}
int DYNCOM_TRANS(ldrt)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
	printf("in %s instruction is not fully implementated.\n", __FUNCTION__);
	return 0;
}
int DYNCOM_TRANS(mcr)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	//FIXME : rfs
	if(RD == 15) {
		printf("in %s is not implementation.\n", __FUNCTION__);
		exit(-1);
	}
	
	if (BITS(8, 11) == 0xf) {
		if(CRn == 0 && OPCODE_2 == 0 && CRm == 0) {
			LET(CP15_MAIN_ID, R(RD));
		} else if(CRn == 1 && CRm == 0 && OPCODE_2 == 0) {
			LET(CP15_CONTROL, R(RD));
		} else if (CRn == 3 && CRm == 0 && OPCODE_2 == 0) {
			LET(CP15_DOMAIN_ACCESS_CONTROL, R(RD));
		} else if (CRn == 2 && CRm == 0 && OPCODE_2 == 0) {
			LET(CP15_TRANSLATION_BASE_TABLE_0, R(RD));
		} else if (CRn == 2 && CRm == 0 && OPCODE_2 == 1) {
			LET(CP15_TRANSLATION_BASE_TABLE_1, R(RD));
		} else if (CRn == 2 && CRm == 0 && OPCODE_2 == 2) {
			LET(CP15_TRANSLATION_BASE_CONTROL, R(RD));
//		} else if (CRn == 7 && CRm == 14 && OPCODE_2 == 0) {
//			LET(R(RD));
		} else {
			printf("mcr is not implementated. CRn is %d, CRm is %d, OPCODE_2 is %d\n", CRn, CRm, OPCODE_2);
			//exit(-1);
		}
	}
}
int DYNCOM_TRANS(mcrr)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(mla)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	/* for 0x00, 0x01*/
	Value *op1 = R(RM);
	Value *op2 = R(RS);
	Value *op3 = R(MUL_RN);
	Value *ret = ADD(MUL(op1,op2), op3);

	LET(MUL_RD, ret);

	if(SBIT)
	{
		GETSIGN(ret,ptr_N);
		EQZERO(ret,ptr_Z);
	}
}
int DYNCOM_TRANS(mov)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	if(RN == 15) {
		if (cpu->is_user_mode) {
			LET(15, CONST(pc + 8));
			LET(PHYS_PC, CONST(pc + 8));
		} else {
			LET(15, ADD(R(15), CONST(8)));
			LET(PHYS_PC, R(15));
		}
	}
	/* for 0x10 0x11 0x30 0x31 */
	Value *op1 = R(RN);
	Value *op2 = SCO_OPERAND(SBIT ? ptr_C : NULL);
	Value *tmp1, *tmp2, *tmp3, *tmp4, *tmp5;
#if 0
	if (SBIT && BITS(25, 27) == 0 && BITS(4, 6) == 0 ) {
		/* Logical shift left by immediate */
		if (BITS(7, 11) != 0) {
			tmp1 = CONST(32 - BITS(7, 11));
			//tmp1 = CONST(BITS(7, 11));
			tmp4 = LSHR(R(RM), tmp1);
			tmp3 = AND(tmp4, CONST(1));
			tmp5 = TRUNC1(tmp3);
		} else {
			printf("in %s\n", __FUNCTION__);
			//exit(-1);
		}
	}

	if (SBIT && BITS(25, 27) == 0 && BITS(4, 6) == 2) {
		/* Logical shift right by immediate */
		if (BITS(7, 11) != 0) {
			//tmp1 = LSHR(R(RM), CONST(BITS(7, 11)));
			tmp1 = CONST(BITS(7, 11) - 1);
			//tmp1 = CONST(BITS(7, 11));
			tmp4 = LSHR(R(RM), tmp1);
			tmp3 = AND(tmp4, CONST(1));
			tmp5 = TRUNC1(tmp3);
		} else {
			printf("in %s\n", __FUNCTION__);
			//exit(-1);
		}
	}

	if (SBIT && BITS(25, 27) == 0 && BITS(4, 6) == 4) {
		if (BITS(7, 11) != 0) {
			tmp1 = CONST(BITS(7, 11) - 1);
			//tmp1 = CONST(BITS(7, 11));
			tmp4 = LSHR(R(RM), tmp1);
			tmp3 = AND(tmp4, CONST(1));
			tmp5 = TRUNC1(tmp3);
		} else {
			printf("in %s\n", __FUNCTION__);
			//exit(-1);
		}
	}
#endif
	LET(RD, op2);
#if 1
	if(SBIT){
		if (RD == 15) {
			LET(CPSR_REG, R(SPSR_REG));

			Value *nzcv = LSHR(AND(R(CPSR_REG), CONST(0xf0000000)), CONST(28));
			Value *n = TRUNC1(AND(LSHR(nzcv, CONST(3)), CONST(1)));
			Value *z = TRUNC1(AND(LSHR(nzcv, CONST(2)), CONST(1)));
			Value *c = TRUNC1(AND(LSHR(nzcv, CONST(1)), CONST(1)));
			Value *v = TRUNC1(AND(LSHR(nzcv, CONST(0)), CONST(1)));
			new StoreInst(n, ptr_N, false, bb);
			new StoreInst(z, ptr_Z, false, bb);
			new StoreInst(c, ptr_C, false, bb);
			new StoreInst(v, ptr_V, false, bb);
		} else {
			GETSIGN(op2,ptr_N);
			EQZERO(op2,ptr_Z);
			/* C in SCO_OPERAND */
		}
	}
	if (RD == 15) {
		SET_NEW_PAGE;
	}
	
	/* Shifter carry out should have been handled in SCO_OPERAND, old code left here in case */
	#if 0
	if (SBIT && BITS(4, 6) == 0 && BITS(25, 27) == 0 && BITS(7, 11) != 0) {
		//Shift by imme
		///* C */ new StoreInst(tmp5, ptr_C, false, bb);
	}

	if (SBIT && BITS(25, 27) == 0 && BITS(4, 6) == 2) {
		/* C */ new StoreInst(tmp5, ptr_C, false, bb);
	}

	if (SBIT && BITS(25, 27) == 0 && BITS(4, 6) == 4) {
		/* C */ new StoreInst(tmp5, ptr_C, false, bb);
	}

	if (SBIT && (RD != 15)) {
		SET_CPSR;
	}
	#endif
#endif
}
int DYNCOM_TRANS(mrc)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	//FIXME : rfs
	if(RD == 15) {
		printf("in %s is not implementation.\n", __FUNCTION__);
		exit(-1);
	}
	
	Value *data = NULL;
	
	if (BITS(8, 11) == 0xf) {
		if(CRn == 0 && OPCODE_2 == 0 && CRm == 0) {
			data = R(CP15_MAIN_ID);
		} else if (CRn == 0 && CRm == 0 && OPCODE_2 == 1) {
			data = R(CP15_CACHE_TYPE);
		} else if (CRn == 1 && CRm == 0 && OPCODE_2 == 0) {
			data = R(CP15_CONTROL);
		} else if (CRn == 2 && CRm == 0 && OPCODE_2 == 0) {
			data = R(CP15_TRANSLATION_BASE_TABLE_0);
		} else if (CRn == 3 && CRm == 0 && OPCODE_2 == 0) {
			data = R(CP15_DOMAIN_ACCESS_CONTROL);
		} else if (CRn == 5 && CRm == 0 && OPCODE_2 == 0) {
			data = R(CP15_FAULT_STATUS);
		} else if (CRn == 5 && CRm == 0 && OPCODE_2 == 1) {
			data = R(CP15_INSTR_FAULT_STATUS);
		} else if (CRn == 6 && CRm == 0 && OPCODE_2 == 0) {
			data = R(CP15_FAULT_ADDRESS);
		}
		else {
			printf("mrc is not implementated. CRn is %d, CRm is %d, OPCODE_2 is %d\n", CRn, CRm, OPCODE_2);
//			exit(-1);
		}
	}
	if (data != NULL)
	{
		if (RD == 15)
		{
			Value *nzcv = LSHR(AND(data, CONST(0xf0000000)), CONST(28));
			Value *n = TRUNC1(AND(LSHR(nzcv, CONST(3)), CONST(1)));
			Value *z = TRUNC1(AND(LSHR(nzcv, CONST(2)), CONST(1)));
			Value *c = TRUNC1(AND(LSHR(nzcv, CONST(1)), CONST(1)));
			Value *v = TRUNC1(AND(LSHR(nzcv, CONST(0)), CONST(1)));
			new StoreInst(n, ptr_N, false, bb);
			new StoreInst(z, ptr_Z, false, bb);
			new StoreInst(c, ptr_C, false, bb);
			new StoreInst(v, ptr_V, false, bb);
		}
		else
			LET(RD,data);
	}
}
int DYNCOM_TRANS(mrrc)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(mrs)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	if (RD != 15) {
		if (BIT(22)) {
			//printf("in %s\n", __FUNCTION__);
			LET(RD, R(SPSR_REG));
		} else {
			SET_CPSR; // FIXME a doubt here, as CPSR is modified only in kernel mode
			LET(RD, R(CPSR_REG));
		}
	}
	else
	{
		UNPREDICTABLE(pc);
	}
}
int DYNCOM_TRANS(msr)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	SET_CPSR;

	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	Value *psr = CONST(0);
	Value *operand;
	uint32_t UnallocMask = 0x06f0fc00, UserMask = 0xf80f0200, PrivMask = 0x000001df, StateMask = 0x01000020;

	if (BIT(25)) {
		int rot_imm = BITS(8, 11) * 2;
		operand = ROTL(CONST(BITS(0, 7)), CONST(32 - rot_imm));
	} else {
		operand = R(RM);
	}
	uint32_t byte_mask = (BIT(16) ? 0xff : 0) | (BIT(17) ? 0xff00 : 0)
				| (BIT(18) ? 0xff0000 : 0) | (BIT(19) ? 0xff000000 : 0);
	uint32_t mask;
	if (!BIT(22)) {
		if (InAPrivilegedMode(core)) {
			mask = byte_mask & (UserMask | PrivMask);
		} else {
			mask = byte_mask & UserMask;
		}
		LET(CPSR_REG, OR(AND(R(CPSR_REG), COM(CONST(mask))), AND(operand, CONST(mask))));
		Value *nzcv = LSHR(AND(R(CPSR_REG), CONST(0xf0000000)), CONST(28));
		Value *n = TRUNC1(AND(LSHR(nzcv, CONST(3)), CONST(1)));
		Value *z = TRUNC1(AND(LSHR(nzcv, CONST(2)), CONST(1)));
		Value *c = TRUNC1(AND(LSHR(nzcv, CONST(1)), CONST(1)));
		Value *v = TRUNC1(AND(LSHR(nzcv, CONST(0)), CONST(1)));
		new StoreInst(n, ptr_N, false, bb);
		new StoreInst(z, ptr_Z, false, bb);
		new StoreInst(c, ptr_C, false, bb);
		new StoreInst(v, ptr_V, false, bb);
	} else {
		mask = byte_mask & (UserMask | PrivMask | StateMask);
		LET(SPSR_REG, OR(AND(R(SPSR_REG), COM(CONST(mask))), AND(operand, CONST(mask))));
	}
}
int DYNCOM_TRANS(mul)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	/* for 0x00, 0x01*/
	Value *op1 = R(RM);
	Value *op2 = R(RS);
	Value *ret = MUL(op1,op2);

	LET(MUL_RD, ret);

	if(SBIT)
	{
		GETSIGN(ret,ptr_N);
		EQZERO(ret,ptr_Z);
	}
}
int DYNCOM_TRANS(mvn)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	/* for 0x1e 0x1f 0x3e 0x3f */
	Value *op1 = R(RN);
	Value *op2 = SCO_OPERAND(SBIT ? ptr_C : NULL);
	Value *ret = XOR(op2, CONST(-1));
	LET(RD, ret);

	if(SBIT){
		if (RD == 15) {
			INCOMPLETE;
			/* CPSR = SPSR */
		} else {
			GETSIGN(ret,ptr_N);
			EQZERO(ret,ptr_Z);
			/* C in SCO_OPERAND */
		}
	}
}
int DYNCOM_TRANS(orr)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	/* for 0x18 0x19 0x38 0x39 */
	Value *op1 = R(RN);
	Value *op2 = SCO_OPERAND(SBIT ? ptr_C : NULL);
	Value *ret = OR(op2, op1);
	LET(RD, ret);

	if(SBIT) {
		if (RD == 15) {
			INCOMPLETE;
		} else {
			GETSIGN(ret,ptr_N);
			EQZERO(ret,ptr_Z);
			/* C in SCO_OPERAND */
		}
	}
}
int DYNCOM_TRANS(pkhbt)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(pkhtb)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(pld)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(qadd)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(qadd16)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(qadd8)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(qaddsubx)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(qdadd)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(qdsub)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(qsub)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(qsub16)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(qsub8)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(qsubaddx)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(rev)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(rev16)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	Value *op1 = R(RM);
	Value *tmp = SWAP32(R(RM));
	Value *ret = OR(SHL(tmp,CONST(16)), LSHR(tmp,CONST(16)));
	
	LET(RD,ret);
}
int DYNCOM_TRANS(revsh)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(rfe)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(rsb)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	/* for 0x06 0x07 0x26 0x27 */
	Value *op1 = R(RN);
	Value *op2 = OPERAND;
	Value *ret = SUB(op2, op1);
	LET(RD, ret);

	if(SBIT) {
		if (RD == 15) {
			INCOMPLETE;
		} else {
			GETSIGN(ret,ptr_N);
			EQZERO(ret,ptr_Z);
			NOTBORROWFROMSUB(op2,op1,ptr_C);
			OVERFLOWFROMSUB(op2,op1,ret,ptr_V);
		}
	}

}
int DYNCOM_TRANS(rsc)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	Value *op1 = R(RN);
	Value *op2 = OPERAND;
	Value *op3 = new LoadInst(ptr_C, "", false, bb);
	Value *carry = ZEXT32(NOT(op3));
	Value *ret = SUB(SUB(op2, op1), carry);
	LET(RD, ret);

	if(SBIT) {
		if (RD == 15) {
			INCOMPLETE;
		} else {
			GETSIGN(ret,ptr_N);
			EQZERO(ret,ptr_Z);
			NOTBORROWFROMSUB(op2,op1,ptr_C);
			OVERFLOWFROMSUB(op2,op1,ret,ptr_V);
		}
	}
}
int DYNCOM_TRANS(sadd16)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(sadd8)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(saddsubx)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(sbc)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	Value *op1 = R(RN);
	Value *op2 = OPERAND;
	Value *op3 = new LoadInst(ptr_C, "", false, bb);
	Value *carry = ZEXT32(NOT(op3));
	op2 = ADD(op2, carry);
	Value *ret = SUB(op1, op2);
	LET(RD, ret);
	if(SBIT) {
		if (RD == 15) {
			INCOMPLETE;
		} else {
			GETSIGN(ret,ptr_N);
			EQZERO(ret,ptr_Z);
			NOTBORROWFROMSUB(op1,op2,ptr_C);
			OVERFLOWFROMSUB(op1,op2,ret,ptr_V);
			#if 0
			Value *tmp1 = AND(ISNEG(op1), ISPOS(op2));
			Value *tmp2 = AND(ISNEG(op1), ISPOS(ret));
			Value *tmp3 = AND(ISPOS(op2), ISPOS(ret));
			Value *res = OR(OR(tmp1, tmp2), tmp3);
			new StoreInst(res, ptr_C, false, bb);
			#endif
		}
	}
}
int DYNCOM_TRANS(sel)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(setend)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(shadd16)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(shadd8)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(shaddsubx)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(shsub16)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(shsub8)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(shsubaddx)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(smla)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(smlad)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(smlal)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	Value *tmp1 = ZEXT64(R(RS));
	Value *tmp2 = ZEXT64(R(RM));
	Value *result = MUL(tmp1, tmp2);
	Value *result_lo = TRUNC32(AND(result, CONST64(0xffffffff)));
	Value *result_hi = TRUNC32(LSHR(AND(result, CONST64(0xffffffff00000000)), CONST64(32)));
	Value *rdlo = ADD(result_lo, R(RDLo));
	Value *carry = SELECT(ICMP_ULT(rdlo, result_lo), CONST(1), CONST(0));
	LET(RDLo, rdlo);
	LET(RDHi, ADD(ADD(result_hi, R(RDHi)), carry));
}
int DYNCOM_TRANS(smlalxy)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(smlald)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(smlaw)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(smlsd)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(smlsld)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(smmla)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(smmls)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(smmul)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(smuad)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(smul)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(smull)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	Value *rs64 = SEXT64(R(RS));
	Value *rm64 = SEXT64(R(RM));

	Value *result = MUL(rs64, rm64);
	Value *rdhi = LSHR(result, CONST64(32));
	Value *rdlo = AND(result, CONST64(0xffffffff));
	LET(BITS(16, 19), TRUNC32(rdhi));
	LET(BITS(12, 15), TRUNC32(rdlo));
	if (SBIT) {
		Value *cpsr_n = TRUNC1(LSHR(rdhi, CONST64(31)));
		new StoreInst(cpsr_n, ptr_N, bb);
		Value *cpsr_z = SELECT(ICMP_EQ(result, CONST64(0)), CONST1(1), CONST1(0));
		new StoreInst(cpsr_z, ptr_Z, bb);
	}
}
int DYNCOM_TRANS(smulw)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(smusd)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(srs)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(ssat)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(ssat16)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(ssub16)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(ssub8)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(ssubaddx)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(stc)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(stm)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	/* Store, WriteBack, PreDec */
	/* STM(1) P = 1, U = 0, W = 1 */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
}
int DYNCOM_TRANS(str)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	/*STR, No WriteBack, Pre Inc, Regist + Immed || Regist */
	/* I = 0, P = 1, U = 1, W = 0, B = 0 */
	Value *addr = GetAddr(cpu, instr, bb);
	//StoreWord(cpu, instr, bb, addr);
	LoadStore(cpu,instr,bb,addr);

}
int DYNCOM_TRANS(strb)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	/*STRB , No WriteBack, Pre Inc, Regist - Immed */
	/*I = 0, P = 1, U = 0, W = 0, B = 1 */

	/* STRB, No WriteBack, Pre Inc, Regist + Immed || Regist */
	/*  I = 0, P = 1, U = 1, B = 1, W = 0 */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
}
int DYNCOM_TRANS(strbt)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
	printf("in %s instruction is not fully implementated.\n", __FUNCTION__);
	return 0;
}
int DYNCOM_TRANS(strd)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	/* STRD P = 0, U = 0, I = 0, W = 0 */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
	return 0;
}
int DYNCOM_TRANS(strex)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
//	Value *addr = GetAddr(cpu, instr, bb);
//	LoadStore(cpu,instr,bb,addr);
	Value *addr = R(RN);
	Value *val = R(RM);
	arch_write_memory(cpu, bb, addr, val, 32);
	LET(RD, CONST(0));
	return 0;
}
int DYNCOM_TRANS(strexb)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
//	Value *addr = GetAddr(cpu, instr, bb);
//	LoadStore(cpu,instr,bb,addr);
	Value *addr = R(RN);
	Value *val = AND(R(RM), CONST(0xff));
	arch_write_memory(cpu, bb, addr, val, 8);
	LET(RD, CONST(0));
}
int DYNCOM_TRANS(strh)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	/* STRH register offset, no write-back, down, post indexed.  */
	/* P = 0, U = 0, I = 0, W = 0 */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
	return 0;
}
int DYNCOM_TRANS(strt)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
	printf("in %s instruction is not fully implementated.\n", __FUNCTION__);
	return 0;
}
int DYNCOM_TRANS(sub)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	if(RN == 15) {
		LET(15, ADD(R(15), CONST(8)));
		LET(PHYS_PC, R(15));
	}
	/* for 0x04 0x05 0x24 0x25 */
	Value *op1 = R(RN);
	Value *op2 = OPERAND;
	Value *ret = SUB(op1, op2);
	LET(RD, ret);

	if(SBIT) {
		if (RD == 15) {
			INCOMPLETE;
		} else {
			GETSIGN(ret,ptr_N);
			EQZERO(ret,ptr_Z);
			NOTBORROWFROMSUB(op1,op2,ptr_C);
			OVERFLOWFROMSUB(op1,op2,ret,ptr_V);
		}
	}
}
int DYNCOM_TRANS(swi)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
#ifdef OPT_LOCAL_REGISTERS
	/* Do nothing, we will finish syscall in trap */
	//LOG("OPT_LOCAL_REGISTERS swi inst\n");
#else
	arch_syscall(cpu, bb, BITS(0,19));
#endif
}
int DYNCOM_TRANS(swp)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(swpb)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(sxtab)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(sxtab16)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(sxtah)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(sxtb)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	#if 0
	Value *rotate = CONST(8 * BITS(10, 11));
	Value *mask = SELECT(ICMP_EQ(rotate, CONST(0)), CONST(0xffffffff), SUB(SHL(CONST(1), rotate), CONST(1)));
	Value *tmp1 = AND(R(RM), mask);
	Value *tmp2 = SHL(R(RM), rotate);
	Value *operand2 = OR(tmp1, tmp2);
	Value *operand3 = AND(operand2, CONST(0xff));
	Value *result = SELECT(ICMP_EQ(AND(operand3, CONST(0x80)), CONST(0)), operand3, OR(CONST(0xffffff00), operand3));
	#endif
	Value *tmp1 = ROTL(R(RM), CONST(8 * BITS(10, 11)));
	Value *tmp2 = AND(tmp1, CONST(0xff));
	//Value *tmp3 = TRUNC8(tmp2);
	//Value *result = SEXT32(tmp2);
	Value *result = SELECT(ICMP_EQ(AND(tmp2, CONST(0x80)), CONST(0)), tmp2, OR(CONST(0xffffff00), tmp2));
	LET(RD, result);
//	LET(RD, tmp2);
}
int DYNCOM_TRANS(sxtb16)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(sxth)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	#if 0
	Value *rotate = CONST(8 * BITS(10, 11));
	Value *mask = SELECT(ICMP_EQ(rotate, CONST(0)), CONST(0xffffffff), SUB(SHL(CONST(1), rotate), CONST(1)));
	Value *tmp1 = AND(R(RM), mask);
	Value *tmp2 = SHL(R(RM), rotate);
	Value *operand2 = OR(tmp1, tmp2);
	Value *result = SELECT(ICMP_EQ(AND(operand2, CONST(0x8000)), CONST(0)), operand2, OR(CONST(0xffff0000), operand2));
	#endif
	Value *tmp1 = ROTL(R(RM), CONST(8 * BITS(10, 11)));
	Value *tmp2 = AND(tmp1, CONST(0xffff));
	Value *result = SELECT(ICMP_EQ(AND(tmp2, CONST(0x8000)), CONST(0)), tmp2, OR(CONST(0xffff0000), tmp2));
//	Value *tmp3 = TRUNC16(tmp2);
//	Value *result = SEXT32(tmp2);
	LET(RD, result);
}
int DYNCOM_TRANS(teq)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	/* for 0x13, 0x33 */
	Value *op1 = R(RN);
	Value *op2 = SCO_OPERAND(ptr_C);

	if(RN == 15)
		op1 = ADD(op1, CONST(8));
	Value *ret = XOR(op1,op2);

	GETSIGN(ret,ptr_N);
	EQZERO(ret,ptr_Z);
	/* C in SCO_OPERAND */
}
int DYNCOM_TRANS(tst)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	/* for 0x11 0x31*/
	Value *op1 = R(RN);
	Value *op2 = SCO_OPERAND(ptr_C);
	Value *ret = AND(op1, op2);

	GETSIGN(ret,ptr_N);
	EQZERO(ret,ptr_Z);
	/* C in SCO_OPERAND */
}
int DYNCOM_TRANS(uadd16)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(uadd8)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(uaddsubx)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(uhadd16)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(uhadd8)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(uhaddsubx)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(uhsub16)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(uhsub8)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(uhsubaddx)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(umaal)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(umlal)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
#if 0
	/* for 0x08 0x09 */
	Value *op1 = R(RS);
	Value *op2 = R(RM);

	/* We can split the 32x32 into four 16x16 operations. This
	   ensures that we do not lose precision on 32bit only hosts.  */
	Value *Lo = MUL(AND(op1, CONST(0xFFFF)), AND(op2, CONST(0xFFFF)));
	Value *mid1 = MUL(AND(op1, CONST(0xFFFF)), AND(LSHR(op2, CONST(16)), CONST(0xFFFF)));
	Value *mid2 = MUL(AND(LSHR(op1, CONST(16)), CONST(0xFFFF)), AND(op2, CONST(0xFFFF)));
	Value *Hi = MUL(LSHR(op1, CONST(16)), AND(LSHR(op2, CONST(16)), CONST(0xFFFF)));

	/* We now need to add all of these results together, taking
	   care to propogate the carries from the additions.  */
	Value *and_op1 = Lo;
	Value *and_op2 = SHL(mid1, CONST(16));
	Value *tmp_RdLo = ADD(and_op1, and_op2);
	//Value *carry =  SELECT(ICMP_EQ(tmp_RdLo, and_op1), ICMP_NE(and_op2, CONST(0)), ICMP_ULT(tmp_RdLo, and_op1));
	Value *carry =  SELECT(ICMP_EQ(tmp_RdLo, and_op1), SELECT(ICMP_NE(and_op2, CONST(0)),CONST(1), CONST(0)), SELECT(ICMP_UGT(and_op1,tmp_RdLo), CONST(1), CONST(0)));

	and_op1 = tmp_RdLo;
	and_op2 = SHL(mid2, CONST(16));
	tmp_RdLo = ADD(and_op1, and_op2);
	carry =  ADD(carry, SELECT(ICMP_EQ(tmp_RdLo, and_op1), SELECT(ICMP_NE(and_op2, CONST(0)),CONST(1), CONST(0)), SELECT(ICMP_UGT(and_op1,tmp_RdLo), CONST(1), CONST(0))));

	Value *tmp_RdHi = ADD(ADD(LSHR(mid1, CONST(16)), LSHR(mid2, CONST(16))), Hi);
	tmp_RdHi = ADD(tmp_RdHi, carry);

	LET(RDLo, ADD(tmp_RdLo, R(RDLo)));
	LET(RDHi, ADD(tmp_RdHi, R(RDHi)));
#endif
	Value *tmp1 = ZEXT64(R(RS));
	Value *tmp2 = ZEXT64(R(RM));
	Value *result = MUL(tmp1, tmp2);
	Value *result_lo = TRUNC32(AND(result, CONST64(0xffffffff)));
	Value *result_hi = TRUNC32(LSHR(AND(result, CONST64(0xffffffff00000000)), CONST64(32)));
	Value *rdlo = ADD(result_lo, R(RDLo));
	Value *carry = SELECT(ICMP_ULT(rdlo, result_lo), CONST(1), CONST(0));
	LET(RDLo, rdlo);
	LET(RDHi, ADD(ADD(result_hi, R(RDHi)), carry));
}
int DYNCOM_TRANS(umull)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	/* for 0x08 0x09 */
	Value *op1 = R(RS);
	Value *op2 = R(RM);

	/* We can split the 32x32 into four 16x16 operations. This
	   ensures that we do not lose precision on 32bit only hosts.  */
	Value *Lo = MUL(AND(op1, CONST(0xFFFF)), AND(op2, CONST(0xFFFF)));
	Value *mid1 = MUL(AND(op1, CONST(0xFFFF)), AND(LSHR(op2, CONST(16)), CONST(0xFFFF)));
	Value *mid2 = MUL(AND(LSHR(op1, CONST(16)), CONST(0xFFFF)), AND(op2, CONST(0xFFFF)));
	Value *Hi = MUL(LSHR(op1, CONST(16)), AND(LSHR(op2, CONST(16)), CONST(0xFFFF)));

	/* We now need to add all of these results together, taking
	   care to propogate the carries from the additions.  */
	Value *and_op1 = Lo;
	Value *and_op2 = SHL(mid1, CONST(16));
	Value *tmp_RdLo = ADD(and_op1, and_op2);
	//Value *carry =  SELECT(ICMP_EQ(tmp_RdLo, and_op1), ICMP_NE(and_op2, CONST(0)), ICMP_ULT(tmp_RdLo, and_op1));
	Value *carry =  SELECT(ICMP_EQ(tmp_RdLo, and_op1), SELECT(ICMP_NE(and_op2, CONST(0)),CONST(1), CONST(0)), SELECT(ICMP_UGT(and_op1,tmp_RdLo), CONST(1), CONST(0)));

	and_op1 = tmp_RdLo;
	and_op2 = SHL(mid2, CONST(16));
	tmp_RdLo = ADD(and_op1, and_op2);
	carry =  ADD(carry, SELECT(ICMP_EQ(tmp_RdLo, and_op1), SELECT(ICMP_NE(and_op2, CONST(0)),CONST(1), CONST(0)), SELECT(ICMP_UGT(and_op1,tmp_RdLo), CONST(1), CONST(0))));

	Value *tmp_RdHi = ADD(ADD(LSHR(mid1, CONST(16)), LSHR(mid2, CONST(16))), Hi);
	tmp_RdHi = ADD(tmp_RdHi, carry);

	LET(RDLo, tmp_RdLo);
	LET(RDHi, tmp_RdHi);
}
int DYNCOM_TRANS(uqadd16)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(uqadd8)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(uqaddsubx)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(uqsub16)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(uqsub8)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(uqsubaddx)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(usad8)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(usada8)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(usat)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(usat16)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(usub16)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(usub8)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(usubaddx)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(uxtab)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	Value *tmp1 = ROTL(R(RM), CONST(32 - 8 * BITS(10, 11)));
	Value *tmp2 = AND(tmp1, CONST(0xff));

	LET(RD, ADD(R(RN), tmp2));
}
int DYNCOM_TRANS(uxtab16)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(uxtah)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
#if 0
	Value *rotate = CONST(8 * BITS(10, 11));
	Value *mask = SELECT(ICMP_EQ(rotate, CONST(0)), CONST(0xffffffff), SUB(SHL(CONST(1), rotate), CONST(1)));
	Value *tmp1 = AND(R(RM), mask);
	Value *tmp3 = SHL(tmp1, SUB(CONST(31), rotate));
	Value *tmp2 = SHL(R(RM), rotate);
	Value *tmp4 = LSHR(tmp2, rotate);
	Value *result = AND(OR(tmp3, tmp4), CONST(0xffff));
#endif
	Value *tmp1 = ROTL(R(RM), CONST(32 - 8 * BITS(10, 11)));
	Value *tmp2 = AND(tmp1, CONST(0xffff));

	LET(RD, ADD(R(RN), tmp2));
}
int DYNCOM_TRANS(uxtb)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
#if 0
	Value *rotate = CONST(8 * BITS(10, 11));
	Value *mask = SELECT(ICMP_EQ(rotate, CONST(0)), CONST(0xffffffff), SUB(SHL(CONST(1), rotate), CONST(1)));
	Value *tmp1 = AND(R(RM), mask);
	Value *tmp3 = SHL(tmp1, SUB(CONST(31), rotate));
	Value *tmp2 = SHL(R(RM), rotate);
	Value *tmp4 = LSHR(tmp2, rotate);
	Value *result = AND(OR(tmp3, tmp4), CONST(0xff));
#endif
	Value *tmp1 = ROTL(R(RM), CONST(8 * BITS(10, 11)));
	Value *tmp2 = AND(tmp1, CONST(0xff));
	//Value *tmp3 = TRUNC8(tmp2);
	//Value *result = SEXT32(tmp2);


	LET(RD, tmp2);
}
int DYNCOM_TRANS(uxtb16)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){}
int DYNCOM_TRANS(uxth)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc)
{
	#if 0
	Value *rotate = CONST(8 * BITS(10, 11));
	Value *mask = SELECT(ICMP_EQ(rotate, CONST(0)), CONST(0xffffffff), SUB(SHL(CONST(1), rotate), CONST(1)));
	Value *tmp1 = AND(R(RM), mask);
	Value *tmp3 = SHL(tmp1, SUB(CONST(31), rotate));
	Value *tmp2 = SHL(R(RM), rotate);
	Value *tmp4 = LSHR(tmp2, rotate);
	Value *result = AND(OR(tmp3, tmp4), CONST(0xffff));
	#endif
	Value *tmp1 = ROTL(R(RM), CONST(8 * BITS(10, 11)));
	Value *tmp2 = AND(tmp1, CONST(0xffff));
	LET(RD, tmp2);
}

int DYNCOM_TRANS(b_cond_thumb)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){
	uint32 tinstr = get_thumb_instr(instr, pc);
	uint32 imm = ((tinstr & 0x03FF) << 1) | ((tinstr & (1 << 10)) ? 0xFF800000 : 0);
	printf("In %s, pc=0x%x, imm=0x%x, instr=0x%x, tinstr=0x%x\n", __FUNCTION__, pc, imm, instr, tinstr);
	//LET(14, ADD(ADD(R(15), CONST(4)), CONST(imm)));
	LET(15, ADD(R(15), CONST(4 + imm)));
	/* return the instruction size */
	return Byte_2;
}

int DYNCOM_TRANS(bl_1_thumb)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){
	uint32 tinstr = get_thumb_instr(instr, pc);
	uint32 imm = (((tinstr & 0x07FF) << 12) | ((tinstr & (1 << 10)) ? 0xFF800000 : 0));
	LET(14, ADD(ADD(R(15), CONST(4)), CONST(imm)));
	/* return the instruction size */
	return Byte_2;
}
int DYNCOM_TRANS(bl_2_thumb)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){
	uint32 tinstr = get_thumb_instr(instr, pc);
	uint32 imm = (tinstr & 0x07FF) << 1;
	LET(15, ADD(R(14), CONST(imm)));
	LET(14, CONST((pc + 2) | 1));
	/* Clear thumb bit */
	STORE(CONST(0), ptr_T);	

	return Byte_2;
}
int DYNCOM_TRANS(blx_1_thumb)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){
	uint32 tinstr = get_thumb_instr(instr, pc);
	uint32 imm = (tinstr & 0x07FF) << 1;
	LET(15, AND(ADD(R(14), CONST(imm)), CONST(~0x3)));
	LET(14, CONST((pc + 2) | 1));
	/* Clear thumb bit */
	STORE(CONST(0), ptr_T);	
	return Byte_2;
}
//end of translation
int DYNCOM_TAG(adc)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = 4;
	if (RD == 15) {
		arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
		*new_pc = NEW_PC_NONE;
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}

int DYNCOM_TAG(add)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	if (RD == 15) {
		arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
		*new_pc = NEW_PC_NONE;
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(and)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	if (RD == 15) {
		arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
		*new_pc = NEW_PC_NONE;
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(bbl)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
	if(!is_user_mode(cpu))
		*new_pc = NEW_PC_NONE;
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(bic)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	if (RD == 15) {
		arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
		*new_pc = NEW_PC_NONE;
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(bkpt)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(blx)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
//	printf("pc is %x in %s instruction is not implementated.\n", pc ,__FUNCTION__);
	arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
	if(!is_user_mode(cpu))
		*new_pc = NEW_PC_NONE;
	/* Thumb mode should be enter */
	*new_pc = NEW_PC_NONE;

	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
// for kernel
//	exit(-1);
	return instr_size;
}
int DYNCOM_TAG(bx)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	if (RD == 15) {
		arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
		*new_pc = NEW_PC_NONE;
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(bxj)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(cdp)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	int instr_size = INSTR_SIZE;
	printf("in %s instruction is not implementated.\n", __FUNCTION__);
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(clrex)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	printf("in %s instruction is not implementated.\n", __FUNCTION__);
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
//	exit(-1);
	return instr_size;
}
int DYNCOM_TAG(clz)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	//printf("in %s instruction is not implementated.\n", __FUNCTION__);
	//exit(-1);
	return instr_size;
}
int DYNCOM_TAG(cmn)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	if (RD == 15) {
		arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
		*new_pc = NEW_PC_NONE;
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(cmp)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	if (RD == 15) {
		arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
		*new_pc = NEW_PC_NONE;
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(cps)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	//printf("in %s instruction is not implementated.\n", __FUNCTION__); // UNC
//	exit(-1);
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
//	arm_tag_stop(cpu, pc, instr, tag, new_pc, next_pc);
	*tag |= TAG_STOP;
	if (BIT(17)) {
		*tag |= TAG_STOP;
		//arm_tag_stop(cpu, pc, instr, tag, new_pc, next_pc);
	}
	//*tag = TAG_CONTINUE;
	*next_pc = pc + 4;
#if 0
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
#endif
	return instr_size;
}
int DYNCOM_TAG(cpy)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	if (RD == 15) {
		arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
		*new_pc = NEW_PC_NONE;
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(eor)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	if (RD == 15) {
		arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
		*new_pc = NEW_PC_NONE;
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(ldc)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	printf("in %s instruction is not implementated.\n", __FUNCTION__);
//	exit(-1);
	return instr_size;
}
int DYNCOM_TAG(ldm)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	if (BIT(15)) {
		arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
		*new_pc = NEW_PC_NONE;
		if (BITS(25, 27) == 4 && BIT(22) == 1 && BIT(20) == 1 && BIT(15) == 1) {
			*tag |= TAG_TRAP;
		}
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	*tag |= TAG_MEMORY;
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(ldr)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	if (RD == 15) {
		arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
		*new_pc = NEW_PC_NONE;
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	*tag |= TAG_MEMORY;
//	if (instr == 0xe59f0054 && (pc & 0xfff) == 0xffc) {
	if ((pc & 0xfff) == 0xffc) {
		*tag |= TAG_NEED_PC;
	}
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(ldrb)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	if (RD == 15) {
		arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
		*new_pc = NEW_PC_NONE;
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	*tag |= TAG_MEMORY;
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(ldrbt)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	printf("in %s instruction is not implementated.\n", __FUNCTION__);
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	*tag |= TAG_MEMORY;
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(ldrd)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	printf("in %s instruction is not implementated.\n", __FUNCTION__);
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	*tag |= TAG_MEMORY;
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(ldrex)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
//	printf("in %s instruction is not implementated.\n", __FUNCTION__);
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	*tag |= TAG_MEMORY;
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
//	exit(-1);
	return instr_size;
}
int DYNCOM_TAG(ldrexb)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	printf("in %s instruction is not implementated.\n", __FUNCTION__);
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	*tag |= TAG_MEMORY;
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(ldrh)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	if (RD == 15) {
		arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
		*new_pc = NEW_PC_NONE;
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	*tag |= TAG_MEMORY;
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(ldrsb)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	*tag |= TAG_MEMORY;
	#if 0
	printf("in %s instruction is not implementated.\n", __FUNCTION__);
	printf("icounter is %x\n", cpu->icounter);
	printf("pc is %x\n", pc);
	printf("instr is %x\n", instr);
	exit(-1);
	#endif
	return instr_size;
}
int DYNCOM_TAG(ldrsh)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	if (RD == 15) {
		arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
		*new_pc = NEW_PC_NONE;
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	*tag |= TAG_MEMORY;
//	exit(-1);
	return instr_size;
}
int DYNCOM_TAG(ldrt)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	printf("in %s instruction is not implementated.\n", __FUNCTION__);
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	*tag |= TAG_MEMORY;
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(mcr)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	#if 0
	printf("icounter is %x\n", cpu->icounter);
	printf("pc is %x\n", pc);
	printf("instr is %x\n", instr);
	printf("in %s instruction is not implementated.\n", __FUNCTION__);
	exit(-1);
	#endif
	//FIXME wfs will be here.
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	return instr_size;
}
int DYNCOM_TAG(mcrr)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(mla)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	if (RD == 15) {
		arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
		*new_pc = NEW_PC_NONE;
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(mov)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	if (RD == 15) {
		arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
		if (SBIT) {
			//arm_tag_trap(cpu, pc, instr, tag, new_pc, next_pc);
			//*tag |= TAG_STOP;
			*tag |= TAG_TRAP;
		} else
			*tag |= TAG_STOP;
		*new_pc = NEW_PC_NONE;
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(mrc)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	if (instr == 0xeef04a10) {
		arm_tag_trap(cpu, pc, instr, tag, new_pc, next_pc);
		return instr_size;
	}
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	#if 0
	printf("icounter is %x\n", cpu->icounter);
	printf("pc is %x\n", pc);
	printf("instr is %x\n", instr);
	printf("in %s instruction is not implementated.\n", __FUNCTION__);
	#endif
//	exit(-1);
	//FIXME: rfs instruction will be here.
	return instr_size;
}
int DYNCOM_TAG(mrrc)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
//	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	printf("in %s instruction is not implementated.\n", __FUNCTION__);
	exit(-1);
	return instr_size;
}
int DYNCOM_TAG(mrs)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	//printf("in %s instruction is not implementated.\n", __FUNCTION__); // UNC
	//exit(-1);
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;

	return instr_size;
}
int DYNCOM_TAG(msr)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
//	if (BIT(16)) {
		arm_tag_trap(cpu, pc, instr, tag, new_pc, next_pc);
//	} else {
//		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
		*tag |= TAG_STOP;
//	}
	//printf("in %s instruction is not implementated.\n", __FUNCTION__);
	//exit(-1);
	return instr_size;
}
int DYNCOM_TAG(mul)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
//	printf("in %s instruction is not implementated.\n", __FUNCTION__);
//	exit(-1);
	return instr_size;
}
int DYNCOM_TAG(mvn)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	if (RD == 15) {
		arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
		*new_pc = NEW_PC_NONE;
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(orr)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	if (RD == 15) {
		arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
		*new_pc = NEW_PC_NONE;
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(pkhbt)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(pkhtb)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(pld)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	int instr_size = INSTR_SIZE;
	printf("in %s instruction is not implementated.\n", __FUNCTION__);
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(qadd)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(qadd16)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(qadd8)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(qaddsubx)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(qdadd)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(qdsub)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(qsub)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(qsub16)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(qsub8)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(qsubaddx)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(rev)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(rev16)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	int instr_size = INSTR_SIZE;
	if ((RD == 15) || (RM == 15)) {
		printf("in %s instruction is not implementated (Unpredictable).\n", __FUNCTION__);
		exit(-1);
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	printf("in %s instruction is not implementated.\n", __FUNCTION__);
	return instr_size;
}
int DYNCOM_TAG(revsh)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(rfe)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(rsb)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	if (RD == 15) {
		arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
		*new_pc = NEW_PC_NONE;
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(rsc)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	if (RD == 15) {
		arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
		*new_pc = NEW_PC_NONE;
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(sadd16)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(sadd8)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(saddsubx)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(sbc)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	if (RD == 15) {
		arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
		*new_pc = NEW_PC_NONE;
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(sel)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(setend)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(shadd16)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(shadd8)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(shaddsubx)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(shsub16)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(shsub8)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(shsubaddx)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(smla)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(smlad)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(smlal)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
//	printf("in %s instruction is not implementated.\n", __FUNCTION__);
//	exit(-1);
	return instr_size;
}
int DYNCOM_TAG(smlalxy)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(smlald)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(smlaw)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(smlsd)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(smlsld)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(smmla)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(smmls)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(smmul)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(smuad)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(smul)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(smull)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	#if 0
	printf("icounter is %x\n", cpu->icounter);
	printf("pc is %x\n", pc);
	printf("instr is %x\n", instr);
	printf("in %s instruction is not implementated.\n", __FUNCTION__);
	exit(-1);
	#endif
	return instr_size;
}
int DYNCOM_TAG(smulw)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(smusd)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(srs)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(ssat)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(ssat16)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(ssub16)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(ssub8)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(ssubaddx)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(stc)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	printf("in %s instruction is not implementated.\n", __FUNCTION__);
//	exit(-1);
	return instr_size;
}
int DYNCOM_TAG(stm)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	*tag |= TAG_MEMORY;
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(str)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	*tag |= TAG_MEMORY;
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(strb)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	*tag |= TAG_MEMORY;
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(strbt)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	*tag |= TAG_MEMORY;
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	printf("in %s instruction is not implementated.\n", __FUNCTION__);
	return instr_size;
}
int DYNCOM_TAG(strd)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	int instr_size = INSTR_SIZE;
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	*tag |= TAG_MEMORY;
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	printf("in %s instruction is not implementated.\n", __FUNCTION__);
	return instr_size;
}
int DYNCOM_TAG(strex)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
//	printf("in %s instruction is not implementated.\n", __FUNCTION__);
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	*tag |= TAG_MEMORY;
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
//	exit(-1);
	return instr_size;
}
int DYNCOM_TAG(strexb)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	printf("in %s instruction is not implementated.\n", __FUNCTION__);
//	exit(-1);
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	*tag |= TAG_MEMORY;
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(strh)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	*tag |= TAG_MEMORY;
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(strt)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	printf("in %s instruction is not implementated.\n", __FUNCTION__);
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	*tag |= TAG_MEMORY;
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(sub)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	if (RD == 15) {
		arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
		*new_pc = NEW_PC_NONE;
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(swi)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	arm_tag_trap(cpu, pc, instr, tag, new_pc, next_pc);
//	*tag |= TAG_STOP;
	return instr_size;
}
int DYNCOM_TAG(swp)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	int instr_size = INSTR_SIZE;
	printf("in %s instruction is not implementated, %x\n", __FUNCTION__, instr);
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(swpb)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	int instr_size = INSTR_SIZE;
	printf("in %s instruction is not implementated, %x\n", __FUNCTION__, instr);
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(sxtab)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(sxtab16)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(sxtah)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(sxtb)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
//	printf("in %s instruction is not implementated.\n", __FUNCTION__);
//	exit(-1);
	return instr_size;
}
int DYNCOM_TAG(sxtb16)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(sxth)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	printf("in %s instruction is not implementated.\n", __FUNCTION__);
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
//	exit(-1);
	return instr_size;
}
int DYNCOM_TAG(teq)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	if (RD == 15) {
		arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
		*new_pc = NEW_PC_NONE;
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(tst)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	if (RD == 15) {
		arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
		*new_pc = NEW_PC_NONE;
	} else {
		arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	}
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(uadd16)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(uadd8)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(uaddsubx)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(uhadd16)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(uhadd8)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(uhaddsubx)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(uhsub16)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(uhsub8)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(uhsubaddx)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(umaal)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(umlal)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
//	printf("in %s instruction is not implementated.\n", __FUNCTION__);
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;

//	exit(-1);
	return instr_size;
}
int DYNCOM_TAG(umull)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(uqadd16)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(uqadd8)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(uqaddsubx)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(uqsub16)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(uqsub8)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(uqsubaddx)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(usad8)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(usada8)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(usat)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(usat16)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(usub16)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(usub8)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(usubaddx)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(uxtab)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	printf("in %s instruction is not implementated.\n", __FUNCTION__);
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
//	exit(-1);
	return instr_size;
}
int DYNCOM_TAG(uxtab16)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(uxtah)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
//	printf("in %s instruction is not implementated.\n", __FUNCTION__);
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
//	exit(-1);
	return instr_size;
}
int DYNCOM_TAG(uxtb)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	printf("in %s instruction is not implementated.\n", __FUNCTION__);
//	exit(-1);
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	return instr_size;
}
int DYNCOM_TAG(uxtb16)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){int instr_size = INSTR_SIZE;printf("in %s instruction is not implementated.\n", __FUNCTION__);exit(-1);return instr_size;}
int DYNCOM_TAG(uxth)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	printf("in %s instruction is not implementated.\n", __FUNCTION__);
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
	//exit(-1);
	return instr_size;
}

int DYNCOM_TAG(b_cond_thumb)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = 2;
	printf("pc is %x in %s instruction is not implementated.\n", pc, __FUNCTION__);
	*tag = TAG_CONTINUE;
	*tag |= TAG_BRANCH;
	*tag |= TAG_CONDITIONAL;
	*next_pc = pc + INSTR_SIZE;
	return instr_size;
}

int DYNCOM_TAG(bl_1_thumb)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = 2;
	printf("pc is %x in %s instruction is not implementated.\n", pc, __FUNCTION__);
	arm_tag_continue(cpu, pc, instr, tag, new_pc, next_pc);
	return instr_size;
}
int DYNCOM_TAG(bl_2_thumb)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = 2;
	arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
	*tag |= TAG_STOP;
 
	printf("pc is %x in %s instruction is not implementated.\n", pc, __FUNCTION__);
	return instr_size;
}
int DYNCOM_TAG(blx_1_thumb)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = 2;
	arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
	*tag |= TAG_STOP;


	printf("pc is %x in %s instruction is not implementated.\n", pc, __FUNCTION__);
	return instr_size;
}
/* Floating point instructions */
int DYNCOM_TAG(fmrx)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	int instr_size = INSTR_SIZE;
	printf("\t\tin %s instruction is not implementated.\n", __FUNCTION__);
	arm_tag_trap(cpu, pc, instr, tag, new_pc, next_pc);
	return instr_size;
}
int DYNCOM_TRANS(fmrx)(cpu_t *cpu, uint32_t instr, BasicBlock *bb, addr_t pc){
	printf("\t\tin %s instruction is not implementated.\n", __FUNCTION__);
	arch_arm_undef(cpu, bb, instr);
}

const INSTRACT arm_instruction_action[] = {
	DYNCOM_FILL_ACTION(adc),
	DYNCOM_FILL_ACTION(add),
	DYNCOM_FILL_ACTION(and),
	DYNCOM_FILL_ACTION(bbl),
	DYNCOM_FILL_ACTION(bic),
	DYNCOM_FILL_ACTION(bkpt),
	DYNCOM_FILL_ACTION(blx),
	DYNCOM_FILL_ACTION(blx),
	DYNCOM_FILL_ACTION(bx),
	DYNCOM_FILL_ACTION(bxj),
	DYNCOM_FILL_ACTION(cdp),
	DYNCOM_FILL_ACTION(clrex),
	DYNCOM_FILL_ACTION(clz),
	DYNCOM_FILL_ACTION(cmn),
	DYNCOM_FILL_ACTION(cmp),
	DYNCOM_FILL_ACTION(cps),
	DYNCOM_FILL_ACTION(cpy),
	DYNCOM_FILL_ACTION(eor),
	DYNCOM_FILL_ACTION(ldc),
	DYNCOM_FILL_ACTION(ldm),
	DYNCOM_FILL_ACTION(ldm),
	DYNCOM_FILL_ACTION(ldm),
	DYNCOM_FILL_ACTION(sxth),
	DYNCOM_FILL_ACTION(uxth),
	DYNCOM_FILL_ACTION(uxtah),
	DYNCOM_FILL_ACTION(rev),
	DYNCOM_FILL_ACTION(rev16),
	DYNCOM_FILL_ACTION(revsh),
	DYNCOM_FILL_ACTION(ldrbt),
	DYNCOM_FILL_ACTION(ldrt),
	DYNCOM_FILL_ACTION(ldr),
	DYNCOM_FILL_ACTION(pld),
	DYNCOM_FILL_ACTION(ldrb),
	DYNCOM_FILL_ACTION(ldrd),
	DYNCOM_FILL_ACTION(ldrex),
	DYNCOM_FILL_ACTION(ldrexb),
	DYNCOM_FILL_ACTION(ldrh),
	DYNCOM_FILL_ACTION(ldrsb),
	DYNCOM_FILL_ACTION(ldrsh),
	DYNCOM_FILL_ACTION(mcr),
	DYNCOM_FILL_ACTION(mcrr),
	DYNCOM_FILL_ACTION(mla),
	DYNCOM_FILL_ACTION(mov),
	DYNCOM_FILL_ACTION(mrc),
	DYNCOM_FILL_ACTION(mrrc),
	DYNCOM_FILL_ACTION(mrs),
	DYNCOM_FILL_ACTION(msr),
	DYNCOM_FILL_ACTION(msr),
	DYNCOM_FILL_ACTION(mul),
	DYNCOM_FILL_ACTION(mvn),
	DYNCOM_FILL_ACTION(orr),
	DYNCOM_FILL_ACTION(pkhbt),
	DYNCOM_FILL_ACTION(pkhtb),
	DYNCOM_FILL_ACTION(qadd),
	DYNCOM_FILL_ACTION(qadd16),
	DYNCOM_FILL_ACTION(qadd8),
	DYNCOM_FILL_ACTION(qaddsubx),
	DYNCOM_FILL_ACTION(qdadd),
	DYNCOM_FILL_ACTION(qdsub),
	DYNCOM_FILL_ACTION(qsub),
	DYNCOM_FILL_ACTION(qsub16),
	DYNCOM_FILL_ACTION(qsub8),
	DYNCOM_FILL_ACTION(qsubaddx),
	DYNCOM_FILL_ACTION(rfe),
	DYNCOM_FILL_ACTION(rsb),
	DYNCOM_FILL_ACTION(rsc),
	DYNCOM_FILL_ACTION(sadd16),
	DYNCOM_FILL_ACTION(sadd8),
	DYNCOM_FILL_ACTION(saddsubx),
	DYNCOM_FILL_ACTION(sbc),
	DYNCOM_FILL_ACTION(sel),
	DYNCOM_FILL_ACTION(setend),
	DYNCOM_FILL_ACTION(shadd16),
	DYNCOM_FILL_ACTION(shadd8),
	DYNCOM_FILL_ACTION(shaddsubx),
	DYNCOM_FILL_ACTION(shsub16),
	DYNCOM_FILL_ACTION(shsub8),
	DYNCOM_FILL_ACTION(shsubaddx),
	DYNCOM_FILL_ACTION(smla),
	DYNCOM_FILL_ACTION(smlad),
	DYNCOM_FILL_ACTION(smlal),
	DYNCOM_FILL_ACTION(smlalxy),
	DYNCOM_FILL_ACTION(smlald),
	DYNCOM_FILL_ACTION(smlaw),
	DYNCOM_FILL_ACTION(smlsd),
	DYNCOM_FILL_ACTION(smlsld),
	DYNCOM_FILL_ACTION(smmla),
	DYNCOM_FILL_ACTION(smmls),
	DYNCOM_FILL_ACTION(smmul),
	DYNCOM_FILL_ACTION(smuad),
	DYNCOM_FILL_ACTION(smul),
	DYNCOM_FILL_ACTION(smull),
	DYNCOM_FILL_ACTION(smulw),
	DYNCOM_FILL_ACTION(smusd),
	DYNCOM_FILL_ACTION(srs),
	DYNCOM_FILL_ACTION(ssat),
	DYNCOM_FILL_ACTION(ssat16),
	DYNCOM_FILL_ACTION(ssub16),
	DYNCOM_FILL_ACTION(ssub8),
	DYNCOM_FILL_ACTION(ssubaddx),
	DYNCOM_FILL_ACTION(stc),
	DYNCOM_FILL_ACTION(stm),
	DYNCOM_FILL_ACTION(stm),
	DYNCOM_FILL_ACTION(sxtb),
	DYNCOM_FILL_ACTION(uxtb),
	DYNCOM_FILL_ACTION(uxtab),
	DYNCOM_FILL_ACTION(strbt),
	DYNCOM_FILL_ACTION(strt),
	DYNCOM_FILL_ACTION(str),
	DYNCOM_FILL_ACTION(strb),
	DYNCOM_FILL_ACTION(strd),
	DYNCOM_FILL_ACTION(strex),
	DYNCOM_FILL_ACTION(strexb),
	DYNCOM_FILL_ACTION(strh),
	DYNCOM_FILL_ACTION(sub),
	DYNCOM_FILL_ACTION(swi),
	DYNCOM_FILL_ACTION(swp),
	DYNCOM_FILL_ACTION(swpb),
	DYNCOM_FILL_ACTION(sxtab),
	DYNCOM_FILL_ACTION(sxtab16),
	DYNCOM_FILL_ACTION(sxtah),
	DYNCOM_FILL_ACTION(sxtb16),
	DYNCOM_FILL_ACTION(teq),
	DYNCOM_FILL_ACTION(tst),
	DYNCOM_FILL_ACTION(uadd16),
	DYNCOM_FILL_ACTION(uadd8),
	DYNCOM_FILL_ACTION(uaddsubx),
	DYNCOM_FILL_ACTION(uhadd16),
	DYNCOM_FILL_ACTION(uhadd8),
	DYNCOM_FILL_ACTION(uhaddsubx),
	DYNCOM_FILL_ACTION(uhsub16),
	DYNCOM_FILL_ACTION(uhsub8),
	DYNCOM_FILL_ACTION(uhsubaddx),
	DYNCOM_FILL_ACTION(umaal),
	DYNCOM_FILL_ACTION(umlal),
	DYNCOM_FILL_ACTION(umull),
	DYNCOM_FILL_ACTION(uqadd16),
	DYNCOM_FILL_ACTION(uqadd8),
	DYNCOM_FILL_ACTION(uqaddsubx),
	DYNCOM_FILL_ACTION(uqsub16),
	DYNCOM_FILL_ACTION(uqsub8),
	DYNCOM_FILL_ACTION(uqsubaddx),
	DYNCOM_FILL_ACTION(usad8),
	DYNCOM_FILL_ACTION(usada8),
	DYNCOM_FILL_ACTION(usat),
	DYNCOM_FILL_ACTION(usat16),
	DYNCOM_FILL_ACTION(usub16),
	DYNCOM_FILL_ACTION(usub8),
	DYNCOM_FILL_ACTION(usubaddx),
	DYNCOM_FILL_ACTION(uxtab16),
	DYNCOM_FILL_ACTION(uxtb16),

	/* The thumb instruction here */	
	DYNCOM_FILL_ACTION(b_cond_thumb),
	DYNCOM_FILL_ACTION(bl_1_thumb),
	DYNCOM_FILL_ACTION(bl_2_thumb),
	DYNCOM_FILL_ACTION(blx_1_thumb)

};
/**
* @brief Translate the thumb instruction to arm instruction
*
* @param cpu
* @param inst
* @param arm_inst
* @param inst_size
* @param br
*
* @return 
*/
static tdstate decode_dyncom_thumb_instr(arm_core_t *core, uint32_t inst, uint32_t *arm_inst, addr_t pc, int* index){
	/* Check if in Thumb mode.  */
	tdstate ret;
	uint32 inst_size;
	/* set translate_pc for thumb_translate function */
	core->translate_pc = pc;
	ret = thumb_translate (core, inst, arm_inst, &inst_size);
	/* For BL thumb instruction, should set its index directly */
	if(ret == t_branch){
		/* FIXME, endian should be judged */
		uint32 tinstr;
		tinstr = get_thumb_instr(inst, pc);

		//tinstr = inst & 0xFFFF;
		//printf("In %s t_branch, inst=0x%x, tinst=0x%x\n", __FUNCTION__, inst, tinstr);
		int inst_index;
		/* table_length */
		int table_length = sizeof(arm_instruction_action) / sizeof(struct instruction_action);

		switch((tinstr & 0xF800) >> 11){
		/* we will translate the thumb instruction directly here */
		case 26:
		case 27:
			if (((tinstr & 0x0F00) != 0x0E00) && ((tinstr & 0x0F00) != 0x0F00)){
				uint32 cond = (tinstr & 0x0F00) >> 8;
				*index = table_length - 4;
			}
			else{
			/* something wrong */
			}
		case 29:
			/* For BLX 1 thumb instruction*/
			*index = table_length - 1;
			printf("In %s, tinstr=0x%x, blx 1 thumb index=%d\n", __FUNCTION__, tinstr, inst_index);
			break;
		case 30:
			/* For BL 1 thumb instruction*/
			*index = table_length - 3;
			printf("In %s, tinstr=0x%x, bl 1 thumb index=%d\n", __FUNCTION__, tinstr, inst_index);
			break;
		case 31:
			/* For BL 2 thumb instruction*/
			*index = table_length - 2;
			printf("In %s, tinstr=0x%x, bl 2 thumb index=%d\n", __FUNCTION__, tinstr, inst_index);
			break;
		default:
			ret = t_undefined;
			break;
		}
	}
	return ret;
}

