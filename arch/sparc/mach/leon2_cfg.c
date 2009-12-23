/*
 * =====================================================================================
 *
 *       Filename:  leon2_cfg.c
 *
 *    Description:  This file implements the LEON2 confiuration register
 *
 *        Version:  1.0
 *        Created:  15/10/08 17:37:05
 *       Modified:  15/10/08 17:37:05
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

#include "../common/types.h"
#include "leon2_cfg.h"

#include <strings.h>

static uint32 leon2_cfg;

static void leon2_cfg_write(void *opaque, uint32 addr, uint32 val)
{
    // do nothing
}

static uint32 leon2_cfg_read(void *opaque, uint32 addr)
{
    return leon2_cfg;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_cfg_init
 *  Description:  This function initializes the LEON2 configuration register.
 *  The function initializes the register with the value 0x700000, whic means
 *  FIXME.
 *  The function also registers the I/O memory area for the LEON2 configuration
 *  register.
 * =====================================================================================
 */
void leon2_cfg_init(uint32 address)
{
    bzero((void*)&leon2_cfg, sizeof(leon2_cfg));

    leon2_cfg = 0x700000;

    leon2_register_io_memory(address, sizeof(leon2_cfg),
                             leon2_cfg_write, leon2_cfg_read, NULL);

}

