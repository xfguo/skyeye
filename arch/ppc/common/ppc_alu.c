/*
 *	PearPC
 *	ppc_alu.cc
 *
 *	Copyright (C) 2003, 2004 Sebastian Biallas (sb@biallas.net)
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
#include "debug/tracers.h"
#include "cpu/debug.h"
*/
#include "debug.h"
#include "tracers.h"
#include "ppc_alu.h"
#include "ppc_dec.h"
#include "ppc_exc.h"
#include "ppc_cpu.h"
#include "ppc_opc.h"
#include "ppc_tools.h"
#include "ppc_mmu.h"
#include "portable/portable.h"

static inline uint32 ppc_mask(int MB, int ME)
{
	uint32 mask;
	if (MB <= ME) {
		if (ME - MB == 31) {
			mask = 0xffffffff;
		} else {
			mask = ((1 << (ME - MB + 1)) - 1) << (31 - ME);
		}
	} else {
		mask = ppc_word_rotl((1 << (32 - MB + ME + 1)) - 1, 31 - ME);
	}
	return mask;
}

/*
 *	addx		Add
 *	.422
 */
void ppc_opc_addx()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	current_core->gpr[rD] = current_core->gpr[rA] + current_core->gpr[rB];
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
}

/*
 *	addox		Add with Overflow
 *	.422
 */
void ppc_opc_addox()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	current_core->gpr[rD] = current_core->gpr[rA] + current_core->gpr[rB];
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
	// update XER flags
	PPC_ALU_ERR("addox unimplemented\n");
}

/*
 *	addcx		Add Carrying
 *	.423
 */
void ppc_opc_addcx()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	uint32 a = current_core->gpr[rA];
	current_core->gpr[rD] = a + current_core->gpr[rB];
	// update xer
	if (current_core->gpr[rD] < a) {
		current_core->xer |= XER_CA;
	} else {
		current_core->xer &= ~XER_CA;
	}
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
}

/*
 *	addcox		Add Carrying with Overflow
 *	.423
 */
void ppc_opc_addcox()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	uint32 a = current_core->gpr[rA];
	current_core->gpr[rD] = a + current_core->gpr[rB];
	// update xer
	if (current_core->gpr[rD] < a) {
		current_core->xer |= XER_CA;
	} else {
		current_core->xer &= ~XER_CA;
	}
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
	// update XER flags
	PPC_ALU_ERR("addcox unimplemented\n");
}

/*
 *	addex		Add Extended
 *	.424
 */
void ppc_opc_addex()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	uint32 a = current_core->gpr[rA];
	uint32 b = current_core->gpr[rB];
	uint32 ca = ((current_core->xer & XER_CA) ? 1 : 0);
	current_core->gpr[rD] = a + b + ca;
	// update xer
	if (ppc_carry_3(a, b, ca)) {
		current_core->xer |= XER_CA;
	} else {
		current_core->xer &= ~XER_CA;
	}
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
}

/*
 *	addeox		Add Extended with Overflow
 *	.424
 */
void ppc_opc_addeox()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	uint32 a = current_core->gpr[rA];
	uint32 b = current_core->gpr[rB];
	uint32 ca = ((current_core->xer & XER_CA) ? 1 : 0);
	current_core->gpr[rD] = a + b + ca;
	// update xer
	if (ppc_carry_3(a, b, ca)) {
		current_core->xer |= XER_CA;
	} else {
		current_core->xer &= ~XER_CA;
	}
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
	// update XER flags
	PPC_ALU_ERR("addeox unimplemented\n");
}

/*
 *	addi		Add Immediate
 *	.425
 */
void ppc_opc_addi()
{
	int rD, rA;
	uint32 imm;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rD, rA, imm);
	current_core->gpr[rD] = (rA ? current_core->gpr[rA] : 0) + imm;
}

/*
 *	addic		Add Immediate Carrying
 *	.426
 */
void ppc_opc_addic()
{
	int rD, rA;
	uint32 imm;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rD, rA, imm);
	uint32 a = current_core->gpr[rA];
	current_core->gpr[rD] = a + imm;
	// update XER
	if (current_core->gpr[rD] < a) {
		current_core->xer |= XER_CA;
	} else {
		current_core->xer &= ~XER_CA;
	}
}

/*
 *	addic.		Add Immediate Carrying and Record
 *	.427
 */
