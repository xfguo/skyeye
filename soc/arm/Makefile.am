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
pxa250 = pxa250/skyeye_mach_pxa250.c pxa250/pxa250_module.c
s3c3410x = s3c3410x/skyeye_mach_s3c3410x.c s3c3410x/s3c3410x_module.c
ns9750 = ns9750/skyeye_mach_ns9750.c ns9750/ns9750_module.c
sharp = sharp/skyeye_mach_sharp.c sharp/sharp_module.c
omap5912 = omap5912/skyeye_mach_omap5912.c omap5912/omap5912_module.c
lpc2210 = lpc2210/skyeye_mach_lpc2210.c lpc2210/lpc2210_module.c
lh79520 = lh79520/skyeye_mach_lh79520.c lh79520/lh79520_module.c
ps7500 = ps7500/skyeye_mach_ps7500.c ps7500/ps7500_module.c
imx9238 = imx9238/skyeye_mach_imx9238.c imx9238/imx9238_module.c
s3c4510b = s3c4510b/skyeye_mach_s3c4510b.c s3c4510b/s3c4510b_module.c

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

libpxa250_la_SOURCES = $(pxa250)
libpxa250_la_LDFLAGS = -module

libs3c3410x_la_SOURCES = $(s3c3410x)
libs3c3410x_la_LDFLAGS = -module

libns9750_la_SOURCES = $(ns9750)
libns9750_la_LDFLAGS = -module

libsharp_la_SOURCES = $(sharp)
libsharp_la_LDFLAGS = -module

libomap5912_la_SOURCES = $(omap5912)
libomap5912_la_LDFLAGS = -module

liblpc2210_la_SOURCES = $(lpc2210)
liblpc2210_la_LDFLAGS = -module

liblh79520_la_SOURCES = $(lh79520)
liblh79520_la_LDFLAGS = -module

libps7500_la_SOURCES = $(ps7500)
libps7500_la_LDFLAGS = -module

libimx9238_la_SOURCES = $(imx9238)
libimx9238_la_LDFLAGS = -module

libs3c4510b_la_SOURCES = $(s3c4510b)
libs3c4510b_la_LDFLAGS = -module

if WIN32
WIN32_LD_COMMON = -no-undefined -shared -Xlinker -L../../common/.libs/ -L/mingw/lib -Xlinker -lcommon -Xlinker --export-all-symbol
libs3c2410_la_LDFLAGS += $(WIN32_LD_COMMON)
libs3c6410_la_LDFLAGS += $(WIN32_LD_COMMON)
libat91rm92_la_LDFLAGS += $(WIN32_LD_COMMON)
libat91_la_LDFLAGS += $(WIN32_LD_COMMON)
libcs89712_la_LDFLAGS += $(WIN32_LD_COMMON)
libep9312_la_LDFLAGS += $(WIN32_LD_COMMON)
libep7312_la_LDFLAGS += $(WIN32_LD_COMMON)
libs3c2440_la_LDFLAGS += $(WIN32_LD_COMMON)
libsa1100_la_LDFLAGS += $(WIN32_LD_COMMON)
libpxa270_la_LDFLAGS += $(WIN32_LD_COMMON)
libpxa250_la_LDFLAGS += $(WIN32_LD_COMMON)
libs3c3410x_la_LDFLAGS += $(WIN32_LD_COMMON)
libns9750_la_LDFLAGS += $(WIN32_LD_COMMON)
libsharp_la_LDFLAGS += $(WIN32_LD_COMMON)
libomap5912_la_LDFLAGS += $(WIN32_LD_COMMON)
liblpc2210_la_LDFLAGS += $(WIN32_LD_COMMON)
liblh79520_la_LDFLAGS += $(WIN32_LD_COMMON)
libps7500_la_LDFLAGS +=  $(WIN32_LD_COMMON)
libimx9238_la_LDFLAGS += $(WIN32_LD_COMMON)
libs3c4510b_la_LDFLAGS += $(WIN32_LD_COMMON)
endif

pkglib_LTLIBRARIES = libs3c6410.la libs3c2410.la libat91rm92.la libat91.la libcs89712.la \
libep9312.la libep7312.la libs3c2440.la libsa1100.la libpxa270.la libpxa250.la libs3c3410x.la \
libns9750.la libsharp.la  libomap5912.la liblpc2210.la liblh79520.la libps7500.la libimx9238.la \
libs3c4510b.la

DEFS = -g -O2 -D_FILE_OFFSET_BITS=64 -DSTANDALONE -DDEFAULT_INLINE=0 @COMMON_CFLAGS@
if GTK_LCD
DEFS += -DGTK_LCD
endif

INCLUDES = -I$(top_srcdir)/arch/arm/mach -I$(top_srcdir)/common @COMMON_INCLUDES@
