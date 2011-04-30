/*
        checkpoint.h - definition for checkpoint related operation
        Copyright (C) 2009 Michael.Kang
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
/**
* @file checkpoint.h
* @brief 
* @author Michael.Kang blackfin.kang@gmail.com
* @version 
* @date 2011-04-30
*/

#ifndef __COMMON_CHECKPOINT_H__
#define __COMMON_CHECKPOINT_H__

#include "skyeye_types.h"

/**
* @brief data type saved to checkpoint
*/
typedef struct chp_data{
	char *name;
	int size;
	void *data;
	struct	chp_data *next;
}chp_data;

/**
* @brief the checkpoint data list
*/
typedef struct chp_list{
	chp_data *head;
	chp_data *tail;
	int num;
}chp_list;

#endif
