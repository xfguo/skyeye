/*
        dis_asm.c - disassemble implementation for all the arch. We refer to some
	source code from binutils.

        Copyright (C) 2003-2010 Skyeye Develop Group
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
 * 01/16/2010   Michael.Kang  <blackfin.kang@gmail.com>
 */

#include <dis-asm.h>
#include <assert.h>

#include "skyeye_types.h"
#include "skyeye_arch.h"

/* The number of zeroes we want to see before we start skipping them.
   The number is arbitrarily chosen.  */

#define DEFAULT_SKIP_ZEROES 8

/* The number of zeroes to skip at the end of a section.  If the
   number of zeroes at the end is between SKIP_ZEROES_AT_END and
   SKIP_ZEROES, they will be disassembled.  If there are fewer than
   SKIP_ZEROES_AT_END, they will be skipped.  This is a heuristic
   attempt to avoid disassembling zeroes inserted by section
   alignment.  */

#define DEFAULT_SKIP_ZEROES_AT_END 3


/* The sorted symbol table.  */
static asymbol **sorted_syms;

/* Number of symbols in `sorted_syms'.  */
static long sorted_symcount = 0;

/* Endianness to disassemble for, or default if BFD_ENDIAN_UNKNOWN.  */
static enum bfd_endian endian = BFD_ENDIAN_LITTLE;

/* Architecture to disassemble for, or default if NULL.  */
static char *machine = NULL;
//static char *machine = "arm";

/* Target specific options to the disassembler.  */
static char *disassembler_options = NULL;

struct disassemble_info disasm_info;
bfd *              abfd;
disassembler_ftype disassemble_fn = NULL;

/* Exit status.  */
static int exit_status = 0;

static int disasm_read_memory(bfd_vma memaddr, bfd_byte *myaddr, unsigned int length, struct disassemble_info *info){
	int ret;
	int i;
	for(i = 0; i < length; i++)
		ret = mem_read(8, memaddr + i, (myaddr + i));
	return 0;
}

static bfd_boolean symbol_is_valid(asymbol *sym, struct disassemble_info * info){
	return FALSE;
}

static void objdump_print_address(bfd_vma vma, struct disassemble_info * info){
}

/* Determine if the given address has a symbol associated with it.  */

static int
objdump_symbol_at_address (bfd_vma vma, struct disassemble_info * info)
{
	asymbol * sym;

	//sym = find_symbol_for_address (vma, info, NULL);
	return (sym != NULL && (bfd_asymbol_value (sym) == vma));
}


void init_disassemble(){
	abfd = malloc(sizeof(bfd));
	init_disassemble_info (&disasm_info, stdout, (fprintf_ftype) fprintf);
	disasm_info.print_address_func = objdump_print_address;
	disasm_info.symbol_at_address_func = objdump_symbol_at_address;

	/*
	 * choose arch infor since we will select different disassemble function.
	 */
	generic_arch_t* arch_instance = get_arch_instance("");
	machine = arch_instance->arch_name;	
	const bfd_arch_info_type *info = bfd_scan_arch (machine);
	if (info == NULL){
        	//fatal (_("Can't use supplied machine %s"), machine);
		printf("Can't use supplied machine %s\n", machine);
		return;
	}

	abfd->arch_info = info;
	
	/*
	 * endian selection
	 */
	if (endian != BFD_ENDIAN_UNKNOWN)
	{
		struct bfd_target *xvec;

		xvec = xmalloc (sizeof (struct bfd_target));
		//memcpy (xvec, abfd->xvec, sizeof (struct bfd_target));
		xvec->byteorder = endian;
		abfd->xvec = xvec;
	}
	/* Get a disassemble function according to the arch and endian of abfd */
	disassemble_fn = disassembler (abfd);
	if(!disassemble_fn)
	{
      /*
	non_fatal (_("Can't disassemble for architecture %s\n"),
                 bfd_printable_arch_mach (bfd_get_arch (abfd), 0));
	*/
		printf("Can't disassemble for architecture %s\n", bfd_printable_arch_mach (bfd_get_arch (abfd), 0));
		exit_status = 1;
		return;
	}

	disasm_info.flavour = bfd_get_flavour (abfd);
	disasm_info.arch = bfd_get_arch (abfd);
	disasm_info.mach = bfd_get_mach (abfd);
	disasm_info.disassembler_options = disassembler_options;
	disasm_info.octets_per_byte = bfd_octets_per_byte (abfd);
	disasm_info.skip_zeroes = DEFAULT_SKIP_ZEROES;
	disasm_info.skip_zeroes_at_end = DEFAULT_SKIP_ZEROES_AT_END;
	disasm_info.disassembler_needs_relocs = FALSE;

#if 1
	if (bfd_big_endian (abfd))
		disasm_info.display_endian = disasm_info.endian = BFD_ENDIAN_BIG;
	else if (bfd_little_endian (abfd))
		disasm_info.display_endian = disasm_info.endian = BFD_ENDIAN_LITTLE;
	else
 /* ??? Aborting here seems too drastic.  We could default to big or little
       instead.  */
	disasm_info.endian = BFD_ENDIAN_UNKNOWN;
#endif
	/* set the default endianess is BFD_ENDIAN_LITTLE */
	disasm_info.display_endian = disasm_info.endian = BFD_ENDIAN_LITTLE;
	disasm_info.symtab = sorted_syms;
	disasm_info.symtab_size = sorted_symcount;
	disasm_info.read_memory_func = disasm_read_memory;
	free (sorted_syms);
}


void disassemble(generic_address_t addr){
	//assert(disasm_info);
	/* if abfd is NULL, we need to run initilization of disassembler */
	if(abfd == NULL)
		init_disassemble();		
	/* if disassemble_fn is NULL, some errors happened. */
	if(disassemble_fn == NULL)
		return;
	printf("0x%x:", addr);
	disassemble_fn(addr, &disasm_info);
	printf("\n");
}
