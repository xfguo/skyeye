#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
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

static int save_chp(char *arg)
{
	chp_data *p;
	int i,ret;
	char dir[100];
	FILE *fp;

	if(arg == NULL || *arg == '\0'){
		strcpy(dir,"default");
	}else{
		strcpy(dir, arg);
	}

	save_mem_to_file(dir);

	if(access(dir, 0) == -1){
		if(mkdir(dir, 0777)){
			printf("create dir %s failed\n", dir);
			return 0;
		}
	}

	strcat(dir,"/config");

	//fp = fopen("config", "wb");
	fp = fopen(dir, "wb");
	if(fp == NULL)
		printf("can't create file %s\n",dir);
	/* check for difference archtecture */
	for( p = chp_data_list.head; p != NULL; p = p->next){
		ret = 0;
		fprintf(fp, "%s=%d\n", p->name, p->size);
		do{
			ret += fwrite(p->data + ret, 1, p->size, fp);
		}while(p->size - ret > 0);

		fprintf(fp,"\n");
	}
	fclose(fp);
}

static int load_chp(char *arg)
{
	chp_data *p;
	int ret,i;
	FILE *fp;
	char dir[100];
	char tmp[100],tmp2[100];

	if(arg == NULL || *arg == '\0'){
		strcpy(dir,"default");
	}else{
		strcpy(dir, arg);
	}

	load_mem_form_flie(dir);

	strcat(dir,"/config");
	//fp = fopen("config", "rb");
	fp = fopen(dir, "rb");
	if(fp == NULL){
		printf("can't open file %s\n, may be it does not exist", dir);
		return 0;
	}

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

static bookmark[100] = {0};
static int set_bookmark(char *arg)
{
	if(arg == NULL || *arg == 0){
		strcpy(bookmark, "defbookmark");
	}else{
		strcpy(bookmark, arg);
	}

	save_chp(bookmark);
}

static int reverse_to(char *arg)
{
	generic_arch_t* arch_instance = get_arch_instance("");
	if(!arch_instance)
		return 1;
	uint32 step = arch_instance->get_step();

	if(bookmark[0])
		load_chp(bookmark);
	else
		printf("please set a bookmark first before using reverse command\n");

	if(arg == NULL || *arg == 0){

	}else{
		int ret;
		step -= arch_instance->get_step();
		ret = atoi(arg);
		ret = step - ret;

		if(ret > 0)
			skyeye_stepi(ret);
		else
			printf("haven't exec enought steps,reverse to the bookmark\n");
	}
}

int init_chp(){

	memset(&chp_data_list, 0, sizeof(chp_data_list));
	/* register callback function */
	//register_callback(log_pc_callback, Step_callback);
	/* add correspinding command */
	add_command("write-configuration", save_chp, "save this breakpoint position and invention.\n");
	add_command("read-configuration", load_chp, "load a breakpoint position.\n");
	add_command("set-bookmark", set_bookmark, "set a bookmark position.\n");
	add_command("reverse-to", reverse_to, "reverse to an old position.\n");/* step or bookmark */
}

/* destruction function for log functionality */
int chpoint_fini(){
}
