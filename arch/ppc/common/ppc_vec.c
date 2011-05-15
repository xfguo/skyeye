/*
 *	PearPC
 *	ppc_vec.cc
 *
 *	Copyright (C) 2004 Daniel Foesch (dfoesch@cs.nsmu.edu)
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

#include <math.h>

/*
 *	FIXME: put somewhere appropriate
 */
#ifndef HAS_LOG2
#define log2(x) log(x)/log(2)
#endif /* HAS_LOG2 */

#ifndef HAS_EXP2
#define exp2(x)	pow(2, x)
#endif /* HAS_EXP2 */

//#include "debug/tracers.h"
#include "ppc_cpu.h"
#include "ppc_dec.h"
#include "ppc_fpu.h"
#include "ppc_vec.h"

#define	SIGN32 0x80000000

/*	PACK_PIXEL	Packs a uint32 pixel to uint16 pixel
 *	v.219
 */
static inline uint16 PACK_PIXEL(uint32 clr)
{
	return (((clr & 0x000000f8) >> 3) |
		((clr & 0x0000f800) >> 6) | ((clr & 0x01f80000) >> 9));
}

/*	UNPACK_PIXEL	Unpacks a uint16 pixel to uint32 pixel
 *	v.276 & v.279
 */
static inline uint32 UNPACK_PIXEL(uint16 clr)
{
	return (((uint32) (clr & 0x001f)) |
		((uint32) (clr & 0x03E0) << 3) |
		((uint32) (clr & 0x7c00) << 6) |
		(((clr) & 0x8000) ? 0xff000000 : 0));
}

static inline uint8 SATURATE_UB(uint16 val)
{
	e500_core_t *current_core = get_current_core();
	if (val & 0xff00) {
		current_core->vscr |= VSCR_SAT;
		return 0xff;
	}
	return val;
}

static inline uint8 SATURATE_0B(uint16 val)
{
	e500_core_t *current_core = get_current_core();
	if (val & 0xff00) {
		current_core->vscr |= VSCR_SAT;
		return 0;
	}
	return val;
}

static inline uint16 SATURATE_UH(uint32 val)
{
	e500_core_t *current_core = get_current_core();
	if (val & 0xffff0000) {
		current_core->vscr |= VSCR_SAT;
		return 0xffff;
	}
	return val;
}

static inline uint16 SATURATE_0H(uint32 val)
{
	e500_core_t *current_core = get_current_core();
	if (val & 0xffff0000) {
		current_core->vscr |= VSCR_SAT;
		return 0;
	}
	return val;
}

static inline sint8 SATURATE_SB(sint16 val)
{
	e500_core_t *current_core = get_current_core();
	if (val > 127) {	// 0x7F
		current_core->vscr |= VSCR_SAT;
		return 127;
	} else if (val < -128) {	// 0x80
		current_core->vscr |= VSCR_SAT;
		return -128;
	}
	return val;
}

static inline uint8 SATURATE_USB(sint16 val)
{
	e500_core_t *current_core = get_current_core();
	if (val > 0xff) {
		current_core->vscr |= VSCR_SAT;
		return 0xff;
	} else if (val < 0) {
		current_core->vscr |= VSCR_SAT;
		return 0;
	}
	return (uint8) val;
}

static inline sint16 SATURATE_SH(sint32 val)
{
	e500_core_t *current_core = get_current_core();
	if (val > 32767) {	// 0x7fff
		current_core->vscr |= VSCR_SAT;
		return 32767;
	} else if (val < -32768) {	// 0x8000
		current_core->vscr |= VSCR_SAT;
		return -32768;
	}
	return val;
}

static inline uint16 SATURATE_USH(sint32 val)
{
	e500_core_t *current_core = get_current_core();
	if (val > 0xffff) {
		current_core->vscr |= VSCR_SAT;
		return 0xffff;
	} else if (val < 0) {
		current_core->vscr |= VSCR_SAT;
		return 0;
	}
	return (uint16) val;
}

static inline sint32 SATURATE_UW(sint64 val)
{
	e500_core_t *current_core = get_current_core();
	if (val > 0xffffffffLL) {
		current_core->vscr |= VSCR_SAT;
		return 0xffffffffLL;
	}
	return val;
}

static inline sint32 SATURATE_SW(sint64 val)
{
	e500_core_t *current_core = get_current_core();
	if (val > 2147483647LL) {	// 0x7fffffff
		current_core->vscr |= VSCR_SAT;
		return 2147483647LL;
	} else if (val < -2147483648LL) {	// 0x80000000
		current_core->vscr |= VSCR_SAT;
		return -2147483648LL;
	}
	return val;
}

/*	vperm		Vector Permutation
 *	v.218
 */
void ppc_opc_vperm()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG_COMMON;
	int vrD, vrA, vrB, vrC;
	int sel;
	Vector_t r;
	PPC_OPC_TEMPL_A(current_core->current_opc, vrD, vrA, vrB, vrC);
	int i;
	for (i = 0; i < 16; i++) {
		sel = current_core->vr[vrC].b[i];
		if (sel & 0x10)
			r.b[i] = VECT_B(current_core->vr[vrB], sel & 0xf);
		else
			r.b[i] = VECT_B(current_core->vr[vrA], sel & 0xf);
	}

	current_core->vr[vrD] = r;
}

/*	vsel		Vector Select
 *	v.238
 */
void ppc_opc_vsel()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB, vrC;
	uint64 mask, val;
	PPC_OPC_TEMPL_A(current_core->current_opc, vrD, vrA, vrB, vrC);

	mask = current_core->vr[vrC].d[0];
	val = current_core->vr[vrB].d[0] & mask;
	val |= current_core->vr[vrA].d[0] & ~mask;
	current_core->vr[vrD].d[0] = val;

	mask = current_core->vr[vrC].d[1];
	val = current_core->vr[vrB].d[1] & mask;
	val |= current_core->vr[vrA].d[1] & ~mask;
	current_core->vr[vrD].d[1] = val;
}

/*	vsrb		Vector Shift Right Byte
 *	v.256
 */
void ppc_opc_vsrb()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 16; i++) {
		current_core->vr[vrD].b[i] =
		    current_core->vr[vrA].b[i] >> (current_core->vr[vrB].
						   b[i] & 0x7);
	}
}

/*	vsrh		Vector Shift Right Half Word
 *	v.257
 */
void ppc_opc_vsrh()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 8; i++) {
		current_core->vr[vrD].h[i] =
		    current_core->vr[vrA].h[i] >> (current_core->vr[vrB].
						   h[i] & 0xf);
	}
}

/*	vsrw		Vector Shift Right Word
 *	v.259
 */
void ppc_opc_vsrw()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		current_core->vr[vrD].w[i] =
		    current_core->vr[vrA].w[i] >> (current_core->vr[vrB].
						   w[i] & 0x1f);
	}
}

/*	vsrab		Vector Shift Right Arithmetic Byte
 *	v.253
 */
void ppc_opc_vsrab()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 16; i++) {
		current_core->vr[vrD].sb[i] =
		    current_core->vr[vrA].sb[i] >> (current_core->vr[vrB].
						    b[i] & 0x7);
	}
}

/*	vsrah		Vector Shift Right Arithmetic Half Word
 *	v.254
 */
void ppc_opc_vsrah()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 8; i++) {
		current_core->vr[vrD].sh[i] =
		    current_core->vr[vrA].sh[i] >> (current_core->vr[vrB].
						    h[i] & 0xf);
	}
}

/*	vsraw		Vector Shift Right Arithmetic Word
 *	v.255
 */
void ppc_opc_vsraw()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		current_core->vr[vrD].sw[i] =
		    current_core->vr[vrA].sw[i] >> (current_core->vr[vrB].
						    w[i] & 0x1f);
	}
}

/*	vslb		Vector Shift Left Byte
 *	v.240
 */
void ppc_opc_vslb()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 16; i++) {
		current_core->vr[vrD].b[i] =
		    current_core->vr[vrA].b[i] << (current_core->vr[vrB].
						   b[i] & 0x7);
	}
}

/*	vslh		Vector Shift Left Half Word
 *	v.242
 */
void ppc_opc_vslh()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 8; i++) {
		current_core->vr[vrD].h[i] =
		    current_core->vr[vrA].h[i] << (current_core->vr[vrB].
						   h[i] & 0xf);
	}
}

/*	vslw		Vector Shift Left Word
 *	v.244
 */
void ppc_opc_vslw()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		current_core->vr[vrD].w[i] =
		    current_core->vr[vrA].w[i] << (current_core->vr[vrB].
						   w[i] & 0x1f);
	}
}

/*	vsr		Vector Shift Right
 *	v.251
 */
void ppc_opc_vsr()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	int shift;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	/* Specs say that the low-order 3 bits of all byte elements in vB
	 *   must be the same, or the result is undefined.  So we can just
	 *   use the same low-order 3 bits for all of our shifts.
	 */
	shift = current_core->vr[vrB].w[0] & 0x7;

	r.d[0] = current_core->vr[vrA].d[0] >> shift;
	r.d[1] = current_core->vr[vrA].d[1] >> shift;

	VECT_D(r, 1) |= VECT_D(current_core->vr[vrA], 0) << (64 - shift);

	current_core->vr[vrD] = r;
}

/*	vsro		Vector Shift Right Octet
 *	v.258
 */
void ppc_opc_vsro()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	int shift, i;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	shift = (current_core->vr[vrB].w[0] >> 3) & 0xf;
#if HOST_ENDIANESS == HOST_ENDIANESS_LE
	for (i = 0; i < (16 - shift); i++) {
		r.b[i] = current_core->vr[vrA].b[i + shift];
	}

	for (; i < 16; i++) {
		r.b[i] = 0;
	}
#elif HOST_ENDIANESS == HOST_ENDIANESS_BE
	for (i = 0; i < shift; i++) {
		r.b[i] = 0;
	}

	for (; i < 16; i++) {
		r.b[i] = current_core->vr[vrA].b[i - shift];
	}
#else
#error Endianess not supported!
#endif

	current_core->vr[vrD] = r;
}

/*	vsl		Vector Shift Left
 *	v.239
 */
void ppc_opc_vsl()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	int shift;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	/* Specs say that the low-order 3 bits of all byte elements in vB
	 *   must be the same, or the result is undefined.  So we can just
	 *   use the same low-order 3 bits for all of our shifts.
	 */
	shift = current_core->vr[vrB].w[0] & 0x7;

	r.d[0] = current_core->vr[vrA].d[0] << shift;
	r.d[1] = current_core->vr[vrA].d[1] << shift;

	VECT_D(r, 0) |= VECT_D(current_core->vr[vrA], 1) >> (64 - shift);

	current_core->vr[vrD] = r;
}

/*	vslo		Vector Shift Left Octet
 *	v.243
 */
void ppc_opc_vslo()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	int shift, i;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	shift = (current_core->vr[vrB].w[0] >> 3) & 0xf;
#if HOST_ENDIANESS == HOST_ENDIANESS_LE
	for (i = 0; i < shift; i++) {
		r.b[i] = 0;
	}

	for (; i < 16; i++) {
		r.b[i] = current_core->vr[vrA].b[i - shift];
	}
#elif HOST_ENDIANESS == HOST_ENDIANESS_BE
	for (i = 0; i < (16 - shift); i++) {
		r.b[i] = current_core->vr[vrA].b[i + shift];
	}

	for (; i < 16; i++) {
		r.b[i] = 0;
	}
