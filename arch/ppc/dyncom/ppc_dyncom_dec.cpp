/*
 *	PearPC
 *	ppc_dec.cc
 *
 *	Copyright (C) 2003, 2004 Sebastian Biallas (sb@biallas.net)
 *	Portions Copyright (C) 2004 Daniel Foesch (dfoesch@cs.nmsu.edu)
 *	Portions Copyright (C) 2004 Apple Computer, Inc.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>

#include "skyeye_types.h"
#include <skyeye.h>
#include <skyeye_dyncom.h>

#include "tracers.h"
#include "ppc_alu.h"
#include "ppc_cpu.h"
#include "ppc_dyncom_dec.h"
#include "ppc_dyncom_alu.h"
#include "ppc_exc.h"
#include "ppc_fpu.h"
#include "ppc_vec.h"
#include "ppc_mmu.h"
#include "ppc_opc.h"

#include "ppc_dyncom_instr_group1.h"
#include "ppc_dyncom_instr_group2.h"
#include "ppc_dyncom_instr_group_f1.h"
#include "ppc_dyncom_instr_group_f2.h"
#include "ppc_dyncom_instr_groupv.h"
#include "ppc_dyncom_instr_main.h"
#include "ppc_dyncom_debug.h"

#define BAD_INSTR {fprintf(stderr, "In %s, cannot parse instruction 0x%x\n", __FUNCTION__, instr);skyeye_exit(-1);}

int opc_default_tag(cpu_t *cpu, uint32_t instr, addr_t phys_addr,tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	*tag = TAG_CONTINUE;
	*next_pc = phys_addr + PPC_INSN_SIZE;
	return PPC_INSN_SIZE;
}
int opc_default_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb){
	return 0;
}

int opc_invalid_tag(cpu_t *cpu, uint32_t instr, addr_t phys_addr,tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	BAD_INSTR;
	return -1;
}
int opc_invalid_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb){
	BAD_INSTR;
	return -1;
}
Value* opc_invalid_translate_cond(cpu_t *cpu, uint32_t instr, BasicBlock *bb){
	BAD_INSTR;
	return NULL;
}
static ppc_opc_func_t ppc_opc_invalid = {
	opc_invalid_tag,
	opc_invalid_translate,
	opc_invalid_translate_cond,
};

static ppc_opc_func_t *ppc_opc_table_main[64];
static ppc_opc_func_t *ppc_opc_table_group_1[529];
static ppc_opc_func_t *ppc_opc_table_group_2[1015];
static ppc_opc_func_t *ppc_opc_table_group_f1[32];
static ppc_opc_func_t *ppc_opc_table_group_f2[712];
/*TODO: Vector instruction will implement later */
static ppc_opc_func_t *ppc_opc_table_groupv[965];

