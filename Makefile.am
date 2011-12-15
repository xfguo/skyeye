
#SUBDIRS =  arch/arm/ arch/bfin arch/coldfire arch/mips arch/ppc/ arch/x86/ device/ utils/ arch/sparc/ libltdl common/ soc/arm/
SUBDIRS =  common/ device/ utils/

if BUILD_ARM
SUBDIRS += arch/arm
SUBDIRS += soc/arm
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

if BUILD_DEFAULT
SUBDIRS += arch/ppc arch/arm soc/arm
default_targets=arm, ppc
endif

if BUILD_ALL
SUBDIRS += arch/arm/ soc/arm arch/bfin arch/coldfire arch/mips arch/ppc/ arch/x86/ arch/sparc
endif

DIST_SUBDIRS = $(SUBDIRS)


#AM_LDFLAGS = $(LIBLS) --start-group $(top_srcdir)/arch/arm/libarm.a $(top_srcdir)/arch/bfin/libbfin.a $(top_srcdir)/arch/coldfire/libcoldfire.a $(top_srcdir)/arch/mips/libmips.a $(top_srcdir)/arch/ppc/libppc.a $(top_srcdir)/device/libdev.a $(top_srcdir)/utils/libutils.a --end-group
#AM_LDFLAGS = $(LIBLS) -L$(top_srcdir)/arch/arm/ -L$(top_srcdir)/arch/bfin/ -L$(top_srcdir)/arch/coldfire/ -L$(top_srcdir)/arch/mips/ -L$(top_srcdir)/device/ -L$(top_srcdir)/utils/ --start-group  -larm -lbfin -lcoldfire  -lmips -L$(top_srcdir)/arch/ppc/ -lppc -ldev -lutils --end-group

skyeye_LDADD =  -L./common -lcommon
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
if LLVM_EXIST
#skyeye_LDFLAGS += `llvm-config --libs ` -lrt -L`llvm-config --libdir`
endif

INCLUDES = @COMMON_INCLUDES@ -Ithird-party/bfd/
DEFS = -DMODET -g -O2 -D_FILE_OFFSET_BITS=64 -DSTANDALONE -DDEFAULT_INLINE=0 @COMMON_CFLAGS@ 
AM_CFLAGS = -W -Wall -Wstrict-prototypes -Wmissing-prototypes 
ACLOCAL_AMFLAGS=-I m4

bin_PROGRAMS = skyeye
skyeye_SOURCES = utils/main/skyeye.c
if WIN32
uart_instance_SOURCES =
mknandflashdump_SOURCES =
prof_convert_SOURCES =
else
bin_PROGRAMS += uart_instance mknandflashdump prof_convert
uart_instance_SOURCES = utils/uart_console/uart_console.c
mknandflashdump_SOURCES = utils/nandflash_dump/mknandflashdump.c
prof_convert_SOURCES = utils/code_cov/prof_convert.c
endif
if LCD
#skyeye_LDADD += @LCD_LIBS@
endif
if LINUX
all:third-party/libiberty/libiberty.a third-party/bfd/.libs/libbfd.so third-party/opcodes/.libs/libopcodes.so
endif

skyeye_gui:
	cd gui/qt4_src && rm -f skyeye_gui && $(MAKE)
	cp -f gui/qt4_src/skyeye-gui $(prefix)/bin/

third-party/bfd/.libs/libbfd.so:
	mkdir -p third-party/bfd/ && cd third-party/bfd/ && ./../../$(top_srcdir)/third-party/bfd/configure --prefix=$(prefix) --disable-werror --enable-shared --enable-targets=all;$(MAKE);

third-party/libiberty/libiberty.a:
	mkdir -p third-party/libiberty && cd third-party/libiberty && ./../../$(srcdir)/third-party/libiberty/configure --prefix=$(prefix) --enable-shared ;$(MAKE);

third-party/opcodes/.libs/libopcodes.so:
	mkdir -p third-party/opcodes/ && cd third-party/opcodes/ && ./../../$(top_srcdir)/third-party/opcodes/configure --prefix=$(prefix) --disable-werror --enable-shared --enable-targets=all;$(MAKE);

libltdl/.lib/libltdl.a:
	mkdir -p libltdl/ && cd libltdl/ && ../$(top_srcdir)/libltdl/configure;$(MAKE);

lib:
	mkdir -p third-party/libiberty && cd third-party/libiberty && ./../../$(srcdir)/third-party/libiberty/configure --prefix=$(prefix) --enable-shared ;$(MAKE);
	mkdir -p third-party/bfd/ && cd third-party/bfd/ && ./../../$(top_srcdir)/third-party/bfd/configure --prefix=$(prefix) --disable-werror --enable-shared --enable-targets=all;$(MAKE);
	mkdir -p third-party/opcodes/ && cd third-party/opcodes/ && ./../../$(top_srcdir)/third-party/opcodes/configure --prefix=$(prefix) --disable-werror --enable-shared --enable-targets=all;$(MAKE);
	mkdir -p libltdl/ && cd libltdl/ && ../$(top_srcdir)/libltdl/configure;$(MAKE);
#cd $(top_srcdir)/third-party/bfd/ && ./configure --prefix=$(prefix) --disable-werror --enable-shared --enable-targets=all;make;
#cd $(top_srcdir)/third-party/opcodes/ && ./configure --prefix=$(prefix) --disable-werror --enable-shared --enable-targets=all;make;
#cd $(top_srcdir)/third-party/readline/ && ./configure --prefix=$(prefix) --enable-shared;make;
build_lib:
	cd $(top_builddir)/third-party/libiberty && $(MAKE);
	cd $(top_builddir)/third-party/bfd/ && $(MAKE);
	cd $(top_builddir)third-party/opcodes/ && $(MAKE);
	cd $(top_builddir)/libltdl/ && make;
install_lib:
	cd $(top_builddir)/third-party/libiberty && $(MAKE) install
	cd $(top_builddir)/third-party/bfd/ && $(MAKE) install
	cd $(top_builddir)/third-party/opcodes/ && $(MAKE) install

clean_lib:
	cd $(top_builddir)/third-party/libiberty && make distclean
	cd $(top_builddir)/third-party/bfd/ && make distclean
	cd $(top_builddir)/third-party/opcodes/ && make distclean
	cd $(top_builddir)/libltdl/ && make distclean;
install-data-hook:
	rm -f -r $(prefix)/testsuite && mkdir $(prefix)/testsuite
if WIN32
	mkdir -p $(prefix)/testsuite/arm_hello && mkdir -p $(prefix)/bin
	cp -a $(top_builddir)/common/.libs/libcommon-0.dll $(prefix)/lib/skyeye/
	cp -a $(top_builddir)/common/.libs/libcommon-0.dll $(prefix)/bin/
endif
	cp -a $(top_srcdir)/testsuite/arm_hello $(prefix)/testsuite/arm_hello/
	cp -a $(top_srcdir)/utils/pycli/*.py $(prefix)/bin/
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
