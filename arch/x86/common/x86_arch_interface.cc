/*

		THIS SOFTWARE IS NOT COPYRIGHTED

   Cygnus offers the following for use in the public domain.  Cygnus
   makes no warranty with regard to the software or it's performance
   and the user accepts the software "AS IS" with all faults.

   CYGNUS DISCLAIMS ANY WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD TO
   THIS SOFTWARE INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

*/
/*
 *  written by Michael.Kang in 20090510
 */

#ifdef __MINGW32__
#include <windows.h>
#endif

#include <signal.h>
#include <stdio.h>

#define NEED_CPU_REG_SHORTCUTS 1
#include "bochs.h"
#include "cpu.h"
#include "gui/textconfig.h"
#define LOG_THIS BX_CPU_THIS_PTR

#include "skyeye_types.h"
#include "skyeye_config.h"
#include "skyeye_options.h"
#include "skyeye_arch.h"
#include "skyeye_exec.h"
#include "skyeye_cell.h"
#include "skyeye_obj.h"
#include "x86_regformat.h"
//#include "x86_defs.h"
#define InstrumentICACHE 0

#if InstrumentICACHE
static unsigned iCacheLookups=0;
static unsigned iCacheMisses=0;

#define InstrICache_StatsMask 0xffffff

#define InstrICache_Stats() {\
  if ((iCacheLookups & InstrICache_StatsMask) == 0) { \
    BX_INFO(("ICACHE lookups: %u, misses: %u, hit rate = %6.2f%% ", \
          iCacheLookups, \
          iCacheMisses,  \
          (iCacheLookups-iCacheMisses) * 100.0 / iCacheLookups)); \
    iCacheLookups = iCacheMisses = 0; \
  } \
}
#define InstrICache_Increment(v) (v)++
#else
#define InstrICache_Stats()
#define InstrICache_Increment(v)
#endif

// Make code more tidy with a few macros.
#if BX_SUPPORT_X86_64==0
#define RIP EIP
#define RCX ECX
#endif

// The CHECK_MAX_INSTRUCTIONS macro allows cpu_loop to execute a few
// instructions and then return so that the other processors have a chance to
// run.  This is used by bochs internal debugger or when simulating
// multiple processors.
//
// If maximum instructions have been executed, return. The zero-count
// means run forever.
#if BX_SUPPORT_SMP || BX_DEBUGGER
  #define CHECK_MAX_INSTRUCTIONS(count) \
    if ((count) > 0) {                  \
      (count)--;                        \
      if ((count) == 0) return;         \
    }
#else
  #define CHECK_MAX_INSTRUCTIONS(count)
#endif


#define NONCACHE  0
static unsigned int cycles;
static char * arch_name = "i386";
static void
sim_size ()
{
}

static void x86_step_once ();
static void per_cpu_step(conf_object_t* cpu);
static void per_cpu_stop(conf_object_t* cpu);

void
x86_init_state ()
{
	bx_user_quit = 0;
	bx_init_siminterface();   // create the SIM object
	static jmp_buf context;
	if (setjmp (context) == 0) {
		SIM->set_quit_context (&context);
		BX_INSTR_INIT_ENV();
		if (bx_init_main(bx_startup_flags.argc, bx_startup_flags.argv) < 0)
		{ BX_INSTR_EXIT_ENV();
			//return 0;
			return;
		}
		// read a param to decide which config interface to start.
		// If one exists, start it.  If not, just begin.
		bx_param_enum_c *ci_param = SIM->get_param_enum(BXPN_SEL_CONFIG_INTERFACE);
		const char *ci_name = ci_param->get_selected();
		if (!strcmp(ci_name, "textconfig")) {
		#if BX_USE_TEXTCONFIG
			init_text_config_interface();   // in textconfig.h
		#else
			BX_PANIC(("configuration interface 'textconfig' not present"));
		#endif
		}
		else if (!strcmp(ci_name, "win32config")) {
#if BX_USE_TEXTCONFIG && defined(WIN32)
	/* FIXME, we should use the configure to check the current build environment. */
			//init_win32_config_interface();
#else
			BX_PANIC(("configuration interface 'win32config' not present"));
#endif
		}
#if BX_WITH_WX
		else if (!strcmp(ci_name, "wx")) {
			PLUG_load_plugin(wx, PLUGTYPE_CORE);
		}
#endif
		else {
			BX_PANIC(("unsupported configuration interface '%s'", ci_name));
		}
		ci_param->set_enabled(0);
		SIM->get_param_enum(BXPN_BOCHS_START)->set(BX_QUICK_START);
		//SIM->begin_simulation(bx_startup_flags.argc, bx_startup_flags.argv);
		#if 1  /* we do not launch configuration interface. */
		int status = SIM->configuration_interface(ci_name, CI_START);
		if (status == CI_ERR_NO_TEXT_CONSOLE)
			BX_PANIC(("Bochs needed the text console, but it was not usable"));
		#endif
    // user quit the config interface, so just quit
	} else {
		// quit via longjmp
	}
	skyeye_exec_t* exec = create_exec();
	exec->priv_data = get_conf_obj_by_cast(NULL, "BX_CPU_C");
	exec->run = per_cpu_step;
	exec->stop = per_cpu_stop;
	add_to_default_cell(exec);
}

static void per_cpu_step(conf_object_t* cpu){
	while(1)
		x86_step_once();
} 
static void per_cpu_stop(conf_object_t* cpu){
}
void
x86_reset_state ()
{
	/*fixme */
	BX_CPU(0)->reset(BX_RESET_HARDWARE);
	cycles = 0;
}

