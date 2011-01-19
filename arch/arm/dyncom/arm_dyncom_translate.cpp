/*
 * libcpu: arm_translate.cpp
 *
 * main translation code
 */

#include "llvm/Instructions.h"

#include "skyeye_dyncom.h"
#include "dyncom/dyncom_llvm.h"
#include "dyncom/frontend.h"
#include "arm_internal.h"
#include "arm_types.h"
#include "dyncom/tag.h"
#include "bank_defs.h"
#include "arm_dyncom_dec.h"

#define ptr_N	cpu->ptr_N
#define ptr_Z	cpu->ptr_Z
#define ptr_C	cpu->ptr_C
#define ptr_V	cpu->ptr_V
#define ptr_I 	cpu->ptr_I
using namespace llvm;

int arm_tag_branch(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc);
typedef int (*tag_func_t)(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc);
typedef int (*translate_func_t)(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
typedef Value* (*translate_cond_func_t)(cpu_t *cpu, addr_t pc, BasicBlock *bb);
typedef struct arm_opc_func_s{
        tag_func_t tag;
        translate_func_t translate;
        translate_cond_func_t translate_cond;
}arm_opc_func_t;
arm_opc_func_t* arm_get_opc_func(uint32_t opc);
Value *arm_translate_cond(cpu_t *cpu, uint32_t instr, BasicBlock *bb);

arm_opc_func_t  arm_opc_table_main[0xff] ;

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

#define INSTR_SIZE 4

#define BAD_INSTR {fprintf(stderr, "In %s, cannot parse instruction 0x%x\n", __FUNCTION__, instr);exit(-1);}
int opc_invalid_tag(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	BAD_INSTR;
	return -1;
}
int opc_invalid_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb){
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

Value * arch_arm_translate_cond(cpu_t *cpu, addr_t pc, BasicBlock *bb)
{
	uint32_t instr;
	bus_read(32, pc, &instr);
	arm_opc_func_t *opc_func = arm_get_opc_func(instr);
	return opc_func->translate_cond(cpu, instr, bb);
}

#define BIT(n) ((instr >> (n)) & 1)
#define BITS(a,b) ((instr >> (a)) & ((1 << (1+(b)-(a)))-1))
int arch_arm_tag_instr(cpu_t *cpu, addr_t pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc) {
        int instr_size = INSTR_SIZE;
        uint32_t instr;
        bus_read(32, pc, &instr);
        arm_opc_func_t *opc_func = arm_get_opc_func(instr);
        if (instr){
                if((BITS(24,27) != 0x9) && (BITS(24,27) != 0x8) && (BITS(24,27) != 0xb) &&(BITS(24,27) != 0xa) && BITS(12,15) == 0xf){
                        arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
			*new_pc = NEW_PC_NONE;
		}else if(((BITS(24,27) == 0x9) || (BITS(24,27)) == 0x8) && BIT(15) && BIT(20)){
                        arm_tag_branch(cpu, pc, instr, tag, new_pc, next_pc);
			*new_pc = NEW_PC_NONE;
		}
                else
                        opc_func->tag(cpu, pc, instr, tag, new_pc, next_pc);
        }
        else
                *tag |= TAG_STOP;

        return instr_size;
}
#undef BITS
#undef BIT


int arch_arm_translate_instr(cpu_t *cpu, addr_t pc, BasicBlock *bb) {
	int instr_size = INSTR_SIZE;
	uint32_t instr;
	if(bus_read(32, pc, &instr)){

	}
	arm_opc_func_t *opc_func = arm_get_opc_func(instr);
	opc_func->translate(cpu, instr, bb);

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

void arm_opc_func_init()
{
	//*arm_opc_table =(arm_opc_func_t *) malloc(sizeof(arm_opc_func_t) * 0xff);
	arm_opc_func_t *arm_opc_table = arm_opc_table_main;
	int i;
	for(i = 0; i < 0x100; i ++){
		arm_opc_table[i].translate_cond = arm_translate_cond;
	}

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

#define instr_size 4
int arm_tag_continue(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	*tag = TAG_CONTINUE;
	*next_pc = pc + instr_size;
	if(instr >> 28 != 0xe)
		*tag = TAG_CONDITIONAL;
}

int arm_tag_stop(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	*tag = TAG_STOP;
	*next_pc = pc + instr_size;
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
}
int arm_tag_ret(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	*tag = TAG_RET;
	*next_pc = pc + instr_size;
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
}

#define ARM_BRANCH_TARGET (BIT(23)?(pc - ((~(BITS(0,23) << 8) >> 6) + 1) + 8):(((BITS(0,23) << 8) >> 6) + pc + 8))
int arm_tag_call(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	uint32_t opc = instr;
	*tag = TAG_CALL;
	*new_pc = ARM_BRANCH_TARGET;
	*next_pc = pc + instr_size;
	if(instr >> 28 != 0xe)
		*tag |= TAG_CONDITIONAL;
}

int arm_tag_branch(cpu_t *cpu, addr_t pc, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	uint32_t opc = instr;
	*tag = TAG_BRANCH;
	*new_pc = ARM_BRANCH_TARGET;
	*next_pc = pc + instr_size;
	if(instr >> 28 != 0xe)
	{
		*tag |= TAG_CONDITIONAL;
	}
}

int init_arm_opc_group0(arm_opc_func_t* arm_opc_table)
{
	{
		(arm_opc_table)[0x0].translate = arm_opc_trans_00;
		(arm_opc_table)[0x0].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x1].translate = arm_opc_trans_01;
		(arm_opc_table)[0x1].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x2].translate = arm_opc_trans_02;
		(arm_opc_table)[0x2].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x3].translate = arm_opc_trans_03;
		(arm_opc_table)[0x3].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x4].translate = arm_opc_trans_04;
		(arm_opc_table)[0x4].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x5].translate = arm_opc_trans_05;
		(arm_opc_table)[0x5].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x6].translate = arm_opc_trans_06;
		(arm_opc_table)[0x6].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x7].translate = arm_opc_trans_07;
		(arm_opc_table)[0x7].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x8].translate = arm_opc_trans_08;
		(arm_opc_table)[0x8].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x9].translate = arm_opc_trans_09;
		(arm_opc_table)[0x9].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xa].translate = arm_opc_trans_0a;
		(arm_opc_table)[0xa].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xb].translate = arm_opc_trans_0b;
		(arm_opc_table)[0xb].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xc].translate = arm_opc_trans_0c;
		(arm_opc_table)[0xc].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xd].translate = arm_opc_trans_0d;
		(arm_opc_table)[0xd].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xe].translate = arm_opc_trans_0e;
		(arm_opc_table)[0xe].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xf].translate = arm_opc_trans_0f;
		(arm_opc_table)[0xf].tag = arm_tag_continue;
	}
}

