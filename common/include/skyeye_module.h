#ifndef __SKYEYE_MODULE_H__
#define __SKYEYE_MODULE_H__
#include "skyeye_types.h"

/*
 * the contructor for module. All the modules should implement it.
 */
void module_init ()  __attribute__((constructor));

/*
 * the decontructor for module. All the modules should implement it.
 */
void module_fini () __attribute__((destructor));


typedef struct skyeye_module_s{
	/*
	 * the name for module, should defined in module as an varaible.
	 */
	char* module_name;
	/*
	 * the library name that contains module
	 */
	char* filename;
	/*
	 * the handler for module operation.
	 */
	void* handler;
	/*
	 * next node of module linklist.
	 */
	struct skyeye_module_s *next;
}skyeye_module_t;

/*
 * load all the modules in the specific directory with specific suffix.
 */
void SKY_load_all_module(const char* lib_dir, char* lib_suffix);

/*
 * load one module by its file name.
 */
exception_t SKY_load_module(const char* module_filename);

/*
 *
 */
exception_t init_module_list();

#endif
