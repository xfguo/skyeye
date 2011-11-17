ppc_mach = mach/skyeye_mach_mpc8560.c mach/skyeye_mach_mpc8572.c mach/mpc8560_boot_linux.c mach/mpc8572_boot_linux.c mach/skyeye_mach_mpc8641d.c mach/mpc8641d_boot_linux.c mach/mpc8641d_boot_vxworks.c
ppc_comm = common/ppc_alu.c common/ppc_cpu.c common/ppc_exc.c  common/ppc_mmu.c    common/ppc_vec.c \
common/ppc_arch_interface.c  common/ppc_dec.c       common/ppc_fpu.c  common/ppc_opc.c \
common/ppc_e500_core.c	      common/ppc_cpm.c						\
common/ppc_e500_exc.c  common/ppc_io.c   common/ppc_tools.c common/ppc_e500_mpic.c common/ppc_module.c
ppc_dyncom = dyncom/ppc_dyncom_run.cpp dyncom/ppc_dyncom_translate.cpp dyncom/ppc_dyncom_interface.cpp dyncom/ppc_dyncom_instr_group1.cpp dyncom/ppc_dyncom_instr_group2.cpp dyncom/ppc_dyncom_instr_group_f1.cpp dyncom/ppc_dyncom_instr_group_f2.cpp dyncom/ppc_dyncom_instr_main.cpp dyncom/ppc_dyncom_instr_groupv.cpp dyncom/ppc_dyncom_dec.cpp dyncom/ppc_dyncom_diff_log.cpp dyncom/ppc_dyncom_parallel.cpp

ppc_linux_syscall = linux_syscall/ppc_syscall.c
#libppc_a_SOURCES = $(ppc_mach) $(ppc_comm)
libppc_la_SOURCES = $(ppc_mach) $(ppc_comm) $(ppc_linux_syscall)

if LLVM_EXIST
libppc_la_SOURCES += $(ppc_dyncom)
endif

pkglib_LTLIBRARIES = libppc.la
# we will link the common module with main
libppc_la_LDFLAGS = -module
if WIN32
libppc_la_LDFLAGS += -no-undefined -shared -module  -Xlinker -L../../common/.libs/ -Xlinker -lcommon -Xlinker -lmsys-1.0 -Xlinker -lmsvcrt -Xlinker -lmsys-1.0 -Xlinker -lkernel32 -Xlinker -lole32 -Xlinker -lkernel32 -Xlinker -lgdi32 -Xlinker -luuid -Xlinker -luser32  -Xlinker -lshell32 -Xlinker --export-all-symbol
endif
#DEFS = -DMODET -g -O2 -D_FILE_OFFSET_BITS=64 -DSTANDALONE -DDEFAULT_INLINE=0
DEFS = @COMMON_CFLAGS@
if LLVM_EXIST
DEFS += -DLLVM_EXIST
endif

#noinst_LIBRARIES =      libppc.a
INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/dyncom/ -I$(top_srcdir)/arch/ppc/common -I$(top_srcdir)/arch/ppc/linux_syscall `/usr/local/bin/llvm-config --cflags` -I$(top_srcdir)/utils/debugger @COMMON_INCLUDES@
