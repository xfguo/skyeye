#ifndef __SKYEYE_CLI_H__
#define __SKYEYE_CLI_H__

typedef int(*cli_func_t)(const char *prompt); 
void register_cli(cli_func_t cli);
#endif
