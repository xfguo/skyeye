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
* @file exec_info.c
* @brief retrieving information from binary in user mode
* @author Alexis He ahe.krosk@gmail.com
* @version 
* @date 2011-08-11
*/

#include "skyeye_types.h"
#include "skyeye_config.h"
#include <skyeye_log.h>
#include "bank_defs.h"

#include "skyeye_pref.h"
#include "skyeye_exec_info.h"
#include "portable/portable.h"
#include <elf.h>
#include <stdlib.h>
//#include <time.h>

/* Note: most content ported from qemu */
sky_exec_info_t* get_skyeye_exec_info()
{
	sky_pref_t *pref = get_skyeye_pref();
	return &(pref->info);
}

#define PUSH_STACK(value) (bus_write(32, sp, value)); sp += 4;
#define PUSH_AUXV(key, value) (bus_write(32, sp, key)); bus_write(32, sp + 4, value); \
	/*printf("auxv: %08x %02d - %08x %08x\n", sp, key, sp + 4, value);*/ sp += 8;

/* copy environment variable strings into stack */
uint32_t fill_envp(uint32_t sp) {
	sky_exec_info_t *info = get_skyeye_exec_info();
	char* envp = info->exec_envp;
	short envc = info->exec_envc;

	/* Copy envp string to the stack
	  first element of envp <-> higher sp : we copy downward */
	char* curr_envp = envp;
	uint32_t offset, index;

	//printf("exec_info: %d envp string in stack \t\t%08x->", envc, sp);
	for(index = 0; index < envc; index++)
	{
		sp -= strlen(curr_envp) + 1;
		//printf("Start loop %d at sp %08x for %s\n", index, sp, curr_envp);
		offset = 0;
		while (1) {
			bus_write(8, sp + offset, *curr_envp);
			//printf("%c\t%d\tat %08x\n", *curr_envp, *curr_envp, sp + offset);
			if (!*curr_envp)
			{
				curr_envp++;
				break;
			}
			curr_envp++;
			offset++;
		}
	}

	// printf("%08x\n", sp);

	return sp;
}

/* copy arguments strings into stack */
uint32_t fill_argv(uint32_t sp) {
	sky_exec_info_t *info = get_skyeye_exec_info();
	short argc = info->exec_argc;
	char *argv = info->exec_argv;

	//printf("exec_info: %d argv string in stack \t\t%08x->", argc, sp);
	char* curr_argv = argv;
	/* compute the size of the whole argv string */
	uint32_t size_argv = 0;
	uint32_t index;
	for(index = 0; index < argc; index++)
	{
		size_argv += strlen(curr_argv) + 1;
		curr_argv += strlen(curr_argv) + 1;
	}
	/* reset curr_argv to point at the string with the lowest address */
	curr_argv = argv;

	/* set user_argv to the start of the strings, for the return value */
	sp = sp - size_argv;
	uint32_t user_argv = sp;
	//printf("%08x\n", sp);

	/* copy */
	uint32_t offset = 0;
	for(index = 0; index < argc; index++)
	{
		//printf("Start loop %d at sp %08x for %s\n", index, sp + offset, curr_argv);
		while (1) {
			bus_write(8, sp + offset, *curr_argv);
			//printf("%c\t%d\tat %08x\n", *curr_argv, *curr_argv, sp + offset);
			if (!*curr_argv)
			{
				curr_argv++;
				offset++;
				break;
			}
			curr_argv++;
			offset++;
		}
	}
	/* alternate faster copy */
	/*for (offset = 0; offset < size_argv; offset++)
	{
		printf("%c\t%d\tat %08x\n", *(curr_argv + offset), *(curr_argv + offset), sp + offset);
		bus_write(8, sp + offset, *(curr_argv + offset));
	}*/
	//sp = (uintptr_t) user_argv;

	return user_argv;
}

