/*
	skyeye_signal.c - implement the signal framework for skyeye
	Copyright (C) 2010 Skyeye Develop Group
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
 *	2010-08-15 Michael.Kang<blackfin.kang@gmail.com>
 */
#include "skyeye_types.h"
#include "skyeye_arch.h"
#include "skyeye_signal.h"

/**
* @brief send interrupt signal to the processor
*
* @param signal
*
* @return 
*/
exception_t send_signal(interrupt_signal_t* signal){
	generic_arch_t* arch_instance = get_arch_instance("");
	arch_instance->signal(signal);
}

