/*  armos.h -- ARMulator OS definitions:  ARM6 Instruction Emulator.
    Copyright (C) 1994 Advanced RISC Machines Ltd.
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#include "bank_defs.h"
#include "dyncom/defines.h"

typedef struct mmap_area{
	mem_bank_t bank;
	void *mmap_addr;
	struct mmap_area *next;
}mmap_area_t;

#ifdef FAST_MEMORY
// depends on configuration
#define mmap_base 0x03000000
#else
#define mmap_base 0x50000000
#endif
static long mmap_next_base = mmap_base;

static mmap_area_t* new_mmap_area(int sim_addr, int len);
static char mmap_mem_write(short size, int addr, uint32_t value);
static char mmap_mem_read(short size, int addr, uint32_t * value);

/***************************************************************************\
*                               SWI numbers                                 *
\***************************************************************************/

#define SWI_Exit                   0x1
#define SWI_Read                   0x3
#define SWI_Write                  0x4
#define SWI_Open                   0x5
#define SWI_Close                  0x6
#define SWI_Seek                   0x13
#define SWI_Rename                 0x26
#define SWI_Break                  0x11

#define SWI_Times		   0x2b
#define SWI_Brk			   0x2d

#define SWI_Mmap                   0x5a
#define SWI_Munmap                   0x5b


#define SWI_GetUID32               0xc7
#define SWI_GetGID32               0xc8
#define SWI_GetEUID32              0xc9
#define SWI_GetEGID32              0xca

#define SWI_ExitGroup		   0xf8

#if 0
#define SWI_Time                   0xd
#define SWI_Clock                  0x61
#define SWI_Time                   0x63
#define SWI_Remove                 0x64
#define SWI_Rename                 0x65
#define SWI_Flen                   0x6c
#endif

#define SWI_Breakpoint             0x180000	/* see gdb's tm-arm.h */

