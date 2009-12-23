/*
 * =====================================================================================
 *
 *       Filename:  iu_default.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  22/04/08 09:57:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <string.h>

#include "types.h"
#include "sparc.h"
#include "traps.h"
#include "../common/memory.h"
#include "bits.h"
#include "iu.h"
#include "debug.h"
#include "stat.h"

#include "skyeye_config.h"

static void iu_init_state(void);
static int  iu_cycle_step(void);
static void iu_error_state(void);
static sparc_instruction_t *iu_get_instr(uint32 instr);
static inline uint32 iu_get_pc(void);
static inline void iu_set_pc(uint32 addr);
static inline int iu_trap_cycle(void);

static void iu_isa_register(void);

#define FORMAT_TYPES    4
#define BICC            0x0
#define CALL            0x1
#define MISC            0x2
#define MEMORY          0x3

static uint64 i_count[FORMAT_TYPES];
trap_handle_t *traps;

struct _i_set
{
    sparc_instruction_t *instr;
};

static struct _i_set *i_set[FORMAT_TYPES];

static iu_config_t iu_config = {
   iu_init_state,
   iu_cycle_step,
   iu_error_state,
   iu_trap_cycle,
   iu_get_pc,
   iu_set_pc,
};

/* 
 * ===  VARIABLE  ======================================================================
 *         Name:  sparc_state
 *  Description:  This variable stores the processor state
 * =====================================================================================
 */
sparc_state_t sparc_state;

/*-----------------------------------------------------------------------------
 *  IU ISA set
 *-----------------------------------------------------------------------------*/
extern sparc_instruction_t i_nop;
extern sparc_instruction_t i_or;
extern sparc_instruction_t i_and;
extern sparc_instruction_t i_andn;
extern sparc_instruction_t i_orn;
extern sparc_instruction_t i_xor;
extern sparc_instruction_t i_xnor;
extern sparc_instruction_t i_orcc;
extern sparc_instruction_t i_andcc;
extern sparc_instruction_t i_andncc;
//extern sparc_instruction_t i_orncc;
//extern sparc_instruction_t i_xorcc;
//extern sparc_instruction_t i_xnorcc;

extern sparc_instruction_t i_sll;
extern sparc_instruction_t i_srl;
extern sparc_instruction_t i_sra;

extern sparc_instruction_t i_add;
extern sparc_instruction_t i_addx;
extern sparc_instruction_t i_addcc;
//extern sparc_instruction_t i_addxcc;

extern sparc_instruction_t i_sub;
extern sparc_instruction_t i_subx;
extern sparc_instruction_t i_subcc;
extern sparc_instruction_t i_subxcc;

extern sparc_instruction_t i_stb;
extern sparc_instruction_t i_sth;
extern sparc_instruction_t i_st;
extern sparc_instruction_t i_std;

extern sparc_instruction_t i_sethi;

extern sparc_instruction_t i_jmpl;

extern sparc_instruction_t i_wry;
//extern sparc_instruction_t i_wrasr;
extern sparc_instruction_t i_wrpsr;
extern sparc_instruction_t i_wrwim;
extern sparc_instruction_t i_wrtbr;

extern sparc_instruction_t i_rdy;
//extern sparc_instruction_t i_rdasr;
extern sparc_instruction_t i_rdpsr;
extern sparc_instruction_t i_rdwim;
extern sparc_instruction_t i_rdtbr;

extern sparc_instruction_t i_ld;
extern sparc_instruction_t i_ldd;
extern sparc_instruction_t i_ldub;
extern sparc_instruction_t i_lduh;
extern sparc_instruction_t i_ldsb;
extern sparc_instruction_t i_ldsh;

extern sparc_instruction_t i_ba;
extern sparc_instruction_t i_bn;
extern sparc_instruction_t i_bne;
extern sparc_instruction_t i_be;
extern sparc_instruction_t i_bg;
extern sparc_instruction_t i_ble;
extern sparc_instruction_t i_bge;
extern sparc_instruction_t i_bl;
extern sparc_instruction_t i_bgu;
extern sparc_instruction_t i_bleu;
extern sparc_instruction_t i_bcc;
extern sparc_instruction_t i_bcs;
extern sparc_instruction_t i_bpos;
extern sparc_instruction_t i_bneg;
extern sparc_instruction_t i_bvc;
extern sparc_instruction_t i_bvs;