void ppc_opc_addic_()
{
	int rD, rA;
	uint32 imm;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rD, rA, imm);
	uint32 a = current_core->gpr[rA];
	current_core->gpr[rD] = a + imm;
	// update XER
	if (current_core->gpr[rD] < a) {
		current_core->xer |= XER_CA;
	} else {
		current_core->xer &= ~XER_CA;
	}
	// update cr0 flags
	ppc_update_cr0(current_core->gpr[rD]);
}

/*
 *	addis		Add Immediate Shifted
 *	.428
 */
void ppc_opc_addis()
{
	int rD, rA;
	uint32 imm;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_D_Shift16(current_core->current_opc, rD, rA, imm);
	current_core->gpr[rD] = (rA ? current_core->gpr[rA] : 0) + imm;
}

/*
 *	addmex		Add to Minus One Extended
 *	.429
 */
void ppc_opc_addmex()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	PPC_OPC_ASSERT(rB == 0);
	uint32 a = current_core->gpr[rA];
	uint32 ca = ((current_core->xer & XER_CA) ? 1 : 0);
	current_core->gpr[rD] = a + ca + 0xffffffff;
	if (a || ca) {
		current_core->xer |= XER_CA;
	} else {
		current_core->xer &= ~XER_CA;
	}
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
}

/*
 *	addmeox		Add to Minus One Extended with Overflow
 *	.429
 */
void ppc_opc_addmeox()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	PPC_OPC_ASSERT(rB == 0);
	uint32 a = current_core->gpr[rA];
	uint32 ca = ((current_core->xer & XER_CA) ? 1 : 0);
	current_core->gpr[rD] = a + ca + 0xffffffff;
	if (a || ca) {
		current_core->xer |= XER_CA;
	} else {
		current_core->xer &= ~XER_CA;
	}
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
	// update XER flags
	PPC_ALU_ERR("addmeox unimplemented\n");
}

/*
 *	addzex		Add to Zero Extended
 *	.430
 */
void ppc_opc_addzex()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	PPC_OPC_ASSERT(rB == 0);
	uint32 a = current_core->gpr[rA];
	uint32 ca = ((current_core->xer & XER_CA) ? 1 : 0);
	current_core->gpr[rD] = a + ca;
	if ((a == 0xffffffff) && ca) {
		current_core->xer |= XER_CA;
	} else {
		current_core->xer &= ~XER_CA;
	}
	// update xer
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
}

/*
 *	addzeox		Add to Zero Extended with Overflow
 *	.430
 */
void ppc_opc_addzeox()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	PPC_OPC_ASSERT(rB == 0);
	uint32 a = current_core->gpr[rA];
	uint32 ca = ((current_core->xer & XER_CA) ? 1 : 0);
	current_core->gpr[rD] = a + ca;
	if ((a == 0xffffffff) && ca) {
		current_core->xer |= XER_CA;
	} else {
		current_core->xer &= ~XER_CA;
	}
	// update xer
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
	// update XER flags
	PPC_ALU_ERR("addzeox unimplemented\n");
}

/*
 *	andx		AND
 *	.431
 */
void ppc_opc_andx()
{
	int rS, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	current_core->gpr[rA] = current_core->gpr[rS] & current_core->gpr[rB];
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rA]);
	}
}

/*
 *	andcx		AND with Complement
 *	.432
 */
void ppc_opc_andcx()
{
	int rS, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	current_core->gpr[rA] = current_core->gpr[rS] & ~current_core->gpr[rB];
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rA]);
	}
}

/*
 *	andi.		AND Immediate
 *	.433
 */
void ppc_opc_andi_()
{
	int rS, rA;
	uint32 imm;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_D_UImm(current_core->current_opc, rS, rA, imm);
	current_core->gpr[rA] = current_core->gpr[rS] & imm;
	// update cr0 flags
	ppc_update_cr0(current_core->gpr[rA]);
}

/*
 *	andis.		AND Immediate Shifted
 *	.434
 */
void ppc_opc_andis_()
{
	int rS, rA;
	uint32 imm;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_D_Shift16(current_core->current_opc, rS, rA, imm);
	current_core->gpr[rA] = current_core->gpr[rS] & imm;
	// update cr0 flags
	ppc_update_cr0(current_core->gpr[rA]);
}

/*
 *	cmp		Compare
 *	.442
 */
uint32 ppc_cmp_and_mask[8] = {
	0xfffffff0,
	0xffffff0f,
	0xfffff0ff,
	0xffff0fff,
	0xfff0ffff,
	0xff0fffff,
	0xf0ffffff,
	0x0fffffff,
};

