libdyncom= translate_singlestep_bb.cpp translate_singlestep.cpp translate_all.cpp translate.cpp timings.cpp tag.cpp stat.cpp sha1.cpp optimize.cpp interface.cpp function.cpp frontend.cpp fp.cpp disasm.cpp basicblock.cpp memory.cpp

DEFS= -I$(top_srcdir) -I$(top_srcdir)/common/include -I./ `llvm-config --cflags`
# We have to use stdc++ library in simics-4.0. Or compilation conflict.
libdyncom_la_LDFLAGS=  `/usr/local/bin/llvm-config --libs --libdir` -lrt

libdyncom_la_SOURCES =  $(libdyncom)
pkglib_LTLIBRARIES = libdyncom.la
