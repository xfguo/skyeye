#ifndef __SKYEYE_LOG_H__
#define __SKYEYE_LOG_H__
#include "skyeye_types.h"
void skyeye_log(log_level_t log_level, char* func_name, char* format,...);
char* get_front_message();
#endif
