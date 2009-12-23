/*
 * =====================================================================================
 *
 *       Filename:  skyeye_mach_leon2.c
 *
 *    Description:  Implementation of the LEON2 processor
 *
 *        Version:  1.0
 *        Created:  15/04/08 15:09:38
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


#include "skyeye_config.h"

#include "../common/types.h"
#include "../common/bits.h"

#include "mach_leon2_regmap.h"
#include "leon2_uart.h"

/*-----------------------------------------------------------------------------
 *  PUBLIC INTERFACE
 *-----------------------------------------------------------------------------*/

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_mach_init
 *  Description:  This function initialezes the machine.
 *  In the LEON2 architecture the On-Chip registers are mapped in the address
 *  0x80000000. The io_read/write functions will be the ones reading this
 *  address.
 *  For the rest of the peripherals, the io2_read/write functions are provided.
 * =====================================================================================
 */
void leon2_mach_init(void * arch_instance, machine_config_t * mach)
{

    // FIXME!: The state is not treated yet
    // Also some more initializations need to be performed
	machine_config_t * this_mach = mach;

    if( !this_mach )
    {
        SKYEYE_ERR ("Error: No machine config structure\n");
        exit( -1 );
    }
    else
    {
        SKYEYE_INFO("%s(): %s initialized\n", __func__, mach->machine_name);
    }

    /*  These routines are defined in the mach_leon2_io.c source file   */
    // read functions
    this_mach->mach_io_read_byte = leon2_io_read_byte;
    this_mach->mach_io_read_halfword = leon2_io_read_word;
    this_mach->mach_io_read_word = leon2_io_read_long;
    // write functions
    this_mach->mach_io_write_byte = leon2_io_write_byte;
    this_mach->mach_io_write_halfword = leon2_io_write_word;
    this_mach->mach_io_write_word = leon2_io_write_long;
    // other functions
    this_mach->mach_io_do_cycle = leon2_io_do_cycle;
    this_mach->mach_io_reset = leon2_io_reset;
    this_mach->mach_set_intr = leon2_set_int;

}


