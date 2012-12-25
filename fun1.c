/* File fun1.c */
/* Various ANA functions. */
/*a collection of internal ana subroutines and functions */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <errno.h>
#include <ctype.h>
#include "install.h"
#include "action.h"
#include "calendar.h"
static char rcsid[] __attribute__ ((unused)) =
"$Id: fun1.c,v 4.0 2001/02/07 20:37:00 strous Exp $";

extern	double	cbrt(double), expm1(double), log1p(double);
double	voigt(double, double), beta(double, double), gamma(double),
	incomplete_gamma(double, double), chi_square(double, double),
	incomplete_beta(double, double, double), student(double, double),
	F(double, double, double), loggamma(double), logbeta(double, double),
	non_central_chi_square(double, double, double), bessel_i0(double),
	bessel_i1(double), bessel_k0(double), bessel_k1(double),
	bessel_kn(int, double), sgn(double);
static	int	result_sym;
int	math_funcs(int, int), math_funcs_2f(int, int, int),
	math_funcs_i_f(int, int, int), math_funcs_3f(int, int, int, int);
int	ana_zerof(int, int []);

enum fd {
  F_SIN, F_COS, F_TAN, F_ASIN, F_ACOS, F_ATAN, F_SINH, F_COSH, F_TANH,
  F_SQRT, F_CBRT, F_EXP, F_EXPM1, F_LOG, F_LOG10, F_LOG1P, F_ERF, F_ERFC,
  F_J0, F_J1, F_Y0, F_Y1, F_GAMMA, F_LOGGAMMA, F_I0, F_I1, F_K0, F_K1,
  F_SGN, F_ASINH, F_ACOSH, F_ATANH
};

enum fdd {
  F_ATAN2, F_POW, F_VOIGT, F_BETA, F_LOGBETA, F_IGAMMA, F_CHI2, F_STUDENT
};

enum fid {
  F_JN, F_YN, F_KN
};

enum fddd {
  F_NCCHI2, F_IBETA, F_FRATIO
};

double (*func_d[])(double) = {
  sin, cos, tan, asin, acos, atan, sinh, cosh, tanh, sqrt, cbrt, exp,
  expm1, log, log10, log1p, erf, erfc, j0, j1, y0, y1, gamma, loggamma,
  bessel_i0, bessel_i1, bessel_k0, bessel_k1, sgn, asinh, acosh, atanh
};

double (*func_id[])(int, double) = {
  jn, yn, bessel_kn
};

double (*func_dd[])(double, double) = {
  atan2, pow, voigt, beta, logbeta, incomplete_gamma, chi_square, student
};

double (*func_ddd[])(double, double, double) = {
  non_central_chi_square, incomplete_beta, F
};

doubleComplex c_sin(double, double), c_cos(double, double),
  c_tan(double, double), c_arcsin(double, double), c_arccos(double,
  double), c_arctan(double, double), c_sinh(double, double),
  c_cosh(double, double), c_tanh(double, double), c_log(double,
  double), c_exp(double, double);

doubleComplex (*func_c[])(double, double) = {
  c_sin, c_cos, c_tan, c_arcsin, c_arccos, c_arctan, c_sinh, c_cosh,
  c_tanh, NULL, NULL, c_exp, NULL, c_log, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL
};

/* NOTE:  pow (and ana_pow) are now implemented as a ANA_BIN_OP instead of */
/* an ANA_EVB, so they are superfluous here.  keep them around just in case */
/* LS 29aug94 */
/*------------------------------------------------------------------------- */
int defined(int symbol, int argument)
/* <argument> = 0: Returns 0 if <symbol> is or links to (through ANA_TRANSFERs)
    a class of ANA_UNDEFINED or ANA_UNUSED, 1 otherwise.
 <argument> != 0: same as <argument> = 0 except when <symbol> is an argument
    to a user-defined subroutine or function.  Then returns 1 if the user
    specified a (possibly undefined) value for <symbol>, 0 otherwise.
 LS 27may96 4aug97 */
{
  int	target;

  target = symbol;
  target = transfer(target);

  if (argument) {
    if (symbol_class(target) == ANA_UNDEFINED
	&& undefined_par(target) == 1)
      return 0;
    if (symbol_class(symbol) == ANA_TRANSFER)
      return transfer_is_parameter(symbol);
  }

  target = eval(target);

  return (!target || symbol_class(target) == ANA_UNUSED
	  || symbol_class(target) == ANA_UNDEFINED)? 0: 1;
}
/*------------------------------------------------------------------------- */
int ana_defined(int narg, int ps[])
/* DEFINED(x) returns 0 if <x> is itself, or is linked through ANA_TRANSFERs */
/* with, a class of ANA_UNUSED or ANA_UNDEFINED.  DEFINED(x,/TARGET) returns 0 */
/* if it is itself, or is linked through ANA_POINTERs with, a class of */
/* ANA_UNUSED.  Dangling ANA_POINTERs, and dangling or unspecified parameters */
/* to user-defined subroutines and functions have DEFINED() = */
/* DEFINED(,/TARGET) = 0.  ANA_POINTERs or parameters that link to */
/* ANA_UNDEFINED have DEFINED() = 0, DEFINED(,/TARGET) = 1.  Other */
/* ANA_POINTERs or parameters have DEFINED() = DEFINED(,/TARGET) = 1. */
/* Other variables have DEFINED() = DEFINED(,/TARGET).  LS 27may96*/
{
  return defined(*ps, internalMode)? ANA_ONE: ANA_ZERO;
}
/*------------------------------------------------------------------------- */
int ana_delete(int narg, int ps[])
/* deletes symbols (deallocates memory & makes undefined) */
{
  int   i, iq;

  for (i = 0; i < narg; i++) {
    iq = *ps++;
    if ((internalMode & 1) == 0
	|| (symbol_class(iq) != ANA_POINTER
	    && symbol_class(iq) != ANA_TRANSFER)) {
      while (symbol_class(iq) == ANA_POINTER
	     || symbol_class(iq) == ANA_TRANSFER) {
	iq = transfer_target(iq);
	if (symbol_class(iq) == ANA_UNUSED)
	  iq = 0;
      }
    }
    undefine(iq);
  }
  return ANA_OK;
}
/*------------------------------------------------------------------------- */
int ana_quit(int narg, int ps[])
/*exit routine, calls are exit,status or quit,status */
{
 int	iq, saveHistory(void);
 void	Quit(int);

 if (narg)
   iq = int_arg(ps[0]);
 else
   iq = 0;
 Quit(iq);
 return ANA_OK;			/* or some compilers complain */
}
/*------------------------------------------------------------------------- */
int ana_cputime(void)
     /*returns an cpu time in seconds */
{
  int	i;

  i = scalar_scratch(ANA_FLOAT);
  scalar_value(i).f = ((float) clock())/CLOCKS_PER_SEC;
  return i;
}
/*------------------------------------------------------------------------- */
int ana_systime(void)
     /* returns the system time in ANA_DOUBLE seconds */
{
  int     i;
  struct timeval tp;
  struct timezone tzp;

  gettimeofday(&tp, &tzp);
  i = scalar_scratch(ANA_DOUBLE);
  scalar_value(i).d = (double) tp.tv_sec + 0.000001* (double) tp.tv_usec;
  return i;
  return ANA_ZERO;		/* return 0 if not available */
}
/*------------------------------------------------------------------------- */
int ana_ctime(void)
     /* returns current time and date in a string */
{
  int	i;
  time_t	t;

  i = string_scratch(24);
  t = time(NULL);
  strncpy(string_value(i), ctime(&t), 24);
  return i;
}
/*------------------------------------------------------------------------- */
int ana_time(void)
     /* returns current time in a string */
     /* added \0 to result string  LS 22may94 */
{
  int	i;
  time_t	t;
  char	*p;
  
  i = string_scratch(8);
  t = time(NULL);
  p = ctime(&t);
  strncpy(string_value(i), p+11, 8);
  *(string_value(i) + 9) = '\0';
  return i;
}
/*------------------------------------------------------------------------- */
int ana_date(void)
     /* returns current date in a string */
{
  int	i;
  time_t	t;
  char	*p, *p2;

  i = string_scratch(12);
  t = time(NULL);
  p = ctime( &t );
  p2 = (char *) string_value(i);
  strncpy(p2, p+4, 6);
  strncpy(p2+6, ", ", 2);
  strncpy(p2+8, p+20, 4);
  p2[12] = '\0';
  return i;
}
/*------------------------------------------------------------------------- */
int ana_jd(void)
/* returns current Julian Day (relative to UTC) in double precision */
{
  time_t	t;
  double	jd;
  int	result;

  t = time(NULL);
  /* NOTE: I assume here that t indicates the number of seconds since */
  /* 00:00:00 UTC on January 1, 1970, as it is supposed to do under */
  /* SunOS 5.4.  ANSI C's time routines are awkward.  It provides a */
  /* routine for specifying a particular *local* time in the internal */
  /* time_t representation, but none for specifying a particular UTC */
  /* time in that format.  If they did, then I could use difftime(). */
  /* Use of the struct tm representation is awkward, since I am not */
  /* interested in the (integer) second, minute, hour, day, and year. */
  /* LS 7oct97 */
  jd = (double) t/86400.0 + 2440587.5;
  result = scalar_scratch(ANA_DOUBLE);
  scalar_value(result).d = jd;
  return result;
}
/*------------------------------------------------------------------------- */
int ana_cjd(void)
/* returns current Chronological Julian Day, relative to the current
   time zone */
{
  int result;

  result = scalar_scratch(ANA_DOUBLE);
  scalar_value(result).d = CJD_now();
  return result;
}
/*------------------------------------------------------------------------- */
int ana_show(int narg, int ps[])
/*show some info about symbols by number or subname */
{
  int	iq, i;
  char	*s, *s2;
  int	ana_dump(int, int []);
  
  if (narg == 0)
    return ana_dump(-1, ps);	/* everybody */
  /* if a number passed, show symbol with that number; if a string, find
     all symbols containing string in name */
  iq = ps[0];
  switch (symbol_class(iq)) {
    case ANA_SCALAR:
      iq = int_arg(iq);
      if (iq < 0 || iq >= NSYM) {
	printf("no such symbol #\n");
	return ANA_ERROR;
      }
      return ana_dump(1, &iq);	/* dump this symbol by number */
    case ANA_STRING:
						/* more interesting */
      s = string_value(iq);
      s2 = strsave(s);
      s = s2;
      while (*s2) {
	*s2 = toupper((int) *s2);
	s2++;
      }
      for (i = 0; i < NSYM; i++) {
	/* get the name */
	s2 = varName(i);
	if (strstr(s2, s) != NULL) {
	  if (ana_dump(1, &i) != ANA_OK)
	    break;
	}
      }
      free(s);
      if (i != NSYM)
	return ANA_ERROR;
      else
	return ANA_OK;
    default:
      printw("show only accepts symbol # or a string containing");
      printw(" part of name\n");
      return ANA_ERROR;
  }
}
/*------------------------------------------------------------------------- */
void symdumpswitch(int nsym, int mode)
{
  char	*s, *typeName(int), *save;
  int	j, *ip;
  int	evalListPtr(int);
  
  if (symbol_class(nsym) == ANA_TRANSFER || symbol_class(nsym) == ANA_POINTER) {
    j = transfer_target(nsym);
    if (j == nsym)
      /* we can't have pointers pointing at themselves: we silently
	 make the pointer undefined instead.  LS 23nov98 */
      symbol_class(nsym) = ANA_UNDEFINED;
    else {
      printwf("%s, ", className(symbol_class(nsym)));
      if (!symbol_class(j))
	j = 0;
      printwf("points at <%d>\n", j);
      if (j) {
	s = symbolProperName(j);
	if (!s)
	  s = "(unnamed)";
	printwf(" *%d %s ", j, s);
	symdumpswitch(j, mode);
      }
      return;
    }
  }
  
  printwf("%s, ", className(symbol_class(nsym)));

  switch (symbol_class(nsym)) {
    case ANA_SCALAR: case ANA_CSCALAR: case ANA_ARRAY:
    case ANA_ASSOC: case ANA_FILEMAP: case ANA_CARRAY:
      printwf("%s, ", typeName(symbol_type(nsym)));
      break;
  }

  switch (symbol_class(nsym)) {
    case ANA_UNDEFINED: case ANA_UNUSED:
      printw("(no value)\n");
      return;
    case ANA_STRING:
      break;
    case ANA_ARRAY: case ANA_CARRAY:
      printwf("#elem = %1d, (", array_size(nsym));
      ip = array_dims(nsym);
      j = array_num_dims(nsym);
      while (j--) {
	printwf("%1d", *ip++);
	if (j)
	  printw(",");
      }
      printw(")\n");
      return;
  case ANA_CLIST:
    printwf("#elem = %1d, ", clist_num_symbols(nsym));
    break;
    case ANA_ASSOC:			/* assoc */
      s = typeName((int) assoc_type(nsym));
      printwf("lun = %d, offset = %d, ", assoc_lun(nsym),
	      assoc_has_offset(nsym)? assoc_offset(nsym): 0);
      break;
    case ANA_LIST_PTR:
      j = evalListPtr(nsym);
      printwf("struct pointer, points at %s\n",
	     symbolIdent(j, I_PARENT));
      printwf(" *%d %s ", sym[j].class? j: 0, varName(sym[j].class? j: 0));
      symdumpswitch(sym[j].class? j: 0, mode);
      return;
    case ANA_SUBROUTINE: case ANA_FUNCTION:
      printwf("# arguments = %1d, # statements = %1d\n",
	      routine_num_parameters(nsym), routine_num_statements(nsym));
      return;
    case ANA_DEFERRED_SUBR: case ANA_DEFERRED_FUNC: case ANA_DEFERRED_BLOCK:
      printwf("file %s\n", deferred_routine_filename(nsym));
      return;
    case ANA_BLOCKROUTINE:
      j = routine_num_statements(nsym);
      if (j)
	printwf("# statements = %1d\n", j);
      else
	printwf("(deferred), file %s\n", deferred_routine_filename(nsym));
      return;
  }						/*end of class switch */

  /* both printwf and symbolIdent use curScrat: use a trick to make it come
     out right.  LS 19oct98 */
  symbolIdent(nsym,mode);
  save = curScrat;
  curScrat += strlen(curScrat) + 1;
  printwf("value = %s\n", save);
  curScrat = save;
  return;
}
/*------------------------------------------------------------------------- */
int ana_dump(int narg, int ps[])
/* show some info about symbols in list */
/* internalMode:  1 fixed, 2 system, 4 zero, 8 local, 16 context */
/* 1 -> all fixed numbers (#s < nFixed)
   2 -> all system variables (nFixed <= #s <= tempSym)
   4 -> all main-level variables outside block routines
   8 -> all local variables
   16 -> all variables of a specific context (in *ps) */
{
 int	i, mode, iq, context = -1, imode, j;
 char	*s, *save;
 extern int	tempSym, nFixed;
 void	setPager(int), resetPager(void);

 mode = 0;
 imode = internalMode;
 if (narg == 0) {
   mode = 1;
   narg = N_NAMED;
 }
 else if (imode & 16) {		/* /CONTEXT */
   mode = 1;
   narg = N_NAMED;
   context = int_arg(*ps);
 }
 setPager(0);
 for (i = 0; i < narg; i++) {
   iq = mode? i: ps[i];
   if (iq < 0 || iq >= NSYM)
     return cerror(ILL_ARG, 0);
   if (!symbol_class(i))
     continue;
   if (mode) {
     if (iq < nFixed) {
       if ((imode & 1) == 0)	/* /FIXED */
	 continue;
     } else if (iq <= tempSym) {
       if ((imode & 2) == 0)	/* /SYSTEM */
	 continue;
     } else if (symbol_context(iq) <= 0) {
       if ((imode & 4) == 0)	/* /ZERO */
	 continue;
     } else if (symbol_context(iq) != context) {
       if ((imode & 8) == 0)	/* /LOCAL */
	 continue;
     }
   }

   if ((imode & 32) && iq > EXE_START) /* /FOLLOW -> evaluate */
     iq = evals(iq);

   save = curScrat;

   printwf("%3d ", iq);

   s = symbolProperName(iq);
   if (s) {			/* has its own name */
     if (symbol_context(iq) > 0) { /* has a "parent" */
       sprintf(curScrat, "%s", symbolProperName(symbol_context(iq)));
       curScrat += strlen(curScrat);
       strcpy(curScrat++, ".");
     }
     sprintf(curScrat, "%s", s);
     curScrat += strlen(curScrat);
   } else
     strcpy(curScrat, "(unnamed)");
   
   printwf("%-9s ", save);

   curScrat = save;

   j = (internalMode & 64)? 0: (I_TRUNCATE | I_LENGTH);
   if (iq < EXE_START)
     j |= I_VALUE;
   symdumpswitch(iq, j);
 }						/*end of loop over args */
 resetPager();
 return 1;
}							/*end of ana_dump */
/*------------------------------------------------------------------------- */
int ana_zero(int narg, int ps[])
/* subroutine version,  ANA_ZERO, x, [y ...] zero symbols in list */
{
  int	i, iq, mq, n;
  extern int	nFixed;
  char	*p;
  pointer	q;
  extern int	scrat[];
  FILE	*fp;
  
  for (i = 0; i < narg; i++) {
    iq = ps[i];
    /* check if it is legal to change this symbol */
    if (iq <= nFixed)
      return cerror(CST_LHS, iq);
    switch (symbol_class(iq)) {
      default:			/* don't do anything */
	break;
      case ANA_FILEMAP:
	mq = file_map_size(iq);
	mq *= ana_type_size[file_map_size(iq)]; /* # bytes to zero */
	p = file_map_file_name(iq); /* file name */
	if ((fp = fopen(p, "r+")) == NULL
	    && (fp = fopen(p, "w+")) == NULL) {
	  printf("File %s; ", p);
	  return cerror(ERR_OPEN, iq);
	}
	if (mq > NSCRAT*sizeof(int)) {
	  zerobytes(scrat, NSCRAT*sizeof(int));
	  n = (int) (mq/NSCRAT/sizeof(int));
	  for (i = 0; i < n; i++)
	    if (fwrite(scrat, 1, NSCRAT*sizeof(int), fp) < 1)
	      return cerror(WRITE_ERR, iq);
	  n = mq - n*NSCRAT*sizeof(int);
	  if (n && (fwrite(scrat, 1, n, fp) < 1))
	    return cerror(WRITE_ERR, iq);
	} else {
	  zerobytes(scrat, mq);
	  if (fwrite(scrat, mq, 1, fp) < 1)
	    return cerror(WRITE_ERR, iq);
	}
	fclose(fp);
	break;
      case ANA_SCALAR: case ANA_CSCALAR: /* scalar case */
	switch (scalar_type(iq)) {
	  case ANA_BYTE:
	    scalar_value(iq).b = 0;
	    break;
	  case ANA_WORD:
	    scalar_value(iq).w = 0;
	    break;
	  case ANA_LONG:
	    scalar_value(iq).l = 0;
	    break;
	  case ANA_FLOAT:
	    scalar_value(iq).f = 0;
	    break;
	  case ANA_DOUBLE:
	    scalar_value(iq).d = 0;
	    break;
	  case ANA_CFLOAT:
	    complex_scalar_data(iq).cf->real = 0.0;
	    complex_scalar_data(iq).cf->imaginary = 0.0;
	    break;
	  case ANA_CDOUBLE:
	    complex_scalar_data(iq).cd->real = 0.0;
	    complex_scalar_data(iq).cd->imaginary = 0.0;
	    break;
	}
	break;			/*end of scalar case */
      case ANA_SCAL_PTR:		/*scalar ptr case */
	switch (scal_ptr_type(iq)) {
	  case ANA_BYTE:
	    *scal_ptr_pointer(iq).b = 0;
	    break;
	  case ANA_WORD:
	    *scal_ptr_pointer(iq).w = 0;
	    break;
	  case ANA_LONG:
	    *scal_ptr_pointer(iq).l = 0;
	    break;
	  case ANA_FLOAT:
	    *scal_ptr_pointer(iq).f = 0;
	    break;
	  case ANA_DOUBLE:
	    *scal_ptr_pointer(iq).d = 0;
	    break;
	  case ANA_CFLOAT:
	    scal_ptr_pointer(iq).cf->real = 0.0;
	    scal_ptr_pointer(iq).cf->imaginary = 0.0;
	    break;
	  case ANA_CDOUBLE:
	    scal_ptr_pointer(iq).cd->real = 0.0;
	    scal_ptr_pointer(iq).cd->imaginary = 0.0;
	    break;
	}
	break;		/*end of scalar ptr case */
      case ANA_ARRAY: case ANA_CARRAY: /*array case */
	if (isStringType(array_type(iq))) { /* string array, remove members */
	  mq = array_size(iq);
	  q.sp = array_data(iq);
	  while (mq--) {
	    if (*q.sp)
	      free(*q.sp);
	    q.sp++;
	  }
	}
	/*try to zero quickly */
	mq = array_size(iq)*ana_type_size[array_type(iq)];
	/*mq should now be the # of bytes in the array, get start */
	p = array_data(iq);
	zerobytes(p, mq);
	break;
      case ANA_STRING:		/* string case */
	/*strings are blanked rather than zeroed */
	/*this may not make as much sense in Unix as it seemed to in VMS, may
	  want to change */
	mq = string_size(iq);
				/*note no ahead structure for strings */
	/* mq should now be the # of bytes +1 in the string, get start */
	p = string_value(iq);
	while (mq--)
	  *p++ = ' ';
	*p = '\0';		/*but null terminate it */
	break;
    }
  }
  return 1;
}							/*end of ana_type */
/*------------------------------------------------------------------------- */
int ana_onef(int narg, int ps[])
/* ONE(x) returns copy of numerical <x> with all elements equal to 1 */
/* LS 7apr98 */
{
  pointer	p;
  int	n, iq;

  iq = ps[0];
  switch (symbol_class(iq)) {
    case ANA_SCAL_PTR:
      iq = dereferenceScalPointer(iq);
    case ANA_SCALAR:
      if (!isFreeTemp(iq))
	iq = scalar_scratch(scalar_type(iq));
      p.b = &scalar_value(iq).b;
      n = 1;
      break;
    case ANA_CSCALAR:
      iq = scalar_scratch(complex_scalar_type(iq));
      p.cf = complex_scalar_data(iq).cf;
      n = 1;
      break;
    case ANA_ARRAY: case ANA_CARRAY:
      if (isNumericalType(array_type(iq))) {
	if (!isFreeTemp(iq))
	  iq = array_clone(iq, array_type(iq));
	p.b = array_data(iq);
	n = array_size(iq);
	break;
      }
    default:
      return cerror(ILL_CLASS, iq);
  }
  switch (array_type(iq)) {
    case ANA_BYTE:
      while (n--)
	*p.b++ = 1;
      break;
    case ANA_WORD:
      while (n--)
	*p.w++ = 1;
      break;
    case ANA_LONG:
      while (n--)
	*p.l++ = 1;
      break;
    case ANA_FLOAT:
      while (n--)
	*p.f++ = 1;
      break;
    case ANA_DOUBLE:
      while (n--)
	*p.d++ = 1;
      break;
    case ANA_CFLOAT:
      while (n--) {
	p.cf->real = 1.0;
	p.cf++->imaginary = 0.0;
      }
      break;
  }
  return iq;
}
/*------------------------------------------------------------------------- */
int ana_one(int narg, int ps[])
/* replaces all values in numerical array ps[0] by ones. */
/* LS 7apr98 */
{
  pointer	p;
  int	n, iq;

  while (narg--) {
    iq = *ps++;
    switch (symbol_class(iq)) {
      case ANA_SCAL_PTR:
	p.b = scal_ptr_pointer(iq).b;
	n = 1;
	break;
      case ANA_SCALAR:
	p.b = &scalar_value(iq).b;
	n = 1;
	break;
      case ANA_CSCALAR:
	p.cf = complex_scalar_data(iq).cf;
	n = 1;
	break;
      case ANA_ARRAY: case ANA_CARRAY:
	if (isNumericalType(iq)) { /* numerical type */
	  p.b = array_data(iq);
	  n = array_size(iq);
	  break;
	}
      default:
	n = 0;			/* flag: ignore this argument */
	break;
    }
    if (n)
      switch (array_type(iq)) {
	case ANA_BYTE:
	  while (n--)
	    *p.b++ = 1;
	  break;
	case ANA_WORD:
	  while (n--)
	    *p.w++ = 1;
	  break;
	case ANA_LONG:
	  while (n--)
	    *p.l++ = 1;
	  break;
	case ANA_FLOAT:
	  while (n--)
	    *p.f++ = 1;
	  break;
	case ANA_DOUBLE:
	  while (n--)
	    *p.d++ = 1;
	  break;
	case ANA_CFLOAT:
	  while (n--) {
	    p.cf->real = 1.0;
	    p.cf++->imaginary = 0.0;
	  }
	  break;
	case ANA_CDOUBLE:
	  while (n--) {
	    p.cd->real = 1.0;
	    p.cd++->imaginary = 0.0;
	  }
	  break;
      }
  }
  return ANA_OK;
}
/*------------------------------------------------------------------------- */
int ana_zerof(int narg, int ps[])
/* function version,  x = ANA_ZERO(y)
   create array x of same type and size as y and set all elements to zero */
{
  int	iq, mq;
  char	*p;

  iq = ps[0];
  switch (symbol_class(iq)) {
    case ANA_SCALAR: case ANA_CSCALAR: /*scalar case */
      /*need an output symbol, are we already a temp ? */
      if (isFreeTemp(iq))
	result_sym = iq;
      else
	result_sym = scalar_scratch(scalar_type(iq));
      switch (scalar_type(iq)) {
	case ANA_BYTE:
	  scalar_value(result_sym).b = 0;
	  break;
	case ANA_WORD:
	  scalar_value(result_sym).w = 0;
	  break;
	case ANA_LONG:
	  scalar_value(result_sym).l = 0;
	  break;
	case ANA_FLOAT:
	  scalar_value(result_sym).f = 0;
	  break;
	case ANA_DOUBLE:
	  scalar_value(result_sym).d = 0;
	  break;
	case ANA_CFLOAT:
	  complex_scalar_data(result_sym).cf->real = 0.0;
	  complex_scalar_data(result_sym).cf->imaginary = 0.0;
	  break;
	case ANA_CDOUBLE:
	  complex_scalar_data(result_sym).cd->real = 0.0;
	  complex_scalar_data(result_sym).cd->imaginary = 0.0;
	  break;
      }
      return result_sym;	/*end of scalar case */
    case ANA_ARRAY: case ANA_CARRAY: /*array case */
      /* need an output symbol, are we already a temp ? */
      if (isFreeTemp(iq))
	result_sym = iq;
      else
	result_sym = array_clone(iq, array_type(iq));
      /* try to zero quickly */
      mq = array_size(result_sym)*ana_type_size[array_type(iq)];
      /* mq should now be the # of bytes in the array, get start */
      p = array_data(result_sym);
      zerobytes(p, mq);
      return result_sym;
    case ANA_STRING:		/* string case */
      mq = string_size(iq);
      result_sym = string_scratch(mq);
      p = string_value(result_sym);
      mq--;
      while (mq--) 
	*p++ = ' ';
      *p = '\0';		/* but null terminate it */
      return result_sym;
    default:
      return cerror(ILL_CLASS, iq);
  }
}
/*------------------------------------------------------------------------- */
int indgen(int narg, int ps[], int isFunc)
/* fills array elements with their element index or with the value of
 one of their coordinates */
