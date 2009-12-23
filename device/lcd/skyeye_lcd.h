/*
	skyeye_lcd.h - skyeye general lcd device file support functions
	Copyright (C) 2003 - 2005 Skyeye Develop Group
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

#ifndef __SKYEYE_LCD_H_
#define __SKYEYE_LCD_H_

#include "skyeye_device.h"

#define LCD_MOD_NONE	0
#define LCD_MOD_GTK   	1
#define LCD_MOD_QT	2
#define LCD_MOD_X   	3
#define LCD_MOD_SDL    	4
#define LCD_MOD_WIN32  	5
#define LCD_MOD_BEOS  	6

struct lcd_device
{
	int mod;

	int width;
	int height;
	int depth;

	void *state;

	uint32 lcd_addr_begin;
	uint32 lcd_addr_end;

	/* private data. */
	void *priv;

	int (*lcd_open) (struct lcd_device *lcd_dev);
	int (*lcd_close) (struct lcd_device *lcd_dev);
	int (*lcd_update) (struct lcd_device *lcd_dev);
	int (*lcd_filter_read) (struct lcd_device *lcd_dev, uint32 addr, uint32 *data, size_t count);
	int (*lcd_filter_write) (struct lcd_device *lcd_dev, uint32 addr, uint32 data, size_t count);

	/* below is customed by device */
	void *device_priv; /* private data for device. */
	uint32 lcd_line_offset; /* pixels from the line's ending to the next line's starting. */
	uint32 (*lcd_lookup_color) (struct lcd_device *lcd_dev, uint32 color); /* return RGB32 color. */
	int lcd_color_right_to_left; /* whether color from right to left when depth < 32 */
	int lcd_dma_swap_word; /* whether to swap word from dma, usually for big endian */
};

/* help functions. */
unsigned char* skyeye_find_lcd_dma (struct lcd_device *lcd_dev);
void skyeye_convert_color_from_lcd_dma (struct lcd_device *lcd_dev,
					int x, int y, int w, int h,
					void (*func)(uint32 color, void *user_data1, void *user_data2),
					void *user_data, void *user_data2);


/* modules */
#ifdef GTK_LCD
int gtk_lcd_init (struct lcd_device *lcd_dev);
#endif

#ifdef WIN32_LCD
int win32_lcd_init (struct lcd_device *lcd_dev);
#endif

#ifdef BEOS_LCD
int beos_lcd_init (struct lcd_device *lcd_dev);
#endif
struct lcd_option
{
        /* display mode. e.g. gtk, qt, X, sdl */
        int mod;

        int width;
        int height;

        int depth;
};

#endif	/*__SKYEYE_LCD_H_*/

