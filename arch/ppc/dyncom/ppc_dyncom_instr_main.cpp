#include "debug.h"
#include "tracers.h"
#include "ppc_dyncom_dec.h"
#include "ppc_exc.h"
#include "ppc_cpu.h"
#include "ppc_dyncom_alu.h"
#include "ppc_dyncom_run.h"
#include "ppc_tools.h"
#include "ppc_mmu.h"

#include "llvm/Instructions.h"
#include <dyncom/dyncom_llvm.h>
#include <dyncom/frontend.h>
#include "dyncom/basicblock.h"
#include "skyeye.h"

#include "ppc_dyncom_debug.h"
/*
 *      addis           Add Immediate Shifted
 *      .428
 */
int opc_addis_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rD, rA;
	uint32 imm;
	PPC_OPC_TEMPL_D_Shift16(instr, rD, rA, imm);
	debug(DEBUG_TRANSLATE, "In %s, opc=0x%x,rA=%d, imm=%d\n", __FUNCTION__, instr, rA, imm);
	if(rA)
		LET(rD, ADD(R(rA), CONST(imm)));
	else
		LET(rD, CONST(imm));
	return 0;
}
/*
 *	addi		Add Immediate
 *	.425
 */
int opc_addi_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rD, rA;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(instr, rD, rA, imm);
	debug(DEBUG_TRANSLATE, "In %s, opc=0x%x,rA=%d, imm=%d\n", __FUNCTION__, instr, rA, imm);
	if(rA)
		LET(rD, ADD(R(rA), CONST(imm)));
	else
		LET(rD, CONST(imm));
	return 0;
}
/*
 *	cmpi		Compare Immediate
 *	.443
 */
static int opc_cmpi_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	uint32 cr;
	int rA;
	e500_core_t* current_core = get_current_core();
	printf("In %s not implemented\n", __FUNCTION__);
	return 0;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, cr, rA, imm);
	cr >>= 2;
	sint32 a = current_core->gpr[rA];
	sint32 b = imm;
	uint32 c;
/*	if (!VALGRIND_CHECK_READABLE(a, sizeof a)) {
		ht_printf("%08x <--i\n", current_core->pc);
//		SINGLESTEP("");
	}*/
	if (a < b) {
		c = 8;
	} else if (a > b) {
		c = 4;
	} else {
		c = 2;
	}
	if (current_core->xer & XER_SO) c |= 1;
	cr = 7-cr;
//	current_core->cr &= ppc_cmp_and_mask[cr];
	current_core->cr |= c<<(cr*4);
	//fprintf(stderr,"in %s,rA=%d,gpr[rA]=0x%d,im=%d,c=%d\n",__FUNCTION__,rA,current_core->gpr[rA],imm,c);
}
/*
 *	rlwinmx		Rotate Left Word Immediate then AND with Mask
 *	.618
 */
static int opc_rlwinmx_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rS, rA, SH, MB, ME;
	PPC_OPC_TEMPL_M(instr, rS, rA, SH, MB, ME);
	//uint32 v = ppc_word_rotl(current_core->gpr[rS], SH);
	Value* v = ROTL(R(rS), SH);
	uint32 mask = ppc_mask(MB, ME);
	//current_core->gpr[rA] = v & mask;
	LET(rA, AND(v, CONST(mask)));
	if (instr & PPC_OPC_Rc) {
		// update cr0 flags
		fprintf(stderr, "Not implemented update cr\n");
		ppc_dyncom_update_cr0(cpu, bb, rA);
	}
	return 0;
}
/*
 *	lwz		Load Word and Zero
 *	.557
 */
static int opc_lwz_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	e500_core_t* current_core = get_current_core();
	int rA, rD;
	uint32 imm;
	printf("In %s not implemented\n", __FUNCTION__);
	return 0;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rD, rA, imm);
	uint32 r;

	int ret = ppc_read_effective_word((rA?current_core->gpr[rA]:0)+imm, &r);
	if (ret == PPC_MMU_OK) {
		current_core->gpr[rD] = r;
	}	
}
/*
 *	lwzu		Load Word and Zero with Update
 *	.558
 */
