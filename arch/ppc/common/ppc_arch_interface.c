/* 
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/*
 * 12/06/2007   Michael.Kang  <blackfin.kang@gmail.com>
 */
#include <string.h>

#include "ppc_cpu.h"
#include "ppc_mmu.h"
#include "ppc_exc.h"
#include "ppc_e500_exc.h"
#include "ppc_memory.h"
#include "ppc_io.h"
#include "types.h"
#include "tracers.h"
#include "sysendian.h"
#include "ppc_irq.h"
#include "ppc_dec.h"
#include "ppc_regformat.h"
#include "bank_defs.h"

#include "skyeye_types.h"
#include "skyeye_config.h"
#include "skyeye_callback.h"
#include "skyeye_arch.h"
#include "skyeye.h"
#include "skyeye_exec.h"
#include "skyeye_mm.h"
#include "skyeye_cell.h"

#ifdef __CYGWIN__
#include <sys/time.h>
#endif
const char *ppc_regstr[] = {
	"R0",
	"R1",
	"R2",
	"R3",
	"R4",
	"R5",
	"R6",
	"R7",
	"R8",
	"R9",
	"R10",
	"R11",
	"R12",
	"R13",
	"R14",
	"R15",
	"R16",
	"R17",
	"R18",
	"R19",
	"R20",
	"R21",
	"R22",
	"R23",
	"R24",
	"R25",
	"R26",
	"R27",
	"R28",
	"R29",
	"R30",
	"R31",
	"PC",
	NULL
};

int ppc_divisor = 0;

static void per_cpu_step(conf_object_t * running_core);
static void per_cpu_stop(conf_object_t * running_core);
static void ppc_reset_state()
{
}

uint8 *init_ram;		/* FIXME: 16k init ram for 8560, will be replaced by memory module */
uint8 *boot_rom;		/* FIXME : default 8M bootrom for 8560, will be replaced by memory module */

unsigned long init_ram_start_addr, init_ram_size;
uint32 boot_romSize;
uint32 boot_rom_start_addr;

FILE *prof_file;

static bool ppc_cpu_init()
{
	PPC_CPU_State *cpu = skyeye_mm_zero(sizeof(PPC_CPU_State));
	machine_config_t *mach = get_current_mach();
	mach->cpu_data = get_conf_obj_by_cast(cpu, "PPC_CPU_State");
	if (!mach->cpu_data)
		return false;
	if (!strcmp(mach->machine_name, "mpc8560")) {
		cpu->core_num = 1;
	} else if (!strcmp(mach->machine_name, "mpc8572"))
		cpu->core_num = 2;
	else if (!strcmp(mach->machine_name, "mpc8641d"))
		cpu->core_num = 2;
	else
		cpu->core_num = 0;

	if (!cpu->core_num) {
		fprintf(stderr,
			"ERROR:you need to set numbers of core in mach_init.\n");
		skyeye_exit(-1);
	} else
		cpu->core = skyeye_mm_zero(sizeof(e500_core_t) * cpu->core_num);
	/* TODO: zero the memory by malloc */

	if (!cpu->core) {
		fprintf(stderr, "Can not allocate memory for ppc core.\n");
		skyeye_exit(-1);
	} else
		printf("%d core is initialized.\n", cpu->core_num);

	int i;
	for (i = 0; i < cpu->core_num; i++) {
		e500_core_t *core = &cpu->core[i];
		ppc_core_init(core, i);
		skyeye_exec_t *exec = create_exec();
		exec->priv_data = get_conf_obj_by_cast(core, "e500_core_t");
		exec->run = per_cpu_step;
		exec->stop = per_cpu_stop;
		skyeye_cell_t* cell = create_cell();
		add_to_cell(exec, cell);

		//add_to_default_cell(exec);
	}

	cpu->boot_core_id = 0;
	/* initialize decoder */
	ppc_dec_init(&cpu->core[cpu->boot_core_id]);
	return true;
}

