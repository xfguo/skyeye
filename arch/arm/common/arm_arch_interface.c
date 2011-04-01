#include "armdefs.h"
#include "armcpu.h"
#include "armemu.h"
#include "arm_regformat.h"

#include "skyeye_internal.h"
#include "skyeye_arch.h"
#include "skyeye_options.h"
#include "skyeye_types.h"
#include "skyeye_signal.h"
#include "skyeye_mm.h"
#include "skyeye_cell.h"
#include "bank_defs.h"
#include "skyeye_log.h"

int debugmode = 0;
//extern int big_endian;
int big_endian = 0;
static cpu_config_t *p_arm_cpu;
static ARMword preset_regfile[16];
extern ARMword skyeye_cachetype;
void arm_core_init (ARMul_State *state, int i);

static void arm_step_once ();
//chy 2005-08-01, borrow from wlm's 2005-07-26's change
ARMword
ARMul_Debug (ARMul_State *state, ARMword pc, ARMword instr)
{
}

void
ARMul_ConsolePrint (ARMul_State *state, const char *format, ...)
{
}
void
ARMul_CallCheck (ARMul_State *state, ARMword cur_pc, ARMword to_pc, ARMword instr)
{
}

static void
base_termios_exit (void)
{
//koodailar remove it for mingw 2005.12.18--------------------------------------                  
#ifndef __MINGW32__
	//tcsetattr (STDIN_FILENO, TCSANOW, &(state->base_termios));
#endif
//end --------------------------------------------------------------------------
}

static per_cpu_step()
{
	arm_step_once();
}

static per_cpu_stop()
{
}

static arm_cpu_init()
{
	ARM_CPU_State *cpu = skyeye_mm(sizeof(ARM_CPU_State));
        machine_config_t *mach = get_current_mach();
        mach->cpu_data = get_conf_obj_by_cast(cpu, "ARM_CPU_State");

	cpu->core_num = 1;
	if(!cpu->core_num){
		fprintf(stderr, "ERROR:you need to set numbers of core in mach_init.\n");
		skyeye_exit(-1);
	}
	else
		cpu->core = skyeye_mm(sizeof(ARMul_State) * cpu->core_num);
	/* TODO: zero the memory by malloc */

	if(!cpu->core){
		fprintf(stderr, "Can not allocate memory for arm core.\n");
		skyeye_exit(-1);
	}
	else
		printf("%d core is initialized.\n", cpu->core_num);

	int i;
	char buf[10];
	for(i = 0; i < cpu->core_num; i++){
		ARMul_State* core = &cpu->core[i];
		arm_core_init(core, i);
		skyeye_exec_t* exec = create_exec();
		exec->priv_data = get_conf_obj_by_cast(core, "ARMul_State");
		exec->run = per_cpu_step;
		exec->stop = per_cpu_stop;
		add_to_default_cell(exec);

		sprintf(buf, "armcore%d", i);
		add_chp_data((void*)core, sizeof(ARMul_State), buf);
	}

	cpu->boot_core_id = 0;
}

static void
arm_reset_state ()
{
	int i;
	ARMul_State *state;

	ARM_CPU_State *cpu = get_current_cpu();
	for(i = 0; i < cpu->core_num; i++){
		state = &cpu->core[i];
		ARMul_Reset (state);
		state->NextInstr = 0;
		state->Emulate = 3;

		/* set some register value */
		if (preset_regfile[1])
			state->Reg[1] = preset_regfile[1];
	}
}

static void
arm_init_state ()
{
	ARMul_EmulateInit ();
	arm_cpu_init();
}

void
arm_core_init (ARMul_State *state, int i)
{
	ARMul_NewState (state);
	state->abort_model = 0;
	state->cpu = p_arm_cpu;
	state->bigendSig = (big_endian ? HIGH : LOW);

	if (!strcmp(p_arm_cpu->cpu_arch_name, "armv3"))
		ARMul_SelectProcessor (state, ARM_v4_Prop);
	if (!strcmp(p_arm_cpu->cpu_arch_name, "armv4"))
		ARMul_SelectProcessor (state, ARM_v4_Prop);
	if (!strcmp(p_arm_cpu->cpu_arch_name, "armv5"))
		ARMul_SelectProcessor (state, ARM_v5_Prop | ARM_v5e_Prop);
	if (!strcmp(p_arm_cpu->cpu_arch_name, "armv6"))
		ARMul_SelectProcessor (state, ARM_v6_Prop);

	if (!strcmp(p_arm_cpu->cpu_name, "arm7tdmi"))
		state->lateabtSig = LOW;
	if (!strcmp(p_arm_cpu->cpu_name, "arm720t")) {
		state->lateabtSig = HIGH;
		state->abort_model = 2;
	}
	if (!strcmp(p_arm_cpu->cpu_name, "arm7tdmis")) {
		ARMul_SelectProcessor(state, ARM_v4_Prop);
		state->lateabtSig = HIGH;
	}
	if (!strcmp(p_arm_cpu->cpu_name, "arm7500fe")) {
		ARMul_SelectProcessor (state, ARM_v4_Prop);
		state->lateabtSig = HIGH;
	}

	if (!strcmp(p_arm_cpu->cpu_name, "sa1100"))
		state->lateabtSig = LOW;
	if (!strcmp(p_arm_cpu->cpu_name, "arm920t"))
		state->lateabtSig = LOW;
	if (!strcmp(p_arm_cpu->cpu_name, "arm926ejs")) {
		/* FIXME:ARM926EJS uses LOW? */
		state->lateabtSig = LOW;
	}

	if (!strcmp(p_arm_cpu->cpu_name, "pxa27x")) {
		ARMul_SelectProcessor (state,
				       ARM_XScale_Prop | ARM_v5_Prop | ARM_v5e_Prop | ARM_PXA27X_Prop);
		//chy 2004-05-09, set lateabtSig
		state->lateabtSig = LOW;
	}
	if (!strcmp(p_arm_cpu->cpu_name, "pxa25x")) {
		ARMul_SelectProcessor (state,
			       ARM_XScale_Prop | ARM_v5_Prop | ARM_v5e_Prop);
		state->lateabtSig = LOW;
	}

	if (!strcmp(p_arm_cpu->cpu_name, "arm11"))
		state->lateabtSig = LOW;

	mmu_init(state);
}

