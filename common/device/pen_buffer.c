#include <stdio.h>
/*
const int pen_buffer_sz = 6;
static int pen_buffer[pen_buffer_sz];
*/
//static int pen_buffer[6];
static int* pen_buffer = NULL;
void register_pen_buffer(int* pb){
	pen_buffer = pb;
}

int* get_pen_buffer(){
	/*
	if(pen_buffer == NULL){
		printf("pen_buffer not implemented.\n");
		return NULL;
	}
	return pen_buffer;
	*/	
	//printf("pen_buffer not implemented.\n");
	return pen_buffer;
}

