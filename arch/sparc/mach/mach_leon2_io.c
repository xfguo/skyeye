/*
        mach_leon2_io.c - implementation of LEON machine I/O simulation
        Copyright (C) 2008  Aitor Viana Sanchez
        for help please send mail to <skyeye-developer@lists.sf.linuxforum.net>

        This program is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2 of the License, or
        (at your option) any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program; if not, write to the Free Software
        Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/


/*-----------------------------------------------------------------------------
 *  11/07/08 17:29:54 Aitor Viana Sanchez
 *-----------------------------------------------------------------------------*/

#include "../common/types.h"
#include "../common/bits.h"
#include "../common/sparc.h"
#include "../common/traps.h"
#include "../common/iu.h"
#include "../common/debug.h"
#include "mach_leon2_io.h"
#include "leon2_uart.h"
#include "leon2_mcfg.h"


#include <stdio.h>
#include <signal.h>

/*  We try to know whether the host architecture is bitendian of little endian
 */
static const int i = 1;
#define is_bigendian() ( (*(char*)&i) == 0 )

#define SUCCESS 1
#define ERROR   0

/*  I/O ERROR message/behaviour */
#define IO_ERR {printf("\n%s I/O error!!!addr = 0x%x, PC = 0x%x\n",__FUNCTION__, addr, PCREG);raise (SIGINT);}

/*-----------------------------------------------------------------------------
 *  PRIVATE INTERFACE
 *-----------------------------------------------------------------------------*/

/*  These are the initial values for all the On-chip peripherals registers  */
//struct _leon2_regmap regmap = 
//{
//    0x00000233, 0x00001220, 0x00000000, 0x00000000, // 0x80000000
//    0x00000000, 0x5001000f, 0x00000000, 0x00000000, // 0x80000010
//    0x00000000, 0x02752bd0, 0x00000000, 0x00000000, // 0x80000020
//    0x00000000, 0x00000000, 0x00000000, 0x00000000, // 0x80000030
//    0x00ffffff, 0x00ffffff, 0x00000003, 0x00ffffff, // 0x80000040
//    0x00000000, 0x00000000, 0x00000000, 0x00000000, // 0x80000050
//    0x00000030, 0x00000031, 0x00000000, 0x00000000, // 0x80000060
//    0x00000000, 0x00000000, 0x00000007, 0x00000000, // 0x80000070
//    0x00000000, 0x00000000, 0x00000007, 0x00000000, // 0x80000080
//    0x00000000, 0x00000000, 0x00000000, 0x00000000, // 0x80000090
//    0x00000000, 0x00000000, 0x00000000, 0x00000000, // 0x800000a0
//    0x00000000, 0x00000000, 0x00000000, 0x00000000, // 0x800000b0
//    0x00000000, 0x00000000, 0x00000000, 0x00000000, // 0x800000c0
//};

/** Onchip peripherals map register */
//static uint32 *pregmap = (uint32 *)(&regmap);

typedef struct _io_areas
{
    io_read_callback io_read;
    io_write_callback io_write;
    uint32 address;
    uint32 bsize;
    void *opaque;
}io_area_t;

static io_area_t *io_area;
static int io_devices = 0;

/** Interrupt pending register  */
//#define IPENDING    (regmap.irq_pending)
/** Interrupt masking register  */
//#define IMASK       (regmap.irq_mask_and_priority)

/*declare the device io functions*/
//leon2_declare_device(leon2_timer_core);
//leon2_declare_device(leon2_uart);

/*  Forward declaration */
static void handle_irq ();

