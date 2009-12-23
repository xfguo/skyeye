/*
 * =====================================================================================
 *
 *       Filename:  memory.c
 *
 *    Description:  This file implements the SPARC memory
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "types.h"
#include "memory.h"
#include "debug.h"


/*-----------------------------------------------------------------------------
 *  PRIVATE FUNCTIONS
 *-----------------------------------------------------------------------------*/

sparc_mem_banks_t mem_bank_list;
//struct _io_memory io_memory;

static struct _sparc_memory_module *sparc_memory_module = NULL;
static int sparc_memory_module_count=0;
static int sparc_memory_module_count_max=0;

/* Make a list of pointers to segments.  Note: we don't make a list of segments
 *  directly, because some segments will want to save a pointer to themselves,
 *  and they can't do that if we're realloc()ing the list as we add more items.
 */
struct _sparc_memory_segment_list_item 
{
    struct _sparc_memory_segment *seg;
};

static struct _sparc_memory_segment_list_item *sparc_memory_segment_list = NULL;
static int sparc_memory_segment_count=0;
static int sparc_memory_segment_count_max=0;

/*-----------------------------------------------------------------------------
 *  PUBLIC FUNCTIONS
 *-----------------------------------------------------------------------------*/

void sparc_memory_module_register(const char *name, 
        void (*setup)(struct _sparc_memory_segment *s))
{
    if(sparc_memory_module_count == sparc_memory_module_count_max) 
    {
        sparc_memory_module_count_max += 4;
        sparc_memory_module = (struct _sparc_memory_module *)realloc(sparc_memory_module,
                sizeof(struct _sparc_memory_module) * 
                sparc_memory_module_count_max);
    }

    sparc_memory_module[sparc_memory_module_count].name = strdup(name);
    sparc_memory_module[sparc_memory_module_count].setup = setup;
    sparc_memory_module_count++;	
}

static unsigned int zero_register = 0;
void sparc_memory_module_setup_segment(char *module_name, int *base_register, int base, int len)
{
    struct _sparc_memory_segment *s;
    struct _sparc_memory_segment_list_item *i;
    int x;
    char movable = 1;

    /* Setup the memory segment */
    if(sparc_memory_segment_count == sparc_memory_segment_count_max) 
    {
        sparc_memory_segment_count_max += 4;
        sparc_memory_segment_list = (struct _sparc_memory_segment_list_item *)realloc(sparc_memory_segment_list,
                sizeof(struct _sparc_memory_segment_list_item) * 
                sparc_memory_segment_count_max);
    }
    i = &sparc_memory_segment_list[sparc_memory_segment_count];
    s = malloc(sizeof(struct _sparc_memory_segment));
    i->seg = s;
    memset(s, 0, sizeof(struct _sparc_memory_segment));


    s->base = base;
    s->base_register = (unsigned int*)base_register;
    s->name = strdup(module_name);
    /* The mask is the inverted length 
     *   len 0x100 ==  mask FFFFFF00 */
    s->mask = ~(len - 1);

    /* Find the module, and run the module setup */
    for(x=0; x < sparc_memory_module_count; x++) {
        if(strcasecmp(sparc_memory_module[x].name, module_name) == 0) 
        {
            DBG("Registering %s(%d) memory bank\n", sparc_memory_module[x].name, sparc_memory_module_count);
            sparc_memory_module[x].setup(s);
            break;
        }
    }
    if(x==sparc_memory_module_count) {
        /* Not found */
        DBG("Could not find module for [%s]\n", module_name);
    }

    /* See if everything above registerd a base address register, if not
     * then give it the zero address */
    if(s->base_register == NULL) {
        s->base_register = &zero_register;
        movable=0;
    }

    DBG("0x%08lx -> 0x%08lx  %s\n",
            *s->base_register + s->base, 
            *s->base_register + s->base + ~s->mask,
            movable ? "(movable)" : "");

    fflush(stdout);
    sparc_memory_segment_count++;
}

char sparc_memory_store(short size, int offset, unsigned int value)
{
    struct _sparc_memory_segment *s;
    unsigned int base_offset;

    /* Value will be in whatever endianness the computer is */
    s = sparc_memory_find_segment_for(offset);

    if( s ) {
        base_offset = (unsigned int)offset - 
            (*s->base_register + s->base);

        /* base_offset = (unsigned int)Offset & ~(s->base); */
        if( s->write != NULL )
            return s->write(s, size, base_offset, value);
    }

    /* Exception 2, access error */
    DBG("%s(): Can't write on 0x%x\n", __func__, offset);
    skyeye_exit(1);
    return 0;
}

char sparc_memory_read(unsigned int *result, short size, int offset)
{
    struct _sparc_memory_segment *s;
    unsigned int base_offset;


    s = sparc_memory_find_segment_for(offset);
    if(s) 
    {

        base_offset = (unsigned int)offset - 
            (*s->base_register + s->base);

        /* base_offset = (unsigned int)Offset & ~(s->base); */
        if(s->read != NULL)
        {
            return s->read(s, result, size, base_offset);
        }
    }

    /* Coulnd't find it in the tables */
    DBG("%s(): Can't read on 0x%x\n", __func__, offset);
    skyeye_exit(1);
    return 0;
}


struct _sparc_memory_segment *sparc_memory_find_segment_for(unsigned int offset)
{
	int x;
	struct _sparc_memory_segment_list_item *i;

	for( x = 0,i = &sparc_memory_segment_list[0]; x < sparc_memory_segment_count; x++, i++) 
    {
		/* FIXME: optimize this by pre-calculation */
		struct _sparc_memory_segment *s = i->seg;
		register unsigned int b = *s->base_register + s->base;
		if( (offset & s->mask) == b) 
        {
			return s;
		}
	}

    DBG("%s: No memory match\n", __func__);
	return NULL;
}

