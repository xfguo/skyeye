/*
 * =====================================================================================
 *
 *       Filename:  i_utils.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/05/08 15:17:14
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

#ifndef _I_UTILS_H__
#define _I_UTILS_H__

#define sign_ext22(x)   sign_ext(x, 22)
#define sign_ext13(x)   sign_ext(x, 13)


void clear_icc(void);
int sign_ext(int x, int nbits);

#endif

