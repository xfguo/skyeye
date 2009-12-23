/*
 * =====================================================================================
 *
 *       Filename:  sparcemu.c
 *
 *    Description:  SPARC implementation
 *
 *        Version:  1.0
 *        Created:  16/04/08 15:01:27
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */


#include "types.h"
#include "sparc.h"
#include "traps.h"
#include "iu.h"

extern sparc_state_t sparc_state;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  tbr_set_tbr
 *  Description:  This function set the value of the TBR register.
 * =====================================================================================
 */
void tbr_set_tbr(uint32 base)
{
    sparc_state_t *state = &sparc_state;
    uint32 b = base & TBR_BASE_MASK;

    /*  We only need to set the base value  */
    state->tbr = 0x0;
    state->tbr |= b;
}

void tbr_set_tt(uint8 tt)
{
    sparc_state_t *state = &sparc_state;
    uint32 tbr = (tt << 4) & TBR_TT_MASK;

    state->tbr |= tbr;


    return;
}

