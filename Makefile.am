
#SUBDIRS =  arch/arm/ arch/bfin arch/coldfire arch/mips arch/ppc/ arch/x86/ device/ utils/ arch/sparc/ libltdl common/ soc/arm/
SUBDIRS =  device/ utils/  libltdl common/ soc/arm/

if BUILD_ARM
SUBDIRS += arch/arm
endif

if BUILD_BFIN
SUBDIRS += arch/bfin
endif


if BUILD_COLDFIRE
SUBDIRS += arch/coldfire
endif


if BUILD_MIPS
SUBDIRS += arch/mips
endif


if BUILD_SPARC
SUBDIRS += arch/sparc
endif


if BUILD_PPC
SUBDIRS += arch/ppc
endif


if BUILD_X86
SUBDIRS += arch/x86
endif

if BUILD_ALL
SUBDIRS += arch/arm/ arch/bfin arch/coldfire arch/mips arch/ppc/ arch/x86/ arch/sparc
endif

DIST_SUBDIRS = $(SUBDIRS)


#AM_LDFLAGS = $(LIBLS) --start-group $(top_srcdir)/arch/arm/libarm.a $(top_srcdir)/arch/bfin/libbfin.a $(top_srcdir)/arch/coldfire/libcoldfire.a $(top_srcdir)/arch/mips/libmips.a $(top_srcdir)/arch/ppc/libppc.a $(top_srcdir)/device/libdev.a $(top_srcdir)/utils/libutils.a --end-group
#AM_LDFLAGS = $(LIBLS) -L$(top_srcdir)/arch/arm/ -L$(top_srcdir)/arch/bfin/ -L$(top_srcdir)/arch/coldfire/ -L$(top_srcdir)/arch/mips/ -L$(top_srcdir)/device/ -L$(top_srcdir)/utils/ --start-group  -larm -lbfin -lcoldfire  -lmips -L$(top_srcdir)/arch/ppc/ -lppc -ldev -lutils --end-group

#skyeye_LDADD =  -L$(top_srcdir)/common -lcommon -L$(top_srcdir)/third-party/bfd/.libs/ -lbfd
#skyeye_LDADD =  
#skyeye_LDADD =  $(top_srcdir)/arch/arm/libarm.a $(top_srcdir)/device/libdev.a $(top_srcdir)/arch/mips/libmips.a $(top_srcdir)/arch/ppc/libppc.a $(top_srcdir)/arch/bfin/libbfin.a $(top_srcdir)/arch/mips/libmips.a $(top_srcdir)/arch/coldfire/libcoldfire.a $(top_srcdir)/arch/sparc/libsparc.a $(top_srcdir)/memory/libmemory.a $(top_srcdir)/third-party/bfd/libbfd.a $(top_srcdir)/third-party/libiberty/libiberty.a $(top_srcdir)/third-party/readline/libreadline.a

# workaround getopt issue on Cygwin platform
#skyeye_LDADD = -lc
# add curses library for ubentu system.
# skyeye_LDADD += -lncurses

# readline library need ncurses

# some functions in "libutils.a" which is required by other libs
#skyeye_LDADD += $(top_srcdir)/utils/libutils.a
#skyeye_LDADD += 
#skyeye_LDFLAGS = -L./common -lcommon
#if LLVM_EXIST
#skyeye_LDFLAGS += `llvm-config --libs ` -lrt -L`llvm-config --libdir`
#endif


INCLUDES = @COMMON_INCLUDES@ -Ithird-party/bfd/
DEFS = -DMODET -g -O2 -D_FILE_OFFSET_BITS=64 -DSTANDALONE -DDEFAULT_INLINE=0 @COMMON_CFLAGS@ 
AM_CFLAGS = -W -Wall -Wstrict-prototypes -Wmissing-prototypes 
ACLOCAL_AMFLAGS=-I m4
bin_PROGRAMS = uart_instance mknandflashdump prof_convert

