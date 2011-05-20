/**
 * @file translate.cpp
 * 
 * This translates a single instruction by calling out into
 * the architecture dependent functions. It will optionally
 * create internal basic blocks if necessary.
 *
 * @author OS Center,TsingHua University (Ported from libcpu)
 * @date 11/11/2010
 */

#include "llvm/Instructions.h"
#include "llvm/BasicBlock.h"

#include "skyeye_dyncom.h"
#include "dyncom/tag.h"
#include "dyncom/basicblock.h"
#include "dyncom/frontend.h"
#include "dyncom/dyncom_llvm.h"
#include "dyncom/defines.h"
/**
 * @brief translate a single instruction. 
 *
 * @param cpu CPU core structure
 * @param pc current address to translate
 * @param tag tag of the address
 * @param bb_target target basic block for branch/call instruction
 * @param bb_trap target basic block for trap
 * @param bb_next non-taken for conditional,the next instruction's basic block
 * @param bb_ret return basic block
 * @param cur_bb current basic block
 *
 * @return returns the basic block where code execution continues, or
 *	NULL if the instruction always branches away
 *	(The caller needs this to link the basic block)
 */
BasicBlock *
translate_instr(cpu_t *cpu, addr_t pc, addr_t next_pc, tag_t tag,
	BasicBlock *bb_target,	/* target for branch/call/rey */
	BasicBlock *bb_trap,	/* target for trap */
	BasicBlock *bb_next,	/* non-taken for conditional */
	BasicBlock *bb_ret, BasicBlock *cur_bb)
{
	BasicBlock *bb_cond = NULL;
	BasicBlock *bb_delay = NULL;
	BasicBlock *bb_zol = NULL;
	BasicBlock *bb_zol_cond = NULL;
	BasicBlock *bb_instr = NULL;

	/* create internal basic blocks if needed */
	if (tag & TAG_CONDITIONAL)
		bb_cond = create_basicblock(cpu, pc, cpu->dyncom_engine->cur_func, BB_TYPE_COND);
	if ((tag & TAG_DELAY_SLOT) && (tag & TAG_CONDITIONAL))
		bb_delay = create_basicblock(cpu, pc, cpu->dyncom_engine->cur_func, BB_TYPE_DELAY);
	/* special case: delay slot */
	if (tag & TAG_DELAY_SLOT) {
		if (tag & TAG_CONDITIONAL) {
			addr_t delay_pc;
			// cur_bb:  if (cond) goto b_cond; else goto bb_delay;
			Value *c = cpu->f.translate_cond(cpu, pc, cur_bb);
			if(tag & TAG_END_PAGE){
				emit_store_pc_cond(cpu, c, cur_bb, next_pc);
				BranchInst::Create(bb_cond, bb_ret, c, cur_bb);
			}
			else
				BranchInst::Create(bb_cond, bb_delay, c, cur_bb);
			// bb_cond: instr; delay; goto bb_target;
			pc += cpu->f.translate_instr(cpu, pc, bb_cond);
			delay_pc = pc;
			cpu->f.translate_instr(cpu, pc, bb_cond);
			BranchInst::Create(bb_target, bb_cond);
			// bb_cond: delay; goto bb_next;
			cpu->f.translate_instr(cpu, delay_pc, bb_delay);
			BranchInst::Create(bb_next, bb_delay);
		} else {
			// cur_bb:  instr; delay; goto bb_target;
			pc += cpu->f.translate_instr(cpu, pc, cur_bb);
			cpu->f.translate_instr(cpu, pc, cur_bb);
			BranchInst::Create(bb_target, cur_bb);
		}
		return NULL; /* don't link */
	}

	/* no delay slot */
	if (tag & TAG_CONDITIONAL) {
		// cur_bb:  if (cond) goto b_cond; else goto bb_next;
		Value *c = cpu->f.translate_cond(cpu, pc, cur_bb);
		if(tag & TAG_END_PAGE){
                        emit_store_pc_cond(cpu, c, cur_bb, next_pc);
                        BranchInst::Create(bb_cond, bb_ret, c, cur_bb);
		}
		else
			BranchInst::Create(bb_cond, bb_next, c, cur_bb);
		cur_bb = bb_cond;
	}

	cpu->f.translate_instr(cpu, pc, cur_bb);
	if (tag & TAG_POSTCOND) {
		Value *c = cpu->f.translate_cond(cpu, pc, cur_bb);
		BranchInst::Create(bb_target, bb_next, c, cur_bb);
	}

	if ((tag & (TAG_END_PAGE | TAG_EXCEPTION)) && !is_user_mode(cpu))
		BranchInst::Create(bb_ret, cur_bb);
	else if (tag & (TAG_BRANCH | TAG_CALL | TAG_RET))
		BranchInst::Create(bb_target, cur_bb);
	else if (tag & TAG_TRAP)
		BranchInst::Create(bb_trap, cur_bb);
	else if (tag & TAG_CONDITIONAL) {/* Add terminator instruction 'br' for conditional instruction */
		BranchInst::Create(bb_next, cur_bb);
	}

#ifdef OPT_LOCAL_REGISTERS
	if (tag & TAG_BEFORE_SYSCALL) {//bb_instr needs a terminator inst.
		emit_store_pc_return(cpu, cur_bb, pc + 4, bb_trap);
		cur_bb = NULL;
	}
	if (tag & TAG_SYSCALL) {//bb_instr needs a terminator inst.
		emit_store_pc_return(cpu, cur_bb, pc + 4, bb_ret);
		cur_bb = NULL;
	}
#endif

	if (tag & TAG_CONTINUE)
		return cur_bb;
	else
		return NULL;
}
