/*
 * =====================================================================================
 *
 *       Filename:  traps.h
 *
 *    Description:  Header file for the SPARC Traps implementation
 *
 *        Version:  1.0
 *        Created:  21/04/08 18:22:29
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */


#ifndef _TRAPS_H_
#define _TRAPS_H_

#include "skyeye_types.h"

/* 
 * ===  CLASS     ======================================================================
 *         Name:  trap_handle_t
 *  Description:  This class defines what must be implemented by the trap
 *  implementation.
 * =====================================================================================
 */
typedef struct _trap_t
{
    int (*init)(void *);
    int (*signal)(int tt);
    int (*trigger)(void);
}trap_handle_t;

/* 
 * ===  CLASS     ======================================================================
 *         Name:  trap_mode_t
 *  Description:  This class defines the trap mode type. A trap may be
 *  synchronous or asynchronous.
 * =====================================================================================
 */
typedef enum _trapmode
{
    NOTRAP = 0,
    SYNCH_TRAP,
    ASYNCH_TRAP,
}trap_mode_t;

/* 
 * ===  CLASS     ======================================================================
 *         Name:  trap_t
 *  Description:  This class defines the parameters related to the traps. The
 *  parameters are:
 *      - tt:       Trap Type
 *      - priority: Trap priority
 *      - mode:     Trap mode (sync or async)
 * =====================================================================================
 */
typedef struct _trap_s
{
    uint8 tt;
    uint8 priority;
    trap_mode_t mode;
}trap_t;

/** exceptions or interrupt request declarations    */
enum {
    RESET   =   0x0,
    DATA_STORE_ERROR,
    INSTR_ACCESS_MMU_MISS,
    INSTR_ACCESS_ERR,
    R_REG_ACCESS_ERR,
    INSTR_ACCESS_EXCEPTION,
    PRIVILEGED_INSTR,
    ILLEGAL_INSTR,
    FP_DISABLED,
    CP_DISABLED,
    UNIMP,
    WATCHPOINT_DETECTED,
    WOF,
    WUF,
    MEM_ADDR_NOT_ALIGNED,
    FP_EXCEPTION,
    CP_EXCEPTION,
    DATA_ACCESS_ERR,
    DATA_ACCESS_MMU_MISS,
    DATA_ACCESS_EXCEPTION,
    TAG_OVERFLOW,
    DIV_BY_ZERO,
};



/*-----------------------------------------------------------------------------
 *  PUBLIC INTERFACE
 *-----------------------------------------------------------------------------*/

/** This structure stores all the handler functions for all the traps   */
extern trap_handle_t trap_handle;
#endif

