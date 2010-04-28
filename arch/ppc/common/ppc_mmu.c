/*
 *	PearPC
 *	ppc_mmu.cc
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

/*	Pages marked: v.???
 *	From: IBM PowerPC MicroProcessor Family: Altivec(tm) Technology...
 *		Programming Environments Manual
 */

#include "debug.h"
#include "tracers.h"
#include "sysendian.h"
#include "io.h"
#include "ppc_cpu.h"
#include "ppc_fpu.h"
#include "ppc_vec.h"
#include "ppc_mmu.h"
#include "ppc_exc.h"
#include "ppc_tools.h"
#include "ppc_memory.h"
#include "ppc_e500_exc.h"

/*
ea = effective address
if translation is an instruction address then
	as = MSR[IS];
else // data address translation
	as = MSR[DS]
for all TLB entries
	if !TLB[entry].v then
		next // compare next TLB entry
	if as != TLB[entry].ts then
		next // compare next TLB entry
	if TLB[entry].tid == 0 then
		goto pid_match
	for all PID registers
		if this PID register == TLB[entry].tid then
			goto pid_match
	endfor
	next // no PIDs matched
 	pid_match://translation match
	mask = ~(1024 << (2 * TLB[entry].tsize)) - 01
	if (ea & mask) != TLB[entry].epn then
		next // no address match
	real address = TLB[entry].rpn | (ea & ~mask) // real address computed
	end translation --success
	endfor
	end translation tlb miss
*/
int e500_effective_to_physical(e500_core_t * core, uint32 addr, int flags, uint32 *result){
	int i,j;
	uint32 mask;
	ppc_tlb_entry_t *entry;
	int tlb1_index;
	int pid_match = 0;

	if((gCPU.bptr & 0x80000000) && (addr >> 12 == 0xFFFFF)){ /* if bootpage translation enabled? */
		//printf("do bootpage translation\n");
		*result = (addr & 0xFFF) | (gCPU.bptr << 12); /* please refer to P259 of MPC8572UM */
		return PPC_MMU_OK;
	}
	i = 0;
	/* walk over tlb0 and tlb1 to find the entry */
	while(i++ < (L2_TLB0_SIZE + L2_TLB1_SIZE)){
		if(i > (L2_TLB0_SIZE - 1)){
			tlb1_index = i - L2_TLB0_SIZE;
			entry = &current_core->mmu.l2_tlb1_vsp[tlb1_index];
		}
		else
			entry = &current_core->mmu.l2_tlb0_4k[i];
		if(!entry->v)
			continue;
		//if(addr == 0xfdff9080)
		//	printf("In %s,entry=0x%x, i = 0x%x, current_core->pir=0x%x\n", __FUNCTION__, entry, i, current_core->pir);
		/* FIXME, not check ts bit now */
		if(entry->ts & 0x0)
			continue;
		if(entry->tid != 0){
			/*
			for(j = 0; j < 3; j++){
				if(current_core->mmu.pid[j] == entry->tid)
					break;
			}*/
			//printf("entry->tid=0x%x\n", entry->tid);
			/* FIXME, we should check all the pid register */	
			if(current_core->mmu.pid[0] != entry->tid)
				continue;
			
		}
		if(i > (L2_TLB0_SIZE - 1)){
			int k,s = 1;
			for(k = 0; k < entry->size; k++)
				s = s * 4; 
			mask = ~((1024 * (s - 1) - 0x1) + 1024);
		}
		else
			mask = ~(1024 * 4 - 0x1);
		if(entry->size != 0xb){
			if((addr & mask) != ((entry->epn << 12) & mask))
				continue;
			/* check rwx bit */
			if(flags == PPC_MMU_WRITE){
				if(current_core->msr & 0x4000){ /* Pr =1 , we are in user mode */
					if(!(entry->usxrw & 0x8)){
						//printf("In %s,usermode,offset=0x%x, entry->usxrw=0x%x,pc=0x%x\n", __FUNCTION__, i, entry->usxrw, current_core->pc);
						ppc_exception(core, DATA_ST, flags, addr);
       			         		return PPC_MMU_EXC;
					}
				}
				else{/* Or PR is 0,we are in Supervisor mode */
					if(!(entry->usxrw & 0x4)){/* we judge SW bit */
						//printf("In %s,Super mode,entry->usxrw=0x%x,pc=0x%x\n", __FUNCTION__, entry->usxrw, current_core->pc);
        	                                ppc_exception(core, DATA_ST, flags, addr);
                	                        return PPC_MMU_EXC;
                                	}
				}
			}

			*result = (entry->rpn << 12) | (addr & ~mask); // get real address
		}
		else {/*if 4G size is mapped, we will not do address check */
			//fprintf(stderr,"warning:4G address is used.\n");
			if(addr < (entry->epn << 12))
				continue;
			 *result = (entry->rpn << 12) | (addr - (entry->epn << 12)); // get real address

		}
		return PPC_MMU_OK;
	}
	//printf("In %s,DATA_TLB exp,addr=0x%x,pc=0x%x, pir=0x%x\n", __FUNCTION__, addr, core->pc, core->pir);
	if(flags == PPC_MMU_CODE){
		ppc_exception(core, INSN_TLB, flags, addr);
		return PPC_MMU_EXC;
	}
	else{
		if(ppc_exception(core, DATA_TLB, flags, addr))
			return PPC_MMU_EXC;
	}
	return PPC_MMU_FATAL;
	
}

