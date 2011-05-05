/* Copyright (C)
* 2011 - SkyeyeTeam
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

/**
* @file cache.c
* @brief decode cache instruction
* @author skyeye team
* @version
* @date 2011-05-05
*/

#include "instr.h"
#include "emul.h"
/**
* @brief decode the instructions about cache
*
* @param mstate mips state
* @param instr  instr code
*
* @return Pipeline info
*/
int 
decode_cache(MIPS_State* mstate, Instr instr) //Use in Cache Instruction, it's unuseable in R3000
{
    	// CP0 is usable in kernel mode or when the CU bit in SR is set.
    	if (!(mstate->mode & kmode) && !bit(mstate->cp0[SR], SR_CU0))
		process_coprocessor_unusable(mstate, 0);
	
    	VA va = sign_extend_UInt32(offset(instr), 16) + mstate->gpr[base(instr)];
    	PA pa;
	if(translate_vaddr(mstate, va, cache_op, &pa) != 0){
		return nothing_special;
	}
    	if (pa != bad_pa) {
		if (bit(instr, 16)) {
	    		// Control data cache.
	    		control_dcache(mstate, va, pa, bits(instr, 20, 18), bit(instr, 17));
		} else {
	    		// Control instruction cache.
	    		control_icache(mstate, va, pa, bits(instr, 20, 18), bit(instr, 17));
		}
    	}
    	return nothing_special;
}
