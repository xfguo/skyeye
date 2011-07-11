#ifndef __SKYEYE_CLASS_H__
#define __SKYEYE_CLASS_H__
typedef skyeye_class{
	char* class_name;
	conf_obj_t* (new_instance)(char* obj_name);
	exception_t (free_instance)(char* obj_name);
	char** interface_list;
}skyeye_class_t;
void register_class(skyeye_class_t* skyeye_class);
#endif