#else
#error Endianess not supported!
#endif

	current_core->vr[vrD] = r;
}

/*	vsldoi		Vector Shift Left Double by Octet Immediate
 *	v.241
 */
void ppc_opc_vsldoi()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG_COMMON;
	int vrD, vrA, vrB, shift, ashift;
	int i;
	Vector_t r;
	PPC_OPC_TEMPL_A(current_core->current_opc, vrD, vrA, vrB, shift);

	shift &= 0xf;
	ashift = 16 - shift;

#if HOST_ENDIANESS == HOST_ENDIANESS_LE
	for (i = 0; i < shift; i++) {
		r.b[i] = current_core->vr[vrB].b[i + ashift];
	}

	for (; i < 16; i++) {
		r.b[i] = current_core->vr[vrA].b[i - shift];
	}
#elif HOST_ENDIANESS == HOST_ENDIANESS_BE
	for (i = 0; i < ashift; i++) {
		r.b[i] = current_core->vr[vrA].b[i + shift];
	}

	for (; i < 16; i++) {
		r.b[i] = current_core->vr[vrB].b[i - ashift];
	}
#else
#error Endianess not supported!
#endif

	current_core->vr[vrD] = r;
}

/*	vrlb		Vector Rotate Left Byte
 *	v.234
 */
void ppc_opc_vrlb()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB, shift;
	Vector_t r;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 16; i++) {
		shift = (current_core->vr[vrB].b[i] & 0x7);

		r.b[i] = current_core->vr[vrA].b[i] << shift;
		r.b[i] |= current_core->vr[vrA].b[i] >> (8 - shift);
	}

	current_core->vr[vrD] = r;
}

/*	vrlh		Vector Rotate Left Half Word
 *	v.235
 */
void ppc_opc_vrlh()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB, shift;
	Vector_t r;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 8; i++) {
		shift = (current_core->vr[vrB].h[i] & 0xf);

		r.h[i] = current_core->vr[vrA].h[i] << shift;
		r.h[i] |= current_core->vr[vrA].h[i] >> (16 - shift);
	}

	current_core->vr[vrD] = r;
}

/*	vrlw		Vector Rotate Left Word
 *	v.236
 */
void ppc_opc_vrlw()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB, shift;
	Vector_t r;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		shift = (current_core->vr[vrB].w[i] & 0x1F);

		r.w[i] = current_core->vr[vrA].w[i] << shift;
		r.w[i] |= current_core->vr[vrA].w[i] >> (32 - shift);
	}

	current_core->vr[vrD] = r;
}

/* With the merges, I just don't see any point in risking that a compiler
 *   might generate actual alu code to calculate anything when it's
 *   compile-time known.  Plus, it's easier to validate it like this.
 */

/*	vmrghb		Vector Merge High Byte
 *	v.195
 */
void ppc_opc_vmrghb()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	VECT_B(r, 0) = VECT_B(current_core->vr[vrA], 0);
	VECT_B(r, 1) = VECT_B(current_core->vr[vrB], 0);
	VECT_B(r, 2) = VECT_B(current_core->vr[vrA], 1);
	VECT_B(r, 3) = VECT_B(current_core->vr[vrB], 1);
	VECT_B(r, 4) = VECT_B(current_core->vr[vrA], 2);
	VECT_B(r, 5) = VECT_B(current_core->vr[vrB], 2);
	VECT_B(r, 6) = VECT_B(current_core->vr[vrA], 3);
	VECT_B(r, 7) = VECT_B(current_core->vr[vrB], 3);
	VECT_B(r, 8) = VECT_B(current_core->vr[vrA], 4);
	VECT_B(r, 9) = VECT_B(current_core->vr[vrB], 4);
	VECT_B(r, 10) = VECT_B(current_core->vr[vrA], 5);
	VECT_B(r, 11) = VECT_B(current_core->vr[vrB], 5);
	VECT_B(r, 12) = VECT_B(current_core->vr[vrA], 6);
	VECT_B(r, 13) = VECT_B(current_core->vr[vrB], 6);
	VECT_B(r, 14) = VECT_B(current_core->vr[vrA], 7);
	VECT_B(r, 15) = VECT_B(current_core->vr[vrB], 7);

	current_core->vr[vrD] = r;
}

/*	vmrghh		Vector Merge High Half Word
 *	v.196
 */
void ppc_opc_vmrghh()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	VECT_H(r, 0) = VECT_H(current_core->vr[vrA], 0);
	VECT_H(r, 1) = VECT_H(current_core->vr[vrB], 0);
	VECT_H(r, 2) = VECT_H(current_core->vr[vrA], 1);
	VECT_H(r, 3) = VECT_H(current_core->vr[vrB], 1);
	VECT_H(r, 4) = VECT_H(current_core->vr[vrA], 2);
	VECT_H(r, 5) = VECT_H(current_core->vr[vrB], 2);
	VECT_H(r, 6) = VECT_H(current_core->vr[vrA], 3);
	VECT_H(r, 7) = VECT_H(current_core->vr[vrB], 3);

	current_core->vr[vrD] = r;
}

/*	vmrghw		Vector Merge High Word
 *	v.197
 */
void ppc_opc_vmrghw()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	VECT_W(r, 0) = VECT_W(current_core->vr[vrA], 0);
	VECT_W(r, 1) = VECT_W(current_core->vr[vrB], 0);
	VECT_W(r, 2) = VECT_W(current_core->vr[vrA], 1);
	VECT_W(r, 3) = VECT_W(current_core->vr[vrB], 1);

	current_core->vr[vrD] = r;
}

/*	vmrglb		Vector Merge Low Byte
 *	v.198
 */
void ppc_opc_vmrglb()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	VECT_B(r, 0) = VECT_B(current_core->vr[vrA], 8);
	VECT_B(r, 1) = VECT_B(current_core->vr[vrB], 8);
	VECT_B(r, 2) = VECT_B(current_core->vr[vrA], 9);
	VECT_B(r, 3) = VECT_B(current_core->vr[vrB], 9);
	VECT_B(r, 4) = VECT_B(current_core->vr[vrA], 10);
	VECT_B(r, 5) = VECT_B(current_core->vr[vrB], 10);
	VECT_B(r, 6) = VECT_B(current_core->vr[vrA], 11);
	VECT_B(r, 7) = VECT_B(current_core->vr[vrB], 11);
	VECT_B(r, 8) = VECT_B(current_core->vr[vrA], 12);
	VECT_B(r, 9) = VECT_B(current_core->vr[vrB], 12);
	VECT_B(r, 10) = VECT_B(current_core->vr[vrA], 13);
	VECT_B(r, 11) = VECT_B(current_core->vr[vrB], 13);
	VECT_B(r, 12) = VECT_B(current_core->vr[vrA], 14);
	VECT_B(r, 13) = VECT_B(current_core->vr[vrB], 14);
	VECT_B(r, 14) = VECT_B(current_core->vr[vrA], 15);
	VECT_B(r, 15) = VECT_B(current_core->vr[vrB], 15);

	current_core->vr[vrD] = r;
}

/*	vmrglh		Vector Merge Low Half Word
 *	v.199
 */
void ppc_opc_vmrglh()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	VECT_H(r, 0) = VECT_H(current_core->vr[vrA], 4);
	VECT_H(r, 1) = VECT_H(current_core->vr[vrB], 4);
	VECT_H(r, 2) = VECT_H(current_core->vr[vrA], 5);
	VECT_H(r, 3) = VECT_H(current_core->vr[vrB], 5);
	VECT_H(r, 4) = VECT_H(current_core->vr[vrA], 6);
	VECT_H(r, 5) = VECT_H(current_core->vr[vrB], 6);
	VECT_H(r, 6) = VECT_H(current_core->vr[vrA], 7);
	VECT_H(r, 7) = VECT_H(current_core->vr[vrB], 7);

	current_core->vr[vrD] = r;
}

/*	vmrglw		Vector Merge Low Word
 *	v.200
 */
void ppc_opc_vmrglw()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	VECT_W(r, 0) = VECT_W(current_core->vr[vrA], 2);
	VECT_W(r, 1) = VECT_W(current_core->vr[vrB], 2);
	VECT_W(r, 2) = VECT_W(current_core->vr[vrA], 3);
	VECT_W(r, 3) = VECT_W(current_core->vr[vrB], 3);

	current_core->vr[vrD] = r;
}

/*	vspltb		Vector Splat Byte
 *	v.245
 */
void ppc_opc_vspltb()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrB;
	uint32 uimm;
	uint64 val;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, uimm, vrB);

	/* The documentation doesn't stipulate what a value higher than 0xf
	 *   will do.  Thus, this is by default an undefined value.  We
	 *   are thus doing this the fastest way that won't crash us.
	 */
	val = VECT_B(current_core->vr[vrB], uimm & 0xf);
	val |= (val << 8);
	val |= (val << 16);
	val |= (val << 32);

	current_core->vr[vrD].d[0] = val;
	current_core->vr[vrD].d[1] = val;
}

/*	vsplth		Vector Splat Half Word
 *	v.246
 */
void ppc_opc_vsplth()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrB;
	uint32 uimm;
	uint64 val;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, uimm, vrB);

	/* The documentation doesn't stipulate what a value higher than 0x7
	 *   will do.  Thus, this is by default an undefined value.  We
	 *   are thus doing this the fastest way that won't crash us.
	 */
	val = VECT_H(current_core->vr[vrB], uimm & 0x7);
	val |= (val << 16);
	val |= (val << 32);

	current_core->vr[vrD].d[0] = val;
	current_core->vr[vrD].d[1] = val;
}

/*	vspltw		Vector Splat Word
 *	v.250
 */
void ppc_opc_vspltw()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrB;
	uint32 uimm;
	uint64 val;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, uimm, vrB);

	/* The documentation doesn't stipulate what a value higher than 0x3
	 *   will do.  Thus, this is by default an undefined value.  We
	 *   are thus doing this the fastest way that won't crash us.
	 */
	val = VECT_W(current_core->vr[vrB], uimm & 0x3);
	val |= (val << 32);

	current_core->vr[vrD].d[0] = val;
	current_core->vr[vrD].d[1] = val;
}

/*	vspltisb	Vector Splat Immediate Signed Byte
 *	v.247
 */
void ppc_opc_vspltisb()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG_COMMON;
	int vrD, vrB;
	uint32 simm;
	uint64 val;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, simm, vrB);
	PPC_OPC_ASSERT(vrB == 0);

	val = (simm & 0x10) ? (simm | 0xE0) : simm;
	val |= (val << 8);
	val |= (val << 16);
	val |= (val << 32);

	current_core->vr[vrD].d[0] = val;
	current_core->vr[vrD].d[1] = val;
}

/*	vspltish	Vector Splat Immediate Signed Half Word
 *	v.248
 */
void ppc_opc_vspltish()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG_COMMON;
	int vrD, vrB;
	uint32 simm;
	uint64 val;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, simm, vrB);
	PPC_OPC_ASSERT(vrB == 0);

	val = (simm & 0x10) ? (simm | 0xFFE0) : simm;
	val |= (val << 16);
	val |= (val << 32);

	current_core->vr[vrD].d[0] = val;
	current_core->vr[vrD].d[1] = val;
}

/*	vspltisw	Vector Splat Immediate Signed Word
 *	v.249
 */
void ppc_opc_vspltisw()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG_COMMON;
	int vrD, vrB;
	uint32 simm;
	uint64 val;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, simm, vrB);
	PPC_OPC_ASSERT(vrB == 0);

	val = (simm & 0x10) ? (simm | 0xFFFFFFE0) : simm;
	val |= (val << 32);

	current_core->vr[vrD].d[0] = val;
	current_core->vr[vrD].d[1] = val;
}

