TESTS = check-axis
check_PROGRAMS = check-axis
check_axis_SOURCES = check-axis.c
AM_CFLAGS = -I$(top_builddir)/src @CHECK_CFLAGS@
LDADD = $(top_builddir)/src/libana.a -lm -lc -ljpeg -lX11 -lgsl @CHECK_LIBS@
