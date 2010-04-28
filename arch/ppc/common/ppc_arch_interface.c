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
#include "ppc_regformat.h"

#include "skyeye_types.h"
#include "skyeye_config.h"

#ifdef __CYGWIN__
#include <sys/time.h>
#endif

PPC_CPU_State gCPU;

int ppc_divisor = 0;

static void
ppc_reset_state ()
{
	//skyeye_config_t* config = get_current_config();
	//config->mach->mach_io_reset(&gCPU);
	//skyeye_config.mach->mach_io_reset(&gCPU);/* set all the default value for register */	
}

byte * init_ram; /* FIXME: 16k init ram for 8560, will be replaced by memory module */
byte * boot_rom; /* FIXME : default 8M bootrom for 8560, will be replaced by memory module */
byte * ddr_ram; /* FIXME: 64M DDR SDRAM, will be replaced by memory module */

unsigned long init_ram_start_addr, init_ram_size;
uint32 boot_romSize;
uint32 boot_rom_start_addr;

FILE * prof_file;


static bool ppc_cpu_init()
{
	skyeye_config_t* config = get_current_config();
	machine_config_t *mach = config->mach;
	if(!strcmp(mach->machine_name, "mpc8560")){
		gCPU.core_num = 1;
	}
	else if(!strcmp(mach->machine_name, "mpc8572"))
		gCPU.core_num = 2;
	else if(!strcmp(mach->machine_name, "mpc8641d"))
                gCPU.core_num = 2;
        else
		gCPU.core_num = 0;

	if(!gCPU.core_num){
		fprintf(stderr, "ERROR:you need to set numbers of core in mach_init.\n");
		skyeye_exit(-1);
	}
	else
		gCPU.core = malloc(sizeof(e500_core_t) * gCPU.core_num);
	/* TODO: zero the memory by malloc */

	if(!gCPU.core){
		fprintf(stderr, "Can not allocate memory for ppc core.\n");
		skyeye_exit(-1);
	}
	else
		printf("%d core is initialized.\n", gCPU.core_num);
	
	int i;
	for(i = 0; i < gCPU.core_num; i++){
		ppc_core_init(&gCPU.core[i], i);
	}

	current_core = &gCPU.core[0];
	/* initialize decoder */
	ppc_dec_init();
	return true;
}
static void
ppc_init_state ()
{
	/* initial phsical memory to DEFAULT_GMEMORY_SIZE */
	if(!(boot_rom = malloc(DEFAULT_BOOTROM_SIZE))){
		fprintf(stderr, "can not initialize physical memory...\n");
		skyeye_exit(-1);
	}
	/*we set start_addr */
	boot_rom_start_addr = 0xFFFFFFFF - DEFAULT_BOOTROM_SIZE + 1;
	boot_romSize = DEFAULT_BOOTROM_SIZE;

	/* initialize init_ram parameters */
	if(!(init_ram = malloc(INIT_RAM_SIZE))){
		fprintf(stderr, "malloc failed!\n");
		skyeye_exit(-1);
	}
#if 1
	if(!(ddr_ram = malloc(DDR_RAM_SIZE))){
		fprintf(stderr, "malloc failed!\n");
                skyeye_exit(-1);

	}
#endif
	init_ram_size = INIT_RAM_SIZE;
	init_ram_start_addr = 0xe4010000;

	memset(&gCPU, 0, sizeof gCPU);
	//skyeye_config.mach->mach_init(&gCPU, skyeye_config.mach);	

	ppc_cpu_init();
	/* write something to a file for debug or profiling */
	if (!prof_file) {
                prof_file = fopen ("./kernel_prof.txt", "w");
        }

	/* Before boot linux, we need to do some preparation */
	//ppc_boot();
}