static void
arm_step_once ()
{
	//ARMul_DoInstr(state);
	ARMul_State *state = get_current_core();
	state->step++;
	state->cycle++;
	state->EndCondition = 0;
	state->stop_simulator = 0;
	state->NextInstr = RESUME;      /* treat as PC change */
        state->Reg[15] = ARMul_DoProg(state);
        //state->Reg[15] = ARMul_DoInstr(state);
        FLUSHPIPE;
}
static void
arm_set_pc (WORD pc)
{
	ARMul_State *state = get_current_core();
	state->Reg[15] = pc;
}
static WORD
arm_get_pc(){
	ARMul_State *state = get_current_core();
	return (WORD)state->Reg[15];
}
static int
arm_ICE_write_byte (WORD addr, uint8_t v)
{
	//return (ARMul_ICE_WriteByte (state, (ARMword) addr, (ARMword) v));
	return bus_write(8, addr, v);
}
static int arm_ICE_read_byte (WORD addr, uint8_t *pv){
	int ret;
	//ret = ARMul_ICE_ReadByte (state, (ARMword) addr, &data);
	//printf("In %s,addr=0x%x,data=0x%x\n", __FUNCTION__, addr, data);
	return bus_read(8, addr, pv);
}
extern void lpc_mach_init ();

//chy 2003-08-11: the cpu_id can be found in linux/arch/arm/boot/compressed/head.S
cpu_config_t arm_cpus[] = {
	{"armv3", "arm710", 0x41007100, 0xfff8ff00, DATACACHE},
	{"armv3", "arm7tdmi", 0x41007700, 0xfff8ff00, NONCACHE},
	{"armv4", "arm720t", 0x41807200, 0xffffff00, DATACACHE},
	{"armv4", "sa1110", SA1110, 0xfffffff0, INSTCACHE},
	{"armv4", "sa1100", SA1100, 0xffffffe0, INSTCACHE},
	{"armv4", "arm920t", 0x41009200, 0xff00fff0, INSTCACHE},
	{"armv5", "arm926ejs", 0x41069260, 0xff0ffff0, INSTCACHE},
	{"xscale", "pxa25x", PXA250, 0xfffffff0, INSTCACHE},
	{"xscale", "pxa27x", PXA270, 0xfffffff0, INSTCACHE},
	{"armv6",  "arm11", 0x0007b000, 0x0007f000, NONCACHE}
};


static int
arm_parse_cpu (const char *params[])
{
	int i;
	for (i = 0; i < (sizeof (arm_cpus) / sizeof (cpu_config_t)); i++) {
		if (!strncmp
		    (params[0], arm_cpus[i].cpu_name, MAX_PARAM_NAME)) {

			p_arm_cpu = &arm_cpus[i];
			SKYEYE_INFO("cpu info: %s, %s, %x, %x, %x \n",
				     p_arm_cpu->cpu_arch_name,
				     p_arm_cpu->cpu_name,
				     p_arm_cpu->cpu_val,
				     p_arm_cpu->cpu_mask,
				     p_arm_cpu->cachetype);
			skyeye_cachetype = p_arm_cpu->cachetype;
			return 0;

		}
	}
	SKYEYE_ERR ("Error: Unkonw cpu name \"%s\"\n", params[0]);
	return -1;

}

int
do_cpu_option (skyeye_option_t *this_option, int num_params,
                const char *params[])
{
	int i;
	for (i = 0; i < (sizeof (arm_cpus) / sizeof (cpu_config_t)); i++) {
		if (!strncmp
		    (params[0], arm_cpus[i].cpu_name, MAX_PARAM_NAME)) {

			p_arm_cpu = &arm_cpus[i];
			SKYEYE_INFO ("cpu info: %s, %s, %x, %x, %x \n",
				     p_arm_cpu->cpu_arch_name,
				     p_arm_cpu->cpu_name,
				     p_arm_cpu->cpu_val,
				     p_arm_cpu->cpu_mask,
				     p_arm_cpu->cachetype);
			skyeye_cachetype = p_arm_cpu->cachetype;
			return 0;

		}
	}
	SKYEYE_ERR ("Error: Unkonw cpu name \"%s\"\n", params[0]);
	return -1;
}

