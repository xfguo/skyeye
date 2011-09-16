#ifndef __SKYEYE_EXEC_INFO_H__
#define __SKYEYE_EXEC_INFO_H__

#include "skyeye_types.h"

#ifdef __cplusplus
 extern "C" {
#endif 

struct _sky_exec_info_s{
	char* exec_argv;
	short exec_argc;
	char* exec_envp;
	short exec_envc;

	uint32_t load_addr;
	uint32_t start_code;
	uint32_t end_code;
	uint32_t start_data;
	uint32_t end_data;
	uint32_t brk;
	uint32_t entry;

	uint32_t arch_page_size;
	uint32_t arch_stack_top;
	uint32_t initial_sp;
	uint32_t mmap_access;
};
typedef struct _sky_exec_info_s sky_exec_info_t;

sky_exec_info_t* get_skyeye_exec_info();

void retrieve_info();

void exec_stack_init();

#ifdef __cplusplus
}
#endif 

#endif
