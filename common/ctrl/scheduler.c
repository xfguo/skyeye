/*
        scheduler.c - Implement a scheduler based on timer. the 
	smallest schedule cycle is one micro second.

        Copyright (C) 2009 - 2010 Michael.Kang
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
 * 06/12/2010   Michael.Kang  <blackfin.kang@gmail.com>
 */

#include "skyeye_sched.h"
#include "skyeye_queue.h"
#include "skyeye_types.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include "skyeye_thread.h"
#include "sim_control.h"
#include "skyeye_lock.h"
#include "skyeye_mm.h"

//#define DEBUG
#include "skyeye_log.h"

/**
* @brief the event type
*/
struct event{
	sched_mode_t mode;		/* scheduler mode  */
	sched_func_t func;		/* scheduler callback function */	
	void *func_arg;			/* argument of scheduler callback */
	int delta;			/* recode the time before scheduler occur */
	int id;				/* the scheduler id */
	unsigned int expiration;	/* scheduler period */
	LIST_ENTRY (event)list_entry;	
};

/* thread scheduler */
LIST_HEAD(list_thread_head, event) thread_head;

/**
* @brief the thread lock
*/
RWLOCK_T  thread_lock;

/**
* @brief the pthread id running event list
*/
static pthread_t pid;

/**
* @brief The wall clock
*/
static uint64_t now_us = 0;

uint64_t get_clock_us(){
	return now_us;
}

/**
* @brief the scheduler of event thread
*/
static void thread_scheduler(void){
	/* 
	 * Check if there is some timer is expiried, we
	 * should execute the corresponding event.
	 */
	struct event *tmp ;
	while(!(SIM_is_running()))
	{
		/* set a thread cancelation-point */
		usleep(100);
	}
	struct timeval tv;
	unsigned long long start_utime, current_utime, last_utime, passed_utime;
	gettimeofday(&tv,NULL);
	start_utime = tv.tv_sec * 1000000 + tv.tv_usec;
	while(1)
	{
		usleep(100);
		gettimeofday(&tv,NULL);
		last_utime = current_utime;
		current_utime = tv.tv_sec * 1000000 + tv.tv_usec;
		/* Update the clock on the wall */
		now_us = current_utime - start_utime;

		/* Calculate how much microseconds passed comparing to last time */
		passed_utime = current_utime - last_utime;
		DBG("In %s, passed_utime=%d\n", __FUNCTION__, passed_utime);
		LIST_FOREACH(tmp, &thread_head,list_entry){
			/* Decrease dleta */
			tmp->delta -= passed_utime;
			DBG("In %s, tmp->delta=%d\n", __FUNCTION__, tmp->delta);
			
			if(tmp->delta == 0 || tmp->delta < 0){
				/* Sometimes, delta is missing */
				if(tmp->delta < 0)
					DBG("scheduler occur later \n");
				/* execute the scheduler callback */
				if(tmp->func != NULL)
					tmp->func((void*)tmp->func_arg);

				if(tmp->mode  == Oneshot_sched){
RW_WRLOCK(thread_lock);
					LIST_REMOVE(tmp, list_entry);		
RW_UNLOCK(thread_lock);
					skyeye_free(tmp);
				}
				else{
					tmp->delta = (int)tmp->expiration;
				}
			}
		}
	}
}

/*
 * we add a period scheduler .
 */

/**
* @brief initialization of thread scheduler
*
* @return 
*/
int init_thread_scheduler(){
	RWLOCK_INIT(thread_lock);
	LIST_INIT(&thread_head);

	/* creat a thread for  scheduling */
	create_thread(thread_scheduler, NULL, &pid); 
}

