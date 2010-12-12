#include <stdlib.h>
#include "skyeye_mm.h"
#include "skyeye_mach.h"
#include "skyeye_options.h"
#include "skyeye_config.h"
static machine_config_t* mach_list;
int
do_mach_option (skyeye_option_t * this_option, int num_params,
                const char *params[])
{
        int ret;
        machine_config_t *mach = get_mach(params[0]);
	skyeye_config_t* config = get_current_config();
	if(mach != NULL){
		config->mach = mach;
		skyeye_log(Info_log,__FUNCTION__,"mach info: name %s, mach_init addr %p\n", config->mach->machine_name, config->mach->mach_init);
		ret = 0;
	}
        else{
                SKYEYE_ERR ("Error: Unknown mach name \"%s\"\n", params[0]);
		ret = -1;
        }
        return ret;
}

void init_mach(){
	mach_list = NULL;
	register_option("mach", do_mach_option, "machine option");
}
void register_mach(const char* mach_name, mach_init_t mach_init){
	machine_config_t * mach;
	mach = skyeye_mm(sizeof(machine_config_t));
	mach->machine_name =  skyeye_strdup(mach_name);
	mach->mach_init = mach_init;
	mach->next = mach_list;
	mach_list = mach;
	//skyeye_log(Debug_log, __FUNCTION__, "regiser mach %s successfully.\n", mach->machine_name);
}
/*
 * get machine by its name
 */
machine_config_t * get_mach(const char* mach_name){
	machine_config_t* node;
	node = mach_list;
	while(node){
		//if(!strncmp(node->machine_name, mach_name, strlen(node->machine_name))){
		if(!strcmp(node->machine_name, mach_name)){
			return node;
		}
		node = node->next;
	}
	return NULL;
}

#if 0
machine_config_t* get_mach(skyeye_config_t* config){
	return config->mach;
}
#endif

machine_config_t * get_mach_list(){
	return mach_list;
}

machine_config_t *get_current_mach(){
	skyeye_config_t* config = get_current_config();
	return config->mach;
}
