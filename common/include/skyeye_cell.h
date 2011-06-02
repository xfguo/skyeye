#ifndef __SKYEYE_CELL_H__
#define __SKYEYE_CELL_H__
#include <pthread.h>
#include "skyeye_exec.h"
#include <skyeye_queue.h>
#ifdef __cplusplus
 extern "C" {
#endif

typedef struct skyeye_cell_s{
	/* The list used to store all the exec object in the cell */
	LIST_HEAD(exec_list_head, skyeye_exec_s) exec_head;
	/* the id to identify the current running exec object */
	int current_exec_id;
	pthread_t thread_id;
	int max_exec_id;
}skyeye_cell_t;

work_thread_t* get_thread_by_cell(skyeye_cell_t* cell);
skyeye_cell_t* get_cell_by_thread_id(pthread_t id);
void add_to_cell(skyeye_exec_t* exec, skyeye_cell_t* cell);
void add_to_default_cell(skyeye_exec_t* exec);
/*
void del_from_cell(skyeye_exec_t* exec, skyeye_cell_t* cell);
void move_to_cell(skyeye_exec_t* exec, skyeye_cell_t* src, skyeye_cell_t* dst);
*/
skyeye_cell_t* create_cell();
#ifdef __cplusplus
}
#endif

#endif