/* read elf binary and retrieve some headers data */
void retrieve_info(char * exec_file, Elf32_Ehdr* elf_header){
	sky_exec_info_t *info = get_skyeye_exec_info();

	FILE* file = fopen(exec_file, "r");
	if(file == NULL){
		fprintf(stderr, "In %s, can not open file %s\n", __FUNCTION__, exec_file);
		exit(-1);
	}
	if (elf_header == NULL)
	{
		Elf32_Ehdr header;
		elf_header = &header;
	}
	if (fread (elf_header, sizeof(Elf32_Ehdr), 1, file) != 1){
		fprintf(stderr, "In %s, could not read the elf reader of %s\n", __FUNCTION__, exec_file);
		exit(-1);
	}

	/* Load address, to compute in the elf loading sequence (need the part before) */
	uint32_t load_addr = -1;
	uint32_t brk = 0;
	uint32_t start_data = -1;
	uint32_t end_data = 0;
	uint32_t start_code = -1;
	uint32_t end_code = 0;
	uint32_t index;
	Elf32_Phdr prog_header;

	for (index = 0; index < elf_header->e_phnum; ++index) {
		if (fread (&prog_header, sizeof(Elf32_Phdr), 1, file) != 1) {
			fprintf(stderr, "In %s, could not read the elf reader of %s\n",
				       	__FUNCTION__, exec_file);
			exit(-1);
		}
		//	printf("PRG %d vaddr %08x\n", index, prog_header.p_vaddr);
		if (prog_header.p_type == PT_LOAD) {
			/* load_addr info */
			if (load_addr > prog_header.p_vaddr)
				load_addr = prog_header.p_vaddr;
			
			/* get data related */
			if (prog_header.p_flags & PF_W) {
				/* FIXME qemu adds a load bias to p_vaddr */
				if (start_data > prog_header.p_vaddr)
					start_data = prog_header.p_vaddr;

				if (end_data < prog_header.p_vaddr + prog_header.p_filesz)
					end_data = prog_header.p_vaddr + prog_header.p_filesz;

				uint32_t v_boundary = prog_header.p_vaddr + prog_header.p_memsz;
				//printf("prog seg %d boundary %x\n", index, v_boundary);
				if (brk < prog_header.p_vaddr + prog_header.p_memsz)
					brk = prog_header.p_vaddr + prog_header.p_memsz;

			}

			/* get code related */
			if (prog_header.p_flags & PF_X) {
				if (start_code > prog_header.p_vaddr)
					start_code = prog_header.p_vaddr;
 
				if (end_code < prog_header.p_vaddr + prog_header.p_filesz)
					end_code = prog_header.p_vaddr + prog_header.p_filesz;

			}
		}
	}

	/* If no data section found */
	if (start_data == -1) {
		start_data = end_data = brk = end_code;
	}

	/* align the brk to a page boundary
	   FIXME may depend on page size */
	if (brk & 0xfff) {
		brk = (brk & ~0xfff) + info->arch_page_size;
		//printf("brk is %x\n", brk);
	}

	info->brk = brk;
	info->load_addr = load_addr;
	info->start_data = start_data;
	info->end_data = end_data;
	info->start_code = start_code;
	info->end_code = end_code;
	info->entry = elf_header->e_entry;

	//printf("brk %p load_addr %p start_data %p end_data %p start_code %p end_code %p\n", brk, load_addr, start_data, end_data, start_code, end_code);

	fclose(file);
}


#define AUXV_SIZE 14

