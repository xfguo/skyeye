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
#include "mmu/tlb.h"
#include "mmu/cache.h"


const char* arm_regstr[] = {
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
	"Cpsr",
	"PHYS_PC",
	"R13_SVC",
	"R14_SVC",
	"R13_ABORT",
	"R14_ABORT",
	"R13_UNDEF",
	"R14_UNDEF",
	"R13_IRQ",
	"R14_IRQ",
	"R8_FIRQ",
	"R9_FIRQ",
	"R10_FIRQ",
	"R11_FIRQ",
	"R12_FIRQ",
	"R13_FIRQ",
	"R14_FIRQ",
	"SPSR_INVALID1",
	"SPSR_INVALID2",
	"SPSR_SVC",
	"SPSR_ABORT",
	"SPSR_UNDEF",
	"SPSR_IRQ",
	"SPSR_FIRQ"

};

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

static register_arm_core_chp(ARMul_State* core, int num)
{
	int j,k,l;
	char buf[100];
	sprintf(buf, "armregs%d", num);
	add_chp_data((void*)(core->Reg),sizeof(core->Reg), buf);
	sprintf(buf, "armregbank%d", num);
	add_chp_data((void*)(core->RegBank),sizeof(core->RegBank), buf);
	sprintf(buf, "cpsr%d", num);
	add_chp_data((void*)&(core->Cpsr),sizeof(core->Cpsr), buf);
	sprintf(buf, "Spsr%d", num);
	add_chp_data((void*)(core->Spsr),sizeof(core->Spsr), buf);
	sprintf(buf, "NF%d", num);
	add_chp_data((void*)&(core->NFlag),sizeof(core->NFlag), buf);
	sprintf(buf, "ZF%d", num);
	add_chp_data((void*)&(core->ZFlag),sizeof(core->ZFlag), buf);
	sprintf(buf, "CF%d", num);
	add_chp_data((void*)&(core->CFlag),sizeof(core->CFlag), buf);
	sprintf(buf, "VF%d", num);
	add_chp_data((void*)&(core->VFlag),sizeof(core->VFlag), buf);
	sprintf(buf, "SF%d", num);
	add_chp_data((void*)&(core->SFlag),sizeof(core->SFlag), buf);
	sprintf(buf, "IFF%d", num);
	add_chp_data((void*)&(core->IFFlags),sizeof(core->IFFlags), buf);
#ifdef MODET
	sprintf(buf, "TFlag%d", num);
	add_chp_data((void*)&(core->TFlag),sizeof(core->TFlag), buf);
#endif
	sprintf(buf, "Bank%d", num);
	add_chp_data((void*)&(core->Bank),sizeof(core->Bank), buf);
	sprintf(buf, "Mode%d", num);
	add_chp_data((void*)&(core->Mode),sizeof(core->Mode), buf);
	sprintf(buf, "instr%d", num);
	add_chp_data((void*)&(core->instr),sizeof(core->instr), buf);
	sprintf(buf, "pc%d", num);
	add_chp_data((void*)&(core->pc),sizeof(core->pc), buf);
	sprintf(buf, "temp%d", num);
	add_chp_data((void*)&(core->temp),sizeof(core->temp), buf);
	sprintf(buf, "loaded%d", num);
	add_chp_data((void*)&(core->loaded),sizeof(core->loaded), buf);
	sprintf(buf, "decded_addr%d", num);
	add_chp_data((void*)&(core->loaded_addr),sizeof(core->loaded_addr), buf);
	sprintf(buf, "decded_addr%d", num);
	add_chp_data((void*)&(core->decoded_addr),sizeof(core->decoded), buf);
	sprintf(buf, "NumInstrs%d", num);
	add_chp_data((void*)&(core->NumInstrs),sizeof(core->NumInstrs), buf);
	sprintf(buf, "NextInstr%d", num);
	add_chp_data((void*)&(core->NextInstr),sizeof(core->NextInstr), buf);
	sprintf(buf, "Vector%d", num);
	add_chp_data((void*)&(core->Vector),sizeof(core->Vector), buf);
	sprintf(buf, "Aborted%d", num);
	add_chp_data((void*)&(core->Aborted),sizeof(core->Aborted), buf);
	sprintf(buf, "Reseted%d", num);
	add_chp_data((void*)&(core->Reseted),sizeof(core->Reseted), buf);
	sprintf(buf, "Base%d", num);
	add_chp_data((void*)&(core->Base),sizeof(core->Base), buf);
	sprintf(buf, "AbortAddr%d", num);
	add_chp_data((void*)&(core->AbortAddr),sizeof(core->AbortAddr), buf);
	sprintf(buf, "NresetSig%d", num);
	add_chp_data((void*)&(core->NresetSig),sizeof(core->NresetSig), buf);
	sprintf(buf, "NfiqSig%d", num);
	add_chp_data((void*)&(core->NfiqSig),sizeof(core->NfiqSig), buf);
	sprintf(buf, "NirqSig%d", num);
	add_chp_data((void*)&(core->NirqSig),sizeof(core->NirqSig), buf);
	sprintf(buf, "abortSig%d", num);
	add_chp_data((void*)&(core->abortSig),sizeof(core->abortSig), buf);
	sprintf(buf, "NtransSig%d", num);
	add_chp_data((void*)&(core->NtransSig),sizeof(core->NtransSig), buf);
	sprintf(buf, "bigendSig%d", num);
	add_chp_data((void*)&(core->bigendSig),sizeof(core->bigendSig), buf);
	sprintf(buf, "prog32Sig%d", num);
	add_chp_data((void*)&(core->prog32Sig),sizeof(core->prog32Sig), buf);
	sprintf(buf, "data32Sig%d", num);
	add_chp_data((void*)&(core->data32Sig),sizeof(core->data32Sig), buf);
	sprintf(buf, "mmu_inited%d", num);
	add_chp_data((void*)&(core->mmu_inited),sizeof(core->mmu_inited), buf);
	sprintf(buf, "mmucontrol%d", num);
	add_chp_data((void*)(&(core->mmu.control)),sizeof(core->mmu.control), buf);
	sprintf(buf, "transtablebase%d", num);
	add_chp_data((void*)(&(core->mmu.translation_table_base)),sizeof(core->mmu.translation_table_base), buf);
	sprintf(buf, "transtable0base%d", num);
	add_chp_data((void*)(&(core->mmu.translation_table_base0)),sizeof(core->mmu.translation_table_base0), buf);
	sprintf(buf, "transtable1base%d", num);
	add_chp_data((void*)(&(core->mmu.translation_table_base1)),sizeof(core->mmu.translation_table_base1), buf);
	sprintf(buf, "transtablectrl%d", num);
	add_chp_data((void*)(&(core->mmu.translation_table_ctrl)),sizeof(core->mmu.translation_table_ctrl), buf);
	sprintf(buf, "domainaccess%d", num);
	add_chp_data((void*)(&(core->mmu.domain_access_control)),sizeof(core->mmu.domain_access_control), buf);
	sprintf(buf, "fault_status%d", num);
	add_chp_data((void*)(&(core->mmu.fault_status)),sizeof(core->mmu.fault_status), buf);
	sprintf(buf, "fault_statusi%d", num);
	add_chp_data((void*)(&(core->mmu.fault_statusi)),sizeof(core->mmu.fault_statusi), buf);
	sprintf(buf, "fault_address%d", num);
	add_chp_data((void*)(&(core->mmu.fault_address)),sizeof(core->mmu.fault_address), buf);
	sprintf(buf, "last_domain%d", num);
	add_chp_data((void*)(&(core->mmu.last_domain)),sizeof(core->mmu.last_domain), buf);
	sprintf(buf, "process_id%d", num);
	add_chp_data((void*)(&(core->mmu.process_id)),sizeof(core->mmu.process_id), buf);

	/* save cache line will be instead by invalid all cache when saving chp */
	switch (core->cpu->cpu_val & core->cpu->cpu_mask) {
	case SA1100:
	case SA1110:
		//sa;
		break;
	case PXA250:
	case PXA270:		//xscale
		//xscale;
		break;
	case 0x41807200:	//arm720t
	case 0x41007700:	//arm7tdmi
	case 0x41007100:	//arm7100
		//arm7100;
		break;
	case 0x41009200:	//arm920t
		for(j = 0; j < core->mmu.u.arm920t_mmu.i_cache.set; j++){
			for( k = 0; k < core->mmu.u.arm920t_mmu.i_cache.way; k ++ ){
				sprintf(buf, "arm920t_icache_%d_%dtag%d",j,k,num);
				add_chp_data((void*)(&(core->mmu.u.arm920t_mmu.i_cache.sets[j].lines[k].tag)), sizeof(ARMword), buf);
				sprintf(buf, "arm920t_icache_%d_%dpa%d",j,k,num);
				add_chp_data((void*)(&(core->mmu.u.arm920t_mmu.i_cache.sets[j].lines[k].pa)), sizeof(ARMword), buf);
				sprintf(buf, "arm920t_icache_%d_%ddata%d",j,k,num);
				add_chp_data((void*)(core->mmu.u.arm920t_mmu.i_cache.sets[j].lines[k].data), core->mmu.u.arm920t_mmu.i_cache.width, buf);
			}
		}

		for(j = 0; j < core->mmu.u.arm920t_mmu.d_cache.set; j++){
			for( k = 0; k < core->mmu.u.arm920t_mmu.d_cache.way; k ++ ){
				sprintf(buf, "arm920t_d_cache_%d_%dtag%d",j,k,num);
				add_chp_data((void*)(&(core->mmu.u.arm920t_mmu.d_cache.sets[j].lines[k].tag)), sizeof(ARMword), buf);
				sprintf(buf, "arm920t_d_cache_%d_%dpa%d",j,k,num);
				add_chp_data((void*)(&(core->mmu.u.arm920t_mmu.d_cache.sets[j].lines[k].pa)), sizeof(ARMword), buf);
				sprintf(buf, "arm920t_d_cache_%d_%ddata%d",j,k,num);
				add_chp_data((void*)(core->mmu.u.arm920t_mmu.d_cache.sets[j].lines[k].data), core->mmu.u.arm920t_mmu.d_cache.width, buf);

				}
		}

		for(j = 0; j < core->mmu.u.arm920t_mmu.wb_t.num; j++){

			for( k = 0; k < core->mmu.u.arm920t_mmu.wb_t.entrys[j].nb; k ++ ){
				sprintf(buf, "arm920t_wb_t_%d_%dpa%d",j,k,num);
				add_chp_data((void*)(&(core->mmu.u.arm920t_mmu.wb_t.entrys[j].pa)), sizeof(wb_entry_t), buf);
				sprintf(buf, "arm920t_wb_t_%d_%ddata%d",j,k,num);
				add_chp_data((void*)(core->mmu.u.arm920t_mmu.wb_t.entrys[j].data),core->mmu.u.arm920t_mmu.wb_t.entrys[j].nb , buf);
			}
		}
		break;
	case 0x41069260:
		//arm926ejs;
		break;
	/* case 0x560f5810: */
	case 0x0007b000:
		//arm1176jzf;
		break;

	default:
		skyeye_exit (-1);
		break;

	};
}

