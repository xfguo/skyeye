#ifndef __SKYEYE_INTERNAL_H__
#define __SKYEYE_INTERNAL_H__
#include "skyeye_config.h"
int skyeye_option_init (skyeye_config_t * config);
void init_callback();
void init_command_list();
void init_stepi();
exception_t init_module_list();
void SKY_load_all_modules(char* lib_dir, char* suffix);
void SKY_unload_all_modules();
int init_thread_scheduler();
int init_timer_scheduler();
void init_arch();
void init_bus();
void init_mach();
int init_bp();
int init_chp();
void add_chp_data(void *data, int size, char *name);
void init_conf_obj();
void stop_all_cell();
void start_all_cell();
void destroy_threads(void);
exception_t mem_reset ();
void io_reset (void * state);

#endif
