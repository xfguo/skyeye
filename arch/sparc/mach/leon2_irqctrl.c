/*
 * =====================================================================================
 *
 *       Filename:  leon2_irqctrl.c
 *
 *    Description:  This file implements the LEON2 interrupt control register
 *
 *        Version:  1.0
 *        Created:  15/10/08 17:52:36
 *       Modified:  15/10/08 17:52:36
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */


#include "../common/types.h"
#include "../common/debug.h"
#include "leon2_irqctrl.h"

#include <strings.h>
#include <stdio.h>
#include <signal.h>

#define IRQCTRL_MASK_AND_PRIORITY_REGISTER  (0x0 >> 2)
#define IRQCTRL_PENDING_REGISTER            (0x4 >> 2)
#define IRQCTRL_FORCE_REGISTER              (0x8 >> 2)
#define IRQCTRL_CLEAR_REGISTER              (0xc >> 2)

static IRQCTRLState leon2_irqctrl;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_irqctrl_write
 *  Description:  This function writes on the LEON2 interrupt control registers.
 * =====================================================================================
 */
static void leon2_irqctrl_write(void *opaque, uint32 addr, uint32 val)
{
    int reg;

    /*  index   */
    reg = (addr - leon2_irqctrl.address) >> 2;

    leon2_irqctrl.regs[reg] = val;

    switch(reg)
    {
        case IRQCTRL_MASK_AND_PRIORITY_REGISTER:
        case IRQCTRL_PENDING_REGISTER:
        case IRQCTRL_FORCE_REGISTER:
            /*  FIXME: check if previously masked IRQ is now enabled and, if so,
             *  re-raise it */
            break;
        case IRQCTRL_CLEAR_REGISTER:
            leon2_irqctrl.regs[IRQCTRL_CLEAR_REGISTER] &= ~val;
            break;
        default:
            ERR("Unknown register at 0x%x", addr);
            raise(SIGINT);
    }
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_irqctrl_read
 *  Description:  This static function reads one of the LEON2 interrupt control
 *  registers and returns the read value.
 * =====================================================================================
 */
static uint32 leon2_irqctrl_read(void *opaque, uint32 addr)
{
    uint32 result;
    int reg;

    /*  index   */
    reg = (addr - leon2_irqctrl.address) >> 2;

    if( reg == IRQCTRL_CLEAR_REGISTER )
        result = 0;
    else
        result = leon2_irqctrl.regs[reg];

    return result;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_irqctrl_set_pending
 *  Description:  This function sets the pending register
 * =====================================================================================
 */
inline void leon2_irqctrl_set_pending(int irq)
{
    leon2_irqctrl.regs[IRQCTRL_PENDING_REGISTER] |= (1 << irq);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_irqctrl_reset_pending
 *  Description:  This function resets the pending register
 * =====================================================================================
 */
inline void leon2_irqctrl_reset_pending(int irq)
{
    leon2_irqctrl.regs[IRQCTRL_PENDING_REGISTER] &= ~(1 << irq);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_irqctrl_pending
 *  Description:  This function returns the content of the pending register
 * =====================================================================================
 */
inline int leon2_irqctrl_pending(void)
{
    return leon2_irqctrl.regs[IRQCTRL_PENDING_REGISTER];
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_irqctrl_mask
 *  Description:  This function returns the content of the mask register
 * =====================================================================================
 */
inline int leon2_irqctrl_mask(void)
{
    return (leon2_irqctrl.regs[IRQCTRL_MASK_AND_PRIORITY_REGISTER] & 0xfffe);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_irqctrl_forced
 *  Description:  This function returns the content of the forced register
 * =====================================================================================
 */
inline int leon2_irqctrl_forced(void)
{
    return leon2_irqctrl.regs[IRQCTRL_FORCE_REGISTER];
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_irqctrl_reset_forced
 *  Description:  This function resets the LEON2 interrupt force register
 * =====================================================================================
 */
inline void leon2_irqctrl_reset_forced(int irq)
{
    leon2_irqctrl.regs[IRQCTRL_FORCE_REGISTER] &= ~(1 << irq);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_irqctrl_init
 *  Description:  this function initializes the LEON2 interrupt registers. The
 *  function also registers the I/O memory area where the registers are mapped.
 * =====================================================================================
 */
void leon2_irqctrl_init(uint32 address)
{
    bzero((void*)&leon2_irqctrl, sizeof(leon2_irqctrl));

    leon2_irqctrl.address = address;

    leon2_register_io_memory(address, IRQCTRL_NREGS * 4, 
                             leon2_irqctrl_write, leon2_irqctrl_read, NULL);

}

