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
#ifndef __SKYEYE_LCD_H__
#define __SKYEYE_LCD_H__
/**
* @file skyeye_lcd.h
* @brief The lcd interface
* @author Michael.Kang blackfin.kang@gmail.com
* @version 0.1
* @date 2011-09-27
*/
typedef struct lcd_surface{
	int width;
	int height;
	int depth;

	uint32 lcd_addr_begin;
	uint32 lcd_addr_end;
	uint32 lcd_line_offset
}lcd_surface_t;
#endif
