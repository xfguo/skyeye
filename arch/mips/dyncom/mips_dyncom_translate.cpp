#include "llvm/Instructions.h"

#define OPT_LOCAL_REGISTERS //XXX

//#include "libcpu.h"
#include <skyeye_dyncom.h>
//#include "libcpu_llvm.h"
#include "dyncom/dyncom_llvm.h"
#include "dyncom/frontend.h"
#include "mips_internal.h"
#include "bank_defs.h"
#include "mips_dyncom_dec.h"

using namespace llvm;


mips_opc_func_t* mips_get_opc_func(uint32_t instr);

static mips_opc_func_t *mips_opc_table_main[64] = {0};
static mips_opc_func_t *mips_opc_table_spec[64] = {0};
static mips_opc_func_t *mips_opc_table_spec2[64] = {0};
static mips_opc_func_t *mips_opc_table_regimm[32] = {0};
static mips_opc_func_t *mips_opc_table_cop0[64] = {0};
static mips_opc_func_t *mips_opc_table_cop1[64] = {0};


//////////////////////////////////////////////////////////////////////
// tagging
//////////////////////////////////////////////////////////////////////
#include "dyncom/tag.h"
int arch_mips_tag_instr(cpu_t *cpu, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc) {
	int instr_size = 4;
	uint32_t instr;
	bus_read(32, phys_pc, &instr);

	mips_opc_func_t *opc_func = mips_get_opc_func(instr);
	if(!opc_func)
		BAD_INSTR

	assert(opc_func->tag != NULL);
	opc_func->tag(cpu, instr, phys_pc, tag, new_pc, next_pc);
	*next_pc = phys_pc + instr_size;

	if(*tag & TAG_DELAY_SLOT)
		*next_pc += 4;

	return instr_size;
}

Value *arch_mips_translate_cond(cpu_t *cpu, addr_t phys_pc, BasicBlock *bb) {
	uint32_t instr_size = 4;
	uint32_t instr;
	bus_read(32, phys_pc, &instr);

	mips_opc_func_t *opc_func = mips_get_opc_func(instr);
	if(!opc_func)
		BAD_INSTR

	return opc_func->translate_cond(cpu, instr, bb);
}

int arch_mips_translate_instr(cpu_t *cpu, addr_t phys_pc, BasicBlock *bb) {
	uint32_t instr_size = 4;
	uint32_t instr;
	bus_read(32, phys_pc, &instr);

	mips_opc_func_t *opc_func = mips_get_opc_func(instr);
	if(!opc_func)
		BAD_INSTR;
	opc_func->translate(cpu, instr, bb);
	return instr_size;
}

Value *
arch_mips_get_imm(cpu_t *cpu, uint32_t instr, uint32_t bits, bool sext,
  BasicBlock *bb) {
	uint32_t imm;
	if (sext)
		imm = (uint32_t)(sint16_t)GetImmediate;
	else
		imm = (uint32_t)(uint16_t)GetImmediate;

	return ConstantInt::get(getIntegerType(bits? bits : cpu->info.word_size), imm);
}



#define MIPS_OPC_FUNC(instr) ((instr) & 0x3f)
#define MIPS_OPC_RT(instr) (((instr) >> 16) & 0x1f)
#define MIPS_OPC_FMT(instr) (((instr >> 21) & 0x1f))
#define MIPS_OPC_MAIN(instr) (((instr) >> 26) & 0x3f)

static mips_opc_func_t *mips_get_opc_spec(uint32_t instr)
{
	uint32_t funcopc = MIPS_OPC_FUNC(instr);

	return mips_opc_table_spec[funcopc];
}

static mips_opc_func_t *mips_get_opc_spec2(uint32_t instr)
{
	uint32_t funcopc = MIPS_OPC_FUNC(instr);

	return mips_opc_table_spec2[funcopc];
}

static mips_opc_func_t *mips_get_opc_regimm(uint32_t instr)
{
	uint32_t rt = MIPS_OPC_RT(instr);

	return mips_opc_table_regimm[rt];
}

static mips_opc_func_t *mips_get_opc_cop0(uint32_t instr)
{
	uint32_t funcopc = MIPS_OPC_FUNC(instr);

	return mips_opc_table_cop0[funcopc];
}

static mips_opc_func_t *mips_get_opc_cop1(uint32_t instr)
{
	uint32_t funcopc = MIPS_OPC_FUNC(instr);
	uint32_t fmt = MIPS_OPC_RT(instr);

	printf(" in decode cop1 , do nothing \n");
}

