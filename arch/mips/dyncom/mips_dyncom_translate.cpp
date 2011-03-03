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
	mips_opc_table_main[18] =  &mips_opc_cop2_func;

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

#define MIPS_OPC_FUNC(instr) ((instr) & 0x3f)
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

#define MIPS_OPC_RT(instr) (((instr) >> 16) & 0x1f)
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

#define MIPS_OPC_FMT(instr) (((instr >> 21) & 0x1f))
static mips_opc_func_t *mips_get_opc_cop1(uint32_t instr)
{
	uint32_t funcopc = MIPS_OPC_FUNC(instr);
	uint32_t fmt = MIPS_OPC_RT(instr);

	printf(" in decode cop1 , do nothing \n");
}

#define MIPS_OPC_MAIN(instr) (((instr) >> 26) & 0x3f)
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

#if 0
#include "dyncom/tag.h"
int arch_mips_tag_instr(cpu_t *cpu, addr_t pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc) {
	//uint32_t instr = INSTR(pc);
	uint32_t instr;
	if(bus_read(32, pc, &instr)){

	}
	switch(instr >> 26) {
		case 0x00: //INCPU_SPECIAL
			switch(instr & 0x3F) {
				case 0x08: //INCPUS_JR
					//XXX is not necessarily a return!
					*tag = TAG_RET | TAG_DELAY_SLOT;
					break;
				case 0x09:  //INCPUS_JALR
					*new_pc = NEW_PC_NONE;
					*tag = TAG_BRANCH | TAG_DELAY_SLOT;
					break;
				case 0x01: //IN_invalid
				case 0x05: //IN_invalid
				case 0x0A: //IN_invalid
				case 0x0B: //IN_invalid
				case 0x0C: //INCPUS_SYSCALL
				case 0x0D: //INCPUS_BREAK
				case 0x0E: //IN_invalid
				case 0x15: //IN_invalid
				case 0x28: //IN_invalid
				case 0x29: //IN_invalid
				case 0x35: //IN_invalid
				case 0x37: //IN_invalid
				case 0x39: //IN_invalid
				case 0x3D: //IN_invalid
					*tag = TAG_TRAP;
					break;
				default:
					*tag = TAG_CONTINUE;
					break;
			}
			break;
		case 0x01: //INCPU_REGIMM
			switch (GetRegimmInstruction) {
				case 0x00: //INCPUR_BLTZ
				case 0x01: //INCPUR_BGEZ
					*new_pc = MIPS_BRANCH_TARGET;
					*tag = TAG_COND_BRANCH | TAG_DELAY_SLOT;
					break;
				case 0x10: //INCPUR_BLTZAL
				case 0x11: //INCPUR_BGEZAL
					*new_pc = MIPS_BRANCH_TARGET;
					*tag = TAG_CALL | TAG_DELAY_SLOT;
					break;
				case 0x02: //INCPUR_BLTZL
				case 0x03: //INCPUR_BGEZL
					*new_pc = MIPS_BRANCH_TARGET;
					*tag = TAG_COND_BRANCH | TAG_DELAY_SLOT;
					break;
				case 0x12: //INCPUR_BLTZALL
				case 0x13: //INCPUR_BGEZALL
					*new_pc = MIPS_BRANCH_TARGET;
					*tag = TAG_CALL | TAG_DELAY_SLOT;
					break;
				case 0x04: //IN_invalid
				case 0x05: //IN_invalid
				case 0x06: //IN_invalid
				case 0x07: //IN_invalid
				case 0x0D: //IN_invalid
				case 0x0F: //IN_invalid
				case 0x14: //IN_invalid
				case 0x15: //IN_invalid
				case 0x16: //IN_invalid
				case 0x17: //IN_invalid
				case 0x18: //IN_invalid
				case 0x19: //IN_invalid
				case 0x1A: //IN_invalid
				case 0x1B: //IN_invalid
				case 0x1C: //IN_invalid
				case 0x1D: //IN_invalid
				case 0x1E: //IN_invalid
				case 0x1F: //IN_invalid
					*tag = TAG_TRAP;
					break;
				default:
					*tag = TAG_CONTINUE;
					break;
			}
		case 0x02: //INCPU_J
			*new_pc = (pc & 0xF0000000) | (GetTarget << 2);
			*tag = TAG_COND_BRANCH;
		case 0x03: //INCPU_JAL
			*new_pc = (pc & 0xF0000000) | (GetTarget << 2);
			*tag = TAG_CALL;
			break;
		case 0x04: //INCPU_BEQ
			if (!RS && !RT) { // special case: B
				*new_pc = MIPS_BRANCH_TARGET;
				*tag = TAG_BRANCH | TAG_DELAY_SLOT;
			} else {
				*new_pc = MIPS_BRANCH_TARGET;
				*tag = TAG_COND_BRANCH | TAG_DELAY_SLOT;
			}
			break;
		case 0x05: //INCPU_BNE
		case 0x06: //INCPU_BLEZ
		case 0x07: //INCPU_BGTZ
			*new_pc = MIPS_BRANCH_TARGET;
			*tag = TAG_COND_BRANCH | TAG_DELAY_SLOT;
			break;
		case 0x10: //INCPU_COP0
			// we don't translate any of the INCPU_COP0 branch
			*tag = TAG_TRAP;
			break;
		case 0x11: //INCPU_COP1
			switch(GetFMT) {
				case 0x03: //IN_invalid
				case 0x07: //IN_invalid
				case 0x09: //IN_invalid
				case 0x0A: //IN_invalid
				case 0x0B: //IN_invalid
				case 0x0C: //IN_invalid
				case 0x0D: //IN_invalid
				case 0x0E: //IN_invalid
				case 0x0F: //IN_invalid
					*tag = TAG_TRAP;
					break;
				case 0x10: //INCOP1_S
					switch(GetCOP1FloatInstruction) {
						case 0x00: //INCOP1_ADD
								case 0x10: //IN_invalid
								case 0x11: //IN_invalid
								case 0x12: //IN_invalid
								case 0x13: //IN_invalid
								case 0x14: //IN_invalid
								case 0x15: //IN_invalid
								case 0x16: //IN_invalid
								case 0x17: //IN_invalid
								case 0x18: //IN_invalid
								case 0x19: //IN_invalid
								case 0x1A: //IN_invalid
								case 0x1B: //IN_invalid
								case 0x1C: //IN_invalid
								case 0x1D: //IN_invalid
								case 0x1E: //IN_invalid
								case 0x1F: //IN_invalid
								case 0x22: //IN_invalid
								case 0x23: //IN_invalid
								case 0x26: //IN_invalid
								case 0x27: //IN_invalid
								case 0x28: //IN_invalid
								case 0x29: //IN_invalid
								case 0x2A: //IN_invalid
								case 0x2B: //IN_invalid
								case 0x2C: //IN_invalid
								case 0x2D: //IN_invalid
								case 0x2E: //IN_invalid
								case 0x2F: //IN_invalid
									*tag = TAG_TRAP;
									break;
								default:
									*tag = TAG_CONTINUE;
									break;
							}
				case 0x12: //IN_invalid
				case 0x13: //IN_invalid
				case 0x16: //IN_invalid
				case 0x17: //IN_invalid
				case 0x18: //IN_invalid
				case 0x19: //IN_invalid
				case 0x1A: //IN_invalid
				case 0x1B: //IN_invalid
				case 0x1C: //IN_invalid
				case 0x1D: //IN_invalid
				case 0x1E: //IN_invalid
				case 0x1F: //IN_invalid
					*tag = TAG_TRAP;
					break;
				default:
					*tag = TAG_CONTINUE;
					break;
			}
		case 0x14: //INCPU_BEQL
		case 0x15: //INCPU_BNEL
		case 0x16: //INCPU_BLEZL
		case 0x17: //INCPU_BGTZL
			*new_pc = MIPS_BRANCH_TARGET;
			*tag = TAG_COND_BRANCH | TAG_DELAY_SLOT;
			break;
		case 0x12: //IN_invalid
		case 0x13: //IN_invalid
		case 0x1C: //IN_invalid
		case 0x1D: //IN_invalid
		case 0x1E: //IN_invalid
		case 0x1F: //IN_invalid
		case 0x32: //IN_invalid
		case 0x33: //IN_invalid
		case 0x36: //IN_invalid
		case 0x3A: //IN_invalid
		case 0x3B: //IN_invalid
		case 0x3E: //IN_invalid
			*tag = TAG_TRAP;
			break;
		default:
			*tag = TAG_CONTINUE;
			break;
	}
	if (*tag & TAG_DELAY_SLOT)
		*next_pc = pc + 8;
	else
		*next_pc = pc + 4;
	return 4;
}
#endif
//////////////////////////////////////////////////////////////////////