/* if called as function: INDGEN(<tgt> [, <axis>])
   if called as subroutine: INDGEN, <tgt> [, <axis>] */
{
  pointer	src, trgt;
  int	result;
  loopInfo	srcinfo, trgtinfo;

  if (isFunc) {
    if (standardLoop(ps[0], narg > 1? ps[1]: 0,
		     SL_UPGRADE | SL_AXISCOORD | SL_ONEAXIS,
		     ANA_BYTE, &srcinfo, &src, &result, &trgtinfo, &trgt) < 0)
      return ANA_ERROR;
  } else {
    if (standardLoop(ps[0], narg > 1? ps[1]: 0,
		     SL_AXISCOORD | SL_ONEAXIS,
		     ANA_BYTE, &srcinfo, &src, NULL, NULL, NULL) < 0)
      return ANA_ERROR;
    trgtinfo = srcinfo;
    trgt = src;
  }

  switch (symbol_type(ps[0])) {
    case ANA_BYTE:
      do 
	*trgt.b = (byte) trgtinfo.coords[0];
      while (advanceLoop(&trgtinfo, &trgt) < trgtinfo.rndim);
      break;
    case ANA_WORD:
      do 
	*trgt.w = (word) trgtinfo.coords[0];
      while (advanceLoop(&trgtinfo, &trgt) < trgtinfo.rndim);
      break;
    case ANA_LONG:
      do 
	*trgt.l = (int) trgtinfo.coords[0];
      while (advanceLoop(&trgtinfo, &trgt) < trgtinfo.rndim);
      break;
    case ANA_FLOAT:
      do 
	*trgt.f = (float) trgtinfo.coords[0];
      while (advanceLoop(&trgtinfo, &trgt) < trgtinfo.rndim);
      break;
    case ANA_DOUBLE:
      do 
	*trgt.d = (double) trgtinfo.coords[0];
      while (advanceLoop(&trgtinfo, &trgt) < trgtinfo.rndim);
      break;
    case ANA_CFLOAT:
      do {
	trgt.cf->real = trgtinfo.coords[0];
	trgt.cf->imaginary = 0.0;
      } while (advanceLoop(&trgtinfo, &trgt) < trgtinfo.rndim);
      break;
    case ANA_CDOUBLE:
      do {
	trgt.cd->real = trgtinfo.coords[0];
	trgt.cd->imaginary = 0.0;
      } while (advanceLoop(&trgtinfo, &trgt) < trgtinfo.rndim);
      break;
  }
  return result;
}
/*------------------------------------------------------------------------- */
int ana_indgen(int narg, int ps[])
{
  return indgen(narg, ps, 1);
}
/*------------------------------------------------------------------------- */
int ana_indgen_s(int narg, int ps[])
{
  return indgen(narg, ps, 0);
}
REGISTER(indgen_s, s, INDGEN, 1, 2, "*");
/*------------------------------------------------------------------------- */
int ana_neg_func(int narg, int ps[])
     /*take the negative of something */
{
  int	n, result;
  pointer	src, trgt;

  /* check that <*ps> is numerical, return number of elements in <n>,
     pointer in <src>.  Also generate a garbage clone of <*ps> with
     the same data type as <*ps> and return a pointer to it in <trgt>
     and its symbol number in <result>. */
  if (getNumerical(*ps, 0, &n, &src, 0, &result, &trgt) < 0)
    return ANA_ERROR;		/* some error */
  switch (symbol_type(*ps)) {
    case ANA_BYTE:
      memcpy(trgt.b, src.b, n);
      puts("WARNING - attempt to take the negative of an unsigned ANA_BYTE");
      break;
    case ANA_WORD:
      while (n--)
	*trgt.w++ = -*src.w++;
      break;
    case ANA_LONG:
      while (n--)
	*trgt.l++ = -*src.l++;
      break;
    case ANA_FLOAT:
      while (n--)
	*trgt.f++ = -*src.f++;
      break;
    case ANA_DOUBLE:
      while (n--)
	*trgt.d++ = -*src.d++;
      break;
    case ANA_CFLOAT:
      while (n--) {
	trgt.cf->real = -(src.cf->real);
	trgt.cf++->imaginary = -(src.cf++->imaginary);
      }
      break;
    case ANA_CDOUBLE:
      while (n--) {
	trgt.cd->real = -(src.cd->real);
	trgt.cd++->imaginary = -(src.cd++->imaginary);
      }
      break;
  }
  return result;
} /* end of ana_neg_func */
/*------------------------------------------------------------------------- */
int ana_isnan(int narg, int ps[])
     /* returns 1 if the argument is not a number (NaN).  Only works */
     /* if IEEE function isnan is available.  LS 28jun97 */
{
  int	n, result, iq, *trgt;
  pointer	src;
  floatComplex	*trgtc;

  iq = *ps;
  switch (symbol_class(iq)) {
    case ANA_SCAL_PTR:
      iq = dereferenceScalPointer(iq);
    case ANA_SCALAR:
      if (scalar_type(iq) < ANA_FLOAT) /* integer type */
	return ana_zerof(narg, ps); /* always OK */
      n = 1;
      src.f = &scalar_value(iq).f;
      result = scalar_scratch(ANA_LONG);
      trgt = &scalar_value(result).l;
      break;
    case ANA_CSCALAR:
      n = 1;
      src.cf = complex_scalar_data(iq).cf;
      result = scalar_scratch(ANA_CFLOAT);
      trgt = &scalar_value(result).l;
      break;
    case ANA_ARRAY:
      if (array_type(iq) < ANA_FLOAT)
	return ana_zerof(narg, ps); /* always OK */
      if (isStringType(array_type(iq)))
	return cerror(ILL_TYPE, *ps); /* no string arrays allowed here */
      n = array_size(iq);
      src.f = (float *) array_data(iq);
      result = array_clone(iq, ANA_LONG);
      trgt = array_data(result);
      break;
    case ANA_CARRAY:
      n = array_size(iq);
      src.f = array_data(iq);
      result = array_clone(iq, ANA_CFLOAT);
      trgtc = array_data(result);
      break;
    default:
      return cerror(ILL_CLASS, *ps);
  }

  switch (symbol_type(*ps)) {
    case ANA_FLOAT:
      while (n--)
	*trgt++ = isnan((double) *src.f++);
      break;
    case ANA_DOUBLE:
      while (n--)
	*trgt++ = isnan(*src.d++);
      break;
    case ANA_CFLOAT:
      while (n--) {
	trgtc->real = isnan(src.cf->real);
	trgtc++->imaginary = isnan(src.cf++->imaginary);
      }
      break;
    case ANA_CDOUBLE:
      while (n--) {
	trgtc->real = isnan(src.cd->real);
	trgtc++->imaginary = isnan(src.cd++->imaginary);
      }
      break;
  }
  return result;
}
/*------------------------------------------------------------------------- */
int zapnan(int narg, int ps[], int func)
/* ZERONANS,value=<value>, <arg1>, <arg2>, ... */
/* ZERONANS(value=<value>, <arg1>) */
/* replaces NaNs in numerical <arg1> &c with <value> (if defined), or */
/* with 0.  LS 27apr99 */
{
  int	size, result, valueSym;
  scalar	value;
  pointer	data, trgt;

  valueSym = ps[0];
  if (valueSym && !symbolIsRealScalar(valueSym))
    return anaerror("Need a real scalar here", valueSym);

  ps++; narg--;

  while (narg--) {
    if (numerical(*ps, NULL, NULL, &size, &data) == ANA_ERROR) {
      if (func)
	return *ps;
      else {
	ps++;
	continue;
      }
    }
    if (symbol_type(*ps) < ANA_FLOAT) { /* nothing to do: no NaN integers */
      if (func)
	return *ps;
      else {
	ps++;
	continue;
      }
    }
    if (func) {
      result = copySym(*ps);
      numerical(result, NULL, NULL, NULL, &trgt);
    } else
      trgt = data;

    switch (symbol_type(*ps)) {
      case ANA_FLOAT:
	value.f = valueSym? float_arg(valueSym): 0.0;
	while (size--) {
	  if (isnan(*data.f++))
	    *trgt.f = value.f;
	  trgt.f++;
	}
	break;
      case ANA_DOUBLE:
	value.d = valueSym? double_arg(valueSym): 0.0;
	while (size--) {
	  if (isnan(*data.d++))
	    *trgt.d = value.d;
	  trgt.d++;
	}
	break;
      case ANA_CFLOAT:
	value.f = valueSym? float_arg(valueSym): 0.0;
	while (size--) {
	  if (isnan(data.cf->real))
	    trgt.cf->real = value.f;
	  if (isnan(data.cf->imaginary))
	    trgt.cf->imaginary = value.f;
	  data.cf++;
	  trgt.cf++;
	}
	break;
      case ANA_CDOUBLE:
	value.d = valueSym? double_arg(valueSym): 0.0;
	while (size--) {
	  if (isnan(data.cd->real))
	    trgt.cd->real = value.d;
	  if (isnan(data.cd->imaginary))
	    trgt.cd->imaginary = value.d;
	  data.cd++;
	  trgt.cd++;
	}
	break;
    }
  }
  return func? result: ANA_OK;
}
/*------------------------------------------------------------------------- */
int ana_zapnan(int narg, int ps[])
/* ZAPNAN,value=<value>, <x1>, <x2>, ... replaces NaNs in the <x>s with the */
/* scalar <value>, which defaults to zero.  LS 8jun98 27apr99 */
{
  return zapnan(narg, ps, 0);
}
/*------------------------------------------------------------------------- */
int ana_zapnan_f(int narg, int ps[])
/* ZAPNAN(value=<value>, <x>) returns a copy of <x> with all NaNs replaced */
/* by the scalar <value> which defaults to zero.  LS 8jun98 27apr99 */
{
  return zapnan(narg, ps, 1);
}
/*------------------------------------------------------------------------- */
int ana_abs(int narg, int ps[])
/*take the absolute value of something */
{
  int	n, result, iq;
  pointer	src, trgt;

  iq = *ps;
  switch (symbol_class(iq)) {
    case ANA_SCAL_PTR:
      iq = dereferenceScalPointer(iq);
    case ANA_SCALAR:
      if (isFreeTemp(iq))
	result = iq;
      else
	result = scalar_scratch(symbol_type(iq));
      src.b = &scalar_value(iq).b;
      trgt.b = &scalar_value(result).b;
      n = 1;
      break;
    case ANA_ARRAY:
      if (array_type(iq) > ANA_DOUBLE)
	return cerror(ILL_TYPE, *ps);
      if (isFreeTemp(iq))
	result = iq;
      else
	result = array_clone(iq, array_type(iq));
      src.b = array_data(iq);
      trgt.b = array_data(result);
      n = array_size(iq);
      break;
    case ANA_CSCALAR:
      result = scalar_scratch(complex_scalar_type(iq)
			      - ANA_CFLOAT + ANA_FLOAT);
      src.cf = complex_scalar_data(iq).cf;
      trgt.b = &scalar_value(result).b;
      n = 1;
      break;
    case ANA_CARRAY:
      result = array_scratch(array_type(iq) - ANA_CFLOAT + ANA_FLOAT,
			     array_num_dims(iq), array_dims(iq));
      src.cf = array_data(iq);
      trgt.b = array_data(result);
      n = array_size(iq);
      break;
    default:
      return cerror(ILL_CLASS, *ps);
  }

  switch (symbol_type(*ps)) {
    case ANA_BYTE:
      memcpy(trgt.b, src.b, n);	/* BYTEs are always nonnegative */
      break;
    case ANA_WORD:
      while (n--) {
	*trgt.w++ = (*src.w > 0)? *src.w: -*src.w;
	src.w++;
      }
      break;
    case ANA_LONG:
      while (n--) {
	*trgt.l++ = (*src.l > 0)? *src.l: -*src.l;
	src.l++;
      }
      break;
    case ANA_FLOAT:
      while (n--) {
	*trgt.f++ = (*src.f >= 0)? *src.f: -*src.f;
	src.f++;
      }
      break;
    case ANA_DOUBLE:
      while (n--) {
	*trgt.d++ = (*src.d >= 0)? *src.d: -*src.d;
	src.d++;
      }
      break;
    case ANA_CFLOAT:
      while (n--) {
	*trgt.f++ = sqrt(src.cf->real*src.cf->real
			 + src.cf->imaginary*src.cf->imaginary);
	src.cf++;
      }
      break;
    case ANA_CDOUBLE:
      while (n--) {
	*trgt.d++ = sqrt(src.cd->real*src.cd->real
			 + src.cd->imaginary*src.cd->imaginary);
	src.cd++;
      }
      break;
  }
  return result;
}						/*end of ana_abs */
/*------------------------------------------------------------------------- */
int ana_complexsquare(int narg, int ps[])
/* returns the complex square of argument <x>, i.e., the product of
   <x> and its complex conjugate; if <x> is not complex, then assumes
   that it came from a call to the FFT function with real argument,
   i.e., that it contains the amplitudes of sine and cosine series.
   LS 2005dec18 */
{
  int	n, result, iq;
  pointer	src, trgt;

  iq = *ps;
  switch (symbol_class(iq)) {
    case ANA_SCAL_PTR:
      iq = dereferenceScalPointer(iq);
    case ANA_SCALAR:
      if (isFreeTemp(iq))
	result = iq;
      else
	result = scalar_scratch(symbol_type(iq));
      src.b = &scalar_value(iq).b;
      trgt.b = &scalar_value(result).b;
      n = 1;
      break;
    case ANA_ARRAY:
      if (array_type(iq) > ANA_DOUBLE)
	return cerror(ILL_TYPE, *ps);
      if (isFreeTemp(iq))
	result = iq;
      else
	result = array_clone(iq, array_type(iq));
      src.b = array_data(iq);
      trgt.b = array_data(result);
      n = array_size(iq);
      break;
    case ANA_CSCALAR:
      result = scalar_scratch(complex_scalar_type(iq)
			      - ANA_CFLOAT + ANA_FLOAT);
      src.cf = complex_scalar_data(iq).cf;
      trgt.b = &scalar_value(result).b;
      n = 1;
      break;
    case ANA_CARRAY:
      result = array_scratch(array_type(iq) - ANA_CFLOAT + ANA_FLOAT,
			     array_num_dims(iq), array_dims(iq));
      src.cf = array_data(iq);
      trgt.b = array_data(result);
      n = array_size(iq);
      break;
    default:
      return cerror(ILL_CLASS, *ps);
  }

  int i, n2;

  switch (symbol_type(*ps)) {
    case ANA_BYTE:
      n2 = n - (n%2);
      trgt.b[0] = src.b[0]*src.b[0];
      for (i = 1; i < n/2; i++)
	trgt.b[i] = src.b[i]*src.b[i] + src.b[n2 - i]*src.b[n2 - i];
      trgt.b[n/2] = src.b[n/2]*src.b[n/2];
      for (i = n/2 + 1; i < n; i++)
	trgt.b[i] = 0;
      break;
    case ANA_WORD:
      n2 = n - (n%2);
      trgt.w[0] = src.w[0]*src.w[0];
      for (i = 1; i < n/2; i++)
	trgt.w[i] = src.w[i]*src.w[i] + src.w[n2 - i]*src.w[n2 - i];
      trgt.w[n/2] = src.w[n/2]*src.w[n/2];
      for (i = n/2 + 1; i < n; i++)
	trgt.w[i] = 0;
      break;
    case ANA_LONG:
      n2 = n - (n%2);
      trgt.l[0] = src.l[0]*src.l[0];
      for (i = 1; i < n/2; i++)
	trgt.l[i] = src.l[i]*src.l[i] + src.l[n2 - i]*src.l[n2 - i];
      trgt.l[n/2] = src.l[n/2]*src.l[n/2];
      for (i = n/2 + 1; i < n; i++)
	trgt.l[i] = 0;
      break;
    case ANA_FLOAT:
      n2 = n - (n%2);
      trgt.f[0] = src.f[0]*src.f[0];
      for (i = 1; i < n/2; i++)
	trgt.f[i] = src.f[i]*src.f[i] + src.f[n2 - i]*src.f[n2 - i];
      trgt.f[n/2] = src.f[n/2]*src.f[n/2];
      for (i = n/2 + 1; i < n; i++)
	trgt.f[i] = 0;
      break;
    case ANA_DOUBLE:
      n2 = n - (n%2);
      trgt.d[0] = src.d[0]*src.d[0];
      for (i = 1; i < n/2; i++)
	trgt.d[i] = src.d[i]*src.d[i] + src.d[n2 - i]*src.d[n2 - i];
      trgt.d[n/2] = src.d[n/2]*src.d[n/2];
      for (i = n/2 + 1; i < n; i++)
	trgt.d[i] = 0;
      break;
    case ANA_CFLOAT:
      while (n--) {
	*trgt.f++ = src.cf->real*src.cf->real
	  + src.cf->imaginary*src.cf->imaginary;
	src.cf++;
      }
      break;
    case ANA_CDOUBLE:
      while (n--) {
	*trgt.d++ = src.cd->real*src.cd->real
	  + src.cd->imaginary*src.cd->imaginary;
	src.cd++;
      }
      break;
  }
  return result;  
}
/*------------------------------------------------------------------------- */
int ana_conjugate(int narg, int ps[])
/* returns the complex conjugate of numerical symbols */
/* LS 31jul98 */
{
  int	result, n;
  pointer	src, trgt;

  if (!symbolIsNumerical(*ps))
    return cerror(ILL_CLASS, *ps);
  
  if (isRealType(symbol_type(*ps)))
    return *ps;			/* conjugate equals original */

  switch (symbol_class(*ps)) {
    case ANA_CSCALAR:
      src.cf = complex_scalar_data(*ps).cf;
      if (isFreeTemp(*ps))
	result = *ps;
      else
	result = scalar_scratch(symbol_type(*ps));
      trgt.cf = complex_scalar_data(result).cf;
      n = 1;
      break;
    case ANA_CARRAY:
      src.cf = array_data(*ps);
      if (isFreeTemp(*ps))
	result = *ps;
      else
	result = array_clone(*ps, symbol_type(*ps));
      trgt.cf = array_data(result);
      n = array_size(*ps);
      break;
    default:
      return cerror(ILL_TYPE, *ps);
  }

  switch (symbol_type(*ps)) {
    case ANA_CFLOAT:
      while (n--) {
	trgt.cf->real = src.cf->real;
	trgt.cf++->imaginary = -src.cf++->imaginary;
      }
      break;
    case ANA_CDOUBLE:
      while (n--) {
	trgt.cd->real = src.cd->real;
	trgt.cd++->imaginary = -src.cd++->imaginary;
      }
      break;
  }
  return result;
}
/*------------------------------------------------------------------------- */
int index_total(int narg, int ps[], int mean)
/* accumulates source values by class */
{
  int	type, offset, *indx, i, size, result, nElem, indices2,
  	outType, haveWeights, p, psign, pp, nbase, j;
  pointer	src, trgt, sum, weights, hist;
  scalar	temp, value;
  floatComplex	tempcf, valuecf;
  doubleComplex	tempcd, valuecd;
  float	temp2f;
  double	temp2d;
  byte	*present;
  extern scalar	lastmin, lastmax;
  int	minmax(int *, int, int);

  if (narg > 3 && ps[3]) {	/* have <weights> */
    if (!symbolIsNumericalArray(ps[3]) /* not a numerical array */
	|| array_size(ps[3]) != array_size(ps[0])) /* or has the wrong size */
      return cerror(INCMP_ARG, ps[3]);
    haveWeights = 1;
  } else
    haveWeights = 0;

  src.v = array_data(ps[0]);	/* source data */
  nElem = array_size(ps[0]);	/* number of source elements */
  type = array_type(ps[0]);	/* source data type */
  if (isComplexType(type))
    outType = ((internalMode & 1)
	       || type == ANA_CDOUBLE)? ANA_CDOUBLE: ANA_CFLOAT;
  else
    outType = ((internalMode & 1)
	       || type == ANA_DOUBLE)? ANA_DOUBLE: ANA_FLOAT;

  if (narg > 2 && ps[2]) {	/* have <power> */
    if (!symbolIsScalar(ps[2]))
      return cerror(NEED_SCAL, ps[2]);
    p = int_arg(ps[2]);
    if (p < 0) {
      psign = -1;			/* sign of power */
      p = -p;
    } else
      psign = +1;
  } else {
    p = 1;
    psign = +1;
  }
  
  /* make <weights> have the same type as <x> (except not complex) */
  if (haveWeights) {
    haveWeights = ana_converts[realType(outType)](1, &ps[3]);
    if (haveWeights == ANA_ERROR) /* some error */
      return ANA_ERROR;
    weights.v = array_data(haveWeights);
  }

  /* need min and max of indices so we can create result array of */
  /* proper size */
  indices2 = ana_long(1, &ps[1]); /* force ANA_LONG */
  indx = array_data(indices2);	/* assumed of same size as <source>! */
  minmax(indx, nElem, ANA_LONG);
  size = lastmax.l + 1;
  offset = 0;
  if (lastmin.l < 0)
    size += (offset = -lastmin.l);
  result = array_scratch(outType, 1, &size);
  trgt.v = array_data(result);
  zerobytes(trgt.b, size*ana_type_size[outType]);
  trgt.b += offset*ana_type_size[outType];
  sum.b += offset*ana_type_size[outType];
  i = nElem;
  if (p == 1) {			/* regular summation */
    if (haveWeights) {		/* have <weights> */
      if (mean) {		/* want average */
	allocate(hist.d, size, double);
	zerobytes(hist.d, size*sizeof(double));
	hist.d += offset;
	switch (outType) {
	  case ANA_FLOAT:
	    switch (type) {
	      case ANA_BYTE:
		while (i--) {		/* get the sum */
		  hist.f[*indx] += *weights.f;
		  trgt.f[*indx] += (float) *src.b++ * *weights.f++;
		  indx++;
		}
		break;
	      case ANA_WORD:
		while (i--) {		/* first, get the sum */
		  hist.f[*indx] += *weights.f;
		  trgt.f[*indx] += (float) *src.w++ * *weights.f++;
		  indx++;
		}
		break;
	      case ANA_LONG:
		while (i--) {		/* first, get the sum */
		  hist.f[*indx] += *weights.f;
		  trgt.f[*indx] += (float) *src.l++ * *weights.f++;
		  indx++;
		}
		break;
	      case ANA_FLOAT:
		while (i--) {		/* first, get the sum */
		  hist.f[*indx] += *weights.f;
		  trgt.f[*indx] += (float) *src.f++ * *weights.f++;
		  indx++;
		}
		break;
	    } /* end of switch (type) */
	    /* and divide by number */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.f[i])
		trgt.f[i] /= hist.f[i];
	    break;
	  case ANA_DOUBLE:
	    switch (type) {
	      case ANA_BYTE:
		while (i--) {		/* get the sum */
		  hist.d[*indx] += *weights.d;
		  trgt.d[*indx] += (double) *src.b++ * *weights.d++;
		  indx++;
		}
		break;
	      case ANA_WORD:
		while (i--) {		/* first, get the sum */
		  hist.d[*indx] += *weights.d;
		  trgt.d[*indx] += (double) *src.w++ * *weights.d++;
		  indx++;
		}
		break;
	      case ANA_LONG:
		while (i--) {		/* first, get the sum */
		  hist.d[*indx] += *weights.d;
		  trgt.d[*indx] += (double) *src.l++ * *weights.d++;
		  indx++;
		}
		break;
	      case ANA_FLOAT:
		while (i--) {		/* first, get the sum */
		  hist.d[*indx] += *weights.d;
		  trgt.d[*indx] += (double) *src.f++ * *weights.d++;
		  indx++;
		}
		break;
	      case ANA_DOUBLE:
		while (i--) {		/* first, get the sum */
		  hist.d[*indx] += *weights.d;
		  trgt.d[*indx] += *src.d++ * *weights.d++;
		  indx++;
		}
		break;
	    } /* end of switch (type) */
	    /* and divide by number */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.d[i])
		trgt.d[i] /= hist.d[i];
	    break;
	  case ANA_CFLOAT:
	    while (i--) {
	      hist.f[*indx] += *weights.f;
	      trgt.cf[*indx].real += (float) src.cf->real * *weights.f;
	      trgt.cf[*indx++].imaginary +=
		(float) src.cf++->imaginary * *weights.f++;
	    }
	    for (i = -offset; i < size - offset; i++)
	      if (hist.f[i]) {
		trgt.cf[i].real /= hist.f[i];
		trgt.cf[i].imaginary /= hist.f[i];
	      } else
		trgt.cf[i].real = trgt.cf->imaginary = 0.0;
	    break;
	  case ANA_CDOUBLE:
	    switch (type) {
	      case ANA_CFLOAT:
		while (i--) {
		  hist.d[*indx] += *weights.d;
		  trgt.cd[*indx].real += (float) src.cf->real * *weights.d;
		  trgt.cd[*indx++].imaginary +=
		    (float) src.cf++->imaginary * *weights.d++;
		}
		break;
	      case ANA_CDOUBLE:
		while (i--) {
		  hist.d[*indx] += *weights.d;
		  trgt.cd[*indx].real += (float) src.cd->real * *weights.d;
		  trgt.cd[*indx++].imaginary +=
		    (float) src.cd++->imaginary * *weights.d++;
		}
		break;
	    } /* end of switch (type) */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.d[i]) {
		trgt.cd[i].real /= hist.d[i];
		trgt.cd[i].imaginary /= hist.d[i];
	      } else
		trgt.cd[i].real = trgt.cf->imaginary = 0.0;
	    break;
	} /* end of switch (outType) */
	free(hist.d - offset);
      } else {			/* want totals */
	switch (outType) {
	  case ANA_FLOAT:
	    switch (type) {
	      case ANA_BYTE:
		while (i--) 		/* get the sum */
		  trgt.f[*indx++] += (float) *src.b++ * *weights.f++;
		break;
	      case ANA_WORD:
		while (i--)		/* first, get the sum */
		  trgt.f[*indx++] += (float) *src.w++ * *weights.f++;
		break;
	      case ANA_LONG:
		while (i--)		/* first, get the sum */
		  trgt.f[*indx++] += (float) *src.l++ * *weights.f++;
		break;
	      case ANA_FLOAT:
		while (i--)		/* first, get the sum */
		  trgt.f[*indx++] += (float) *src.f++ * *weights.f++;
		break;
	    } /* end of switch (type) */
	    break;
	  case ANA_DOUBLE:
	    switch (type) {
	      case ANA_BYTE:
		while (i--) 	/* get the sum */
		  trgt.d[*indx++] += (double) *src.b++ * *weights.d++;
		break;
	      case ANA_WORD:
		while (i--) 		/* first, get the sum */
		  trgt.d[*indx++] += (double) *src.w++ * *weights.d++;
		break;
	      case ANA_LONG:
		while (i--) 		/* first, get the sum */
		  trgt.d[*indx++] += (double) *src.l++ * *weights.d++;
		break;
	      case ANA_FLOAT:
		while (i--) 		/* first get the sum */
		  trgt.d[*indx++] += (double) *src.f++ * *weights.d++;
		break;
	      case ANA_DOUBLE:
		while (i--) 		/* first get the sum */
		  trgt.d[*indx++] += *src.d++ * *weights.d++;
		break;
	    } /* end of switch (type) */
	    break;
	  case ANA_CFLOAT:
	    while (i--) {
	      trgt.cf[*indx].real += src.cf->real * *weights.f;
	      trgt.cf[*indx++].imaginary += src.cf++->imaginary * *weights.f++;
	    }
	    break;
	  case ANA_CDOUBLE:
	    switch (type) {
	      case ANA_CFLOAT:
		while (i--) {
		  trgt.cd[*indx].real += src.cf->real * *weights.d;
		  trgt.cd[*indx++].imaginary +=
		    src.cf++->imaginary * *weights.d++;
		}
		break;
	      case ANA_CDOUBLE:
		while (i--) {
		  trgt.cd[*indx].real += src.cd->real * *weights.d;
		  trgt.cd[*indx++].imaginary +=
		    src.cd++->imaginary * *weights.d++;
		}
		break;
	    }
	    break;
	} /* end of switch (outType) */
      }	/* end of if (mean) else */
    } else {			/* no <weights>: each element counts once */
      if (mean) {		/* want average */
	allocate(hist.l, size, int);
	zerobytes(hist.l, size*sizeof(int));
	hist.l += offset;
	switch (outType) {
	  case ANA_FLOAT:
	    switch (type) {
	      case ANA_BYTE:
		while (i--) {		/* get the sum */
		  hist.l[*indx]++;
		  trgt.f[*indx] += (float) *src.b++;
		  indx++;
		}
		break;
	      case ANA_WORD:
		while (i--) {		/* first, get the sum */
		  hist.l[*indx]++;
		  trgt.f[*indx] += (float) *src.w++;
		  indx++;
		}
		break;
	      case ANA_LONG:
		while (i--) {		/* first, get the sum */
		  hist.l[*indx]++;
		  trgt.f[*indx] += (float) *src.l++;
		  indx++;
		}
		break;
	      case ANA_FLOAT:
		while (i--) {		/* first, get the sum */
		  hist.l[*indx]++;
		  trgt.f[*indx] += (float) *src.f++;
		  indx++;
		}
		break;
	    } /* end of switch (type) */
	    /* and divide by number */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.l[i])
		trgt.f[i] /= hist.l[i];
	    break;
	  case ANA_DOUBLE:
	    switch (type) {
	      case ANA_BYTE:
		while (i--) {		/* get the sum */
		  hist.l[*indx]++;
		  trgt.d[*indx] += (float) *src.b++;
		  indx++;
		}
		break;
	      case ANA_WORD:
		while (i--) {		/* first, get the sum */
		  hist.l[*indx]++;
		  trgt.d[*indx] += (float) *src.w++;
		  indx++;
		}
		break;
	      case ANA_LONG:
		while (i--) {		/* first, get the sum */
		  hist.l[*indx]++;
		  trgt.d[*indx] += (float) *src.l++;
		  indx++;
		}
		break;
	      case ANA_FLOAT:
		while (i--) {		/* first, get the sum */
		  hist.l[*indx]++;
		  trgt.d[*indx] += (float) *src.f++;
		  indx++;
		}
		break;
	      case ANA_DOUBLE:
		while (i--) {		/* first, get the sum */
		  hist.l[*indx]++;
		  trgt.d[*indx] += *src.d++;
		  indx++;
		}
		break;
	    } /* end of switch (type) */
	    /* and divide by number */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.l[i])
		trgt.d[i] /= hist.l[i];
	    break;
	  case ANA_CFLOAT:
	    while (i--) {
	      hist.l[*indx]++;
	      trgt.cf[*indx].real += (float) src.cf->real;
	      trgt.cf[*indx++].imaginary += (float) src.cf++->imaginary;
	    }
	    for (i = -offset; i < size - offset; i++)
	      if (hist.l[i]) {
		trgt.cf[i].real /= hist.l[i];
		trgt.cf[i].imaginary /= hist.l[i];
	      } else
		trgt.cf[i].real = trgt.cf->imaginary = 0.0;
	    break;
	  case ANA_CDOUBLE:
	    switch (type) {
	      case ANA_CFLOAT:
		while (i--) {
		  hist.l[*indx]++;
		  trgt.cd[*indx].real += (float) src.cf->real;
		  trgt.cd[*indx++].imaginary += (float) src.cf++->imaginary;
		}
		break;
	      case ANA_CDOUBLE:
		while (i--) {
		  hist.l[*indx]++;
		  trgt.cd[*indx].real += (float) src.cd->real;
		  trgt.cd[*indx++].imaginary += (float) src.cd++->imaginary;
		}
		break;
	    } /* end of switch (type) */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.l[i]) {
		trgt.cd[i].real /= hist.l[i];
		trgt.cd[i].imaginary /= hist.l[i];
	      } else
		trgt.cd[i].real = trgt.cf->imaginary = 0.0;
	    break;
	} /* end of switch (outType) */
	free(hist.l - offset);
      } else {			/* want totals */
	switch (outType) {
	  case ANA_FLOAT:
	    switch (type) {
	      case ANA_BYTE:
		while (i--) 		/* get the sum */
		  trgt.f[*indx++] += (float) *src.b++;
		break;
	      case ANA_WORD:
		while (i--)		/* first, get the sum */
		  trgt.f[*indx++] += (float) *src.w++;
		break;
	      case ANA_LONG:
		while (i--)		/* first, get the sum */
		  trgt.f[*indx++] += (float) *src.l++;
		break;
	      case ANA_FLOAT:
		while (i--)		/* first, get the sum */
		  trgt.f[*indx++] += (float) *src.f++;
		break;
	    } /* end of switch (type) */
	    break;
	  case ANA_DOUBLE:
	    switch (type) {
	      case ANA_BYTE:
		while (i--) 	/* get the sum */
		  trgt.d[*indx++] += (float) *src.b++;
		break;
	      case ANA_WORD:
		while (i--) 		/* first, get the sum */
		  trgt.d[*indx++] += (float) *src.w++;
		break;
	      case ANA_LONG:
		while (i--) 		/* first, get the sum */
		  trgt.d[*indx++] += (float) *src.l++;
		break;
	      case ANA_FLOAT:
		while (i--) 		/* first get the sum */
		  trgt.d[*indx++] += (float) *src.f++;
		break;
	      case ANA_DOUBLE:
		while (i--) 		/* first get the sum */
		  trgt.d[*indx++] += *src.d++;
		break;
	    } /* end of switch (type) */
	    break;
	  case ANA_CFLOAT:
	    while (i--) {
	      trgt.cf[*indx].real += src.cf->real;
	      trgt.cf[*indx++].imaginary += src.cf++->imaginary;
	    }
	    break;
	  case ANA_CDOUBLE:
	    switch (type) {
	      case ANA_CFLOAT:
		while (i--) {
		  trgt.cd[*indx].real += src.cf->real;
		  trgt.cd[*indx++].imaginary += src.cf++->imaginary;
		}
		break;
	      case ANA_CDOUBLE:
		while (i--) {
		  trgt.cd[*indx].real += src.cd->real;
		  trgt.cd[*indx++].imaginary += src.cd++->imaginary;
		}
		break;
	    }
	    break;
	} /* end of switch (outType) */
      }	/* end of if (mean) else */
    } /* end of if (haveWeights) else */
  } else {			/* power summation */
    /* we set up for the calculation of the powers.  We use a scheme that */
    /* minimizes the number of multiplications that need to be performed. */
    present = (byte *) curScrat;/* some scratch space */
    pp = p;
    while (pp) {
      *present++ = (pp & 1);
      pp >>= 1;
    }
    nbase = present - (byte *) curScrat; /* number of bits in the exponent */
    present = (byte *) curScrat;
    if (haveWeights) {		/* weighted power summation */
      if (mean) {		/* want averages */
	allocate(hist.d, size, double);
	zerobytes(hist.d, size*sizeof(double));
	hist.d += offset;
	switch (outType) {
	  case ANA_FLOAT:
	    switch (type) {
	      case ANA_BYTE:
		while (i--) {
		  temp.f = *src.b++; /* data value */
		  value.f = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.f *= temp.f;
		    temp.f *= temp.f;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.f = value.f? *weights.f/value.f: 0.0;
		  else
		    value.f *= *weights.f;
		  trgt.f[*indx] += value.f;
		  hist.f[*indx] += *weights.f++;
		  indx++;
		}
		break;
	      case ANA_WORD:
		while (i--) {
		  temp.f = *src.w++; /* data value */
		  value.f = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.f *= temp.f;
		    temp.f *= temp.f;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.f = value.f? *weights.f/value.f: 0.0;
		  else
		    value.f *= *weights.f;
		  trgt.f[*indx] += value.f;
		  hist.f[*indx] += *weights.f++;
		  indx++;
		}
		break;
	      case ANA_LONG:
		while (i--) {
		  temp.f = *src.l++; /* data value */
		  value.f = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.f *= temp.f;
		    temp.f *= temp.f;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.f = value.f? *weights.f/value.f: 0.0;
		  else
		    value.f *= *weights.f;
		  trgt.f[*indx] += value.f;
		  hist.f[*indx] += *weights.f++;
		  indx++;
		}
		break;
	      case ANA_FLOAT:
		while (i--) {
		  temp.f = *src.f++; /* data value */
		  value.f = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.f *= temp.f;
		    temp.f *= temp.f;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.f = value.f? *weights.f/value.f: 0.0;
		  else
		    value.f *= *weights.f;
		  trgt.f[*indx] += value.f;
		  hist.f[*indx] += *weights.f++;
		  indx++;
		}
		break;
	    } /* end of switch (type) */
	    /* and divide by number */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.f[i])
		trgt.f[i] /= hist.f[i];
	    break;
	  case ANA_DOUBLE:
	    switch (type) {
	      case ANA_BYTE:
		while (i--) {
		  temp.d = *src.b++; /* data value */
		  value.d = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.d = value.d? *weights.d/value.d: 0.0;
		  else
		    value.d *= *weights.d;
		  trgt.d[*indx] += value.d;
		  hist.d[*indx] += *weights.d++;
		  indx++;
		}
		break;
	      case ANA_WORD:
		while (i--) {
		  temp.d = *src.w++; /* data value */
		  value.d = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.d = value.d? *weights.d/value.d: 0.0;
		  else
		    value.d *= *weights.d;
		  trgt.d[*indx] += value.d;
		  hist.d[*indx] += *weights.d++;
		  indx++;
		}
		break;
	      case ANA_LONG:
		while (i--) {
		  temp.d = *src.l++; /* data value */
		  value.d = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.d = value.d? *weights.d/value.d: 0.0;
		  else
		    value.d *= *weights.d;
		  trgt.d[*indx] += value.d;
		  hist.d[*indx] += *weights.d++;
		  indx++;
		}
		break;
	      case ANA_FLOAT:
		while (i--) {
		  temp.d = *src.f++; /* data value */
		  value.d = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.d = value.d? *weights.d/value.d: 0.0;
		  else
		    value.d *= *weights.d;
		  trgt.d[*indx] += value.d;
		  hist.d[*indx] += *weights.d++;
		  indx++;
		}
		break;
	      case ANA_DOUBLE:
		while (i--) {
		  temp.d = *src.d++; /* data value */
		  value.d = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.d = value.d? *weights.d/value.d: 0.0;
		  else
		    value.d *= *weights.d;
		  trgt.d[*indx] += value.d;
		  hist.d[*indx] += *weights.d++;
		  indx++;
		}
		break;
	    } /* end of switch (type) */
	    /* and divide by number */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.d[i])
		trgt.d[i] /= hist.d[i];
	    break;
	  case ANA_CFLOAT:
	    while (i--) {
	      tempcf.real = src.cf->real; /* data value */
	      tempcf.imaginary = src.cf++->imaginary;
	      valuecf.real = 1.0;
	      valuecf.imaginary = 0.0;
	      for (j = 0; j < nbase; j++) {
		if (present[j]) { /* valuecf *= tempcf */
		  temp2f = valuecf.real*tempcf.real
		    - valuecf.imaginary*tempcf.imaginary;
		  valuecf.imaginary = valuecf.real*tempcf.imaginary
		    + valuecf.imaginary*tempcf.real;
		  valuecf.real = temp2f;
		}
		/* replace tempcf with tempcf*tempcf */
		temp2f = tempcf.real*tempcf.real
		  - tempcf.imaginary*tempcf.imaginary;
		tempcf.imaginary = 2*tempcf.real*tempcf.imaginary;
		tempcf.real = temp2f;
	      }
	      /* we now have the data value to the given unsigned power */
	      /* add in the exponent sign and the weighte */
	      if (psign == -1) { /* replace valuecf by *weights.f/valuecf */
		temp2f = valuecf.real*valuecf.real
		  + valuecf.imaginary*valuecf.imaginary;
		if (temp2f) {
		  temp2f = *weights.f/temp2f;
		  valuecf.real *= temp2f;
		  valuecf.imaginary *= -temp2f;
		}
	      } else {
		valuecf.real *= *weights.f;
		valuecf.imaginary *= *weights.f;
	      }
	      trgt.cf[*indx].real += valuecf.real;
	      trgt.cf[*indx].imaginary += valuecf.imaginary;
	      hist.f[*indx] += *weights.f++;
	      indx++;
	    }
	    /* and divide by number */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.f[i]) {
		trgt.cf[i].real /= hist.f[i];
		trgt.cf[i].imaginary /= hist.f[i];
	      }
	    break;
	  case ANA_CDOUBLE:
	    switch (type) {
	      case ANA_CFLOAT:
		while (i--) {
		  tempcd.real = src.cf->real; /* data value */
		  tempcd.imaginary = src.cf++->imaginary;
		  valuecd.real = 1.0;
		  valuecd.imaginary = 0.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j]) { /* valuecf *= tempcf */
		      temp2d = valuecd.real*tempcd.real
			- valuecd.imaginary*tempcd.imaginary;
		      valuecd.imaginary = valuecd.real*tempcd.imaginary
			+ valuecd.imaginary*tempcd.real;
		      valuecd.real = temp2d;
		    }
		    /* replace tempcd with tempcd*tempcd */
		    temp2d = tempcd.real*tempcd.real
		      - tempcd.imaginary*tempcd.imaginary;
		    tempcd.imaginary = 2*tempcd.real*tempcd.imaginary;
		    tempcd.real = temp2d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1) {/* replace valuecd by *weights.f/valuecd */
		    temp2d = valuecd.real*valuecd.real
		      + valuecd.imaginary*valuecd.imaginary;
		    if (temp2d) {
		      temp2d = *weights.d/temp2d;
		      valuecd.real *= temp2d;
		      valuecd.imaginary *= -temp2d;
		    }
		  } else {
		    valuecd.real *= *weights.d;
		    valuecd.imaginary *= *weights.d;
		  }
		  trgt.cd[*indx].real += valuecd.real;
		  trgt.cd[*indx].imaginary += valuecd.imaginary;
		  hist.d[*indx] += *weights.d++;
		  indx++;
		}
		break;
	      case ANA_CDOUBLE:
		while (i--) {
		  tempcd.real = src.cd->real; /* data value */
		  tempcd.imaginary = src.cd++->imaginary;
		  valuecd.real = 1.0;
		  valuecd.imaginary = 0.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j]) { /* valuecf *= tempcf */
		      temp2d = valuecd.real*tempcd.real
			- valuecd.imaginary*tempcd.imaginary;
		      valuecd.imaginary = valuecd.real*tempcd.imaginary
			+ valuecd.imaginary*tempcd.real;
		      valuecd.real = temp2d;
		    }
		    /* replace tempcd with tempcd*tempcd */
		    temp2d = tempcd.real*tempcd.real
		      - tempcd.imaginary*tempcd.imaginary;
		    tempcd.imaginary = 2*tempcd.real*tempcd.imaginary;
		    tempcd.real = temp2d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1) {/* replace valuecd by *weights.f/valuecd */
		    temp2d = valuecd.real*valuecd.real
		      + valuecd.imaginary*valuecd.imaginary;
		    if (temp2d) {
		      temp2d = *weights.d/temp2d;
		      valuecd.real *= temp2d;
		      valuecd.imaginary *= -temp2d;
		    }
		  } else {
		    valuecd.real *= *weights.d;
		    valuecd.imaginary *= *weights.d;
		  }
		  trgt.cd[*indx].real += valuecd.real;
		  trgt.cd[*indx].imaginary += valuecd.imaginary;
		  hist.d[*indx] += *weights.d++;
		  indx++;
		}
		break;
	    } /* end of switch (type) */
	    /* and divide by number */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.d[i]) {
		trgt.cd[i].real /= hist.d[i];
		trgt.cd[i].imaginary /= hist.d[i];
	      }
	    break;
	} /* end of switch (outType) */
	free(hist.d - offset);
      } else {			/* want totals */
	switch (outType) {
	  case ANA_FLOAT:
	    switch (type) {
	      case ANA_BYTE:
		while (i--) {
		  temp.f = *src.b++; /* data value */
		  value.f = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.f *= temp.f;
		    temp.f *= temp.f;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.f = value.f? *weights.f/value.f: 0.0;
		  else
		    value.f *= *weights.f;
		  weights.f++;
		  trgt.f[*indx] += value.f;
		  indx++;
		}
		break;
	      case ANA_WORD:
		while (i--) {
		  temp.f = *src.w++; /* data value */
		  value.f = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.f *= temp.f;
		    temp.f *= temp.f;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.f = value.f? *weights.f/value.f: 0.0;
		  else
		    value.f *= *weights.f;
		  weights.f++;
		  trgt.f[*indx] += value.f;
		  indx++;
		}
		break;
	      case ANA_LONG:
		while (i--) {
		  temp.f = *src.l++; /* data value */
		  value.f = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.f *= temp.f;
		    temp.f *= temp.f;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.f = value.f? *weights.f/value.f: 0.0;
		  else
		    value.f *= *weights.f;
		  weights.f++;
		  trgt.f[*indx] += value.f;
		  indx++;
		}
		break;
	      case ANA_FLOAT:
		while (i--) {
		  temp.f = *src.f++; /* data value */
		  value.f = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.f *= temp.f;
		    temp.f *= temp.f;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.f = value.f? *weights.f/value.f: 0.0;
		  else
		    value.f *= *weights.f;
		  weights.f++;
		  trgt.f[*indx] += value.f;
		  indx++;
		}
		break;
	    } /* end of switch (type) */
	    break;
	  case ANA_DOUBLE:
	    switch (type) {
	      case ANA_BYTE:
		while (i--) {
		  temp.d = *src.b++; /* data value */
		  value.d = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.d = value.d? *weights.d/value.d: 0.0;
		  else
		    value.d *= *weights.d;
		  weights.d++;
		  trgt.d[*indx] += value.d;
		  indx++;
		}
		break;
	      case ANA_WORD:
		while (i--) {
		  temp.d = *src.w++; /* data value */
		  value.d = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.d = value.d? *weights.d/value.d: 0.0;
		  else
		    value.d *= *weights.d;
		  weights.d++;
		  trgt.d[*indx] += value.d;
		  indx++;
		}
		break;
	      case ANA_LONG:
		while (i--) {
		  temp.d = *src.l++; /* data value */
		  value.d = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.d = value.d? *weights.d/value.d: 0.0;
		  else
		    value.d *= *weights.d;
		  weights.d++;
		  trgt.d[*indx] += value.d;
		  indx++;
		}
		break;
	      case ANA_FLOAT:
		while (i--) {
		  temp.d = *src.f++; /* data value */
		  value.d = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.d = value.d? *weights.d/value.d: 0.0;
		  else
		    value.d *= *weights.d;
		  weights.d++;
		  trgt.d[*indx] += value.d;
		  indx++;
		}
		break;
	      case ANA_DOUBLE:
		while (i--) {
		  temp.d = *src.d++; /* data value */
		  value.d = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.d = value.d? *weights.d/value.d: 0.0;
		  else
		    value.d *= *weights.d;
		  weights.d++;
		  trgt.d[*indx] += value.d;
		  indx++;
		}
		break;
	    } /* end of switch (type) */
	    break;
	  case ANA_CFLOAT:
	    while (i--) {
	      tempcf.real = src.cf->real; /* data value */
	      tempcf.imaginary = src.cf++->imaginary;
	      valuecf.real = 1.0;
	      valuecf.imaginary = 0.0;
	      for (j = 0; j < nbase; j++) {
		if (present[j]) { /* valuecf *= tempcf */
		  temp2f = valuecf.real*tempcf.real
		    - valuecf.imaginary*tempcf.imaginary;
		  valuecf.imaginary = valuecf.real*tempcf.imaginary
		    + valuecf.imaginary*tempcf.real;
		  valuecf.real = temp2f;
		}
		/* replace tempcf with tempcf*tempcf */
		temp2f = tempcf.real*tempcf.real
		  - tempcf.imaginary*tempcf.imaginary;
		tempcf.imaginary = 2*tempcf.real*tempcf.imaginary;
		tempcf.real = temp2f;
	      }
	      /* we now have the data value to the given unsigned power */
	      /* add in the exponent sign and the weighte */
	      if (psign == -1) { /* replace valuecf by *weights.f/valuecf */
		temp2f = valuecf.real*valuecf.real
		  + valuecf.imaginary*valuecf.imaginary;
		if (temp2f) {
		  temp2f = *weights.f/temp2f;
		  valuecf.real *= temp2f;
		  valuecf.imaginary *= -temp2f;
		}
	      } else {
		valuecf.real *= *weights.f;
		valuecf.imaginary *= *weights.f;
	      }
	      weights.f++;
	      trgt.cf[*indx].real += valuecf.real;
	      trgt.cf[*indx].imaginary += valuecf.imaginary;
	      indx++;
	    }
	    break;
	  case ANA_CDOUBLE:
	    switch (type) {
	      case ANA_CFLOAT:
		while (i--) {
		  tempcd.real = src.cf->real; /* data value */
		  tempcd.imaginary = src.cf++->imaginary;
		  valuecd.real = 1.0;
		  valuecd.imaginary = 0.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j]) { /* valuecf *= tempcf */
		      temp2d = valuecd.real*tempcd.real
			- valuecd.imaginary*tempcd.imaginary;
		      valuecd.imaginary = valuecd.real*tempcd.imaginary
			+ valuecd.imaginary*tempcd.real;
		      valuecd.real = temp2d;
		    }
		    /* replace tempcd with tempcd*tempcd */
		    temp2d = tempcd.real*tempcd.real
		      - tempcd.imaginary*tempcd.imaginary;
		    tempcd.imaginary = 2*tempcd.real*tempcd.imaginary;
		    tempcd.real = temp2d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1) {/* replace valuecd by *weights.f/valuecd */
		    temp2d = valuecd.real*valuecd.real
		      + valuecd.imaginary*valuecd.imaginary;
		    if (temp2d) {
		      temp2d = *weights.d/temp2d;
		      valuecd.real *= temp2d;
		      valuecd.imaginary *= -temp2d;
		    }
		  } else {
		    valuecd.real *= *weights.d;
		    valuecd.imaginary *= *weights.d;
		  }
		  weights.d++;
		  trgt.cd[*indx].real += valuecd.real;
		  trgt.cd[*indx].imaginary += valuecd.imaginary;
		  indx++;
		}
		break;
	      case ANA_CDOUBLE:
		while (i--) {
		  tempcd.real = src.cd->real; /* data value */
		  tempcd.imaginary = src.cd++->imaginary;
		  valuecd.real = 1.0;
		  valuecd.imaginary = 0.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j]) { /* valuecf *= tempcf */
		      temp2d = valuecd.real*tempcd.real
			- valuecd.imaginary*tempcd.imaginary;
		      valuecd.imaginary = valuecd.real*tempcd.imaginary
			+ valuecd.imaginary*tempcd.real;
		      valuecd.real = temp2d;
		    }
		    /* replace tempcd with tempcd*tempcd */
		    temp2d = tempcd.real*tempcd.real
		      - tempcd.imaginary*tempcd.imaginary;
		    tempcd.imaginary = 2*tempcd.real*tempcd.imaginary;
		    tempcd.real = temp2d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1) {/* replace valuecd by *weights.f/valuecd */
		    temp2d = valuecd.real*valuecd.real
		      + valuecd.imaginary*valuecd.imaginary;
		    if (temp2d) {
		      temp2d = *weights.d/temp2d;
		      valuecd.real *= temp2d;
		      valuecd.imaginary *= -temp2d;
		    }
		  } else {
		    valuecd.real *= *weights.d;
		    valuecd.imaginary *= *weights.d;
		  }
		  weights.d++;
		  trgt.cd[*indx].real += valuecd.real;
		  trgt.cd[*indx].imaginary += valuecd.imaginary;
		  indx++;
		}
		break;
	    } /* end of switch (type) */
	    break;
	} /* end of switch (outType) */
      }	/* end of if (mean) else */
    } else {			/* unweighted power summation */
      if (mean) {		/* want averages */
	allocate(hist.l, size, int);
	zerobytes(hist.l, size*sizeof(int));
	hist.l += offset;
	switch (outType) {
	  case ANA_FLOAT:
	    switch (type) {
	      case ANA_BYTE:
		while (i--) {
		  temp.f = *src.b++; /* data value */
		  value.f = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.f *= temp.f;
		    temp.f *= temp.f;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.f = value.f? 1.0/value.f: 0.0;
		  trgt.f[*indx] += value.f;
		  hist.l[*indx]++;
		  indx++;
		}
		break;
	      case ANA_WORD:
		while (i--) {
		  temp.f = *src.w++; /* data value */
		  value.f = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.f *= temp.f;
		    temp.f *= temp.f;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.f = value.f? 1.0/value.f: 0.0;
		  trgt.f[*indx] += value.f;
		  hist.l[*indx]++;
		  indx++;
		}
		break;
	      case ANA_LONG:
		while (i--) {
		  temp.f = *src.l++; /* data value */
		  value.f = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.f *= temp.f;
		    temp.f *= temp.f;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.f = value.f? 1.0/value.f: 0.0;
		  trgt.f[*indx] += value.f;
		  hist.l[*indx]++;
		  indx++;
		}
		break;
	      case ANA_FLOAT:
		while (i--) {
		  temp.f = *src.f++; /* data value */
		  value.f = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.f *= temp.f;
		    temp.f *= temp.f;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.f = value.f? 1.0/value.f: 0.0;
		  trgt.f[*indx] += value.f;
		  hist.l[*indx]++;
		  indx++;
		}
		break;
	    } /* end of switch (type) */
	    /* and divide by number */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.f[i])
		trgt.f[i] /= hist.l[i];
	    break;
	  case ANA_DOUBLE:
	    switch (type) {
	      case ANA_BYTE:
		while (i--) {
		  temp.d = *src.b++; /* data value */
		  value.d = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.d = value.d? 1.0/value.d: 0.0;
		  trgt.d[*indx] += value.d;
		  hist.l[*indx]++;
		  indx++;
		}
		break;
	      case ANA_WORD:
		while (i--) {
		  temp.d = *src.w++; /* data value */
		  value.d = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.d = value.d? 1.0/value.d: 0.0;
		  trgt.d[*indx] += value.d;
		  hist.l[*indx]++;
		  indx++;
		}
		break;
	      case ANA_LONG:
		while (i--) {
		  temp.d = *src.l++; /* data value */
		  value.d = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.d = value.d? 1.0/value.d: 0.0;
		  trgt.d[*indx] += value.d;
		  hist.l[*indx]++;
		  indx++;
		}
		break;
	      case ANA_FLOAT:
		while (i--) {
		  temp.d = *src.f++; /* data value */
		  value.d = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.d = value.d? 1.0/value.d: 0.0;
		  trgt.d[*indx] += value.d;
		  hist.l[*indx]++;
		  indx++;
		}
		break;
	      case ANA_DOUBLE:
		while (i--) {
		  temp.d = *src.d++; /* data value */
		  value.d = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.d = value.d? 1.0/value.d: 0.0;
		  trgt.d[*indx] += value.d;
		  hist.l[*indx]++;
		  indx++;
		}
		break;
	    } /* end of switch (type) */
	    /* and divide by number */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.l[i])
		trgt.d[i] /= hist.l[i];
	    break;
	  case ANA_CFLOAT:
	    while (i--) {
	      tempcf.real = src.cf->real; /* data value */
	      tempcf.imaginary = src.cf++->imaginary;
	      valuecf.real = 1.0;
	      valuecf.imaginary = 0.0;
	      for (j = 0; j < nbase; j++) {
		if (present[j]) { /* valuecf *= tempcf */
		  temp2f = valuecf.real*tempcf.real
		    - valuecf.imaginary*tempcf.imaginary;
		  valuecf.imaginary = valuecf.real*tempcf.imaginary
		    + valuecf.imaginary*tempcf.real;
		  valuecf.real = temp2f;
		}
		/* replace tempcf with tempcf*tempcf */
		temp2f = tempcf.real*tempcf.real
		  - tempcf.imaginary*tempcf.imaginary;
		tempcf.imaginary = 2*tempcf.real*tempcf.imaginary;
		tempcf.real = temp2f;
	      }
	      /* we now have the data value to the given unsigned power */
	      /* add in the exponent sign and the weighte */
	      if (psign == -1) { /* replace valuecf by *weights.f/valuecf */
		temp2f = valuecf.real*valuecf.real
		  + valuecf.imaginary*valuecf.imaginary;
		if (temp2f) {
		  temp2f = 1.0/temp2f;
		  valuecf.real *= temp2f;
		  valuecf.imaginary *= -temp2f;
		}
	      }
	      trgt.cf[*indx].real += valuecf.real;
	      trgt.cf[*indx].imaginary += valuecf.imaginary;
	      hist.l[*indx]++;
	      indx++;
	    }
	    /* and divide by number */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.f[i]) {
		trgt.cf[i].real /= hist.l[i];
		trgt.cf[i].imaginary /= hist.l[i];
	      }
	    break;
	  case ANA_CDOUBLE:
	    switch (type) {
	      case ANA_CFLOAT:
		while (i--) {
		  tempcd.real = src.cf->real; /* data value */
		  tempcd.imaginary = src.cf++->imaginary;
		  valuecd.real = 1.0;
		  valuecd.imaginary = 0.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j]) { /* valuecf *= tempcf */
		      temp2d = valuecd.real*tempcd.real
			- valuecd.imaginary*tempcd.imaginary;
		      valuecd.imaginary = valuecd.real*tempcd.imaginary
			+ valuecd.imaginary*tempcd.real;
		      valuecd.real = temp2d;
		    }
		    /* replace tempcd with tempcd*tempcd */
		    temp2d = tempcd.real*tempcd.real
		      - tempcd.imaginary*tempcd.imaginary;
		    tempcd.imaginary = 2*tempcd.real*tempcd.imaginary;
		    tempcd.real = temp2d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1) {/* replace valuecd by *weights.f/valuecd */
		    temp2d = valuecd.real*valuecd.real
		      + valuecd.imaginary*valuecd.imaginary;
		    if (temp2d) {
		      temp2d = 1.0/temp2d;
		      valuecd.real *= temp2d;
		      valuecd.imaginary *= -temp2d;
		    }
		  }
		  trgt.cd[*indx].real += valuecd.real;
		  trgt.cd[*indx].imaginary += valuecd.imaginary;
		  hist.l[*indx]++;
		  indx++;
		}
		break;
	      case ANA_CDOUBLE:
		while (i--) {
		  tempcd.real = src.cd->real; /* data value */
		  tempcd.imaginary = src.cd++->imaginary;
		  valuecd.real = 1.0;
		  valuecd.imaginary = 0.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j]) { /* valuecf *= tempcf */
		      temp2d = valuecd.real*tempcd.real
			- valuecd.imaginary*tempcd.imaginary;
		      valuecd.imaginary = valuecd.real*tempcd.imaginary
			+ valuecd.imaginary*tempcd.real;
		      valuecd.real = temp2d;
		    }
		    /* replace tempcd with tempcd*tempcd */
		    temp2d = tempcd.real*tempcd.real
		      - tempcd.imaginary*tempcd.imaginary;
		    tempcd.imaginary = 2*tempcd.real*tempcd.imaginary;
		    tempcd.real = temp2d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1) {/* replace valuecd by *weights.f/valuecd */
		    temp2d = valuecd.real*valuecd.real
		      + valuecd.imaginary*valuecd.imaginary;
		    if (temp2d) {
		      temp2d = 1.0/temp2d;
		      valuecd.real *= temp2d;
		      valuecd.imaginary *= -temp2d;
		    }
		  }
		  trgt.cd[*indx].real += valuecd.real;
		  trgt.cd[*indx].imaginary += valuecd.imaginary;
		  hist.l[*indx]++;
		  indx++;
		}
		break;
	    } /* end of switch (type) */
	    /* and divide by number */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.l[i]) {
		trgt.cd[i].real /= hist.l[i];
		trgt.cd[i].imaginary /= hist.l[i];
	      }
	    break;
	} /* end of switch (outType) */
	free(hist.l - offset);
      } else {			/* want unweighted power totals */
	switch (outType) {
	  case ANA_FLOAT:
	    switch (type) {
	      case ANA_BYTE:
		while (i--) {
		  temp.f = *src.b++; /* data value */
		  value.f = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.f *= temp.f;
		    temp.f *= temp.f;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.f = value.f? 1.0/value.f: 0.0;
		  trgt.f[*indx] += value.f;
		  indx++;
		}
		break;
	      case ANA_WORD:
		while (i--) {
		  temp.f = *src.w++; /* data value */
		  value.f = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.f *= temp.f;
		    temp.f *= temp.f;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.f = value.f? 1.0/value.f: 0.0;
		  trgt.f[*indx] += value.f;
		  indx++;
		}
		break;
	      case ANA_LONG:
		while (i--) {
		  temp.f = *src.l++; /* data value */
		  value.f = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.f *= temp.f;
		    temp.f *= temp.f;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.f = value.f? 1.0/value.f: 0.0;
		  trgt.f[*indx] += value.f;
		  indx++;
		}
		break;
	      case ANA_FLOAT:
		while (i--) {
		  temp.f = *src.f++; /* data value */
		  value.f = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.f *= temp.f;
		    temp.f *= temp.f;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.f = value.f? 1.0/value.f: 0.0;
		  trgt.f[*indx] += value.f;
		  indx++;
		}
		break;
	    } /* end of switch (type) */
	    break;
	  case ANA_DOUBLE:
	    switch (type) {
	      case ANA_BYTE:
		while (i--) {
		  temp.d = *src.b++; /* data value */
		  value.d = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.d = value.d? 1.0/value.d: 0.0;
		  trgt.d[*indx] += value.d;
		  indx++;
		}
		break;
	      case ANA_WORD:
		while (i--) {
		  temp.d = *src.w++; /* data value */
		  value.d = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.d = value.d? 1.0/value.d: 0.0;
		  trgt.d[*indx] += value.d;
		  indx++;
		}
		break;
	      case ANA_LONG:
		while (i--) {
		  temp.d = *src.l++; /* data value */
		  value.d = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.d = value.d? 1.0/value.d: 0.0;
		  trgt.d[*indx] += value.d;
		  indx++;
		}
		break;
	      case ANA_FLOAT:
		while (i--) {
		  temp.d = *src.f++; /* data value */
		  value.d = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.d = value.d? 1.0/value.d: 0.0;
		  trgt.d[*indx] += value.d;
		  indx++;
		}
		break;
	      case ANA_DOUBLE:
		while (i--) {
		  temp.d = *src.d++; /* data value */
		  value.d = 1.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1)/* negative exponent: must divide */
		    value.d = value.d? 1.0/value.d: 0.0;
		  trgt.d[*indx] += value.d;
		  indx++;
		}
		break;
	    } /* end of switch (type) */
	    break;
	  case ANA_CFLOAT:
	    while (i--) {
	      tempcf.real = src.cf->real; /* data value */
	      tempcf.imaginary = src.cf++->imaginary;
	      valuecf.real = 1.0;
	      valuecf.imaginary = 0.0;
	      for (j = 0; j < nbase; j++) {
		if (present[j]) { /* valuecf *= tempcf */
		  temp2f = valuecf.real*tempcf.real
		    - valuecf.imaginary*tempcf.imaginary;
		  valuecf.imaginary = valuecf.real*tempcf.imaginary
		    + valuecf.imaginary*tempcf.real;
		  valuecf.real = temp2f;
		}
		/* replace tempcf with tempcf*tempcf */
		temp2f = tempcf.real*tempcf.real
		  - tempcf.imaginary*tempcf.imaginary;
		tempcf.imaginary = 2*tempcf.real*tempcf.imaginary;
		tempcf.real = temp2f;
	      }
	      /* we now have the data value to the given unsigned power */
	      /* add in the exponent sign and the weighte */
	      if (psign == -1) { /* replace valuecf by *weights.f/valuecf */
		temp2f = valuecf.real*valuecf.real
		  + valuecf.imaginary*valuecf.imaginary;
		if (temp2f) {
		  temp2f = 1.0/temp2f;
		  valuecf.real *= temp2f;
		  valuecf.imaginary *= -temp2f;
		}
	      }
	      trgt.cf[*indx].real += valuecf.real;
	      trgt.cf[*indx].imaginary += valuecf.imaginary;
	      indx++;
	    }
	    break;
	  case ANA_CDOUBLE:
	    switch (type) {
	      case ANA_CFLOAT:
		while (i--) {
		  tempcd.real = src.cf->real; /* data value */
		  tempcd.imaginary = src.cf++->imaginary;
		  valuecd.real = 1.0;
		  valuecd.imaginary = 0.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j]) { /* valuecf *= tempcf */
		      temp2d = valuecd.real*tempcd.real
			- valuecd.imaginary*tempcd.imaginary;
		      valuecd.imaginary = valuecd.real*tempcd.imaginary
			+ valuecd.imaginary*tempcd.real;
		      valuecd.real = temp2d;
		    }
		    /* replace tempcd with tempcd*tempcd */
		    temp2d = tempcd.real*tempcd.real
		      - tempcd.imaginary*tempcd.imaginary;
		    tempcd.imaginary = 2*tempcd.real*tempcd.imaginary;
		    tempcd.real = temp2d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1) {/* replace valuecd by *weights.f/valuecd */
		    temp2d = valuecd.real*valuecd.real
		      + valuecd.imaginary*valuecd.imaginary;
		    if (temp2d) {
		      temp2d = 1.0/temp2d;
		      valuecd.real *= temp2d;
		      valuecd.imaginary *= -temp2d;
		    }
		  }
		  trgt.cd[*indx].real += valuecd.real;
		  trgt.cd[*indx].imaginary += valuecd.imaginary;
		  indx++;
		}
		break;
	      case ANA_CDOUBLE:
		while (i--) {
		  tempcd.real = src.cd->real; /* data value */
		  tempcd.imaginary = src.cd++->imaginary;
		  valuecd.real = 1.0;
		  valuecd.imaginary = 0.0;
		  for (j = 0; j < nbase; j++) {
		    if (present[j]) { /* valuecf *= tempcf */
		      temp2d = valuecd.real*tempcd.real
			- valuecd.imaginary*tempcd.imaginary;
		      valuecd.imaginary = valuecd.real*tempcd.imaginary
			+ valuecd.imaginary*tempcd.real;
		      valuecd.real = temp2d;
		    }
		    /* replace tempcd with tempcd*tempcd */
		    temp2d = tempcd.real*tempcd.real
		      - tempcd.imaginary*tempcd.imaginary;
		    tempcd.imaginary = 2*tempcd.real*tempcd.imaginary;
		    tempcd.real = temp2d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weighte */
		  if (psign == -1) {/* replace valuecd by *weights.f/valuecd */
		    temp2d = valuecd.real*valuecd.real
		      + valuecd.imaginary*valuecd.imaginary;
		    if (temp2d) {
		      temp2d = 1.0/temp2d;
		      valuecd.real *= temp2d;
		      valuecd.imaginary *= -temp2d;
		    }
		  }
		  trgt.cd[*indx].real += valuecd.real;
		  trgt.cd[*indx].imaginary += valuecd.imaginary;
		  indx++;
		}
		break;
	    } /* end of switch (type) */
	    break;
	} /* end of switch (outType) */
      }	/* end of if (mean) else */
    } /* end of if (haveWeights) */
  } /* end of if (p == 1) else */

  return result;
}
/*-----------------------------------------------------------------------*/
int total(int narg, int ps[], int mean)
/* TOTAL(x, [ mode, POWER=p, WEIGHTS=w, /KEEPDIMS, /FLOAT, /DOUBLE]) */

