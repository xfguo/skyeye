/*
        skyeye_sched.h - some definition for skyeye scheduler.

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

#ifndef __SKYEYE_SCHED_H__
#define __SKYEYE_SCHED_H__
#include <sys/time.h>

/* timer scheduler mode */
typedef enum{
	Oneshot_sched,		/* scheduled once */
	Periodic_sched		/* scheduled once in a period */
}sched_mode_t;

 
/* define scheduler callback function type */ 
typedef	void(*sched_func_t)(void *arg);

/* 
 * create a  thread scheduler. 
 */
int create_thread_scheduler(unsigned int ms, sched_mode_t mode, sched_func_t func, void *arg, int *id);

/*
 * mode the attributes of a scheduler
 */
int mod_thread_scheduler(int id, unsigned int ms, sched_mode_t mode);

/*
 * remove a scheduler from the queue 
 */
int del_thread_scheduler(int id);


/*
 * initializa the thread scheduler  queue 
 */
int init_thread_scheduler(void);

/*
 * destroy the thread scheduler queue 
 */
int fini_thread_scheduler(void);

/*
 *list the attributes of all thread scheduler
 */
void list_thread_scheduler(void);

/* 
 * create a  timer scheduler. 
 */
int create_timer_scheduler(unsigned int ms, sched_mode_t mode, sched_func_t func, void *arg, int *id);

/*
 * mode the attributes of a scheduler
 */
int mod_timer_scheduler(int id, unsigned int ms, sched_mode_t mode);

/*
 * remove a scheduler from the queue 
 */
int del_timer_scheduler(int id);

/*
 * initializa the timer scheduler  queue 
 */
int init_timer_scheduler(void);

/*
 * destroy the timer scheduler queue 
 */
int fini_timer_scheduler(void);

/*
 *list the attributes of all timer scheduler
 */
void list_timer_scheduler(void);
#endif
