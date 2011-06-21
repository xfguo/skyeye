#ifndef __ARM_TRANS__
#define __ARM_TRANS__
#include "skyeye_dyncom.h"

int arch_arm_tag_instr(cpu_t *cpu, addr_t pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc);
Value *
arch_arm_translate_cond(cpu_t *cpu, addr_t pc, BasicBlock *bb);
int arch_arm_translate_instr(cpu_t *cpu, addr_t pc, BasicBlock *bb);
void arm_opc_func_init();
void arch_arm_flus_instr_category();

typedef std::map<addr_t, int> decoder_cache;
extern decoder_cache dc_map;

#endif