static void ppc_init_state()
{
#if 0
	/* initial phsical memory to DEFAULT_GMEMORY_SIZE */
	if (!(boot_rom = malloc(DEFAULT_BOOTROM_SIZE))) {
		fprintf(stderr, "can not initialize physical memory...\n");
		skyeye_exit(-1);
	}
	/*we set start_addr */
	boot_rom_start_addr = 0xFFFFFFFF - DEFAULT_BOOTROM_SIZE + 1;
	boot_romSize = DEFAULT_BOOTROM_SIZE;

	/* initialize init_ram parameters */
	if (!(init_ram = malloc(INIT_RAM_SIZE))) {
		fprintf(stderr, "malloc failed!\n");
		skyeye_exit(-1);
	}
	init_ram_size = INIT_RAM_SIZE;
	init_ram_start_addr = 0xe4010000;
#endif
	if (ppc_cpu_init() == false) {
		fprintf(stderr, "cpu initialization failed.\n");
	}

	/* initialize the alignment and endianess for powerpc */
	generic_arch_t *arch_instance = get_arch_instance(NULL);
	arch_instance->alignment = UnAlign;
	arch_instance->endianess = Big_endian;

	/* Before boot linux, we need to do some preparation */
	//ppc_boot();
}

static void per_cpu_step(conf_object_t * running_core)
{
	uint32 real_addr;
	e500_core_t *core =
	    (e500_core_t *) get_cast_conf_obj(running_core, "e500_core_t");
	PPC_CPU_State *cpu = get_current_cpu();
	/* Check the second core and boot flags */
	if (core->pir) {
		if (!(cpu->eebpcr & 0x2000000))
			return;
	}
	/* sometimes, core->npc will be changed by another core */
	if (core->ipi_flag) {
		if(core->ipr & IPI0){
			//printf("In %s, ipi exception\n", __FUNCTION__);
			pthread_spin_lock(&(core->ipr_spinlock));
 			core->ipi_flag = 0;
			core->ipr &= ~IPI0;
			pthread_spin_unlock(&(core->ipr_spinlock));

 			ppc_exception (core, EXT_INT, 0, 0);
		}
		else if(core->ipr & UART0){
			//printf("In %s, uart exception\n", __FUNCTION__);
			pthread_spin_lock(&(core->ipr_spinlock));
 			core->ipi_flag = 0;
			core->ipr &= ~UART0;
			pthread_spin_unlock(&(core->ipr_spinlock));

			ppc_exception (core, EXT_INT, 0, 0);
		}
 		core->pc = core->npc;
	}
	/* check if we need to run some callback functions at this time */
	if (!core->pir) {
		generic_arch_t *arch_instance = get_arch_instance("");
		exec_callback(Step_callback, arch_instance);
	}
	core->step++;
	core->npc = core->pc + 4;
	switch (ppc_effective_to_physical
		(core, core->pc, PPC_MMU_CODE, &real_addr)) {
	case PPC_MMU_OK:
		break;
		/* we had TLB miss and need to jump to its handler */
	case PPC_MMU_EXC:
		goto exec_npc;
	case PPC_MMU_FATAL:
		/* TLB miss */
		fprintf(stderr, "TLB missed at 0x%x\n", core->pc);
		skyeye_exit(-1);
	default:
		/* TLB miss */
		fprintf(stderr,
			"Something wrong during address translation at 0x%x\n",
			core->pc);
		skyeye_exit(-1);
	};
	uint32 instr;
	if (bus_read(32, real_addr, &instr) != 0) {
		/* some error handler */
	}
	core->current_opc = instr;
	ppc_exec_opc(core);
	//debug_log(core);      
exec_npc:
	if (!ppc_divisor) {
		core->dec_io_do_cycle(core);
		ppc_divisor = 0;
	} else
		ppc_divisor--;
	core->pc = core->npc;
}

static void per_cpu_stop(conf_object_t * running_core)
{
	e500_core_t *core =
	    (e500_core_t *) get_cast_conf_obj(running_core, "e500_core_t");
}

