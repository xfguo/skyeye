/*
 * =====================================================================================
 *
 *       Filename:  types.h
 *
 *    Description:  Commong types
 *
 *        Version:  1.0
 *        Created:  16/04/08 12:37:02
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

#ifndef __SPARC_TYPES_H_
#define __SPARC_TYPES_H_
#include "skyeye_types.h"
/*  SPARC types */
/*
typedef unsigned long long   uint64;
typedef unsigned int   uint32;
typedef unsigned char   uint8;
typedef unsigned short  uint16;
*/
typedef long long       int64;
typedef int             int32;
typedef short           int16;
typedef char            int8;
#ifndef NULL
#define		NULL	((void *)0)
#endif
#if 0
typedef enum _boolean_t
{
    false = 0,
    true = 1,
}skyeye_boolean_t;
#endif
//typedef bool skyeye_boolean_t;
#endif

