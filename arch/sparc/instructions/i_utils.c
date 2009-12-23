/*
 * =====================================================================================
 *
 *       Filename:  i_utils.c
 *
 *    Description:  This file implements all the utility functions needed by the
 *    instructions
 *
 *        Version:  1.0
 *        Created:  06/05/08 15:06:11
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */


#include "../common/bits.h"
#include "../common/sparc.h"
#include "../common/traps.h"
#include "../common/iu.h"
#include "../common/debug.h"

int sign_ext(int x, int nbits)
{
//    int n = nbits - 1;
//
//    if( bit(x, n) )
//        return (set_bits(x, 31, nbits));
//    else return x;
    int n = 32 - nbits;


    return (int)( (x << n) >> n );
}

void clear_icc(void)
{
    /*  clear the condition */
    clear_bit(PSRREG, PSR_icc_Z);
    clear_bit(PSRREG, PSR_icc_N);
    clear_bit(PSRREG, PSR_icc_V);
    clear_bit(PSRREG, PSR_icc_C);
}

