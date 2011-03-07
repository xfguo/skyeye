
#ifndef __MIPS_DYNCOM_DEC__
#define __MIPS_DYNCOM_DEC__
//////////////////////////////////////////////////////////////////////
// MIPS: instruction decoding
//////////////////////////////////////////////////////////////////////

#define RS	((instr >> 21) & 0x1F)
#define RT	((instr >> 16) & 0x1F)
#define RD	((instr >> 11) & 0x1F)
#define SA	((instr >> 6) & 0x1F)
#define BASE	((instr >> 16) & 0x1F)
#define OFFSET	(instr & 0xFFFF)

#define SHAMT	((instr >> 6) & 0x1F)
#define FUNCT	(instr & 0x3F)

#define FMT     ((instr >> 21) & 0x1F)
#define FT	((instr >> 16) & 0x1F)
#define FS	((instr >> 11) & 0x1F)
#define FD	((instr >> 6) & 0x1F)
#define COND	(instr & 0xF)

#define COFUN	(instr & 0x1FFFFFF)

#define BAD_INSTR do{printf("BAD instr in %s\n", __func__); exit(0);}while(0);
#define BAD do{printf("BAD instr in %s\n", __func__); exit(0);}while(0);

#define GetSA	((instr >> 6) & 0x1F)
#define GetImmediate (instr & 0xFFFF)
#define GetTarget (instr & 0x3FFFFFF)

#define GetFunction (instr & 0x3F)
//#define GetSpecialInstruction GetFunction
#define GetRegimmInstruction RT
#define GetFMT RS
//#define GetCacheType (RT&BitM2)
//#define GetCacheInstr ((RT>>2)&BitM3)
#define GetCOP1FloatInstruction GetFunction

#define MIPS_BRANCH_TARGET ((uint32_t)(phys_pc + 4 + (uint32_t)(((sint32_t)(sint16_t)GetImmediate<<2))))
#define MIPS_INSN_SIZE 4

#define IMM arch_mips_get_imm(cpu, instr, 0, true, bb)
#define IMMU arch_mips_get_imm(cpu, instr, 0, false, bb)
#define IMM32 arch_mips_get_imm(cpu, instr, 32, true, bb)

#define HI 32
#define LO 33
#define LET_PC(v) new StoreInst(v, cpu->ptr_PC, bb)
#define PC new LoadInst(cpu->ptr_PC, "", false, bb)
#define LINKr(i) LET32(i, CONST((uint64_t)(sint64_t)(sint32_t)pc+8))
#define LINK LINKr(31)

#define nothing_special         0       // nothing special
#define branch_delay            1       // current instruction in the branch-delay slot
#define instr_addr_error        2       // instruction address error
#define branch_nodelay  3       // syscall instruction or eret instruction

