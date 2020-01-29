@CODE_COVERAGE_RULES@

INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/include
LDADD = $(top_builddir)/src/.libs/libcgroup.la \
	$(top_builddir)/src/tools/.libs/libcgrouptools.la

bin_PROGRAMS = cgxget cgxset

cgxget_SOURCES = cgxget.c
cgxget_LIBS = $(CODE_COVERAGE_LIBS)
cgxget_CFLAGS = $(CODE_COVERAGE_CFLAGS)

cgxset_SOURCES = cgxset.c
cgxset_LIBS = $(CODE_COVERAGE_LIBS)
cgxset_CFLAGS = $(CODE_COVERAGE_CFLAGS)
