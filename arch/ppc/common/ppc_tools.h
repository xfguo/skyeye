/*
 *	PearPC
 *	ppc_tools.h
 *
 *	Copyright (C) 2003 Sebastian Biallas (sb@biallas.net)
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

#ifndef __PPC_TOOLS_H__
#define __PPC_TOOLS_H__

#include "skyeye_types.h"
#include "ppc_fpu.h"
static inline bool_t ppc_carry_3(uint32 a, uint32 b, uint32 c)
{
	if ((a + b) < a) {
		return True;
	}
	if ((a + b + c) < c) {
		return True;
	}
	return False;
}

static inline uint32 ppc_word_rotl(uint32 data, int n)
{
	n &= 0x1f;
	return (data << n) | (data >> (32 - n));
}

void ppc_fpu_single_to_double(ppc_single * s, ppc_double * d);
uint32 ppc_fpu_pack_single(ppc_double * d, uint32 * res);
void ppc_fpu_unpack_single(ppc_single * res, uint32 d);
uint32 ppc_fpu_pack_double(ppc_double * d, uint64 * res);
uint32 ppc_fpu_unpack_double(ppc_double * d, uint64 * res);
int ppc_fpu_normalize(ppc_double * d);
int ppc_fpu_normalize_single(ppc_single * s);
int ppc_fpu_normalize_quadro(ppc_quadro * d);
uint32 ppc_fpu_pack_double_as_single(ppc_double * d, uint64 * res);
uint32 ppc_fpu_double_to_int(ppc_double * d);
#endif