void ppc_opc_cmp()
{
	uint32 cr;
	int rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_X(current_core->current_opc, cr, rA, rB);
	cr >>= 2;
	sint32 a = current_core->gpr[rA];
	sint32 b = current_core->gpr[rB];
	uint32 c;
	if (a < b) {
		c = 8;
	} else if (a > b) {
		c = 4;
	} else {
		c = 2;
	}
	if (current_core->xer & XER_SO)
		c |= 1;
	cr = 7 - cr;
	current_core->cr &= ppc_cmp_and_mask[cr];
	current_core->cr |= c << (cr * 4);
}

/*
 *	cmpi		Compare Immediate
 *	.443
 */
void ppc_opc_cmpi()
{
	uint32 cr;
	int rA;
	e500_core_t *current_core = get_current_core();
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, cr, rA, imm);
	cr >>= 2;
	sint32 a = current_core->gpr[rA];
	sint32 b = imm;
	uint32 c;
	if (a < b) {
		c = 8;
	} else if (a > b) {
		c = 4;
	} else {
		c = 2;
	}
	if (current_core->xer & XER_SO)
		c |= 1;
	cr = 7 - cr;
	current_core->cr &= ppc_cmp_and_mask[cr];
	current_core->cr |= c << (cr * 4);
}

/*
 *	cmpl		Compare Logical
 *	.444
 */
void ppc_opc_cmpl()
{
	uint32 cr;
	e500_core_t *current_core = get_current_core();
	int rA, rB;
	PPC_OPC_TEMPL_X(current_core->current_opc, cr, rA, rB);
	cr >>= 2;
	uint32 a = current_core->gpr[rA];
	uint32 b = current_core->gpr[rB];
	uint32 c;
	if (a < b) {
		c = 8;
	} else if (a > b) {
		c = 4;
	} else {
		c = 2;
	}
	if (current_core->xer & XER_SO)
		c |= 1;
	cr = 7 - cr;
	current_core->cr &= ppc_cmp_and_mask[cr];
	current_core->cr |= c << (cr * 4);
}

/*
 *	cmpli		Compare Logical Immediate
 *	.445
 */
void ppc_opc_cmpli()
{
	uint32 cr;
	e500_core_t *current_core = get_current_core();
	int rA;
	uint32 imm;
	PPC_OPC_TEMPL_D_UImm(current_core->current_opc, cr, rA, imm);
	cr >>= 2;
	uint32 a = current_core->gpr[rA];
	uint32 b = imm;
	uint32 c;
	if (a < b) {
		c = 8;
	} else if (a > b) {
		c = 4;
	} else {
		c = 2;
	}
	if (current_core->xer & XER_SO)
		c |= 1;
	cr = 7 - cr;
	current_core->cr &= ppc_cmp_and_mask[cr];
	current_core->cr |= c << (cr * 4);
}

/*
 *	cntlzwx		Count Leading Zeros Word
 *	.447
 */
void ppc_opc_cntlzwx()
{
	int rS, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	PPC_OPC_ASSERT(rB == 0);
	uint32 n = 0;
	uint32 x = 0x80000000;
	uint32 v = current_core->gpr[rS];
	while (!(v & x)) {
		n++;
		if (n == 32)
			break;
		x >>= 1;
	}
	current_core->gpr[rA] = n;
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rA]);
	}
}

/*
 *	crand		Condition Register AND
 *	.448
 */
void ppc_opc_crand()
{
	int crD, crA, crB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_X(current_core->current_opc, crD, crA, crB);
	if ((current_core->cr & (1 << (31 - crA)))
	    && (current_core->cr & (1 << (31 - crB)))) {
		current_core->cr |= (1 << (31 - crD));
	} else {
		current_core->cr &= ~(1 << (31 - crD));
	}
}

/*
 *	crandc		Condition Register AND with Complement
 *	.449
 */
void ppc_opc_crandc()
{
	int crD, crA, crB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_X(current_core->current_opc, crD, crA, crB);
	if ((current_core->cr & (1 << (31 - crA)))
	    && !(current_core->cr & (1 << (31 - crB)))) {
		current_core->cr |= (1 << (31 - crD));
	} else {
		current_core->cr &= ~(1 << (31 - crD));
	}
}

/*
 *	creqv		Condition Register Equivalent
 *	.450
 */