int e600_effective_to_physical(e500_core_t * core, uint32 addr, int flags, uint32 *result)
{
	
	int i;
	if (flags & PPC_MMU_CODE) {
		if (!(core->msr & MSR_IR)) {
			*result = addr;
			return PPC_MMU_OK;
		}
		/*
		 * BAT translation .329
		 */

		uint32 batu = (core->msr & MSR_PR ? BATU_Vp : BATU_Vs);		

		for (i=0; i<4; i++) {
			uint32 bl17 = core->ibat_bl17[i];
			uint32 addr2 = addr & (bl17 | 0xf001ffff);
			if (BATU_BEPI(addr2) == BATU_BEPI(core->ibatu[i])) {
				// bat applies to this address
				if (core->ibatu[i] & batu) {
					// bat entry valid
					uint32 offset = BAT_EA_OFFSET(addr);
					uint32 page = BAT_EA_11(addr);
					page &= ~bl17;
					page |= BATL_BRPN(core->ibatl[i]);
					// fixme: check access rights
					*result = page | offset;
					return PPC_MMU_OK;
				}
			}
		}
	} else {
		if (!(core->msr & MSR_DR)) {
			*result = addr;
			return PPC_MMU_OK;
		}
		/*
		 * BAT translation .329
		 */

		uint32 batu = (core->msr & MSR_PR ? BATU_Vp : BATU_Vs);

		for (i=0; i<4; i++) {
			uint32 bl17 = core->dbat_bl17[i];
			uint32 addr2 = addr & (bl17 | 0xf001ffff);
			if (BATU_BEPI(addr2) == BATU_BEPI(core->dbatu[i])) {
				// bat applies to this address
				if (core->dbatu[i] & batu) {
					// bat entry valid
					uint32 offset = BAT_EA_OFFSET(addr);
					uint32 page = BAT_EA_11(addr);
					page &= ~bl17;
					page |= BATL_BRPN(core->dbatl[i]);
					// fixme: check access rights
					*result = page | offset;
					return PPC_MMU_OK;
				}
			}
		}
	}
	
	/*
	 * Address translation with segment register
	 */
	uint32 sr = core->sr[EA_SR(addr)];

	if (sr & SR_T) {
		// woea
		// FIXME: implement me
		PPC_MMU_ERR("sr & T\n");
	} else {

#ifdef TLB	
		for (i=0; i<4; i++) {
			if ((addr & ~0xfff) == (core->tlb_va[i])) {
				core->tlb_last = i;
//				ht_printf("TLB: %d: %08x -> %08x\n", i, addr, core->tlb_pa[i] | (addr & 0xfff));
				*result = core->tlb_pa[i] | (addr & 0xfff);
				return PPC_MMU_OK;
			}
		}
#endif
		// page address translation
		if ((flags & PPC_MMU_CODE) && (sr & SR_N)) {
			// segment isnt executable
			if (!(flags & PPC_MMU_NO_EXC)) {
				//ppc_exception(PPC_EXC_ISI, PPC_EXC_SRR1_GUARD);         /////////////////////////////////del by yuan
				return PPC_MMU_EXC;
			}
			return PPC_MMU_FATAL;
		}
		uint32 offset = EA_Offset(addr);         // 12 bit
		uint32 page_index = EA_PageIndex(addr);  // 16 bit
		uint32 VSID = SR_VSID(sr);               // 24 bit
		uint32 api = EA_API(addr);               //  6 bit (part of page_index)
		// VSID.page_index = Virtual Page Number (VPN)
		// Hashfunction no 1 "xor" .360
		uint32 hash1 = (VSID ^ page_index);


		uint32 pteg_addr = ((hash1 & core->pagetable_hashmask)<<6) | core->pagetable_base;
		int key;
		if (core->msr & MSR_PR) {
			key = (sr & SR_Kp) ? 4 : 0;
		} else {
			key = (sr & SR_Ks) ? 4 : 0;
		}


		uint32 pte_protection_offset = ((flags&PPC_MMU_WRITE) ? 8:0) + key;

		for (i=0; i<8; i++) {
			uint32 pte;

			if (ppc_read_physical_word(pteg_addr, &pte)) {
				if (!(flags & PPC_MMU_NO_EXC)) {
					PPC_MMU_ERR("read physical in address translate failed\n");
					return PPC_MMU_EXC;
				}
				return PPC_MMU_FATAL;
			}
	//		printf("YUAN:func=%s, line=%d, pte=0x%x\n", __func__, __LINE__, pte);

			if ((pte & PTE1_V) && (!(pte & PTE1_H))) {
				if (VSID == PTE1_VSID(pte) && (api == PTE1_API(pte))) {
					// page found
					if (ppc_read_physical_word(pteg_addr+4, &pte)) {
						if (!(flags & PPC_MMU_NO_EXC)) {
							PPC_MMU_ERR("read physical in address translate failed\n");
							return PPC_MMU_EXC;
						}
						return PPC_MMU_FATAL;
					}
					// check accessmode .346
/*
					if (!ppc_pte_protection[pte_protection_offset + PTE2_PP(pte)]) {
						if (!(flags & PPC_MMU_NO_EXC)) {
							if (flags & PPC_MMU_CODE) {
								PPC_MMU_WARN("correct impl? code + read protection\n");
								ppc_exception(PPC_EXC_ISI, PPC_EXC_SRR1_PROT, addr);
								return PPC_MMU_EXC;
							} else {
								if (flags & PPC_MMU_WRITE) {
									ppc_exception(PPC_EXC_DSI, PPC_EXC_DSISR_PROT | PPC_EXC_DSISR_STORE, addr);
								} else {
									ppc_exception(PPC_EXC_DSI, PPC_EXC_DSISR_PROT, addr);
								}
								return PPC_MMU_EXC;
							}
						}
						return PPC_MMU_FATAL;
					}
	*/
					// ok..
					uint32 pap = PTE2_RPN(pte);
					*result = pap | offset;
#ifdef TLB
					core->tlb_last++;
					core->tlb_last &= 3;
					core->tlb_pa[core->tlb_last] = pap;
					core->tlb_va[core->tlb_last] = addr & ~0xfff;					
//					ht_printf("TLB: STORE %d: %08x -> %08x\n", core->tlb_last, addr, pap);
#endif
					// update access bits
					if (flags & PPC_MMU_WRITE) {
						pte |= PTE2_C | PTE2_R;
					} else {
						pte |= PTE2_R;
					}
					ppc_write_physical_word(pteg_addr+4, pte);
					return PPC_MMU_OK;
				}
			}
			pteg_addr+=8;
		}
		
		// Hashfunction no 2 "not" .360
		hash1 = ~hash1;
		pteg_addr = ((hash1 & core->pagetable_hashmask)<<6) | core->pagetable_base;
		for (i=0; i<8; i++) {
			uint32 pte;
			if (ppc_read_physical_word(pteg_addr, &pte)) {
				if (!(flags & PPC_MMU_NO_EXC)) {
					PPC_MMU_ERR("read physical in address translate failed\n");
					return PPC_MMU_EXC;
				}
				return PPC_MMU_FATAL;
			}
			if ((pte & PTE1_V) && (pte & PTE1_H)) {
				if (VSID == PTE1_VSID(pte) && (api == PTE1_API(pte))) {
					// page found
					if (ppc_read_physical_word(pteg_addr+4, &pte)) {
						if (!(flags & PPC_MMU_NO_EXC)) {
							PPC_MMU_ERR("read physical in address translate failed\n");
							return PPC_MMU_EXC;
						}
						return PPC_MMU_FATAL;
					}
					// check accessmode
					int key;
					if (core->msr & MSR_PR) {
						key = (sr & SR_Kp) ? 4 : 0;
					} else {
						key = (sr & SR_Ks) ? 4 : 0;
					}
/*
					if (!ppc_pte_protection[((flags&PPC_MMU_WRITE)?8:0) + key + PTE2_PP(pte)]) {
						if (!(flags & PPC_MMU_NO_EXC)) {
							if (flags & PPC_MMU_CODE) {
								PPC_MMU_WARN("correct impl? code + read protection\n");
								ppc_exception(PPC_EXC_ISI, PPC_EXC_SRR1_PROT, addr);
								return PPC_MMU_EXC;
							} else {
								if (flags & PPC_MMU_WRITE) {
									ppc_exception(PPC_EXC_DSI, PPC_EXC_DSISR_PROT | PPC_EXC_DSISR_STORE, addr);
								} else {
									ppc_exception(PPC_EXC_DSI, PPC_EXC_DSISR_PROT, addr);
								}
								return PPC_MMU_EXC;
							}
						}
						return PPC_MMU_FATAL;
					}
*/
					// ok..
					*result = PTE2_RPN(pte) | offset;
					
					// update access bits
					if (flags & PPC_MMU_WRITE) {
						pte |= PTE2_C | PTE2_R;
					} else {
						pte |= PTE2_R;
					}
					ppc_write_physical_word(pteg_addr+4, pte);
//					PPC_MMU_WARN("hash function 2 used!\n");
//					gSinglestep = true;
					return PPC_MMU_OK;
				}
			}
			pteg_addr+=8;
		}
	}

	// page fault
	if (!(flags & PPC_MMU_NO_EXC)) {
		if (flags & PPC_MMU_CODE) {
			e600_ppc_exception(core, PPC_EXC_ISI, PPC_EXC_SRR1_PAGE, addr);
		} else {
			if (flags & PPC_MMU_WRITE) {
				e600_ppc_exception(core, PPC_EXC_DSI, PPC_EXC_DSISR_PAGE | PPC_EXC_DSISR_STORE, addr);
			} else {
				e600_ppc_exception(core, PPC_EXC_DSI, PPC_EXC_DSISR_PAGE, addr);
			}
		}
		return PPC_MMU_EXC;
	}


	return PPC_MMU_FATAL;
}

int ppc_effective_to_physical(e500_core_t * core, uint32 addr, int flags, uint32 *result)
{
	int ret;

	if(core->pvr == 0x80040010)    /*PVR for mpc8641D*/
	{
		ret = e600_effective_to_physical(core, addr, flags, result);
		if(ret == PPC_MMU_OK)
			return PPC_MMU_OK;
	}
	else if(core->pvr == 0x8020000)  /*PVR for mpc8560*/
	{
		ret = e500_effective_to_physical(core, addr, flags, result);
		if(ret == PPC_MMU_OK)
			return PPC_MMU_OK;
	}

	return PPC_MMU_FATAL;
}

int e500_mmu_init(e500_mmu_t * mmu){
	memset(mmu, 0, sizeof(e500_mmu_t));
	/* the initial tlb map of real hardware */
	ppc_tlb_entry_t * entry = &mmu->l2_tlb1_vsp[0];
	entry->v = 1; /* entry is valid */
	entry->ts = 0; /* address space 0 */
	entry->tid = 0; /* TID value for shared(global) page */
	entry->epn = 0xFFFFF; /* Address of last 4k byte in address space*/
	entry->rpn = 0xFFFFF; /* Address of last 4k byte in address space*/
	entry->size = 0x1; /* 4k byte page size */
	/* usxrw should be initialized to 010101 */
	entry->usxrw |= 0x15; /* Full supervisor mode access allowed */
	entry->usxrw &= 0x15; /* No user mode access allowed */
	entry->wimge = 0x8; /* Caching-inhibited, non-coherent,big-endian*/
	entry->x = 0; /* Reserved system attributes */
	entry->u = 0; /* User attribute bits */
	entry->iprot = 1; /* Page is protected from invalidation */
	mmu->tlbcfg[0] = 0x4110200;
	mmu->tlbcfg[1] = 0x101bc010;
}

void ppc_mmu_tlb_invalidate()
{
	current_core->effective_code_page = 0xffffffff;
}

/*
pagetable:
min. 2^10 (64k) PTEGs
PTEG = 64byte
The page table can be any size 2^n where 16 <= n <= 25.

A PTEG contains eight
PTEs of eight bytes each; therefore, each PTEG is 64 bytes long.
*/

bool FASTCALL ppc_mmu_set_sdr1(uint32 newval, bool quiesce)
{
	/* if (newval == current_core->sdr1)*/ quiesce = false;
	PPC_MMU_TRACE("new pagetable: sdr1 = 0x%08x\n", newval);
	uint32 htabmask = SDR1_HTABMASK(newval);
	uint32 x = 1;
	uint32 xx = 0;
	int n = 0;
	while ((htabmask & x) && (n < 9)) {
		n++;
		xx|=x;
		x<<=1;
	}
	if (htabmask & ~xx) {
		PPC_MMU_TRACE("new pagetable: broken htabmask (%05x)\n", htabmask);
		return false;
	}
	uint32 htaborg = SDR1_HTABORG(newval);
	if (htaborg & xx) {
		PPC_MMU_TRACE("new pagetable: broken htaborg (%05x)\n", htaborg);
		return false;
	}
	current_core->pagetable_base = htaborg<<16;
	current_core->sdr1 = newval;
	current_core->pagetable_hashmask = ((xx<<10)|0x3ff);
	PPC_MMU_TRACE("new pagetable: sdr1 accepted\n");
	PPC_MMU_TRACE("number of pages: 2^%d pagetable_start: 0x%08x size: 2^%d\n", n+13, current_core->pagetable_base, n+16);
	if (quiesce) {
		//prom_quiesce();
	}
	return true;
}

bool FASTCALL ppc_mmu_page_create(uint32 ea, uint32 pa)
{
	uint32 sr = current_core->sr[EA_SR(ea)];
	uint32 page_index = EA_PageIndex(ea);  // 16 bit
	uint32 VSID = SR_VSID(sr);             // 24 bit
	uint32 api = EA_API(ea);               //  6 bit (part of page_index)
	uint32 hash1 = (VSID ^ page_index);
	uint32 pte, pte2;
	uint32 h = 0;
	int j;
	for (j=0; j<2; j++) {
		uint32 pteg_addr = ((hash1 & current_core->pagetable_hashmask)<<6) | current_core->pagetable_base;
		int i;
		for (i=0; i<8; i++) {
			if (ppc_read_physical_word(pteg_addr, &pte)) {
				PPC_MMU_ERR("read physical in address translate failed\n");
				return false;
			}
			if (!(pte & PTE1_V)) {
				// free pagetable entry found
				pte = PTE1_V | (VSID << 7) | h | api;
				pte2 = (PA_RPN(pa) << 12) | 0;
				if (ppc_write_physical_word(pteg_addr, pte)
				 || ppc_write_physical_word(pteg_addr+4, pte2)) {
					return false;
				} else {
					// ok
					return true;
				}
			}
			pteg_addr+=8;
		}
		hash1 = ~hash1;
		h = PTE1_H;
	}
	return false;
}

inline bool FASTCALL ppc_mmu_page_free(uint32 ea)
{
	return true;
}

