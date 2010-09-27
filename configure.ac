dnl Process this file with autoconf to produce a configure script.

dnl This file is part of the ANA version 3 distribution.
dnl Author: Louis Strous
dnl $Header: /usr/local/cvsroot/ana/configure.in,v 4.0 2001/01/22 21:56:39 strous Exp $

dnl If we find site.c where we expect it, then we assume that the rest
dnl of the source files are here as well.
AC_INIT(site.c)

dnl takes config.h.in and replace #define values according to results
dnl of the following tests.
AC_CONFIG_HEADERS(config.h)

AC_CONFIG_AUX_DIR(/usr/share/automake-1.9)

dnl determine canonical host type
AC_CANONICAL_HOST
AC_MSG_RESULT([Host type: $host])
dnl assign to PLATFORM preprocessor symbol
AC_DEFINE_UNQUOTED(PLATFORM, "${host}")

dnl Checks for programs.
dnl if CC is yet unset, then check for "gcc" and use "cc" if that's
dnl not found.  Set output variable CC to the name of the found
dnl compiler.  If the C compiler does not produce executables that can
dnl run on the current system, then set shell variable
dnl "cross_compiling" to yes, otherwise no.
AC_PROG_CC

dnl Locates the X window system include files and libraries; sets
dnl shell variables x_includes and x_libraries accordingly, unless
dnl they are in directories the compiler searches by default; if none
dnl are found, set shell variable no_x to yes
AC_PATH_X

dnl Checks for libraries.
dnl add -lX11 to LIBS; define HAVE_LIBX11
dnl AC_CHECK_LIB(X11, XOpenDisplay)

dnl add -lc to LIBS; define HAVE_LIBC
AC_CHECK_LIB(c, printf)

dnl add -lm to LIBS; define HAVE_LIBM
AC_CHECK_LIB(m, sin)

dnl add -ljpeg to LIBS; define HAVE_LIBJPEG
AC_CHECK_LIB(jpeg, jpeg_start_decompress, have_libjpeg=1;
LIBS="${LIBS} -ljpeg")

dnl defines STDC_HEADERS if the system has ANSI C header files
dnl (stdlib.h, stdarg.h, string.h, and float.h are checked; and also
dnl memchr() in string.h, free() in stdlib.h, and that ctype.h works
dnl on characters with the high bit set.)
AC_HEADER_STDC

AC_HEADER_DIRENT
AC_HEADER_STDBOOL
AC_HEADER_TIME

AC_TYPE_SIGNAL

dnl for each listed header file that exists, define HAVE_HEADER-FILE
AC_CHECK_HEADERS(fcntl.h, float.h, limits.h, malloc.h, regex.h, stddef.h,
sys/ioctl.h, sys/time.h, termio.h, termios.h)

dnl check existence of sys/mtio.h, define HAVE_SYS_MTIO_H accordingly,
dnl and set ac_have_sys_mtio_h accordingly
AC_CHECK_HEADER(sys/mtio.h, ac_have_mtio_h=1, ac_have_mtio_h=0)

AC_FUNC_ALLOCA
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STAT
AC_FUNC_STRTOD
AC_FUNC_VPRINTF

dnl Checks for typedefs, structures, and compiler characteristics.
dnl if keyword "const" is not fully supported, then define "const" to
dnl be empty.
AC_C_CONST

dnl if size_t is not defined, define size_t to be "unsigned"
AC_TYPE_SIZE_T

#dnl if a program may include both time.h and sys/time.h, then define
#dnl TIME_WITH_SYS_TIME.  Use with HAVE_SYS_TIME_H.
#AC_HEADER_TIME

dnl defined if words are stored with the most significant byte first
AC_C_BIGENDIAN

dnl sets SIZEOF_LONG_LONG_INT to the number of bytes in a long int.
dnl we use it to check if we have 64-bit integers
AC_CHECK_SIZEOF(long long int)

