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
* @file skyeye_mm.c
* @brief the memory management module of SkyEye
* @author Michael.Kang blackfin.kang@gmail.com
* @version 
* @date 2011-04-30
*/

#include <string.h>
#include "skyeye_mm.h"

/**
* @brief memory allocation function for a given size
*
* @param sz
*
* @return 
*/
void* skyeye_mm(size_t sz){
	//printf("In %s, sz=0x%x\n", __FUNCTION__, sz);
	void* result = (unsigned long)malloc(sz);
	return result;
}

/**
* @brief allocate a zeroed memory
*
* @param sz
*
* @return 
*/
void* skyeye_mm_zero(size_t sz){
	void* result = malloc(sz);
	memset(result, 0, sz);
	return result;
}

/**
* @brief duplicate a string
*
* @param s
*
* @return 
*/
char* skyeye_strdup(const char* s){
	char* result = strdup(s);
	return result;
}

/**
* @brief free the allocated memory
*
* @param p
*/
void skyeye_free(void* p){
	free(p);
	p = NULL;
	return;
}

