/*
	skyeye_signal.h - define the signal framework for skyeye
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
#ifndef __SKYEYE_SIGNAL_H__
#define __SKYEYE_SIGNAL_H__
typedef enum signal{
	/* maintain the previous level of the signal, do none of change */
	Prev_level = -1,
	/* change the signal level to low */
	Low_level,
	/* change the signal level to high */
	High_level,
}signal_t;

typedef struct{
                /* Normal irq signal in arm */
                signal_t irq;
                /* Fast irq signal in arm */
                signal_t firq;
		/* reset signal in arm dyf add when move sa1100 to soc dir  2010.9.21*/
		signal_t reset;
}arm_signal_t;

typedef	struct{
	/* Hardware interrupt signal in mips */
	signal_t HARD_IP[5];
}mips_signal_t;

typedef	struct{
	/* Normal interrupt signal in powerpc */
	signal_t intr;
	/* Critical interrupt signal in powerpc */
	signal_t cintr;
	signal_t core_reset;
}powerpc_signal_t;

typedef union interrupt_signal{
	arm_signal_t arm_signal;
	mips_signal_t mips_signal;
	powerpc_signal_t powerpc_signal;
}interrupt_signal_t;

exception_t send_signal(interrupt_signal_t* signal);

#endif