dnl sets SIZEOF_UNSIGNED_CHAR to the number of bytes in an unsigned char.
dnl we use it to check if unsigned char is recognized
AC_CHECK_SIZEOF(unsigned char)

dnl sets SIZEOF_UNSIGNED_SHORT_INT to the number of bytes in an unsigned short
dnl int.  We use it to check if unsigned short int is recognized
AC_CHECK_SIZEOF(unsigned short int)

dnl if void is not defined in sys/types.h, stdlib.h, or stddef.h, then
dnl define it to be equal to char.
AC_CHECK_TYPE(void, char)

dnl Checks for library functions.
dnl add -traditional to CC if using the GNU C compiler and ioctl does
dnl not work properly without -traditional.
AC_PROG_GCC_TRADITIONAL

dnl for each specified function that is available, define HAVE_FUNCTION.
AC_CHECK_FUNCS(bcopy, bzero, floor, getcwd, gettimeofday, getwd,
hypot, memmove, pow, putenv, regcomp, rint, select, sqrt, strcasecmp,
strchr, strcspn, strdup, strerror, strncasecmp, strpbrk, strrchr,
strspn, strstr, strtod, strtol, strtoul)

dnl check for infinity
AC_TRY_RUN(main() { int x; x = 1.0/0.0; return 0; }, ac_have_infty=1)

dnl user-selectable packages
AC_ARG_WITH(terminal,
[  --with-terminal[=term]  specify terminal selection method
   choices:  terminfo termcap dummyterm; defaults to dummyterm])
AC_ARG_WITH(jpeg,
[  --without-jpeg          disable jpeg routines])
AC_ARG_WITH(motif,
[  --without-motif         do not include Motif routines])
AC_ARG_ENABLE(debug,
[  --enable-debug          enable debugging routines])
AC_ARG_ENABLE(develop,
[  --enable-develop        enable development routines])

MOREOBS=
BISONPARSE=
BISONCALC=
EXEC=ana
MORELIBS=
MOREINCLDIRS=
AC_SUBST(BISONPARSE)
AC_SUBST(BISONCALC)
AC_SUBST(EXEC)
if test x$no_x = x; then
  AC_MSG_RESULT([including X11 support])
  AC_DEFINE(X11)
  MOREOBS="${MOREOBS} xport.o zoom.o menu.o color.o"
  if test x$x_libraries != "x"; then
    MORELIBS="-L${x_libraries}"
    AC_MSG_RESULT([Adding library directory option $MORELIBS])
  fi
  if test x$x_libraries != "x"; then
    MOREINCLDIRS="-I${x_includes}"
    AC_MSG_RESULT([Adding include file directory option $MOREINCLDIRS])
  fi
  LIBS="${LIBS} -lX11"
  if test x$with_motif != xno; then
    AC_CHECK_LIB(Xt, XtAppInitialize, ac_have_libXt=1)
    AC_CHECK_LIB(Xext, XSetExtensionErrorHandler, ac_have_libXext=1)
    AC_CHECK_LIB(Xm, XmStringCreateSimple, ac_have_libXm=1, , -lXt -lXext)
    if test x$ac_have_libXt = "x1" && test x$ac_have_libXext = "x1" && test x$ac_have_libXm = "x1"; then
      AC_MSG_RESULT([including Motif support])
      MOREOBS="${MOREOBS} motif.o motifextra.o"
      LIBS="${LIBS} -lXt -lXm -lXext"
      AC_DEFINE(MOTIF)
      cat >> confdefs.h << EOF
#define HAVE_LIBXM 1
#define HAVE_LIBXT 1
#define HAVE_LIBXEXT 1
EOF
    fi
  fi
