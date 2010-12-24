/*
 *	ppc_dyncom_alu.h - Definition of some translation function for 
 *	powerpc dyncom. 
 *
 *	Copyright (C) 2010 Michael.kang (blackfin.kang@gmail.com)
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
/*
 * 05/16/2010   Michael.Kang  <blackfin.kang@gmail.com>
 */

#ifndef __PPC_DYNCOM_ALU_H__
#define __PPC_DYNCOM_ALU_H__
#include "ppc_dyncom_dec.h"

extern ppc_opc_func_t ppc_opc_addis_func;
extern ppc_opc_func_t ppc_opc_addi_func;
extern ppc_opc_func_t ppc_opc_orx_func;
extern ppc_opc_func_t ppc_opc_rlwinmx_func;

extern ppc_opc_func_t ppc_opc_twi_func;		//  3
extern ppc_opc_func_t ppc_opc_mulli_func;		//  7
extern ppc_opc_func_t ppc_opc_subfic_func;	//  8
extern ppc_opc_func_t ppc_opc_cmpli_func;
extern ppc_opc_func_t ppc_opc_cmpi_func;
extern ppc_opc_func_t ppc_opc_addic_func;		
extern ppc_opc_func_t ppc_opc_addic__func;		
extern ppc_opc_func_t ppc_opc_addi_func;
extern ppc_opc_func_t ppc_opc_addis_func;
extern ppc_opc_func_t ppc_opc_bcx_func;
extern ppc_opc_func_t ppc_opc_sc_func;
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
extern ppc_opc_func_t ppc_opc_mtspr_func;
extern ppc_opc_func_t ppc_opc_mfspr_func;

#endif
