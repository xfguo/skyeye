#include <stdlib.h>
#include <sim_control.h>
#include "skyeye_types.h"
#include "skyeye_config.h"
#include "skyeye_pref.h"
#include "skyeye_module.h"
#include "skyeye_arch.h"
#include "skyeye_callback.h"
/* FIXME, we should get it from prefix varaible after ./configure */
#ifndef SKYEYE_MODULE_DIR
const char* default_lib_dir = "/opt/skyeye/lib/skyeye/";
#else
const char* default_lib_dir = SKYEYE_MODULE_DIR;
#endif

void SIM_init_command_line(void){
}

void SIM_init_environment(char** argv, bool_t handle_signals){
}

void SIM_main_loop(void){
}

/* we try to do init in batch mode */
static exception_t try_init(){
}

void SIM_cli();

/*
 *
 */
void SIM_init(){
	sky_pref_t* pref;
	char* welcome_str = get_front_message();
	/* 
	 * get the corrent_config_file and do some initialization 
	 */
	skyeye_config_t* config = get_current_config();
	skyeye_option_init(config);
	/*
	 * initialization of callback data structure, it needs to 
	 * be initialized at very beginning.
	 */
	init_callback();

	/*
	 * initilize the data structure for command
	 * register some default built-in command
	 */
	init_command_list();

	init_stepi();

	/*
	 * initialization of module manangement 
	 */
	init_module_list();
	
	/*
	 * initialization the timer scheduler 
	 */
	init_thread_scheduler();
	init_timer_scheduler();

	
	/*
	 * initialization of architecture and cores
	 */
	init_arch();
	
	/*
	 * initialization of bus and memory module
	 */
	init_bus();


	/*
	 * initialization of machine module
	 */
	init_mach();

	
	/*
	 * initialization of breakpoint, that depends on callback module.
	 */
	init_bp();

	/* 
	 * get the current preference for simulator 
	 */
	pref = get_skyeye_pref();

	/* 
	 * loading all the modules in search directory 
	 */
	if(!pref->module_search_dir)
		pref->module_search_dir = skyeye_strdup(default_lib_dir);
	SKY_load_all_modules(pref->module_search_dir, NULL);

	/* save the original termios */
	struct termios tmp;
	tcgetattr(0, &tmp);
	memcpy(&pref->saved_term, &tmp, sizeof(struct termios));

	//skyeye_config_t *config;
	//config = malloc(sizeof(skyeye_config_t));
	/*	
	if(pref->autoboot == True){
		SIM_start();
		SIM_run();
	}
	*/
	/*
	 * if we run simulator in GUI or external IDE, we do not need to
	 * launch our CLI.
	 */
	if(pref->interactive_mode == True){
		SIM_cli();		
	}
	else{
		SIM_start();
		SIM_run();
	}
}

void SIM_start(void){
	sky_pref_t *pref;
	/* get the current preference for simulator */
	pref = get_skyeye_pref();
	skyeye_config_t* config = get_current_config();
	if(pref->conf_filename)
		skyeye_read_config(pref->conf_filename);

	if(config->arch == NULL){
		skyeye_log(Error_log, __FUNCTION__, "Should provide valid arch option in your config file.\n");
		return;
	}
	generic_arch_t *arch_instance = get_arch_instance(config->arch->arch_name);

	if(config->mach == NULL){
		skyeye_log(Error_log, __FUNCTION__, "Should provide valid mach option in your config file.\n");
		return;
	}

	arch_instance->init();
	
	/* reset all the memory */
	mem_reset();

	config->mach->mach_init(arch_instance, config->mach);
	/* reset current arch_instanc */
	arch_instance->reset();
	/* reset all the values of mach */
	io_reset(arch_instance);
	
	if(pref->exec_file){
		exception_t ret;
		/* 
		 * If no relocation is indicated, we will load elf file by 
		 * virtual address
		 */
		if((((~pref->exec_load_mask) & pref->exec_load_base) == 0x0) &&
			(arch_instance->mmu_write != NULL))
			ret = load_elf(pref->exec_file, Virt_addr);
		else
			ret = load_elf(pref->exec_file, Phys_addr);
	}

	/* set pc from config */
	generic_address_t pc = (config->start_address & pref->exec_load_mask)|pref->exec_load_base; 
	skyeye_log(Info_log, __FUNCTION__, "Set PC to the address 0x%x\n", pc);
	arch_instance->set_pc(pc);

	/* Call bootmach callback */
	exec_callback(Bootmach_callback, arch_instance);	

	pthread_t id;
	create_thread(skyeye_loop, arch_instance, &id);
	
	/* 
	 * At this time, if we set conf file, then we parse it
	 * Or do it later.
	 */

	/*
	if(pref->conf_filename)
		skyeye_read_config(pref->conf_filename);
	*/
#if 0	
	else{
		/* try to run in batch mode */
		if(skyeye_read_config(pref->conf_filename) == No_exp){
		/* if can not */
			init_threads();
			SIM_run();
		}
	}
#endif	
}
void SIM_cli(){
	skyeye_cli();
}
void SIM_run(){
	skyeye_start();
}
#if 0
void SIM_break_simulation(const char *msg){
}
void SIM_pause(){
	skyeye_pause();
}
#endif
void SIM_continue(generic_arch_t* arch_instance){
	skyeye_continue();
}

void SIM_stop(generic_arch_t* arch_instance){
	skyeye_pause();	
}
void SIM_fini(){
	sky_pref_t *pref = get_skyeye_pref();
	//pthread_cancel();
	generic_arch_t* arch_instance = get_arch_instance("");
	SIM_stop(arch_instance);
	printf("Destroy threads.\n");
	destroy_threads();
	printf("Unload all modules.\n");
	/* unload all the module */
	SKY_unload_all_modules();
	/* free the memory */
	printf("exit.\n");
	/* restore the environment */
	tcsetattr(0, TCSANOW, &pref->saved_term);

	return;
	//exit(0);
}
#if 0
void register_cli(void (*skyeye_cli)(), char *module_name){
	SIM_cli = skyeye_cli;
}
#endif