int leon2_register_io_memory(
        uint32 address,     /*  I/O address */
        uint32 size,        /*  I/O area size (bytes)   */
        io_read_callback fwrite,   /*  read functin callback   */
        io_write_callback fread,  /*  write function callback */
        void *opaque)       /*  private data    */
{

    DBG("I/O device at 0x%x\n", address);
    io_area = (io_area_t *)realloc(io_area, (io_devices + 1) * sizeof(io_area_t)); 
    if ( io_area == NULL )
    {
        ERR("FATAL ERROR! No space available");
        raise(SIGINT);
    }

    io_area[io_devices].io_read = fread;
    io_area[io_devices].io_write = fwrite;
    io_area[io_devices].address = address;
    io_area[io_devices].bsize = size;
    io_area[io_devices].opaque = opaque;

    io_devices++;

    return ERROR;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_regmap_read
 *  Description:  This function reads the REGMAP On-Chip peripheral registers.
 *  The function takes into account that the simulator is running in a little
 *  endian machine, and the LEON2 is big endian.
 * =====================================================================================
 */
//uint32 leon2_regmap_read(unsigned int * result, int size, int offset)
//{
//    unsigned char *ptr = (unsigned char *)(&pregmap[offset / 4]);
//
//
//    if(size == 32)
//    {
//        *result = (*ptr<<24) | (*(ptr+1)<<16) | (*(ptr+2)<<8) | *(ptr+3);
//        if( !is_bigendian() )
//            *result = htonl(*result);
//    }
//    else if (size == 16)
//    {
//        *result = (*ptr<< 8) | *(ptr+1);
//        if( !is_bigendian() )
//            *result = htons(*result);
//    }
//    else
//        *result = *ptr;
//
//    return SUCCESS;
//}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_regmap_write
 *  Description:  This function perform the writin in the LEON2 I/O area. The
 *  function receives the address where to perform the writing, the value and
 *  the size of the writing.
 *  The size may be 32, 16 or 8 bits.
 * =====================================================================================
 */
//uint32 leon2_regmap_write(int size, int offset, unsigned int value)
//{
//    unsigned char *ptr = (unsigned char *)(&pregmap[offset / 4]);
//
//    if(size == 32) 
//    {
//        *(ptr+3)= (value >> 24) & 0xFF;
//        *(ptr+1)= (value >> 16) & 0xFF; 
//        *(ptr+2)= (value >>  8) & 0xFF; 
//        *(ptr+0)= (value      ) & 0xFF;
//
//
//    } else if (size == 16) 
//    {
//        *ptr 	= (value >>  8) & 0xFF;
//        *(ptr+1)= (value      ) & 0xFF; 
//
//    } else if ( size == 8 )
//    {
//        *ptr 	= (value      ) & 0xFF;
//    }
//    else
//        return ERROR;
//
//    /*  Everything OK   */
//    return SUCCESS;
//}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_irq
 *  Description:  @see leon2_set_int
 *  !FIXME: we need to handle more than one HW interrupt. Something like event
 *  queue would be the solution. So far, only one event at time is handled.
 * =====================================================================================
 */
static void handle_irq ()
{
    uint32 pending, mask;

    pending = mask = 0;

    pending = leon2_irqctrl_pending();
    mask = leon2_irqctrl_mask();
    
    /*  
     *  The interrupt is raised if it is not masked. 
     *  IMASK[irq] = 0 means masked
     *  IMASK[irq] = 1 means enabled
     */
    /*  If all the interrupts are masked, just return   */
    if( (pending & mask) == 0 )
        return;

    /*  FIXME:  Tenemos que mirar tambien como se gestionan los traps del
     *  procesador (common/traps.c) porque puede que tengamos que hacer algo
     *  conjunto    */
    ERR("INTERRUPT RAISED (pending = 0x%x, mask = 0x%x)", pending, mask);
    raise(SIGINT);

}


/*=============================================================================
 *  PUBLIC INTERFACES
 *=============================================================================*/

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_set_int
 *  Description:  This function acts as the interrupt controler for the leon2.
 *  The function receives the interrupt number. When an interrupt is generated,
 *  the corresponding bit is set in the interrupt pending register. The pending
 *  bits are ANDed with the interrupt mask register and then forwarded to the
 *  priority selector.
 *  Each interrupt may be assigned to one of two lecels as programmed in the
 *  interrupt level register.
 *      - Level 1 has a higher priority than level0.
 *  The interrupts are prioritised within each level, with interrupt 15 having
 *  the highest priority and interrupt 1 the lowest.
 *
 *  @see handle_irq() function
 * =====================================================================================
 */
void leon2_set_int (int irq)
{
//    IPENDING |= (1 << irq);
    leon2_irqctrl_set_pending(irq);
//    ERR("TBI");
//    raise(SIGINT);
//    set_bit(IPENDING, irq);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon_io_do_cycle
 *  Description:  This function performs the I/O cycling. In this routine all
 *  the post checks in the I/O devices are performed:
 *      - UART checking for transmission
 *      - Clock cycling
 *      - IRQ management
 *
 *  This function is called from the sparc_step_once()
 *
 *  @see sparc_step_once()
 * =====================================================================================
 */
void leon2_io_do_cycle(void * state)
{
    /*  FIXME: it is not used so far    */
    sparc_state_t *pstate = state;

    //  UART cycle
    leon2_uart_cycle(state);

     // TIMER cycling
    leon2_timer_core_cycle(state);

    /*-----------------------------------------------------------------------------
     *  Handle all the possible IRQs during the I/Os cycles.
     *-----------------------------------------------------------------------------*/
    handle_irq ();
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_io_read_byte
 *  Description:  This function performs read operation over the I/O mapped
 *  area.
 *  Parameters:
 *      - state:    Not used
 *      - addr:     This is the address where the byte is being read from
 * =====================================================================================
 */
uint32 leon2_io_read_byte(void * state, uint32 addr)
{
    IO_ERR;
    return ERROR;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_io_read_word
 *  Description:  This function performs read operation over the I/O mapped
 *  area.
 *  Parameters:
 *      - state:    Not used
 *      - addr:     This is the address where the word (2 bytes) is being read from
 * =====================================================================================
 */
uint32 leon2_io_read_word (void * state, uint32 addr)
{
    IO_ERR;
    return ERROR;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_io_read_long
 *  Description:  This function performs read operation over the I/O mapped
 *  area.
 *  Parameters:
 *      - state:    Not used
 *      - addr:     This is the address where the long word (4 bytes) is being read from
 * =====================================================================================
 */
uint32 leon2_io_read_long (void * state, uint32 addr)
{
    int i;
    uint32 result;

    for( i = 0; i < io_devices; ++i)
    {
        if( (addr - io_area[i].address) < io_area[i].bsize )
        {
            result = io_area[i].io_read(NULL, addr);
            return result;
        }
    }

    return 0;
//    IO_ERR;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_io_write_byte
 *  Description:  This function performs write operation over the I/O mapped
 *  area.
 *  Parameters:
 *      - state:    Not used
 *      - addr:     This is the address where the byte is being written to
 *      - v:        The value to be written
 * =====================================================================================
 */
void leon2_io_write_byte (void * state, uint32 addr, uint32 v)
{
    IO_ERR;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_io_write_word
 *  Description:  This function performs write operation over the I/O mapped
 *  area.
 *  Parameters:
 *      - state:    Not used
 *      - addr:     This is the address where the word (2 bytes) is being written to
 *      - v:        The value to be written
 * =====================================================================================
 */
void leon2_io_write_word (void * state, uint32 addr, uint32 v)
{
    IO_ERR;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_io_write_long
 *  Description:  This function performs write operation over the I/O mapped
 *  area.
 *  Parameters:
 *      - state:    Not used
 *      - addr:     This is the address where the long word (4 bytes) is being written to
 *      - v:        The value to be written
 * =====================================================================================
 */
void leon2_io_write_long (void * state, uint32 addr, uint32 v)
{
    int i;

    for( i = 0; i < io_devices; ++i)
    {
        if( (addr - io_area[i].address) < io_area[i].bsize )
        {
            io_area[i].io_write(NULL, addr, v);
            return;
        }
    }

//    IO_ERR;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_io_reset
 *  Description:  Reset all the values related to the I/O mapped memory.
 *  Configuring the devices with the default settings.
 * =====================================================================================
 */
void leon2_io_reset(void *state)
{
    io_devices = 0;

    /*  Register all the I/O devices    */
    leon2_uart_init(state, UART1_ADDR, 50);
    leon2_timer_core_init(state, TIMER1_ADDR, 100);
    leon2_mcfg_init(MCFG_ADDR);
    leon2_cfg_init(CFG_ADDR);
    leon2_irqctrl_init(IRQCTRL_ADDR);
}