void ppc_opc_creqv()
{
	int crD, crA, crB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_X(current_core->current_opc, crD, crA, crB);
	if (((current_core->cr & (1 << (31 - crA)))
	     && (current_core->cr & (1 << (31 - crB))))
	    || (!(current_core->cr & (1 << (31 - crA)))
		&& !(current_core->cr & (1 << (31 - crB))))) {
		current_core->cr |= (1 << (31 - crD));
	} else {
		current_core->cr &= ~(1 << (31 - crD));
	}
}

/*
 *	crnand		Condition Register NAND
 *	.451
 */
void ppc_opc_crnand()
{
	int crD, crA, crB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_X(current_core->current_opc, crD, crA, crB);
	if (!
	    ((current_core->cr & (1 << (31 - crA)))
	     && (current_core->cr & (1 << (31 - crB))))) {
		current_core->cr |= (1 << (31 - crD));
	} else {
		current_core->cr &= ~(1 << (31 - crD));
	}
}

/*
 *	crnor		Condition Register NOR
 *	.452
 */
void ppc_opc_crnor()
{
	int crD, crA, crB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_X(current_core->current_opc, crD, crA, crB);
	uint32 t = (1 << (31 - crA)) | (1 << (31 - crB));
	if (!(current_core->cr & t)) {
		current_core->cr |= (1 << (31 - crD));
	} else {
		current_core->cr &= ~(1 << (31 - crD));
	}
}

/*
 *	cror		Condition Register OR
 *	.453
 */
void ppc_opc_cror()
{
	int crD, crA, crB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_X(current_core->current_opc, crD, crA, crB);
	uint32 t = (1 << (31 - crA)) | (1 << (31 - crB));
	if (current_core->cr & t) {
		current_core->cr |= (1 << (31 - crD));
	} else {
		current_core->cr &= ~(1 << (31 - crD));
	}
}

/*
 *	crorc		Condition Register OR with Complement
 *	.454
 */
void ppc_opc_crorc()
{
	int crD, crA, crB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_X(current_core->current_opc, crD, crA, crB);
	if ((current_core->cr & (1 << (31 - crA)))
	    || !(current_core->cr & (1 << (31 - crB)))) {
		current_core->cr |= (1 << (31 - crD));
	} else {
		current_core->cr &= ~(1 << (31 - crD));
	}
}

/*
 *	crxor		Condition Register XOR
 *	.448
 */
void ppc_opc_crxor()
{
	int crD, crA, crB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_X(current_core->current_opc, crD, crA, crB);
	if ((!(current_core->cr & (1 << (31 - crA)))
	     && (current_core->cr & (1 << (31 - crB))))
	    || ((current_core->cr & (1 << (31 - crA)))
		&& !(current_core->cr & (1 << (31 - crB))))) {
		current_core->cr |= (1 << (31 - crD));
	} else {
		current_core->cr &= ~(1 << (31 - crD));
	}
}

/*
 *	divwx		Divide Word
 *	.470
 */
void ppc_opc_divwx()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	if (!current_core->gpr[rB]) {
		PPC_ALU_WARN("division by zero @%08x\n", current_core->pc);
	}
	sint32 a = current_core->gpr[rA];
	sint32 b = current_core->gpr[rB];
	current_core->gpr[rD] = a / b;
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
}

/*
 *	divwox		Divide Word with Overflow
 *	.470
 */
void ppc_opc_divwox()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	if (!current_core->gpr[rB]) {
		PPC_ALU_ERR("division by zero\n");
	}
	sint32 a = current_core->gpr[rA];
	sint32 b = current_core->gpr[rB];
	current_core->gpr[rD] = a / b;
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
	// update XER flags
	PPC_ALU_ERR("divwox unimplemented\n");
}

/*
 *	divwux		Divide Word Unsigned
 *	.472
 */
void ppc_opc_divwux()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	if (!current_core->gpr[rB]) {
		PPC_ALU_WARN("division by zero @%08x\n", current_core->pc);
	}
	current_core->gpr[rD] = current_core->gpr[rA] / current_core->gpr[rB];
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
}

/*
 *	divwuox		Divide Word Unsigned with Overflow
 *	.472
 */
void ppc_opc_divwuox()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	if (!current_core->gpr[rB]) {
//              PPC_ALU_ERR("division by zero\n");
	}
	current_core->gpr[rD] = current_core->gpr[rA] / current_core->gpr[rB];
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
	// update XER flags
	PPC_ALU_ERR("divwuox unimplemented\n");
}