/*	mfvscr		Move from Vector Status and Control Register
 *	v.129
 */
void ppc_opc_mfvscr()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG_COMMON;
	int vrD, vrA, vrB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	PPC_OPC_ASSERT(vrA == 0);
	PPC_OPC_ASSERT(vrB == 0);

	VECT_W(current_core->vr[vrD], 3) = current_core->vscr;
	VECT_W(current_core->vr[vrD], 2) = 0;
	VECT_D(current_core->vr[vrD], 0) = 0;
}

/*	mtvscr		Move to Vector Status and Control Register
 *	v.130
 */
void ppc_opc_mtvscr()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG_COMMON;
	int vrD, vrA, vrB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	PPC_OPC_ASSERT(vrA == 0);
	PPC_OPC_ASSERT(vrD == 0);

	current_core->vscr = VECT_W(current_core->vr[vrB], 3);
}

/*	vpkuhum		Vector Pack Unsigned Half Word Unsigned Modulo
 *	v.224
 */
void ppc_opc_vpkuhum()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	VECT_B(r, 0) = VECT_B(current_core->vr[vrA], 1);
	VECT_B(r, 1) = VECT_B(current_core->vr[vrA], 3);
	VECT_B(r, 2) = VECT_B(current_core->vr[vrA], 5);
	VECT_B(r, 3) = VECT_B(current_core->vr[vrA], 7);
	VECT_B(r, 4) = VECT_B(current_core->vr[vrA], 9);
	VECT_B(r, 5) = VECT_B(current_core->vr[vrA], 11);
	VECT_B(r, 6) = VECT_B(current_core->vr[vrA], 13);
	VECT_B(r, 7) = VECT_B(current_core->vr[vrA], 15);

	VECT_B(r, 8) = VECT_B(current_core->vr[vrB], 1);
	VECT_B(r, 9) = VECT_B(current_core->vr[vrB], 3);
	VECT_B(r, 10) = VECT_B(current_core->vr[vrB], 5);
	VECT_B(r, 11) = VECT_B(current_core->vr[vrB], 7);
	VECT_B(r, 12) = VECT_B(current_core->vr[vrB], 9);
	VECT_B(r, 13) = VECT_B(current_core->vr[vrB], 11);
	VECT_B(r, 14) = VECT_B(current_core->vr[vrB], 13);
	VECT_B(r, 15) = VECT_B(current_core->vr[vrB], 15);

	current_core->vr[vrD] = r;
}

/*	vpkuwum		Vector Pack Unsigned Word Unsigned Modulo
 *	v.226
 */
void ppc_opc_vpkuwum()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	VECT_H(r, 0) = VECT_H(current_core->vr[vrA], 1);
	VECT_H(r, 1) = VECT_H(current_core->vr[vrA], 3);
	VECT_H(r, 2) = VECT_H(current_core->vr[vrA], 5);
	VECT_H(r, 3) = VECT_H(current_core->vr[vrA], 7);

	VECT_H(r, 4) = VECT_H(current_core->vr[vrB], 1);
	VECT_H(r, 5) = VECT_H(current_core->vr[vrB], 3);
	VECT_H(r, 6) = VECT_H(current_core->vr[vrB], 5);
	VECT_H(r, 7) = VECT_H(current_core->vr[vrB], 7);

	current_core->vr[vrD] = r;
}

/*	vpkpx		Vector Pack Pixel32
 *	v.219
 */
void ppc_opc_vpkpx()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	VECT_H(r, 0) = PACK_PIXEL(VECT_W(current_core->vr[vrA], 0));
	VECT_H(r, 1) = PACK_PIXEL(VECT_W(current_core->vr[vrA], 1));
	VECT_H(r, 2) = PACK_PIXEL(VECT_W(current_core->vr[vrA], 2));
	VECT_H(r, 3) = PACK_PIXEL(VECT_W(current_core->vr[vrA], 3));

	VECT_H(r, 4) = PACK_PIXEL(VECT_W(current_core->vr[vrB], 0));
	VECT_H(r, 5) = PACK_PIXEL(VECT_W(current_core->vr[vrB], 1));
	VECT_H(r, 6) = PACK_PIXEL(VECT_W(current_core->vr[vrB], 2));
	VECT_H(r, 7) = PACK_PIXEL(VECT_W(current_core->vr[vrB], 3));

	current_core->vr[vrD] = r;
}

/*	vpkuhus		Vector Pack Unsigned Half Word Unsigned Saturate
 *	v.225
 */
void ppc_opc_vpkuhus()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	VECT_B(r, 0) = SATURATE_UB(VECT_H(current_core->vr[vrA], 0));
	VECT_B(r, 1) = SATURATE_UB(VECT_H(current_core->vr[vrA], 1));
	VECT_B(r, 2) = SATURATE_UB(VECT_H(current_core->vr[vrA], 2));
	VECT_B(r, 3) = SATURATE_UB(VECT_H(current_core->vr[vrA], 3));
	VECT_B(r, 4) = SATURATE_UB(VECT_H(current_core->vr[vrA], 4));
	VECT_B(r, 5) = SATURATE_UB(VECT_H(current_core->vr[vrA], 5));
	VECT_B(r, 6) = SATURATE_UB(VECT_H(current_core->vr[vrA], 6));
	VECT_B(r, 7) = SATURATE_UB(VECT_H(current_core->vr[vrA], 7));

	VECT_B(r, 8) = SATURATE_UB(VECT_H(current_core->vr[vrB], 0));
	VECT_B(r, 9) = SATURATE_UB(VECT_H(current_core->vr[vrB], 1));
	VECT_B(r, 10) = SATURATE_UB(VECT_H(current_core->vr[vrB], 2));
	VECT_B(r, 11) = SATURATE_UB(VECT_H(current_core->vr[vrB], 3));
	VECT_B(r, 12) = SATURATE_UB(VECT_H(current_core->vr[vrB], 4));
	VECT_B(r, 13) = SATURATE_UB(VECT_H(current_core->vr[vrB], 5));
	VECT_B(r, 14) = SATURATE_UB(VECT_H(current_core->vr[vrB], 6));
	VECT_B(r, 15) = SATURATE_UB(VECT_H(current_core->vr[vrB], 7));

	current_core->vr[vrD] = r;
}

/*	vpkshss		Vector Pack Signed Half Word Signed Saturate
 *	v.220
 */
void ppc_opc_vpkshss()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	VECT_B(r, 0) = SATURATE_SB(VECT_H(current_core->vr[vrA], 0));
	VECT_B(r, 1) = SATURATE_SB(VECT_H(current_core->vr[vrA], 1));
	VECT_B(r, 2) = SATURATE_SB(VECT_H(current_core->vr[vrA], 2));
	VECT_B(r, 3) = SATURATE_SB(VECT_H(current_core->vr[vrA], 3));
	VECT_B(r, 4) = SATURATE_SB(VECT_H(current_core->vr[vrA], 4));
	VECT_B(r, 5) = SATURATE_SB(VECT_H(current_core->vr[vrA], 5));
	VECT_B(r, 6) = SATURATE_SB(VECT_H(current_core->vr[vrA], 6));
	VECT_B(r, 7) = SATURATE_SB(VECT_H(current_core->vr[vrA], 7));

	VECT_B(r, 8) = SATURATE_SB(VECT_H(current_core->vr[vrB], 0));
	VECT_B(r, 9) = SATURATE_SB(VECT_H(current_core->vr[vrB], 1));
	VECT_B(r, 10) = SATURATE_SB(VECT_H(current_core->vr[vrB], 2));
	VECT_B(r, 11) = SATURATE_SB(VECT_H(current_core->vr[vrB], 3));
	VECT_B(r, 12) = SATURATE_SB(VECT_H(current_core->vr[vrB], 4));
	VECT_B(r, 13) = SATURATE_SB(VECT_H(current_core->vr[vrB], 5));
	VECT_B(r, 14) = SATURATE_SB(VECT_H(current_core->vr[vrB], 6));
	VECT_B(r, 15) = SATURATE_SB(VECT_H(current_core->vr[vrB], 7));

	current_core->vr[vrD] = r;
}

/*	vpkuwus		Vector Pack Unsigned Word Unsigned Saturate
 *	v.227
 */
void ppc_opc_vpkuwus()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	VECT_H(r, 0) = SATURATE_UH(VECT_W(current_core->vr[vrA], 0));
	VECT_H(r, 1) = SATURATE_UH(VECT_W(current_core->vr[vrA], 1));
	VECT_H(r, 2) = SATURATE_UH(VECT_W(current_core->vr[vrA], 2));
	VECT_H(r, 3) = SATURATE_UH(VECT_W(current_core->vr[vrA], 3));

	VECT_H(r, 4) = SATURATE_UH(VECT_W(current_core->vr[vrB], 0));
	VECT_H(r, 5) = SATURATE_UH(VECT_W(current_core->vr[vrB], 1));
	VECT_H(r, 6) = SATURATE_UH(VECT_W(current_core->vr[vrB], 2));
	VECT_H(r, 7) = SATURATE_UH(VECT_W(current_core->vr[vrB], 3));

	current_core->vr[vrD] = r;
}

/*	vpkswss		Vector Pack Signed Word Signed Saturate
 *	v.222
 */
void ppc_opc_vpkswss()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	VECT_H(r, 0) = SATURATE_SH(VECT_W(current_core->vr[vrA], 0));
	VECT_H(r, 1) = SATURATE_SH(VECT_W(current_core->vr[vrA], 1));
	VECT_H(r, 2) = SATURATE_SH(VECT_W(current_core->vr[vrA], 2));
	VECT_H(r, 3) = SATURATE_SH(VECT_W(current_core->vr[vrA], 3));

	VECT_H(r, 4) = SATURATE_SH(VECT_W(current_core->vr[vrB], 0));
	VECT_H(r, 5) = SATURATE_SH(VECT_W(current_core->vr[vrB], 1));
	VECT_H(r, 6) = SATURATE_SH(VECT_W(current_core->vr[vrB], 2));
	VECT_H(r, 7) = SATURATE_SH(VECT_W(current_core->vr[vrB], 3));

	current_core->vr[vrD] = r;
}

/*	vpkshus		Vector Pack Signed Half Word Unsigned Saturate
 *	v.221
 */
void ppc_opc_vpkshus()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	VECT_B(r, 0) = SATURATE_USB(VECT_H(current_core->vr[vrA], 0));
	VECT_B(r, 1) = SATURATE_USB(VECT_H(current_core->vr[vrA], 1));
	VECT_B(r, 2) = SATURATE_USB(VECT_H(current_core->vr[vrA], 2));
	VECT_B(r, 3) = SATURATE_USB(VECT_H(current_core->vr[vrA], 3));
	VECT_B(r, 4) = SATURATE_USB(VECT_H(current_core->vr[vrA], 4));
	VECT_B(r, 5) = SATURATE_USB(VECT_H(current_core->vr[vrA], 5));
	VECT_B(r, 6) = SATURATE_USB(VECT_H(current_core->vr[vrA], 6));
	VECT_B(r, 7) = SATURATE_USB(VECT_H(current_core->vr[vrA], 7));

	VECT_B(r, 8) = SATURATE_USB(VECT_H(current_core->vr[vrB], 0));
	VECT_B(r, 9) = SATURATE_USB(VECT_H(current_core->vr[vrB], 1));
	VECT_B(r, 10) = SATURATE_USB(VECT_H(current_core->vr[vrB], 2));
	VECT_B(r, 11) = SATURATE_USB(VECT_H(current_core->vr[vrB], 3));
	VECT_B(r, 12) = SATURATE_USB(VECT_H(current_core->vr[vrB], 4));
	VECT_B(r, 13) = SATURATE_USB(VECT_H(current_core->vr[vrB], 5));
	VECT_B(r, 14) = SATURATE_USB(VECT_H(current_core->vr[vrB], 6));
	VECT_B(r, 15) = SATURATE_USB(VECT_H(current_core->vr[vrB], 7));

	current_core->vr[vrD] = r;
}

