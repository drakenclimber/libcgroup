@CODE_COVERAGE_RULES@

INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/include
LDADD = $(top_builddir)/src/.libs/libcgroup.la

bin_PROGRAMS = cgxget

cgaget_SOURCES = cgxget.c
cgaget_LIBS = $(CODE_COVERAGE_LIBS)
cgaget_CFLAGS = $(CODE_COVERAGE_CFLAGS)
