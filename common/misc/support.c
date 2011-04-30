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
* @file support.c
* @brief some misc function
* @author Michael.Kang blackfin.kang@gmail.com
* @version 
* @date 2011-04-30
*/

#include "skyeye_config.h"

/**
* @brief show supported architectures and machines
*
* @param arch
* @param machines[]
*/
static void display_arch_support(
	const char * arch,
	machine_config_t machines[]
){
	int i;
        printf ( "-------- %s architectures ---------\n", arch );

	for ( i=0 ; machines[i].machine_name ; i++ )
		printf("%s \n",machines[i].machine_name);
}
void display_all_support(){
#if 0
	extern machine_config_t arm_machines[];
	extern machine_config_t bfin_machines[];
	extern machine_config_t coldfire_machines[];
	extern machine_config_t mips_machines[];
	extern machine_config_t ppc_machines[];

 	printf (
                  "----------- Architectures and CPUs simulated by SkyEye-------------\n");
	display_arch_support( "ARM", arm_machines );
	display_arch_support( "BlackFin", bfin_machines );
	display_arch_support( "Coldfire", coldfire_machines );
	display_arch_support( "MIPS", mips_machines );
	display_arch_support( "PowerPC", ppc_machines );
#endif
}