static void ppc_opc_init_group_1(){
	uint i;
	for (i=0; i<(sizeof ppc_opc_table_group_1 / sizeof ppc_opc_table_group_1[0]); i++) {
		ppc_opc_table_group_1[i] = &ppc_opc_invalid;
	}
	ppc_opc_table_group_1[33] = &ppc_opc_crnor_func;
	ppc_opc_table_group_1[129] = &ppc_opc_crandc_func;
	ppc_opc_table_group_1[193] = &ppc_opc_crxor_func;
	ppc_opc_table_group_1[225] = &ppc_opc_crnand_func;
	ppc_opc_table_group_1[257] = &ppc_opc_crand_func;
	ppc_opc_table_group_1[289] = &ppc_opc_creqv_func;
	ppc_opc_table_group_1[417] = &ppc_opc_crorc_func;
	ppc_opc_table_group_1[449] = &ppc_opc_cror_func;
	ppc_opc_table_group_1[528] = &ppc_opc_bcctrx_func; 
	ppc_opc_table_group_1[16] = &ppc_opc_bclrx_func;
	ppc_opc_table_group_1[0] = &ppc_opc_mcrf_func;
	ppc_opc_table_group_1[50] = &ppc_opc_rfi_func;
	ppc_opc_table_group_1[150] = &ppc_opc_isync_func;
}
// main opcode 31
static void ppc_opc_init_group_2()
{
	uint i;
	for (i=0; i<(sizeof ppc_opc_table_group_2 / sizeof ppc_opc_table_group_2[0]); i++) {
		ppc_opc_table_group_2[i] = &ppc_opc_invalid;
	}
	ppc_opc_table_group_2[0] = &ppc_opc_cmp_func;
	ppc_opc_table_group_2[4] = &ppc_opc_tw_func;
	ppc_opc_table_group_2[8] = &ppc_opc_subfcx_func;//+
	ppc_opc_table_group_2[10] = &ppc_opc_addcx_func;//+
	ppc_opc_table_group_2[11] = &ppc_opc_mulhwux_func;
	ppc_opc_table_group_2[15] = &ppc_opc_isel_func;
	ppc_opc_table_group_2[19] = &ppc_opc_mfcr_func;
	ppc_opc_table_group_2[20] = &ppc_opc_lwarx_func;
	ppc_opc_table_group_2[23] = &ppc_opc_lwzx_func;
	ppc_opc_table_group_2[24] = &ppc_opc_slwx_func;
	ppc_opc_table_group_2[26] = &ppc_opc_cntlzwx_func;
	ppc_opc_table_group_2[28] = &ppc_opc_andx_func;
	ppc_opc_table_group_2[32] = &ppc_opc_cmpl_func;
	ppc_opc_table_group_2[40] = &ppc_opc_subfx_func;
	ppc_opc_table_group_2[47] = &ppc_opc_iselgt_func;
	ppc_opc_table_group_2[54] = &ppc_opc_dcbst_func;
	ppc_opc_table_group_2[55] = &ppc_opc_lwzux_func;
	ppc_opc_table_group_2[60] = &ppc_opc_andcx_func;
	ppc_opc_table_group_2[75] = &ppc_opc_mulhwx_func;
	ppc_opc_table_group_2[79] = &ppc_opc_iseleq_func;
	ppc_opc_table_group_2[83] = &ppc_opc_mfmsr_func;
	ppc_opc_table_group_2[86] = &ppc_opc_dcbf_func;
	ppc_opc_table_group_2[87] = &ppc_opc_lbzx_func;
	ppc_opc_table_group_2[104] = &ppc_opc_negx_func;
	ppc_opc_table_group_2[119] = &ppc_opc_lbzux_func;
	ppc_opc_table_group_2[124] = &ppc_opc_norx_func;
	ppc_opc_table_group_2[131] = &ppc_opc_wrtee_func;
	ppc_opc_table_group_2[136] = &ppc_opc_subfex_func;//+
	ppc_opc_table_group_2[138] = &ppc_opc_addex_func;//+
	ppc_opc_table_group_2[143] = &ppc_opc_isel_func;
	ppc_opc_table_group_2[144] = &ppc_opc_mtcrf_func;
	ppc_opc_table_group_2[146] = &ppc_opc_mtmsr_func;
	ppc_opc_table_group_2[150] = &ppc_opc_stwcx__func;
	ppc_opc_table_group_2[151] = &ppc_opc_stwx_func;
	ppc_opc_table_group_2[163] = &ppc_opc_wrteei_func;
	ppc_opc_table_group_2[166] = &ppc_opc_dcbtls_func;
	ppc_opc_table_group_2[183] = &ppc_opc_stwux_func;
	ppc_opc_table_group_2[200] = &ppc_opc_subfzex_func;//+
	ppc_opc_table_group_2[202] = &ppc_opc_addzex_func;//+
	ppc_opc_table_group_2[207] = &ppc_opc_isel_func;
	ppc_opc_table_group_2[210] = &ppc_opc_mtsr_func;
	ppc_opc_table_group_2[215] = &ppc_opc_stbx_func;
	ppc_opc_table_group_2[232] = &ppc_opc_subfmex_func;//+
	ppc_opc_table_group_2[234] = &ppc_opc_addmex_func;
	ppc_opc_table_group_2[235] = &ppc_opc_mullwx_func;//+
	ppc_opc_table_group_2[242] = &ppc_opc_mtsrin_func;
	ppc_opc_table_group_2[246] = &ppc_opc_dcbtst_func;
	ppc_opc_table_group_2[247] = &ppc_opc_stbux_func;
	ppc_opc_table_group_2[266] = &ppc_opc_addx_func;//+
	ppc_opc_table_group_2[278] = &ppc_opc_dcbt_func;
	ppc_opc_table_group_2[279] = &ppc_opc_lhzx_func;
	ppc_opc_table_group_2[284] = &ppc_opc_eqvx_func;
	ppc_opc_table_group_2[306] = &ppc_opc_tlbie_func;
	ppc_opc_table_group_2[310] = &ppc_opc_eciwx_func;
	ppc_opc_table_group_2[311] = &ppc_opc_lhzux_func;
	ppc_opc_table_group_2[316] = &ppc_opc_xorx_func;
	ppc_opc_table_group_2[339] = &ppc_opc_mfspr_func;
	ppc_opc_table_group_2[343] = &ppc_opc_lhax_func;
	ppc_opc_table_group_2[335] = &ppc_opc_isel_func;
	ppc_opc_table_group_2[370] = &ppc_opc_tlbia_func;
	ppc_opc_table_group_2[371] = &ppc_opc_mfspr_func;
	ppc_opc_table_group_2[375] = &ppc_opc_lhaux_func;
	ppc_opc_table_group_2[407] = &ppc_opc_sthx_func;
	ppc_opc_table_group_2[412] = &ppc_opc_orcx_func;
	ppc_opc_table_group_2[438] = &ppc_opc_ecowx_func;
	ppc_opc_table_group_2[439] = &ppc_opc_sthux_func;
	ppc_opc_table_group_2[444] = &ppc_opc_orx_func;
	ppc_opc_table_group_2[459] = &ppc_opc_divwux_func;//+
	ppc_opc_table_group_2[463] = &ppc_opc_isel_func;
	ppc_opc_table_group_2[467] = &ppc_opc_mtspr_func;
	ppc_opc_table_group_2[470] = &ppc_opc_dcbi_func;
	ppc_opc_table_group_2[476] = &ppc_opc_nandx_func;
	ppc_opc_table_group_2[491] = &ppc_opc_divwx_func;//+
	ppc_opc_table_group_2[512] = &ppc_opc_mcrxr_func;
	ppc_opc_table_group_2[527] = &ppc_opc_isel_func;
	ppc_opc_table_group_2[533] = &ppc_opc_lswx_func;
	ppc_opc_table_group_2[534] = &ppc_opc_lwbrx_func;
	ppc_opc_table_group_2[535] = &ppc_opc_lfsx_func;
	ppc_opc_table_group_2[536] = &ppc_opc_srwx_func;
	ppc_opc_table_group_2[566] = &ppc_opc_tlbsync_func;
	ppc_opc_table_group_2[567] = &ppc_opc_lfsux_func;
	ppc_opc_table_group_2[591] = &ppc_opc_isel_func;
	ppc_opc_table_group_2[595] = &ppc_opc_mfsr_func;
	ppc_opc_table_group_2[597] = &ppc_opc_lswi_func;
	ppc_opc_table_group_2[598] = &ppc_opc_sync_func;
	ppc_opc_table_group_2[599] = &ppc_opc_lfdx_func;
	ppc_opc_table_group_2[631] = &ppc_opc_lfdux_func;
	ppc_opc_table_group_2[659] = &ppc_opc_mfsrin_func;
	ppc_opc_table_group_2[661] = &ppc_opc_stswx_func;
	ppc_opc_table_group_2[662] = &ppc_opc_stwbrx_func;
	ppc_opc_table_group_2[663] = &ppc_opc_stfsx_func;
	ppc_opc_table_group_2[695] = &ppc_opc_stfsux_func;
	ppc_opc_table_group_2[725] = &ppc_opc_stswi_func;
	ppc_opc_table_group_2[727] = &ppc_opc_stfdx_func;
	ppc_opc_table_group_2[758] = &ppc_opc_dcba_func;
	ppc_opc_table_group_2[759] = &ppc_opc_stfdux_func;
	ppc_opc_table_group_2[783] = &ppc_opc_isel_func;
	ppc_opc_table_group_2[786] = &ppc_opc_tlbivax_func; /* TLB invalidated virtual address indexed */
	ppc_opc_table_group_2[790] = &ppc_opc_lhbrx_func;
	ppc_opc_table_group_2[792] = &ppc_opc_srawx_func;
	ppc_opc_table_group_2[815] = &ppc_opc_isel_func;
	ppc_opc_table_group_2[824] = &ppc_opc_srawix_func;
	ppc_opc_table_group_2[847] = &ppc_opc_isel_func;
	ppc_opc_table_group_2[854] = &ppc_opc_eieio_func;
	ppc_opc_table_group_2[911] = &ppc_opc_isel_func;
	ppc_opc_table_group_2[914] = &ppc_opc_tlbsx_func;
	ppc_opc_table_group_2[918] = &ppc_opc_sthbrx_func;
	ppc_opc_table_group_2[922] = &ppc_opc_extshx_func;
	ppc_opc_table_group_2[946] = &ppc_opc_tlbrehi_func;
	ppc_opc_table_group_2[954] = &ppc_opc_extsbx_func;
	ppc_opc_table_group_2[975] = &ppc_opc_isel_func;
	ppc_opc_table_group_2[943] = &ppc_opc_isel_func;
	ppc_opc_table_group_2[978] = &ppc_opc_tlbwe_func; /* TLB write entry */
	ppc_opc_table_group_2[982] = &ppc_opc_icbi_func;
	ppc_opc_table_group_2[983] = &ppc_opc_stfiwx_func;
	ppc_opc_table_group_2[1014] = &ppc_opc_dcbz_func;
	ppc_opc_table_group_2[822] = &ppc_opc_dss_func;      /*Temporarily modify*/
	if ((get_current_core()->pvr & 0xffff0000) == 0x000c0000) {
		/* Added for Altivec support */
		ppc_opc_table_group_2[6] = &ppc_opc_lvsl_func;
		ppc_opc_table_group_2[7] = &ppc_opc_lvebx_func;
		ppc_opc_table_group_2[38] = &ppc_opc_lvsr_func;
		ppc_opc_table_group_2[39] = &ppc_opc_lvehx_func;
		ppc_opc_table_group_2[71] = &ppc_opc_lvewx_func;
		ppc_opc_table_group_2[103] = &ppc_opc_lvx_func;
		ppc_opc_table_group_2[135] = &ppc_opc_stvebx_func;
		ppc_opc_table_group_2[167] = &ppc_opc_stvehx_func;
		ppc_opc_table_group_2[199] = &ppc_opc_stvewx_func;
		ppc_opc_table_group_2[231] = &ppc_opc_stvx_func;
		ppc_opc_table_group_2[342] = &ppc_opc_dst_func;
		ppc_opc_table_group_2[359] = &ppc_opc_lvxl_func;
		ppc_opc_table_group_2[374] = &ppc_opc_dstst_func;
		ppc_opc_table_group_2[487] = &ppc_opc_stvxl_func;
		ppc_opc_table_group_2[822] = &ppc_opc_dss_func;
	}
}
static void ppc_opc_init_group_f1(){
	uint i;
	for (i=0; i<(sizeof ppc_opc_table_group_f1 / sizeof ppc_opc_table_group_f1[0]); i++) {
		ppc_opc_table_group_f1[i] = &ppc_opc_invalid;
	}
	ppc_opc_table_group_f1[18] = &ppc_opc_fdivsx_func;
	ppc_opc_table_group_f1[20] = &ppc_opc_fsubsx_func;
	ppc_opc_table_group_f1[21] = &ppc_opc_faddsx_func;
	ppc_opc_table_group_f1[22] = &ppc_opc_fsqrtsx_func;
	ppc_opc_table_group_f1[24] = &ppc_opc_fresx_func;
	ppc_opc_table_group_f1[25] = &ppc_opc_fmulsx_func;
	ppc_opc_table_group_f1[28] = &ppc_opc_fmsubsx_func;
	ppc_opc_table_group_f1[29] = &ppc_opc_fmaddsx_func;
	ppc_opc_table_group_f1[30] = &ppc_opc_fnmsubsx_func;
	ppc_opc_table_group_f1[31] = &ppc_opc_fnmaddsx_func;
}
static void ppc_opc_init_group_f2(){
	uint i;
	for (i=0; i<(sizeof ppc_opc_table_group_f2 / sizeof ppc_opc_table_group_f2[0]); i++) {
		ppc_opc_table_group_f2[i] = &ppc_opc_invalid;
	}
	ppc_opc_table_group_f2[18] = &ppc_opc_fdivx_func;
	ppc_opc_table_group_f2[20] = &ppc_opc_fsubx_func;
	ppc_opc_table_group_f2[21] = &ppc_opc_faddx_func;
	ppc_opc_table_group_f2[22] = &ppc_opc_fsqrtx_func;
	ppc_opc_table_group_f2[23] = &ppc_opc_fselx_func;
	ppc_opc_table_group_f2[25] = &ppc_opc_fmulx_func;
	ppc_opc_table_group_f2[26] = &ppc_opc_frsqrtex_func;
	ppc_opc_table_group_f2[28] = &ppc_opc_fmsubx_func;
	ppc_opc_table_group_f2[29] = &ppc_opc_fmaddx_func;
	ppc_opc_table_group_f2[30] = &ppc_opc_fnmsubx_func;
	ppc_opc_table_group_f2[31] = &ppc_opc_fnmaddx_func;
	ppc_opc_table_group_f2[0] = &ppc_opc_fcmpu_func;
	ppc_opc_table_group_f2[12] = &ppc_opc_frspx_func;
	ppc_opc_table_group_f2[14] = &ppc_opc_fctiwx_func;
	ppc_opc_table_group_f2[15] = &ppc_opc_fctiwzx_func;
	ppc_opc_table_group_f2[32] = &ppc_opc_fcmpo_func;
	ppc_opc_table_group_f2[38] = &ppc_opc_mtfsb1x_func;
	ppc_opc_table_group_f2[40] = &ppc_opc_fnegx_func;
	ppc_opc_table_group_f2[64] = &ppc_opc_mcrfs_func;
	ppc_opc_table_group_f2[70] = &ppc_opc_mtfsb0x_func;
	ppc_opc_table_group_f2[72] = &ppc_opc_fmrx_func;
	ppc_opc_table_group_f2[134] = &ppc_opc_mtfsfix_func;
	ppc_opc_table_group_f2[136] = &ppc_opc_fnabsx_func;
	ppc_opc_table_group_f2[264] = &ppc_opc_fabsx_func;
	ppc_opc_table_group_f2[583] = &ppc_opc_mffsx_func;
	ppc_opc_table_group_f2[711] = &ppc_opc_mtfsfx_func;
}

