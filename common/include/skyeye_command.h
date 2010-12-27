#ifndef __SKYEYE_COMMAND_H__
#define __SKYEYE_COMMAND_H__
void skyeye_cli();
typedef int (*command_func_t)(char* arg);
exception_t add_command(char* command_name, command_func_t func, char* helper);
exception_t run_command(char* command_str);
#endif
