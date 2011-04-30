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
* @file skyeye_exec.c
* @brief the skyeye exec implementation
* @author Michael.Kang blackfin.kang@gmail.com
* @version 
* @date 2011-04-30
*/

#include "skyeye_cell.h"
#include "skyeye_exec.h"
#include "skyeye_mm.h"

/**
* @brief register an exec object to a cell
*
* @param exec
* @param cell
*/
void register_exec(skyeye_exec_t* exec, skyeye_cell_t* cell){
	add_to_cell(exec, cell);
}

/**
* @brief create an exec object
*
* @return 
*/
skyeye_exec_t* create_exec(){
	skyeye_exec_t* exec = skyeye_mm(sizeof(skyeye_exec_t));
	exec->exec_id = 0;
	exec->run = exec->stop = NULL;
	exec->priv_data = NULL;
	return exec;
}
