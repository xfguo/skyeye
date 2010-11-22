/**
  * @file ppc_syscall.c
  *
  * The implementation of powerpc system call for simulate user mode
  *
  * @author Michael.Kang
  * @Version
  * @Date: 2010-11-22
  */

/**
* @brief The implementation of system call for user mode
*
* @param core the e500 core.
*
* @return 
*/
#include "ppc_cpu.h"
int ppc_syscall(e500_core_t* core){
	int syscall_number = core->gpr[3];
	switch(syscall_number){
		default:
			printf("In %s, syscall number is %d, not implemented.\n", __FUNCTION__, core->gpr[3]);
			exit(-1);
	}
	return 0;
}
