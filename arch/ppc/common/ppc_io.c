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
#define UART_IRQ 26

extern byte * ddr_ram;

#define DEFAULT_CCSR_MEM 0xFF700000
#define CCSR_MEM_SIZE 0x100000
#define GET_CCSR_BASE(reg)(((reg >> 8)&0xFFF) << 20)


int FASTCALL ppc_read_effective_word(uint32 addr, uint32 *result)
{
	uint32 p;
	int r;
	if (!(r = ppc_effective_to_physical(current_core, addr, PPC_MMU_READ, &p))) {
		if((p >= GET_CCSR_BASE(gCPU.ccsr)) && (p < (GET_CCSR_BASE(gCPU.ccsr) + CCSR_MEM_SIZE))){
			skyeye_config_t *config = get_current_config();
			*result = config->mach->mach_io_read_word(&gCPU, (p - GET_CCSR_BASE(gCPU.ccsr)));
			//*result = skyeye_config.mach->mach_io_read_word(&gCPU, (p - GET_CCSR_BASE(gCPU.ccsr)));
		}
		#if 1
		else if((p >= boot_rom_start_addr) && (p < (boot_rom_start_addr - 1 + boot_romSize )))
			*result = ppc_word_from_BE(*((int *)&boot_rom[p - boot_rom_start_addr]));
		else if((p >= init_ram_start_addr) && (p < (init_ram_start_addr + init_ram_size)))
                        *result = ppc_word_from_BE(*((int *)&init_ram[p - init_ram_start_addr]));
		else if((p >= 0x0) && (p < (0x0 + DDR_RAM_SIZE))){
			*result = ppc_word_from_BE(*((int *)&ddr_ram[p]));
		}
		else{
			fprintf(stderr,"in %s, can not find address 0x%x,pc=0x%x\n", __FUNCTION__, p, current_core->pc);
			//skyeye_exit(-1);
		}
		#else
		else{
			if(bus_read(32, p, result) != 0){
			}
			*result = ppc_word_from_BE(*result);
		}
		#endif
	}
	return r;
}

int FASTCALL ppc_read_effective_half(uint32 addr, uint16 *result)
{
	uint32 p;
	int r;
	if (!(r = ppc_effective_to_physical(current_core, addr, PPC_MMU_READ, &p))) {
		//ppc_io_read_halfword(&current_core-> p);
		//printf("DBG:ccsr=0x%x,CCSR_BASE=0x%x\n",current_core->ccsr.ccsr,GET_CCSR_BASE(current_core->ccsr.ccsr));
		if((p >= GET_CCSR_BASE(gCPU.ccsr)) && (p < (GET_CCSR_BASE(gCPU.ccsr) + CCSR_MEM_SIZE))){
			//*result = skyeye_config.mach->mach_io_read_halfword(&gCPU, (p - GET_CCSR_BASE(gCPU.ccsr)));
			skyeye_config_t* config = get_current_config();
			*result = config->mach->mach_io_read_halfword(&gCPU, (p - GET_CCSR_BASE(gCPU.ccsr)));
		}
		#if 1
		else if((p >= boot_rom_start_addr) && (p < (boot_rom_start_addr - 1 + boot_romSize )))
			*result = ppc_half_from_BE(*((sint16 *)&boot_rom[p - boot_rom_start_addr]));
		else if((p >= init_ram_start_addr) && (p < (init_ram_start_addr + init_ram_size)))
                        *result = ppc_half_from_BE(*((sint16 *)&init_ram[p - init_ram_start_addr]));
		else if((p >= 0x0) && (p < (0x0 + DDR_RAM_SIZE)))
		{
                        *result = ppc_half_from_BE(*((sint16 *)&ddr_ram[p]));
                }
		#else
		else{
			if(bus_read(16, p, result) != 0){
                        }
                        *result = ppc_half_from_BE(*result);

			//fprintf(stderr,"in %s, can not find address 0x%x,pc=0x%x\n", __FUNCTION__, p, current_core->pc);
			//skyeye_exit(-1);
		}	
		#endif
	}
	return r;
}

