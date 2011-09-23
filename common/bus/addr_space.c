/**
* @file addr_space.c
* @brief The addr_space class
* @author Michael.Kang blackfin.kang@gmail.com
* @version 0.1
* @date 2011-07-11
*/

#ifdef DEBUG
#undef DEBUG
#endif
//#define DEBUG
#include <skyeye_log.h>

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
	//map->target = target;
	map->memory_space = memory_space;
	map->priority = priority;

	int i = 0;
	for(; i < MAX_MAP; i++){
		map_info_t* iterator = space->map_array[i];
		if(iterator == NULL){
		 	space->map_array[i] = map;
			DBG("In %s, map added successfully @%d\n", __FUNCTION__, i);
			return No_exp;
		}
	}
	return Excess_range_exp;
}
exception_t del_map(){
	return No_exp;
}
static exception_t space_read(conf_object_t* addr_space, generic_address_t addr, void* buf, size_t count){
	addr_space_t* space = (addr_space_t*)(addr_space->obj);
	int i = 0;
	for(; i < MAX_MAP; i++){
		map_info_t* iterator = space->map_array[i];
		if(iterator == NULL)
			continue;
		DBG("In %s, i=%d, addr=0x%x, base_addr=0x%x, length=0x%x\n", __FUNCTION__, i, addr, iterator->base_addr, iterator->length);
		if(iterator->base_addr <= addr && ((iterator->base_addr + iterator->length) > addr)){
			return iterator->memory_space->read(iterator->memory_space->conf_obj, (addr - iterator->base_addr), buf, count);
		}
	}

	return Not_found_exp;
}
static exception_t space_write(conf_object_t* addr_space, generic_address_t addr, void* buf, size_t count){
	addr_space_t* space = (addr_space_t*)(addr_space->obj);
	int i = 0;
	for(; i < MAX_MAP; i++){
		map_info_t* iterator = space->map_array[i];
		if(iterator == NULL)
			continue;
		DBG("In %s, i=%d, addr=0x%x, base_addr=0x%x, length=0x%x\n", __FUNCTION__, i, addr, iterator->base_addr, iterator->length);
		if(iterator->base_addr <= addr && ((iterator->base_addr + iterator->length) > addr)){
			return iterator->memory_space->write(iterator->memory_space->conf_obj, (addr - iterator->base_addr), buf, count);
		}
	}
	return Not_found_exp;
}

/**
* @brief new instance for addr_space_t, Note that the instance is put to conf_obj hash table.
*
* @param obj_name the instance name
*
* @return  new instance
*/
addr_space_t* new_addr_space(char* obj_name){
	addr_space_t* space = skyeye_mm_zero(sizeof(addr_space_t));
	space->obj = new_conf_object(obj_name, space);
	space->memory_space = skyeye_mm_zero(sizeof(memory_space_intf));
	space->memory_space->read = space_read;
	space->memory_space->write = space_write;
	return space;
}

void free_addr_space(char* obj_name){
}
