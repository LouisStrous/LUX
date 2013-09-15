#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([ana], [5.0], [ana@quae.nl])
AC_LANG([C])
AC_CONFIG_SRCDIR([src/ana.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE
PKG_CHECK_MODULES([CHECK],[check >= 0.9.4])

AC_CANONICAL_HOST
AC_DEFINE_UNQUOTED([PLATFORM], "${host}", [The canonical host type])

# Options

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_YACC

# Checks for libraries.
AC_CHECK_LIB(c, printf)
AC_CHECK_LIB(m, sin)
AC_CHECK_LIB(jpeg, jpeg_start_decompress)
AC_CHECK_LIB(gsl, gsl_sf_bessel_J0)
AC_CHECK_LIB(X11, XOpenDisplay)

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 sofa/Makefile
                 sofa/src/Makefile
                 src/Makefile
                 doc/Makefile
                 test/Makefile])
AC_OUTPUT
