/* Copyright (C) 
* 2011 - Michael.Kang blackfin.kang@gmail.com
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
* @file basicblock.h
* @brief 
* @author Michael.Kang blackfin.kang@gmail.com
* @version 
* @date 2011-05-05
*/
#ifndef __DYNCOM_BASICBLOCK_H__
#define __DYNCOM_BASICBLOCK_H__
enum {
	BB_TYPE_NORMAL   = 'L', /* basic block for instructions */
	BB_TYPE_COND     = 'C', /* basic block for "taken" case of cond. execution */
	BB_TYPE_DELAY    = 'D', /* basic block for delay slot in non-taken case of cond. exec. */
	BB_TYPE_EXTERNAL = 'E'  /* basic block for unknown addresses; just traps */
};

bool is_start_of_basicblock(cpu_t *cpu, addr_t a);
bool needs_dispatch_entry(cpu_t *cpu, addr_t a);
BasicBlock *create_basicblock(cpu_t *cpu, addr_t addr, Function *f, uint8_t bb_type);
const BasicBlock *lookup_basicblock(cpu_t *cpu, Function* f, addr_t pc, BasicBlock *bb_ret, uint8_t bb_type);
void emit_store_pc(cpu_t *cpu, BasicBlock *bb_branch, addr_t new_pc);
void emit_store_pc_return(cpu_t *cpu, BasicBlock *bb_branch, addr_t new_pc, BasicBlock *bb_ret);
void emit_store_pc_end_page(cpu_t *cpu, tag_t tag, BasicBlock *bb, addr_t new_pc);
void emit_store_pc_cond(cpu_t *cpu, tag_t tag, Value *cond, BasicBlock *bb, addr_t new_pc);
void arm_emit_store_pc(cpu_t *cpu, BasicBlock *bb_branch, addr_t new_pc);
#endif
