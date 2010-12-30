#ifndef __PPC_DYNCOM_INSTR_GROUP2__
#define __PPC_DYNCOM_INSTR_GROUP2__

#ifdef __cplusplus
 extern "C" {
#endif

extern ppc_opc_func_t ppc_opc_cmp_func;
extern ppc_opc_func_t ppc_opc_tw_func;
extern ppc_opc_func_t ppc_opc_subfcx_func;//+
extern ppc_opc_func_t ppc_opc_addcx_func;//+
extern ppc_opc_func_t ppc_opc_mulhwux_func;
extern ppc_opc_func_t ppc_opc_isel_func;
extern ppc_opc_func_t ppc_opc_mfcr_func;
extern ppc_opc_func_t ppc_opc_lwarx_func;
extern ppc_opc_func_t ppc_opc_lwzx_func;
extern ppc_opc_func_t ppc_opc_slwx_func;
extern ppc_opc_func_t ppc_opc_cntlzwx_func;
extern ppc_opc_func_t ppc_opc_andx_func;
extern ppc_opc_func_t ppc_opc_cmpl_func;
extern ppc_opc_func_t ppc_opc_subfx_func;
extern ppc_opc_func_t ppc_opc_iselgt_func;
extern ppc_opc_func_t ppc_opc_dcbst_func;
extern ppc_opc_func_t ppc_opc_lwzux_func;
extern ppc_opc_func_t ppc_opc_andcx_func;
extern ppc_opc_func_t ppc_opc_mulhwx_func;
extern ppc_opc_func_t ppc_opc_iseleq_func;
extern ppc_opc_func_t ppc_opc_mfmsr_func;
extern ppc_opc_func_t ppc_opc_dcbf_func;
extern ppc_opc_func_t ppc_opc_lbzx_func;
extern ppc_opc_func_t ppc_opc_negx_func;
extern ppc_opc_func_t ppc_opc_lbzux_func;
extern ppc_opc_func_t ppc_opc_norx_func;
extern ppc_opc_func_t ppc_opc_wrtee_func;
extern ppc_opc_func_t ppc_opc_subfex_func;//+
extern ppc_opc_func_t ppc_opc_addex_func;//+
extern ppc_opc_func_t ppc_opc_isel_func;
extern ppc_opc_func_t ppc_opc_mtcrf_func;
extern ppc_opc_func_t ppc_opc_mtmsr_func;
extern ppc_opc_func_t ppc_opc_stwcx__func;
extern ppc_opc_func_t ppc_opc_stwx_func;
extern ppc_opc_func_t ppc_opc_wrteei_func;
extern ppc_opc_func_t ppc_opc_dcbtls_func;
extern ppc_opc_func_t ppc_opc_stwux_func;
extern ppc_opc_func_t ppc_opc_subfzex_func;//+
extern ppc_opc_func_t ppc_opc_addzex_func;//+
extern ppc_opc_func_t ppc_opc_isel_func;
extern ppc_opc_func_t ppc_opc_mtsr_func;
extern ppc_opc_func_t ppc_opc_stbx_func;
extern ppc_opc_func_t ppc_opc_subfmex_func;//+
extern ppc_opc_func_t ppc_opc_addmex_func;
extern ppc_opc_func_t ppc_opc_mullwx_func;//+
extern ppc_opc_func_t ppc_opc_mtsrin_func;
extern ppc_opc_func_t ppc_opc_dcbtst_func;
extern ppc_opc_func_t ppc_opc_stbux_func;
extern ppc_opc_func_t ppc_opc_addx_func;//+
extern ppc_opc_func_t ppc_opc_dcbt_func;
extern ppc_opc_func_t ppc_opc_lhzx_func;
extern ppc_opc_func_t ppc_opc_eqvx_func;
extern ppc_opc_func_t ppc_opc_tlbie_func;
extern ppc_opc_func_t ppc_opc_eciwx_func;
extern ppc_opc_func_t ppc_opc_lhzux_func;
extern ppc_opc_func_t ppc_opc_xorx_func;
extern ppc_opc_func_t ppc_opc_mfspr_func;
extern ppc_opc_func_t ppc_opc_lhax_func;
extern ppc_opc_func_t ppc_opc_isel_func;
extern ppc_opc_func_t ppc_opc_tlbia_func;
extern ppc_opc_func_t ppc_opc_mfspr_func;
extern ppc_opc_func_t ppc_opc_lhaux_func;
extern ppc_opc_func_t ppc_opc_sthx_func;
extern ppc_opc_func_t ppc_opc_orcx_func;
extern ppc_opc_func_t ppc_opc_ecowx_func;
extern ppc_opc_func_t ppc_opc_sthux_func;
extern ppc_opc_func_t ppc_opc_orx_func;
extern ppc_opc_func_t ppc_opc_divwux_func;//+
extern ppc_opc_func_t ppc_opc_isel_func;
extern ppc_opc_func_t ppc_opc_mtspr_func;
extern ppc_opc_func_t ppc_opc_dcbi_func;
extern ppc_opc_func_t ppc_opc_nandx_func;
extern ppc_opc_func_t ppc_opc_divwx_func;//+
extern ppc_opc_func_t ppc_opc_mcrxr_func;
extern ppc_opc_func_t ppc_opc_isel_func;
extern ppc_opc_func_t ppc_opc_lswx_func;
extern ppc_opc_func_t ppc_opc_lwbrx_func;
extern ppc_opc_func_t ppc_opc_lfsx_func;
extern ppc_opc_func_t ppc_opc_srwx_func;
extern ppc_opc_func_t ppc_opc_tlbsync_func;
extern ppc_opc_func_t ppc_opc_lfsux_func;
extern ppc_opc_func_t ppc_opc_isel_func;
extern ppc_opc_func_t ppc_opc_mfsr_func;
extern ppc_opc_func_t ppc_opc_lswi_func;
extern ppc_opc_func_t ppc_opc_sync_func;
extern ppc_opc_func_t ppc_opc_lfdx_func;
extern ppc_opc_func_t ppc_opc_lfdux_func;
extern ppc_opc_func_t ppc_opc_mfsrin_func;
extern ppc_opc_func_t ppc_opc_stswx_func;
extern ppc_opc_func_t ppc_opc_stwbrx_func;
extern ppc_opc_func_t ppc_opc_stfsx_func;
extern ppc_opc_func_t ppc_opc_stfsux_func;
extern ppc_opc_func_t ppc_opc_stswi_func;
extern ppc_opc_func_t ppc_opc_stfdx_func;
extern ppc_opc_func_t ppc_opc_dcba_func;
extern ppc_opc_func_t ppc_opc_stfdux_func;
extern ppc_opc_func_t ppc_opc_isel_func;
extern ppc_opc_func_t ppc_opc_tlbivax_func; /* TLB invalidated virtual address indexed */
extern ppc_opc_func_t ppc_opc_lhbrx_func;
extern ppc_opc_func_t ppc_opc_srawx_func;
extern ppc_opc_func_t ppc_opc_isel_func;
extern ppc_opc_func_t ppc_opc_srawix_func;
extern ppc_opc_func_t ppc_opc_isel_func;
extern ppc_opc_func_t ppc_opc_eieio_func;
extern ppc_opc_func_t ppc_opc_isel_func;
extern ppc_opc_func_t ppc_opc_tlbsx_func;
extern ppc_opc_func_t ppc_opc_sthbrx_func;
extern ppc_opc_func_t ppc_opc_extshx_func;
extern ppc_opc_func_t ppc_opc_tlbrehi_func;
extern ppc_opc_func_t ppc_opc_extsbx_func;
extern ppc_opc_func_t ppc_opc_isel_func;
extern ppc_opc_func_t ppc_opc_isel_func;
extern ppc_opc_func_t ppc_opc_tlbwe_func; /* TLB write entry */
extern ppc_opc_func_t ppc_opc_icbi_func;
extern ppc_opc_func_t ppc_opc_stfiwx_func;
extern ppc_opc_func_t ppc_opc_dcbz_func;
extern ppc_opc_func_t ppc_opc_dss_func;      /*Temporarily modify*/
extern ppc_opc_func_t ppc_opc_lvsl_func;
extern ppc_opc_func_t ppc_opc_lvebx_func;
extern ppc_opc_func_t ppc_opc_lvsr_func;
extern ppc_opc_func_t ppc_opc_lvehx_func;
extern ppc_opc_func_t ppc_opc_lvewx_func;
extern ppc_opc_func_t ppc_opc_lvx_func;
extern ppc_opc_func_t ppc_opc_stvebx_func;
extern ppc_opc_func_t ppc_opc_stvehx_func;
extern ppc_opc_func_t ppc_opc_stvewx_func;
extern ppc_opc_func_t ppc_opc_stvx_func;
extern ppc_opc_func_t ppc_opc_dst_func;
extern ppc_opc_func_t ppc_opc_lvxl_func;
extern ppc_opc_func_t ppc_opc_dstst_func;
extern ppc_opc_func_t ppc_opc_stvxl_func;
extern ppc_opc_func_t ppc_opc_dss_func;

#ifdef __cplusplus
}
#endif
#endif