static void ppc_step_once()
{
	int i;
	PPC_CPU_State *cpu = get_current_cpu();
	/* workaround boot sequence for dual core,
	 * we need the first core initialize some variable for second core. */
	for (i = 0; i < cpu->core_num; i++) {
		/* if CPU1_EN is set? */
		if (!i || cpu->eebpcr & 0x2000000)
			per_cpu_step(&cpu->core[i]);
	}
	/* for peripheral */
	skyeye_config_t *config = get_current_config();
	config->mach->mach_io_do_cycle(cpu);
}

static void ppc_set_pc(generic_address_t pc)
{
	PPC_CPU_State *cpu = get_current_cpu();
	int i;
	
	cpu->core[cpu->boot_core_id].pc = pc;
	/* Fixme, for e500 core, the first instruction should be executed at 0xFFFFFFFC */
	//gCPU.pc = 0xFFFFFFFC;
}

static generic_address_t ppc_get_pc()
{
	PPC_CPU_State *cpu = get_current_cpu();
	return cpu->core[0].pc;
}

/*
 * Since mmu of ppc always enabled, so we can write virtual address here
 */
static int ppc_ICE_write_byte(generic_address_t addr, uint8_t v)
{
	ppc_write_effective_byte(addr, v);

	/* if failed, return -1 */
	return 0;
}

/*
 * Since mmu of ppc always enabled, so we can read virtual address here
 */
static int ppc_ICE_read_byte(generic_address_t addr, uint8_t * pv)
{
	/**
	 *  work around for ppc debugger
	 */
	if ((addr & 0xFFFFF000) == 0xBFFFF000)
		return 0;
	ppc_read_effective_byte(addr, pv);
	return 0;
}

static int ppc_parse_cpu(const char *params[])
{
	return 0;
}

extern void mpc8560_mach_init();
extern void mpc8572_mach_init();
extern void mpc8641d_mach_init();
machine_config_t ppc_machines[] = {
	/* machine define for MPC8560 */
	{"mpc8560", mpc8560_mach_init, NULL, NULL, NULL},
	{"mpc8572", mpc8572_mach_init, NULL, NULL, NULL},
	{"mpc8641d", mpc8641d_mach_init, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL},
};

static uint32 ppc_get_step()
{
	PPC_CPU_State *cpu = get_current_cpu();
	return cpu->core[0].step;
}

static uint32 ppc_get_regnum()
{
	return PPC_MAX_REGNUM;
}

static char *ppc_get_regname_by_id(int id)
{
	return ppc_regstr[id];
	//return NULL;
}

static uint32 ppc_get_regval_by_id(int id)
{
	/* we return the reg value of core 0 by default */
	int core_id = 0;
	PPC_CPU_State *cpu = get_current_cpu();
	if (id >= 0 && id < 32)
		return cpu->core[0].gpr[id];
	switch (id) {
	case PC_REG:
		return cpu->core[core_id].pc;
	case MSR:
		return cpu->core[core_id].msr;
	case CR_REG:
		return cpu->core[core_id].cr;
	case LR_REG:
		return cpu->core[core_id].lr;
	case CTR_REG:
		return cpu->core[core_id].ctr;
	case XER_REG:
		return cpu->core[core_id].xer;
	case FPSCR:
		return cpu->core[core_id].fpscr;
	default:
		/* can not find any corrsponding register */
		return 0;
	}
}

static exception_t ppc_mmu_read(short size, generic_address_t addr,
				uint32_t * value)
{
	uint32 result;
	/*
	 * work around for ppc gdb remote debugger
	 */
	if ((addr & 0xFFFFF000) == 0xBFFFF000)
		return 0;
	switch (size) {
	case 8:
		ppc_read_effective_byte(addr, &result);
		*(uint8_t *) value = (uint8_t) result;
		break;
	case 16:
		ppc_read_effective_half(addr, &result);
		*(uint16_t *) value = (uint16_t) result;
		break;
	case 32:
		ppc_read_effective_word(addr, &result);
		*value = result;
		break;
	default:
		fprintf(stderr, "In %s, invalid data length %d\n", __FUNCTION__,
			size);
		return Invarg_exp;
	}
	return No_exp;
}

