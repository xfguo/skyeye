/*
        ram.c - necessary ram module definition for skyeye
        Copyright (C) 2003 Skyeye Develop Group
        for help please send mail to <skyeye-developer@lists.gro.clinux.org>

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
 * 12/16/2006   Michael.Kang  <blackfin.kang@gmail.com>
 */

#include "skyeye_config.h"
#include "assert.h"
#include "bank_defs.h"
#include "skyeye_ram.h"
#include "skyeye_arch.h"
#include "skyeye_pref.h"

/* All the memory including rom and dram */
static mem_state_t global_memory;
static uint32_t
mem_read_byte (uint32_t addr)
{
	uint32_t data, offset;
	mem_bank_t * global_mbp = bank_ptr(addr);
	if(global_mbp == NULL){
		printf("Can not find the suitable bank for the address 0x%x\n", addr);
		return 0;
	}
	mem_config_t * memmap = get_global_memmap();
	mem_state_t * memory = get_global_memory();
	data = memory->rom[global_mbp -
			      memmap->mem_banks][(addr -
							   global_mbp->
							   addr) >> 2];
	//printf("In %s, banks=0x%x,offset=0x%x\n", __FUNCTION__, global_mbp - memmap->mem_banks, (addr - global_mbp-> addr) >> 2);
	generic_arch_t* arch_instance = get_arch_instance(NULL);
	offset = (((uint32_t) arch_instance->endianess * 3) ^ (addr & 3)) << 3;	/* bit offset into the word */
	//printf("In %s,data=0x%x\n, offset=0x%x, addr=0x%x\n", __FUNCTION__, data, offset, addr);
	return (data >> offset & 0xffL);
}

static uint32_t
mem_read_halfword (uint32_t addr)
{
	uint32_t data, offset;
	mem_bank_t * global_mbp = bank_ptr(addr);
	mem_config_t * memmap = get_global_memmap();
	data = global_memory.rom[global_mbp -
			      memmap->mem_banks][(addr -
							   global_mbp->
							   addr) >> 2];

	generic_arch_t* arch_instance = get_arch_instance(NULL);
	offset = (((uint32_t) arch_instance->endianess * 2) ^ (addr & 2)) << 3;	/* bit offset into the word */

	return (data >> offset) & 0xffff;
}

static uint32_t
mem_read_word (uint32_t addr)
{
	uint32_t data;
	mem_bank_t * global_mbp = bank_ptr(addr);
	mem_config_t * memmap = get_global_memmap();
	data = global_memory.rom[global_mbp -
			      memmap->mem_banks][(addr -
							   global_mbp->
							   addr) >> 2];
	return data;
}

static void
mem_write_byte (uint32_t addr, uint32_t data)
{
	uint32_t *temp, offset;

	mem_bank_t * global_mbp = bank_ptr(addr);
	mem_config_t * memmap = get_global_memmap();
	mem_state_t* memory = get_global_memory();
#ifdef DBCT
	if (!skyeye_config.no_dbct) {
		//teawater add for arm2x86 2005.03.18----------------------------------
		tb_setdirty (arch_instance, addr, global_mbp);
	}
#endif
	//printf("In %s, addr=0x%x,banks=0x%x,offset=0x%x\n", __FUNCTION__, addr, global_mbp - memmap->mem_banks, (addr - global_mbp-> addr) >> 2);
	assert(global_mbp != NULL);
	assert(memmap != NULL);	
	temp = &global_memory.rom[global_mbp -
			       memmap->mem_banks][(addr -
							    global_mbp->
							    addr) >> 2];
	generic_arch_t* arch_instance = get_arch_instance(NULL);
	offset = (((uint32_t) arch_instance->endianess * 3) ^ (addr & 3)) << 3;
	/* bit offset into the word */
	//printf("In %s, temp=0x%x,data=0x%x\n", __FUNCTION__, temp, *temp);
	//printf("In %s, temp=0x%x\n", __FUNCTION__, temp);
	*temp = (*temp & ~(0xffL << offset)) | ((data & 0xffL) << offset);
}

