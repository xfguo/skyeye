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
* @file lcd_gtk.h
* @brief The lcd low level function defined by gtk
* @author Michael.Kang blackfin.kang@gmail.com
* @version 0.1
* @date 2011-09-21
*/

#ifndef __LCD_GTK_H__
#define __LCD_GTK_H__
#include <skyeye_types.h>
typedef struct SkyEyeLCD_GTK {
	int width;
	int virtual_width;
	int height;
	int depth;

	GdkRectangle update_rect;
	gboolean update_all;

	guchar *rgbbuf;
	guint32 *fbmem;
	GdkRgbCmap *colormap;
	guint timer;

	GtkWidget *window;
	GtkWidget *drawing;

} SkyEyeLCD_GTK;

typedef struct lcd_gtk_device{
	SkyEyeLCD_GTK* gtk_win;	
	int mod;

	int width;
	int height;
	int depth;

	uint32 lcd_addr_begin;
	uint32 lcd_addr_end;

	uint32 lcd_line_offset; /* pixels from the line's ending to the next line's starting. */
	uint32 (*lcd_lookup_color) (conf_object_t *lcd_dev, uint32 color); /* return RGB32 color. */
}lcd_gtk_device;
#endif
