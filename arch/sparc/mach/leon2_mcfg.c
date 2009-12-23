/*
 * =====================================================================================
 *
 *       Filename:  leon2_mcfg.c
 *
 *    Description:  This File implements the LEON2 Memory Controller
 *
 *        Version:  1.0
 *        Created:  15/10/08 17:24:06
 *       Modified:  15/10/08 17:24:06
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

#include "../common/types.h"
#include "leon2_mcfg.h"

#include <strings.h>

static MCFGState leon2_mcfg;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_mcfg_write
 *  Description:  This function writes in the memory configuration register.
 *  The function does nothing.
 * =====================================================================================
 */
static void leon2_mcfg_write(void *opaque, uint32 addr, uint32 val)
{
    // do nothing
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_mcfg_read
 *  Description:  This function reads from the MCFG register.
 *  Return:
 *      The function returns the 32bit word been read.
 * =====================================================================================
 */
static uint32 leon2_mcfg_read(void *opaque, uint32 addr)
{
    uint32 result;
    int reg;

    /*  index   */
    reg = (addr - leon2_mcfg.address) >> 2;
    result = ((uint32 *)&leon2_mcfg.regs)[reg];

    return result;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_mcfg_init
 *  Description:  This function configures the MCFG I/O device. The function
 *  initializes the first two memory configuration registers to pre-defined
 *  values and registers an I/O memory area for the device
 *  Return:
 *      The function returns nothing
 * =====================================================================================
 */
void leon2_mcfg_init(uint32 address)
{
    bzero((void*)&leon2_mcfg, sizeof(leon2_mcfg));

    leon2_mcfg.address = address;
    leon2_mcfg.regs[0] = 0x00000233;
    leon2_mcfg.regs[1] = 0x00001220;

    leon2_register_io_memory(address, MCFG_REGS * 4, 
                             leon2_mcfg_write, leon2_mcfg_read, NULL);

}


