/* 
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/*
 * 12/06/2007   Michael.Kang  <blackfin.kang@gmail.com>
 */

#ifndef __PPC_MEMORY_H__
#define __PPC_MEMORY_H__

//byte boot_rom[8 * 1024 * 1024];/* default 8M boot rom for e500 core */
#define INIT_RAM_SIZE 0x4000
extern uint8 *init_ram;		/* 16k init ram for 8560 */
extern uint8 *boot_rom;		/* default 8M bootrom for 8560 */
extern unsigned long init_ram_start_addr, init_ram_size;
extern uint32 boot_romSize;
extern uint32 boot_rom_start_addr;
#define DEFAULT_BOOTROM_SIZE (8 * 1024 * 1024)

#endif
