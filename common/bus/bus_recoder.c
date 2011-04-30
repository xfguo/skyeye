/*
        bus_recoder.c - record the activities of bus
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
 * 12/26/2009   Michael.Kang  <blackfin.kang@gmail.com>
 */

#include "skyeye_bus.h"

/**
* @brief The buffer for the memory record
*/
static bus_recorder_t buffer;


/**
* @brief initialization of bus recorder
*/
void init_bus_recorder(){
}

/**
* @brief Snooping the bus activity
*
* @param rw the flag of read or write
* @param size
* @param addr
* @param value
* @param when before or after of the bus activity
*/
void bus_snoop(access_t rw, short size, int addr, uint32_t value, before_after_t when){
	buffer.rw = rw;	
	buffer.size = size;
	buffer.addr = addr;
	buffer.value = value;
	buffer.when = when;
}

/**
* @brief Get last bus activity
*
* @param rw The flag of read or write
*
* @return return the last bus record
*/
bus_recorder_t* get_last_bus_access(access_t rw){
	return &buffer;
}
