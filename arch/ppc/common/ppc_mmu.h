/*
 *	PearPC
 *	ppc_mmu.h
 *
 *	Copyright (C) 2003, 2004 Sebastian Biallas (sb@biallas.net)
 *	Copyright (C) 2004 Daniel Foesch (dfoesch@cs.nmsu.edu)
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

#ifndef __PPC_MMU_H__
#define __PPC_MMU_H__

#include "skyeye_types.h"
#include "ppc_e500_core.h"

#ifdef __cplusplus
extern "C" {
#endif

	extern uint8 *gMemory;
	extern uint32 gMemorySize;

#define PPC_MMU_READ  1
#define PPC_MMU_WRITE 2
#define PPC_MMU_CODE  4
#define PPC_MMU_SV    8
#define PPC_MMU_NO_EXC 16

#define PPC_MMU_OK 0
#define PPC_MMU_EXC 1
#define PPC_MMU_FATAL 2

	int e500_mmu_init(e500_mmu_t * mmu);
	uint32_t get_end_of_page(uint32 phys_addr);
	uint32_t get_begin_of_page(uint32_t phys_addr);
	int ppc_effective_to_physical(e500_core_t * core, uint32 addr,
				      int flags, uint32 * result);
	int e600_effective_to_physical(e500_core_t * core, uint32 addr,
				       int flags, uint32 * result);
	int e500_effective_to_physical(e500_core_t * core, uint32 addr,
				       int flags, uint32 * result);
	bool_t ppc_mmu_set_sdr1(uint32 newval, bool_t quiesce);
	void ppc_mmu_tlb_invalidate(e500_core_t * core);

	int ppc_read_physical_dword(uint32 addr, uint64 * result);
	int ppc_read_physical_word(uint32 addr, uint32 * result);
	int ppc_read_physical_half(uint32 addr, uint16 * result);
	int ppc_read_physical_byte(uint32 addr, uint8 * result);

	int ppc_read_effective_code(uint32 addr, uint32 * result);
	int ppc_read_effective_dword(uint32 addr, uint64 * result);
	int ppc_read_effective_word(uint32 addr, uint32 * result);
	int ppc_read_effective_half(uint32 addr, uint16 * result);
	int ppc_read_effective_byte(uint32 addr, uint8 * result);

	int ppc_write_physical_dword(uint32 addr, uint64 data);
	int ppc_write_physical_word(uint32 addr, uint32 data);
	int ppc_write_physical_half(uint32 addr, uint16 data);
	int ppc_write_physical_byte(uint32 addr, uint8 data);

	int ppc_write_effective_dword(uint32 addr, uint64 data);
	int ppc_write_effective_word(uint32 addr, uint32 data);
	int ppc_write_effective_half(uint32 addr, uint16 data);
	int ppc_write_effective_byte(uint32 addr, uint8 data);

	int ppc_direct_physical_memory_handle(uint32 addr, uint8 * ptr);
	int ppc_direct_effective_memory_handle(uint32 addr, uint8 * ptr);
	int ppc_direct_effective_memory_handle_code(uint32 addr, uint8 * ptr);
	bool_t ppc_mmu_page_create(uint32 ea, uint32 pa);
	bool_t ppc_mmu_page_free(uint32 ea);
	bool_t ppc_init_physical_memory(uint32 size);

/*
pte: (page table entry)
1st word:
0     V    Valid
1-24  VSID Virtual Segment ID
25    H    Hash function
26-31 API  Abbreviated page index
2nd word:
0-19  RPN  Physical page number
20-22 res
23    R    Referenced bit
24    C    Changed bit
25-28 WIMG Memory/cache control bits
29    res
30-31 PP   Page protection bits
*/

/*
 *	MMU Opcodes
 */
	void ppc_opc_dcbz();
	void ppc_opc_dcbtls();

	void ppc_opc_lbz();
	void ppc_opc_lbzu();
	void ppc_opc_lbzux();
	void ppc_opc_lbzx();
	void ppc_opc_lfd();
	void ppc_opc_lfdu();
	void ppc_opc_lfdux();
	void ppc_opc_lfdx();
	void ppc_opc_lfs();
	void ppc_opc_lfsu();
	void ppc_opc_lfsux();
	void ppc_opc_lfsx();
	void ppc_opc_lha();
	void ppc_opc_lhau();
	void ppc_opc_lhaux();
	void ppc_opc_lhax();
	void ppc_opc_lhbrx();
	void ppc_opc_lhz();
	void ppc_opc_lhzu();
	void ppc_opc_lhzux();
	void ppc_opc_lhzx();
	void ppc_opc_lmw();
	void ppc_opc_lswi();
	void ppc_opc_lswx();
	void ppc_opc_lwarx();
	void ppc_opc_lwbrx();
	void ppc_opc_lwz();
	void ppc_opc_lwzu();
	void ppc_opc_lwzux();
	void ppc_opc_lwzx();
	void ppc_opc_lvx();	/* for altivec support */
	void ppc_opc_lvxl();
	void ppc_opc_lvebx();
	void ppc_opc_lvehx();
	void ppc_opc_lvewx();
	void ppc_opc_lvsl();
	void ppc_opc_lvsr();
	void ppc_opc_dst();

	void ppc_opc_stb();
	void ppc_opc_stbu();
	void ppc_opc_stbux();
	void ppc_opc_stbx();
	void ppc_opc_stfd();
	void ppc_opc_stfdu();
	void ppc_opc_stfdux();
	void ppc_opc_stfdx();
	void ppc_opc_stfiwx();
	void ppc_opc_stfs();
	void ppc_opc_stfsu();
	void ppc_opc_stfsux();
	void ppc_opc_stfsx();
	void ppc_opc_sth();
	void ppc_opc_sthbrx();
	void ppc_opc_sthu();
	void ppc_opc_sthux();
	void ppc_opc_sthx();
	void ppc_opc_stmw();
	void ppc_opc_stswi();
	void ppc_opc_stswx();
	void ppc_opc_stw();
	void ppc_opc_stwbrx();
	void ppc_opc_stwcx_();
	void ppc_opc_stwu();
	void ppc_opc_stwux();
	void ppc_opc_stwx();
	void ppc_opc_stvx();	/* for altivec support */
	void ppc_opc_stvxl();
	void ppc_opc_stvebx();
	void ppc_opc_stvehx();
	void ppc_opc_stvewx();
	void ppc_opc_dstst();
	void ppc_opc_dss();
	void ppc_opc_tlbivax();
	void ppc_opc_tlbwe();
	void ppc_opc_tlbrehi();
	void ppc_opc_wrteei();
	void ppc_opc_wrtee();
#ifdef __cplusplus
}
#endif
#endif