int init_arm_opc_group1(arm_opc_func_t* arm_opc_table)
{
	{
		(arm_opc_table)[0x0].translate = arm_opc_trans_10;
		(arm_opc_table)[0x0].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x1].translate = arm_opc_trans_11;
		(arm_opc_table)[0x1].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x2].translate = arm_opc_trans_12;
		(arm_opc_table)[0x2].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x3].translate = arm_opc_trans_13;
		(arm_opc_table)[0x3].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x4].translate = arm_opc_trans_14;
		(arm_opc_table)[0x4].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x5].translate = arm_opc_trans_15;
		(arm_opc_table)[0x5].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x6].translate = arm_opc_trans_16;
		(arm_opc_table)[0x6].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x7].translate = arm_opc_trans_17;
		(arm_opc_table)[0x7].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x8].translate = arm_opc_trans_18;
		(arm_opc_table)[0x8].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x9].translate = arm_opc_trans_19;
		(arm_opc_table)[0x9].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xa].translate = arm_opc_trans_1a;
		(arm_opc_table)[0xa].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xb].translate = arm_opc_trans_1b;
		(arm_opc_table)[0xb].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xc].translate = arm_opc_trans_1c;
		(arm_opc_table)[0xc].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xd].translate = arm_opc_trans_1d;
		(arm_opc_table)[0xd].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xe].translate = arm_opc_trans_1e;
		(arm_opc_table)[0xe].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xf].translate = arm_opc_trans_1f;
		(arm_opc_table)[0xf].tag = arm_tag_continue;
	}
}

