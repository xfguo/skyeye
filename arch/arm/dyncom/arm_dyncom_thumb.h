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
* @file arm_dyncom_thumb.h
* @brief The thumb dyncom
* @author Michael.Kang blackfin.kang@gmail.com
* @version 78.77
* @date 2011-11-07
*/
#ifndef __ARM_DYNCOM_THUMB_H__
#define __ARM_DYNCOM_THUMB_H__
#include "armdefs.h"
//#include <armemu.h>
#include <dyncom_types.h>
//typedef arm_core_t arm_processor;
/* Copied from armemu.h */
/* Thumb support.  */
typedef enum
{
	t_undefined,		/* Undefined Thumb instruction.  */
	t_decoded,		/* Instruction decoded to ARM equivalent.  */
	t_branch		/* Thumb branch (already processed).  */
}tdstate;

tdstate
thumb_translate (arm_core_t *cpu, uint32_t instr, uint32_t* ainstr, uint32_t* inst_size);
static inline uint32 get_thumb_instr(uint32 instr, addr_t pc){
	uint32 tinstr;
	if((pc & 0x3) != 0)
		tinstr = instr >> 16;
	else
		tinstr = instr & 0xFFFF;
	return tinstr;
}
#endif
