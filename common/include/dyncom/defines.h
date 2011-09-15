/*
 *	defines.h - The macro definition for profiling and debug.  
 *
 *	Copyright (C) 2010 Michael.kang (blackfin.kang@gmail.com)
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
 * 01/16/2011   Michael.Kang  <blackfin.kang@gmail.com>
 */

#ifndef __DYNCOM_DEFINE_H__
#define __DYNCOM_DEFINE_H__
/*
 * Add big switches for testing here.
 * libcpu shouldn't use many ifdefs. Consider dropping one of
 * the alteratives or make it a runtime option (i.e. let the
 * client decide).
 */

// Copy register set paramters into a local array this hints LLVM to
// not care about writing back the contents too often.
// This works particularly well with PromoteMemoryToReg optimization
// pass, which removes the local arrays and make LLVM do the register
// allocation for us
//#define OPT_LOCAL_REGISTERS

/* The USER_MODE_OPT can not work with kernel running */
#define USER_MODE_OPT 1

// DFS limit when CPU_CODEGEN_TAG_LIMIT is set by the client.
// '6' is the optimum for OpenBSD's 'date' on M88K.
#if USER_MODE_OPT
#define LIMIT_TAGGING_DFS 2
#else
#define LIMIT_TAGGING_DFS 1
#endif
/* FAST_MEMORY is only used in user mode simulation */
#if USER_MODE_OPT
#define FAST_MEMORY
#endif

#define ENABLE_DEBUG_ME 0
/* Running kernel should set ENABLE_ICOUNTER */
#if USER_MODE_OPT
#define ENABLE_ICOUNTER 0
#else
#define ENABLE_ICOUNTER 1
#endif

/* Micros of time prifile */
#define THREAD_CLOCK 0
#define TIMING_PROF 0
#endif
