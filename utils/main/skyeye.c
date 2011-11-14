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

/**
 *  author chenyu <yuchen@tsinghua.edu.cn>
 *  teawater <c7code-uc@yahoo.com.cn> add elf load function in 2005.08.30
 */


#ifdef __CYGWIN__
#include <getopt.h>
#else
#include <unistd.h>
#endif

#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

// Anthony Lee 2006-08-22 : for Win32API
#ifdef __MINGW32__
#undef WORD
#undef byte
#include <windows.h>
#endif
#include <assert.h>

#include "skyeye_types.h"
#include "skyeye_misc.h"
#include "skyeye_config.h"
//#include "skyeye_uart.h"
#include "config.h"
#include <setjmp.h>
#include <ctype.h>
#include <sim_control.h>
#include <skyeye_loader.h>
#if 0
struct _sky_pref_s{
	//generic_address_t elf_load_base;
	//uinteger_t elf_load_mask;
};
typedef struct _sky_pref_s sky_pref_t;
#endif
#include "skyeye_pref.h"
#include "skyeye_exec_info.h"
#include "portable/portable.h"

/**
 * A global variable , point to the current archtecture
 */

int global_argc;
char **global_argv;
jmp_buf ctrl_c_cleanup;

#define DEFAULT_CONFIG_FILE "skyeye.conf"

static void
base_termios_exit (void)
{
	//tcsetattr (STDIN_FILENO, TCSANOW, &(state->base_termios));
}

extern int init_register_type();
/**
 *  Initialize all the gloval variable
 */
static int
init ()
{
	static int done = 0;
	int ret = -1;
	if (!done) {
		done = 1;
		/*some option should init before read config. e.g. uart option. */
#if 0 /*move to common/ctrl related function */
		initialize_all_devices ();
		initialize_all_arch ();
		/* parse skyeye.conf to set skyeye_config */
		skyeye_option_init (&skyeye_config);
		if((ret = skyeye_read_config()) < 0)
			return ret;

#endif

#if 0
		/* we should check if some members of skyeye_config is initialized */
		if(!skyeye_config.arch){
			fprintf(stderr, "arch is not initialization or you have not provide arch option in skyeye.conf.\n");
                        skyeye_exit(-1);
		}
		if(!skyeye_config.mach){
			fprintf(stderr, "mach is not initialization or you have not provide mach option in skyeye.conf.\n");
			skyeye_exit(-1);
		}	
#endif

#if 0 /*move to module loading stage */
		/* initialize register type for gdb server */
		if((ret = init_register_type()) < 0)
			return ret;/* Failed to initialize register type */
#endif

#if 0
		arch_instance =
			(generic_arch_t *) malloc (sizeof (generic_arch_t));
		if (!arch_instance) {
			printf ("malloc error!\n");
			return -1;
		}
		arch_instance->init = skyeye_config.arch->init;
		arch_instance->reset = skyeye_config.arch->reset;
		arch_instance->step_once = skyeye_config.arch->step_once;
		arch_instance->set_pc = skyeye_config.arch->set_pc;
		arch_instance->get_pc = skyeye_config.arch->get_pc;
		arch_instance->ICE_write_byte = skyeye_config.arch->ICE_write_byte;
		arch_instance->ICE_read_byte = skyeye_config.arch->ICE_read_byte;

		arch_instance->init ();
		arch_instance->reset ();
		arch_instance->big_endian = Big_endian;
#endif
#if 0 /* move to module loading stage */
		skyeye_uart_converter_setup();
#endif
#if 0
		if(skyeye_config.code_cov.prof_on)
			cov_init(skyeye_config.code_cov.start, skyeye_config.code_cov.end);
#endif

#if 0
		mem_reset(); /* initialize of memory module */
#endif
	}
	return 1;
}
#if 0
#include "armemu.h"
#include "skyeye2gdb.h"
extern ARMul_State * state;
extern struct SkyEye_ICE skyeye_ice;
#endif
/**
 *  step run for the simulator
 */
