#ifndef __LOADER_H__
#define __LOADER_H__
#include "skyeye_types.h"
exception_t SKY_load_elf(const char* elf_filename, addr_type_t addr_type);
exception_t load_file(const char* filename, generic_address_t load_addr);
exception_t load_data(void* src, size_t len, generic_address_t load_addr);
endian_t get_elf_endian(const char* elf_filename);
#endif
