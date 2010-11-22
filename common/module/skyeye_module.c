/*
        skyeye_module.c - management of model module and provide the interface for them. 
        Copyright (C) 2003 Skyeye Develop Group
        for help please send mail to <skyeye-developer@lists.gro.clinux.org>

        This program is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2 of the License, or
        (at your option) any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program; if not, write to the Free Software
        Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
/*
 * 05/16/2009   Michael.Kang  <blackfin.kang@gmail.com>
 */

#include <dlfcn.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <ltdl.h>

#include "skyeye_module.h"
#if 0
mmi_bool_t   mmi_register_instance_creator (const char * modname, mmi_instance_creator fn){
	printf("mmi_instance_creator is registered.\n");
	return mmi_true;
}
#endif

/* on *nix platform, the suffix of shared library is so. */
const char* Default_libsuffix = ".so";
/* we will not load the prefix with the following string */
const char* Reserved_libprefix = "libcommon";

const char Dir_splitter = '/';

typedef struct skyeye_modules_s{
	skyeye_module_t* list;
	int total;
}skyeye_modules_t;

static skyeye_modules_t* skyeye_modules;

static void set_module_list(skyeye_module_t *node){
	skyeye_modules->list = node;
}
exception_t init_module_list(){
	int errors = 0;
	skyeye_modules = skyeye_mm(sizeof(skyeye_modules_t));
	/* Initialise libltdl. */
	errors = lt_dlinit ();

	if(skyeye_modules == NULL)
		return Malloc_exp;
	return No_exp;
}
skyeye_module_t* get_module_list(){
	return skyeye_modules->list;
}

static exception_t register_skyeye_module(char* module_name, char* filename, void* handler){
	exception_t ret;
	skyeye_module_t* node;
	skyeye_module_t* list;
	list = get_module_list();
	if(module_name == NULL|| filename == NULL)
		return Invarg_exp;

	node = malloc(sizeof(skyeye_module_t));
	if(node == NULL){
		skyeye_log(Error_log, __FUNCTION__, get_exp_str(Malloc_exp));
		return Malloc_exp;
	}
		
	node->module_name = strdup(module_name);
	if(node->module_name == NULL){
		free(node);
		return Malloc_exp;
	}

	node->filename = strdup(filename);
	if(node->filename == NULL){
		free(node->module_name);
		free(node);
		return Malloc_exp;
	}
	
	node->handler = handler;

	node->next = list;;
	set_module_list(node);
	return No_exp;
}

/* Be careful to save a copy of the error message,
   since the  next API call may overwrite the original. */