Value *
arch_mips_get_imm(cpu_t *cpu, uint32_t instr, uint32_t bits, bool sext,
  BasicBlock *bb) {
	uint64_t imm;
	if (sext)
		imm = (uint64_t)(sint16_t)GetImmediate;
	else
		imm = (uint64_t)(uint16_t)GetImmediate;

	return ConstantInt::get(getIntegerType(bits? bits : cpu->info.word_size), imm);
}

#define IMM arch_mips_get_imm(cpu, instr, 0, true, bb)
#define IMMU arch_mips_get_imm(cpu, instr, 0, false, bb)
#define IMM32 arch_mips_get_imm(cpu, instr, 32, true, bb)

//////////////////////////////////////////////////////////////////////

Value *
arch_mips_get_sa(cpu_t *cpu, uint32_t instr, uint32_t bits, BasicBlock *bb) {
	return ConstantInt::get(getIntegerType(bits? bits : cpu->info.word_size), GetSA);
}

#define SA arch_mips_get_sa(cpu, instr, 0, bb)
#define SA32 arch_mips_get_sa(cpu, instr, 32, bb)

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#define LET_PC(v) new StoreInst(v, cpu->ptr_PC, bb)

#define LINKr(i) LET32(i, CONST((uint64_t)(sint64_t)(sint32_t)pc+8))

