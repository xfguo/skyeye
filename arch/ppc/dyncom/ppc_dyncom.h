#ifndef __PPC_DYNCOM_H__
#define __PPC_DYNCOM_H__
#include <stdio.h>
#include <stdlib.h>
#include "skyeye_dyncom.h"

//#include "dyncom/dyncom_llvm.h"
#include "skyeye_types.h"

#ifdef __cplusplus
 extern "C" {
#endif

void ppc_translate_opc(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
Value *
arch_powerpc_translate_cond(cpu_t *cpu, addr_t pc, BasicBlock *bb);

int arch_powerpc_tag_instr(cpu_t *cpu, addr_t pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc);

int arch_powerpc_translate_instr(cpu_t *cpu, addr_t pc, BasicBlock *bb);

static int arch_powerpc_disasm_instr(cpu_t *cpu, addr_t pc, char* line, unsigned int max_line);

static int arch_powerpc_translate_loop_helper(cpu_t* cpu, addr_t pc, BasicBlock *bb, BasicBlock *bb_ret, BasicBlock *bb_next, BasicBlock *bb_cond); 

#define PPC_GPR_SIZE 32
#define PPC_FPR_SIZE 32

/**
* @brief The register number for special register, should keep the same 
*	 order with e500_core_t
*/
typedef enum{
//	CR_REGNUM = (PPC_GPR_SIZE + PPC_FPR_SIZE),
	CR_REGNUM = 0,
	FPSCR_REGNUM,
	XER_REGNUM,
	XER_CA_REGNUM,
	LR_REGNUM,
	CTR_REGNUM,
	MSR_REGNUM,
	PVR_REGNUM,
	PC_REGNUM,
	NPC_REGNUM,
	PPC_DYNCOM_MAX_SPR_REGNUM,
}e500_regnum_t;
#define PPC_DYNCOM_GPR_SIZE 32
#define PPC_XR_SIZE (MAX_REGNUM - CR_REGNUM)
#define SR(N) (PPC_GPR_SIZE + PPC_FPR_SIZE + N)

#ifdef __cplusplus
}
#endif

#endif
