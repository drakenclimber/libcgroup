@CODE_COVERAGE_RULES@

INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/include
LDADD = $(top_builddir)/src/.libs/libcgroup.la \
	$(top_builddir)/src/tools/.libs/libcgrouptools.la

bin_PROGRAMS = cgxget cgxset

lib_LTLIBRARIES = libcgroupabstraction.la
libcgroupabstraction_la_SOURCES = abstraction-common.c abstraction-common.h
libcgroupabstraction_la_CFLAGS = $(CODE_COVERAGE_CFLAGS) -DSTATIC=static
libcgroupabstraction_la_LDFLAGS = -Wl \
	-version-number $(LIBRARY_VERSION_MAJOR):$(LIBRARY_VERSION_MINOR):$(LIBRARY_VERSION_RELEASE)

cgxget_SOURCES = cgxget.c abstraction-common.c abstraction-common.h \
		 ctrl-cpu.c ctrl-cpuset.c
cgxget_LIBS = $(CODE_COVERAGE_LIBS)
cgxget_CFLAGS = $(CODE_COVERAGE_CFLAGS)

cgxset_SOURCES = cgxset.c abstraction-common.c abstraction-common.h \
		 ctrl-cpu.c ctrl-cpuset.c
cgxset_LIBS = $(CODE_COVERAGE_LIBS)
cgxset_CFLAGS = $(CODE_COVERAGE_CFLAGS)