Value *arch_mips_get_imm(cpu_t *cpu, uint32_t instr, uint32_t bits, bool sext, BasicBlock *bb);
typedef int (*tag_func_t)(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc);
typedef int (*translate_func_t)(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
typedef Value *(*translate_cond_func_t)(cpu_t *cpu, uint32_t instr, BasicBlock *bb);

typedef struct mips_opc_func_s{
	tag_func_t tag;
	translate_func_t translate;
	translate_cond_func_t translate_cond;
}mips_opc_func_t;


extern mips_opc_func_t mips_opc_j_func;
extern mips_opc_func_t mips_opc_jal_func;
extern mips_opc_func_t mips_opc_beq_func;
extern mips_opc_func_t mips_opc_bne_func;
extern mips_opc_func_t mips_opc_blez_func;
extern mips_opc_func_t mips_opc_bgtz_func;
extern mips_opc_func_t mips_opc_addi_func;
extern mips_opc_func_t mips_opc_addiu_func;
extern mips_opc_func_t mips_opc_slti_func;
extern mips_opc_func_t mips_opc_sltiu_func;
extern mips_opc_func_t mips_opc_andi_func;
extern mips_opc_func_t mips_opc_ori_func;
extern mips_opc_func_t mips_opc_xori_func;
extern mips_opc_func_t mips_opc_lui_func;
/*extern mips_opc_func_t mips_opc_cop0_func;*/ /* cop0 func has a single table */
/*extern mips_opc_func_t mips_opc_cop1_func;*/ /* cop1 func has a single table */
extern mips_opc_func_t mips_opc_cop2_func;
extern mips_opc_func_t mips_opc_beql_func;
extern mips_opc_func_t mips_opc_bnel_func;
extern mips_opc_func_t mips_opc_blezl_func;
extern mips_opc_func_t mips_opc_bgtzl_func;
extern mips_opc_func_t mips_opc_daddi_func;
extern mips_opc_func_t mips_opc_daddiu_func;
extern mips_opc_func_t mips_opc_ldl_func;
extern mips_opc_func_t mips_opc_ldr_func;
//extern mips_opc_func_t mips_opc_spec2_func;/* specail func has a single table */
extern mips_opc_func_t mips_opc_lb_func;
extern mips_opc_func_t mips_opc_lh_func;
extern mips_opc_func_t mips_opc_lwl_func;
extern mips_opc_func_t mips_opc_lw_func;
extern mips_opc_func_t mips_opc_lbu_func;
extern mips_opc_func_t mips_opc_lhu_func;
extern mips_opc_func_t mips_opc_lwr_func;
extern mips_opc_func_t mips_opc_lwu_func;
extern mips_opc_func_t mips_opc_sb_func;
extern mips_opc_func_t mips_opc_sh_func;
extern mips_opc_func_t mips_opc_swl_func;
extern mips_opc_func_t mips_opc_sw_func;
extern mips_opc_func_t mips_opc_sdl_func;
extern mips_opc_func_t mips_opc_sdr_func;
extern mips_opc_func_t mips_opc_swr_func;
extern mips_opc_func_t mips_opc_cache_func;
extern mips_opc_func_t mips_opc_ll_func;
extern mips_opc_func_t mips_opc_lwc1_func;
extern mips_opc_func_t mips_opc_lwc2_func;
extern mips_opc_func_t mips_opc_pref_func;
extern mips_opc_func_t mips_opc_lld_func;
extern mips_opc_func_t mips_opc_ldc1_func;
extern mips_opc_func_t mips_opc_ldc2_func;
extern mips_opc_func_t mips_opc_ld_func;
extern mips_opc_func_t mips_opc_sc_func;
extern mips_opc_func_t mips_opc_swc1_func;
extern mips_opc_func_t mips_opc_swc2_func;
extern mips_opc_func_t mips_opc_scd_func;
extern mips_opc_func_t mips_opc_sdc1_func;
extern mips_opc_func_t mips_opc_sd_func;

/* spec */
extern mips_opc_func_t mips_opc_sll_func;
extern mips_opc_func_t mips_opc_null_func;
extern mips_opc_func_t mips_opc_srl_func;
extern mips_opc_func_t mips_opc_sra_func;
extern mips_opc_func_t mips_opc_sllv_func;
extern mips_opc_func_t mips_opc_srlv_func;
extern mips_opc_func_t mips_opc_srav_func;
extern mips_opc_func_t mips_opc_jr_func;
extern mips_opc_func_t mips_opc_jalr_func;
extern mips_opc_func_t mips_opc_movz_func;
extern mips_opc_func_t mips_opc_movn_func;
extern mips_opc_func_t mips_opc_syscall_func;
extern mips_opc_func_t mips_opc_break_func;
extern mips_opc_func_t mips_opc_sync_func;
extern mips_opc_func_t mips_opc_mfhi_func;
extern mips_opc_func_t mips_opc_mthi_func;
extern mips_opc_func_t mips_opc_mflo_func;
extern mips_opc_func_t mips_opc_mtlo_func;
extern mips_opc_func_t mips_opc_dsllv_func;
extern mips_opc_func_t mips_opc_dsrlv_func;
extern mips_opc_func_t mips_opc_dsrav_func;
extern mips_opc_func_t mips_opc_mult_func;
extern mips_opc_func_t mips_opc_multu_func;
extern mips_opc_func_t mips_opc_div_func;
extern mips_opc_func_t mips_opc_divu_func;
extern mips_opc_func_t mips_opc_dmult_func;
extern mips_opc_func_t mips_opc_dmultu_func;
extern mips_opc_func_t mips_opc_ddiv_func;
extern mips_opc_func_t mips_opc_ddivu_func;
extern mips_opc_func_t mips_opc_add_func;
extern mips_opc_func_t mips_opc_addu_func;
extern mips_opc_func_t mips_opc_sub_func;
extern mips_opc_func_t mips_opc_subu_func;
extern mips_opc_func_t mips_opc_and_func;
extern mips_opc_func_t mips_opc_or_func;
extern mips_opc_func_t mips_opc_xor_func;
extern mips_opc_func_t mips_opc_nor_func;
extern mips_opc_func_t mips_opc_slt_func;
extern mips_opc_func_t mips_opc_sltu_func;
extern mips_opc_func_t mips_opc_dadd_func;
extern mips_opc_func_t mips_opc_daddu_func;
extern mips_opc_func_t mips_opc_dsub_func;
extern mips_opc_func_t mips_opc_dsubu_func;
extern mips_opc_func_t mips_opc_tge_func;
extern mips_opc_func_t mips_opc_tgeu_func;
extern mips_opc_func_t mips_opc_tlt_func;
extern mips_opc_func_t mips_opc_tltu_func;
extern mips_opc_func_t mips_opc_teq_func;
extern mips_opc_func_t mips_opc_tne_func;
extern mips_opc_func_t mips_opc_dsll_func;
extern mips_opc_func_t mips_opc_dsrl_func;
extern mips_opc_func_t mips_opc_dsra_func;
extern mips_opc_func_t mips_opc_dsll32_func;
extern mips_opc_func_t mips_opc_dsrl32_func;
extern mips_opc_func_t mips_opc_dsra32_func;

/* spec 2 */
extern mips_opc_func_t mips_opc_fadd_func;
extern mips_opc_func_t mips_opc_fsub_func;
extern mips_opc_func_t mips_opc_fmul_func;
extern mips_opc_func_t mips_opc_fdiv_func;
extern mips_opc_func_t mips_opc_sqrt_func;
extern mips_opc_func_t mips_opc_abs_func;
extern mips_opc_func_t mips_opc_mov_func;
extern mips_opc_func_t mips_opc_neg_func;
extern mips_opc_func_t mips_opc_roundl_func;
extern mips_opc_func_t mips_opc_truncl_func;
extern mips_opc_func_t mips_opc_ceil_func;
extern mips_opc_func_t mips_opc_floorl_func;
extern mips_opc_func_t mips_opc_roundw_func;
extern mips_opc_func_t mips_opc_truncw_func;
extern mips_opc_func_t mips_opc_ceilw_func;
extern mips_opc_func_t mips_opc_floorw_func;
extern mips_opc_func_t mips_opc_cvts_func;
extern mips_opc_func_t mips_opc_cvtd_func;
extern mips_opc_func_t mips_opc_cvtw_func;
extern mips_opc_func_t mips_opc_cvtl_func;
extern mips_opc_func_t mips_opc_cf_func;
extern mips_opc_func_t mips_opc_cun_func;
extern mips_opc_func_t mips_opc_ceq_func;
extern mips_opc_func_t mips_opc_cueq_func;
extern mips_opc_func_t mips_opc_colt_func;
extern mips_opc_func_t mips_opc_cult_func;
extern mips_opc_func_t mips_opc_cole_func;
extern mips_opc_func_t mips_opc_cule_func;
extern mips_opc_func_t mips_opc_csf_func;
extern mips_opc_func_t mips_opc_cngle_func;
extern mips_opc_func_t mips_opc_cseq_func;
extern mips_opc_func_t mips_opc_cngl_func;
extern mips_opc_func_t mips_opc_clt_func;
extern mips_opc_func_t mips_opc_cnge_func;
extern mips_opc_func_t mips_opc_cle_func;
extern mips_opc_func_t mips_opc_cngt_func;

/* reg imm */
extern mips_opc_func_t mips_opc_bltz_func;
extern mips_opc_func_t mips_opc_bgez_func;
extern mips_opc_func_t mips_opc_bltzl_func;
extern mips_opc_func_t mips_opc_bgezl_func;
extern mips_opc_func_t mips_opc_tgei_func;
extern mips_opc_func_t mips_opc_tgeiu_func;
extern mips_opc_func_t mips_opc_tlti_func;
extern mips_opc_func_t mips_opc_tltiu_func;
extern mips_opc_func_t mips_opc_teqi_func;
extern mips_opc_func_t mips_opc_tnei_func;
extern mips_opc_func_t mips_opc_bltzal_func;
extern mips_opc_func_t mips_opc_bgezal_func;
extern mips_opc_func_t mips_opc_bltzall_func;
extern mips_opc_func_t mips_opc_bgezall_func;

extern mips_opc_func_t mips_opc_tlbr_func;
extern mips_opc_func_t mips_opc_tlbwi_func;
extern mips_opc_func_t mips_opc_tlbwr_func;
extern mips_opc_func_t mips_opc_tlbp_func;
extern mips_opc_func_t mips_opc_rfe_func;
extern mips_opc_func_t mips_opc_eref_func;
extern mips_opc_func_t mips_opc_wait_func;
#endif
