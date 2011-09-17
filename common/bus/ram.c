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
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "skyeye_config.h"
#include "assert.h"
#include "bank_defs.h"
#include "skyeye_ram.h"
#include "skyeye_arch.h"
#include "skyeye_pref.h"
#include "skyeye_swapendian.h"
#include "skyeye_mm.h"

/* All the memory including rom and dram */

/**
* @brief The whole memory including rom and dram
*/
static mem_state_t global_memory;

/**
* @brief byte read function of ram
*
* @param addr
*
* @return 
*/
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
	generic_arch_t* arch_instance = get_arch_instance(NULL);

	/* judge the alignment */
	if(arch_instance->alignment == Align)
	{
		data = memory->rom[global_mbp -
			      	memmap->mem_banks][(addr -
								   global_mbp->
							   		addr) >> 2];
	}
	else if(arch_instance->alignment == UnAlign)
	{
		uint8_t *base = &global_memory.rom[global_mbp -memmap->mem_banks][0];
		data = base[addr - global_mbp->addr];
	}
	/* judge the endianess */
	if(arch_instance->endianess == Little_endian)
	{
		offset = (((uint32_t) arch_instance->endianess * 3) ^ (addr & 3)) << 3;	/* bit offset into the word */
		return (data >> offset & 0xffL);
	}
	else if(arch_instance->endianess == Big_endian)
	{
		return data;	
	}

}

/**
* @brief the half read function of the data
*
* @param addr
*
* @return 
*/
static uint32_t
mem_read_halfword (uint32_t addr)
{
	uint32_t data, offset;
	mem_bank_t * global_mbp = bank_ptr(addr);
	if(global_mbp == NULL){
		printf("Can not find the suitable bank for the address 0x%x\n", addr);
		return 0;
	}

	mem_config_t * memmap = get_global_memmap();
	generic_arch_t* arch_instance = get_arch_instance(NULL);

	/* judge the alignment */
	if(arch_instance->alignment == Align)
	{
		data = global_memory.rom[global_mbp -
				      memmap->mem_banks][(addr -
								   global_mbp->
								   addr) >> 2];
	}
	else if(arch_instance->alignment == UnAlign)
	{
		uint8_t *base = &global_memory.rom[global_mbp -memmap->mem_banks][0];
		data = *(uint16_t *)(&base[addr - global_mbp->addr]);
	}

	/* judge the endianess */
	if(arch_instance->endianess == Little_endian)
	{
		offset = (((uint32_t) arch_instance->endianess * 2) ^ (addr & 2)) << 3;	/* bit offset into the word */
		return (data >> offset) & 0xffff;
	}
	else if(arch_instance->endianess == Big_endian)
	{
		return half_from_BE(data);
	}
}

/**
* @brief Word read of the ram
*
* @param addr
*
* @return 
*/
static uint32_t
mem_read_word (uint32_t addr)
{
	uint32_t data;
	mem_bank_t * global_mbp = bank_ptr(addr);
	if(global_mbp == NULL){
		printf("Can not find the suitable bank for the address 0x%x\n", addr);
		return 0;
	}

	mem_config_t * memmap = get_global_memmap();
	generic_arch_t* arch_instance = get_arch_instance(NULL);

	/* judge the alignment */
	if(arch_instance->alignment == Align)
	{
		data = global_memory.rom[global_mbp -
					  memmap->mem_banks][(addr -
								   global_mbp->
								   addr) >> 2];
	}
	else if(arch_instance->alignment == UnAlign)
	{
		uint8_t *base = &global_memory.rom[global_mbp -memmap->mem_banks][0];
		data = *(uint32_t *)(&base[addr - global_mbp->addr]);
	}

	/* judge the endianess */
	if(arch_instance->endianess == Little_endian)
		return data;
	else if(arch_instance->endianess == Big_endian)
		return word_from_BE(data);
	
}

