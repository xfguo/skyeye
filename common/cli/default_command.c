#include <errno.h>

#include "skyeye_cli.h"
#include "skyeye_module.h"
#include "skyeye_options.h"
#include "skyeye_mach.h"
#include "skyeye_pref.h"
#include "skyeye_arch.h"
#include "bank_defs.h"
//#include "arm_regformat.h"
/*
 * Run SkyEye from the beginning 
 */
com_run (arg)
     char *arg;
{
  if (!arg)
    arg = "";
  int flag = SIM_run();
  return flag;

  //sprintf (syscom, "ls -FClg %s", arg);
  //return (system (syscom));
}

/*
 * Continue running of the interrupted SkyEye 
 */
com_cont (arg)
     char *arg;
{
  if (!arg)
    arg = "";
  int flag = SIM_continue();
  return flag;
  //sprintf (syscom, "ls -FClg %s", arg);
  //return (system (syscom));
}

/*
 * stop running of  SkyEye 
 */
com_stop (arg)
     char *arg;
{
  if (!arg)
    arg = "";
  int flag = SIM_stop();
  return flag;
  //sprintf (syscom, "ls -FClg %s", arg);
  //return (system (syscom));
}

/*
 * stop running of  SkyEye 
 */

com_start (arg)
     char *arg;
{
	int flag = 0;
	SIM_start();
	return flag;
}

/*
 * step run of SkyEye
 */
com_si(char* arg){
	int flag = 0;
	char** endptr;
	int steps;
	if(arg == NULL || *arg == '\0') 
		steps = 1;
	else{
		errno = 0;
		steps = strtoul(arg, endptr, 10);
		#if 0
	/* if *nptr is not '\0' but **endptr is '\0', the entire string is valid */
		if(**endptr != '\0'){
			printf("Can not run the given steps.\n ");
			return flag;
		}
		#endif
		#if 0	
		if(errno != 0){
			if(errno == EINVAL)
				printf("Not valid digital format.\n");
			else if(errno == ERANGE)
				printf("Out of range for your step number.\n");
			else
				printf("Some unknown error in your stepi command.\n");
			return 1;
		}
		#endif
	}
	skyeye_stepi(steps);
	return flag;
}

com_x(char* arg){
	int flag = 0;
	char** endptr;
	int addr;
	uint32 value;
	if(arg == NULL || *arg == '\0') 
		return;
	else{
		errno = 0;
		addr = strtoul(arg, endptr, 16);
		#if 0
	/* if *nptr is not '\0' but **endptr is '\0', the entire string is valid */
		if(**endptr != '\0'){
			printf("Can not run the given steps.\n ");
			return flag;
		}
		#endif
		#if 0	
		if(errno != 0){
			if(errno == EINVAL)
				printf("Not valid digital format.\n");
			else if(errno == ERANGE)
				printf("Out of range for memory address.\n");
			else
				printf("Some unknown error in your x command.\n");
			return 1;
		}
		#endif
	}
	bus_read(32, addr, &value);
	printf("0x%x:0x%x\n", addr, value);
	return flag;
}


/* String to pass to system ().  This is for the LIST, VIEW and RENAME
   commands. */
static char syscom[1024];

/* List the file(s) named in arg. */
com_list (arg)
     char *arg;
{
  if (!arg)
    arg = "";

  sprintf (syscom, "ls -FClg %s", arg);
  return (system (syscom));
}

com_view (arg)
     char *arg;
{
  if (!valid_argument ("view", arg))
    return 1;

#if defined (__MSDOS__)
  /* more.com doesn't grok slashes in pathnames */
  sprintf (syscom, "less %s", arg);
#else
  sprintf (syscom, "more %s", arg);
#endif
  return (system (syscom));
}

com_rename (arg)
     char *arg;
{
  too_dangerous ("rename");
  return (1);
}

com_stat (arg)
     char *arg;
{
  struct stat finfo;

  if (!valid_argument ("stat", arg))
    return (1);

  if (stat (arg, &finfo) == -1)
    {
      perror (arg);
      return (1);
    }

  printf ("Statistics for `%s':\n", arg);

  printf ("%s has %d link%s, and is %d byte%s in length.\n",
	  arg,
          finfo.st_nlink,
          (finfo.st_nlink == 1) ? "" : "s",
          finfo.st_size,
          (finfo.st_size == 1) ? "" : "s");
  printf ("Inode Last Change at: %s", ctime (&finfo.st_ctime));
  printf ("      Last access at: %s", ctime (&finfo.st_atime));
  printf ("    Last modified at: %s", ctime (&finfo.st_mtime));
  return (0);
}