/*	vpkswus		Vector Pack Signed Word Unsigned Saturate
 *	v.223
 */
void ppc_opc_vpkswus()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	VECT_H(r, 0) = SATURATE_USH(VECT_W(current_core->vr[vrA], 0));
	VECT_H(r, 1) = SATURATE_USH(VECT_W(current_core->vr[vrA], 1));
	VECT_H(r, 2) = SATURATE_USH(VECT_W(current_core->vr[vrA], 2));
	VECT_H(r, 3) = SATURATE_USH(VECT_W(current_core->vr[vrA], 3));

	VECT_H(r, 4) = SATURATE_USH(VECT_W(current_core->vr[vrB], 0));
	VECT_H(r, 5) = SATURATE_USH(VECT_W(current_core->vr[vrB], 1));
	VECT_H(r, 6) = SATURATE_USH(VECT_W(current_core->vr[vrB], 2));
	VECT_H(r, 7) = SATURATE_USH(VECT_W(current_core->vr[vrB], 3));

	current_core->vr[vrD] = r;
}

/*	vupkhsb		Vector Unpack High Signed Byte
 *	v.277
 */
void ppc_opc_vupkhsb()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	PPC_OPC_ASSERT(vrA == 0);

	VECT_SH(r, 0) = VECT_SB(current_core->vr[vrB], 0);
	VECT_SH(r, 1) = VECT_SB(current_core->vr[vrB], 1);
	VECT_SH(r, 2) = VECT_SB(current_core->vr[vrB], 2);
	VECT_SH(r, 3) = VECT_SB(current_core->vr[vrB], 3);
	VECT_SH(r, 4) = VECT_SB(current_core->vr[vrB], 4);
	VECT_SH(r, 5) = VECT_SB(current_core->vr[vrB], 5);
	VECT_SH(r, 6) = VECT_SB(current_core->vr[vrB], 6);
	VECT_SH(r, 7) = VECT_SB(current_core->vr[vrB], 7);

	current_core->vr[vrD] = r;
}

/*	vupkhpx		Vector Unpack High Pixel32
 *	v.279
 */
void ppc_opc_vupkhpx()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	PPC_OPC_ASSERT(vrA == 0);

	VECT_W(r, 0) = UNPACK_PIXEL(VECT_H(current_core->vr[vrB], 0));
	VECT_W(r, 1) = UNPACK_PIXEL(VECT_H(current_core->vr[vrB], 1));
	VECT_W(r, 2) = UNPACK_PIXEL(VECT_H(current_core->vr[vrB], 2));
	VECT_W(r, 3) = UNPACK_PIXEL(VECT_H(current_core->vr[vrB], 3));

	current_core->vr[vrD] = r;
}

/*	vupkhsh		Vector Unpack High Signed Half Word
 *	v.278
 */
void ppc_opc_vupkhsh()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	PPC_OPC_ASSERT(vrA == 0);

	VECT_SW(r, 0) = VECT_SH(current_core->vr[vrB], 0);
	VECT_SW(r, 1) = VECT_SH(current_core->vr[vrB], 1);
	VECT_SW(r, 2) = VECT_SH(current_core->vr[vrB], 2);
	VECT_SW(r, 3) = VECT_SH(current_core->vr[vrB], 3);

	current_core->vr[vrD] = r;
}

/*	vupklsb		Vector Unpack Low Signed Byte
 *	v.280
 */
void ppc_opc_vupklsb()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	PPC_OPC_ASSERT(vrA == 0);

	VECT_SH(r, 0) = VECT_SB(current_core->vr[vrB], 8);
	VECT_SH(r, 1) = VECT_SB(current_core->vr[vrB], 9);
	VECT_SH(r, 2) = VECT_SB(current_core->vr[vrB], 10);
	VECT_SH(r, 3) = VECT_SB(current_core->vr[vrB], 11);
	VECT_SH(r, 4) = VECT_SB(current_core->vr[vrB], 12);
	VECT_SH(r, 5) = VECT_SB(current_core->vr[vrB], 13);
	VECT_SH(r, 6) = VECT_SB(current_core->vr[vrB], 14);
	VECT_SH(r, 7) = VECT_SB(current_core->vr[vrB], 15);

	current_core->vr[vrD] = r;
}

/*	vupklpx		Vector Unpack Low Pixel32
 *	v.279
 */
void ppc_opc_vupklpx()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	PPC_OPC_ASSERT(vrA == 0);

	VECT_W(r, 0) = UNPACK_PIXEL(VECT_H(current_core->vr[vrB], 4));
	VECT_W(r, 1) = UNPACK_PIXEL(VECT_H(current_core->vr[vrB], 5));
	VECT_W(r, 2) = UNPACK_PIXEL(VECT_H(current_core->vr[vrB], 6));
	VECT_W(r, 3) = UNPACK_PIXEL(VECT_H(current_core->vr[vrB], 7));

	current_core->vr[vrD] = r;
}

/*	vupklsh		Vector Unpack Low Signed Half Word
 *	v.281
 */
void ppc_opc_vupklsh()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	Vector_t r;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	PPC_OPC_ASSERT(vrA == 0);

	VECT_SW(r, 0) = VECT_SH(current_core->vr[vrB], 4);
	VECT_SW(r, 1) = VECT_SH(current_core->vr[vrB], 5);
	VECT_SW(r, 2) = VECT_SH(current_core->vr[vrB], 6);
	VECT_SW(r, 3) = VECT_SH(current_core->vr[vrB], 7);

	current_core->vr[vrD] = r;
}

/*	vaddubm		Vector Add Unsigned Byte Modulo
 *	v.141
 */
void ppc_opc_vaddubm()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint8 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 16; i++) {
		res = current_core->vr[vrA].b[i] + current_core->vr[vrB].b[i];
		current_core->vr[vrD].b[i] = res;
	}
}

/*	vadduhm		Vector Add Unsigned Half Word Modulo
 *	v.143
 */
void ppc_opc_vadduhm()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint16 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 8; i++) {
		res = current_core->vr[vrA].h[i] + current_core->vr[vrB].h[i];
		current_core->vr[vrD].h[i] = res;
	}
}

/*	vadduwm		Vector Add Unsigned Word Modulo
 *	v.145
 */
void ppc_opc_vadduwm()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint32 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		res = current_core->vr[vrA].w[i] + current_core->vr[vrB].w[i];
		current_core->vr[vrD].w[i] = res;
	}
}

/*	vaddfp		Vector Add Float Point
 *	v.137
 */
void ppc_opc_vaddfp()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	float res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {	//FIXME: This might not comply with Java FP
		res = current_core->vr[vrA].f[i] + current_core->vr[vrB].f[i];
		current_core->vr[vrD].f[i] = res;
	}
}

/*	vaddcuw		Vector Add Carryout Unsigned Word
 *	v.136
 */
void ppc_opc_vaddcuw()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint32 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		res = current_core->vr[vrA].w[i] + current_core->vr[vrB].w[i];
		current_core->vr[vrD].w[i] =
		    (res < current_core->vr[vrA].w[i]) ? 1 : 0;
	}
}

/*	vaddubs		Vector Add Unsigned Byte Saturate
 *	v.142
 */
void ppc_opc_vaddubs()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint16 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 16; i++) {
		res =
		    (uint16) current_core->vr[vrA].b[i] +
		    (uint16) current_core->vr[vrB].b[i];
		current_core->vr[vrD].b[i] = SATURATE_UB(res);
	}
}

/*	vaddsbs		Vector Add Signed Byte Saturate
 *	v.138
 */
void ppc_opc_vaddsbs()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	sint16 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 16; i++) {
		res =
		    (sint16) current_core->vr[vrA].sb[i] +
		    (sint16) current_core->vr[vrB].sb[i];
		current_core->vr[vrD].b[i] = SATURATE_SB(res);
	}
}

/*	vadduhs		Vector Add Unsigned Half Word Saturate
 *	v.144
 */
void ppc_opc_vadduhs()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint32 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 8; i++) {
		res =
		    (uint32) current_core->vr[vrA].h[i] +
		    (uint32) current_core->vr[vrB].h[i];
		current_core->vr[vrD].h[i] = SATURATE_UH(res);
	}
}

/*	vaddshs		Vector Add Signed Half Word Saturate
 *	v.139
 */
void ppc_opc_vaddshs()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	sint32 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 8; i++) {
		res =
		    (sint32) current_core->vr[vrA].sh[i] +
		    (sint32) current_core->vr[vrB].sh[i];
		current_core->vr[vrD].h[i] = SATURATE_SH(res);
	}
}

/*	vadduws		Vector Add Unsigned Word Saturate
 *	v.146
 */
void ppc_opc_vadduws()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint32 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		res = current_core->vr[vrA].w[i] + current_core->vr[vrB].w[i];

		// We do this to prevent us from having to do 64-bit math
		if (res < current_core->vr[vrA].w[i]) {
			res = 0xFFFFFFFF;
			current_core->vscr |= VSCR_SAT;
		}

		/*      64-bit math             |       32-bit hack
		 *      ------------------------+-------------------------------------
		 *      add, addc       (a+b)   |       add             (a+b)
		 *      sub, subb       (r>ub)  |       sub             (r<a)
		 */

		current_core->vr[vrD].w[i] = res;
	}
}

/*	vaddsws		Vector Add Signed Word Saturate
 *	v.140
 */
void ppc_opc_vaddsws()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint32 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		res = current_core->vr[vrA].w[i] + current_core->vr[vrB].w[i];

		// We do this to prevent us from having to do 64-bit math
		if (((current_core->vr[vrA].w[i] ^ current_core->vr[vrB].
		      w[i]) & SIGN32) == 0) {
			// the signs of both operands are the same

			if (((res ^ current_core->vr[vrA].w[i]) & SIGN32) != 0) {
				// sign of result != sign of operands

				// if res is negative, should have been positive
				res = (res & SIGN32) ? (SIGN32 - 1) : SIGN32;
				current_core->vscr |= VSCR_SAT;
			}
		}

		/*      64-bit math             |       32-bit hack
		 *      ------------------------+-------------------------------------
		 *      add, addc       (a+b)   |       add             (a+b)
		 *      sub, subb       (r>ub)  |       xor, and        (sign == sign)
		 *      sub, subb       (r<lb)  |       xor, and        (sign != sign)
		 *                              |       and             (which)
		 */

		current_core->vr[vrD].w[i] = res;
	}
}

/*	vsububm		Vector Subtract Unsigned Byte Modulo
 *	v.265
 */
void ppc_opc_vsububm()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint8 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 16; i++) {
		res = current_core->vr[vrA].b[i] - current_core->vr[vrB].b[i];
		current_core->vr[vrD].b[i] = res;
	}
}

/*	vsubuhm		Vector Subtract Unsigned Half Word Modulo
 *	v.267
 */
