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
#include "bank_defs.h"

chp_list chp_data_list;
static int save_chp()
{
	chp_data *p;
	int i,ret;
	FILE *fp;

	fp = fopen("config", "wb");
	if(fp == NULL)
		printf("can't create file config\n");
	/* check for difference archtecture */
	for( p = chp_data_list.head; p != NULL; p = p->next){
		ret = 0;
		fprintf(fp, "%s=%d\n", p->name, p->size);

		do{
			ret += fwrite(p->data + ret, 1, p->size, fp);
		}while(p->size - ret > 0);

		fclose(fp);
	}

	save_mem_to_file();
}

static int load_chp()
{
	chp_data *p;
	int ret,i;
	FILE *fp;
	char tmp[100],tmp2[100];

	fp = fopen("config", "rb");
	if(fp == NULL)
		printf("can't create file config\n");

	/* check for difference archtecture */
	while(fgets(tmp, 100, fp) != NULL){
		for( p = chp_data_list.head; p != NULL; p = p->next){
			ret = 0;
			sprintf(tmp2, "%s=%d\n", p->name, p->size);

			if(!strcmp(tmp2, tmp)){
				do{
					ret += fread(p->data + ret + ret, 1, p->size, fp);
				}while(p->size - ret > 0);

				break;
			}else
				continue;
		}
		memset(tmp, 0, 100);
		memset(tmp2, 0, 100);
	}

	fclose(fp);

	load_mem_form_flie();
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
	add_command("read-configuration", load_chp, "load a breakpoint position.\n");
#if 0
	add_command("read-configuration", load_chpoint, "load a breakpoint position.\n");
	add_command("set-bookmark", save_chpoint_mem, "set a bookmark position.\n");
	add_command("reverse-to", reverse_to, "reverse to an old position.\n");/* step or bookmark */
#endif
}

/* destruction function for log functionality */
int chpoint_fini(){
}
