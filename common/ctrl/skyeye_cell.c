/* Copyright (C) 
* 2011 - Michael.Kang blackfin.kang@gmail.com
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
* 
*/
/**
* @file skyeye_cell.c
* @brief the cell implementation
* @author Michael.Kang blackfin.kang@gmail.com
* @version 
* @date 2011-04-30
*/

#include <assert.h>
#include <unistd.h>
#include "skyeye_obj.h"
#include "skyeye_internal.h"
#include "skyeye_thread.h"
#include "skyeye_cell.h"
#include "skyeye_mm.h"
#include "sim_control.h"
#include "skyeye_callback.h"

/**
* @brief the default cell
*/
static skyeye_cell_t* default_cell = NULL;

/**
* @brief Add an exec object to the cell
*
* @param exec
* @param cell
*/
void add_to_cell(skyeye_exec_t* exec, skyeye_cell_t* cell){
	exec->exec_id = cell->max_exec_id++;
	LIST_INSERT_HEAD(&cell->exec_head, exec, list_entry);
}

/**
* @brief get the thread object by its cell
*
* @param cell
*
* @return 
*/
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
		generic_arch_t *arch_instance = get_arch_instance(NULL);
		exec_callback(Step_callback, arch_instance);
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

/**
* @brief start all the cell
*/
void start_all_cell(){
	start_all_thread();
	return;
}

/**
* @brief stop all the cell
*/
void stop_all_cell(){
	stop_all_thread();
	return;
}

/**
* @brief get the current running exec object from the cell
*
* @param id
*
* @return 
*/
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
