/*
 * =====================================================================================
 *
 *       Filename:  traps.c
 *
 *    Description:  Traps SPARC implementation
 *
 *        Version:  1.0
 *        Created:  21/04/08 18:21:30
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

#include "types.h"
#include "traps.h"
#include "sparc.h"
#include "iu.h"
#include "bits.h"
#include "stat.h"
#include "skyeye.h"

#include <stdio.h>

/*  Forward declarations    */
static void tbr_set_tt(uint16 tt);

static int init(void *);
static int trigger(void);
static int signal(int trap);

/** this is the trap handle class which will be called by the IU to handle all
 * the trap situations  */
trap_handle_t trap_handle = {
    init,
    signal,
    trigger,
};

/* 
 * ===  CLASS     ======================================================================
 *         Name:  tt_table
 *  Description:  This structure gets all the information regarding the
 *  interrupt/exceptions and the priority
 * =====================================================================================
 */
static trap_t tt_table[] = {
    //tt  prio type
    {0x00, 1, NOTRAP},  // reset
    {0x2B, 2, NOTRAP},  // data store error
    {0x3C, 2, NOTRAP},  // instruction access MMU miss
    {0x21, 3, NOTRAP},  // instruction access error
    {0x20, 4, NOTRAP},  // r register access error
    {0x01, 5, NOTRAP},  //  instruction access exception
    {0x03, 6, NOTRAP},  //  priviledge instruction
    {0x02, 7, NOTRAP},  //  illegal instruction
    {0x04, 8, NOTRAP},  //  fp disabled
    {0x24, 8, NOTRAP},  //  cp disabled
    {0x25, 8, NOTRAP},  //  unimplemented FLUSH
    {0x0B, 8, NOTRAP},  //  watchpoint detected
    {0x05, 9, NOTRAP},  //  window overflow
    {0x06, 9, NOTRAP},  //  window underflow
    {0x07, 10, NOTRAP}, //  mem address not aligned
    {0x08, 11, NOTRAP}, //  fp exception
    {0x28, 11, NOTRAP}, //  cp exception
    {0x29, 12, NOTRAP}, //  data access error
    {0x2C, 12, NOTRAP}, //  data access MMU miss
    {0x09, 13, NOTRAP}, //  data access exception
    {0x0A, 14, NOTRAP}, //  tag overflow
    {0x2A, 15, NOTRAP}, //  division by zero
};

/** This value stores the number of elements in the tt_table    */
static int tt_table_elem = sizeof(tt_table) / sizeof(trap_t);

/** Local copy of the processor state   */
static sparc_state_t *pstate = (sparc_state_t *)0x0;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  init
 *  Description:  This function initializes the trap module. The function store
 *  a local copy of the processor state 'sparc_state'.
 * =====================================================================================
 */
static int init(void *state_)
{
    /*  FIXME!: to be implemented   */

    if( !state_ )
        return SPARC_ERROR;
    /*  this is the processor state */
    pstate = state_;

//    DBG("TRAPS initialized\n");
    return SPARC_SUCCESS;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  signal
 *  Description:  This function signals a trap. That means that the irq_pending
 *  flag will be flagged, and the TT field of the TBR register will set to the
 *  correct value
 * =====================================================================================
 */
static int signal(int trap)
{

    if( (trap < 0))
        return SPARC_ERROR;
    else if(trap >= tt_table_elem)
    {
        /*  SW trap, the 'trap' value is the tt */
        tbr_set_tt(trap);
    }
    else if( (trap > 0) || (trap < tt_table_elem) )   
    {
        /*  Hw trap, the 'trap' value is the index in the table */
        tbr_set_tt(tt_table[trap].tt);
    }

    pstate->irq_pending = 1;
//    DBG("%s: trap tt=0x%x\n", __func__, trap);

#ifdef SPARC_ENABLE_STAT
    /*  Do statistics   */
    switch(trap)
    {
        case WUF:
            statistics.nunderflow++;
            break;
        case WOF:
            statistics.noverflow++;
            break;
    }
#endif

    return SPARC_SUCCESS;
    
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  trigger
 *  Description:  This function triggers a trap. A trap causes the following to
 *  occur, if ET = 1:
 *  - Traps are disabled, ET = 0
 *  - The existing user supervisor mode is preserved, PS <- S
 *  - The user/supervisor mode is changed to supervisor, S = 1
 *  - The register window is advanced to a new window, CWP = (CWP - 1) % NWINDOWS
 *  - The trapped program counters are saved in local registers 1 and 2 of the
 *  new window, r[17] = PC, r[18] = NPC
 *  - The tt field is written to the particular value that identifies the
 *  exception of interrupt request, except as defined for 'Reset Trap' of 'Error
 *  Mode'
 *  - If the trap is 'Reset Trap' control is transfered to 0 address, PC = 0,
 *  NPC = 4
 *  - if the trap is not a 'Reset Trap' control is transferred into the trap
 *  table, PC=TBR, NPC = TBR + 4
 *
 *  If ET = 0 and a precise trap occurs, the processor enters in error_mode
 *  state and halts execution. If ET = 0 and an interrupt request or an
 *  interrupting or deferred exception occurs, it is ignored.
 * =====================================================================================
 */
static int trigger(void)
{
    int et = bit(PSRREG, PSR_ET);

//    DBG("%s: TBR 0x%x ET=%d PSR=0x%x\n", __func__, TBRREG, et, PSRREG);

    if( et )
    {
        int S;

        // disable traps
        clear_bit(PSRREG, PSR_ET);
        
        // supervisor mode is preserved
        S = bit(PSRREG, PSR_S);
        if( S ) set_bit(PSRREG, PSR_PS); else clear_bit(PSRREG, PSR_PS);

        /*  put the processor in supervisor mode    */
        set_bit(PSRREG, PSR_S);

        /*  adavance the register window    */
        iu_sub_cwp();

        /*  save trapped program counters   */
        REG(L1) = PCREG;
        REG(L2) = NPCREG;

        PCREG = TBRREG;
        NPCREG = TBRREG + 4;

//        DBG("%s: TBR 0x%x CWP=0x%x WIM=0x%x\n", __func__, TBRREG, CWP, WIMREG);
    }
    else
    {
        /*  FIXME: this needs to be specified   */
        SKYEYE_ERR("Interrupt while ET disabled, processor halt (PC = 0x%x, NPC = 0x%x)\n", PCREG, NPCREG);
        skyeye_exit(1);
    }

    return SPARC_SUCCESS;

}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  tbr_set_tt
 *  Description:  This function sets the TT field of the TBR register to the
 *  given value 'tt'
 * =====================================================================================
 */
static void tbr_set_tt(uint16 tt)
{
    sparc_state_t *state = &sparc_state;
    uint32 tbr = (tt << 4) & TBR_TT_MASK;

    /*  we nned to clear the TT field to ensure that not overlaping is produced
     *  in the OR operation bellow  */
    state->tbr &= TBR_BASE_MASK;
    /*  Update the TBR  */
    state->tbr |= tbr;

    return;
}