#if 0
void
sim_resume (int step)
{
	/* workaround here: we have different run mode on arm */
	if(!strcmp(skyeye_config.arch->arch_name, "arm")){
		state->EndCondition = 0;
		stop_simulator = 0;

		if (step) {
			state->Reg[15] = ARMul_DoInstr (state);

			if (state->EndCondition == 0) {
				//chy 20050729 ????
				printf ("error in sim_resume for state->EndCondition");
				skyeye_exit (-1);
			}
		}
		else {
			state->NextInstr = RESUME;	/* treat as PC change */
			state->Reg[15] = ARMul_DoProg (state);
		}
		FLUSHPIPE;
	}
	/* other target simulator step run */
	else {
		do {
			/* if we are in remote debugmode we will check ctrl+c and breakpoint */
			if(remote_debugmode){
				int i;
				WORD addr;

				/* to detect if Ctrl+c is pressed.. */
				if(remote_interrupt())
					return;
				addr = arch_instance->get_pc();
				for (i = 0;i < skyeye_ice.num_bps;i++){
					if(skyeye_ice.bps[i] == addr)
            					return;
				} /* for */
                                if (skyeye_ice.tps_status==TRACE_STARTED)
                                {
                                    for (i=0;i<skyeye_ice.num_tps;i++)
                                    {
                                        if (((skyeye_ice.tps[i].tp_address==addr)&&             (skyeye_ice.tps[i].status==TRACEPOINT_ENABLED))||(skyeye_ice.tps[i].status==TRACEPOINT_STEPPING))
                                        {
                                            handle_tracepoint(i);
                                        }
                               	    }
                                }
			} /* if(remote_debugmode) */
			if (skyeye_config.log.logon >= 1) {
				WORD pc = arch_instance->get_pc();
				if (pc >= skyeye_config.log.start &&
					    pc <= skyeye_config.log.end) {
#if !defined(__MINGW32__)
					char * func_name = get_sym(pc);
					if(func_name)
						fprintf (skyeye_logfd,"\n in %s\n", func_name);
#endif
				/*
					if (skyeye_config.log.logon >= 2)
						fprintf (skyeye_logfd,
                                                         "pc=0x%x", pc);

						SKYEYE_OUTREGS (skyeye_logfd);
					if (skyeye_config.log.logon >= 3)
						SKYEYE_OUTMOREREGS
							(skyeye_logfd);
				*/
				}
			}/* if (skyeye_config.log.logon >= 1) */

			arch_instance->step_once ();
		}while(!step);
	}
}
#endif
#if 0
void sim_resume(int step){
	do {
		/* if we are in remote debugmode we will check ctrl+c and breakpoint */
		if(remote_debugmode){
			int i;
			WORD addr;
			/* to detect if Ctrl+c is pressed.. */
			if(remote_interrupt())
				return;
			addr = arch_instance->get_pc();
			for (i = 0;i < skyeye_ice.num_bps;i++){
				if(skyeye_ice.bps[i] == addr)
      					return;
			} /* for */
                        if (skyeye_ice.tps_status==TRACE_STARTED)
                        {
	                        for (i=0;i<skyeye_ice.num_tps;i++)
                                {
        	                        if (((skyeye_ice.tps[i].tp_address==addr)&& 
					            (skyeye_ice.tps[i].status==TRACEPOINT_ENABLED))||(skyeye_ice.tps[i].status==TRACEPOINT_STEPPING))
                	                {
                                            handle_tracepoint(i);
                                        }
                               	}
                        }
		} /* if(remote_debugmode) */
		if (skyeye_config.log.logon >= 1) {
			WORD pc = arch_instance->get_pc();
			if (pc >= skyeye_config.log.start &&
				    pc <= skyeye_config.log.end) {
#if !defined(__MINGW32__)
				char * func_name = get_sym(pc);
				if(func_name)
					fprintf (skyeye_logfd,"\n in %s\n", func_name);
#endif
				/*
					if (skyeye_config.log.logon >= 2)
						fprintf (skyeye_logfd,
                                                         "pc=0x%x", pc);

						SKYEYE_OUTREGS (skyeye_logfd);
					if (skyeye_config.log.logon >= 3)
						SKYEYE_OUTMOREREGS
							(skyeye_logfd);
				*/
			}
		}/* if (skyeye_config.log.logon >= 1) */

		arch_instance->step_once ();
	}while(!step);
}
#endif


void
usage ()
{	printf("%s\n",PACKAGE_STRING);
	printf("Bug report: %s\n", PACKAGE_BUGREPORT);

	printf ("Usage: skyeye [options] -e program [program args]\n");
	printf ( "Default mode is STANDALONE mode\n");
	printf (
		 "--------------------------------------------------------------------------------\n");
	printf ( "Options:\n");
	printf (
		 "-e exec-file     The (ELF executable format) kernel file name.\n");
	printf (
		 "-n               Non-interactive mode, i.e. command line is not available.\n");
	printf (
		 "-l load_address,load_address_mask\n");
	printf (
		 "                 Load ELF file to another address, not its entry.\n");
	printf ( /* 2007-03-29 by Anthony Lee : for specify big endian when non ELF */
		 "-b               Specify the data type is big endian when non \"-e\" option.\n");
	printf (
		 "-d               In GDB Server mode (can be connected by GDB).\n");
	printf (
		 "-c config-file   The Skyeye configuration file name.\n");
	printf (
		 "-h               The SkyEye command options, and ARCHs and CPUs simulated.\n");
	printf (
		 "-u               User application emulation. To use with -e.\n");
	printf (
		 "-m               Direct mapping to memory (in User application emulation).\n");
	printf (
		 "                 Faster but may not be compatible with some applications.\n");
	printf (
		 "--------------------------------------------------------------------------------\n");
}

