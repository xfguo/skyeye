ppc_mach = mach/skyeye_mach_mpc8560.c mach/skyeye_mach_mpc8572.c mach/mpc8560_boot_linux.c mach/mpc8572_boot_linux.c mach/skyeye_mach_mpc8641d.c mach/mpc8641d_boot_linux.c
ppc_comm = common/ppc_alu.c common/ppc_cpu.c common/ppc_exc.c  common/ppc_mmu.c    common/ppc_vec.c \
common/ppc_arch_interface.c  common/ppc_dec.c       common/ppc_fpu.c  common/ppc_opc.c \
common/ppc_e500_core.c	      common/ppc_cpm.c						\
common/ppc_e500_exc.c  common/ppc_io.c   common/ppc_tools.c common/ppc_e500_mpic.c common/ppc_module.c
ppc_dyncom = dyncom/ppc_dyncom_run.cpp dyncom/ppc_dyncom_translate.cpp
#libppc_a_SOURCES = $(ppc_mach) $(ppc_comm)
libppc_la_SOURCES = $(ppc_mach) $(ppc_comm) $(ppc_dyncom)
pkglib_LTLIBRARIES = libppc.la
# we will link the common module with main
libppc_la_LDFLAGS = -module
#DEFS = -DMODET -g -O2 -D_FILE_OFFSET_BITS=64 -DSTANDALONE -DDEFAULT_INLINE=0
#noinst_LIBRARIES =      libppc.a
INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/dyncom/ -I$(top_srcdir)/arch/ppc/common `llvm-config --cflags` -I$(top_srcdir)/utils/debugger @COMMON_INCLUDES@
