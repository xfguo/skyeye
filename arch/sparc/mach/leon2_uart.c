/*
 * =====================================================================================
 *
 *       Filename:  leon2_uart.c
 *
 *    Description:  This file implements the LEON2 UART on-chip device.
 *
 *        Version:  1.0
 *        Created:  24/06/08 10:51:22
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

#include "../common/types.h"
#include "../common/bits.h"
#include "mach_leon2_regmap.h"
#include "leon2_uart.h"

#include <skyeye.h>

#define UART_DATA_REGISTER      (0x0 >> 2)
#define UART_STATUS_REGISTER    (0x4 >> 2)
#define UART_CONTROL_REGISTER   (0x8 >> 2)
#define UART_SCALER_REGISTER    (0xc >> 2)

#define UART_NREGS  4

static UARTState leon2_uart;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_uart_cycle
 *  Description:  This function performs the execution of the UART cycle. The
 *  function checks whether there is some to transmit from the UART.
 *
 *  @TODO: The implementation is not finished. Still the implementation does not
 *  care about the possible generated interrupts.
 * =====================================================================================
 */
void leon2_uart_cycle(void *pstate)
{

    /*-----------------------------------------------------------------------------
     *  TRANSMITER OPERATION
     *-----------------------------------------------------------------------------*/

    /*  Check whether there is something to transmit from the UART. If
     *  transmitter is enabled through the TE bit in the control register, the
     *  data is transmiter by means of the skyeye uart  */
    if( leon2_uart.regs.control.transmitter_enable )
    {
        if( !leon2_uart.regs.status.transmitter_hold_register_empty )
        {
            char c = leon2_uart.regs.data;

            /*  Call the SKYEYE uart function to print out the character    */
			skyeye_uart_write(-1, &c, 1, NULL);

            /*  clear the transmitter data register */
//            clear_bits(regmap.uart_1_data, LEON2_UART_DATA_last, LEON2_UART_DATA_first);

            /*  Indicate the transmiter hold register is empty  */
            leon2_uart.regs.status.transmitter_hold_register_empty = 1;
            /*  Indicate the transmiter shift register is empty  */
            leon2_uart.regs.status.transmitter_shift_register_empty = 1;
        }

    }
    else
    {
        /*  UART Transmiter not enabled    */
    }

    /*-----------------------------------------------------------------------------
     *  RECEIVER OPERATION
     *-----------------------------------------------------------------------------*/
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_uart_write
 *  Description:  This function perform the writing in the UART device
 * =====================================================================================
 */
static void leon2_uart_write(void *opaque, uint32 addr, uint32 v)
{
    int reg;

    /*  index   */
    reg = (addr - leon2_uart.address) >> 2;
    switch(reg)
    {
        case UART_DATA_REGISTER:

//            if( !leon2_uart.regs.control.transmitter_enable )
//                break;

            /*  Write the information in the DATA register  */
            leon2_uart.regs.data = v;

            /*  Indicate that the transmitter hold register is NOT empty    */
            leon2_uart.regs.status.transmitter_hold_register_empty = 0;

            break;
        case UART_STATUS_REGISTER:
            // do nothing
            break;
        default:
            ((uint32 *)&leon2_uart.regs)[reg] = v;
            break;

    };


}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_uart_read
 *  Description:  This function performs the reading in the UART device.
 * =====================================================================================
 */
static uint32 leon2_uart_read(void *opaque, uint32 addr)
{
    uint32 result;
    int reg;

    /*  index   */
    reg = (addr - leon2_uart.address) >> 2;

    result = ((uint32 *)&leon2_uart.regs)[reg];

    if( reg == UART_DATA_REGISTER)
        leon2_uart.regs.status.data_ready = 0;

    return result;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_uart_init
 *  Description:  This function initializes the UART for the LEON2 machine.
 *
 *  The function returns nothing because no error may be generated.
 * =====================================================================================
 */
void leon2_uart_init(void *opaque, uint32 address, uint32 freq)
{
    /*  Clear the UART register   */
    /*  Zeored all the TIMER registers  */
    bzero( (void*)&leon2_uart.regs, sizeof(struct regs));

    leon2_uart.address = address;

    /*  Enable the transmitter and receiver by default  */
    leon2_uart.regs.control.transmitter_enable = 1;
    leon2_uart.regs.control.receiver_enable = 1;

    /*  Indicate that the transmitter hold register is EMPTY    */
    leon2_uart.regs.status.transmitter_hold_register_empty = 1;
    leon2_uart.regs.status.transmitter_shift_register_empty = 1;

    /*  Register the timer  */
    leon2_register_io_memory(address, UART_NREGS * 4, 
                             leon2_uart_write, leon2_uart_read, NULL);
}