inline int FASTCALL ppc_direct_physical_memory_handle(uint32 addr, byte *ptr)
{
#if 0
	if (addr < boot_romSize) {
		//ptr = &boot_rom[addr];
		return PPC_MMU_OK;
	}
#endif
	return PPC_MMU_FATAL;
}

int FASTCALL ppc_direct_effective_memory_handle(uint32 addr, byte *ptr)
{
	uint32 ea;
	int r;
	if (!((r = ppc_effective_to_physical(current_core, addr, PPC_MMU_READ, &ea)))) {
		return ppc_direct_physical_memory_handle(ea, ptr);
	}
	return r;
}

int FASTCALL ppc_direct_effective_memory_handle_code(uint32 addr, byte *ptr)
{
	uint32 ea;
	int r;
	if (!((r = ppc_effective_to_physical(current_core, addr, PPC_MMU_READ | PPC_MMU_CODE, &ea)))) {
		return ppc_direct_physical_memory_handle(ea, ptr);
	}
	return r;
}

inline int FASTCALL ppc_read_physical_qword(uint32 addr, Vector_t *result)
{
#if 0
	if (addr < boot_romSize) {
		// big endian
		VECT_D(*result,0) = ppc_dword_from_BE(*((uint64*)(boot_rom+addr)));
		VECT_D(*result,1) = ppc_dword_from_BE(*((uint64*)(boot_rom+addr+8)));
		return PPC_MMU_OK;
	}
#endif
	return io_mem_read128(addr, (uint128 *)result);
}

inline int FASTCALL ppc_read_physical_dword(uint32 addr, uint64 *result)
{
#if 0
	if (addr < boot_romSize) {
		// big endian
		*result = ppc_dword_from_BE(*((uint64*)(boot_rom+addr)));
		return PPC_MMU_OK;
	}
#endif
	int ret = io_mem_read64(addr, result);
	*result = ppc_bswap_dword(result);

	return ret;
}

int FASTCALL ppc_read_physical_word(uint32 addr, uint32 *result)
{
#if 0
	if (addr < boot_romSize) {
		// big endian
		*result = ppc_word_from_BE(*((uint32*)(boot_rom+addr)));
		return PPC_MMU_OK;
	}
#endif
	if (addr < DDR_RAM_SIZE) {
		// big endian
		*result = ppc_word_from_BE(*((int *)&ddr_ram[addr]));
		return PPC_MMU_OK;
	}
	int ret = io_mem_read(addr, result, 4);
	*result = ppc_bswap_word(result);
	return ret;
}

inline int FASTCALL ppc_read_physical_half(uint32 addr, uint16 *result)
{
#if 0
	if (addr < boot_romSize) {
		// big endian
		*result = ppc_half_from_BE(*((uint16*)(boot_rom+addr)));
		return PPC_MMU_OK;
	}
#endif
	uint32 r;
	int ret = io_mem_read(addr, r, 2);
	*result = ppc_bswap_half(r);
	return ret;
}

inline int FASTCALL ppc_read_physical_byte(uint32 addr, uint8 *result)
{
#if 0
	if (addr < boot_romSize) {
		// big endian
		*result = boot_rom[addr];
		return PPC_MMU_OK;
	}
#endif
	uint32 r;
	int ret = io_mem_read(addr, r, 1);
	*result = r;
	return ret;
}

inline int FASTCALL ppc_read_effective_code(uint32 addr, uint32 *result)
{
	if (addr & 3) {
		// EXC..bla
		return PPC_MMU_FATAL;
	}
	uint32 p;
	int r;
	if (!((r=ppc_effective_to_physical(current_core, addr, PPC_MMU_READ | PPC_MMU_CODE, &p)))) {
		return ppc_read_physical_word(p, result);
	}
	return r;
}

inline int FASTCALL ppc_read_effective_qword(uint32 addr, Vector_t *result)
{
	uint32 p;
	int r;

	addr &= ~0x0f;

	if (!(r = ppc_effective_to_physical(current_core, addr, PPC_MMU_READ, &p))) {
		return ppc_read_physical_qword(p, result);
	}

	return r;
}

inline int FASTCALL ppc_read_effective_dword(uint32 addr, uint64 *result)
{
	uint32 p;
	int r;
	if (!(r = ppc_effective_to_physical(current_core, addr, PPC_MMU_READ, &p))) {
#if 0
		if (EA_Offset(addr) > 4088) {
			// read overlaps two pages.. tricky
			byte *r1, *r2;
			byte b[14];
			ppc_effective_to_physical((addr & ~0xfff)+4089, PPC_MMU_READ, &p);
			if ((r = ppc_direct_physical_memory_handle(p, r1))) return r;
			if ((r = ppc_effective_to_physical((addr & ~0xfff)+4096, PPC_MMU_READ, &p))) return r;
			if ((r = ppc_direct_physical_memory_handle(p, r2))) return r;
			memmove(&b[0], r1, 7);
			memmove(&b[7], r2, 7);
			memmove(&result, &b[EA_Offset(addr)-4089], 8);
			result = ppc_dword_from_BE(result);
			return PPC_MMU_OK;
		} else {
			return ppc_read_physical_dword(p, result);
		}
#endif
	}
	return r;

}
inline int FASTCALL ppc_write_physical_qword(uint32 addr, Vector_t *data)
{
#if 0
	if (addr < boot_romSize) {
		// big endian
		*((uint64*)(boot_rom+addr)) = ppc_dword_to_BE(VECT_D(*data,0));
		*((uint64*)(boot_rom+addr+8)) = ppc_dword_to_BE(VECT_D(*data,1));
		return PPC_MMU_OK;
	}
#endif
	if (io_mem_write128(addr, (uint128 *)data) == IO_MEM_ACCESS_OK) {
		return PPC_MMU_OK;
	} else {
		return PPC_MMU_FATAL;
	}
}

inline int FASTCALL ppc_write_physical_dword(uint32 addr, uint64 data)
{
#if 0
	if (addr < boot_romSize) {
		// big endian
		*((uint64*)(boot_rom+addr)) = ppc_dword_to_BE(data);
		return PPC_MMU_OK;
	}
#endif
	if (io_mem_write64(addr, ppc_bswap_dword(data)) == IO_MEM_ACCESS_OK) {
		return PPC_MMU_OK;
	} else {
		return PPC_MMU_FATAL;
	}
}

inline int FASTCALL ppc_write_physical_word(uint32 addr, uint32 data)
{
#if 0
	if (addr < boot_romSize) {
		// big endian
		*((uint32*)(boot_rom+addr)) = ppc_word_to_BE(data);
		return PPC_MMU_OK;
	}
#endif
	return io_mem_write(addr, ppc_bswap_word(data), 4);
}
inline int FASTCALL ppc_write_effective_qword(uint32 addr, Vector_t data)
{
	uint32 p;
	int r;

	addr &= ~0x0f;

	if (!((r=ppc_effective_to_physical(current_core, addr, PPC_MMU_WRITE, &p)))) {
		return ppc_write_physical_qword(p, &data);
	}
	return r;
}

inline int FASTCALL ppc_write_effective_dword(uint32 addr, uint64 data)
{
	uint32 p;
	int r;
	if (!((r=ppc_effective_to_physical(current_core, addr, PPC_MMU_WRITE, &p)))) {
		if (EA_Offset(addr) > 4088) {
			// write overlaps two pages.. tricky
			byte *r1, *r2;
			byte b[14];
			ppc_effective_to_physical(current_core, (addr & ~0xfff)+4089, PPC_MMU_WRITE, &p);
			if ((r = ppc_direct_physical_memory_handle(p, r1))) return r;
			if ((r = ppc_effective_to_physical(current_core, (addr & ~0xfff)+4096, PPC_MMU_WRITE, &p))) return r;
			if ((r = ppc_direct_physical_memory_handle(p, r2))) return r;
			data = ppc_dword_to_BE(data);
			memmove(&b[0], r1, 7);
			memmove(&b[7], r2, 7);
			memmove(&b[EA_Offset(addr)-4089], &data, 8);
			memmove(r1, &b[0], 7);
			memmove(r2, &b[7], 7);
			return PPC_MMU_OK;
		} else {
			return ppc_write_physical_dword(p, data);
		}
	}
	return r;
}
/***************************************************************************
 *	DMA Interface
 */

bool	ppc_dma_write(uint32 dest, const void *src, uint32 size)
{
#if 0
	if (dest > boot_romSize || (dest+size) > boot_romSize) return false;
#endif	
	byte *ptr;
	ppc_direct_physical_memory_handle(dest, ptr);
	
	memcpy(ptr, src, size);
	return true;
}

bool	ppc_dma_read(void *dest, uint32 src, uint32 size)
{
#if 0
	if (src > boot_romSize || (src+size) > boot_romSize) return false;
#endif	
	byte *ptr;
	ppc_direct_physical_memory_handle(src, ptr);
	
	memcpy(dest, ptr, size);
	return true;
}

bool	ppc_dma_set(uint32 dest, int c, uint32 size)
{
#if 0
	if (dest > boot_romSize || (dest+size) > boot_romSize) return false;
#endif	
	byte *ptr;
	ppc_direct_physical_memory_handle(dest, ptr);
	
	memset(ptr, c, size);
	return true;
}


/***************************************************************************
 *	DEPRECATED prom interface
 */
bool ppc_prom_set_sdr1(uint32 newval, bool quiesce)
{
	return ppc_mmu_set_sdr1(newval, quiesce);
}

bool ppc_prom_effective_to_physical(uint32 *result, uint32 ea)
{
	return ppc_effective_to_physical(current_core, ea, PPC_MMU_READ|PPC_MMU_SV|PPC_MMU_NO_EXC, result) == PPC_MMU_OK;
}