static void
mem_write_halfword (uint32_t addr, uint32_t data)
{
	unsigned long *temp, offset;
	mem_bank_t * global_mbp = bank_ptr(addr);
	mem_config_t * memmap = get_global_memmap();

#ifdef DBCT
	if (!skyeye_config.no_dbct) {
		tb_setdirty (arch_instance, addr, global_mbp);
	}
#endif
	temp = &global_memory.rom[global_mbp -
			       memmap->mem_banks][(addr -
							    global_mbp->
							    addr) >> 2];
	generic_arch_t* arch_instance = get_arch_instance(NULL);
	offset = (((uint32_t) arch_instance->endianess * 2) ^ (addr & 2)) << 3;	/* bit offset into the word */

	*temp = (*temp & ~(0xffffL << offset)) | ((data & 0xffffL) << offset);
}

static void
mem_write_word (uint32_t addr, uint32_t data)
{
	mem_bank_t * global_mbp = bank_ptr(addr);
	mem_config_t * memmap = get_global_memmap();

#ifdef DBCT
	if (!skyeye_config.no_dbct) {
		//teawater add for arm2x86 2005.03.18----------------------------------
		tb_setdirty (arch_instance, addr, global_mbp);
	}
#endif
	global_memory.rom[global_mbp -
		       memmap->mem_banks][(addr -
						    global_mbp->addr) >> 2] =
	data;
}
/*
 * allocate memory space for the banks
 */
exception_t
mem_reset (void * state)
{
	int i, num, bank;
	FILE *f;
	unsigned char *p;
	int s;
	uint32_t swap;

	/*
	 * we will clear the previous config when memory reset
	 */
	mem_config_t *mc = get_global_memmap();

	mem_state_t * mem = &global_memory;
	//memset(mem, 0, sizeof(mem_state_t));	
	uint32_t* tmp;
	/*tmp = skyeye_mm(0x100);
	printf("tmp=0x%lx\n", tmp);
	tmp[5] = 0x123;*/
	mem_bank_t *mb = mc->mem_banks;
	num = mc->current_num;
	/* 
	 * scan all the bank in the memory map and allocate memory for memory bank	   */
	for (i = 0; i < num; i++) {
		bank = i;
		/* we do not need to allocate memory for IO bank */
		if(mb[bank].type == MEMTYPE_IO)
			continue;
		if (global_memory.rom[bank]){
			skyeye_free (global_memory.rom[bank]);
		}
		global_memory.rom_size[bank] = 0;
		//chy 2003-09-21: if mem type =MEMTYPE_IO, we need not malloc space for it.
		global_memory.rom_size[bank] = mb[bank].len;
		//global_memory.rom[bank] = skyeye_mm (mb[bank].len);
		global_memory.rom[bank] = malloc (mb[bank].len);
	#if 0 /* FIXME we can not use skyeye_mm in the above line or will segmentation fault. */
		printf("len=0x%x, \n", mb[bank].len);
		//tmp = skyeye_mm (mb[bank].len);
		//tmp = skyeye_mm (0x100000);
		tmp = malloc(0x400000);
		printf("tmp=0x%lx, sizeof(tmp)=0x%x\n", tmp, sizeof(tmp));
		printf("tmp=0x%lx\n", tmp);
        	tmp[5] = 0x123;
		printf("len=0x%x, \n", mb[bank].len);
		printf("global_memory.rom[0]=0x%x, \n", global_memory.rom[bank]);
		printf("global_memory.rom[0]=0x%x, global_memory.rom[0][0]=0x%x\n", global_memory.rom[bank], global_memory.rom[bank][0]);
	#endif
		if (!global_memory.rom[bank]) {
			fprintf (stderr,
				 "SKYEYE: mem_reset: Error allocating mem for bank number %d.\n", bank);
			//skyeye_exit (-1);
			return Malloc_exp;
		}
	#if 1 
		if (mb[bank].filename
			    && (f = fopen (mb[bank].filename, "rb"))) {
			if (fread(global_memory.rom[bank], 1, mb[bank].len,
				     f) <= 0) {
				perror ("fread");
				fprintf (stderr, "Failed to load '%s'\n",
						 mb[bank].filename);
					skyeye_exit (-1);
				}
				fclose (f);

				p = (char *) global_memory.rom[bank];
				s = 0;
				sky_pref_t* pref = get_skyeye_pref();	
				while (s < global_memory.rom_size[bank]) {
					//if (arch_instance->big_endian == HIGH)	/*big enddian? */
					if(pref->endian == Big_endian)
						swap = ((uint32_t) p[3]) |
							(((uint32_t) p[2]) <<
							 8) | (((uint32_t)
								p[1]) << 16) |
							(((uint32_t) p[0]) <<
							 24);
					else
						swap = ((uint32_t) p[0]) |
							(((uint32_t) p[1]) <<
							 8) | (((uint32_t)
								p[2]) << 16) |
							(((uint32_t) p[3]) <<
							 24);
					*(uint32_t *) p = swap;
					p += 4;
					s += 4;
				}
				/*ywc 2004-03-30 */
				//printf("Loaded ROM %s\n", mb[bank].filename);
				if (mb[bank].type == MEMTYPE_FLASH) {
					printf ("Loaded FLASH %s\n",
						mb[bank].filename);
				}
				else if (mb[bank].type == MEMTYPE_RAM) {
					printf ("Loaded RAM   %s\n",
						mb[bank].filename);
				}
				else if (mb[bank].type == MEMTYPE_ROM) {
					printf ("Loaded ROM   %s\n",
						mb[bank].filename);
				}

			}
			else if (mb[bank].filename[0] != '\0') {
				perror (mb[bank].filename);
				fprintf (stderr,
					 "bank %d, Couldn't open boot ROM %s - execution will "
					 "commence with the debuger.\n", bank,
					 mb[bank].filename);
				skyeye_exit (-1);
			}
#endif



	}/*end  for(i = 0;i < num; i++) */

}

