#ifndef __SKYEYE_SYMBOL_H__
#define __SKYEYE_SYMBOL_H__
#include "skyeye_types.h"
void init_symbol_table(char* filename, char* arch_name);
char *get_sym(generic_address_t address);

/**
* @brief Get the target name defined in bfd library
*
* @param arch_name the arch name in skyeye
*
* @return the target name in bfd library
*/
char* get_bfd_target(const char* arch_name);

struct symbolInfo {
        struct symbolInfo *next;
        uint8 *name;
        uint32_t address;
};
void _print_func_name(uint32_t address);

#endif