/* set the argv, envp, and auxv tables. return value can be discarded */
uint32_t set_tables(uint32_t sp, char * user_argv, char* user_envp, Elf32_Ehdr* elf_header)
{
	sky_exec_info_t *info = get_skyeye_exec_info();

	char* envp = info->exec_envp;
	short argc = info->exec_argc;
	char *argv = info->exec_argv;
	short envc = info->exec_envc;

	uint32_t index;
	/* FIXME may need to align the stack during the filling to a 16bit boundary */
	
	/* fill the stack upwards */
	/* Arguments count */
	//printf("exec_info: argc: %08x %d\n", sp, argc);
	PUSH_STACK(argc);
	/* Arguments ptr */
	char *curr_argv = user_argv; /* curr_argv points at user space, argv at physical addr */
	for (index = 0; index < argc; index++)
	{
		//printf("exec_info: argv: %08x %08x %s\n", sp, curr_argv, argv);
		PUSH_STACK((uintptr_t) curr_argv);
		curr_argv += strlen(argv) + 1; 
		argv += strlen(argv) + 1; 
	}
	
	/* zero word */
	//printf("0   : %08x\n", sp);
	PUSH_STACK(0);

	/* Environment pointers */
	/* first element of envp <-> high sp
	   push the address of last element of envp first */
	char* curr_envp = user_envp;
	uint32_t c = 0;
	for(index = 0; index < envc; index++ )
	{	
		//printf("envp: %08x %08x\n", sp, curr_envp);
		PUSH_STACK((uintptr_t) curr_envp);
		do 
		{
			bus_read(8, (uintptr_t) curr_envp, &c);
			curr_envp++;
		} while (c);
	}

	/* zero word */
	//printf("0   : %08x\n", sp);
	PUSH_STACK(0);

	/* Auxiliary vector */
#ifndef __WIN32__
	PUSH_AUXV(AT_CLKTCK, sysconf(_SC_CLK_TCK));
	PUSH_AUXV(AT_EGID, getegid());
	PUSH_AUXV(AT_GID, getgid());
	PUSH_AUXV(AT_EUID, geteuid());
	PUSH_AUXV(AT_UID, getuid());
#else
	PUSH_AUXV(AT_CLKTCK, 0);
	PUSH_AUXV(AT_EGID, 0);
	PUSH_AUXV(AT_GID, 0);
	PUSH_AUXV(AT_EUID, 0);
	PUSH_AUXV(AT_UID, 0);
#endif
	PUSH_AUXV(AT_HWCAP, 0); /* FIXME missing */
	PUSH_AUXV(AT_ENTRY, info->entry);
	PUSH_AUXV(AT_FLAGS, 0);
	PUSH_AUXV(AT_BASE, 0); /* FIXME missing */
	PUSH_AUXV(AT_PAGESZ, info->arch_page_size);
	PUSH_AUXV(AT_PHNUM, elf_header->e_phnum);
	PUSH_AUXV(AT_PHENT, elf_header->e_phnum * elf_header->e_phentsize);
	PUSH_AUXV(AT_PHDR, info->load_addr + elf_header->e_phoff);
	PUSH_AUXV(AT_NULL, 0);

	/* FIXME some more auxv may be platform dependant */

	return sp;
}


void exec_stack_init()
{
	sky_exec_info_t *info = get_skyeye_exec_info();
	sky_pref_t *pref = get_skyeye_pref();
	int index, offset = 0;

	uintptr_t stack_top = info->arch_stack_top;
	uintptr_t sp = stack_top;

	/* --------------------------------------------------------------------- */
	skyeye_log(Debug_log, __FUNCTION__, "exec_info: Copying string in user mode stack from %08x\n", sp);

	sp = fill_envp(sp);
	char* user_envp = (char *) sp;
	//printf("exec_info: user_envp points at: %08x\n", sp);

	sp = fill_argv(sp);
	char *user_argv = (char *) sp;
	//printf("exec_info: user_argv points at: %08x\n", sp);

	/* optional padding 0 - 16
	   take this opportunity to align sp */
	sp &= ~0xf;
	sp -= 0x4;

	//printf("exec_info: sp normalized\t\t\t%08x\n", sp);
	
	/* ---------------------------------------------------------------------
	   retrieving some info before filling the stack with elf tables */
	Elf32_Ehdr elf_header;
	retrieve_info(pref->exec_file, &elf_header);

	/* compute stack bottom to do an upward fill */
	short argc = info->exec_argc;
	short envc = info->exec_envc;
	uintptr_t table_top = sp;
	uint32_t table_size = 4 + 4*argc + 4 + 4*envc + 4 + 2*4*AUXV_SIZE + 8;
	sp = (table_top - table_size);
	
	//printf("exec_info: argv/envp/auxv tables in stack \t%08x->%08x\n", table_top, sp);
	/* fill table directly at the specified address
	   FIXME may need to align the stack during the filling to a 16bit boundary */
	set_tables(sp, user_argv, user_envp, &elf_header);

	/* set the initial sp at start */
	info->initial_sp = sp;
	skyeye_log(Debug_log, __FUNCTION__, "exec_info: initial sp at %08x\n", info->initial_sp);
}

