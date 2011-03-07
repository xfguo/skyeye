#include "llvm/Instructions.h"
#include <skyeye_dyncom.h>
#include "dyncom/dyncom_llvm.h"
#include "dyncom/frontend.h"
#include "mips_internal.h"
#include "bank_defs.h"
#include "mips_dyncom_dec.h"
#include "dyncom/tag.h"
#include "mips_dyncom_instr_main.h"
#include "mips_dyncom_instr_special.h"
#include "mips_dyncom_instr_regimm.h"

Value *
opc_translate_cond(cpu_t *cpu, addr_t pc, BasicBlock *bb)
{
	//uint32_t instr = INSTR(pc);
	uint32_t instr;
	if(bus_read(32, pc, &instr)){

	}

	LOG("cond (%08llx) %08x\n", pc, instr);

	switch(instr >> 26) {
	case 0x01: /* INCPU_REGIMM */
		switch (GetRegimmInstruction) {
			case 0x00: /* INCPUR_BLTZ */	return ICMP_SLT(R(RS),CONST(0));
			case 0x01: /* INCPUR_BGEZ */	return ICMP_SGE(R(RS),CONST(0));
			case 0x02: /* INCPUR_BLTZL */	return ICMP_SLT(R(RS),CONST(0));
			case 0x03: /* INCPUR_BGEZL */	return ICMP_SGE(R(RS),CONST(0));
			case 0x10: /* INCPUR_BLTZAL */	return ICMP_SLT(R(RS),CONST(0));
			case 0x11: /* INCPUR_BGEZAL */	return ICMP_SGE(R(RS),CONST(0));
			case 0x12: /* INCPUR_BLTZALL */	return ICMP_SLT(R(RS),CONST(0));
			case 0x13: /* INCPUR_BGEZALL */	return ICMP_SGE(R(RS),CONST(0));
		}
	case 0x04: /* INCPU_BEQ */
		if (!RS && !RT) // special case: B
			return NULL; /* should never be reached */
		else
			return ICMP_EQ(R(RS), R(RT));
	case 0x05: /* INCPU_BNE */		return ICMP_NE(R(RS), R(RT));
	case 0x06: /* INCPU_BLEZ */		return ICMP_SLE(R(RS),CONST(0));
	case 0x07: /* INCPU_BGTZ */		return ICMP_SGT(R(RS),CONST(0));
	case 0x14: /* INCPU_BEQL */		return ICMP_EQ(R(RS), R(RT));
	case 0x15: /* INCPU_BNEL */		return ICMP_NE(R(RS), R(RT));
	case 0x16: /* INCPU_BLEZL */	return ICMP_SLE(R(RS), CONST(0));
	case 0x17: /* INCPU_BGTZL */	return ICMP_SGT(R(RS), CONST(0));
  default: assert(0 && "Not handled"); return NULL;
	}
}

int opc_default_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	*tag = TAG_CONTINUE;
	*next_pc = phys_pc + MIPS_INSN_SIZE;

	return MIPS_INSN_SIZE;
}

int opc_branch_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	*tag = TAG_COND_BRANCH | TAG_DELAY_SLOT;
	*new_pc = MIPS_BRANCH_TARGET;
	*next_pc = phys_pc + MIPS_INSN_SIZE;

	return MIPS_INSN_SIZE;
}

int opc_call_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	*tag = TAG_CALL | TAG_DELAY_SLOT;
	*new_pc = MIPS_BRANCH_TARGET;
	*next_pc = phys_pc + MIPS_INSN_SIZE;

	return MIPS_INSN_SIZE;
}

int opc_jr_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	*tag = TAG_RET | TAG_DELAY_SLOT;

	return 0;
}

int opc_jalr_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	*new_pc = NEW_PC_NONE;
	*tag = TAG_BRANCH | TAG_DELAY_SLOT;

	return 0;
}

int opc_trap_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	*tag = TAG_TRAP;
}

/* main */
//mips_opc_func_t mips_opc_special_func;/* specail func has a single table */
//mips_opc_func_t mips_opc_regimm_func;/* regimm func has a signle table */

