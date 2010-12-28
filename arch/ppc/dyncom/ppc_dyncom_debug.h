#ifndef __PPC_DYNCOM_DEBUG__
#define __PPC_DYNCOM_DEBUG__

#define DEBUG_TAG					1
#define DEBUG_TRANSLATE			1 << 1
#define DEBUG_TRANSLATE_COND		1 << 2
#define DEBUG_DEC					1 << 3

#define DEBUG_MASK				0xffffffff

#define debug(debug_flag, ...)								\
	do {													\
		if (debug_flag & DEBUG_MASK)						\
			printf("[PPC_DYNCOM_DEBUG-%x]", debug_flag);	\
			printf(__VA_ARGS__);							\
	} while(0)

#endif