mips_opc_func_t *mips_get_opc_func(uint32_t instr)
{
	uint32_t mainopc = MIPS_OPC_MAIN(instr);

	if(mainopc == 0)
		return mips_get_opc_spec(instr);
	else if(mainopc == 1)
		return mips_get_opc_regimm(instr);
	else if(mainopc == 16)
		return mips_get_opc_cop0(instr);
	else if(mainopc == 17)
		return mips_get_opc_cop0(instr);
	else if(mainopc == 28)
		return mips_get_opc_spec2(instr);
	else
		return mips_opc_table_main[mainopc];
}

void mips_dyncom_dec_cop0_init()
{
	mips_opc_table_cop0[0] = &mips_opc_tlbr_func;
	mips_opc_table_cop0[1] = &mips_opc_tlbwi_func;
	mips_opc_table_cop0[2] = &mips_opc_tlbwr_func;
	mips_opc_table_cop0[3] = &mips_opc_tlbp_func;
	mips_opc_table_cop0[4] = &mips_opc_rfe_func;
	mips_opc_table_cop0[5] = &mips_opc_eref_func;
	mips_opc_table_cop0[6] = &mips_opc_wait_func;
}

void mips_dyncom_dec_regimm_init()
{
	mips_opc_table_regimm[0] = &mips_opc_bltz_func;
	mips_opc_table_regimm[1] = &mips_opc_bgez_func;
	mips_opc_table_regimm[2] = &mips_opc_bltzl_func;
	mips_opc_table_regimm[3] = &mips_opc_bgezl_func;

	mips_opc_table_regimm[8] = &mips_opc_tgei_func;
	mips_opc_table_regimm[9] = &mips_opc_tgeiu_func;
	mips_opc_table_regimm[10] = &mips_opc_tlti_func;
	mips_opc_table_regimm[11] = &mips_opc_tltiu_func;
	mips_opc_table_regimm[12] = &mips_opc_teqi_func;

	mips_opc_table_regimm[14] = &mips_opc_tnei_func;

	mips_opc_table_regimm[16] = &mips_opc_bltzal_func;
	mips_opc_table_regimm[17] = &mips_opc_bgezal_func;
	mips_opc_table_regimm[18] = &mips_opc_bltzall_func;
	mips_opc_table_regimm[19] = &mips_opc_bgezall_func;
}

void mips_dyncom_dec_spec2_init()
{
	mips_opc_table_spec2[0] = &mips_opc_fadd_func;
	mips_opc_table_spec2[1] = &mips_opc_fsub_func;
	mips_opc_table_spec2[2] = &mips_opc_fmul_func;
	mips_opc_table_spec2[3] = &mips_opc_fdiv_func;
	mips_opc_table_spec2[4] = &mips_opc_sqrt_func;
	mips_opc_table_spec2[5] = &mips_opc_abs_func;
	mips_opc_table_spec2[6] = &mips_opc_mov_func;
	mips_opc_table_spec2[7] = &mips_opc_neg_func;
	mips_opc_table_spec2[8] = &mips_opc_roundl_func;
	mips_opc_table_spec2[9] = &mips_opc_truncl_func;
	mips_opc_table_spec2[10] = &mips_opc_ceil_func;
	mips_opc_table_spec2[11] = &mips_opc_floorl_func;
	mips_opc_table_spec2[12] = &mips_opc_roundw_func;
	mips_opc_table_spec2[13] = &mips_opc_truncw_func;
	mips_opc_table_spec2[14] = &mips_opc_ceilw_func;
	mips_opc_table_spec2[15] = &mips_opc_floorw_func;

	mips_opc_table_spec2[32] = &mips_opc_cvts_func;
	mips_opc_table_spec2[33] = &mips_opc_cvtd_func;

	mips_opc_table_spec2[36] = &mips_opc_cvtw_func;
	mips_opc_table_spec2[37] = &mips_opc_cvtl_func;

	mips_opc_table_spec2[48] = &mips_opc_cf_func;
	mips_opc_table_spec2[49] = &mips_opc_cun_func;
	mips_opc_table_spec2[50] = &mips_opc_ceq_func;
	mips_opc_table_spec2[51] = &mips_opc_cueq_func;
	mips_opc_table_spec2[52] = &mips_opc_colt_func;
	mips_opc_table_spec2[53] = &mips_opc_cult_func;
	mips_opc_table_spec2[54] = &mips_opc_cole_func;
	mips_opc_table_spec2[55] = &mips_opc_cule_func;
	mips_opc_table_spec2[56] = &mips_opc_csf_func;
	mips_opc_table_spec2[57] = &mips_opc_cngle_func;
	mips_opc_table_spec2[58] = &mips_opc_cseq_func;
	mips_opc_table_spec2[59] = &mips_opc_cngl_func;
	mips_opc_table_spec2[60] = &mips_opc_clt_func;
	mips_opc_table_spec2[61] = &mips_opc_cnge_func;
	mips_opc_table_spec2[62] = &mips_opc_cle_func;
	mips_opc_table_spec2[63] = &mips_opc_cngt_func;
}

