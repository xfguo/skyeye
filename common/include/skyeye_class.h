#ifndef __SKYEYE_CLASS_H__
#define __SKYEYE_CLASS_H__
#include <skyeye_obj.h>
typedef enum{
	/* Need to be saved for checkpointing */
	Class_Persistent,
	/* Do not need to be saved for checkpointing */
	Class_temp
}class_kind_t;

typedef enum {
        Val_Invalid,
        Val_String,
        Val_Integer,
        Val_Floating,
        Val_List,
        Val_Data,
        Val_Nil,
        Val_Object,
        Val_Dict,
        Val_Boolean,

        Sim_Val_Unresolved_Object
} value_kind_t;

typedef struct attr_value{
	value_kind_t kind;
        union {
                const char *string;          /* Sim_Val_String */
                integer_t integer;           /* Sim_Val_Integer */
                bool_t boolean;                /* Sim_Val_Boolean */
                double floating;             /* Sim_Val_Floating */

                /* Sim_Val_List */
                struct {
                        size_t size;
                        struct attr_value *vector;  /* [size] */
                } list;

                /* Sim_Val_Dict */
                struct {
                        size_t size;
                        struct attr_dict_pair *vector;  /* [size] */
                } dict;

                /* Sim_Val_Data */
                struct {
                        size_t size;
                        uint8 *data;         /* [size] */
                } data;

                struct conf_object *object;       /* Sim_Val_Object */
	}u;
}attr_value_t;

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
