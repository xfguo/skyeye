#ifndef __PPC_DYNCOM_DEBUG__
#define __PPC_DYNCOM_DEBUG__

#include <math.h>

#define DEBUG_TAG					1
#define DEBUG_TRANSLATE				1 << 1
#define DEBUG_TRANSLATE_COND		1 << 2
#define DEBUG_DEC					1 << 3
#define DEBUG_INTERFACE				1 << 4

#define DEBUG_MASK				0xffffffff

static char *debug_name[] = {"TAG",
	"TRANSLATE",
	"TRANSLATE_COND",
	"DEC",
	"INTERFACE"};

#define debug(debug_flag, ...)												\
	do {																	\
		if (debug_flag & DEBUG_MASK)										\
			printf("[PPC_DYNCOM_DEBUG-%s] ",								\
					debug_name[(int)log2(debug_flag)]);						\
			printf(__VA_ARGS__);											\
	} while(0)

#endif
