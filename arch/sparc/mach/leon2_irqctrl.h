/*
 * =====================================================================================
 *
 *       Filename:  leon2_irqctrl.h
 *
 *    Description:  This file defines all the resources for the LEON2 interrupt
 *    control register
 *
 *        Version:  1.0
 *        Created:  15/10/08 17:53:36
 *       Modified:  15/10/08 17:53:36
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

#ifndef _LEON2_IRQCTL_H_
#define _LEON2_IRQCTL_H_

#ifndef __SPARC_TYPES_H_
#error "types.h header file must be included before including leon2_irqctrl.h"
#endif

#define IRQCTRL_NREGS   4

typedef struct IRQCTRLState
{
    uint32 address;
    uint32 regs[IRQCTRL_NREGS];
}IRQCTRLState;

void leon2_irqctrl_init(uint32 address);
inline void leon2_irqctrl_set_pending(int irq);
inline void leon2_irqctrl_reset_pending(int irq);
inline int leon2_irqctrl_pending(void);
inline int leon2_irqctrl_forced(void);
inline void leon2_irqctrl_reset_forced(int irq);
inline int leon2_irqctrl_mask(void);

#endif