bool ppc_prom_page_create(uint32 ea, uint32 pa)
{
	uint32 sr = current_core->sr[EA_SR(ea)];
	uint32 page_index = EA_PageIndex(ea);  // 16 bit
	uint32 VSID = SR_VSID(sr);             // 24 bit
	uint32 api = EA_API(ea);               //  6 bit (part of page_index)
	uint32 hash1 = (VSID ^ page_index);
	uint32 pte, pte2;
	uint32 h = 0;
	int j;
	for (j=0; j<2; j++) {
		uint32 pteg_addr = ((hash1 & current_core->pagetable_hashmask)<<6) | current_core->pagetable_base;
		int i;
		for (i=0; i<8; i++) {
			if (ppc_read_physical_word(pteg_addr, &pte)) {
				PPC_MMU_ERR("read physical in address translate failed\n");
				return false;
			}
			if (!(pte & PTE1_V)) {
				// free pagetable entry found
				pte = PTE1_V | (VSID << 7) | h | api;
				pte2 = (PA_RPN(pa) << 12) | 0;
				if (ppc_write_physical_word(pteg_addr, pte)
				 || ppc_write_physical_word(pteg_addr+4, pte2)) {
					return false;
				} else {
					// ok
					return true;
				}
			}
			pteg_addr+=8;
		}
		hash1 = ~hash1;
		h = PTE1_H;
	}
	return false;
}

bool ppc_prom_page_free(uint32 ea)
{
	return true;
}


/***************************************************************************
 *	MMU Opcodes
 */

#include "ppc_dec.h"

/*
 *	dcbz		Data Cache Clear to Zero
 *	.464
 */
void ppc_opc_dcbz()
{
#ifdef E500
	//printf("DBG:In %s, for e500,cache is not implemented.\n",__FUNCTION__);
        //PPC_L1_CACHE_LINE_SIZE
        int rA, rD, rB;
        PPC_OPC_TEMPL_X(current_core->current_opc, rD, rA, rB);
        // assert rD=0
        uint32 a = (rA?current_core->gpr[rA]:0)+current_core->gpr[rB];
        // bytes of per Cache line is 32 bytes 
        int i = 0;
        for(; i < 32; i += 4)
                ppc_write_effective_word(a + i, 0);


#else
	//PPC_L1_CACHE_LINE_SIZE
	int rA, rD, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rD, rA, rB);
	// assert rD=0
	uint32 a = (rA?current_core->gpr[rA]:0)+current_core->gpr[rB];
	// BAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
	ppc_write_effective_dword(a, 0)
	|| ppc_write_effective_dword(a+8, 0)
	|| ppc_write_effective_dword(a+16, 0)
	|| ppc_write_effective_dword(a+24, 0);
#endif
}
void ppc_opc_dcbtls(){
#ifdef E500
        //printf("DBG:In %s, for e500,cache is not implemented.\n",__FUNCTION__);
#else
	fprintf(stderr,"In %s, cache is not implemented.\n",__FUNCTION__);
#endif

}

/*
 *	lbz		Load Byte and Zero
 *	.521
 */
void ppc_opc_lbz()
{
	int rA, rD;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rD, rA, imm);
	uint8 r;
	int ret = ppc_read_effective_byte((rA?current_core->gpr[rA]:0)+imm, &r);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rD] = r;
	}
}
/*
 *	lbzu		Load Byte and Zero with Update
 *	.522
 */
void ppc_opc_lbzu()
{
	int rA, rD;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rD, rA, imm);
	// FIXME: check rA!=0 && rA!=rD
	uint8 r;
	int ret = ppc_read_effective_byte(current_core->gpr[rA]+imm, &r);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rA] += imm;
		current_core->gpr[rD] = r;
	}	
}
/*
 *	lbzux		Load Byte and Zero with Update Indexed
 *	.523
 */
void ppc_opc_lbzux()
{
	int rA, rD, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rD, rA, rB);
	// FIXME: check rA!=0 && rA!=rD
	uint8 r;
	int ret = ppc_read_effective_byte(current_core->gpr[rA]+current_core->gpr[rB], &r);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rA] += current_core->gpr[rB];
		current_core->gpr[rD] = r;
	}
}
/*
 *	lbzx		Load Byte and Zero Indexed
 *	.524
 */
void ppc_opc_lbzx()
{
	int rA, rD, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rD, rA, rB);
	uint8 r;
	int ret = ppc_read_effective_byte((rA?current_core->gpr[rA]:0)+current_core->gpr[rB], &r);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rD] = r;
	}
}
/*
 *	lfd		Load Floating-Point Double
 *	.530
 */
void ppc_opc_lfd()
{
	if ((current_core->msr & MSR_FP) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_FPU, 0, 0);
		return;
	}
	int rA, frD;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, frD, rA, imm);
	uint64 r;
	int ret = ppc_read_effective_dword((rA?current_core->gpr[rA]:0)+imm, &r);
	if (ret == PPC_MMU_OK) {
		current_core->fpr[frD] = r;
	}	
}
/*
 *	lfdu		Load Floating-Point Double with Update
 *	.531
 */
void ppc_opc_lfdu()
{
	if ((current_core->msr & MSR_FP) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_FPU, 0, 0);
		return;
	}
	int rA, frD;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, frD, rA, imm);
	// FIXME: check rA!=0
	uint64 r;
	int ret = ppc_read_effective_dword(current_core->gpr[rA]+imm, &r);
	if (ret == PPC_MMU_OK) {
		current_core->fpr[frD] = r;
		current_core->gpr[rA] += imm;
	}	
}
/*
 *	lfdux		Load Floating-Point Double with Update Indexed
 *	.532
 */
void ppc_opc_lfdux()
{
	if ((current_core->msr & MSR_FP) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_FPU, 0, 0);
		return;
	}
	int rA, frD, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, frD, rA, rB);
	// FIXME: check rA!=0
	uint64 r;
	int ret = ppc_read_effective_dword(current_core->gpr[rA]+current_core->gpr[rB], &r);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rA] += current_core->gpr[rB];
		current_core->fpr[frD] = r;
	}	
}
/*
 *	lfdx		Load Floating-Point Double Indexed
 *	.533
 */
void ppc_opc_lfdx()
{
	if ((current_core->msr & MSR_FP) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_FPU, 0, 0);
		return;
	}
	int rA, frD, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, frD, rA, rB);
	uint64 r;
	int ret = ppc_read_effective_dword((rA?current_core->gpr[rA]:0)+current_core->gpr[rB], &r);
	if (ret == PPC_MMU_OK) {
		current_core->fpr[frD] = r;
	}	
}
/*
 *	lfs		Load Floating-Point Single
 *	.534
 */
void ppc_opc_lfs()
{
	if ((current_core->msr & MSR_FP) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_FPU, 0, 0);
		return;
	}
	int rA, frD;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, frD, rA, imm);
	uint32 r;
	int ret = ppc_read_effective_word((rA?current_core->gpr[rA]:0)+imm, &r);
	if (ret == PPC_MMU_OK) {
		ppc_single s;
		ppc_double d;
		ppc_fpu_unpack_single(&s, r);
		ppc_fpu_single_to_double(&s, &d);
		ppc_fpu_pack_double(&d, &(current_core->fpr[frD]));
	}	
}
/*
 *	lfsu		Load Floating-Point Single with Update
 *	.535
 */
void ppc_opc_lfsu()
{
	if ((current_core->msr & MSR_FP) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_FPU, 0, 0);
		return;
	}
	int rA, frD;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, frD, rA, imm);
	// FIXME: check rA!=0
	uint32 r;
	int ret = ppc_read_effective_word(current_core->gpr[rA]+imm, &r);
	if (ret == PPC_MMU_OK) {
		ppc_single s;
		ppc_double d;
		ppc_fpu_unpack_single(&s, r);
		ppc_fpu_single_to_double(&s, &d);
		ppc_fpu_pack_double(&d, &(current_core->fpr[frD]));
		current_core->gpr[rA] += imm;
	}	
}
/*
 *	lfsux		Load Floating-Point Single with Update Indexed
 *	.536
 */
void ppc_opc_lfsux()
{
	if ((current_core->msr & MSR_FP) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_FPU, 0, 0);
		return;
	}
	int rA, frD, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, frD, rA, rB);
	// FIXME: check rA!=0
	uint32 r;
	int ret = ppc_read_effective_word(current_core->gpr[rA]+current_core->gpr[rB], &r);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rA] += current_core->gpr[rB];
		ppc_single s;
		ppc_double d;
		ppc_fpu_unpack_single(&s, r);
		ppc_fpu_single_to_double(&s, &d);
		ppc_fpu_pack_double(&d, &(current_core->fpr[frD]));
	}	
}
/*
 *	lfsx		Load Floating-Point Single Indexed
 *	.537
 */
void ppc_opc_lfsx()
{
	if ((current_core->msr & MSR_FP) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_FPU, 0, 0);
		return;
	}
	int rA, frD, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, frD, rA, rB);
	uint32 r;
	int ret = ppc_read_effective_word((rA?current_core->gpr[rA]:0)+current_core->gpr[rB], &r);
	if (ret == PPC_MMU_OK) {
		ppc_single s;
		ppc_double d;
		ppc_fpu_unpack_single(&s, r);
		ppc_fpu_single_to_double(&s, &d);
		ppc_fpu_pack_double(&d, &(current_core->fpr[frD]));
	}	
}
/*
 *	lha		Load Half Word Algebraic
 *	.538
 */
void ppc_opc_lha()
{
	int rA, rD;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rD, rA, imm);
	uint16 r;
	int ret = ppc_read_effective_half((rA?current_core->gpr[rA]:0)+imm, &r);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rD] = (r&0x8000)?(r|0xffff0000):r;
	}
}
/*
 *	lhau		Load Half Word Algebraic with Update
 *	.539
 */
void ppc_opc_lhau()
{
	int rA, rD;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rD, rA, imm);
	uint16 r;
	// FIXME: rA != 0
	int ret = ppc_read_effective_half(current_core->gpr[rA]+imm, &r);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rA] += imm;
		current_core->gpr[rD] = (r&0x8000)?(r|0xffff0000):r;
	}
}
/*
 *	lhaux		Load Half Word Algebraic with Update Indexed
 *	.540
 */
