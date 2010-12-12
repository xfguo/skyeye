#include <assert.h>
#include "skyeye_thread.h"
#include "skyeye_cell.h"

static skyeye_cell_t* default_cell = NULL;
void add_to_cell(skyeye_exec_t* exec, skyeye_cell_t* cell){
	exec->exec_id = cell->max_exec_id++;
	LIST_INSERT_HEAD(&cell->exec_head, exec, list_entry);
}

work_thread_t* get_thread_by_cell(skyeye_cell_t* cell){
	return get_thread_by_id(cell->thread_id);
}

/**
* @brief the default function for all the thread containing cell
*/
static void cell_running(conf_object_t* argp){
	struct skyeye_exec_s *iterator;
	skyeye_cell_t* cell = (skyeye_cell_t *)get_cast_conf_obj(argp, "skyeye_cell_t");
	assert(cell != NULL);
	while(1){
		while(!SIM_is_running()){
			usleep(100);
		}
		LIST_FOREACH(iterator, &cell->exec_head,list_entry){
			cell->current_exec_id = iterator->exec_id;
			iterator->run(iterator->priv_data);
		}
	}
}

/**
* @brief create a new cell with its thread
*
* @param cell
*/
skyeye_cell_t* create_cell(){
	pthread_t id;
	skyeye_cell_t* cell = (skyeye_cell_t*)skyeye_mm(sizeof(skyeye_cell_t));
	conf_object_t* argp = get_conf_obj_by_cast(cell, "skyeye_cell_t");
	create_thread(cell_running, argp, &id);
	cell->thread_id = id;
	cell->current_exec_id = cell->max_exec_id = 0;
	LIST_INIT(&cell->exec_head);
	return cell;
}

/**
* @brief  Get the default cell instance
*
* @return  the default cell instance
*/
skyeye_cell_t* get_default_cell(){
	if(default_cell == NULL){
		default_cell = create_cell();
		//printf("In %s, default_cell->thread_id=%d\n", __FUNCTION__, default_cell->thread_id);
	}
	return default_cell;
}

/**
* @brief add an exec object to the default cell
*
* @param exec
*/
void add_to_default_cell(skyeye_exec_t* exec){
	add_to_cell(exec, get_default_cell());
}
/**
* @brief start the running of a cell
*
* @param cell
*
* @return 
*/
bool_t start_cell(skyeye_cell_t* cell){
	work_thread_t* thread = get_thread_by_cell(cell);
	start_thread(thread);
}

/**
* @brief stop the running of the given cell
*
* @param cell the given cell instance
*
* @return the triggerred exception
*/
exception_t stop_cell(skyeye_cell_t* cell){
	work_thread_t* thread = get_thread_by_cell(cell);
	stop_thread(thread);
	return No_exp;
}

void start_all_cell(){
	start_all_thread();
	return;
}

void stop_all_cell(){
	stop_all_thread();
	return;
}

conf_object_t* get_current_exec_priv(pthread_t id){
	struct skyeye_exec_s *iterator ;
	skyeye_cell_t* cell = get_cell_by_thread_id(id);
	assert(cell != NULL);
	LIST_FOREACH(iterator, &cell->exec_head,list_entry){
		if(iterator->exec_id == cell->current_exec_id){
			return iterator->priv_data;
		}
	}
	return NULL;
}
