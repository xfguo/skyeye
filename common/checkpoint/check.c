#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include "skyeye_config.h"
#include "skyeye_arch.h"
#include "skyeye_callback.h"
#include "sim_control.h"
#include "skyeye_command.h"
#include "skyeye_thread.h"
#include "skyeye_mach.h"
#include "checkpoint.h"
#include "skyeye_mm.h"

chp_list chp_data_list;
static int save_chp()
{
	chp_data *p;
	int i;
	/* check for difference archtecture */
	for( p = chp_data_list.head; p != NULL; p = p->next){
		for( i = 0; i < p->size; i ++ ){
			printf("%x", ((char*)(p->data))[i]);
		}
	}
}

void add_chp_data(void *data, int size, char *name)
{
	chp_data *tmp;
	tmp = skyeye_mm_zero(sizeof(chp_data));

	tmp->data = data;
	tmp->size = size;
	tmp->next = NULL;
	tmp->name = skyeye_mm_zero(strlen(name) + 1);
	strcpy(tmp->name, name);

	if(chp_data_list.head == NULL)
		chp_data_list.head = tmp;
	if(chp_data_list.tail == NULL){
		chp_data_list.tail = tmp;
	}else{
		chp_data_list.tail->next = tmp;
		chp_data_list.tail = tmp;
	}

	chp_data_list.num ++;
}

int init_chp(){

	memset(&chp_data_list, 0, sizeof(chp_data_list));
	/* register callback function */
	//register_callback(log_pc_callback, Step_callback);
	/* add correspinding command */
	add_command("write-configuration", save_chp, "save this breakpoint position and invention.\n");
#if 0
	add_command("read-configuration", load_chpoint, "load a breakpoint position.\n");
	add_command("set-bookmark", save_chpoint_mem, "set a bookmark position.\n");
	add_command("reverse-to", reverse_to, "reverse to an old position.\n");/* step or bookmark */
#endif
}

/* destruction function for log functionality */
int chpoint_fini(){
}
