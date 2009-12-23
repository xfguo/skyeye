/*
 * =====================================================================================
 *
 *       Filename:  leon2_timer_unit.c
 *
 *    Description:  This file implementes the LEON2 processor timer unit
 *
 *        Version:  1.0
 *        Created:  24/06/08 10:31:42
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
#include "leon2_timer_unit.h"

#define TIMER_CTRL_EN   0   //  Enable
#define TIMER_CTRL_RL   1   // Reload counter
#define TIMER_CTRL_LD   2   // Load counter

#define TIMER1_COUNTER_REGISTER     (0x00 >> 2)
#define TIMER1_RELOAD_REGISTER      (0x04 >> 2)
#define TIMER1_CONTROL_REGISTER     (0x08 >> 2)
#define WATCHDOG_REGISTER           (0x0C >> 2)
#define TIMER2_COUNTER_REGISTER     (0x10 >> 2)
#define TIMER2_RELOAD_REGISTER      (0x14 >> 2)
#define TIMER2_CONTROL_REGISTER     (0x18 >> 2)
#define PRESCALER_COUNTER_REGISTER  (0x20 >> 2)
#define PRESCALER_RELOAD_REGISTER   (0x24 >> 2)

#define TIMER_NREGS 10

#define COUNTER_1   ((uint32*)&leon2_timer.regs)[TIMER1_COUNTER_REGISTER]
#define RELOAD_1    ((uint32*)&leon2_timer.regs)[TIMER1_RELOAD_REGISTER]
#define CONTROL_1   ((uint32*)&leon2_timer.regs)[TIMER1_CONTROL_REGISTER]
#define COUNTER_2   ((uint32*)&leon2_timer.regs)[TIMER2_COUNTER_REGISTER]
#define RELOAD_2    ((uint32*)&leon2_timer.regs)[TIMER2_RELOAD_REGISTER]
#define CONTROL_2   ((uint32*)&leon2_timer.regs)[TIMER2_CONTROL_REGISTER]
#define PRE_COUNTER (leon2_timer.regs.prescaler_counter)
#define PRE_RELOAD  (leon2_timer.regs.prescaler_reload)
#define WDOG        (leon2_timer.regs.watchdog)
#define UNDERFLOW   (0xffffffff)


//#define WDOG_COUNTER_OFF    ((int)&regmap.watchdog_register - (int)&regmap)

/*  the WDOG is not enabled until the first write is performed in the WDOG
 *  counter register    */
static bool_t wdog_enabled = False;

static TIMERState leon2_timer;