void ppc_opc_lhaux()
{
	int rA, rD, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rD, rA, rB);
	uint16 r;
	// FIXME: rA != 0
	int ret = ppc_read_effective_half(current_core->gpr[rA]+current_core->gpr[rB], &r);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rA] += current_core->gpr[rB];
		current_core->gpr[rD] = (r&0x8000)?(r|0xffff0000):r;
	}
}
/*
 *	lhax		Load Half Word Algebraic Indexed
 *	.541
 */
void ppc_opc_lhax()
{
	int rA, rD, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rD, rA, rB);
	uint16 r;
	// FIXME: rA != 0
	int ret = ppc_read_effective_half((rA?current_core->gpr[rA]:0)+current_core->gpr[rB], &r);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rD] = (r&0x8000) ? (r|0xffff0000):r;
	}
}
/*
 *	lhbrx		Load Half Word Byte-Reverse Indexed
 *	.542
 */
void ppc_opc_lhbrx()
{
	int rA, rD, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rD, rA, rB);
	uint16 r;
	int ret = ppc_read_effective_half((rA?current_core->gpr[rA]:0)+current_core->gpr[rB], &r);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rD] = ppc_bswap_half(r);
	}
}
/*
 *	lhz		Load Half Word and Zero
 *	.543
 */
void ppc_opc_lhz()
{
	int rA, rD;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rD, rA, imm);
	uint16 r;
	int ret = ppc_read_effective_half((rA?current_core->gpr[rA]:0)+imm, &r);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rD] = r;
	}
}
/*
 *	lhzu		Load Half Word and Zero with Update
 *	.544
 */
void ppc_opc_lhzu()
{
	int rA, rD;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rD, rA, imm);
	uint16 r;
	// FIXME: rA!=0
	int ret = ppc_read_effective_half(current_core->gpr[rA]+imm, &r);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rD] = r;
		current_core->gpr[rA] += imm;
	}
}
/*
 *	lhzux		Load Half Word and Zero with Update Indexed
 *	.545
 */
void ppc_opc_lhzux()
{
	int rA, rD, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rD, rA, rB);
	uint16 r;
	// FIXME: rA != 0
	int ret = ppc_read_effective_half(current_core->gpr[rA]+current_core->gpr[rB], &r);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rA] += current_core->gpr[rB];
		current_core->gpr[rD] = r;
	}
}
/*
 *	lhzx		Load Half Word and Zero Indexed
 *	.546
 */
void ppc_opc_lhzx()
{
	int rA, rD, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rD, rA, rB);
	uint16 r;
	int ret = ppc_read_effective_half((rA?current_core->gpr[rA]:0)+current_core->gpr[rB], &r);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rD] = r;
	}
}
/*
 *	lmw		Load Multiple Word
 *	.547
 */
void ppc_opc_lmw()
{
	int rD, rA;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rD, rA, imm);
	uint32 ea = (rA ? current_core->gpr[rA] : 0) + imm;
	while (rD <= 31) {
		if (ppc_read_effective_word(ea, &(current_core->gpr[rD]))) {
			return;
		}
		rD++;
		ea += 4;
	}
}
/*
 *	lswi		Load String Word Immediate
 *	.548
 */
void ppc_opc_lswi()
{
	int rA, rD, NB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rD, rA, NB);
	if (NB==0) NB=32;
	uint32 ea = rA ? current_core->gpr[rA] : 0;
	uint32 r = 0;
	int i = 4;
	uint8 v;
	while (NB > 0) {
		if (!i) {
			i = 4;
			current_core->gpr[rD] = r;
			rD++;
			rD%=32;
			r = 0;
		}
		if (ppc_read_effective_byte(ea, &v)) {
			return;
		}
		r<<=8;
		r|=v;
		ea++;
		i--;
		NB--;
	}
	while (i) { r<<=8; i--; }
	current_core->gpr[rD] = r;
}
/*
 *	lswx		Load String Word Indexed
 *	.550
 */
void ppc_opc_lswx()
{
	int rA, rD, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rD, rA, rB);
	int NB = XER_n(current_core->xer);
	uint32 ea = current_core->gpr[rB] + (rA ? current_core->gpr[rA] : 0);

	uint32 r = 0;
	int i = 4;
	uint8 v;
	while (NB > 0) {
		if (!i) {
			i = 4;
			current_core->gpr[rD] = r;
			rD++;
			rD%=32;
			r = 0;
		}
		if (ppc_read_effective_byte(ea, &v)) {
			return;
		}
		r<<=8;
		r|=v;
		ea++;
		i--;
		NB--;
	}
	while (i) { r<<=8; i--; }
	current_core->gpr[rD] = r;
}
/*
 *	lwarx		Load Word and Reserve Indexed
 *	.553
 */
void ppc_opc_lwarx()
{
	int rA, rD, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rD, rA, rB);
	uint32 r;
	int ret = ppc_read_effective_word((rA?current_core->gpr[rA]:0)+current_core->gpr[rB], &r);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rD] = r;
		current_core->reserve = r;
		current_core->have_reservation = 1;
	}
}
/*
 *	lwbrx		Load Word Byte-Reverse Indexed
 *	.556
 */
void ppc_opc_lwbrx()
{
	int rA, rD, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rD, rA, rB);
	uint32 r;
	int ret = ppc_read_effective_word((rA?current_core->gpr[rA]:0)+current_core->gpr[rB], &r);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rD] = ppc_bswap_word(r);
	}
}
/*
 *	lwz		Load Word and Zero
 *	.557
 */
void ppc_opc_lwz()
{
	int rA, rD;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rD, rA, imm);
	uint32 r;

	int ret = ppc_read_effective_word((rA?current_core->gpr[rA]:0)+imm, &r);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rD] = r;
	}	

}
/*
 *	lbzu		Load Word and Zero with Update
 *	.558
 */
void ppc_opc_lwzu()
{
	int rA, rD;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rD, rA, imm);
	// FIXME: check rA!=0 && rA!=rD
	uint32 r;
	int ret = ppc_read_effective_word(current_core->gpr[rA]+imm, &r);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rA] += imm;
		current_core->gpr[rD] = r;
	}	
}
/*
 *	lwzux		Load Word and Zero with Update Indexed
 *	.559
 */
void ppc_opc_lwzux()
{
	int rA, rD, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rD, rA, rB);
	// FIXME: check rA!=0 && rA!=rD
	uint32 r;
	int ret = ppc_read_effective_word(current_core->gpr[rA]+current_core->gpr[rB], &r);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rA] += current_core->gpr[rB];
		current_core->gpr[rD] = r;
	}
}
/*
 *	lwzx		Load Word and Zero Indexed
 *	.560
 */
void ppc_opc_lwzx()
{
	int rA, rD, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rD, rA, rB);
	uint32 r;
	int ret = ppc_read_effective_word((rA?current_core->gpr[rA]:0)+current_core->gpr[rB], &r);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rD] = r;
	}
}

/*      lvx	     Load Vector Indexed
 *      v.127
 */
void ppc_opc_lvx()
{
#ifndef __VEC_EXC_OFF__
	if ((current_core->msr & MSR_VEC) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_VEC, 0, 0);
		return;
	}
#endif
	VECTOR_DEBUG;
	int rA, vrD, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, rA, rB);
	Vector_t r;

	int ea = ((rA?current_core->gpr[rA]:0)+current_core->gpr[rB]);

	int ret = ppc_read_effective_qword(ea, &r);
	if (ret == PPC_MMU_OK) {
		current_core->vr[vrD] = r;
	}
}

/*      lvxl	    Load Vector Index LRU
 *      v.128
 */
void ppc_opc_lvxl()
{
	ppc_opc_lvx();
	/* This instruction should hint to the cache that the value won't be
	 *   needed again in memory anytime soon.  We don't emulate the cache,
	 *   so this is effectively exactly the same as lvx.
	 */
}

/*      lvebx	   Load Vector Element Byte Indexed
 *      v.119
 */
void ppc_opc_lvebx()
{
#ifndef __VEC_EXC_OFF__
	if ((current_core->msr & MSR_VEC) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_VEC, 0, 0);
		return;
	}
#endif
	VECTOR_DEBUG;
	int rA, vrD, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, rA, rB);
	uint32 ea;
	uint8 r;
	ea = (rA?current_core->gpr[rA]:0)+current_core->gpr[rB];
	int ret = ppc_read_effective_byte(ea, &r);
	if (ret == PPC_MMU_OK) {
		VECT_B(current_core->vr[vrD], ea & 0xf) = r;
	}
}

/*      lvehx	   Load Vector Element Half Word Indexed
 *      v.121
 */
void ppc_opc_lvehx()
{
#ifndef __VEC_EXC_OFF__
	if ((current_core->msr & MSR_VEC) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_VEC, 0, 0);
		return;
	}
#endif
	VECTOR_DEBUG;
	int rA, vrD, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, rA, rB);
	uint32 ea;
	uint16 r;
	ea = ((rA?current_core->gpr[rA]:0)+current_core->gpr[rB]) & ~1;
	int ret = ppc_read_effective_half(ea, &r);
	if (ret == PPC_MMU_OK) {
		VECT_H(current_core->vr[vrD], (ea & 0xf) >> 1) = r;
	}
}

/*      lvewx	   Load Vector Element Word Indexed
 *      v.122
 */
void ppc_opc_lvewx()
{
#ifndef __VEC_EXC_OFF__
	if ((current_core->msr & MSR_VEC) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_VEC, 0, 0);
		return;
	}
#endif
	VECTOR_DEBUG;
	int rA, vrD, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, rA, rB);
	uint32 ea;
	uint32 r;
	ea = ((rA?current_core->gpr[rA]:0)+current_core->gpr[rB]) & ~3;
	int ret = ppc_read_effective_word(ea, &r);
	if (ret == PPC_MMU_OK) {
		VECT_W(current_core->vr[vrD], (ea & 0xf) >> 2) = r;
	}
}

#if HOST_ENDIANESS == HOST_ENDIANESS_LE
static byte lvsl_helper[] = {
	0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18,
	0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10,
	0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08,
	0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00
};
#elif HOST_ENDIANESS == HOST_ENDIANESS_BE
static byte lvsl_helper[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
};
#else
#error Endianess not supported!
#endif

/*
 *      lvsl	    Load Vector for Shift Left
 *      v.123
 */
