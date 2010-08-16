s3c2410 = s3c2410x/skyeye_mach_s3c2410x.c s3c2410x/s3c2410x_module.c

libs3c2410_la_SOURCES = $(s3c2410)
libs3c2410_la_LDFLAGS = -module

pkglib_LTLIBRARIES = libs3c2410.la

DEFS = -g -O2 -D_FILE_OFFSET_BITS=64 -DSTANDALONE -DDEFAULT_INLINE=0 

INCLUDES =  $(top_srcdir)/common @COMMON_INCLUDES@ 