#if 0
void skyeye_exit(int ret)
{
	/*
	 * 2007-01-24 removed the term-io functions by Anthony Lee,
	 * moved to "device/uart/skyeye_uart_stdio.c".
	 */
	exit( ret);
}
#endif
#ifdef __MINGW32__
static BOOL init_win32_socket()
{
	WSADATA wsdData;
	if(WSAStartup(0x202, &wsdData) != 0 || LOBYTE(wsdData.wVersion) != 2 || HIBYTE(wsdData.wVersion) != 2) return FALSE;
	return TRUE;
}

static void cancel_win32_socket()
{
	WSACleanup();
}
#endif


#ifndef __BEOS__
/* 2007-01-31 disabled by Anthony Lee on BeOS for multi-thread safe. */
void sigint_handler (int signum)
{
#if 0
	 if(skyeye_config.code_cov.prof_on)
		cov_fini(skyeye_config.code_cov.prof_filename);
#endif
	longjmp (ctrl_c_cleanup, 1);
}
#endif

int init_option(int argc, char** argv, sky_pref_t* pref){
	int c;
	int index;
	bool_t interactive_mode = True;
	bool_t autoboot_mode = False;
	bool_t user_mode = False;
	bool_t mmap_access = False;
	bool_t interpret_mode = False;
	uint32_t bot_log = 0;
	uint32_t top_log = 0;
	int remote_debugmode = 0;
	endian_t endian = Little_endian;
	
	char* exec_file = NULL;
	char* exec_argv = NULL;
	char* exec_envp = NULL;
	int exec_argc = 0;
	int exec_envc = 0;
	generic_address_t elf_load_base = 0x0;
	uint32_t elf_load_mask = 0xFFFFFFFF;
	/**
	 *  name of current config file
	 */
	char *skyeye_config_filename = NULL;
	
	uint32 uart_port;

	//char *exec_file = NULL;
	int ret = 0;
	opterr = 0;
	while ((c = getopt (argc, argv, "be:dc:p:nl:humig:")) != -1)
		switch (c) {
		case 'e':
			exec_file = optarg;
			goto loop_exit;
		case 'd':
			remote_debugmode = 1;
			break;
		case 'h':
			usage();
			display_all_support();
			ret = -1;
		case 'c':
			skyeye_config_filename = optarg;
			break;
		case 'p':
                        uart_port = strtoul(optarg, NULL, 10);
			break;
		case 'n':
			interactive_mode = False;
			/* 
			 * Under non-interactive mode, we should 
			 * autoboot the simulator
			 */
			autoboot_mode = True;
			break;
		case 'l':
		{
			char * tok = ",";
			char * str1 = strtok(optarg, tok);
			char * str2 = (char *)(optarg + strlen(str1) + 1);
			elf_load_base = strtoul(str1, NULL, 16);
			elf_load_mask = strtoul(str2, NULL, 16);
		}
			break;
		case 'b':
			endian = Big_endian;
			break;
		case 'u':
			user_mode = True;
			break;
		case 'm':
			mmap_access = True;
			break;
		case 'i':
			interpret_mode = True;
			break;
		case 'g':
			{
				char * atok = ",";
				char * astr1 = strtok(optarg, atok);
				char * astr2 = (char *)(optarg + strlen(astr1) + 1);
				bot_log = strtoul(astr1, NULL, 16);
				top_log = strtoul(astr2, NULL, 16);
			}
			break;
		case '?':
			if (isprint (optopt))
				fprintf (stderr, "Unknown option `-%c'.\n",
					 optopt);
			else
				fprintf (stderr,
					 "Unknown option character `\\x%x'.\n",
					 optopt);
			ret = -1;
			return ret;
		/*
		case 'v':
			display_all_support();
			goto exit_skyeye;
		*/
		default:
			fprintf(stderr, "Default option .....\n");
			break;
	}

loop_exit:

	if (skyeye_config_filename == NULL)
                skyeye_config_filename = DEFAULT_CONFIG_FILE;

	if (!exec_file)
		for (index = optind; index < argc; index++)
			printf ("Non-option argument %s\n", argv[index]);
	else
	{
		// sequence to get arguments from the command line
		exec_argv = (char *) (optarg);
		char * temp_arg = exec_argv;
		exec_argc = argc - optind + 1;
		index = 0;
		//printf(">>> Found %d args starting at %p : %s\n", exec_argc, exec_argv, exec_argv);
		while( strlen(temp_arg) && (index < exec_argc) )
		{
			//printf(">>>%s\n", temp_arg);
			temp_arg = (char *) (temp_arg + strlen(temp_arg) + 1);
			index++;
		}
		exec_envp = temp_arg;
		while ( strlen(temp_arg) )
		{
			exec_envc += 1;
			//printf(">>%02d>>%s\n", exec_envc, temp_arg);
			temp_arg = (char *) (temp_arg + strlen(temp_arg) + 1);
			index++;
		}
		// last envp is not part of envc
		exec_envc -= 1;
		//printf(">>> Found %d envp starting at %p : %s\n", exec_envc, exec_envp, exec_envp);
	}

	/* detection of endian */
#if 0
	if (exec_file) tea_load_exec(exec_file, 1);
	if(big_endian)
		printf("Your elf file is big endian.\n");
	else
		printf("Your elf file is little endian.\n");
#endif
	assert(pref != NULL);
	pref->conf_filename = strdup(skyeye_config_filename);
	if(pref->conf_filename == NULL)
		exit(-1);
	/*
	if(interactive_mode)
		pref->interactive_mode = True;
	else
		pref->interactive_mode = False;
	*/
	pref->interactive_mode = interactive_mode;
	pref->autoboot = autoboot_mode;

	if(exec_file){
		pref->exec_file = strdup(exec_file);
		if(!pref->exec_file){
			fprintf(stderr, "Can not allocate memory.\n");
			exit(-1);
		}
		endian = get_elf_endian(exec_file);
		if (user_mode)
		{
			sky_exec_info_t *info = get_skyeye_exec_info();
			info->exec_argc = exec_argc;
			info->exec_argv = exec_argv;
			info->exec_envp = exec_envp;
			info->exec_envc = exec_envc;
		
			/* read complementary information */
			retrieve_info(pref->exec_file, NULL);
		
			info->mmap_access = mmap_access;
		}
	}
	
	pref->exec_load_base = elf_load_base;
	pref->exec_load_mask = elf_load_mask;
	pref->endian = endian;
	pref->uart_port = uart_port;
	pref->user_mode_sim = user_mode;
	pref->interpret_mode = interpret_mode;
	pref->bot_log = bot_log;
	pref->top_log = top_log;
	pref->start_logging = 0;
	return ret;
}

