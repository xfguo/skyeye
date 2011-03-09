#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "skyeye_pref.h"
#include "skyeye_log.h"
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

/* a set of apis for operating some boot parameters*/
void set_exec_file(const char *filename)
{
	if (filename == NULL) {
		skyeye_log(Error_log, __func__,
					"kernel filename is NULL\n");
		exit(0);
	}
	skyeye_pref->exec_file = strdup(filename);
}
char *get_exec_file()
{
	return skyeye_pref->exec_file;
}

void set_exec_load_base(const generic_address_t addr)
{
/*
	if(addr == NULL)
	{
		skyeye_log(Error_log, __func__, "addr is NULL\n");
		exit(0);
	}
*/
	skyeye_pref->exec_load_base = addr;
}
generic_address_t get_exec_load_base()
{
	return skyeye_pref->exec_load_base;
}

void set_exec_load_mask(const uint32_t addr)
{
/*
	if(addr == NULL)
		exit(0);
*/
	skyeye_pref->exec_load_mask = addr;
}
uint32_t get_exec_load_mask()
{
	return skyeye_pref->exec_load_mask;
}

void set_conf_filename(const char *filename)
{
	if (filename == NULL) {
		skyeye_log(Error_log, __func__,
					"The path of skyeye.conf is NULL\n");
		exit(0);
	}
	skyeye_pref->conf_filename = strdup(filename);
}

/*
char *get_conf_filename()
{
	return skyeye_pref->conf_filename;
}
*/
void set_interactive_mode(const bool_t mode)
{
	skyeye_pref->interactive_mode = mode;
}
bool_t get_interactive_mode()
{
	return skyeye_pref->interactive_mode;
}

void set_endian(const endian_t endian)
{
	skyeye_pref->endian = endian;
}
bool_t get_endian()
{
	return skyeye_pref->endian;
}

void set_autoboot(const bool_t value)
{
	skyeye_pref->autoboot = value;
}
bool_t get_autoboot()
{
	return skyeye_pref->autoboot;
}
void set_user_mode(const uint32_t value){
	skyeye_pref->user_mode_sim = value;
}
bool_t get_user_mode(){
	return skyeye_pref->user_mode_sim;
}

void set_uart_port(const uint32_t value){
	skyeye_pref->uart_port = value;
}
uint32_t get_uart_port(){
	return skyeye_pref->uart_port;
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
