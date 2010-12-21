/*
 *	PearPC
 *	sysendian.h
 *
 *	Copyright (C) 1999-2004 Stefan Weyergraf
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __SKYEYE_SWAPENDIAN_H__
#define __SKYEYE_SWAPENDIAN_H__

#include "skyeye_types.h"
//#include "config.h"

static inline uint32 bswap_word(uint32 data)
{
	return (data>>24)|((data>>8)&0xff00)|((data<<8)&0xff0000)|(data<<24);
}

static inline uint64 bswap_dword(uint64 data)
{
	return (((uint64)bswap_word(data)) << 32) | (uint64)bswap_word(data >> 32);
}

static inline uint16 bswap_half(uint16 data)
{
	return (data<<8)|(data>>8);
}

#if HOST_ENDIANESS == HOST_ENDIANESS_LE

/*
 *		Little-endian machine
 */

#	define dword_from_BE(data)	(bswap_dword(data))
#	define word_from_BE(data)	(bswap_word(data))
#	define half_from_BE(data)	(bswap_half(data))

#	define dword_from_LE(data)	((uint64)(data))
#	define word_from_LE(data)	((uint32)(data))
#	define half_from_LE(data)	((uint16)(data))

#	define dword_to_LE(data)	dword_from_LE(data)
#	define word_to_LE(data)	word_from_LE(data)
#	define half_to_LE(data)	half_from_LE(data)

#	define dword_to_BE(data)	dword_from_BE(data)
#	define word_to_BE(data)	word_from_BE(data)
#	define half_to_BE(data)	half_from_BE(data)

#elif HOST_ENDIANESS == HOST_ENDIANESS_BE

/*
 *		Big-endian machine
 */
#	define dword_from_BE(data)	((uint64)(data))
#	define word_from_BE(data)	((uint32)(data))
#	define half_from_BE(data)	((uint16)(data))

#	define dword_from_LE(data)	(bswap_dword(data))
#	define word_from_LE(data)	(bswap_word(data))
#	define half_from_LE(data)	(bswap_half(data))

#	define dword_to_LE(data)	dword_from_LE(data)
#	define word_to_LE(data)	word_from_LE(data)
#	define half_to_LE(data)	half_from_LE(data)

#	define dword_to_BE(data)	dword_from_BE(data)
#	define word_to_BE(data)	word_from_BE(data)
#	define half_to_BE(data)	half_from_BE(data)

#else

/*
 *		Weird-endian machine
 *	HOST_ENDIANESS is neither little- nor big-endian?
 */
#	error "What kind of a weird machine do you have? It's neither little- nor big-endian??? This is unsupported."

#endif

#endif