static void ppc_opc_init_groupv()
{
	uint i;
	for (i=0; i<(sizeof ppc_opc_table_groupv / sizeof ppc_opc_table_groupv[0]);i++) {
		ppc_opc_table_groupv[i] = &ppc_opc_invalid;
	}
}

// main opcode 04
static ppc_opc_func_t* ppc_opc_group_v(cpu_t* cpu, BasicBlock* bb)
{
	return &ppc_opc_invalid;
}

// main opcode 19
static inline ppc_opc_func_t* ppc_opc_group_1(uint32 instr)
{
	uint32 ext = PPC_OPC_EXT(instr);
	debug(DEBUG_DEC, "In %s, ext=%d\n", __FUNCTION__, ext);
	if (ext >= (sizeof ppc_opc_table_group_1 / sizeof ppc_opc_table_group_1[0])) {
		return &ppc_opc_invalid;
	}
	return ppc_opc_table_group_1[ext];
}

// main opcode 31
static inline ppc_opc_func_t* ppc_opc_group_2(uint32 opc)
{
	uint32 ext = PPC_OPC_EXT(opc);
	debug(DEBUG_DEC, "In %s, ext=%d\n", __FUNCTION__, ext);
	if (ext >= (sizeof ppc_opc_table_group_2 / sizeof ppc_opc_table_group_2[0])) {
		return &ppc_opc_invalid;
	}
	return ppc_opc_table_group_2[ext];
}
// main opcode 59
static inline ppc_opc_func_t* ppc_opc_group_f1(uint32 instr)
{
	uint32 ext = PPC_OPC_EXT(instr);
	uint32_t index = ext & 0x1f;
	debug(DEBUG_DEC, "In %s, ext=%d\n", __FUNCTION__, ext);
	return ppc_opc_table_group_f1[index];
}
// main opcode 63
static inline ppc_opc_func_t* ppc_opc_group_f2(uint32 instr)
{
	uint32 ext = PPC_OPC_EXT(instr);
	uint32_t index = ext & 0x1f;
	debug(DEBUG_DEC, "In %s, ext=%d\n", __FUNCTION__, ext);
	return ppc_opc_table_group_f2[index];
}