int init_env(){
	int ret;
#ifdef __MINGW32__
	init_win32_socket();
	atexit(cancel_win32_socket);
#endif

#ifndef __BEOS__
	/* 2007-01-31 disabled by Anthony Lee on BeOS for multi-thread safe. */
	if (setjmp (ctrl_c_cleanup) != 0) {
		return -1;
	}

	signal (SIGINT, sigint_handler);
#endif
	return 0;
}


/**
 *  The main function of skyeye
 */

int
main (int argc, char **argv)
{
	int ret;

	sky_pref_t* pref = get_skyeye_pref();
	assert(pref != NULL);	
	/* initialization of options from command line */
	ret = init_option(argc, argv, pref);
	/* set the current preference for skyeye */
	//update_skyeye_pref(pref);
	/* return non-zero represent not run skyeye */
	if(ret != 0)
		exit(0);
	else
		SIM_init();
	/* Do anything you want to do , or just deadloop here. */
	while(1)
		sleep(1);
	;
#if 0
	if(ret < 0)
		goto exit_skyeye;

	/* set some environment for running */
	init_env();

	/* initialization of all the data structure of simulation */
	if((ret = init ()) < 0)
		goto exit_skyeye;
#endif

#if 0
	/* load elf file when running elf image */
	if (exec_file) {
		if (tea_load_exec (exec_file, 0)) {
			fprintf (stderr, "load \"%s\" error\n", exec_file);
			goto exit_skyeye;
		}
#if !defined(__MINGW32__)
		/* get its symbol to debug */
		init_symbol_table(exec_file);
#endif
	}
#endif
	/* set the start address for pc */
#if 0
	if (skyeye_config.start_address != 0){
		unsigned long addr = (skyeye_config.start_address & load_mask)|load_base;
		arch_instance->set_pc (addr);
		printf ("start addr is set to 0x%08x by exec file.\n",
                        (unsigned int) addr);

	}
#endif
	/* never return */
	//SIM_start();
	//fflush(stdout);
    
#if 0
	/* running simulaion */ 
	if (remote_debugmode == 0)
		sim_resume (0);
	else{	
		printf ("remote_debugmode= %d, filename = %s, server TCP port is 12345\n",
			remote_debugmode, skyeye_config_filename);
		sim_debug ();
	}
#endif
exit_skyeye:
	return ret;
}


#ifdef __MINGW32__
int _stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return main(__argc, __argv);
}
#endif // __MINGW32__
