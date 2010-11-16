#ifndef __PPC_DYNCOM_H__
#define __PPC_DYNCOM_H__
#include <stdio.h>
#include <stdlib.h>
#include "llvm/Instructions.h"

#include "dyncom_llvm.h"
#include "skyeye_dyncom.h"
#include "skyeye_types.h"

Value *
arch_powerpc_translate_cond(cpu_t *cpu, addr_t pc, BasicBlock *bb);

int arch_powerpc_tag_instr(cpu_t *cpu, addr_t pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc);

int arch_powerpc_translate_instr(cpu_t *cpu, addr_t pc, BasicBlock *bb);

int arch_powerpc_disasm_instr(cpu_t *cpu, addr_t pc, char* line, unsigned int max_line);

int arch_powerpc_translate_loop_helper(cpu_t* cpu, addr_t pc, BasicBlock *bb, BasicBlock *bb_ret, BasicBlock *bb_next, BasicBlock *bb_cond); 

typedef struct{
	uint32 pc;
}reg_powerpc_t;

#endif
