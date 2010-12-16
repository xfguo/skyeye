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
void arm_translate_opc(cpu_t* cpu, uint32_t opc, BasicBlock *bb);
void arm_dec_init();
void arm_dyncom_dec_init();

typedef int (*tag_func_t)(cpu_t *cpu, uint32_t instr, tag_t *tag, addr_t *new_pc, addr_t *next_pc);
typedef int (*translate_func_t)(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
typedef Value* (*translate_cond_func_t)(cpu_t *cpu, addr_t pc, BasicBlock *bb);
typedef struct arm_opc_func_s{
        tag_func_t tag;
        translate_func_t translate;
        translate_cond_func_t translate_cond;
}arm_opc_func_t;
arm_opc_func_t* arm_get_opc_func(uint32_t opc);
using namespace llvm;

#define INSTR_SIZE 4

Value * arch_arm_translate_cond(cpu_t *cpu, addr_t pc, BasicBlock *bb)
{
	uint32_t instr;
	bus_read(32, pc, &instr);
	arm_opc_func_t *opc_func = arm_get_opc_func(instr);
	return opc_func->translate_cond(cpu, instr, bb);
}

int arch_arm_tag_instr(cpu_t *cpu, addr_t pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc) {
	int instr_size = INSTR_SIZE;
	uint32_t instr;
	bus_read(32, pc, &instr);
	arm_opc_func_t *opc_func = arm_get_opc_func(instr);
	opc_func->tag(cpu, instr, tag, new_pc, next_pc);
	return instr_size;
}

int arch_arm_translate_instr(cpu_t *cpu, addr_t pc, BasicBlock *bb) {
	int instr_size = INSTR_SIZE;
	uint32_t instr;
	if(bus_read(32, pc, &instr)){

	}
	arm_opc_func_t *opc_func = arm_get_opc_func(instr);
	opc_func->translate(cpu, instr, bb);
	return instr_size;
}

