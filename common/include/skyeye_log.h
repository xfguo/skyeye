#ifndef __SKYEYE_LOG_H__
#define __SKYEYE_LOG_H__
#include "skyeye_types.h"
#include <stdio.h>
#ifdef __cplusplus
 extern "C" {
#endif

#ifdef DEBUG
#define DBG(fmt, ...) do { fprintf(stderr, fmt, ## __VA_ARGS__); } while (0)
#else
#define DBG(fmt, ...) do { } while (0)
#endif

void skyeye_log(log_level_t log_level, const char* func_name, char* format,...);
char* get_front_message();

char* get_exp_str(exception_t exp);
#ifdef __cplusplus
}
#endif

#endif
