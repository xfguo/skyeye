#ifndef __DEFAULT_COMMANDS_H__
#define __DEFAULT_COMMANDS_H__
int com_run(char* arg);
int com_cont(char* arg);
int com_stop(char* arg);
int com_si(char* arg);
int com_view(char* arg);
int com_rename(char* arg);
int com_delete(char* arg);
int com_list(char* arg);
int com_stat(char* arg);
int com_start(char* arg);
int com_cd(char* arg);
int com_pwd(char* arg);
int com_quit(char* arg);
int com_list_modules(char* arg);
int com_list_options(char* arg);
int com_list_machines(char* arg);
int com_show_pref(char* arg);
int com_load_module(char* arg);
int com_load_conf(char* arg);
int com_show_map(char* arg);
int com_info(char* arg);
int com_x(char* arg);
exception_t run_command(char* command_str);
#endif