int init_arm_opc_group2(arm_opc_func_t* arm_opc_table)
{
	{
		(arm_opc_table)[0x0].translate = arm_opc_trans_20;
		(arm_opc_table)[0x0].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x1].translate = arm_opc_trans_21;
		(arm_opc_table)[0x1].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x2].translate = arm_opc_trans_22;
		(arm_opc_table)[0x2].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x3].translate = arm_opc_trans_23;
		(arm_opc_table)[0x3].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x4].translate = arm_opc_trans_24;
		(arm_opc_table)[0x4].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x5].translate = arm_opc_trans_25;
		(arm_opc_table)[0x5].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x6].translate = arm_opc_trans_26;
		(arm_opc_table)[0x6].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x7].translate = arm_opc_trans_27;
		(arm_opc_table)[0x7].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x8].translate = arm_opc_trans_28;
		(arm_opc_table)[0x8].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x9].translate = arm_opc_trans_29;
		(arm_opc_table)[0x9].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xa].translate = arm_opc_trans_2a;
		(arm_opc_table)[0xa].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xb].translate = arm_opc_trans_2b;
		(arm_opc_table)[0xb].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xc].translate = arm_opc_trans_2c;
		(arm_opc_table)[0xc].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xd].translate = arm_opc_trans_2d;
		(arm_opc_table)[0xd].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xe].translate = arm_opc_trans_2e;
		(arm_opc_table)[0xe].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xf].translate = arm_opc_trans_2f;
		(arm_opc_table)[0xf].tag = arm_tag_continue;
	}
}

int init_arm_opc_group3(arm_opc_func_t* arm_opc_table)
{
	{
		(arm_opc_table)[0x0].translate = arm_opc_trans_30;
		(arm_opc_table)[0x0].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x1].translate = arm_opc_trans_31;
		(arm_opc_table)[0x1].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x2].translate = arm_opc_trans_32;
		(arm_opc_table)[0x2].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x3].translate = arm_opc_trans_33;
		(arm_opc_table)[0x3].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x4].translate = arm_opc_trans_34;
		(arm_opc_table)[0x4].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x5].translate = arm_opc_trans_35;
		(arm_opc_table)[0x5].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x6].translate = arm_opc_trans_36;
		(arm_opc_table)[0x6].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x7].translate = arm_opc_trans_37;
		(arm_opc_table)[0x7].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x8].translate = arm_opc_trans_38;
		(arm_opc_table)[0x8].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x9].translate = arm_opc_trans_39;
		(arm_opc_table)[0x9].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xa].translate = arm_opc_trans_3a;
		(arm_opc_table)[0xa].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xb].translate = arm_opc_trans_3b;
		(arm_opc_table)[0xb].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xc].translate = arm_opc_trans_3c;
		(arm_opc_table)[0xc].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xd].translate = arm_opc_trans_3d;
		(arm_opc_table)[0xd].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xe].translate = arm_opc_trans_3e;
		(arm_opc_table)[0xe].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xf].translate = arm_opc_trans_3f;
		(arm_opc_table)[0xf].tag = arm_tag_continue;
	}
}

