#include "libcpu.h"
#include "mips_internal.h"
#include "mips_interface.h"
#include "frontend.h"

static uint32_t arch_mips_read_memory(cpu_t *cpu, addr_t addr, uint32_t size)
{

}

static uint32_t arch_mips_write_memory(cpu_t *cpu, addr_t addr, uint32_t value, uint32_t size)
{

}

static void cpu_set_flags_codegen(cpu_t *cpu, uint32_t f)
{
	cpu->dyncom_engine->flags_codegen = f;
}

static void
arch_mips_init(cpu_t *cpu, cpu_archinfo_t *info, cpu_archrf_t *rf)
{
	// Basic Information
	info->name = "mips";
	//info->full_name = "MIPS R4000";
	info->full_name = "mips_dyncom";

	// This architecture is biendian, accept whatever the
	// client wants, override other flags.
	info->common_flags &= CPU_FLAG_ENDIAN_MASK;
	// Both r0 and x0 are hardwired to zero.
	info->common_flags |= CPU_FLAG_HARDWIRE_GPR0;
	info->common_flags |= CPU_FLAG_HARDWIRE_FPR0;
	// The byte size is 8bits.
	// The float size is 64bits.
	info->byte_size = 8;
	info->float_size = 80;
	if (info->arch_flags & CPU_MIPS_IS_64BIT) {
		// The word size is 64bits.
		// The address size is 64bits.
		info->word_size = 64;
		info->address_size = 64; //XXX actually it's 32!
	} else {
		// The word size is 32bits.
		// The address size is 32bits.
		info->word_size = 32;
		info->address_size = 32;
	}
	// Page size is 4K or 16M
	info->min_page_size = 4096;
	info->max_page_size = 16777216;
	info->default_page_size = 4096;
	// There are 32 32-bit GPRs
	info->register_count[CPU_REG_GPR] = 32;
	info->register_size[CPU_REG_GPR] = info->word_size;
	// There are 2 extra registers, HI/LO for MUL/DIV insn.
	info->register_count[CPU_REG_XR] = 2;
	info->register_size[CPU_REG_XR] = info->word_size;

	if (info->arch_flags & CPU_MIPS_IS_64BIT) {
		reg_mips64_t *reg;
		reg = (reg_mips64_t*)malloc(sizeof(reg_mips64_t));
		for (int i=0; i<32; i++)
			reg->r[i] = 0;
		reg->pc = 0;

		cpu->rf.pc = &reg->pc;
		cpu->rf.grf = reg;
	} else {
#if 0
		reg_mips32_t *reg;
		reg = (reg_mips32_t*)malloc(sizeof(reg_mips32_t));
		for (int i=0; i<32; i++)
			reg->r[i] = 0;
		reg->pc = 0;

		cpu->rf.pc = &reg->pc;
		cpu->rf.grf = reg;
#endif
	}

	cpu->redirection = false;

	//debug
	cpu_set_flags_debug(cpu, 0
		| CPU_DEBUG_PRINT_IR
		| COU_DEBUG_LOG
	);
	cpu_set_flags_codegen(cpu, CPU_CODEGEN_TAG_LIMIT);

	set_memory_operator(arch_mips_read_memory, arch_mips_write_memory);
	LOG("%d bit MIPS initialized.\n", info->word_size);
}

static void
arch_mips_done(cpu_t *cpu)
{
	free(cpu->rf.grf);
}

static addr_t
arch_mips_get_pc(cpu_t *cpu, void *reg)
{
#if 0
	unsigned int *grf = (unsigned int *)reg;
	return grf[15];
#endif
}

static uint64_t
arch_mips_get_psr(cpu_t *, void *)
{
	return 0;
}

static int
arch_mips_get_reg(cpu_t *cpu, void *reg, unsigned reg_no, uint64_t *value)
{
	return (0);
}

arch_func_t arch_func_mips = {
	arch_mips_init,
	arch_mips_done,
	arch_mips_get_pc,
	NULL, /* emit_decode_reg */
	NULL, /* spill_reg_state */
	arch_mips_tag_instr,
	arch_mips_disasm_instr,
	arch_mips_translate_cond,
	arch_mips_translate_instr,
	// idbg support
	arch_mips_get_psr,
	arch_mips_get_reg,
	NULL
};

static void mips_debug_func(cpu_t* cpu)
{

}

void mips_dyncom_init(mips_core_t *core)
{
	cpu_t* cpu = cpu_new(0, 0, arch_func_mips);

	/* init the reg structure */
	cpu->rf.pc = &core->pc;
	cpu->rf.phys_pc = &core->pc;
	cpu->rf.grf = core->gpr;
	cpu->rf.frf = core->fpr;
	cpu->rf.srf = core->cp0;

	cpu->cpu_data = (conf_object_t*)core;
	core->dyncom_cpu = get_conf_obj_by_cast(cpu, "cpu_t");
	cpu->debug_func = mips_debug_func;

	sky_pref_t *pref = get_skyeye_pref();
	if(pref->user_mode_sim){
		cpu->syscall_func = arm_dyncom_syscall;
	}
	else
		cpu->syscall_func = NULL;

	cpu->dyncom_engine->code_start = 0x0;
	cpu->dyncom_engine->code_end = 0x100000;
	cpu->dyncom_engine->code_entry = 0x0;

	return;
}

void mips_dyncom_run(cpu_t *cpu)
{
	mips_core_t *core = (mips_core_t*)cpu->cpu_data;
	addr_t phys_pc = core->pc;

	int rc = cpu_run(cpu);
	switch (rc) {
		case JIT_RETURN_NOERR:
			break;
		case JIT_RETURN_SINGLESTEP:
		case JIT_RETURN_FUNCNOTFOUND:
			cpu_tag(cpu, core->Reg[15]);
			cpu->dyncom_engine->functions = 0;
			cpu_translate(cpu);

		/*
		 *	IF singlestep, we run it here, otherwise, break.
		 */
			if(cpu->dyncom_engine->flags_debug & CPU_DEBUG_SINGLESTEP)
			{
				rc = cpu_run(cpu);
				if(rc != JIT_RETURN_TRAP)
					break;
			}
			else
				break;
		case JIT_RETURN_TRAP:
			break;

		default:
			fprintf(stderr, "unknown return code:%d\n", rc);
			skyeye_exit(-1);
	}
	return;
}

void mips_dyncom_stop()
{}

void mips_dyncom_fini()
{}
