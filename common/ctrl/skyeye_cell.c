#include "skyeye_thread.h"
#include "skyeye_cell.h"
void add_to_cell(skyeye_exec_t* exec, skyeye_cell_t* cell){
	LIST_INSERT_HEAD(&cell->exec_head, exec, list_entry);
}

/**
* @brief the default function for all the thread containing cell
*/
static void cell_running(skyeye_cell_t* cell){
	struct skyeye_exec_s *iterator ;
	work_thread_t* thread = get_thread_by_cell(cell);
	while(1){
		while(thread->state != Running_state){
			usleep(100);
		}
		LIST_FOREACH(iterator, &cell->exec_head,list_entry){
			cell->current_exec_id = iterator->exec_id;
			iterator->run(iterator->priv_data);
		}
	}
}

/**
* @brief create a cell
*
* @param cell
*/
skyeye_cell_t* create_cell(){
	pthread_t id;
	skyeye_cell_t* cell = (skyeye_cell_t*)skyeye_mm(sizeof(skyeye_cell_t));
	create_thread(cell_running, cell, &id);
	cell->thread_id = id;
	cell->current_exec_id = 0;
	return cell;
}

/**
* @brief 
*
* @return 
*/
skyeye_cell_t* create_default_cell(conf_object_t* arch_instance){
	skyeye_cell_t* cell = create_cell();
	skyeye_exec_t* exec = get_default_exec(arch_instance);
	add_to_cell(exec, cell);
	return cell;
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

conf_object_t* get_current_exec_priv(pthread_t id){
	struct skyeye_exec_s *iterator ;
	skyeye_cell_t* cell = get_cell_by_thread_id(id);
	LIST_FOREACH(iterator, &cell->exec_head,list_entry){
		if(iterator->exec_id == cell->current_exec_id){
			return iterator->priv_data;
		}
	}
	return NULL;
}

