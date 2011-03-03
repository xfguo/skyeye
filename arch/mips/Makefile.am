mips_mach = mach/skyeye_mach_au1100.c  mach/skyeye_mach_nedved.c mach/skyeye_mach_fulong.c mach/skyeye_mach_gs32eb1.c
mips_comm = common/cache.c   common/decoder.c    common/icache.c common/mipsio.c    common/tlb.c \
common/cp0.c     common/emul.c       common/interrupt.c            common/mipsmem.c \
common/dcache.c  common/exception.c  common/mips_arch_interface.c  common/multiply.c common/mips_module.c common/mips_regformat.c
mips_dyncom = dyncom/mips_dyncom_run.cpp dyncom/mips_dyncom_translate.cpp dyncom/mips_dyncom_interface.cpp dyncom/mips_dyncom_dec.cpp

libmips_la_SOURCES = $(mips_mach) $(mips_comm) $(mips_dyncom)
pkglib_LTLIBRARIES = libmips.la

# we will link the common module with main
libmips_la_LDFLAGS = -module
DEFS = -g -O2 -D_FILE_OFFSET_BITS=64 -DSTANDALONE -DDEFAULT_INLINE=0 -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS
INCLUDES = -I./common -I$(top_srcdir)/utils/debugger -I$(top_srcdir)/device/uart @COMMON_INCLUDES@
