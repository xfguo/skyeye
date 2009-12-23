/*
 * =====================================================================================
 *
 *       Filename:  memory.c
 *
 *    Description:  RAM and ROM memory modules implementation
 *
 *        Version:  1.0
 *        Created:  15/04/08 17:30:01
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */


/*-----------------------------------------------------------------------------
 *  23/06/08 15:24:39 Aitor Viana Sanchez
 *-----------------------------------------------------------------------------*/

/* RAM and ROM memory modules */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "memory.h"
#include "debug.h"

static const int i = 1;
#define is_bigendian() ( (*(char*)&i) == 0 )

static void sparc_ram_fini(struct _sparc_memory_segment *s);
static char sparc_ram_read(struct _sparc_memory_segment *s, unsigned int *result, short size, unsigned int offset);
static char sparc_ram_write(struct _sparc_memory_segment *s, short size, unsigned int offset, unsigned int value);
static char sparc_rom_write(struct _sparc_memory_segment *s, short size, unsigned int offset, unsigned int value);
static char sparc_rom_read(struct _sparc_memory_segment *s, unsigned int *result, short size, unsigned int offset);
static void sparc_rom_reset(struct _sparc_memory_segment *s);

typedef struct _mem_data {
    char *begin;
    char *end;
    int len;
}mem_data_t;


static mem_data_t *mem_allocation(struct _sparc_memory_segment *s)
{
    mem_data_t *mem;
    int len;

    /*  FIXME: dynamic memory allocation...care */
    mem = malloc(sizeof(mem_data_t));
    len * (~s->mask) + 1;
    mem->begin = malloc((size_t)(len + 3)); // FIXME: dynamic memory allocation
    mem->end = mem->begin + len;
    mem->len = len;


    return mem;
}

/*-----------------------------------------------------------------------------
 *  PUBLIC INTERFACE
 *-----------------------------------------------------------------------------*/

void sparc_ram_setup(struct _sparc_memory_segment *s)
{
    int len;
    struct _mem_data *mem;

    mem = mem_allocation(s);
    s->data = mem;

    /* Now set the functions to use */
    s->fini = &sparc_ram_fini;
    s->read = &sparc_ram_read;
    s->write = &sparc_ram_write;
    s->reset = NULL;
    s->update = NULL;
}


void sparc_rom_setup(struct _sparc_memory_segment *s)
{

    /*  We need to allocate memory for this memory module   */
    struct _mem_data *mem;

    mem = mem_allocation(s);
    s->data = mem;

    /*  Setup the call-backs    */
    s->write = &sparc_rom_write;
    s->read = &sparc_rom_read;
    s->reset = &sparc_rom_reset;
    s->fini = &sparc_ram_fini;
    s->update = NULL;
}

/*-----------------------------------------------------------------------------
 *  PRIVATE INTERFACE
 *-----------------------------------------------------------------------------*/

static void sparc_ram_fini(struct _sparc_memory_segment *s)
{
    struct _mem_data *mem = (struct _mem_data *)s->data;
    free(mem->begin);
    free(s->name);
    free(s->data);
}

static void sparc_rom_reset(struct _sparc_memory_segment *s)
{
    struct _mem_data *mem = (struct _mem_data *)s->data;
    int x;

    /* Map out the rom in bigendian */
    for(x=0;x<s->code_len;x++) 
    {
        DBG(" code[%d] = 0x%02x\n", x, s->code[x]);
    }

    /* Endianness doesn't matter if we can't do unaligned 
     *  reads/writes.  We just convert everything to big endian */
    memcpy(mem->begin, s->code, s->code_len);

}

static char sparc_ram_read(struct _sparc_memory_segment *s, unsigned int *result, 
        short size, unsigned int offset)
{
    struct _mem_data *mem = (struct _mem_data *)s->data;
    unsigned char *ptr = (unsigned char *)(mem->begin + offset);


    if(size == 32)
        *result = (*ptr<<24) | (*(ptr+1)<<16) | (*(ptr+2)<<8) | *(ptr+3);
    else if (size == 16)
        *result = (*ptr<< 8) | *(ptr+1);
    else
        *result = *ptr;

    return 1;
}

static char sparc_ram_write(struct _sparc_memory_segment *s, short size, 
        unsigned int offset, unsigned int value)
{
    unsigned char *ptr;
    struct _mem_data *mem = (struct _mem_data *)s->data;


    /* Normal memory access */
    ptr = (unsigned char *)(mem->begin + offset);
    if(size == 32) 
    {
        *ptr 	= (value >> 24) & 0xFF;
        *(ptr+1)= (value >> 16) & 0xFF; 
        *(ptr+2)= (value >>  8) & 0xFF; 
        *(ptr+3)= (value      ) & 0xFF;
        return 1;
    } else if (size == 16) 
    {
        *ptr 	= (value >>  8) & 0xFF;
        *(ptr+1)= (value      ) & 0xFF; 
        return 1;
    } else if ( size == 8 )
    {
        *ptr 	= (value      ) & 0xFF;
        return 1;
    }

    /*	memcpy(ptr, &templ, 4);*/
    return 0;

}

static char sparc_rom_write(struct _sparc_memory_segment *s, short size, 
        unsigned int offset, unsigned int value)
{
    DBG("size=%d, offset=0x%08lx, value=0x%08lx\n", size, offset, value);
    DBG("Cannot write to ROM, go away.\n");
    return 1;
}

static char sparc_rom_read(
        struct _sparc_memory_segment *s, 
        unsigned int *result, 
        short size, 
        unsigned int offset)
{

    struct _mem_data *mem = (struct _mem_data *)s->data;
    unsigned char *ptr = (unsigned char *)(mem->begin + offset);
    DBG("Reading from ROM\n");

    if( result == NULL )
    {
        printf("%s: Error reading from ROM\n");
        return 0;
    }

    if(size == 32)
    {
        *result = (*ptr<<24) | (*(ptr+1)<<16) | (*(ptr+2)<<8) | *(ptr+3);
        if( !is_bigendian() )
            *result = htonl(*result);
    }
    else if (size == 16)
    {
        *result = (*ptr<< 8) | *(ptr+1);
        if( !is_bigendian() )
            *result = htons(*result);
    }
    else
    {
        *result = *ptr;
    }

    return 1;
}

