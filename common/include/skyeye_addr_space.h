/**
* @file skyeye_addr_space.h
* @brief The addr_space class
* @author Michael.Kang blackfin.kang@gmail.com
* @version 0.1
* @date 2011-07-11
*/

#ifndef __SKYEYE_ADDR_SPACE_H__
#define __SKYEYE_ADDR_SPACE_H__
#include <skyeye_types.h>
#include <memory_space.h>
#define MAX_MAP 8

typedef struct map_info{
	generic_address_t base_addr;
	generic_address_t length;
	generic_address_t start;
	memory_space_intf* memory_space;
	int priority;
	int swap_endian;
}map_info_t;

typedef struct addr_space{
	conf_object_t* obj;
	map_info_t* map_array[MAX_MAP];
	memory_space_intf* memory_space;
}addr_space_t;

addr_space_t* new_addr_space(char* obj_name);
void free_addr_space(char* obj_name);

exception_t add_map(addr_space_t* space, generic_address_t base_addr, generic_address_t length, generic_address_t start, memory_space_intf* memory_space, int priority, int swap_endian);

#endif
