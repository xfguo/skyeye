/*
 *	stat.h - The statistics of timing.  
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
#ifndef __DYNCOM_STAT_H__
#define __DYNCOM_STAT_H__
#include "dyncom/defines.h"
void update_timing(cpu_t *cpu, int index, bool start);
#if TIMING_PROF
#define UPDATE_TIMING(cpu, index, start) update_timing(cpu, index, start)
#else
#define UPDATE_TIMING(cpu, index, start)
#endif
#endif
