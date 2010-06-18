#include <stdio.h>
#include <errno.h>
#include "skyeye_types.h"
#include "skyeye_arch.h"
#include "skyeye_module.h"
const char* skyeye_module = "disassemble";

const int INSN_LENGTH = 4;

int com_disassemble(char* arg){
        char** endptr;
        generic_address_t addr;
        if(arg == NULL || *arg == '\0'){
                generic_arch_t* arch_instance = get_arch_instance(NULL);
                addr = arch_instance->get_pc();
        }
        else{
#if 0
                addr = strtoul(arg, endptr, 16);
                if(**endptr != '\0'){
                        printf("Not valid address format.\n");
                        return 1;
                }
#endif
		errno = 0;
                addr = strtoul(arg, NULL, 16);
#if 0
		if(errno != 0){
                        if(errno == EINVAL)
                                printf("Not valid address format.\n");
                        else if(errno == ERANGE)
                                printf("Out of range for your disassemble address.\n");
                        else
                                printf("Some unknown error in your disassemble command.\n");
                        return 1;
                }
#endif
        }
	/* we will disassemble one instructions once */
	int i = 0;
	for(i = 0; i < 1; i++)
		disassemble(addr + (i * INSN_LENGTH));
}

void module_init(){
	//init_disassemble();
	add_command("disassemble", com_disassemble, "Disassemble the given address.\n");
}

void module_fini(){
}