#define LINK LINKr(31)

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#if 0
Value *
arch_mips_translate_cond(cpu_t *cpu, addr_t pc, BasicBlock *bb)
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
#endif

#if 0
int
arch_mips_translate_instr(cpu_t *cpu, addr_t pc, BasicBlock *bb)
{
#define BAD printf("%s:%d\n", __func__, __LINE__); exit(1);
#define LOGX LOG("%s:%d\n", __func__, __LINE__);

//LOG("%s:%d %p, %p\n", __func__, __LINE__, bb_dispatch, bb);

	//uint32_t instr = INSTR(pc);
	uint32_t instr;

	if(bus_read(32, pc, &instr)){

	}
	LOG("translating (%08llx) %08x\n", pc, instr);

	switch(instr >> 26) {
	case 0x00: /* INCPU_SPECIAL */
		switch(instr & 0x3F) {
			case 0x00: /* INCPUS_SLL */		LET32(RD,SHL(R32(RT),SA32));	break; /* XXX special case NOP? */
			case 0x02: /* INCPUS_SRL */		LET32(RD,LSHR(R32(RT),SA32));	break;
			case 0x03: /* INCPUS_SRA */		LET32(RD,ASHR(R32(RT),SA32));	break;
			case 0x04: /* INCPUS_SLLV */	LET32(RD,SHL(R32(RT),R32(RS)));	break;
			case 0x06: /* INCPUS_SRLV */	LET32(RD,LSHR(R32(RT),R32(RS)));	break;
			case 0x07: /* INCPUS_SRAV */	LET32(RD,ASHR(R32(RT),R32(RS)));	break;
			case 0x08: /* INCPUS_JR */
			{
				LET_PC(R(RS));
				break;
			}
			case 0x09: /* INCPUS_JALR */
			{
				LET_PC(SUB(R(RS),CONST(4)));
				LINKr(RD);
				break;
			}
			case 0x0C: /* INCPUS_SYSCALL */	BAD;
			case 0x0D: /* INCPUS_BREAK */	BAD;
			case 0x0F: /* INCPUS_SYNC */	BAD;
			case 0x10: /* INCPUS_MFHI */	BAD;
			case 0x11: /* INCPUS_MTHI */	BAD;
			case 0x12: /* INCPUS_MFLO */	BAD;
			case 0x13: /* INCPUS_MTLO */	BAD;
			case 0x14: /* INCPUS_DSLLV */	BAD;
			case 0x16: /* INCPUS_DSRLV */	BAD;
			case 0x17: /* INCPUS_DSRAV */	BAD;
			case 0x18: /* INCPUS_MULT */	BAD;
			case 0x19: /* INCPUS_MULTU */	BAD;
			case 0x1A: /* INCPUS_DIV */		BAD;
			case 0x1B: /* INCPUS_DIVU */	BAD;
			case 0x1C: /* INCPUS_DMULT */	BAD;
			case 0x1D: /* INCPUS_DMULTU */	BAD;
			case 0x1E: /* INCPUS_DDIV */	BAD;
			case 0x1F: /* INCPUS_DDIVU */	BAD;
			case 0x20: /* INCPUS_ADD */		LET32(RD,ADD(R32(RS), R32(RT)));		break; //XXX same??
			case 0x21: /* INCPUS_ADDU */	LET32(RD,ADD(R32(RS), R32(RT)));		break; //XXX same??
			case 0x22: /* INCPUS_SUB */		LET32(RD,SUB(R32(RS), R32(RT)));		break; //XXX same??
			case 0x23: /* INCPUS_SUBU */	LET32(RD,SUB(R32(RS), R32(RT)));		break; //XXX same??
			case 0x24: /* INCPUS_AND */		LET32(RD,AND(R32(RS), R32(RT)));		break;
			case 0x25: /* INCPUS_OR */		LET32(RD,OR(R32(RS), R32(RT)));		break;
			case 0x26: /* INCPUS_XOR */		LET32(RD,XOR(R32(RS), R32(RT)));		break;
			case 0x27: /* INCPUS_NOR */		LET32(RD,XOR(OR(R32(RS), R32(RT)),CONST32((unsigned long)-1)));	break;
			case 0x2A: /* INCPUS_SLT */		LET_ZEXT(RD,ICMP_SLT(R(RS),R(RT)));	break;
			case 0x2B: /* INCPUS_SLTU */	LET_ZEXT(RD,ICMP_ULT(R(RS),R(RT)));	break;
			case 0x2C: /* INCPUS_DADD */	LET(RD,ADD(R(RS), R(RT)));			break; //XXX same??
			case 0x2D: /* INCPUS_DADDU */	LET(RD,ADD(R(RS), R(RT)));			break; //XXX same??
			case 0x2E: /* INCPUS_DSUB */	LET(RD,SUB(R(RS), R(RT)));			break; //XXX same??
			case 0x2F: /* INCPUS_DSUBU */	LET(RD,SUB(R(RS), R(RT)));			break; //XXX same??
			case 0x30: /* INCPUS_TGE */		BAD;
			case 0x31: /* INCPUS_TGEU */	BAD;
			case 0x32: /* INCPUS_TLT */		BAD;
			case 0x33: /* INCPUS_TLTU */	BAD;
			case 0x34: /* INCPUS_TEQ */		BAD;
			case 0x36: /* INCPUS_TNE */		BAD;
			case 0x38: /* INCPUS_DSLL */	LET(RD,SHL(R(RT),SA));				break;
			case 0x3A: /* INCPUS_DSRL */	LET(RD,LSHR(R(RT),SA));			break;
			case 0x3B: /* INCPUS_DSRA */	LET(RD,ASHR(R(RT),SA));			break;
			case 0x3C: /* INCPUS_DSLL32 */	LET(RD,SHL(R(RT),ADD(SA,CONST(32))));		break;
			case 0x3E: /* INCPUS_DSRL32 */	LET(RD,LSHR(R(RT),ADD(SA,CONST(32))));		break;
			case 0x3F: /* INCPUS_DSRA32 */	LET(RD,ASHR(R(RT),ADD(SA,CONST(32))));		break;
			default:
				printf("INVALID %s:%d\n", __func__, __LINE__); exit(1);
		}
		break;
	case 0x01: /* INCPU_REGIMM */
		switch (GetRegimmInstruction) {
			case 0x00: /* INCPUR_BLTZ */	/* jump */;	break;
			case 0x01: /* INCPUR_BGEZ */	/* jump */;	break;
			case 0x02: /* INCPUR_BLTZL */	BAD; /* jump likely */;	break;
			case 0x03: /* INCPUR_BGEZL */	BAD; /* jump likely */;	break;
			case 0x08: /* INCPUR_TGEI */	BAD;
			case 0x09: /* INCPUR_TGEIU */	BAD;
			case 0x0A: /* INCPUR_TLTI */	BAD;
			case 0x0B: /* INCPUR_TLTIU */	BAD;
			case 0x0C: /* INCPUR_TEQI */	BAD;
			case 0x0E: /* INCPUR_TNEI */	BAD;
			case 0x10: /* INCPUR_BLTZAL */	LINK;	break;
			case 0x11: /* INCPUR_BGEZAL */	LINK;	break;
			case 0x12: /* INCPUR_BLTZALL */	LINK; BAD; /* likely */	break;
			case 0x13: /* INCPUR_BGEZALL */	LINK; BAD; /* likely */	break;
			default:
				printf("INVALID %s:%d\n", __func__, __LINE__); exit(1);
		}
	case 0x02: /* INCPU_J */
		break;
	case 0x03: /* INCPU_JAL */
		LINK;
		break;
	case 0x04: /* INCPU_BEQ */		/* jump */;	break;
	case 0x05: /* INCPU_BNE */		/* jump */;	break;
	case 0x06: /* INCPU_BLEZ */		/* jump */;	break;
	case 0x07: /* INCPU_BGTZ */		/* jump */;	break;
	case 0x08: /* INCPU_ADDI */		LET32(RT, ADD(R32(RS), IMM32));					break; //XXX same??
	case 0x09: /* INCPU_ADDIU */	LET32(RT, ADD(R32(RS), IMM32));					break; //XXX same??
	case 0x0A: /* INCPU_SLTI */		LET_ZEXT(RT,ICMP_ULT(R(RS),IMM));				break; //XXX same??
	case 0x0B: /* INCPU_SLTIU */	LET_ZEXT(RT,ICMP_ULT(R(RS),IMM));				break; //XXX same??
	case 0x0C: /* INCPU_ANDI */		LET(RT,AND(R(RS), IMMU));						break;
	case 0x0D: /* INCPU_ORI */		LET(RT,OR(R(RS), IMMU));						break;
	case 0x0E: /* INCPU_XORI */		LET(RT,XOR(R(RS), IMMU));						break;
	case 0x0F: /* INCPU_LUI */		LET(RT,SHL(IMMU,CONST(16)));				break;
	case 0x10: /* INCPU_COP0 */
		switch (GetFMT) {
			case 0x00: /* INCOP0_MFC0 */	BAD;
			case 0x04: /* INCOP0_MTC0 */	BAD;
			case 0x10: /* INCOP0_TLB */
				switch(GetFunction) {
					case 0x01: /* INCOP0TLB_TLBR */	BAD;
					case 0x02: /* INCOP0TLB_TLBWI */	BAD;
					case 0x06: /* INCOP0TLB_TLBWR */	BAD;
					case 0x08: /* INCOP0TLB_TLBP */	BAD;
					case 0x18: /* INCOP0TLB_ERET */	BAD;
					default:
						printf("INVALID %s:%d\n", __func__, __LINE__); exit(1);
				}
			default:
				printf("INVALID %s:%d\n", __func__, __LINE__); exit(1);
		}
	case 0x11: /* INCPU_COP1 */
		switch(GetFMT) {
			case 0x00: /* INCOP1_MFC1 */	BAD;
			case 0x01: /* INCOP1_DMFC1 */	BAD;
			case 0x02: /* INCOP1_CFC1 */	BAD;
			case 0x04: /* INCOP1_MTC1 */	BAD;
			case 0x05: /* INCOP1_DMTC1 */	BAD;
			case 0x06: /* INCOP1_CTC1 */	BAD;
			case 0x08: /* INCOP1_BC */	BAD;
			case 0x10: /* INCOP1_S */
				switch(GetCOP1FloatInstruction) {
					case 0x00: /* INCOP1_ADD */	BAD;
					case 0x01: /* INCOP1_SUB */	BAD;
					case 0x02: /* INCOP1_MUL */	BAD;
					case 0x03: /* INCOP1_DIV */	BAD;
					case 0x04: /* INCOP1_SQRT */	BAD;
					case 0x05: /* INCOP1_ABS */	BAD;
					case 0x06: /* INCOP1_MOV */	BAD;
					case 0x07: /* INCOP1_NEG */	BAD;
					case 0x08: /* INCOP1_ROUND_L */	BAD;
					case 0x09: /* INCOP1_TRUNC_L */	BAD;
					case 0x0A: /* INCOP1_CEIL_L */	BAD;
					case 0x0B: /* INCOP1_FLOOR_L */	BAD;
					case 0x0C: /* INCOP1_ROUND_W */	BAD;
					case 0x0D: /* INCOP1_TRUNC_W */	BAD;
					case 0x0E: /* INCOP1_CEIL_W */	BAD;
					case 0x0F: /* INCOP1_FLOOR_W */	BAD;
					case 0x20: /* INCOP1_CVT_S */	BAD;
					case 0x21: /* INCOP1_CVT_D */	BAD;
					case 0x24: /* INCOP1_CVT_W */	BAD;
					case 0x25: /* INCOP1_CVT_L */	BAD;
					case 0x30: /* INCOP1_C_F */	BAD;
					case 0x31: /* INCOP1_C_UN */	BAD;
					case 0x32: /* INCOP1_C_EQ */	BAD;
					case 0x33: /* INCOP1_C_UEQ */	BAD;
					case 0x34: /* INCOP1_C_OLT */	BAD;
					case 0x35: /* INCOP1_C_ULT */	BAD;
					case 0x36: /* INCOP1_C_OLE */	BAD;
					case 0x37: /* INCOP1_C_ULE */	BAD;
					case 0x38: /* INCOP1_C_SF */	BAD;
					case 0x39: /* INCOP1_C_NGLE */	BAD;
					case 0x3A: /* INCOP1_C_SEQ */	BAD;
					case 0x3B: /* INCOP1_C_NGL */	BAD;
					case 0x3C: /* INCOP1_C_LT */	BAD;
					case 0x3D: /* INCOP1_C_NGE */	BAD;
					case 0x3E: /* INCOP1_C_LE */	BAD;
					case 0x3F: /* INCOP1_C_NGT */	BAD;
					default:
						printf("INVALID %s:%d\n", __func__, __LINE__); exit(1);
				}
			case 0x11: /* INCOP1_D */	BAD;
			case 0x14: /* INCOP1_W */	BAD;
			case 0x15: /* INCOP1_L */	BAD;
			default:
				printf("INVALID %s:%d\n", __func__, __LINE__); exit(1);
		}
	case 0x14: /* INCPU_BEQL */		BAD; /* jump likely */			break;
	case 0x15: /* INCPU_BNEL */		BAD; /* jump likely */			break;
	case 0x16: /* INCPU_BLEZL */	BAD; /* jump likely */	break;
	case 0x17: /* INCPU_BGTZL */	BAD; /* jump likely */	break;
	case 0x18: /* INCPU_DADDI */	LET(RT,ADD(R(RS),IMM));								break; //XXX same??
	case 0x19: /* INCPU_DADDIU */	LET(RT,ADD(R(RS),IMM));								break; //XXX same??
	case 0x1A: /* INCPU_LDL */		BAD;
	case 0x1B: /* INCPU_LDR */		BAD;
	case 0x20: /* INCPU_LB */		LOAD8S(RT,ADD(R32(RS),IMM32));							break;
	case 0x21: /* INCPU_LH */		LOAD16S(RT,ADD(R32(RS),IMM32));						break;
	case 0x22: /* INCPU_LWL */		BAD;
	case 0x23: /* INCPU_LW */		LOAD32(RT,ADD(R32(RS),IMM32));							break; //XXX ignores misalignment
	case 0x24: /* INCPU_LBU */		LOAD8(RT,ADD(R32(RS),IMM32));							break;
	case 0x25: /* INCPU_LHU */		LOAD16(RT,ADD(R32(RS),IMM32));							break;
	case 0x26: /* INCPU_LWR */		BAD;
	case 0x27: /* INCPU_LWU */		BAD;
	case 0x28: /* INCPU_SB */		STORE8(R(RT),ADD(R32(RS),IMM32));							break;
	case 0x29: /* INCPU_SH */		STORE16(R(RT),ADD(R32(RS),IMM32));							break;
	case 0x2A: /* INCPU_SWL */		BAD;
	case 0x2B: /* INCPU_SW */		STORE32(R(RT),ADD(R32(RS),IMM32));						break;
	case 0x2C: /* INCPU_SDL */	BAD;
	case 0x2D: /* INCPU_SDR */	BAD;
	case 0x2E: /* INCPU_SWR */	BAD;
	case 0x2F: /* INCPU_CACHE */	break; /* no-op */
	case 0x30: /* INCPU_LL */		break; /* no-op */
	case 0x31: /* INCPU_LWC1 */	BAD;
	case 0x34: /* INCPU_LLD */		break; /* no-op */
	case 0x35: /* INCPU_LDC1 */	BAD;
	case 0x37: /* INCPU_LD */	BAD;
	case 0x38: /* INCPU_SC */		break; /* no-op */
	case 0x39: /* INCPU_SWC1 */	BAD;
	case 0x3C: /* INCPU_SCD */	BAD;
	case 0x3D: /* INCPU_SDC1 */	BAD;
	case 0x3F: /* INCPU_SD */	BAD;
	default:
		printf("INVALID %s:%d\n", __func__, __LINE__); exit(1);
	}

	tag_t dummy1;
	addr_t dummy2, dummy3;
	return arch_mips_tag_instr(cpu, pc, &dummy1, &dummy2, &dummy3);
}
#endif

//printf("%s:%d PC=$%04X\n", __func__, __LINE__, pc);
//printf("%s:%d\n", __func__, __LINE__);
