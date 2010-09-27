s3c2410 = s3c2410x/skyeye_mach_s3c2410x.c s3c2410x/s3c2410x_module.c
s3c6410 = s3c6410x/skyeye_mach_s3c6410x.c s3c6410x/s3c6410x_module.c


libs3c2410_la_SOURCES = $(s3c2410)
libs3c2410_la_LDFLAGS = -module

libs3c6410_la_SOURCES = $(s3c6410)
libs3c6410_la_LDFLAGS = -module

pkglib_LTLIBRARIES = libs3c6410.la libs3c2410.la 

DEFS = -g -O2 -D_FILE_OFFSET_BITS=64 -DSTANDALONE -DDEFAULT_INLINE=0 

INCLUDES = -I$(top_srcdir)/arch/arm/common -I$(top_srcdir)/common @COMMON_INCLUDES@ 
