#include "llvm/Instructions.h"
#include <skyeye_dyncom.h>
#include "dyncom/dyncom_llvm.h"
#include "dyncom/frontend.h"
#include "mips_internal.h"
#include "bank_defs.h"
#include "mips_dyncom_dec.h"
#include "dyncom/tag.h"

int mips_tag_default(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	*tag = TAG_CONTINUE;
	*next_pc = phys_pc + MIPS_INSN_SIZE;

	return MIPS_INSN_SIZE;
}

int mips_tag_branch(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	*tag = TAG_COND_BRANCH | TAG_DELAY_SLOT;
	*new_pc = MIPS_BRANCH_TARGET;
	*next_pc = phys_pc + MIPS_INSN_SIZE;

	return MIPS_INSN_SIZE;
}

int mips_tag_call(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	*tag = TAG_CALL | TAG_DELAY_SLOT;
	*new_pc = MIPS_BRANCH_TARGET;
	*next_pc = phys_pc + MIPS_INSN_SIZE;

	return MIPS_INSN_SIZE;
}

int mips_tag_jr(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	*tag = TAG_RET | TAG_DELAY_SLOT;

	return 0;
}

int mips_tag_jalr(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	*new_pc = NEW_PC_NONE;
	*tag = TAG_BRANCH | TAG_DELAY_SLOT;

	return 0;
}

int mips_tag_trap(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	*tag = TAG_TRAP;
}

/* main */
//mips_opc_func_t mips_opc_special_func;/* specail func has a single table */
//mips_opc_func_t mips_opc_regimm_func;/* regimm func has a signle table */

mips_opc_func_t mips_opc_j_func;
mips_opc_func_t mips_opc_jal_func;
mips_opc_func_t mips_opc_beq_func;
mips_opc_func_t mips_opc_bne_func;
mips_opc_func_t mips_opc_blez_func;
mips_opc_func_t mips_opc_bgtz_func;
mips_opc_func_t mips_opc_addi_func;
mips_opc_func_t mips_opc_addiu_func;
mips_opc_func_t mips_opc_slti_func;
mips_opc_func_t mips_opc_sltiu_func;
mips_opc_func_t mips_opc_andi_func;
mips_opc_func_t mips_opc_ori_func;
mips_opc_func_t mips_opc_xori_func;
mips_opc_func_t mips_opc_lui_func;
/*mips_opc_func_t mips_opc_cop0_func;*/ /* cop0 func has a single table */
/*mips_opc_func_t mips_opc_cop1_func;*/ /* cop1 func has a single table */
mips_opc_func_t mips_opc_cop2_func;
mips_opc_func_t mips_opc_beql_func;
mips_opc_func_t mips_opc_bnel_func;
mips_opc_func_t mips_opc_blezl_func;
mips_opc_func_t mips_opc_bgtzl_func;
mips_opc_func_t mips_opc_daddi_func;
mips_opc_func_t mips_opc_daddiu_func;
mips_opc_func_t mips_opc_ldl_func;
mips_opc_func_t mips_opc_ldr_func;
//mips_opc_func_t mips_opc_spec2_func;/* specail func has a single table */
mips_opc_func_t mips_opc_lb_func;
mips_opc_func_t mips_opc_lh_func;
mips_opc_func_t mips_opc_lwl_func;
mips_opc_func_t mips_opc_lw_func;
mips_opc_func_t mips_opc_lbu_func;
mips_opc_func_t mips_opc_lhu_func;
mips_opc_func_t mips_opc_lwr_func;
mips_opc_func_t mips_opc_lwu_func;
mips_opc_func_t mips_opc_sb_func;
mips_opc_func_t mips_opc_sh_func;
mips_opc_func_t mips_opc_swl_func;
mips_opc_func_t mips_opc_sw_func;
mips_opc_func_t mips_opc_sdl_func;
mips_opc_func_t mips_opc_sdr_func;
mips_opc_func_t mips_opc_swr_func;
mips_opc_func_t mips_opc_cache_func;
mips_opc_func_t mips_opc_ll_func;
mips_opc_func_t mips_opc_lwc1_func;
mips_opc_func_t mips_opc_lwc2_func;
mips_opc_func_t mips_opc_pref_func;
mips_opc_func_t mips_opc_lld_func;
mips_opc_func_t mips_opc_ldc1_func;
mips_opc_func_t mips_opc_ldc2_func;
mips_opc_func_t mips_opc_ld_func;
mips_opc_func_t mips_opc_sc_func;
mips_opc_func_t mips_opc_swc1_func;
mips_opc_func_t mips_opc_swc2_func;
mips_opc_func_t mips_opc_scd_func;
mips_opc_func_t mips_opc_sdc1_func;
mips_opc_func_t mips_opc_sd_func;