void leon2_timer_core_cycle(void *pstate)
{

    /*-----------------------------------------------------------------------------
     *  TIMER cycling
     *-----------------------------------------------------------------------------*/

    /*  Prescaler is clocked by the system clock and decremented on each clock
     *  cycle. When the prescaler underflows, it is reloaded from the prescaler
     *  reload register and a timer tick is generated for the two timer and the
     *  wdog. The effective division rate is therefore equal to prescaler reload
     *  register value + 1  */
    PRE_COUNTER--;
    if( PRE_COUNTER == UNDERFLOW )
    {
        PRE_COUNTER = PRE_RELOAD;

        /*  Tick for timers (if enabled) and wdog (allways)    */
        if( bit(CONTROL_1, TIMER_CTRL_EN) )
            COUNTER_1--;
        if( bit(CONTROL_2, TIMER_CTRL_EN) )
            COUNTER_2--;

        /*  When each timer underflow, it will automatically be reloaded with
         *  the value of the timer reload register if the reload bit is set,
         *  otherwise it will stop (at 0xffffff) and reset the enable bit. An
         *  interrupt will be generated after each underflow    */
        if( COUNTER_1 == UNDERFLOW )
        {
            if( bit(CONTROL_1, TIMER_CTRL_RL) )
                COUNTER_1 = RELOAD_1;

            /*  Signal the trap */
            leon2_set_int(TIMER1_INT);
        }
        if( COUNTER_2 == UNDERFLOW )
        {
            if( bit(CONTROL_2, TIMER_CTRL_RL) )
                COUNTER_2 = RELOAD_2;

            /*  Signal the trap */
            leon2_set_int(TIMER2_INT);
        }

        if( wdog_enabled )
            WDOG--;
        /*  The WDOG interrupt line is attached to the USER_DEFINED_INT1
         *  interrupt. This interrupt is non maskarable */
        if( WDOG == UNDERFLOW )
            leon2_set_int(USER_DEFINED_INT1);
    }
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_timer_core_write
 *  Description:  TBD
 * =====================================================================================
 */
static void leon2_timer_core_write(void *opaque, uint32 addr, uint32 val)
{
    int reg, enabled;
    int was_enabled = leon2_timer.regs.control_1.enable;

    /*  Clear the last 8 bits, always reserved  */
    val &= 0x00ffffff;
    reg = (addr - leon2_timer.address) >> 2;
    ((uint32*)&leon2_timer.regs)[reg] = val;

    if( TIMER1_CONTROL_REGISTER == reg )
    {
        if( leon2_timer.regs.control_1.load_counter )
        {
            leon2_timer.regs.counter_1 = leon2_timer.regs.reload_1;
            leon2_timer.regs.control_1.load_counter = 0;

            enabled = leon2_timer.regs.control_1.enable;

            if( (was_enabled != enabled) && !enabled )
            {
//                timer_stop();
            }
            if( enabled)
            {
//                timer_recalibrate();
            }
        }
    }
}

//void leon2_timer_core_write(uint32 size, uint32 addr, uint8 v)
//{
//    /*  Check which register is going to be written */
//    leon2_regmap_write(size, addr, v);
//
//    /*  If we are writing to the wdog, the wdog is enabled for ever */
//    if( addr == WDOG_COUNTER_OFF )
//        wdog_enabled = True;
//
//    /*  Perform the masking for all the registers   */
//    COUNTER_1 &= 0x00FFFFFF;
//    COUNTER_2 &= 0x00FFFFFF;
//    WDOG &= 0x00FFFFFF;
//
//    RELOAD_1 &= 0x00FFFFFF;
//    RELOAD_2 &= 0x00FFFFFF;
//
//    CONTROL_1 &= 0x00000007;
//    CONTROL_2 &= 0x00000007;
//
//    PRE_COUNTER &= 0x00FFFFFF;
//    PRE_RELOAD &= 0x00FFFFFF;
//}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_timer_core_read
 *  Description:  TBD
 * =====================================================================================
 */
static uint32 leon2_timer_core_read(void *opaque, uint32 addr)
{
    uint32 result;
    int reg;

    /*  index   */
    reg = (addr - leon2_timer.address) >> 2;

    result = ((uint32 *)&leon2_timer.regs)[reg];

    return result;

}

//uint32 leon2_timer_core_read(uint32 size, uint32 addr)
//{
//    uint32 res = 0;
//
//    leon2_regmap_read(&res, size, addr);
//
//    switch(addr)
//    {
//        /*  The load counter (LD) bit always reads as a 'zero'  */
//        case LEON2_REGMAP_TIMER1_CTRL:
//        case LEON2_REGMAP_TIMER2_CTRL:
//            res &= 0x00000006;
//    };
//
//    return res;
//}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_timer_core_init
 *  Description:  This function initializes the timer_core unit for the LEON2
 *  processor.
 *  The function resets all the registers related to the TIMER1/2, WDOG and
 *  prescaler.
 *  The function returns nothing because no error may be generated.
 * =====================================================================================
 */
void leon2_timer_core_init(void *state, uint32 address, int freq)
{
    /*-----------------------------------------------------------------------------
     *  TIMER CONFIGURATION/INITIALIZATION
     *-----------------------------------------------------------------------------*/
    /*  Zeored all the TIMER registers  */
    bzero( (void*)&leon2_timer.regs, sizeof(struct regs));

    leon2_timer.address = address;

    /*  Register the timer  */
    leon2_register_io_memory(   address, TIMER_NREGS * 4, 
                                leon2_timer_core_write, leon2_timer_core_read, NULL);

}



