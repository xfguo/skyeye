#ifndef __SKYEYE_EXEC_H__
#define __SKYEYE_EXEC_H__
#include "skyeye_thread.h"
#include "skyeye_types.h"
#include "skyeye_queue.h"
#ifdef __cplusplus
 extern "C" {
#endif

typedef struct skyeye_exec_s{
	void (*run)(conf_object_t *obj);
	void (*stop)(conf_object_t *obj);
	conf_object_t* priv_data;
	int exec_id;
	LIST_ENTRY (skyeye_exec_s)list_entry;
}skyeye_exec_t;

/**
* @brief Get the private data of the current exec object
*
* @param id the id of pthread that exec belongs to
*
* @return 
*/
conf_object_t* get_current_exec_priv(pthread_t id);

skyeye_exec_t* create_exec();
#ifdef __cplusplus
}
#endif

#endif