#skyeye_SOURCES = utils/main/skyeye.c
uart_instance_SOURCES = utils/uart_console/uart_console.c
mknandflashdump_SOURCES = utils/nandflash_dump/mknandflashdump.c
prof_convert_SOURCES = utils/code_cov/prof_convert.c
if LCD
#skyeye_LDADD += @LCD_LIBS@
endif
all:third-party/bfd/.libs/libbfd.so third-party/libiberty/libiberty.a third-party/opcodes/.libs/libopcodes.so

skyeye_gui:
	cd gui/qt4_src && rm -f skyeye_gui && make
	cp -f gui/qt4_src/skyeye-gui $(prefix)/bin/

third-party/bfd/.libs/libbfd.so:
	mkdir -p third-party/bfd/ && cd third-party/bfd/ && ./../../$(top_srcdir)/third-party/bfd/configure --prefix=$(prefix) --disable-werror --enable-shared --enable-targets=all;make;

third-party/libiberty/libiberty.a:
	mkdir -p third-party/libiberty && cd third-party/libiberty && ./../../$(srcdir)/third-party/libiberty/configure --prefix=$(prefix) --enable-shared ;make;

third-party/opcodes/.libs/libopcodes.so:
	mkdir -p third-party/opcodes/ && cd third-party/opcodes/ && ./../../$(top_srcdir)/third-party/opcodes/configure --prefix=$(prefix) --disable-werror --enable-shared --enable-targets=all;make;

lib:
	mkdir -p third-party/libiberty && cd third-party/libiberty && ./../../$(srcdir)/third-party/libiberty/configure --prefix=$(prefix) --enable-shared ;make;
	mkdir -p third-party/bfd/ && cd third-party/bfd/ && ./../../$(top_srcdir)/third-party/bfd/configure --prefix=$(prefix) --disable-werror --enable-shared --enable-targets=all;make;
	mkdir -p third-party/opcodes/ && cd third-party/opcodes/ && ./../../$(top_srcdir)/third-party/opcodes/configure --prefix=$(prefix) --disable-werror --enable-shared --enable-targets=all;make;
#cd $(top_srcdir)/third-party/bfd/ && ./configure --prefix=$(prefix) --disable-werror --enable-shared --enable-targets=all;make;
#cd $(top_srcdir)/third-party/opcodes/ && ./configure --prefix=$(prefix) --disable-werror --enable-shared --enable-targets=all;make;
#cd $(top_srcdir)/third-party/readline/ && ./configure --prefix=$(prefix) --enable-shared;make;
build_lib:
	cd $(top_builddir)/third-party/libiberty && make;
	cd $(top_builddir)/third-party/bfd/ && make;
	cd $(top_builddir)third-party/opcodes/ && make;
install_lib:
	cd $(top_builddir)/third-party/libiberty && make install
	cd $(top_builddir)/third-party/bfd/ && make install
	cd $(top_builddir)/third-party/opcodes/ && make install

clean_lib:
	cd $(top_builddir)/third-party/libiberty && make distclean
	cd $(top_builddir)/third-party/bfd/ && make distclean
	cd $(top_builddir)/third-party/opcodes/ && make distclean
install-data-hook:
	rm -f -r $(prefix)/testsuite && mkdir $(prefix)/testsuite
	cp -a $(top_srcdir)/testsuite/arm_hello $(prefix)/testsuite/arm_hello/
	cp -a $(top_srcdir)/pyshell/* $(prefix)/bin/
#	rm -f -r $(prefix)/conf && mkdir $(prefix)/conf
#	cp -a $(top_srcdir)/conf/* $(prefix)/conf

# workaround for bfd library. we need to reconfigure it.
all-local:
#	cd gui/qt4_src && $(MAKE) $(AM_MAKEFLAGS) all
#	cd third-party/bfd/ && ./configure --prefix=$(prefix) --disable-werror --enable-shared --enable-targets=all;make;
#	cd third-party/opcodes/ && ./configure --prefix=$(prefix) --disable-werror --enable-shared --enable-targets=all;make;make install
install-exec-local:
#	cd third-party/bfd/ && make install
#	cd third-party/opcodes/ && make install
