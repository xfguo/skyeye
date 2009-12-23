#include "skyeye_module.h"
const char* skyeye_module = "gdbserver";
void com_remote_gdb();
void module_init(){
	//init_register_type();
	//add_command("remote-gdb", com_remote_gdb, "Open remote gdb debugger.\n");
}
void module_fini(){
}