int init_arm_opc_group4(arm_opc_func_t* arm_opc_table)
{
	{
		(arm_opc_table)[0x0].translate = arm_opc_trans_40;
		(arm_opc_table)[0x0].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x1].translate = arm_opc_trans_41;
		(arm_opc_table)[0x1].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x2].translate = arm_opc_trans_42;
		(arm_opc_table)[0x2].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x3].translate = arm_opc_trans_43;
		(arm_opc_table)[0x3].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x4].translate = arm_opc_trans_44;
		(arm_opc_table)[0x4].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x5].translate = arm_opc_trans_45;
		(arm_opc_table)[0x5].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x6].translate = arm_opc_trans_46;
		(arm_opc_table)[0x6].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x7].translate = arm_opc_trans_47;
		(arm_opc_table)[0x7].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x8].translate = arm_opc_trans_48;
		(arm_opc_table)[0x8].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x9].translate = arm_opc_trans_49;
		(arm_opc_table)[0x9].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xa].translate = arm_opc_trans_4a;
		(arm_opc_table)[0xa].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xb].translate = arm_opc_trans_4b;
		(arm_opc_table)[0xb].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xc].translate = arm_opc_trans_4c;
		(arm_opc_table)[0xc].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xd].translate = arm_opc_trans_4d;
		(arm_opc_table)[0xd].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xe].translate = arm_opc_trans_4e;
		(arm_opc_table)[0xe].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xf].translate = arm_opc_trans_4f;
		(arm_opc_table)[0xf].tag = arm_tag_continue;
	}
}

int init_arm_opc_group5(arm_opc_func_t* arm_opc_table)
{
	{
		(arm_opc_table)[0x0].translate = arm_opc_trans_50;
		(arm_opc_table)[0x0].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x1].translate = arm_opc_trans_51;
		(arm_opc_table)[0x1].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x2].translate = arm_opc_trans_52;
		(arm_opc_table)[0x2].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x3].translate = arm_opc_trans_53;
		(arm_opc_table)[0x3].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x4].translate = arm_opc_trans_54;
		(arm_opc_table)[0x4].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x5].translate = arm_opc_trans_55;
		(arm_opc_table)[0x5].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x6].translate = arm_opc_trans_56;
		(arm_opc_table)[0x6].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x7].translate = arm_opc_trans_57;
		(arm_opc_table)[0x7].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x8].translate = arm_opc_trans_58;
		(arm_opc_table)[0x8].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x9].translate = arm_opc_trans_59;
		(arm_opc_table)[0x9].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xa].translate = arm_opc_trans_5a;
		(arm_opc_table)[0xa].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xb].translate = arm_opc_trans_5b;
		(arm_opc_table)[0xb].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xc].translate = arm_opc_trans_5c;
		(arm_opc_table)[0xc].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xd].translate = arm_opc_trans_5d;
		(arm_opc_table)[0xd].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xe].translate = arm_opc_trans_5e;
		(arm_opc_table)[0xe].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xf].translate = arm_opc_trans_5f;
		(arm_opc_table)[0xf].tag = arm_tag_continue;
	}
}

int init_arm_opc_group6(arm_opc_func_t* arm_opc_table)
{
	{
		(arm_opc_table)[0x0].translate = arm_opc_trans_60;
		(arm_opc_table)[0x0].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x1].translate = arm_opc_trans_61;
		(arm_opc_table)[0x1].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x2].translate = arm_opc_trans_62;
		(arm_opc_table)[0x2].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x3].translate = arm_opc_trans_63;
		(arm_opc_table)[0x3].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x4].translate = arm_opc_trans_64;
		(arm_opc_table)[0x4].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x5].translate = arm_opc_trans_65;
		(arm_opc_table)[0x5].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x6].translate = arm_opc_trans_66;
		(arm_opc_table)[0x6].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x7].translate = arm_opc_trans_67;
		(arm_opc_table)[0x7].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x8].translate = arm_opc_trans_68;
		(arm_opc_table)[0x8].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x9].translate = arm_opc_trans_69;
		(arm_opc_table)[0x9].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xa].translate = arm_opc_trans_6a;
		(arm_opc_table)[0xa].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xb].translate = arm_opc_trans_6b;
		(arm_opc_table)[0xb].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xc].translate = arm_opc_trans_6c;
		(arm_opc_table)[0xc].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xd].translate = arm_opc_trans_6d;
		(arm_opc_table)[0xd].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xe].translate = arm_opc_trans_6e;
		(arm_opc_table)[0xe].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xf].translate = arm_opc_trans_6f;
		(arm_opc_table)[0xf].tag = arm_tag_continue;
	}
}