else
 AC_MSG_RESULT([WARNING: No X Windows support was included!  Possible reasons:
 (1) You asked for it, by specifying argument --without-x.
 (2) The required X11 include and library files were not found
     in standard locations (e.g., /usr/include/X11 and /usr/lib/).
 (3) The required X11 include and library files were not found
     in the locations you specified with arguments --x-includes
     and --x-libraries.
 You won't be able to display any plots or images on screen.])
fi 

if test x$ac_have_infty = x1; then
  AC_MSG_RESULT([allow 1.0/0.0])
  AC_DEFINE(INFTY, (1.0/0.0))
fi

if test $have_libjpeg && test x$with_jpeg != xno; then
  AC_MSG_RESULT([including JPEG support])
  AC_DEFINE(HAVE_LIBJPEG)
  MOREOBS="${MOREOBS} jpeg.o"
fi

if test x$ac_have_mtio_h = x1; then
  AC_MSG_RESULT([including magnetic tape support])
  AC_DEFINE(HAVE_SYS_MTIO_H)
  MOREOBS="${MOREOBS} tape.o"
fi

if test x$enable_debug = xyes || test x$enable_develop = xyes; then
  AC_CHECK_PROG(HAVEBISON, bison, yes, no)
  if test x$HAVEBISON = xyes; then
    AC_MSG_RESULT([found "bison" parser generator])
    BISONPARSE="bison --defines=anaparser.c.tab.h --output-file=anaparser.c.tab.c anaparser.c"
    BISONCALC="bison -p calc calculator.c"
  else
    AC_MSG_RESULT([did not find "bison" parser generator -- using existing anaparser.c.tab.c])
  fi
fi

if test x$enable_develop = xyes; then
  AC_MSG_RESULT([including development routines])
  AC_DEFINE(DEVELOP)
fi

if test x$enable_debug = xyes; then
  AC_MSG_RESULT([including developer debugging support])
  AC_DEFINE(YYDEBUG)
  AC_DEFINE(DEBUG)
  EXEC=anap
  MOREOBS="${MOREOBS} debug.o"
fi

UPDATELEVEL=
AC_SUBST(UPDATELEVEL)
if test x$enable_develop = xyes; then
  AC_MSG_RESULT([including developer environment])
  UPDATELEVEL="./updatelevel"
  AC_DEFINE(DEVELOP)
  MOREOBS="${MOREOBS} projection.o"
fi

if test x$with_terminal = xtermcap; then
  AC_MSG_RESULT([including termcap support])
  MOREOBS="${MOREOBS} termcap.o"
elif test x$with_terminal = xterminfo; then
  AC_MSG_RESULT([including terminfo support])
  MOREOBS="${MOREOBS} terminfo.o"
else
  AC_MSG_RESULT([including dummyterm support])
  MOREOBS="${MOREOBS} dummyterm.o"
fi

#if test x$ac_have_search_h = x1; then
#  MOREOBS="${MOREOBS} btree.o"
#fi

AC_SUBST(MOREOBS)
AC_SUBST(MORELIBS)
AC_SUBST(MOREINCLDIRS)

AC_OUTPUT(Makefile anainstall-sh, chmod u+x anainstall-sh; echo timestamp > stamp-h)

dnl AC_OUTPUT(Makefile)

# We try to prepare an .anaenv shell script that sets useful
# environment variables.  As far as I can tell, the script has to be
# compatible with the user's shell program.  Shell programs come in
# two major, incompatible flavors: sh-like, and csh-like.  Both shells
# set environment variable SHELL to the name of the shell that is
# currently running, but the ./configure program always uses "sh", so
# while we're running ./configure, SHELL is always equal to "sh" (with
# some prepended directory names), and cannot be used to deduce the
# user's default shell.  Instead, we try to grab the shell's name from
# the user's entry in /etc/passwd.  If that doesn't work, then we look
# for .profile or .cshrc in the user's home directory and deduce the
# shell from that.  If that fails, too, then we assume csh.
# 
# Csh-like shells offer an environment variable PWD that contains the
# name of the current directory, but not all sh-like shells offer such
# a variable, so we get it from the pwd command instead.
#
# To make the definitions from .anaenv available outside that script
# itself, csh-users should execute "source .anaenv", and sh-users
# should execute ". .anaenv".  For csh-users, the new definitions show
# up in the output from "env", but for sh-users it need not.  The
# proper test to see if the definitions are there is thus to use one
# of them; for example, "cd $ANA_GENERIC" should work.

CURDIR=`pwd`		# the current directory

exec_prefix=`eval echo $exec_prefix`	# fully expand

# we try to determine what shell you use, first by checking your
# entry in the /etc/passwd file, for which we need to know your
# username.
if test x${USER} = x; then	# username isn't in $USER
  USER=${LOGNAME}		# try $LOGNAME instead
fi

if test x${USER} != x; then	# we found your username, now check
				# the /etc/passwd file
  USERSHELL=`grep "^${USER}:" /etc/passwd 2>/dev/null | sed -e 's/.*://g'`
else
  USERSHELL=
fi

if test x${USERSHELL} = x; then
  # could not check the passwd file; look for likely initialization files
  if test -r ~/.cshrc; then
    USERSHELL=/csh
  elif test -r ~/.profile; then
    USERSHELL=/sh
  elif test -r ~/.bashrc; then
    USERSHELL=/sh
  else
    echo I could not determine what shell you use.  I\'m going to assume csh.
    USERSHELL=/csh
  fi
fi

case $USERSHELL in
  */csh | */tcsh)
    echo Creating ./.anaenv
    echo C-shell style\; add \"source .anaenv\" to your .login file.
    cat <<EOF > ./.anaenv
#! $USERSHELL
# This script was generated automatically by autoconf
# to set environment variables for the ANA program.
setenv ANADIR ${prefix}/share/ana
setenv ANA_PATH .:~/ana # example specification of standard *.ana directories
setenv ANADRIVE0 /dev/rmt/0 # example specification for ANA tape unit 0
EOF
  ;;
  *)
    echo Creating ./.anaenv
    echo Bourne-shell style\; add \". .anaenv\" to your .profile file.
    cat <<EOF > ./.anaenv