/* TOTAL(array) sums all elements of <array> and returns a ANA_SCALAR. */
/* TOTAL(array, axis) sums all elements along dimension <axis> (if this */
/*   is a ANA_SCALAR) and returns an array with one less dimensions than */
/*   <array> (or a scalar).  If <axis> is an array with a number of
     elements different from that of <array>, then it is taken to be a
     list of dimensions along which must be summed.  */
/* TOTAL(array, index) collects each element of <array> at index <index> */
/*   in the result, if <index> is an ANA_ARRAY with the same number of
     elements as <array>.  */
/* The result is at least ANA_LONG.   LS 14jan96 */
/* TOTAL(array [, axis], POWER=p) returns the total of the <p>th */
/*   (integer) power of <array>.  LS 22jul98 */
/* Fixed erroneous cast to (float) in (double) summations.  LS 11jul2000 */
/* Allow ANA_LONG output.  LS 27oct2010 */
{
  int	result, done, p, psign, pp, outtype, type, nbase, i, haveWeights, n;
  byte	*present;
  scalar	sum, value, temp, w;
  floatComplex	sumcf, tempcf, valuecf;
  doubleComplex	sumcd, tempcd, valuecd;
  float	temp2f;
  double	temp2d;
  pointer	src, trgt, weights;
  loopInfo	srcinfo, trgtinfo, winfo;
  
#if DEBUG_VOCAL
  debugout1("in total(), %d arg(s)", narg);
  debugout("checking <x>, <mode>");
#endif
  if (narg > 1 && ps[1] && symbolIsNumericalArray(ps[1])
      && symbolIsNumericalArray(ps[0])
      && array_size(ps[1]) == array_size(ps[0])) /* collect by class */
    return index_total(narg, ps, mean);
  
  if (narg > 2 && ps[2]) {	/* have <power> */
#if DEBUG_VOCAL
    debugout("checking <power>");
#endif
    if (!symbolIsScalar(ps[2]))
      return cerror(NEED_SCAL, ps[2]);
    p = int_arg(ps[2]);
    if (p < 0) {
      psign = -1;			/* sign of power */
      p = -p;
    } else
      psign = +1;
  } else {
    p = 1;
    psign = +1;
  }
  
#if DEBUG_VOCAL
  debugout("determining output type");
#endif
  type = array_type(ps[0]);
  
  outtype = type;
  switch (internalMode & 5) {
    case 0:
      if (outtype < ANA_LONG)
        outtype = ANA_LONG;
      break;
    case 1:                       /* /DOUBLE */
    case 5:                       /* /DOUBLE, /FLOAT */
      if (isComplexType(type))
        outtype = ANA_CDOUBLE;
      else if (outtype < ANA_DOUBLE)
        outtype = ANA_DOUBLE;
      break;
    case 4:                       /* /FLOAT */
      if (outtype < ANA_FLOAT)
        outtype = ANA_FLOAT;
      break;
  }
#if DEBUG_VOCAL
  debugout1("output type: %s", typeName(outtype));
#endif

  if (narg > 3 && ps[3]) {	/* have <weights> */
#if DEBUG_VOCAL
    debugout("checking <weights>");
#endif
    if (!symbolIsNumericalArray(ps[3]) /* not a numerical array */
	|| array_size(ps[3]) != array_size(ps[0])) /* or wrong size */
      return cerror(INCMP_ARG, ps[3]);
    for (i = 0; i < array_num_dims(ps[3]) - 1; i++) /* check dimensions */
      if (array_dims(ps[3])[i] != array_dims(ps[0])[i])
	return cerror(INCMP_DIMS, ps[3]);
    haveWeights = 1;
  } else
    haveWeights = 0;

#if DEBUG_VOCAL
  debugout("standardLoop for <x>");
#endif
  /* set up for walking through <x> */
  if (standardLoop(ps[0], narg > 1? ps[1]: 0,
		   SL_COMPRESSALL /* selected dims removed from result */
		   | SL_UPGRADE	/* result at least of source type */
		   | SL_EACHCOORD /* want all coordinates */
		   | SL_UNIQUEAXES /* no duplicate axes allowed */
		   | SL_AXESBLOCK /* rearrange selected axes to front */
		   | SL_NEGONED	/* axis negative -> treat as 1D */
		   | ((internalMode & 2)? SL_ONEDIMS: 0), /* omit -> 1 */
		   outtype,
		   &srcinfo, &src, &result, &trgtinfo, &trgt) < 0)
    return ANA_ERROR;
#if DEBUG_VOCAL
  debugout("back from standardLoop for <x>");
#endif

  /* set up for walking through <weights> as well (if present) */
  if (haveWeights) {
#if DEBUG_VOCAL
    debugout("standardLoop for <weights>");
#endif
    /* we make sure that <weights> has the same data type as <x> -- except */
    /* that <weights> is never complex */
    haveWeights = ana_converts[realType(array_type(ps[0]))](1, &ps[3]);
    if (standardLoop(haveWeights, ps[1],
		     (ps[1]? 0: SL_ALLAXES) | SL_EACHCOORD | SL_AXESBLOCK,
		     0, &winfo, &weights, NULL, NULL, NULL) < 0)
      return ANA_ERROR;
  }
    
  
  if (!haveWeights) {
#if DEBUG_VOCAL
    debugout("calculating # elements per result");
#endif
    /* calculate how many elements go into each result */
    if (srcinfo.naxes) {	/* user specified one or more axes */
      n = 1;
      for (i = 0; i < srcinfo.naxes; i++)
	n *= srcinfo.dims[srcinfo.axes[i]];
    } else if (symbolIsArray(ps[0]))
      n = array_size(ps[0]);
    else
      n = 1;
#if DEBUG_VOCAL
    debugout1("# elements per result: %d", n);
#endif
  }

  if (!srcinfo.naxes) {		/* no axes specified */
#if DEBUG_VOCAL
    debugout("adjusting srcinfo.naxes");
#endif
    srcinfo.naxes++;		/* or no proper summing */
    if (haveWeights) {
#if DEBUG_VOCAL
      debugout("adjusting winfo.naxes");
#endif
      winfo.naxes++;
    }
  }
  
  if (p == 1) {			/* regular summation */
    if (haveWeights) {		/* have <weights> */
#if DEBUG_VOCAL
      debugout("Weighted regular summation");
#endif
      switch (outtype) {
	case ANA_LONG:
	  switch (type) {
	    case ANA_BYTE:
	      do {
		sum.l = 0.0;
		w.l = 0.0;
		do {
		  sum.l += *src.b * *weights.b;
		  w.l += *weights.b;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		*trgt.l++ = mean? (w.l? sum.l/w.l: 0): sum.l;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_WORD:
	      do {
		sum.l = 0.0;
		w.l = 0.0;
		do {
		  sum.l += *src.w * *weights.w;
		  w.l += *weights.w;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		*trgt.l++ = mean? (w.l? sum.l/w.l: 0): sum.l;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_LONG:
	      do {
		sum.l = 0.0;
		w.l = 0.0;
		do {
		  sum.l += *src.l * *weights.l;
		  w.l += *weights.l;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		*trgt.l++ = mean? (w.l? sum.l/w.l: 0): sum.l;
	      } while (done < srcinfo.rndim);
	      break;
	  } /* end of switch (type) */
	  break;
	case ANA_FLOAT:
	  switch (type) {
	    case ANA_BYTE:
	      do {
		sum.f = 0.0;
		w.f = 0.0;
		do {
		  sum.f += *src.b * *weights.b;
		  w.f += *weights.b;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		*trgt.f++ = mean? (w.f? sum.f/w.f: 0): sum.f;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_WORD:
	      do {
		sum.f = 0.0;
		w.f = 0.0;
		do {
		  sum.f += *src.w * *weights.w;
		  w.f += *weights.w;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		*trgt.f++ = mean? (w.f? sum.f/w.f: 0): sum.f;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_LONG:
	      do {
		sum.f = 0.0;
		w.f = 0.0;
		do {
		  sum.f += *src.l * *weights.l;
		  w.f += *weights.l;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		*trgt.f++ = mean? (w.f? sum.f/w.f: 0): sum.f;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_FLOAT:
	      do {
		sum.f = 0.0;
		w.f = 0.0;
		do {
		  sum.f += *src.f * *weights.f;
		  w.f += *weights.f;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		*trgt.f++ = mean? (w.f? sum.f/w.f: 0): sum.f;
	      } while (done < srcinfo.rndim);
	      break;
            /* no cases ANA_DOUBLE, ANA_CFLOAT, or ANA_CDOUBLE: if <x> */
	    /* is any of those types, then so is the output */
	  } /* end of switch (type) */
	  break;
	case ANA_DOUBLE:
	  switch (type) {
	    case ANA_BYTE:
	      do {
		sum.d = 0.0;
		w.d = 0.0;
		do {
		  sum.d += (double) *src.b * *weights.b;
		  w.d += *weights.b;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		*trgt.d++ = mean? (w.d? sum.d/w.d: 0): sum.d;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_WORD:
	      do {
		sum.d = 0.0;
		w.d = 0.0;
		do {
		  sum.d += (double) *src.w * *weights.w;
		  w.d += *weights.w;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		*trgt.d++ = mean? (w.d? sum.d/w.d: 0): sum.d;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_LONG:
	      do {
		sum.d = 0.0;
		w.d = 0.0;
		do {
		  sum.d += (double) *src.l * *weights.l;
		  w.d += *weights.l;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		*trgt.d++ = mean? (w.d? sum.d/w.d: 0): sum.d;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_FLOAT:
	      do {
		sum.d = 0.0;
		w.d = 0.0;
		do {
		  sum.d += (double) *src.f * *weights.f;
		  w.d += *weights.f;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		*trgt.d++ = mean? (w.d? sum.d/w.d: 0): sum.d;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_DOUBLE:
	      do {
		sum.d = 0.0;
		w.d = 0.0;
		do {
		  sum.d += *src.d * *weights.d;
		  w.d += *weights.d;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		*trgt.d++ = mean? (w.d? sum.d/w.d: 0): sum.d;
	      } while (done < srcinfo.rndim);
	      break;
            /* no cases ANA_CFLOAT, or ANA_CDOUBLE: if <x> */
	    /* is any of those types, then so is the output */
	  } /* end of switch (type) */
	  break;
	case ANA_CFLOAT:
	  /* if we get here then <x> must itself be CFLOAT */
	  do {
	    sumcf.real = sumcf.imaginary = 0.0;
	    w.f = 0.0;
	    do {
	      sumcf.real += src.cf->real * *weights.f;
	      sumcf.imaginary += src.cf->imaginary * *weights.f;
	      w.f += *weights.f;
	    } while ((done = (advanceLoop(&winfo, &weights),
			      advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
	    if (mean) {
	      if (w.f) {
		trgt.cf->real = sumcf.real/w.f;
		trgt.cf->imaginary = sumcf.imaginary/w.f;
	      } else
		trgt.cf->real = trgt.cf->imaginary = 0.0;
	    } else {
	      trgt.cf->real = sumcf.real;
	      trgt.cf->imaginary = sumcf.imaginary;
	    }
	    trgt.cf++;
	  } while (done < srcinfo.rndim);
	  break;
	case ANA_CDOUBLE:
	  /* if we get here then <x> must be CFLOAT or CDOUBLE */
	  switch (type) {
	    case ANA_CFLOAT:
	      do {
		sumcd.real = sumcd.imaginary = 0.0;
		w.d = 0.0;
		do {
		  sumcd.real += (double) src.cf->real * *weights.f;
		  sumcd.imaginary += (double) src.cf->imaginary * *weights.f;
		  w.d += *weights.d;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		if (mean) {
		  if (w.d) {
		    trgt.cd->real = sumcd.real/w.d;
		    trgt.cd->imaginary = sumcd.imaginary/w.d;
		  } else
		    trgt.cd->real = trgt.cd->imaginary = 0.0;
		} else {
		  trgt.cd->real = sumcd.real;
		  trgt.cd->imaginary = sumcd.imaginary;
		}
		trgt.cd++;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_CDOUBLE:
	      do {
		sumcd.real = sumcd.imaginary = 0.0;
		w.d = 0.0;
		do {
		  sumcd.real += src.cd->real * *weights.d;
		  sumcd.imaginary += src.cd->imaginary * *weights.d;
		  w.d += *weights.d;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		if (mean) {
		  if (w.d) {
		    trgt.cd->real = sumcd.real/w.d;
		    trgt.cd->imaginary = sumcd.imaginary/w.d;
		  } else
		    trgt.cd->real = trgt.cd->imaginary = 0.0;
		} else {
		  trgt.cd->real = sumcd.real;
		  trgt.cd->imaginary = sumcd.imaginary;
		}
		trgt.cd++;
	      } while (done < srcinfo.rndim);
	      break;
	  } /* end of switch (type) */
	  break;
      }	/* end of switch (outtype) */
    } else {			/* no <weights>: plain summation */
#if DEBUG_VOCAL
      debugout("Regular unweighted summing");
#endif
      switch (outtype) {
	case ANA_LONG:
	  switch (type) {
	    case ANA_BYTE:
	      do {
		sum.l = 0.0;
		do
		  sum.l += *src.b;
		while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		*trgt.l++ = mean? sum.l/n: sum.l;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_WORD:
	      do {
		sum.l = 0.0;
		do
		  sum.l += *src.w;
		while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		*trgt.l++ = mean? sum.l/n: sum.l;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_LONG:
	      do {
		sum.l = 0.0;
		do
		  sum.l += *src.l;
		while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		*trgt.l++ = mean? sum.l/n: sum.l;
	      } while (done < srcinfo.rndim);
	      break;
	  } /* end of switch (type) */
	  break;
	case ANA_FLOAT:
	  switch (type) {
	    case ANA_BYTE:
	      do {
		sum.f = 0.0;
		do
		  sum.f += *src.b;
		while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		*trgt.f++ = mean? sum.f/n: sum.f;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_WORD:
	      do {
		sum.f = 0.0;
		do
		  sum.f += *src.w;
		while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		*trgt.f++ = mean? sum.f/n: sum.f;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_LONG:
	      do {
		sum.f = 0.0;
		do
		  sum.f += *src.l;
		while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		*trgt.f++ = mean? sum.f/n: sum.f;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_FLOAT:
	      do {
		sum.f = 0.0;
		do
		  sum.f += *src.f;
		while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		*trgt.f++ = mean? sum.f/n: sum.f;
	      } while (done < srcinfo.rndim);
	      break;
           /* no cases DOUBLE, CFLOAT, or CDOUBLE */
	  } /* end of switch (type) */
	  break;
	case ANA_DOUBLE:
	  switch (type) {
	    case ANA_BYTE:
	      do {
		sum.d = 0.0;
		do
		  sum.d += *src.b;
		while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		*trgt.d++ = mean? sum.d/n: sum.d;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_WORD:
	      do {
		sum.d = 0.0;
		do
		  sum.d += *src.w;
		while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		*trgt.d++ = mean? sum.d/n: sum.d;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_LONG:
	      do {
		sum.d = 0.0;
		do
		  sum.d += *src.l;
		while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		*trgt.d++ = mean? sum.d/n: sum.d;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_FLOAT:
	      do {
		sum.d = 0.0;
		do
		  sum.d += *src.f;
		while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		*trgt.d++ = mean? sum.d/n: sum.d;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_DOUBLE:
	      do {
		sum.d = 0.0;
		do
		  sum.d += *src.d;
		while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		*trgt.d++ = mean? sum.d/n: sum.d;
	      } while (done < srcinfo.rndim);
	      break;
           /* no cases CFLOAT, CDOUBLE */
	  } /* end of switch (type) */
	  break;
	case ANA_CFLOAT:
	  do {
	    sumcf.real = sumcf.imaginary = 0.0;
	    do {
	      sumcf.real += src.cf->real;
	      sumcf.imaginary += src.cf->imaginary;
	    } while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
	    if (mean) {
	      trgt.cf->real = sumcf.real/n;
	      trgt.cf++->imaginary = sumcf.imaginary/n;
	    } else {
	      trgt.cf->real = sumcf.real;
	      trgt.cf++->imaginary = sumcf.imaginary;
	    }
	  } while (done < srcinfo.rndim);
	  break;
	case ANA_CDOUBLE:
	  switch (type) {
	    case ANA_CFLOAT:
	      do {
		sumcd.real = sumcd.imaginary = 0.0;
		do {
		  sumcd.real += src.cf->real;
		  sumcd.imaginary += src.cf->imaginary;
		} while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		if (mean) {
		  trgt.cd->real = sumcd.real/n;
		  trgt.cd++->imaginary = sumcd.imaginary/n;
		} else {
		  trgt.cd->real = sumcd.real;
		  trgt.cd++->imaginary = sumcd.imaginary;
		}
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_CDOUBLE:
	      do {
		sumcd.real = sumcd.imaginary = 0.0;
		do {
		  sumcd.real += src.cd->real;
		  sumcd.imaginary += src.cd->imaginary;
		} while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		if (mean) {
		  trgt.cd->real = sumcd.real/n;
		  trgt.cd++->imaginary = sumcd.imaginary/n;
		} else {
		  trgt.cd->real = sumcd.real;
		  trgt.cd++->imaginary = sumcd.imaginary;
		}
	      } while (done < srcinfo.rndim);
	      break;
	  } /* end of switch (type) */
	  break;
      }	/* end of switch (outtype) */
    } /* end of if (haveWeights) else */
  } else {			/* power summation */
#if DEBUG_VOCAL
    debugout("power summation");
#endif
    /* we set up for the calculation of the powers.  We use a scheme that */
    /* minimizes the number of multiplications that need to be performed. */
    present = (byte *) curScrat;/* some scratch space */
    pp = p;
    while (pp) {
      *present++ = (pp & 1);
      pp >>= 1;
    }
    nbase = present - (byte *) curScrat; /* number of bits in the exponent */
    present = (byte *) curScrat;
    if (haveWeights) {		/* weighted power summation */
#if DEBUG_VOCAL
      debugout("weighted power summation");
#endif
      switch (outtype) {
	case ANA_LONG:
	  switch (type) {
	    case ANA_BYTE:
	      do {
		sum.l = w.l = 0.0;
		do {
		  temp.l = *src.b; /* data value */
		  value.l = 1.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i])
		      value.l *= temp.l;
		    temp.l *= temp.l;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weight */
		  if (psign == -1) /* negative exponent: must divide */
		    value.l = value.l? *weights.b/value.l: 0.0;
		  else
		    value.l *= *weights.b;
		  sum.l += value.l;
		  w.l += *weights.b;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		*trgt.l++ = mean? (w.l? sum.l/w.l: 0.0): sum.l;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_WORD:
	      do {
		sum.l = w.l = 0.0;
		do {
		  temp.l = *src.w; /* data value */
		  value.l = 1.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i])
		      value.l *= temp.l;
		    temp.l *= temp.l;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weight */
		  if (psign == -1) /* negative exponent: must divide */
		    value.l = value.l? *weights.w/value.l: 0.0;
		  else
		    value.l *= *weights.w;
		  sum.l += value.l;
		  w.l += *weights.w;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		*trgt.l++ = mean? (w.l? sum.l/w.l: 0.0): sum.l;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_LONG:
	      do {
		sum.l = w.l = 0.0;
		do {
		  temp.l = *src.l; /* data value */
		  value.l = 1.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i])
		      value.l *= temp.l;
		    temp.l *= temp.l;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weight */
		  if (psign == -1) /* negative exponent: must divide */
		    value.l = value.l? *weights.l/value.l: 0.0;
		  else
		    value.l *= *weights.l;
		  sum.l += value.l;
		  w.l += *weights.l;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		*trgt.l++ = mean? (w.l? sum.l/w.l: 0.0): sum.l;
	      } while (done < srcinfo.rndim);
	      break;
	  } /* end of switch (type) */
	  break;
	case ANA_FLOAT:
	  switch (type) {
	    case ANA_BYTE:
	      do {
		sum.f = w.f = 0.0;
		do {
		  temp.f = *src.b; /* data value */
		  value.f = 1.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i])
		      value.f *= temp.f;
		    temp.f *= temp.f;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weight */
		  if (psign == -1) /* negative exponent: must divide */
		    value.f = value.f? *weights.b/value.f: 0.0;
		  else
		    value.f *= *weights.b;
		  sum.f += value.f;
		  w.f += *weights.b;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		*trgt.f++ = mean? (w.f? sum.f/w.f: 0.0): sum.f;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_WORD:
	      do {
		sum.f = w.f = 0.0;
		do {
		  temp.f = *src.w; /* data value */
		  value.f = 1.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i])
		      value.f *= temp.f;
		    temp.f *= temp.f;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weight */
		  if (psign == -1) /* negative exponent: must divide */
		    value.f = value.f? *weights.w/value.f: 0.0;
		  else
		    value.f *= *weights.w;
		  sum.f += value.f;
		  w.f += *weights.w;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		*trgt.f++ = mean? (w.f? sum.f/w.f: 0.0): sum.f;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_LONG:
	      do {
		sum.f = w.f = 0.0;
		do {
		  temp.f = *src.l; /* data value */
		  value.f = 1.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i])
		      value.f *= temp.f;
		    temp.f *= temp.f;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weight */
		  if (psign == -1) /* negative exponent: must divide */
		    value.f = value.f? *weights.l/value.f: 0.0;
		  else
		    value.f *= *weights.l;
		  sum.f += value.f;
		  w.f += *weights.l;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		*trgt.f++ = mean? (w.f? sum.f/w.f: 0.0): sum.f;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_FLOAT:
	      do {
		sum.f = w.f = 0.0;
		do {
		  temp.f = *src.f; /* data value */
		  value.f = 1.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i])
		      value.f *= temp.f;
		    temp.f *= temp.f;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weight */
		  if (psign == -1) /* negative exponent: must divide */
		    value.f = value.f? *weights.f/value.f: 0.0;
		  else
		    value.f *= *weights.f;
		  sum.f += value.f;
		  w.f += *weights.f;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		*trgt.f++ = mean? (w.f? sum.f/w.f: 0.0): sum.f;
	      } while (done < srcinfo.rndim);
	      break;
	  } /* end of switch (type) */
	  break;
	case ANA_DOUBLE:
	  switch (type) {
	    case ANA_BYTE:
	      do {
		sum.d = w.d = 0.0;
		do {
		  temp.d = *src.b; /* data value */
		  value.d = 1.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weight */
		  if (psign == -1) /* negative exponent: must divide */
		    value.d = value.d? *weights.b/value.d: 0.0;
		  else
		    value.d *= *weights.b;
		  sum.d += value.d;
		  w.d += *weights.b;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		*trgt.d++ = mean? (w.d? sum.d/w.d: 0.0): sum.d;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_WORD:
	      do {
		sum.d = w.d = 0.0;
		do {
		  temp.d = *src.w; /* data value */
		  value.d = 1.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weight */
		  if (psign == -1) /* negative exponent: must divide */
		    value.d = value.d? *weights.w/value.d: 0.0;
		  else
		    value.d *= *weights.w;
		  sum.d += value.d;
		  w.d += *weights.w;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		*trgt.d++ = mean? (w.d? sum.d/w.d: 0.0): sum.d;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_LONG:
	      do {
		sum.d = w.d = 0.0;
		do {
		  temp.d = *src.l; /* data value */
		  value.d = 1.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weight */
		  if (psign == -1) /* negative exponent: must divide */
		    value.d = value.d? *weights.l/value.d: 0.0;
		  else
		    value.d *= *weights.l;
		  sum.d += value.d;
		  w.d += *weights.l;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		*trgt.d++ = mean? (w.d? sum.d/w.d: 0.0): sum.d;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_FLOAT:
	      do {
		sum.d = w.d = 0.0;
		do {
		  temp.d = *src.f; /* data value */
		  value.d = 1.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weight */
		  if (psign == -1) /* negative exponent: must divide */
		    value.d = value.d? *weights.f/value.d: 0.0;
		  else
		    value.d *= *weights.f;
		  sum.d += value.d;
		  w.d += *weights.f;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		*trgt.d++ = mean? (w.d? sum.d/w.d: 0.0): sum.d;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_DOUBLE:
	      do {
		sum.d = w.d = 0.0;
		do {
		  temp.d = *src.d; /* data value */
		  value.d = 1.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weight */
		  if (psign == -1) /* negative exponent: must divide */
		    value.d = value.d? *weights.d/value.d: 0.0;
		  else
		    value.d *= *weights.d;
		  sum.d += value.d;
		  w.d += *weights.d;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		*trgt.d++ = mean? (w.d? sum.d/w.d: 0.0): sum.d;
	      } while (done < srcinfo.rndim);
	      break;
	  } /* end of switch (type) */
	  break;
	case ANA_CFLOAT:
	  do {
	    sumcf.real = sumcf.imaginary = w.f = 0.0;
	    do {
	      tempcf.real = src.cf->real;
	      tempcf.imaginary = src.cf->imaginary;
	      valuecf.real = 1.0;
	      valuecf.imaginary = 0.0;
	      for (i = 0; i < nbase; i++) {
		if (present[i]) { /* valuecf *= tempcf */
		  temp2f = valuecf.real*tempcf.real
		    - valuecf.imaginary*tempcf.imaginary;
		  valuecf.imaginary = valuecf.real*tempcf.imaginary
		    + valuecf.imaginary*tempcf.real;
		  valuecf.real = temp2f;
		}
		/* replace tempcf with tempcf*tempcf */
		temp2f = tempcf.real*tempcf.real
		  - tempcf.imaginary*tempcf.imaginary;
		tempcf.imaginary = 2*tempcf.real*tempcf.imaginary;
		tempcf.real = temp2f;
	      }
	      if (psign == -1) { /* replace valuecf by *weights.f/valuecf */
		temp2f = valuecf.real*valuecf.real
		  + valuecf.imaginary*valuecf.imaginary;
		if (temp2f) {
		  temp2f = *weights.f/temp2f;
		  valuecf.real *= temp2f;
		  valuecf.imaginary *= -temp2f;
		}
	      } else {
		valuecf.real *= *weights.f;
		valuecf.imaginary *= *weights.f;
	      }
	      sumcf.real += valuecf.real;
	      sumcf.imaginary *= valuecf.imaginary;
	      w.f += *weights.f;
	    } while ((done = (advanceLoop(&winfo, &weights),
			      advanceLoop(&srcinfo, &src)))
		     < srcinfo.naxes);
	    if (mean) {
	      if (w.f) {
		trgt.cf->real = sumcf.real/w.f;
		trgt.cf->imaginary = sumcf.imaginary/w.f;
	      } else
		trgt.cf->real = trgt.cf->imaginary = 0.0;
	    } else {
	      trgt.cf->real = sumcf.real;
	      trgt.cf->imaginary = sumcf.imaginary;
	    }
	    trgt.cf++;
	  } while (done < srcinfo.rndim);
	  break;
	case ANA_CDOUBLE:
	  switch (type) {
	    case ANA_CFLOAT:
	      do {
		sumcd.real = sumcd.imaginary = w.d = 0.0;
		do {
		  tempcd.real = src.cf->real;
		  tempcd.imaginary = src.cf->imaginary;
		  valuecd.real = 1.0;
		  valuecd.imaginary = 0.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i]) { /* valuecd *= tempcd */
		      temp2d = valuecd.real*tempcd.real
			- valuecd.imaginary*tempcd.imaginary;
		      valuecd.imaginary = valuecd.real*tempcd.imaginary
			+ valuecd.imaginary*tempcd.real;
		      valuecd.real = temp2d;
		    }
		    /* replace tempcd with tempcd*tempcd */
		    temp2d = tempcd.real*tempcd.real
		      - tempcd.imaginary*tempcd.imaginary;
		    tempcd.imaginary = 2*tempcd.real*tempcd.imaginary;
		    tempcd.real = temp2d;
		  }
		  if (psign == -1) { /* replace valuecd by *weights.f/valuecd */
		    temp2d = valuecd.real*valuecd.real
		      + valuecd.imaginary*valuecd.imaginary;
		    if (temp2d) {
		      temp2d = *weights.f/temp2d;
		      valuecd.real *= temp2d;
		      valuecd.imaginary *= -temp2d;
		    }
		  } else {
		    valuecd.real *= *weights.f;
		    valuecd.imaginary *= *weights.f;
		  }
		  sumcd.real += valuecd.real;
		  sumcd.imaginary *= valuecd.imaginary;
		  w.d += *weights.f;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		if (mean) {
		  if (w.d) {
		    trgt.cd->real = sumcd.real/w.d;
		    trgt.cd->imaginary = sumcd.imaginary/w.d;
		  } else
		    trgt.cd->real = trgt.cd->imaginary = 0.0;
		} else {
		  trgt.cd->real = sumcd.real;
		  trgt.cd->imaginary = sumcd.imaginary;
		}
		trgt.cd++;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_CDOUBLE:
	      do {
		sumcd.real = sumcd.imaginary = w.d = 0.0;
		do {
		  tempcd.real = src.cd->real;
		  tempcd.imaginary = src.cd->imaginary;
		  valuecd.real = 1.0;
		  valuecd.imaginary = 0.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i]) { /* valuecd *= tempcd */
		      temp2d = valuecd.real*tempcd.real
			- valuecd.imaginary*tempcd.imaginary;
		      valuecd.imaginary = valuecd.real*tempcd.imaginary
			+ valuecd.imaginary*tempcd.real;
		      valuecd.real = temp2d;
		    }
		    /* replace tempcd with tempcd*tempcd */
		    temp2d = tempcd.real*tempcd.real
		      - tempcd.imaginary*tempcd.imaginary;
		    tempcd.imaginary = 2*tempcd.real*tempcd.imaginary;
		    tempcd.real = temp2d;
		  }
		  if (psign == -1) { /* replace valuecd by *weights.f/valuecd */
		    temp2d = valuecd.real*valuecd.real
		      + valuecd.imaginary*valuecd.imaginary;
		    if (temp2d) {
		      temp2d = *weights.d/temp2d;
		      valuecd.real *= temp2d;
		      valuecd.imaginary *= -temp2d;
		    }
		  } else {
		    valuecd.real *= *weights.d;
		    valuecd.imaginary *= *weights.d;
		  }
		  sumcd.real += valuecd.real;
		  sumcd.imaginary *= valuecd.imaginary;
		  w.d += *weights.d;
		} while ((done = (advanceLoop(&winfo, &weights),
				  advanceLoop(&srcinfo, &src)))
			 < srcinfo.naxes);
		if (mean) {
		  if (w.d) {
		    trgt.cd->real = sumcd.real/w.d;
		    trgt.cd->imaginary = sumcd.imaginary/w.d;
		  } else
		    trgt.cd->real = trgt.cd->imaginary = 0.0;
		} else {
		  trgt.cd->real = sumcd.real;
		  trgt.cd->imaginary = sumcd.imaginary;
		}
		trgt.cd++;
	      } while (done < srcinfo.rndim);
	      break;
	  } /* end of switch (type) */
	  break;
      }	/* end of switch (outtype) */
    } else {			/* unweighted power summation */
#if DEBUG_VOCAL
      debugout("unweighted power summation");
#endif
      switch (outtype) {
	case ANA_LONG:
	  switch (type) {
	    case ANA_BYTE:
	      do {
		sum.l = 0.0;
		do {
		  temp.l = *src.b; /* data value */
		  value.l = 1.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i])
		      value.l *= temp.l;
		    temp.l *= temp.l;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weight */
		  if (psign == -1) /* negative exponent: must divide */
		    value.l = value.l? 1.0/value.l: 0.0;
		  sum.l += value.l;
		} while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		*trgt.l++ = mean? sum.l/n: sum.l;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_WORD:
	      do {
		sum.l = 0.0;
		do {
		  temp.l = *src.w; /* data value */
		  value.l = 1.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i])
		      value.l *= temp.l;
		    temp.l *= temp.l;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weight */
		  if (psign == -1) /* negative exponent: must divide */
		    value.l = value.l? 1.0/value.l: 0.0;
		  sum.l += value.l;
		} while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		*trgt.l++ = mean? sum.l/n: sum.l;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_LONG:
	      do {
		sum.l = 0.0;
		do {
		  temp.l = *src.l; /* data value */
		  value.l = 1.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i])
		      value.l *= temp.l;
		    temp.l *= temp.l;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weight */
		  if (psign == -1) /* negative exponent: must divide */
		    value.l = value.l? 1.0/value.l: 0.0;
		  sum.l += value.l;
		} while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		*trgt.l++ = mean? sum.l/n: sum.l;
	      } while (done < srcinfo.rndim);
	      break;
	  } /* end of switch (type) */
	  break;
	case ANA_FLOAT:
	  switch (type) {
	    case ANA_BYTE:
	      do {
		sum.f = 0.0;
		do {
		  temp.f = *src.b; /* data value */
		  value.f = 1.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i])
		      value.f *= temp.f;
		    temp.f *= temp.f;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weight */
		  if (psign == -1) /* negative exponent: must divide */
		    value.f = value.f? 1.0/value.f: 0.0;
		  sum.f += value.f;
		} while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		*trgt.f++ = mean? sum.f/n: sum.f;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_WORD:
	      do {
		sum.f = 0.0;
		do {
		  temp.f = *src.w; /* data value */
		  value.f = 1.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i])
		      value.f *= temp.f;
		    temp.f *= temp.f;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weight */
		  if (psign == -1) /* negative exponent: must divide */
		    value.f = value.f? 1.0/value.f: 0.0;
		  sum.f += value.f;
		} while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		*trgt.f++ = mean? sum.f/n: sum.f;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_LONG:
	      do {
		sum.f = 0.0;
		do {
		  temp.f = *src.l; /* data value */
		  value.f = 1.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i])
		      value.f *= temp.f;
		    temp.f *= temp.f;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weight */
		  if (psign == -1) /* negative exponent: must divide */
		    value.f = value.f? 1.0/value.f: 0.0;
		  sum.f += value.f;
		} while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		*trgt.f++ = mean? sum.f/n: sum.f;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_FLOAT:
	      do {
		sum.f = 0.0;
		do {
		  temp.f = *src.f; /* data value */
		  value.f = 1.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i])
		      value.f *= temp.f;
		    temp.f *= temp.f;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weight */
		  if (psign == -1) /* negative exponent: must divide */
		    value.f = value.f? 1.0/value.f: 0.0;
		  sum.f += value.f;
		} while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		*trgt.f++ = mean? sum.f/n: sum.f;
	      } while (done < srcinfo.rndim);
	      break;
	  } /* end of switch (type) */
	  break;
	case ANA_DOUBLE:
	  switch (type) {
	    case ANA_BYTE:
	      do {
		sum.d = 0.0;
		do {
		  temp.d = *src.b; /* data value */
		  value.d = 1.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weight */
		  if (psign == -1) /* negative exponent: must divide */
		    value.d = value.d? 1.0/value.d: 0.0;
		  sum.d += value.d;
		} while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		*trgt.d++ = mean? sum.d/n: sum.d;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_WORD:
	      do {
		sum.d = 0.0;
		do {
		  temp.d = *src.w; /* data value */
		  value.d = 1.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weight */
		  if (psign == -1) /* negative exponent: must divide */
		    value.d = value.d? 1.0/value.d: 0.0;
		  sum.d += value.d;
		} while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		*trgt.d++ = mean? sum.d/n: sum.d;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_LONG:
	      do {
		sum.d = 0.0;
		do {
		  temp.d = *src.l; /* data value */
		  value.d = 1.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weight */
		  if (psign == -1) /* negative exponent: must divide */
		    value.d = value.d? 1.0/value.d: 0.0;
		  sum.d += value.d;
		} while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		*trgt.d++ = mean? sum.d/n: sum.d;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_FLOAT:
	      do {
		sum.d = 0.0;
		do {
		  temp.d = *src.f; /* data value */
		  value.d = 1.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weight */
		  if (psign == -1) /* negative exponent: must divide */
		    value.d = value.d? 1.0/value.d: 0.0;
		  sum.d += value.d;
		} while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		*trgt.d++ = mean? sum.d/n: sum.d;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_DOUBLE:
	      do {
		sum.d = 0.0;
		do {
		  temp.d = *src.d; /* data value */
		  value.d = 1.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i])
		      value.d *= temp.d;
		    temp.d *= temp.d;
		  }
		  /* we now have the data value to the given unsigned power */
		  /* add in the exponent sign and the weight */
		  if (psign == -1) /* negative exponent: must divide */
		    value.d = value.d? 1.0/value.d: 0.0;
		  sum.d += value.d;
		} while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		*trgt.d++ = mean? sum.d/n: sum.d;
	      } while (done < srcinfo.rndim);
	      break;
	  } /* end of switch (type) */
	  break;
	case ANA_CFLOAT:
	  do {
	    sumcf.real = sumcf.imaginary = 0.0;
	    do {
	      tempcf.real = src.cf->real;
	      tempcf.imaginary = src.cf->imaginary;
	      valuecf.real = 1.0;
	      valuecf.imaginary = 0.0;
	      for (i = 0; i < nbase; i++) {
		if (present[i]) { /* valuecf *= tempcf */
		  temp2f = valuecf.real*tempcf.real
		    - valuecf.imaginary*tempcf.imaginary;
		  valuecf.imaginary = valuecf.real*tempcf.imaginary
		    + valuecf.imaginary*tempcf.real;
		  valuecf.real = temp2f;
		}
		/* replace tempcf with tempcf*tempcf */
		temp2f = tempcf.real*tempcf.real
		  - tempcf.imaginary*tempcf.imaginary;
		tempcf.imaginary = 2*tempcf.real*tempcf.imaginary;
		tempcf.real = temp2f;
	      }
	      if (psign == -1) { /* replace valuecf by *weights.f/valuecf */
		temp2f = valuecf.real*valuecf.real
		  + valuecf.imaginary*valuecf.imaginary;
		if (temp2f) {
		  temp2f = 1.0/temp2f;
		  valuecf.real *= temp2f;
		  valuecf.imaginary *= -temp2f;
		}
	      }
	      sumcf.real += valuecf.real;
	      sumcf.imaginary *= valuecf.imaginary;
	    } while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
	    if (mean) {
	      trgt.cf->real = sumcf.real/n;
	      trgt.cf->imaginary = sumcf.imaginary/n;
	    } else {
	      trgt.cf->real = sumcf.real;
	      trgt.cf->imaginary = sumcf.imaginary;
	    }
	    trgt.cf++;
	  } while (done < srcinfo.rndim);
	  break;
	case ANA_CDOUBLE:
	  switch (type) {
	    case ANA_CFLOAT:
	      do {
		sumcd.real = sumcd.imaginary = 0.0;
		do {
		  tempcd.real = src.cf->real;
		  tempcd.imaginary = src.cf->imaginary;
		  valuecd.real = 1.0;
		  valuecd.imaginary = 0.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i]) { /* valuecd *= tempcd */
		      temp2d = valuecd.real*tempcd.real
			- valuecd.imaginary*tempcd.imaginary;
		      valuecd.imaginary = valuecd.real*tempcd.imaginary
			+ valuecd.imaginary*tempcd.real;
		      valuecd.real = temp2d;
		    }
		    /* replace tempcd with tempcd*tempcd */
		    temp2d = tempcd.real*tempcd.real
		      - tempcd.imaginary*tempcd.imaginary;
		    tempcd.imaginary = 2*tempcd.real*tempcd.imaginary;
		    tempcd.real = temp2d;
		  }
		  if (psign == -1) { /* replace valuecd by *weights.f/valuecd */
		    temp2d = valuecd.real*valuecd.real
		      + valuecd.imaginary*valuecd.imaginary;
		    if (temp2d) {
		      temp2d = 1.0/temp2d;
		      valuecd.real *= temp2d;
		      valuecd.imaginary *= -temp2d;
		    }
		  }
		  sumcd.real += valuecd.real;
		  sumcd.imaginary *= valuecd.imaginary;
		} while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		if (mean) {
		  trgt.cd->real = sumcd.real/n;
		  trgt.cd->imaginary = sumcd.imaginary/n;
		} else {
		  trgt.cd->real = sumcd.real;
		  trgt.cd->imaginary = sumcd.imaginary;
		}
		trgt.cd++;
	      } while (done < srcinfo.rndim);
	      break;
	    case ANA_CDOUBLE:
	      do {
		sumcd.real = sumcd.imaginary = 0.0;
		do {
		  tempcd.real = src.cd->real;
		  tempcd.imaginary = src.cd->imaginary;
		  valuecd.real = 1.0;
		  valuecd.imaginary = 0.0;
		  for (i = 0; i < nbase; i++) {
		    if (present[i]) { /* valuecd *= tempcd */
		      temp2d = valuecd.real*tempcd.real
			- valuecd.imaginary*tempcd.imaginary;
		      valuecd.imaginary = valuecd.real*tempcd.imaginary
			+ valuecd.imaginary*tempcd.real;
		      valuecd.real = temp2d;
		    }
		    /* replace tempcd with tempcd*tempcd */
		    temp2d = tempcd.real*tempcd.real
		      - tempcd.imaginary*tempcd.imaginary;
		    tempcd.imaginary = 2*tempcd.real*tempcd.imaginary;
		    tempcd.real = temp2d;
		  }
		  if (psign == -1) { /* replace valuecd by *weights.f/valuecd */
		    temp2d = valuecd.real*valuecd.real
		      + valuecd.imaginary*valuecd.imaginary;
		    if (temp2d) {
		      temp2d = 1.0/temp2d;
		      valuecd.real *= temp2d;
		      valuecd.imaginary *= -temp2d;
		    }
		  }
		  sumcd.real += valuecd.real;
		  sumcd.imaginary *= valuecd.imaginary;
		} while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
		if (mean) {
		  trgt.cd->real = sumcd.real/n;
		  trgt.cd->imaginary = sumcd.imaginary/n;
		} else {
		  trgt.cd->real = sumcd.real;
		  trgt.cd->imaginary = sumcd.imaginary;
		}
		trgt.cd++;
	      } while (done < srcinfo.rndim);
	      break;
	  } /* end of switch (type) */
	  break;
      }	/* end of switch (outtype) */
    } /* end of if (haveWeights) else */
  } /* end of if (p == 1) else */
    
#if DEBUG_VOCAL
  debugout("zapping <haveWeights>");
#endif
  zapTemp(haveWeights);
#if DEBUG_VOCAL
  debugout("exiting total()");
#endif
  return result;
}
#undef DEBUG_VOCAL
/*------------------------------------------------------------------------- */
int ana_total(int narg, int ps[])
 {
   return total(narg, ps, 0);
 }
/*------------------------------------------------------------------------- */
int ana_mean(int narg, int ps[])
 {
   return total(narg, ps, 1);
 }
/*------------------------------------------------------------------------- */
doubleComplex c_sin(double real, double imaginary)
/* complex sine */
{
  doubleComplex	result;

  result.real = sin(real)*cosh(imaginary);
  result.imaginary = cos(real)*sinh(imaginary);
  return result;
}
/*------------------------------------------------------------------------- */
doubleComplex c_cos(double real, double imaginary)
/* complex cosine */
{
  doubleComplex	result;

  result.real = cos(real)*cosh(imaginary);
  result.imaginary = -sin(real)*sinh(imaginary);
  return result;
}
/*------------------------------------------------------------------------- */
doubleComplex c_tan(double real, double imaginary)
/* complex tangent */
{
  doubleComplex result;
  double	factor;
  
  factor = 1.0/(cos(2*real) + cosh(2*imaginary));
  result.real = sin(2*real)*factor;
  result.imaginary = sinh(2*imaginary)*factor;
  return result;
}
/*------------------------------------------------------------------------- */
doubleComplex c_arcsin(double real, double imaginary)
/* complex arc sine */
{
  double	a, b, c, d;
  doubleComplex	result;

  imaginary *= imaginary;
  c = real + 1;
  c = sqrt(c*c + imaginary);
  d = real - 1;
  d = sqrt(d*d + imaginary);
  a = 0.5*(c + d);
  b = 0.5*(c - d);
  result.real = asin(b);
  result.imaginary = log(a + sqrt(a*a - 1));
  return result;
}
/*------------------------------------------------------------------------- */
doubleComplex c_arccos(double real, double imaginary)
/* complex arc cosine */
{
  double	a, b, c, d;
  doubleComplex	result;

  imaginary *= imaginary;
  c = real + 1;
  c = sqrt(c*c + imaginary);
  d = real - 1;
  d = sqrt(d*d + imaginary);
  a = 0.5*(c + d);
  b = 0.5*(c - d);
  result.real = acos(b);
  result.imaginary = -log(a + sqrt(a*a - 1));
  return result;
}
/*------------------------------------------------------------------------- */
doubleComplex c_arctan(double real, double imaginary)
/* complex arc tangent */
{
  double	a, b;
  doubleComplex	result;

  result.real = 0.5*atan(2*real/(1 - real*real - imaginary*imaginary));
  real *= real;
  a = imaginary + 1;
  a = real + a*a;
  b = imaginary - 1;
  b = real + b*b;
  result.imaginary = 0.25*log(a/b);
  return result;
}
/*------------------------------------------------------------------------- */
doubleComplex c_sinh(double real, double imaginary)
/* complex hyperbolic sine */
{
  doubleComplex	result;

  result.real = sinh(real)*cos(imaginary);
  result.imaginary = cosh(real)*sin(imaginary);
  return result;
}
/*------------------------------------------------------------------------- */
doubleComplex c_cosh(double real, double imaginary)
/* complex hyperbolic cosine */
{
  doubleComplex	result;

  result.real = cosh(real)*cos(imaginary);
  result.imaginary = sinh(real)*sin(imaginary);
  return result;
}
/*------------------------------------------------------------------------- */
doubleComplex c_tanh(double real, double imaginary)
/* complex hyperbolic tangent */
{
  double	factor;
  doubleComplex	result;

  factor = 1.0/(cosh(2*real) + cos(2*imaginary));
  result.real = sinh(2*real)*factor;
  result.imaginary = sin(2*imaginary)*factor;
  return result;
}
/*------------------------------------------------------------------------- */
doubleComplex c_log(double real, double imaginary)
/* complex logarithm */
{
  doubleComplex	result;

  result.real = 0.5*log(real*real + imaginary*imaginary);
  result.imaginary = atan2(imaginary, real);
  return result;
}
/*------------------------------------------------------------------------- */
doubleComplex c_exp(double real, double imaginary)
/* complex exponential */
{
  double	r;
  doubleComplex	result;

  r = exp(real);
  result.real = r*cos(imaginary);
  result.imaginary = r*sin(imaginary);
  return result;
}
/*------------------------------------------------------------------------- */
/*------------------------------------------------------------------------- */
/*------------------------------------------------------------------------- */
/*------------------------------------------------------------------------- */
/*math functions with 1 argument, all just call math_funcs with approiate
code, all have names of form ana_xxx... */
/*------------------------------------------------------------------------- */
int ana_sin(int narg, int ps[])
{
  return math_funcs(ps[0], F_SIN);
}
/*------------------------------------------------------------------------- */
int ana_cos(int narg, int ps[])
{
  return math_funcs(ps[0], F_COS);
}
/*------------------------------------------------------------------------- */
int ana_tan(int narg, int ps[])
{
  return math_funcs(ps[0], F_TAN);
}
/*------------------------------------------------------------------------- */
int ana_asin(int narg, int ps[])
{
  return math_funcs(ps[0], F_ASIN);
}
/*------------------------------------------------------------------------- */
int ana_acos(int narg, int ps[])
{
  return math_funcs(ps[0], F_ACOS);
}
/*------------------------------------------------------------------------- */
int ana_atan(int narg, int ps[])
{
  return math_funcs(ps[0], F_ATAN);
}
/*------------------------------------------------------------------------- */
int ana_sinh(int narg, int ps[])
{
  return math_funcs(ps[0], F_SINH);
}
/*------------------------------------------------------------------------- */
int ana_cosh(int narg, int ps[])
{
  return math_funcs(ps[0], F_COSH);
}
/*------------------------------------------------------------------------- */
int ana_tanh(int narg, int ps[])
{
  return math_funcs(ps[0], F_TANH);
}
/*------------------------------------------------------------------------- */
int ana_asinh(int narg, int ps[])
{
  return math_funcs(ps[0], F_ASINH);
}
/*------------------------------------------------------------------------- */
int ana_acosh(int narg, int ps[])
{
  return math_funcs(ps[0], F_ACOSH);
}
/*------------------------------------------------------------------------- */
int ana_atanh(int narg, int ps[])
{
  return math_funcs(ps[0], F_ATANH);
}
/*------------------------------------------------------------------------- */
int ana_sqrt(int narg, int ps[])
{
  return math_funcs(ps[0], F_SQRT);
}
/*------------------------------------------------------------------------- */
int ana_cbrt(int narg, int ps[])
{
  return math_funcs(ps[0], F_CBRT);
}
/*------------------------------------------------------------------------- */
int ana_exp(int narg, int ps[])
{
  return math_funcs(ps[0], F_EXP);
}
/*------------------------------------------------------------------------- */
int ana_expm1(int narg, int ps[])
{
  return math_funcs(ps[0], F_EXPM1);
}
/*------------------------------------------------------------------------- */
int ana_log(int narg, int ps[])
{
  return math_funcs(ps[0], F_LOG);
}
/*------------------------------------------------------------------------- */
int ana_log10(int narg, int ps[])
{
  return math_funcs(ps[0], F_LOG10);
}
/*------------------------------------------------------------------------- */
int ana_log1p(int narg, int ps[])
{
  return math_funcs(ps[0], F_LOG1P);
}
/*------------------------------------------------------------------------- */
int ana_erf(int narg, int ps[])
{
  return math_funcs(ps[0], F_ERF);
}
/*------------------------------------------------------------------------- */
int ana_erfc(int narg, int ps[])
{
  return math_funcs(ps[0], F_ERFC);
}
/*------------------------------------------------------------------------- */
int ana_atan2(int narg, int ps[])
/*the right way to do atan's, the atan2 function, 2 arguments */
{
  return math_funcs_2f(ps[0], ps[1], F_ATAN2);
}
/*------------------------------------------------------------------------- */
int ana_j0(int narg, int ps[])
{
  return math_funcs(ps[0], F_J0);
}
/*------------------------------------------------------------------------- */
int ana_j1(int narg, int ps[])
{
  return math_funcs(ps[0], F_J1);
}
/*------------------------------------------------------------------------- */
int ana_jn(int narg, int ps[])
{
  return math_funcs_i_f(ps[0], ps[1], F_JN);
}
/*------------------------------------------------------------------------- */
int ana_y0(int narg, int ps[])
{
  return math_funcs(ps[0], F_Y0);
}
/*------------------------------------------------------------------------- */
int ana_y1(int narg, int ps[])
{
  return math_funcs(ps[0], F_Y1);
}
/*------------------------------------------------------------------------- */
int ana_yn(int narg, int ps[])
{
  return math_funcs_i_f(ps[0], ps[1], F_YN);
}
/*------------------------------------------------------------------------- */
int ana_pow(int narg, int ps[])
{
  return math_funcs_2f(ps[0], ps[1], F_POW);
}
/*------------------------------------------------------------------------- */
int ana_voigt(int narg, int ps[])
/* the voigt function.  LS */
{
  return math_funcs_2f(ps[0], ps[1], F_VOIGT);
}
/*------------------------------------------------------------------------- */
int ana_gamma(int narg, int ps[])
/* returns the gamma function - or the natural logarithm of it (if keyword */
/* /LOG is present)  LS 11jan96 */
{
  return math_funcs(ps[0], (internalMode & 1)? F_LOGGAMMA: F_GAMMA);
}
/*------------------------------------------------------------------------- */
int ana_beta(int narg, int ps[])
/* the beta function: beta(x,y) = gamma(x)*gamma(y)/gamma(x+y). */
/* Switch /COMPLEMENT returns one minus the beta function. */
/* LS 11jan96 22jul96 */
{
  return math_funcs_2f(ps[0], ps[1], (internalMode & 1)? F_LOGBETA: F_BETA);
}
/*------------------------------------------------------------------------- */
int ana_incomplete_gamma(int narg, int ps[])
/* the incomplete gamma function P(a,x).  LS 11jan96 */
{
  return math_funcs_2f(ps[0], ps[1], F_IGAMMA);
}
/*------------------------------------------------------------------------- */
int ana_chi_square(int narg, int ps[])
/* the chi-square function chi2(chi2, nu).  LS 11jan96 19oct96 */
{
  return math_funcs_2f(ps[0], ps[1], F_CHI2);
}
/*------------------------------------------------------------------------- */
int ana_noncentral_chi_square(int narg, int ps[])
/* the noncentral chi-squre function ncchi2(chi2, nu, nc) */
{
  return math_funcs_3f(ps[0], ps[1], ps[2], F_NCCHI2);
}
/*------------------------------------------------------------------------- */
int ana_bessel_i0(int narg, int ps[])
/* the modified bessel function I0 */
{
  return math_funcs(ps[0], F_I0);
}
/*------------------------------------------------------------------------- */
int ana_bessel_i1(int narg, int ps[])
/* the modified bessel function I1 */
{
  return math_funcs(ps[0], F_I1);
}
/*------------------------------------------------------------------------- */
int ana_bessel_k0(int narg, int ps[])
/* the modified bessel function K0 */
{
  return math_funcs(ps[0], F_K0);
}
/*------------------------------------------------------------------------- */
int ana_bessel_k1(int narg, int ps[])
/* the modified bessel function K1 */
{
  return math_funcs(ps[0], F_K1);
}
/*------------------------------------------------------------------------- */
int ana_bessel_kn(int narg, int ps[])
/* the modified bessel function Kn */
{
  return math_funcs_i_f(ps[0], ps[1], F_KN);
}
/*------------------------------------------------------------------------- */
int ana_sgn(int narg, int ps[])
/* the signum function: returns +1 if the argument is positive, -1 if */
/* negative, and 0 if zero.  LS 19may98 */
{
  return math_funcs(ps[0], F_SGN);
}
/*------------------------------------------------------------------------- */
int ana_incomplete_beta(int narg, int ps[])
/* the incomplete beta function I_x(a,b).  LS 15jan96 */
{
  return math_funcs_3f(ps[0], ps[1], ps[2], F_IBETA);
}
/*------------------------------------------------------------------------- */
int ana_student(int narg, int ps[])
/* Student's t-distribution.  LS 15jan96 */
{
  return math_funcs_2f(ps[0], ps[1], F_STUDENT);
}
/*------------------------------------------------------------------------- */
int ana_f_ratio(int narg, int ps[])
/* F variance ratio.  LS 15jan96 */
{
  return math_funcs_3f(ps[0], ps[1], ps[2], F_FRATIO);
}
/*------------------------------------------------------------------------- */
int math_funcs(int nsym, int code)
     /*general program for floating point functions */
{
  int	n, result, type, out_type;
  pointer	trgt, src;
  doubleComplex	value;

  errno = 0;			/* or "old" errors might get reported */
  type = symbol_type(nsym);
  if (isComplexType(type) && !*func_c[code])
    return cerror(ILL_TYPE, nsym, typeName(type));
  /* check that <nsym> is numerical.  Return its number of elements in
     <n>, a pointer to its data in <src>.  Also generate a garbage
     clone in <result> with its data type equal to the greater of
     <nsym>'s and ANA_FLOAT and return its pointer in <trgt>. */
  if (getNumerical(nsym, ANA_FLOAT, &n, &src, GN_UPGRADE, &result, &trgt) < 0)
    return ANA_ERROR;		/* result */
  out_type = symbol_type(result);
  /*addresses and count set up, now do the calculations in a loop determined
    by input and output types, only certain combinations possible */
  switch (out_type) {
    case ANA_FLOAT:
      switch (type) {
	case ANA_BYTE:
	  while (n--)
	    *trgt.f++ = (*func_d[code])(*src.b++);
	  break;
	case ANA_WORD:
	  while (n--)
	    *trgt.f++ = (*func_d[code])(*src.w++);
	  break;
	case ANA_LONG:
	  while (n--)
	    *trgt.f++ = (*func_d[code])(*src.l++);
	  break;
	case ANA_FLOAT:
	  while (n--)
	    *trgt.f++ = (*func_d[code])(*src.f++);
	  break;
      }
      break;
    case ANA_DOUBLE:
      switch (type) {
	case ANA_BYTE:
	  while (n--)
	    *trgt.d++ = (*func_d[code])(*src.b++);
	  break;
	case ANA_WORD:
	  while (n--)
	    *trgt.d++ = (*func_d[code])(*src.w++);
	  break;
	case ANA_LONG:
	  while (n--)
	    *trgt.d++ = (*func_d[code])(*src.l++);
	  break;
	case ANA_FLOAT:
	  while (n--)
	    *trgt.d++ = (*func_d[code])(*src.f++);
	  break;
	case ANA_DOUBLE:
	  while (n--)
	    *trgt.d++ = (*func_d[code])(*src.d++);
	  break;
      }
      break;
    case ANA_CFLOAT:
      /* input type must be CFLOAT, too */
      while (n--) {
	value = (*func_c[code])(src.cf->real, src.cf->imaginary);
	trgt.cf->real = value.real;
	trgt.cf++->imaginary = value.imaginary;
	src.cf++;
      }
      break;
    case ANA_CDOUBLE:
      /* input type is CDOUBLE, too */
      while (n--) {
	value = (*func_c[code])(src.cd->real, src.cd->imaginary);
	trgt.cd->real = value.real;
	trgt.cd++->imaginary = value.imaginary;
	src.cd++;
      }
      break;
  }
  return result;
}						/*end of math_funcs */
/*------------------------------------------------------------------------- */
int math_funcs_2f(int nsym1, int nsym2, int code)
/*general program for floating point functions with 2 floating arguments */
/*messier than the 1 argument case but not as bad as binary ops routines */
/*assumes that the function requires double arguments (most C functions) */
{
  int	n1, n2, nelem, i, result_sym, type1, type2, out_type;
  pointer	src1, src2, trgt;
  double	value;

  errno = 0;

  /* both arguments must be numerical: scalars, scalar pointers, or
     numerical arrays */
  if (numerical(nsym1, NULL, NULL, &n1, &src1) == ANA_ERROR)
    return ANA_ERROR;
  if (numerical(nsym2, NULL, NULL, &n2, &src2) == ANA_ERROR)
    return ANA_ERROR;

  /* the two arguments must either have the same number of arguments,
   or one or both of them must have a single argument, which is then
   combined with all elements of the other argument */
  if (n1 != n2 && n1 > 1 && n2 > 1)
    return cerror(INCMP_ARG, nsym2);

  type1 = symbol_type(nsym1);
  type2 = symbol_type(nsym2);

  /* we take the greatest data type for the result, or FLOAT, whichever
   is greater */
  if (type1 == ANA_DOUBLE || type2 == ANA_DOUBLE)
    out_type = ANA_DOUBLE;
  else
    out_type = ANA_FLOAT;

  if (n2 > n1 || symbol_class(nsym1) != ANA_ARRAY) {
    nelem = n2;
    i = nsym2;
  } else {
    nelem = n1;
    i = nsym1;
  }

  /* figure out what to take for result symbol: if either argument is
   an array, then so is the result.  If both are non-arrays, then the
   result is a scalar.  If both are arrays of equal size, then we copy
   the dimensional structure of the first argument into the result;
   If both are arrays but of unequal size (i.e., one has size 1),
   then we copy the structure of the argument with the most elements */
  if (n1 > 1
      || n2 > 1
      || symbol_class(nsym1) == ANA_ARRAY
      || symbol_class(nsym2) == ANA_ARRAY) { /* need an array for output */
    result_sym = 0;
    if (isFreeTemp(nsym1)
	&& symbol_class(nsym1) == ANA_ARRAY
	&& n1 == nelem
	&& symbol_type(nsym1) == out_type)
      result_sym = nsym1;
    else if (isFreeTemp(nsym2)
	     && symbol_class(nsym2) == ANA_ARRAY
	     && n2 == nelem
	     && symbol_type(nsym2) == out_type) {
      if (symbol_class(nsym1) != ANA_ARRAY)
	result_sym = nsym2;
      else {
	for (i = 0; i < array_num_dims(nsym2); i++)
	  if (array_dims(nsym2)[i] != array_dims(nsym1)[i])
	    break;
	if (i == array_num_dims(nsym2) + 1)
	  result_sym = nsym2;
      }
    }
    if (!result_sym) {
      if (n2 > n1)
	result_sym = array_clone(nsym2, out_type);
      else
	result_sym = array_clone(nsym1, out_type);
    }
    trgt.l = array_data(result_sym);
  } else {			/* a scalar will do */
    result_sym = scalar_scratch(out_type);
    trgt.l = &scalar_value(result_sym).l;
  }

  if (n1 == n2) {		/* advance both argument pointers */
    switch (type1) {
      case ANA_BYTE:
	switch (type2) {
	  case ANA_BYTE:
	    while (nelem--)
	      *trgt.f++ = (*func_dd[code])(*src1.b++, *src2.b++);
	    break;
	  case ANA_WORD:
	    while (nelem--)
	      *trgt.f++ = (*func_dd[code])(*src1.b++, *src2.w++);
	    break;
	  case ANA_LONG:
	    while (nelem--)
	      *trgt.f++ = (*func_dd[code])(*src1.b++, *src2.l++);
	    break;
	  case ANA_FLOAT:
	    while (nelem--)
	      *trgt.f++ = (*func_dd[code])(*src1.b++, *src2.f++);
	    break;
	  case ANA_DOUBLE:
	    while (nelem--)
	      *trgt.d++ = (*func_dd[code])(*src1.b++, *src2.d++);
	    break;
	}
	break;
      case ANA_WORD:
	switch (type2) {
	  case ANA_BYTE:
	    while (nelem--)
	      *trgt.f++ = (*func_dd[code])(*src1.w++, *src2.b++);
	    break;
	  case ANA_WORD:
	    while (nelem--)
	      *trgt.f++ = (*func_dd[code])(*src1.w++, *src2.w++);
	    break;
	  case ANA_LONG:
	    while (nelem--)
	      *trgt.f++ = (*func_dd[code])(*src1.w++, *src2.l++);
	    break;
	  case ANA_FLOAT:
	    while (nelem--)
	      *trgt.f++ = (*func_dd[code])(*src1.w++, *src2.f++);
	    break;
	  case ANA_DOUBLE:
	    while (nelem--)
	      *trgt.d++ = (*func_dd[code])(*src1.w++, *src2.d++);
	    break;
	}
	break;
      case ANA_LONG:
	switch (type2) {
	  case ANA_BYTE:
	    while (nelem--)
	      *trgt.f++ = (*func_dd[code])(*src1.l++, *src2.b++);
	    break;
	  case ANA_WORD:
	    while (nelem--)
	      *trgt.f++ = (*func_dd[code])(*src1.l++, *src2.w++);
	    break;
	  case ANA_LONG:
	    while (nelem--)
	      *trgt.f++ = (*func_dd[code])(*src1.l++, *src2.l++);
	    break;
	  case ANA_FLOAT:
	    while (nelem--)
	      *trgt.f++ = (*func_dd[code])(*src1.l++, *src2.f++);
	    break;
	  case ANA_DOUBLE:
	    while (nelem--)
	      *trgt.d++ = (*func_dd[code])(*src1.l++, *src2.d++);
	    break;
	}
	break;
      case ANA_FLOAT:
	switch (type2) {
	  case ANA_BYTE:
	    while (nelem--)
	      *trgt.f++ = (*func_dd[code])(*src1.f++, *src2.b++);
	    break;
	  case ANA_WORD:
	    while (nelem--)
	      *trgt.f++ = (*func_dd[code])(*src1.f++, *src2.w++);
	    break;
	  case ANA_LONG:
	    while (nelem--)
	      *trgt.f++ = (*func_dd[code])(*src1.f++, *src2.l++);
	    break;
	  case ANA_FLOAT:
	    while (nelem--)
	      *trgt.f++ = (*func_dd[code])(*src1.f++, *src2.f++);
	    break;
	  case ANA_DOUBLE:
	    while (nelem--)
	      *trgt.d++ = (*func_dd[code])(*src1.f++, *src2.d++);
	    break;
	}
	break;
      case ANA_DOUBLE:
	switch (type2) {
	  case ANA_BYTE:
	    while (nelem--)
	      *trgt.d++ = (*func_dd[code])(*src1.d++, *src2.b++);
	    break;
	  case ANA_WORD:
	    while (nelem--)
	      *trgt.d++ = (*func_dd[code])(*src1.d++, *src2.w++);
	    break;
	  case ANA_LONG:
	    while (nelem--)
	      *trgt.d++ = (*func_dd[code])(*src1.d++, *src2.l++);
	    break;
	  case ANA_FLOAT:
	    while (nelem--)
	      *trgt.d++ = (*func_dd[code])(*src1.d++, *src2.f++);
	    break;
	  case ANA_DOUBLE:
	    while (nelem--)
	      *trgt.d++ = (*func_dd[code])(*src1.d++, *src2.d++);
	    break;
	}
	break;
    }
  } else if (n1 > n2) {		/* nsym1 pointer is advanced, but not nsym2 */
    switch (type2) {
      case ANA_BYTE:
	value = *src2.b;
	break;
      case ANA_WORD:
	value = *src2.w;
	break;
      case ANA_LONG:
	value = *src2.l;
	break;
      case ANA_FLOAT:
	value = *src2.f;
	break;
      case ANA_DOUBLE:
	value = *src2.d;
	break;
    }
    if (type2 < ANA_DOUBLE)
      switch (type1) {
	case ANA_BYTE:
	  while (nelem--)
	    *trgt.f++ = (*func_dd[code])(*src1.b++, value);
	  break;
	case ANA_WORD:
	  while (nelem--)
	    *trgt.f++ = (*func_dd[code])(*src1.w++, value);
	  break;
	case ANA_LONG:
	  while (nelem--)
	    *trgt.f++ = (*func_dd[code])(*src1.l++, value);
	  break;
	case ANA_FLOAT:
	  while (nelem--)
	    *trgt.f++ = (*func_dd[code])(*src1.f++, value);
	  break;
	case ANA_DOUBLE:
	  while (nelem--)
	    *trgt.d++ = (*func_dd[code])(*src1.d++, value);
	  break;
      }
    else
      switch (type1) {
	case ANA_BYTE:
	  while (nelem--)
	    *trgt.d++ = (*func_dd[code])(*src1.b++, value);
	  break;
	case ANA_WORD:
	  while (nelem--)
	    *trgt.d++ = (*func_dd[code])(*src1.w++, value);
	  break;
	case ANA_LONG:
	  while (nelem--)
	    *trgt.d++ = (*func_dd[code])(*src1.l++, value);
	  break;
	case ANA_FLOAT:
	  while (nelem--)
	    *trgt.d++ = (*func_dd[code])(*src1.f++, value);
	  break;
	case ANA_DOUBLE:
	  while (nelem--)
	    *trgt.d++ = (*func_dd[code])(*src1.d++, value);
	  break;
      }
  } else {			/* nsym2 pointer is advanced, but not nsym1 */
    switch (type1) {
      case ANA_BYTE:
	value = *src1.b;
	break;
      case ANA_WORD:
	value = *src1.w;
	break;
      case ANA_LONG:
	value = *src1.l;
	break;
      case ANA_FLOAT:
	value = *src1.f;
	break;
      case ANA_DOUBLE:
	value = *src1.d;
	break;
    }
    if (type1 < ANA_DOUBLE)
      switch (type2) {
	case ANA_BYTE:
	  while (nelem--)
	    *trgt.f++ = (*func_dd[code])(value, *src2.b++);
	  break;
	case ANA_WORD:
	  while (nelem--)
	    *trgt.f++ = (*func_dd[code])(value, *src2.w++);
	  break;
	case ANA_LONG:
	  while (nelem--)
	    *trgt.f++ = (*func_dd[code])(value, *src2.l++);
	  break;
	case ANA_FLOAT:
	  while (nelem--)
	    *trgt.f++ = (*func_dd[code])(value, *src2.f++);
	  break;
	case ANA_DOUBLE:
	  while (nelem--)
	    *trgt.d++ = (*func_dd[code])(value, *src2.d++);
	  break;
      }
    else
      switch (type2) {
	case ANA_BYTE:
	  while (nelem--)
	    *trgt.d++ = (*func_dd[code])(value, *src2.b++);
	  break;
	case ANA_WORD:
	  while (nelem--)
	    *trgt.d++ = (*func_dd[code])(value, *src2.w++);
	  break;
	case ANA_LONG:
	  while (nelem--)
	    *trgt.d++ = (*func_dd[code])(value, *src2.l++);
	  break;
	case ANA_FLOAT:
	  while (nelem--)
	    *trgt.d++ = (*func_dd[code])(value, *src2.f++);
	  break;
	case ANA_DOUBLE:
	  while (nelem--)
	    *trgt.d++ = (*func_dd[code])(value, *src2.d++);
	  break;
      }
  }
  return result_sym;
}
/*------------------------------------------------------------------------- */
int math_funcs_i_f(int nsym1, int nsym2, int code)
/*general program for floating point functions with int and float arguments */
/*assumes that the function requires double arguments (most C functions) */
{
  int	n1, n2, nelem, i, result_sym, type1, type2, out_type, valuei;
  pointer	src1, src2, trgt;
  double	valued;

  errno = 0;

  /* both arguments must be numerical: scalars, scalar pointers, or
     numerical arrays */
  if (numerical(nsym1, NULL, NULL, &n1, &src1) == ANA_ERROR)
    return ANA_ERROR;
  if (numerical(nsym1, NULL, NULL, &n2, &src2) == ANA_ERROR)
    return ANA_ERROR;

  /* the two arguments must either have the same number of arguments,
   or one or both of them must have a single argument, which is then
   combined with all elements of the other argument */
  if (n1 != n2 && n1 > 1 && n2 > 1)
    return cerror(INCMP_ARG, nsym2);

  type1 = symbol_type(nsym1);
  type2 = symbol_type(nsym2);

  /* the output symbol is DOUBLE if <nsym2> is DOUBLE, and FLOAT otherwise */
  if (type2 == ANA_DOUBLE)
    out_type = ANA_DOUBLE;
  else
    out_type = ANA_FLOAT;

  if (n2 > n1 || symbol_class(nsym1) != ANA_ARRAY) {
    nelem = n2;
    i = nsym2;
  } else {
    nelem = n1;
    i = nsym1;
  }

  /* figure out what to take for result symbol: if either argument is
   an array, then so is the result.  If both are non-arrays, then the
   result is a scalar.  If both are arrays of equal size, then we copy
   the dimensional structure of the first argument into the result;
   If both are arrays but of unequal size (i.e., one has size 1),
   then we copy the structure of the argument with the most elements */
  if (n1 > 1
      || n2 > 1
      || symbol_class(nsym1) == ANA_ARRAY
      || symbol_class(nsym2) == ANA_ARRAY) { /* need an array for output */
    result_sym = 0;
    if (isFreeTemp(nsym1)
	&& symbol_class(nsym1) == ANA_ARRAY
	&& n1 == nelem
	&& symbol_type(nsym1) == out_type)
      result_sym = nsym1;
    else if (isFreeTemp(nsym2)
	     && symbol_class(nsym2) == ANA_ARRAY
	     && n2 == nelem
             && symbol_type(nsym2) == out_type) {
      if (symbol_class(nsym1) != ANA_ARRAY)
	result_sym = nsym2;
      else {
	for (i = 0; i < array_num_dims(nsym2); i++)
	  if (array_dims(nsym2)[i] != array_dims(nsym1)[i])
	    break;
	if (i == array_num_dims(nsym2) + 1)
	  result_sym = nsym2;
      }
    }
    if (!result_sym)
      result_sym = array_clone(i, out_type);
    trgt.l = array_data(result_sym);
  } else {			/* a scalar will do */
    result_sym = scalar_scratch(out_type);
    trgt.l = &scalar_value(result_sym).l;
  }

  if (n1 == n2) {		/* advance both argument pointers */
    switch (type1) {
      case ANA_BYTE:
	switch (type2) {
	  case ANA_BYTE:
	    while (nelem--)
	      *trgt.f++ = (*func_id[code])(*src1.b++, *src2.b++);
	    break;
	  case ANA_WORD:
	    while (nelem--)
	      *trgt.f++ = (*func_id[code])(*src1.b++, *src2.w++);
	    break;
	  case ANA_LONG:
	    while (nelem--)
	      *trgt.f++ = (*func_id[code])(*src1.b++, *src2.l++);
	    break;
	  case ANA_FLOAT:
	    while (nelem--)
	      *trgt.f++ = (*func_id[code])(*src1.b++, *src2.f++);
	    break;
	  case ANA_DOUBLE:
	    while (nelem--)
	      *trgt.d++ = (*func_id[code])(*src1.b++, *src2.d++);
	    break;
	}
	break;
      case ANA_WORD:
	switch (type2) {
	  case ANA_BYTE:
	    while (nelem--)
	      *trgt.f++ = (*func_id[code])(*src1.w++, *src2.b++);
	    break;
	  case ANA_WORD:
	    while (nelem--)
	      *trgt.f++ = (*func_id[code])(*src1.w++, *src2.w++);
	    break;
	  case ANA_LONG:
	    while (nelem--)
	      *trgt.f++ = (*func_id[code])(*src1.w++, *src2.l++);
	    break;
	  case ANA_FLOAT:
	    while (nelem--)
	      *trgt.f++ = (*func_id[code])(*src1.w++, *src2.f++);
	    break;
	  case ANA_DOUBLE:
	    while (nelem--)
	      *trgt.d++ = (*func_id[code])(*src1.w++, *src2.d++);
	    break;
	}
	break;
      case ANA_LONG:
	switch (type2) {
	  case ANA_BYTE:
	    while (nelem--)
	      *trgt.f++ = (*func_id[code])(*src1.l++, *src2.b++);
	    break;
	  case ANA_WORD:
	    while (nelem--)
	      *trgt.f++ = (*func_id[code])(*src1.l++, *src2.w++);
	    break;
	  case ANA_LONG:
	    while (nelem--)
	      *trgt.f++ = (*func_id[code])(*src1.l++, *src2.l++);
	    break;
	  case ANA_FLOAT:
	    while (nelem--)
	      *trgt.f++ = (*func_id[code])(*src1.l++, *src2.f++);
	    break;
	  case ANA_DOUBLE:
	    while (nelem--)
	      *trgt.d++ = (*func_id[code])(*src1.l++, *src2.d++);
	    break;
	}
	break;
      case ANA_FLOAT:
	switch (type2) {
	  case ANA_BYTE:
	    while (nelem--)
	      *trgt.f++ = (*func_id[code])(*src1.f++, *src2.b++);
	    break;
	  case ANA_WORD:
	    while (nelem--)
	      *trgt.f++ = (*func_id[code])(*src1.f++, *src2.w++);
	    break;
	  case ANA_LONG:
	    while (nelem--)
	      *trgt.f++ = (*func_id[code])(*src1.f++, *src2.l++);
	    break;
	  case ANA_FLOAT:
	    while (nelem--)
	      *trgt.f++ = (*func_id[code])(*src1.f++, *src2.f++);
	    break;
	  case ANA_DOUBLE:
	    while (nelem--)
	      *trgt.d++ = (*func_id[code])(*src1.f++, *src2.d++);
	    break;
	}
	break;
      case ANA_DOUBLE:
	switch (type2) {
	  case ANA_BYTE:
	    while (nelem--)
	      *trgt.f++ = (*func_id[code])(*src1.d++, *src2.b++);
	    break;
	  case ANA_WORD:
	    while (nelem--)
	      *trgt.f++ = (*func_id[code])(*src1.d++, *src2.w++);
	    break;
	  case ANA_LONG:
	    while (nelem--)
	      *trgt.f++ = (*func_id[code])(*src1.d++, *src2.l++);
	    break;
	  case ANA_FLOAT:
	    while (nelem--)
	      *trgt.f++ = (*func_id[code])(*src1.d++, *src2.f++);
	    break;
	  case ANA_DOUBLE:
	    while (nelem--)
	      *trgt.d++ = (*func_id[code])(*src1.d++, *src2.d++);
	    break;
	}
	break;
    }
  } else if (n1 > n2) {		/* nsym1 pointer is advanced, but not nsym2 */
    switch (type2) {
      case ANA_BYTE:
	valuei = *src2.b;
	break;
      case ANA_WORD:
	valuei = *src2.w;
	break;
      case ANA_LONG:
	valuei = *src2.l;
	break;
      case ANA_FLOAT:
	valuei = *src2.f;
	break;
      case ANA_DOUBLE:
	valuei = *src2.d;
	break;
    }
    if (type2 == ANA_FLOAT)
      switch (type1) {
	case ANA_BYTE:
	  while (nelem--)
	    *trgt.f++ = (*func_id[code])(*src1.b++, valuei);
	  break;
	case ANA_WORD:
	  while (nelem--)
	    *trgt.f++ = (*func_id[code])(*src1.w++, valuei);
	  break;
	case ANA_LONG:
	  while (nelem--)
	    *trgt.f++ = (*func_id[code])(*src1.l++, valuei);
	  break;
	case ANA_FLOAT:
	  while (nelem--)
	    *trgt.f++ = (*func_id[code])(*src1.f++, valuei);
	  break;
	case ANA_DOUBLE:
	  while (nelem--)
	    *trgt.f++ = (*func_id[code])(*src1.d++, valuei);
	  break;
      }
    else
      switch (type1) {
	case ANA_BYTE:
	  while (nelem--)
	    *trgt.d++ = (*func_id[code])(*src1.b++, valuei);
	  break;
	case ANA_WORD:
	  while (nelem--)
	    *trgt.d++ = (*func_id[code])(*src1.w++, valuei);
	  break;
	case ANA_LONG:
	  while (nelem--)
	    *trgt.d++ = (*func_id[code])(*src1.l++, valuei);
	  break;
	case ANA_FLOAT:
	  while (nelem--)
	    *trgt.d++ = (*func_id[code])(*src1.f++, valuei);
	  break;
	case ANA_DOUBLE:
	  while (nelem--)
	    *trgt.d++ = (*func_id[code])(*src1.d++, valuei);
	  break;
      }
  } else {			/* nsym2 pointer is advanced, but not nsym1 */
    switch (type1) {
      case ANA_BYTE:
	valued = *src1.b;
	break;
      case ANA_WORD:
	valued = *src1.w;
	break;
      case ANA_LONG:
	valued = *src1.l;
	break;
      case ANA_FLOAT:
	valued = *src1.f;
	break;
      case ANA_DOUBLE:
	valued = *src1.d;
	break;
    }
    if (type1 == ANA_FLOAT)
      switch (type2) {
	case ANA_BYTE:
	  while (nelem--)
	    *trgt.f++ = (*func_id[code])(valued, *src2.b++);
	  break;
	case ANA_WORD:
	  while (nelem--)
	    *trgt.f++ = (*func_id[code])(valued, *src2.w++);
	  break;
	case ANA_LONG:
	  while (nelem--)
	    *trgt.f++ = (*func_id[code])(valued, *src2.l++);
	  break;
	case ANA_FLOAT:
	  while (nelem--)
	    *trgt.f++ = (*func_id[code])(valued, *src2.f++);
	  break;
	case ANA_DOUBLE:
	  while (nelem--)
	    *trgt.f++ = (*func_id[code])(valued, *src2.d++);
	  break;
      }
    else
      switch (type2) {
	case ANA_BYTE:
	  while (nelem--)
	    *trgt.d++ = (*func_id[code])(valued, *src2.b++);
	  break;
	case ANA_WORD:
	  while (nelem--)
	    *trgt.d++ = (*func_id[code])(valued, *src2.w++);
	  break;
	case ANA_LONG:
	  while (nelem--)
	    *trgt.d++ = (*func_id[code])(valued, *src2.l++);
	  break;
	case ANA_FLOAT:
	  while (nelem--)
	    *trgt.d++ = (*func_id[code])(valued, *src2.f++);
	  break;
	case ANA_DOUBLE:
	  while (nelem--)
	    *trgt.d++ = (*func_id[code])(valued, *src2.d++);
	  break;
      }
  }
  return result_sym;
}						/*end of math_funcs_i_f */
/*------------------------------------------------------------------------- */
int math_funcs_3f(int sym1, int sym2, int sym3, int code)
/* mathematical function with three arguments.  This code goes for */
/* conciseness, not for speed.  It is assumed that the calculation */
/* of the actual mathematical function takes more time than */
/* looping through the input does.  At the moment, each of the three */
/* arguments may be a ANA_SCALAR or an ANA_ARRAY.  Any arrays must all have */
/* the same number of elements.  The dimensional structures are not */
/* checked.  LS 15jan96 */
{
  int	n1, n2, n3, iq, n, type1, type2, type3, step1, step2, step3, type;
  pointer	src1, src2, src3, trgt;
  double	val1, val2, val3, val;

  errno = 0;
  /* get sizes and pointers */
  if (getNumerical(sym1, 0, &n1, &src1, 0, NULL, NULL) < 0
      || getNumerical(sym2, 0, &n2, &src2, 0, NULL, NULL) < 0
      || getNumerical(sym3, 0, &n3, &src3, 0, NULL, NULL) < 0) 
    return ANA_ERROR;
				/* crude check on sizes */
  if ((n1 != n2 && n1 > 1 && n2 > 1)
      || (n1 != n3 && n1 > 1 && n3 > 1)
      || (n2 != n3 && n2 > 1 && n3 > 1))
    return cerror(INCMP_DIMS, sym2);

  type1 = symbol_type(sym1);	/* get data types */
  type2 = symbol_type(sym2);
  type3 = symbol_type(sym3);
  type = (type1 == ANA_DOUBLE	/* get output type (at least ANA_FLOAT) */
	  || type2 == ANA_DOUBLE
	  || type3 == ANA_DOUBLE)? ANA_DOUBLE: ANA_FLOAT;
  if (n1 > 1) {			/* get output symbol */
    iq = array_clone(sym1, type);
    n = n1;
  } else if (n2 > 1) {
    iq = array_clone(sym2, type);
    n = n2;
  } else if (n3 > 1) {
    iq = array_clone(sym3, type);
    n = n3;
  }
  else {			/* none has more than 1 element
				   -> ANA_SCALAR */
    iq = scalar_scratch(type);
    n = 1;
  }
  if (symbol_class(iq) == ANA_SCALAR) /* get pointer to output data */
    trgt.b =  &scalar_value(iq).b;
  else
    trgt.b = (byte *) array_data(iq);
  step1 = (n1 == 1)? 0: ana_type_size[type1]; /* get step sizes */
  step2 = (n2 == 1)? 0: ana_type_size[type2];
  step3 = (n3 == 1)? 0: ana_type_size[type3];
  /* if any of the arguments has only one element, then evaluate that */
  /* element once. */
  if (!step1)
    switch (type1) {
      case ANA_BYTE:
	val1 = (double) *src1.b;
	break;
      case ANA_WORD:
	val1 = (double) *src1.w;
	break;
      case ANA_LONG:
	val1 = (double) *src1.l;
	break;
      case ANA_FLOAT:
	val1 = (double) *src1.f;
	break;
      case ANA_DOUBLE:
	val1 = (double) *src1.d;
	break;
    }
  if (!step2)
    switch (type2) {
      case ANA_BYTE:
	val2 = (double) *src2.b;
	break;
      case ANA_WORD:
	val2 = (double) *src2.w;
	break;
      case ANA_LONG:
	val2 = (double) *src2.l;
	break;
      case ANA_FLOAT:
	val2 = (double) *src2.f;
	break;
      case ANA_DOUBLE:
	val2 = (double) *src2.d;
	break;
    }
  if (!step3)
    switch (type3) {
      case ANA_BYTE:
	val3 = (double) *src3.b;
	break;
      case ANA_WORD:
	val3 = (double) *src3.w;
	break;
      case ANA_LONG:
	val3 = (double) *src3.l;
	break;
      case ANA_FLOAT:
	val3 = (double) *src3.f;
	break;
      case ANA_DOUBLE:
	val3 = (double) *src3.d;
	break;
    }
  while (n--) {			/* loop over all elements */
    if (step1) {		/* new element */
      switch (type1) {
	case ANA_BYTE:
	  val1 = (double) *src1.b;
	  break;
	case ANA_WORD:
	  val1 = (double) *src1.w;
	  break;
	case ANA_LONG:
	  val1 = (double) *src1.l;
	  break;
	case ANA_FLOAT:
	  val1 = (double) *src1.f;
	  break;
	case ANA_DOUBLE:
	  val1 = (double) *src1.d;
	  break; }
      src1.b += step1;
    }
    if (step2) {
      switch (type2) {
	case ANA_BYTE:
	  val2 = (double) *src2.b;
	  break;
	case ANA_WORD:
	  val2 = (double) *src2.w;
	  break;
	case ANA_LONG:
	  val2 = (double) *src2.l;
	  break;
	case ANA_FLOAT:
	  val2 = (double) *src2.f;
	  break;
	case ANA_DOUBLE:
	  val2 = (double) *src2.d;
	  break;
      }
      src2.b += step2;
    }
    if (step3) {
      switch (type3) {
	case ANA_BYTE:
	  val3 = (double) *src3.b;
	  break;
	case ANA_WORD:
	  val3 = (double) *src3.w;
	  break;
	case ANA_LONG:
	  val3 = (double) *src3.l;
	  break;
	case ANA_FLOAT:
	  val3 = (double) *src3.f;
	  break;
	case ANA_DOUBLE:
	  val3 = (double) *src3.d;
	  break;
      }
      src3.b += step3;
    }
    val = (*func_ddd[code])(val1, val2, val3); /* get function value */
    if (type == ANA_FLOAT)		/* store result */
      *trgt.f++ = (float) val;
    else
      *trgt.d++ = val;
  }
  return iq;
}
/*------------------------------------------------------------------------- */
double voigt(double a, double v)
{
    double	anhs, avsd, sumb, avss;
    int		n;
    double	c1, d1, p2, ab, ef, al, aa,
		av, aph, avs, vph, sum = 0;

    if (a < 0) a = -a;
    if (a <= 1e-8) return exp(-v*v);
    av = a * v;
    avs = av * av;
    avsd = a * a - v * v;
    avss = a * a + v * v;
    vph = v * 4*M_PI;
    aph = a * 4*M_PI;
    c1 = exp(-aph) - cos(vph);
    d1 = sin(vph);
    ab = cos(2*av)*c1 - sin(2*av)*d1;
    if (ab) al = ab/(c1*c1 + d1*d1); else al = 0;
    if (a <= 2*M_PI)
    { ef = aph - avsd;
      if (ef > 100.) ef = 100;
      p2 = 2*exp(-ef)*al;
      if (a == 2*M_PI) p2 /= 2; }
    else p2 = 0;
    anhs = 0;
    for (n = 1; n < 100; n++)
    { anhs += (2*n - 1)/4.0;
      aa = avsd + anhs;
      sumb = exp(-anhs)*(avss + anhs)/(aa*aa + 4*avs);
      sum += sumb;
      if (sumb < 1e-6) break; }
    return a/(2*M_PI*avss) + a/M_PI*sum + p2;
}
/*------------------------------------------------------------------------- */
double loggamma(double x)
/* returns the natural logarithm of the absolute value of the gamma */
/* function at ordinate <x>. */
/* Uses the approximation of Lanczos.   LS 11jan96 22jul96 */
{
  static double	a[6] =
  { 76.18009172947146, -86.50532032941677, 24.01409824083091,
      -1.231739572450155, 1.208650973866179e-3, -5.395239384953e-6 };
  char	flip;
  int	i;
  double	y, z, w;

  if (x <= 0)
  { x = 1 - x;
    flip = 1; }
  else
    flip = 0;
  z = 1.000000000190015;
  w = x;
  for (i = 0; i < 6; i++)
    z += a[i]/w++;
  y = x + 4.5;
  y = (x - 0.5)*log(y) - y + log(z) + 0.918938533204672742;
  if (flip)
    y = 1.14472988584940017 - y - log(fabs(sin(x*M_PI)));
  return y;
}
/*------------------------------------------------------------------------- */
double gamma(double x)
/* returns the value of the gamma function at ordinate <x>.  LS 22jul96 */
{
  static double	a[6] =
  { 76.18009172947146, -86.50532032941677, 24.01409824083091,
      -1.231739572450155, 1.208650973866179e-3, -5.395239384953e-6 };
  char	flip;
  int	i;
  double	y, z, w;

  if (x <= 0)
  { x = 1 - x;
    flip = 1; }
  else
    flip = 0;
  z = 1.000000000190015;
  w = x;
  for (i = 0; i < 6; i++)
    z += a[i]/w++;
  y = x + 4.5;
  y = (x - 0.5)*log(y) - y + log(z) + 0.918938533204672742;
  if (flip)
    y = 1.14472988584940017 - y - log(fabs(sin(x*M_PI)));
  y = exp(y);
  if (flip && ((int) x) % 2 == 1)
    y = -y;
  return y;
}
/*------------------------------------------------------------------------- */
double incomplete_gamma(double a, double x)
/* returns the incomplete gamma function P(a,x) */
/* series expansion for x < a + 1, and continued fraction for x > a + 1 */
/* if internalMode & 1, then 1 - P(a,x) is returned. */
/* LS 11jan96 22jul96 */
{
  double	z, g, z0, c, d, tiny, aa, bb, del;
  int	i;

  if (a < 0.0)
    return sqrt(-1);
  if (!x)
    return (internalMode & 1)? 1.0: 0.0;
  c = a;
  if (x < a + 1)		/* series */
  { z = z0 = g = 1./a;
    do
    { z0 = z;
      g *= x/++c;
      z += g; }
    while (z0 != z);
    z = -x + a*log(x) + log(z) - loggamma(a);
    switch (internalMode & 3) {
    case 0:			/* regular */
      return exp(z);
    case 1:			/* /COMPLEMENT */
      return -expm1(z);
    case 2:			/* /LOG */
      return z;
    case 3:			/* /LOG,/COMPLEMENT */
      return log(-expm1(z));
    }
  } else {				/* continued fraction */
    tiny = DBL_EPSILON*DBL_EPSILON;
    c = 1.0/tiny;
    z = d = 1.0/(x + 1 - a);
    aa = a - 1;
    bb = x + 3 - a;
    i = 1;
    while (1)
    { d = bb + i*aa*d;
      d = 1.0/(d? d: tiny);
      c = bb + (i++)*(aa--)/c;
      if (!c)
	c = tiny;
      del = c*d;
      z *= del;
      if (fabs(del - 1) < 3E-7)
	break;
      bb += 2; }
    z = -x + a*log(x) + log(z) - loggamma(a); /* log(1 - igamma) */
    switch (internalMode & 3) {
    case 0:			/* regular */
      return -expm1(z);
    case 1:			/* /COMPLEMENT */
      return exp(z);
    case 2:			/* /LOG */
      return log(-expm1(z));
    case 3:			/* /LOG,/COMPLEMENT */
      return z;
    }
  }
  return 1.0;			/* or some compilers complain */
}
/*------------------------------------------------------------------------- */
double beta(double x, double y)
/* returns the beta function. */
/* LS 15jan96 22jul96 */
{
  return exp(loggamma(x) + loggamma(y) - loggamma(x + y));
}
/*------------------------------------------------------------------------- */
double logbeta(double x, double y)
/* returns the logarithm of the beta function. */
/* LS 15jan96 22jul96 */
{
  return loggamma(x) + loggamma(y) - loggamma(x + y);
}
/*------------------------------------------------------------------------- */
double chi_square(double chi2, double nu)
/* returns the chi-square function or, if internalMode & 1, its */
/* complement.  LS 15jan95 22jul96 */
{
  return incomplete_gamma(nu/2, chi2/2);
}
/*------------------------------------------------------------------------- */
double incomplete_beta(double x, double a, double b)
/* returns the incomplete beta function or (if internalMode & 1) its */
/* complement or (if internalMode & 2) its natural logarithm. */
/* LS 15jan96 5nov96 */
{
  double	x0, tiny, c, f, d, e1, e2, e3, e4, e5, aa, del, k, g;
  char	flip;
  int	j;

  if (a <= 0 || b <= 0)
    return -1.0;			/* error condition */
  if (x == 0)
    return (internalMode & 1)? 1.0: 0.0;
  if (x == 1)
    return (internalMode & 1)? 0.0: 1.0;
  x0 = (a + 1)/(a + b + 2);
  if (x > x0)
  { x = 1 - x;
    x0 = a;
    a = b;
    b = x0;
    flip = 1; }
  else
    flip = 0;
  tiny = DBL_EPSILON*DBL_EPSILON;
  c = 1.0/tiny;
  f = d = 1.0;
  e1 = -a;
  e2 = a + b;
  e3 = a;
  e4 = a + 1;
  e5 = b - 1;
  j = 1;
  k = 1.0;
  while (1)
  { if (j % 2)
      aa = x * e1-- * e2++ / e3++ / e4++;
    else
      aa = x * k++ * e5-- / e3++ / e4++;
    d = 1 + aa*d;
    d = 1.0/(d? d: tiny);
    c = 1 + aa/c;
    if (!c)
      c = tiny;
    del = c*d;
    f *= del;
    if (fabs(del - 1) < 3e-7)
      break;
    j++; }
  /* we use logbeta() even if /LOG is not selected to guard against */
  /* dividing by zero (if beta(a,b) < machine precision).  LS 30jan98 */
  g = a*log(x) + b*log1p(-x) - log(a) - logbeta(a,b);
  if (internalMode & 2)		/* return natural logarithm */
  { f = log(f) + g;
    return (flip ^ (internalMode & 1))? log1p(-exp(f)): f; }
  else
  { f *= exp(g);
    return (flip ^ (internalMode & 1))? 1 - f: f; }
}
/*------------------------------------------------------------------------- */
double student(double t, double nu)
/* returns Student's t distribution or, if internalMode & 1, its complement */
/* or, if internalMode & 2, its natural logarithm.  LS 22jul96 5nov96 */
{
  double result;

  internalMode ^= 1;
  result = incomplete_beta(nu/(nu + t*t), nu/2, 0.5);
  internalMode ^= 1;
  return result;
}
/*------------------------------------------------------------------------- */
double F(double F, double nu1, double nu2)
/* returns the F-ratio function or, if internalMode & 1, its complement */
/* LS 22jul96 */
{
  return incomplete_beta(nu2/(nu2 + nu1*F), nu2/2, nu1/2);
}
/*------------------------------------------------------------------------- */
double non_central_chi_square(double chi2, double nu, double lambda)
/* returns the value of the non-central chi-square distribution for */
/* chi-square <chi2>, <nu> degrees of freedom, and non-centrality */
/* parameter <lambda>.  algorithm of my own devising.  LS 19oct96  */
{
  double	a, b, c, d, y, c2, l2, i1, i2;

  if (chi2 < 0.0 || lambda < 0.0 || nu < 0) /* all parameters must be */
					    /* non-negative */
    return sqrt(-1);		/* generate error */
  l2 = lambda/2.0;
  c2 = chi2/2.0;
  i1 = 1;
  i2 = 1 + nu/2;
  a = 1.0;
  b = chi_square(chi2, nu);
  c = exp(log(c2)*(nu/2) - c2 - loggamma(nu/2 + 1));
  y = a*b;
  do
  { a *= l2/i1++;
    b -= c;
    c *= c2/i2++;
    d = a*b;
    y += d; }
  while (d*3e7 > y);
  return y*exp(-l2);
}
/*------------------------------------------------------------------------- */
double bessel_i0(double x)
/* returns value of the modified Bessel function of order zero I0. */
/* LS 2dec96.  uses Abramowitz & Stegun approximations. */
/* if internalMode & 1 then returns I0(x)*exp(-x).  LS 3dec95 */
{
  double	t;
  
  if (x < 0)
    x = -x;
  if (x < 3.75)
  { t = x/3.75;
    t *= t;
    t = (((((0.0045813*t + 0.0360768)*t + 0.2659732)*t + 1.2067492)*t
	  + 3.0899424)*t + 3.5156229)*t + 1;
    return internalMode & 1? t*exp(-x): t; }
  t = 3.75/x;
  t = ((((((((0.00392377*t - 0.01647633)*t + 0.02635537)*t - 0.02057706)*t
	   + 0.00916281)*t - 0.00157565)*t + 0.00225319)*t + 0.01328592)*t
       + 0.39894228)/sqrt(x);
  return (internalMode & 1)? t: t*exp(x);
}
/*------------------------------------------------------------------------- */
double bessel_i1(double x)
/* returns value of the modified Bessel function of order one I1. */
/* LS 3dec96.  uses Abramowitz & Stegun approximations. */
{
  double	t;
  double	sign;

  if (x < 0)
  { sign = -1;
    x = -x; }
  else sign = 1;
  if (x < 3.75)
  { t = x/3.75;
    t *= t;
    return ((((((0.00032411*t + 0.00301532)*t + 0.02658733)*t
	       + 0.15084934)*t + 0.51498869)*t + 0.87890594)*t + 0.5)*x*sign; }
  t = 3.75/x;
  return ((((((((-0.00420059*t + 0.01787654)*t - 0.02895312)*t
	       + 0.02282967)*t - 0.01031555)*t + 0.00163801)*t
	    - 0.00362018)*t - 0.03988024)*t + 0.39894228)/sqrt(x)*exp(x)*sign;
}
/*------------------------------------------------------------------------- */
double bessel_k0(double x)
/* returns value of the modified Bessel function of order zero K0. */
/* LS 3dec96.  uses Abramowitz & Stegun approximations. */
{
  double	t;

  if (x <= 0)
    return sqrt(-1);		/* generate error */
  if (x <= 2)
  { t = x/2;
    t *= t;
    return (((((0.00000740*t + 0.00010750)*t + 0.00262698)*t
	      + 0.03488590)*t + 0.23069756)*t + 0.42278420)*t
		- 0.57721566 - log(x/2)*bessel_i0(x); }
  t = 2/x;
  return ((((((0.00053208*t - 0.00251540)*t + 0.00587872)*t
	     - 0.01062446)*t + 0.02189568)*t - 0.07832358)*t
	  + 1.25331414)*exp(-x)/sqrt(x);
}
/*------------------------------------------------------------------------- */
double bessel_k1(double x)
/* returns value of the modified Bessel function of order one K1. */
/* LS 3dec96.  uses Abramowitz & Stegun approximations. */
{
  double	t;
  
  if (x <= 0)
    return sqrt(-1);		/* generate domain error */
  if (x <= 2)
  { t = x/2;
    t *= t;
    return ((((((-0.00004686*t - 0.00110404)*t - 0.01919402)*t
	       - 0.18156897)*t - 0.67278579)*t + 0.15443144)*t
	    + 1 + x*log(x/2)*bessel_i1(x))/x; }
  t = 2/x;
  return ((((((-0.00068245*t + 0.00325614)*t - 0.00780353)*t
	     + 0.01504268)*t - 0.03655620)*t + 0.23498619)*t
	  + 1.25331414)*exp(-x)/sqrt(x);
}
/*------------------------------------------------------------------------- */
double bessel_kn(int n, double x)
/* returns value of the modified Bessel function of order n Kn. */
/* LS 3dec96.  uses Abramowitz & Stegun approximations. */
{
  double	z, b0, b1, b2;
  int	i;

  if (n < 2 || x <= 0)
    return sqrt(-1);		/* generate error */
  b1 = bessel_k0(x);
  b2 = bessel_k1(x);
  n--;
  for (i = 1; i < n; i++)
  { b0 = b1;
    b1 = b2;
    z = 2.0/x;
    b2 = b0 + i*b1*z; }
  return b2;
}
/*------------------------------------------------------------------------- */
double sgn(double x)
/* returns the sign of x: +1 if x > 0, -1 if x < 0, and 0 if x == 0. */
/* LS 19may98 */
{
  if (x > 0)
    return 1.0;
  if (x < 0)
    return -1.0;
  return 0.0;
}
/*------------------------------------------------------------------------- */
int ana_array_statistics(int narg, int ps[])
/* returns mean, sdev, and other statistics for an array */
/* this a subroutine with a variable number of arguments of the
   form:  array_statistics, array, max, min, mean, [sdev, skew, kurtosis]
   where mean, etc are returned values, either float or double */
{
  int	n, nc, sdev_flag=0, skew_flag=0, kurtosis_flag=0;
  double	fac;
  pointer	q1;
  int	*save_ptr;
  int	type;
  int	iq;

  iq = ps[0];
  if (numerical(iq, NULL, NULL, &n, &q1) == ANA_ERROR)
    return ANA_ERROR;
  save_ptr = q1.l;
  nc = n;
  type = symbol_type(iq);
  if (isComplexType(type))
    return anaerror("Sorry, complex numbers not implemented.", ps[0]);
  fac = 1.0/(double) n;
  switch (narg) {
    case 7:
      kurtosis_flag = 1;
    case 6:
      skew_flag = 1;
    case 5:
      sdev_flag = 1;
  }
 /* two major branches, one for doubles and one for everyone else */
  if (type < ANA_DOUBLE) {
    float	s, sdev, skew, kurtosis, mean, ss, max, min, xq;
    int	imax, imin;
    
    s = sdev = skew = kurtosis = 0.0;
   /* need a first pass to get the mean value */
    switch (type) {
      case ANA_WORD:
	imax = imin = (int) *q1.w;
	while (n--) { 
	  iq = (int) *q1.w++;
	  s += (float) iq;
	  if (iq > imax)
	    imax = iq;
	  else if (iq < imin)
	    imin = iq;
	}
	break;
      case ANA_LONG:
	imax = imin = *q1.l;
	while (n--) { 
	  iq = *q1.l++;
	  s += (float) iq; 
	  if (iq > imax)
	    imax = iq; 
	  else if (iq < imin) 
	    imin = iq;
	}
	break;
      case ANA_FLOAT:
	max = min = *q1.f;
	while (n--) {
	  xq = *q1.f++;
	  s += xq;
	  if (xq > max)
	    max = xq; 
	  else if (xq < min)
	    min = xq;
	}
	break;
      case ANA_BYTE:
	imax = imin = (int) *q1.b;
	while (n--) { 
	  iq = (int) *q1.b++;
	  s += (float) iq;
	  if (iq > imax)
	    imax = iq;
	  else if (iq < imin)
	    imin = iq;
	}
	break;
    }
    mean = s * fac;
    /* finish the mean returned value now */
    redef_scalar(ps[3], 3, &mean);
    if (type == ANA_LONG) {
      redef_scalar(ps[1], ANA_LONG, &max);
      redef_scalar(ps[2], ANA_LONG, &min);}
    else {
      redef_scalar(ps[1], ANA_WORD, &imax);
      redef_scalar(ps[2], ANA_WORD, &imin);}
    /* need a second pass unless only mean was required */
    if (sdev_flag) {
      n = nc;
      q1.l = save_ptr;
      switch (type) {
	case ANA_WORD:
	  while (n--) {
	    s = (float) *q1.w++ - mean; 
	    ss = s * s; 
	    sdev += ss; 
	    if (skew_flag) {
	      skew += ss*s; 
	      if (kurtosis_flag) 
		kurtosis += ss*ss;
	    }
	  }
	  break;
	case ANA_LONG:
	  while (n--) {
	    s = (float) *q1.l++ - mean; 
	    ss = s * s;
	    sdev += ss; 
	    if (skew_flag) {
	      skew += ss*s; 
	      if (kurtosis_flag)
		kurtosis += ss*ss;
	    }
	  } break;
	case ANA_FLOAT:
	  while (n--) {
	    s = *q1.f++ - mean; 
	    ss = s * s; 
	    sdev += ss; 
	    if (skew_flag) {
	      skew += ss*s;
	      if (kurtosis_flag)
		kurtosis += ss*ss;
	    }
	  } break;
	case ANA_BYTE:
	  while (n--) {
	    s = (float) *q1.b++ - mean; 
	    ss = s * s; 
	    sdev += ss; 
	    if (skew_flag) {
	      skew += ss*s; 
	      if (kurtosis_flag)
		kurtosis += ss*ss;
	    }
	  } break;
      }
      sdev = sqrt(sdev/(nc-1));
      redef_scalar(ps[4], ANA_FLOAT, &sdev);
      if (skew_flag) {
	skew = skew/(nc*pow(sdev, 3));
	redef_scalar(ps[5], ANA_FLOAT, &skew);
      }
      if (kurtosis_flag) {
	kurtosis = kurtosis/(nc*pow(sdev, 4) - 3.0);
	redef_scalar(ps[6], ANA_FLOAT, &kurtosis);
      }
    }
    return ANA_OK;
  } else {
    /* the doubles case */
    double	s, sdev, skew, kurtosis, mean, ss, max, min, xq;

    s = sdev = skew = kurtosis = 0.0;
    /* need a first pass to get the mean value */
    max = min = *q1.d;
    while (n--) {
      xq = *q1.d++;
      s += xq;
      if (xq > max)
	max = xq;
      else if (xq < min) 
	min = xq;
    }
    mean = s * fac;
    /* finish the mean returned value now */
    redef_scalar(ps[3], ANA_DOUBLE, &mean);
    redef_scalar(ps[1], ANA_DOUBLE, &max);
    redef_scalar(ps[2], ANA_DOUBLE, &min);
    /* need a second pass unless only mean was required */
    if (sdev_flag) {
      n = nc;
      q1.l = save_ptr;
      while (n--) {
	s = *q1.d++ - mean;
	ss = s * s;
	sdev += ss; 
	if (skew_flag) { 
	  skew += ss*s;
	  if (kurtosis_flag)
	    kurtosis += ss*ss;
	}
      }
      
      sdev = sqrt(sdev/(nc-1));
      redef_scalar(ps[4], ANA_DOUBLE, &sdev);
      if (skew_flag) {
	skew = skew/(nc*pow(sdev, 3));
	redef_scalar(ps[5], ANA_DOUBLE, &skew);
      }
      if (kurtosis_flag) {
	kurtosis = kurtosis/(nc*pow(sdev, 4) - 3.0);
	redef_scalar(ps[6], ANA_DOUBLE, &kurtosis);
      }
    }
    return ANA_OK;
  }
}
/*------------------------------------------------------------------------- */
