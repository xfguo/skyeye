#ifndef __SKYEYE_CLASS_H__
#define __SKYEYE_CLASS_H__
#include <skyeye_obj.h>
#include <skyeye_attr.h>
typedef enum{
	/* Need to be saved for checkpointing */
	Class_Persistent,
	/* Do not need to be saved for checkpointing */
	Class_temp
}class_kind_t;

typedef struct skyeye_class{
	char* class_name;
	char* class_desc;
	conf_object_t* (*new_instance)(char* obj_name);
	exception_t (*free_instance)(char* obj_name);
	attr_value_t* (*get_attr)(const char* attr_name, conf_object_t* obj);
	exception_t (*set_attr)(const char* attr_name, conf_object_t* obj, attr_value_t);
	char** interface_list;
}skyeye_class_t;
void SKY_register_class(const char* name, skyeye_class_t* skyeye_class);

conf_object_t* pre_conf_obj(const char* objname, const char* class_name);

//SIM_register_typed_attribute

#endif
