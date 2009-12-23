/*
 * =====================================================================================
 *
 *       Filename:  iu.h
 *
 *    Description:  SPARC Integuer Unit
 *
 *        Version:  1.0
 *        Created:  22/04/08 09:57:35
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

#ifndef _IU_H_
#define _IU_H_

#ifndef __SPARC_H__
#error "arch/sparc/common/sparc.h header file must be included"
#endif
#ifndef _TRAPS_H_
#error "arch/sparc/common/traps.h header file must be included"
#endif

/** Number of slied windows */
#define N_WINDOWS       8
#define MAX_NWINDOWS    32

/** Processor State Register (PSR) fields   */
enum
{
    PSR_impl_last = 31,
    PSR_impl_first = 28,
    PSR_ver_last = 27,
    PSR_ver_first = 24,
    PSR_icc_N = 23,
    PSR_icc_Z = 22,
    PSR_icc_V = 21,
    PSR_icc_C = 20,
    PSR_reserved_last = 19,
    PSR_reserved_first = 14,
    PSR_EC = 13,
    PSR_EF = 12,
    PSR_PIL_last = 11,
    PSR_PIL_first = 8,
    PSR_S = 7,
    PSR_PS = 6,
    PSR_ET = 5,
    PSR_CWP_last = 4,
    PSR_CWP_first = 0,
};

enum
{
    TBR_tba_last = 31,
    TBR_tba_first = 12,
    TBR_tt_last = 11,
    TBR_tt_first = 4,
};

/** window register offsets  */
enum { OUT_REG_OFF = 0, LOCAL_REG_OFF = 8, IN_REG_OFF = 16,};

typedef struct _sparc_state
{
    /*  Register set    */
    uint32 global[8];
    uint32 regbase[MAX_NWINDOWS * 16 + 8];
    uint32 *regwptr[2];
    uint32 fp_regs[32]; // FPU registers
    uint32 y;   // multiply/divide register
    uint32 psr; // PSR register
    uint32 wim; // window invalid register
    uint32 tbr; // Trap base register
    uint32 pc;  // Program counter
    uint32 npc; // Next program counter
    uint32 fpsr;    // Floating point status register
    uint32 cpsr;    // coprocesor status register

    uint8 cwp;  // Current window pointer

    /*  CPU state   */
    uint8 mode;         // CPU mode
    uint32 pipeline;    // instruction in the delay-slot


    int irq_pending;        // IRQ pending flag
    uint64 cycle_counter;   // processor cycle counter
	uint32 steps;	// how manys instructions executed by processor

}sparc_state_t;

/** Defines to deal with the TBR register   */
#define TBR_BASE_MASK   0xFFFFF000
#define TBR_TT_MASK     0x00000FF0

/** 
 * This is the sparc_state variable, which maintains the SPARC CPU status
 * information
 * */
extern sparc_state_t sparc_state;

#define REG(x)  sparc_state.regwptr[(x > 7)][x - 8*(x > 7)]
#define FPREG(x)  sparc_state.fp_regs[x]
#define YREG  sparc_state.y
#define PSRREG  sparc_state.psr
#define WIMREG  sparc_state.wim
#define TBRREG  sparc_state.tbr
#define PCREG  sparc_state.pc
#define NPCREG  sparc_state.npc
#define FPSRREG  sparc_state.fpsr
#define CPSRREG  sparc_state.cpsr
#define CWP     (bits(sparc_state.psr, PSR_CWP_last, PSR_CWP_first))

/*-----------------------------------------------------------------------------
 *  PUBLIC INTERFACE
 *-----------------------------------------------------------------------------*/
uint32  iu_sub_cwp(void);
uint32  iu_add_cwp(void);
int     sign_ext(int, int);
int     init_sparc_iu(void);
void    iu_set_cwp(int new_cwp);

extern trap_handle_t *traps;

//#define sign_ext22(x)   sign_ext(x, 22)
//#define sign_ext13(x)   sign_ext(x, 13)

#define psr_get_carry()         bit(sparc_state.psr, PSR_icc_C)
#define psr_set_carry()         set_bit(sparc_state.psr, PSR_icc_C)
#define psr_clear_carry()       clear_bit(sparc_state.psr, PSR_icc_C)

#define psr_get_overflow()         bit(sparc_state.psr, PSR_icc_V)
#define psr_set_overflow()      set_bit(sparc_state.psr, PSR_icc_V)
#define psr_clear_overflow()    clear_bit(sparc_state.psr, PSR_icc_V)

#define psr_get_zero()         bit(sparc_state.psr, PSR_icc_Z)
#define psr_set_zero()          set_bit(sparc_state.psr, PSR_icc_Z)
#define psr_clear_zero()        clear_bit(sparc_state.psr, PSR_icc_Z)

#define psr_get_neg()         bit(sparc_state.psr, PSR_icc_N)
#define psr_set_neg()           set_bit(sparc_state.psr, PSR_icc_N)
#define psr_clear_neg()         clear_bit(sparc_state.psr, PSR_icc_N)
    
#endif

