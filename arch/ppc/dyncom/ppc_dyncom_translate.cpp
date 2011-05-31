#include <stdio.h>
#include <stdlib.h>
#include "llvm/Instructions.h"

#include "bank_defs.h"
#include "skyeye_dyncom.h"
#include "skyeye.h"
#include "dyncom/dyncom_llvm.h"
#include "dyncom/frontend.h"
#include "ppc_dyncom.h"
#include "dyncom/tag.h"
#include "dyncom/basicblock.h"

#include "ppc_dyncom_dec.h"
#include "ppc_dyncom_debug.h"

#define BAD_INSTR {fprintf(stderr, "In %s, cannot parse instruction 0x%x\n", __FUNCTION__, instr);skyeye_exit(-1);}

using namespace llvm;

Value *
arch_powerpc_translate_cond(cpu_t *cpu, addr_t phys_pc, BasicBlock *bb){
	debug(DEBUG_TAG, "In %s, pc=0x%x\n", __FUNCTION__, phys_pc);
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
#define END_ADDR 0x14
	debug(DEBUG_TAG, "In %s, pc=0x%x\n", __FUNCTION__, phys_pc);
	bus_read(32, phys_pc, &instr);
	ppc_opc_func_t* opc_func = ppc_get_opc_func(instr);
	if(!opc_func)
		BAD_INSTR;
	//assert(opc_func->tag != NULL);
	if(opc_func->tag == NULL){
		printf("Some NULL tag functions at 0x%x, instr = 0x%x\n", phys_pc, instr);
		opc_default_tag(cpu, instr, phys_pc,tag, new_pc, next_pc);
	}
	else
		opc_func->tag(cpu, instr, phys_pc, tag, new_pc, next_pc);

	*next_pc = phys_pc + instr_size;
	if(phys_pc == END_ADDR)
		*tag |= TAG_STOP;
	return instr_size;
}

int arch_powerpc_translate_instr(cpu_t *cpu, addr_t real_addr, BasicBlock *bb){
	uint32 instr_size = 4;
	uint32 instr;
	if(bus_read(32, real_addr, &instr) != 0){
		/* some error handler */
	}
	/* trap */
	if(instr == 0x7fe00008)
		return instr_size;
	/* dssall */
	if(instr == 0x7e00066c)
		return instr_size;
	debug(DEBUG_TRANSLATE, "In %s, pc=0x%x, instr=0x%x\n", __FUNCTION__, real_addr, instr);
	ppc_opc_func_t* opc_func = ppc_get_opc_func(instr);
	if(!opc_func)
		BAD_INSTR;
	opc_func->translate(cpu, instr, bb);
	return instr_size;
}
