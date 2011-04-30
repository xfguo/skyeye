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
* @file bfd_target.c
* @brief the targe list defined by bfd library
* @author Michael.Kang blackfin.kang@gmail.com
* @version 
* @date 2011-04-30
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
const char* elf32_powerpc = "elf32-powerpc";
const char* elf32_littlearm = "elf32-littlearm";
const char* elf32_bigarm = "elf32-bigarm";

/**
* @brief get the target name defined by bfd library
*
* @param arch_name
*
* @return 
*/
char* get_bfd_target(const char* arch_name){
	if(strncmp("powerpc", arch_name, sizeof("powerpc")) == 0){
		return elf32_powerpc;
	}
	else if(strncmp("arm", arch_name, sizeof("arm")) == 0 ){
		return elf32_littlearm;
	}
	else
		return NULL;
}
