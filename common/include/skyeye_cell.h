#ifndef __SKYEYE_CELL_H__
#define __SKYEYE_CELL_H__
#include <pthread.h>
#include "skyeye_exec.h"
#include <skyeye_queue.h>

typedef struct skyeye_cell_s{
	/* The list used to store all the exec object in the cell */
	LIST_HEAD(exec_list_head, skyeye_exec_s) exec_head;
	/* the id to identify the current running exec object */
	int current_exec_id;
	pthread_t thread_id;
}skyeye_cell_t;
/*
void add_to_cell(skyeye_exec_t* exec, skyeye_cell_t* cell);
void del_from_cell(skyeye_exec_t* exec, skyeye_cell_t* cell);
void move_to_cell(skyeye_exec_t* exec, skyeye_cell_t* src, skyeye_cell_t* dst);
*/
#endif
