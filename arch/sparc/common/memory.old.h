/*
 * =====================================================================================
 *
 *       Filename:  memory.h
 *
 *    Description:  Memory module header file
 *
 *        Version:  1.0
 *        Created:  15/04/08 17:30:40
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

#ifndef _SPARC_MEMORY_
#define _SPARC_MEMORY_

#include "skyeye_config.h"
#include "code_cov.h"

#ifndef __SPARC_TYPES_H_
#error "arch/sparc/common/types.h header fle must be included before memory.h"
#endif

#define MAX_STRING      10

/* Memory modules */
struct _sparc_memory_segment {
	char *name;
	unsigned int base;
	unsigned int *base_register;
	unsigned int mask;
	unsigned short interrupt_line;
	char *code;
	unsigned int code_len;
	void (*fini)(struct _sparc_memory_segment *s);
	char (*read)(struct _sparc_memory_segment *s, unsigned int *result, short size, unsigned int offset);
	char (*write)(struct _sparc_memory_segment *s, short size, unsigned int offset, unsigned int value);
	void (*reset)(struct _sparc_memory_segment *s);
	void (*update)(struct _sparc_memory_segment *s);
	void *data;
	
};

/*  Memory mapped I/O   */
//struct _io_memory
//{
//    uint32 io;
//    uint32 io2;
//    uint32 io_size;
//    uint32 io2_size;
//	uint32 (*io_write)(int size, int offset, unsigned int value);
//	uint32 (*io_read)(unsigned int * result, int size, int offset);
//	uint32 (*io2_write)(int size, int offset, unsigned int value);
//	uint32 (*io2_read)(unsigned int * result, int size, int offset);
//};

struct _sparc_memory_module {
	char *name;
	void (*setup)(struct _sparc_memory_segment *s);
};

typedef enum _mem_type
{
    R = 1,
    W = 2,
    RW = 3,
}mem_type_t;

#define MAX_MEM_BANKS   10

typedef struct _sparc_memory_bank_info
{
    unsigned int num_banks;
    char bank_name[MAX_STRING];
    unsigned int addr;
    unsigned int size;
    mem_type_t type;
    struct _sparc_memory_segment s;
}sparc_mem_bank_info_t;

typedef struct _sparc_mem_banks
{
    sparc_mem_bank_info_t mb[MAX_MEM_BANKS];
    unsigned int num_banks;
}sparc_mem_banks_t;

/*-----------------------------------------------------------------------------
 *  PUBLIC INTERFACE
 *-----------------------------------------------------------------------------*/

extern sparc_mem_banks_t mem_bank_list;

void sparc_memory_init(void);
void sparc_memory_module_register(const char *name, void (*setup)(struct _sparc_memory_segment *s));
char sparc_memory_store(short size, int offset, unsigned int value);
char sparc_memory_read(unsigned int *result, short size, int offset);
struct _sparc_memory_segment *sparc_memory_find_segment_for(unsigned int offset);
void sparc_memory_module_setup_segment(char *module_name, int *base_register, int base, int len);


extern void sparc_ram_setup(struct _sparc_memory_segment *s);
extern void sparc_rom_setup(struct _sparc_memory_segment *s);
extern char sparc_memory_store(short size, int offset, unsigned int value);

#define sparc_memory_store_byte(offset, value) sparc_memory_store(8, offset, value)
#define sparc_memory_store_word16(offset, value) sparc_memory_store(16, offset, value)
#define sparc_memory_store_word32(offset, value)    \
    ({  \
     COV_PROF(WRITE_FLAG, offset);   \
     sparc_memory_store(32, offset, value);  \
     })

#define sparc_memory_read_word32(value,offset) 	sparc_memory_read(value, 32, offset)
#define sparc_memory_read_word16(value,offset) 	sparc_memory_read(value, 16, offset)
#define sparc_memory_read_byte(value,offset) 	sparc_memory_read(value, 8, offset)

#endif

