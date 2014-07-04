/* This is file fun3.c.

Copyright 2013 Louis Strous, Richard Shine

This file is part of LUX.

LUX is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your
option) any later version.

LUX is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with LUX.  If not, see <http://www.gnu.org/licenses/>.
*/
/* File fun3.c */
/* Various LUX functions. */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/types.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>
#include <float.h>
#include <stdlib.h>		/* for strtol */
#include <errno.h>
#include "action.h"
#include "editorcharclass.h"
#include "luxparser.h"

int32_t	ezffti(int32_t *, float *), fade_xsize, fade_ysize, fade_dim[2];
int32_t rffti(int32_t *, float *);
int32_t rfftb(int32_t *, float *, float *);
int32_t rfftf(int32_t *, float *, float *);
int32_t rfftid(int32_t *, double *);
int32_t rfftbd(int32_t *, double *, double *);
int32_t rfftfd(int32_t *, double *, double *);
int16_t	*fade1, *fade2;
extern int32_t	scalemax, scalemin, lastmaxloc, lastminloc, maxhistsize,
  histmin, histmax, fftdp, lastmax_sym, lastmin_sym, errno;
extern scalar	lastmin, lastmax;
static	int32_t	nold = 0, fftdpold, mqold;
static pointer	work;
int32_t	minmax(int32_t *, int32_t, int32_t), neutral(void *, int32_t);
void	zerobytes(void *, int32_t), zap(int32_t);
int32_t	simple_scale(void *, int32_t, int32_t, void *);
void convertWidePointer(wideScalar *, int32_t, int32_t);
/*------------------------------------------------------------------------- */
int32_t evalString(char *expr, int32_t nmax)
/* compiles and evaluates a string, interpreting it as code */
/* non-strings are just evaluated.  If the second argument it set, */
/* then it determines up to how many numbers are read from the string */
/* and put in an array. */
/* If instead of a second argument the keyword /ALLNUMBER is specified, */
/* then all numbers are read from the string and put in an array. */
/* All read numbers have no letter immediately preceding them.  Minus signs */
/* are honored.  All valid LUX number styles are supported (including */
/* type and base specifications, and fractional and exponentiated numbers). */
/* If numbers are read, then any non-number is ignored. */
/* LS 2may96 */
{
 char	*save, *text;
 int32_t	result, readNumber(YYSTYPE *), n, type, temp, size;
 void	translateLine(void), convertScalar(scalar *, int32_t, int32_t);
 pointer	p;
 scalar	s;
 extern char	*currentChar;
 extern int32_t	tempSym;
 YYSTYPE	valp;
 int16_t	popList(void);
 int32_t	compileString(char *);
 void	pushList(int16_t);

 if (nmax < 0)			/* /ALLNUMBER */
   nmax = INT32_MAX;
 if (nmax) {
   save = currentChar;		/* remember what we were interpreting before */
   currentChar = expr;
   n = 0;
   type = LUX_INT8;
   while (n < nmax) {
     while (!isNumberChar((uint8_t) *currentChar)
	    && *currentChar) /* seek digit */
       currentChar++;
     if (!*currentChar)		/* no (more) digits */
       break;
     if (currentChar > expr && isalpha((uint8_t) currentChar[-1])) {
       /* not a separate number */
       while (isNumberChar((uint8_t) *currentChar))
	 currentChar++;
       continue;
     }
     readNumber(&valp);		/* read number in symbol # *lvalp */
     pushList(valp);
     if (scalar_type(valp) > type)
       type = scalar_type(valp); /* remember highest type */
     n++;
   }
   if (n) {			/* got some numbers */
     size = lux_type_size[type];
     if (n == 1) {		/* only a single number: use LUX_SCALAR */
       result = scalar_scratch(type);
       p.b = &scalar_value(result).b + size; 
     } else {
       result = array_scratch(type, 1, &n);
       p.b = (uint8_t *) array_data(result) + n*size;
     }
     while (n--) {
       temp = popList();
       convertScalar(&s, temp, type);
       p.b -= size;
       memcpy(p.b, &s, size);	/* this assumes that all data types are */
				/* left-aligned in the union. */
       zap(temp);
     }
   } else
     result = LUX_ZERO;		/* nothing found; return 0 */
   currentChar = save;
   return result;
 }
 /* If we get here then we're interpreting the complete line as an */
 /* LUX expression */
 allocate(text, strlen(expr) + 7, char);
 strcpy(text, "!TEMP=");	/* we assign the evaluation result to !TEMP */
 strcat(text, expr);
 result = compileString(text);
 free(text);
 return (result > 0)? copySym(tempSym): LUX_ERROR;
}
/*------------------------------------------------------------------------- */
int32_t lux_eval(int32_t narg, int32_t ps[])
/* evaluates a string argument.  If there is a single argument and that
   argument is a single name, then we seek the number of the symbol that
   has that name and return the number, if any.  Otherwise we return the
   value of the expression.  LS 7jan99 */
{
  int32_t	nmax, result;
  char	*expr, *p1, *p2, *p3, c;
  
  if (symbol_class(*ps) != LUX_STRING)
    return *ps;
  if (narg > 1) {		/* read numbers */
    nmax = int_arg(ps[1]);
    if (nmax < 1)
      return luxerror("Cannot read a negative number of values", ps[1]);
  } else
    nmax = INT32_MAX;
  if (narg == 1 || internalMode & 1) /* /ALLNUMBER */
    nmax = -nmax;
  expr = string_arg(*ps);

  p1 = expr;
  while (*p1 && isWhiteSpace((int32_t) *p1))
    p1++;
  if (isFirstChar((int32_t) *p1)) {	/* an identifier? */
    p2 = p1 + 1;
    while (isNextChar((int32_t) *p2)) /* span the identifier */
      p2++;
    p3 = p2;
    while (*p3 && isWhiteSpace((int32_t) *p3))
      p3++;
    if (!*p3) {			/* found an identifier and whitespace */
      c = *p2;			/* save */
      *p2 = '\0';		/* temporary end */
      p1 = strsave(p1);		/* make a copy we can modify */
      *p2 = c;			/* restore */
      p2 = p1;
      while (*p2) {		/* transform to upper case */
	*p2 = toupper(*p2);
	p2++;
      }
      if (curContext > 0	/* we're inside some subroutine or function */
	  && *p1 != '$'
	  && *p1 != '#'
	  && *p1 != '!')	/* and the variable is a local one */
	result = lookForVarName(p1, curContext);
      else			/* we look for a global variable
				   and create it if necessary */
	result = findVarName(p1, 0);
      free(p1);			/* don't need it anymore */
      if (result != LUX_ERROR)
	return result;
    }
  } 
  result = evalString(expr, nmax);
  if (result > 0 && !isFreeTemp(result))
    result = copySym(result);
  return result;
}
/*------------------------------------------------------------------------- */
int32_t lux_tense(int32_t narg, int32_t ps[])			/* tense function */ 
/* splines under tension */
/* THE CALL IS  YF=TENSE(X,Y,XF,[SIGMA],[DLHS,DRHS]) */
{
  int32_t	i, iq, len[3], n, result_sym, nf;
  double	sigma, der_left, der_right, *st, *yf;
  pointer p[3];
  array	*h;
  int32_t	curv1_(int32_t *n, double *x, double *y, double *slp1, double *slpn,
	   double *yp, double *temp, double *sigma, double *xf, double *yf,
	   int32_t *nf);
					/* first 3 args must be 1-D vectors */
  for (i=0;i<3;i++) {
    iq = ps[i];
    if ( sym[iq].class != LUX_ARRAY ) return cerror(NEED_ARR, ps[i]);
    h = (array *) sym[iq].spec.array.ptr;
    if ( h->ndim != 1) return cerror(NEED_1D_ARR, ps[i]); 	/* ck if 1-D */
    /* double each one */
    iq = lux_double(1, &iq);
    h = (array *) sym[iq].spec.array.ptr;
    len[i] = h->dims[0];
    p[i].l = (int32_t *) ((char *)h + sizeof(array));
  }
  /* take smaller of X and Y size */
  n = MIN( len[0], len[1]);
  sigma = -1.0;	der_left = 0.0;	der_right = 0.0;	/* defaults */
  if (narg > 3) sigma = -double_arg( ps[3] );
  /* sign of sigma is a flag, + => that slopes (der's) are input */
  if (narg > 4)  { der_left = double_arg( ps[4] );  sigma = - sigma; }
  if (narg > 5) der_right = double_arg( ps[5] );
  result_sym = array_clone(ps[2], 4);
  h = (array *) sym[result_sym].spec.array.ptr;
  yf = (double *) ((char *)h + sizeof(array));
  nf = len[2];
  /* scratch storage for 2 double arrays */
  st = (double *) malloc( 16 * n );
  /*printf("n = %d\n", n);*/
  iq = curv1_(&n, p[0].d, p[1].d, &der_left, &der_right, st, (st+n), &sigma,
	      p[2].d, yf, &nf);
  /*printf("returned iq = %d\n", iq);*/
  free( st);
  if ( iq == 1) return result_sym; else return -1;
}
/*------------------------------------------------------------------------- */
int32_t lux_tense_curve(int32_t narg, int32_t ps[])/* tense_curve function */
/* splines under tension */
/* THE CALL IS  XY=TENSE_CURVE(X,Y,XS,[SIGMA],[DLHS,DRHS]) */
{
/*
  this differs from plain TENSE in that it allows any open ended
  curve to be defined, it does not have to be a single valued function
  of x
  the specification of points along the curve is complicated because of
  this, the XS input is in units of the polygon length of the curve
  and can range from 0 to 1.0
  the output is a 2-D array containing matching x and y coordinates on
  the curve
*/
  int32_t	i, iq, len[3], n, result_sym, dim[2], nf;
  double	sigma, der_left, der_right, *st, *yf, *xf;
  pointer p[3];
  array	*h;
  int32_t	kurv1_(int32_t *n, double *x, double *y, double *slp1, double *slpn,
	   double *xp, double *yp, double *temp, double *sigma, double *t,
	   double *xs, double *ys, int32_t *nf);

				/* first 3 args must be 1-D vectors */
for (i=0;i<3;i++) {
iq = ps[i];
if ( sym[iq].class != 4 ) return cerror(NEED_ARR, ps[i]);
h = (array *) sym[iq].spec.array.ptr;
if ( h->ndim != 1) return cerror(NEED_1D_ARR, ps[i]); 	/* ck if 1-D */
						/* double each one */
iq = lux_double(1, &iq);
h = (array *) sym[iq].spec.array.ptr;
len[i] = h->dims[0];
p[i].l = (int32_t *) ((char *)h + sizeof(array));
}
/* take smaller of X and Y size */
n = MIN( len[0], len[1]);
sigma = 1.0;	der_left = 0.0;	der_right = 0.0;	/* defaults */
if (narg > 3) sigma = double_arg( ps[3] );
if (narg > 4) der_left = double_arg( ps[4] );
if (narg > 5) der_right = double_arg( ps[5] );
sigma = - sigma;
dim[0] = nf = len[2];	dim[1] = 2;
result_sym = array_scratch(4, 2, dim);
h = (array *) sym[result_sym].spec.array.ptr;
xf = (double *) ((char *)h + sizeof(array));
yf = xf + nf;
				/* scratch storage for 3 double arrays */
st = (double *) malloc( 24 * n );
iq = kurv1_( &n, p[0].d, p[1].d, &der_left, &der_right, st,(st+n),(st+n+n), 
	&sigma, p[2].d, xf, yf, &nf);
free( st);
if ( iq == 1) return result_sym; else return -1;
}
/*------------------------------------------------------------------------- */
int32_t lux_tense_loop(int32_t narg, int32_t ps[])/* tense_loop function */		
/* generates spline under tension for any closed (x,y) curve */
/* THE CALL IS  XY=TENSE_LOOP(X,Y,XS,[SIGMA]) */
{
  /*
    this differs from TENSE_CURVE in that it allows a closed
    curve to be defined, it does not have to be a single valued function
    of x
    the specification of points along the curve is complicated because of
    this, the XS input is in units of the polygon length of the curve
    and can range from 0 to 1.0
    the output is a 2-D array containing matching x and y coordinates on
    the curve
    */
  int32_t	i, iq, len[3], n, result_sym, dim[2], nf;
  double	sigma, *st, *yf, *xf;
  pointer p[3];
  array	*h;
  int32_t kurvp1_(int32_t *n, double *x, double *y, double *xp, double *yp,
	      double *temp, double *sigma, double *t, double *xs, double *ys,
	      int32_t *nf);
					/* first 3 args must be 1-D vectors */
for (i=0;i<3;i++) {
iq = ps[i];
if ( sym[iq].class != 4 ) return cerror(NEED_ARR, ps[i]);
h = (array *) sym[iq].spec.array.ptr;
if ( h->ndim != 1) return cerror(NEED_1D_ARR, ps[i]); 	/* ck if 1-D */
						/* double each one */
iq = lux_double(1, &iq);
h = (array *) sym[iq].spec.array.ptr;
len[i] = h->dims[0];
p[i].l = (int32_t *) ((char *)h + sizeof(array));
}
/* take smaller of X and Y size */
n = MIN( len[0], len[1]);
sigma = 1.0;	/* defaults */
if (narg > 3) sigma = double_arg( ps[3] );
sigma = - sigma;
dim[0] = nf = len[2];	dim[1] = 2;
result_sym = array_scratch(4, 2, dim);
h = (array *) sym[result_sym].spec.array.ptr;
xf = (double *) ((char *)h + sizeof(array));
yf = xf + nf;
				/* scratch storage for 3 double arrays */
				/* one of which is 2*nf long */
st = (double *) malloc( 32 * n );
iq = kurvp1_( &n, p[0].d, p[1].d, st,(st+n),(st+n+n), 
	&sigma, p[2].d, xf, yf, &nf);
free( st);
if ( iq == 1) return result_sym; else return -1;
}
/*------------------------------------------------------------------------- */
int32_t lux_sc(int32_t narg, int32_t ps[])	/* sc routine */		
/* sine-cosine style FFT --  sc, x, s, c */
{
  int32_t	iq, nd, outer, n, mq, dim[MAX_DIMS], type, jq, nn;
  pointer	q1,q2,q3;
  int32_t	ezffti(int32_t *n, float *wsave), ezfftid(int32_t *n, double *wsave),
    ezfftf(int32_t *n, float *r, float *azero, float *a, float *b, float *wsave),
    ezfftfd(int32_t *n, double *r, double *azero, double *a, double *b,
	    double *wsave);

  iq = ps[0];
  if (!symbolIsArray(iq))
    return cerror(NEED_ARR, ps[0]);
  /*float the input */
  if (fftdp == 0)		/* single precision */
    iq = lux_float(1, ps);
  else				/* double precision */
    iq = lux_double(1, ps);

  q1.l = array_data(iq);

  /* find inner and product of all outer dimensions */
  nd = array_num_dims(iq);
  n = array_dims(iq)[0];
  outer = array_size(iq)/n;
  memcpy(dim + 1, array_dims(iq)+ 1, (nd - 1)*sizeof(int32_t));
  mq = array_size(iq)*lux_type_size[array_type(iq)];

	/* scratch storage is needed, can we use the last one setup ? */
  if (nold != n || fftdp != fftdpold || mq > mqold) {/* set up work space */
    if (nold)
      free(work.b);		/* delete old if any */
    mq = n*28 + 120;
    if (!fftdp)
      mq = mq/2;
    work.l = (int32_t *) malloc(mq);
    if (!fftdp)
      ezffti(&n, work.f);
    else
      ezfftid(&n, work.d);
    nold = n;
    fftdpold = fftdp;
    mqold = mq;
  }
					/* configure the output arrays */
  nn = n/2 + 1;
  dim[0] = nn;
  iq = ps[1];
  jq = ps[2];
  type = (fftdp? LUX_DOUBLE: LUX_FLOAT);
  if (redef_array(iq, type, nd, dim) != 1
      || redef_array(jq, type, nd, dim) != 1)
    return LUX_ERROR;
					/* get addresses */
  q2.l = array_data(iq);		/* <s> */
  q3.l = array_data(jq);		/* <c> */
					/* loop over the outer dimensions */
  while (outer--) {
    switch (type) {
      case LUX_FLOAT:
	ezfftf(&n, q1.f, q3.f, q3.f + 1, q2.f + 1, work.f);
	*q2.f = 0.0;		/* zero-frequency sine always equal to 0 */
	q1.f += n;
	q2.f += nn;
	q3.f += nn;
	break;
      case LUX_DOUBLE:
	ezfftfd(&n, q1.d, q3.d, q3.d + 1, q2.d + 1, work.d);
	*q2.d = 0.0;		/* zero-frequency sine always equal to 0 */
	q1.d += n;
	q2.d += nn;
	q3.d += nn;
	break;
    }
  }
  return 1;
}
/*------------------------------------------------------------------------- */
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_fft_halfcomplex.h>
static gsl_fft_real_wavetable* rwave = NULL;
static int32_t nrwave = -1;
static gsl_fft_real_workspace* rwork = NULL;
static int32_t nrwork = -1;
static gsl_fft_halfcomplex_wavetable* hwave = NULL;
static int32_t nhwave = -1;
static double *ffttemp = NULL;
static int32_t nffttemp = -1;

