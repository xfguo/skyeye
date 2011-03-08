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

#if 0
#ifdef READLINE_LIBRARY
#  include "readline.h"
#  include "history.h"
#else
#  include <readline/readline.h>
#  include <readline/history.h>
#endif
#endif
#include "skyeye_types.h"
#include "skyeye_module.h"
#include "skyeye_options.h"

typedef int icpfunc_t (char *);

/* A structure which contains information on the commands this program
   can understand. */
struct command_s{
  char *name;			/* User printable name of the function. */
  icpfunc_t *func;		/* Function to call to do the job. */
  char *doc;			/* Documentation for this function.  */
  struct command_s *next;
};
typedef struct command_s  COMMAND;
char *dupstr (char* s);

void initialize_readline ();
int execute_line (char* line);
COMMAND *find_command (char* name);
skyeye_module_t* get_module_list();
skyeye_option_t* get_option_list();
int valid_argument (char* caller, char *arg);
void too_dangerous (char *caller);
#endif
