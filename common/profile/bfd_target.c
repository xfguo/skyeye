#include <stdlib.h>
#include <stdio.h>
#include <string.h>
const char* elf32_powerpc = "elf32-powerpc";
const char* elf32_littlearm = "elf32-littlearm";
const char* elf32_bigarm = "elf32-bigarm";
char* get_bfd_target(const char* arch_name){
	if(strcmp("powerpc", arch_name) == 0){
		return elf32_powerpc;
	}
	else if(strcmp("arm", arch_name) == 0 ){
		return elf32_littlearm;
	}
	else
		return NULL;
}