gsl_fft_real_wavetable *update_rwave(int32_t n)
{
  if (n != nrwave) {
    gsl_fft_real_wavetable_free(rwave);
    rwave = gsl_fft_real_wavetable_alloc(n);
    nrwave = rwave? n: -1;
  }
  return rwave;
}
/*------------------------------------------------------------------------- */
void clear_rwave(void)
{
  gsl_fft_real_wavetable_free(rwave);
  rwave = NULL;
  nrwave = -1;
}
/*------------------------------------------------------------------------- */
gsl_fft_halfcomplex_wavetable *update_hwave(int32_t n)
{
  if (n != nhwave) {
    gsl_fft_halfcomplex_wavetable_free(hwave);
    hwave = gsl_fft_halfcomplex_wavetable_alloc(n);
    nhwave = hwave? n: -1;
  }
  return hwave;
}
/*------------------------------------------------------------------------- */
void clear_hwave(void)
{
  gsl_fft_halfcomplex_wavetable_free(hwave);
  hwave = NULL;
  nhwave = -1;
}
/*------------------------------------------------------------------------- */
gsl_fft_real_workspace *update_rwork(int32_t n)
{
  if (n != nrwork) {
    gsl_fft_real_workspace_free(rwork);
    rwork = gsl_fft_real_workspace_alloc(n);
    nrwork = rwork? n: -1;
  }
  return rwork;
}
/*------------------------------------------------------------------------- */
void clear_rwork(void)
{
  gsl_fft_real_workspace_free(rwork);
  rwork = NULL;
  nrwork = -1;
}
/*------------------------------------------------------------------------- */
double *update_ffttemp(int32_t n)
{
  if (n != nffttemp) {
    free(ffttemp);
    ffttemp = malloc(n*sizeof(double));
    nffttemp = ffttemp? n: -1;
  }
  return ffttemp;
}
/*------------------------------------------------------------------------- */
void clear_ffttemp(void)
{
  free(ffttemp);
  ffttemp = NULL;
  nffttemp = -1;
}
/*------------------------------------------------------------------------- */
int32_t gsl_fft(double *data, size_t n, size_t stride)
{
  if (!update_rwave(n) || !update_rwork(n))
    return 1;
  
  int32_t result = gsl_fft_real_transform(data, stride, n, rwave, rwork);
  if (internalMode & 2) {	/* /AMPLITUDES */
    double factor1 = 1.0/n;
    /* average */
    *data *= factor1;
    data += stride;
    /* non-Nyquist non-zero frequencies */
    int32_t i;
    double factor2 = 2.0/n;
    /* 5 -> 3; 6 -> 3; 7 -> 4 */
    for (i = 1; i <= (n + 1)/2; i += 2) {
      *data *= factor2;
      data += stride;
    }
    /* Nyquist frequency */
    if (n % 2 == 0)
      *data *= factor1;
  }
  return result;
}
BIND(gsl_fft, i_sd_iDaLarDq_000, f, FFT, 1, 2, "1ALLAXES:2AMPLITUDES");
BIND(gsl_fft, i_sd_iDaLa_000, s, FFT, 1, 2, "1ALLAXES:2AMPLITUDES");
/*------------------------------------------------------------------------- */
int32_t gsl_fft_back(double *data, size_t n, size_t stride)
{
  if (!update_hwave(n) || !update_rwork(n))
    return 1;

  if (internalMode & 2) {	/* /AMPLITUDES */
    double factor1 = n;
    /* average */
    *data *= factor1;
    data += stride;
    /* non-Nyquist non-zero frequencies */
    int32_t i;
    double factor2 = n/2.0;
    /* 5 -> 3; 6 -> 3; 7 -> 4 */
    for (i = 1; i <= (n + 1)/2; i += 2) {
      *data *= factor2;
      data += stride;
    }
    /* Nyquist frequency */
    if (n % 2 == 0)
      *data *= factor1;
  }
  return gsl_fft_halfcomplex_inverse(data, stride, n, hwave, rwork);
}
BIND(gsl_fft_back, i_sd_iDaLarDq_000, f, FFTB, 1, 2, "1ALLAXES:2AMPLITUDES");
BIND(gsl_fft_back, i_sd_iDaLa_000, s, FFTB, 1, 2, "1ALLAXES:2AMPLITUDES");
/*------------------------------------------------------------------------- */
int32_t hilbert(double *data, size_t n, size_t stride)
{
  if (!update_rwave(n) || !update_hwave(n) || !update_rwork(n))
    return 1;
  
  int32_t result = gsl_fft_real_transform(data, stride, n, rwave, rwork);
  if (result)
    return 1;
  int32_t i;
  for (i = 1; i < n - 1; i+= 2) {
    /* advance phase by 90 degrees */
    double t = data[i*stride];
    data[i*stride] = -data[(i + 1)*stride];
    data[(i + 1)*stride] = t;
  }
  return gsl_fft_halfcomplex_inverse(data, stride, n, hwave, rwork);
}
BIND(hilbert, i_sd_iDaLarDq_000, f, HILBERT, 1, 2, "1ALLAXES");
BIND(hilbert, i_sd_iDaLa_000, s, HILBERT, 1, 2, "1ALLAXES");
/*------------------------------------------------------------------------- */
int32_t gsl_fft_expand(double *sdata, size_t scount, size_t sstride,
		   double *tdata, size_t tcount, size_t tstride)
{
  if (!update_rwave(scount) || !update_hwave(tcount))
    return 1;
  
  int32_t i, result;
  double *sdata2 = sdata;
  double *tdata2 = tdata;
  if (tcount >= scount) {
    /* copy source to target, append zeros, forward fft of copied source data,
       then backward fft including appended zeros */
    for (i = 0; i < scount; i++) {
      *tdata2 = *sdata2;
      sdata2 += sstride;
      tdata2 += tstride;
    }
    for ( ; i < tcount; i++) {
      *tdata2 = 0.0;
      tdata2 += tstride;
    }
    update_rwork(scount);
    result = gsl_fft_real_transform(tdata, tstride, scount, rwave, rwork);
    if (result)
      return 1;
    update_rwork(tcount);
    result = gsl_fft_halfcomplex_inverse(tdata, tstride, tcount, hwave, rwork);
    if (result)
      return 1;
  } else {
    /* copy source to temporary storage, forward fft, then backward fft
       for target size, then copy to target */
    double *temp = malloc(tcount*sizeof(double));
    if (!temp) {
      errno = ENOMEM;
      return 1;
    }
    double *temp2 = temp;
    for (i = 0; i < scount; i++) {
      *temp2++ = *sdata2;
      sdata2 += sstride;
    }
    update_rwork(scount);
    result = gsl_fft_real_transform(temp, 1, scount, rwave, rwork);
    if (!result) {
      update_rwork(tcount);
      result = gsl_fft_halfcomplex_inverse(temp, 1, tcount, hwave, rwork);
    }
    if (!result) {
      temp2 = temp;
      for (i = 0; i < tcount; i++) {
	*tdata2 = *temp2++;
	tdata2 += tstride;
      }
    }
    free(temp);
  }
  return 0;
}
/*------------------------------------------------------------------------- */
int32_t lux_fft_expand(int32_t narg, int32_t ps[])
{
  pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D1;rD1", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  double factor = *ptrs[1].d;
  if (factor <= 0)
    return luxerror("Need positive expansion factor", ps[1]);
  
  int32_t ndim = infos[0].ndim;
  int32_t *dims = malloc(ndim*sizeof(int32_t));
  if (!dims)
    return cerror(ALLOC_ERR, 0);
  memcpy(dims, infos[0].dims, ndim*sizeof(int32_t));
  dims[0] = (int32_t) (dims[0]*factor + 0.5);
  standard_redef_array(iq, LUX_DOUBLE, ndim, dims, 0, NULL,
		       &ptrs[2], &infos[2]);
  free(dims);
  setAxes(&infos[0], 1, NULL, SL_EACHBLOCK);
  setAxes(&infos[2], 1, NULL, SL_EACHBLOCK);

  do {
    gsl_fft_expand(ptrs[0].d, infos[0].dims[0], 1,
		   ptrs[2].d, infos[2].dims[0], 1);
    ptrs[0].d += infos[0].singlestep[1];
    ptrs[2].d += infos[2].singlestep[2];
  } while (advanceLoop(&infos[0], &ptrs[0]),
	   advanceLoop(&infos[2], &ptrs[2]) < infos[2].ndim);
  return iq;
}
REGISTER(fft_expand, f, FFTEXPAND, 2, 2, NULL);
/*------------------------------------------------------------------------- */
//#define SQRTHALF	M_SQRT1_2
//#define SQRTWO		M_SQRT2
/*------------------------------------------------------------------------- */
int32_t lux_real(int32_t narg, int32_t ps[])
/* returns the real part of the argument.  LS 17nov98 */
{
  int32_t	iq, outtype, result, n;
  pointer	value, trgt;

  if (!symbolIsNumerical(ps[0]))
    return cerror(ILL_CLASS, ps[0]);
  if (!isComplexType(symbol_type(ps[0])))
    return ps[0];		/* nothing to do */

  outtype = (symbol_type(ps[0]) == LUX_CFLOAT)? LUX_FLOAT: LUX_DOUBLE;
  
  iq = ps[0];
  switch (symbol_class(iq)) {
    case LUX_SCAL_PTR:
      value.cf = scal_ptr_pointer(iq).cf;
      result = scalar_scratch(outtype);
      trgt.b = &scalar_value(result).b;
      n = 1;
      break;
    case LUX_CSCALAR:
      value.cf = complex_scalar_data(iq).cf;
      result = scalar_scratch(outtype);
      trgt.b = &scalar_value(result).b;
      n = 1;
      break;
    case LUX_CARRAY:
      value.v = array_data(iq);
      result = array_clone(iq, outtype);
      trgt.v = array_data(result);
      n = array_size(iq);
      break;
  }

  switch (outtype) {
    case LUX_FLOAT:
      while (n--)
	*trgt.f++ = value.cf++->real;
      break;
    case LUX_DOUBLE:
      while (n--)
	*trgt.d++ = value.cd++->real;
      break;
  }
  return result;
}
/*------------------------------------------------------------------------- */
int32_t lux_imaginary(int32_t narg, int32_t ps[])
/* returns the imaginary part of the argument.  LS 17nov98 */
{
  int32_t	iq, outtype, result, n;
  pointer	value, trgt;

  if (!symbolIsNumerical(ps[0]))
    return cerror(ILL_CLASS, ps[0]);
  if (!isComplexType(symbol_type(ps[0])))
    return lux_zero(1, &ps[0]);	/* no imaginary part */

  outtype = (symbol_type(ps[0]) == LUX_CFLOAT)? LUX_FLOAT: LUX_DOUBLE;
  
  iq = ps[0];
  switch (symbol_class(iq)) {
    case LUX_SCAL_PTR:
      value.cf = scal_ptr_pointer(iq).cf;
      result = scalar_scratch(outtype);
      trgt.b = &scalar_value(result).b;
      n = 1;
      break;
    case LUX_CSCALAR:
      value.cf = complex_scalar_data(iq).cf;
      result = scalar_scratch(outtype);
      trgt.b = &scalar_value(result).b;
      n = 1;
      break;
    case LUX_CARRAY:
      value.v = array_data(iq);
      result = array_clone(iq, outtype);
      trgt.v = array_data(result);
      n = array_size(iq);
      break;
  }

  switch (outtype) {
    case LUX_FLOAT:
      while (n--)
	*trgt.f++ = value.cf++->imaginary;
      break;
    case LUX_DOUBLE:
      while (n--)
	*trgt.d++ = value.cd++->imaginary;
      break;
  }
  return result;
}
/*------------------------------------------------------------------------- */
int32_t lux_arg(int32_t narg, int32_t ps[])
/* returns the complex argument of the argument.  LS 17nov98 */
{
  int32_t	iq, outtype, result, type, n;
  pointer	value, trgt;

  if (!symbolIsNumerical(ps[0]))
    return cerror(ILL_CLASS, ps[0]);

  type = symbol_type(ps[0]);
  outtype = (type == LUX_DOUBLE || symbol_type(ps[0]) == LUX_CDOUBLE)?
    LUX_DOUBLE: LUX_FLOAT;
  
  iq = ps[0];
  switch (symbol_class(iq)) {
    case LUX_SCAL_PTR:
      value.cf = scal_ptr_pointer(iq).cf;
      result = scalar_scratch(outtype);
      trgt.b = &scalar_value(result).b;
      n = 1;
      break;
    case LUX_SCALAR:
      value.b = &scalar_value(iq).b;
      result = scalar_scratch(outtype);
      trgt.b = &scalar_value(result).b;
      n = 1;
      break;
    case LUX_CSCALAR:
      value.cf = complex_scalar_data(iq).cf;
      result = scalar_scratch(outtype);
      trgt.b = &scalar_value(result).b;
      n = 1;
      break;
    case LUX_CARRAY: case LUX_ARRAY:
      value.v = array_data(iq);
      result = array_clone(iq, outtype);
      trgt.v = array_data(result);
      n = array_size(iq);
      break;
  }

  switch (outtype) {
    case LUX_FLOAT:
      switch (type) {
	case LUX_INT8:
	  while (n--)
	    *trgt.f++ = 0.0;
	  break;
	case LUX_INT16:
	  while (n--)
	    *trgt.f++ = (*value.w++ >= 0)? 0.0: M_PI;
	  break;
	case LUX_INT32:
	  while (n--)
	    *trgt.f++ = (*value.l++ >= 0)? 0.0: M_PI;
	  break;
	case LUX_INT64:
	  while (n--)
	    *trgt.f++ = (*value.q++ >= 0)? 0.0: M_PI;
	  break;
	case LUX_FLOAT:
	  while (n--)
	    *trgt.f++ = (*value.f++ >= 0)? 0.0: M_PI;
	  break;
	case LUX_CFLOAT:
	  while (n--) {
	    *trgt.f++ = atan2(value.cf->imaginary, value.cf->real);
	    value.cf++;
	  }
	  break;
      }
      break;
    case LUX_DOUBLE:
      switch (type) {
	case LUX_INT8:
	  while (n--)
	    *trgt.d++ = 0.0;
	  break;
	case LUX_INT16:
	  while (n--)
	    *trgt.d++ = (*value.w++ >= 0)? 0.0: M_PI;
	  break;
	case LUX_INT32:
	  while (n--)
	    *trgt.d++ = (*value.l++ >= 0)? 0.0: M_PI;
	  break;
	case LUX_INT64:
	  while (n--)
	    *trgt.d++ = (*value.q++ >= 0)? 0.0: M_PI;
	  break;
	case LUX_FLOAT:
	  while (n--)
	    *trgt.d++ = (*value.f++ >= 0)? 0.0: M_PI;
	  break;
	case LUX_DOUBLE:
	  while (n--)
	    *trgt.d++ = (*value.d++ >= 0)? 0.0: M_PI;
	  break;
	case LUX_CDOUBLE:
	  while (n--) {
	    *trgt.d++ = atan2(value.cd->imaginary, value.cd->real);
	    value.cd++;
	  }
	  break;
      }
      break;
  }
  return result;
}
/*------------------------------------------------------------------------- */
int32_t lux_complex(int32_t narg, int32_t ps[])
/* returns a complex version of the argument */
{
  if (!symbolIsNumerical(ps[0]))
    return cerror(ILL_CLASS, ps[0]);
  if (isComplexType(symbol_type(ps[0])))
    return ps[0];		/* already done */
  return (symbol_type(ps[0]) == LUX_DOUBLE)?
    lux_cdouble(1, ps): lux_cfloat(1, ps);
}
/*------------------------------------------------------------------------- */
int32_t fftshift(int32_t narg, int32_t ps[], int32_t subroutine)
/* y = FFTSHIFT(data,dist) */
{
  int32_t	iq, n, mq, type, j, jq, result, step, ndist;
  pointer	src, trgt, work, tmp, otmp, src0, trgt0, dist,
    sines, cosines;
  scalar	v, factor;
  int32_t	rfftb(int32_t *, float *, float *), rfftf(int32_t *, float *, float *),
    rfftbd(int32_t *, double *, double *), rfftfd(int32_t *, double *, double *),
    rffti(int32_t *, float *), rfftid(int32_t *, double *), lux_indgen(int32_t, int32_t *);
  loopInfo	srcinfo, trgtinfo;

  iq = ps[0];			/* <data> */
  if (!symbolIsArray(iq))
    return cerror(NEED_ARR, ps[0]);

  if (subroutine) {
    type = symbol_type(iq);
    switch (type) {
      case LUX_FLOAT:
	fftdp = 0;
	break;
      case LUX_DOUBLE:
	fftdp = 1;
	break;
      default:
	return cerror(ILL_TYPE, iq);
    }
  } else {
    /*float the input */
    if (fftdp == 0) {		/* single precision */
      iq = lux_float(1, ps);
      type = LUX_FLOAT;
    } else {			/* double precision */
      iq = lux_double(1, ps);
      type = LUX_DOUBLE;
    }
  }

  jq = lux_indgen(1, &ps[1]);	/* axes */
  
  if (subroutine) {
    if (standardLoop(iq, jq, SL_EACHROW, type,
		     &srcinfo, &src, NULL, NULL, NULL) < 0)
      return LUX_ERROR;
    trgtinfo = srcinfo;
    trgt = src;
    trgtinfo.data = &trgt;
    trgtinfo.data0 = trgt.v;
    result = iq;
  } else {
    if (standardLoop(iq, jq, SL_EACHROW, type,
		     &srcinfo, &src, &result, &trgtinfo, &trgt) < 0)
      /* restore dimensional structure of iq, if necessary */
      return LUX_ERROR;
  }

  if (numerical(ps[1], NULL, NULL, &ndist, NULL) == LUX_ERROR)
    return LUX_ERROR;
  numerical(((type == LUX_FLOAT)? lux_float: lux_double)(1, &ps[1]),
	    NULL, NULL, NULL, &dist);

  trgt0 = trgt;
  src0 = src;

  work.b = otmp.b = sines.b = cosines.b = NULL;
  do {
    trgt = trgt0;
    src = src0;
    n = srcinfo.rdims[0];
    mq = n*20 + 120;
    if (!fftdp)
      mq = mq/2;
    work.l = (int32_t *) realloc(work.l, mq);
    /* get space to store phase numbers */
    j = (n/2)*lux_type_size[type];
    sines.f = malloc(j);
    cosines.f = malloc(j);
    if (!work.l || !sines.f || !cosines.f) {
      if (!subroutine) 
	zap(result);
      result = cerror(ALLOC_ERR, 0);
      break;
    }
    if (!fftdp)
      rffti(&n, work.f);
    else
      rfftid(&n, work.d);

    tmp.f = otmp.f = realloc(otmp.b, n*lux_type_size[type]);
    if (tmp.f == NULL) {
      if (!subroutine)
	zap(result);
      result = cerror(ALLOC_ERR, 0);
      break;
    }

    /* the FFT routines (e.g., rfftf) return results in the following order:
     first the sum, then for each frequency (in order of increasing frequencies)
     the cosine and the sine coefficients, and finally (for even numbers of
     values) the Nyquist term */

    step = srcinfo.step[0];

    switch (type) {
      case LUX_FLOAT:
	if (*dist.f) {
	  /* now calculate the phase factors */
	  v.f = 2*M_PI*(*dist.f)/n;
	  for (j = 1; j <= n/2; j++) {
	    sines.f[j - 1] = sin(v.f*j);
	    cosines.f[j - 1] = cos(v.f*j);
	  }
	  factor.f = 1.0/n;
	  do {
	    /* put current data string in tmp.f */
	    for (j = 0; j < n; j++) {
	      *tmp.f++ = *src.f;
	      src.f += step;
	    }
	    tmp.f = otmp.f;
	    /* apply forward FFT */
	    rfftf(&n, tmp.f, work.f);
	    /* advance the phases:
	       the total (tmp.f[0]) remains unchanged */
	    for (j = 1; j <= (n - 1)/2; j++) {
	      v.f = tmp.f[2*j - 1]*cosines.f[j - 1]
		+ tmp.f[2*j]*sines.f[j - 1];
	      tmp.f[2*j] = tmp.f[2*j]*cosines.f[j - 1]
		- tmp.f[2*j - 1]*sines.f[j - 1];
	      tmp.f[2*j - 1] = v.f;
	    }
	    if (!(n % 2))
	      tmp.f[n - 1] *= cosines.f[n/2 - 1];
	    /* apply backward FFT */
	    rfftb(&n, tmp.f, work.f);
	    /* put in result array */
	    for (j = 0; j < n; j++) {
	      *trgt.f = *tmp.f++ * factor.f;
	      trgt.f += step;
	    }
	    tmp.f = otmp.f;
	  } while (advanceLoop(&trgtinfo, &trgt),
		   advanceLoop(&srcinfo, &src) < srcinfo.rndim);
	} else if (src0.f != trgt0.f)
	  memcpy(trgt0.f, src0.f, array_size(iq)*sizeof(type));
	dist.f++;
	break;
      case LUX_DOUBLE:
	if (*dist.d) {
	  /* now calculate the phase factors */
	  v.d = 2*M_PI*(*dist.d)/n;
	  for (j = 1; j <= n/2; j++) {
	    sines.d[j - 1] = sin(v.d*j);
	    cosines.d[j - 1] = cos(v.d*j);
	  }
	  factor.d = 0.5/n;
	  do {
	    /* put current data string in tmp.d */
	    for (j = 0; j < n; j++) {
	      *tmp.d++ = *src.d;
	      src.d += step;
	    }
	    tmp.d = otmp.d;
	    /* apply forward FFT */
	    rfftfd(&n, tmp.d, work.d);
	    /* advance the phases:
	       the total (tmp.f[0]) remains unchanged */
	    for (j = 1; j <= (n - 1)/2; j++) {
	      v.d = tmp.d[2*j - 1]*cosines.d[j - 1]
		+ tmp.d[2*j]*sines.d[j - 1];
	      tmp.d[2*j] = tmp.d[2*j]*cosines.d[j - 1]
		- tmp.d[2*j - 1]*sines.d[j - 1];
	      tmp.d[2*j - 1] = v.d;
	    }
	    if (!(n % 2))
	      tmp.d[n - 1] *= cosines.d[n/2 - 1];
	    /* apply backward FFT */
	    rfftbd(&n, tmp.d, work.d);
	    /* put in result array */
	    for (j = 0; j < n; j++) {
	      *trgt.d = *tmp.d++ * factor.d;
	      trgt.d += step;
	    }
	    tmp.d = otmp.d;
	  } while (advanceLoop(&trgtinfo, &trgt),
		   advanceLoop(&srcinfo, &src) < srcinfo.rndim);
	} else if (src0.d != trgt0.d)
	  memcpy(trgt0.d, src0.d, array_size(iq)*sizeof(type));
	dist.d++;
	break;
    }
    src = trgt = src0 = trgt0;	/* target of last pass becomes source
				   of next one */
  } while (nextLoops(&srcinfo, &trgtinfo));

  free(otmp.b);
  free(work.f);
  free(sines.f);
  free(cosines.f);
  return subroutine? LUX_OK: result;
}
/*------------------------------------------------------------------------- */
int32_t lux_fftshift(int32_t narg, int32_t ps[])
{
  return fftshift(narg, ps, 1);
}
/*------------------------------------------------------------------------- */
int32_t lux_fftshift_f(int32_t narg, int32_t ps[])
{
  return fftshift(narg, ps, 0);
}
/*------------------------------------------------------------------------- */
int32_t lux_power(int32_t narg, int32_t ps[])
/* power function: POWER(data [,axis, /POWER, /SHAPE]) */
{
  int32_t	iq, n, mq, type, j, jq, outDims[MAX_DIMS], result, step;
  pointer	src, trgt, work, tmp, otmp, src0, trgt0;
  scalar	factor;
  int32_t	rfftb(int32_t *, float *, float *), rfftf(int32_t *, float *, float *),
    rfftbd(int32_t *, double *, double *), rfftfd(int32_t *, double *, double *),
    rffti(int32_t *, float *), rfftid(int32_t *, double *);
  loopInfo	srcinfo;

  iq = ps[0];
  if (!symbolIsArray(iq))
    return cerror(NEED_ARR, ps[0]);

  /*float the input */
  if (fftdp == 0) {		/* single precision */
    iq = lux_float(1, ps);
    type = LUX_FLOAT;
  } else {			/* double precision */
    iq = lux_double(1, ps);
    type = LUX_DOUBLE;
  }

  if (narg > 1 && ps[1]) 	/* have <axis> */
    jq = ps[1];
  else				/* take the first axis */
    jq = (internalMode & 4)? SL_TAKEONED: LUX_ZERO;
  
  
  if (standardLoop(iq, jq, SL_EACHROW | SL_ONEAXIS, type,
		   &srcinfo, &src, NULL, NULL, NULL) < 0)
    return LUX_ERROR;
  src0 = src;

  /* get result symbol */
  outDims[0] = srcinfo.rdims[0]/2 + 1;
  memcpy(outDims + 1, srcinfo.rdims + 1, (srcinfo.rndim - 1)*sizeof(int32_t));
  result = array_scratch(type, srcinfo.rndim, outDims);
  trgt0.f = array_data(result);

  work.b = otmp.b = NULL;
  do {
    trgt = trgt0;
    src = src0;
    n = srcinfo.rdims[0];
    mq = n*20 + 120;
    if (!fftdp)
      mq = mq/2;
    work.l = (int32_t *) realloc(work.l, mq);
    if (!work.l) {
      zap(result);
      result = cerror(ALLOC_ERR, 0);
      break;
    }
    if (!fftdp)
      rffti(&n, work.f);
    else
      rfftid(&n, work.d);

    tmp.f = otmp.f = realloc(otmp.b, n*lux_type_size[type]);
    if (tmp.f == NULL) {
      zap(result);
      result = cerror(ALLOC_ERR, 0);
      break;
    }

    step = srcinfo.step[0];
    switch (type) {
      case LUX_FLOAT:
	factor.f = 2.0/n;
	do {
	  for (j = 0; j < n; j++) {
	    *tmp.f++ = *src.f;
	    src.f += step;
	  }
	  tmp.f = otmp.f;
	  rfftf(&n, tmp.f, work.f);
	  *tmp.f *= 0.5*factor.f; /* average */
	  *trgt.f = *tmp.f * *tmp.f;
	  if (internalMode & 3)  /* /POWER */
	    *trgt.f *= 2;
	  if (internalMode & 2)	/* /SHAPE */
	    *trgt.f *= 2;
	  trgt.f++;
	  tmp.f++;
	  for (j = 0; j < n/2 - 1; j++) {
	    *tmp.f *= factor.f;
	    tmp.f[1] *= factor.f;
	    *trgt.f++ = tmp.f[0]*tmp.f[0] + tmp.f[1]*tmp.f[1];
	    tmp.f += 2;
	  }
	  *tmp.f *= 0.5*factor.f; /* highest frequency */
	  *trgt.f = tmp.f[0]*tmp.f[0];
	  if (internalMode & 3)  /* /POWER */
	    *trgt.f *= 2;
	  if (internalMode & 2)	/* /SHAPE */
	    *trgt.f *= 2;
	  trgt.f++;
	  tmp.f = otmp.f;
	} while (advanceLoop(&srcinfo, &src) < srcinfo.rndim);
	break;
      case LUX_DOUBLE:
	factor.d = 2.0/n;
	do {
	  for (j = 0; j < n; j++) {
	    *tmp.d++ = *src.d;
	    src.d += step;
	  }
	  tmp.d = otmp.d;
	  rfftfd(&n, tmp.d, work.d);
	  *tmp.d *= 0.5*factor.d; /* average */
	  *trgt.d = *tmp.d * *tmp.d;
	  if (internalMode & 3)  /* /POWER */
	    *trgt.d *= 2;
	  if (internalMode & 2)	/* /SHAPE */
	    *trgt.d *= 2;
	  trgt.d++;
	  tmp.d++;
	  for (j = 0; j < n/2 - 1; j++) {
	    *tmp.d *= factor.d;
	    tmp.d[1] *= factor.d;
	    *trgt.d++ = tmp.d[0]*tmp.d[0] + tmp.d[1]*tmp.d[1];
	    tmp.d += 2;
	  }
	  *tmp.d *= 0.5*factor.d; /* highest frequency */
	  *trgt.d = tmp.d[0]*tmp.d[0];
	  if (internalMode & 3)  /* /POWER */
	    *trgt.d *= 2;
	  if (internalMode & 2)	/* /SHAPE */
	    *trgt.d *= 2;
	  trgt.d++;
	  tmp.d = otmp.d;
	} while (advanceLoop(&srcinfo, &src) < srcinfo.rndim);
	break;
    }
    src0 = trgt0;
  } while (nextLoop(&srcinfo));

  free(otmp.b);
  free(work.f);
  return result;
}
/*------------------------------------------------------------------------- */
int32_t lux_scb(int32_t narg, int32_t ps[])	/* scb routine */		
/* backwards sine-cosine style FFT --  scb, x, s, c */
{
  int32_t	iq, nd, outer, nx, n, mq, dim[8], type, j, jq, outer2, nx2;
  pointer	q1,q2,q3;
  int32_t	ezffti(int32_t *n, float *wsave), ezfftid(int32_t *n, double *wsave),
    ezfftb(int32_t *n, float *r, float *azero, float *a, float *b, float *wsave),
    ezfftbd(int32_t *n, double *r, double *azero, double *a, double *b,
	    double *wsave);

  iq = ps[1];
  jq = ps[2];
					/* s and c must be arrays */
  if (symbol_class(iq) != LUX_ARRAY
      || symbol_class(jq) != LUX_ARRAY)
    return cerror(NEED_ARR, 0);
  if (!fftdp) {
    iq = lux_float(1, &iq);
    jq = lux_float(1, &jq);
  } else {
    iq = lux_double(1, &iq);
    jq = lux_double(1, &jq);
  }
			/* s and c must have similar structures */
  q2.l = array_data(iq);
  nd = array_num_dims(iq);
  nx = array_dims(iq)[0];
  outer = array_size(iq)/nx;
  memcpy(dim + 1, array_dims(iq) + 1, (nd - 1)*sizeof(int32_t));
  q3.l = array_data(jq);
  nx2 = array_dims(jq)[0];
  outer2 = array_size(jq)/nx2;

  /* the inner dimensions must agree and the product of the outers */
  if (nx != nx2 || outer != outer2)
    return luxerror("SCB - sine and cosine arrays don't match", ps[1]);
  n = (nx - 1)*2;

  /* Because of the way the sine and cosine coefficients are stored by */
  /* lux_sc, the results for an even number of data points have the */
  /* same number of elements as the results for the next higher odd */
  /* number of data points.  If the number of data points is even, then */
  /* the highest sine component is equal to zero.  If the number of */
  /* data points is odd, then the highest sine component is in general */
  /* not equal to zero, but may be equal to zero if the data happen */
  /* to be just right for that.  Because also the user can specify */
  /* arbitrary values for the sine and cosine components, there really */
  /* is no way for us to figure out for sure if any given call to */
  /* lux_scb is supposed to return an even or an odd number of data points. */
  /* We shall adopt the following rules:  if the user specified /ODD or */
  /* /EVEN, then we take that hint.  If the user specified neither /ODD */
  /* nor /EVEN, then we inspect the highest sine component (or components */
  /* if the data have multiple dimensions): if any of the highest sine */
  /* components are unequal to zero, then we assume an odd number of */
  /* data points, otherwise an even number.  LS 16jun98 */

  switch (internalMode & 3) {
    case 0:			/* none: look at highest sine component(s) */
      switch (fftdp) {
	case 0:
	  q1.f = q2.f + (nx - 1);
	  for (j = 0; j < outer; j++) {
	    if (*q1.f) {
	      n++;
	      break;
	    }
	    q1.f += nx;
	  }
	  break;
	case 1:
	  q1.d = q2.d + (nx - 1);
	  for (j = 0; j < outer; j++) {
	    if (*q1.d) {
	      n++;
	      break;
	    }
	    q1.d += nx;
	  }
	  break;
      }
      break;
    case 1:			/* /EVEN */
      break;
    case 2:			/* /ODD */
      n++;
      break;
  }

  mq = array_size(iq)*lux_type_size[iq];
  /* scratch storage is needed, can we use the last one setup ? */
  if (nold != n || fftdp != fftdpold || mq > mqold) {/* set up work space */
    if (nold)
      free(work.b);		/* delete old if any */
    mq = n*28 + 120;
    if (!fftdp)
      mq = mq/2;
    work.l = (int32_t *) malloc(mq);
    if (!fftdp)
      ezffti(&n, work.f);
    else
      ezfftid(&n, work.d);
    nold = n;
    fftdpold = fftdp;
    mqold = mq;
  }

  /* configure the output array */
  dim[0] = n;
  iq = ps[0];
  type = fftdp? LUX_DOUBLE: LUX_FLOAT;
  if (redef_array(iq, type, nd, dim) != 1)
    return LUX_ERROR;
					/* get addresse */
  q1.l = array_data(iq);
  /* loop over the outer dimensions */
  while (outer--) {
    switch (type) {
      case LUX_FLOAT:
	ezfftb(&n, q1.f, q3.f, q3.f + 1, q2.f + 1, work.f);
	q1.f += n;
	q2.f += nx;
	q3.f += nx;
	break;
      case LUX_DOUBLE:
	ezfftbd(&n, q1.d, q3.d, q3.d + 1, q2.d + 1, work.d);
	q1.d += n;
	q2.d += nx;
	q3.d += nx;
	break;
    }
  }
  return 1;
}
/*------------------------------------------------------------------------- */
int32_t lux_histr(int32_t narg, int32_t ps[]) /* histr function */
/* running sum histogram, normalized to 1.0 */
{
  int32_t	iq, n, nd, j, type, size, nRepeat;
  float	sum, fac;
  array	*h;
  pointer q1, q2;
  int32_t	lux_hist(int32_t, int32_t []);

					/* first get a normal histogram */
  if ( (iq = lux_hist(narg,ps) ) <= 0 ) return iq;
/* the returned symbol should be a long array, convert in place to a float,
	this depends on float and long (int32_t) being 32 bits! */
			/* check for some impossible errors */
  if ( sym[iq].class != LUX_ARRAY ) {
    return cerror(IMPOSSIBLE, iq);}
  type = sym[iq].type;
  if (type != LUX_INT32 ) return cerror(IMPOSSIBLE, iq);
  h = (array *) sym[iq].spec.array.ptr;
  q1.l = (int32_t *) ((char *)h + sizeof(array));
  nd = h->ndim;	n = 1;
  for(j=0;j<nd;j++) n *= h->dims[j];
  sym[iq].type = LUX_FLOAT;		/* change to float */
  if (internalMode & 1)		/* along each 0th dimension */
  { size = h->dims[0];  nRepeat = n/size; }
  else
  { size = n;  nRepeat = 1; }
  while (nRepeat--)
  { sum = 0.0;  j = size;  q2.f = q1.f;
    while (j--) { sum += (float) *q2.l; *q2.f++ = sum; }
    fac = 1.0 / sum;  j = size;  q2.f = q1.f;
    while (j--) { *q2.f = fac *  *q2.f; q2.f++; }
    q1.f = q2.f; }
  return iq;
}
/*------------------------------------------------------------------------- */
int32_t findint(int32_t current, int32_t *value, int32_t nValue)
/* finds <current> in list <value> (<nValue> elements) which must be sorted
 in ascending order.  Returns nonnegative index of found <current>, or
 minus the index of the next higher present value, if <current> itself
 is not found. */
{
  int32_t	low, mid, high;
  
  low = 0;
  high = nValue - 1;
  while (low <= high) {
    mid = (low + high)/2;
    if (current < value[mid])
      high = mid - 1;
    else if (current > value[mid])
      low = mid + 1;
    else
      return mid;
  }
  return -low - 1;
}
/*------------------------------------------------------------------------- */
int32_t lux_hist_dense(int32_t narg, int32_t ps[])
/* returns compact histogram for widely spaced data values. */
/* HIST(x,l) returns list of present data values in <l> and corresponding */
/* histogram as return value.  LS 18feb98 */
{
  int32_t	nStore, nValue, nData, *value, *freq, x, y, result;
  char	*avalue, *afreq;
  pointer	src;
  int32_t	findint(int32_t, int32_t *, int32_t);

  if (numerical(ps[0], NULL, NULL, &nData, &src) == LUX_ERROR)
    return LUX_ERROR;
  nStore = 512;
  avalue = (char *) malloc(nStore*sizeof(int32_t) + sizeof(array));
  afreq = (char *) malloc(nStore*sizeof(int32_t) + sizeof(array));
  if (!avalue || !afreq)
    return cerror(ALLOC_ERR, 0);
  value = (int32_t *) (avalue + sizeof(array));
  freq = (int32_t *) (afreq + sizeof(array));

  nValue = 0;
  while (nData--) {
    switch (symbol_type(ps[0])) {
    case LUX_INT8:
      x = (int32_t) *src.b++;
      break;
    case LUX_INT16:
      x = (int32_t) *src.w++;
      break;
    case LUX_INT32:
      x = (int32_t) *src.l++;
      break;
    case LUX_INT64:
      x = *src.q++;
      break;
    case LUX_FLOAT:
      x = (int32_t) *src.f++;
      break;
    case LUX_DOUBLE:
      x = (int32_t) *src.d++;
      break;
    }
    y = findint(x, value, nValue);
    if (y >= 0)			/* already in list */
      freq[y]++;
    else {			/* not yet in list */
      if (++nValue == nStore) { /* need to allocate more room */
	nStore += 512;
	avalue = (char *) realloc(avalue, nStore*sizeof(int32_t) + sizeof(array));
	afreq = (char *) realloc(afreq, nStore*sizeof(int32_t) + sizeof(array));
	if (!avalue || !afreq)
	  return cerror(ALLOC_ERR, 0);
	value = (int32_t *) (avalue + sizeof(array));
	freq = (int32_t *) (afreq + sizeof(array));
      }
      y = -y;
      memmove(value + y, value + y - 1, (nValue - y)*sizeof(int32_t));
      memmove(freq + y, freq + y - 1, (nValue - y)*sizeof(int32_t));
      value[--y] = x;
      freq[y] = 1;
    }
  }
  avalue = (char *) realloc(avalue, nValue*sizeof(int32_t) + sizeof(array));
  afreq = (char *) realloc(afreq, nValue*sizeof(int32_t) + sizeof(array));
  if (!avalue || !afreq)
    return cerror(ALLOC_ERR, 0);

  getFreeTempVariable(result);
  symbol_class(result) = LUX_ARRAY;
  array_type(result) = LUX_INT32;
  array_header(result) = (array *) afreq;
  array_num_dims(result) = 1;
  *array_dims(result) = nValue;
  array_header(result)->c1 = array_header(result)->c2
    = array_header(result)->nfacts = 0;
  array_header(result)->facts = NULL;
  symbol_memory(result) = nValue*sizeof(int32_t) + sizeof(array);

  symbol_class(ps[1]) = LUX_ARRAY;
  array_type(ps[1]) = LUX_INT32;
  array_header(ps[1]) = (array *) avalue;
  array_num_dims(ps[1]) = 1;
  *array_dims(ps[1]) = nValue;
  array_header(ps[1])->c1 = array_header(ps[1])->c2
    = array_header(ps[1])->nfacts = 0;
  array_header(ps[1])->facts = NULL;
  symbol_memory(ps[1]) = symbol_memory(result);

  return result;
}
/*------------------------------------------------------------------------- */
int32_t lux_hist(int32_t narg, int32_t ps[]) /* histogram function */
				 /* (frequency distribution) */
