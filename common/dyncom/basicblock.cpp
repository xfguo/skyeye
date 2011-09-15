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
#include <dyncom/frontend.h>

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
		 (TAG_START_PAGE && !is_user_mode(cpu))|
		 TAG_AFTER_EXCEPTION|	/* instruction after a exception instruction */
		 TAG_AFTER_SYSCALL	|
		 TAG_ENTRY))			/* client wants to enter guest code here */
		&& (tag & TAG_CODE);	/* only if we actually tagged it */
}

void
arm_emit_store_pc(cpu_t *cpu, BasicBlock *bb_branch, addr_t new_pc)
{
	if(is_user_mode(cpu)){
		Value *v_pc = ConstantInt::get(getIntegerType(cpu->info.address_size), new_pc);
		new StoreInst(v_pc, cpu->ptr_PHYS_PC, bb_branch);
	}else{
#if 0
		Value *v_phys_pc = ConstantInt::get(getIntegerType(cpu->info.address_size), new_pc);
		Value *v_offset = BinaryOperator::Create(Instruction::And, v_phys_pc, CONST(0xfff), "", bb_branch);
		Value *v_page_effec = new LoadInst(cpu->ptr_CURRENT_PAGE_EFFEC, "", false, bb_branch);
		Value *v_effec_pc = BinaryOperator::Create(Instruction::Or, v_offset, v_page_effec, "", bb_branch); 
		new SoreInst(v_phys_pc, cpu->ptr_PHYS_PC, bb_branch);
#endif
		new StoreInst(CONST(new_pc), cpu->ptr_PC, bb_branch);
		new StoreInst(CONST(new_pc), cpu->ptr_PHYS_PC, bb_branch);
		//Value **regs = cpu->ptr_gpr;
		//new StoreInst(CONST(new_pc), regs[15], bb_branch);
		//new StoreInst(CONST(new_pc), cpu->ptr_PC, bb_branch);
		//LET(15, CONST(new_pc));
		//arch_put_reg(cpu, 15, CONST(new_pc), 0, false, bb_branch);
	}
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
	if(is_user_mode(cpu)){
		Value *v_pc = ConstantInt::get(getIntegerType(cpu->info.address_size), new_pc);
		new StoreInst(v_pc, cpu->ptr_PHYS_PC, bb_branch);
		//new StoreInst(v_pc, cpu->ptr_PC, bb_branch);
	}else{
		Value *v_phys_pc = ConstantInt::get(getIntegerType(cpu->info.address_size), new_pc);
		Value *v_offset = BinaryOperator::Create(Instruction::And, v_phys_pc, CONST(0xfff), "", bb_branch);
		Value *v_page_effec = new LoadInst(cpu->ptr_CURRENT_PAGE_EFFEC, "", false, bb_branch);
		Value *v_effec_pc = BinaryOperator::Create(Instruction::Or, v_offset, v_page_effec, "", bb_branch); 
		new StoreInst(v_phys_pc, cpu->ptr_PHYS_PC, bb_branch);
		new StoreInst(v_effec_pc, cpu->ptr_PC, bb_branch);
		//Value **regs = cpu->ptr_gpr;
		//new StoreInst(CONST(new_pc), regs[15], bb_branch);
		//new StoreInst(CONST(new_pc), cpu->ptr_PC, bb_branch);
		//LET(15, CONST(new_pc));
		//arch_put_reg(cpu, 15, CONST(new_pc), 0, false, bb_branch);
	}
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
 * @brief store the next pc when current pc is end of a page. used in kernel simulation.
 *
 * @param cpu
 * @param bb
 * @param new_pc
 */
void
emit_store_pc_end_page(cpu_t *cpu, tag_t tag, BasicBlock *bb, addr_t new_pc)
{
	/* if branch instruction at the end of a page, current pc is used to
	 * count the new pc in the translation of the instruction.
	 */
	if(!(tag & TAG_BRANCH)){
#if 0
		Value *v_phys_pc = ConstantInt::get(getIntegerType(cpu->info.address_size), new_pc);
		Value *v_offset = BinaryOperator::Create(Instruction::And, v_phys_pc, CONST(0xfff), "", bb);
		Value *v_page_effec = new LoadInst(cpu->ptr_CURRENT_PAGE_EFFEC, "", false, bb);
		Value *next_page_effec = BinaryOperator::Create(Instruction::Add, CONST(0x1000), v_page_effec, "", bb);
		Value *v_effec_pc = BinaryOperator::Create(Instruction::Or, v_offset, next_page_effec, "", bb); 
		new StoreInst(v_phys_pc, cpu->ptr_PHYS_PC, bb);
		new StoreInst(v_effec_pc, cpu->ptr_PC, bb);
#else
		//new StoreInst(v_phys_pc, cpu->ptr_PHYS_PC, bb);
		//new StoreInst(CONST(new_pc), cpu->ptr_PC, bb);
		if(save_pc_before_exec(cpu)){ /* for powerpc case */
			Value *v_phys_pc = ConstantInt::get(getIntegerType(cpu->info.address_size), new_pc);
			Value *v_offset = BinaryOperator::Create(Instruction::And, v_phys_pc, CONST(0xfff), "", bb);
			Value *v_page_effec = new LoadInst(cpu->ptr_CURRENT_PAGE_EFFEC, "", false, bb);
			Value *next_page_effec = BinaryOperator::Create(Instruction::Add, CONST(0x1000), v_page_effec, "", bb);
			Value *v_effec_pc = BinaryOperator::Create(Instruction::Or, v_offset, next_page_effec, "", bb); 
			new StoreInst(v_phys_pc, cpu->ptr_PHYS_PC, bb);
			new StoreInst(v_effec_pc, cpu->ptr_PC, bb);
		}
		else{
			Value *pc = new LoadInst(cpu->ptr_PC, "", false, bb);
			if (!(tag & TAG_NEED_PC)) {
				new StoreInst(ADD(pc, CONST(4)), cpu->ptr_PC, bb);
			}
			Value *new_page_effec = AND(ADD(pc, CONST(4)), CONST(0xfffff000));
			new StoreInst(new_page_effec, cpu->ptr_CURRENT_PAGE_EFFEC, bb);
			//new StoreInst(CONST(new_pc), cpu->ptr_PHYS_PC, bb);
		}
#endif
	}
}
void
emit_store_pc_cond(cpu_t *cpu, tag_t tag, Value *cond, BasicBlock *bb, addr_t new_pc)
{
	/* if cond is true, do not save pc */
	Value *v_phys_pc = ConstantInt::get(getIntegerType(cpu->info.address_size), new_pc);
	Value *v_offset = BinaryOperator::Create(Instruction::And, v_phys_pc, CONST(0xfff), "", bb);
	Value *v_page_effec = new LoadInst(cpu->ptr_CURRENT_PAGE_EFFEC, "", false, bb);
	Value *next_page_effec;
	if (tag & TAG_BRANCH) {
		next_page_effec = BinaryOperator::Create(Instruction::Add, CONST(0x1000), v_page_effec, "", bb);
	} else {
		next_page_effec = BinaryOperator::Create(Instruction::Add, CONST(0), v_page_effec, "", bb);
	}
	Value *v_effec_pc = BinaryOperator::Create(Instruction::Or, v_offset, next_page_effec, "", bb); 
	Value *orig_phys_pc = new LoadInst(cpu->ptr_PHYS_PC, "", false, bb);
	Value *orig_pc = new LoadInst(cpu->ptr_PC, "", false, bb);
	new StoreInst(SELECT(cond, orig_phys_pc, v_phys_pc), cpu->ptr_PHYS_PC, bb);
	new StoreInst(SELECT(cond, orig_pc, v_effec_pc), cpu->ptr_PC, bb);
#if 0
	Value *v_phys_pc = ConstantInt::get(getIntegerType(cpu->info.address_size), new_pc);
	Value *v_offset = BinaryOperator::Create(Instruction::And, v_phys_pc, CONST(0xfff), "", bb_branch);
	Value *v_page_effec = new LoadInst(cpu->ptr_CURRENT_PAGE_EFFEC, "", false, bb_branch);
	Value *v_effec_pc = BinaryOperator::Create(Instruction::Or, v_offset, v_page_effec, "", bb_branch); 
	new StoreInst(v_phys_pc, cpu->ptr_PHYS_PC, bb_branch);
	new StoreInst(v_effec_pc, cpu->ptr_PC, bb_branch);
#endif
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