mips_opc_func_t mips_opc_j_func = {
	opc_jr_tag,
	opc_j_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_jal_func = {
	opc_jalr_tag,
	opc_jal_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_beq_func = {
	opc_branch_tag,
	opc_beq_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_bne_func ={
	opc_branch_tag,
	opc_beq_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_blez_func ={
	opc_branch_tag,
	opc_blez_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_bgtz_func ={
	opc_branch_tag,
	opc_bgtz_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_addi_func ={
	opc_default_tag,
	opc_addi_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_addiu_func ={
	opc_default_tag,
	opc_addiu_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_slti_func ={
	opc_default_tag,
	opc_slti_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_sltiu_func ={
	opc_default_tag,
	opc_sltiu_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_andi_func ={
	opc_default_tag,
	opc_andi_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_ori_func ={
	opc_default_tag,
	opc_ori_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_xori_func ={
	opc_default_tag,
	opc_xori_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_lui_func ={
	opc_default_tag,
	opc_lui_trans,
	opc_translate_cond
};

/*mips_opc_func_t mips_opc_cop0_func;*/ /* cop0 func has a single table */
/*mips_opc_func_t mips_opc_cop1_func;*/ /* cop1 func has a single table */
/*mips_opc_func_t mips_opc_cop2_func;*/ /* cop1 func has a single table */
mips_opc_func_t mips_opc_beql_func ={
	opc_branch_tag,
	opc_beql_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_bnel_func ={
	opc_branch_tag,
	opc_bnel_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_blezl_func ={
	opc_branch_tag,
	opc_blezl_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_bgtzl_func ={
	opc_branch_tag,
	opc_bgtzl_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_daddi_func;
mips_opc_func_t mips_opc_daddiu_func;

mips_opc_func_t mips_opc_ldl_func;
mips_opc_func_t mips_opc_ldr_func;
//mips_opc_func_t mips_opc_spec2_func;/* specail func has a single table */
mips_opc_func_t mips_opc_lb_func;

mips_opc_func_t mips_opc_lh_func ={
	opc_default_tag,
	opc_lh_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_lwl_func ={
	opc_default_tag,
	opc_lwl_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_lw_func ={
	opc_default_tag,
	opc_lwl_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_lbu_func ={
	opc_default_tag,
	opc_lbu_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_lhu_func ={
	opc_default_tag,
	opc_lhu_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_lwr_func ={
	opc_default_tag,
	opc_lwr_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_lwu_func ={
	opc_default_tag,
	opc_lwu_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_sb_func ={
	opc_default_tag,
	opc_sb_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_sh_func ={
	opc_default_tag,
	opc_sh_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_swl_func ={
	opc_default_tag,
	opc_swl_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_sw_func ={
	opc_default_tag,
	opc_sw_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_sdl_func;
mips_opc_func_t mips_opc_sdr_func;
mips_opc_func_t mips_opc_swr_func;

mips_opc_func_t mips_opc_cache_func ={
	opc_default_tag,
	opc_cache_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_ll_func ={
	opc_default_tag,
	opc_ll_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_lwc1_func;
mips_opc_func_t mips_opc_lwc2_func;
mips_opc_func_t mips_opc_pref_func;
mips_opc_func_t mips_opc_lld_func;
mips_opc_func_t mips_opc_ldc1_func;
mips_opc_func_t mips_opc_ldc2_func;
mips_opc_func_t mips_opc_ld_func;

mips_opc_func_t mips_opc_sc_func = {
	opc_default_tag,
	opc_sc_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_swc1_func;
mips_opc_func_t mips_opc_swc2_func;
mips_opc_func_t mips_opc_scd_func;
mips_opc_func_t mips_opc_sdc1_func;
mips_opc_func_t mips_opc_sd_func;

/* spec */
mips_opc_func_t mips_opc_sll_func ={
	opc_default_tag,
	opc_sll_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_null_func;

mips_opc_func_t mips_opc_srl_func ={
	opc_default_tag,
	opc_srl_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_sra_func ={
	opc_default_tag,
	opc_sra_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_sllv_func ={
	opc_default_tag,
	opc_sllv_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_srlv_func ={
	opc_default_tag,
	opc_srlv_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_srav_func ={
	opc_default_tag,
	opc_srav_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_jr_func ={
	opc_jr_tag,
	opc_jr_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_jalr_func ={
	opc_jr_tag,
	opc_jalr_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_movz_func ={
	opc_default_tag,
	opc_movz_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_movn_func ={
	opc_default_tag,
	opc_movn_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_syscall_func ={
	opc_default_tag,
	opc_syscall_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_break_func ={
	opc_default_tag,
	opc_break_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_sync_func ={
	opc_default_tag,
	opc_sync_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_mfhi_func ={
	opc_default_tag,
	opc_mfhi_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_mthi_func ={
	opc_default_tag,
	opc_mthi_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_mflo_func ={
	opc_default_tag,
	opc_mflo_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_mtlo_func ={
	opc_default_tag,
	opc_mtlo_trans,
	opc_translate_cond
};

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

mips_opc_func_t mips_opc_add_func ={
	opc_default_tag,
	opc_add_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_addu_func;

mips_opc_func_t mips_opc_sub_func ={
	opc_default_tag,
	opc_sub_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_subu_func ={
	opc_default_tag,
	opc_subu_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_and_func ={
	opc_default_tag,
	opc_and_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_or_func ={
	opc_default_tag,
	opc_or_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_xor_func ={
	opc_default_tag,
	opc_xor_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_nor_func ={
	opc_default_tag,
	opc_nor_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_slt_func ={
	opc_default_tag,
	opc_slt_trans,
	opc_translate_cond
};

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
mips_opc_func_t mips_opc_bltz_func ={
	opc_branch_tag,
	opc_bltz_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_bgez_func ={
	opc_branch_tag,
	opc_bgez_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_bltzl_func ={
	opc_branch_tag,
	opc_bltzl_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_bgezl_func ={
	opc_branch_tag,
	opc_bgezl_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_tgei_func;
mips_opc_func_t mips_opc_tgeiu_func;
mips_opc_func_t mips_opc_tlti_func;
mips_opc_func_t mips_opc_tltiu_func;
mips_opc_func_t mips_opc_teqi_func;
mips_opc_func_t mips_opc_tnei_func;

mips_opc_func_t mips_opc_bltzal_func ={
	opc_branch_tag,
	opc_bltzal_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_bgezal_func ={
	opc_branch_tag,
	opc_bgezal_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_bltzall_func ={
	opc_branch_tag,
	opc_bltzall_trans,
	opc_translate_cond
};

mips_opc_func_t mips_opc_bgezall_func ={
	opc_branch_tag,
	opc_bgezall_trans,
	opc_translate_cond
};

/* cop0 */
mips_opc_func_t mips_opc_tlbr_func;
mips_opc_func_t mips_opc_tlbwi_func;
mips_opc_func_t mips_opc_tlbwr_func;
mips_opc_func_t mips_opc_tlbp_func;
mips_opc_func_t mips_opc_rfe_func;
mips_opc_func_t mips_opc_eref_func;
mips_opc_func_t mips_opc_wait_func;
