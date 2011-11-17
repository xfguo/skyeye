#ifndef __SKYEYE_CLI_H__
#define __SKYEYE_CLI_H__
#ifdef __WIN32__
extern int setenv(const char *name, const char *value, int replace);
#endif
typedef int(*cli_func_t)(const char *prompt); 
void register_cli(cli_func_t cli);
#endif