/*
 *	eqvx		Equivalent
 *	.480
 */
void ppc_opc_eqvx()
{
	int rS, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	current_core->gpr[rA] =
	    ~(current_core->gpr[rS] ^ current_core->gpr[rB]);
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rA]);
	}
}

/*
 *	extsbx		Extend Sign Byte
 *	.481
 */
void ppc_opc_extsbx()
{
	int rS, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	PPC_OPC_ASSERT(rB == 0);
	current_core->gpr[rA] = current_core->gpr[rS];
	if (current_core->gpr[rA] & 0x80) {
		current_core->gpr[rA] |= 0xffffff00;
	} else {
		current_core->gpr[rA] &= ~0xffffff00;
	}
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rA]);
	}
}

/*
 *	extshx		Extend Sign Half Word
 *	.482
 */
void ppc_opc_extshx()
{
	int rS, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	PPC_OPC_ASSERT(rB == 0);
	current_core->gpr[rA] = current_core->gpr[rS];
	if (current_core->gpr[rA] & 0x8000) {
		current_core->gpr[rA] |= 0xffff0000;
	} else {
		current_core->gpr[rA] &= ~0xffff0000;
	}
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rA]);
	}
}

/*
 *	mulhwx		Multiply High Word
 *	.595
 */
void ppc_opc_mulhwx()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	sint64 a = (sint32) current_core->gpr[rA];
	sint64 b = (sint32) current_core->gpr[rB];
	sint64 c = a * b;
	current_core->gpr[rD] = ((uint64) c) >> 32;
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
//              PPC_ALU_WARN("mulhw. correct?\n");
	}
}

/*
 *	mulhwux		Multiply High Word Unsigned
 *	.596
 */
void ppc_opc_mulhwux()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	uint64 a = current_core->gpr[rA];
	uint64 b = current_core->gpr[rB];
	uint64 c = a * b;
	current_core->gpr[rD] = c >> 32;
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
}

/*
 *	mulli		Multiply Low Immediate
 *	.598
 */
void ppc_opc_mulli()
{
	int rD, rA;
	e500_core_t *current_core = get_current_core();
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rD, rA, imm);
	// FIXME: signed / unsigned correct?
	current_core->gpr[rD] = current_core->gpr[rA] * imm;
}

/*
 *	mullwx		Multiply Low Word
 *	.599
 */
void ppc_opc_mullwx()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	current_core->gpr[rD] = current_core->gpr[rA] * current_core->gpr[rB];
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
	if (current_core->current_opc & PPC_OPC_OE) {
		// update XER flags
		PPC_ALU_ERR("mullwx unimplemented\n");
	}
}

/*
 *	nandx		NAND
 *	.600
 */
void ppc_opc_nandx()
{
	int rS, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	current_core->gpr[rA] =
	    ~(current_core->gpr[rS] & current_core->gpr[rB]);
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rA]);
	}
}

/*
 *	negx		Negate
 *	.601
 */
void ppc_opc_negx()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	PPC_OPC_ASSERT(rB == 0);
	current_core->gpr[rD] = -current_core->gpr[rA];
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
}

/*
 *	negox		Negate with Overflow
 *	.601
 */
void ppc_opc_negox()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	PPC_OPC_ASSERT(rB == 0);
	current_core->gpr[rD] = -current_core->gpr[rA];
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
	// update XER flags
	PPC_ALU_ERR("negox unimplemented\n");
}

/*
 *	norx		NOR
 *	.602
 */
void ppc_opc_norx()
{
	int rS, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	current_core->gpr[rA] =
	    ~(current_core->gpr[rS] | current_core->gpr[rB]);
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rA]);
	}
}

/*
 *	orx		OR
 *	.603
 */
void ppc_opc_orx()
{
	int rS, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	current_core->gpr[rA] = current_core->gpr[rS] | current_core->gpr[rB];
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rA]);
	}
}

/*
 *	orcx		OR with Complement
 *	.604
 */
void ppc_opc_orcx()
{
	int rS, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	current_core->gpr[rA] = current_core->gpr[rS] | ~current_core->gpr[rB];
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rA]);
	}
}

/*
 *	ori		OR Immediate
 *	.605
 */