void ppc_opc_vsubuhm()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint16 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 8; i++) {
		res = current_core->vr[vrA].h[i] - current_core->vr[vrB].h[i];
		current_core->vr[vrD].h[i] = res;
	}
}

/*	vsubuwm		Vector Subtract Unsigned Word Modulo
 *	v.269
 */
void ppc_opc_vsubuwm()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint32 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		res = current_core->vr[vrA].w[i] - current_core->vr[vrB].w[i];
		current_core->vr[vrD].w[i] = res;
	}
}

/*	vsubfp		Vector Subtract Float Point
 *	v.261
 */
void ppc_opc_vsubfp()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	float res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {	//FIXME: This might not comply with Java FP
		res = current_core->vr[vrA].f[i] - current_core->vr[vrB].f[i];
		current_core->vr[vrD].f[i] = res;
	}
}

/*	vsubcuw		Vector Subtract Carryout Unsigned Word
 *	v.260
 */
void ppc_opc_vsubcuw()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint32 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		res = current_core->vr[vrA].w[i] - current_core->vr[vrB].w[i];
		current_core->vr[vrD].w[i] =
		    (res <= current_core->vr[vrA].w[i]) ? 1 : 0;
	}
}

/*	vsububs		Vector Subtract Unsigned Byte Saturate
 *	v.266
 */
void ppc_opc_vsububs()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint16 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 16; i++) {
		res =
		    (uint16) current_core->vr[vrA].b[i] -
		    (uint16) current_core->vr[vrB].b[i];

		current_core->vr[vrD].b[i] = SATURATE_0B(res);
	}
}

/*	vsubsbs		Vector Subtract Signed Byte Saturate
 *	v.262
 */
void ppc_opc_vsubsbs()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	sint16 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 16; i++) {
		res =
		    (sint16) current_core->vr[vrA].sb[i] -
		    (sint16) current_core->vr[vrB].sb[i];

		current_core->vr[vrD].sb[i] = SATURATE_SB(res);
	}
}

/*	vsubuhs		Vector Subtract Unsigned Half Word Saturate
 *	v.268
 */
void ppc_opc_vsubuhs()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint32 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 8; i++) {
		res =
		    (uint32) current_core->vr[vrA].h[i] -
		    (uint32) current_core->vr[vrB].h[i];

		current_core->vr[vrD].h[i] = SATURATE_0H(res);
	}
}

/*	vsubshs		Vector Subtract Signed Half Word Saturate
 *	v.263
 */
void ppc_opc_vsubshs()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	sint32 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 8; i++) {
		res =
		    (sint32) current_core->vr[vrA].sh[i] -
		    (sint32) current_core->vr[vrB].sh[i];

		current_core->vr[vrD].sh[i] = SATURATE_SH(res);
	}
}

/*	vsubuws		Vector Subtract Unsigned Word Saturate
 *	v.270
 */
void ppc_opc_vsubuws()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint32 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		res = current_core->vr[vrA].w[i] - current_core->vr[vrB].w[i];

		// We do this to prevent us from having to do 64-bit math
		if (res > current_core->vr[vrA].w[i]) {
			res = 0;
			current_core->vscr |= VSCR_SAT;
		}

		/*      64-bit math             |       32-bit hack
		 *      ------------------------+-------------------------------------
		 *      sub, subb       (a+b)   |       sub             (a+b)
		 *      sub, subb       (r>ub)  |       sub             (r<a)
		 */

		current_core->vr[vrD].w[i] = res;
	}
}

/*	vsubsws		Vector Subtract Signed Word Saturate
 *	v.264
 */
void ppc_opc_vsubsws()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint32 res, tmp;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		tmp = -current_core->vr[vrB].w[i];
		res = current_core->vr[vrA].w[i] + tmp;

		// We do this to prevent us from having to do 64-bit math
		if (((current_core->vr[vrA].w[i] ^ tmp) & SIGN32) == 0) {
			// the signs of both operands are the same

			if (((res ^ tmp) & SIGN32) != 0) {
				// sign of result != sign of operands

				// if res is negative, should have been positive
				res = (res & SIGN32) ? (SIGN32 - 1) : SIGN32;
				current_core->vscr |= VSCR_SAT;
			}
		}

		/*      64-bit math             |       32-bit hack
		 *      ------------------------+-------------------------------------
		 *      sub, subc       (a+b)   |       neg, add        (a-b)
		 *      sub, subb       (r>ub)  |       xor, and        (sign == sign)
		 *      sub, subb       (r<lb)  |       xor, and        (sign != sign)
		 *                              |       and             (which)
		 */

		current_core->vr[vrD].w[i] = res;
	}
}

/*	vmuleub		Vector Multiply Even Unsigned Byte
 *	v.209
 */
void ppc_opc_vmuleub()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint16 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 8; i++) {
		res = (uint16) current_core->vr[vrA].b[VECT_EVEN(i)] *
		    (uint16) current_core->vr[vrB].b[VECT_EVEN(i)];

		current_core->vr[vrD].h[i] = res;
	}
}

/*	vmulesb		Vector Multiply Even Signed Byte
 *	v.207
 */
void ppc_opc_vmulesb()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	sint16 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 8; i++) {
		res = (sint16) current_core->vr[vrA].sb[VECT_EVEN(i)] *
		    (sint16) current_core->vr[vrB].sb[VECT_EVEN(i)];

		current_core->vr[vrD].sh[i] = res;
	}
}

/*	vmuleuh		Vector Multiply Even Unsigned Half Word
 *	v.210
 */
void ppc_opc_vmuleuh()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint32 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		res = (uint32) current_core->vr[vrA].h[VECT_EVEN(i)] *
		    (uint32) current_core->vr[vrB].h[VECT_EVEN(i)];

		current_core->vr[vrD].w[i] = res;
	}
}

/*	vmulesh		Vector Multiply Even Signed Half Word
 *	v.208
 */
void ppc_opc_vmulesh()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	sint32 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		res = (sint32) current_core->vr[vrA].sh[VECT_EVEN(i)] *
		    (sint32) current_core->vr[vrB].sh[VECT_EVEN(i)];

		current_core->vr[vrD].sw[i] = res;
	}
}

/*	vmuloub		Vector Multiply Odd Unsigned Byte
 *	v.213
 */
void ppc_opc_vmuloub()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint16 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 8; i++) {
		res = (uint16) current_core->vr[vrA].b[VECT_ODD(i)] *
		    (uint16) current_core->vr[vrB].b[VECT_ODD(i)];

		current_core->vr[vrD].h[i] = res;
	}
}

/*	vmulosb		Vector Multiply Odd Signed Byte
 *	v.211
 */
void ppc_opc_vmulosb()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	sint16 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 8; i++) {
		res = (sint16) current_core->vr[vrA].sb[VECT_ODD(i)] *
		    (sint16) current_core->vr[vrB].sb[VECT_ODD(i)];

		current_core->vr[vrD].sh[i] = res;
	}
}

/*	vmulouh		Vector Multiply Odd Unsigned Half Word
 *	v.214
 */
void ppc_opc_vmulouh()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint32 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		res = (uint32) current_core->vr[vrA].h[VECT_ODD(i)] *
		    (uint32) current_core->vr[vrB].h[VECT_ODD(i)];

		current_core->vr[vrD].w[i] = res;
	}
}

/*	vmulosh		Vector Multiply Odd Signed Half Word
 *	v.212
 */
void ppc_opc_vmulosh()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	sint32 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		res = (sint32) current_core->vr[vrA].sh[VECT_ODD(i)] *
		    (sint32) current_core->vr[vrB].sh[VECT_ODD(i)];

		current_core->vr[vrD].sw[i] = res;
	}
}

/*	vmaddfp		Vector Multiply Add Floating Point
 *	v.177
 */
void ppc_opc_vmaddfp()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB, vrC;
	double res;
	PPC_OPC_TEMPL_A(current_core->current_opc, vrD, vrA, vrB, vrC);
	int i;
	for (i = 0; i < 4; i++) {	//FIXME: This might not comply with Java FP
		res =
		    (double)current_core->vr[vrA].f[i] *
		    (double)current_core->vr[vrC].f[i];

		res = (double)current_core->vr[vrB].f[i] + res;

		current_core->vr[vrD].f[i] = (float)res;
	}
}

/*	vmhaddshs	Vector Multiply High and Add Signed Half Word Saturate
 *	v.185
 */
void ppc_opc_vmhaddshs()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB, vrC;
	sint32 prod;
	PPC_OPC_TEMPL_A(current_core->current_opc, vrD, vrA, vrB, vrC);
	int i;
	for (i = 0; i < 8; i++) {
		prod =
		    (sint32) current_core->vr[vrA].sh[i] *
		    (sint32) current_core->vr[vrB].sh[i];

		prod = (prod >> 15) + (sint32) current_core->vr[vrC].sh[i];

		current_core->vr[vrD].sh[i] = SATURATE_SH(prod);
	}
}

/*	vmladduhm	Vector Multiply Low and Add Unsigned Half Word Modulo
 *	v.194
 */
void ppc_opc_vmladduhm()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB, vrC;
	uint32 prod;
	PPC_OPC_TEMPL_A(current_core->current_opc, vrD, vrA, vrB, vrC);
	int i;
	for (i = 0; i < 8; i++) {
		prod =
		    (uint32) current_core->vr[vrA].h[i] *
		    (uint32) current_core->vr[vrB].h[i];

		prod = prod + (uint32) current_core->vr[vrC].h[i];

		current_core->vr[vrD].h[i] = prod;
	}
}

/*	vmhraddshs	Vector Multiply High Round and Add Signed Half Word Saturate
 *	v.186
 */
void ppc_opc_vmhraddshs()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB, vrC;
	sint32 prod;
	PPC_OPC_TEMPL_A(current_core->current_opc, vrD, vrA, vrB, vrC);
	int i;
	for (i = 0; i < 8; i++) {
		prod =
		    (sint32) current_core->vr[vrA].sh[i] *
		    (sint32) current_core->vr[vrB].sh[i];

		prod += 0x4000;
		prod = (prod >> 15) + (sint32) current_core->vr[vrC].sh[i];

		current_core->vr[vrD].sh[i] = SATURATE_SH(prod);
	}
}

/*	vmsumubm	Vector Multiply Sum Unsigned Byte Modulo
 *	v.204
 */
void ppc_opc_vmsumubm()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB, vrC;
	uint32 temp;
	PPC_OPC_TEMPL_A(current_core->current_opc, vrD, vrA, vrB, vrC);
	int i;
	for (i = 0; i < 4; i++) {
		temp = current_core->vr[vrC].w[i];

		temp += (uint16) current_core->vr[vrA].b[i << 2] *
		    (uint16) current_core->vr[vrB].b[i << 2];

		temp += (uint16) current_core->vr[vrA].b[(i << 2) + 1] *
		    (uint16) current_core->vr[vrB].b[(i << 2) + 1];

		temp += (uint16) current_core->vr[vrA].b[(i << 2) + 2] *
		    (uint16) current_core->vr[vrB].b[(i << 2) + 2];

		temp += (uint16) current_core->vr[vrA].b[(i << 2) + 3] *
		    (uint16) current_core->vr[vrB].b[(i << 2) + 3];

		current_core->vr[vrD].w[i] = temp;
	}
}

/*	vmsumuhm	Vector Multiply Sum Unsigned Half Word Modulo
 *	v.205
 */
