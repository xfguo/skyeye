#ifndef __PPC_DYNCOM_DEBUG__
#define __PPC_DYNCOM_DEBUG__

#include <math.h>

#define DEBUG_CORE 0
#define START_DEBUG_ICOUNT -1 
//#define START_DEBUG_ICOUNT 219010000
#define START_DEBUG_PC 0xffffffff
//#define START_DEBUG_PC -1

#define STOP_DEBUG_ICOUNT -1
//#define STOP_DEBUG_ICOUNT 219010395
#define STOP_DEBUG_PC 0xffffffff
extern int ppc_dyncom_start_debug_flag;

#define DEBUG_TAG					1
#define DEBUG_TRANSLATE				1 << 1
#define DEBUG_TRANSLATE_COND		1 << 2
#define DEBUG_DEC					1 << 3
#define DEBUG_INTERFACE				1 << 4
#define DEBUG_NOT_TEST				1 << 5
#define DEBUG_RUN					1 << 6

//#define DEBUG_MASK				0xffffffff
#define DEBUG_MASK				0x0

static char *debug_name[] = {"TAG",
	"TRANSLATE",
	"TRANSLATE_COND",
	"DEC",
	"INTERFACE",
	"NOTTEST",
	"RUN"};

#define debug(debug_flag, ...)												\
	do {																	\
		if ((debug_flag & DEBUG_MASK) && ppc_dyncom_start_debug_flag){		\
			printf("[PPC_DYNCOM_DEBUG-%s] ",								\
					debug_name[(int)log2(debug_flag)]);						\
			printf(__VA_ARGS__);											\
		}																	\
	} while(0)

#endif
