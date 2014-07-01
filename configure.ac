#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([lux], [1.0.2], [lux@quae.nl])
AC_LANG([C++])
AC_CONFIG_SRCDIR([src/lux.cc])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE
PKG_CHECK_MODULES([CHECK],[check >= 0.9.4])

AC_CANONICAL_HOST
AC_DEFINE_UNQUOTED([PLATFORM], "${host}", [The canonical host type])

# Options

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AC_PROG_YACC
AC_CHECK_PROGS([DOXYGEN], [doxygen])

# Checks for libraries.
AC_CHECK_LIB(c, printf)
AC_CHECK_LIB(m, sin)
AC_CHECK_LIB(jpeg, jpeg_start_decompress)
AC_CHECK_LIB(gsl, gsl_sf_bessel_J0)
AC_CHECK_LIB(X11, XOpenDisplay)
AC_CHECK_LIB(readline, readline)

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T

# Checks for library functions.
AC_CHECK_FUNCS([clock_gettime])

AC_CONFIG_FILES([Makefile
                 sofa/Makefile
                 sofa/src/Makefile
                 src/Makefile
                 src/fonts/Makefile
                 doc/Makefile
                 test/Makefile])
AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])
AM_COND_IF([HAVE_DOXYGEN],[AC_CONFIG_FILES([doc/Doxyfile])])
AC_OUTPUT
