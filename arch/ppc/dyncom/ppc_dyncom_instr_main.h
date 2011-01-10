#ifndef __PPC_DYNCOM_INSTR_MAIN__
#define __PPC_DYNCOM_INSTR_MAIN__

#ifdef __cplusplus
 extern "C" {
#endif

extern ppc_opc_func_t ppc_opc_twi_func;
extern ppc_opc_func_t ppc_opc_mulli_func;
extern ppc_opc_func_t ppc_opc_subfic_func;
extern ppc_opc_func_t ppc_opc_cmpli_func;
extern ppc_opc_func_t ppc_opc_cmpi_func;
extern ppc_opc_func_t ppc_opc_addic_func;		
extern ppc_opc_func_t ppc_opc_addic__func;		
extern ppc_opc_func_t ppc_opc_addi_func;
extern ppc_opc_func_t ppc_opc_addis_func;
extern ppc_opc_func_t ppc_opc_bcx_func;
extern ppc_opc_func_t ppc_opc_sc_func;
extern ppc_opc_func_t ppc_opc_bx_func;
extern ppc_opc_func_t ppc_opc_rlwimix_func;
extern ppc_opc_func_t ppc_opc_rlwinmx_func;
extern ppc_opc_func_t ppc_opc_rlwnmx_func;
extern ppc_opc_func_t ppc_opc_ori_func;
extern ppc_opc_func_t ppc_opc_oris_func;
extern ppc_opc_func_t ppc_opc_xori_func;
extern ppc_opc_func_t ppc_opc_xoris_func;
extern ppc_opc_func_t ppc_opc_andi__func;
extern ppc_opc_func_t ppc_opc_andis__func;
extern ppc_opc_func_t ppc_opc_lwz_func;
extern ppc_opc_func_t ppc_opc_lwzu_func;
extern ppc_opc_func_t ppc_opc_lbz_func;
extern ppc_opc_func_t ppc_opc_lbzu_func;
extern ppc_opc_func_t ppc_opc_stw_func;
extern ppc_opc_func_t ppc_opc_stwu_func;
extern ppc_opc_func_t ppc_opc_stb_func;
extern ppc_opc_func_t ppc_opc_stbu_func;
extern ppc_opc_func_t ppc_opc_lhz_func;
extern ppc_opc_func_t ppc_opc_lhzu_func;
extern ppc_opc_func_t ppc_opc_lha_func;
extern ppc_opc_func_t ppc_opc_lhau_func;
extern ppc_opc_func_t ppc_opc_sth_func;
extern ppc_opc_func_t ppc_opc_sthu_func;
extern ppc_opc_func_t ppc_opc_lmw_func;
extern ppc_opc_func_t ppc_opc_stmw_func;
extern ppc_opc_func_t ppc_opc_lfs_func;
extern ppc_opc_func_t ppc_opc_lfsu_func;
extern ppc_opc_func_t ppc_opc_lfd_func;
extern ppc_opc_func_t ppc_opc_lfdu_func;
extern ppc_opc_func_t ppc_opc_stfs_func;
extern ppc_opc_func_t ppc_opc_stfsu_func;
extern ppc_opc_func_t ppc_opc_stfd_func;
extern ppc_opc_func_t ppc_opc_stfdu_func;
extern ppc_opc_func_t ppc_opc_none_func;

#ifdef __cplusplus
}
#endif

#endif
