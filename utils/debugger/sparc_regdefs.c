/*
 * =====================================================================================
 *
 *       Filename:  sparc_regdefs.c
 *
 *    Description:  necessary sparc definition for skyeye debugger
 *
 *        Version:  1.0
 *        Created:  15/04/08 15:45:55
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

/*
 * 2009-11-02 Michael.Kang, rewrite according to the new framework.
 */
#include "skyeye.h"
#include "skyeye2gdb.h"
#include "skyeye_types.h"
#include "skyeye_arch.h"

#define NUMREGS 72
     
/* Number of bytes of registers.  */
#define NUMREGBYTES (NUMREGS * 4)
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sparc_regiter_raw_size
 *  Description:  This function returns the number of bytes register for the
 *  given register number.
 *  In the case of the SPARC architecture, all the registers's got 4 bytes.
 * =====================================================================================
 */
static int sparc_register_raw_size(int x)
{
    return 4;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sparc_regiter_byte
 *  Description:  This function return the byte from which the register is store
 *  on.
 * =====================================================================================
 */
static int sparc_register_byte(int x)
{
    return 4 * x;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sparc_store_register
 *  Description:  This function stores the value given in the 'memory' pointer,
 *  to the 'x' register.
 * =====================================================================================
 */
static int sparc_store_register(int rn, unsigned char * memory)
{
#if 0
    uint32 val;

    /*  Get the value from memory   */
    val = frommem(memory);

    /*  Store the value to the register */
    if( rn >= G0 && rn<= I7 )
        REG(rn) = val;
    else if( rn >= F0 && rn<= F31 )
        FPREG(rn) = val;
    else if( rn == Y )
        YREG = val;
    else if( rn == PSR )
        PSRREG = val;
    else if( rn == WIM )
        WIMREG = val;
    else if( rn == TBR )
        TBRREG = val;
    else if(rn == PC )
        PCREG = val;
    else if( rn == NPC )
        NPCREG = val;
    else if( rn == FPSR )
        FPSRREG = val;
    else if( rn == CPSR )
        CPSRREG = val;
    else return -1;

    return 0;
#endif
	generic_arch_t* arch_instance = get_arch_instance("");
	arch_instance->set_regval_by_id(rn, frommem(memory));
	return 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sparc_fetch_register
 *  Description:  Send the register information out to memory.
 * =====================================================================================
 */
static int sparc_fetch_register(int rn, unsigned char * memory)
{
#if 0
    uint32 val;

    /*  Store the value to the register */
    if( rn >= G0 && rn<= I7 )
        val = REG(rn);
    else if( rn >= F0 && rn<= F31 )
        val = FPREG(rn);
    else if( rn == Y )
        val = YREG;
    else if( rn == PSR )
        val = PSRREG;
    else if( rn == WIM )
        val = WIMREG;
    else if( rn == TBR )
        val = TBRREG;
    else if(rn == PC )
        val = PCREG;
    else if( rn == NPC )
        val = NPCREG;
    else if( rn == FPSR )
        val = FPSRREG;
    else if( rn == CPSR )
        val = CPSRREG;
    else return -1;


    tomem(memory, val);
    return 0;
#endif
	uint32 regval;
	generic_arch_t* arch_instance = get_arch_instance("");
	regval = arch_instance->get_regval_by_id(rn);
	tomem (memory, regval);
	return 0;
}

static register_defs_t sparc_reg_defs;

/*
 * register sparc register type to the array
 */
void init_sparc_register_defs(void)
{
    SKYEYE_DBG("%s: Registering the GDB stub for remote debugging\n", __func__);

	sparc_reg_defs.name = "sparc";
	sparc_reg_defs.register_raw_size = sparc_register_raw_size;
	sparc_reg_defs.register_bytes = NUMREGBYTES;
	sparc_reg_defs.register_byte = sparc_register_byte;
	sparc_reg_defs.num_regs = NUMREGS;
	sparc_reg_defs.max_register_raw_size = 4;
	sparc_reg_defs.store_register = sparc_store_register;
	sparc_reg_defs.fetch_register = sparc_fetch_register;
	sparc_reg_defs.endian_flag = Big_endian;
	register_reg_type(&sparc_reg_defs);
}


