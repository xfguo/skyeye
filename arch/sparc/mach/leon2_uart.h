/*
 * =====================================================================================
 *
 *       Filename:  leon2_uart.h
 *
 *    Description:  UART variables
 *
 *        Version:  1.0
 *        Created:  20/05/08 15:53:23
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

#ifndef _LEON2_UART_H_
#define _LEON2_UART_H_

#ifndef __SPARC_TYPES_H_
#error "types.h header file must be included before including leon2_timer_unit.h"
#endif

/*-----------------------------------------------------------------------------
 *  UART variables
 *-----------------------------------------------------------------------------*/

typedef struct UARTState
{
    uint32 address;
    struct regs
    {
        uint32 data;
        struct
        {
            int data_ready                          :1;
            int transmitter_shift_register_empty    :1;
            int transmitter_hold_register_empty     :1;
            int break_received                      :1;
            int overrun                             :1;
            int parity_error                        :1;
            int framing_error                       :1;
            int reserved                            :25;
        } status;
        struct
        {
            int receiver_enable                     :1;
            int transmitter_enable                  :1;
            int receiver_interrupt_enable           :1;
            int transmitter_interrupt_enable        :1;
            int parity_select                       :1;
            int parity_enable                       :1;
            int flow_control                        :1;
            int loop_back                           :1;
            int external_clock                      :1;
            int reserved                            :23;
        } control;
        uint32 scaler;
    }regs;
}UARTState;

#endif