static int opc_lwzu_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	e500_core_t* current_core = get_current_core();
	int rA, rD;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(instr, rD, rA, imm);
	// FIXME: check rA!=0 && rA!=rD
	arch_read_memory(cpu, bb, ADD(R(rA), CONST(imm)), 0, 32);
	/* FIXME:update rD */
	return 0;
}
/*
 *	stw		Store Word
 *	.659
 */
static int opc_stw_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rA, rS;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(instr, rS, rA, imm);
	arch_write_memory(cpu, bb, ADD(R(rA), CONST(imm)), R(rS), 32);
	return 0;
}
/*
 *	stwu		Store Word with Update
 *	.663
 */
static int opc_stwu_translate(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int rA, rS;
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(instr, rS, rA, imm);
	// FIXME: check rA!=0
	/* FIXME: no MMU now */
	arch_write_memory(cpu, bb, ADD(R(rA), CONST(imm)), R(rS), 32);
	/* updata rS */
	//LET(R(rS), ADD(R(rA), CONST(imm)));
	return 0;
}
/*
 *	bx		Branch
 *	.435
 */
int opc_bx_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	e500_core_t* current_core = get_core_from_dyncom_cpu(cpu);
	uint32 li;
	PPC_OPC_TEMPL_I(instr, li);
	*tag = TAG_BRANCH;
	/*if the branch target is out of the page, stop the tagging */
	debug(DEBUG_TAG, "pc=0x%x,page_begin=0x%x,page_end=0x%x\n",
			current_core->pc,get_begin_of_page(current_core->pc),get_end_of_page(current_core->pc));
	if(li < get_begin_of_page(current_core->pc) && li > get_end_of_page(current_core->pc)){
		*tag |= TAG_STOP;
	}
	*new_pc = li + phys_pc;
	debug(DEBUG_TAG, "In %s, new_pc=0x%x\n", __FUNCTION__, *new_pc);
	return PPC_INSN_SIZE;
}
static int opc_bx_translate(cpu_t* cpu, uint32_t instr, BasicBlock* bb)
{
	e500_core_t* current_core = get_core_from_dyncom_cpu(cpu);
	uint32 li;
	PPC_OPC_TEMPL_I(instr, li);
	if (!(instr & PPC_OPC_AA)) {
		arch_store(CONST(li + current_core->phys_pc), cpu->ptr_PHYS_PC, bb);
		debug(DEBUG_TRANSLATE, "In %s, li=0x%x, pc=0x%x, br 0x%x\n",
				__FUNCTION__, li, current_core->phys_pc, li + current_core->phys_pc);
	}else{
		arch_store(CONST(li), cpu->ptr_PHYS_PC, bb);
		debug(DEBUG_TRANSLATE, "In %s, li=0x%x, br 0x%x\n", __FUNCTION__, li, li);
	}
	if (instr & PPC_OPC_LK) {
		LET32_BY_PTR(&current_core->lr, CONST(4 + current_core->phys_pc));
	}
	return PPC_INSN_SIZE;
}
/*
 *	bcx		Branch Conditional
 *	.436
 */