/**
* @brief the byte write of the ram
*
* @param addr the written address
* @param data the written data
*/
static void
mem_write_byte (uint32_t addr, uint32_t data)
{
	uint32_t *temp, offset;

	mem_bank_t * global_mbp = bank_ptr(addr);
	if(global_mbp == NULL){
		printf("Can not find the suitable bank for the address 0x%x\n", addr);
		return 0;
	}

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
	generic_arch_t* arch_instance = get_arch_instance(NULL);

	/* judge the alignment */
	if(arch_instance->alignment == Align)
	{
		temp = &global_memory.rom[global_mbp -
					   memmap->mem_banks][(addr -
									global_mbp->
									addr) >> 2];
	}
	else if(arch_instance->alignment == UnAlign)
	{
		uint8_t *base = &global_memory.rom[global_mbp -memmap->mem_banks][0];
		temp = (uint32_t *)(&base[addr - global_mbp->addr]);
	}

	/* judge the endianess */
	if(arch_instance->endianess == Little_endian)
	{
		offset = (((uint32_t) arch_instance->endianess * 3) ^ (addr & 3)) << 3;/* bit offset into the word */
		*temp = (*temp & ~(0xffL << offset)) | ((data & 0xffL) << offset);
	}
	else if(arch_instance->endianess == Big_endian)
		*(uint8_t *)temp = data;
}

/**
* @brief the halfword write function
*
* @param addr the written address
* @param data the written data
*/
static void
mem_write_halfword (uint32_t addr, uint32_t data)
{
	uint32_t *temp, offset;
	mem_bank_t * global_mbp = bank_ptr(addr);
	if(global_mbp == NULL){
		printf("Can not find the suitable bank for the address 0x%x\n", addr);
		return 0;
	}

	mem_config_t * memmap = get_global_memmap();

#ifdef DBCT
	if (!skyeye_config.no_dbct) {
		tb_setdirty (arch_instance, addr, global_mbp);
	}
#endif
	generic_arch_t* arch_instance = get_arch_instance(NULL);

	/* judge the alignment */
	if(arch_instance->alignment == Align)
	{
		temp = &global_memory.rom[global_mbp -
					   memmap->mem_banks][(addr -
									global_mbp->
									addr) >> 2];
	}
	else if(arch_instance->alignment == UnAlign)
	{
		uint8_t *base = &global_memory.rom[global_mbp -memmap->mem_banks][0];
		temp = (uint32_t *)(&base[addr - global_mbp->addr]);
	}

	/* judge the endianess */
	if(arch_instance->endianess == Little_endian)
	{
		offset = (((uint32_t) arch_instance->endianess * 2) ^ (addr & 2)) << 3;	/* bit offset into the word */
		*temp = (*temp & ~(0xffffL << offset)) | ((data & 0xffffL) << offset);
	}
	else if(arch_instance->endianess == Big_endian)
		*(uint16_t *)temp = half_to_BE(data);
}

/**
* @brief the word write function of the ram
*
* @param addr the written address
* @param data the written data
*/
static void
mem_write_word (uint32_t addr, uint32_t data)
{
	uint32_t *temp;
	mem_bank_t * global_mbp = bank_ptr(addr);
	if(global_mbp == NULL){
		printf("Can not find the suitable bank for the address 0x%x\n", addr);
		return 0;
	}

	mem_config_t * memmap = get_global_memmap();

#ifdef DBCT
	if (!skyeye_config.no_dbct) {
		//teawater add for arm2x86 2005.03.18----------------------------------
		tb_setdirty (arch_instance, addr, global_mbp);
	}
#endif
	generic_arch_t* arch_instance = get_arch_instance(NULL);

	/* judge the alignment */
	if(arch_instance->alignment == Align)
	{
		temp = &global_memory.rom[global_mbp -
				   		memmap->mem_banks][(addr -
								global_mbp->addr) >> 2];
	}	
	else if(arch_instance->alignment == UnAlign)
	{
		uint8_t *base = &global_memory.rom[global_mbp -memmap->mem_banks][0];
		temp = (uint32_t *)(&base[addr - global_mbp->addr]);
	}

	/* judge the endianess */
	if(arch_instance->endianess == Little_endian)
		*temp = data;
	else if(arch_instance->endianess == Big_endian)
		*temp = word_to_BE(data);
}
/*
 * allocate memory space for the banks
 */

/**
* @brief The memory initialization that do memory allocation
*
* @return the exception
*/
exception_t
mem_reset ()
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
	
	/* if direct mmap access, we just don't use membanks so we ignore the next part */
	if (get_skyeye_exec_info()->mmap_access)
		return ;

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
		global_memory.rom[bank] = skyeye_mm (mb[bank].len);
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

