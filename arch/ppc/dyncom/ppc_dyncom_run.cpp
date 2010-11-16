/*
 * The interface of dynamic compiled mode for ppc simulation
 *
 * 08/22/2010 Michael.Kang (blackfin.kang@gmail.com)
 */
#include <skyeye_dyncom.h>
#include <skyeye_types.h>
#include "ppc_dyncom.h"

/* physical register for powerpc archtecture */
static void arch_powerpc_init(cpu_t *cpu, cpu_archinfo_t *info, cpu_archrf_t *rf)
{
	// Basic Information
	info->name = "powerpc";
	info->full_name = "powerpc_common";

	// This architecture is biendian, accept whatever the
	// client wants, override other flags.
	info->common_flags &= CPU_FLAG_ENDIAN_MASK;

	info->delay_slots = 0;
	// The byte size is 8bits.
	// The word size is 32bits.
	// The float size is 64bits.
	// The address size is 32bits.
	info->byte_size = 8;
	info->word_size = 32;
	info->float_size = 64;
	info->address_size = 32;
	// There are 16 32-bit GPRs
	info->register_count[CPU_REG_GPR] = 32;
	info->register_size[CPU_REG_GPR] = info->word_size;
	// There is also 1 extra register to handle PSR.
	info->register_count[CPU_REG_XR] = 0;
	info->register_size[CPU_REG_XR] = 32;
	cpu->redirection = false;
	/* Initilize different register set for different core */
	//cpu->rf.pc = gCPU.core[cpu->id]->pc;
	//cpu->rf.grf = gCPU.core[cpu->id]->gpr;
}

static void
arch_powerpc_done(cpu_t *cpu)
{
	free(cpu->rf.grf);
}

static addr_t
arch_powerpc_get_pc(cpu_t *, void *reg)
{
	return ((reg_powerpc_t *)reg)->pc;
}

static uint64_t
arch_powerpc_get_psr(cpu_t *, void *reg)
{
	return 0;
}

static int
arch_powerpc_get_reg(cpu_t *cpu, void *reg, unsigned reg_no, uint64_t *value)
{
	return (0);
}

static arch_func_t arch_func_powerpc = {
	arch_powerpc_init,
	arch_powerpc_done,
	arch_powerpc_get_pc,
	NULL,
	NULL,
	arch_powerpc_tag_instr,
	arch_powerpc_disasm_instr,
	arch_powerpc_translate_cond,
	arch_powerpc_translate_instr,
        arch_powerpc_translate_loop_helper,
	// idbg support
	arch_powerpc_get_psr,
	arch_powerpc_get_reg,
	NULL
};

void ppc_dyncom_init(){
}

void ppc_dyncom_run(){
}

void ppc_dyncom_stop(){
}

void ppc_dyncom_fini(){
}