/**
* @brief create a thread event
*
* @param ms
* @param mode
* @param func
* @param arg
* @param id
*
* @return 
*/
int create_thread_scheduler(unsigned int ms, sched_mode_t mode, sched_func_t func, void *arg, int *id){
	
	/* create a new event */
	struct event* e = skyeye_mm(sizeof(struct event));
	if( e == NULL){
		return Malloc_exp;
	}

	/* check arguments */
	if( ms < 0){
		return Invarg_exp;
	}
	if(mode != Periodic_sched && mode != Oneshot_sched){
		return Invarg_exp;
	}
	if(func == NULL){
		return Invarg_exp;
	}
	e->mode = mode;		
	e->expiration = ms;
	e->delta = ms;
	e->func = func;
	e->func_arg = (void*)arg;

	/* get the new event id */
	if(LIST_EMPTY(&thread_head))
		*id = 0;
	else
		*id = LIST_FIRST(&thread_head)->id + 1;
	e->id = *id; 	

RW_WRLOCK(thread_lock);
	/* insert the event to the list */
	LIST_INSERT_HEAD(&thread_head, e, list_entry);
RW_UNLOCK(thread_lock);

	return No_exp;
}

/**
* @brief modify expiration and mode of the thread scheduler
*
* @param id
* @param ms
* @param mode
*
* @return 
*/
int mod_thread_scheduler(int id,unsigned int ms, sched_mode_t mode){
	struct event *tmp;
	LIST_FOREACH(tmp, &thread_head,list_entry){
		if(tmp->id == id){
	
			if( ms < 0){
				return Invarg_exp;
			}
			if(mode != Periodic_sched && mode != Oneshot_sched){
				return Invarg_exp;
			}

RW_WRLOCK(thread_lock);
			/* update the scheduler */
			tmp->expiration = ms;
			tmp->delta = (int)ms;
			tmp->mode = mode;
RW_UNLOCK(thread_lock);
			return No_exp;	
		}
	}
	return  Invarg_exp;
}

/**
* @brief delete an event in a thread scheduler
*
* @param id
*
* @return 
*/
int del_thread_scheduler(int id){
	struct event *tmp ;
	struct event *q = NULL;
	LIST_FOREACH(tmp, &thread_head,list_entry){
		if(tmp->id == id){
			q = tmp;
RW_WRLOCK(thread_lock);
			LIST_REMOVE(tmp, list_entry);		
RW_UNLOCK(thread_lock);
			skyeye_free(q);
			return No_exp;
		}
	}
	return Invarg_exp;
}

/**
* @brief list all the event in thread scheduler
*/
void list_thread_scheduler(void)
{
	struct event *tmp ;
	printf("id\tperiod\tdleta\tmode\n");
	LIST_FOREACH(tmp, &thread_head,list_entry){
		printf("%d\t%d\t%d\t%s\n", tmp->id, tmp->expiration, tmp->delta, (tmp->mode==0)?"Oneshot_sched":"Periodic_sched");
	}
}

/**
* @brief destructor for the thread scheduler
*
* @return 
*/
int fini_thread_scheduler(){
	struct event *tmp ;
	struct event *q = NULL;
	
	LIST_FOREACH(tmp, &thread_head,list_entry){
			q = tmp;
RW_WRLOCK(thread_lock);
			LIST_REMOVE(tmp, list_entry);		
RW_UNLOCK(thread_lock);
			skyeye_free(q);
	}
	
	RWLOCK_DESTROY(thread_lock);
	if(LIST_EMPTY(&thread_head))		
		return No_exp;
}
/* thread shcheduler end */

/* timer scheduler */
static struct itimerval value, ovalue;

LIST_HEAD(list_timer_head, event) timer_head;
RWLOCK_T  timer_lock;	

static void timer_scheduler(int signo){
	switch (signo){
		case SIGVTALRM:
			signal(SIGVTALRM, timer_scheduler);
			break;
		default:
			/* ignored and do nothing */
			return;
	}
	/* 
	 * Check if there is some timer is expiried, we
	 * should execute the corresponding event.
	 */
	struct event *tmp;
	LIST_FOREACH(tmp, &timer_head,list_entry){
		/* Decrease dleta */
		tmp->delta -= 1;
		
		if(tmp->delta == 0 || tmp->delta < 0){
			/* wrong time value */
			if(tmp->delta < 0)
				printf("timer scheduler occur later \n");
			else	
				/* execute the scheduler callback */
				if(tmp->func != NULL)
					tmp->func((void*)tmp->func_arg);

			if(tmp->mode  == Oneshot_sched){
RW_WRLOCK(timer_lock);
				LIST_REMOVE(tmp, list_entry);		
RW_UNLOCK(timer_lock);
				skyeye_free(tmp);
			}
			else{
				tmp->delta = (int)tmp->expiration;
			}
		}
	}
}

