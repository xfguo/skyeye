bfin_mach = mach/bf533_io.c  mach/bf537_io.c
bfin_comm = common/bfin_arch_interface.c  common/bfin-dis.c  common/iomem.c common/bfin_module.c
libbfin_la_SOURCES = $(bfin_comm) $(bfin_mach)
pkglib_LTLIBRARIES = libbfin.la
# we will link the common module with main
libbfin_la_LDFLAGS = -module
INCLUDES = -I./common -I./mach @COMMON_INCLUDES@