static void debug_log(e500_core_t * core){
	static uint32_t dbg_start = 0xc0000000;
        static uint32_t dbg_end = 0xfff83254;
        static int flag = 0;
	/*
	if(core->pc == dbg_start)
		flag = 0;
	*/
	if(core->pc > dbg_start)
		flag = 0;

	/*
	if(flag)
		printf("In %s,pc=0x%x\n", __FUNCTION__, core->pc);
	*/
	if(flag){
		//if(core->pc >= 0xC0000000)	
		fprintf(prof_file,"DBG:before r4=0x%x,r3=0x%x,r5=0x%x,pc=0x%x, npc=0x%x, &npc=0x%x, pir=0x%x\n", core->gpr[4], core->gpr[3], core->gpr[5], core->pc, core->npc, &core->npc, core->pir);
		if(core->pir == 1)
			fprintf(prof_file,"DBG:before r4=0x%x,r3=0x%x,r5=0x%x,pc=0x%x, npc=0x%x, &npc=0x%x, pir=0x%x\n", core->gpr[4], core->gpr[3], core->gpr[5], core->pc, core->npc, &core->npc, core->pir);
	}
	if(flag){
                //fprintf(prof_file,"DBG:before pc=0x%x,r0=0x%x,r3=0x%x,r31=0x%x,ddr_ram[0xc21701e4 + 48]=0x%x\n", gCPU.pc, gCPU.gpr[0], gCPU.gpr[3], gCPU.gpr[31], *(int *)&ddr_ram[0x21701e4 + 48]);
                        //fprintf(prof_file,"DBG:before pc=0x%x,r0=0x%x,r1=0x%x,r3=0x%x,r4=0x%x,r5=0x%x,r8=0x%x,r30=0x%x, r31=0x%x, lr=0x%x\n", gCPU.pc, gCPU.gpr[0], gCPU.gpr[1], gCPU.gpr[3], gCPU.gpr[4], gCPU.gpr[5], gCPU.gpr[8], gCPU.gpr[30], gCPU.gpr[31], gCPU.lr);
                //if(core->pc >= 0xC0000000)
		/*
                if(core->pir)
                	fprintf(prof_file,"DBG 0:pc=0x%x, npc=0x%x, &npc=0x%x, pir=0x%x\n", core->pc, core->npc, &core->npc, core->pir);
		*/
        }
	/*if (skyeye_config.log.logon >= 1 && !core->pir)
		skyeye_log(core->pc);
	*/
}

static void per_cpu_step(e500_core_t * running_core){
	uint32 real_addr;
	e500_core_t *core = running_core;

	/* sometimes, core->npc will changed by another core */
	if(core->ipi_flag){
		core->pc = core->npc;
		core->ipi_flag = 0;
	}
	core->step++;
	core->npc = core->pc + 4;

	switch(	ppc_effective_to_physical(core, core->pc, 0, &real_addr))
	{
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
   			fprintf(stderr, "Something wrong during address translation at 0x%x\n", core->pc);
             		 skyeye_exit(-1);
	
	};
#if 0
	uint32 instr;
	if(bus_read(32, real_addr, &instr) != 0){
		/* some error handler */
	}
	core->current_opc = ppc_word_from_BE(instr);
#else
	if(real_addr > boot_rom_start_addr)
		core->current_opc = ppc_word_from_BE(*((int *)&boot_rom[real_addr - boot_rom_start_addr]));
	else if(real_addr >=0 && real_addr < DDR_RAM_SIZE)
		core->current_opc = ppc_word_from_BE(*((int *)&ddr_ram[real_addr]));  	
	else{
		fprintf(stderr,"Can not get instruction from addr 0x%x\n",real_addr);
		skyeye_exit(-1);
	}
#endif
	ppc_exec_opc(core);
	//debug_log(core);	
exec_npc:
	if(!ppc_divisor){
		dec_io_do_cycle(core);
		ppc_divisor = 0;
	}
	else
		ppc_divisor--;
	//core->pc = core->npc;
	core->pc = gCPU.core[core->pir].npc;
	core->pc = core->npc;
}

/* Fixme later */
e500_core_t * current_core;