int init_arm_opc_group7(arm_opc_func_t* arm_opc_table)
{
	{
		(arm_opc_table)[0x0].translate = arm_opc_trans_70;
		(arm_opc_table)[0x0].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x1].translate = arm_opc_trans_71;
		(arm_opc_table)[0x1].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x2].translate = arm_opc_trans_72;
		(arm_opc_table)[0x2].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x3].translate = arm_opc_trans_73;
		(arm_opc_table)[0x3].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x4].translate = arm_opc_trans_74;
		(arm_opc_table)[0x4].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x5].translate = arm_opc_trans_75;
		(arm_opc_table)[0x5].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x6].translate = arm_opc_trans_76;
		(arm_opc_table)[0x6].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x7].translate = arm_opc_trans_77;
		(arm_opc_table)[0x7].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x8].translate = arm_opc_trans_78;
		(arm_opc_table)[0x8].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x9].translate = arm_opc_trans_79;
		(arm_opc_table)[0x9].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xa].translate = arm_opc_trans_7a;
		(arm_opc_table)[0xa].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xb].translate = arm_opc_trans_7b;
		(arm_opc_table)[0xb].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xc].translate = arm_opc_trans_7c;
		(arm_opc_table)[0xc].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xd].translate = arm_opc_trans_7d;
		(arm_opc_table)[0xd].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xe].translate = arm_opc_trans_7e;
		(arm_opc_table)[0xe].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xf].translate = arm_opc_trans_7f;
		(arm_opc_table)[0xf].tag = arm_tag_continue;
	}
}

int init_arm_opc_group8(arm_opc_func_t* arm_opc_table)
{
	{
		(arm_opc_table)[0x0].translate = arm_opc_trans_80;
		(arm_opc_table)[0x0].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x1].translate = arm_opc_trans_81;
		(arm_opc_table)[0x1].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x2].translate = arm_opc_trans_82;
		(arm_opc_table)[0x2].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x3].translate = arm_opc_trans_83;
		(arm_opc_table)[0x3].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x4].translate = arm_opc_trans_84;
		(arm_opc_table)[0x4].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x5].translate = arm_opc_trans_85;
		(arm_opc_table)[0x5].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x6].translate = arm_opc_trans_86;
		(arm_opc_table)[0x6].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x7].translate = arm_opc_trans_87;
		(arm_opc_table)[0x7].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x8].translate = arm_opc_trans_88;
		(arm_opc_table)[0x8].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x9].translate = arm_opc_trans_89;
		(arm_opc_table)[0x9].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xa].translate = arm_opc_trans_8a;
		(arm_opc_table)[0xa].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xb].translate = arm_opc_trans_8b;
		(arm_opc_table)[0xb].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xc].translate = arm_opc_trans_8c;
		(arm_opc_table)[0xc].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xd].translate = arm_opc_trans_8d;
		(arm_opc_table)[0xd].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xe].translate = arm_opc_trans_8e;
		(arm_opc_table)[0xe].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xf].translate = arm_opc_trans_8f;
		(arm_opc_table)[0xf].tag = arm_tag_continue;
	}
}

int init_arm_opc_group9(arm_opc_func_t* arm_opc_table)
{
	{
		(arm_opc_table)[0x0].translate = arm_opc_trans_90;
		(arm_opc_table)[0x0].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x1].translate = arm_opc_trans_91;
		(arm_opc_table)[0x1].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x2].translate = arm_opc_trans_92;
		(arm_opc_table)[0x2].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x3].translate = arm_opc_trans_93;
		(arm_opc_table)[0x3].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x4].translate = arm_opc_trans_94;
		(arm_opc_table)[0x4].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x5].translate = arm_opc_trans_95;
		(arm_opc_table)[0x5].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x6].translate = arm_opc_trans_96;
		(arm_opc_table)[0x6].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x7].translate = arm_opc_trans_97;
		(arm_opc_table)[0x7].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x8].translate = arm_opc_trans_98;
		(arm_opc_table)[0x8].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x9].translate = arm_opc_trans_99;
		(arm_opc_table)[0x9].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xa].translate = arm_opc_trans_9a;
		(arm_opc_table)[0xa].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xb].translate = arm_opc_trans_9b;
		(arm_opc_table)[0xb].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xc].translate = arm_opc_trans_9c;
		(arm_opc_table)[0xc].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xd].translate = arm_opc_trans_9d;
		(arm_opc_table)[0xd].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xe].translate = arm_opc_trans_9e;
		(arm_opc_table)[0xe].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xf].translate = arm_opc_trans_9f;
		(arm_opc_table)[0xf].tag = arm_tag_continue;
	}
}