void ppc_opc_ori()
{
	int rS, rA;
	e500_core_t *current_core = get_current_core();
	uint32 imm;
	PPC_OPC_TEMPL_D_UImm(current_core->current_opc, rS, rA, imm);
	current_core->gpr[rA] = current_core->gpr[rS] | imm;
}

/*
 *	oris		OR Immediate Shifted
 *	.606
 */
void ppc_opc_oris()
{
	int rS, rA;
	e500_core_t *current_core = get_current_core();
	uint32 imm;
	PPC_OPC_TEMPL_D_Shift16(current_core->current_opc, rS, rA, imm);
	current_core->gpr[rA] = current_core->gpr[rS] | imm;
}

/*
 *	rlwimix		Rotate Left Word Immediate then Mask Insert
 *	.617
 */
void ppc_opc_rlwimix()
{
	int rS, rA, SH, MB, ME;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_M(current_core->current_opc, rS, rA, SH, MB, ME);
	uint32 v = ppc_word_rotl(current_core->gpr[rS], SH);
	uint32 mask = ppc_mask(MB, ME);
	current_core->gpr[rA] = (v & mask) | (current_core->gpr[rA] & ~mask);
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rA]);
	}
}

/*
 *	rlwinmx		Rotate Left Word Immediate then AND with Mask
 *	.618
 */
void ppc_opc_rlwinmx()
{
	int rS, rA, SH, MB, ME;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_M(current_core->current_opc, rS, rA, SH, MB, ME);
	uint32 v = ppc_word_rotl(current_core->gpr[rS], SH);
	uint32 mask = ppc_mask(MB, ME);
	current_core->gpr[rA] = v & mask;
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rA]);
	}
}

/*
 *	rlwnmx		Rotate Left Word then AND with Mask
 *	.620
 */
void ppc_opc_rlwnmx()
{
	int rS, rA, rB, MB, ME;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_M(current_core->current_opc, rS, rA, rB, MB, ME);
	uint32 v = ppc_word_rotl(current_core->gpr[rS], current_core->gpr[rB]);
	uint32 mask = ppc_mask(MB, ME);
	current_core->gpr[rA] = v & mask;
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rA]);
	}
}

/*
 *	slwx		Shift Left Word
 *	.625
 */
void ppc_opc_slwx()
{
	int rS, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	uint32 s = current_core->gpr[rB] & 0x3f;
	if (s > 31) {
		current_core->gpr[rA] = 0;
	} else {
		current_core->gpr[rA] = current_core->gpr[rS] << s;
	}
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rA]);
	}
}

/*
 *	srawx		Shift Right Algebraic Word
 *	.628
 */
void ppc_opc_srawx()
{
	int rS, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	uint32 SH = current_core->gpr[rB] & 0x3f;
	current_core->gpr[rA] = current_core->gpr[rS];
	current_core->xer &= ~XER_CA;
	if (current_core->gpr[rA] & 0x80000000) {
		uint32 ca = 0;
		uint i;
		for (i = 0; i < SH; i++) {
			if (current_core->gpr[rA] & 1)
				ca = 1;
			current_core->gpr[rA] >>= 1;
			current_core->gpr[rA] |= 0x80000000;
		}
		if (ca)
			current_core->xer |= XER_CA;
	} else {
		if (SH > 31) {
			current_core->gpr[rA] = 0;
		} else {
			current_core->gpr[rA] >>= SH;
		}
	}
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rA]);
	}
}

/*
 *	srawix		Shift Right Algebraic Word Immediate
 *	.629
 */
void ppc_opc_srawix()
{
	int rS, rA;
	e500_core_t *current_core = get_current_core();
	uint32 SH;
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, SH);
	current_core->gpr[rA] = current_core->gpr[rS];
	current_core->xer &= ~XER_CA;
	if (current_core->gpr[rA] & 0x80000000) {
		uint32 ca = 0;
		uint i;
		for (i = 0; i < SH; i++) {
			if (current_core->gpr[rA] & 1)
				ca = 1;
			current_core->gpr[rA] >>= 1;
			current_core->gpr[rA] |= 0x80000000;
		}
		if (ca)
			current_core->xer |= XER_CA;
	} else {
		if (SH > 31) {
			current_core->gpr[rA] = 0;
		} else {
			current_core->gpr[rA] >>= SH;
		}
	}
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rA]);
	}
}

/*
 *	srwx		Shift Right Word
 *	.631
 */
