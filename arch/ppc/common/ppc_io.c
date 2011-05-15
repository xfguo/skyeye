/*
        ppc_io.c - necessary arm definition for skyeye debugger
        Copyright (C) 2003-2007 Skyeye Develop Group
        for help please send mail to <skyeye-developer@lists.sf.linuxforum.net>

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
        Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
/*
 * 04/26/2006   Michael.Kang  <blackfin.kang@gmail.com>
 */

#include <stdint.h>

#include "skyeye_config.h"

#include "types.h"
#include "ppc_mmu.h"
#include "ppc_memory.h"
#include "ppc_e500_exc.h"
#include "ppc_cpu.h"
#include "sysendian.h"
#include "bank_defs.h"
#define UART_IRQ 26

/* For e500 */
//#define DEFAULT_CCSR_MEM 0xFF700000
//#define CCSR_MEM_SIZE 0x100000
//#define GET_CCSR_BASE(reg)(((reg >> 8)&0xFFF) << 20)

/* For e600 */
//#define DEFAULT_CCSR_MEM 0xF8000000
//#define CCSR_MEM_SIZE 0x100000
//#define GET_CCSR_BASE(reg)(((reg >> 8)&0xFFFF) << 16)
static inline bool_t in_ccsr_range(uint32 p)
{
	PPC_CPU_State *cpu = get_current_cpu();
	e500_core_t *current_core = get_current_core();
	return (p >= current_core->get_ccsr_base(cpu->ccsr))
	    && (p <
		(current_core->get_ccsr_base(cpu->ccsr) +
		 current_core->ccsr_size));
}

int FASTCALL ppc_read_effective_word(uint32 addr, uint32 * result)
{
	e500_core_t *current_core = get_current_core();
	PPC_CPU_State *cpu = get_current_cpu();
	uint32 p;
	int r;
	if (!
	    (r =
	     ppc_effective_to_physical(current_core, addr, PPC_MMU_READ, &p))) {
		if (in_ccsr_range(p)) {
			skyeye_config_t *config = get_current_config();
			*result =
			    config->mach->mach_io_read_word(cpu,
							    (p -
							     current_core->
							     get_ccsr_base(cpu->
									   ccsr)));
		} else {
			if (bus_read(32, p, result) != 0) {
			}
		}
	}
	return r;
}

int FASTCALL ppc_read_effective_half(uint32 addr, uint16 * result)
{
	e500_core_t *current_core = get_current_core();
	PPC_CPU_State *cpu = get_current_cpu();
	uint32 p;
	int r;
	if (!
	    (r =
	     ppc_effective_to_physical(current_core, addr, PPC_MMU_READ, &p))) {
		//ppc_io_read_halfword(&current_core-> p);
		//printf("DBG:ccsr=0x%x,CCSR_BASE=0x%x\n",current_core->ccsr.ccsr,GET_CCSR_BASE(current_core->ccsr.ccsr));
		if (in_ccsr_range(p)) {
			//*result = skyeye_config.mach->mach_io_read_halfword(&gCPU, (p - GET_CCSR_BASE(gCPU.ccsr)));
			skyeye_config_t *config = get_current_config();
			*result =
			    config->mach->mach_io_read_halfword(cpu,
								(p -
								 current_core->
								 get_ccsr_base
								 (cpu->ccsr)));
		} else {
			if (bus_read(16, p, result) != 0) {
			}
			//fprintf(stderr,"in %s, can not find address 0x%x,pc=0x%x\n", __FUNCTION__, p, current_core->pc);
			//skyeye_exit(-1);
		}
	}
	return r;
}

int ppc_read_effective_byte(uint32 addr, uint8 * result)
{
	e500_core_t *current_core = get_current_core();
	PPC_CPU_State *cpu = get_current_cpu();
	uint32 p;
	int r;
	if (!
	    (r =
	     ppc_effective_to_physical(current_core, addr, PPC_MMU_READ, &p))) {
		//ppc_io_read_byte (&current_core-> p);
		//printf("\nDBG:in %s,addr=0x%x,p=0x%x\n", __FUNCTION__, addr,p);
		//printf("DBG:ccsr=0x%x,CCSR_BASE=0x%x\n",current_core->ccsr.ccsr,GET_CCSR_BASE(current_core->ccsr.ccsr));
		if (in_ccsr_range(p)) {
			int offset = p - current_core->get_ccsr_base(cpu->ccsr);
			skyeye_config_t *config = get_current_config();
			*result = config->mach->mach_io_read_byte(cpu, offset);
			//*result = skyeye_config.mach->mach_io_read_byte(&gCPU, offset);
			//printf("In %s, offset=0x%x, *result=0x%x\n", __FUNCTION__, offset, *result);
			return r;
		} else {
			if (bus_read(8, p, result) != 0) {
			}
			//fprintf(stderr,"in %s, can not find address 0x%x,pc=0x%x\n", __FUNCTION__, p, current_core->pc);
			//skyeye_exit(-1);
		}
	}
	return r;
}

