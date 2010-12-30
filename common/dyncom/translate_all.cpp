/**
 * @file translate_all.cpp
 * 
 * This translates all known code by creating basic blocks and
 * filling them with instructions.
 *
 * @author OS Center,TsingHua University (Ported from libcpu)
 * @date 11/11/2010
 */

#include "llvm/BasicBlock.h"
#include "llvm/Instructions.h"

#include "dyncom/frontend.h"
#include "skyeye_dyncom.h"
#include "dyncom/dyncom_llvm.h"
#include "dyncom/basicblock.h"
#include "disasm.h"
#include "dyncom/tag.h"
#include "translate.h"
//#include "libcpu_run.h"

/**
 * @brief translate all the instructions. 
 *
 * @param cpu CPU core structure
 * @param bb_ret return basic block
 * @param bb_trap trap basic block
 *
 * @return dispatch basic block 
 */
BasicBlock *
cpu_translate_all(cpu_t *cpu, BasicBlock *bb_ret, BasicBlock *bb_trap)
{
	// find all instructions that need labels and create basic blocks for them
	int bbs = 0;
	addr_t pc;
	pc = cpu->dyncom_engine->tag_start;
	while (pc <= cpu->dyncom_engine->tag_end) { 
		// Do not create the basic block if it is already present in some other function.
		//if (is_start_of_basicblock(cpu, pc) && !(get_tag(cpu, pc) & TAG_TRANSLATED)) {
		if (is_start_of_basicblock(cpu, pc)) {
			//if ((get_tag(cpu, pc) & TAG_TRANSLATED)) {
				//printf("TRANSLATED BB %x\n", pc);
			//}
				create_basicblock(cpu, pc, cpu->dyncom_engine->cur_func, BB_TYPE_NORMAL);
				bbs++;
		//	} else {
		//		LOG("TRANSLATED BB %x\n", pc);
		//	}
		}
		pc++;
	}
	LOG("bbs: %d\n", bbs);

	// create dispatch basicblock
	BasicBlock* bb_dispatch = BasicBlock::Create(_CTX(), "dispatch", cpu->dyncom_engine->cur_func, 0);
	Value *v_pc = new LoadInst(cpu->ptr_PHYS_PC, "", false, bb_dispatch);
	SwitchInst* sw = SwitchInst::Create(v_pc, bb_ret, bbs, bb_dispatch);

	// translate basic blocks
	bbaddr_map &bb_addr = cpu->dyncom_engine->func_bb[cpu->dyncom_engine->cur_func];
	bbaddr_map::const_iterator it;
	for (it = bb_addr.begin(); it != bb_addr.end(); it++) {
		pc = it->first;
		BasicBlock *cur_bb = it->second;

		tag_t tag;
		BasicBlock *bb_target = NULL, *bb_next = NULL, *bb_cont = NULL, *bb_cond = NULL;

		// Tag the function as translated.
		or_tag(cpu, pc, TAG_TRANSLATED);

		LOG("basicblock: L%08llx\n", (unsigned long long)pc);

		// Add dispatch switch case for basic block.
		ConstantInt* c = ConstantInt::get(getIntegerType(cpu->info.address_size), pc);
		sw->addCase(c, cur_bb);

		do {
			tag_t dummy1;

			tag = get_tag(cpu, pc);

			/* get address of the following instruction */
			addr_t new_pc, next_pc;
			cpu->f.tag_instr(cpu, pc, &dummy1, &new_pc, &next_pc);

			/* get target basic block */
			if (tag & TAG_RET)
				bb_target = bb_dispatch;
			if (tag & (TAG_CALL|TAG_BRANCH|TAG_POSTCOND)) {
				if (new_pc == NEW_PC_NONE) /* translate_instr() will set PC */
					bb_target = bb_dispatch;
				else
					bb_target = (BasicBlock*)lookup_basicblock(cpu, cpu->dyncom_engine->cur_func, new_pc, bb_ret, BB_TYPE_NORMAL);
			}
			#if 0
			if (new_pc > cpu->code_end) {
				return bb_dispatch;
			}
			#endif
			/* get not-taken basic block */
			if (tag & (TAG_CONDITIONAL | TAG_ZEROVERHEADLOOP | TAG_POSTCOND | TAG_WINDOWCHECK | TAG_LAST_INST))
 				bb_next = (BasicBlock*)lookup_basicblock(cpu, cpu->dyncom_engine->cur_func, next_pc, bb_ret, BB_TYPE_NORMAL);
			//update pc
			emit_store_pc(cpu, cur_bb, pc);
		       arch_inc_icounter(cpu, cur_bb);
#if 1// Only for debug all the execution instructions
			arch_debug_me(cpu, cur_bb);
#endif

			bb_cont = translate_instr(cpu, pc, tag, bb_target, bb_trap, bb_next, bb_ret, cur_bb);
			// if instr is the last one in loop body, return value "bb_cont" is zero-overhead-loop conditional basicblock.
			if (tag & TAG_ZEROVERHEADLOOP) {
				//cpu->f.translate_loop_helper(cpu, pc, cur_bb, bb_ret, bb_next, bb_cont);
				//bb_cont = NULL;
			}
			pc = next_pc;
			if ((tag & TAG_WINDOWCHECK) && bb_cont) {//change instruction bb to the new one
				cur_bb = bb_cont;
			}
			
		} while (
					/* new basic block starts here (and we haven't translated it yet)*/
					(!is_start_of_basicblock(cpu, pc)) &&
					/* end of code section */ //XXX no: this is whether it's TAG_CODE
					is_code(cpu, pc) &&
					/* last intruction jumped away */
					bb_cont
				);

		/* link with next basic block if there isn't a control flow instr. already */
		if (bb_cont) {
			BasicBlock *target = (BasicBlock*)lookup_basicblock(cpu, cpu->dyncom_engine->cur_func, pc, bb_ret, BB_TYPE_NORMAL);
			LOG("info: linking continue $%04llx!\n", (unsigned long long)pc);
			BranchInst::Create(target, bb_cont);
		}
    }

	return bb_dispatch;
}
