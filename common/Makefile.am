# we will generate libcommon.so, and set its attribute to RTLD_LAZY|RTLD_GLOBAL) when use dlopen to load it.
common_checkpoint = checkpoint/check.c
common_breakpoint = breakpoint/breakpoint.c
common_misc = misc/support.c
common_module = module/skyeye_module.c
common_ctrl = ctrl/thread_ctrl.c ctrl/sim_ctrl.c ctrl/step.c ctrl/scheduler.c ctrl/skyeye_thread.c ctrl/skyeye_cell.c ctrl/skyeye_exec.c
common_conf_parser = conf_parser/skyeye_options.c conf_parser/skyeye_config.c conf_parser/misc_options.c conf_parser/conf_obj.c conf_parser/skyeye_class.c
common_log = log/skyeye_log.c
common_loader = loader/loader_elf.c loader/loader_file.c
common_mm = mm/skyeye_mm.c
common_cli = cli/skyeye_command.c cli/skyeye_cli.c cli/default_command.c
common_portable = portable/mman.c portable/usleep.c portable/gettimeofday.c
common_preference = preference/skyeye_pref.c
common_profile = profile/symbol.c profile/bfd_target.c
common_memory = bus/bank_ops.c  bus/io.c  bus/ram.c bus/flash.c bus/skyeye_bus.c bus/bus_recoder.c bus/addr_space.c
common_core = core/skyeye_arch.c
common_device = device/skyeye_device.c device/pen_buffer.c device/skyeye_uart_ops.c device/skyeye_signal.c
common_mach = mach/skyeye_mach.c
common_callback = callback/callback.c
common_dyncom= dyncom/translate_singlestep_bb.cpp dyncom/translate_singlestep.cpp dyncom/translate_all.cpp dyncom/translate.cpp dyncom/timings.cpp dyncom/tag.cpp dyncom/stat.cpp dyncom/sha1.cpp dyncom/optimize.cpp dyncom/interface.cpp dyncom/function.cpp dyncom/frontend.cpp dyncom/fp.cpp dyncom/disasm.cpp dyncom/basicblock.cpp dyncom/memory.cpp

pkglib_LTLIBRARIES = libcommon.la

include_HEADERS = ./include/sim_control.h ./include/skyeye_callback.h \
./include/skyeye_types.h ./include/skyeye_module.h ./include/int_register.h \
./include/skyeye_log.h ./include/skyeye_arch.h ./include/skyeye_loader.h \
./include/skyeye_uart_ops.h  ./include/memory_space.h ./include/skyeye_uart.h \
./include/breakpoint.h ./include/skyeye_api.h ./include/skyeye_misc.h \
./include/arch_regdefs.h ./include/skyeye_config.h ./include/skyeye_device.h \
./include/skyeye.h ./include/skyeye_mm.h ./include/skyeye_obj.h\
./include/skyeye_options.h ./include/skyeye_mach.h ./include/skyeye_pref.h \
./include/portable/mman.h ./include/portable/usleep.h ./include/skyeye_ram.h \
./include/skyeye_queue.h ./include/skyeye_signal.h ./include/skyeye_lock.h\
./include/skyeye_sched.h ./include/skyeye_addr_space.h ./include/bank_defs.h 

libcommon_la_SOURCES = $(common_module) $(common_misc) $(common_breakpoint) $(common_ctrl) $(common_portable) $(common_preference) $(common_core) $(common_conf_parser) $(common_log) $(common_cli) $(common_mm) $(common_mach) $(common_device) $(common_memory) $(common_loader) $(common_callback) $(common_profile) $(common_checkpoint)

libcommon_la_LIBADD = -L$(top_builddir)/third-party/bfd/.libs/ -lbfd -L$(top_builddir)/third-party/libiberty/pic -liberty @LIBLTDL@ @LIBADD_DL@

if LLVM_EXIST
libcommon_la_SOURCES += $(common_dyncom)
endif


if WIN32
else
#libcommon_la_LIBADD += -L$(top_builddir)/third-party/readline/
endif
# we will link the common module with main
#libcommon_la_LDFLAGS = -module
libcommon_la_LDFLAGS =

skyeye_module_dir = \"$(prefix)/lib/skyeye\"

DEFS = -g -O2 -D_FILE_OFFSET_BITS=64 -DSTANDALONE -DDEFAULT_INLINE=0 -DSKYEYE_MODULE_DIR=$(skyeye_module_dir) @COMMON_CFLAGS@

INCLUDES =  @COMMON_INCLUDES@ -I$(top_srcdir)/third-party/include/ -I$(top_srcdir)/third-party/bfd/ -I$(top_builddir)/third-party/bfd/ -I$(top_srcdir)/third-party @INCLTDL@
if LLVM_EXIST
INCLUDES+= `llvm-config --cflags`
libcommon_la_LDFLAGS += `llvm-config --libs ` -lrt -L`llvm-config --libdir`
endif

if LCD
INCLUDES += @LCD_CFLAGS@
endif 
