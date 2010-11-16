#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>
#include "skyeye_dyncom.h"
typedef uint32_t (*read_memory_t)(cpu_t *cpu, addr_t addr, uint32_t size);
typedef void (*write_memory_t)(cpu_t *cpu, addr_t addr, uint32_t value, uint32_t size);

extern read_memory_t read_memory;
extern write_memory_t write_memory;

/* 
 * set memory read/write operator
 */
void set_memory_operator(read_memory_t read_memory_op, write_memory_t write_memory_op);

#endif
