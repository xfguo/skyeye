#include "skyeye_mm.h"
#include "skyeye_types.h"
int main(){
	const int sz = 0x100;
	uint32* t = skyeye_mm(sz);	
	if(!t){
		printf("mm failed.\n");
		exit(-1);
	}
	t[5] = 0x2222;
	printf("mm succ.\n");
	skyeye_free(t);
	
}