/* general histogram function */
/* keyword /FIRST produces a histogram of all elements along the 0th */
/* dimension for all higher dimensions */
/* LS 12jul2000: added /SILENT keyword (internalMode & 8) to suppress */
/* warnings about negative histogram elements. */
{
  int32_t	iq, i, n, range, type, result_sym, *dims, nRepeat,
  	ndim, axis, one = 1, size;
  array	*h;
  pointer q1, q2;
  int32_t	lux_zero(int32_t, int32_t []);
  void convertWidePointer(wideScalar *, int32_t, int32_t);

  if (narg == 2)
    return lux_hist_dense(narg, ps);
  iq = ps[0];
  switch (symbol_class(iq))
  { case LUX_ARRAY:
      type = array_type(iq);
      q1.l = array_data(iq);
      n = array_size(iq);
      dims = array_dims(iq);
      ndim = array_num_dims(iq);
      break;
    case LUX_SCALAR:
      type = scalar_type(iq);
      q1.l = &scalar_value(iq).l;
      n = 1;
      dims = &one;
      ndim = 1;
      break;
    default:
      return cerror(ILL_CLASS, iq);
    }
  /* always need the range */
  minmax( q1.l, n, type);
  /* get long (int32_t) versions of min and max */
  convertPointer(&lastmin, type, LUX_INT32);
  convertPointer(&lastmax, type, LUX_INT32);
  /* create a long array for results */
  histmin = lastmin.l;
  histmax = lastmax.l;
  /* make the min 0 if it is greater */
  histmin = histmin > 0 ? 0 : histmin;
  if (histmin < 0 && (internalMode & 8) == 0) {
    printf("WARNING - histogram of %s contains negative entries, use\n",
	   symbolIdent(iq, I_TRUNCATE));
    printf("!histmin and !histmax to find range included\n");
  }
  range = histmax + 1;
  if (histmin < 0)
    range -= histmin;
  if (range > maxhistsize) {
    if ((internalMode & 2) == 0)  /* no /IGNORELIMIT */
      printf("range (%d) larger than !maxhistsize (%d)\n",
	     range, maxhistsize);
    if (internalMode & 4) {	/* /INCREASELIMIT */
      if ((internalMode & 2) == 0)
	puts("Increasing !maxhistsize to accomodate");
      maxhistsize = range;
    } else
      if (!(internalMode & 2)) /* no /IGNORELIMIT or /INCREASELIMIT */
	return LUX_ERROR;
  }
  axis = internalMode & 1;
  if (axis)
  { size = *dims;
    nRepeat = n/size;
    one = nRepeat*range;
    result_sym = array_scratch(LUX_INT32, 1, &one);
    h = HEAD(result_sym);
    h->dims[0] = range;
    if (ndim > 1) memcpy(&h->dims[1], &dims[1], sizeof(int32_t)*(ndim - 1));
    h->ndim = ndim; }
  else
  { one = range;
    size = n;
    nRepeat = 1;
    result_sym = array_scratch(2, 1, &one);
    h = HEAD(result_sym); }
  q2.l = (int32_t *) ((char *)h + sizeof(array));
  lux_zero( 1, &result_sym);		/* need to zero initially */
  /* now accumulate the distribution */
  while (nRepeat--)
  { n = size;
    switch (type)
    { case LUX_INT8:
	while (n--)
	{ i = *q1.b++ - histmin;  q2.l[i]++; }  break;
      case LUX_INT16:
	while (n--)
	{ i = *q1.w++ - histmin;  q2.l[i]++; }  break;
      case LUX_INT32:
	while (n--)
	{ i = *q1.l++ - histmin;  q2.l[i]++; }  break;
      case LUX_INT64:
	while (n--)
	{ i = *q1.q++ - histmin;  q2.q[i]++; }  break;
      case LUX_FLOAT:
	while (n--)
	{ i = *q1.f++ - histmin;  q2.l[i]++; }  break;
      case LUX_DOUBLE:
	while (n--)
	{ i = *q1.d++ - histmin;  q2.l[i]++; }  break;
      }
    q2.l += range; }
  return result_sym;
}
/*------------------------------------------------------------------------- */
int32_t lux_sieve(int32_t narg, int32_t ps[])
/* X=SIEVE(array,condition), where condition is normally a logical array
   array and condition must both be arrays and condition must be at least
   as long as array
   the output is a 1-D vector containing all elements of array for which
   the corresponding element in condition is non-zero
   12may92 LS:
   if <condition> does not contain non-zero elements, then a scalar -1 is
   returned.  alternate use: X=SIEVE(condition) yields the same as
   X=SIEVE(LUX_INT32(INDGEN(condition)),condition) */
{
  int32_t	iq, n, nc, cnt, *p, type, typec, result_sym;
  pointer q1, q2, q3;

  iq = ps[0];
  if (iq < 0)
    return iq;			/* pass-thru errors */
  if (symbol_class(iq) == LUX_SCAL_PTR)
    iq = dereferenceScalPointer(iq);
  if (narg == 2) 			/* two-argument case */
					/* first arg must be an array */
  { switch (symbol_class(iq))
    { case LUX_SCAL_PTR:
	iq = dereferenceScalPointer(iq);
      case LUX_SCALAR:
	q1.l = &scalar_value(iq).l;
	n = 1;
	break;
      case LUX_ARRAY:
	q1.l = (int32_t *) array_data(iq);
	n = array_size(iq);
	break;
      default:
	return cerror(NEED_ARR, iq); }
    type = symbol_type(iq);
				/* second argument is the conditional */
    iq = ps[1]; }

  switch (symbol_class(iq))
  { case LUX_SCALAR:
      nc = 1;			/*scalar case */
      p = &scalar_value(iq).l;
      break;
    case LUX_ARRAY:		/*array case */
      p = (int32_t *) array_data(iq);
      nc = array_size(iq);
      break; }
  typec = symbol_type(iq);
  /* unlike the VMS code, we do a double pass, first to get the count */
  /* use the min of size of array and conditional */
  if (narg == 1)
  { n = nc;
    type = LUX_INT32; }
  else
    n = n <= nc ? n : nc;
  nc = n;
  cnt = 0;
  q3.l = p;
  switch (typec)
  { case LUX_INT8:
      while (nc--)
	if (*q3.b++)
	  cnt++;
      break;
    case LUX_INT16:
      while (nc--)
	if (*q3.w++)
	  cnt++;
      break;
    case LUX_INT32:
      while (nc--)
	if (*q3.l++)
	  cnt++;
      break;
    case LUX_INT64:
      while (nc--)
	if (*q3.q++)
	  cnt++;
      break;
    case LUX_FLOAT:
      while (nc--)
	if (*q3.f++)
	  cnt++;
      break;
    case LUX_DOUBLE:
      while (nc--)
	if (*q3.d++)
	  cnt++;
      break; }
  q3.l = p;
 /* if count is zero, then no boolean TRUEs; return a scalar -1.  LS 12may92*/
  if (cnt == 0)
  { result_sym = scalar_scratch(LUX_INT32);
    scalar_value(result_sym).l = -1;
    return result_sym; }
  result_sym = array_scratch(type, 1, &cnt); /*for the result */
  q2.l = array_data(result_sym);
  nc = n;
  if (narg == 1)		/* one-argument case      LS 12may92 */
  { n = 0;			/* use as indgen counter */
    switch (typec)
    { case LUX_INT8:
	while (nc--)
	{ if (*q3.b++)
	    *q2.l++ = n;
	  n++; }
	break;
      case LUX_INT16:
	while (nc--)
	{ if (*q3.w++)
	    *q2.l++ = n;
	  n++; }
	break;
      case LUX_INT32:
	while (nc--)
	{ if (*q3.l++)
	    *q2.l++ = n;
	  n++; }
	break;
      case LUX_INT64:
	while (nc--)
	{ if (*q3.q++)
	    *q2.l++ = n;
	  n++; }
	break;
      case LUX_FLOAT:
	while (nc--)
	{ if (*q3.f++)
	    *q2.l++ = n;
	  n++; }
	break;
      case LUX_DOUBLE:
	while (nc--)
	{ if (*q3.d++)
	    *q2.l++ = n;
	  n++; }
	break; }
  }
  else				/* two-argument case */
    /* this is a 25 case situation */
    switch (typec) {
    case LUX_INT8:
      switch (type)
      { case LUX_INT8:
	  while (nc--)
	  { if (*q3.b++)
	      *q2.b++ = *q1.b;
	    q1.b++; }
	  break;
	case LUX_INT16:
	  while (nc--)
	  { if (*q3.b++)
	      *q2.w++ = *q1.w;
	    q1.w++; }
	  break;
	case LUX_INT32:
	  while (nc--)
	  { if (*q3.b++)
	      *q2.l++ = *q1.l;
	    q1.l++; }
	  break;
	case LUX_INT64:
	  while (nc--)
	  { if (*q3.b++)
	      *q2.q++ = *q1.q;
	    q1.q++; }
	  break;
	case LUX_FLOAT:
	  while (nc--)
	  { if (*q3.b++)
	      *q2.f++ = *q1.f;
	    q1.f++; }
	  break;
	case LUX_DOUBLE:
	  while (nc--)
	  { if (*q3.b++)
	      *q2.d++ = *q1.d;
	    q1.d++; }
	  break; }
      break;
    case LUX_INT16:
      switch (type)
      { case LUX_INT8:
	  while (nc--)
	  { if (*q3.w++)
	      *q2.b++ = *q1.b;
	    q1.b++; }
	  break;
	case LUX_INT16:
	  while (nc--)
	  { if (*q3.w++)
	      *q2.w++ = *q1.w;
	    q1.w++; }
	  break;
	case LUX_INT32:
	  while (nc--)
	  { if (*q3.w++)
	      *q2.l++ = *q1.l;
	    q1.l++; }
	  break;
	case LUX_INT64:
	  while (nc--)
	  { if (*q3.w++)
	      *q2.q++ = *q1.q;
	    q1.q++; }
	  break;
	case LUX_FLOAT:
	  while (nc--)
	  { if (*q3.w++)
	      *q2.f++ = *q1.f;
	    q1.f++; }
	  break;
	case LUX_DOUBLE:
	  while (nc--)
	  { if (*q3.w++)
	      *q2.d++ = *q1.d;
	    q1.d++; }
	  break; }
      break;
    case LUX_INT32:
      switch (type)
      { case LUX_INT8:
	  while (nc--)
	  { if (*q3.l++)
	      *q2.b++ = *q1.b;
	    q1.b++; }
	  break;
	case LUX_INT16:
	  while (nc--)
	  { if (*q3.l++)
	      *q2.w++ = *q1.w;
	    q1.w++; }
	  break;
	case LUX_INT32:
	  while (nc--)
	  { if (*q3.l++)
	      *q2.l++ = *q1.l;
	    q1.l++; }
	  break;
	case LUX_INT64:
	  while (nc--)
	  { if (*q3.l++)
	      *q2.q++ = *q1.q;
	    q1.q++; }
	  break;
	case LUX_FLOAT:
	  while (nc--)
	  { if (*q3.l++)
	      *q2.f++ = *q1.f;
	    q1.f++; }
	  break;
	case LUX_DOUBLE:
	  while (nc--)
	  { if (*q3.l++)
	      *q2.d++ = *q1.d;
	    q1.d++; }
	  break; }
      break;
    case LUX_INT64:
      switch (type)
      { case LUX_INT8:
	  while (nc--)
	  { if (*q3.q++)
	      *q2.b++ = *q1.b;
	    q1.b++; }
	  break;
	case LUX_INT16:
	  while (nc--)
	  { if (*q3.q++)
	      *q2.w++ = *q1.w;
	    q1.w++; }
	  break;
	case LUX_INT32:
	  while (nc--)
	  { if (*q3.q++)
	      *q2.l++ = *q1.l;
	    q1.l++; }
	  break;
	case LUX_INT64:
	  while (nc--)
	  { if (*q3.q++)
	      *q2.q++ = *q1.q;
	    q1.q++; }
	  break;
	case LUX_FLOAT:
	  while (nc--)
	  { if (*q3.q++)
	      *q2.f++ = *q1.f;
	    q1.f++; }
	  break;
	case LUX_DOUBLE:
	  while (nc--)
	  { if (*q3.q++)
	      *q2.d++ = *q1.d;
	    q1.d++; }
	  break; }
      break;
    case LUX_FLOAT:
      switch (type)
      { case LUX_INT8:
	  while (nc--)
	  { if (*q3.f++)
	      *q2.b++ = *q1.b;
	    q1.b++; }
	  break;
	case LUX_INT16:
	  while (nc--)
	  { if (*q3.f++)
	      *q2.w++ = *q1.w;
	    q1.w++; }
	  break;
	case LUX_INT32:
	  while (nc--)
	  { if (*q3.f++)
	      *q2.l++ = *q1.l;
	    q1.l++; }
	  break;
	case LUX_INT64:
	  while (nc--)
	  { if (*q3.f++)
	      *q2.q++ = *q1.q;
	    q1.q++; }
	  break;
	case LUX_FLOAT:
	  while (nc--)
	  { if (*q3.f++)
	      *q2.f++ = *q1.f;
	    q1.f++; }
	  break;
	case LUX_DOUBLE:
	  while (nc--)
	  { if (*q3.f++)
	      *q2.d++ = *q1.d;
	    q1.d++; }
	  break; }
      break;
    case LUX_DOUBLE:
      switch (type)
      { case LUX_INT8:
	  while (nc--)
	  { if (*q3.d++)
	      *q2.b++ = *q1.b;
	    q1.b++; }
	  break;
	case LUX_INT16:
	  while (nc--)
	  { if (*q3.d++)
	      *q2.w++ = *q1.w;
	    q1.w++; }
	  break;
	case LUX_INT32:
	  while (nc--)
	  { if (*q3.d++)
	      *q2.l++ = *q1.l;
	    q1.l++; }
	  break;
	case LUX_INT64:
	  while (nc--)
	  { if (*q3.d++)
	      *q2.q++ = *q1.q;
	    q1.q++; }
	  break;
	case LUX_FLOAT:
	  while (nc--)
	  { if (*q3.d++)
	      *q2.f++ = *q1.f;
	    q1.f++; }
	  break;
	case LUX_DOUBLE:
	  while (nc--)
	  { if (*q3.d++)
	      *q2.d++ = *q1.d;
	    q1.d++; }
	  break; }
      break; }
  return result_sym;
}
/*------------------------------------------------------------------------- */
int32_t lux_maxf(int32_t narg, int32_t ps[])
				/* finds the max element in an array */
{
  int32_t	maxormin(int32_t, int32_t [], int32_t);

  return maxormin(narg, ps, 0);
}
/*------------------------------------------------------------------------- */
int32_t index_maxormin(int32_t source, int32_t indices, int32_t code)
{
  int32_t	nIndx, type, offset, *indx, i, result, size, nElem, indices2;
  char	*any;
  pointer	src, trgt;
  extern scalar	lastmin, lastmax;

  src.l = array_data(source);
  nElem = array_size(source);
  type = array_type(source);
  indices2 = lux_long(1, &indices);
  indx = array_data(indices2);
  nIndx = array_size(indices2);
  if (nIndx != nElem)
    return cerror(INCMP_ARR, indices);
  /* find min and max of indices so we can create a result array of the */
  /* correct size */
  minmax(indx, nIndx, LUX_INT32);
  /* use offset so we can treat negative indices */
  size = lastmax.l + 1;
  offset = 0;
  if (lastmin.l < 0)
    size += (offset = -lastmin.l);
  result = array_scratch(code < 2? type: LUX_INT32, 1, &size);
  /* make trgt.b point to the element associated with index zero */
  trgt.b = (uint8_t *) array_data(result) + offset*lux_type_size[array_type(result)];
  /* Initialize result to zero */
  zerobytes(array_data(result), size*lux_type_size[array_type(result)]);
  /* use "any" as an array of flags indicating if any number has been */
  /* put in the corresponding result element so far */
  allocate(any, size, char);
  zerobytes(any, size);		/* initialize to empty */
  any += offset;		/* in case of negative indices */
  switch (type) {
    case LUX_INT8:
      for (i = 0; i < nIndx; i++)
	switch (code) {
	  case 0: /* maxf */
	    if (any[*indx]) {
	      if (src.b[i] > trgt.b[*indx])
		trgt.b[*indx] = src.b[i];
	    } else {
	      trgt.b[*indx] = src.b[i];
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	  case 1: /* minf */
	    if (any[*indx]) {
	      if (src.b[i] < trgt.b[*indx])
		trgt.b[*indx] = src.b[i];
	    } else {
	      trgt.b[*indx] = src.b[i];
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	  case 2: /* maxloc */
	    if (any[*indx]) {
	      if (src.b[i] > src.b[trgt.l[*indx]])
		trgt.l[*indx] = i;
	    } else {
	      trgt.l[*indx] = i;
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	  case 3: /* minloc */
	    if (any[*indx]) {
	      if (src.b[i] < src.b[trgt.l[*indx]])
		trgt.l[*indx] = i;
	    } else {
	      trgt.l[*indx] = i;
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	}
      break;
    case LUX_INT16:
      for (i = 0; i < nIndx; i++)
	switch (code) {
	  case 0: /* maxf */
	    if (any[*indx]) {
	      if (src.w[i] > trgt.w[*indx])
		trgt.w[*indx] = src.w[i];
	    } else {
	      trgt.w[*indx] = src.w[i];
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	  case 1: /* minf */
	    if (any[*indx]) {
	      if (src.w[i] < trgt.w[*indx])
		trgt.w[*indx] = src.w[i];
	    } else {
	      trgt.w[*indx] = src.w[i];
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	  case 2: /* maxloc */
	    if (any[*indx]) {
	      if (src.w[i] > src.w[trgt.l[*indx]])
		trgt.l[*indx] = i;
	    } else {
	      trgt.l[*indx] = i;
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	  case 3: /* minloc */
	    if (any[*indx]) {
	      if (src.w[i] < src.w[trgt.l[*indx]])
		trgt.l[*indx] = i;
	    } else {
	      trgt.l[*indx] = i;
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	}
      break;
    case LUX_INT32:
      for (i = 0; i < nIndx; i++)
	switch (code) {
	  case 0: /* maxf */
	    if (any[*indx]) {
	      if (src.l[i] > trgt.l[*indx])
		trgt.l[*indx] = src.l[i];
	    } else {
	      trgt.l[*indx] = src.l[i];
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	  case 1: /* minf */
	    if (any[*indx]) {
	      if (src.l[i] < trgt.l[*indx])
		trgt.l[*indx] = src.l[i];
	    } else {
	      trgt.l[*indx] = src.l[i];
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	  case 2: /* maxloc */
	    if (any[*indx]) {
	      if (src.l[i] > src.l[trgt.l[*indx]])
		trgt.l[*indx] = i;
	    } else {
	      trgt.l[*indx] = i;
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	  case 3: /* minloc */
	    if (any[*indx]) {
	      if (src.l[i] < src.l[trgt.l[*indx]])
		trgt.l[*indx] = i;
	    } else {
	      trgt.l[*indx] = i;
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	}
      break;
    case LUX_INT64:
      for (i = 0; i < nIndx; i++)
	switch (code) {
	  case 0: /* maxf */
	    if (any[*indx]) {
	      if (src.q[i] > trgt.q[*indx])
		trgt.q[*indx] = src.q[i];
	    } else {
	      trgt.q[*indx] = src.q[i];
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	  case 1: /* minf */
	    if (any[*indx]) {
	      if (src.q[i] < trgt.q[*indx])
		trgt.q[*indx] = src.q[i];
	    } else {
	      trgt.q[*indx] = src.q[i];
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	  case 2: /* maxloc */
	    if (any[*indx]) {
	      if (src.q[i] > src.q[trgt.l[*indx]])
		trgt.l[*indx] = i;
	    } else {
	      trgt.l[*indx] = i;
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	  case 3: /* minloc */
	    if (any[*indx]) {
	      if (src.q[i] < src.q[trgt.l[*indx]])
		trgt.l[*indx] = i;
	    } else {
	      trgt.l[*indx] = i;
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	}
      break;
    case LUX_FLOAT:
      for (i = 0; i < nIndx; i++)
	switch (code) {
	  case 0: /* maxf */
	    if (any[*indx]) {
	      if (src.f[i] > trgt.f[*indx])
		trgt.f[*indx] = src.f[i];
	    } else {
	      trgt.f[*indx] = src.f[i];
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	  case 1: /* minf */
	    if (any[*indx]) {
	      if (src.f[i] < trgt.f[*indx])
		trgt.f[*indx] = src.f[i];
	    } else {
	      trgt.f[*indx] = src.f[i];
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	  case 2: /* maxloc */
	    if (any[*indx]) {
	      if (src.f[i] > src.f[trgt.l[*indx]])
		trgt.l[*indx] = i;
	    } else {
	      trgt.l[*indx] = i;
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	  case 3: /* minloc */
	    if (any[*indx]) {
	      if (src.f[i] < src.f[trgt.l[*indx]])
		trgt.l[*indx] = i;
	    } else {
	      trgt.l[*indx] = i;
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	}
      break;
    case LUX_DOUBLE:
      for (i = 0; i < nIndx; i++)
	switch (code) {
	  case 0: /* maxf */
	    if (any[*indx]) {
	      if (src.d[i] > trgt.d[*indx])
		trgt.d[*indx] = src.d[i];
	    } else {
	      trgt.d[*indx] = src.d[i];
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	  case 1: /* minf */
	    if (any[*indx]) {
	      if (src.d[i] < trgt.d[*indx])
		trgt.d[*indx] = src.d[i];
	    } else {
	      trgt.d[*indx] = src.d[i];
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	  case 2: /* maxloc */
	    if (any[*indx]) {
	      if (src.d[i] > src.d[trgt.l[*indx]])
		trgt.l[*indx] = i;
	    } else {
	      trgt.l[*indx] = i;
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	  case 3: /* minloc */
	    if (any[*indx]) {
	      if (src.d[i] < src.d[trgt.l[*indx]])
		trgt.l[*indx] = i;
	    } else {
	      trgt.l[*indx] = i;
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	}
      break;
    case LUX_CFLOAT:
      for (i = 0; i < nIndx; i++)
	switch (code) {
	  case 0: /* maxf */
	    if (any[*indx]) {
	      if (complexMag2(src.cf[i]) > complexMag2(trgt.cf[*indx]))
		trgt.cf[*indx] = src.cf[i];
	    } else {
	      trgt.cf[*indx] = src.cf[i];
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	  case 1: /* minf */
	    if (any[*indx]) {
	      if (complexMag2(src.cf[i]) < complexMag2(trgt.cf[*indx]))
		trgt.cf[*indx] = src.cf[i];
	    } else {
	      trgt.cf[*indx] = src.cf[i];
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	  case 2: /* maxloc */
	    if (any[*indx]) {
	      if (complexMag2(src.cf[i]) > complexMag2(src.cf[trgt.l[*indx]]))
		trgt.l[*indx] = i;
	    } else {
	      trgt.l[*indx] = i;
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	  case 3: /* minloc */
	    if (any[*indx]) {
	      if (complexMag2(src.cf[i]) < complexMag2(src.cf[trgt.l[*indx]]))
		trgt.l[*indx] = i;
	    } else {
	      trgt.l[*indx] = i;
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	}
      break;
    case LUX_CDOUBLE:
      for (i = 0; i < nIndx; i++)
	switch (code) {
	  case 0: /* maxf */
	    if (any[*indx]) {
	      if (complexMag2(src.cd[i]) > complexMag2(trgt.cd[*indx]))
		trgt.cd[*indx] = src.cd[i];
	    } else {
	      trgt.cd[*indx] = src.cd[i];
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	  case 1: /* minf */
	    if (any[*indx]) {
	      if (complexMag2(src.cd[i]) < complexMag2(trgt.cd[*indx]))
		trgt.cd[*indx] = src.cd[i];
	    } else {
	      trgt.cd[*indx] = src.cd[i];
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	  case 2: /* maxloc */
	    if (any[*indx]) {
	      if (complexMag2(src.cd[i]) > complexMag2(src.cd[trgt.l[*indx]]))
		trgt.l[*indx] = i;
	    } else {
	      trgt.l[*indx] = i;
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	  case 3: /* minloc */
	    if (any[*indx]) {
	      if (complexMag2(src.cd[i]) < complexMag2(src.cd[trgt.l[*indx]]))
		trgt.l[*indx] = i;
	    } else {
	      trgt.l[*indx] = i;
	      any[*indx] = 1;
	    }
	    indx++;
	    break;
	}
      break;
    }
  free(any - offset);
  return result;
}
/*------------------------------------------------------------------------- */
int32_t maxormin(int32_t narg, int32_t ps[], int32_t code)
/* finds extreme values.  arguments:  <data> [, <axes>] [, /KEEPDIMS] */
/* <code>: 0 -> max value; 1 -> min value; 2 -> max location;
   3 -> min location */
{
  int32_t	mode, minloc, maxloc, n, result, n1;
  loopInfo	srcinfo, trgtinfo;
  pointer	src, trgt;
  scalar	min, max;
  double	value;
  extern scalar	lastmin, lastmax;
  extern int32_t	lastminloc, lastmaxloc;
  extern int32_t	lastmin_sym, lastmax_sym;

  if (narg == 2
      && symbolIsNumericalArray(ps[0])
      && symbolIsNumericalArray(ps[1])
      && array_size(ps[0]) == array_size(ps[1]))
    return index_maxormin(ps[0], ps[1], code);

  mode = SL_UNIQUEAXES | SL_COMPRESSALL | SL_AXESBLOCK;
  if (code & 2)			/* seek location */
    mode |= SL_EXACT;		/* output must be exactly LUX_INT32 */
  else
    mode |= SL_KEEPTYPE;	/* output must be same type as input */
  if (internalMode & 1)		/* /KEEPDIMS */
    mode |= SL_ONEDIMS;

  if (standardLoop(ps[0], narg > 1? ps[1]: 0, mode, LUX_INT32, &srcinfo,
		   &src, &result, &trgtinfo, &trgt) == LUX_ERROR)
    return LUX_ERROR;

  n1 = srcinfo.naxes? srcinfo.naxes: 1;

  switch (srcinfo.type) {
    case LUX_INT8:
      do {
	/* take first value as initial values */
	memcpy(&min, src.b, srcinfo.stride);
	memcpy(&max, src.b, srcinfo.stride);
	minloc = maxloc = src.b - (uint8_t *) srcinfo.data0;
	do {
	  if (*src.b > max.b) {
	    max.b = *src.b;
	    maxloc = src.b - (uint8_t *) srcinfo.data0;
	  } else if (*src.b < min.b) {
	    min.b = *src.b;
	    minloc = src.b - (uint8_t *) srcinfo.data0;
	  }
	} while ((n = advanceLoop(&srcinfo, &src)) < n1);
	switch (code) {
	  case 0:		/* max value */
	    *trgt.b++ = max.b;
	    break;
	  case 1:		/* min value */
	    *trgt.b++ = min.b;
	    break;
	  case 2:		/* max location */
	    *trgt.l++ = maxloc;
	    break;
	  case 3:		/* min location */
	    *trgt.l++ = minloc;
	    break;
	}
      } while (n < srcinfo.rndim);
      break;
    case LUX_INT16:
      do {
	/* take first value as initial values */
	memcpy(&min, src.w, srcinfo.stride);
	memcpy(&max, src.w, srcinfo.stride);
	minloc = maxloc = src.w - (int16_t *) srcinfo.data0;
	do {
	  if (*src.w > max.w) {
	    max.w = *src.w;
	    maxloc = src.w - (int16_t *) srcinfo.data0;
	  } else if (*src.w < min.w) {
	    min.w = *src.w;
	    minloc = src.w - (int16_t *) srcinfo.data0;
	  }
	} while ((n = advanceLoop(&srcinfo, &src)) < n1);
	switch (code) {
	  case 0:		/* max value */
	    *trgt.w++ = max.w;
	    break;
	  case 1:		/* min value */
	    *trgt.w++ = min.w;
	    break;
	  case 2:		/* max location */
	    *trgt.l++ = maxloc;
	    break;
	  case 3:		/* min location */
	    *trgt.l++ = minloc;
	    break;
	}
      } while (n < srcinfo.rndim);
      break;
    case LUX_INT32:
      do {
	/* take first value as initial values */
	memcpy(&min, src.l, srcinfo.stride);
	memcpy(&max, src.l, srcinfo.stride);
	minloc = maxloc = src.l - (int32_t *) srcinfo.data0;
	do {
	  if (*src.l > max.l) {
	    max.l = *src.l;
	    maxloc = src.l - (int32_t *) srcinfo.data0;
	  } else if (*src.l < min.l) {
	    min.l = *src.l;
	    minloc = src.l - (int32_t *) srcinfo.data0;
	  }
	} while ((n = advanceLoop(&srcinfo, &src)) < n1);
	switch (code) {
	  case 0:		/* max value */
	    *trgt.l++ = max.l;
	    break;
	  case 1:		/* min value */
	    *trgt.l++ = min.l;
	    break;
	  case 2:		/* max location */
	    *trgt.l++ = maxloc;
	    break;
	  case 3:		/* min location */
	    *trgt.l++ = minloc;
	    break;
	}
      } while (n < srcinfo.rndim);
      break;
    case LUX_INT64:
      do {
	/* take first value as initial values */
	memcpy(&min, src.q, srcinfo.stride);
	memcpy(&max, src.q, srcinfo.stride);
	minloc = maxloc = src.q - (int64_t *) srcinfo.data0;
	do {
	  if (*src.q > max.q) {
	    max.q = *src.q;
	    maxloc = src.q - (int64_t *) srcinfo.data0;
	  } else if (*src.q < min.q) {
	    min.q = *src.q;
	    minloc = src.q - (int64_t *) srcinfo.data0;
	  }
	} while ((n = advanceLoop(&srcinfo, &src)) < n1);
	switch (code) {
	  case 0:		/* max value */
	    *trgt.q++ = max.q;
	    break;
	  case 1:		/* min value */
	    *trgt.q++ = min.q;
	    break;
	  case 2:		/* max location */
	    *trgt.l++ = maxloc;
	    break;
	  case 3:		/* min location */
	    *trgt.l++ = minloc;
	    break;
	}
      } while (n < srcinfo.rndim);
      break;
    case LUX_FLOAT:
      do {
	/* take first value as initial values */
	memcpy(&min, src.f, srcinfo.stride);
	memcpy(&max, src.f, srcinfo.stride);
	minloc = maxloc = src.f - (float *) srcinfo.data0;
	do {
	  if (*src.f > max.f) {
	    max.f = *src.f;
	    maxloc = src.f - (float *) srcinfo.data0;
	  } else if (*src.f < min.f) {
	    min.f = *src.f;
	    minloc = src.f - (float *) srcinfo.data0;
	  }
	} while ((n = advanceLoop(&srcinfo, &src)) < n1);
	switch (code) {
	  case 0:		/* max value */
	    *trgt.f++ = max.f;
	    break;
	  case 1:		/* min value */
	    *trgt.f++ = min.f;
	    break;
	  case 2:		/* max location */
	    *trgt.l++ = maxloc;
	    break;
	  case 3:		/* min location */
	    *trgt.l++ = minloc;
	    break;
	}
      } while (n < srcinfo.rndim);
      break;
    case LUX_DOUBLE:
      do {
	/* take first value as initial values */
	memcpy(&min, src.d, srcinfo.stride);
	memcpy(&max, src.d, srcinfo.stride);
	minloc = maxloc = src.d - (double *) srcinfo.data0;
	do {
	  if (*src.d > max.d) {
	    max.d = *src.d;
	    maxloc = src.d - (double *) srcinfo.data0;
	  } else if (*src.d < min.d) {
	    min.d = *src.d;
	    minloc = src.d - (double *) srcinfo.data0;
	  }
	} while ((n = advanceLoop(&srcinfo, &src)) < n1);
	switch (code) {
	  case 0:		/* max value */
	    *trgt.d++ = max.d;
	    break;
	  case 1:		/* min value */
	    *trgt.d++ = min.d;
	    break;
	  case 2:		/* max location */
	    *trgt.l++ = maxloc;
	    break;
	  case 3:		/* min location */
	    *trgt.l++ = minloc;
	    break;
	}
      } while (n < srcinfo.rndim);
      break;
    case LUX_CFLOAT:
      /* we compare based on the absolute value */
      do {
	/* take first value as initial values */
	max.f = min.f = src.cf->real*src.cf->real
	  + src.cf->imaginary*src.cf->imaginary;
	minloc = maxloc = src.cf - (floatComplex *) srcinfo.data0;
	do {
	  value = src.cf->real*src.cf->real
	    + src.cf->imaginary*src.cf->imaginary;
	  if (value > max.f) {
	    max.f = value;
	    maxloc = src.cf - (floatComplex *) srcinfo.data0;
	  } else if (value < min.d) {
	    min.f = value;
	    minloc = src.cf - (floatComplex *) srcinfo.data0;
	  }
	} while ((n = advanceLoop(&srcinfo, &src)) < n1);
	switch (code) {
	  case 0:		/* max value */
	    *trgt.cf++ = ((floatComplex *) srcinfo.data0)[maxloc];
	    break;
	  case 1:		/* min value */
	    *trgt.cf++ = ((floatComplex *) srcinfo.data0)[minloc];
	    break;
	  case 2:		/* max location */
	    *trgt.l++ = maxloc;
	    break;
	  case 3:		/* min location */
	    *trgt.l++ = minloc;
	    break;
	}
      } while (n < srcinfo.rndim);
      max.f = sqrt(max.f);
      min.f = sqrt(min.f);
      break;
    case LUX_CDOUBLE:
      /* we compare based on the absolute value */
      do {
	/* take first value as initial values */
	max.d = min.d = src.cd->real*src.cd->real
	  + src.cd->imaginary*src.cd->imaginary;
	minloc = maxloc = src.cd - (doubleComplex *) srcinfo.data0;
	do {
	  value = src.cd->real*src.cd->real
	    + src.cd->imaginary*src.cd->imaginary;
	  if (value > max.d) {
	    max.d = value;
	    maxloc = src.cd - (doubleComplex *) srcinfo.data0;
	  } else if (value < min.d) {
	    min.d = value;
	    minloc = src.cd - (doubleComplex *) srcinfo.data0;
	  }
	} while ((n = advanceLoop(&srcinfo, &src)) < n1);
	switch (code) {
	  case 0:		/* max value */
	    *trgt.cd++ = ((doubleComplex *) srcinfo.data0)[maxloc];
	    break;
	  case 1:		/* min value */
	    *trgt.cd++ = ((doubleComplex *) srcinfo.data0)[minloc];
	    break;
	  case 2:		/* max location */
	    *trgt.l++ = maxloc;
	    break;
	  case 3:		/* min location */
	    *trgt.l++ = minloc;
	    break;
	}
      } while (n < srcinfo.rndim);
      max.d = sqrt(max.d);
      min.d = sqrt(min.d);
      break;
  default:
    break;
  }

  /* put results in global symbols */
  lastmin = min;
  lastmax = max;
  symbol_type(lastmin_sym) = symbol_type(lastmax_sym) = realType(srcinfo.type);
  lastmaxloc = maxloc;
  lastminloc = minloc;
  return result;
}
/*------------------------------------------------------------------------- */
int32_t lux_minf(int32_t narg, int32_t ps[])
				/* finds the min element in an array */
{
  return maxormin(narg, ps, 1);
}
/*------------------------------------------------------------------------- */
int32_t lux_minloc(narg,ps)
				/* finds the min element in an array */
int32_t	narg, ps[];
{
return maxormin(narg, ps, 3);
}
/*------------------------------------------------------------------------- */
int32_t lux_maxloc(narg,ps)
				/* finds the max element in an array */
int32_t	narg, ps[];
{
return maxormin(narg, ps, 2);
}
/*------------------------------------------------------------------------- */
void scale(pointer data, uint8_t type, int32_t size, double datalow, double datahigh,
	   void *dest, double trgtlow, double trgthigh)
/* returns a copy of the data at <data>, linearly transformed such that
 <datalow> maps to <trgtlow> and <datahigh> to <trgthigh>.  Data that falls
 outside of this range is mapped to the nearest valid result. 
 LS 20sep98 */
/* <trgt> is assumed to point to data of type <colorIndexType>.  LS 23mar99 */
{
  double	drange, dfac, doff;
  float	ffac, foff;
  pointer	trgt;
#if HAVE_LIBX11
  extern int32_t	colorIndexType;
#else
  int32_t	colorIndexType = LUX_INT8;
#endif

  trgt.b = dest;
  drange = datahigh - datalow;
  if (!drange) {
    neutral(trgt.b, size);
    return;
  }
  dfac = (trgthigh - trgtlow)/drange;

  switch (type) {
    case LUX_INT8:
      ffac = (float) dfac;
      foff = trgtlow - datalow*ffac;
      switch (colorIndexType) {
	case LUX_INT8:
	  while (size--) {
	    if (*data.b < datalow)
	      *trgt.b++ = trgtlow;
	    else if (*data.b > datahigh)
	      *trgt.b++ = trgthigh;
	    else
	      *trgt.b++ = *data.b*ffac + foff;
	    data.b++;
	  }
	  break;
	case LUX_INT16:
	  while (size--) {
	    if (*data.b < datalow)
	      *trgt.w++ = trgtlow;
	    else if (*data.b > datahigh)
	      *trgt.w++ = trgthigh;
	    else
	      *trgt.w++ = *data.b*ffac + foff;
	    data.b++;
	  }
	  break;
	case LUX_INT32:
	  while (size--) {
	    if (*data.b < datalow)
	      *trgt.l++ = trgtlow;
	    else if (*data.b > datahigh)
	      *trgt.l++ = trgthigh;
	    else
	      *trgt.l++ = *data.b*ffac + foff;
	    data.b++;
	  }
	  break;
	case LUX_INT64:
	  while (size--) {
	    if (*data.b < datalow)
	      *trgt.q++ = trgtlow;
	    else if (*data.b > datahigh)
	      *trgt.q++ = trgthigh;
	    else
	      *trgt.q++ = *data.b*ffac + foff;
	    data.b++;
	  }
	  break;
      }
      break;
    case LUX_INT16:
      ffac = (float) dfac;
      foff = trgtlow - datalow*ffac;
      switch (colorIndexType) {
	case LUX_INT8:
	  while (size--) {
	    if (*data.w < datalow)
	      *trgt.b++ = trgtlow;
	    else if (*data.w > datahigh)
	      *trgt.b++ = trgthigh;
	    else
	      *trgt.b++ = *data.w*ffac + foff;
	    data.w++;
	  }
	  break;
	case LUX_INT16:
	  while (size--) {
	    if (*data.w < datalow)
	      *trgt.w++ = trgtlow;
	    else if (*data.w > datahigh)
	      *trgt.w++ = trgthigh;
	    else
	      *trgt.w++ = *data.w*ffac + foff;
	    data.w++;
	  }
	  break;
	case LUX_INT32:
	  while (size--) {
	    if (*data.w < datalow)
	      *trgt.l++ = trgtlow;
	    else if (*data.w > datahigh)
	      *trgt.l++ = trgthigh;
	    else
	      *trgt.l++ = *data.w*ffac + foff;
	    data.w++;
	  }
	  break;
	case LUX_INT64:
	  while (size--) {
	    if (*data.w < datalow)
	      *trgt.q++ = trgtlow;
	    else if (*data.w > datahigh)
	      *trgt.q++ = trgthigh;
	    else
	      *trgt.q++ = *data.w*ffac + foff;
	    data.w++;
	  }
	  break;
      }
      break;
    case LUX_INT32:
      ffac = (float) dfac;
      foff = trgtlow - datalow*ffac;
      switch (colorIndexType) {
	case LUX_INT8:
	  while (size--) {
	    if (*data.l < datalow)
	      *trgt.b++ = trgtlow;
	    else if (*data.l > datahigh)
	      *trgt.b++ = trgthigh;
	    else
	      *trgt.b++ = *data.l*ffac + foff;
	    data.l++;
	  }
	  break;
	case LUX_INT16:
	  while (size--) {
	    if (*data.l < datalow)
	      *trgt.w++ = trgtlow;
	    else if (*data.l > datahigh)
	      *trgt.w++ = trgthigh;
	    else
	      *trgt.w++ = *data.l*ffac + foff;
	    data.l++;
	  }
	  break;
	case LUX_INT32:
	  while (size--) {
	    if (*data.l < datalow)
	      *trgt.l++ = trgtlow;
	    else if (*data.l > datahigh)
	      *trgt.l++ = trgthigh;
	    else
	      *trgt.l++ = *data.l*ffac + foff;
	    data.l++;
	  }
	  break;
	case LUX_INT64:
	  while (size--) {
	    if (*data.l < datalow)
	      *trgt.q++ = trgtlow;
	    else if (*data.l > datahigh)
	      *trgt.q++ = trgthigh;
	    else
	      *trgt.q++ = *data.l*ffac + foff;
	    data.l++;
	  }
	  break;
      }
      break;
    case LUX_INT64:
      ffac = (float) dfac;
      foff = trgtlow - datalow*ffac;
      switch (colorIndexType) {
	case LUX_INT8:
	  while (size--) {
	    if (*data.q < datalow)
	      *trgt.b++ = trgtlow;
	    else if (*data.q > datahigh)
	      *trgt.b++ = trgthigh;
	    else
	      *trgt.b++ = *data.q*ffac + foff;
	    data.q++;
	  }
	  break;
	case LUX_INT16:
	  while (size--) {
	    if (*data.q < datalow)
	      *trgt.w++ = trgtlow;
	    else if (*data.q > datahigh)
	      *trgt.w++ = trgthigh;
	    else
	      *trgt.w++ = *data.q*ffac + foff;
	    data.q++;
	  }
	  break;
	case LUX_INT32:
	  while (size--) {
	    if (*data.q < datalow)
	      *trgt.l++ = trgtlow;
	    else if (*data.q > datahigh)
	      *trgt.l++ = trgthigh;
	    else
	      *trgt.l++ = *data.q*ffac + foff;
	    data.q++;
	  }
	  break;
	case LUX_INT64:
	  while (size--) {
	    if (*data.q < datalow)
	      *trgt.q++ = trgtlow;
	    else if (*data.q > datahigh)
	      *trgt.q++ = trgthigh;
	    else
	      *trgt.q++ = *data.q*ffac + foff;
	    data.q++;
	  }
	  break;
      }
      break;
    case LUX_FLOAT:
      ffac = (float) dfac;
      foff = trgtlow - datalow*ffac;
      switch (colorIndexType) {
	case LUX_INT8:
	  while (size--) {
	    if (*data.f < datalow)
	      *trgt.b++ = trgtlow;
	    else if (*data.f > datahigh)
	      *trgt.b++ = trgthigh;
	    else
	      *trgt.b++ = *data.f*ffac + foff;
	    data.f++;
	  }
	  break;
	case LUX_INT16:
	  while (size--) {
	    if (*data.f < datalow)
	      *trgt.w++ = trgtlow;
	    else if (*data.f > datahigh)
	      *trgt.w++ = trgthigh;
	    else
	      *trgt.w++ = *data.f*ffac + foff;
	    data.f++;
	  }
	  break;
	case LUX_INT32:
	  while (size--) {
	    if (*data.f < datalow)
	      *trgt.l++ = trgtlow;
	    else if (*data.f > datahigh)
	      *trgt.l++ = trgthigh;
	    else
	      *trgt.l++ = *data.f*ffac + foff;
	    data.f++;
	  }
	  break;
	case LUX_INT64:
	  while (size--) {
	    if (*data.f < datalow)
	      *trgt.q++ = trgtlow;
	    else if (*data.f > datahigh)
	      *trgt.q++ = trgthigh;
	    else
	      *trgt.q++ = *data.f*ffac + foff;
	    data.f++;
	  }
	  break;
      }
      break;
    case LUX_DOUBLE:
      doff = trgtlow - datalow*dfac;
      switch (colorIndexType) {
	case LUX_INT8:
	  while (size--) {
	    if (*data.d < datalow)
	      *trgt.b++ = trgtlow;
	    else if (*data.d > datahigh)
	      *trgt.b++ = trgthigh;
	    else
	      *trgt.b++ = *data.d*dfac + doff;
	    data.d++;
	  }
	  break;
	case LUX_INT16:
	  while (size--) {
	    if (*data.d < datalow)
	      *trgt.w++ = trgtlow;
	    else if (*data.d > datahigh)
	      *trgt.w++ = trgthigh;
	    else
	      *trgt.w++ = *data.d*dfac + doff;
	    data.d++;
	  }
	  break;
	case LUX_INT32:
	  while (size--) {
	    if (*data.d < datalow)
	      *trgt.l++ = trgtlow;
	    else if (*data.d > datahigh)
	      *trgt.l++ = trgthigh;
	    else
	      *trgt.l++ = *data.d*dfac + doff;
	    data.d++;
	  }
	  break;
	case LUX_INT64:
	  while (size--) {
	    if (*data.d < datalow)
	      *trgt.q++ = trgtlow;
	    else if (*data.d > datahigh)
	      *trgt.q++ = trgthigh;
	    else
	      *trgt.q++ = *data.d*dfac + doff;
	    data.d++;
	  }
	  break;
      }
      break;
  }
}
/*------------------------------------------------------------------------- */
int32_t lux_scale(int32_t narg, int32_t ps[])
/* scale an array to the range (!scalemin,!scalemax) and put into an array */
/* of the smallest data type that can fit any allowed color cell index  */
/* add keyword /FULLRANGE: scale between 0 and the greatest color */
/* cell index.  LS 30jul96 23mar99 */
{
  int32_t	iq, type, result_sym, n, oldScalemin, oldScalemax;
  scalar	min, max;
  register	pointer q1, q2;
  double	sd, qd;
#if HAVE_LIBX11
  extern double	zoom_clo, zoom_chi;
  extern int32_t	threeColors;
  extern int32_t	colorIndexType, display_cells;
#else
  int32_t	colorIndexType = LUX_INT8, display_cells = 256;
#endif

  iq = ps[0];
  if (symbol_class(iq) != LUX_ARRAY)
    return cerror(NEED_ARR, iq);
  type = array_type(iq);
  q1.l = array_data(iq);
  n = array_size(iq);
  result_sym = array_clone(iq, LUX_INT8);
  q2.l = array_data(result_sym);
  /* if only one arg., then we scale between the min and max of array */
  if (narg == 1 && (internalMode & 2) == 0) {/* 1 arg and no /ZOOM */
    minmax(q1.l, n, type);	/* get the min and max */
    oldScalemin = scalemin;
    oldScalemax = scalemax;
    if (internalMode & 1) {	/* /FULLRANGE */
      scalemin = 0;
      scalemax = display_cells - 1;
    }
#if HAVE_LIBX11
    else if (threeColors) {
      scalemin = 0;
      scalemax = display_cells/3 - 1;
    }
#endif
    simple_scale(q1.l, n, type, q2.b); /* scale and put in output array */
    scalemin = oldScalemin;
    scalemax = oldScalemax;
    return result_sym;		/* done for this case */
  } else {
    /* more than one arg., so we expect the min and max  to be specified */
    if (narg == 2)
      return cerror(WRNG_N_ARG, 0);
    /* this is messier than the simple scale because of over/under flows */
#if HAVE_LIBX11
    if (narg == 1) {		/* have /ZOOM */
      min.d = zoom_clo;
      max.d = zoom_chi;
    } else
#endif
      if (type == LUX_DOUBLE) { /* array is double, get input as double */
	min.d = double_arg(ps[1]);
	max.d = double_arg(ps[2]);
      } else {
	min.d = float_arg(ps[1]);
	max.d = float_arg(ps[2]);
      }
    if (max.d == min.d) {
      neutral(q2.b, n);
      return result_sym;
    }
    if (internalMode & 1) {	/* /BYTE */
      sd = (double) 255.999;
      qd = (double) 0.0;
    } else
#if HAVE_LIBX11
      if (threeColors) {
	sd = (double) 84.999;
	qd = (double) 0.0;
      } else
#endif
      {
	sd = (double) scalemax + 0.999;
	qd = (double) scalemin;
      }
    scale(q1, type, n, min.d, max.d, q2.b, qd, sd);
  }
  return result_sym;
}
/*------------------------------------------------------------------------- */
int32_t lux_scalerange(int32_t narg, int32_t ps[])
/* scale an array and put in BYTE result 
   SCALERANGE(<data>, <lowgrey>, <higrey> [, <lowvalue>, <hivalue>, /BYTE,
              /ZOOM])
   by default, scales min data value to a relative grey level of <lowgrey>
   and max data value to a relative grey level of <higrey>.  
   0 must be less than or equal to <logrey>, which must be less than or
   equal to <higrey>, which must be less than or equal to 1.
   If <lovalue> and <hivalue> are specified, then data values are restricted
   to between those limits, and those limit values are scaled to <logrey>
   and <higrey>, respectively.  If /BYTE is specified, then the full
   range of greyscale values is assumed to run from 0 to 255 (inclusive);
   otherwise it is taken to run from !SCALEMIN to !SCALEMAX.  If /ZOOM
   is specified, then !ZOOMLOW and !ZOOMHIGH are taken for <lowvalue>
   and <hivalue>, respectively.  LS 16oct98 */
{
  int32_t	iq, type, result_sym, n, oldScalemin, oldScalemax;
  scalar	min, max;
  register	pointer q1, q2;
  double	sd, qd, logrey, higrey;
#if HAVE_LIBX11
  extern double	zoom_clo, zoom_chi;
  extern int32_t	threeColors;
  extern int32_t	colorIndexType, display_cells;
#else
  int32_t	colorIndexType = LUX_INT8, display_cells = 256;
#endif

  iq = ps[0];
  if (symbol_class(iq) != LUX_ARRAY)
    return cerror(NEED_ARR, iq);
  logrey = double_arg(ps[1]);
  higrey = double_arg(ps[2]);
  if (logrey < 0.0)
    logrey = 0.0;
  if (higrey < logrey)
    higrey = logrey;
  if (higrey > 1.0)
    higrey = 1.0;
  type = array_type(iq);
  q1.l = array_data(iq);
  n = array_size(iq);
  result_sym = array_clone(iq, colorIndexType);
  q2.l = array_data(result_sym);
  /* if only three args, then we scale between the min and max of array */
  if (narg == 3 && (internalMode & 2) == 0) {/* 3 arg and no /ZOOM */
    minmax(q1.l, n, type);	/* get the min and max */
    oldScalemin = scalemin;	/* remember for later */
    oldScalemax = scalemax;
    if (internalMode & 1) {	/* /BYTE */
      scalemin = (int32_t) (logrey*(display_cells - 0.001));
      scalemax = (int32_t) (higrey*(display_cells - 0.001));
    }
#if HAVE_LIBX11
    else if (threeColors) {
      scalemin = (int32_t) ((display_cells/3 - 0.001)*logrey);
      scalemax = (int32_t) ((display_cells/3 - 0.001)*higrey);
    }
#endif
    else {
      iq = scalemax - scalemin;
      scalemin = (int32_t) (iq*logrey + scalemin);
      scalemax = (int32_t) (scalemax - iq*(1 - higrey));
    }
    simple_scale(q1.l, n, type, q2.b); /* scale and put in output array */
    scalemin = oldScalemin;	/* restore */
    scalemax = oldScalemax;
    return result_sym;		/* done for this case */
  } else {
    /* more than three arg., so we expect the min and max to be specified */
    if (narg != 5)
      return cerror(WRNG_N_ARG, 0);
    /* this is messier than the simple scale because of over/under flows */
#if HAVE_LIBX11
    if (narg == 3) {		/* have /ZOOM */
      min.d = zoom_clo;
      max.d = zoom_chi;
    } else
#endif
      if (type == LUX_DOUBLE) { /* array is double, get input as double */
	min.d = double_arg(ps[3]);
	max.d = double_arg(ps[4]);
      } else {
	min.d = float_arg(ps[3]);
	max.d = float_arg(ps[4]);
      }
    if (internalMode & 1) {	/* /BYTE */
      sd = (double) (display_cells - 0.001)*higrey;
      qd = (double) (display_cells - 0.001)*logrey;
    }
#if HAVE_LIBX11
    else if (threeColors) {
      qd = (double) (display_cells/3 - 0.001)*logrey;
      sd = (double) (display_cells/3 - 0.001)*higrey;
    }
#endif
    else {
      iq = scalemax - scalemin;
      qd = iq*logrey + scalemin;
      sd = scalemax - iq*(1 - higrey);
    }
    scale(q1, type, n, min.d, max.d, q2.b, qd, sd);
  }
  return result_sym;
}
/*------------------------------------------------------------------------- */
int32_t minmax(int32_t *p, int32_t n, int32_t type)
/* finds the min and max (and their locations) for an array */
{
  pointer q;
  scalar	min, max;
/* pointers on OSF machines are bigger than ints, so casts from pointers */
/* to ints must be avoided!  thus, changed  minloc, maxloc  from int32_t to */
/* uint8_t *.  LS27jul94 */
  void	*minloc, *maxloc;
  int32_t	 nc;

  q.l = p;
  nc = n - 1;
  minloc = maxloc = (uint8_t *) p;
  switch (type) {
    case LUX_INT8:
      min.b = max.b = *q.b++;
      while (nc--) {
	if (*q.b > max.b) {
	  max.b = *q.b;
	  maxloc = q.b;
	} else if (*q.b < min.b) {
	  min.b = *q.b;
	  minloc = q.b;
	}
	q.b++;
      }
      lastmin.b = min.b;
      lastmax.b = max.b;
      scalar_type(lastmin_sym) = scalar_type(lastmax_sym) = LUX_INT8;
      break;
    case LUX_INT16:
      min.w = max.w = *q.w++;
      while (nc--) {
	if (*q.w > max.w) {
	  max.w = *q.w;
	  maxloc = q.w;
	} else if (*q.w < min.w) {
	  min.w = *q.w;
	  minloc = q.w;
	}
	q.w++;
      }
      lastmin.w = min.w;
      lastmax.w = max.w;
      scalar_type(lastmin_sym) = scalar_type(lastmax_sym) = LUX_INT16;
      break;
    case LUX_INT32:
      min.l = max.l = *q.l++;
      while (nc--) {
	if (*q.l > max.l) {
	  max.l = *q.l;
	  maxloc = q.l;
	} else if (*q.l < min.l) {
	  min.l = *q.l;
	  minloc = q.l;
	}
	q.l++;
      }
      lastmin.l = min.l;
      lastmax.l = max.l;
      scalar_type(lastmin_sym) = scalar_type(lastmax_sym) = LUX_INT32;
      break;
    case LUX_INT64:
      min.q = max.q = *q.q++;
      while (nc--) {
	if (*q.q > max.q) {
	  max.q = *q.q;
	  maxloc = q.q;
	} else if (*q.q < min.q) {
	  min.q = *q.q;
	  minloc = q.q;
	}
	q.q++;
      }
      lastmin.q = min.q;
      lastmax.q = max.q;
      scalar_type(lastmin_sym) = scalar_type(lastmax_sym) = LUX_INT64;
      break;
    case LUX_FLOAT:
      min.f = max.f = *q.f++;
      while (nc--) {
	if (*q.f > max.f) {
	  max.f = *q.f;
	  maxloc = q.f;
	} else if (*q.f < min.f) {
	  min.f = *q.f;
	  minloc = q.f;
	}
	q.f++;
      }
      lastmin.f = min.f;
      lastmax.f = max.f;
      scalar_type(lastmin_sym) = scalar_type(lastmax_sym) = LUX_FLOAT;
      break;
    case LUX_DOUBLE:
      min.d = max.d = *q.d++;
      while (nc--) {
	if (*q.d > max.d) {
	  max.d = *q.d;
	  maxloc = q.d;
	} else if (*q.d < min.d) {
	  min.d = *q.d;
	  minloc = q.d;
	}
	q.d++;
      }
      lastmin.d = min.d;
      lastmax.d = max.d;
      scalar_type(lastmin_sym) = scalar_type(lastmax_sym) = LUX_DOUBLE;
      break;
    }						/* end of type switch */
  lastmaxloc = ((uint8_t *) maxloc - (uint8_t *) p) / lux_type_size[type];
  lastminloc = ((uint8_t *) minloc - (uint8_t *) p) / lux_type_size[type];
  return 1;
}
/*------------------------------------------------------------------------- */
int32_t simple_scale(void *p1, int32_t n, int32_t type, void *p2)
/* scale n elements of the given data <type> starting at <p1> into the */
/* array at <p2> */
/* uses the min and max stashed in lastmin and lastmax */
/* assumes that the input array values are all inbetween the min and max */ 
/* this means we don't need to check for over and under flow */
/* assumes <p2> is of type <colorIndexType>.  LS 23mar99 */
{
  register	pointer q1, q2;
  register	scalar	range, min;
  register	int32_t	xq;
  register	float	fq;
  register	double	dq;
#if HAVE_LIBX11
  extern int32_t	connect_flag, colorIndexType;
#else
  int32_t	colorIndexType = LUX_INT8;
#endif

#if HAVE_LIBX11
  if (!connect_flag)
    setup_x();
#endif
  q1.l = p1;
  q2.l = p2;
  switch (type) {
    case LUX_INT8:
      min.b = lastmin.b;
      range.l = lastmax.b - min.b;
      xq = (int32_t) (scalemax - scalemin);
      if (range.l == 0)
	return neutral(p2, n);
      switch (colorIndexType) {
	case LUX_INT8:
	  while (n--)
	    *q2.b++ = (xq*(*q1.b++ - min.b))/range.l + scalemin;
	  break;
	case LUX_INT16:
	  while (n--)
	    *q2.w++ = (xq*(*q1.b++ - min.b))/range.l + scalemin;
	  break;
	case LUX_INT32:
	  while (n--)
	    *q2.l++ = (xq*(*q1.b++ - min.b))/range.l + scalemin;
	  break;
	case LUX_INT64:
	  while (n--)
	    *q2.q++ = (xq*(*q1.b++ - min.b))/range.l + scalemin;
	  break;
      }
      break;
    case LUX_INT16:
      min.w = lastmin.w;
      range.l = lastmax.w - min.w;
      xq = (int32_t) (scalemax - scalemin);
      if (range.l == 0)
	return neutral(p2, n);
      switch (colorIndexType) {
	case LUX_INT8:
	  while (n--)
	    *q2.b++ = (xq*(*q1.w++ - min.w))/range.l + scalemin;
	  break;
	case LUX_INT16:
	  while (n--)
	    *q2.w++ = (xq*(*q1.w++ - min.w))/range.l + scalemin;
	  break;
	case LUX_INT32:
	  while (n--)
	    *q2.l++ = (xq*(*q1.w++ - min.w))/range.l + scalemin;
	  break;
	case LUX_INT64:
	  while (n--)
	    *q2.q++ = (xq*(*q1.w++ - min.w))/range.l + scalemin;
	  break;
      }
      break;
    case LUX_INT32:
      /* to avoid possible range problems, calculation is done in fp */
      min.l = lastmin.l;
      range.l = lastmax.l - min.l;
      if (range.l == 0)
	return neutral(p2, n);
      fq = (float) (scalemax - scalemin)/(float) range.l;
      switch (colorIndexType) {
	case LUX_INT8:
	  while (n--)
	    *q2.b++ = (fq*(*q1.l++ - min.l)) + scalemin;
	  break;
	case LUX_INT16:
	  while (n--)
	    *q2.w++ = (fq*(*q1.l++ - min.l)) + scalemin;
	  break;
	case LUX_INT32:
	  while (n--)
	    *q2.l++ = (fq*(*q1.l++ - min.l)) + scalemin;
	  break;
	case LUX_INT64:
	  while (n--)
	    *q2.q++ = (fq*(*q1.l++ - min.l)) + scalemin;
	  break;
      }
      break;
    case LUX_INT64:
      /* to avoid possible range problems, calculation is done in fp */
      min.q = lastmin.q;
      range.q = lastmax.q - min.q;
      if (range.q == 0)
	return neutral(p2, n);
      fq = (float) (scalemax - scalemin)/(float) range.q;
      switch (colorIndexType) {
	case LUX_INT8:
	  while (n--)
	    *q2.b++ = (fq*(*q1.q++ - min.q)) + scalemin;
	  break;
	case LUX_INT16:
	  while (n--)
	    *q2.w++ = (fq*(*q1.q++ - min.q)) + scalemin;
	  break;
	case LUX_INT32:
	  while (n--)
	    *q2.l++ = (fq*(*q1.q++ - min.q)) + scalemin;
	  break;
	case LUX_INT64:
	  while (n--)
	    *q2.q++ = (fq*(*q1.q++ - min.q)) + scalemin;
	  break;
      }
      break;
    case LUX_FLOAT:
      min.f = lastmin.f;
      range.f = lastmax.f - min.f;
      if (range.f == 0)
	return neutral(p2, n);
      fq = (float) (scalemax-scalemin)/range.f;
      switch (colorIndexType) {
	case LUX_INT8:
	  while (n--)
	    *q2.b++ = (fq*(*q1.f++ - min.f)) + scalemin;
	  break;
	case LUX_INT16:
	  while (n--)
	    *q2.w++ = (fq*(*q1.f++ - min.f)) + scalemin;
	  break;
	case LUX_INT32:
	  while (n--)
	    *q2.l++ = (fq*(*q1.f++ - min.f)) + scalemin;
	  break;
	case LUX_INT64:
	  while (n--)
	    *q2.q++ = (fq*(*q1.f++ - min.f)) + scalemin;
	  break;
      }
      break;
    case LUX_DOUBLE:
      min.d = lastmin.d;
      range.d = lastmax.d - min.d;
      if (range.d == 0)
	return neutral(p2, n );
      dq = (double) (scalemax - scalemin)/range.d;
      switch (colorIndexType) {
	case LUX_INT8:
	  while (n--)
	    *q2.b++ = (dq*(*q1.d++ - min.d)) + scalemin;
	  break;
	case LUX_INT16:
	  while (n--)
	    *q2.w++ = (dq*(*q1.d++ - min.d)) + scalemin;
	  break;
	case LUX_INT32:
	  while (n--)
	    *q2.l++ = (dq*(*q1.d++ - min.d)) + scalemin;
	  break;
	case LUX_INT64:
	  while (n--)
	    *q2.q++ = (dq*(*q1.d++ - min.d)) + scalemin;
	  break;
      }
      break;
  }
  return LUX_OK;
}
/*------------------------------------------------------------------------- */
int32_t neutral(void *p, int32_t n)
/* fills an array of type <colorIndexType> with (scalemax-scalemin)/2 */
{
  int32_t	xq;
  pointer	pp;
#if HAVE_LIBX11
  extern int32_t	colorIndexType;
#else
  int32_t	colorIndexType = LUX_INT8;
#endif

  pp.b = p;
  xq = (scalemax - scalemin)/2;
  switch (colorIndexType) {
    case LUX_INT8:
      while (n--)
	*pp.b++ = xq;
      break;
    case LUX_INT16:
      while (n--)
	*pp.w++ = xq;
      break;
    case LUX_INT32:
      while (n--)
	*pp.l++ = xq;
      break;
    case LUX_INT64:
      while (n--)
	*pp.q++ = xq;
      break;
  }
  return LUX_OK;
}
/*------------------------------------------------------------------------- */
const csplineInfo empty_cubic_spline(void) {
  const csplineInfo c = { NULL, NULL, NULL, NULL };
  return c;
}
/*------------------------------------------------------------------------- */
void cleanup_cubic_spline_tables(csplineInfo *cspl) {
  if (cspl->spline) {
    gsl_spline_free(cspl->spline);
    gsl_interp_accel_free(cspl->acc);
    cspl->spline = NULL;
    cspl->acc = NULL;
  }
  free(cspl->x);
  cspl->x = NULL;
  free(cspl->y);
  cspl->y = NULL;
}
/*------------------------------------------------------------------------- */
int32_t cubic_spline_tables(void *xx, int32_t xType, int32_t xStep,
			void *yy, int32_t yType, int32_t yStep,
			int32_t nPoints, uint8_t periodic, uint8_t akima,
			csplineInfo *cspl)
/* installs a cubic spline for later quick multiple interpolations */
/* <xx> must point to the x coordinates, which are of LUX data type <xType> */
/* Each next <xStep>th element is taken.  Likewise for <yy>, <yType>, */
/* and <yStep>.  <nPoints> is the number of data points that are installed. */
/* The <xx> must be in ascending order!  If <periodic> is set, then
   the data is assumed to be periodic; otherwise it is not. */
/* LS 9may98; redone using GSL 2009sep27 */
{
  int32_t	n;
  pointer xin, yin;
  double *x, *y;

  const gsl_interp_type *interp_type;
  if (akima)
    interp_type = periodic? gsl_interp_akima_periodic: gsl_interp_akima;
  else
    interp_type = periodic? gsl_interp_cspline_periodic: gsl_interp_cspline;

  uint32_t minsize = gsl_interp_type_min_size(interp_type);
  if (nPoints < minsize) {
    luxerror("Have %d data points but need at least %d for this interpolation",
	     0, nPoints, minsize);
    return 1;
  }

  cleanup_cubic_spline_tables(cspl);
  cspl->spline = gsl_spline_alloc(interp_type, nPoints);

  cspl->acc = gsl_interp_accel_alloc();

  x = cspl->x = malloc(nPoints*sizeof(double));
  y = cspl->y = malloc(nPoints*sizeof(double));
  xin.b = (uint8_t *) xx;
  yin.b = (uint8_t *) yy;

  /* first copy x (with necessary type conversion) */
  if (xx) {			/* have x */
    n = nPoints;
    switch (xType) {
    case LUX_INT8:
      while (n--) {
	*x++ = (double) *xin.b;
	xin.b += xStep;
      }
      break;
    case LUX_INT16:
      while (n--) {
	*x++ = (double) *xin.w;
	xin.w += xStep;
      }
      break;
    case LUX_INT32:
      while (n--) {
	*x++ = (double) *xin.l;
	xin.l += xStep;
      }
      break;
    case LUX_INT64:
      while (n--) {
	*x++ = (double) *xin.q;
	xin.q += xStep;
      }
      break;
    case LUX_FLOAT:
      while (n--) {
	*x++ = (double) *xin.f;
	xin.f += xStep;
      }
      break;
    case LUX_DOUBLE:
      while (n--) {
	*x++ = *xin.d;
	xin.d += xStep;
      }
      break;
    } /* end of switch (xType) */
    x -= nPoints;
  } else {			/* no x: use indgen */
    for (n = 0; n < nPoints; n++)
      *x++ = (double) n;
    x -= nPoints;
  }

  /* then copy y (with necessary type conversion) */
  if (yy) {			/* have y */
    n = nPoints;
    switch (yType) {
    case LUX_INT8:
      while (n--) {
	*y++ = (double) *yin.b;
	yin.b += yStep;
      }
      break;
    case LUX_INT16:
      while (n--) {
	*y++ = (double) *yin.w;
	yin.w += yStep;
      }
      break;
    case LUX_INT32:
      while (n--) {
	*y++ = (double) *yin.l;
	yin.l += yStep;
      }
      break;
    case LUX_INT64:
      while (n--) {
	*y++ = (double) *yin.q;
	yin.q += yStep;
      }
      break;
    case LUX_FLOAT:
      while (n--) {
	*y++ = (double) *yin.f;
	yin.f += yStep;
      }
      break;
    case LUX_DOUBLE:
      while (n--) {
	*y++ = *yin.d;
	yin.d += yStep;
      }
      break;
    } /* end of switch (yType) */
    y -= nPoints;
  } else {			/* no y: use indgen */
    for (n = 0; n < nPoints; n++)
      *y++ = (double) n;
    y -= nPoints;
  }
  gsl_spline_init(cspl->spline, cspl->x, cspl->y, nPoints);
  return 0;
}
/*------------------------------------------------------------------------- */
double cspline_value(double x, csplineInfo *cspl)
/* interpolate using cubic splines.   Assumes <cspl> contains the required */
/* information about the cubic spline (installed with cubic_spline_tables() */
{
  gsl_spline *s = cspl->spline;

  if (x < s->x[0]) {
    double d1 = gsl_spline_eval_deriv(s, s->x[0], cspl->acc);
    double d2 = gsl_spline_eval_deriv2(s, s->x[0], cspl->acc);
    double dx = x - s->x[0];
    return s->y[0] + dx*(d1 + dx*d2);
  }
  if (x > s->x[s->size - 1]) {
    int32_t n = s->size - 1;
    double d1 = gsl_spline_eval_deriv(s, s->x[n], cspl->acc);
    double d2 = gsl_spline_eval_deriv2(s, s->x[n], cspl->acc);
    double dx = x - s->x[n];
    return s->y[n] + dx*(d1 + dx*d2);
  }
  return gsl_spline_eval(s, x, cspl->acc);
}
/*------------------------------------------------------------------------- */
double cspline_derivative(double x, csplineInfo *cspl)
/* returns the derivative of a cubic spline at position <x>.  Assumes that */
/* the required information about the spline is available in <cspl> */
/* (installed with cubic_spline_tables). */
/* LS 9may98 */
{
  gsl_spline *s = cspl->spline;
  if (x < s->x[0])
    return (s->y[1] - s->y[0])/(s->x[1] - s->x[0]);
  if (x > s->x[s->size - 1]) {
    int32_t n = s->size;
    return (s->y[n-1] - s->y[n-2])/(s->x[n-1] - s->x[n-2]);
  }
  return gsl_spline_eval_deriv(cspl->spline, x, cspl->acc);
}
/*------------------------------------------------------------------------- */
void cspline_value_and_derivative(double x, double *v, double *d,
				  csplineInfo *cspl)
/* returns interpolated value and derivative using cubic splines. LS 9may98 */
{
  gsl_spline_eval_e(cspl->spline, x, cspl->acc, v);
  gsl_spline_eval_deriv_e(cspl->spline, x, cspl->acc, d);
}
/*------------------------------------------------------------------------- */
double cspline_second_derivative(double x, csplineInfo *cspl)
/* returns the value of the second derivative of the spline indicated by
 <cspl> at position <x>.  LS 11may98 */
{
  return gsl_spline_eval_deriv2(cspl->spline, x, cspl->acc);
}
/*------------------------------------------------------------------------- */
#define LIMIT	40
double find_cspline_value(double value, double x1, double x2,
			  csplineInfo *cspl)
/* seeks and returns that value of the coordinate between <x1> and <x2> */
/* at which the specified <value> occurs, using cubic spline interpolation */
/* <x1> must be smaller than <x2>. */
/* LS 9may98 */
{
  double	v1, v2, vc, dt, x, tiny, dtnr, dx;
  int32_t	i, slow;

  tiny = 2*DBL_EPSILON;

  v1 = cspline_value(x1, cspl);
  v2 = cspline_value(x2, cspl);
  while ((value - v1)*(value - v2) > 0) { /* we're not yet bracketing
					     the sought value */
    x1 -= 0.5;
    x2 += 0.5;
    v1 = cspline_value(x1, cspl);
    v2 = cspline_value(x2, cspl);
  }
  /* OK, we're bracketing the desired value.  Now home in on it */
  dt = x2 - x1;
  x = 0.5*(x1 + x2);
  i = 0;
  do {
    cspline_value_and_derivative(x, &vc, &dtnr, cspl);
    dtnr = (value - vc)/dtnr;		/* Newton-Raphson correction */
    dx = fabs(dtnr);
    slow = (x + dx > x2 || x + dx < x1 || dx > 0.5*dt);
    /* maintain bracketing */
    if ((vc - value)*(vc - v1) > 0) {
      x2 = x;
      v2 = vc;
    } else {
      x1 = x;
      v1 = vc;
    }
    if (slow) {
      x = (x1 + x2)*0.5;
      dt = 0.5*(x2 - x1);
    } else {			/* accept Newton-Raphson step */
      x += dtnr;
      dt = dx;
    }
    i++;
  } while (dt > tiny && i < LIMIT);
  return x;
}
/*------------------------------------------------------------------------- */
void find_cspline_extremes(double x1, double x2, double *minpos, double *min,
			   double *maxpos, double *max, csplineInfo *cspl)
/* determines the position and values of extremes between positions <x1>
 and <x2>, using cubic spline interpolation.  <x1> must be smaller that
 <x2>.  The found positions and values are returned through the
 appropriate pointers, if these are non-zero.  If more than one
 extreme of a fixed sign exist between <x1> and <x2>, then it
 is undefined which one is returned.  LS 11may98 */
{
  double	v1, v2, dt, x, tiny, dtnr, vc, dx, xx1, xx2;
  int32_t	i, slow, sgn;

  tiny = 2*DBL_EPSILON;
  v1 = cspline_derivative(x1, cspl);
  v2 = cspline_derivative(x2, cspl);
  xx1 = x1;
  xx2 = x2;
  if ((v1 > 0) && (v2 > v2)) { /* continously climbing */
    if (minpos)
      *minpos = x1;
    if (min)
      *min = cspline_value(x1, cspl);
    if (maxpos)
      *maxpos = x2;
    if (max)
      *max = cspline_value(x2, cspl);
    return;
  }
  if (v1 < 0 && v2 < 0) { /* continuously falling */
    if (minpos)
      *minpos = x2;
    if (min)
      *min = cspline_value(x2, cspl);
    if (maxpos)
      *maxpos = x1;
    if (max)
      *max = cspline_value(x1, cspl);
    return;
  }
  /* if we get here then we assume we are bracketing a local extreme */
  if (v1 > 0 || v2 < 0)
    sgn = 1;			/* a local maximum */
  else				/* a local minimum */
    sgn = -1;
  if ((sgn > 0 && (max != NULL || maxpos != NULL))
      || (min != NULL || minpos != NULL)) { /* we want this one */
    dt = x2 - x1;
    x = 0.5*(x1 + x2);
    i = 0;
    do {
      vc = cspline_derivative(x, cspl);
      dtnr = -vc/cspline_second_derivative(x, cspl);
      dx = fabs(dtnr);
      slow = (x + dx > x2 || x + dx < x1 || dx > 0.5*dt);
      /* maintain bracketing */
      if (vc*v1 < 0) {
	x2 = x;
	v2 = vc;
      } else {
	x1 = x;
	v1 = vc;
      }
      if (slow) {
	x = (x1 + x2)*0.5;
	dt = 0.5*(x2 - x1);
      } else {			/* accept Newton-Raphson step */
	x += dtnr;
	dt = dx;
      }
      i++;
    } while (dt > tiny && i < LIMIT);
  }
  v1 = cspline_value(xx1, cspl);
  v2 = cspline_value(xx2, cspl);
  if (sgn < 0) {		/* it is a minimum */
    if (minpos)
      *minpos = x;
    if (min)
      *min = cspline_value(x, cspl);
    if (maxpos)
      *maxpos = (v2 > v1)? xx2: xx1;
    if (max)
      *max = (v2 > v1)? v2: v1;
  } else {			/* it is a maximum */
    if (maxpos)
      *maxpos = x;
    if (max)
      *max = cspline_value(x, cspl);
    if (minpos)
      *minpos = (v1 < v2)? xx1: xx2;
    if (min)
      *min = (v1 < v2)? v1: v2;
  }
}
/*------------------------------------------------------------------------- */
int32_t lux_cubic_spline(int32_t narg, int32_t ps[])
/* Cubic spline interpolation along the first dimension of an array.
   ynew = CSPLINE(xtab, ytab [, xnew] [, /KEEP, /PERIODIC, /GETDERIVATIVE])
     interpolates in the table of <ytab> versus <xtab> (must be in
     ascending order without duplicates) at positions <xnew>.  /KEEP
     signals retention of <xtab>, <ytab>, and the derived second
     derivatives for use in subsequent interpolations from the same
     table.  /GETDERIVATIVE indicates that the first derivative rather
     than the value of the spline at the indicated positions should be
     returned.  /PERIODIC indicates that the data is periodic.
   ynew = CSPLINE(xnew)
     Interpolates at positions <xnew> in the table specified last in a
     call to CSPLINE, in which the /KEEP switch must have been specified.
   dnew = CSPLINE(xnew, /GETDERIVATIVE)
     Returns the interpolated first derivative at positions <xnew> in
     the table specified last in a call to CSPLINE, in which the /KEEP
     switch must have been specified.
   ynew = CSPLINE()
     Clear the table that was retained in a call to CSPLINE employing the
     /KEEP switch.  (In case the table is real big and you want to get
     rid of it.)
   LS 29apr96 */
{
  static char	haveTable = '\0';
  static csplineInfo	cspl = { NULL, NULL, NULL, NULL };
  int32_t	xNewSym, xTabSym, yTabSym, size, oldType, result_sym;
  pointer	src, trgt;
  int32_t	lux_convert(int32_t, int32_t [], int32_t, int32_t);

  /* first check on all the arguments */
  if (!narg) { 			/* CSPLINE(): clean-up */
    cleanup_cubic_spline_tables(&cspl);
    return 1;
  }

  xTabSym = yTabSym = xNewSym = 0;
  if (narg >= 2) {		/* have <xtab> and <ytab> */
    xTabSym = *ps++;
    yTabSym = *ps++;
  }
  if (narg % 2 == 1)		/* have <xnew> */
    xNewSym = *ps++;

  if (xNewSym && !haveTable && !xTabSym)
    return luxerror("Need table to interpolate from", xNewSym);
  if (xTabSym && symbol_class(xTabSym) != LUX_ARRAY)
    return cerror(NEED_ARR, xTabSym);
  if (yTabSym && symbol_class(yTabSym) != LUX_ARRAY)
    return cerror(NEED_ARR, yTabSym);
  if (xTabSym && array_size(xTabSym) != array_size(yTabSym))
    return cerror(INCMP_ARR, yTabSym);

  if (xTabSym) {		/* install new table */
    if (cubic_spline_tables(array_data(xTabSym), array_type(xTabSym), 1,
			    array_data(yTabSym), array_type(yTabSym), 1,
			    array_size(xTabSym), internalMode & 2,
			    internalMode & 4, &cspl))
      return LUX_ERROR;		/* some error */
    haveTable = 1;
  }

  result_sym = LUX_ONE;
  if (xNewSym) {		/* do interpolation */
    oldType = symbol_type(xNewSym);
    switch (symbol_class(xNewSym)) {
      case LUX_SCAL_PTR:
	xNewSym = dereferenceScalPointer(xNewSym); /* fall-thru */
      case LUX_SCALAR:
	if (oldType != LUX_DOUBLE)
	  xNewSym = lux_convert(1, &xNewSym, LUX_DOUBLE, 1);
	src.b = &scalar_value(xNewSym).b;
	size = 1;
	result_sym = scalar_scratch(LUX_DOUBLE);
	trgt.b = &scalar_value(result_sym).b;
	break;
      case LUX_ARRAY:
	if (oldType <= LUX_DOUBLE) { /* not a string array */
	  if (oldType != LUX_DOUBLE)
	    xNewSym = lux_convert(1, &xNewSym, LUX_DOUBLE, 1);
	  src.b = (uint8_t *) array_data(xNewSym);
	  size = array_size(xNewSym);
	} else
	  return cerror(ILL_TYPE, xNewSym);
	result_sym = array_clone(xNewSym, LUX_DOUBLE);
	trgt.b = (uint8_t *) array_data(result_sym);
	break;
      default:
	result_sym = cerror(ILL_TYPE, xNewSym);
    }
    if (result_sym != LUX_ERROR) { /* no error */
      if (internalMode & 8) 	/* return the interpolated derivative */
	while (size--)
	  *trgt.d++ = cspline_derivative(*src.d++, &cspl);
      else			/* return the interpolated value */
	while (size--)
	  *trgt.d++ = cspline_value(*src.d++, &cspl);
    }
  }
  if ((internalMode & 1) == 0 && xTabSym) { /* no /KEEP, and xnew was
					     specified so throw away table */
    cleanup_cubic_spline_tables(&cspl);
    haveTable = 0;
  }
  return result_sym;
}
/*------------------------------------------------------------------------- */
int32_t lux_cubic_spline_extreme(int32_t narg, int32_t ps[])
/* CSPLINE_EXTREME, <x>, <y> [, <axis>, POS=<pos>, MINPOS=<minpos>,
   MINVAL=<min>, MAXPOS=<maxpos>, MAXVAL=<max>, /KEEPDIMS, /PERIODIC]
 */
{
  int32_t	iq, dims[MAX_DIMS], ndim, step, pos, i, mode;
  double	thisextpos, thisext, x1, x2;
  loopInfo	yinfo;
  pointer	y, x, minpos, min, maxpos, max, rightedge, ptr, q;
  csplineInfo	cspl;

  if (!symbolIsNumericalArray(ps[0]))
    return cerror(NEED_NUM_ARR, ps[0]);
  if (!symbolIsNumericalArray(ps[1]))
    return cerror(NEED_NUM_ARR, ps[1]);

  mode = SL_COMPRESS | SL_ONEAXIS | SL_NEGONED | SL_EACHROW | SL_AXISCOORD
    | SL_SRCUPGRADE;

  if (standardLoop(ps[1], (narg > 2 && ps[2])? ps[2]: LUX_ZERO, mode,
		   LUX_FLOAT, &yinfo, &y, NULL, NULL, NULL) == LUX_ERROR)
    return LUX_ERROR;

  /* <x> must have one element for each element of <y> along the indicated
     <axis>. */
  if (array_size(ps[0]) != yinfo.rdims[0])
    return cerror(INCMP_ARG, ps[1]);
  iq = lux_converts[yinfo.type](1, &ps[0]);
  x.f = array_data(iq);
  
  if (narg > 3 && ps[3]) { 	/* have <pos> */
    pos = int_arg(ps[3]);
    if (pos < 0 || pos >= yinfo.rdims[0])
      return luxerror("Index out of range", ps[3]);
  } else				/* no <pos> */
    pos = -1;

  /* prepare the output variables */
  if (internalMode & 1) {
    memcpy(dims, yinfo.dims, yinfo.ndim*sizeof(int32_t));
    dims[yinfo.axes[0]] = 1;
    ndim = yinfo.ndim;
  } else if (yinfo.rndim == 1)
    ndim = 0;
  else {
    memcpy(dims, yinfo.dims, yinfo.axes[0]*sizeof(int32_t));
    memcpy(dims + yinfo.axes[0], yinfo.dims + yinfo.axes[0] + 1,
	   (yinfo.ndim - yinfo.axes[0] - 1)*sizeof(int32_t));
    ndim = yinfo.ndim - 1;
  }

  if (narg > 4 && ps[4]) {	/* <minpos> */
    if (!symbolIsNamed(ps[4]))
      return luxerror("Need named variable here", ps[4]);
    if (ndim) {
      to_scratch_array(ps[4], yinfo.type, ndim, dims);
      minpos.f = array_data(ps[4]);
    } else {
      to_scalar(ps[4], yinfo.type);
      minpos.f = &scalar_value(ps[4]).f;
    }
  } else
    minpos.f = NULL;

  if (narg > 5 && ps[5]) {	/* <min> */
    if (!symbolIsNamed(ps[5]))
      return luxerror("Need named variable here", ps[5]);
    if (ndim) {
      to_scratch_array(ps[5], yinfo.type, ndim, dims);
      min.f = array_data(ps[5]);
    } else {
      to_scalar(ps[5], yinfo.type);
      min.f = &scalar_value(ps[5]).f;
    }
  } else
    min.f = NULL;

  if (narg > 6 && ps[6]) {	/* <maxpos> */
    if (!symbolIsNamed(ps[6]))
      return luxerror("Need named variable here", ps[6]);
    if (ndim) {
      to_scratch_array(ps[6], yinfo.type, ndim, dims);
      maxpos.f = array_data(ps[6]);
    } else {
      to_scalar(ps[6], yinfo.type);
      maxpos.f = &scalar_value(ps[6]).f;
    }
  } else
    maxpos.f = NULL;

  if (narg > 7 && ps[7]) {	/* <max> */
    if (!symbolIsNamed(ps[7]))
      return luxerror("Need named variable here", ps[7]);
    if (ndim) {
      to_scratch_array(ps[7], yinfo.type, ndim, dims);
      max.f = array_data(ps[7]);
    } else {
      to_scalar(ps[7], yinfo.type);
      max.f = &scalar_value(ps[7]).f;
    }
  } else
    max.f = NULL;

  step = yinfo.step[0];
  
  /* now do the work */
  switch (yinfo.type) {
    case LUX_FLOAT:
      do {
	/* install table for cubic spline interpolation */
	cubic_spline_tables(x.f, LUX_FLOAT, 1,
			    y.f, LUX_FLOAT, step,
			    yinfo.rdims[0], internalMode & 2? 1: 0,
			    internalMode & 4? 1: 0,
			    &cspl);

	rightedge.f = y.f + step*(yinfo.rdims[0] - 1);

	if (min.f || minpos.f) {
	  /* first, we go for the minimum */
	  if (pos >= 0) {
	    ptr.f = y.f + pos*step; /* start position */
	    /* now seek the local minimum */
	    if (ptr.f > y.f + 1 && ptr.f[-step] < *ptr.f)
	      while (ptr.f > y.f + 1 && ptr.f[-step] < *ptr.f)
		ptr.f -= step;
	    else
	      while (ptr.f < rightedge.f - 1 && ptr.f[step] < *ptr.f)
		ptr.f += step;
	  } else {		/* find absolute minimum */
	    ptr.f = q.f = y.f;
	    while (q.f < rightedge.f - 1) {
	      q.f += step;
	      if (*q.f < *ptr.f)
		ptr.f = q.f;
	    }
	  }

	  /* the cubic spline may dip below the lower of the surrounding
	     specified data points: we must find the local minimum in
	     the cubic spline. */
	  i = (ptr.f - y.f)/step;
	  if (x.f) {
	    x1 = x.f[i - 1];
	    x2 = x.f[i + 1];
	  } else {
	    x1 = i - 1;
	    x2 = i + 1;
	  }
	  find_cspline_extremes(x1, x2, &thisextpos, &thisext, NULL, NULL,
				&cspl);
	  if (minpos.f)
	    *minpos.f++ = thisextpos;
	  if (min.f)
	    *min.f++ = thisext;
	}
	  
	if (max.f || maxpos.f) {
	  /* then, we go for the maximum */
	  if (pos >= 0) {
	    ptr.f = y.f + pos*step; /* start position */
	    /* now seek the local minimum */
	    if (ptr.f > y.f + 1 && ptr.f[-step] > *ptr.f)
	      while (ptr.f > y.f + 1 && ptr.f[-step] > *ptr.f)
		ptr.f -= step;
	    else
	      while (ptr.f < rightedge.f - 1 && ptr.f[step] > *ptr.f)
		ptr.f += step;
	  } else {		/* find absolute minimum */
	    ptr.f = q.f = y.f;
	    while (q.f < rightedge.f - 1) {
	      q.f += step;
	      if (*q.f > *ptr.f)
		ptr.f = q.f;
	    }
	  }

	  /* the cubic spline may dip below the lower of the surrounding
	     specified data points: we must find the local minimum in
	     the cubic spline. */
	  i = (ptr.f - y.f)/step;
	  if (x.f) {
	    x1 = x.f[i - 1];
	    x2 = x.f[i + 1];
	  } else {
	    x1 = i - 1;
	    x2 = i + 1;
	  }
	  find_cspline_extremes(x1, x2, NULL, NULL, &thisextpos, &thisext,
				&cspl);
	  if (maxpos.f)
	    *maxpos.f++ = thisextpos;
	  if (max.f)
	    *max.f++ = thisext;
	}
        y.f += step*yinfo.rdims[0];
      } while (advanceLoop(&yinfo, &y) < yinfo.rndim);
      break;
    case LUX_DOUBLE:
      do {
	/* install table for cubic spline interpolation */
	cubic_spline_tables(x.d, LUX_DOUBLE, 1,
			    y.d, LUX_DOUBLE, step,
			    yinfo.rdims[0], internalMode & 2? 1: 0,
			    internalMode & 4? 1: 0,
			    &cspl);

	rightedge.d = y.d + step*(yinfo.rdims[0] - 1);

	if (min.d || minpos.d) {
	  /* first, we go for the minimum */
	  if (pos >= 0) {
	    ptr.d = y.d + pos*step; /* start position */
	    /* now seek the local minimum */
	    if (ptr.d > y.d + 1 && ptr.d[-step] < *ptr.d) /* at local max,
							 go left */
	      while (ptr.d > y.d + 1 && ptr.d[-step] < *ptr.d)
		ptr.d -= step;
	    else
	      while (ptr.d < rightedge.d - 1 && ptr.d[step] < *ptr.d)
		ptr.d += step;
	  } else {		/* find absolute minimum */
	    ptr.d = q.d = y.d;
	    while (q.d < rightedge.d - 1) {
	      q.d += step;
	      if (*q.d < *ptr.d)
		ptr.d = q.d;
	    }
	  }

	  /* the cubic spline may dip below the lower of the surrounding
	     specified data points: we must find the local minimum in
	     the cubic spline. */
	  i = (ptr.d - y.d)/step;
	  if (x.d) {
	    x1 = x.d[i - 1];
	    x2 = x.d[i + 1];
	  } else {
	    x1 = i - 1;
	    x2 = i + 1;
	  }
	  find_cspline_extremes(x1, x2, &thisextpos, &thisext, NULL, NULL,
				&cspl);
	  if (minpos.d)
	    *minpos.d++ = thisextpos;
	  if (min.d)
	    *min.d++ = thisext;
	}
	  
	if (max.d || maxpos.d) {
	  /* then, we go for the maximum */
	  if (pos >= 0) {
	    ptr.d = y.d + pos*step; /* start position */
	    /* now seek the local minimum */
	    if (ptr.d > y.d + 1 && ptr.d[-step] > *ptr.d)
	      while (ptr.d > y.d + 1 && ptr.d[-step] > *ptr.d)
		ptr.d -= step;
	    else
	      while (ptr.d < rightedge.d - 1 && ptr.d[step] > *ptr.d)
		ptr.d += step;
	  } else {		/* find absolute minimum */
	    ptr.d = q.d = y.d;
	    while (q.d < rightedge.d - 1) {
	      q.d += step;
	      if (*q.d > *ptr.d)
		ptr.d = q.d;
	    }
	  }

	  /* the cubic spline may dip below the lower of the surrounding
	     specified data points: we must find the local minimum in
	     the cubic spline. */
	  i = (ptr.d - y.d)/step;
	  if (x.d) {
	    x1 = x.d[i - 1];
	    x2 = x.d[i + 1];
	  } else {
	    x1 = i - 1;
	    x2 = i + 1;
	  }
	  find_cspline_extremes(x1, x2, NULL, NULL, &thisextpos, &thisext,
				&cspl);
	  if (maxpos.d)
	    *maxpos.d++ = thisextpos;
	  if (max.d)
	    *max.d++ = thisext;
	}
        y.d += step*yinfo.rdims[0];
      } while (advanceLoop(&yinfo, &y) < yinfo.rndim);
      break;
  default:
    break;
  }
  return LUX_OK;
}
/*------------------------------------------------------------------------- */
int32_t lux_strtol(int32_t narg, int32_t ps[])
/* corresponds to C strtol */
{
  char	*string;
  int32_t	base, number, result;

  if (symbol_class(ps[0]) != LUX_STRING)
    return cerror(NEED_STR, ps[0]);
  string = string_arg(ps[0]);
  if (narg > 1) {
    if (symbol_class(ps[1]) != LUX_SCALAR)
      return cerror(NEED_SCAL, ps[1]);
    base = int_arg(ps[1]);
  } else
    base = 10;
  number = strtol(string, NULL, base);
  result = scalar_scratch(LUX_INT32);
  scalar_value(result).l = number;
  return result;
}
/*------------------------------------------------------------------------- */
uint32_t bitmask[] = {
  0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff,
  0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};
static uint32_t bits[16] = {
  1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768
};
int32_t lux_extract_bits_f(int32_t narg, int32_t ps[])/* get a bit field from data */
/* call is xint = extract_bits(array, start, width) */
/* if width is < 0, then abs(width) is used for width and the sign
   is extended */
{
  int32_t	result_sym, value;
  int32_t	extract_bits(int32_t, int32_t [], int32_t *);

  if (extract_bits(narg, ps, &value) != 1)
    return LUX_ERROR;
  result_sym = scalar_scratch(LUX_INT16);
  scalar_value(result_sym).w = (short) value;
  return result_sym;
}
/*------------------------------------------------------------------------ */
int32_t lux_extract_bits(int32_t narg, int32_t ps[])/* get a bit field from data */
/* call is extract_bits, result, array, start, width */
/* if width is < 0, then abs(width) is used for width and the sign
   is extended */
{
  int32_t	result_sym, value;
  int32_t	extract_bits(int32_t, int32_t [], int32_t *);

  result_sym = ps[0];
  if (extract_bits(narg - 1, &ps[1], &value) != 1)
    return LUX_ERROR;
  return redef_scalar(result_sym, LUX_INT16, &value);
}
/*------------------------------------------------------------------------ */
int32_t extract_bits(int32_t narg, int32_t ps[], int32_t *value)/* internal routine */
{
  int32_t	n, start, width, iq, j, sign_flag, type;
  pointer	q;
  
  iq = ps[0];
  if (numerical(iq, NULL, NULL, &n, &q) == LUX_ERROR)
    return LUX_ERROR;
  type = symbol_type(iq);
  n = n * lux_type_size[type];	/* now the # of bytes */
  if (int_arg_stat(ps[1], &start) != 1)
    return LUX_ERROR;
  if (int_arg_stat(ps[2], &width) != 1)
    return LUX_ERROR;
  /* check for sign extension */
  if (width < 0) {
    width = -width;
    sign_flag = 1;
  } else
    sign_flag = 0;
  if (start + width > 8*n)	/* added LS 12feb99 */
    return luxerror("Extracting bits beyond the end of the data", ps[2]);
  /* start is a bit count, get the I*2 index */
  j = start/16;
  iq = (int32_t) q.w[j];
  j = start % 16;	/* our shift */
  iq = (iq >> j) & bitmask[width];
  if (sign_flag)
    /* messy, but probably not used too often */
    if (iq & bits[width-1])
      /* the top bit of the bit field is set, so need to extend it */
      iq = iq | ( 0xffffffff - bitmask[width]);
  *value = iq;
  return LUX_OK;
}
/*------------------------------------------------------------------------- */
int32_t lux_fade_init(int32_t narg, int32_t ps[])
/* initializes a fade (or dissolve) between 2 uint8_t arrays, the actual
   fade result is obtained with the subroutine fade
   call is: fade_init, x1, x2 */
{
  uint8_t	*p1, *p2;
  int16_t	*q1, *q2;
  int32_t	n, iq;

  iq = ps[0];
  if (!symbolIsNumericalArray(iq) || array_num_dims(iq) != 2)
    return cerror(NEED_2D_ARR, iq);
  if (array_type(iq) != LUX_INT8)
    return cerror(NEED_BYTE, iq);
  p1 = array_data(iq);
  fade_xsize = array_dims(iq)[0];
  fade_ysize = array_dims(iq)[1];
  fade_dim[0] = fade_xsize;
  fade_dim[1] = fade_ysize;
  
  iq = ps[1];
  if (!symbolIsNumericalArray(iq) || array_num_dims(iq) != 2)
    return cerror(NEED_2D_ARR, iq);
  if (array_type(iq) != LUX_INT8)
    return cerror(NEED_BYTE, iq);
  if (array_dims(iq)[0] != fade_xsize || array_dims(iq)[1] != fade_ysize)
    return cerror(INCMP_ARG, iq);

  p2 = array_data(iq);
  /* set up the I*2 arrays for quick fades */
  if (fade1) { free(fade1); fade1 = NULL; }
  if (fade2) { free(fade2); fade2 = NULL; }
  /* since the inputs are uint8_t arrays, n1 is the size in bytes, we need 2*n1
     for the fade arrays */
  n = 2 * fade_xsize * fade_ysize;
  fade1 = (short *) malloc(n);
  fade2 = (short *) malloc(n);
  n = fade_xsize * fade_ysize;	q1 = fade1;	q2 = fade2;
  while (n--) { *q1++ = ( (short) *p2) << 8 ;
  *q2++ = (( (short) *p1++) - ( (short) *p2++));
  }
 
  /*
    while (n--) { *q1++ = ( (short) *p1++);
    *q2++ = ( (short) *p1++);
    }
  */
  return LUX_OK;
}
/*------------------------------------------------------------------------- */
int32_t lux_fade(int32_t narg, int32_t ps[])
 /* does a fade (or dissolve) between 2 uint8_t arrays, must be initialized
 with fade_init
 call is: fade, weight, result - where n is from 0 to 256 and represents
 the weight of the first array that was passed to fade_init
 result is a subroutine argument to avoid mallocs */
{
  uint8_t	*p1;
  int16_t	*q1, *q2, wt;
  int32_t	weight, n, iq;

  if (int_arg_stat(ps[0], &weight) != LUX_OK) return LUX_ERROR;
  wt = (int16_t) weight;
  iq = ps[1];
  if (redef_array(iq, 0, 2, fade_dim) != LUX_OK) return LUX_ERROR;

  p1 = array_data(iq);
  n = fade_xsize * fade_ysize;	q1 = fade1;	q2 = fade2;
  while (n--) { 
    *p1++ = (uint8_t) (( wt * *q2++ + *q1++) >> 8);
  }
  return LUX_OK;
}
/*------------------------------------------------------------------------- */
