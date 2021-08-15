#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([lux], [1.3.0], [lux@quae.nl])
AM_EXTRA_RECURSIVE_TARGETS([debug])
AC_LANG([C++])
AC_CONFIG_SRCDIR([src/lux.cc])
AC_CONFIG_MACRO_DIR([m4])

# Location of our own implementation of missing "standard" functions.
AC_CONFIG_LIBOBJ_DIR([lib])

AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE
#AM_PATH_CPPUNIT()

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

# Checks for libraries.  The order is important!
AC_CHECK_LIB([c], [printf])
AC_CHECK_LIB([m], [sin])
AC_CHECK_LIB([jpeg], [jpeg_start_decompress])
AC_CHECK_LIB([gslcblas], [cblas_dgemm])
AC_CHECK_LIB([gsl], [gsl_sf_bessel_J0])
AC_CHECK_LIB([X11], [XOpenDisplay])
AC_CHECK_LIB([readline], [readline])
AC_CHECK_LIB([sofa_c], [iauBi00])
AC_CHECK_LIB([calceph], [calceph_open])

# Check for a C++ library that doesn't export any function with C binding.
# Complicated!
AC_MSG_CHECKING([for OIIO::TypeDesc in -lOpenImageIO])
AC_LANG_PUSH([C++])
SAVED_LDFLAGS=$LDFLAGS
LDFLAGS="$LDFLAGS -lOpenImageIO"
AC_LINK_IFELSE(
  [AC_LANG_PROGRAM([#include <OpenImageIO/imageio.h>],
                   [OIIO::TypeDesc x])],
  [TEST_LIBS="$TEST_LIBS -lOpenImageIO"]
  [AC_DEFINE(
    [HAVE_LIBOPENIMAGEIO],
    [1],
    [Define to 1 if you have the 'OpenImageIO' library (-lOpenImageIO).]
  )]
  [AC_MSG_RESULT([yes])],
  [AC_MSG_RESULT([no])]
  LDFLAGS=$SAVED_LDFLAGS)
AC_LANG_POP([C++])

AC_CHECK_LIB([CppUTest], [cpputest_malloc],
  [AC_DEFINE(
    [HAVE_LIBCPPUTEST],
    [1],
    [Define to 1 if you have the 'CppUTest' library (-lCppUTest).]
   )
   CPPUTESTLIBS="-lCppUTest $CPPUTESTLIBS"
  ],
  [echo "No CppUTest-based tests will be run."]
)
if test "x$ac_cv_lib_CppUTest_cpputest_malloc" = xyes; then
  AC_CHECK_LIB([CppUTestExt], [mock_c],
    [AC_DEFINE(
      [HAVE_LIBCPPUTESTEXT],
      [1],
      [Define to 1 if you have the 'CppUTestExt' library (-lCppUTestExt).]
     )
     CPPUTESTLIBS="-lCppUTestExt $CPPUTESTLIBS"
    ],
    [echo "No CppUTestExt-based tests will be run."],
    [-lCppUTest]
  )
fi

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

AC_SUBST([CPPUTESTLIBS])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/fonts/Makefile
                 doc/Makefile
                 test/Makefile])

AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])
AM_COND_IF([HAVE_DOXYGEN],[AC_CONFIG_FILES([doc/Doxyfile])])

AM_CONDITIONAL([HAVE_CPPUTEST], [test -n "$CPPUTEST"])

AC_OUTPUT
