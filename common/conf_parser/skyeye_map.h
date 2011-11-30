#include "skyeye_types.h"
#ifdef __cplusplus
extern "C"{
#endif
typedef enum {
	SE_INSERT = 0,
	SE_DELETE,
	SE_FIND
}map_method_t;

typedef struct SE_ENTRY{
	char *key;
	void *data;
}se_entry;

exception_t skyeye_map_operate(se_entry *entry, map_method_t method);
exception_t put_conf_obj(char* objname, void* obj);
void* get_conf_obj(char* objname);
void skyeye_erase_map(void);
#ifdef __cplusplus
}
#endif