int init_arm_opc_group10(arm_opc_func_t* arm_opc_table)
{
	{
		(arm_opc_table)[0x0].translate = arm_opc_trans_a0;
		(arm_opc_table)[0x0].tag = arm_tag_branch;
	}

	{
		(arm_opc_table)[0x1].translate = arm_opc_trans_a1;
		(arm_opc_table)[0x1].tag = arm_tag_branch;
	}

	{
		(arm_opc_table)[0x2].translate = arm_opc_trans_a2;
		(arm_opc_table)[0x2].tag = arm_tag_branch;
	}

	{
		(arm_opc_table)[0x3].translate = arm_opc_trans_a3;
		(arm_opc_table)[0x3].tag = arm_tag_branch;
	}

	{
		(arm_opc_table)[0x4].translate = arm_opc_trans_a4;
		(arm_opc_table)[0x4].tag = arm_tag_branch;
	}

	{
		(arm_opc_table)[0x5].translate = arm_opc_trans_a5;
		(arm_opc_table)[0x5].tag = arm_tag_branch;
	}

	{
		(arm_opc_table)[0x6].translate = arm_opc_trans_a6;
		(arm_opc_table)[0x6].tag = arm_tag_branch;
	}

	{
		(arm_opc_table)[0x7].translate = arm_opc_trans_a7;
		(arm_opc_table)[0x7].tag = arm_tag_branch;
	}

	{
		(arm_opc_table)[0x8].translate = arm_opc_trans_a8;
		(arm_opc_table)[0x8].tag = arm_tag_branch;
	}

	{
		(arm_opc_table)[0x9].translate = arm_opc_trans_a9;
		(arm_opc_table)[0x9].tag = arm_tag_branch;
	}

	{
		(arm_opc_table)[0xa].translate = arm_opc_trans_aa;
		(arm_opc_table)[0xa].tag = arm_tag_branch;
	}

	{
		(arm_opc_table)[0xb].translate = arm_opc_trans_ab;
		(arm_opc_table)[0xb].tag = arm_tag_branch;
	}

	{
		(arm_opc_table)[0xc].translate = arm_opc_trans_ac;
		(arm_opc_table)[0xc].tag = arm_tag_branch;
	}

	{
		(arm_opc_table)[0xd].translate = arm_opc_trans_ad;
		(arm_opc_table)[0xd].tag = arm_tag_branch;
	}

	{
		(arm_opc_table)[0xe].translate = arm_opc_trans_ae;
		(arm_opc_table)[0xe].tag = arm_tag_branch;
	}

	{
		(arm_opc_table)[0xf].translate = arm_opc_trans_af;
		(arm_opc_table)[0xf].tag = arm_tag_branch;
	}
}

