#include <stdio.h>
#include <stdlib.h>
#include "llvm/Instructions.h"

#include "bank_defs.h"
#include "skyeye_dyncom.h"
#include "dyncom/dyncom_llvm.h"
#include "dyncom/frontend.h"
#include "ppc_dyncom.h"
#include "dyncom/tag.h"
#include "dyncom/basicblock.h"

#include "ppc_dyncom_dec.h"

#define BAD_INSTR {fprintf(stderr, "In %s, cannot parse instruction 0x%x\n", __FUNCTION__, instr);exit(-1);}

using namespace llvm;

Value *
arch_powerpc_translate_cond(cpu_t *cpu, addr_t phys_pc, BasicBlock *bb){
	uint32_t instr;
	bus_read(32, phys_pc, &instr);
	ppc_opc_func_t* opc_func = ppc_get_opc_func(instr);
	if(!opc_func)
		BAD_INSTR;
	return opc_func->translate_cond(cpu, instr, bb);
}

/**
* @brief  tagging the instruction at pc
*
* @param cpu
* @param pc
* @param tag
* @param new_pc
* @param next_pc
*
* @return 
*/
int arch_powerpc_tag_instr(cpu_t *cpu, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	int instr_size = 4;
	uint32_t instr;
	printf("In %s, pc=0x%x\n", __FUNCTION__, phys_pc);
	bus_read(32, phys_pc, &instr);
	ppc_opc_func_t* opc_func = ppc_get_opc_func(instr);
	if(!opc_func)
		BAD_INSTR;
	assert(!opc_func->tag);
	opc_func->tag(cpu, instr, tag, new_pc, next_pc);
	return instr_size;
}

int arch_powerpc_translate_instr(cpu_t *cpu, addr_t real_addr, BasicBlock *bb){
	uint32 instr_size = 4;
	uint32 instr;
	printf("In %s, pc=0x%x\n", __FUNCTION__, real_addr);
	if(bus_read(32, real_addr, &instr) != 0){
		/* some error handler */
	}
	ppc_opc_func_t* opc_func = ppc_get_opc_func(instr);
	if(!opc_func)
		BAD_INSTR;
	opc_func->translate(cpu, instr, bb);
	return instr_size;
}
