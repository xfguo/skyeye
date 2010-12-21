#include <stdlib.h>
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#ifdef HAVE_SYS_FILE_H
#  include <sys/file.h>
#endif
#include <sys/stat.h>

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#if defined (HAVE_STRING_H)
#  include <string.h>
#else /* !HAVE_STRING_H */
#  include <strings.h>
#endif /* !HAVE_STRING_H */


#ifdef READLINE_LIBRARY
#  include "readline.h"
#  include "history.h"
#else
#  include <readline/readline.h>
#  include <readline/history.h>
#endif

#include "skyeye_types.h"
#include "skyeye_cli.h"
#include "default_command.h"
#include "skyeye_command.h"


/* **************************************************************** */
/*                                                                  */
/*                       predefined  commands                       */
/*                                                                  */
/* **************************************************************** */
int com_help(char* arg);
COMMAND default_cli_commands[] = {
  {"help", com_help, "List all the category for the commands.\n"},
  {"?", com_help, "Synonym for `help'.\n"},
  //{ "cd", com_cd, "Change to directory DIR" },
  //{ "delete", com_delete, "Delete FILE" },
 // { "list", com_list, "List files in DIR" },
  { "ls", com_list, "Synonym for `list'" },
 // { "pwd", com_pwd, "Print the current working directory" },
  { "quit", com_quit, "Quit SkyEye " },
  { "q", com_quit, "Quit SkyEye " },
 // { "rename", com_rename, "Rename FILE to NEWNAME" },
 // { "stat", com_stat, "Print out statistics on FILE" },
//  { "view", com_view, "View the contents of FILE" },
  { "run", com_run, "Start the simulator." },
  { "stop", com_stop, "Stop the running of simulator." },
  { "continue", com_cont, "Continue the running of interrupted simulator." },
  { "stepi", com_si, "step into ." },
  { "start", com_start, "start simulator." },
  {"list-modules", com_list_modules, "List all the loaded module."},
  {"show-pref", com_show_pref, "Show the current preference for SkyEye."},
  {"show-map", com_show_map, "Show the current memory map for the machine."},
  {"list-options", com_list_options, "List all the available options for SkyEye."},
  {"list-machines", com_list_machines, "List all the supported machines for SkyEye."},
  {"load-conf", com_load_conf, "load a config file and parse it for SkyEye."},
  {"info", com_info, "show information for various objects. "},
  {"x", com_x, "display memory value at the address. "},
  { (char *)NULL, (rl_icpfunc_t *)NULL, (char *)NULL }
};

static COMMAND *command_list;
int com_help(char *command);
void init_command_list(){
	command_list = NULL;
	COMMAND* node;
	int index = 0;
	/* add all the predefined command into the list */	
	node = &default_cli_commands[index];
	while(node->name){
		/* FIXME, do we need to check the return of add_command? */
		add_command(node->name, node->func, node->doc);
		node = &default_cli_commands[index++];	
	}	
}

exception_t add_command(char* command_str, rl_icpfunc_t *func, char* help_str){
	COMMAND *command = malloc(sizeof(COMMAND));
	if(command == NULL){
		return Malloc_exp;
	}
	command->name = strdup(command_str);
	if(command->name == NULL){
		free(command);
		return Malloc_exp;
	}
	command->func = func;
	command->doc = strdup(help_str);
	if(command->doc == NULL){
		free(command);
		free(command->name);
		return Malloc_exp;
	}
	command->next = NULL;

	/* we will insert the command node into the head of list */
	command->next = command_list;
	command_list = command;

	return No_exp;
}

void delete_command(char* command_str){
	COMMAND* node;
	node = command_list;
	if(command_str && node){
		for(;!node; node = node->next){
			if(!strncmp(node->name, command_str, strlen(node->name))){
			}
		}
	}
}
/* Look up NAME as the name of a command, and return a pointer to that
   command.  Return a NULL pointer if NAME isn't a command name. */
COMMAND *
find_command (name)
     char *name;
{
  COMMAND *node;
  if(name == NULL)
	return ((COMMAND *)NULL);
  node = command_list;
  while(node){
	if(!strncmp(name, node->name, strlen(node->name))){
		return node;
	}
	node = node->next;
  }
  return ((COMMAND *)NULL);
}

exception_t run_command(char* command_str){
	COMMAND* command = find_command(command_str);
	if(command != NULL && command->func != NULL)
		(*command->func)(NULL);
	return No_exp;
}

/* Generator function for command completion.  STATE lets us know whether
   to start from scratch; without any state (i.e. STATE == 0), then we
   start at the top of the list. */
char *
command_generator (text, state)
     const char *text;
     int state;
{
  //static int list_index, len;
  char *name;
  static int len;
  static COMMAND * node;

  /* If this is a new word to complete, initialize now.  This includes
     saving the length of TEXT for efficiency, and initializing the index
     variable to 0. */
  if (!state)
    {
      len = strlen (text);
      node = command_list;
    }

  /* Return the next name which partially matches from the command list. */
  while (node && (name = node->name))
    {
      node = node->next;
      if(strncmp (name, text, len) == 0)
        return (dupstr(name));
    }

  /* If no names matched, then return NULL. */
  return ((char *)NULL);
}

/* Print out help for ARG, or for all of the commands if ARG is
   not present. */
com_help (arg)
     char *arg;
{
  int printed = 0;
  COMMAND * node;
  node = command_list;

  /* Return the next name which partially matches from the command list. */
  while (node)
  {
      if (!*arg && (strcmp (arg, node->name) == 0))
       {
          printf ("%s\t\t%s.\n", node->name, node->doc);
          printed++;
	  break;
        }
	node = node->next;
  }
  if (!printed)
  {
    printf ("No commands match '%s'.  Possibilties are:\n", arg);
    node = command_list;
    while (node){
          printf ("%s : %s\n", node->name, node->doc);
	  node = node->next;
    }
  }
  return (0);
}
