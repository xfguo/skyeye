/*
 * =====================================================================================
 *
 *       Filename:  iomem.c
 *
 *    Description:  This file implements all the I/O related functions
 *
 *        Version:  1.0
 *        Created:  09/06/08 12:00:53
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

/* RAM and ROM memory modules */
#include "skyeye_config.h"
#include <stdio.h>

#include "types.h"
#include "memory.h"

//static void iomem_fini(struct _sparc_memory_segment *s);
static char iomem_read(struct _sparc_memory_segment *s, unsigned int *result, short size, unsigned int offset);
static char iomem_write(struct _sparc_memory_segment *s, short size, unsigned int offset, unsigned int value);

/*-----------------------------------------------------------------------------
 *  PRIVATE FUNCTIONS
 *-----------------------------------------------------------------------------*/

static char iomem_read(
        struct _sparc_memory_segment *s, 
        unsigned int *result, 
        short size, 
        unsigned int offset)
{

    switch(size)
    {
        case 8:
            *result = skyeye_config.mach->mach_io_read_byte(NULL, s->base + offset);
            break;
        case 16:
            *result = skyeye_config.mach->mach_io_read_halfword(NULL, s->base + offset);
            break;
        case 32:
            *result = skyeye_config.mach->mach_io_read_word(NULL, s->base + offset);
            break;
        default:
            SKYEYE_DBG("%s: Invalid size reading from I/O\n");
            *result = 0;
            return 0;
    }

    return 1;
}

static char iomem_write(
        struct _sparc_memory_segment *s, 
        short size, 
        unsigned int offset, 
        unsigned int value)
{
    switch(size)
    {
        case 8:
            skyeye_config.mach->mach_io_write_byte(NULL, s->base + offset, value);
            break;
        case 16:
            skyeye_config.mach->mach_io_write_halfword(NULL, s->base + offset, value);
            break;
        case 32:
            skyeye_config.mach->mach_io_write_word(NULL, s->base + offset, value);
            break;
        default:
            SKYEYE_DBG("%s: Invalid size writing to I/O\n");
            return 0;
    }

    return 1;
}


/*-----------------------------------------------------------------------------
 *  PUBLIC INTERFACE
 *-----------------------------------------------------------------------------*/


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sparc_iomem_setup
 *  Description:  This function initializes all the callbacks related to the I/O
 *  memory.
 *  The function receives an structure, where all the callbacks will be
 *  registered.
 * =====================================================================================
 */
void sparc_iomem_setup(struct _sparc_memory_segment *s)
{
    /*  setup all the functions */
    s->read = &iomem_read;
    s->write = &iomem_write;
    s->reset = NULL;
    s->fini = NULL;
    s->reset = NULL;
    s->update = NULL;
}

