/* File fun3.c */
/* Various ANA functions. */
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
#include "action.h"
#include "editorcharclass.h"
#include "anaparser.c.tab.h"
static char rcsid[] __attribute__ ((unused)) =
 "$Id: fun3.c,v 4.0 2001/02/07 20:37:01 strous Exp $";

int	ezffti(int *, float *), fade_xsize, fade_ysize, fade_dim[2];
word	*fade1, *fade2;
extern int	scalemax, scalemin, lastmaxloc, lastminloc, maxhistsize,
	histmin, histmax, fftdp, lastmax_sym, lastmin_sym;
extern scalar	lastmin, lastmax;
static	int	nold = 0, fftdpold, mqold;
static pointer	work;
int	minmax(int *, int, int), neutral(void *, int);
void	zerobytes(void *, int), zap(int);
int	simple_scale(void *, int, int, void *);
/*------------------------------------------------------------------------- */
int evalString(char *expr, int nmax)
/* compiles and evaluates a string, interpreting it as code */
/* non-strings are just evaluated.  If the second argument it set, */
/* then it determines up to how many numbers are read from the string */
/* and put in an array. */
/* If instead of a second argument the keyword /ALLNUMBER is specified, */
/* then all numbers are read from the string and put in an array. */
/* All read numbers have no letter immediately preceding them.  Minus signs */
/* are honored.  All valid ANA number styles are supported (including */
/* type and base specifications, and fractional and exponentiated numbers). */
/* If numbers are read, then any non-number is ignored. */
/* LS 2may96 */
{
 char	*save, *text;
 int	result, readNumber(YYSTYPE *), n, type, temp, size;
 void	translateLine(void), convertScalar(scalar *, int, int);
 pointer	p;
 scalar	s;
 extern char	*currentChar;
 extern int	tempSym;
 YYSTYPE	valp;
 word	popList(void);
 int	compileString(char *);
 void	pushList(word);

 if (nmax < 0)			/* /ALLNUMBER */
   nmax = INT_MAX;
 if (nmax) {
   save = currentChar;		/* remember what we were interpreting before */
   currentChar = expr;
   n = 0;
   type = ANA_BYTE;
   while (n < nmax) {
     while (!isNumberChar((byte) *currentChar)
	    && *currentChar) /* seek digit */
       currentChar++;
     if (!*currentChar)		/* no (more) digits */
       break;
     if (currentChar > expr && isalpha((byte) currentChar[-1])) {
       /* not a separate number */
       while (isNumberChar((byte) *currentChar))
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
     size = ana_type_size[type];
     if (n == 1) {		/* only a single number: use ANA_SCALAR */
       result = scalar_scratch(type);
       p.b = &scalar_value(result).b + size; 
     } else {
       result = array_scratch(type, 1, &n);
       p.b = (byte *) array_data(result) + n*size;
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
     result = ANA_ZERO;		/* nothing found; return 0 */
   currentChar = save;
   return result;
 }
 /* If we get here then we're interpreting the complete line as an */
 /* ANA expression */
 allocate(text, strlen(expr) + 7, char);
 strcpy(text, "!TEMP=");	/* we assign the evaluation result to !TEMP */
 strcat(text, expr);
 result = compileString(text);
 free(text);
 return (result > 0)? copySym(tempSym): ANA_ERROR;
}
/*------------------------------------------------------------------------- */
int ana_eval(int narg, int ps[])
/* evaluates a string argument.  If there is a single argument and that
   argument is a single name, then we seek the number of the symbol that
   has that name and return the number, if any.  Otherwise we return the
   value of the expression.  LS 7jan99 */
{
  int	nmax, result;
  char	*expr, *p1, *p2, *p3, c;
  
  if (symbol_class(*ps) != ANA_STRING)
    return *ps;
  if (narg > 1) {		/* read numbers */
    nmax = int_arg(ps[1]);
    if (nmax < 1)
      return anaerror("Cannot read a negative number of values", ps[1]);
  } else
    nmax = INT_MAX;
  if (narg == 1 || internalMode & 1) /* /ALLNUMBER */
    nmax = -nmax;
  expr = string_arg(*ps);

  p1 = expr;
  while (*p1 && isWhiteSpace((int) *p1))
    p1++;
  if (isFirstChar((int) *p1)) {	/* an identifier? */
    p2 = p1 + 1;
    while (isNextChar((int) *p2)) /* span the identifier */
      p2++;
    p3 = p2;
    while (*p3 && isWhiteSpace((int) *p3))
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
      if (result != ANA_ERROR)
	return result;
    }
  } 
  result = evalString(expr, nmax);
  if (result > 0 && !isFreeTemp(result))
    result = copySym(result);
  return result;
}
/*------------------------------------------------------------------------- */
int ana_tense(int narg, int ps[])			/* tense function */ 
/* splines under tension */
/* THE CALL IS  YF=TENSE(X,Y,XF,[SIGMA],[DLHS,DRHS]) */
{
  int	i, iq, len[3], n, result_sym, nf;
  double	sigma, der_left, der_right, *st, *yf;
  pointer p[3];
  array	*h;
  int	curv1_(int *n, double *x, double *y, double *slp1, double *slpn,
	   double *yp, double *temp, double *sigma, double *xf, double *yf,
	   int *nf);
					/* first 3 args must be 1-D vectors */
  for (i=0;i<3;i++) {
    iq = ps[i];
    if ( sym[iq].class != ANA_ARRAY ) return cerror(NEED_ARR, ps[i]);
    h = (array *) sym[iq].spec.array.ptr;
    if ( h->ndim != 1) return cerror(NEED_1D_ARR, ps[i]); 	/* ck if 1-D */
    /* double each one */
    iq = ana_double(1, &iq);
    h = (array *) sym[iq].spec.array.ptr;
    len[i] = h->dims[0];
    p[i].l = (int *) ((char *)h + sizeof(array));
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
int ana_tense_curve(int narg, int ps[])/* tense_curve function */
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
  int	i, iq, len[3], n, result_sym, dim[2], nf;
  double	sigma, der_left, der_right, *st, *yf, *xf;
  pointer p[3];
  array	*h;
  int	kurv1_(int *n, double *x, double *y, double *slp1, double *slpn,
	   double *xp, double *yp, double *temp, double *sigma, double *t,
	   double *xs, double *ys, int *nf);

				/* first 3 args must be 1-D vectors */
for (i=0;i<3;i++) {
iq = ps[i];
if ( sym[iq].class != 4 ) return cerror(NEED_ARR, ps[i]);
h = (array *) sym[iq].spec.array.ptr;
if ( h->ndim != 1) return cerror(NEED_1D_ARR, ps[i]); 	/* ck if 1-D */
						/* double each one */
iq = ana_double(1, &iq);
h = (array *) sym[iq].spec.array.ptr;
len[i] = h->dims[0];
p[i].l = (int *) ((char *)h + sizeof(array));
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
int ana_tense_loop(int narg, int ps[])/* tense_loop function */		
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
  int	i, iq, len[3], n, result_sym, dim[2], nf;
  double	sigma, *st, *yf, *xf;
  pointer p[3];
  array	*h;
  int kurvp1_(int *n, double *x, double *y, double *xp, double *yp,
	      double *temp, double *sigma, double *t, double *xs, double *ys,
	      int *nf);
					/* first 3 args must be 1-D vectors */
for (i=0;i<3;i++) {
iq = ps[i];
if ( sym[iq].class != 4 ) return cerror(NEED_ARR, ps[i]);
h = (array *) sym[iq].spec.array.ptr;
if ( h->ndim != 1) return cerror(NEED_1D_ARR, ps[i]); 	/* ck if 1-D */
						/* double each one */
iq = ana_double(1, &iq);
h = (array *) sym[iq].spec.array.ptr;
len[i] = h->dims[0];
p[i].l = (int *) ((char *)h + sizeof(array));
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
int ana_sc(int narg, int ps[])	/* sc routine */		
/* sine-cosine style FFT --  sc, x, s, c */
{
  int	iq, nd, outer, n, mq, dim[MAX_DIMS], type, jq, nn;
  pointer	q1,q2,q3;
  int	ezffti(int *n, float *wsave), ezfftid(int *n, double *wsave),
    ezfftf(int *n, float *r, float *azero, float *a, float *b, float *wsave),
    ezfftfd(int *n, double *r, double *azero, double *a, double *b,
	    double *wsave);

  iq = ps[0];
  if (!symbolIsArray(iq))
    return cerror(NEED_ARR, ps[0]);
  /*float the input */
  if (fftdp == 0)		/* single precision */
    iq = ana_float(1, ps);
  else				/* double precision */
    iq = ana_double(1, ps);

  q1.l = array_data(iq);

  /* find inner and product of all outer dimensions */
  nd = array_num_dims(iq);
  n = array_dims(iq)[0];
  outer = array_size(iq)/n;
  memcpy(dim + 1, array_dims(iq)+ 1, (nd - 1)*sizeof(int));
  mq = array_size(iq)*ana_type_size[array_type(iq)];

	/* scratch storage is needed, can we use the last one setup ? */
  if (nold != n || fftdp != fftdpold || mq > mqold) {/* set up work space */
    if (nold)
      free(work.b);		/* delete old if any */
    mq = n*28 + 120;
    if (!fftdp)
      mq = mq/2;
    work.l = (int *) malloc(mq);
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
  type = (fftdp? ANA_DOUBLE: ANA_FLOAT);
  if (redef_array(iq, type, nd, dim) != 1
      || redef_array(jq, type, nd, dim) != 1)
    return ANA_ERROR;
					/* get addresses */
  q2.l = array_data(iq);		/* <s> */
  q3.l = array_data(jq);		/* <c> */
					/* loop over the outer dimensions */
  while (outer--) {
    switch (type) {
      case ANA_FLOAT:
	ezfftf(&n, q1.f, q3.f, q3.f + 1, q2.f + 1, work.f);
	*q2.f = 0.0;		/* zero-frequency sine always equal to 0 */
	q1.f += n;
	q2.f += nn;
	q3.f += nn;
	break;
      case ANA_DOUBLE:
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
#define SQRTHALF	(0.707106781186548)
#define SQRTWO		(1.414213562373095)
int fft(int narg, int ps[], int isFunction)
/* This routine calculates the fourier transform of a real data set. */
/* y = FFT(x [, axes, DIRECTION=<direction>]) */
/* y = FFT(x) and y = FFT(x,DIRECTION=+1) calculate the forward, and */
/* y = FFT(x,/back) and y = FFT(x,DIRECTION=-1) the backward transform. */
{
  int	iq, n, mq, type, j, jq, direction, result, step, doublePrecision;
  pointer	src, trgt, work, tmp, otmp, src0, trgt0;
  scalar	factor;
  int	rfftb(int *, float *, float *), rfftf(int *, float *, float *),
    rfftbd(int *, double *, double *), rfftfd(int *, double *, double *),
    rffti(int *, float *), rfftid(int *, double *);
  int mode;
  loopInfo	srcinfo, trgtinfo;

  iq = ps[0];
  if (!symbolIsArray(iq))
    return cerror(NEED_ARR, ps[0]);

  if (isFunction) {		/* function form */
    /*float the input */
    if (fftdp == 0) {		/* single precision */
      iq = ana_float(1, ps);
      type = ANA_FLOAT;
    } else {			/* double precision */
      iq = ana_double(1, ps);
      type = ANA_DOUBLE;
    }
  } else {			/* subroutine form: data type must match */
    type = array_type(ps[0]);
    switch (array_type(ps[0])) {
      case ANA_FLOAT:
	if (fftdp) {
	  fftdp = 0;
	  puts("FFT - Changed !FFTDP to 0 to match FLOAT data type");
	}
	break;
      case ANA_DOUBLE:
	if (fftdp == 0) {
	  fftdp = 1;
	  puts("FFT - Changed !FFTDP to 1 to match DOUBLE data type");
	}
	break;
      default:
	return anaerror("Need FLOAT or DOUBLE data", ps[0]);
    }
  }      

  if (narg > 2 && ps[2])	/* have <direction> */
    direction = int_arg(ps[1]);
  else
    direction = (internalMode & 1)? -1: 1;

  mode = 0;
  jq = 0;
  if (narg > 1 && ps[1]) 	/* have <axes> */
    jq = ps[1];
  else if (internalMode & 2)	/* /ALL */
    mode = SL_ALLAXES;
  else				/* take the first axis */
    jq = ANA_ZERO;
  
  if (isFunction) {
    if (standardLoop(iq, jq, mode | SL_EACHROW | SL_UNIQUEAXES, type,
		     &srcinfo, &src, &result, &trgtinfo, &trgt) < 0)
      return ANA_ERROR;
    src0 = src;
    trgt0 = trgt;
  } else {
    if (standardLoop(iq, jq, mode | SL_EACHROW | SL_UNIQUEAXES, type,
		     &srcinfo, &src, NULL, NULL, NULL) < 0)
      return ANA_ERROR;
    trgtinfo = srcinfo;
    trgtinfo.data = &trgt;
    trgt0 = trgt = src0 = src;
    result = ANA_ONE;
  }

  doublePrecision = (fftdp || type == ANA_DOUBLE);
  work.b = otmp.b = NULL;
  do {
    trgt = trgt0;
    src = src0;
    n = srcinfo.rdims[0];	/* number of data elements in each FFT */
    mq = n*20 + 120;		/* scratch memory space */
    if (!doublePrecision)
      mq = mq/2;
    work.l = (int *) realloc(work.l, mq);
    if (!work.l) {
      zap(result);
      result = cerror(ALLOC_ERR, 0);
      break;
    }
    if (doublePrecision)
      rfftid(&n, work.d);	/* initialize scratch space */
    else
      rffti(&n, work.f);

    tmp.f = otmp.f = realloc(otmp.b, n*ana_type_size[doublePrecision?
						  ANA_DOUBLE: ANA_FLOAT]);
    if (tmp.f == NULL) {
      zap(result);
      result = cerror(ALLOC_ERR, 0);
      break;
    }

    step = srcinfo.step[0];	/* basic step size */
    if (doublePrecision) {
      switch (type) {
	case ANA_FLOAT:
	  if (direction < 0) { 	/* /BACK */
	    do {
	      tmp.d += n - 1;
	      if (n % 2) {
		tmp.d--;
		*tmp.d = 0.5**src.f; /* highest frequency cosine */
	      } else
		*tmp.d = *src.f;
	      tmp.d -= 2;
	      src.f += step;
	      for (j = 0; j < n/2 - 1; j++) {
		*tmp.d = 0.5**src.f;	/* cosines */
		tmp.d -= 2;
		src.f += step;
	      }
	      tmp.d[1] = *src.f; /* average */
	      src.f += step;
	      tmp.d += 3;
	      for (j = 0; j < (n - 1)/2; j++) {
		*tmp.d = -0.5**src.f;	/* sines */
		tmp.d += 2;
		src.f += step;
	      }
	      tmp.d = otmp.d;
	      rfftbd(&n, tmp.d, work.d);
	      for (j = 0; j < n; j++) {
		*trgt.f = *tmp.d++;
		trgt.f += step;
	      }
	      tmp.d = otmp.d;
	    } while (advanceLoops(&srcinfo, &trgtinfo) < srcinfo.rndim);
	  } else {			/* forward */
	    factor.d = 2.0/n;
	    do {
	      for (j = 0; j < n; j++) {
		*tmp.d++ = *src.f;
		src.f += step;
	      }
	      tmp.d = otmp.d;
	      rfftfd(&n, tmp.d, work.d);
	      tmp.d += n - 1 - (n % 2); /*  highest frequency cosine */
	      if (n % 2 == 0)
		tmp.d[0] *= 0.5;
	      for (j = 0; j < n/2; j++) {
		*trgt.f = *tmp.d *factor.d; /* the cosines */
		tmp.d -= 2;
		trgt.f += step;
	      }
	      *trgt.f = tmp.d[1] * 0.5 * factor.d; /* the average */
	      trgt.f += step;
	      tmp.d += 3;
	      for (j = 0; j < (n - 1)/2; j++) {
		*trgt.f = -*tmp.d *factor.d; /* the sines */
		tmp.d += 2;
		trgt.f += step;
	      }
	      tmp.d = otmp.d;
	    } while (advanceLoops(&srcinfo, &trgtinfo) < srcinfo.rndim);
	  }
	  break;
	case ANA_DOUBLE:
	  if (direction < 0) { 	/* /BACK */
	    do {
	      tmp.d += n - 1;
	      if (n % 2) {
		tmp.d--;
		*tmp.d = 0.5**src.d; /* highest frequency cosine */
	      } else
		*tmp.d = *src.d;
	      tmp.d -= 2;
	      src.d += step;
	      for (j = 0; j < n/2 - 1; j++) {
		*tmp.d = 0.5**src.d;	/* cosines */
		tmp.d -= 2;
		src.d += step;
	      }
	      tmp.d[1] = *src.d; /* average */
	      src.d += step;
	      tmp.d += 3;
	      for (j = 0; j < (n - 1)/2; j++) {
		*tmp.d = -0.5**src.d;	/* sines */
		tmp.d += 2;
		src.d += step;
	      }
	      tmp.d = otmp.d;
	      rfftbd(&n, tmp.d, work.d);
	      for (j = 0; j < n; j++) {
		*trgt.d = *tmp.d++;
		trgt.d += step;
	      }
	      tmp.d = otmp.d;
	    } while (advanceLoops(&srcinfo, &trgtinfo) < srcinfo.rndim);
	  } else {			/* forward */
	    factor.d = 2.0/n;
	    do {
	      for (j = 0; j < n; j++) {
		*tmp.d++ = *src.d;
		src.d += step;
	      }
	      tmp.d = otmp.d;
	      rfftfd(&n, tmp.d, work.d);
	      tmp.d += n - 1 - (n % 2); /*  highest frequency cosine */
	      if (n % 2 == 0)
		tmp.d[0] *= 0.5;
	      for (j = 0; j < n/2; j++) {
		*trgt.d = *tmp.d *factor.d; /* the cosines */
		tmp.d -= 2;
		trgt.d += step;
	      }
	      *trgt.d = tmp.d[1] * 0.5 * factor.d; /* the average */
	      trgt.d += step;
	      tmp.d += 3;
	      for (j = 0; j < (n - 1)/2; j++) {
		*trgt.d = -*tmp.d *factor.d; /* the sines */
		tmp.d += 2;
		trgt.d += step;
	      }
	      tmp.d = otmp.d;
	    } while (advanceLoops(&srcinfo, &trgtinfo) < srcinfo.rndim);
	  }
	  break;
      }
    } else {			/* single-precision calculations
				   must be ANA_FLOAT data */
      if (direction < 0) { 	/* /BACK */
	do {
	  tmp.f += n - 1;
	  if (n % 2) {
	    tmp.f--;
	    *tmp.f = 0.5**src.f; /* highest frequency cosine */
	  } else
	    *tmp.f = *src.f;
	  tmp.f -= 2;
	  src.f += step;
	  for (j = 0; j < n/2 - 1; j++) {
	    *tmp.f = 0.5**src.f;	/* cosines */
	    tmp.f -= 2;
	    src.f += step;
	  }
	  tmp.f[1] = *src.f; /* average */
	  src.f += step;
	  tmp.f += 3;
	  for (j = 0; j < (n - 1)/2; j++) {
	    *tmp.f = -0.5**src.f;	/* sines */
	    tmp.f += 2;
	    src.f += step;
	  }
	  tmp.f = otmp.f;
	  rfftb(&n, tmp.f, work.f);
	  for (j = 0; j < n; j++) {
	    *trgt.f = *tmp.f++;
	    trgt.f += step;
	  }
	  tmp.f = otmp.f;
	} while (advanceLoops(&srcinfo, &trgtinfo) < srcinfo.rndim);
      } else {			/* forward */
	factor.f = 2.0/n;
	do {
	  for (j = 0; j < n; j++) {
	    *tmp.f++ = *src.f;
	    src.f += step;
	  }
	  tmp.f = otmp.f;
	  rfftf(&n, tmp.f, work.f);
	  tmp.f += n - 1 - (n % 2); /*  highest frequency cosine */
	  if (n % 2 == 0)
	    tmp.f[0] *= 0.5;
	  for (j = 0; j < n/2; j++) {
	    *trgt.f = *tmp.f *factor.f; /* the cosines */
	    tmp.f -= 2;
	    trgt.f += step;
	  }
	  *trgt.f = tmp.f[1] * 0.5 * factor.f; /* the average */
	  trgt.f += step;
	  tmp.f += 3;
	  for (j = 0; j < (n - 1)/2; j++) {
	    *trgt.f = -*tmp.f *factor.f; /* the sines */
	    tmp.f += 2;
	    trgt.f += step;
	  }
	  tmp.f = otmp.f;
	} while (advanceLoops(&srcinfo, &trgtinfo) < srcinfo.rndim);
      }
    }
    src = trgt = src0 = trgt0;	/* target of last pass becomes source of
				 next one */
  } while (nextLoops(&srcinfo, &trgtinfo));
  
  free(otmp.b);
  free(work.f);
  return result;
}
/*------------------------------------------------------------------------- */
int single_fft(pointer src, int n, int type, int backwards)
{
  int	mq;
  static int	nsave = 0;
  static pointer	temp;

  if (!n) {			/* cleanup */
    free(temp.v);
    nsave = 0;
    return ANA_OK;
  }

  mq = (type == ANA_DOUBLE)? n*20 + 120: n*10 + 60;
  if (n != nsave) {
    temp.v = realloc(temp.v, mq);
    if (!temp.v)
      return cerror(ALLOC_ERR, 0);
  }

  switch (type) {
    case ANA_FLOAT:
      if (n != nsave) {
	rffti(&n, temp.f);
	nsave = n;
      }
      if (backwards)
	rfftb(&n, src.f, temp.f);
      else
	rfftf(&n, src.f, temp.f);
      break;
    case ANA_DOUBLE:
      if (n != nsave) {
	rfftid(&n, temp.d);
	nsave = n;
      }
      if (backwards)
	rfftbd(&n, src.d, temp.d);
      else
	rfftfd(&n, src.d, temp.d);
      break;
    default:
      return cerror(ILL_TYPE, 0);
  }
  return ANA_OK;
}
/*------------------------------------------------------------------------- */
int cfft(int narg, int ps[], int isFunction)
/* This routine calculates the fourier transform of a complex data set. */
/* y = FFTC(x [, axes, DIRECTION=<direction>]) */
/* y = FFTC(x) and y = FFTC(x,DIRECTION=+1) calculate the forward, and */
/* y = FFTC(x,/back) and y = FFTC(x,DIRECTION=-1) the backward transform. */
{
  int	iq, n, mq, type, j, jq, direction, result, step, doublePrecision;
  pointer	src, trgt, work, tmp, otmp, src0, trgt0;
  scalar	factor;
  int	cfftb(int *, float *, float *), cfftf(int *, float *, float *),
    cffti(int *, float *), cfftid(int *, double *),
    cfftbd(int *, double *, double *), cfftfd(int *, double *, double *);
  loopInfo	srcinfo, trgtinfo;
  int mode;

  iq = ps[0];
  if (!symbolIsArray(iq))
    return cerror(NEED_ARR, ps[0]);

  if (isFunction) {		/* function form */
    /*float the input */
    if (fftdp == 0) {		/* single precision */
      iq = ana_cfloat(1, ps);
      type = ANA_CFLOAT;
    } else {			/* double precision */
      iq = ana_cdouble(1, ps);
      type = ANA_CDOUBLE;
    }
  } else {			/* subroutine form: data type must match */
    type = array_type(ps[0]);
    switch (array_type(ps[0])) {
      case ANA_CFLOAT:
	if (fftdp) {
	  fftdp = 0;
	  puts("FFT - Changed !FFTDP to 0 to match CFLOAT data type");
	}
	break;
      case ANA_CDOUBLE:
	if (fftdp == 0) {
	  fftdp = 1;
	  puts("FFT - Changed !FFTDP to 1 to match CDOUBLE data type");
	}
	break;
      default:
	return anaerror("Need CFLOAT or CDOUBLE data", ps[0]);
    }
  }      

  if (narg > 2 && ps[2] && symbol_class(ps[2]) != ANA_UNDEFINED) /* have <direction> */
    direction = int_arg(ps[1]);
  else
    direction = (internalMode & 1)? -1: 1;

  mode = 0;
  jq = 0;
  if (narg > 1 && ps[1] && symbol_class(ps[1]) != ANA_UNDEFINED) /* have <axes> */
    jq = ps[1];
  else if (internalMode & 2)	/* /ALL */
    mode = SL_ALLAXES;
  else				/* take the first axis */
    jq = ANA_ZERO;
  
  if (isFunction) {
    if (standardLoop(iq, jq, mode | SL_EACHROW | SL_UNIQUEAXES, type,
		     &srcinfo, &src, &result, &trgtinfo, &trgt) < 0)
      return ANA_ERROR;
    src0 = src;
    trgt0 = trgt;
  } else {
    if (standardLoop(iq, jq, mode | SL_EACHROW | SL_UNIQUEAXES, type,
		     &srcinfo, &src, NULL, NULL, NULL) < 0)
      return ANA_ERROR;
    trgtinfo = srcinfo;
    trgtinfo.data = &trgt;
    trgt0 = trgt = src0 = src;
    result = ANA_ONE;
  }

  doublePrecision = (fftdp || type == ANA_CDOUBLE);
  work.f = otmp.f = NULL;
  do {
    trgt = trgt0;
    src = src0;
    n = srcinfo.rdims[0];	/* number of data elements in each FFT */
    mq = n*64 + 240;		/* scratch memory space */
    if (!doublePrecision)
      mq = mq/2;
    work.l = (int *) realloc(work.l, mq);
    if (!work.l) {
      zap(result);
      result = cerror(ALLOC_ERR, 0);
      break;
    }
    if (doublePrecision)
      cfftid(&n, work.d);	/* initialize scratch space */
    else
      cffti(&n, work.f);

    tmp.f = otmp.f = realloc(otmp.b, n*ana_type_size[type]);
    if (tmp.f == NULL) {
      zap(result);
      result = cerror(ALLOC_ERR, 0);
      break;
    }

    step = srcinfo.step[0];	/* basic step size */
    if (doublePrecision) {
      switch (type) {
	case ANA_CFLOAT:
	  if (direction < 0) { 	/* /BACK */
	    do {
	      tmp.cd += n/2;
	      for (j = n/2; j < n; j++) {
		*tmp.d++ = (double) src.cf->real;
		*tmp.d++ = (double) src.cf->imaginary;
		src.cf += step;
	      }
	      tmp.cd = otmp.cd;
	      for (j = 0; j < n/2; j++) {
		*tmp.d++ = (double) src.cf->real;
		*tmp.d++ = (double) src.cf->imaginary;
		src.cf += step;
	      }
	      tmp.cd = otmp.cd;
	      cfftbd(&n, tmp.d, work.d);
	      for (j = 0; j < n; j++) {
		trgt.cf->real = (float) *tmp.d++;
		trgt.cf->imaginary = (float) *tmp.d++;
		trgt.cf += step;
	      }
	      tmp.cd = otmp.cd;
	    } while (advanceLoops(&srcinfo, &trgtinfo) < srcinfo.rndim);
	  } else {			/* forward */
	    factor.d = 1.0/n;
	    do {
	      for (j = 0; j < n; j++) {
		*tmp.d++ = (double) src.cf->real;
		*tmp.d++ = (double) src.cf->imaginary;
		src.cf += step;
	      }
	      tmp.cd = otmp.cd;
	      cfftfd(&n, tmp.d, work.d);
	      tmp.cd += n/2;
	      for (j = n/2; j < n; j++) {
		trgt.cf->real = *tmp.d++*factor.d;
		trgt.cf->imaginary = *tmp.d++*factor.d;
		trgt.cf += step;
	      }	    
	      tmp.cd = otmp.cd;
	      for (j = 0; j < n/2; j++) {
		trgt.cf->real = *tmp.d++*factor.d;
		trgt.cf->imaginary = *tmp.d++*factor.d;
		trgt.cf += step;
	      }	    
	      tmp.cd = otmp.cd;
	    } while (advanceLoops(&srcinfo, &trgtinfo) < srcinfo.rndim);
	  }
	  break;
	case ANA_CDOUBLE:
	  if (direction < 0) { 	/* /BACK */
	    do {
	      tmp.cd += n/2;
	      for (j = n/2; j < n; j++) {
		*tmp.d++ = src.cd->real;
		*tmp.d++ = src.cd->imaginary;
		src.cd += step;
	      }
	      tmp.cd = otmp.cd;
	      for (j = 0; j < n/2; j++) {
		*tmp.d++ = src.cd->real;
		*tmp.d++ = src.cd->imaginary;
		src.cd += step;
	      }
	      tmp.cd = otmp.cd;
	      cfftbd(&n, tmp.d, work.d);
	      for (j = 0; j < n; j++) {
		trgt.cd->real = *tmp.d++;
		trgt.cd->imaginary = *tmp.d++;
		trgt.cd += step;
	      }
	      tmp.cd = otmp.cd;
	    } while (advanceLoops(&srcinfo, &trgtinfo) < srcinfo.rndim);
	  } else {			/* forward */
	    factor.d = 1.0/n;
	    do {
	      for (j = 0; j < n; j++) {
		*tmp.d++ = src.cd->real;
		*tmp.d++ = src.cd->imaginary;
		src.cd += step;
	      }
	      tmp.cd = otmp.cd;
	      cfftfd(&n, tmp.d, work.d);
	      tmp.cd += n/2;
	      for (j = n/2; j < n; j++) {
		trgt.cd->real = *tmp.d++*factor.d;
		trgt.cd->imaginary = *tmp.d++*factor.d;
		trgt.cd += step;
	      }	    
	      tmp.cd = otmp.cd;
	      for (j = 0; j < n/2; j++) {
		trgt.cd->real = *tmp.d++*factor.d;
		trgt.cd->imaginary = *tmp.d++*factor.d;
		trgt.cd += step;
	      }	    
	      tmp.cd = otmp.cd;
	    } while (advanceLoops(&srcinfo, &trgtinfo) < srcinfo.rndim);
	  }
	  break;
      }
    } else {			/* single precision calculations;
				   must be ANA_CFLOAT */
      if (direction < 0) { 	/* /BACK */
	do {
	  tmp.cf += n/2;
	  for (j = n/2; j < n; j++) {
	    *tmp.f++ = src.cf->real;
	    *tmp.f++ = src.cf->imaginary;
	    src.cf += step;
	  }
	  tmp.cf = otmp.cf;
	  for (j = 0; j < n/2; j++) {
	    *tmp.f++ = src.cf->real;
	    *tmp.f++ = src.cf->imaginary;
	    src.cf += step;
	  }
	  tmp.cf = otmp.cf;
	  cfftb(&n, tmp.f, work.f);
	  for (j = 0; j < n; j++) {
	    trgt.cf->real = *tmp.f++;
	    trgt.cf->imaginary = *tmp.f++;
	    trgt.cf += step;
	  }
	  tmp.cf = otmp.cf;
	} while (advanceLoops(&srcinfo, &trgtinfo) < srcinfo.rndim);
      } else {			/* forward */
	factor.f = 1.0/n;
	do {
	  for (j = 0; j < n; j++) {
	    *tmp.f++ = src.cf->real;
	    *tmp.f++ = src.cf->imaginary;
	    src.cf += step;
	  }
	  tmp.cf = otmp.cf;
	  cfftf(&n, tmp.f, work.f);
	  tmp.cf += n/2;
	  for (j = n/2; j < n; j++) {
	    trgt.cf->real = *tmp.f++*factor.f;
	    trgt.cf->imaginary = *tmp.f++*factor.f;
	    trgt.cf += step;
	  }	    
	  tmp.cf = otmp.cf;
	  for (j = 0; j < n/2; j++) {
	    trgt.cf->real = *tmp.f++*factor.f;
	    trgt.cf->imaginary = *tmp.f++*factor.f;
	    trgt.cf += step;
	  }	    
	  tmp.cf = otmp.cf;
	} while (advanceLoops(&srcinfo, &trgtinfo) < srcinfo.rndim);
      }
    }
    src = trgt = src0 = trgt0;	/* target of last pass becomes source of
				 next one */
  } while (nextLoops(&srcinfo, &trgtinfo));

  free(otmp.b);
  free(work.f);
  return result;
}
/*------------------------------------------------------------------------- */
int ana_fft_f(int narg, int ps[])
{
  if (isComplexType(symbol_type(ps[0])))
    return cfft(narg, ps, 1);
  else
    return fft(narg, ps, 1);
}
/*------------------------------------------------------------------------- */
int ana_fft(int narg, int ps[])
{
  if (isComplexType(symbol_type(ps[0])))
    return cfft(narg, ps, 0);
  else
    return fft(narg, ps, 0);
}
/*------------------------------------------------------------------------- */
int ana_sccomplex(int narg, int ps[])
/* SCCOMPLEX(data [, /TOCOMPLEX, /TOSC])
   transforms <data> between the sine-cosine fft and complex fft
   representations.  If /TOCOMPLEX is specified, then <data> is assumed
   to be in sine-cosine representation and is transformed to a complex
   representation.  A dimension of 2 is added at the beginning of <data>'s
   dimensions: the first index refers to the real part, and the second
   index to the imaginary part of each number.

   If /TOSC is specified, and if the first dimension of <data> is equal to
   2, then <data> is assumed to be in complex representation as described
   above, and is transformed into sine-cosine representation.  If /TOSC
   is specified but the first dimension of <data> is not equal to 2, then
   an error is generated.

   If neither /TOCOMPLEX nor /TOSC is specified, then /TOCOMPLEX is
   assumed if the first dimension is unequal to 2, and /TOSC otherwise.

   LS 20jul98
*/
{
  int type, result, coords[MAX_DIMS], acoords[MAX_DIMS], i, ntemp,
    center, indx, k, sign[MAX_DIMS], j, n, c, nodata, direction,
    index1, index2;
  scalar	s;
  pointer	src, trgt, temp, src0;
  loopInfo	srcinfo, trgtinfo;

  if (!symbolIsArray(ps[0]))
    return cerror(NEED_ARR, ps[0]); /* need array */
  type = array_type(ps[0]);
  if (type < ANA_FLOAT)
    type = ANA_FLOAT;		/* data type must be at least FLOAT */

  switch (internalMode & 3) {
    case 1:			/* /TOCOMPLEX */
      direction = +1;
      break;
    case 2:			/* /TOSC */
      direction = -1;
      break;
    default:
      if (array_dims(ps[0])[0] == 2)
	direction = -1;		/* assume /TOSC */
      else
	direction = +1;		/* assume /TOCOMPLEX */
  }

  if (direction == +1) {	/* /TOCOMPLEX */
    /* Strategy:

       A complex FFT component is equal to the cosine component plus or
       minus the sine component times the imaginary unit.  The sign of
       the multiplication factor for the sine component is equal to the
       sign of the frequency.  We combine each corresponding cosine and
       sine component appropriately into a complex component, repeating
       the process for each dimension.

       We maintain the frequency scheme of the sine/cosine pattern
       (i.e., the one generated by the FFT function: the all-zero
       frequency corresponds to all subscripts equal to the size of the
       corresponding dimension, divided by 2), with negative frequencies
       before, and positive frequencies after the all-zero frequency.
       
       Complication arise in two cases: (1) If the frequency is equal
       to zero in some dimension, then there is no sine component
       (i.e., no imaginary component) corresponding to the cosine
       component in that dimension.  (2) If the frequency is maximally
       negative in some dimension and the size of that dimension is
       even, then there is no sine component corresponding to the
       cosine component in that dimension.  In both of these cases,
       the cosine component should be multiplied by 2 for every
       dimension in which these cases arise.  For normalization
       purposes, the results are divided by 2^(number-of-dimensions).
       After this, the sum of each coefficient times the corresponding
       Fourier basis function is equal to the original data.
              
       The sine/cosine components that contribute to a particular
       complex component all have the same absolute coordinates relative
       to the all-zero frequency position: only the signs of the
       relative coordinates vary from sine/cosine component to component.
       
       Examples in one dimension:
         sine/cosine   -->         complex
           1  2  3         (1 - 3i)/2  2  (1 + 3i)/2

           1  2  3  4      1  (2 - 4i)/2  3  (2 + 4i)/2

       Example in two dimensions:
         sine/cosine       
           1  2  3         
           4  5  6         
           7  8  9         

       First we treat dimension 0:
        (1 - 3i)/4  2/2  (1 + 3i)/4
        (4 - 6i)/4  5    (4 + 6i)/4
        (7 - 9i)/4  8/2  (7 + 9i)/4

       Then dimension 1:
                            complex
       ((1 - 3i) - (7 - 9i)i)/4  (2 - 8i)/2  ((1 + 3i) - (7 + 9i)i)/4
       (4 - 6i)/2                5           (4 + 6i)/2
       ((1 - 3i) + (7 - 9i)i)/4  (2 + 8i)/2  ((1 + 3i) + (7 + 9i)i)/4

       which is equal to
         -2 - 2.5i  1 - 4i  2.5 -    i
          2 -   3i  5         2 +   3i
        2.5 +    i  1 + 4i   -2 + 2.5i
    */

    if (standardLoop(ps[0], 0,
		     SL_ALLAXES | SL_EACHCOORD | SL_SRCUPGRADE | SL_UPGRADE,
		     type, &srcinfo, &src, NULL, NULL, NULL) == ANA_ERROR)
      return ANA_ERROR;
    src0 = src;

    /* calculate the offset of the center (zero frequency) position */
    center = 0;
    for (i = 0; i < srcinfo.ndim; i++)
      center += (srcinfo.dims[i]/2)*srcinfo.singlestep[i];
    
    /* calculate # data values on which each Fourier component depends */
    ntemp = 2;
    for (i = 1; i < srcinfo.ndim; i++)
      ntemp *= 2;			/* calculate 2^ndim */
    
    /* reserve space for intermediate results */
    temp.b = malloc(ntemp*ana_type_size[type]);
    if (temp.b == NULL)
      return cerror(ALLOC_ERR, 0);

    /* create result symbol: add a dimension of 2 at the beginning */
    trgtinfo.dims[0] = 2;
    memcpy(trgtinfo.dims + 1, srcinfo.dims, srcinfo.ndim*sizeof(int));
    trgtinfo.ndim = srcinfo.ndim + 1;
    result = array_scratch(type, trgtinfo.ndim, trgtinfo.dims);
    if (result == ANA_ERROR) {
      free(temp.b);
      return ANA_ERROR;
    }
    trgt.f = (float *) array_data(result);
    setupDimensionLoop(&trgtinfo, trgtinfo.ndim, trgtinfo.dims, type,
		       trgtinfo.ndim, NULL, &trgt, SL_EACHCOORD | SL_EACHROW);

    switch (type) {
      case ANA_FLOAT:
	do {
	  /* we calculate the coordinates relative to the all-zero frequency */
	  for (i = 0; i < srcinfo.ndim; i++) {
	    c = srcinfo.coords[i] - srcinfo.rdims[i]/2;	/* relative freq. */
	    sign[i] = (c < 0)? -1: (c > 0)? +1: 0; /* get its sign */
	    acoords[i] = c*sign[i]; /* and absolute value, too */
	  }
	  
	  /* initialize for extraction of proper cosine/sine components */
	  indx = 0;		/* initialize index offset for calculation
				   of first complex component */
	  for (i = 0; i < srcinfo.ndim; i++) {
	    coords[i] = -1;	/* sign of current offset for this component */
	    indx -= srcinfo.singlestep[i]*acoords[i];
	  }

	  s.f = 1.0/ntemp;	/* product of all imaginary units
				 for the current component, + normalization */
	  for (i = 0; i < ntemp; i++) {
	    n = 1;
	    nodata = 0;		/* initialize: 0 -> value available;
				 > 0 -> no value available */
	    for (k = 0; k < srcinfo.ndim; k++) /* check all dimensions */
	      if (!acoords[k] /* in center of this dimension */
		  || (!srcinfo.coords[k] /* or at beginning */
		      && srcinfo.dims[k] % 2 == 0)) { /* and even dim */
		n *= 2;
		if ((i >> k) % 2) /* imaginary part */
		  nodata = 1;
	      }
	    temp.f[i] = nodata? 0.0: src0.f[center + indx]*s.f*n; /* value */
	    for (k = 0; k < srcinfo.ndim; k++) { /* advance to next
						  sine/cosine component */
	      coords[k] = -coords[k]; /* sign of frequency offset changes */
	      indx += coords[k]*srcinfo.singlestep[k]*2*acoords[k];
	      if (sign[k] < 0)
		s.f = -s.f;	/* multiply one more imaginary unit */
	      if (coords[k] > 0)
		break;		/* done updating */
	    }
	  }
	  
	  /* combine the individual real and imaginary parts into one
	     real and one imaginary part */
	  for (n = ntemp/4; n; n /= 2) {
	    j = 0;
	    for (i = 0; i < n; ) {
	      temp.f[i++] = temp.f[j] - temp.f[j + 3]; /* real part */
	      temp.f[i++] = temp.f[j + 1] + temp.f[j + 2]; /* imaginary part */
	      j += 4;
	    }
	  }  
	  *trgt.f++ = temp.f[0]; /* store complex component */
	  *trgt.f++ = temp.f[1];
	  advanceLoop(&trgtinfo);
	} while (advanceLoop(&srcinfo) < srcinfo.ndim);
	break;
      case ANA_DOUBLE:
	do {
	  /* we calculate the coordinates relative to the all-zero frequency */
	  for (i = 0; i < srcinfo.ndim; i++) {
	    c = srcinfo.coords[i] - srcinfo.rdims[i]/2;	/* relative freq. */
	    sign[i] = (c < 0)? -1: (c > 0)? +1: 0; /* get its sign */
	    acoords[i] = c*sign[i]; /* and absolute value, too */
	  }
	  
	  /* initialize for extraction of proper cosine/sine components */
	  indx = 0;		/* initialize index offset for calculation
				   of first complex component */
	  for (i = 0; i < srcinfo.ndim; i++) {
	    coords[i] = -1;	/* sign of current offset for this component */
	    indx -= srcinfo.singlestep[i]*acoords[i];
	  }

	  s.d = (double) 1.0/ntemp;	/* product of all imaginary units
				 for the current component, + normalization */
	  for (i = 0; i < ntemp; i++) {
	    n = 1;
	    nodata = 0;		/* initialize: 0 -> value available;
				 > 0 -> no value available */
	    for (k = 0; k < srcinfo.ndim; k++) /* check all dimensions */
	      if (!acoords[k] /* in center of this dimension */
		  || (!srcinfo.coords[k] /* or at beginning */
		      && srcinfo.dims[k] % 2 == 0)) { /* and even dim */
		n *= 2;
		if ((i >> k) % 2) /* imaginary part */
		  nodata = 1;
	      }
	    temp.d[i] = nodata? 0.0: src0.d[center + indx]*s.d*n; /* value */
	    for (k = 0; k < srcinfo.ndim; k++) { /* advance to next
						  sine/cosine component */
	      coords[k] = -coords[k]; /* sign of frequency offset changes */
	      indx += coords[k]*srcinfo.singlestep[k]*2*acoords[k];
	      if (sign[k] < 0)
		s.d = -s.d;		/* multiply one more imaginary unit */
	      if (coords[k] > 0)
		break;		/* done updating */
	    }
	  }
	  
	  /* combine the individual real and imaginary parts into one
	     real and one imaginary part */
	  for (n = ntemp/4; n; n /= 2) {
	    j = 0;
	    for (i = 0; i < n; ) {
	      temp.d[i++] = temp.d[j] - temp.d[j + 3]; /* real part */
	      temp.d[i++] = temp.d[j + 1] + temp.d[j + 2]; /* imaginary part */
	      j += 4;
	    }
	  }  
	  *trgt.d++ = temp.d[0]; /* store complex component */
	  *trgt.d++ = temp.d[1];
	  advanceLoop(&trgtinfo);
	} while (advanceLoop(&srcinfo) < srcinfo.ndim);
	break;
    }
  } else {			/* /TOSC */

    if (standardLoop(ps[0], 0,
		     SL_ALLAXES | SL_EACHCOORD | SL_SRCUPGRADE | SL_UPGRADE
		     | SL_AXESBLOCK, type, &srcinfo, &src,
		     NULL, NULL, NULL) == ANA_ERROR)
      return ANA_ERROR;
    src0 = src;
    
    /* in walking through the array, we ignore the first dimension: */
    /* we take care of that one manually. */
    memmove(srcinfo.axes, srcinfo.axes + 1, --srcinfo.naxes*sizeof(int));
    rearrangeDimensionLoop(&srcinfo);
    srcinfo.advanceaxis = 1;	/* we take care of the active one */

    /* calculate the offset of the center (zero frequency) position */
    center = 0;
    for (i = 0; i < srcinfo.naxes; i++)
      center += (srcinfo.rdims[i]/2)*srcinfo.rsinglestep[i];

    /* create result symbol */
    result = array_clone(ps[0], type);
    if (result == ANA_ERROR)
      return ANA_ERROR;
    trgt.f = (float *) array_data(result);
    setupDimensionLoop(&trgtinfo, srcinfo.ndim, srcinfo.dims, type,
		       srcinfo.ndim, NULL, &trgt, SL_EACHCOORD | SL_EACHROW);

    temp.b = malloc(4*ana_type_size[type]);
    if (!temp.b)
      return cerror(ALLOC_ERR, 0);
    
    /* Strategy:

       A particular cosine component is equal to the sum of the
       complex components for positive and negative frequencies.  A
       particular sine component is equal to the difference between
       the complex components for positive and negative frequencies,
       divided by the imaginary unit.  At frequency zero, the cosine
       component is equal to the complex component, and there is no
       sine component.

       In principle, the complex spectrum may not derive from strictly
       real data, so we calculate the sine/cosine representation of the
       real and imaginary parts separately.

       Examples in one dimension:

       0.5 - 1.5i  2  0.5 + 1.5i

       --> (0.5 - 1.5i) + (0.5 + 1.5i)   2   ((0.5 + 1.5i) - (0.5 - 1.5i))/i
       =    1  2  3

       1  1 - 2i  3  1 + 2i 

       -->  1  (1 - 2i) + (1 + 2i)  3  ((1 + 2i) - (1 - 2i))/i
       =    1  2  3  4

       Example in two dimensions:

         -2 -2.5i  1 - 4i  2.5 -    i
          2 -  3i  5         2 +   3i
        2.5 +   i  1 + 4i   -2 + 2.5i

       First treat dimension 0:
        0.5 - 3.5i  1 - 4i  3.5 - 4.5i
          4         5       6
        0.5 + 3.5i  1 + 4i  3.5 + 4.5i

        Then dimension 1:
          1  2  3
          4  5  6
          7  8  9
     */

    switch (type) {
      case ANA_FLOAT:
	do {
	  do {
	    for (j = 0; j <= srcinfo.rdims[0]/2; j++) {

	      indx = 0;		/* initialize index offset for calculation
				   of first complex component */
	      srcinfo.coords[0] = j;
	      indx = 0;
	      for (i = 0; i < srcinfo.naxes; i++) {
		acoords[i] = srcinfo.coords[i] - srcinfo.rdims[i]/2;
		indx += srcinfo.rsinglestep[i]*acoords[i];
	      }
	      
	      index1 = center + indx;
	      if (!acoords[0]
		  || (!j && srcinfo.rdims[0] % 2 == 0)) {
		/* have no separate positive/negative-frequency values */
		trgt.f[index1] = src0.f[index1];
		trgt.f[index1 + 1] = src0.f[index1 + 1];
	      } else {
		index2 = index1 - 2*srcinfo.rsinglestep[0]*acoords[0];
		temp.f[0] = src0.f[index1] + src0.f[index2];
		temp.f[1] = src0.f[index1 + 1] + src0.f[index2 + 1];
		temp.f[2] = src0.f[index2 + 1] - src0.f[index1 + 1];
		temp.f[3] = src0.f[index1] - src0.f[index2];
		trgt.f[index1] = temp.f[0];
		trgt.f[index1 + 1] = temp.f[1];
		trgt.f[index2] = temp.f[2];
		trgt.f[index2 + 1] = temp.f[3];
	      }
	    }
	  } while (advanceLoop(&srcinfo) < srcinfo.naxes);
	  src0 = trgt;		/* previous target is new source */
	} while (nextLoop(&srcinfo));
	break;
      case ANA_DOUBLE:
	do {
	  do {
	    for (j = 0; j <= srcinfo.rdims[0]/2; j++) {

	      indx = 0;		/* initialize index offset for calculation
				   of first complex component */
	      srcinfo.coords[0] = j;
	      indx = 0;
	      for (i = 0; i < srcinfo.naxes; i++) {
		acoords[i] = srcinfo.coords[i] - srcinfo.rdims[i]/2;
		indx += srcinfo.rsinglestep[i]*acoords[i];
	      }
	      
	      index1 = center + indx;
	      if (!acoords[0]
		  || (!j && srcinfo.rdims[0] % 2 == 0)) {
		/* have no separate positive/negative-frequency values */
		trgt.d[index1] = src0.d[index1];
		trgt.d[index1 + 1] = src0.d[index1 + 1];
	      } else {
		index2 = index1 - 2*srcinfo.rsinglestep[0]*acoords[0];
		temp.d[0] = src0.d[index1] + src0.d[index2];
		temp.d[1] = src0.d[index1 + 1] + src0.d[index2 + 1];
		temp.d[2] = src0.d[index2 + 1] - src0.d[index1 + 1];
		temp.d[3] = src0.d[index1] - src0.d[index2];
		trgt.d[index1] = temp.d[0];
		trgt.d[index1 + 1] = temp.d[1];
		trgt.d[index2] = temp.d[2];
		trgt.d[index2 + 1] = temp.d[3];
	      }
	    }
	  } while (advanceLoop(&srcinfo) < srcinfo.naxes);
	  src0 = trgt;		/* previous target is new source */
	} while (nextLoop(&srcinfo));
	break;
    }
  }

  free(temp.b);
  return result;
}
/*------------------------------------------------------------------------- */
int ana_real(int narg, int ps[])
/* returns the real part of the argument.  LS 17nov98 */
{
  int	iq, outtype, result, n;
  pointer	value, trgt;

  if (!symbolIsNumerical(ps[0]))
    return cerror(ILL_CLASS, ps[0]);
  if (!isComplexType(symbol_type(ps[0])))
    return ps[0];		/* nothing to do */

  outtype = (symbol_type(ps[0]) == ANA_CFLOAT)? ANA_FLOAT: ANA_DOUBLE;
  
  iq = ps[0];
  switch (symbol_class(iq)) {
    case ANA_SCAL_PTR:
      value.cf = scal_ptr_pointer(iq).cf;
      result = scalar_scratch(outtype);
      trgt.b = &scalar_value(result).b;
      n = 1;
      break;
    case ANA_CSCALAR:
      value.cf = complex_scalar_data(iq).cf;
      result = scalar_scratch(outtype);
      trgt.b = &scalar_value(result).b;
      n = 1;
      break;
    case ANA_CARRAY:
      value.v = array_data(iq);
      result = array_clone(iq, outtype);
      trgt.v = array_data(result);
      n = array_size(iq);
      break;
  }

  switch (outtype) {
    case ANA_FLOAT:
      while (n--)
	*trgt.f++ = value.cf++->real;
      break;
    case ANA_DOUBLE:
      while (n--)
	*trgt.d++ = value.cd++->real;
      break;
  }
  return result;
}
/*------------------------------------------------------------------------- */
int ana_imaginary(int narg, int ps[])
/* returns the imaginary part of the argument.  LS 17nov98 */
{
  int	iq, outtype, result, n;
  pointer	value, trgt;

  if (!symbolIsNumerical(ps[0]))
    return cerror(ILL_CLASS, ps[0]);
  if (!isComplexType(symbol_type(ps[0])))
    return ana_zero(1, &ps[0]);	/* no imaginary part */

  outtype = (symbol_type(ps[0]) == ANA_CFLOAT)? ANA_FLOAT: ANA_DOUBLE;
  
  iq = ps[0];
  switch (symbol_class(iq)) {
    case ANA_SCAL_PTR:
      value.cf = scal_ptr_pointer(iq).cf;
      result = scalar_scratch(outtype);
      trgt.b = &scalar_value(result).b;
      n = 1;
      break;
    case ANA_CSCALAR:
      value.cf = complex_scalar_data(iq).cf;
      result = scalar_scratch(outtype);
      trgt.b = &scalar_value(result).b;
      n = 1;
      break;
    case ANA_CARRAY:
      value.v = array_data(iq);
      result = array_clone(iq, outtype);
      trgt.v = array_data(result);
      n = array_size(iq);
      break;
  }

  switch (outtype) {
    case ANA_FLOAT:
      while (n--)
	*trgt.f++ = value.cf++->imaginary;
      break;
    case ANA_DOUBLE:
      while (n--)
	*trgt.d++ = value.cd++->imaginary;
      break;
  }
  return result;
}
/*------------------------------------------------------------------------- */
int ana_arg(int narg, int ps[])
/* returns the complex argument of the argument.  LS 17nov98 */
{
  int	iq, outtype, result, type, n;
  pointer	value, trgt;

  if (!symbolIsNumerical(ps[0]))
    return cerror(ILL_CLASS, ps[0]);

  type = symbol_type(ps[0]);
  outtype = (type == ANA_DOUBLE || symbol_type(ps[0]) == ANA_CDOUBLE)?
    ANA_DOUBLE: ANA_FLOAT;
  
  iq = ps[0];
  switch (symbol_class(iq)) {
    case ANA_SCAL_PTR:
      value.cf = scal_ptr_pointer(iq).cf;
      result = scalar_scratch(outtype);
      trgt.b = &scalar_value(result).b;
      n = 1;
      break;
    case ANA_SCALAR:
      value.b = &scalar_value(iq).b;
      result = scalar_scratch(outtype);
      trgt.b = &scalar_value(result).b;
      n = 1;
      break;
    case ANA_CSCALAR:
      value.cf = complex_scalar_data(iq).cf;
      result = scalar_scratch(outtype);
      trgt.b = &scalar_value(result).b;
      n = 1;
      break;
    case ANA_CARRAY: case ANA_ARRAY:
      value.v = array_data(iq);
      result = array_clone(iq, outtype);
      trgt.v = array_data(result);
      n = array_size(iq);
      break;
  }

  switch (outtype) {
    case ANA_FLOAT:
      switch (type) {
	case ANA_BYTE:
	  while (n--)
	    *trgt.f++ = 0.0;
	  break;
	case ANA_WORD:
	  while (n--)
	    *trgt.f++ = (*value.w++ >= 0)? 0.0: PI;
	  break;
	case ANA_LONG:
	  while (n--)
	    *trgt.f++ = (*value.l++ >= 0)? 0.0: PI;
	  break;
	case ANA_FLOAT:
	  while (n--)
	    *trgt.f++ = (*value.f++ >= 0)? 0.0: PI;
	  break;
	case ANA_CFLOAT:
	  while (n--) {
	    *trgt.f++ = atan2(value.cf->imaginary, value.cf->real);
	    value.cf++;
	  }
	  break;
      }
      break;
    case ANA_DOUBLE:
      switch (type) {
	case ANA_BYTE:
	  while (n--)
	    *trgt.d++ = 0.0;
	  break;
	case ANA_WORD:
	  while (n--)
	    *trgt.d++ = (*value.w++ >= 0)? 0.0: PI;
	  break;
	case ANA_LONG:
	  while (n--)
	    *trgt.d++ = (*value.l++ >= 0)? 0.0: PI;
	  break;
	case ANA_FLOAT:
	  while (n--)
	    *trgt.d++ = (*value.f++ >= 0)? 0.0: PI;
	  break;
	case ANA_DOUBLE:
	  while (n--)
	    *trgt.d++ = (*value.d++ >= 0)? 0.0: PI;
	  break;
	case ANA_CDOUBLE:
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
int ana_complex(int narg, int ps[])
/* returns a complex version of the argument */
{
  if (!symbolIsNumerical(ps[0]))
    return cerror(ILL_CLASS, ps[0]);
  if (isComplexType(symbol_type(ps[0])))
    return ps[0];		/* already done */
  return (symbol_type(ps[0]) == ANA_DOUBLE)?
    ana_cdouble(1, ps): ana_cfloat(1, ps);
}
/*------------------------------------------------------------------------- */
int ana_hilbert(int narg, int ps[])
/* This routine calculates the Hilbert transform of a real data set. */
/* y = HILBERT(x [, axes, DIRECTION=<direction>]) */
/* y = HILBERT(x) and y = HILBERT(x,DIRECTION=+1) calculate the forward, and */
/* y = HILBERT(x,/back) and y = HILBERT(x,DIRECTION=-1) the backward */
/* transform.  LS 10jun98 */
{
  int	iq, n, mq, type, j, jq, direction, result, step;
  pointer	src, trgt, work, tmp, otmp, src0, trgt0;
  scalar	v, factor;
  int	rfftb(int *, float *, float *), rfftf(int *, float *, float *),
    rfftbd(int *, double *, double *), rfftfd(int *, double *, double *),
    rffti(int *, float *), rfftid(int *, double *);
  loopInfo	srcinfo, trgtinfo;
  int mode;

  iq = ps[0];
  if (!symbolIsArray(iq))
    return cerror(NEED_ARR, ps[0]);

  /*float the input */
  if (fftdp == 0) {		/* single precision */
    iq = ana_float(1, ps);
    type = ANA_FLOAT;
  } else {			/* double precision */
    iq = ana_double(1, ps);
    type = ANA_DOUBLE;
  }

  if (narg > 2 && ps[2])	/* have <direction> */
    direction = int_arg(ps[2]);
  else
    direction = (internalMode & 1)? -1: 1;

  mode = 0;
  jq = 0;
  if (narg > 1 && ps[1]) 	/* have <axes> */
    jq = ps[1];
  else if (internalMode & 2)	/* /ALL */
    mode = SL_ALLAXES;
  else				/* take the first axis */
    jq = ANA_ZERO;
  
  if (standardLoop(iq, jq, mode | SL_EACHROW | SL_UNIQUEAXES, type,
		   &srcinfo, &src, &result, &trgtinfo, &trgt) < 0)
    /* restore dimensional structure of iq, if necessary */
    return ANA_ERROR;
  trgt0 = trgt;
  src0 = src;

  work.b = otmp.b = NULL;
  do {
    trgt = trgt0;
    src = src0;
    n = srcinfo.rdims[0];
    mq = n*20 + 120;
    if (!fftdp)
      mq = mq/2;
    work.l = (int *) realloc(work.l, mq);
    if (!work.l) {
      zap(result);
      result = cerror(ALLOC_ERR, 0);
      break;
    }
    if (!fftdp)
      rffti(&n, work.f);
    else
      rfftid(&n, work.d);

    tmp.f = otmp.f = realloc(otmp.b, n*ana_type_size[type]);
    if (tmp.f == NULL) {
      zap(result);
      result = cerror(ALLOC_ERR, 0);
      break;
    }

    step = srcinfo.step[0];

    switch (type) {
      case ANA_FLOAT:
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
	  /* advance the phases */
	  if (!(internalMode & 4))/* don't keep the average */
	    tmp.f[0] = 0.0;	/* average */
	  if (direction < 0)	/* shift phases over -90 degrees */
	    for (j = 1; j < n - 1; j += 2) {
	      v.f = tmp.f[j];
	      tmp.f[j] = -tmp.f[j + 1];
	      tmp.f[j + 1] = v.f;
	    }
	  else			/* shift phases over +90 degrees */
	    for (j = 1; j < n - 1; j += 2) {
	      v.f = tmp.f[j];
	      tmp.f[j] = tmp.f[j + 1];
	      tmp.f[j + 1] = -v.f;
	    }
	  if (j == n - 1
	      && !(internalMode & 8)) /* zero the highest frequency */
	    tmp.f[n - 1] = 0.0;	/* highest frequency */
	  /* apply backward FFT */
	  rfftb(&n, tmp.f, work.f);
	  /* put in result array */
	  for (j = 0; j < n; j++) {
	    *trgt.f = *tmp.f++ * factor.f;
	    trgt.f += step;
	  }
	  tmp.f = otmp.f;
	} while (advanceLoops(&srcinfo, &trgtinfo) < srcinfo.rndim);
	break;
      case ANA_DOUBLE:
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
	  /* advance the phases */
	  if (!(internalMode & 4))/* don't keep the average */
	    tmp.d[0] = 0.0;	/* average */
	  if (!(internalMode & 8)) /* don't keep the highest frequency */
	    tmp.d[1] = 0.0;	/* highest frequency */
	  if (direction < 0)	/* shift phases over -90 degrees */
	    for (j = 2; j < n; j += 2) {
	      v.d = tmp.d[j];
	      tmp.d[j] = -tmp.d[j + 1];
	      tmp.d[j + 1] = v.d;
	    }
	  else			/* shift phases over +90 degrees */
	    for (j = 2; j < n; j += 2) {
	      v.d = tmp.d[j];
	      tmp.d[j] = tmp.d[j + 1];
	      tmp.d[j + 1] = -v.d;
	    }
	  /* apply backward FFT */
	  rfftbd(&n, tmp.d, work.d);
	  /* put in result array */
	  for (j = 0; j < n; j++) {
	    *trgt.d = *tmp.d++ * factor.d;
	    trgt.d += step;
	  }
	  tmp.d = otmp.d;
	} while (advanceLoops(&srcinfo, &trgtinfo) < srcinfo.rndim);
	break;
    }
    src = trgt = src0 = trgt0;	/* target of last pass becomes source
				 of next one */
  } while (nextLoops(&srcinfo, &trgtinfo));

  free(otmp.b);
  free(work.f);
  return result;
}
/*------------------------------------------------------------------------- */
int fftshift(int narg, int ps[], int subroutine)
/* y = FFTSHIFT(data,dist) */
{
  int	iq, n, mq, type, j, jq, result, step, ndist;
  pointer	src, trgt, work, tmp, otmp, src0, trgt0, dist,
    sines, cosines;
  scalar	v, factor;
  int	rfftb(int *, float *, float *), rfftf(int *, float *, float *),
    rfftbd(int *, double *, double *), rfftfd(int *, double *, double *),
    rffti(int *, float *), rfftid(int *, double *), ana_indgen(int, int *);
  loopInfo	srcinfo, trgtinfo;

  iq = ps[0];			/* <data> */
  if (!symbolIsArray(iq))
    return cerror(NEED_ARR, ps[0]);

  if (subroutine) {
    type = symbol_type(iq);
    switch (type) {
      case ANA_FLOAT:
	fftdp = 0;
	break;
      case ANA_DOUBLE:
	fftdp = 1;
	break;
      default:
	return cerror(ILL_TYPE, iq);
    }
  } else {
    /*float the input */
    if (fftdp == 0) {		/* single precision */
      iq = ana_float(1, ps);
      type = ANA_FLOAT;
    } else {			/* double precision */
      iq = ana_double(1, ps);
      type = ANA_DOUBLE;
    }
  }

  jq = ana_indgen(1, &ps[1]);	/* axes */
  
  if (subroutine) {
    if (standardLoop(iq, jq, SL_EACHROW, type,
		     &srcinfo, &src, NULL, NULL, NULL) < 0)
      return ANA_ERROR;
    trgtinfo = srcinfo;
    trgt = src;
    trgtinfo.data = &trgt;
    trgtinfo.data0 = trgt.v;
    result = iq;
  } else {
    if (standardLoop(iq, jq, SL_EACHROW, type,
		     &srcinfo, &src, &result, &trgtinfo, &trgt) < 0)
      /* restore dimensional structure of iq, if necessary */
      return ANA_ERROR;
  }

  if (numerical(ps[1], NULL, NULL, &ndist, NULL) == ANA_ERROR)
    return ANA_ERROR;
  numerical(((type == ANA_FLOAT)? ana_float: ana_double)(1, &ps[1]),
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
    work.l = (int *) realloc(work.l, mq);
    /* get space to store phase numbers */
    j = (n/2)*ana_type_size[type];
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

    tmp.f = otmp.f = realloc(otmp.b, n*ana_type_size[type]);
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
      case ANA_FLOAT:
	if (*dist.f) {
	  /* now calculate the phase factors */
	  v.f = 2*PI*(*dist.f)/n;
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
	  } while (advanceLoops(&srcinfo, &trgtinfo) < srcinfo.rndim);
	} else if (src0.f != trgt0.f)
	  memcpy(trgt0.f, src0.f, array_size(iq)*sizeof(type));
	dist.f++;
	break;
      case ANA_DOUBLE:
	if (*dist.d) {
	  /* now calculate the phase factors */
	  v.d = 2*PI*(*dist.d)/n;
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
	  } while (advanceLoops(&srcinfo, &trgtinfo) < srcinfo.rndim);
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
  return subroutine? ANA_OK: result;
}
/*------------------------------------------------------------------------- */
int ana_fftshift(int narg, int ps[])
{
  return fftshift(narg, ps, 1);
}
/*------------------------------------------------------------------------- */
int ana_fftshift_f(int narg, int ps[])
{
  return fftshift(narg, ps, 0);
}
/*------------------------------------------------------------------------- */
int ana_power(int narg, int ps[])
/* power function: POWER(data [,axis, /POWER, /SHAPE]) */
{
  int	iq, n, mq, type, j, jq, outDims[MAX_DIMS], result, step;
  pointer	src, trgt, work, tmp, otmp, src0, trgt0;
  scalar	factor;
  int	rfftb(int *, float *, float *), rfftf(int *, float *, float *),
    rfftbd(int *, double *, double *), rfftfd(int *, double *, double *),
    rffti(int *, float *), rfftid(int *, double *);
  loopInfo	srcinfo;

  iq = ps[0];
  if (!symbolIsArray(iq))
    return cerror(NEED_ARR, ps[0]);

  /*float the input */
  if (fftdp == 0) {		/* single precision */
    iq = ana_float(1, ps);
    type = ANA_FLOAT;
  } else {			/* double precision */
    iq = ana_double(1, ps);
    type = ANA_DOUBLE;
  }

  if (narg > 1 && ps[1]) 	/* have <axis> */
    jq = ps[1];
  else				/* take the first axis */
    jq = (internalMode & 4)? SL_TAKEONED: ANA_ZERO;
  
  
  if (standardLoop(iq, jq, SL_EACHROW | SL_ONEAXIS, type,
		   &srcinfo, &src, NULL, NULL, NULL) < 0)
    return ANA_ERROR;
  src0 = src;

  /* get result symbol */
  outDims[0] = srcinfo.rdims[0]/2 + 1;
  memcpy(outDims + 1, srcinfo.rdims + 1, (srcinfo.rndim - 1)*sizeof(int));
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
    work.l = (int *) realloc(work.l, mq);
    if (!work.l) {
      zap(result);
      result = cerror(ALLOC_ERR, 0);
      break;
    }
    if (!fftdp)
      rffti(&n, work.f);
    else
      rfftid(&n, work.d);

    tmp.f = otmp.f = realloc(otmp.b, n*ana_type_size[type]);
    if (tmp.f == NULL) {
      zap(result);
      result = cerror(ALLOC_ERR, 0);
      break;
    }

    step = srcinfo.step[0];
    switch (type) {
      case ANA_FLOAT:
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
	} while (advanceLoop(&srcinfo) < srcinfo.rndim);
	break;
      case ANA_DOUBLE:
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
	} while (advanceLoop(&srcinfo) < srcinfo.rndim);
	break;
    }
    src0 = trgt0;
  } while (nextLoop(&srcinfo));

  free(otmp.b);
  free(work.f);
  return result;
}
/*------------------------------------------------------------------------- */
int ana_scb(int narg, int ps[])	/* scb routine */		
/* backwards sine-cosine style FFT --  scb, x, s, c */
{
  int	iq, nd, outer, nx, n, mq, dim[8], type, j, jq, nd2, outer2, nx2;
  pointer	q1,q2,q3;
  int	ezffti(int *n, float *wsave), ezfftid(int *n, double *wsave),
    ezfftb(int *n, float *r, float *azero, float *a, float *b, float *wsave),
    ezfftbd(int *n, double *r, double *azero, double *a, double *b,
	    double *wsave);

  iq = ps[1];
  jq = ps[2];
					/* s and c must be arrays */
  if (symbol_class(iq) != ANA_ARRAY
      || symbol_class(jq) != ANA_ARRAY)
    return cerror(NEED_ARR, 0);
  if (!fftdp) {
    iq = ana_float(1, &iq);
    jq = ana_float(1, &jq);
  } else {
    iq = ana_double(1, &iq);
    jq = ana_double(1, &jq);
  }
			/* s and c must have similar structures */
  q2.l = array_data(iq);
  nd = array_num_dims(iq);
  nx = array_dims(iq)[0];
  outer = array_size(iq)/nx;
  memcpy(dim + 1, array_dims(iq) + 1, (nd - 1)*sizeof(int));
  q3.l = array_data(jq);
  nd2 = array_num_dims(jq);
  nx2 = array_dims(jq)[0];
  outer2 = array_size(jq)/nx2;

  /* the inner dimensions must agree and the product of the outers */
  if (nx != nx2 || outer != outer2)
    return anaerror("SCB - sine and cosine arrays don't match", ps[1]);
  n = (nx - 1)*2;

  /* Because of the way the sine and cosine coefficients are stored by */
  /* ana_sc, the results for an even number of data points have the */
  /* same number of elements as the results for the next higher odd */
  /* number of data points.  If the number of data points is even, then */
  /* the highest sine component is equal to zero.  If the number of */
  /* data points is odd, then the highest sine component is in general */
  /* not equal to zero, but may be equal to zero if the data happen */
  /* to be just right for that.  Because also the user can specify */
  /* arbitrary values for the sine and cosine components, there really */
  /* is no way for us to figure out for sure if any given call to */
  /* ana_scb is supposed to return an even or an odd number of data points. */
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

  mq = array_size(iq)*ana_type_size[iq];
  /* scratch storage is needed, can we use the last one setup ? */
  if (nold != n || fftdp != fftdpold || mq > mqold) {/* set up work space */
    if (nold)
      free(work.b);		/* delete old if any */
    mq = n*28 + 120;
    if (!fftdp)
      mq = mq/2;
    work.l = (int *) malloc(mq);
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
  type = fftdp? ANA_DOUBLE: ANA_FLOAT;
  if (redef_array(iq, type, nd, dim) != 1)
    return ANA_ERROR;
					/* get addresse */
  q1.l = array_data(iq);
  /* loop over the outer dimensions */
  while (outer--) {
    switch (type) {
      case ANA_FLOAT:
	ezfftb(&n, q1.f, q3.f, q3.f + 1, q2.f + 1, work.f);
	q1.f += n;
	q2.f += nx;
	q3.f += nx;
	break;
      case ANA_DOUBLE:
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
int ana_histr(int narg, int ps[]) /* histr function */		
/* running sum histogram, normalized to 1.0 */
{
  int	iq, n, nd, j, type, size, nRepeat;
  float	sum, fac;
  array	*h;
  pointer q1, q2;
  int	ana_hist(int, int []);

					/* first get a normal histogram */
  if ( (iq = ana_hist(narg,ps) ) <= 0 ) return iq;
/* the returned symbol should be a long array, convert in place to a float,
	this depends on float and long (int) being 32 bits! */
			/* check for some impossible errors */
  if ( sym[iq].class != ANA_ARRAY ) {
    return cerror(IMPOSSIBLE, iq);}
  type = sym[iq].type;
  if (type != ANA_LONG ) return cerror(IMPOSSIBLE, iq);
  h = (array *) sym[iq].spec.array.ptr;
  q1.l = (int *) ((char *)h + sizeof(array));
  nd = h->ndim;	n = 1;
  for(j=0;j<nd;j++) n *= h->dims[j];
  sym[iq].type = ANA_FLOAT;		/* change to float */
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
int findint(int current, int *value, int nValue)
/* finds <current> in list <value> (<nValue> elements) which must be sorted
 in ascending order.  Returns nonnegative index of found <current>, or
 minus the index of the next higher present value, if <current> itself
 is not found. */
{
  int	low, mid, high;
  
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
int ana_hist_dense(int narg, int ps[])
/* returns compact histogram for widely spaced data values. */
/* HIST(x,l) returns list of present data values in <l> and corresponding */
/* histogram as return value.  LS 18feb98 */
{
  int	nStore, nValue, nData, *value, *freq, x, y, result;
  char	*avalue, *afreq;
  pointer	src;
  int	findint(int, int *, int);

  if (numerical(ps[0], NULL, NULL, &nData, &src) == ANA_ERROR)
    return ANA_ERROR;
  nStore = 512;
  avalue = (char *) malloc(nStore*sizeof(int) + sizeof(array));
  afreq = (char *) malloc(nStore*sizeof(int) + sizeof(array));
  if (!avalue || !afreq)
    return cerror(ALLOC_ERR, 0);
  value = (int *) (avalue + sizeof(array));
  freq = (int *) (afreq + sizeof(array));

  nValue = 0;
  while (nData--) {
    switch (symbol_type(ps[0])) {
    case ANA_BYTE:
      x = (int) *src.b++;
      break;
    case ANA_WORD:
      x = (int) *src.w++;
      break;
    case ANA_LONG:
      x = (int) *src.l++;
      break;
    case ANA_FLOAT:
      x = (int) *src.f++;
      break;
    case ANA_DOUBLE:
      x = (int) *src.d++;
      break;
    }
    y = findint(x, value, nValue);
    if (y >= 0)			/* already in list */
      freq[y]++;
    else {			/* not yet in list */
      if (++nValue == nStore) { /* need to allocate more room */
	nStore += 512;
	avalue = (char *) realloc(avalue, nStore*sizeof(int) + sizeof(array));
	afreq = (char *) realloc(afreq, nStore*sizeof(int) + sizeof(array));
	if (!avalue || !afreq)
	  return cerror(ALLOC_ERR, 0);
	value = (int *) (avalue + sizeof(array));
	freq = (int *) (afreq + sizeof(array));
      }
      y = -y;
      memmove(value + y, value + y - 1, (nValue - y)*sizeof(int));
      memmove(freq + y, freq + y - 1, (nValue - y)*sizeof(int));
      value[--y] = x;
      freq[y] = 1;
    }
  }
  avalue = (char *) realloc(avalue, nValue*sizeof(int) + sizeof(array));
  afreq = (char *) realloc(afreq, nValue*sizeof(int) + sizeof(array));
  if (!avalue || !afreq)
    return cerror(ALLOC_ERR, 0);

  getFreeTempVariable(result);
  symbol_class(result) = ANA_ARRAY;
  array_type(result) = ANA_LONG;
  array_header(result) = (array *) afreq;
  array_num_dims(result) = 1;
  *array_dims(result) = nValue;
  array_header(result)->c1 = array_header(result)->c2
    = array_header(result)->nfacts = 0;
  array_header(result)->facts = NULL;
  symbol_memory(result) = nValue*sizeof(int) + sizeof(array);

  symbol_class(ps[1]) = ANA_ARRAY;
  array_type(ps[1]) = ANA_LONG;
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
int ana_hist(int narg, int ps[]) /* histogram function */
				 /* (frequency distribution) */
/* general histogram function */
/* keyword /FIRST produces a histogram of all elements along the 0th */
/* dimension for all higher dimensions */
/* LS 12jul2000: added /SILENT keyword (internalMode & 8) to suppress */
/* warnings about negative histogram elements. */
{
  void	convertPointer(scalar *, int, int);
  int	iq, i, n, range, type, result_sym, *dims, nRepeat,
  	ndim, axis, one = 1, size;
  array	*h;
  pointer q1, q2;
  int	ana_zero(int, int []);

  if (narg == 2)
    return ana_hist_dense(narg, ps);
  iq = ps[0];
  switch (symbol_class(iq))
  { case ANA_ARRAY:
      type = array_type(iq);
      q1.l = array_data(iq);
      n = array_size(iq);
      dims = array_dims(iq);
      ndim = array_num_dims(iq);
      break;
    case ANA_SCALAR:
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
  /* get long (int) versions of min and max */
  convertWidePointer(&lastmin, type, ANA_LONG);
  convertWidePointer(&lastmax, type, ANA_LONG);
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
	return ANA_ERROR;
  }
  axis = internalMode & 1;
  if (axis)
  { size = *dims;
    nRepeat = n/size;
    one = nRepeat*range;
    result_sym = array_scratch(ANA_LONG, 1, &one);
    h = HEAD(result_sym);
    h->dims[0] = range;
    if (ndim > 1) memcpy(&h->dims[1], &dims[1], sizeof(int)*(ndim - 1));
    h->ndim = ndim; }
  else
  { one = range;
    size = n;
    nRepeat = 1;
    result_sym = array_scratch(2, 1, &one);
    h = HEAD(result_sym); }
  q2.l = (int *) ((char *)h + sizeof(array));
  ana_zero( 1, &result_sym);		/* need to zero initially */
  /* now accumulate the distribution */
  while (nRepeat--)
  { n = size;
    switch (type)
    { case ANA_BYTE:
	while (n--)
	{ i = *q1.b++ - histmin;  q2.l[i]++; }  break;
      case ANA_WORD:
	while (n--)
	{ i = *q1.w++ - histmin;  q2.l[i]++; }  break;
      case ANA_LONG:
	while (n--)
	{ i = *q1.l++ - histmin;  q2.l[i]++; }  break;
      case ANA_FLOAT:
	while (n--)
	{ i = *q1.f++ - histmin;  q2.l[i]++; }  break;
      case ANA_DOUBLE:
	while (n--)
	{ i = *q1.d++ - histmin;  q2.l[i]++; }  break;
      }
    q2.l += range; }
  return result_sym;
}
/*------------------------------------------------------------------------- */
int ana_sieve(int narg, int ps[])
/* X=SIEVE(array,condition), where condition is normally a logical array
   array and condition must both be arrays and condition must be at least
   as long as array 
   the output is a 1-D vector containing all elements of array for which 
   the corresponding element in condition is non-zero 
   12may92 LS:
   if <condition> does not contain non-zero elements, then a scalar -1 is
   returned.  alternate use: X=SIEVE(condition) yields the same as
   X=SIEVE(ANA_LONG(INDGEN(condition)),condition) */ 
{
  int	iq, n, nc, cnt, *p, type, typec, result_sym;
  pointer q1, q2, q3;

  iq = ps[0];
  if (iq < 0)
    return iq;			/* pass-thru errors */
  if (symbol_class(iq) == ANA_SCAL_PTR)
    iq = dereferenceScalPointer(iq);
  if (narg == 2) 			/* two-argument case */
					/* first arg must be an array */
  { switch (symbol_class(iq))
    { case ANA_SCAL_PTR:
	iq = dereferenceScalPointer(iq);
      case ANA_SCALAR:
	q1.l = &scalar_value(iq).l;
	n = 1;
	break;
      case ANA_ARRAY:
	q1.l = (int *) array_data(iq);
	n = array_size(iq);
	break;
      default:
	return cerror(NEED_ARR, iq); }
    type = symbol_type(iq);
				/* second argument is the conditional */
    iq = ps[1]; }

  switch (symbol_class(iq))
  { case ANA_SCALAR:
      nc = 1;			/*scalar case */
      p = &scalar_value(iq).l;
      break;
    case ANA_ARRAY:		/*array case */
      p = (int *) array_data(iq);
      nc = array_size(iq);
      break; }
  typec = symbol_type(iq);
  /* unlike the VMS code, we do a double pass, first to get the count */
  /* use the min of size of array and conditional */
  if (narg == 1)
  { n = nc;
    type = ANA_LONG; }
  else
    n = n <= nc ? n : nc;
  nc = n;
  cnt = 0;
  q3.l = p;
  switch (typec)
  { case ANA_BYTE:
      while (nc--)
	if (*q3.b++)
	  cnt++;
      break;
    case ANA_WORD:
      while (nc--)
	if (*q3.w++)
	  cnt++;
      break;
    case ANA_LONG:
      while (nc--)
	if (*q3.l++)
	  cnt++;
      break;
    case ANA_FLOAT:
      while (nc--)
	if (*q3.f++)
	  cnt++;
      break;
    case ANA_DOUBLE:
      while (nc--)
	if (*q3.d++)
	  cnt++;
      break; }
  q3.l = p;
 /* if count is zero, then no boolean TRUEs; return a scalar -1.  LS 12may92*/
  if (cnt == 0)
  { result_sym = scalar_scratch(ANA_LONG);
    scalar_value(result_sym).l = -1;
    return result_sym; }
  result_sym = array_scratch(type, 1, &cnt); /*for the result */
  q2.l = array_data(result_sym);
  nc = n;
  if (narg == 1)		/* one-argument case      LS 12may92 */
  { n = 0;			/* use as indgen counter */
    switch (typec)
    { case ANA_BYTE:
	while (nc--)
	{ if (*q3.b++)
	    *q2.l++ = n;
	  n++; }
	break;
      case ANA_WORD:
	while (nc--)
	{ if (*q3.w++)
	    *q2.l++ = n;
	  n++; }
	break;
      case ANA_LONG:
	while (nc--)
	{ if (*q3.l++)
	    *q2.l++ = n;
	  n++; }
	break;
      case ANA_FLOAT:
	while (nc--)
	{ if (*q3.f++)
	    *q2.l++ = n;
	  n++; }
	break;
      case ANA_DOUBLE:
	while (nc--)
	{ if (*q3.d++)
	    *q2.l++ = n;
	  n++; }
	break; }
  }
  else				/* two-argument case */
    /* this is a 25 case situation */
    switch (typec) {
    case ANA_BYTE:
      switch (type)
      { case ANA_BYTE:
	  while (nc--)
	  { if (*q3.b++)
	      *q2.b++ = *q1.b;
	    q1.b++; }
	  break;
	case ANA_WORD:
	  while (nc--)
	  { if (*q3.b++)
	      *q2.w++ = *q1.w;
	    q1.w++; }
	  break;
	case ANA_LONG:
	  while (nc--)
	  { if (*q3.b++)
	      *q2.l++ = *q1.l;
	    q1.l++; }
	  break;
	case ANA_FLOAT:
	  while (nc--)
	  { if (*q3.b++)
	      *q2.f++ = *q1.f;
	    q1.f++; }
	  break;
	case ANA_DOUBLE:
	  while (nc--)
	  { if (*q3.b++)
	      *q2.d++ = *q1.d;
	    q1.d++; }
	  break; }
      break;
    case ANA_WORD:
      switch (type)
      { case ANA_BYTE:
	  while (nc--)
	  { if (*q3.w++)
	      *q2.b++ = *q1.b;
	    q1.b++; }
	  break;
	case ANA_WORD:
	  while (nc--)
	  { if (*q3.w++)
	      *q2.w++ = *q1.w;
	    q1.w++; }
	  break;
	case ANA_LONG:
	  while (nc--)
	  { if (*q3.w++)
	      *q2.l++ = *q1.l;
	    q1.l++; }
	  break;
	case ANA_FLOAT:
	  while (nc--)
	  { if (*q3.w++)
	      *q2.f++ = *q1.f;
	    q1.f++; }
	  break;
	case ANA_DOUBLE:
	  while (nc--)
	  { if (*q3.w++)
	      *q2.d++ = *q1.d;
	    q1.d++; }
	  break; }
      break;
    case ANA_LONG:
      switch (type) 
      { case ANA_BYTE:
	  while (nc--)
	  { if (*q3.l++)
	      *q2.b++ = *q1.b;
	    q1.b++; }
	  break;
	case ANA_WORD:
	  while (nc--)
	  { if (*q3.l++)
	      *q2.w++ = *q1.w;
	    q1.w++; }
	  break;
	case ANA_LONG:
	  while (nc--)
	  { if (*q3.l++)
	      *q2.l++ = *q1.l;
	    q1.l++; }
	  break;
	case ANA_FLOAT:
	  while (nc--)
	  { if (*q3.l++)
	      *q2.f++ = *q1.f;
	    q1.f++; }
	  break;
	case ANA_DOUBLE:
	  while (nc--)
	  { if (*q3.l++)
	      *q2.d++ = *q1.d;
	    q1.d++; }
	  break; }
      break;
    case ANA_FLOAT:
      switch (type) 
      { case ANA_BYTE:
	  while (nc--)
	  { if (*q3.f++)
	      *q2.b++ = *q1.b;
	    q1.b++; }
	  break;
	case ANA_WORD:
	  while (nc--)
	  { if (*q3.f++)
	      *q2.w++ = *q1.w;
	    q1.w++; }
	  break;
	case ANA_LONG:
	  while (nc--)
	  { if (*q3.f++)
	      *q2.l++ = *q1.l;
	    q1.l++; }
	  break;
	case ANA_FLOAT:
	  while (nc--)
	  { if (*q3.f++)
	      *q2.f++ = *q1.f;
	    q1.f++; }
	  break;
	case ANA_DOUBLE:
	  while (nc--)
	  { if (*q3.f++)
	      *q2.d++ = *q1.d;
	    q1.d++; }
	  break; }
      break;
    case ANA_DOUBLE:
      switch (type) 
      { case ANA_BYTE:
	  while (nc--)
	  { if (*q3.d++)
	      *q2.b++ = *q1.b;
	    q1.b++; }
	  break;
	case ANA_WORD:
	  while (nc--)
	  { if (*q3.d++)
	      *q2.w++ = *q1.w;
	    q1.w++; }
	  break;
	case ANA_LONG:
	  while (nc--)
	  { if (*q3.d++)
	      *q2.l++ = *q1.l;
	    q1.l++; }
	  break;
	case ANA_FLOAT:
	  while (nc--)
	  { if (*q3.d++)
	      *q2.f++ = *q1.f;
	    q1.f++; }
	  break;
	case ANA_DOUBLE:
	  while (nc--)
	  { if (*q3.d++)
	      *q2.d++ = *q1.d;
	    q1.d++; }
	  break; }
      break; }
  return result_sym;
}
/*------------------------------------------------------------------------- */
int ana_maxf(int narg, int ps[])
				/* finds the max element in an array */
{
  int	maxormin(int, int [], int);

  return maxormin(narg, ps, 0);
}
/*------------------------------------------------------------------------- */
int index_maxormin(int source, int indices, int code)
{
  int	nIndx, type, offset, *indx, i, result, size, nElem, indices2;
  char	*any;
  pointer	src, trgt;
  extern scalar	lastmin, lastmax;

  src.l = array_data(source);
  nElem = array_size(source);
  type = array_type(source);
  indices2 = ana_long(1, &indices);
  indx = array_data(indices2);
  nIndx = array_size(indices2);
  if (nIndx != nElem)
    return cerror(INCMP_ARR, indices);
  /* find min and max of indices so we can create a result array of the */
  /* correct size */
  minmax(indx, nIndx, ANA_LONG);
  /* use offset so we can treat negative indices */
  size = lastmax.l + 1;
  offset = 0;
  if (lastmin.l < 0)
    size += (offset = -lastmin.l);
  result = array_scratch(code < 2? type: ANA_LONG, 1, &size);
  /* make trgt.b point to the element associated with index zero */
  trgt.b = (byte *) array_data(result) + offset*ana_type_size[array_type(result)];
  /* Initialize result to zero */
  zerobytes(array_data(result), size*ana_type_size[array_type(result)]);
  /* use "any" as an array of flags indicating if any number has been */
  /* put in the corresponding result element so far */
  allocate(any, size, char);
  zerobytes(any, size);		/* initialize to empty */
  any += offset;		/* in case of negative indices */
  switch (type) {
    case ANA_BYTE:
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
    case ANA_WORD:
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
    case ANA_LONG:
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
    case ANA_FLOAT:
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
    case ANA_DOUBLE:
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
    case ANA_CFLOAT:
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
    case ANA_CDOUBLE:
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
int maxormin(int narg, int ps[], int code)
/* finds extreme values.  arguments:  <data> [, <axes>] [, /KEEPDIMS] */
/* <code>: 0 -> max value; 1 -> min value; 2 -> max location;
   3 -> min location */
{
  int	mode, minloc, maxloc, n, result, n1;
  loopInfo	srcinfo, trgtinfo;
  pointer	src, trgt;
  scalar	min, max;
  double	value;
  extern scalar	lastmin, lastmax;
  extern int	lastminloc, lastmaxloc;
  extern int	lastmin_sym, lastmax_sym;

  if (narg == 2
      && symbolIsNumericalArray(ps[0])
      && symbolIsNumericalArray(ps[1])
      && array_size(ps[0]) == array_size(ps[1]))
    return index_maxormin(ps[0], ps[1], code);

  mode = SL_UNIQUEAXES | SL_COMPRESSALL | SL_AXESBLOCK;
  if (code & 2)			/* seek location */
    mode |= SL_EXACT;		/* output must be exactly ANA_LONG */
  else
    mode |= SL_KEEPTYPE;	/* output must be same type as input */
  if (internalMode & 1)		/* /KEEPDIMS */
    mode |= SL_ONEDIMS;

  if (standardLoop(ps[0], narg > 1? ps[1]: 0, mode, ANA_LONG, &srcinfo,
		   &src, &result, &trgtinfo, &trgt) == ANA_ERROR)
    return ANA_ERROR;

  n1 = srcinfo.naxes? srcinfo.naxes: 1;

  switch (srcinfo.type) {
    case ANA_BYTE:
      do {
	/* take first value as initial values */
	memcpy(&min, src.b, srcinfo.stride);
	memcpy(&max, src.b, srcinfo.stride);
	minloc = maxloc = src.b - (byte *) srcinfo.data0;
	do {
	  if (*src.b > max.b) {
	    max.b = *src.b;
	    maxloc = src.b - (byte *) srcinfo.data0;
	  } else if (*src.b < min.b) {
	    min.b = *src.b;
	    minloc = src.b - (byte *) srcinfo.data0;
	  }
	} while ((n = advanceLoop(&srcinfo)) < n1);
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
    case ANA_WORD:
      do {
	/* take first value as initial values */
	memcpy(&min, src.w, srcinfo.stride);
	memcpy(&max, src.w, srcinfo.stride);
	minloc = maxloc = src.w - (word *) srcinfo.data0;
	do {
	  if (*src.w > max.w) {
	    max.w = *src.w;
	    maxloc = src.w - (word *) srcinfo.data0;
	  } else if (*src.w < min.w) {
	    min.w = *src.w;
	    minloc = src.w - (word *) srcinfo.data0;
	  }
	} while ((n = advanceLoop(&srcinfo)) < n1);
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
    case ANA_LONG:
      do {
	/* take first value as initial values */
	memcpy(&min, src.l, srcinfo.stride);
	memcpy(&max, src.l, srcinfo.stride);
	minloc = maxloc = src.l - (int *) srcinfo.data0;
	do {
	  if (*src.l > max.l) {
	    max.l = *src.l;
	    maxloc = src.l - (int *) srcinfo.data0;
	  } else if (*src.l < min.l) {
	    min.l = *src.l;
	    minloc = src.l - (int *) srcinfo.data0;
	  }
	} while ((n = advanceLoop(&srcinfo)) < n1);
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
    case ANA_FLOAT:
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
	} while ((n = advanceLoop(&srcinfo)) < n1);
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
    case ANA_DOUBLE:
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
	} while ((n = advanceLoop(&srcinfo)) < n1);
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
    case ANA_CFLOAT:
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
	} while ((n = advanceLoop(&srcinfo)) < n1);
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
    case ANA_CDOUBLE:
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
	} while ((n = advanceLoop(&srcinfo)) < n1);
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
int ana_minf(int narg, int ps[])
				/* finds the min element in an array */
{
  return maxormin(narg, ps, 1);
}
/*------------------------------------------------------------------------- */
int ana_minloc(narg,ps)
				/* finds the min element in an array */
int	narg, ps[];
{
return maxormin(narg, ps, 3);
}
/*------------------------------------------------------------------------- */
int ana_maxloc(narg,ps)
				/* finds the max element in an array */
int	narg, ps[];
{
return maxormin(narg, ps, 2);
}
/*------------------------------------------------------------------------- */
void scale(pointer data, byte type, int size, double datalow, double datahigh,
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
#ifdef X11
  extern int	colorIndexType;
#else
  int	colorIndexType = ANA_BYTE;
#endif

  trgt.b = dest;
  drange = datahigh - datalow;
  if (!drange) {
    neutral(trgt.b, size);
    return;
  }
  dfac = (trgthigh - trgtlow)/drange;

  switch (type) {
    case ANA_BYTE:
      ffac = (float) dfac;
      foff = trgtlow - datalow*ffac;
      switch (colorIndexType) {
	case ANA_BYTE:
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
	case ANA_WORD:
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
	case ANA_LONG:
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
      }
      break;
    case ANA_WORD:
      ffac = (float) dfac;
      foff = trgtlow - datalow*ffac;
      switch (colorIndexType) {
	case ANA_BYTE:
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
	case ANA_WORD:
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
	case ANA_LONG:
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
      }
      break;
    case ANA_LONG:
      ffac = (float) dfac;
      foff = trgtlow - datalow*ffac;
      switch (colorIndexType) {
	case ANA_BYTE:
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
	case ANA_WORD:
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
	case ANA_LONG:
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
      }
      break;
    case ANA_FLOAT:
      ffac = (float) dfac;
      foff = trgtlow - datalow*ffac;
      switch (colorIndexType) {
	case ANA_BYTE:
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
	case ANA_WORD:
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
	case ANA_LONG:
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
      }
      break;
    case ANA_DOUBLE:
      doff = trgtlow - datalow*dfac;
      switch (colorIndexType) {
	case ANA_BYTE:
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
	case ANA_WORD:
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
	case ANA_LONG:
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
      }
      break;
  }
}
/*------------------------------------------------------------------------- */
int ana_scale(int narg, int ps[])
/* scale an array to the range (!scalemin,!scalemax) and put into an array */
/* of the smallest data type that can fit any allowed color cell index  */
/* add keyword /FULLRANGE: scale between 0 and the greatest color */
/* cell index.  LS 30jul96 23mar99 */
{
  int	iq, type, result_sym, nd, n, oldScalemin, oldScalemax;
  scalar	min, max;
  register	pointer q1, q2;
  double	sd, qd;
#ifdef X11
  extern double	zoom_clo, zoom_chi;
  extern int	threeColors;
  extern int	colorIndexType, display_cells;
#else
  int	colorIndexType = ANA_BYTE, display_cells = 256;
#endif

  iq = ps[0];
  if (symbol_class(iq) != ANA_ARRAY)
    return cerror(NEED_ARR, iq);
  type = array_type(iq);
  q1.l = array_data(iq);
  nd = array_num_dims(iq);
  n = array_size(iq);
  result_sym = array_clone(iq, colorIndexType);
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
#ifdef X11
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
#ifdef X11
    if (narg == 1) {		/* have /ZOOM */
      min.d = zoom_clo;
      max.d = zoom_chi;
    } else
#endif
      if (type == ANA_DOUBLE) { /* array is double, get input as double */
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
#ifdef X11
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
int ana_scalerange(int narg, int ps[])
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
  int	iq, type, result_sym, nd, n, oldScalemin, oldScalemax;
  scalar	min, max;
  register	pointer q1, q2;
  double	sd, qd, logrey, higrey;
#ifdef X11
  extern double	zoom_clo, zoom_chi;
  extern int	threeColors;
  extern int	colorIndexType, display_cells;
#else
  int	colorIndexType = ANA_BYTE, display_cells = 256;
#endif

  iq = ps[0];
  if (symbol_class(iq) != ANA_ARRAY)
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
  nd = array_num_dims(iq);
  n = array_size(iq);
  result_sym = array_clone(iq, colorIndexType);
  q2.l = array_data(result_sym);
  /* if only three args, then we scale between the min and max of array */
  if (narg == 3 && (internalMode & 2) == 0) {/* 3 arg and no /ZOOM */
    minmax(q1.l, n, type);	/* get the min and max */
    oldScalemin = scalemin;	/* remember for later */
    oldScalemax = scalemax;
    if (internalMode & 1) {	/* /BYTE */
      scalemin = (int) (logrey*(display_cells - 0.001));
      scalemax = (int) (higrey*(display_cells - 0.001));
    }
#ifdef X11
    else if (threeColors) {
      scalemin = (int) ((display_cells/3 - 0.001)*logrey);
      scalemax = (int) ((display_cells/3 - 0.001)*higrey);
    }
#endif
    else {
      iq = scalemax - scalemin;
      scalemin = (int) (iq*logrey + scalemin);
      scalemax = (int) (scalemax - iq*(1 - higrey));
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
#ifdef X11
    if (narg == 3) {		/* have /ZOOM */
      min.d = zoom_clo;
      max.d = zoom_chi;
    } else
#endif
      if (type == ANA_DOUBLE) { /* array is double, get input as double */
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
#ifdef X11
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
int minmax(int *p, int n, int type)
/* finds the min and max (and their locations) for an array */
{
  pointer q;
  scalar	min, max;
/* pointers on OSF machines are bigger than ints, so casts from pointers */
/* to ints must be avoided!  thus, changed  minloc, maxloc  from int to */
/* byte *.  LS27jul94 */
  void	*minloc, *maxloc;
  int	 nc;

  q.l = p;
  nc = n - 1;
  minloc = maxloc = (byte *) p;
  switch (type) {
    case ANA_BYTE:
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
      scalar_type(lastmin_sym) = scalar_type(lastmax_sym) = ANA_BYTE;
      break;
    case ANA_WORD:
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
      scalar_type(lastmin_sym) = scalar_type(lastmax_sym) = ANA_WORD;
      break;
    case ANA_LONG:
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
      scalar_type(lastmin_sym) = scalar_type(lastmax_sym) = ANA_LONG;
      break;
    case ANA_FLOAT:
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
      scalar_type(lastmin_sym) = scalar_type(lastmax_sym) = ANA_FLOAT;
      break;
    case ANA_DOUBLE:
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
      scalar_type(lastmin_sym) = scalar_type(lastmax_sym) = ANA_DOUBLE;
      break;
    }						/* end of type switch */
  lastmaxloc = ((byte *) maxloc - (byte *) p) / ana_type_size[type];
  lastminloc = ((byte *) minloc - (byte *) p) / ana_type_size[type];
  return 1;
}
/*------------------------------------------------------------------------- */
int simple_scale(void *p1, int n, int type, void *p2)
/* scale n elements of the given data <type> starting at <p1> into the */
/* array at <p2> */
/* uses the min and max stashed in lastmin and lastmax */
/* assumes that the input array values are all inbetween the min and max */ 
/* this means we don't need to check for over and under flow */
/* assumes <p2> is of type <colorIndexType>.  LS 23mar99 */
{
  register	pointer q1, q2;
  register	scalar	range, min;
  register	int	xq;
  register	float	fq;
  register	double	dq;
#ifdef X11
  extern int	connect_flag, colorIndexType;
#else
  int	colorIndexType = ANA_BYTE;
#endif

#ifdef X11
  if (!connect_flag)
    setup_x();
#endif
  q1.l = p1;
  q2.l = p2;
  switch (type) {
    case ANA_BYTE:
      min.b = lastmin.b;
      range.l = lastmax.b - min.b;
      xq = (int) (scalemax - scalemin);
      if (range.l == 0)
	return neutral(p2, n);
      switch (colorIndexType) {
	case ANA_BYTE:
	  while (n--)
	    *q2.b++ = (xq*(*q1.b++ - min.b))/range.l + scalemin;
	  break;
	case ANA_WORD:
	  while (n--)
	    *q2.w++ = (xq*(*q1.b++ - min.b))/range.l + scalemin;
	  break;
	case ANA_LONG:
	  while (n--)
	    *q2.l++ = (xq*(*q1.b++ - min.b))/range.l + scalemin;
	  break;
      }
      break;
    case ANA_WORD:
      min.w = lastmin.w;
      range.l = lastmax.w - min.w;
      xq = (int) (scalemax - scalemin);	
      if (range.l == 0)
	return neutral(p2, n);
      switch (colorIndexType) {
	case ANA_BYTE:
	  while (n--)
	    *q2.b++ = (xq*(*q1.w++ - min.w))/range.l + scalemin;
	  break;
	case ANA_WORD:
	  while (n--)
	    *q2.w++ = (xq*(*q1.w++ - min.w))/range.l + scalemin;
	  break;
	case ANA_LONG:
	  while (n--)
	    *q2.l++ = (xq*(*q1.w++ - min.w))/range.l + scalemin;
	  break;
      }
      break;
    case ANA_LONG:
      /* to avoid possible range problems, calculation is done in fp */
      min.l = lastmin.l;
      range.l = lastmax.l - min.l;
      if (range.l == 0)
	return neutral(p2, n);
      fq = (float) (scalemax - scalemin)/(float) range.l;
      switch (colorIndexType) {
	case ANA_BYTE:
	  while (n--)
	    *q2.b++ = (fq*(*q1.l++ - min.l)) + scalemin;
	  break;
	case ANA_WORD:
	  while (n--)
	    *q2.w++ = (fq*(*q1.l++ - min.l)) + scalemin;
	  break;
	case ANA_LONG:
	  while (n--)
	    *q2.l++ = (fq*(*q1.l++ - min.l)) + scalemin;
	  break;
      }
      break;
    case ANA_FLOAT:
      min.f = lastmin.f;
      range.f = lastmax.f - min.f;	
      if (range.f == 0)
	return neutral(p2, n);
      fq = (float) (scalemax-scalemin)/range.f;
      switch (colorIndexType) {
	case ANA_BYTE:
	  while (n--)
	    *q2.b++ = (fq*(*q1.f++ - min.f)) + scalemin;
	  break;
	case ANA_WORD:
	  while (n--)
	    *q2.w++ = (fq*(*q1.f++ - min.f)) + scalemin;
	  break;
	case ANA_LONG:
	  while (n--)
	    *q2.l++ = (fq*(*q1.f++ - min.f)) + scalemin;
	  break;
      }
      break;
    case ANA_DOUBLE:
      min.d = lastmin.d;
      range.d = lastmax.d - min.d;	
      if (range.d == 0)
	return neutral(p2, n );
      dq = (double) (scalemax - scalemin)/range.d;
      switch (colorIndexType) {
	case ANA_BYTE:
	  while (n--)
	    *q2.b++ = (dq*(*q1.d++ - min.d)) + scalemin;
	  break;
	case ANA_WORD:
	  while (n--)
	    *q2.w++ = (dq*(*q1.d++ - min.d)) + scalemin;
	  break;
	case ANA_LONG:
	  while (n--)
	    *q2.l++ = (dq*(*q1.d++ - min.d)) + scalemin;
	  break;
      }
      break;
  }
  return ANA_OK;
}
/*------------------------------------------------------------------------- */
int neutral(void *p, int n)
/* fills an array of type <colorIndexType> with (scalemax-scalemin)/2 */
{
  int	xq;
  pointer	pp;
#ifdef X11
  extern int	colorIndexType;
#else
  int	colorIndexType = ANA_BYTE;
#endif

  pp.b = p;
  xq = (scalemax - scalemin)/2;
  switch (colorIndexType) {
    case ANA_BYTE:
      while (n--)
	*pp.b++ = xq;
      break;
    case ANA_WORD:
      while (n--)
	*pp.w++ = xq;
      break;
    case ANA_LONG:
      while (n--)
	*pp.l++ = xq;
      break;
  }
  return ANA_OK;
}
/*------------------------------------------------------------------------- */
const csplineInfo empty_cubic_spline(void) {
  const csplineInfo c = { NULL, NULL };
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
}
/*------------------------------------------------------------------------- */
int cubic_spline_tables(void *xx, int xType, int xStep,
			void *yy, int yType, int yStep,
			int nPoints, byte periodic,
			csplineInfo *cspl)
/* installs a cubic spline for later quick multiple interpolations */
/* <xx> must point to the x coordinates, which are of ANA data type <xType> */
/* Each next <xStep>th element is taken.  Likewise for <yy>, <yType>, */
/* and <yStep>.  <nPoints> is the number of data points that are installed. */
/* The <xx> must be in ascending order!  If <periodic> is set, then
   the data is assumed to be periodic; otherwise it is not. */
/* LS 9may98; redone using GSL 2009sep27 */
{
  int	n, i;
  pointer xin, yin;
  double *x, *y;

  if (!nPoints)
    return 1;

  cleanup_cubic_spline_tables(cspl);
  cspl->spline = gsl_spline_alloc(periodic? gsl_interp_cspline_periodic:
				  gsl_interp_cspline, nPoints);
  cspl->acc = gsl_interp_accel_alloc();

  x = malloc(nPoints*sizeof(double));
  y = malloc(nPoints*sizeof(double));
  xin.b = (byte *) xx;
  yin.b = (byte *) yy;

  /* first copy x (with necessary type conversion) */
  if (xx) {			/* have x */
    n = nPoints;
    switch (xType) {
    case ANA_BYTE:
      while (n--) {
	*x++ = (double) *xin.b;
	xin.b += xStep;
      }
      break;
    case ANA_WORD:
      while (n--) {
	*x++ = (double) *xin.w;
	xin.w += xStep;
      }
      break;
    case ANA_LONG:
      while (n--) {
	*x++ = (double) *xin.l;
	xin.l += xStep;
      }
      break;
    case ANA_FLOAT:
      while (n--) {
	*x++ = (double) *xin.f;
	xin.f += xStep;
      }
      break;
    case ANA_DOUBLE:
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
    case ANA_BYTE:
      while (n--) {
	*y++ = (double) *yin.b;
	yin.b += yStep;
      }
      break;
    case ANA_WORD:
      while (n--) {
	*y++ = (double) *yin.w;
	yin.w += yStep;
      }
      break;
    case ANA_LONG:
      while (n--) {
	*y++ = (double) *yin.l;
	yin.l += yStep;
      }
      break;
    case ANA_FLOAT:
      while (n--) {
	*y++ = (double) *yin.f;
	yin.f += yStep;
      }
      break;
    case ANA_DOUBLE:
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
  gsl_spline_init(cspl->spline, x, y, nPoints);
  return 1;
}
/*------------------------------------------------------------------------- */
double cspline_value(double x, csplineInfo *cspl)
/* interpolate using cubic splines.   Assumes <cspl> contains the required */
/* information about the cubic spline (installed with cubic_spline_tables() */
{
  return gsl_spline_eval(cspl->spline, x, cspl->acc);
}
/*------------------------------------------------------------------------- */
double cspline_derivative(double x, csplineInfo *cspl)
/* returns the derivative of a cubic spline at position <x>.  Assumes that */
/* the required information about the spline is available in <cspl> */
/* (installed with cubic_spline_tables). */
/* LS 9may98 */
{
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
  int	i, slow;

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
  int	i, slow, sgn;

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
int ana_cubic_spline(int narg, int ps[])
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
  static csplineInfo	cspl = { NULL, NULL };
  int	xNewSym, xTabSym, yTabSym, size, oldType, result_sym;
  pointer	src, trgt;
  int	ana_convert(int, int [], int, int);

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
    return anaerror("Need table to interpolate from", xNewSym);
  if (xTabSym && symbol_class(xTabSym) != ANA_ARRAY)
    return cerror(NEED_ARR, xTabSym);
  if (yTabSym && symbol_class(yTabSym) != ANA_ARRAY)
    return cerror(NEED_ARR, yTabSym);
  if (xTabSym && array_size(xTabSym) != array_size(yTabSym))
    return cerror(INCMP_ARR, yTabSym);

  if (xTabSym) {		/* install new table */
    if (cubic_spline_tables(array_data(xTabSym), array_type(xTabSym), 1,
			    array_data(yTabSym), array_type(yTabSym), 1,
			    array_size(xTabSym), internalMode & 2, &cspl)
	< 0)
      return ANA_ERROR;		/* some error */
    haveTable = 1;
  }

  result_sym = ANA_ONE;
  if (xNewSym) {		/* do interpolation */
    oldType = symbol_type(xNewSym);
    switch (symbol_class(xNewSym)) {
      case ANA_SCAL_PTR:
	xNewSym = dereferenceScalPointer(xNewSym); /* fall-thru */
      case ANA_SCALAR:
	if (oldType != ANA_DOUBLE)
	  xNewSym = ana_convert(1, &xNewSym, ANA_DOUBLE, 1);
	src.b = &scalar_value(xNewSym).b;
	size = 1;
	result_sym = scalar_scratch(ANA_DOUBLE);
	trgt.b = &scalar_value(result_sym).b;
	break;
      case ANA_ARRAY:
	if (oldType <= ANA_DOUBLE) { /* not a string array */
	  if (oldType != ANA_DOUBLE)
	    xNewSym = ana_convert(1, &xNewSym, ANA_DOUBLE, 1);
	  src.b = (byte *) array_data(xNewSym);
	  size = array_size(xNewSym);
	} else
	  return cerror(ILL_TYPE, xNewSym);
	result_sym = array_clone(xNewSym, ANA_DOUBLE);
	trgt.b = (byte *) array_data(result_sym);
	break;
      default:
	result_sym = cerror(ILL_TYPE, xNewSym);
    }
    if (result_sym != ANA_ERROR) { /* no error */
      if (internalMode & 4) 	/* return the interpolated derivative */
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
int ana_cubic_spline_extreme(int narg, int ps[])
/* CSPLINE_EXTREME, <x>, <y> [, <axis>, POS=<pos>, MINPOS=<minpos>,
   MINVAL=<min>, MAXPOS=<maxpos>, MAXVAL=<max>, /KEEPDIMS, /PERIODIC]
 */
{
  int	iq, dims[MAX_DIMS], ndim, step, pos, i, mode;
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

  if (standardLoop(ps[1], (narg > 2 && ps[2])? ps[2]: ANA_ZERO, mode,
		   ANA_FLOAT, &yinfo, &y, NULL, NULL, NULL) == ANA_ERROR)
    return ANA_ERROR;

  /* <x> must have one element for each element of <y> along the indicated
     <axis>. */
  if (array_size(ps[0]) != yinfo.rdims[0])
    return cerror(INCMP_ARG, ps[1]);
  iq = ana_converts[yinfo.type](1, &ps[0]);
  x.f = array_data(iq);
  
  if (narg > 3 && ps[3]) { 	/* have <pos> */
    pos = int_arg(ps[3]);
    if (pos < 0 || pos >= yinfo.rdims[0])
      return anaerror("Index out of range", ps[3]);
  } else				/* no <pos> */
    pos = -1;

  /* prepare the output variables */
  if (internalMode & 1) {
    memcpy(dims, yinfo.dims, yinfo.ndim*sizeof(int));
    dims[yinfo.axes[0]] = 1;
    ndim = yinfo.ndim;
  } else if (yinfo.rndim == 1)
    ndim = 0;
  else {
    memcpy(dims, yinfo.dims, yinfo.axes[0]*sizeof(int));
    memcpy(dims + yinfo.axes[0], yinfo.dims + yinfo.axes[0] + 1,
	   (yinfo.ndim - yinfo.axes[0] - 1)*sizeof(int));
    ndim = yinfo.ndim - 1;
  }

  if (narg > 4 && ps[4]) {	/* <minpos> */
    if (!symbolIsNamed(ps[4]))
      return anaerror("Need named variable here", ps[4]);
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
      return anaerror("Need named variable here", ps[5]);
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
      return anaerror("Need named variable here", ps[6]);
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
      return anaerror("Need named variable here", ps[7]);
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
    case ANA_FLOAT:
      do {
	/* install table for cubic spline interpolation */
	cubic_spline_tables(x.f, ANA_FLOAT, 1,
			    y.f, ANA_FLOAT, step,
			    yinfo.rdims[0], internalMode & 2? 1: 0,
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
      } while (advanceLoop(&yinfo) < yinfo.rndim);
      break;
    case ANA_DOUBLE:
      do {
	/* install table for cubic spline interpolation */
	cubic_spline_tables(x.d, ANA_DOUBLE, 1,
			    y.d, ANA_DOUBLE, step,
			    yinfo.rdims[0], internalMode & 2? 1: 0,
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
      } while (advanceLoop(&yinfo) < yinfo.rndim);
      break;
  }
  return ANA_OK;
}
/*------------------------------------------------------------------------- */
int ana_strtol(int narg, int ps[])
/* corresponds to C strtol */
{
  char	*string;
  int	base, number, result;

  if (symbol_class(ps[0]) != ANA_STRING)
    return cerror(NEED_STR, ps[0]);
  string = string_arg(ps[0]);
  if (narg > 1) {
    if (symbol_class(ps[1]) != ANA_SCALAR)
      return cerror(NEED_SCAL, ps[1]);
    base = int_arg(ps[1]);
  } else
    base = 10;
  number = strtol(string, NULL, base);
  result = scalar_scratch(ANA_LONG);
  scalar_value(result).l = number;
  return result;
}
/*------------------------------------------------------------------------- */
unsigned int bitmask[] = { 
  0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff,
  0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};
static unsigned int bits[16] = {
  1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768
};
int ana_extract_bits_f(int narg, int ps[])/* get a bit field from data */
/* call is xint = extract_bits(array, start, width) */
/* if width is < 0, then abs(width) is used for width and the sign
   is extended */
{
  int	result_sym, value;
  int	extract_bits(int, int [], int *);

  if (extract_bits(narg, ps, &value) != 1)
    return ANA_ERROR;
  result_sym = scalar_scratch(ANA_WORD);
  scalar_value(result_sym).w = (short) value;
  return result_sym;
}
/*------------------------------------------------------------------------ */
int ana_extract_bits(int narg, int ps[])/* get a bit field from data */
/* call is extract_bits, result, array, start, width */
/* if width is < 0, then abs(width) is used for width and the sign
   is extended */
{
  int	result_sym, value;
  int	extract_bits(int, int [], int *);

  result_sym = ps[0];
  if (extract_bits(narg - 1, &ps[1], &value) != 1)
    return ANA_ERROR;
  return redef_scalar(result_sym, ANA_WORD, &value);
}
/*------------------------------------------------------------------------ */
int extract_bits(int narg, int ps[], int *value)/* internal routine */
{
  int	n, start, width, iq, j, sign_flag, type;
  pointer	q;
  
  iq = ps[0];
  if (numerical(iq, NULL, NULL, &n, &q) == ANA_ERROR)
    return ANA_ERROR;
  type = symbol_type(iq);
  n = n * ana_type_size[type];	/* now the # of bytes */
  if (int_arg_stat(ps[1], &start) != 1)
    return ANA_ERROR;
  if (int_arg_stat(ps[2], &width) != 1)
    return ANA_ERROR;
  /* check for sign extension */
  if (width < 0) {
    width = -width;
    sign_flag = 1;
  } else
    sign_flag = 0;
  if (start + width > 8*n)	/* added LS 12feb99 */
    return anaerror("Extracting bits beyond the end of the data", ps[2]);
  /* start is a bit count, get the I*2 index */
  j = start/16;
  iq = (int) q.w[j];
  j = start % 16;	/* our shift */
  iq = (iq >> j) & bitmask[width];
  if (sign_flag)
    /* messy, but probably not used too often */
    if (iq & bits[width-1])
      /* the top bit of the bit field is set, so need to extend it */
      iq = iq | ( 0xffffffff - bitmask[width]);
  *value = iq;
  return ANA_OK;
}
/*------------------------------------------------------------------------- */
int ana_fade_init(int narg, int ps[])
/* initializes a fade (or dissolve) between 2 byte arrays, the actual
   fade result is obtained with the subroutine fade
   call is: fade_init, x1, x2 */
{
  byte	*p1, *p2;
  word	*q1, *q2;
  int	n1, n2, n, nd, i, iq;

  iq = ps[0];
  if (!symbolIsNumericalArray(iq) || array_num_dims(iq) != 2)
    return cerror(NEED_2D_ARR, iq);
  if (array_type(iq) != ANA_BYTE)
    return cerror(NEED_BYTE, iq);
  p1 = array_data(iq);
  fade_xsize = array_dims(iq)[0];
  fade_ysize = array_dims(iq)[1];
  fade_dim[0] = fade_xsize;
  fade_dim[1] = fade_ysize;
  
  iq = ps[1];
  if (!symbolIsNumericalArray(iq) || array_num_dims(iq) != 2)
    return cerror(NEED_2D_ARR, iq);
  if (array_type(iq) != ANA_BYTE)
    return cerror(NEED_BYTE, iq);
  if (array_dims(iq)[0] != fade_xsize || array_dims(iq)[1] != fade_ysize)
    return cerror(INCMP_ARG, iq);

  p2 = array_data(iq);
  /* set up the I*2 arrays for quick fades */
  if (fade1) { free(fade1); fade1 = NULL; }
  if (fade2) { free(fade2); fade2 = NULL; }
  /* since the inputs are byte arrays, n1 is the size in bytes, we need 2*n1
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
  return ANA_OK;
}
/*------------------------------------------------------------------------- */
int ana_fade(int narg, int ps[])
 /* does a fade (or dissolve) between 2 byte arrays, must be initialized
 with fade_init
 call is: fade, weight, result - where n is from 0 to 256 and represents
 the weight of the first array that was passed to fade_init
 result is a subroutine argument to avoid mallocs */
{
  byte	*p1;
  word	*q1, *q2, wt;
  int	weight, n, iq;

  if (int_arg_stat(ps[0], &weight) != ANA_OK) return ANA_ERROR;
  wt = (word) weight;
  iq = ps[1];
  if (redef_array(iq, 0, 2, fade_dim) != ANA_OK) return ANA_ERROR;

  p1 = array_data(iq);
  n = fade_xsize * fade_ysize;	q1 = fade1;	q2 = fade2;
  while (n--) { 
    *p1++ = (byte) (( wt * *q2++ + *q1++) >> 8);
  }
  return ANA_OK;
}
/*------------------------------------------------------------------------- */
