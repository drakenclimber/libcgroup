@CODE_COVERAGE_RULES@

INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/include
LDADD = $(top_builddir)/src/.libs/libcgroup.la

bin_PROGRAMS = cgxget

cgxget_SOURCES = cgxget.c abstraction-common.c abstraction-common.h \
		 ctrl-cpu.c
cgxget_LIBS = $(CODE_COVERAGE_LIBS)
cgxget_CFLAGS = $(CODE_COVERAGE_CFLAGS)
