/*

        thread_ctrl.c - create and control thread
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
 * 05/12/2009   Michael.Kang  <blackfin.kang@gmail.com>
 */

#include <errno.h>
#include <pthread.h>
#include <stdint.h>

#include "skyeye_types.h"
#include "skyeye_arch.h"
#include "skyeye_callback.h"
#include "breakpoint.h"
#include "skyeye_obj.h"
#include "skyeye_exec.h"
#include "skyeye_thread.h"
#include "skyeye_cell.h"

#define MAX_THREAD_NUMBER 256
static work_thread_t pthread_pool[MAX_THREAD_NUMBER];
static exception_t alloc_thread(work_thread_t** thread){
	int i;
        for(i = 0; i < MAX_THREAD_NUMBER; i++)
                if(pthread_pool[i].state == Blank_state){
			*thread = &pthread_pool[i];
			break;
		}

	if(i >= MAX_THREAD_NUMBER)
		return Malloc_exp;
	else
		return No_exp;
}

/**
* @brief Get a pthread instance from thread pool
*
* @param id the given thread id
*
* @return  the pthread instance found
*/
work_thread_t* get_thread_by_id(pthread_t id){
	int i;
	for(i = 0; i < MAX_THREAD_NUMBER; i++){
		if(pthread_equal(pthread_pool[i].id,id) != 0)
			return &pthread_pool[i]; 
	}
	return NULL;
}

skyeye_cell_t* get_cell_by_thread_id(pthread_t id){
	work_thread_t* thread = get_thread_by_id(id);
	skyeye_cell_t* cell = (skyeye_cell_t*)get_cast_conf_obj(thread->priv_data, "skyeye_cell_t");
	return cell;
}
void create_thread(void *(*start_funcp)(void *), void* argp, pthread_t * idp)
{
        int res;
	work_thread_t* pthread;
	if(alloc_thread(&pthread) != No_exp){
		skyeye_log(Error_log, __FUNCTION__, "Can not alloc thread.\n");
		return;
	}	

        do {
                pthread_attr_t attr;
                pthread_attr_init(&attr); /* initialize attr with default attributes */
                pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);     /* schedule like a process */
                res = pthread_create( idp, &attr, start_funcp, (void *)argp );
                pthread_attr_destroy(&attr);
        } while (res == -1 && errno == EAGAIN);

        if (res == -1) {
		perror("failed to create thread");
		exit(-1);
	}
	/* we have allocate and create pthread successfully until now */
	pthread->state = Idle_state;
	pthread->id = *idp;
	pthread->priv_data = argp;
}


/**
* @brief Running a thread with the given id
*
* @param id the thread id
*/
void start_thread_by_id(pthread_t id){
	work_thread_t* thread = get_thread_by_id(id);
	thread->state = Running_state;
}

/**
* @brief stop a thread
*
* @param id the thread id
*/
void stop_thread_by_id(pthread_t id){
	work_thread_t* thread = get_thread_by_id(id);
	thread->state = Stopped_state;
}

void start_thread(work_thread_t* thread){
	thread->state = Running_state;
}

void stop_thread(work_thread_t* thread){
	thread->state = Stopped_state;
}
thread_state_t get_thread_state(pthread_t id){
       work_thread_t* thread = get_thread_by_id(id);
       if(thread != NULL)
               return thread->state;
       else
               return Blank_state;
}

/**
* @brief Judge if a pthread does exist in our thread pool
*
* @param id the judge thread id
*
* @return True means exists, and false means not.
*/
bool_t thread_exist(pthread_t id){
	int i;
	for(i = 0; i < MAX_THREAD_NUMBER; i++){
		if(pthread_equal(pthread_pool[i].id,id) != 0)
			return True; 
	}
	return False;
}
/**
* @brief Initialization of pthread pool
*/
void
init_threads(void)
{
	int i;
	for(i = 0; i < MAX_THREAD_NUMBER; i++){
		pthread_pool[i].state = Blank_state;
	}
}

/**
* @brief Destroy all the thread except itself
*/
void destroy_threads(void){
	int i;
        for(i = 0; i < MAX_THREAD_NUMBER; i++){
		/*
		 * Before cancel a thread, maybe we should stop it at first?
		 */
                if(pthread_pool[i].state != Blank_state){
			if(pthread_pool[i].id == pthread_self())
				continue;
			pthread_cancel(pthread_pool[i].id);
			pthread_join(pthread_pool[i].id, NULL);
		}
        }
}

/**
* @brief Set all the thread to running mode
*/
void start_all_thread(){
	int i;
        for(i = 0; i < MAX_THREAD_NUMBER; i++){
		/*
		 * Before start a thread, check the data
		 */
                if((pthread_pool[i].state != Blank_state) && (pthread_pool[i].priv_data != NULL)){
			printf("In %s, the thread %d is set to running\n", __FUNCTION__, i);
			pthread_pool[i].state = Running_state;
		}
	}
}

/**
* @brief stop all the running thread
*/
void stop_all_thread(){
	int i;
        for(i = 0; i < MAX_THREAD_NUMBER; i++){
		/*
		 * Before start a thread, check the data
		 */
                if((pthread_pool[i].state == Running_state) && (pthread_pool[i].priv_data != NULL)){
			printf("In %s, the thread %d is set to stopped\n", __FUNCTION__, i);
			pthread_pool[i].state = Stopped_state;
		}
	}
}

conf_object_t* get_thread_priv(pthread_t id){
	work_thread_t* thread = get_thread_by_id(id);
	return thread->priv_data;
}