void ppc_opc_vmsumuhm()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB, vrC;
	uint32 temp;
	PPC_OPC_TEMPL_A(current_core->current_opc, vrD, vrA, vrB, vrC);
	int i;
	for (i = 0; i < 4; i++) {
		temp = current_core->vr[vrC].w[i];

		temp += (uint32) current_core->vr[vrA].h[i << 1] *
		    (uint32) current_core->vr[vrB].h[i << 1];
		temp += (uint32) current_core->vr[vrA].h[(i << 1) + 1] *
		    (uint32) current_core->vr[vrB].h[(i << 1) + 1];

		current_core->vr[vrD].w[i] = temp;
	}
}

/*	vmsummbm	Vector Multiply Sum Mixed-Sign Byte Modulo
 *	v.201
 */
void ppc_opc_vmsummbm()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB, vrC;
	sint32 temp;
	PPC_OPC_TEMPL_A(current_core->current_opc, vrD, vrA, vrB, vrC);
	int i;
	for (i = 0; i < 4; i++) {
		temp = current_core->vr[vrC].sw[i];

		temp += (sint16) current_core->vr[vrA].sb[i << 2] *
		    (uint16) current_core->vr[vrB].b[i << 2];
		temp += (sint16) current_core->vr[vrA].sb[(i << 2) + 1] *
		    (uint16) current_core->vr[vrB].b[(i << 2) + 1];
		temp += (sint16) current_core->vr[vrA].sb[(i << 2) + 2] *
		    (uint16) current_core->vr[vrB].b[(i << 2) + 2];
		temp += (sint16) current_core->vr[vrA].sb[(i << 2) + 3] *
		    (uint16) current_core->vr[vrB].b[(i << 2) + 3];

		current_core->vr[vrD].sw[i] = temp;
	}
}

/*	vmsumshm	Vector Multiply Sum Signed Half Word Modulo
 *	v.202
 */
void ppc_opc_vmsumshm()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB, vrC;
	sint32 temp;
	PPC_OPC_TEMPL_A(current_core->current_opc, vrD, vrA, vrB, vrC);
	int i;
	for (i = 0; i < 4; i++) {
		temp = current_core->vr[vrC].sw[i];

		temp += (sint32) current_core->vr[vrA].sh[i << 1] *
		    (sint32) current_core->vr[vrB].sh[i << 1];
		temp += (sint32) current_core->vr[vrA].sh[(i << 1) + 1] *
		    (sint32) current_core->vr[vrB].sh[(i << 1) + 1];

		current_core->vr[vrD].sw[i] = temp;
	}
}

/*	vmsumuhs	Vector Multiply Sum Unsigned Half Word Saturate
 *	v.206
 */
void ppc_opc_vmsumuhs()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB, vrC;
	uint64 temp;
	PPC_OPC_TEMPL_A(current_core->current_opc, vrD, vrA, vrB, vrC);

	/* For this, there's no way to get around 64-bit math.  If we use
	 *   the hacks used before, then we have to do it so often, that
	 *   we'll outpace the 64-bit math in execution time.
	 */
	int i;
	for (i = 0; i < 4; i++) {
		temp = current_core->vr[vrC].w[i];

		temp += (uint32) current_core->vr[vrA].h[i << 1] *
		    (uint32) current_core->vr[vrB].h[i << 1];

		temp += (uint32) current_core->vr[vrA].h[(i << 1) + 1] *
		    (uint32) current_core->vr[vrB].h[(i << 1) + 1];

		current_core->vr[vrD].w[i] = SATURATE_UW(temp);
	}
}

/*	vmsumshs	Vector Multiply Sum Signed Half Word Saturate
 *	v.203
 */
void ppc_opc_vmsumshs()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB, vrC;
	sint64 temp;
	PPC_OPC_TEMPL_A(current_core->current_opc, vrD, vrA, vrB, vrC);

	/* For this, there's no way to get around 64-bit math.  If we use
	 *   the hacks used before, then we have to do it so often, that
	 *   we'll outpace the 64-bit math in execution time.
	 */
	int i;
	for (i = 0; i < 4; i++) {
		temp = current_core->vr[vrC].sw[i];

		temp += (sint32) current_core->vr[vrA].sh[i << 1] *
		    (sint32) current_core->vr[vrB].sh[i << 1];
		temp += (sint32) current_core->vr[vrA].sh[(i << 1) + 1] *
		    (sint32) current_core->vr[vrB].sh[(i << 1) + 1];

		current_core->vr[vrD].sw[i] = SATURATE_SW(temp);
	}
}

/*	vsum4ubs	Vector Sum Across Partial (1/4) Unsigned Byte Saturate
 *	v.275
 */
void ppc_opc_vsum4ubs()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint64 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	/* For this, there's no way to get around 64-bit math.  If we use
	 *   the hacks used before, then we have to do it so often, that
	 *   we'll outpace the 64-bit math in execution time.
	 */
	int i;
	for (i = 0; i < 4; i++) {
		res = (uint64) current_core->vr[vrB].w[i];

		res += (uint64) current_core->vr[vrA].b[(i << 2)];
		res += (uint64) current_core->vr[vrA].b[(i << 2) + 1];
		res += (uint64) current_core->vr[vrA].b[(i << 2) + 2];
		res += (uint64) current_core->vr[vrA].b[(i << 2) + 3];

		current_core->vr[vrD].w[i] = SATURATE_UW(res);
	}
}

/*	vsum4sbs	Vector Sum Across Partial (1/4) Signed Byte Saturate
 *	v.273
 */
void ppc_opc_vsum4sbs()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	sint64 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		res = (sint64) current_core->vr[vrB].sw[i];

		res += (sint64) current_core->vr[vrA].sb[(i << 2)];
		res += (sint64) current_core->vr[vrA].sb[(i << 2) + 1];
		res += (sint64) current_core->vr[vrA].sb[(i << 2) + 2];
		res += (sint64) current_core->vr[vrA].sb[(i << 2) + 3];

		current_core->vr[vrD].sw[i] = SATURATE_SW(res);
	}
}

/*	vsum4shs	Vector Sum Across Partial (1/4) Signed Half Word Saturate
 *	v.274
 */
void ppc_opc_vsum4shs()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	sint64 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		res = (sint64) current_core->vr[vrB].sw[i];

		res += (sint64) current_core->vr[vrA].sh[(i << 1)];
		res += (sint64) current_core->vr[vrA].sh[(i << 1) + 1];

		current_core->vr[vrD].sw[i] = SATURATE_SW(res);
	}
}

/*	vsum2sws	Vector Sum Across Partial (1/2) Signed Word Saturate
 *	v.272
 */
void ppc_opc_vsum2sws()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	sint64 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	res =
	    (sint64) current_core->vr[vrA].sw[0] +
	    (sint64) current_core->vr[vrA].sw[1];
	res += (sint64) current_core->vr[vrB].sw[VECT_ODD(0)];

	current_core->vr[vrD].w[VECT_ODD(0)] = SATURATE_SW(res);
	current_core->vr[vrD].w[VECT_EVEN(0)] = 0;

	res =
	    (sint64) current_core->vr[vrA].sw[2] +
	    (sint64) current_core->vr[vrA].sw[3];
	res += (sint64) current_core->vr[vrB].sw[VECT_ODD(1)];

	current_core->vr[vrD].w[VECT_ODD(1)] = SATURATE_SW(res);
	current_core->vr[vrD].w[VECT_EVEN(1)] = 0;
}

/*	vsumsws		Vector Sum Across Signed Word Saturate
 *	v.271
 */
void ppc_opc_vsumsws()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	sint64 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	res =
	    (sint64) current_core->vr[vrA].sw[0] +
	    (sint64) current_core->vr[vrA].sw[1];
	res +=
	    (sint64) current_core->vr[vrA].sw[2] +
	    (sint64) current_core->vr[vrA].sw[3];

	res += (sint64) VECT_W(current_core->vr[vrB], 3);

	VECT_W(current_core->vr[vrD], 3) = SATURATE_SW(res);
	VECT_W(current_core->vr[vrD], 2) = 0;
	VECT_W(current_core->vr[vrD], 1) = 0;
	VECT_W(current_core->vr[vrD], 0) = 0;
}

/*	vnmsubfp	Vector Negative Multiply-Subtract Floating Point
 *	v.215
 */
void ppc_opc_vnmsubfp()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB, vrC;
	double res;
	PPC_OPC_TEMPL_A(current_core->current_opc, vrD, vrA, vrB, vrC);
	int i;
	for (i = 0; i < 4; i++) {	//FIXME: This might not comply with Java FP
		res =
		    (double)current_core->vr[vrA].f[i] *
		    (double)current_core->vr[vrC].f[i];

		res = (double)current_core->vr[vrB].f[i] - res;

		current_core->vr[vrD].f[i] = (float)res;
	}
}

/*	vavgub		Vector Average Unsigned Byte
 *	v.152
 */
void ppc_opc_vavgub()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint16 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 16; i++) {
		res = (uint16) current_core->vr[vrA].b[i] +
		    (uint16) current_core->vr[vrB].b[i] + 1;

		current_core->vr[vrD].b[i] = (res >> 1);
	}
}

/*	vavguh		Vector Average Unsigned Half Word
 *	v.153
 */
void ppc_opc_vavguh()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint32 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 8; i++) {
		res = (uint32) current_core->vr[vrA].h[i] +
		    (uint32) current_core->vr[vrB].h[i] + 1;

		current_core->vr[vrD].h[i] = (res >> 1);
	}
}

/*	vavguw		Vector Average Unsigned Word
 *	v.154
 */
void ppc_opc_vavguw()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint64 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		res = (uint64) current_core->vr[vrA].w[i] +
		    (uint64) current_core->vr[vrB].w[i] + 1;

		current_core->vr[vrD].w[i] = (res >> 1);
	}
}

/*	vavgsb		Vector Average Signed Byte
 *	v.149
 */
void ppc_opc_vavgsb()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	sint16 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 16; i++) {
		res = (sint16) current_core->vr[vrA].sb[i] +
		    (sint16) current_core->vr[vrB].sb[i] + 1;

		current_core->vr[vrD].sb[i] = (res >> 1);
	}
}

/*	vavgsh		Vector Average Signed Half Word
 *	v.150
 */
void ppc_opc_vavgsh()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	sint32 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 8; i++) {
		res = (sint32) current_core->vr[vrA].sh[i] +
		    (sint32) current_core->vr[vrB].sh[i] + 1;

		current_core->vr[vrD].sh[i] = (res >> 1);
	}
}

/*	vavgsw		Vector Average Signed Word
 *	v.151
 */
void ppc_opc_vavgsw()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	sint64 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		res = (sint64) current_core->vr[vrA].sw[i] +
		    (sint64) current_core->vr[vrB].sw[i] + 1;

		current_core->vr[vrD].sw[i] = (res >> 1);
	}
}

/*	vmaxub		Vector Maximum Unsigned Byte
 *	v.182
 */
void ppc_opc_vmaxub()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint8 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 16; i++) {
		res = current_core->vr[vrA].b[i];

		if (res < current_core->vr[vrB].b[i])
			res = current_core->vr[vrB].b[i];

		current_core->vr[vrD].b[i] = res;
	}
}

/*	vmaxuh		Vector Maximum Unsigned Half Word
 *	v.183
 */
void ppc_opc_vmaxuh()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint16 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 8; i++) {
		res = current_core->vr[vrA].h[i];

		if (res < current_core->vr[vrB].h[i])
			res = current_core->vr[vrB].h[i];

		current_core->vr[vrD].h[i] = res;
	}
}

