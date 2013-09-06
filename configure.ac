#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([ana], [5.0], [ana@quae.nl])
AC_CONFIG_SRCDIR([src/ana.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_YACC

# Checks for libraries.
AC_PATH_XTRA
AC_DEFINE([X11],[!defined(X_DISPLAY_MISSING)],[Define if X11 is present])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
