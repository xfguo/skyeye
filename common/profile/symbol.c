/*  armsym.c -- Main instruction emulation:  SA11x Instruction Emulator.
    Copyright (C) 2001 Princeton University 

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
/**
* @file symbol.c
* @brief the symbol management
* @author Michael.Kang blackfin.kang@gmail.com
* @version 
* @date 2011-04-30
*/

#include <config.h>
#include <assert.h>
#include "bfd.h"
#include <search.h>
#include "skyeye_symbol.h"
#include "skyeye_mm.h"
#include "symbol.h"

#define _GNU_SOURCE     /* Expose declaration of tdestroy() */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


static long storage_needed = 0;
static asymbol **symbol_table = NULL;
static unsigned long number_of_symbols = 0, kernel_number = 0;

/**************************************************************************
  This function read the symbol list and store into a table
  It then generates a tree based on the value of function symbol
**************************************************************************/

void *root = NULL;

int
compare_address(const void *pa, const void *pb)
{

   if (((SYM_FUNC *) pa)->address == ((SYM_FUNC *) pb)->address)
   {
	   return 0;
   }
   else if (((SYM_FUNC *) pa)->address > ((SYM_FUNC *) pb)->address)
   {
	   return 1;
   }
   return -1;
}

int
compare_function(const void *pa, const void *pb)
{
   return strcmp(((SYM_FUNC *) pa)->name,((SYM_FUNC *) pb)->name); 
}

void
action(const void *nodep, const VISIT which, const int depth)
{
   SYM_FUNC *datap;

   switch (which) {
   case preorder:
	   datap = *(SYM_FUNC **) nodep;
	   break;
   case postorder:
	   datap = *(SYM_FUNC **) nodep;
	   break;
   case endorder:
	   datap = *(SYM_FUNC **) nodep;
	   break;
   case leaf:
	   datap = *(SYM_FUNC **) nodep;
	   break;
   }
}

/**
* @brief  initialization of a symbol table
*
* @param filename
* @param arch_name
*/
void init_symbol_table(char* filename, char* arch_name)
{
	long i,j, digit;
	SYM_FUNC **val;
	SYM_FUNC *symp;
	asymbol *symptr;
	generic_address_t address;
	bfd *abfd;

	if(!filename){
		fprintf(stderr, "Can not get correct kernel filename!Maybe your skyeye.conf have something wrong!\n");
		return;
	}
	abfd = bfd_openr (filename, get_bfd_target(arch_name));

	if (!bfd_check_format(abfd, bfd_object)) {
		bfd_close(abfd);
		printf("In %s, wrong bfd format\n", __FUNCTION__) ;
		return;
		//exit(0);
	}

	storage_needed = bfd_get_symtab_upper_bound(abfd);
	if (storage_needed < 0){
		printf("FAIL\n");
		exit(0);
	}

	symbol_table = (asymbol **) skyeye_mm_zero (storage_needed);
	if(symbol_table == NULL){
		fprintf(stderr, "Can not alloc memory for symbol table.\n");
		exit(0);
	}

	number_of_symbols =
		bfd_canonicalize_symtab (abfd, symbol_table);
	kernel_number = number_of_symbols; /* <tktan> BUG200106022219 */

	if (number_of_symbols < 0){
		printf("FAIL\n");
		exit(0);
	}

	for (i = 0; i < number_of_symbols; i++) {
		symptr = symbol_table[i] ;
		address = symptr->value + symptr->section->vma; // adjust for section address

		/*I don't know why there are "$a" and "$d" in symbol table. I just remove them here.*/
		if (strcmp(symbol_table[i]->name,"$a") == 0 || strcmp(symbol_table[i]->name,"$d") == 0 )
			continue;
		if (((i<kernel_number) && (symbol_table[i]->flags == 0x01)) || // <tktan> BUG200105172154, BUG200106022219 
		((i<kernel_number) && (symbol_table[i]->flags == 0x02)) || // <tktan> BUG200204051654
		(symbol_table[i]->flags & 0x10)) { // Is a function symbol
		  // printf("%x %8x %s\n", symbol_table[i]->flags, key, symbol_table[i]->name);

		 // *************************************************
		  // This is allocating memory for a struct funcsym
		  // *************************************************
			symp = (SYM_FUNC *) skyeye_mm_zero(sizeof(SYM_FUNC));
			strcpy(symp->name,symbol_table[i]->name);
			symp->address = address;
			symp->total_cycle = 0;
			symp->total_energy = 0;
			symp->instances = 0;

			val = (SYM_FUNC **)tsearch((void *) symp, &root, compare_function);
			val = (SYM_FUNC **)tsearch((void *) symp, &root, compare_address);
			strcpy((*val)->name,symp->name);
			if (val == NULL)
			   exit(EXIT_FAILURE);
			else if (compare_function((void *)(*(SYM_FUNC **) val),(void *)symp) != 0)
			   free(symp);
		}
	}

	return;
}
/***************************************************************
  This function get check the tree for an node
  If it exists, the corresponding pointer to the SYM_FUNC will
  be returned
*************************************************************/

/**
* @brief get the symbol of a given address
*
* @param address
*
* @return 
*/
char *get_sym(generic_address_t address)
{
	SYM_FUNC **symp;
	SYM_FUNC *node;

//	printf("GetSym %x\n", address);
	/*
	 * If storage_needed is zero, means symbol table is not initialized.
	 */
	if(storage_needed == 0)
		return NULL;

    node = (SYM_FUNC *) skyeye_mm_zero(sizeof(SYM_FUNC));
	node->address = address;
    symp = (SYM_FUNC **)tfind((void *) node, &root, compare_address);
	if(symp != NULL)
		return (*symp)->name;
	free(node);
	return NULL;
}

/**
* @brief get the address of a given function
*
* @param function name
*
* @return 
*/
generic_address_t get_addr(char func_name[128])
{
	SYM_FUNC **symp;
	SYM_FUNC *node;

	/*
	 * If storage_needed is zero, means symbol table is not initialized.
	 */
	if(storage_needed == 0)
		return NULL;

    node = (SYM_FUNC *) skyeye_mm_zero(sizeof(SYM_FUNC));
	if (node == NULL)
		exit(-1);
	strcpy(node->name,func_name);
    symp = (SYM_FUNC **)tfind((void *) node, &root, compare_function);
	if(symp != NULL)
	{
		return (*symp)->address;
	}
	free(node);
	return 0x0;
}
