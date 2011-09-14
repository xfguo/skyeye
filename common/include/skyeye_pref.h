#ifndef __SKYEYE_DEFS_H__
#define __SKYEYE_DEFS_H__
#include <termios.h>
#include "skyeye_types.h"
#include "skyeye_exec_info.h"

#ifdef __cplusplus
 extern "C" {
#endif 

void set_exec_file(const char *filename);
char *get_exec_file();

void set_exec_load_base(const generic_address_t addr);
generic_address_t get_exec_load_base();

void set_exec_load_mask(const uint32_t addr);
uint32_t get_exec_load_mask();

void set_conf_filename(const char *filename);
char *get_conf_filename();

void set_interactive_mode(const bool_t mode);
bool_t get_interactive_mode();

void set_endian(const endian_t endian);
bool_t get_endian();

void set_autoboot(const bool_t value);
bool_t get_autoboot();

void set_uart_port(const uint32_t value);
uint32_t get_uart_port();

bool_t get_user_mode();
void set_user_mode(const uint32_t value);
struct _sky_pref_s{
	//generic_address_t elf_load_base;
	//uinteger_t elf_load_mask;
	/* */
	char* module_search_dir;
	generic_address_t start_address;
	//char *elf_file;
	bool_t remote_debugmode;

	char* exec_file;
	generic_address_t exec_load_base;
	uint32_t exec_load_mask;

	char* conf_filename;
	bool_t interactive_mode;
	endian_t endian;

	/* FIXME sky_exec_info_t is inside sky_pref_t, to avoid
	   the complexity of allocating memory separately 
	   (it is after all attached to exec_file)
	   It is not very clean for now so maybe we should put it elsewhere */
	sky_exec_info_t info;

	/*
	 * if true, we will run the simulator loop before cli.
	 */
	bool_t autoboot;

	bool_t user_mode_sim;

	uint32 uart_port;

	struct termios saved_term;
};
typedef struct _sky_pref_s sky_pref_t;

sky_pref_t * get_skyeye_pref();
char* get_conf_filename();

#ifdef __cplusplus
}
#endif 

#endif