int FASTCALL ppc_write_effective_word(uint32 addr, uint32 data)
{
	PPC_CPU_State *cpu = get_current_cpu();
	e500_core_t *current_core = get_current_core();
	uint32 p;
	int r;
	if (addr & 0x3) {
		//ppc_exception for unalign
	}
	if (!
	    ((r =
	      ppc_effective_to_physical(current_core, addr, PPC_MMU_WRITE,
					&p)))) {
		if (in_ccsr_range(p)) {
			int offset = p - current_core->get_ccsr_base(cpu->ccsr);
			//skyeye_config.mach->mach_io_write_word(&gCPU, offset, data);
			skyeye_config_t *config = get_current_config();
			config->mach->mach_io_write_word(cpu, offset, data);
			//printf("DBG:write to CCSR,value=0x%x,offset=0x%x,pc=0x%x\n", data, offset,current_core->pc);
		} else {
			bus_write(32, p, data);
			//fprintf(stderr,"in %s, can not find address 0x%x,pc=0x%x,ccsr_base=0x%x, ccsr=0x%x\n", __FUNCTION__, p, current_core->pc, GET_CCSR_BASE(gCPU.ccsr), gCPU.ccsr);
			//skyeye_exit(-1);
		}
	}
	return r;
}

int FASTCALL ppc_write_effective_half(uint32 addr, uint16 data)
{
	uint32 p;
	int r;
	PPC_CPU_State *cpu = get_current_cpu();
	e500_core_t *current_core = get_current_core();
	if (!
	    ((r =
	      ppc_effective_to_physical(current_core, addr, PPC_MMU_WRITE,
					&p)))) {
		if (in_ccsr_range(p)) {
			int offset = p - current_core->get_ccsr_base(cpu->ccsr);
			//skyeye_config.mach->mach_io_write_halfword(&gCPU, offset, data);
			skyeye_config_t *config = get_current_config();
			config->mach->mach_io_write_halfword(cpu, offset, data);
			//printf("DBG:write to CCSR,value=0x%x,offset=0x%x,pc=0x%x\n", data, offset,current_core->pc);
		} else {
			bus_write(16, p, data);
			//fprintf(stderr,"in %s, can not find address 0x%x,pc=0x%x\n", __FUNCTION__, p, current_core->pc);
			//skyeye_exit(-1);
		}
	}
	return r;
}

int FASTCALL ppc_write_effective_byte(uint32 addr, uint8 data)
{
	uint32 p;
	int r;
	e500_core_t *current_core = get_current_core();
	PPC_CPU_State *cpu = get_current_cpu();
	if (!
	    ((r =
	      ppc_effective_to_physical(current_core, addr, PPC_MMU_WRITE,
					&p)))) {
		//ppc_io_write_byte (&current_core-> p, data);

		//printf("DBG:in %s,addr=0x%x,p=0x%x, data=0x%x, pc=0x%x\n", __FUNCTION__, addr,p, data, current_core->pc);
		//printf("DBG:ccsr=0x%x,CCSR_BASE=0x%x",current_core->ccsr.ccsr,GET_CCSR_BASE(current_core->ccsr.ccsr));
		if (in_ccsr_range(p)) {
			int offset = p - current_core->get_ccsr_base(cpu->ccsr);
			//skyeye_config.mach->mach_io_write_byte(&gCPU, offset, data);
			skyeye_config_t *config = get_current_config();
			config->mach->mach_io_write_byte(cpu, offset, data);
			return r;
			//printf("DBG:write to CCSR,value=0x%x,offset=0x%x\n", data, offset);
		} else {
			bus_write(8, p, data);
			//fprintf(stderr,"in %s, can not find address 0x%x,pc=0x%x,ccsr=0x%x\n", __FUNCTION__, p, current_core->pc, gCPU.ccsr);
			//skyeye_exit(-1);
		}
	}
	return r;
}
