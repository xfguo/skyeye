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
* @file pen_buffer.c
* @brief the pen buffer management
* @author Michael.Kang blackfin.kang@gmail.com
* @version 
* @date 2011-04-30
*/

#include <stdio.h>
/*
const int pen_buffer_sz = 6;
static int pen_buffer[pen_buffer_sz];
*/
//static int pen_buffer[6];

/**
* @brief the buffer of the touchscreen
*/
static int* pen_buffer = NULL;

/**
* @brief registe the buffer of touchscreen
*
* @param pb
*/
void register_pen_buffer(int* pb){
	pen_buffer = pb;
}

/**
* @brief get the registered the buffer
*
* @return 
*/
int* get_pen_buffer(){
	/*
	if(pen_buffer == NULL){
		printf("pen_buffer not implemented.\n");
		return NULL;
	}
	return pen_buffer;
	*/	
	//printf("pen_buffer not implemented.\n");
	return pen_buffer;
}

