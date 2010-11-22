#ifndef _LIBCPU_TYPES_H_
#define _LIBCPU_TYPES_H_

#include <sys/types.h>

typedef signed char sint8_t;
typedef signed short sint16_t;
typedef signed int sint32_t;
typedef signed long long sint64_t;

#ifndef sun
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
//typedef unsigned long long uint64_t;
#endif

typedef uint32_t addr_t;

typedef uint32_t tag_t;

#endif