int FASTCALL ppc_read_effective_byte(uint32 addr, uint8 *result)
{ 
	uint32 p;
	int r;
	if (!(r = ppc_effective_to_physical(current_core, addr, PPC_MMU_READ, &p))) {
		//ppc_io_read_byte (&current_core-> p);
		//printf("\nDBG:in %s,addr=0x%x,p=0x%x\n", __FUNCTION__, addr,p);
		//printf("DBG:ccsr=0x%x,CCSR_BASE=0x%x\n",current_core->ccsr.ccsr,GET_CCSR_BASE(current_core->ccsr.ccsr));
		if((p >= GET_CCSR_BASE(gCPU.ccsr)) && (p < (GET_CCSR_BASE(gCPU.ccsr) + CCSR_MEM_SIZE))){
			int offset = p - GET_CCSR_BASE(gCPU.ccsr);
			skyeye_config_t* config = get_current_config();
			*result = config->mach->mach_io_read_byte(&gCPU, offset);
			//*result = skyeye_config.mach->mach_io_read_byte(&gCPU, offset);
			//printf("In %s, offset=0x%x, *result=0x%x\n", __FUNCTION__, offset, *result);
			return r;
		}
		#if 1
		else if((p >= boot_rom_start_addr) && (p < (boot_rom_start_addr - 1 + boot_romSize )))
			*result = *((byte *)&boot_rom[p - boot_rom_start_addr]);
		else if((p >= init_ram_start_addr) && (p < (init_ram_start_addr + init_ram_size)))
                        *result = *((byte *)&init_ram[p - init_ram_start_addr]);
		else if((p >= 0x0) && (p < (0x0 + DDR_RAM_SIZE))){
                        *result = *((byte *)&ddr_ram[p]);
                }
		#else
		else{
			if(bus_read(8, p, result) != 0){
			}
			//fprintf(stderr,"in %s, can not find address 0x%x,pc=0x%x\n", __FUNCTION__, p, current_core->pc);
			//skyeye_exit(-1);
		}	
		#endif
	}
	return r;
}


int FASTCALL ppc_write_effective_word(uint32 addr, uint32 data)
{
	uint32 p;
	int r;
	if(addr & 0x3){
		//ppc_exception for unalign
	}
	if (!((r=ppc_effective_to_physical(current_core, addr, PPC_MMU_WRITE, &p)))) {
		if(p >= GET_CCSR_BASE(gCPU.ccsr) && p <(GET_CCSR_BASE(gCPU.ccsr) + CCSR_MEM_SIZE)){
			int offset = p - GET_CCSR_BASE(gCPU.ccsr);
			//skyeye_config.mach->mach_io_write_word(&gCPU, offset, data);
			skyeye_config_t* config = get_current_config();
			config->mach->mach_io_write_word(&gCPU, offset, data);
			//printf("DBG:write to CCSR,value=0x%x,offset=0x%x,pc=0x%x\n", data, offset,current_core->pc);
		}
		#if 1
		else if((p >= boot_rom_start_addr) && (p < (boot_rom_start_addr + boot_romSize )))

                        *((int *)&boot_rom[p - boot_rom_start_addr]) = ppc_word_to_BE(data);
		else if((p >= init_ram_start_addr) && (p < (init_ram_start_addr + init_ram_size)))
			*((int *)&init_ram[p - init_ram_start_addr]) = ppc_word_to_BE(data);
		else if((p >= 0x0) && (p < (0x0 + DDR_RAM_SIZE))){
			*((int *)&ddr_ram[p]) = ppc_word_to_BE(data);
		}
		#else
                else{
			bus_write(32, p, ppc_word_to_BE(data));	
                        //fprintf(stderr,"in %s, can not find address 0x%x,pc=0x%x,ccsr_base=0x%x, ccsr=0x%x\n", __FUNCTION__, p, current_core->pc, GET_CCSR_BASE(gCPU.ccsr), gCPU.ccsr);
                        //skyeye_exit(-1);
                }
		#endif
	}
	return r;
}

