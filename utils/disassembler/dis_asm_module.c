#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "skyeye_types.h"
#include "skyeye_command.h"
#include "skyeye_arch.h"
#include "skyeye_module.h"
const char* skyeye_module = "disassemble";

const int INSN_LENGTH = 4;

int dis_getcount(char* args, generic_address_t *addr);
int dis_getaddr(char* args, generic_address_t *count);
void disassemble(generic_address_t addr);

int com_disassemble(char* arg){
	char** endptr;
	generic_address_t addr;
	generic_address_t count = 1;
	int result;
	int i = 0;

        if(arg == NULL || *arg == '\0'){
		generic_arch_t* arch_instance = get_arch_instance(NULL);
		addr = arch_instance->get_pc();
        }
        else{
		result = dis_getaddr(arg, &addr);
		if(result == -1){
			return 0;
		}else if(addr == 0){
			generic_arch_t* arch_instance = get_arch_instance(NULL);
			addr = arch_instance->get_pc();
		}
		result = dis_getcount(arg, &count);
		if(result == -1 || count == 0)
			count = 1;

        }
#if 0
		addr = strtoul(arg, endptr, 16);
		if(**endptr != '\0'){
			printf("Not valid address format.\n");
			return 1;
		}
		errno = 0;
		addr = strtoul(arg, NULL, 16);
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
	/* we will disassemble count instructions once */
	for(i = 0; i < count; i++)
		disassemble(addr + (i * INSN_LENGTH));
}

void module_init(){
	//init_disassemble();
	add_command("disassemble", com_disassemble, "Disassemble the given address.\n");
}

void module_fini(){
}
