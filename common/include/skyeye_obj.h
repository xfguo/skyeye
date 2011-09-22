#ifndef __SKYEYE_OBJ_H__
#define __SKYEYE_OBJ_H__
#include <string.h>

#include <skyeye_types.h>
#ifdef __cplusplus
 extern "C" {
#endif
#define MAX_OBJNAME 1024
void* get_cast_conf_obj(conf_object_t* conf_obj, const char* type_string);
conf_object_t* get_conf_obj_by_cast(void* obj, const char* type_string);

conf_object_t* new_conf_object(const char* objname, void* obj);

void* get_conf_obj(char* objname);
exception_t put_conf_obj(char* objname, void* obj);
/**
* @brief compose an interface object name for hash table
*
* @param iface_objname
* @param objname
* @param iface_name
*/
static inline void get_strcat_objname(char* iface_objname, const char* objname, const char* iface_name){
	memcpy(&iface_objname[0], objname, strlen(objname));
	iface_objname[strlen(objname)] = '\0';
	strcat(iface_objname, iface_name);
	return;
}

#ifdef __cplusplus
}
#endif

#endif
