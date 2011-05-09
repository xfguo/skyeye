/* 
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/*
 * 03/27/2011   Michael.Kang  <blackfin.kang@gmail.com>
 */

#ifndef __PPC_DYNCOM_PARALLEL_H__
#define __PPC_DYNCOM_PARALLEL_H__
#include "ppc_dyncom_run.h"
void init_compiled_queue(cpu_t* cpu);
void launch_compiled_queue(cpu_t* cpu, uint32_t pc);

typedef enum{
	PURE_INTERPRET = 0,
	PURE_DYNCOM,
	HYBRID,
	MAX_RUNNING_MODE,
}running_mode_t;

#endif

