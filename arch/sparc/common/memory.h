/*
 * =====================================================================================
 *
 *       Filename:  memory.h
 *
 *    Description:  Memory module header file
 *
 *        Version:  1.0
 *        Created:  15/04/08 17:30:40
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

#ifndef _SPARC_MEMORY_
#define _SPARC_MEMORY_

#include "skyeye_config.h"
//#include "code_cov.h"

#ifndef __SPARC_TYPES_H_
#error "arch/sparc/common/types.h header fle must be included before memory.h"
#endif

#define MAX_STRING      10


#define sparc_memory_store_byte(offset, value) bus_write(8, offset, value)
#define sparc_memory_store_word16(offset, value) bus_write(16, offset, value)
#define sparc_memory_store_word32(offset, value)    \
    ({  \
     bus_write(32, offset, value);  \
     })

#define sparc_memory_read_word32(value,offset) 	bus_read(32, offset, value)
#define sparc_memory_read_word16(value,offset) 	bus_read(16, offset, value)
#define sparc_memory_read_byte(value,offset) 	bus_read(8, offset, value)

#endif

