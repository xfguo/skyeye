s3c2410 = s3c2410x/skyeye_mach_s3c2410x.c s3c2410x/s3c2410x_module.c
s3c6410 = s3c6410x/skyeye_mach_s3c6410x.c s3c6410x/s3c6410x_module.c
at91 = at91/skyeye_mach_at91.c at91/at91_module.c
at91rm92 = at91rm92/skyeye_mach_at91rm92.c at91rm92/at91rm92_module.c


libs3c2410_la_SOURCES = $(s3c2410)
libs3c2410_la_LDFLAGS = -module

libs3c6410_la_SOURCES = $(s3c6410)
libs3c6410_la_LDFLAGS = -module

libat91rm92_la_SOURCES = $(at91rm92)
libat91rm92_la_LDFLAGS = -module

libat91_la_SOURCES =  $(at91)
libat91_la_LDFLAGS = -module


pkglib_LTLIBRARIES = libs3c6410.la libs3c2410.la libat91rm92.la libat91.la

DEFS = -g -O2 -D_FILE_OFFSET_BITS=64 -DSTANDALONE -DDEFAULT_INLINE=0 

INCLUDES = -I$(top_srcdir)/arch/arm/common -I$(top_srcdir)/common @COMMON_INCLUDES@ 
