@CODE_COVERAGE_RULES@

INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/include
LDADD = $(top_builddir)/src/.libs/libcgroup.la

if WITH_TOOLS

bin_PROGRAMS = cgexec cgclassify cgcreate cgset cgget cgdelete lssubsys\
		lscgroup cgsnapshot cgxget cgxset

sbin_PROGRAMS = cgconfigparser cgclear

lib_LTLIBRARIES = libcgrouptools.la
libcgrouptools_la_SOURCES = tools-common.c tools-common.h \
	cgget.c cgget-main.c \
	abstraction-common.c abstraction-common.h \
	ctrl-cpu.c ctrl-cpuset.c
libcgrouptools_la_CFLAGS = $(CODE_COVERAGE_CFLAGS) -DSTATIC=static
libcgrouptools_la_LDFLAGS = -Wl \
	-version-number $(LIBRARY_VERSION_MAJOR):$(LIBRARY_VERSION_MINOR):$(LIBRARY_VERSION_RELEASE)

cgexec_SOURCES = cgexec.c tools-common.c tools-common.h
cgexec_LIBS = $(CODE_COVERAGE_LIBS)
cgexec_CFLAGS = $(CODE_COVERAGE_CFLAGS)

cgclassify_SOURCES = cgclassify.c tools-common.c tools-common.h
cgclassify_LIBS = $(CODE_COVERAGE_LIBS)
cgclassify_CFLAGS = $(CODE_COVERAGE_CFLAGS)

cgcreate_SOURCES = cgcreate.c tools-common.c tools-common.h
cgcreate_LIBS = $(CODE_COVERAGE_LIBS)
cgcreate_CFLAGS = $(CODE_COVERAGE_CFLAGS)

cgset_SOURCES = cgset.c cgset-main.c tools-common.c tools-common.h \
	abstraction-common.c abstraction-common.h \
	ctrl-cpu.c ctrl-cpuset.c
cgset_LIBS = $(CODE_COVERAGE_LIBS)
cgset_CFLAGS = $(CODE_COVERAGE_CFLAGS)

cgget_SOURCES = cgget.c cgget-main.c tools-common.c tools-common.h \
	abstraction-common.c abstraction-common.h \
	ctrl-cpu.c ctrl-cpuset.c
cgget_LIBS = $(CODE_COVERAGE_LIBS)
cgget_CFLAGS = $(CODE_COVERAGE_CFLAGS)

cgconfigparser_SOURCES = cgconfig.c tools-common.c tools-common.h
cgconfigparser_LIBS = $(CODE_COVERAGE_LIBS)
cgconfigparser_CFLAGS = $(CODE_COVERAGE_CFLAGS)

cgclear_SOURCES = cgclear.c tools-common.c tools-common.h
cgclear_LIBS = $(CODE_COVERAGE_LIBS)
cgclear_CFLAGS = $(CODE_COVERAGE_CFLAGS)

cgdelete_SOURCES = cgdelete.c tools-common.c tools-common.h
cgdelete_LIBS = $(CODE_COVERAGE_LIBS)
cgdelete_CFLAGS = $(CODE_COVERAGE_CFLAGS)

lssubsys_SOURCES = lssubsys.c
lssubsys_LIBS = $(CODE_COVERAGE_LIBS)
lssubsys_CFLAGS = $(CODE_COVERAGE_CFLAGS)

lscgroup_SOURCES = tools-common.c lscgroup.c
lscgroup_LIBS = $(CODE_COVERAGE_LIBS)
lscgroup_CFLAGS = $(CODE_COVERAGE_CFLAGS)

cgsnapshot_SOURCES = cgsnapshot.c
cgsnapshot_LIBS = $(CODE_COVERAGE_LIBS)
cgsnapshot_CFLAGS = $(CODE_COVERAGE_CFLAGS)

cgxget_SOURCES = cgxget.c cgget.c \
	tools-common.c tools-common.h \
	abstraction-common.c abstraction-common.h \
	ctrl-cpu.c ctrl-cpuset.c
cgxget_LIBS = $(CODE_COVERAGE_LIBS)
cgxget_CFLAGS = $(CODE_COVERAGE_CFLAGS)

cgxset_SOURCES = cgxset.c cgset.c \
	tools-common.c tools-common.h \
	abstraction-common.c abstraction-common.h \
	ctrl-cpu.c ctrl-cpuset.c
cgxset_LIBS = $(CODE_COVERAGE_LIBS)
cgxset_CFLAGS = $(CODE_COVERAGE_CFLAGS)

install-exec-hook:
	chmod u+s $(DESTDIR)$(bindir)/cgexec

endif