int init_arm_opc_group11(arm_opc_func_t* arm_opc_table)
{
	{
		(arm_opc_table)[0x0].translate = arm_opc_trans_b0;
		(arm_opc_table)[0x0].tag = arm_tag_call;
	}

	{
		(arm_opc_table)[0x1].translate = arm_opc_trans_b1;
		(arm_opc_table)[0x1].tag = arm_tag_call;
	}

	{
		(arm_opc_table)[0x2].translate = arm_opc_trans_b2;
		(arm_opc_table)[0x2].tag = arm_tag_call;
	}

	{
		(arm_opc_table)[0x3].translate = arm_opc_trans_b3;
		(arm_opc_table)[0x3].tag = arm_tag_call;
	}

	{
		(arm_opc_table)[0x4].translate = arm_opc_trans_b4;
		(arm_opc_table)[0x4].tag = arm_tag_call;
	}

	{
		(arm_opc_table)[0x5].translate = arm_opc_trans_b5;
		(arm_opc_table)[0x5].tag = arm_tag_call;
	}

	{
		(arm_opc_table)[0x6].translate = arm_opc_trans_b6;
		(arm_opc_table)[0x6].tag = arm_tag_call;
	}

	{
		(arm_opc_table)[0x7].translate = arm_opc_trans_b7;
		(arm_opc_table)[0x7].tag = arm_tag_call;
	}

	{
		(arm_opc_table)[0x8].translate = arm_opc_trans_b8;
		(arm_opc_table)[0x8].tag = arm_tag_call;
	}

	{
		(arm_opc_table)[0x9].translate = arm_opc_trans_b9;
		(arm_opc_table)[0x9].tag = arm_tag_call;
	}

	{
		(arm_opc_table)[0xa].translate = arm_opc_trans_ba;
		(arm_opc_table)[0xa].tag = arm_tag_call;
	}

	{
		(arm_opc_table)[0xb].translate = arm_opc_trans_bb;
		(arm_opc_table)[0xb].tag = arm_tag_call;
	}

	{
		(arm_opc_table)[0xc].translate = arm_opc_trans_bc;
		(arm_opc_table)[0xc].tag = arm_tag_call;
	}

	{
		(arm_opc_table)[0xd].translate = arm_opc_trans_bd;
		(arm_opc_table)[0xd].tag = arm_tag_call;
	}

	{
		(arm_opc_table)[0xe].translate = arm_opc_trans_be;
		(arm_opc_table)[0xe].tag = arm_tag_call;
	}

	{
		(arm_opc_table)[0xf].translate = arm_opc_trans_bf;
		(arm_opc_table)[0xf].tag = arm_tag_call;
	}
}

int init_arm_opc_group12(arm_opc_func_t* arm_opc_table)
{
	{
		(arm_opc_table)[0x0].translate = arm_opc_trans_c0;
		(arm_opc_table)[0x0].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x1].translate = arm_opc_trans_c1;
		(arm_opc_table)[0x1].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x2].translate = arm_opc_trans_c2;
		(arm_opc_table)[0x2].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x3].translate = arm_opc_trans_c3;
		(arm_opc_table)[0x3].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x4].translate = arm_opc_trans_c4;
		(arm_opc_table)[0x4].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x5].translate = arm_opc_trans_c5;
		(arm_opc_table)[0x5].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x6].translate = arm_opc_trans_c6;
		(arm_opc_table)[0x6].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x7].translate = arm_opc_trans_c7;
		(arm_opc_table)[0x7].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x8].translate = arm_opc_trans_c8;
		(arm_opc_table)[0x8].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x9].translate = arm_opc_trans_c9;
		(arm_opc_table)[0x9].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xa].translate = arm_opc_trans_ca;
		(arm_opc_table)[0xa].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xb].translate = arm_opc_trans_cb;
		(arm_opc_table)[0xb].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xc].translate = arm_opc_trans_cc;
		(arm_opc_table)[0xc].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xd].translate = arm_opc_trans_cd;
		(arm_opc_table)[0xd].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xe].translate = arm_opc_trans_ce;
		(arm_opc_table)[0xe].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xf].translate = arm_opc_trans_cf;
		(arm_opc_table)[0xf].tag = arm_tag_continue;
	}
}