static exception_t ppc_mmu_write(short size, generic_address_t addr,
				 uint32_t value)
{
	switch (size) {
	case 8:
		//mem_write_byte (offset, value);
		ppc_write_effective_byte(addr, value);
		break;
	case 16:
		//mem_write_halfword(offset, value);
		ppc_write_effective_half(addr, value);
		break;
	case 32:
		ppc_write_effective_word(addr, value);
		//mem_write_word(offset, value);
		break;
	default:
		fprintf(stderr, "In %s, invalid data length %d\n", __FUNCTION__,
			size);
		return Invarg_exp;
	}
	return No_exp;
}

void init_ppc_arch()
{
	static arch_config_t ppc_arch;
	ppc_arch.arch_name = "powerpc";
	ppc_arch.init = ppc_init_state;
	ppc_arch.reset = ppc_reset_state;
	ppc_arch.set_pc = ppc_set_pc;
	ppc_arch.get_pc = ppc_get_pc;
	ppc_arch.get_step = ppc_get_step;
	ppc_arch.step_once = ppc_step_once;
	ppc_arch.ICE_write_byte = ppc_ICE_write_byte;
	ppc_arch.ICE_read_byte = ppc_ICE_read_byte;
	ppc_arch.parse_cpu = ppc_parse_cpu;
	ppc_arch.get_regval_by_id = ppc_get_regval_by_id;
	ppc_arch.get_regname_by_id = ppc_get_regname_by_id;
	ppc_arch.get_regnum = ppc_get_regnum;
	ppc_arch.mmu_read = ppc_mmu_read;
	ppc_arch.mmu_write = ppc_mmu_write;
	//ppc_arch.parse_mach = ppc_parse_mach;
	register_arch(&ppc_arch);
}

void print_ppc_arg(FILE * log)
{
	e500_core_t *current_core = get_current_core();
	if (log)
		fprintf(log, "r3=0x%x,r4=0x%x,r5=0x%x\n", current_core->gpr[3],
			current_core->gpr[4], current_core->gpr[5]);
}

static void debug_log(e500_core_t * core)
{
	static uint32_t dbg_start = 0xc0000000;
	static uint32_t dbg_end = 0xfff83254;
	static int flag = 0;
	/*
	   if(core->pc == dbg_start)
	   flag = 0;
	 */
	if (core->pc > dbg_start)
		flag = 0;
	/*
	   if(flag)
	   printf("In %s,pc=0x%x\n", __FUNCTION__, core->pc);
	 */
	if (flag) {
		//if(core->pc >= 0xC0000000)    
		fprintf(prof_file,
			"DBG:before r4=0x%x,r3=0x%x,r5=0x%x,pc=0x%x, npc=0x%x, &npc=0x%x, pir=0x%x\n",
			core->gpr[4], core->gpr[3], core->gpr[5], core->pc,
			core->npc, &core->npc, core->pir);
		if (core->pir == 1)
			fprintf(prof_file,
				"DBG:before r4=0x%x,r3=0x%x,r5=0x%x,pc=0x%x, npc=0x%x, &npc=0x%x, pir=0x%x\n",
				core->gpr[4], core->gpr[3], core->gpr[5],
				core->pc, core->npc, &core->npc, core->pir);
	}
	if (flag) {
		/*
		if(core->pc >= 0xC0000000)
			if(core->pir)
				fprintf(prof_file,"DBG 0:pc=0x%x, npc=0x%x, &npc=0x%x, pir=0x%x\n",
				core->pc, core->npc, &core->npc, core->pir);
		 */
	}
	/*if (skyeye_config.log.logon >= 1 && !core->pir)
	   skyeye_log(core->pc);
	 */
}
