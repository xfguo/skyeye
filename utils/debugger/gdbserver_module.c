/*
        gdbserver_module.c - definition for gdb module
        Copyright (C) 2003-2007 Skyeye Develop Group
        for help please send mail to <skyeye-developer@lists.sf.linuxforum.net>

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
 * 01/02/2010   Michael.Kang  <blackfin.kang@gmail.com>
 */
#include "skyeye_module.h"
#include "skyeye_command.h"
extern int init_register_type();
const char* skyeye_module = "gdbserver";
void com_remote_gdb();
void module_init(){
	init_register_type();
	add_command("remote-gdb", com_remote_gdb, "Open remote gdb debugger.\n");
}
void module_fini(){
}

