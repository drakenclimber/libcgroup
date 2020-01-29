@CODE_COVERAGE_RULES@

INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/include
LDADD = $(top_builddir)/src/.libs/libcgroup.la \
	$(top_builddir)/src/tools/.libs/libcgrouptools.la

bin_PROGRAMS = cgxget cgxset

cgxget_SOURCES = cgxget.c abstraction-common.c abstraction-common.h \
		 ctrl-cpu.c ctrl-cpuset.c
cgxget_LIBS = $(CODE_COVERAGE_LIBS)
cgxget_CFLAGS = $(CODE_COVERAGE_CFLAGS)

cgxset_SOURCES = cgxset.c abstraction-common.c abstraction-common.h \
		 ctrl-cpu.c ctrl-cpuset.c
cgxset_LIBS = $(CODE_COVERAGE_LIBS)
cgxset_CFLAGS = $(CODE_COVERAGE_CFLAGS)
