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
#include <pthread.h>
#include "skyeye_types.h"
#ifdef __cplusplus
 extern "C" {
#endif
typedef enum thread_state{
	/* does not create. */
	Blank_state,
	/* Idle State */
	Idle_state,
	/* Running state */
	Running_state,
	/* Stopped state */
	Stopped_state,
	/* waiting for cancel */
	Dead_state,
}thread_state_t;

typedef struct work_thread_s{
	//conf_object_t* priv_data;
	//skyeye_cell_t* cell;
	pthread_t id;
	thread_state_t state;
	//skyeye_exec_t thread_ctrl;
	//skyeye_cell_t cell;
	conf_object_t* priv_data;
}work_thread_t;

/*
 * the utility for create a thread. start_funcp is the start function for the thread, argp  
 * is the argument for startfuncp, and idp is the id for the thread.
 */
void create_thread(void *(*start_funcp)(void *), void * argp, pthread_t * idp);
bool_t thread_exist(pthread_t id);
conf_object_t* get_thread_priv(pthread_t id);
thread_state_t get_thread_state(pthread_t id);

void start_all_thread();
void stop_all_thread();
void start_thread(work_thread_t* thread);
void stop_thread(work_thread_t* thread);
work_thread_t* get_thread_by_id(pthread_t id);
void skyeye_exit(int ret);
#ifdef __cplusplus
}
#endif

#endif
