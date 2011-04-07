#ifndef _SKYEYE_OS_H_
#define _SKYEYE_OS_H_

#include <skyeye_options.h>

void init_os_option();

int do_os_option(skyeye_option_t *this_option, int num_params,
		const char *params[]);

#endif