/**
* @brief The generic memory read function
*
* @param size data width
* @param offset data offset
* @param value the return value of read from
*
* @return the flash used to indicate the success or failure
*/
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

/**
* @brief the write function of the ram
*
* @param size the data width
* @param offset the data offset
* @param value the data value written to
*
* @return the flag
*/
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

/**
* @brief get the host address for a guest address
*
* @param guest_addr the guest address
*
* @return the host address for the given guest address
*/
unsigned long get_dma_addr(unsigned long guest_addr){
        /* checking for direct mmap access in user mode */
	if(get_skyeye_exec_info()->mmap_access)
	{
		printf("In %s, guest_addr=0x%lx, \n", __FUNCTION__, (unsigned long) guest_addr);
		return guest_addr;
	}

	unsigned char * host_addr;
        mem_bank_t * global_mbp = bank_ptr(guest_addr);
        mem_config_t * memmap = get_global_memmap();
	if(global_mbp == NULL){
		fprintf(stderr, "Can not find the bank for the address 0x%lx\n", guest_addr);
		return 0;
	}
	
	host_addr = &global_memory.rom[global_mbp - memmap->mem_banks][(guest_addr - global_mbp->addr) >> 2];
        printf("In %s, host_addr=0x%llx, \n", __FUNCTION__, (unsigned long)host_addr);
        return (unsigned long)host_addr;
}


/**
* @brief the warnning function of the read-only memory
*
* @param size the data width
* @param offset the data offset
* @param value the data written to
*
* @return 
*/
char warn_write(short size, int offset, uint32_t value){
	SKYEYE_ERR("Read-only ram\n");
}

/**
* @brief get the whole memory block
*
* @return 
*/
mem_state_t * get_global_memory(){
	return &global_memory;
}

/**
* @brief save all the memory data to a file
*
* @param dir the location of saved memory data
*
* @return the result of operation
*/
int save_mem_to_file(char *dir)
{
	int i,j,bank,ret = 0;
	mem_config_t *mc = get_global_memmap();
	int num = mc->current_num;
	char buf[100];
	FILE *fp;

	if(access(dir, 0) == -1){
		if(mkdir(dir, 0777)){
			printf("create dir %s failed\n", dir);
			return 0;
		}
	}

	for (i = 0; i < num; i++) {
		bank = i;
		ret = 0;

		strcpy(buf, dir);
		sprintf(buf+strlen(dir), "/ram%d", i);
		fp = fopen(buf, "wb");
		if(fp == NULL)
			printf("can't create a mem copy file %s \n", buf);

		fprintf(fp, "%d=%d\n", bank, global_memory.rom_size[bank]);

		do{
			ret += fwrite(global_memory.rom[bank] + ret, 1, global_memory.rom_size[bank], fp);
		}while(global_memory.rom_size[bank] - ret > 0);

		fclose(fp);
	}
}


/**
* @brief load all the memory data from a saved image file
*
* @param dir the location of the saved memory image file
*
* @return 
*/
int load_mem_from_file(char *dir)
{

	int i,j,bank,ret = 0;
	mem_config_t *mc = get_global_memmap();
	int num = mc->current_num;
	char buf[100];
	char tmp[100];
	char tmp2[100];
	FILE *fp;

	for (i = 0; i < num; i++) {
		bank = i;
		ret = 0;

		strcpy(buf, dir);
		sprintf(buf+strlen(buf), "/ram%d", i);
		fp = fopen(buf, "rb");
		if(fp == NULL){
			printf("can't find a mem copy file %s, may be it does not exist \n", buf);
			return 0;
		}

		fgets(tmp, 100, fp);
		sprintf(tmp2, "%d=%d\n", bank, global_memory.rom_size[bank]);

		if(!strcmp(tmp2, tmp))
			do{
				ret += fread(global_memory.rom[bank] + ret, 1, global_memory.rom_size[bank], fp);
			}while(global_memory.rom_size[bank] - ret > 0);
		else
			printf("different size of a bank\n");

		fclose(fp);
	}
}
