/**
  * @file ppc_syscall.h
  *
  * The header of powerpc system call for simulate user mode
  *
  * @author Michael.Kang
  * @Version
  * @Date: 2010-11-22
  */

#ifndef __PPC_SYSCALL_H__
#define __PPC_SYSCALL_H__

#ifdef __cplusplus
 extern "C" {
#endif

int ppc_syscall(e500_core_t* core);

#ifdef __cplusplus
}
#endif

#endif