int FASTCALL ppc_write_effective_half(uint32 addr, uint16 data)
{	
	uint32 p;
	int r;
	if (!((r=ppc_effective_to_physical(current_core, addr, PPC_MMU_WRITE, &p)))) {
		if(p >= GET_CCSR_BASE(gCPU.ccsr) && p <(GET_CCSR_BASE(gCPU.ccsr) + CCSR_MEM_SIZE)){
			int offset = p - GET_CCSR_BASE(gCPU.ccsr);
			//skyeye_config.mach->mach_io_write_halfword(&gCPU, offset, data);
			skyeye_config_t* config = get_current_config();
                        config->mach->mach_io_write_halfword(&gCPU, offset, data);
			//printf("DBG:write to CCSR,value=0x%x,offset=0x%x,pc=0x%x\n", data, offset,current_core->pc);
		}
		#if 1
		else if((p >= boot_rom_start_addr) && (p < (boot_rom_start_addr + boot_romSize )))
                        *((sint16 *)&boot_rom[p - boot_rom_start_addr]) = ppc_half_to_BE(data);
		else if((p >= init_ram_start_addr) && (p < (init_ram_start_addr + init_ram_size)))
			*((sint16 *)&init_ram[p - init_ram_start_addr]) = ppc_half_to_BE(data);
		else if((p >= 0x0) && (p < (0x0 + DDR_RAM_SIZE))){
                        *((sint16 *)&ddr_ram[p]) = ppc_half_to_BE(data);
		}
		#else
                else{
			bus_write(16, p, ppc_half_to_BE(data));	
                        //fprintf(stderr,"in %s, can not find address 0x%x,pc=0x%x\n", __FUNCTION__, p, current_core->pc);
                        //skyeye_exit(-1);
                }
		#endif
	}
	return r;
}

int FASTCALL ppc_write_effective_byte(uint32 addr, uint8 data)
{
	uint32 p;
        int r;
        if (!((r=ppc_effective_to_physical(current_core, addr, PPC_MMU_WRITE, &p)))) {
		//ppc_io_write_byte (&current_core-> p, data);

                //printf("DBG:in %s,addr=0x%x,p=0x%x, data=0x%x, pc=0x%x\n", __FUNCTION__, addr,p, data, current_core->pc);
                //printf("DBG:ccsr=0x%x,CCSR_BASE=0x%x",current_core->ccsr.ccsr,GET_CCSR_BASE(current_core->ccsr.ccsr));
                if(p >= GET_CCSR_BASE(gCPU.ccsr) && p <(GET_CCSR_BASE(gCPU.ccsr) + CCSR_MEM_SIZE)){
                        int offset = p - GET_CCSR_BASE(gCPU.ccsr);
			//skyeye_config.mach->mach_io_write_byte(&gCPU, offset, data);
			skyeye_config_t* config = get_current_config();
			config->mach->mach_io_write_byte(&gCPU, offset, data);
			return r;
                        //printf("DBG:write to CCSR,value=0x%x,offset=0x%x\n", data, offset);
                }
		#if 1
                else if((p >= boot_rom_start_addr) && (p < (boot_rom_start_addr + boot_romSize )))

                        *((byte *)&boot_rom[p - boot_rom_start_addr]) = data;
                else if((p >= init_ram_start_addr) && (p < (init_ram_start_addr + init_ram_size)))
                        *((byte *)&init_ram[p - init_ram_start_addr]) = data;
		else if((p >= 0x0) && (p < (0x0 + DDR_RAM_SIZE))){                         
			*((byte *)&ddr_ram[p]) = data;    
		}
		#else
                else{
			bus_write(8, p, data);
                        //fprintf(stderr,"in %s, can not find address 0x%x,pc=0x%x,ccsr=0x%x\n", __FUNCTION__, p, current_core->pc, gCPU.ccsr);
                        //skyeye_exit(-1);
                }
		#endif
        }
        return r;
}
