#include "skyeye_mm.h"
void* skyeye_mm(size_t sz){
	//printf("In %s, sz=0x%x\n", __FUNCTION__, sz);
	void* result = (unsigned long)malloc(sz);
	return result;
}
void* skyeye_mm_zero(size_t sz){
	void* result = malloc(sz);
	memset(result, 0, sz);
	return result;
}

char* skyeye_strdup(const char* s){
	char* result = strdup(s);
	return result;
}
void skyeye_free(void* p){
	free(p);
	p = NULL;
	return;
}

