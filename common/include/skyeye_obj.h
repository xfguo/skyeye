#ifndef __SKYEYE_OBJ_H__
#define __SKYEYE_OBJ_H__
#include <skyeye_types.h>
#ifdef __cplusplus
 extern "C" {
#endif

void* get_cast_conf_obj(conf_object_t* conf_obj, const char* type_string);
conf_object_t* get_conf_obj_by_cast(void* obj, const char* type_string);

#ifdef __cplusplus
}
#endif

#endif
