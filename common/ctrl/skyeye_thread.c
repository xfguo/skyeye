/*
        skyeye_thread.c - create and control main thread
        Copyright (C) 2003-2011 Michael.Kang
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
 * 11/25/2010   Michael.Kang  <blackfin.kang@gmail.com>
 */
#include <unistd.h>
#include <skyeye_types.h>
#include <skyeye_thread.h>
#include <skyeye_callback.h>
#include <skyeye_exec.h>
#include <skyeye_log.h>
#include <sim_control.h>

/**
* @brief skyeye running flag
*/
static bool_t skyeye_running;

/**
* @brief the default running exec object
*/
static skyeye_exec_t* default_exec;

/**
* @brief pause skyeye running
*/
void skyeye_pause(void){
	skyeye_running = False;
}

/**
* @brief set the running is true
*/
void skyeye_continue(void){
	skyeye_running = True;
}

/**
* @brief start the simulator
*/
void skyeye_start(void){
	skyeye_running = True;
}

/* compatiable with old version */
/* 
 * some unknown exception happened.we need to stop the simulator
 * and return to CLI.
 */

/**
* @brief some unknown exception happened.we need to stop the simulator 
*	 and return to CLI.
*
* @param ret
*/
void skyeye_exit(int ret){
	skyeye_pause();
	skyeye_log(Error_log, __FUNCTION__, "Some unknown exception happened.\n");
	SIM_fini();
}

void skyeye_break(void){
}

#if 0
bool_t skyeye_is_running(void){
	return skyeye_running;
} 
#endif

/*
 * mainloop of simulatior
 */
void skyeye_loop(generic_arch_t *arch_instance){
	for (;;) {
		/* check if we need to run some callback functions at this time */
		exec_callback(Step_callback, arch_instance);
        	while (!skyeye_running) {
                        /*
                         * spin until it's time to go.  this is useful when
                         * we're not auto-starting.
                         */
			usleep(100);
                }
		/* run step once */
		arch_instance->step_once ();
	}
}

/**
* @brief get the default exec object
*
* @param arch_instance
*
* @return 
*/
skyeye_exec_t* get_default_exec(arch_instance){
	if(default_exec == NULL){
		default_exec = create_exec();
		default_exec->run = skyeye_loop;
		default_exec->stop = skyeye_pause;
		default_exec->exec_id = 0;
		default_exec->priv_data = arch_instance;
	}
	return default_exec;
}