int opc_bcx_tag(cpu_t *cpu, uint32_t instr, addr_t phys_pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc){
	e500_core_t* current_core = get_core_from_dyncom_cpu(cpu);
	*tag = TAG_COND_BRANCH;
	*new_pc = NEW_PC_NONE;
	debug(DEBUG_TAG, "In %s, new_pc=0x%x\n", __FUNCTION__, *new_pc);
	return PPC_INSN_SIZE;
}
static int opc_bcx_translate(cpu_t* cpu, uint32_t instr, BasicBlock* bb)
{
	return 0;
#if 0
	e500_core_t* current_core = get_current_core();
	uint32 BO, BI, BD;
	PPC_OPC_TEMPL_B(current_core->current_opc, BO, BI, BD);
	if (!(BO & 4)) {
		current_core->ctr--;
	}
	bool_t bo2 = ((BO & 2)?1:0);
	bool_t bo8 = ((BO & 8)?1:0); // branch condition true
	bool_t cr = ((current_core->cr & (1<<(31-BI)))?1:0) ;
	if (((BO & 4) || ((current_core->ctr!=0) ^ bo2))
	&& ((BO & 16) || (!(cr ^ bo8)))) {
		if (!(current_core->current_opc & PPC_OPC_AA)) {
			BD += current_core->pc;
		}
		if (current_core->current_opc & PPC_OPC_LK) {
			current_core->lr = current_core->pc + 4;
		}
		current_core->npc = BD;
	}
	//fprintf(stderr,"in %s,BO=0x%x,BI=0x%x,BD=0x%x,cr=0x%x,cr^bo8=0x%x,ctr=0x%x,pc=0x%x\n",__FUNCTION__,BO,BI,BD,current_core->cr,(cr ^ bo8),current_core->ctr, current_core->pc);
#endif
}

/* Interfaces */
ppc_opc_func_t ppc_opc_twi_func;
ppc_opc_func_t ppc_opc_mulli_func;
ppc_opc_func_t ppc_opc_subfic_func;
ppc_opc_func_t ppc_opc_cmpli_func;
ppc_opc_func_t ppc_opc_cmpi_func = {
	opc_default_tag,
	opc_cmpi_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_addic_func;		
ppc_opc_func_t ppc_opc_addic__func;		
ppc_opc_func_t ppc_opc_addi_func = {
	opc_default_tag,
	opc_addi_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_addis_func = {
	opc_default_tag,
	opc_addis_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_sc_func;
ppc_opc_func_t ppc_opc_bx_func = {
	opc_bx_tag,
	opc_bx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_bcx_func = {
	opc_bcx_tag,
	opc_bcx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_rlwimix_func;
ppc_opc_func_t ppc_opc_rlwinmx_func = {
	opc_default_tag,
	opc_rlwinmx_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_rlwnmx_func;
ppc_opc_func_t ppc_opc_ori_func;
ppc_opc_func_t ppc_opc_oris_func;
ppc_opc_func_t ppc_opc_xori_func;
ppc_opc_func_t ppc_opc_xoris_func;
ppc_opc_func_t ppc_opc_andi__func;
ppc_opc_func_t ppc_opc_andis__func;
ppc_opc_func_t ppc_opc_lwz_func = {
	opc_default_tag,
	opc_lwz_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_lwzu_func = {
	opc_default_tag,
	opc_lwzu_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_lbz_func;
ppc_opc_func_t ppc_opc_lbzu_func;
ppc_opc_func_t ppc_opc_stw_func = {
	opc_default_tag,
	opc_stw_translate,
	opc_invalid_translate_cond,
};
 ppc_opc_func_t ppc_opc_stwu_func = {
	opc_default_tag,
	opc_stwu_translate,
	opc_invalid_translate_cond,
};
ppc_opc_func_t ppc_opc_stb_func;
ppc_opc_func_t ppc_opc_stbu_func;
ppc_opc_func_t ppc_opc_lhz_func;
ppc_opc_func_t ppc_opc_lhzu_func;
ppc_opc_func_t ppc_opc_lha_func;
ppc_opc_func_t ppc_opc_lhau_func;
ppc_opc_func_t ppc_opc_sth_func;
ppc_opc_func_t ppc_opc_sthu_func;
ppc_opc_func_t ppc_opc_lmw_func;
ppc_opc_func_t ppc_opc_stmw_func;
ppc_opc_func_t ppc_opc_lfs_func;
ppc_opc_func_t ppc_opc_lfsu_func;
ppc_opc_func_t ppc_opc_lfd_func;
ppc_opc_func_t ppc_opc_lfdu_func;
ppc_opc_func_t ppc_opc_stfs_func;
ppc_opc_func_t ppc_opc_stfsu_func;
ppc_opc_func_t ppc_opc_stfd_func;
ppc_opc_func_t ppc_opc_stfdu_func;
