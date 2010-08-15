#utils_config = config/skyeye_config.c  config/skyeye_options.c
utils_debugger = debugger/arch_regdefs.c debugger/gdbserver.c debugger/gdb_tracepoint.c  debugger/skyeye2gdb.c debugger/gdbserver_module.c debugger/arm_regdefs.c debugger/bfin_regdefs.c debugger/cf_regdefs.c debugger/mips_regdefs.c debugger/ppc_regdefs.c debugger/sparc_regdefs.c

utils_disassembler = disassembler/dis_asm.c disassembler/dis_asm_module.c

utils_code_cov = code_cov/code_cov.c code_cov/code_cov_module.c

utils_log = log/log.c log/log_module.c

utils_bus_log = bus_log/bus_log.c bus_log/bus_log_module.c

utils_pmon = perf_monitor/pmon.c perf_monitor/pmon_module.c

utils_shutdown = shutdown/shutdown_module.c

libgdbserver_la_SOURCES = $(utils_debugger)
libgdbserver_la_LDFLAGS = -module

libcodecov_la_SOURCES = $(utils_code_cov)   
libcodecov_la_LDFLAGS = -module

liblog_la_SOURCES = $(utils_log)   
liblog_la_LDFLAGS = -module

libpmon_la_SOURCES = $(utils_pmon)   
libpmon_la_LDFLAGS = -module

libbus_log_la_SOURCES = $(utils_bus_log)
libbus_log_la_LDFLAGS = -module

libshutdown_la_SOURCES = $(utils_shutdown)
libshutdown_la_LDFLAGS = -module

libdisasm_la_SOURCES = $(utils_disassembler)
libdisasm_la_LDFLAGS = -module -L./$(top_builddir)/third-party/opcodes/.libs/ -L./$(top_builddir)/third-party/bfd/.libs/ -L./$(top_builddir)/third-party/libiberty/pic/ -lopcodes -lbfd -liberty

pkglib_LTLIBRARIES = libcodecov.la libgdbserver.la libdisasm.la liblog.la libpmon.la libbus_log.la libshutdown.la
#pkglib_LTLIBRARIES = libutils.la libgdbserver.la

DEFS = -DMODET -g -O2 -D_FILE_OFFSET_BITS=64 -DSTANDALONE -DDEFAULT_INLINE=0 

INCLUDES =  -I$(top_srcdir)/third-party/include/ -I$(top_srcdir)/third-party/bfd/ -I$(top_srcdir)/common @COMMON_INCLUDES@ 
