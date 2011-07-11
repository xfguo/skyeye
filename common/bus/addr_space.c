/**
* @file addr_space.c
* @brief The addr_space class
* @author Michael.Kang blackfin.kang@gmail.com
* @version 0.1
* @date 2011-07-11
*/

#include <skyeye_types.h>
#include <memory_space.h>
#include <skyeye_addr_space.h>
#include <skyeye_mm.h>
#include "skyeye_obj.h"

exception_t add_map(addr_space_t* space, generic_address_t base_addr, generic_address_t length, generic_address_t start, memory_space_intf* memory_space, int priority, int swap_endian){
	map_info_t* map = skyeye_mm(sizeof(map_info_t));
	map->base_addr = base_addr;
	map->length = length;
	map->start = start;
	map->memory_space = memory_space;
	map->priority = priority;

	int i = 0;
	for(; i < MAX_MAP; i++){
		map_info_t* iterator = space->map_array[i];
		if(iterator == NULL){
		 	space->map_array[i] = map;
			return No_exp;
		}
	}
	return Excess_range_exp;
}
exception_t del_map(){
	return No_exp;
}
static exception_t space_read(conf_object_t* addr_space, generic_address_t addr, void* buf, size_t count){
	return No_exp;
}
static exception_t space_write(conf_object_t* addr_space, generic_address_t addr, void* buf, size_t count){
	return No_exp;
}

addr_space_t* new_addr_space(char* obj_name){
	addr_space_t* space = skyeye_mm(sizeof(addr_space_t));
	space->obj = new_conf_object(obj_name, space);
	space->memory_space = skyeye_mm(sizeof(memory_space_intf));
	space->memory_space->read = space_read;
	space->memory_space->write = space_write;
	return space;
}

void free_addr_space(char* obj_name){
}