/*	vmaxuw		Vector Maximum Unsigned Word
 *	v.184
 */
void ppc_opc_vmaxuw()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint32 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		res = current_core->vr[vrA].w[i];

		if (res < current_core->vr[vrB].w[i])
			res = current_core->vr[vrB].w[i];

		current_core->vr[vrD].w[i] = res;
	}
}

/*	vmaxsb		Vector Maximum Signed Byte
 *	v.179
 */
void ppc_opc_vmaxsb()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	sint8 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 16; i++) {
		res = current_core->vr[vrA].sb[i];

		if (res < current_core->vr[vrB].sb[i])
			res = current_core->vr[vrB].sb[i];

		current_core->vr[vrD].sb[i] = res;
	}
}

/*	vmaxsh		Vector Maximum Signed Half Word
 *	v.180
 */
void ppc_opc_vmaxsh()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	sint16 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 8; i++) {
		res = current_core->vr[vrA].sh[i];

		if (res < current_core->vr[vrB].sh[i])
			res = current_core->vr[vrB].sh[i];

		current_core->vr[vrD].sh[i] = res;
	}
}

/*	vmaxsw		Vector Maximum Signed Word
 *	v.181
 */
void ppc_opc_vmaxsw()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	sint32 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		res = current_core->vr[vrA].sw[i];

		if (res < current_core->vr[vrB].sw[i])
			res = current_core->vr[vrB].sw[i];

		current_core->vr[vrD].sw[i] = res;
	}
}

/*	vmaxfp		Vector Maximum Floating Point
 *	v.178
 */
void ppc_opc_vmaxfp()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	float res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {	//FIXME: This might not comply with Java FP
		res = current_core->vr[vrA].f[i];

		if (res < current_core->vr[vrB].f[i])
			res = current_core->vr[vrB].f[i];

		current_core->vr[vrD].f[i] = res;
	}
}

/*	vminub		Vector Minimum Unsigned Byte
 *	v.191
 */
void ppc_opc_vminub()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint8 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 16; i++) {
		res = current_core->vr[vrA].b[i];

		if (res > current_core->vr[vrB].b[i])
			res = current_core->vr[vrB].b[i];

		current_core->vr[vrD].b[i] = res;
	}
}

/*	vminuh		Vector Minimum Unsigned Half Word
 *	v.192
 */
void ppc_opc_vminuh()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint16 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 8; i++) {
		res = current_core->vr[vrA].h[i];

		if (res > current_core->vr[vrB].h[i])
			res = current_core->vr[vrB].h[i];

		current_core->vr[vrD].h[i] = res;
	}
}

/*	vminuw		Vector Minimum Unsigned Word
 *	v.193
 */
void ppc_opc_vminuw()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	uint32 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		res = current_core->vr[vrA].w[i];

		if (res > current_core->vr[vrB].w[i])
			res = current_core->vr[vrB].w[i];

		current_core->vr[vrD].w[i] = res;
	}
}

/*	vminsb		Vector Minimum Signed Byte
 *	v.188
 */
void ppc_opc_vminsb()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	sint8 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 16; i++) {
		res = current_core->vr[vrA].sb[i];

		if (res > current_core->vr[vrB].sb[i])
			res = current_core->vr[vrB].sb[i];

		current_core->vr[vrD].sb[i] = res;
	}
}

/*	vminsh		Vector Minimum Signed Half Word
 *	v.189
 */
void ppc_opc_vminsh()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	sint16 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 8; i++) {
		res = current_core->vr[vrA].sh[i];

		if (res > current_core->vr[vrB].sh[i])
			res = current_core->vr[vrB].sh[i];

		current_core->vr[vrD].sh[i] = res;
	}
}

/*	vminsw		Vector Minimum Signed Word
 *	v.190
 */
void ppc_opc_vminsw()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	sint32 res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		res = current_core->vr[vrA].sw[i];

		if (res > current_core->vr[vrB].sw[i])
			res = current_core->vr[vrB].sw[i];

		current_core->vr[vrD].sw[i] = res;
	}
}

/*	vminfp		Vector Minimum Floating Point
 *	v.187
 */
void ppc_opc_vminfp()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	float res;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {	//FIXME: This might not comply with Java FP
		res = current_core->vr[vrA].f[i];

		if (res > current_core->vr[vrB].f[i])
			res = current_core->vr[vrB].f[i];

		current_core->vr[vrD].f[i] = res;
	}
}

/*	vrfin		Vector Round to Floating-Point Integer Nearest
 *	v.231
 */
void ppc_opc_vrfin()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	PPC_OPC_ASSERT(vrA == 0);

	/* Documentation doesn't dictate how this instruction should
	 *   round from a middle point.  With a test on a real G4, it was
	 *   found to be round to nearest, with bias to even if equidistant.
	 *
	 * This is covered by the function rint()
	 */
	int i;
	for (i = 0; i < 4; i++) {	//FIXME: This might not comply with Java FP
		current_core->vr[vrD].f[i] = rintf(current_core->vr[vrB].f[i]);
	}
}

/*	vrfip		Vector Round to Floating-Point Integer toward Plus Infinity
 *	v.232
 */
void ppc_opc_vrfip()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	PPC_OPC_ASSERT(vrA == 0);
	int i;
	for (i = 0; i < 4; i++) {	//FIXME: This might not comply with Java FP
		current_core->vr[vrD].f[i] = ceilf(current_core->vr[vrB].f[i]);
	}
}

/*	vrfim		Vector Round to Floating-Point Integer toward Minus Infinity
 *	v.230
 */
void ppc_opc_vrfim()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	PPC_OPC_ASSERT(vrA == 0);
	int i;
	for (i = 0; i < 4; i++) {	//FIXME: This might not comply with Java FP
		current_core->vr[vrD].f[i] = floorf(current_core->vr[vrB].f[i]);
	}
}

/*	vrfiz	Vector Round to Floating-Point Integer toward Zero
 *	v.233
 */
void ppc_opc_vrfiz()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	PPC_OPC_ASSERT(vrA == 0);
	int i;
	for (i = 0; i < 4; i++) {	//FIXME: This might not comply with Java FP
		current_core->vr[vrD].f[i] = truncf(current_core->vr[vrD].f[i]);
	}
}

/*	vrefp		Vector Reciprocal Estimate Floating Point
 *	v.228
 */
void ppc_opc_vrefp()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	PPC_OPC_ASSERT(vrA == 0);

	/* This emulation generates an exact value, instead of an estimate.
	 *   This is technically within specs, but some test-suites expect the
	 *   exact estimate value returned by G4s.  These anomolous failures
	 *   should be ignored.
	 */
	int i;
	for (i = 0; i < 4; i++) {	//FIXME: This might not comply with Java FP
		current_core->vr[vrD].f[i] = 1 / current_core->vr[vrB].f[i];
	}
}

/*	vrsqrtefp	Vector Reciprocal Square Root Estimate Floating Point
 *	v.237
 */
void ppc_opc_vrsqrtefp()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	PPC_OPC_ASSERT(vrA == 0);

	/* This emulation generates an exact value, instead of an estimate.
	 *   This is technically within specs, but some test-suites expect the
	 *   exact estimate value returned by G4s.  These anomolous failures
	 *   should be ignored.
	 */
	int i;
	for (i = 0; i < 4; i++) {	//FIXME: This might not comply with Java FP
		current_core->vr[vrD].f[i] =
		    1 / sqrt(current_core->vr[vrB].f[i]);
	}
}

/*	vlogefp		Vector Log2 Estimate Floating Point
 *	v.175
 */
void ppc_opc_vlogefp()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	PPC_OPC_ASSERT(vrA == 0);

	/* This emulation generates an exact value, instead of an estimate.
	 *   This is technically within specs, but some test-suites expect the
	 *   exact estimate value returned by G4s.  These anomolous failures
	 *   should be ignored.
	 */
	int i;
	for (i = 0; i < 4; i++) {	//FIXME: This might not comply with Java FP
		current_core->vr[vrD].f[i] = log2(current_core->vr[vrB].f[i]);
	}
}

/*	vexptefp	Vector 2 Raised to the Exponent Estimate Floating Point
 *	v.173
 */
void ppc_opc_vexptefp()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	PPC_OPC_ASSERT(vrA == 0);

	/* This emulation generates an exact value, instead of an estimate.
	 *   This is technically within specs, but some test-suites expect the
	 *   exact estimate value returned by G4s.  These anomolous failures
	 *   should be ignored.
	 */
	int i;
	for (i = 0; i < 4; i++) {	//FIXME: This might not comply with Java FP
		current_core->vr[vrD].f[i] = exp2(current_core->vr[vrB].f[i]);
	}
}

/*	vcfux		Vector Convert from Unsigned Fixed-Point Word
 *	v.156
 */
void ppc_opc_vcfux()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrB;
	uint32 uimm;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, uimm, vrB);
	int i;
	for (i = 0; i < 4; i++) {	//FIXME: This might not comply with Java FP
		current_core->vr[vrD].f[i] =
		    ((float)current_core->vr[vrB].w[i]) / (1 << uimm);
	}
}

/*	vcfsx		Vector Convert from Signed Fixed-Point Word
 *	v.155
 */
void ppc_opc_vcfsx()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrB;
	uint32 uimm;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, uimm, vrB);
	int i;
	for (i = 0; i < 4; i++) {	//FIXME: This might not comply with Java FP
		current_core->vr[vrD].f[i] =
		    ((float)current_core->vr[vrB].sw[i]) / (1 << uimm);
	}
}

/*	vctsxs		Vector Convert To Signed Fixed-Point Word Saturate
 *	v.171
 */
void ppc_opc_vctsxs()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrB;
	uint32 uimm;
	float ftmp;
	sint32 tmp;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, uimm, vrB);
	int i;
	for (i = 0; i < 4; i++) {	//FIXME: This might not comply with Java FP
		ftmp = current_core->vr[vrB].f[i] * (float)(1 << uimm);
		ftmp = truncf(ftmp);

		tmp = (sint32) ftmp;

		if (ftmp > 2147483647.0) {
			tmp = 2147483647;	// 0x7fffffff
			current_core->vscr |= VSCR_SAT;
		} else if (ftmp < -2147483648.0) {
			tmp = -2147483648LL;	// 0x80000000
			current_core->vscr |= VSCR_SAT;
		}

		current_core->vr[vrD].sw[i] = tmp;
	}
}

/*	vctuxs		Vector Convert to Unsigned Fixed-Point Word Saturate
 *	v.172
 */
void ppc_opc_vctuxs()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrB;
	uint32 tmp, uimm;
	float ftmp;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, uimm, vrB);
	int i;
	for (i = 0; i < 4; i++) {	//FIXME: This might not comply with Java FP
		ftmp = current_core->vr[vrB].f[i] * (float)(1 << uimm);
		ftmp = truncf(ftmp);

		tmp = (uint32) ftmp;

		if (ftmp > 4294967295.0) {
			tmp = 0xffffffff;
			current_core->vscr |= VSCR_SAT;
		} else if (ftmp < 0) {
			tmp = 0;
			current_core->vscr |= VSCR_SAT;
		}

		current_core->vr[vrD].w[i] = tmp;
	}
}

/*	vand		Vector Logical AND
 *	v.147
 */
void ppc_opc_vand()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	current_core->vr[vrD].d[0] =
	    current_core->vr[vrA].d[0] & current_core->vr[vrB].d[0];
	current_core->vr[vrD].d[1] =
	    current_core->vr[vrA].d[1] & current_core->vr[vrB].d[1];
}

