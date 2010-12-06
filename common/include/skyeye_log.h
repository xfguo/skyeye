#ifndef __SKYEYE_LOG_H__
#define __SKYEYE_LOG_H__
#include "skyeye_types.h"
#ifdef __cplusplus
 extern "C" {
#endif

void skyeye_log(log_level_t log_level, const char* func_name, char* format,...);
char* get_front_message();

#ifdef __cplusplus
}
#endif

#endif