extern sparc_instruction_t i_flush;

extern sparc_instruction_t i_save;
extern sparc_instruction_t i_restore;

extern sparc_instruction_t i_ldf;
extern sparc_instruction_t i_lddf;

extern sparc_instruction_t i_call;

extern sparc_instruction_t i_rett;

extern sparc_instruction_t i_ta;

extern sparc_instruction_t i_stf;
extern sparc_instruction_t i_stdf;
extern sparc_instruction_t i_stfsr;

extern sparc_instruction_t i_udiv;
extern sparc_instruction_t i_smul;
extern sparc_instruction_t i_umul;

extern sparc_instruction_t i_mulscc;

/*-----------------------------------------------------------------------------
 *  End of ISA set
 *-----------------------------------------------------------------------------*/

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  init_sparc_iu
 *  Description:  
 * =====================================================================================
 */
int init_sparc_iu(void)
{
    sparc_register_iu(&iu_config);

    /*  If we reach that point means success    */
    return 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iu_init
 *  Description:  This function initializes the SPARC Integer Unit
 * =====================================================================================
 */
static void iu_init_state(void)
{
    int i;
    sparc_state_t *state = &sparc_state;

    DBG("%s(): IU initialization\n", __func__);

    state->irq_pending = 0;
    state->cycle_counter = 0;
    state->regwptr[0] = &state->global[0];
    state->regwptr[1] = state->regbase + (0 * 16);  /*  we start in the cwp = 0 */
    PSRREG = 0x0;
    WIMREG = 0x2;

    /*  
     *  Put the processor in supervisor mode, S=1 PS=1
     *  Enable the traps
     *  Enable the floating point unit
     */
    set_bit(PSRREG, PSR_S);
    set_bit(PSRREG, PSR_PS);
    set_bit(PSRREG, PSR_ET);
    set_bit(PSRREG, PSR_EF);
    for( i = 0; i < FORMAT_TYPES; ++i)
        i_count[i] = 0;

    /*  initialize the %sp and %fp pointers to the end of the RAM to allow
     *  execution from RAM space.
     *  Some operating systems (RTEMS) assumes %sp to point before the end of
     *  the RAM. So we initialize the %fp to the end_of_RAM and the %sp to
     *  the end_of_RAM - 16 bytes   */
	printf("fp and sp should be initialized here.\n");
	REG(FP) = (0x40000000 + 0x040000) & 0xFFFFFFF0;
	REG(SP) = REG(FP) - 0x180;
#if 0
    for(i = 0; i < mem_bank_list.num_banks; ++i)
    {
        if (!strncmp ("ram", mem_bank_list.mb[i].bank_name, strlen ("ram")))
        {
            REG(FP) = mem_bank_list.mb[i].addr + mem_bank_list.mb[i].size;
            /*  Sure about the 4-byte alignment */
            REG(FP) = REG(FP) & 0xFFFFFFF0;
            /*  The frame pointer is 48 bytes below the stack pointer   */
            REG(SP) = REG(FP) - 0x180;
        }
    }
#endif



    /*  Register the trap handling  */
    traps = &trap_handle;
    /*  initialize the trap handing. The processor state is needed  */
    traps->init(state);

    iu_isa_register();
}

static inline int iu_trap_cycle(void)
{
    if( sparc_state.irq_pending )
    {
        traps->trigger();
        sparc_state.irq_pending = 0;
    }
    return 0;
}

static inline void iu_set_pc(uint32 addr)
{
    PCREG = addr;
    NPCREG = PCREG + 4;

}

static inline uint32 iu_get_pc(void)
{
    return (PCREG);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iu_cycle
 *  Description:  This function executes a processor cycle
 *  Return:
 *      The function returns the number of cycles consumed in the instruction
 *      execution.
 * =====================================================================================
 */
static int iu_cycle_step(void)
{
    int cycles = 0;
    uint32 pc;
    sparc_instr instr;
    sparc_instruction_t *pi;

    /*  program counter */
    pc = iu_get_pc();

    /*  Read the memory where the next instruction is   */
    sparc_memory_read_word32(&instr, pc);

    /*  Perform the code coverage if configured */
    /*  Code coverage   */
#if 0
    COV_PROF(EXEC_FLAG, pc);
#endif
    DBG("0x%08x\t0x%08x\t", pc, instr);
//    DBG("%s(): instr = 0x%x\n", __func__, instr);


    if( (pi = iu_get_instr(instr)) == NULL )
    {
        DBG("Instruction not implemented at PC=0x%x\n", pc);
        skyeye_exit(1);
    }

    cycles = (*pi->execute)(&sparc_state);

    if( cycles > 0 )
        sparc_state.cycle_counter += cycles;

    /*  Clear the %g0. This register must be allways 0   */
    REG(G0) = 0;

    /*  Clear the WIM register  */
//    WIMREG = WIMREG & 0x000000FF;
	sparc_state.steps++;
    return cycles;
}

static void iu_error_state(void)
{
    /*  FIXME!: to be implemented   */
}

static void iu_isa_register(void)
{
    DBG("Registering all the ISA set\n", __func__);

    /*  FIXME!: this function needs to return status value  */
    iu_i_register(&i_nop);

    // B.11 Logical instructions
    iu_i_register(&i_or);
    iu_i_register(&i_and);
    iu_i_register(&i_andn);
    iu_i_register(&i_orn);
    iu_i_register(&i_xor);
    iu_i_register(&i_xnor);
    iu_i_register(&i_orcc);
    iu_i_register(&i_andcc);
    iu_i_register(&i_andncc);
//    iu_i_register(&i_orncc);
//    iu_i_register(&i_xorcc);
//    iu_i_register(&i_xnorcc);

    // Sparc.v8 B.12 Shift Instructions
    iu_i_register(&i_sll);
    iu_i_register(&i_srl);
    iu_i_register(&i_sra);

    // Sparc.v8 B.13 Add instructions
    iu_i_register(&i_add);
    iu_i_register(&i_addx);
    iu_i_register(&i_addcc);
//    iu_i_register(&i_addxcc);

    // Sparc.v8 B.15 Substract instructions
    iu_i_register(&i_sub);
    iu_i_register(&i_subx);
    iu_i_register(&i_subcc);
    iu_i_register(&i_subxcc);

    // Sparc.v8 B.4 Store Integer Instructions
    iu_i_register(&i_stb);
    iu_i_register(&i_sth);
    iu_i_register(&i_st);
    iu_i_register(&i_std);

    // Sparc.v8 B.9 SETHI instruction
    iu_i_register(&i_sethi);

    /*  Sparc.v8 B.25 jump and link instruction */
    iu_i_register(&i_jmpl);

    /*  Sparc.v8 B.29 Write State Register Instructions */
    iu_i_register(&i_wry);
//    iu_i_register(&i_wrasr);
    iu_i_register(&i_wrpsr);
    iu_i_register(&i_wrwim);
    iu_i_register(&i_wrtbr);

    /*  Sparc.v8 B.28 Read State Register Instructions */
    iu_i_register(&i_rdy);
//    iu_i_register(&i_wrasr);
    iu_i_register(&i_rdpsr);
    iu_i_register(&i_rdwim);
    iu_i_register(&i_rdtbr);

    /*  Sparc.v8 B.1 Load Integer Instructinos */
    iu_i_register(&i_ldsb);
    iu_i_register(&i_ldsh);
    iu_i_register(&i_ldub);
    iu_i_register(&i_lduh);
    iu_i_register(&i_ld);
    iu_i_register(&i_ldd);

    /*  Sparc.v8 B.21 Branch on Integer Condition Codes */
    iu_i_register(&i_ba);
    iu_i_register(&i_bn);
    iu_i_register(&i_bne);
    iu_i_register(&i_be);
    iu_i_register(&i_bg);
    iu_i_register(&i_ble);
    iu_i_register(&i_bge);
    iu_i_register(&i_bl);
    iu_i_register(&i_bgu);
    iu_i_register(&i_bleu);
    iu_i_register(&i_bcc);
    iu_i_register(&i_bcs);
    iu_i_register(&i_bpos);
    iu_i_register(&i_bneg);
    iu_i_register(&i_bvc);
    iu_i_register(&i_bvs);

    iu_i_register(&i_flush);

    /*  Sparc.v8 B.20 Save and Restore instructions */
    iu_i_register(&i_save);
    iu_i_register(&i_restore);

    /*  Sparc.v8 B.2 Load Floating Point instructions   */
    iu_i_register(&i_ldf);
    iu_i_register(&i_lddf);

    /*  Sparc.v8 B.24 Call and Link instructions   */
    iu_i_register(&i_call);

    /*  Sparc.v8 B.26 Return from Trap instruction */
    iu_i_register(&i_rett);

    /*  Sparc.v8 B.27 Trap on Integer condition codes instructions  */
    iu_i_register(&i_ta);
//    iu_i_register(&i_tn);
//    iu_i_register(&i_tne);
//    iu_i_register(&i_te);
//    iu_i_register(&i_tg);
//    iu_i_register(&i_tle);
//    iu_i_register(&i_tge);
//    iu_i_register(&i_tl);
//    iu_i_register(&i_tgu);
//    iu_i_register(&i_tleu);
//    iu_i_register(&i_tcc);
//    iu_i_register(&i_tcs);
//    iu_i_register(&i_tpos);
//    iu_i_register(&i_tneg);
//    iu_i_register(&i_tvc);
//    iu_i_register(&i_tvs);

    iu_i_register(&i_stf);
    iu_i_register(&i_stdf);
    iu_i_register(&i_stfsr);

    iu_i_register(&i_udiv);
    iu_i_register(&i_smul);
    iu_i_register(&i_umul);

    iu_i_register(&i_mulscc);


    int i = 0, n_instructions = 0;
    for (i = 0; i <  FORMAT_TYPES; ++i)
        n_instructions += i_count[i];

    DBG("ISA: %d registered instructions\n", n_instructions);
}

static sparc_instruction_t *iu_get_instr(uint32 instr)
{
    int i;
    sparc_state_t *state = &sparc_state;
    int format;

    format = bits(instr, 31, 30);
    if( (format < 0) || (format > FORMAT_TYPES) )
        return NULL;

    if( !state ) return NULL;

    for( i = 0; i < i_count[format]; ++i )
    {
        if( (i_set[format][i].instr) && i_set[format][i].instr->disassemble(instr, state) )
        {
//            DBG("%s: opcode - 0x%x\n", __func__, i_set[i].instr->opcode_mask);
            return i_set[format][i].instr;
        }
    }

#ifdef SPARC_ENABLE_STAT
    /*  Perform statistics  */
    switch(format)
    {
        case BICC:
            statistics.nbicc++;
            break;
        case MEMORY:
            statistics.nmemi++;
            break;
        case CALL:
            statistics.ncall++;
            break;
        case MISC:
            statistics.nmisci++;
            break;

    }
#endif

    return NULL;
}

sparc_return_t iu_i_register(sparc_instruction_t *i_new)
{
    /*  Sanity check    */
    if( !i_new ) return SPARC_ISA_NEW_I_ERR;

    int format = i_new->format;

    /*  Increment the instruction counter   */
    i_count[format]++;

    /*  Allocate new room for the new instruction   */
    i_set[format] = (struct _i_set *)realloc(i_set[format], i_count[format] * sizeof(struct _i_set) );
    if( !i_set[format] )
    {
        DBG("%s(): Error registering new instruction");
        return SPARC_ISA_NEW_I_ERR;
    }

    /*  register the new instruction    */
    i_set[format][i_count[format] - 1].instr = i_new;


    return SPARC_SUCCESS;
}

void iu_set_cwp(int new_cwp)
{
    sparc_state_t *env = &sparc_state;

    if( CWP == (N_WINDOWS - 1) )
        memcpy32(env->regbase, (env->regbase + (N_WINDOWS * 16)));

    clear_bits(PSRREG, PSR_CWP_last, PSR_CWP_first);
    PSRREG |= new_cwp;

    if( new_cwp == (N_WINDOWS - 1) )
        memcpy32((env->regbase + (N_WINDOWS * 16)), env->regbase);

    env->regwptr[1] = env->regbase + (new_cwp * 16);
}

uint32 iu_sub_cwp(void)
{
    uint32 cwp;
    uint32 psr = PSRREG;

    REG(0) = 0;     // It hard-wired to 0

    /*  Get the CWP */
    cwp = CWP;

    cwp = (cwp - 1) & (N_WINDOWS - 1);

    iu_set_cwp(cwp);

    return cwp;
}

uint32 iu_add_cwp(void)
{
    uint32 cwp;
    uint32 psr = PSRREG;

    REG(0) = 0;     // It hard-wired to 0

    /*  Get the CWP */
    cwp = CWP;

    cwp = (cwp + 1) & (N_WINDOWS - 1);

    iu_set_cwp(cwp);

    return cwp;
}


