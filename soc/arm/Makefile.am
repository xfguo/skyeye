s3c2410 = s3c2410x/skyeye_mach_s3c2410x.c s3c2410x/s3c2410x_module.c
s3c6410 = s3c6410x/skyeye_mach_s3c6410x.c s3c6410x/s3c6410x_module.c
at91 = at91/skyeye_mach_at91.c at91/at91_module.c
at91rm92 = at91rm92/skyeye_mach_at91rm92.c at91rm92/at91rm92_module.c
cs89712 = cs89712/skyeye_mach_cs89712.c cs89712/cs89712_module.c
ep9312 = ep9312/skyeye_mach_ep9312.c ep9312/ep9312_module.c
ep7312 = ep7312/skyeye_mach_ep7312.c ep7312/ep7312_module.c
s3c2440 = s3c2440/skyeye_mach_s3c2440.c s3c2440/s3c2440_module.c
sa1100 = sa1100/skyeye_mach_sa1100.c sa1100/sa1100_module.c
pxa270 = pxa270/skyeye_mach_pxa270.c pxa270/pxa270_module.c


libs3c2410_la_SOURCES = $(s3c2410)
libs3c2410_la_LDFLAGS = -module

libs3c6410_la_SOURCES = $(s3c6410)
libs3c6410_la_LDFLAGS = -module

libat91rm92_la_SOURCES = $(at91rm92)
libat91rm92_la_LDFLAGS = -module

libat91_la_SOURCES =  $(at91)
libat91_la_LDFLAGS = -module

libcs89712_la_SOURCES = $(cs89712)
libcs89712_la_LDFLAGS = -module

libep9312_la_SOURCES = $(ep9312)
libep9312_la_LDFLAGS = -module

libep7312_la_SOURCES = $(ep7312)
libep7312_la_LDFLAGS = -module

libs3c2440_la_SOURCES = $(s3c2440)
libs3c2440_la_LDFLAGS = -module

libsa1100_la_SOURCES = $(sa1100)
libsa1100_la_LDFLAGS = -module

libpxa270_la_SOURCES = $(pxa270)
libpxa270_la_LDFLAGS = -module

pkglib_LTLIBRARIES = libs3c6410.la libs3c2410.la libat91rm92.la libat91.la libcs89712.la \
libep9312.la libep7312.la libs3c2440.la libsa1100.la libpxa270.la

DEFS = -g -O2 -D_FILE_OFFSET_BITS=64 -DSTANDALONE -DDEFAULT_INLINE=0 

INCLUDES = -I$(top_srcdir)/arch/arm/mach -I$(top_srcdir)/common @COMMON_INCLUDES@
