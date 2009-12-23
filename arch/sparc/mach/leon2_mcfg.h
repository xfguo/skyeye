/*
 * =====================================================================================
 *
 *       Filename:  leon2_mcfg.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  15/10/08 17:24:36
 *       Modified:  15/10/08 17:24:36
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

#ifndef _LEON2_MCFG_H_
#define _LEON2_MCFG_H_

#ifndef __SPARC_TYPES_H_
#error "types.h header file must be included before including leon2_timer_unit.h"
#endif


#define MCFG_REGS   3

typedef struct MCFGState
{
    uint32 address;
    uint32 regs[MCFG_REGS];
}MCFGState;

void leon2_mcfg_init(uint32 address);

#endif