#! $USERSHELL
# This script was generated automatically by autoconf
# to set environment variables for the ANA program.
ANADIR=${prefix}/share/ana
ANA_PATH=.:~/ana # example specification of standard *.ana directories
ANADRIVE0=/dev/rmt/0 # example specification for ANA tape unit 0
export ANADIR
export ANA_PATH
export ANADRIVE0
EOF
  ;;
esac

chmod a+x ./.anaenv	# make executable

echo Creating ./uninstall-sh
cat <<EOF > ./uninstall-sh
#! /bin/sh
# This script was generated automatically by autoconf
# to remove the ANA files from their installation locations.
echo Removing ${exec_prefix}/libexec/${TARGET}/${EXEC}
rm -f ${exec_prefix}/libexec/${TARGET}/${EXEC}
echo Removing ${exec_prefix}/bin/hosttype
rm -f ${exec_prefix}/bin/hosttype
echo Removing ${prefix}/share/ana/fonts "(directory)"
rm -rf ${prefix}/share/ana/fonts
echo Removing ${prefix}/share/ana/ana.texinfo
rm -f ${prefix}/share/ana/ana.texinfo
echo Removing ${exec_prefix}/bin/${EXEC}
rm -f ${exec_prefix}/bin/${EXEC}
echo We now remove those affected directories that are empty, and all
echo of their empty parent directories.
echo Checking ${exec_prefix}/libexec/${TARGET}
rmdir -p ${exec_prefix}/libexec/${TARGET}
echo Checking ${exec_prefix}/bin
rmdir -p ${exec_prefix}/bin
echo Checking ${prefix}/share/ana
rmdir -p ${prefix}/share/ana
EOF
chmod a+x ./uninstall-sh # make executable

echo Creating ./ana.sh
cat <<EOF > ./ana.sh
#! /bin/sh
${exec_prefix}/libexec/\`${exec_prefix}/bin/hosttype\`/${EXEC} \$*
EOF

cat <<EOF
Done.
To compile ANA, type "make".
To compile with optimization, type "make opt".
To install in the target directories after compilation, type "make install".
EOF


