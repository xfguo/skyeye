/*
 * =====================================================================================
 *
 *       Filename:  sparc.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  15/04/08 11:42:41
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

#ifndef __SPARC_H__
#define __SPARC_H__

#include "types.h"
#ifndef __SPARC_TYPES_H_
#error "arch/sparc/common/types.h shall be included before sparc.h header"
#endif

#if 1
enum regnames { G0, G1, G2, G3, G4, G5, G6, G7,
                O0, O1, O2, O3, O4, O5, SP, O7,
                L0, L1, L2, L3, L4, L5, L6, L7,
                I0, I1, I2, I3, I4, I5, FP, I7,
                /*  From here down cannot be used in REG()  */
                F0, F1, F2, F3, F4, F5, F6, F7,
                F8, F9, F10, F11, F12, F13, F14, F15,
                F16, F17, F18, F19, F20, F21, F22, F23,
                F24, F25, F26, F27, F28, F29, F30, F31,
                Y, PSR, WIM, TBR, PC, NPC, FPSR, CPSR };
#endif

/**
 *  \class sparc_return_t
 *  \brief  This structure defines all the possible error codes
 */
typedef enum _sparc_return_t
{
    SPARC_SUCCESS = 0,
    SPARC_ERROR = -1,

    SPARC_ISA_INIT_ERR  =   -100,
    SPARC_ISA_NEW_I_ERR =  -101,

    SPARC_EMU_BAD_TT    = -200,

}sparc_return_t;

/**
 *  \class sparc_instruction_t
 *  \brief  This structure defines the instruction attributes.
 *
 *  \par execute:   This is the function to be executed.
 *  \par code:      This is the instructon code.
 *  \par disassemble: This is the functions which disassembles the instruction.
 */
typedef struct _i 
{
	int (*execute)(void *);
	int (*disassemble)(uint32 instr, void *);
	uint32 opcode_mask;
    int priviledge;
    int format;
}sparc_instruction_t;

/**
 *  \class iu_config_t
 *  \brief  This class defines which functions shall be implemented by the Integer
 *  Unit module.
 *   
 *  \par    iu_init_state:  This function shall initialize the integer unit
 *  registering the ISA set, and initialize the traps
 *  \par    iu_cycle_step:  This function executes the next instruction
 *  \par    iu_error_state: This function implements the IU error state
 *  \par    iu_trap_cycle:  This function chekcs whether there is a trap pending
 *  and handles it
 *  \par    iu_set_pc:      This function set the program counter
 *  \par    iu_get_pc:      This function returns the program counter
 */
typedef struct _iu_config
{
    void (*iu_init_state)(void);
    int (*iu_cycle_step)(void);     /*  returns the number of cycles consumed   */
    void (*iu_error_state)(void);
    int (*iu_trap_cycle)(void);
    uint32 (*iu_get_pc)(void);
    void (*iu_set_pc)(uint32);
}iu_config_t;

/** This is the instruction type, in SPARC 32 bits  */
typedef uint32 sparc_instr;

/*-----------------------------------------------------------------------------
 *  PUBLIC INTERFACE
 *-----------------------------------------------------------------------------*/
void sparc_register_iu(iu_config_t *iu_);
void init_sparc_arch();


#endif

