/*
	dev_lcd_s3c44b0x.h - skyeye S3C44B0X lcd controllor simulation
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

/*
 * 01/27/2007   initial version by Anthony Lee
 */

#ifndef __DEV_LCD_S3C44B0X_H_
#define __DEV_LCD_S3C44B0X_H_

#define LCDCON1		(0x0)
#define LCDCON2		(0x4)
#define LCDCON3		(0x40)
#define LCDSADDR1	(0x8)
#define LCDSADDR2	(0xc)
#define LCDSADDR3	(0x10)

#define REDLUT		(0x14)
#define GREENLUT	(0x18)
#define BLUELUT		(0x1c)

#define DP1_2		(0x20)
#define DP4_7		(0x24)
#define DP3_5		(0x28)
#define DP2_3		(0x2c)
#define DP5_7		(0x30)
#define DP3_4		(0x34)
#define DP4_5		(0x38)
#define DP6_7		(0x3c)
#define DITHMODE	(0x44)


typedef struct lcd_s3c44b0x_io
{
	uint32 lcdcon1;
	uint32 lcdcon2;
	uint32 lcdcon3;
	uint32 lcdsaddr1;
	uint32 lcdsaddr2;
	uint32 lcdsaddr3;

	uint32 lcdredlut;
	uint32 lcdgreenlut;
	uint32 lcdbluelut;

	uint32 lcddp12;
	uint32 lcddp47;
	uint32 lcddp35;
	uint32 lcddp23;
	uint32 lcddp57;
	uint32 lcddp34;
	uint32 lcddp45;
	uint32 lcddp67;

	uint32 lcddithmode;
} lcd_s3c44b0x_io_t;

#endif /* _DEV_LCD_S3C44B0X_H_ */