static char *
dlerrordup (char *errormsg)
{
  char *error = (char *) lt_dlerror ();
  if (error && !errormsg)
    errormsg = strdup (error);
  return errormsg;
}
//#define Check_Failed_Module 0
exception_t SKY_load_module(const char* module_filename){
	exception_t ret;
	char **module_name;
 	lt_dlhandle * handler;
	const char* err_str = NULL;
	//skyeye_log(Debug_log, __FUNCTION__, "module_filename = %s\n", module_filename);
#ifndef Check_Failed_Module
        handler = lt_dlopenext(module_filename);
#else
        //handler = dlopen(module_filename, RTLD_LAZY);
	handler = dlopen(module_filename, RTLD_NOW);
	if(handler == NULL){
		err_str = dlerror();
		skyeye_log(Error_log, __FUNCTION__, "dll error: %s\n", err_str);
	}
	return Dll_open_exp;
#endif
        if (handler == NULL)
        {
        	err_str = dlerrordup(err_str);
                skyeye_log(Warnning_log, __FUNCTION__, "%s\n", err_str);
		return Dll_open_exp;
        }
	
	module_name = lt_dlsym(handler, "skyeye_module");
	if((err_str = dlerrordup(err_str)) != NULL){
		skyeye_log(Warnning_log, __FUNCTION__, "dll error %s\n", err_str);
		skyeye_log(Warnning_log, __FUNCTION__, "Invalid module in file %s\n", module_filename);
		lt_dlclose(handler);
		return Invmod_exp;
	}
	//skyeye_log(Debug_log, __FUNCTION__, "Load module %s\n", *module_name);
		
	ret = register_skyeye_module(*module_name, module_filename, handler);
	if(ret != No_exp){
		lt_dlclose(handler);
		return ret;
	}
	return No_exp;	
}
void SKY_load_all_modules(char* lib_dir, char* suffix){
	/* we assume the length of dirname + filename does not over 1024 */
	char full_filename[1024];
	char* lib_suffix;
	/* Find all the module under lib_dir */
	DIR *module_dir = opendir(lib_dir);
	exception_t exp;
	/*FIXME we should throw some exception. */
	if(module_dir == NULL)
		return;
	if(suffix == NULL)
		lib_suffix = Default_libsuffix;
	else
		lib_suffix = suffix;	
	struct dirent* dir_ent;
	while((dir_ent = readdir(module_dir)) != NULL){
		char* mod_name = dir_ent->d_name;
		/* exclude the library not end with lib_suffix */
		char* suffix = strrchr(mod_name, '.');
		if(suffix == NULL)
				continue;
		else{
			//skyeye_log(Debug_log, __FUNCTION__, "file suffix=%s\n", suffix);
			if(strcmp(suffix, lib_suffix))
				continue;
		}
			/* exclude the reserved library */
		if(!strncmp(mod_name, Reserved_libprefix, strlen(Reserved_libprefix)))
			continue;

		/* contruct the full filename for module */
		int lib_dir_len = strlen(lib_dir);
		memset(&full_filename[0], '\0', 1024);
		strncpy(&full_filename[0], lib_dir, lib_dir_len);	
		full_filename[lib_dir_len] = Dir_splitter;
		full_filename[lib_dir_len + 1] = '\0';
		//skyeye_log(Debug_log, __FUNCTION__, "1 full_filename=%s\n", full_filename);
		strncat(full_filename, mod_name, strlen(mod_name) + 1);
		//skyeye_log(Debug_log, __FUNCTION__, "full_filename=%s\n", full_filename);
		/* Try to load a module */
		exp = SKY_load_module(full_filename);
		if(exp != No_exp)
			skyeye_log(Info_log, __FUNCTION__, "Can not load module from file %s.\n", dir_ent->d_name);
		//}
	}
	closedir(module_dir);
}
skyeye_module_t * get_module_by_name(const char* module_name){
	skyeye_module_t* list = get_module_list();
	while(list != NULL){
		if(!strncmp(list->module_name, module_name, strlen(module_name)))
			return list;
		list = list->next;
	}
	return NULL;
}

void get_modules(){
		
}

void SKY_unload_all_modules(){
	skyeye_module_t* list = get_module_list();
        while(list != NULL){
		skyeye_module_t* node = list;
                list = list->next;
		/* unload a module and free its memory */
		if(node->handler != NULL)
			lt_dlclose(node->handler);
		if(node->module_name != NULL)
			skyeye_free(node->module_name);
		if(node->filename != NULL)
			skyeye_free(node->filename);
		skyeye_free(node);	
        }
	skyeye_modules->list = NULL;	
        return;
}
#if 0
void get_next_module(skyeye_module_t* module){
}

attr_value_t SKY_get_all_failed_modules(void){
}

attr_value_t SKY_get_all_modules(void){
}
#endif
void SKY_module_list_refresh(void){
}
#if 0
/* example for loading of dynamic library */
int main(){
	void * handler;
	char * err_str;
	void (*func)();
	handler = dlopen("./dumb_mmi.so", RTLD_LAZY);
	err_str = dlerror();
   	if (handler == NULL)
   	{
     		printf("dlopen Error, %s\n", err_str);
		exit(-1);
   	}
	func = dlsym(handler,"mmi_init");
	//func();
}
#endif
