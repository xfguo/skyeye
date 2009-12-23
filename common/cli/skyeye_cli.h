#ifndef __CLI_SKYEYE_CLI_H__
#define __CLI_SKYEYE_CLI_H__

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

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

#ifdef READLINE_LIBRARY
#  include "readline.h"
#  include "history.h"
#else
#  include <readline/readline.h>
#  include <readline/history.h>
#endif
#include "skyeye_types.h"

/* A structure which contains information on the commands this program
   can understand. */
struct command_s{
  char *name;			/* User printable name of the function. */
  rl_icpfunc_t *func;		/* Function to call to do the job. */
  char *doc;			/* Documentation for this function.  */
  struct command_s *next;
};
typedef struct command_s  COMMAND;

void skyeye_cli();

typedef int (*command_func_t)(char* arg);
exception_t add_command(char* command_name, command_func_t func, char* helper);
#endif