/*
 * we add a period scheduler whose interval is one ms.
 */
int init_timer_scheduler(){
	RWLOCK_INIT(timer_lock);
	LIST_INIT(&timer_head);
	signal(SIGVTALRM, timer_scheduler);

	value.it_value.tv_sec = 1;
	value.it_value.tv_usec = 0;
	/* set the interval to 1ms */
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_usec = 1000;

	setitimer(ITIMER_VIRTUAL, &value, &ovalue);
}

/* create and add an timer event */
int create_timer_scheduler(unsigned int ms, sched_mode_t mode, sched_func_t func, void *arg, int *id){
	
	/* create a new event */
	struct event* e = skyeye_mm(sizeof(struct event));
	if( e == NULL){
		return Malloc_exp;
	}

	/* check arguments */
	if( ms < 0){
		return Invarg_exp;
	}
	if(mode != Periodic_sched && mode != Oneshot_sched){
		return Invarg_exp;
	}
	if(func == NULL){
		return Invarg_exp;
	}
	e->mode = mode;		
	e->expiration = ms;
	e->delta = ms;
	e->func = func;
	e->func_arg = (void*)arg;

	/* get the new event id */
	if(LIST_EMPTY(&timer_head))
		*id = 0;
	else
		*id = LIST_FIRST(&timer_head)->id + 1;
	e->id = *id; 	

RW_WRLOCK(timer_lock);
	/* insert the event to the list */
	LIST_INSERT_HEAD(&timer_head, e, list_entry);
RW_UNLOCK(timer_lock);

	return No_exp;
}

/* modify expiration and mode of the timer scheduler */
int mod_timer_scheduler(int id,unsigned int ms, sched_mode_t mode){
	struct event *tmp;
	LIST_FOREACH(tmp, &timer_head,list_entry){
		if(tmp->id == id){
	
			if( ms < 0){
				return Invarg_exp;
			}
			if(mode != Periodic_sched && mode != Oneshot_sched){
				return Invarg_exp;
			}

RW_WRLOCK(timer_lock);
			/* update the scheduler */
			tmp->expiration = ms;
			tmp->delta = (int)ms;
			tmp->mode = mode;
RW_UNLOCK(timer_lock);
			
			return No_exp;	
		}
	}
	return  Invarg_exp;
}

/* remove a timer scheduler from the queue */
int del_timer_scheduler(int id){
	struct event *tmp ;
	struct event *q = NULL;
	LIST_FOREACH(tmp, &timer_head,list_entry){
		if(tmp->id == id){
			q = tmp;
RW_WRLOCK(timer_lock);
			LIST_REMOVE(tmp, list_entry);		
RW_UNLOCK(timer_lock);
			skyeye_free(q);
			return No_exp;
		}
	}
	return Invarg_exp;
}

/* list the attributes of all timer scheduler */
void list_timer_scheduler(void)
{
	struct event *tmp ;
	printf("id\tperiod\tdleta\tmode\n");
	LIST_FOREACH(tmp, &timer_head,list_entry){
		printf("%d\t%d\t%d\t%s\n", tmp->id, tmp->expiration, tmp->delta, (tmp->mode==0)?"Oneshot_sched":"Periodic_sched");
	}


}

int fini_timer_scheduler(){
	struct event *tmp ;
	struct event *q = NULL;

	setitimer(ITIMER_VIRTUAL, &ovalue, NULL);
	
	LIST_FOREACH(tmp, &timer_head,list_entry){
			q = tmp;
RW_WRLOCK(timer_lock);
			LIST_REMOVE(tmp, list_entry);		
RW_UNLOCK(timer_lock);
			skyeye_free(q);
	}
	
	RWLOCK_DESTROY(timer_lock);
	if(LIST_EMPTY(&timer_head))		
		return No_exp;
}
/* timer cheduler end */