static void
ppc_step_once ()
{
	int i;
	/* workaround boot sequence for dual core, we need the first core initialize some variable for second core. */

	for(i = 0; i < gCPU.core_num; i++ ){
		current_core = &gCPU.core[i];
		/* if CPU1_EN is set? */
		if(!i || gCPU.eebpcr & 0x2000000)
			per_cpu_step(current_core);
	}
	/* for peripheral */
	skyeye_config_t* config = get_current_config();
	config->mach->mach_io_do_cycle(&gCPU);
	//skyeye_config.mach->mach_io_do_cycle(&gCPU);
}

static void
ppc_set_pc (WORD pc)
{
	int i;
        for(i = 0; i < gCPU.core_num; i++ )
		gCPU.core[i].pc = pc;
	/* Fixme, for e500 core, the first instruction should be executed at 0xFFFFFFFC */
	//gCPU.pc = 0xFFFFFFFC;
}
static WORD
ppc_get_pc(int core_id){
	return gCPU.core[0].pc;
}
/*
 * Since mmu of ppc always enabled, so we can write virtual address here
 */
static int
ppc_ICE_write_byte (WORD addr, uint8_t v)
{
	ppc_write_effective_byte(addr, v);

	/* if failed, return -1*/
	return 0;
}

/*
 * Since mmu of ppc always enabled, so we can read virtual address here
 */
static int ppc_ICE_read_byte (WORD addr, uint8_t *pv){
	/**
	 *  work around for ppc debugger
	 */
	if ((addr & 0xFFFFF000) == 0xBFFFF000)
		return 0;

	ppc_read_effective_byte(addr, pv);
	return 0;
}

static int
ppc_parse_cpu (const char *params[])
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
	{NULL,	NULL,			NULL,NULL, NULL},
};
#if 0
static int
ppc_parse_mach (machine_config_t * mach, const char *params[])
{	
	int i;
	for (i = 0; i < (sizeof (ppc_machines) / sizeof (machine_config_t));
	     i++) {
		if (!strncmp
		    (params[0], ppc_machines[i].machine_name,
		     MAX_PARAM_NAME)) {
			skyeye_config.mach = &ppc_machines[i];
			SKYEYE_INFO
				("mach info: name %s, mach_init addr %p\n",
				 skyeye_config.mach->machine_name,
				 skyeye_config.mach->mach_init);
			return 0;
		}
	}
	SKYEYE_ERR ("Error: Unkonw mach name \"%s\"\n", params[0]);

	return -1;
}
#endif
static uint32 ppc_get_step(){
        uint32 step = current_core->step;
        return step;
}
static char* ppc_get_regname_by_id(int id){
        return ppc_regstr[id];
        //return NULL;
}
static uint32 ppc_get_regval_by_id(int id){
        if(id == PC)
                return current_core->pc;
        return current_core->gpr[id];
}

static exception_t ppc_mmu_read(short size, generic_address_t addr, uint32_t * value){
	uint32 result;
	switch(size){
                case 8:
                        ppc_read_effective_byte (addr, &result);
			*(uint8_t *)value = (uint8_t)result;
                        break;
                case 16:
			ppc_read_effective_half(addr, &result);
                        *(uint16_t *)value = (uint16_t)result;
                        break;
                case 32:
			ppc_read_effective_word(addr, &result);
                        *value = result;
                        break;
                default:
                        fprintf(stderr, "In %s, invalid data length %d\n", __FUNCTION__, size);
                        return Invarg_exp;
        }
	return No_exp;
}

static exception_t ppc_mmu_write(short size, generic_address_t addr, uint32_t value){
	switch(size){
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
			fprintf(stderr, "In %s, invalid data length %d\n", __FUNCTION__, size);
			return Invarg_exp;
	}
	return No_exp;
}
void
init_ppc_arch ()
{

	static arch_config_t ppc_arch;

	ppc_arch.arch_name = "ppc";
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
	ppc_arch.mmu_read = ppc_mmu_read;
	ppc_arch.mmu_write = ppc_mmu_write;
	//ppc_arch.parse_mach = ppc_parse_mach;

	register_arch (&ppc_arch);
}

void print_ppc_arg(FILE * log){
	if(log)
		fprintf(log, "r3=0x%x,r4=0x%x,r5=0x%x\n", current_core->gpr[3], current_core->gpr[4], current_core->gpr[5]);
}
