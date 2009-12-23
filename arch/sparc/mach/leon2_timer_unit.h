/*
 * =====================================================================================
 *
 *       Filename:  leon2_timer_unit.h
 *
 *    Description:  This file includes all the definitions for the TIMER
 *
 *        Version:  1.0
 *        Created:  15/10/08 15:25:00
 *       Modified:  15/10/08 15:25:00
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */


#ifndef _LEON2_TIMER_H_
#define _LEON2_TIMER_H_

#ifndef __SPARC_TYPES_H_
#error "types.h header file must be included before including leon2_timer_unit.h"
#endif


/* 
 * ===  CLASS  ======================================================================
 *         Name:  TIMERState
 *  Description:  This class stores all the information related to the LEON2
 *  timer units.
 * =====================================================================================
 */
typedef struct TIMERState
{
    uint32  address;
    struct regs
    {
        uint32 counter_1;
        uint32 reload_1;
        struct {
            int enable          :1;
            int reload_counter  :1;
            int load_counter    :1;
            int reserved        :29;
        }control_1;

        uint32 watchdog;
        uint32 unused[4];   // Timer 2 registers not implemented
        uint32 prescaler_counter;
        uint32 prescaler_reload;
    }regs;


    int frequency;
}TIMERState;

void leon2_timer_core_init(void *state, uint32 address, int freq);

#endif