void ppc_opc_lvsl()
{
#ifndef __VEC_EXC_OFF__
	if ((current_core->msr & MSR_VEC) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_VEC, 0, 0);
		return;
	}
#endif
	VECTOR_DEBUG;
	int rA, vrD, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, rA, rB);
	uint32 ea;
	ea = ((rA?current_core->gpr[rA]:0)+current_core->gpr[rB]);
#if HOST_ENDIANESS == HOST_ENDIANESS_LE
	memmove(&current_core->vr[vrD], lvsl_helper+0x10-(ea & 0xf), 16);
#elif HOST_ENDIANESS == HOST_ENDIANESS_BE
	memmove(&current_core->vr[vrD], lvsl_helper+(ea & 0xf), 16);
#else
#error Endianess not supported!
#endif
}

/*
 *      lvsr	    Load Vector for Shift Right
 *      v.125
 */
void ppc_opc_lvsr()
{
#ifndef __VEC_EXC_OFF__
	if ((current_core->msr & MSR_VEC) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_VEC, 0, 0);
		return;
	}
#endif
	VECTOR_DEBUG;
	int rA, vrD, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, rA, rB);
	uint32 ea;
	ea = ((rA?current_core->gpr[rA]:0)+current_core->gpr[rB]);
#if HOST_ENDIANESS == HOST_ENDIANESS_LE
	memmove(&current_core->vr[vrD], lvsl_helper+(ea & 0xf), 16);
#elif HOST_ENDIANESS == HOST_ENDIANESS_BE
	memmove(&current_core->vr[vrD], lvsl_helper+0x10-(ea & 0xf), 16);
#else
#error Endianess not supported!
#endif
}

/*
 *      dst	     Data Stream Touch
 *      v.115
 */
void ppc_opc_dst()
{
	VECTOR_DEBUG;
	/* Since we are not emulating the cache, this is a nop */
}

/*
 *	stb		Store Byte
 *	.632
 */
void ppc_opc_stb()
{
	int rA, rS;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rS, rA, imm);
	ppc_write_effective_byte((rA?current_core->gpr[rA]:0)+imm, (uint8)current_core->gpr[rS]) != PPC_MMU_FATAL;
}
/*
 *	stbu		Store Byte with Update
 *	.633
 */
void ppc_opc_stbu()
{
	int rA, rS;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rS, rA, imm);
	// FIXME: check rA!=0
	int ret = ppc_write_effective_byte(current_core->gpr[rA]+imm, (uint8)current_core->gpr[rS]);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rA] += imm;
	}
}
/*
 *	stbux		Store Byte with Update Indexed
 *	.634
 */
void ppc_opc_stbux()
{
	int rA, rS, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	// FIXME: check rA!=0
	int ret = ppc_write_effective_byte(current_core->gpr[rA]+current_core->gpr[rB], (uint8)current_core->gpr[rS]);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rA] += current_core->gpr[rB];
	}
}
/*
 *	stbx		Store Byte Indexed
 *	.635
 */
void ppc_opc_stbx()
{
	int rA, rS, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	ppc_write_effective_byte((rA?current_core->gpr[rA]:0)+current_core->gpr[rB], (uint8)current_core->gpr[rS]) != PPC_MMU_FATAL;
}
/*
 *	stfd		Store Floating-Point Double
 *	.642
 */
void ppc_opc_stfd()
{
	if ((current_core->msr & MSR_FP) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_FPU, 0, 0);
		return;
	}
	int rA, frS;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, frS, rA, imm);
	ppc_write_effective_dword((rA?current_core->gpr[rA]:0)+imm, current_core->fpr[frS]) != PPC_MMU_FATAL;
}
/*
 *	stfdu		Store Floating-Point Double with Update
 *	.643
 */
void ppc_opc_stfdu()
{
	if ((current_core->msr & MSR_FP) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_FPU ,0 ,0);
		return;
	}
	int rA, frS;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, frS, rA, imm);
	// FIXME: check rA!=0
	int ret = ppc_write_effective_dword(current_core->gpr[rA]+imm, current_core->fpr[frS]);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rA] += imm;
	}
}
/*
 *	stfd		Store Floating-Point Double with Update Indexed
 *	.644
 */
void ppc_opc_stfdux()
{
	if ((current_core->msr & MSR_FP) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_FPU, 0, 0);
		return;
	}
	int rA, frS, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, frS, rA, rB);
	// FIXME: check rA!=0
	int ret = ppc_write_effective_dword(current_core->gpr[rA]+current_core->gpr[rB], current_core->fpr[frS]);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rA] += current_core->gpr[rB];
	}
}
/*
 * tlbivax	TLB invalidated virtual address indexed
 * .786
 */
void ppc_opc_tlbivax()
{
	int rA, rD, rB;
        PPC_OPC_TEMPL_X(current_core->current_opc, rD, rA, rB);
	int i = 0,j = 0;
	uint32 mask;
	ppc_tlb_entry_t *entry;
	int tlb1_index;
	uint32 addr;
	if(rA) 
		addr = current_core->gpr[rA]+current_core->gpr[rB];
	else
		addr = current_core->gpr[rB];
	//printf("In %s,addr=0x%x,pc=0x%x\n", __FUNCTION__,  addr, current_core->pc);
	/* check if IA bit is set */
	if(addr & 0x4){
		int i,j;
		/* Now we only have TLB0 and TLB1 */
		if((addr >> 3) & 0x3 > 1)
			return;
		if((addr >> 3) & 0x3){
			for(j = 0; j < L2_TLB1_SIZE; j++)
		                if(!current_core->mmu.l2_tlb1_vsp[j].iprot)
                		        current_core->mmu.l2_tlb1_vsp[j].v = 0;

		}
		else{
			for(i = 0; i < L2_TLB0_SIZE; i++)
		                if(!current_core->mmu.l2_tlb0_4k[i].iprot)
                		        current_core->mmu.l2_tlb0_4k[i].v = 0;
		}
		return;
	}
	/* walk over tlb0 and tlb1 to find the entry */
	while(i++ < (L2_TLB0_SIZE + L2_TLB1_SIZE)){
		if(i > (L2_TLB0_SIZE - 1)){
			tlb1_index = i - L2_TLB0_SIZE;
			entry = &current_core->mmu.l2_tlb1_vsp[tlb1_index];
		}
		else
			entry = &current_core->mmu.l2_tlb0_4k[i];
		/* check if entry is protected. */
		if(entry->iprot)
			continue;
		/* FIXME, not check ts bit now */
		if(entry->ts & 0x0)
			continue;
		if(entry->tid != 0){
			/* FIXME, we should check all the pid register */
			if(current_core->mmu.pid[0] != entry->tid)
				continue;
		}
		if(i > (L2_TLB0_SIZE - 1)){
			int k,s = 1;
			for(k = 0; k < entry->size; k++)
				s = s * 4; 
			mask = ~((1024 * (s - 1) - 0x1) + 1024);
		}
		else
			mask = ~(1024 * 4 - 0x1);
		if(entry->size != 0xb){
			if((addr & mask) != ((entry->epn << 12) & mask))
				continue;
		}
		else {/*if 4G size is mapped, we will not do address check */
			//fprintf(stderr,"warning:4G address is used.\n");
			if(addr < (entry->epn << 12))
				continue;

		}
		//printf("In %s,found ,offset = 0x%x,addr=0x%x,pc=0x%x, pir=0x%x\n", __FUNCTION__, i, addr, current_core->pc, current_core->pir);
		entry->v = 0;
	}
}
/*
 * tlbwe TLB write entry
 * .978
 */
/*
 * Fixme, now only support e500
 */
void ppc_opc_tlbwe()
{
	ppc_tlb_entry_t * entry;	
	int offset;
	if(TLBSEL(current_core->mmu.mas[0]) == 0x0){
		offset = ((ESEL(current_core->mmu.mas[0]) & 0xC) << 4) | (EPN(current_core->mmu.mas[2]) & 0x3f);
		/* Fixme: we just implement a simple round-robin replace for TLB0. that is not as described in manual of e500 */
		#if 0
		static int tlb0_nv = 0;
		//offset = ((tlb0_nv & 0x1) << 7) | (EPN(current_core->mmu.mas[2]) & 0x7f);
		offset = tlb0_nv++;
		if(tlb0_nv == 0xff)
			tlb0_nv = 0;
		#endif
		if(offset >= L2_TLB0_SIZE){
			fprintf(stderr, "Out of TLB size..\n");
			skyeye_exit(-1);
		}
		else{
			entry = &current_core->mmu.l2_tlb0_4k[0 + offset];
			/* update TLB0[NV] with MAS0[NV] */
			current_core->mmu.tlb0_nv = current_core->mmu.mas[0] & 0x1; 
		}
	}
	else{
		offset = ESEL(current_core->mmu.mas[0]);
		if(offset >= L2_TLB1_SIZE){
                        fprintf(stderr, "Out of TLB size..\n");
			skyeye_exit(-1);
                }
                else
			entry = &current_core->mmu.l2_tlb1_vsp[0 + offset];
	}
	entry->v = current_core->mmu.mas[1] >> 31;
	entry->iprot = (current_core->mmu.mas[1] >> 30) & 0x1;
	entry->ts = (current_core->mmu.mas[1] >> 12) & 0x1;
	entry->tid = (current_core->mmu.mas[1] >> 16) & 0xFF;
	entry->size = (current_core->mmu.mas[1] >> 8) & 0xF;
	entry->wimge = (current_core->mmu.mas[2] & 0x1F);
	entry->x = (current_core->mmu.mas[2] >> 5) & 0x3;
	entry->epn = (current_core->mmu.mas[2] >> 12) & 0xFFFFF;
	entry->usxrw = current_core->mmu.mas[3] & 0x3F;
	entry->u = (current_core->mmu.mas[3]) >> 6 & 0xF;
	entry->rpn = (current_core->mmu.mas[3] >> 12) & 0xFFFFF;
	current_core->mmu.tlb0_nv = current_core->mmu.mas[0] & 0x3;
	/* workaround for second core */
	//if(entry->epn == 0xfdff9)
	//	entry->v = 1;
	//printf("In %s, entry=0x%x, entry->v=0x%x,rpn=0x%x, epn=0x%x, offset=0x%x, tid=0x%x, mas2=0x%x, pc=0x%x, pir=0x%x\n", __FUNCTION__, entry, entry->v, entry->rpn, entry->epn, offset, entry->tid, current_core->mmu.mas[2], current_core->pc, current_core->pir);
	//printf("In %s, current_core->mmu.tlb0_nv=0x%x\n", __FUNCTION__, current_core->mmu.tlb0_nv);
	//printf("In %s, rpn=0x%x, epn=0x%x, offset=0x%x,usxrw=0x%x,tid=0x%x,pc=0x%x\n", __FUNCTION__, entry->rpn, entry->epn, offset, entry->usxrw, entry->tid ,current_core->pc);
}