int init_arm_opc_group13(arm_opc_func_t* arm_opc_table)
{
	{
		(arm_opc_table)[0x0].translate = arm_opc_trans_d0;
		(arm_opc_table)[0x0].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x1].translate = arm_opc_trans_d1;
		(arm_opc_table)[0x1].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x2].translate = arm_opc_trans_d2;
		(arm_opc_table)[0x2].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x3].translate = arm_opc_trans_d3;
		(arm_opc_table)[0x3].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x4].translate = arm_opc_trans_d4;
		(arm_opc_table)[0x4].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x5].translate = arm_opc_trans_d5;
		(arm_opc_table)[0x5].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x6].translate = arm_opc_trans_d6;
		(arm_opc_table)[0x6].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x7].translate = arm_opc_trans_d7;
		(arm_opc_table)[0x7].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x8].translate = arm_opc_trans_d8;
		(arm_opc_table)[0x8].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x9].translate = arm_opc_trans_d9;
		(arm_opc_table)[0x9].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xa].translate = arm_opc_trans_da;
		(arm_opc_table)[0xa].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xb].translate = arm_opc_trans_db;
		(arm_opc_table)[0xb].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xc].translate = arm_opc_trans_dc;
		(arm_opc_table)[0xc].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xd].translate = arm_opc_trans_dd;
		(arm_opc_table)[0xd].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xe].translate = arm_opc_trans_de;
		(arm_opc_table)[0xe].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xf].translate = arm_opc_trans_df;
		(arm_opc_table)[0xf].tag = arm_tag_continue;
	}
}

int init_arm_opc_group14(arm_opc_func_t* arm_opc_table)
{
	{
		(arm_opc_table)[0x0].translate = arm_opc_trans_e0;
		(arm_opc_table)[0x0].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x1].translate = arm_opc_trans_e1;
		(arm_opc_table)[0x1].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x2].translate = arm_opc_trans_e2;
		(arm_opc_table)[0x2].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x3].translate = arm_opc_trans_e3;
		(arm_opc_table)[0x3].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x4].translate = arm_opc_trans_e4;
		(arm_opc_table)[0x4].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x5].translate = arm_opc_trans_e5;
		(arm_opc_table)[0x5].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x6].translate = arm_opc_trans_e6;
		(arm_opc_table)[0x6].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x7].translate = arm_opc_trans_e7;
		(arm_opc_table)[0x7].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x8].translate = arm_opc_trans_e8;
		(arm_opc_table)[0x8].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x9].translate = arm_opc_trans_e9;
		(arm_opc_table)[0x9].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xa].translate = arm_opc_trans_ea;
		(arm_opc_table)[0xa].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xb].translate = arm_opc_trans_eb;
		(arm_opc_table)[0xb].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xc].translate = arm_opc_trans_ec;
		(arm_opc_table)[0xc].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xd].translate = arm_opc_trans_ed;
		(arm_opc_table)[0xd].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xe].translate = arm_opc_trans_ee;
		(arm_opc_table)[0xe].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xf].translate = arm_opc_trans_ef;
		(arm_opc_table)[0xf].tag = arm_tag_continue;
	}
}

int init_arm_opc_group15(arm_opc_func_t* arm_opc_table)
{
	{
		(arm_opc_table)[0x0].translate = arm_opc_trans_f0;
		(arm_opc_table)[0x0].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x1].translate = arm_opc_trans_f1;
		(arm_opc_table)[0x1].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x2].translate = arm_opc_trans_f2;
		(arm_opc_table)[0x2].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x3].translate = arm_opc_trans_f3;
		(arm_opc_table)[0x3].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x4].translate = arm_opc_trans_f4;
		(arm_opc_table)[0x4].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x5].translate = arm_opc_trans_f5;
		(arm_opc_table)[0x5].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x6].translate = arm_opc_trans_f6;
		(arm_opc_table)[0x6].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x7].translate = arm_opc_trans_f7;
		(arm_opc_table)[0x7].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x8].translate = arm_opc_trans_f8;
		(arm_opc_table)[0x8].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0x9].translate = arm_opc_trans_f9;
		(arm_opc_table)[0x9].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xa].translate = arm_opc_trans_fa;
		(arm_opc_table)[0xa].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xb].translate = arm_opc_trans_fb;
		(arm_opc_table)[0xb].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xc].translate = arm_opc_trans_fc;
		(arm_opc_table)[0xc].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xd].translate = arm_opc_trans_fd;
		(arm_opc_table)[0xd].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xe].translate = arm_opc_trans_fe;
		(arm_opc_table)[0xe].tag = arm_tag_continue;
	}

	{
		(arm_opc_table)[0xf].translate = arm_opc_trans_ff;
		(arm_opc_table)[0xf].tag = arm_tag_continue;
	}
}