void mips_dyncom_dec_spec_init()
{
	mips_opc_table_spec[0] = &mips_opc_sll_func;
	mips_opc_table_spec[2] = &mips_opc_srl_func;
	mips_opc_table_spec[3] = &mips_opc_sra_func;
	mips_opc_table_spec[4] = &mips_opc_sllv_func;
	mips_opc_table_spec[6] = &mips_opc_srlv_func;
	mips_opc_table_spec[7] = &mips_opc_srav_func;
	mips_opc_table_spec[8] = &mips_opc_jr_func;
	mips_opc_table_spec[9] = &mips_opc_jalr_func;
	mips_opc_table_spec[10] = &mips_opc_movz_func;
	mips_opc_table_spec[11] = &mips_opc_movn_func;
	mips_opc_table_spec[12] = &mips_opc_syscall_func;
	mips_opc_table_spec[13] = &mips_opc_break_func;
	mips_opc_table_spec[15] = &mips_opc_sync_func;
	mips_opc_table_spec[16] = &mips_opc_mfhi_func;
	mips_opc_table_spec[17] = &mips_opc_mthi_func;
	mips_opc_table_spec[18] = &mips_opc_mflo_func;
	mips_opc_table_spec[19] = &mips_opc_mtlo_func;
	mips_opc_table_spec[20] = &mips_opc_dsllv_func;
	mips_opc_table_spec[22] = &mips_opc_dsrlv_func;
	mips_opc_table_spec[23] = &mips_opc_dsrav_func;
	mips_opc_table_spec[24] = &mips_opc_mult_func;
	mips_opc_table_spec[25] = &mips_opc_multu_func;
	mips_opc_table_spec[26] = &mips_opc_div_func;
	mips_opc_table_spec[27] = &mips_opc_divu_func;
	mips_opc_table_spec[28] = &mips_opc_dmult_func;
	mips_opc_table_spec[29] = &mips_opc_dmultu_func;
	mips_opc_table_spec[30] = &mips_opc_ddiv_func;
	mips_opc_table_spec[31] = &mips_opc_ddivu_func;
	mips_opc_table_spec[32] = &mips_opc_add_func;
	mips_opc_table_spec[33] = &mips_opc_addu_func;
	mips_opc_table_spec[34] = &mips_opc_sub_func;
	mips_opc_table_spec[35] = &mips_opc_subu_func;
	mips_opc_table_spec[36] = &mips_opc_and_func;
	mips_opc_table_spec[37] = &mips_opc_or_func;
	mips_opc_table_spec[38] = &mips_opc_xor_func;
	mips_opc_table_spec[39] = &mips_opc_nor_func;
	mips_opc_table_spec[42] = &mips_opc_slt_func;
	mips_opc_table_spec[43] = &mips_opc_sltu_func;
	mips_opc_table_spec[44] = &mips_opc_dadd_func;
	mips_opc_table_spec[45] = &mips_opc_daddu_func;
	mips_opc_table_spec[46] = &mips_opc_dsub_func;
	mips_opc_table_spec[47] = &mips_opc_dsubu_func;
	mips_opc_table_spec[48] = &mips_opc_tge_func;
	mips_opc_table_spec[49] = &mips_opc_tgeu_func;
	mips_opc_table_spec[50] = &mips_opc_tlt_func;
	mips_opc_table_spec[51] = &mips_opc_tltu_func;
	mips_opc_table_spec[52] = &mips_opc_teq_func;
	mips_opc_table_spec[54] = &mips_opc_tne_func;
	mips_opc_table_spec[56] = &mips_opc_dsll_func;
	mips_opc_table_spec[58] = &mips_opc_dsrl_func;
	mips_opc_table_spec[59] = &mips_opc_dsra_func;
	mips_opc_table_spec[60] = &mips_opc_dsll32_func;
	mips_opc_table_spec[62] = &mips_opc_dsrl32_func;
	mips_opc_table_spec[63] = &mips_opc_dsra32_func;
}