void ppc_opc_tlbsx(){
	ppc_tlb_entry_t *entry;
        int tlb1_index;
	int va,ea;
	int mask;
        int i = 0;
	int rA, rD, rB;
        PPC_OPC_TEMPL_X(current_core->current_opc, rD, rA, rB);
	ea = current_core->gpr[rB];
        /* walk over tlb0 and tlb1 to find the entry */

	//printf("In %s, ea=0x%x\n", __FUNCTION__, ea);
	while(i++ < (L2_TLB0_SIZE + L2_TLB1_SIZE)){
        	if(i > (L2_TLB0_SIZE - 1)){
                	tlb1_index = i - L2_TLB0_SIZE;
                	entry = &current_core->mmu.l2_tlb1_vsp[tlb1_index];
                }
                else
                        entry = &current_core->mmu.l2_tlb0_4k[i];
                if(!entry->v)
                        continue;
                /* FIXME, not check ts bit now */
                if(entry->ts & 0x0)
                        continue;
		if(entry->tid != 0 && entry->tid != ((current_core->mmu.mas[6] & 0xFF0000) >> 16)){
                                continue;
                }
		//printf("In %s,entry->tid=0x%x,mas[6]=0x%x\n", __FUNCTION__, entry->tid, current_core->mmu.mas[6]);
		if(i > (L2_TLB0_SIZE - 1)){
                        int k,s = 1;
                        for(k = 0; k < entry->size; k++)
                                s = s * 4;
                        mask = ~(1024 * s - 0x1);
                }
                else
                        mask = ~(1024 * 4 - 0x1);
		/* we found the entry */
		if((ea & mask) == (entry->epn << 12)){
			//printf("In %s, found entry,i=0x%x entry->usxrw=0x%x,entry->rpn=0x%x, pc=0x%x\n", __FUNCTION__, i, entry->usxrw, entry->rpn, current_core->pc);
			if(i > (L2_TLB0_SIZE - 1)){
				current_core->mmu.mas[0] |= (0x1 << 28);
				current_core->mmu.mas[0] |= (current_core->mmu.mas[0] & 0xFFF0FFFF) | ((tlb1_index & 0xC) << 16) ;
				current_core->mmu.mas[2] = (current_core->mmu.mas[2] & 0xFFFFF000) | (entry->epn << 12);
                                /* set v bit to one */
                                //current_core->mmu.mas[1] &= 0x80000000;

                                current_core->mmu.mas[3] = (current_core->mmu.mas[3] & 0xFFFFFFC0) | entry->usxrw;
                                current_core->mmu.mas[3] = (current_core->mmu.mas[3] & 0xFFF) | (entry->rpn << 12);

			}
        	        else{
				current_core->mmu.mas[0] &= ~(0x1 << 28);
				current_core->mmu.mas[0] = (current_core->mmu.mas[0] & 0xFFFFFFFC) | (current_core->mmu.tlb0_nv & 0x3);
				/* fill ESEL */
				current_core->mmu.mas[0] = (current_core->mmu.mas[0] & 0xFFF0FFFF) | (((i & 0xC0) >> 4)  << 16);
				current_core->mmu.mas[2] = (current_core->mmu.mas[2] & 0xFFFFF000) | (entry->epn << 12);
				/* set v bit to one */
				//current_core->mmu.mas[1] &= 0x80000000;
				
				current_core->mmu.mas[3] = (current_core->mmu.mas[3] & 0xFFFFFFC0) | entry->usxrw;
				current_core->mmu.mas[3] = (current_core->mmu.mas[3] & 0xFFF) | (entry->rpn << 12);
			}
			current_core->mmu.mas[1] = (current_core->mmu.mas[1] & 0xFF0000) | (entry->tid << 16);
			current_core->mmu.mas[1] |= 0x80000000;
			//printf("In %s,mas3=0x%x\n", __FUNCTION__, current_core->mmu.mas[3]);
			break;
		}
	}

	/* set valid bit in mas[1] to zero */
	//current_core->mmu.mas[1] &= ~0x80000000;
	//printf("In %s, missing\n", __FUNCTION__);
}

void ppc_opc_tlbrehi(){
	ppc_tlb_entry_t * entry;
        int offset;
        if(TLBSEL(current_core->mmu.mas[0]) == 0x0){
                offset = ((ESEL(current_core->mmu.mas[0]) & 0xC) << 4) | (EPN(current_core->mmu.mas[2]) & 0x3f);
                if(offset > L2_TLB0_SIZE){
                        fprintf(stderr, "Out of TLB size..\n");
                        skyeye_exit(-1);
                }
                else
                        entry = &current_core->mmu.l2_tlb0_4k[0 + offset];
		current_core->mmu.mas[0] = (current_core->mmu.mas[0] & 0xFFFFFFFC) | (current_core->mmu.tlb0_nv & 0x3);
        }
        else{
                offset = ESEL(current_core->mmu.mas[0]);
                if(offset > L2_TLB1_SIZE){
                        fprintf(stderr, "Out of TLB size..\n");
                        skyeye_exit(-1);
                }
                else
                        entry = &current_core->mmu.l2_tlb1_vsp[0 + offset];
        }
	current_core->mmu.mas[1] = (current_core->mmu.mas[1] & 0x7FFFFFFF) | (entry->v << 31);
	current_core->mmu.mas[1] = (current_core->mmu.mas[1] & 0xbFFFFFFF) | (entry->iprot << 30);
	current_core->mmu.mas[1] = (current_core->mmu.mas[1] & 0xFFFFEFFF) | (entry->ts << 12);
	current_core->mmu.mas[1] = (current_core->mmu.mas[1] & 0xFF00FFFF) | (entry->tid << 16);
	current_core->mmu.mas[1] = (current_core->mmu.mas[1] & 0xFFFFF0FF) | (entry->size << 8);
	current_core->mmu.mas[2] = (current_core->mmu.mas[2] & 0xFFFFFFE0) | (entry->wimge);
	current_core->mmu.mas[2] = (current_core->mmu.mas[2] & 0xFFFFFF9F) | (entry->x << 5);

	current_core->mmu.mas[2] = (current_core->mmu.mas[2] & 0xFFF) | (entry->epn << 12);

	current_core->mmu.mas[3] = (current_core->mmu.mas[3] & 0xFFFFFFC0) | (entry->usxrw);

	current_core->mmu.mas[3] = (current_core->mmu.mas[3] & 0xFFFFFFC3F) | (entry->u << 6);
	current_core->mmu.mas[3] = (current_core->mmu.mas[3] & 0xFFF) | (entry->rpn << 12);

}
/*
 *	stfdx		Store Floating-Point Double Indexed
 *	.645
 */
void ppc_opc_stfdx()
{
	if ((current_core->msr & MSR_FP) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_FPU, 0, 0);
		return;
	}
	int rA, frS, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, frS, rA, rB);
	ppc_write_effective_dword((rA?current_core->gpr[rA]:0)+current_core->gpr[rB], current_core->fpr[frS]) != PPC_MMU_FATAL;
}
/*
 *	stfiwx		Store Floating-Point as Integer Word Indexed
 *	.646
 */
void ppc_opc_stfiwx()
{
	if ((current_core->msr & MSR_FP) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_FPU, 0, 0);
		return;
	}
	int rA, frS, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, frS, rA, rB);
	ppc_write_effective_word((rA?current_core->gpr[rA]:0)+current_core->gpr[rB], (uint32)current_core->fpr[frS]) != PPC_MMU_FATAL;
}
/*
 *	stfs		Store Floating-Point Single
 *	.647
 */
void ppc_opc_stfs()
{
	if ((current_core->msr & MSR_FP) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_FPU, 0, 0);
		return;
	}
	int rA, frS;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, frS, rA, imm);
	uint32 s;
	ppc_double d;
	ppc_fpu_unpack_double(&d, current_core->fpr[frS]);
	ppc_fpu_pack_single(&d, &s);
	ppc_write_effective_word((rA?current_core->gpr[rA]:0)+imm, s) != PPC_MMU_FATAL;
}
/*
 *	stfsu		Store Floating-Point Single with Update
 *	.648
 */
void ppc_opc_stfsu()
{
	if ((current_core->msr & MSR_FP) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_FPU, 0, 0);
		return;
	}
	int rA, frS;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, frS, rA, imm);
	// FIXME: check rA!=0
	uint32 s;
	ppc_double d;
	ppc_fpu_unpack_double(&d, current_core->fpr[frS]);
	ppc_fpu_pack_single(&d, &s);
	int ret = ppc_write_effective_word(current_core->gpr[rA]+imm, s);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rA] += imm;
	}
}
/*
 *	stfsux		Store Floating-Point Single with Update Indexed
 *	.649
 */
void ppc_opc_stfsux()
{
	if ((current_core->msr & MSR_FP) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_FPU, 0, 0);
		return;
	}
	int rA, frS, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, frS, rA, rB);
	// FIXME: check rA!=0
	uint32 s;
	ppc_double d;
	ppc_fpu_unpack_double(&d, current_core->fpr[frS]);
	ppc_fpu_pack_single(&d, &s);
	int ret = ppc_write_effective_word(current_core->gpr[rA]+current_core->gpr[rB], s);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rA] += current_core->gpr[rB];
	}
}
/*
 *	stfsx		Store Floating-Point Single Indexed
 *	.650
 */
