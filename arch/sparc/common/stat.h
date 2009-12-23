/*
 * =====================================================================================
 *
 *       Filename:  stat.h
 *
 *    Description:  This file implements all the structures for statistics
 *
 *        Version:  1.0
 *        Created:  01/10/08 14:07:18
 *       Modified:  01/10/08 14:07:18
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

#ifndef __STAT__H_
#define __STAT__H_

#ifndef __SPARC_TYPES_H_
#error  "arch/sparc/common/types.h shall be included before stat.h"
#endif

typedef struct _stat
{
    uint64  starttime;      /*  Simulation start time   */
    uint64  tottime;        /*  Simulation total time   */
    uint64  nload;          /*  LOAD instructions   */
    uint64  nstore;         /*  STORE instructions  */
    uint64  noverflow;      /*  overflows   */
    uint64  nunderflow;     /*  underflows  */
    uint64  nbicc;          /*  BICC instructions   */
    uint64  nmemi;          /*  Memory instructinos */
    uint64  ncall;          /*  CALL instructions   */
    uint64  nmisci;         /*  Remaining instructions  */
}s_stat;


extern struct _stat statistics;

void STAT_reset(void);
void STAT_fini(void);
void STAT_show(void);

#endif

