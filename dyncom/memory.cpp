#include "memory.h"
read_memory_t read_memory = NULL;
write_memory_t write_memory = NULL;
/* 
 * set memory read/write operator
 */
void set_memory_operator(read_memory_t read_memory_op, write_memory_t write_memory_op){
	if((read_memory_op == NULL) || (write_memory_op == NULL)){
		fprintf(stderr, "memory_operator can not be NULL!\n");
		exit(-1);
	}
	read_memory = read_memory_op;
	write_memory = write_memory_op;
}