/**
 * @brief The main API of decode!
 *
 * @param opc
 *
 * @return 
 */
ppc_opc_func_t* ppc_get_opc_func(uint32_t opc)
{
	uint32 mainopc = PPC_OPC_MAIN(opc);
	if(mainopc == 31)
		return ppc_opc_group_2(opc);
	else if(mainopc == 19)
		return ppc_opc_group_1(opc);
	else if(mainopc == 59)
		return ppc_opc_group_f1(opc);
	else if(mainopc == 63)
		return ppc_opc_group_f2(opc);
	else{
		debug(DEBUG_DEC, "In %s,mainopc=%d\n", __FUNCTION__, mainopc);
		return ppc_opc_table_main[mainopc];
	}
}

/**
 * @brief The init function
 */
void ppc_dyncom_dec_init()
{
	ppc_opc_init_group_1();
	ppc_opc_init_group_2();
	ppc_opc_init_group_f1();
	ppc_opc_init_group_f2();
	int i;
	for (i=0; i<(sizeof ppc_opc_table_main / sizeof ppc_opc_table_main[0]); i++) {
                ppc_opc_table_main[i] = &ppc_opc_invalid;
        }
	ppc_opc_table_main[0] = &ppc_opc_none_func;
	ppc_opc_table_main[3] = &ppc_opc_twi_func;
	ppc_opc_table_main[7] = &ppc_opc_mulli_func;
	ppc_opc_table_main[8] = &ppc_opc_subfic_func;
	ppc_opc_table_main[10] = &ppc_opc_cmpli_func;
	ppc_opc_table_main[11] = &ppc_opc_cmpi_func;
	ppc_opc_table_main[12] = &ppc_opc_addic_func;		
	ppc_opc_table_main[13] = &ppc_opc_addic__func;		
	ppc_opc_table_main[14] = &ppc_opc_addi_func;
	ppc_opc_table_main[15] = &ppc_opc_addis_func;
	ppc_opc_table_main[16] = &ppc_opc_bcx_func;
	ppc_opc_table_main[17] = &ppc_opc_sc_func;
	ppc_opc_table_main[18] = &ppc_opc_bx_func;
	ppc_opc_table_main[20] = &ppc_opc_rlwimix_func;
	ppc_opc_table_main[21] = &ppc_opc_rlwinmx_func;
	ppc_opc_table_main[23] = &ppc_opc_rlwnmx_func;
	ppc_opc_table_main[24] = &ppc_opc_ori_func;
	ppc_opc_table_main[25] = &ppc_opc_oris_func;
	ppc_opc_table_main[26] = &ppc_opc_xori_func;
	ppc_opc_table_main[27] = &ppc_opc_xoris_func;
	ppc_opc_table_main[28] = &ppc_opc_andi__func;
	ppc_opc_table_main[29] = &ppc_opc_andis__func;
	ppc_opc_table_main[32] = &ppc_opc_lwz_func;
	ppc_opc_table_main[33] = &ppc_opc_lwzu_func;
	ppc_opc_table_main[34] = &ppc_opc_lbz_func;
	ppc_opc_table_main[35] = &ppc_opc_lbzu_func;
	ppc_opc_table_main[36] = &ppc_opc_stw_func;
	ppc_opc_table_main[37] = &ppc_opc_stwu_func;
	ppc_opc_table_main[38] = &ppc_opc_stb_func;
	ppc_opc_table_main[39] = &ppc_opc_stbu_func;
	ppc_opc_table_main[40] = &ppc_opc_lhz_func;
	ppc_opc_table_main[41] = &ppc_opc_lhzu_func;
	ppc_opc_table_main[42] = &ppc_opc_lha_func;
	ppc_opc_table_main[43] = &ppc_opc_lhau_func;
	ppc_opc_table_main[44] = &ppc_opc_sth_func;
	ppc_opc_table_main[45] = &ppc_opc_sthu_func;
	ppc_opc_table_main[46] = &ppc_opc_lmw_func;
	ppc_opc_table_main[47] = &ppc_opc_stmw_func;
	ppc_opc_table_main[48] = &ppc_opc_lfs_func;
	ppc_opc_table_main[49] = &ppc_opc_lfsu_func;
	ppc_opc_table_main[50] = &ppc_opc_lfd_func;
	ppc_opc_table_main[51] = &ppc_opc_lfdu_func;
	ppc_opc_table_main[52] = &ppc_opc_stfs_func;
	ppc_opc_table_main[53] = &ppc_opc_stfsu_func;
	ppc_opc_table_main[54] = &ppc_opc_stfd_func;
	ppc_opc_table_main[55] = &ppc_opc_stfdu_func;
}