machine_config_t arm_machines[] = {
	/* machine define for cpu without mmu */
	{"lpc", lpc_mach_init, NULL, NULL, NULL},		/* PHILIPS LPC2xxxx */
	{NULL, NULL, NULL, NULL, NULL},
};

/*mem bank*/
extern ARMword real_read_word (ARMul_State *state, ARMword addr);
extern void real_write_word (ARMul_State *state, ARMword addr, ARMword data);
extern ARMword io_read_word (ARMul_State *state, ARMword addr);
extern void io_write_word (ARMul_State *state, ARMword addr, ARMword data);

/*ywc 2005-03-30*/
extern ARMword flash_read_byte (ARMul_State *state, ARMword addr);
extern void flash_write_byte (ARMul_State *state, ARMword addr,
			      ARMword data);
extern ARMword flash_read_halfword (ARMul_State *state, ARMword addr);
extern void flash_write_halfword (ARMul_State *state, ARMword addr,
				  ARMword data);
extern ARMword flash_read_word (ARMul_State *state, ARMword addr);
extern void flash_write_word (ARMul_State *state, ARMword addr,
			      ARMword data);
static int
arm_parse_regfile (int num_params, const char *params[]){	
	char name[MAX_PARAM_NAME], value[MAX_PARAM_NAME];
	int i, num;
	int reg_value;
	for (i = 0; i < num_params; i++) {
		if (split_param (params[i], name, value) < 0){
			SKYEYE_ERR
				("Error: mem_bank %d has wrong parameter \"%s\".\n",
				 num, name);
		}
		else if (!strncmp ("r1", name, strlen (name))) {
			//chy 2003-09-21: process type
			if (value[0] == '0' && value[1] == 'x')
        	       		reg_value = strtoul (value, NULL, 16);
	                else
                		reg_value = strtoul (value, NULL, 10);
			preset_regfile[1] = reg_value;
			SKYEYE_INFO("R1 is set to %d.\n", reg_value);
		}
		else {
			SKYEYE_ERR
				("Error: regfile %d has unknow parameter \"%s\".\n",
				 num, name);
		}
	}
	return 0;
}

static uint32 arm_get_step(){
	ARMul_State *state = get_current_core();
	uint32 step = state->NumInstrs;
        return step;
}
static char* arm_get_regname_by_id(int id){
        return arm_regstr[id];
}
static uint32 arm_get_regval_by_id(int id){
	ARMul_State * state = get_current_core();
	if (id == CPSR_REG)
		return state->Cpsr;
	else
        	return state->Reg[id];
}
static uint32 arm_get_regnum(){
	return MAX_REG_NUM;
}
static exception_t arm_set_register_by_id(int id, uint32 value){
	ARMul_State *state = get_current_core();
        state->Reg[id] = value;
        return No_exp;
}

static exception_t arm_signal(interrupt_signal_t *signal){
	ARMul_State *state = get_current_core();
	arm_signal_t *arm_signal = &signal->arm_signal;
	if (arm_signal->irq != Prev_level)
		state->NirqSig = arm_signal->irq;
	if (arm_signal->firq != Prev_level)
		state->NfiqSig = arm_signal->firq;

	/* reset signal in arm dyf add when move sa1100 to soc dir  2010.9.21*/
	if (arm_signal->reset != Prev_level)
		state->NresetSig = arm_signal->reset;
	return No_exp;
}
void
init_arm_arch ()
{
	static arch_config_t arm_arch;
	memset(&arm_arch, '\0', sizeof(arm_arch));

	arm_arch.arch_name = "arm";
	arm_arch.init = arm_init_state;
	arm_arch.reset = arm_reset_state;
	arm_arch.set_pc = arm_set_pc;
	arm_arch.get_pc = arm_get_pc;
	arm_arch.get_step = arm_get_step;
	arm_arch.step_once = arm_step_once;
	arm_arch.ICE_write_byte = arm_ICE_write_byte;
	arm_arch.ICE_read_byte = arm_ICE_read_byte;
	arm_arch.parse_cpu = arm_parse_cpu;
	//arm_arch.parse_mach = arm_parse_mach;
	//arm_arch.parse_mem = arm_parse_mem;
	arm_arch.parse_regfile = arm_parse_regfile;
	arm_arch.get_regval_by_id = arm_get_regval_by_id;
	arm_arch.get_regname_by_id = arm_get_regname_by_id;
	arm_arch.set_regval_by_id = arm_set_register_by_id;
	arm_arch.get_regnum = arm_get_regnum;
	arm_arch.signal = arm_signal;

	register_arch (&arm_arch);
}
