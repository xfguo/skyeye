/**
 * @file basicblock.cpp
 * 
 * Basic block handling (create, lookup)
 * 
 * @author OS Center,TsingHua University (Ported from libcpu)
 * @date 11/11/2010
 */

#include "llvm/Constants.h"
#include "llvm/Instructions.h"
#include "llvm/BasicBlock.h"

#include <skyeye_dyncom.h>
#include <dyncom/dyncom_llvm.h>
#include <dyncom/basicblock.h>
#include <dyncom/tag.h>

/**
 * @brief Determine an address is the start of a basicblock or not.
 *
 * @param cpu The CPU core structure
 * @param a The address to be determined
 *
 * @return true if start of a basicblock,false otherwise
 */
bool
is_start_of_basicblock(cpu_t *cpu, addr_t a)
{
	tag_t tag = get_tag(cpu, a);
	return (tag &
		(TAG_BRANCH_TARGET |	/* someone jumps/branches here */
		 TAG_SUBROUTINE |		/* someone calls this */
		 TAG_AFTER_CALL |		/* instruction after a call */
		 TAG_AFTER_COND |		/* instruction after a branch */
		 TAG_AFTER_TRAP |		/* instruction after a trap */
		 TAG_SYSCALL	|
		 TAG_AFTER_SYSCALL	|
		 TAG_ENTRY))			/* client wants to enter guest code here */
		&& (tag & TAG_CODE);	/* only if we actually tagged it */
}
/**
 * @brief Store PC to cpu structure 
 *
 * @param cpu The CPU core structure
 * @param bb_branch The basicblock to store the llvm instruction
 * @param new_pc The PC to be stored
 */
void
emit_store_pc(cpu_t *cpu, BasicBlock *bb_branch, addr_t new_pc)
{
	Value *v_pc = ConstantInt::get(getIntegerType(cpu->info.address_size), new_pc);
	new StoreInst(v_pc, cpu->ptr_PHYS_PC, bb_branch);
}
/**
 * @brief Store PC to cpu structure,then jump to the ret basicblock
 *
 * @param cpu The CPU core structure
 * @param bb_branch The basicblock to store the llvm instruction
 * @param new_pc The PC to be stored
 * @param bb_ret The ret basicblock of the current JIT Function
 */
void
emit_store_pc_return(cpu_t *cpu, BasicBlock *bb_branch, addr_t new_pc, BasicBlock *bb_ret)
{
	emit_store_pc(cpu, bb_branch, new_pc);
	BranchInst::Create(bb_ret, bb_branch);
}
/**
 * @brief Create a basicblock and put it to the current JIT Function.Add it to func_bb map.
 *
 * @param cpu The CPU core structure
 * @param addr The address,to be used as the part of the name of the basicblock
 * @param f The JIT Function to store the basicblock
 * @param bb_type The type of the basicblock to be created.And used as part of the basicblock name.
 *
 * @return The pointer of the basicblock created 
 */
BasicBlock *
create_basicblock(cpu_t *cpu, addr_t addr, Function *f, uint8_t bb_type) {
	char label[17];
	snprintf(label, sizeof(label), "%c%08llx", bb_type, (unsigned long long)addr);
	LOG("creating basic block %s\n", label);
	BasicBlock *bb = BasicBlock::Create(_CTX(), label, f, 0);

	// if it's a label, cache the new basic block.
	if (bb_type == BB_TYPE_NORMAL)
		cpu->dyncom_engine->func_bb[f][addr] = bb;

	return bb;
}
/**
 * @brief Lookup the basicblock according to the address.If not find,create a basicblock and put
 *			it into the map and return it.
 *
 * @param cpu The CPU core structure
 * @param f If not find,create a basicblock and put it into JIT Function f.
 * @param pc Address,used to find the basicblock
 * @param bb_ret The ret basicblock of the current JIT Function
 * @param bb_type The type of the basicblock to be created.And used as part of the basicblock name.
 *
 * @return The result basicblock,found or created. 
 */
const BasicBlock *
lookup_basicblock(cpu_t *cpu, Function* f, addr_t pc, BasicBlock *bb_ret, uint8_t bb_type) {
	// lookup for the basicblock associated to pc in specified function 'f'
	bbaddr_map &bb_addr = cpu->dyncom_engine->func_bb[f];
	bbaddr_map::const_iterator i = bb_addr.find(pc);
	if (i != bb_addr.end())
		return i->second;

	LOG("basic block %c%08llx not found in function %p - creating return basic block!\n", bb_type, pc, f);
	BasicBlock *new_bb = create_basicblock(cpu, pc, cpu->dyncom_engine->cur_func, BB_TYPE_EXTERNAL);
	emit_store_pc_return(cpu, new_bb, pc, bb_ret);

	return new_bb;
}
