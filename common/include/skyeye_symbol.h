#ifndef __SKYEYE_SYMBOL_H__
#define __SKYEYE_SYMBOL_H__
#include "skyeye_types.h"
void init_symbol_table(char* filename);
char *get_sym(generic_address_t address);
#endif
