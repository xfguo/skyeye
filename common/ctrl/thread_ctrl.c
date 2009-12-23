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

//#include "armemu.h"
//#include "skyeye2gdb.h"
//extern ARMul_State * state;
//extern struct SkyEye_ICE skyeye_ice;
//extern int remote_debugmode;

int running;

#define MAX_THREAD_NUMBER 256
typedef enum thread_state{
	/* does not create. */
	Blank_state,
	/* Idle State */
	Idle_state,
	/* Running state */
	Running_state,
	/* waiting for cancel */
	Dead_state,
}thread_state_t;
typedef struct work_thread_s{
	pthread_t id;
	thread_state_t state;
}work_thread_t;
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

void skyeye_pause(void){
	running = 0;
}

void skyeye_continue(void){
	running = 1;
}

void skyeye_start(void){
	running = 1;
}

/* compatiable with old version */
/* 
 * some unknown exception happened.we need to stop the simulator
 * and return to CLI.
 */
void skyeye_exit(int ret){
	skyeye_pause();
	skyeye_log(Error_log, __FUNCTION__, "Some unknown exception happened.\n");
}

void skyeye_break(void){
}

#if 0
static int exec_callback(){
	/* test breakpoint and tracepoint */
	int i;
	address_t addr;
	addr = arch_instance->get_pc();
        for (i = 0;i < skyeye_ice.num_bps;i++){
        	if(skyeye_ice.bps[i] == addr)
                	return 1;
	} /* for */

	/* for remote debug, we need to detect the interrupt from remote side */
	if(remote_debugmode){
		/* to detect if Ctrl+c is pressed.. */
		if(remote_interrupt()){
			running = 0;
			return 1;
		}
	}
//#if 0 /* FIXME, will move to tracepoint module */
	addr = arch_instance->get_pc();
	        if (skyeye_ice.tps_status==TRACE_STARTED)
        {
        	for (i=0;i<skyeye_ice.num_tps;i++)
        arch_instance        {
                       	if (((skyeye_ice.tps[i].tp_address==addr)&& (skyeye_ice.tps[i].status==TRACEPOINT_ENABLED))||(skyeye_ice.tps[i].status==TRACEPOINT_STEPPING))
                        {
         	              	handle_tracepoint(i);
                         }
                }
	}
//#endif
	return 0;
}
#endif

int skyeye_is_running(void){
	return running;
} 
/*
 * mainloop of simulatior
 */
void skyeye_loop(generic_arch_t *arch_instance){
	for (;;) {
		/* chech if we need to run some callback functions at this time */
		exec_callback(Step_callback, arch_instance);
        	while (!running) {
                        /*
                         * spin until it's time to go.  this is useful when
                         * we're not auto-starting.
                         */
			sleep(1);
                }
	#if 0	
		/* check callback function */	
		int break_now_flag = exec_callback();
		if(break_now_flag)
			goto start;
	#endif
		/* run step once */
		arch_instance->step_once ();
	}
}

void create_thread(void *(*start_funcp)(void *), void * argp, pthread_t * idp)
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
                res = pthread_create( idp, &attr, start_funcp, argp );
                pthread_attr_destroy(&attr);
        } while (res == -1 && errno == EAGAIN);

        if (res == -1) {
		perror("failed to create thread");
		exit(-1);
	}
	/* we have allocate and create pthread successfully until now */
	pthread->state = Idle_state;
	pthread->id = *idp;
}

void
init_threads(void)
{
	int i;
	for(i = 0; i < MAX_THREAD_NUMBER; i++){
		pthread_pool[i].state = Blank_state;
	}
	//generic_arch_t* arch_instance = get_arch_instance();
        /* create the execution thread */
        //create_thread(skyeye_loop, arch_instance, &id);
}
void destroy_threads(void){
	int i;
        for(i = 0; i < MAX_THREAD_NUMBER; i++){
		/*
		 * Before cancel a thread, maybe we should stop it at first?
		 */
                if(pthread_pool[i].state != Blank_state){
			pthread_cancel(pthread_pool[i].id);
		}
        }
}
