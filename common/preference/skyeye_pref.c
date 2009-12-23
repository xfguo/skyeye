#include <stdlib.h>
#include <assert.h>
#include "skyeye_pref.h"
static sky_pref_t *skyeye_pref;
static exception_t init_skyeye_pref(sky_pref_t** pref){
	*pref = malloc(sizeof(sky_pref_t));
	//skyeye_log(Debug_log, __FUNCTION__, "pref = 0x%x\n", *pref);
	if(*pref == NULL){
		skyeye_log(Error_log, __FUNCTION__, get_exp_str(Malloc_exp));
		return Malloc_exp;
	}
	memset(*pref, 0, sizeof(sky_pref_t));
	
	return No_exp;
}
void update_skyeye_pref(sky_pref_t *pref){
	/*
	if(skyeye_pref->module_search_dir)
		free(skyeye_pref->module_search_dir)
	skyeye_pref->module_search_dir = 
	*/
}

/* FIXME, that is maybe not thread-safe */
sky_pref_t * get_skyeye_pref(){
	if(skyeye_pref == NULL){
		init_skyeye_pref(&skyeye_pref);/* set the load base */
		skyeye_pref->exec_load_base = 0x0;
        	skyeye_pref->exec_load_mask = 0xFFFFFFFF;

	}
	//skyeye_log(Debug_log, __FUNCTION__, "skyeye_pref = 0x%x\n", skyeye_pref);
	return skyeye_pref;
}

char* get_conf_filename(){
	sky_pref_t* pref = get_skyeye_pref();
	assert(pref != NULL);
	return pref->conf_filename;
}
