/*
 * =====================================================================================
 *
 *       Filename:  helper.c
 *
 *    Description:  Help functions
 *
 *        Version:  1.0
 *        Created:  01/10/08 18:37:04
 *       Modified:  01/10/08 18:37:04
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

#include "types.h"

void memcpy32(uint32 *d, uint32 *s)
{
    d[0] = s[0];
    d[1] = s[1];
    d[2] = s[2];
    d[3] = s[3];
    d[4] = s[4];
    d[5] = s[5];
    d[6] = s[6];
    d[7] = s[7];
}