static arm_cpu_init()
{
	ARM_CPU_State *cpu = skyeye_mm_zero(sizeof(ARM_CPU_State));
        machine_config_t *mach = get_current_mach();
        mach->cpu_data = get_conf_obj_by_cast(cpu, "ARM_CPU_State");

	cpu->core_num = 1;
	if(!cpu->core_num){
		fprintf(stderr, "ERROR:you need to set numbers of core in mach_init.\n");
		skyeye_exit(-1);
	}
	else
		cpu->core = skyeye_mm_zero(sizeof(ARMul_State) * cpu->core_num);
	/* TODO: zero the memory by malloc */

	if(!cpu->core){
		fprintf(stderr, "Can not allocate memory for arm core.\n");
		skyeye_exit(-1);
	}
	else
		printf("%d core is initialized.\n", cpu->core_num);

	int i;

	cpu->boot_core_id = 0;
	for(i = 0; i < cpu->core_num; i++){
		ARMul_State* core = &cpu->core[i];
		arm_core_init(core, i);
		skyeye_exec_t* exec = create_exec();
		exec->priv_data = get_conf_obj_by_cast(core, "arm_core_t");
		//exec->priv_data = get_conf_obj_by_cast(core, "ARMul_State");
		exec->run = per_cpu_step;
		exec->stop = per_cpu_stop;
		add_to_default_cell(exec);

		register_arm_core_chp(core, i);
	}
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

/**
* @brief Only return the number of regular register
*
* @return the number of regular register
*/
static uint32 arm_get_gpr_regnum(){
	return CPSR_REG;
	//return MAX_REG_NUM;
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
	arm_arch.get_regnum = arm_get_gpr_regnum;
	arm_arch.signal = arm_signal;

	register_arch (&arm_arch);
}
