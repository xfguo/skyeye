#ifndef __SKYEYE_OPTIONS_H__
#define __SKYEYE_OPTIONS_H__
#include "skyeye_types.h"
#ifdef __cplusplus
 extern "C" {
#endif

#define MAX_OPTION_NAME 32
#define MAX_PARAM_NAME  256
#if 0
typedef struct skyeye_option_s
{
	char *option_name;
	int (*do_option) (struct skyeye_option_s * this_opion,
			  int num_params, const char *params[]);
	char* helper;
#if 0
	int do_num;		/*number of call do_option function */
	int max_do_num;		/*max number of call do_option function. 
				   where should we reset these values? */
#endif
	struct skyeye_option_t *next;
} skyeye_option_t;
#endif
struct skyeye_option_s
{
	char *option_name;
	int (*do_option) (struct skyeye_option_s * this_opion,
			  int num_params, const char *params[]);
	char* helper;
#if 0
	int do_num;		/*number of call do_option function */
	int max_do_num;		/*max number of call do_option function. 
				   where should we reset these values? */
#endif
	struct skyeye_option_s *next;
};
typedef struct skyeye_option_s skyeye_option_t;

typedef int(*do_option_t)(skyeye_option_t *option, int num_params, const char *params[]) ; 
exception_t register_option(char* option_name, do_option_t do_option_func, char* helper);
int do_deprecated_option (skyeye_option_t * this_option, int num_params,
                 const char *params[]);

int
split_param (const char *param, char *name, char *value);
int
parse_line_formatted (int num_params, const char *params[]);
int get_interrupts (char value[], uint32 * interrupts);

#ifdef __cplusplus
}
#endif

#endif