/* Execute a single instruction.  */
static void
x86_step_once ()
{
	cycles++;
	if (BX_SMP_PROCESSORS == 1) {
      // only one processor, run as fast as possible by notmessing with quantums and loops.
		BX_CPU(0)->cpu_loop(0);
        	if (bx_pc_system.kill_bochs_request)
          		return;
      // for one processor, the only reason for cpu_loop toreturn is
      // that kill_bochs_request was set by the GUI interface.
    	}
    	else {
		// SMP simulation: do a few instructions on each processor, then switch
      // to another.  Increasing quantum speeds up overall performance, but
      // reduces granularity of synchronization between processors.
		int processor = 0;
		int quantum = SIM->get_param_num(BXPN_SMP_QUANTUM)->get();
		// do some instructions in each processor
			BX_CPU(processor)->cpu_loop(quantum);
			processor = (processor+1) % BX_SMP_PROCESSORS;
			if (bx_pc_system.kill_bochs_request)
				return;
			if (processor == 0)
				BX_TICKN(quantum);
	}
}
static void
x86_set_pc (generic_address_t addr)
{
	//EIP = addr;
	EIP = 0xFFF0;
}
static generic_address_t x86_get_pc(){
	return EIP;
}
cpu_config_t x86_cpu[] = {
	{NULL,NULL,0,0,0}
};
//chy 2006-04-15
static int 
x86_ICE_write_byte (generic_address_t addr, uint8_t v)
{
	return 0;
}
static int
x86_ICE_read_byte(generic_address_t addr, uint8_t * pv){
	return 0;
}
static int
x86_parse_cpu (cpu_config_t * cpu, const char *param[])
{
}
extern void pc_mach_init (void * arch_instance, machine_config_t * this_mach);
machine_config_t x86_machines[] = {
	{"pc", pc_mach_init, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL},
};

#if 0
static int
x86_parse_mach (machine_config_t * cpu, const char *params[])
{
	int i;
	for (i = 0; i < (sizeof (x86_machines) / sizeof (machine_config_t));
	     i++) {
		if (!strncmp
		    (params[0], x86_machines[i].machine_name,
		     MAX_PARAM_NAME)) {
			skyeye_config.mach = &x86_machines[i];
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

static uint32 x86_get_step(){
        return cycles;
}
static char* x86_get_regname_by_id(int id){
        return x86_regstr[id];
}
static uint32 x86_get_regval_by_id(int id){
        //return state->Reg[id];
	uint32 ret;
	switch(id){
		case Eax:
			ret = EAX;
			break;
		case Ebx:
			ret = EBX;
			break;
		case Ecx:
			ret = ECX;
			break;
		case Edx:
                        ret = EDX;
                        break;
		case Esp:
                        ret = ESP;
                        break;
		case Ebp:
                        ret = EBP;
                        break;
		case Esi:
                        ret = ESI;
                        break;
		case Edi:
                        ret = EDI;
                        break;
		case Eip:
                        ret = EIP;
                        break;
		case Cs:
			ret = BX_CPU_THIS_PTR sregs[BX_SEG_REG_CS].selector.value;
                        break;
		case Ds:
			ret = BX_CPU_THIS_PTR sregs[BX_SEG_REG_DS].selector.value;
			break;
		case Es:
                        ret = BX_CPU_THIS_PTR sregs[BX_SEG_REG_ES].selector.value;
                        break;
		case Ss:
                        ret = BX_CPU_THIS_PTR sregs[BX_SEG_REG_SS].selector.value;
                        break;
		case Fs:
                        ret = BX_CPU_THIS_PTR sregs[BX_SEG_REG_FS].selector.value;
			break;
		case Gs:
                        ret = BX_CPU_THIS_PTR sregs[BX_SEG_REG_GS].selector.value;
                        break;
		default:
			ret = 0;
			fprintf(stderr, "Can not find register for id %d.\n", id);
			break;
	}
	return ret;
}
static exception_t x86_set_register_by_id(int id, uint32 value){
        //state->Reg[id] = value;
        return No_exp;
}

exception_t mmu_read(short length, generic_address_t addr,uint32_t *value)
{
	switch(length)
	{
		case 8:
			*value = BX_CPU(0)->read_virtual_byte_32(BX_SEG_REG_CS,addr);break;
		case 16:
			*value = BX_CPU(0)->read_virtual_word_32(BX_SEG_REG_CS,addr);break;
		case 32:
			*value = BX_CPU(0)->read_virtual_dword_32(BX_SEG_REG_CS,addr);break;
		default:
			*value = 0;
			fprintf(stderr,"Wrong argument value of  'length' in function %s \n",__FUNCTION__);
	}
	return No_exp;
}
void
init_x86_arch ()
{
	static arch_config_t x86_arch;

	x86_arch.arch_name = arch_name;
	x86_arch.init = x86_init_state;
	x86_arch.reset = x86_reset_state;
	x86_arch.step_once = x86_step_once;
	x86_arch.set_pc = x86_set_pc;
	x86_arch.get_pc = x86_get_pc;
	x86_arch.ICE_write_byte = x86_ICE_write_byte;
	x86_arch.ICE_read_byte = x86_ICE_read_byte;
	//x86_arch.parse_cpu = x86_parse_cpu;
	//x86_arch.parse_mach = x86_parse_mach;
	x86_arch.get_regval_by_id = x86_get_regval_by_id;
	x86_arch.get_regname_by_id = x86_get_regname_by_id;
	x86_arch.get_step = x86_get_step;
	x86_arch.mmu_read = mmu_read;
	register_arch (&x86_arch);
}
