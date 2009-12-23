/*
 * =====================================================================================
 *
 *       Filename:  leon2_cfg.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  15/10/08 17:43:23
 *       Modified:  15/10/08 17:43:23
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

#ifndef _LEON2_CFG_H_
#define _LEON2_CFG_H_

#ifndef __SPARC_TYPES_H_
#error "types.h header file must be included before including leon2_timer_unit.h"
#endif

void leon2_cfg_init(uint32 address);

#endif


