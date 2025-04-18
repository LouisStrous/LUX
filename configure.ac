#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([lux], [1.5.0], [lux@quae.nl])
AM_EXTRA_RECURSIVE_TARGETS([debug])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_LANG([C++])
AC_CONFIG_SRCDIR([src/lux.cc])
AC_CONFIG_MACRO_DIR([m4])

# Location of our own implementation of missing "standard" functions.
AC_CONFIG_LIBOBJ_DIR([lib])

AC_CONFIG_HEADERS([config.h])

AC_CANONICAL_HOST
AC_DEFINE_UNQUOTED([PLATFORM], "${host}", [The canonical host type])

# Options

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AC_PROG_YACC
if test "x$ac_cv_prog_YACC" = "xbison -y"; then
   ac_cv_prog_YACC=bison
else
   AC_MSG_ERROR([need bison])
fi
AC_CHECK_PROGS([DOXYGEN], [doxygen])

AM_PROG_AR

# Checks for libraries.  The order is important!
AC_CHECK_LIB([c], [printf])
AC_CHECK_LIB([m], [sin])
AC_CHECK_LIB([jpeg], [jpeg_start_decompress])
AC_CHECK_LIB([gslcblas], [cblas_dgemm])
AC_CHECK_LIB([gsl], [gsl_sf_bessel_J0])
AC_CHECK_LIB([readline], [readline])
AC_CHECK_LIB([sofa_c], [iauBi00])
AC_CHECK_LIB([calceph], [calceph_open])

# Check for a C++ library that doesn't export any function with C binding.
# Complicated!
AC_MSG_CHECKING([for OIIO::TypeDesc in -lOpenImageIO])
AC_LANG_PUSH([C++])
SAVED_LDFLAGS=$LDFLAGS
LIBS="-lOpenImageIO -lOpenImageIO_Util $LIBS"
AC_LINK_IFELSE(
  [AC_LANG_PROGRAM([#include <OpenImageIO/imageio.h>],
                   [OIIO::TypeDesc x])],
  [TEST_LIBS="$TEST_LIBS -lOpenImageIO -lOpenImageIO_Util"]
  [AC_DEFINE(
    [HAVE_LIBOPENIMAGEIO],
    [1],
    [Define to 1 if you have the 'OpenImageIO' library (-lOpenImageIO).]
  )]
  [AC_MSG_RESULT([yes])],
  [AC_MSG_RESULT([no])]
  LDFLAGS=$SAVED_LDFLAGS)
AC_LANG_POP([C++])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T

# Checks for library functions.
AC_CHECK_FUNCS([clock_gettime])

# Checks for library functions, with replacements if needed.
AC_REPLACE_FUNCS([sincos])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/fonts/Makefile
                 doc/Makefile])

AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])
AM_COND_IF([HAVE_DOXYGEN],[AC_CONFIG_FILES([doc/Doxyfile])])

AC_OUTPUT