com_delete (arg)
     char *arg;
{
  too_dangerous ("delete");
  return (1);
}


/* Change to the directory ARG. */
com_cd (arg)
     char *arg;
{
  if (chdir (arg) == -1)
    {
      perror (arg);
      return 1;
    }

  com_pwd ("");
  return (0);
}

/* Print out the current working directory. */
com_pwd (ignore)
     char *ignore;
{
  char dir[1024], *s;

  s = getcwd (dir, sizeof(dir) - 1);
  if (s == 0)
    {
      printf ("Error getting pwd: %s\n", dir);
      return 1;
    }

  printf ("Current directory is %s\n", dir);
  return 0;
}

extern void set_cli_done();
/* The user wishes to quit using this program.  Just set DONE non-zero. */
com_quit (arg)
     char *arg;
{
  set_cli_done();  
  SIM_fini();
  exit (0);
}

int com_list_modules(char* arg){
	char* format = "%-20s\t%s\n";
	printf(format, "Module Name", "File Name");
	skyeye_module_t* list = get_module_list();
	while(list != NULL){
		printf(format, list->module_name, list->filename);
		list = list->next;
	}
	return 0;
}

int com_list_options(char* arg){
	char* format = "%-20s\t%s\n";
	printf(format, "Option Name", "Description");
	skyeye_option_t* list = get_option_list();
	while(list != NULL){
		//printf("option_name = %s\n", list->option_name);
		printf(format, list->option_name, list->helper);
		list = list->next;
	}
	return 0;
}

int com_list_machines(char* arg){
	char* format = "%-20s\n";
	printf(format, "Machine Name");
	machine_config_t* list = get_mach_list();
	while(list != NULL){
		//printf("option_name = %s\n", list->option_name);
		printf(format, list->machine_name);
		list = list->next;
	}
	return 0;
}

int com_show_pref(char* arg){
	char* format = "%-30s\t%s\n";
	char* int_format = "%-30s\t0x%x\n";
	sky_pref_t * pref = get_skyeye_pref();
	printf(format, "Module search directorys:", pref->module_search_dir);
	printf(int_format, "Boot address:", pref->start_address);
	printf(format, "Executable file:", pref->exec_file);
	printf(int_format, "Load base for executable file:", pref->exec_load_base);
	printf(int_format, "Load mask for executable file:", pref->exec_load_mask);
	printf(format, "SkyEye config file:", pref->conf_filename);
	printf(format, "Endian of exec file:", pref->endian == Big_endian?"Big Endian":"Little endian");
	return 0;
}

int com_show_map(char* arg){
	mem_bank_t* bank;
	mem_config_t* memmap = get_global_memmap();
	char* format1 = "%-30s\t%s\t%20s\n";
	printf(format1, "Start Addr", "Length", "Type");
	char* format2 = "0x%-30x\t0x%x\t%20s\n";
	
	for (bank = memmap->mem_banks; bank->len; bank++)
		printf(format2, bank->addr, bank->addr + bank->len, 
			bank->type?"memory":"IO");
	return 0;
}

int com_load_module(char* arg){
	return 0;
}
int com_info(char* arg){
	char* info_target[] = {"registers", "breakpoint"};
	if(arg == NULL){
		printf("Available options is %s, %s\n", info_target[0], info_target[1]);
		return 0;
	}
	if(!strncmp(arg,info_target[0], strlen(info_target[0]))){
	/* display the information of int register */
		generic_arch_t* arch_instance = get_arch_instance();
		if(arch_instance == NULL)
			return;
		if(arch_instance->get_regval_by_id){
			/* only for arm */		
			int i = 0;
			uint32 reg_value = 0;
			//while(arm_regstr[i]){
			while(arch_instance->get_regname_by_id(i)){
				reg_value = arch_instance->get_regval_by_id(i);
				printf("%s\t0x%x\n", arch_instance->get_regname_by_id(i), reg_value);
				i++;
			}
		}
		else{
			printf("Current processor not implement the interface.\n");
			return 0;
		}
	}
	else if(!strncmp(arg, info_target[1], strlen(info_target[1]))){
	/* display the information of current breakpoint */
	}
	else{
		printf("can not find information for the object %s\n", arg);
	}
}
int com_load_conf(char* arg){
	exception_t ret;
	if ((ret = skyeye_read_config(arg)) != No_exp){
		if(ret == Conf_format_exp){
			printf("Can not parse the config file  %s correctly.\n ", arg);
		}
		if(ret == File_open_exp)
			printf("Can not open the config file %s\n", arg);
		else
			printf("Unknown error when read config from the file %s\n", arg);
	}
	return 0;
}