char mem_read(short size, int offset, uint32_t * value){
	void * state;
	switch(size){
		case 8:
			*value = (uint8_t)mem_read_byte (offset);
			break;
		case 16:
			*value = (uint16_t)mem_read_halfword(offset);
			break;
		case 32:
			*value = mem_read_word(offset);
			break;
		default:
			fprintf(stderr, "In %s, invalid data length %d\n", __FUNCTION__, size);
			return -1;
	}
	return 0;

}

char mem_write(short size, int offset, uint32_t value){
	switch(size){
		case 8:
                        mem_write_byte (offset, value);
                        break;
                case 16:
                        mem_write_halfword(offset, value);
                        break;
                case 32:
                        mem_write_word(offset, value);
                        break;
                default:
			fprintf(stderr, "In %s, invalid data length %d\n", __FUNCTION__, size);
                        return -1;
	}
	return 0;
}

/* 
 * Here, we translate an address from guest machine 
 * to the address of host machine, you can use it as dma transfer. 
 */
#if 0
unsigned char * get_dma_addr(unsigned long guest_addr){
        unsigned char * host_addr;
        mem_bank_t * global_mbp = bank_ptr(guest_addr);
        mem_config_t * memmap = get_global_memmap();
        host_addr = &global_memory.rom[global_mbp -
                              memmap->mem_banks][(guest_addr -
                                                           global_mbp->
                                                           addr) >> 2];
	printf("In %s, host_addr=0x%llx, \n", __FUNCTION__, (unsigned long)host_addr);
        return host_addr;
}
#endif
unsigned long get_dma_addr(unsigned long guest_addr){
        unsigned char * host_addr;
        mem_bank_t * global_mbp = bank_ptr(guest_addr);
        mem_config_t * memmap = get_global_memmap();
	if(global_mbp == NULL){
		fprintf(stderr, "Can not find the bank for the address 0x%x\n", guest_addr);
		return 0;
	}
        host_addr = &global_memory.rom[global_mbp -
                              memmap->mem_banks][(guest_addr -
                                                           global_mbp->
                                                           addr) >> 2];
        printf("In %s, host_addr=0x%llx, \n", __FUNCTION__, (unsigned long)host_addr);
        return (unsigned long)host_addr;
}


char warn_write(short size, int offset, uint32_t value){
	SKYEYE_ERR("Read-only ram\n");
}
mem_state_t * get_global_memory(){
	return &global_memory;
}
