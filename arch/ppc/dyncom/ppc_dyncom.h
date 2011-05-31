#ifndef __PPC_DYNCOM_H__
#define __PPC_DYNCOM_H__
#include <stdio.h>
#include <stdlib.h>
#include "skyeye_dyncom.h"

//#include "dyncom/dyncom_llvm.h"
#include "skyeye_types.h"
#include "ppc_e500_core.h"

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

#define VECTOR_T_SIZE sizeof(Vector_t)
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
	PHYS_PC_REGNUM,
	RESERVE_REGNUM,
	HAVE_RESERVATION_REGNUM,
	ICOUNT_REGNUM,

	IBATU_REGNUM,
	IBATL_REGNUM = IBATU_REGNUM + 4,
	IBAT_BL17_REGNUM = IBATL_REGNUM + 4,
	DBATU_REGNUM = IBAT_BL17_REGNUM + 4,
	DBATL_REGNUM = DBATU_REGNUM + 4,
	DBAT_BL17_REGNUM = DBATL_REGNUM + 4,
	SDR1_REGNUM = DBAT_BL17_REGNUM + 4,
	SR_REGNUM,
	DAR_REGNUM = SR_REGNUM + 16,
	DSISR_REGNUM,
	SPRG_REGNUM,
	SRR_REGNUM = SPRG_REGNUM + 8,
	DEC_REGNUM = SRR_REGNUM + 2,
	EAR_REGNUM,
	PIR_REGNUM,
	TB_REGNUM,					/* 64 bit */
	HID_REGNUM = TB_REGNUM + 2,
	CURRENT_OPC_REGNUM = HID_REGNUM + 16,	/* 88 */
	EXCEPTION_PENDING_REGNUM,
	DEC_EXCEPTION_REGNUM,
	EXT_EXCEPTION_REGNUM,
	STOP_EXCEPTION_REGNUM,
	SINGLESTEP_IGNORE_REGNUM,
	PAGETABLE_BASE_REGNUM,
	PAGETAGLE_HASHMASK_REGNUM,
	PDEC_REGNUM,
	PTB_REGNUM = PDEC_REGNUM + 2,			/* 100 */
	E600_IBATU_REGNUM = PTB_REGNUM + 2,
	E600_IBATL_REGNUM = E600_IBATU_REGNUM + 4,
	E600_DBATU_REGNUM = E600_IBATL_REGNUM + 4,
	E600_DBATL_REGNUM = E600_DBATU_REGNUM + 4,
	E600_PTE_REGNUM = E600_DBATL_REGNUM + 4,
	E600_TLBMISS_REGNUM = E600_PTE_REGNUM + 2,
	E600_ICTC_REGNUM,
	E600_HID_REGNUM = E600_ICTC_REGNUM + 2, 
	E600_UPMC_REGNUM = E600_HID_REGNUM + 2,
	E600_USIAR_REGNUM = E600_UPMC_REGNUM + 6,
	E600_UMMCR_REGNUM,
	E600_SPRG_REGNUM = E600_UMMCR_REGNUM + 3,
	E600_LDSTCR_REGNUM = E600_SPRG_REGNUM + 4,	/* 139 */
	E600_LDSTDB_REGNUM,
	E600_MSSCR0_REGNUM,
	E600_MSSSR0_REGNUM,
	E600_ICTRL_REGNUM,
	E600_L2CR_REGNUM,
	E600_MMCR2_REGNUM,
	E600_BAMR_REGNUM,
	L1CSR_REGNUM,
	CSRR_REGNUM = L1CSR_REGNUM + 2,
	MCSRR_REGNUM = CSRR_REGNUM + 2,
	ESR_REGNUM = MCSRR_REGNUM + 2,
	MCSR_REGNUM,
	DEAR_REGNUM,
	DBCR_REGNUM,
	DBSR_REGNUM = DBCR_REGNUM + 3,
	TCR_REGNUM,
	TSR_REGNUM,
	DAC_REGNUM,
	IPVR_REGNUM = DAC_REGNUM + 2,
	IVOR_REGNUM,
	IAC_REGNUM = IVOR_REGNUM + 16,
	TBL_REGNUM = IAC_REGNUM + 2,
	TBU_REGNUM,						/* 184 */
	EFFECTIVE_CODE_PAGE_REGNUM,
	SYSCALL_NUMBER_REGNUM,
	SPEFSCR_REGNUM,
	VSCR_REGNUM,
	VRSAVE_REGNUM,
	VR_REGNUM,			/*vector*/
	VTEMP_REGNUM = VR_REGNUM + (36 * VECTOR_T_SIZE / 4),

	PPC_DYNCOM_MAX_SPR_REGNUM,
}e500_regnum_t;
#define PPC_DYNCOM_GPR_SIZE 32
#define PPC_XR_SIZE (MAX_REGNUM - CR_REGNUM)
#define SR(N) (PPC_GPR_SIZE + PPC_FPR_SIZE + N)

#define PPC_EXC_DSI_ADDR 0x300
#ifdef __cplusplus
}
#endif

#endif
