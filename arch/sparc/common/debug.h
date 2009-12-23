/*
 * =====================================================================================
 *
 *       Filename:  i_debug.h
 *
 *    Description:  Debug macros.
 *
 *        Version:  1.0
 *        Created:  18/04/08 08:19:38
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

#ifndef _SPARC_DEBUG_H_
#define _SPARC_DEBUG_H_

#if SPARC_DEBUG
#include <stdio.h>

#define DBG(msg...) fprintf(stderr, ##msg)
#else
#define DBG(msg...)
#endif

#define ERR(fmt,args...) fprintf(stderr, "%s:%d: " fmt "\n", __FILE__, __LINE__, ##args);

#endif

