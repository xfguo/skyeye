/*
        skyeye_thread.h - thread related function definition for skyeye
        Copyright (C) 2003 Skyeye Develop Group
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
 * 10/02/2009   Michael.Kang  <blackfin.kang@gmail.com>
 */

#ifndef __SKYEYE_THREAD_H__
#define __SKYEYE_THREAD_H__
#include "skyeye_arch.h"
/*
 * the utility for create a thread. start_funcp is the start function for the thread, argp  * is the argument for startfuncp, and idp is the id for the thread.
 */
void create_thread(void *(*start_funcp)(void *), void * argp, pthread_t * idp);

#endif