void ppc_opc_stfsx()
{
	if ((current_core->msr & MSR_FP) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_FPU, 0, 0);
		return;
	}
	int rA, frS, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, frS, rA, rB);
	uint32 s;
	ppc_double d;
	ppc_fpu_unpack_double(&d, current_core->fpr[frS]);
	ppc_fpu_pack_single(&d, &s);
	ppc_write_effective_word((rA?current_core->gpr[rA]:0)+current_core->gpr[rB], s) != PPC_MMU_FATAL;
}
/*
 *	sth		Store Half Word
 *	.651
 */
void ppc_opc_sth()
{
	int rA, rS;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rS, rA, imm);
	ppc_write_effective_half((rA?current_core->gpr[rA]:0)+imm, (uint16)current_core->gpr[rS]) != PPC_MMU_FATAL;
	/*if(current_core->pc >= 0xfff830e4 && current_core->pc <= 0xfff83254)
		fprintf(prof_file, "DBG:in %s,pc=0x%x\n", __FUNCTION__, current_core->pc);
	*/
}
/*
 *	sthbrx		Store Half Word Byte-Reverse Indexed
 *	.652
 */
void ppc_opc_sthbrx()
{
	int rA, rS, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	ppc_write_effective_half((rA?current_core->gpr[rA]:0)+current_core->gpr[rB], ppc_bswap_half(current_core->gpr[rS])) != PPC_MMU_FATAL;
}
/*
 *	sthu		Store Half Word with Update
 *	.653
 */
void ppc_opc_sthu()
{
	int rA, rS;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rS, rA, imm);
	// FIXME: check rA!=0
	int ret = ppc_write_effective_half(current_core->gpr[rA]+imm, (uint16)current_core->gpr[rS]);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rA] += imm;
	}
}
/*
 *	sthux		Store Half Word with Update Indexed
 *	.654
 */
void ppc_opc_sthux()
{
	int rA, rS, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	// FIXME: check rA!=0
	int ret = ppc_write_effective_half(current_core->gpr[rA]+current_core->gpr[rB], (uint16)current_core->gpr[rS]);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rA] += current_core->gpr[rB];
	}
}
/*
 *	sthx		Store Half Word Indexed
 *	.655
 */
void ppc_opc_sthx()
{
	int rA, rS, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	ppc_write_effective_half((rA?current_core->gpr[rA]:0)+current_core->gpr[rB], (uint16)current_core->gpr[rS]) != PPC_MMU_FATAL;
}
/*
 *	stmw		Store Multiple Word
 *	.656
 */
void ppc_opc_stmw()
{
	int rS, rA;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rS, rA, imm);
	uint32 ea = (rA ? current_core->gpr[rA] : 0) + imm;
	while (rS <= 31) {
		if (ppc_write_effective_word(ea, current_core->gpr[rS])) {
			return;
		}
		rS++;
		ea += 4;
	}
}
/*
 *	stswi		Store String Word Immediate
 *	.657
 */
void ppc_opc_stswi()
{
	int rA, rS, NB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, NB);
	if (NB==0) NB=32;
	uint32 ea = rA ? current_core->gpr[rA] : 0;
	uint32 r = 0;
	int i = 0;
	
	while (NB > 0) {
		if (!i) {
			r = current_core->gpr[rS];
			rS++;
			rS%=32;
			i = 4;
		}
		if (ppc_write_effective_byte(ea, (r>>24))) {
			return;
		}
		r<<=8;
		ea++;
		i--;
		NB--;
	}
}
/*
 *	stswx		Store String Word Indexed
 *	.658
 */
void ppc_opc_stswx()
{
	int rA, rS, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	int NB = XER_n(current_core->xer);
	uint32 ea = current_core->gpr[rB] + (rA ? current_core->gpr[rA] : 0);
	uint32 r = 0;
	int i = 0;
	
	while (NB > 0) {
		if (!i) {
			r = current_core->gpr[rS];
			rS++;
			rS%=32;
			i = 4;
		}
		if (ppc_write_effective_byte(ea, (r>>24))) {
			return;
		}
		r<<=8;
		ea++;
		i--;
		NB--;
	}
}
/*
 *	stw		Store Word
 *	.659
 */
void ppc_opc_stw()
{
	int rA, rS;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rS, rA, imm);
	ppc_write_effective_word((rA?current_core->gpr[rA]:0)+imm, current_core->gpr[rS]) != PPC_MMU_FATAL;
}
/*
 *	stwbrx		Store Word Byte-Reverse Indexed
 *	.660
 */
void ppc_opc_stwbrx()
{
	int rA, rS, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	// FIXME: doppelt gemoppelt
	ppc_write_effective_word((rA?current_core->gpr[rA]:0)+current_core->gpr[rB], ppc_bswap_word(current_core->gpr[rS])) != PPC_MMU_FATAL;
}
/*
 *	stwcx.		Store Word Conditional Indexed
 *	.661
 */
void ppc_opc_stwcx_()
{
	int rA, rS, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	current_core->cr &= 0x0fffffff;
	if (current_core->have_reservation) {
		current_core->have_reservation = false;
		uint32 v;
		if (ppc_read_effective_word((rA?current_core->gpr[rA]:0)+current_core->gpr[rB], &v)) {
			return;
		}
		if (v==current_core->reserve) {
			if (ppc_write_effective_word((rA?current_core->gpr[rA]:0)+current_core->gpr[rB], current_core->gpr[rS])) {
				return;
			}
			current_core->cr |= CR_CR0_EQ;
		}
		if (current_core->xer & XER_SO) {
			current_core->cr |= CR_CR0_SO;
		}
	}
}
/*
 *	stwu		Store Word with Update
 *	.663
 */
void ppc_opc_stwu()
{
	int rA, rS;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rS, rA, imm);
	// FIXME: check rA!=0
	int ret = ppc_write_effective_word(current_core->gpr[rA]+imm, current_core->gpr[rS]);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rA] += imm;
	}
}
/*
 *	stwux		Store Word with Update Indexed
 *	.664
 */
void ppc_opc_stwux()
{
	int rA, rS, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	// FIXME: check rA!=0
	int ret = ppc_write_effective_word(current_core->gpr[rA]+current_core->gpr[rB], current_core->gpr[rS]);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rA] += current_core->gpr[rB];
	}
}
/*
 *	stwx		Store Word Indexed
 *	.665
 */
void ppc_opc_stwx()
{
	int rA, rS, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	ppc_write_effective_word((rA?current_core->gpr[rA]:0)+current_core->gpr[rB], current_core->gpr[rS]) != PPC_MMU_FATAL;
}

/*      stvx	    Store Vector Indexed
 *      v.134
 */
void ppc_opc_stvx()
{
#ifndef __VEC_EXC_OFF__
	if ((current_core->msr & MSR_VEC) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_VEC, 0, 0);
		return;
	}
#endif
	VECTOR_DEBUG;
	int rA, vrS, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrS, rA, rB);

	int ea = ((rA?current_core->gpr[rA]:0)+current_core->gpr[rB]);

	ppc_write_effective_qword(ea, current_core->vr[vrS]) != PPC_MMU_FATAL;
}

/*      stvxl	   Store Vector Indexed LRU
 *      v.135
 */
void ppc_opc_stvxl()
{
	ppc_opc_stvx();
	/* This instruction should hint to the cache that the value won't be
	 *   needed again in memory anytime soon.  We don't emulate the cache,
	 *   so this is effectively exactly the same as lvx.
	 */
}

/*      stvebx	  Store Vector Element Byte Indexed
 *      v.131
 */
void ppc_opc_stvebx()
{
#ifndef __VEC_EXC_OFF__
	if ((current_core->msr & MSR_VEC) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_VEC, 0, 0);
		return;
	}
#endif
	VECTOR_DEBUG;
	int rA, vrS, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrS, rA, rB);
	uint32 ea;
	ea = (rA?current_core->gpr[rA]:0)+current_core->gpr[rB];
	ppc_write_effective_byte(ea, VECT_B(current_core->vr[vrS], ea & 0xf));
}

/*      stvehx	  Store Vector Element Half Word Indexed
 *      v.132
 */
void ppc_opc_stvehx()
{
#ifndef __VEC_EXC_OFF__
	if ((current_core->msr & MSR_VEC) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_VEC, 0, 0);
		return;
	}
#endif
	VECTOR_DEBUG;
	int rA, vrS, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrS, rA, rB);
	uint32 ea;
	ea = ((rA?current_core->gpr[rA]:0)+current_core->gpr[rB]) & ~1;
	ppc_write_effective_half(ea, VECT_H(current_core->vr[vrS], (ea & 0xf) >> 1));
}

/*      stvewx	  Store Vector Element Word Indexed
 *      v.133
 */
void ppc_opc_stvewx()
{
#ifndef __VEC_EXC_OFF__
	if ((current_core->msr & MSR_VEC) == 0) {
		ppc_exception(current_core, PPC_EXC_NO_VEC, 0, 0);
		return;
	}
#endif
	VECTOR_DEBUG;
	int rA, vrS, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrS, rA, rB);
	uint32 ea;
	ea = ((rA?current_core->gpr[rA]:0)+current_core->gpr[rB]) & ~3;
	ppc_write_effective_word(ea, VECT_W(current_core->vr[vrS], (ea & 0xf) >> 2));
}

/*      dstst	   Data Stream Touch for Store
 *      v.117
 */
void ppc_opc_dstst()
{
	VECTOR_DEBUG;
	/* Since we are not emulating the cache, this is a nop */
}

/*      dss	     Data Stream Stop
 *      v.114
 */
void ppc_opc_dss()
{
	VECTOR_DEBUG;
	/* Since we are not emulating the cache, this is a nop */
}
void ppc_opc_wrteei(){
	if ((current_core->current_opc >> 15) & 0x1)
		current_core->msr |= 0x00008000;
	else
		current_core->msr &= ~0x00008000;
}
void ppc_opc_wrtee(){
	int rA, rS, rB;
        PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
        if ((current_core->gpr[rS] >> 15) & 0x1)
                current_core->msr |= 0x00008000;
        else
                current_core->msr &= ~0x00008000;

}