void ppc_opc_srwx()
{
	int rS, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	uint32 v = current_core->gpr[rB] & 0x3f;
	if (v > 31) {
		current_core->gpr[rA] = 0;
	} else {
		current_core->gpr[rA] = current_core->gpr[rS] >> v;
	}
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rA]);
	}
}

/*
 *	subfx		Subtract From
 *	.666
 */
void ppc_opc_subfx()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	current_core->gpr[rD] =
	    ~current_core->gpr[rA] + current_core->gpr[rB] + 1;
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
}

/*
 *	subfox		Subtract From with Overflow
 *	.666
 */
void ppc_opc_subfox()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	current_core->gpr[rD] =
	    ~current_core->gpr[rA] + current_core->gpr[rB] + 1;
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
	// update XER flags
	PPC_ALU_ERR("subfox unimplemented\n");
}

/*
 *	subfcx		Subtract From Carrying
 *	.667
 */
void ppc_opc_subfcx()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	uint32 a = current_core->gpr[rA];
	uint32 b = current_core->gpr[rB];
	current_core->gpr[rD] = ~a + b + 1;
	// update xer
	if (ppc_carry_3(~a, b, 1)) {
		current_core->xer |= XER_CA;
	} else {
		current_core->xer &= ~XER_CA;
	}
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
}

/*
 *	subfcox		Subtract From Carrying with Overflow
 *	.667
 */
void ppc_opc_subfcox()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	uint32 a = current_core->gpr[rA];
	uint32 b = current_core->gpr[rB];
	current_core->gpr[rD] = ~a + b + 1;
	// update xer
	if (ppc_carry_3(~a, b, 1)) {
		current_core->xer |= XER_CA;
	} else {
		current_core->xer &= ~XER_CA;
	}
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
	// update XER flags
	PPC_ALU_ERR("subfcox unimplemented\n");
}

/*
 *	subfex		Subtract From Extended
 *	.668
 */
void ppc_opc_subfex()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	uint32 a = current_core->gpr[rA];
	uint32 b = current_core->gpr[rB];
	uint32 ca = ((current_core->xer & XER_CA) ? 1 : 0);
	current_core->gpr[rD] = ~a + b + ca;
	// update xer
	if (ppc_carry_3(~a, b, ca)) {
		current_core->xer |= XER_CA;
	} else {
		current_core->xer &= ~XER_CA;
	}
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
}

/*
 *	subfeox		Subtract From Extended with Overflow
 *	.668
 */
void ppc_opc_subfeox()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	uint32 a = current_core->gpr[rA];
	uint32 b = current_core->gpr[rB];
	uint32 ca = ((current_core->xer & XER_CA) ? 1 : 0);
	current_core->gpr[rD] = ~a + b + ca;
	// update xer
	if (ppc_carry_3(~a, b, ca)) {
		current_core->xer |= XER_CA;
	} else {
		current_core->xer &= ~XER_CA;
	}
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
	// update XER flags
	PPC_ALU_ERR("subfeox unimplemented\n");
}

/*
 *	subfic		Subtract From Immediate Carrying
 *	.669
 */
void ppc_opc_subfic()
{
	int rD, rA;
	e500_core_t *current_core = get_current_core();
	uint32 imm;
	PPC_OPC_TEMPL_D_SImm(current_core->current_opc, rD, rA, imm);
	uint32 a = current_core->gpr[rA];
	current_core->gpr[rD] = ~a + imm + 1;
	// update XER
	if (ppc_carry_3(~a, imm, 1)) {
		current_core->xer |= XER_CA;
	} else {
		current_core->xer &= ~XER_CA;
	}

}

/*
 *	subfmex		Subtract From Minus One Extended
 *	.670
 */
void ppc_opc_subfmex()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	PPC_OPC_ASSERT(rB == 0);
	uint32 a = current_core->gpr[rA];
	uint32 ca = ((current_core->xer & XER_CA) ? 1 : 0);
	current_core->gpr[rD] = ~a + ca + 0xffffffff;
	// update XER
	if ((a != 0xffffffff) || ca) {
		current_core->xer |= XER_CA;
	} else {
		current_core->xer &= ~XER_CA;
	}
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
}

/*
 *	subfmeox	Subtract From Minus One Extended with Overflow
 *	.670
 */
