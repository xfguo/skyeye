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
* @file io.h
* @brief io read/write interface
* @author Michael.Kang blackfin.kang@gmail.com
* @version 
* @date 2011-04-30
*/

#ifndef __MEMORY_IO_H__
#define __MEMORY_IO_H__

void io_reset(void* state);
void io_do_cycle(void * state);
uint32_t io_read_byte(void* state, uint32_t addr);
uint32_t io_read_halfword(void* state, uint32_t addr);
uint32_t io_read_word(void* state, uint32_t addr);
void io_write_byte(void * state, uint32_t addr,uint32_t data);
void io_write_halfword(void * state, uint32_t addr, uint32_t data);
void io_write_word(void * state, uint32_t addr, uint32_t data);
int io_read(short size, uint32_t addr, uint32_t * value);
int io_write(short size, uint32_t addr, uint32_t value);
#endif
