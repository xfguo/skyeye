#include <iostream>
#include <string>
#include <map>
#include <stdio.h>
#include "skyeye_map.h"
#include "skyeye_types.h"
#include "skyeye_log.h"

using namespace std;

/* Create skyeye_map */
map<string , void*> skyeye_map;

/*
 * @brief skyeye_map operations that comprise insert, delete and find.
 *
 * @param entry
 * @param method
 *
 * @return
 */
exception_t skyeye_map_operate(se_entry *entry, map_method_t method)
{
	int map_num;
	char *key = entry->key;
	void *data = entry->data;

	switch(method){
		case SE_FIND:{
			map<string ,void*>::iterator iter;
			iter = skyeye_map.find(string(key));
			if(iter != skyeye_map.end()){
				entry->data = iter->second;
				return No_exp;
			}else{
				skyeye_log(Error_log,
					"skyeye_map_operate",
					"Find skyeye_map error!\n");
				break;
			}
		}
		case SE_INSERT:{
			pair<map<string, void*>::iterator, bool> Insert_Pair;
			Insert_Pair = skyeye_map.insert
				(map<string, void*>::value_type
				(string(key), data));
			if(Insert_Pair.second)
				return No_exp;
			else{
				skyeye_log(Error_log,
					"skyeye_map_operate",
					"Insert skyeye_map error!\n");
				break;
			}
		}
		case SE_DELETE:
			map_num = skyeye_map.size() - 1;
			skyeye_map.erase(string(key));
			if(map_num == skyeye_map.size())
				return No_exp;
			else{
				skyeye_log(Error_log,
					"skyeye_map_operate",
					"Delete skyeye_map error!\n");
				break;
			}
		default:
			return Invarg_exp;
	}
	return Invarg_exp;
}

/*
 * @brief place an object to the skyeye_map
 *
 * @param objname
 * @param obj
 *
 * @return
 */
exception_t put_conf_obj(char* objname, void* obj)
{
	se_entry entry;
	entry.key = objname;
	entry.data = obj;

	exception_t ret = skyeye_map_operate(&entry, SE_INSERT);

	return ret;
}

/*
 * Get a named object from hash table
 */
void* get_conf_obj(char* objname)
{
	se_entry entry;
	entry.key = objname;
	entry.data = NULL;
	exception_t ret;

	ret = skyeye_map_operate(&entry, SE_FIND);
	if(ret == No_exp)
		return entry.data;
	else
		return NULL;
}
/*
 * erase skyeye_map and free memory
 */
void skyeye_erase_map(void)
{
	map<string, void*>::iterator first = skyeye_map.begin();
	map<string, void*>::iterator last = skyeye_map.end();
	if(first == last) /* skyeye_map is empty */
		return ;
	else
		skyeye_map.erase(first, last);
	return ;
}