/*	vandc		Vector Logical AND with Complement
 *	v.148
 */
void ppc_opc_vandc()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	current_core->vr[vrD].d[0] =
	    current_core->vr[vrA].d[0] & ~current_core->vr[vrB].d[0];
	current_core->vr[vrD].d[1] =
	    current_core->vr[vrA].d[1] & ~current_core->vr[vrB].d[1];
}

/*	vor		Vector Logical OR
 *	v.217
 */
void ppc_opc_vor()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG_COMMON;
	int vrD, vrA, vrB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	current_core->vr[vrD].d[0] =
	    current_core->vr[vrA].d[0] | current_core->vr[vrB].d[0];
	current_core->vr[vrD].d[1] =
	    current_core->vr[vrA].d[1] | current_core->vr[vrB].d[1];
}

/*	vnor		Vector Logical NOR
 *	v.216
 */
void ppc_opc_vnor()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	current_core->vr[vrD].d[0] =
	    ~(current_core->vr[vrA].d[0] | current_core->vr[vrB].d[0]);
	current_core->vr[vrD].d[1] =
	    ~(current_core->vr[vrA].d[1] | current_core->vr[vrB].d[1]);
}

/*	vxor		Vector Logical XOR
 *	v.282
 */
void ppc_opc_vxor()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG_COMMON;
	int vrD, vrA, vrB;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);

	current_core->vr[vrD].d[0] =
	    current_core->vr[vrA].d[0] ^ current_core->vr[vrB].d[0];
	current_core->vr[vrD].d[1] =
	    current_core->vr[vrA].d[1] ^ current_core->vr[vrB].d[1];
}

#define CR_CR6		(0x00f0)
#define CR_CR6_EQ	(1<<7)
#define CR_CR6_NE	(1<<5)

/*	vcmpequbx	Vector Compare Equal-to Unsigned Byte
 *	v.160
 */
void ppc_opc_vcmpequbx()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	int tf = CR_CR6_EQ | CR_CR6_NE;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 16; i++) {
		if (current_core->vr[vrA].b[i] == current_core->vr[vrB].b[i]) {
			current_core->vr[vrD].b[i] = 0xff;
			tf &= ~CR_CR6_NE;
		} else {
			current_core->vr[vrD].b[i] = 0;
			tf &= ~CR_CR6_EQ;
		}
	}

	if (PPC_OPC_VRc & current_core->current_opc) {
		current_core->cr &= ~CR_CR6;
		current_core->cr |= tf;
	}
}

/*	vcmpequhx	Vector Compare Equal-to Unsigned Half Word
 *	v.161
 */
void ppc_opc_vcmpequhx()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	int tf = CR_CR6_EQ | CR_CR6_NE;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 8; i++) {
		if (current_core->vr[vrA].h[i] == current_core->vr[vrB].h[i]) {
			current_core->vr[vrD].h[i] = 0xffff;
			tf &= ~CR_CR6_NE;
		} else {
			current_core->vr[vrD].h[i] = 0;
			tf &= ~CR_CR6_EQ;
		}
	}

	if (PPC_OPC_VRc & current_core->current_opc) {
		current_core->cr &= ~CR_CR6;
		current_core->cr |= tf;
	}
}

/*	vcmpequwx	Vector Compare Equal-to Unsigned Word
 *	v.162
 */
void ppc_opc_vcmpequwx()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	int tf = CR_CR6_EQ | CR_CR6_NE;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		if (current_core->vr[vrA].w[i] == current_core->vr[vrB].w[i]) {
			current_core->vr[vrD].w[i] = 0xffffffff;
			tf &= ~CR_CR6_NE;
		} else {
			current_core->vr[vrD].w[i] = 0;
			tf &= ~CR_CR6_EQ;
		}
	}

	if (PPC_OPC_VRc & current_core->current_opc) {
		current_core->cr &= ~CR_CR6;
		current_core->cr |= tf;
	}
}

/*	vcmpeqfpx	Vector Compare Equal-to-Floating Point
 *	v.159
 */
void ppc_opc_vcmpeqfpx()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	int tf = CR_CR6_EQ | CR_CR6_NE;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {	//FIXME: This might not comply with Java FP
		if (current_core->vr[vrA].f[i] == current_core->vr[vrB].f[i]) {
			current_core->vr[vrD].w[i] = 0xffffffff;
			tf &= ~CR_CR6_NE;
		} else {
			current_core->vr[vrD].w[i] = 0;
			tf &= ~CR_CR6_EQ;
		}
	}

	if (PPC_OPC_VRc & current_core->current_opc) {
		current_core->cr &= ~CR_CR6;
		current_core->cr |= tf;
	}
}

/*	vcmpgtubx	Vector Compare Greater-Than Unsigned Byte
 *	v.168
 */
void ppc_opc_vcmpgtubx()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	int tf = CR_CR6_EQ | CR_CR6_NE;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 16; i++) {
		if (current_core->vr[vrA].b[i] > current_core->vr[vrB].b[i]) {
			current_core->vr[vrD].b[i] = 0xff;
			tf &= ~CR_CR6_NE;
		} else {
			current_core->vr[vrD].b[i] = 0;
			tf &= ~CR_CR6_EQ;
		}
	}

	if (PPC_OPC_VRc & current_core->current_opc) {
		current_core->cr &= ~CR_CR6;
		current_core->cr |= tf;
	}
}

/*	vcmpgtsbx	Vector Compare Greater-Than Signed Byte
 *	v.165
 */
void ppc_opc_vcmpgtsbx()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	int tf = CR_CR6_EQ | CR_CR6_NE;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 16; i++) {
		if (current_core->vr[vrA].sb[i] > current_core->vr[vrB].sb[i]) {
			current_core->vr[vrD].b[i] = 0xff;
			tf &= ~CR_CR6_NE;
		} else {
			current_core->vr[vrD].b[i] = 0;
			tf &= ~CR_CR6_EQ;
		}
	}

	if (PPC_OPC_VRc & current_core->current_opc) {
		current_core->cr &= ~CR_CR6;
		current_core->cr |= tf;
	}
}

/*	vcmpgtuhx	Vector Compare Greater-Than Unsigned Half Word
 *	v.169
 */
void ppc_opc_vcmpgtuhx()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	int tf = CR_CR6_EQ | CR_CR6_NE;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 8; i++) {
		if (current_core->vr[vrA].h[i] > current_core->vr[vrB].h[i]) {
			current_core->vr[vrD].h[i] = 0xffff;
			tf &= ~CR_CR6_NE;
		} else {
			current_core->vr[vrD].h[i] = 0;
			tf &= ~CR_CR6_EQ;
		}
	}

	if (PPC_OPC_VRc & current_core->current_opc) {
		current_core->cr &= ~CR_CR6;
		current_core->cr |= tf;
	}
}

/*	vcmpgtshx	Vector Compare Greater-Than Signed Half Word
 *	v.166
 */
void ppc_opc_vcmpgtshx()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	int tf = CR_CR6_EQ | CR_CR6_NE;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 8; i++) {
		if (current_core->vr[vrA].sh[i] > current_core->vr[vrB].sh[i]) {
			current_core->vr[vrD].h[i] = 0xffff;
			tf &= ~CR_CR6_NE;
		} else {
			current_core->vr[vrD].h[i] = 0;
			tf &= ~CR_CR6_EQ;
		}
	}

	if (PPC_OPC_VRc & current_core->current_opc) {
		current_core->cr &= ~CR_CR6;
		current_core->cr |= tf;
	}
}

/*	vcmpgtuwx	Vector Compare Greater-Than Unsigned Word
 *	v.170
 */
void ppc_opc_vcmpgtuwx()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	int tf = CR_CR6_EQ | CR_CR6_NE;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		if (current_core->vr[vrA].w[i] > current_core->vr[vrB].w[i]) {
			current_core->vr[vrD].w[i] = 0xffffffff;
			tf &= ~CR_CR6_NE;
		} else {
			current_core->vr[vrD].w[i] = 0;
			tf &= ~CR_CR6_EQ;
		}
	}

	if (PPC_OPC_VRc & current_core->current_opc) {
		current_core->cr &= ~CR_CR6;
		current_core->cr |= tf;
	}
}

/*	vcmpgtswx	Vector Compare Greater-Than Signed Word
 *	v.167
 */
void ppc_opc_vcmpgtswx()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	int tf = CR_CR6_EQ | CR_CR6_NE;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {
		if (current_core->vr[vrA].sw[i] > current_core->vr[vrB].sw[i]) {
			current_core->vr[vrD].w[i] = 0xffffffff;
			tf &= ~CR_CR6_NE;
		} else {
			current_core->vr[vrD].w[i] = 0;
			tf &= ~CR_CR6_EQ;
		}
	}

	if (PPC_OPC_VRc & current_core->current_opc) {
		current_core->cr &= ~CR_CR6;
		current_core->cr |= tf;
	}
}

/*	vcmpgtfpx	Vector Compare Greater-Than Floating-Point
 *	v.164
 */
void ppc_opc_vcmpgtfpx()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	int tf = CR_CR6_EQ | CR_CR6_NE;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {	//FIXME: This might not comply with Java FP
		if (current_core->vr[vrA].f[i] > current_core->vr[vrB].f[i]) {
			current_core->vr[vrD].w[i] = 0xffffffff;
			tf &= ~CR_CR6_NE;
		} else {
			current_core->vr[vrD].w[i] = 0;
			tf &= ~CR_CR6_EQ;
		}
	}

	if (PPC_OPC_VRc & current_core->current_opc) {
		current_core->cr &= ~CR_CR6;
		current_core->cr |= tf;
	}
}

/*	vcmpgefpx	Vector Compare Greater-Than-or-Equal-to Floating Point
 *	v.163
 */
void ppc_opc_vcmpgefpx()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	int tf = CR_CR6_EQ | CR_CR6_NE;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {	//FIXME: This might not comply with Java FP
		if (current_core->vr[vrA].f[i] >= current_core->vr[vrB].f[i]) {
			current_core->vr[vrD].w[i] = 0xffffffff;
			tf &= ~CR_CR6_NE;
		} else {
			current_core->vr[vrD].w[i] = 0;
			tf &= ~CR_CR6_EQ;
		}
	}

	if (PPC_OPC_VRc & current_core->current_opc) {
		current_core->cr &= ~CR_CR6;
		current_core->cr |= tf;
	}
}

/*	vcmpbfpx	Vector Compare Bounds Floating Point
 *	v.157
 */
void ppc_opc_vcmpbfpx()
{
	e500_core_t *current_core = get_current_core();
	VECTOR_DEBUG;
	int vrD, vrA, vrB;
	int le, ge;
	int ib = CR_CR6_NE;
	PPC_OPC_TEMPL_X(current_core->current_opc, vrD, vrA, vrB);
	int i;
	for (i = 0; i < 4; i++) {	//FIXME: This might not comply with Java FP
		le = (current_core->vr[vrA].f[i] <=
		      current_core->vr[vrB].f[i]) ? 0 : 0x80000000;
		ge = (current_core->vr[vrA].f[i] >=
		      -current_core->vr[vrB].f[i]) ? 0 : 0x40000000;

		current_core->vr[vrD].w[i] = le | ge;
		if (le | ge) {
			ib = 0;
		}
	}

	if (PPC_OPC_VRc & current_core->current_opc) {
		current_core->cr &= ~CR_CR6;
		current_core->cr |= ib;
	}
}
