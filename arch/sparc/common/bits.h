/*
 * =====================================================================================
 *
 *       Filename:  sparcemu.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  16/04/08 15:55:55
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

#ifndef _SPARC_EMU_
#define _SPARC_EMU_


/*-----------------------------------------------------------------------------
 *  Bit operations
 *-----------------------------------------------------------------------------*/

#define bitsmask(n, m) ((~(uint32)0 ) >> (m) << (31 - (n) + (m)) >> (31 - (n)))

//#define bits_clear(x, n, m) ((x) & ~bitsmask((n), (m)))
//#define bit_clear(x, b)  bits_clear(x, b, b)

//#define bits_set(x, n, m) ((x) | bitsmask((n), (m)))
//#define bit_set(x, b)  bits_set(x, b, b)
#define bit(x, n) (((x) >> n) & 1)
#define bits(x, n, m) \
    ({		\
     uint32 y = (x);	\
     y << (31 - (n)) >> (31 - (n) + (m)); \
     })

#define set_bits(x, n, m) ((x) |= bitsmask((n), (m)))
#define set_bit(x, b)  set_bits(x, b, b)

#define clear_bits(x, n, m) ((x) &= ~bitsmask((n), (m)))
#define clear_bit(x, b)     clear_bits(x, b, b)

#endif