void ppc_opc_subfmeox()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	PPC_OPC_ASSERT(rB == 0);
	uint32 a = current_core->gpr[rA];
	uint32 ca = ((current_core->xer & XER_CA) ? 1 : 0);
	current_core->gpr[rD] = ~a + ca + 0xffffffff;
	// update XER
	if ((a != 0xffffffff) || ca) {
		current_core->xer |= XER_CA;
	} else {
		current_core->xer &= ~XER_CA;
	}
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
	// update XER flags
	PPC_ALU_ERR("subfmeox unimplemented\n");
}

/*
 *	subfzex		Subtract From Zero Extended
 *	.671
 */
void ppc_opc_subfzex()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	PPC_OPC_ASSERT(rB == 0);
	uint32 a = current_core->gpr[rA];
	uint32 ca = ((current_core->xer & XER_CA) ? 1 : 0);
	current_core->gpr[rD] = ~a + ca;
	if (!a && ca) {
		current_core->xer |= XER_CA;
	} else {
		current_core->xer &= ~XER_CA;
	}
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
}

/*
 *	subfzeox	Subtract From Zero Extended with Overflow
 *	.671
 */
void ppc_opc_subfzeox()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	PPC_OPC_ASSERT(rB == 0);
	uint32 a = current_core->gpr[rA];
	uint32 ca = ((current_core->xer & XER_CA) ? 1 : 0);
	current_core->gpr[rD] = ~a + ca;
	if (!a && ca) {
		current_core->xer |= XER_CA;
	} else {
		current_core->xer &= ~XER_CA;
	}
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rD]);
	}
	// update XER flags
	PPC_ALU_ERR("subfzeox unimplemented\n");
}

/*
 *	xorx		XOR
 *	.680
 */
void ppc_opc_xorx()
{
	int rS, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_X(current_core->current_opc, rS, rA, rB);
	current_core->gpr[rA] = current_core->gpr[rS] ^ current_core->gpr[rB];
	if (current_core->current_opc & PPC_OPC_Rc) {
		// update cr0 flags
		ppc_update_cr0(current_core->gpr[rA]);
	}
}

/*
 *	xori		XOR Immediate
 *	.681
 */
void ppc_opc_xori()
{
	int rS, rA;
	e500_core_t *current_core = get_current_core();
	uint32 imm;
	PPC_OPC_TEMPL_D_UImm(current_core->current_opc, rS, rA, imm);
	current_core->gpr[rA] = current_core->gpr[rS] ^ imm;
}

/*
 *	xoris		XOR Immediate Shifted
 *	.682
 */
void ppc_opc_xoris()
{
	int rS, rA;
	e500_core_t *current_core = get_current_core();
	uint32 imm;
	PPC_OPC_TEMPL_D_Shift16(current_core->current_opc, rS, rA, imm);
	current_core->gpr[rA] = current_core->gpr[rS] ^ imm;
}

void ppc_opc_iseleq()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	//int crb = 2;
	if (current_core->cr & CR_CR0_EQ)
		current_core->gpr[rD] = current_core->gpr[rA];
	else
		current_core->gpr[rD] = current_core->gpr[rB];
#ifdef E500
	//fprintf(stderr,"In %s, not implemented in E500.\n", __FUNCTION__);
#else
	fprintf(stderr, "In %s, not implemented .\n", __FUNCTION);
#endif
}

void ppc_opc_iselgt()
{
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	int crb = ((current_core->current_opc) >> 6) & 0x1f;
	if (current_core->cr & CR_CR0_GT)
		current_core->gpr[rD] = current_core->gpr[rA];
	else
		current_core->gpr[rD] = current_core->gpr[rB];
#ifdef E500
	//fprintf(stderr,"In %s, crb=0x%x, not implemented in E500.\n", __FUNCTION__, crb);
#else
	fprintf(stderr, "In %s, not implemented .\n", __FUNCTION);
#endif
}

void ppc_opc_isel()
{
#ifdef E500
	int rD, rA, rB;
	e500_core_t *current_core = get_current_core();
	PPC_OPC_TEMPL_XO(current_core->current_opc, rD, rA, rB);
	int crb = ((current_core->current_opc) >> 6) & 0x1f;
	if (current_core->cr & (1 << (31 - crb)))
		current_core->gpr[rD] = current_core->gpr[rA];
	else
		current_core->gpr[rD] = current_core->gpr[rB];
	//fprintf(stderr,"In %s, crb=0x%x,not implemented in E500.\n", __FUNCTION__, crb);
#else
	fprintf(stderr, "In %s, not implemented .\n", __FUNCTION__);
#endif
}