/* spec */
mips_opc_func_t mips_opc_sll_func;
mips_opc_func_t mips_opc_null_func;
mips_opc_func_t mips_opc_srl_func;
mips_opc_func_t mips_opc_sra_func;
mips_opc_func_t mips_opc_sllv_func;
mips_opc_func_t mips_opc_srlv_func;
mips_opc_func_t mips_opc_srav_func;
mips_opc_func_t mips_opc_jr_func;
mips_opc_func_t mips_opc_jalr_func;
mips_opc_func_t mips_opc_movz_func;
mips_opc_func_t mips_opc_movn_func;
mips_opc_func_t mips_opc_syscall_func;
mips_opc_func_t mips_opc_break_func;
mips_opc_func_t mips_opc_sync_func;
mips_opc_func_t mips_opc_mfhi_func;
mips_opc_func_t mips_opc_mthi_func;
mips_opc_func_t mips_opc_mflo_func;
mips_opc_func_t mips_opc_mtlo_func;
mips_opc_func_t mips_opc_dsllv_func;
mips_opc_func_t mips_opc_dsrlv_func;
mips_opc_func_t mips_opc_dsrav_func;
mips_opc_func_t mips_opc_mult_func;
mips_opc_func_t mips_opc_multu_func;
mips_opc_func_t mips_opc_div_func;
mips_opc_func_t mips_opc_divu_func;
mips_opc_func_t mips_opc_dmult_func;
mips_opc_func_t mips_opc_dmultu_func;
mips_opc_func_t mips_opc_ddiv_func;
mips_opc_func_t mips_opc_ddivu_func;
mips_opc_func_t mips_opc_add_func;
mips_opc_func_t mips_opc_addu_func;
mips_opc_func_t mips_opc_sub_func;
mips_opc_func_t mips_opc_subu_func;
mips_opc_func_t mips_opc_and_func;
mips_opc_func_t mips_opc_or_func;
mips_opc_func_t mips_opc_xor_func;
mips_opc_func_t mips_opc_nor_func;
mips_opc_func_t mips_opc_slt_func;
mips_opc_func_t mips_opc_sltu_func;
mips_opc_func_t mips_opc_dadd_func;
mips_opc_func_t mips_opc_daddu_func;
mips_opc_func_t mips_opc_dsub_func;
mips_opc_func_t mips_opc_dsubu_func;
mips_opc_func_t mips_opc_tge_func;
mips_opc_func_t mips_opc_tgeu_func;
mips_opc_func_t mips_opc_tlt_func;
mips_opc_func_t mips_opc_tltu_func;
mips_opc_func_t mips_opc_teq_func;
mips_opc_func_t mips_opc_tne_func;
mips_opc_func_t mips_opc_dsll_func;
mips_opc_func_t mips_opc_dsrl_func;
mips_opc_func_t mips_opc_dsra_func;
mips_opc_func_t mips_opc_dsll32_func;
mips_opc_func_t mips_opc_dsrl32_func;
mips_opc_func_t mips_opc_dsra32_func;

/* spec 2 */
mips_opc_func_t mips_opc_fadd_func;
mips_opc_func_t mips_opc_fsub_func;
mips_opc_func_t mips_opc_fmul_func;
mips_opc_func_t mips_opc_fdiv_func;
mips_opc_func_t mips_opc_sqrt_func;
mips_opc_func_t mips_opc_abs_func;
mips_opc_func_t mips_opc_mov_func;
mips_opc_func_t mips_opc_neg_func;
mips_opc_func_t mips_opc_roundl_func;
mips_opc_func_t mips_opc_truncl_func;
mips_opc_func_t mips_opc_ceil_func;
mips_opc_func_t mips_opc_floorl_func;
mips_opc_func_t mips_opc_roundw_func;
mips_opc_func_t mips_opc_truncw_func;
mips_opc_func_t mips_opc_ceilw_func;
mips_opc_func_t mips_opc_floorw_func;
mips_opc_func_t mips_opc_cvts_func;
mips_opc_func_t mips_opc_cvtd_func;
mips_opc_func_t mips_opc_cvtw_func;
mips_opc_func_t mips_opc_cvtl_func;
mips_opc_func_t mips_opc_cf_func;
mips_opc_func_t mips_opc_cun_func;
mips_opc_func_t mips_opc_ceq_func;
mips_opc_func_t mips_opc_cueq_func;
mips_opc_func_t mips_opc_colt_func;
mips_opc_func_t mips_opc_cult_func;
mips_opc_func_t mips_opc_cole_func;
mips_opc_func_t mips_opc_cule_func;
mips_opc_func_t mips_opc_csf_func;
mips_opc_func_t mips_opc_cngle_func;
mips_opc_func_t mips_opc_cseq_func;
mips_opc_func_t mips_opc_cngl_func;
mips_opc_func_t mips_opc_clt_func;
mips_opc_func_t mips_opc_cnge_func;
mips_opc_func_t mips_opc_cle_func;
mips_opc_func_t mips_opc_cngt_func;

/* reg imm */
mips_opc_func_t mips_opc_bltz_func;
mips_opc_func_t mips_opc_bgez_func;
mips_opc_func_t mips_opc_bltzl_func;
mips_opc_func_t mips_opc_bgezl_func;
mips_opc_func_t mips_opc_tgei_func;
mips_opc_func_t mips_opc_tgeiu_func;
mips_opc_func_t mips_opc_tlti_func;
mips_opc_func_t mips_opc_tltiu_func;
mips_opc_func_t mips_opc_teqi_func;
mips_opc_func_t mips_opc_tnei_func;
mips_opc_func_t mips_opc_bltzal_func;
mips_opc_func_t mips_opc_bgezal_func;
mips_opc_func_t mips_opc_bltzall_func;
mips_opc_func_t mips_opc_bgezall_func;

/* cop0 */
mips_opc_func_t mips_opc_tlbr_func;
mips_opc_func_t mips_opc_tlbwi_func;
mips_opc_func_t mips_opc_tlbwr_func;
mips_opc_func_t mips_opc_tlbp_func;
mips_opc_func_t mips_opc_rfe_func;
mips_opc_func_t mips_opc_eref_func;
mips_opc_func_t mips_opc_wait_func;
