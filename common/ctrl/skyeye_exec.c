#include "skyeye_cell.h"
#include "skyeye_exec.h"
#include "skyeye_mm.h"
void register_exec(skyeye_exec_t* exec, skyeye_cell_t* cell){
	add_to_cell(exec, cell);
}

skyeye_exec_t* create_exec(){
	skyeye_exec_t* exec = skyeye_mm(sizeof(skyeye_exec_t));
	exec->exec_id = 0;
	exec->run = exec->stop = NULL;
	exec->priv_data = NULL;
	return exec;
}