void mips_dyncom_dec_main_init()
{
	//mips_opc_table_main[0] =  &mips_opc_special_func; /* specail func has a single table */
	//mips_opc_table_main[1] =  &mips_opc_regimm_func;  /* regimm func has a signle table */
	mips_opc_table_main[2] =  &mips_opc_j_func;
	mips_opc_table_main[3] =  &mips_opc_jal_func;
	mips_opc_table_main[4] =  &mips_opc_beq_func;
	mips_opc_table_main[5] =  &mips_opc_bne_func;
	mips_opc_table_main[6] =  &mips_opc_blez_func;
	mips_opc_table_main[7] =  &mips_opc_bgtz_func;
	mips_opc_table_main[8] =  &mips_opc_addi_func;
	mips_opc_table_main[9] =  &mips_opc_addiu_func;
	mips_opc_table_main[10] =  &mips_opc_slti_func;
	mips_opc_table_main[11] =  &mips_opc_sltiu_func;
	mips_opc_table_main[12] =  &mips_opc_andi_func;
	mips_opc_table_main[13] =  &mips_opc_ori_func;
	mips_opc_table_main[14] =  &mips_opc_xori_func;
	mips_opc_table_main[15] =  &mips_opc_lui_func;
	//mips_opc_table_main[16] =  &mips_opc_cop0_func; /* cop0 func has a single table */
	//mips_opc_table_main[17] =  &mips_opc_cop1_func;
	//mips_opc_table_main[18] =  &mips_opc_cop2_func;

	mips_opc_table_main[20] =  &mips_opc_beql_func;
	mips_opc_table_main[21] =  &mips_opc_bnel_func;
	mips_opc_table_main[22] =  &mips_opc_blezl_func;
	mips_opc_table_main[23] =  &mips_opc_bgtzl_func;
	mips_opc_table_main[24] =  &mips_opc_daddi_func;
	mips_opc_table_main[25] =  &mips_opc_daddiu_func;
	mips_opc_table_main[26] =  &mips_opc_ldl_func;
	mips_opc_table_main[27] =  &mips_opc_ldr_func;
	//mips_opc_table_main[28] =  &mips_opc_special2_func;
	mips_opc_table_main[32] =  &mips_opc_lb_func;
	mips_opc_table_main[33] =  &mips_opc_lh_func;
	mips_opc_table_main[34] =  &mips_opc_lwl_func;
	mips_opc_table_main[35] =  &mips_opc_lw_func;
	mips_opc_table_main[36] =  &mips_opc_lbu_func;
	mips_opc_table_main[37] =  &mips_opc_lhu_func;
	mips_opc_table_main[38] =  &mips_opc_lwr_func;
	mips_opc_table_main[39] =  &mips_opc_lwu_func;
	mips_opc_table_main[40] =  &mips_opc_sb_func;
	mips_opc_table_main[41] =  &mips_opc_sh_func;
	mips_opc_table_main[42] =  &mips_opc_swl_func;
	mips_opc_table_main[43] =  &mips_opc_sw_func;
	mips_opc_table_main[44] =  &mips_opc_sdl_func;
	mips_opc_table_main[45] =  &mips_opc_sdr_func;
	mips_opc_table_main[46] =  &mips_opc_swr_func;
	mips_opc_table_main[47] =  &mips_opc_cache_func;
	mips_opc_table_main[48] =  &mips_opc_ll_func;
	mips_opc_table_main[49] =  &mips_opc_lwc1_func;
	mips_opc_table_main[50] =  &mips_opc_lwc2_func;
	mips_opc_table_main[51] =  &mips_opc_pref_func;
	mips_opc_table_main[52] =  &mips_opc_lld_func;
	mips_opc_table_main[53] =  &mips_opc_ldc1_func;
	mips_opc_table_main[54] =  &mips_opc_ldc2_func;
	mips_opc_table_main[55] =  &mips_opc_ld_func;
	mips_opc_table_main[56] =  &mips_opc_sc_func;
	mips_opc_table_main[57] =  &mips_opc_swc1_func;
	mips_opc_table_main[58] =  &mips_opc_swc2_func;
	mips_opc_table_main[60] =  &mips_opc_scd_func;
	mips_opc_table_main[61] =  &mips_opc_sdc1_func;
	mips_opc_table_main[63] =  &mips_opc_sd_func;
}

void ppc_dyncom_dec_init()
{
	mips_dyncom_dec_main_init();
	mips_dyncom_dec_regimm_init();
	mips_dyncom_dec_spec_init();
	mips_dyncom_dec_spec2_init();
	mips_dyncom_dec_cop0_init();
}
