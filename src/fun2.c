/* This is file fun2.c.

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
/* File fun2.c */
/* Various LUX functions. */
/* internal lux subroutines and functions */
/*this is file fun2.c */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/time.h>		/* for select() */
#include <sys/types.h>
/*#include <malloc.h>*/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>
#include "action.h"
#include "install.h"

extern int32_t	nFixed;
static	int32_t	result_sym , type, detrend_flag;
static	char	templine[256];		/* a temp for output and istring */
void	zerobytes(void *, int32_t);
int32_t	f_decomp(float *, int32_t, int32_t), d_decomp(double *, int32_t, int32_t),
  f_solve(float *, float *, int32_t, int32_t), d_solve(double *, double *, int32_t, int32_t),
  lux_replace(int32_t, int32_t);
/*------------------------------------------------------------------------- */
int32_t lux_runsum(int32_t narg, int32_t ps[])
/*generate a running sum
  syntax:  y = runsum(x [,axis,order])
  <axis> is the dimension along which summing is performed; for each of the
  remaining coordinates summing is started over at zero.  <order> is the
  summation order, such that n-th order summation is the reverse of the
  n-th order difference (see lux_differ).  If <axis> isn't specified,
  or is negative, then <x> is taken to be a 1D array and summation is
  done with floating point or double-precision arithmetic.  Otherwise,
  the result has the same type as <x>.  axis & order extension by LS 26nov92
  implementation of 1D double-precision case LS 11feb2007 */
{
 register pointer q1,q2,p;
 int32_t	result_sym, iq, axis, m, n, i, j, done, *dims, ndim, tally[8], step[8];
 int32_t	xdims[8], order, size;
 array	*h;

 if (narg > 2) order = int_arg(ps[2]); else order = -1;
 if (narg > 1) axis = int_arg(ps[1]); else axis = -1;
 iq = ps[0];
 if (iq < 0) return iq;					/* error pass-thru */
 if (sym[iq].class == LUX_SCAL_PTR) iq = dereferenceScalPointer(iq);
							/*float the input */
 type = sym[iq].type;
 if (axis == -1 && type < LUX_FLOAT) {
   iq = lux_float(1, ps);	/* pseudo-1D case */
   type = LUX_FLOAT;
 }
 switch (sym[iq].class)	{
   case LUX_SCALAR:
	/*trivial, just return value */
     return iq;
   case LUX_ARRAY:
     h = HEAD(iq);
     if (axis >= 0)					/* axis specified */
     { dims = h->dims; ndim = h->ndim; 
       if (axis >= ndim) return cerror(ILL_DIM, ps[1]); }
     else
     { dims = &m; ndim = 1; axis = 0; }
     if (axis >= 0 && h->dims[axis] < ABS(order) + 1) /* nothing to do */
       return ps[0];
     q1.l = LPTR(h);
     GET_SIZE(m, h->dims, h->ndim);
     if (ABS(order) > m - 1)
     { puts("Runsum order too large for input array");
       return cerror(INCMP_ARG, ps[2]); }
     if (order == 0) order = 1;
     memcpy(xdims, dims, sizeof(int32_t)*ndim);		/* copy dims */
     result_sym = array_clone(iq, type);
     h = HEAD(result_sym);
     q2.l = LPTR(h);
	     /* set up for walk through array */
     for (i = 0; i < ndim; i++) tally[i] = 1;
     n = size = *step = lux_type_size[type];
     for (i = 1; i < ndim; i++) step[i] = (n *= dims[i - 1]);
     if (axis)				 /* put requested axis first */
     { n = *step; *step = step[axis]; step[axis] = n;
       n = *xdims; *xdims = xdims[axis]; xdims[axis] = n; }
     for (i = ndim - 1; i; i--) step[i] -= step[i - 1]*xdims[i - 1];
	/* treat edge */
     p.b = q2.b;
     for (i = 0; i < ABS(order); i++)
     { memcpy(q2.b, q1.b, size);
       q2.b += *step;  q1.b += *step; }
     *xdims -= ABS(order);
     do                                         /* main loop */
     { switch (type)
       { case LUX_BYTE:	*q2.b = *q1.b + *p.b; break;
         case LUX_WORD:	*q2.w = *q1.w + *p.w; break;
         case LUX_LONG:	*q2.l = *q1.l + *p.l; break;
         case LUX_FLOAT:	*q2.f = *q1.f + *p.f; break;
         case LUX_DOUBLE:	*q2.d = *q1.d + *p.d; break; }
       q2.b += *step; q1.b += *step; p.b += *step;
       for (i = 0; i < ndim; i++)
       { if (tally[i]++ != xdims[i])
         { done = 0;
           break; }
         tally[i] = 1; done = 1;
         q1.b += step[i + 1];
         q2.b += step[i + 1]; }
       if (i > 0 && i < ndim)	/* start of new line -- improved 19feb95 LS */
       { p.b = q2.b;
	 for (j = 0; j < ABS(order); j++)
	 { memcpy(q2.b, q1.b, size);
	   q2.b += *step;
	   q1.b += *step; }
       }
     } while (!done);
     break;
   default:
     return cerror(ILL_CLASS, iq);
 }
 return result_sym;
}						/*end of lux_runsum */
/*------------------------------------------------------------------------- */
int32_t index_sdev(int32_t narg, int32_t ps[], int32_t sq)
/* calculates standard deviations or variances by class. */
/* f(<x>, <classes> [, <weights>, /SAMPLE, /POPULATION, /DOUBLE]) */
/* we assume that ps[0] and ps[1] are numerical arrays with the same number */
/* of dimensions. */
/* see at sdev() for more info. */
{
  int32_t	type, offset, *indx, i, size, result, nElem, indices2,
  	outType, haveWeights, shift;
  pointer	src, trgt, sum, weights, hist;
  scalar	temp;
  extern scalar	lastmin, lastmax;
  int32_t	minmax(int32_t *, int32_t, int32_t);

  if (narg > 2 && ps[2]) {	/* have <weights> */
    if (!symbolIsNumericalArray(ps[2]) /* not a numerical array */
	|| array_size(ps[2]) != array_size(ps[0])) /* or has the wrong size */
      return cerror(INCMP_ARG, ps[2]);
    haveWeights = 1;
  } else
    haveWeights = 0;

  shift = (internalMode & 1)? 0: 1;

  src.v = array_data(ps[0]);	/* source data */
  nElem = array_size(ps[0]);	/* number of source elements */
  type = array_type(ps[0]);	/* source data type */
  if (internalMode & 4)		/* /DOUBLE */
    outType = LUX_DOUBLE;
  else if (isComplexType(type))
    outType = (type == LUX_CDOUBLE)? LUX_DOUBLE: LUX_FLOAT;
  else
    outType = (type == LUX_DOUBLE)? LUX_DOUBLE: LUX_FLOAT;

  /* make <weights> have the same type as <x> (except not complex) */
  if (haveWeights) {
    haveWeights = lux_converts[realType(outType)](1, &ps[2]);
    weights.v = array_data(haveWeights);
  }

  /* need min and max of indices so we can create result array of */
  /* proper size */
  indices2 = lux_long(1, &ps[1]); /* force LUX_LONG */
  indx = array_data(indices2);	/* assumed of same size as <source>! */
  minmax(indx, nElem, LUX_LONG);
  size = lastmax.l + 1;
  offset = 0;
  if (lastmin.l < 0)
    size += (offset = -lastmin.l);
  result = array_scratch(outType, 1, &size);
  trgt.l = array_data(result);
  allocate(sum.b, size*lux_type_size[outType], uint8_t);
  zerobytes(trgt.b, size*lux_type_size[outType]);
  zerobytes(sum.b, size*lux_type_size[outType]);
  trgt.b += offset*lux_type_size[outType];
  sum.b += offset*lux_type_size[outType];
  i = nElem;
  if (haveWeights) {
    allocate(hist.d, size, double);
    zerobytes(hist.d, size*sizeof(double));
    hist.d += offset;
    switch (outType) {
      case LUX_FLOAT:
	switch (type) {
	  case LUX_BYTE:
	    while (i--) {		/* first, get the sum */
	      hist.f[*indx] += *weights.b;
	      sum.f[*indx] += (float) *src.b++ * *weights.b++;
	      indx++;
	    }
	    /* calculate the average */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.f[i])
		sum.f[i] /= hist.f[i];
	    src.b -= nElem;
	    indx -= nElem;
	    weights.b -= nElem;
	    /* now the squared deviations */
	    while (nElem--) {
	      temp.f = (float) *src.b++ - sum.f[*indx];
	      trgt.f[*indx++] += temp.f*temp.f* *weights.b++;
	    }
	    break;
	  case LUX_WORD:
	    while (i--) {		/* first, get the sum */
	      hist.f[*indx] += *weights.w;
	      sum.f[*indx] += (float) *src.w++ * *weights.w++;
	      indx++;
	    }
	    /* calculate the average */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.f[i])
		sum.f[i] /= hist.f[i];
	    src.w -= nElem;
	    indx -= nElem;
	    weights.w -= nElem;
	    /* now the squared deviations */
	    while (nElem--) {
	      temp.f = (float) *src.w++ - sum.f[*indx];
	      trgt.f[*indx++] += temp.f*temp.f* *weights.w++;
	    }
	    break;
	  case LUX_LONG:
	    while (i--) {		/* first, get the sum */
	      hist.f[*indx] += *weights.l;
	      sum.f[*indx] += (float) *src.l++ * *weights.l++;
	      indx++;
	    }
	    /* calculate the average */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.f[i])
		sum.f[i] /= hist.f[i];
	    src.l -= nElem;
	    indx -= nElem;
	    weights.l -= nElem;
	    /* now the squared deviations */
	    while (nElem--) {
	      temp.f = (float) *src.l++ - sum.f[*indx];
	      trgt.f[*indx++] += temp.f*temp.f* *weights.l++;
	    }
	    break;
	  case LUX_FLOAT:
	    while (i--) {		/* first, get the sum */
	      hist.f[*indx] += *weights.f;
	      sum.f[*indx] += (float) *src.f++ * *weights.f++;
	      indx++;
	    }
	    /* calculate the average */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.f[i])
		sum.f[i] /= hist.f[i];
	    src.f -= nElem;
	    indx -= nElem;
	    weights.f -= nElem;
	    /* now the squared deviations */
	    while (nElem--) {
	      temp.f = (float) *src.f++ - sum.f[*indx];
	      trgt.f[*indx++] += temp.f*temp.f* *weights.f++;
	    }
	    break;
	  /* no case LUX_DOUBLE: if <x> is DOUBLE, then the output */
	  /* is also DOUBLE */
	  case LUX_CFLOAT:
	    while (i--) {		/* first, get the sum */
	      hist.f[*indx] += *weights.f;
	      sum.cf[*indx].real += src.cf->real * *weights.f;
	      sum.cf[*indx].imaginary += src.cf->imaginary* *weights.f;
	      src.cf++;
	      indx++;
	      weights.f++;
	    }
	    /* calculate the average */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.f[i]) {
		sum.cf[i].real /= hist.f[i];
		sum.cf[i].imaginary /= hist.f[i];
	      }
	    src.cf -= nElem;
	    indx -= nElem;
	    weights.f -= nElem;
	    /* now the squared deviations */
	    while (nElem--) {
	      temp.f = src.cf->real - sum.cf[*indx].real;
	      trgt.f[*indx] += temp.f*temp.f* *weights.f;
	      temp.f = src.cf->imaginary - sum.cf[*indx].imaginary;
	      trgt.f[*indx++] += temp.f*temp.f* *weights.f++;
	      src.cf++;
	    }
	    break;
	  /* no case LUX_CDOUBLE: if <x> is CDOUBLE, then the output */
	  /* is also CDOUBLE */
	}
	/* and divide by number */
	for (i = -offset; i < size - offset; i++)
	  if (hist.f[i])
	    trgt.f[i] /= hist.f[i];
	break;
      case LUX_DOUBLE:
	switch (type) {
	  case LUX_BYTE:
	    while (i--) {		/* first, get the sum */
	      hist.d[*indx] += *weights.b;
	      sum.d[*indx] += (double) *src.b++ * *weights.b++;
	      indx++;
	    }
	    /* calculate the average */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.d[i])
		sum.d[i] /= hist.d[i];
	    src.b -= nElem;
	    indx -= nElem;
	    weights.b -= nElem;
	    /* now the squared deviations */
	    while (nElem--) {
	      temp.d = (double) *src.b++ - sum.d[*indx];
	      trgt.d[*indx++] += temp.d*temp.d* *weights.b++;
	    }
	    break;
	  case LUX_WORD:
	    while (i--) {		/* first, get the sum */
	      hist.d[*indx] += *weights.w;
	      sum.d[*indx] += (double) *src.w++ * *weights.w++;
	      indx++;
	    }
	    /* calculate the average */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.d[i])
		sum.d[i] /= hist.d[i];
	    src.w -= nElem;
	    indx -= nElem;
	    weights.w -= nElem;
	    /* now the squared deviations */
	    while (nElem--) {
	      temp.d = (double) *src.w++ - sum.d[*indx];
	      trgt.d[*indx++] += temp.d*temp.d* *weights.w++;
	    }
	    break;
	  case LUX_LONG:
	    while (i--) {		/* first, get the sum */
	      hist.d[*indx] += *weights.l;
	      sum.d[*indx] += (double) *src.l++ * *weights.l++;
	      indx++;
	    }
	    /* calculate the average */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.d[i])
		sum.d[i] /= hist.d[i];
	    src.l -= nElem;
	    indx -= nElem;
	    weights.l -= nElem;
	    /* now the squared deviations */
	    while (nElem--) {
	      temp.d = (double) *src.l++ - sum.d[*indx];
	      trgt.d[*indx++] += temp.d*temp.d* *weights.l++;
	    }
	    break;
	  case LUX_FLOAT:
	    while (i--) {		/* first, get the sum */
	      hist.d[*indx] += *weights.f;
	      sum.d[*indx] += (double) *src.f++ * *weights.f++;
	      indx++;
	    }
	    /* calculate the average */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.d[i])
		sum.d[i] /= hist.d[i];
	    src.f -= nElem;
	    indx -= nElem;
	    weights.f -= nElem;
	    /* now the squared deviations */
	    while (nElem--) {
	      temp.d = (double) *src.f++ - sum.d[*indx];
	      trgt.d[*indx++] += temp.d*temp.d* *weights.f++;
	    }
	    break;
	  case LUX_DOUBLE:
	    while (i--) {		/* first, get the sum */
	      hist.d[*indx] += *weights.d;
	      sum.d[*indx] += *src.d++ * *weights.d++;
	      indx++;
	    }
	    /* calculate the average */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.d[i])
		sum.d[i] /= hist.d[i];
	    src.d -= nElem;
	    indx -= nElem;
	    weights.d -= nElem;
	    /* now the squared deviations */
	    while (nElem--) {
	      temp.d = *src.d++ - sum.d[*indx];
	      trgt.d[*indx++] += temp.d*temp.d* *weights.d++;
	    }
	    break;
	  case LUX_CFLOAT:
	    while (i--) {		/* first, get the sum */
	      hist.d[*indx] += *weights.f;
	      sum.cd[*indx].real += src.cf->real * *weights.f;
	      sum.cd[*indx].imaginary += src.cf->imaginary * *weights.f;
	      src.cf++;
	      indx++;
	      weights.f++;
	    }
	    /* calculate the average */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.d[i]) {
		sum.cd[i].real /= hist.d[i];
		sum.cd[i].imaginary /= hist.d[i];
	      }
	    src.cf -= nElem;
	    indx -= nElem;
	    weights.f -= nElem;
	    /* now the squared deviations */
	    while (nElem--) {
	      temp.d = src.cf->real - sum.cd[*indx].real;
	      trgt.d[*indx] += temp.d*temp.d* *weights.f;
	      temp.d = src.cf->imaginary - sum.cd[*indx].imaginary;
	      trgt.d[*indx++] += temp.d*temp.d* *weights.f++;
	      src.cf++;
	    }
	    break;
	  case LUX_CDOUBLE:
	    while (i--) {		/* first, get the sum */
	      hist.d[*indx] += *weights.d;
	      sum.cd[*indx].real += src.cd->real * *weights.d;
	      sum.cd[*indx].imaginary += src.cd->imaginary * *weights.d;
	      src.cd++;
	      indx++;
	      weights.d++;
	    }
	    /* calculate the average */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.d[i]) {
		sum.cd[i].real /= hist.d[i];
		sum.cd[i].imaginary /= hist.d[i];
	      }
	    src.cd -= nElem;
	    indx -= nElem;
	    weights.d -= nElem;
	    /* now the squared deviations */
	    while (nElem--) {
	      temp.d = src.cd->real - sum.cd[*indx].real;
	      trgt.d[*indx] += temp.d*temp.d* *weights.d;
	      temp.d = src.cd->imaginary - sum.cd[*indx].imaginary;
	      trgt.d[*indx++] += temp.d*temp.d* *weights.d++;
	      src.cd++;
	    }
	    break;
	}
	/* and divide by number */
	for (i = -offset; i < size - offset; i++)
	  if (hist.d[i])
	    trgt.d[i] /= hist.d[i];
	break;
    }
    free(hist.d - offset);
  } else {			/* no <weights>: each element counts once */
    allocate(hist.l, size, int32_t);
    zerobytes(hist.l, size*sizeof(int32_t));
    hist.l += offset;
    switch (outType) {
      case LUX_FLOAT:
	switch (type) {
	  case LUX_BYTE:
	    while (i--) {		/* first, get the sum */
	      hist.l[*indx]++;
	      sum.f[*indx] += *src.b++;
	      indx++;
	    }
	    /* calculate the average */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.l[i])
		sum.f[i] /= hist.l[i];
	    src.b -= nElem;
	    indx -= nElem;
	    /* now the squared deviations */
	    while (nElem--) {
	      temp.f = (float) *src.b++ - sum.f[*indx];
	      trgt.f[*indx++] += temp.f*temp.f;
	    }
	    break;
	  case LUX_WORD:
	    while (i--) {		/* first, get the sum */
	      hist.l[*indx]++;
	      sum.f[*indx] += *src.w++;
	      indx++;
	    }
	    /* calculate the average */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.l[i])
		sum.f[i] /= hist.l[i];
	    src.w -= nElem;
	    indx -= nElem;
	    /* now the squared deviations */
	    while (nElem--) {
	      temp.f = (float) *src.w++ - sum.f[*indx];
	      trgt.f[*indx++] += temp.f*temp.f;
	    }
	    break;
	  case LUX_LONG:
	    while (i--) {		/* first, get the sum */
	      hist.l[*indx]++;
	      sum.f[*indx] += *src.l++;
	      indx++;
	    }
	    /* calculate the average */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.l[i])
		sum.f[i] /= hist.l[i];
	    src.l -= nElem;
	    indx -= nElem;
	    /* now the squared deviations */
	    while (nElem--) {
	      temp.f = (float) *src.l++ - sum.f[*indx];
	      trgt.f[*indx++] += temp.f*temp.f;
	    }
	    break;
	  case LUX_FLOAT:
	    while (i--) {		/* first, get the sum */
	      hist.l[*indx]++;
	      sum.f[*indx] += *src.f++;
	      indx++;
	    }
	    /* calculate the average */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.l[i])
		sum.f[i] /= hist.l[i];
	    src.f -= nElem;
	    indx -= nElem;
	    /* now the squared deviations */
	    while (nElem--) {
	      temp.f = *src.f++ - sum.f[*indx];
	      trgt.f[*indx++] += temp.f*temp.f;
	    }
	    break;
          /* no case LUX_DOUBLE: if <x> is DOUBLE then so is the output */
	  case LUX_CFLOAT:
	    while (i--) {		/* first, get the sum */
	      hist.l[*indx]++;
	      sum.cf[*indx].real += src.cf->real;
	      sum.cf[*indx].imaginary += src.cf->imaginary;
	      src.cf++;
	      indx++;
	    }
	    /* calculate the average */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.l[i]) {
		sum.cf[i].real /= hist.l[i];
		sum.cf[i].imaginary /= hist.l[i];
	      }
	    src.cf -= nElem;
	    indx -= nElem;
	    /* now the squared deviations */
	    while (nElem--) {
	      temp.f = src.cf->real - sum.cf[*indx].real;
	      trgt.f[*indx] += temp.f*temp.f;
	      temp.f = src.cf->imaginary - sum.cf[*indx].imaginary;
	      trgt.f[*indx++] += temp.f*temp.f;
	      src.cf++;
	    }
	    break;
	  /* no case LUX_CDOUBLE: if <x> is CDOUBLE, then the output */
	  /* is also CDOUBLE */
	}
	/* and divide by number */
	for (i = -offset; i < size - offset; i++)
	  if (hist.l[i] > 1)
	    trgt.f[i] = trgt.f[i]/(hist.l[i] - shift);
	break;
      case LUX_DOUBLE:
	switch (type) {
	  case LUX_BYTE:
	    while (i--) {		/* first, get the sum */
	      hist.l[*indx]++;
	      sum.d[*indx] += *src.b++;
	      indx++;
	    }
	    /* calculate the average */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.l[i])
		sum.d[i] /= hist.l[i];
	    src.b -= nElem;
	    indx -= nElem;
	    /* now the squared deviations */
	    while (nElem--) {
	      temp.d = (double) *src.b++ - sum.d[*indx];
	      trgt.d[*indx++] += temp.d*temp.d;
	    }
	    break;
	  case LUX_WORD:
	    while (i--) {		/* first, get the sum */
	      hist.l[*indx]++;
	      sum.d[*indx] += *src.w++;
	      indx++;
	    }
	    /* calculate the average */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.l[i])
		sum.d[i] /= hist.l[i];
	    src.w -= nElem;
	    indx -= nElem;
	    /* now the squared deviations */
	    while (nElem--) {
	      temp.d = (double) *src.w++ - sum.d[*indx];
	      trgt.d[*indx++] += temp.d*temp.d;
	    }
	    break;
	  case LUX_LONG:
	    while (i--) {		/* first, get the sum */
	      hist.l[*indx]++;
	      sum.d[*indx] += *src.l++;
	      indx++;
	    }
	    /* calculate the average */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.l[i])
		sum.d[i] /= hist.l[i];
	    src.l -= nElem;
	    indx -= nElem;
	    /* now the squared deviations */
	    while (nElem--) {
	      temp.d = (double) *src.l++ - sum.d[*indx];
	      trgt.d[*indx++] += temp.d*temp.d;
	    }
	    break;
	  case LUX_FLOAT:
	    while (i--) {		/* first, get the sum */
	      hist.l[*indx]++;
	      sum.d[*indx] += (double) *src.f++;
	      indx++;
	    }
	    /* calculate the average */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.l[i])
		sum.d[i] /= hist.l[i];
	    src.f -= nElem;
	    indx -= nElem;
	    /* now the squared deviations */
	    while (nElem--) {
	      temp.d = (double) *src.f++ - sum.d[*indx];
	      trgt.d[*indx++] += temp.d*temp.d;
	    }
	    break;
	  case LUX_DOUBLE:
	    while (i--) {		/* first, get the sum */
	      hist.l[*indx]++;
	      sum.d[*indx] += *src.d++;
	      indx++;
	    }
	    /* calculate the average */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.l[i])
		sum.d[i] /= hist.l[i];
	    src.d -= nElem;
	    indx -= nElem;
	    /* now the squared deviations */
	    while (nElem--) {
	      temp.d = *src.d++ - sum.d[*indx];
	      trgt.d[*indx++] += temp.d*temp.d;
	    }
	    break;
	  case LUX_CFLOAT:
	    while (i--) {		/* first, get the sum */
	      hist.l[*indx]++;
	      sum.cd[*indx].real += src.cf->real;
	      sum.cd[*indx].imaginary += src.cf->imaginary;
	      src.cf++;
	      indx++;
	    }
	    /* calculate the average */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.l[i]) {
		sum.cd[i].real /= hist.l[i];
		sum.cd[i].imaginary /= hist.l[i];
	      }
	    src.cf -= nElem;
	    indx -= nElem;
	    /* now the squared deviations */
	    while (nElem--) {
	      temp.d = (double) src.cf->real - sum.cd[*indx].real;
	      trgt.d[*indx] += temp.d*temp.d;
	      temp.d = (double) src.cf->imaginary - sum.cd[*indx].imaginary;
	      trgt.d[*indx++] += temp.d*temp.d;
	      src.cf++;
	    }
	    break;
	  case LUX_CDOUBLE:
	    while (i--) {		/* first, get the sum */
	      hist.l[*indx]++;
	      sum.cd[*indx].real += src.cd->real;
	      sum.cd[*indx].imaginary += src.cd->imaginary;
	      src.cd++;
	      indx++;
	    }
	    /* calculate the average */
	    for (i = -offset; i < size - offset; i++)
	      if (hist.l[i]) {
		sum.cd[i].real /= hist.l[i];
		sum.cd[i].imaginary /= hist.l[i];
	      }
	    src.cd -= nElem;
	    indx -= nElem;
	    /* now the squared deviations */
	    while (nElem--) {
	      temp.d = src.cd->real - sum.cd[*indx].real;
	      trgt.d[*indx] += temp.d*temp.d;
	      temp.d = src.cd->imaginary - sum.cd[*indx].imaginary;
	      trgt.d[*indx++] += temp.d*temp.d;
	      src.cd++;
	    }
	    break;
	}
	/* and divide by number */
	for (i = -offset; i < size - offset; i++)
	  if (hist.l[i] > 1)
	    trgt.d[i] = trgt.d[i]/(hist.l[i] - shift);
	break;
    }
    free(hist.l - offset);
  }
  if (sq)			/* standard deviation rater than variance */
    switch (outType) {
    case LUX_FLOAT:
      while (size--) {
	*trgt.f = sqrt(*trgt.f);
	trgt.f++;
      }
      break;
    case LUX_DOUBLE:
      while (size--) {
	*trgt.d = sqrt(*trgt.d);
	trgt.d++;
      }
      break;
    }
  free(sum.b - offset*lux_type_size[outType]);
  return result;
}
/*------------------------------------------------------------------------- */
int32_t lux_covariance(int32_t narg, int32_t ps[])
/* f(<x> , <y> [, <mode>, WEIGHTS=<weights>, /SAMPLE, /POPULATION, /KEEPDIMS, */
/*         /DOUBLE]) */
/* Returns the covariance of <x> and <y>, which must be */
/* numerical arrays with the same dimentions.  By default, returns the */
/* sample covariance. */
/* <mode>: if it is a scalar, then returns a result for each run along the */
/*    dimension indicated by <mode>.  If it is an array with the same number */
/*    of elements as <x>, then each element of <mode> identifies the integer */
/*    class that the corresponding element of <x> belongs to, and a result */
/*    is returned for each class. */
/* <weights>: the weight of each element of <x>.  This parameter must be a */
/*    numerical array with the same dimensions as <x>.  If <weights> is */
/*    specified, then /SAMPLE is ignored. */
/* /SAMPLE: return the sample covariance. */
/* /POPULATION: return the population covariance. */
/* /KEEPDIMS: if this keyword is specified, then the dimension(s) along */
/*    which the calculations of the deviations or variances are performed */
/*    are set to 1 in the result.  If this keyword is not specified, then */
/*    any dimensions equal to 1 in the result are omitted, and if only a */
/*    single value is returned then this is returned as a scalar. */
/* /DOUBLE: by default, the result is DOUBLE only if <x> is DOUBLE or */
/*    CDOUBLE, and the result is FLOAT otherwise.  If this keyword is set, */
/*    then the result is always DOUBLE. */
/* LS 2011-04-15 */
{
  int32_t	result, n, i, done, n2, save[MAX_DIMS], outtype, haveWeights;
  pointer	xsrc, ysrc, trgt, xsrc0, ysrc0, weight, weight0;
  double	xmean, ymean, xtemp, ytemp, cov, nn;
  loopInfo	xsrcinfo, ysrcinfo, trgtinfo, winfo;
  extern scalar	lastmean;
  extern int32_t	lastsdev_sym, lastmean_sym;
  
  /* return values by class? */
#ifdef IMPLEMENT_LATER
  if (narg > 2 && ps[2] && symbolIsNumericalArray(ps[2])
      && symbolIsNumericalArray(ps[0])
      && symbolIsNumericalArray(ps[1])
      && array_size(ps[0]) == array_size(ps[1])
      && array_size(ps[0]) == array_size(ps[2]))
    return index_covariance(narg, ps);
#endif

  if (internalMode & 4) 	/* /DOUBLE */
    outtype = LUX_DOUBLE;
  else if (isComplexType(symbol_type(ps[0])))
      outtype = (symbol_type(ps[0]) == LUX_CFLOAT)? LUX_FLOAT: LUX_DOUBLE;
    else
      outtype = (symbol_type(ps[0]) == LUX_DOUBLE)? LUX_DOUBLE: LUX_FLOAT;

  if (isComplexType(symbol_type(ps[1]))) {
    if (!isComplexType(outtype)
        || symbol_type(ps[1]) > outtype)
      outtype = symbol_type(ps[1]);
  } else if (symbol_type(ps[1]) > outtype)
    outtype = symbol_type(ps[1]);

  if (!symbolIsNumericalArray(ps[0]))
    return cerror(NEED_NUM_ARR, ps[0]);
  if (!symbolIsNumericalArray(ps[1]))
    return cerror(NEED_NUM_ARR, ps[1]);
  if (array_size(ps[0]) != array_size(ps[1])
      || array_num_dims(ps[0]) != array_num_dims(ps[1]))
    return cerror(INCMP_ARG, ps[1]);
  for (i = 0; i < array_num_dims(ps[0]); i++)
    if (array_dims(ps[1])[i] != array_dims(ps[0])[i])
      return cerror(INCMP_DIMS, ps[1]);
  
  if (narg > 3 && ps[3]) {	/* have <weights> */
    if (!symbolIsNumericalArray(ps[3])	/* but it's not a numerical array */
	|| array_size(ps[3]) != array_size(ps[0])) /* or has wrong size */
      return cerror(INCMP_ARG, ps[3]);
    for (i = 0; i < array_num_dims(ps[3]) - 1; i++) /* check the dimensions */
      if (array_dims(ps[3])[i] != array_dims(ps[0])[i])
	return cerror(INCMP_DIMS, ps[3]);
    haveWeights = 1;
  } else
    haveWeights = 0;
    
  /* set up for traversing the data, and create an output symbol, too */
  if (standardLoop(ps[0], (narg > 2 && ps[2])? ps[2]: 0,
		   SL_COMPRESSALL /* omit all axis dimensions from result */
		   | (haveWeights? 0: SL_ALLAXES)
		   | SL_EXACT | SL_SRCUPGRADE
		   | SL_EACHCOORD /* need all coordinates */
		   | SL_UNIQUEAXES /* no duplicate axes allowed */
		   | SL_AXESBLOCK /* treat axes as a block */
		   | ((internalMode & 2)? SL_ONEDIMS: 0), /* omit -> 1 */
                   outtype, &xsrcinfo, &xsrc, &result, &trgtinfo, &trgt) < 0)
    return LUX_ERROR;
  
  if (standardLoop(ps[1], (narg > 2 && ps[2])? ps[2]: 0,
		   SL_COMPRESSALL /* omit all axis dimensions from result */
		   | (haveWeights? 0: SL_ALLAXES)
		   | SL_SRCUPGRADE
		   | SL_EACHCOORD /* need all coordinates */
		   | SL_UNIQUEAXES /* no duplicate axes allowed */
		   | SL_AXESBLOCK /* treat axes as a block */
		   | ((internalMode & 2)? SL_ONEDIMS: 0), /* omit -> 1 */
                   outtype, &ysrcinfo, &ysrc, NULL, NULL, NULL) < 0)
    return LUX_ERROR;

  /* set up for traversing the weights, too */
  if (haveWeights) {
    /* we make sure that <weights> has the same data type as <x> -- except */
    /* that <weights> is never complex */
    haveWeights = lux_converts[realType(outtype)](1, &ps[3]);
    if (standardLoop(haveWeights, ps[2],
		     (ps[2]? 0: SL_ALLAXES) | SL_EACHCOORD | SL_AXESBLOCK,
		     0, &winfo, &weight, NULL, NULL, NULL) < 0)
      return LUX_ERROR;
  }
		      
  if (!xsrcinfo.naxes) {
    xsrcinfo.naxes++;
    ysrcinfo.naxes++;
    if (haveWeights)
      winfo.naxes++;
  }

  /* we make two passes: one to get the average, and then one to calculate */
  /* the standard deviation; this way we limit truncation and roundoff */
  /* errors */
  if (haveWeights) {
    switch (symbol_type(ps[0])) {
      case LUX_BYTE:
	do {
	  xmean = 0.0;
          ymean = 0.0;
	  memcpy(save, xsrcinfo.coords, xsrcinfo.ndim*sizeof(int32_t));
	  xsrc0 = xsrc;
          ysrc0 = ysrc;
	  weight0 = weight;
	  nn = 0;
	  do {
	    xmean += (double) *xsrc.b * *weight.b;
            ymean += (double) *ysrc.b * *weight.b;
	    nn += (double) *weight.b;
	  }
	  while (advanceLoop(&winfo, &weight), 
		 advanceLoop(&ysrcinfo, &ysrc),
                 advanceLoop(&xsrcinfo, &xsrc) < xsrcinfo.naxes);
	  memcpy(xsrcinfo.coords, save, xsrcinfo.ndim*sizeof(int32_t));
	  memcpy(ysrcinfo.coords, save, ysrcinfo.ndim*sizeof(int32_t));
	  xsrc = xsrc0;
          ysrc = ysrc0;
	  weight = weight0;
	  xmean /= (nn? nn: 1);
          ymean /= (nn? nn: 1);
	  cov = 0.0;
	  do {
	    xtemp = ((double) *xsrc.b - xmean);
            ytemp = ((double) *ysrc.b - ymean);
	    cov += xtemp*ytemp* *weight.b;
	  } while ((done = (advanceLoop(&winfo, &weight),
			    advanceLoop(&ysrcinfo, &ysrc),
			    advanceLoop(&xsrcinfo, &xsrc))) < xsrcinfo.naxes);
	  if (!nn)
	    nn = 1;
	  cov /= nn;
	  switch (outtype) {
	    case LUX_FLOAT:
	      *trgt.f++ = (float) cov;
	      break;
	    case LUX_DOUBLE:
	      *trgt.d++ = cov;
	      break;
	  }
	} while (done < xsrcinfo.rndim);
	break;
      case LUX_WORD:
	do {
	  xmean = 0.0;
          ymean = 0.0;
	  memcpy(save, xsrcinfo.coords, xsrcinfo.ndim*sizeof(int32_t));
	  xsrc0 = xsrc;
          ysrc0 = ysrc;
	  weight0 = weight;
	  nn = 0;
	  do {
	    xmean += (double) *xsrc.w * *weight.w;
            ymean += (double) *ysrc.w * *weight.w;
	    nn += (double) *weight.w;
	  }
	  while (advanceLoop(&winfo, &weight),
		 advanceLoop(&ysrcinfo, &ysrc),
                 advanceLoop(&xsrcinfo, &xsrc) < xsrcinfo.naxes);
	  memcpy(xsrcinfo.coords, save, xsrcinfo.ndim*sizeof(int32_t));
	  memcpy(ysrcinfo.coords, save, ysrcinfo.ndim*sizeof(int32_t));
	  xsrc = xsrc0;
          ysrc = ysrc0;
	  weight = weight0;
	  xmean /= (nn? nn: 1);
          ymean /= (nn? nn: 1);
	  cov = 0.0;
	  do {
	    xtemp = ((double) *xsrc.w - xmean);
            ytemp = ((double) *ysrc.w - ymean);
	    cov += xtemp*ytemp* *weight.w;
	  } while ((done = (advanceLoop(&winfo, &weight),
			    advanceLoop(&ysrcinfo, &ysrc),
			    advanceLoop(&xsrcinfo, &xsrc))) < xsrcinfo.naxes);
	  if (!nn)
	    nn = 1;
	  cov /= nn;
	  switch (outtype) {
	    case LUX_FLOAT:
	      *trgt.f++ = (float) cov;
	      break;
	    case LUX_DOUBLE:
	      *trgt.d++ = cov;
	      break;
	  }
	} while (done < xsrcinfo.rndim);
	break;
      case LUX_LONG:
	do {
	  xmean = 0.0;
          ymean = 0.0;
	  memcpy(save, xsrcinfo.coords, xsrcinfo.ndim*sizeof(int32_t));
	  xsrc0 = xsrc;
          ysrc0 = ysrc;
	  weight0 = weight;
	  nn = 0;
	  do {
	    xmean += (double) *xsrc.l * *weight.l;
            ymean += (double) *ysrc.l * *weight.l;
	    nn += (double) *weight.l;
	  }
	  while (advanceLoop(&winfo, &weight),
		 advanceLoop(&ysrcinfo, &ysrc),
                 advanceLoop(&xsrcinfo, &xsrc) < xsrcinfo.naxes);
	  memcpy(xsrcinfo.coords, save, xsrcinfo.ndim*sizeof(int32_t));
	  memcpy(ysrcinfo.coords, save, ysrcinfo.ndim*sizeof(int32_t));
	  xsrc = xsrc0;
          ysrc = ysrc0;
	  weight = weight0;
	  xmean /= (nn? nn: 1);
          ymean /= (nn? nn: 1);
	  cov = 0.0;
	  do {
	    xtemp = ((double) *xsrc.l - xmean);
            ytemp = ((double) *ysrc.l - ymean);
	    cov += xtemp*ytemp* *weight.l;
	  } while ((done = (advanceLoop(&winfo, &weight),
			    advanceLoop(&ysrcinfo, &ysrc),
			    advanceLoop(&xsrcinfo, &xsrc))) < xsrcinfo.naxes);
	  if (!nn)
	    nn = 1;
	  cov /= nn;
	  switch (outtype) {
	    case LUX_FLOAT:
	      *trgt.f++ = (float) cov;
	      break;
	    case LUX_DOUBLE:
	      *trgt.d++ = cov;
	      break;
	  }
	} while (done < xsrcinfo.rndim);
	
	break;
      case LUX_FLOAT:
	do {
	  xmean = 0.0;
          ymean = 0.0;
	  memcpy(save, xsrcinfo.coords, xsrcinfo.ndim*sizeof(int32_t));
	  xsrc0 = xsrc;
          ysrc0 = ysrc;
	  weight0 = weight;
	  nn = 0;
	  do {
	    xmean += (double) *xsrc.f * *weight.f;
            ymean += (double) *ysrc.f * *weight.f;
	    nn += (double) *weight.f;
	  }
	  while (advanceLoop(&winfo, &weight),
		 advanceLoop(&ysrcinfo, &ysrc),
                 advanceLoop(&xsrcinfo, &xsrc) < xsrcinfo.naxes);
	  memcpy(xsrcinfo.coords, save, xsrcinfo.ndim*sizeof(int32_t));
	  memcpy(ysrcinfo.coords, save, ysrcinfo.ndim*sizeof(int32_t));
	  xsrc = xsrc0;
          ysrc = ysrc0;
	  weight = weight0;
	  xmean /= (nn? nn: 1);
          ymean /= (nn? nn: 1);
	  cov = 0.0;
	  do {
	    xtemp = ((double) *xsrc.f - xmean);
            ytemp = ((double) *ysrc.f - ymean);
	    cov += xtemp*ytemp* *weight.f;
	  } while ((done = (advanceLoop(&winfo, &weight),
			    advanceLoop(&ysrcinfo, &ysrc),
			    advanceLoop(&xsrcinfo, &xsrc))) < xsrcinfo.naxes);
	  if (!nn)
	    nn = 1;
	  cov /= nn;
	  switch (outtype) {
	    case LUX_FLOAT:
	      *trgt.f++ = (float) cov;
	      break;
	    case LUX_DOUBLE:
	      *trgt.d++ = cov;
	      break;
	  }
	} while (done < xsrcinfo.rndim);
	
	break;
      case LUX_DOUBLE:
	do {
	  xmean = 0.0;
          ymean = 0.0;
	  memcpy(save, xsrcinfo.coords, xsrcinfo.ndim*sizeof(int32_t));
	  xsrc0 = xsrc;
          ysrc0 = ysrc;
	  weight0 = weight;
	  nn = 0;
	  do {
	    xmean += (double) *xsrc.d * *weight.d;
            ymean += (double) *ysrc.d * *weight.d;
	    nn += (double) *weight.d;
	  }
	  while (advanceLoop(&winfo, &weight),
		 advanceLoop(&ysrcinfo, &ysrc),
                 advanceLoop(&xsrcinfo, &xsrc) < xsrcinfo.naxes);
	  memcpy(xsrcinfo.coords, save, xsrcinfo.ndim*sizeof(int32_t));
	  memcpy(ysrcinfo.coords, save, ysrcinfo.ndim*sizeof(int32_t));
	  xsrc = xsrc0;
          ysrc = ysrc0;
	  weight = weight0;
	  xmean /= (nn? nn: 1);
          ymean /= (nn? nn: 1);
	  cov = 0.0;
	  do {
	    xtemp = ((double) *xsrc.d - xmean);
            ytemp = ((double) *ysrc.d - ymean);
	    cov += xtemp*ytemp* *weight.d;
	  } while ((done = (advanceLoop(&winfo, &weight),
			    advanceLoop(&ysrcinfo, &ysrc),
			    advanceLoop(&xsrcinfo, &xsrc))) < xsrcinfo.naxes);
	  if (!nn)
	    nn = 1;
	  cov /= nn;
          *trgt.d++ = cov;
	} while (done < xsrcinfo.rndim);
	break;
    }
    zapTemp(haveWeights);	/* delete if it is a temp */
  } else {
    n = 1;			/* initialize number of values per sdev */
    for (i = 0; i < xsrcinfo.naxes; i++)
      n *= xsrcinfo.rdims[i];
    n2 = (internalMode & 1)? n: n - 1; /* sample or population sdev */
    if (!n2)
      return luxerror("Single values have no sample standard deviation", ps[0]);

    switch (symbol_type(ps[0])) {
      case LUX_BYTE:
	do {
	  xmean = 0.0;
          ymean = 0.0;
	  memcpy(save, xsrcinfo.coords, xsrcinfo.ndim*sizeof(int32_t));
	  xsrc0 = xsrc;
          ysrc0 = ysrc;
	  do {
	    xmean += (double) *xsrc.b;
            ymean += (double) *ysrc.b;
	  } while (advanceLoop(&ysrcinfo, &ysrc),
		   advanceLoop(&xsrcinfo, &xsrc) < xsrcinfo.naxes);
	  memcpy(xsrcinfo.coords, save, xsrcinfo.ndim*sizeof(int32_t));
	  memcpy(ysrcinfo.coords, save, ysrcinfo.ndim*sizeof(int32_t));
	  xsrc = xsrc0;
          ysrc = ysrc0;
	  xmean /= n;
          ymean /= n;
	  cov = 0.0;
	  do {
	    xtemp = ((double) *xsrc.b - xmean);
            ytemp = ((double) *ysrc.b - ymean);
	    cov += xtemp*ytemp;
	  } while ((done = (advanceLoop(&ysrcinfo, &ysrc),
			    advanceLoop(&xsrcinfo, &xsrc))) < xsrcinfo.naxes);
	  switch (outtype) {
	    case LUX_FLOAT:
	      *trgt.f++ = (float) (cov/n2);
	      break;
	    case LUX_DOUBLE:
	      *trgt.d++ = (cov/n2);
	      break;
	  }
	} while (done < xsrcinfo.rndim);
	break;
      case LUX_WORD:
	do {
	  xmean = 0.0;
          ymean = 0.0;
	  memcpy(save, xsrcinfo.coords, xsrcinfo.ndim*sizeof(int32_t));
	  memcpy(ysrcinfo.coords, save, ysrcinfo.ndim*sizeof(int32_t));
	  xsrc0 = xsrc;
          ysrc0 = ysrc;
	  do {
	    xmean += (double) *xsrc.w;
            ymean += (double) *ysrc.w;
	  } while (advanceLoop(&ysrcinfo, &ysrc),
		   advanceLoop(&xsrcinfo, &xsrc) < xsrcinfo.naxes);
	  memcpy(xsrcinfo.coords, save, xsrcinfo.ndim*sizeof(int32_t));
	  xsrc = xsrc0;
          ysrc = ysrc0;
	  xmean /= n;
          ymean /= n;
	  cov = 0.0;
	  do {
	    xtemp = ((double) *xsrc.w - xmean);
            ytemp = ((double) *ysrc.w - ymean);
	    cov += xtemp*ytemp;
	  } while ((done = (advanceLoop(&ysrcinfo, &ysrc),
			    advanceLoop(&xsrcinfo, &xsrc))) < xsrcinfo.naxes);
	  switch (outtype) {
	    case LUX_FLOAT:
	      *trgt.f++ = (float) (cov/n2);
	      break;
	    case LUX_DOUBLE:
	      *trgt.d++ = (cov/n2);
	      break;
	  }
	} while (done < xsrcinfo.rndim);
        break;
      case LUX_LONG:
	do {
	  xmean = 0.0;
          ymean = 0.0;
	  memcpy(save, xsrcinfo.coords, xsrcinfo.ndim*sizeof(int32_t));
	  xsrc0 = xsrc;
          ysrc0 = ysrc;
	  do {
	    xmean += (double) *xsrc.l;
            ymean += (double) *ysrc.l;
	  } while (advanceLoop(&ysrcinfo, &ysrc),
		   advanceLoop(&xsrcinfo, &xsrc) < xsrcinfo.naxes);
	  memcpy(xsrcinfo.coords, save, xsrcinfo.ndim*sizeof(int32_t));
	  memcpy(ysrcinfo.coords, save, ysrcinfo.ndim*sizeof(int32_t));
	  xsrc = xsrc0;
          ysrc = ysrc0;
	  xmean /= n;
          ymean /= n;
	  cov = 0.0;
	  do {
	    xtemp = ((double) *xsrc.l - xmean);
            ytemp = ((double) *ysrc.l - ymean);
	    cov += xtemp*ytemp;
	  } while ((done = (advanceLoop(&ysrcinfo, &ysrc),
			    advanceLoop(&xsrcinfo, &xsrc))) < xsrcinfo.naxes);
	  switch (outtype) {
	    case LUX_FLOAT:
	      *trgt.f++ = (float) (cov/n2);
	      break;
	    case LUX_DOUBLE:
	      *trgt.d++ = (cov/n2);
	      break;
	  }
	} while (done < xsrcinfo.rndim);
        break;
      case LUX_FLOAT:
	do {
	  xmean = 0.0;
          ymean = 0.0;
	  memcpy(save, xsrcinfo.coords, xsrcinfo.ndim*sizeof(int32_t));
	  xsrc0 = xsrc;
          ysrc0 = ysrc;
	  do {
	    xmean += (double) *xsrc.f;
            ymean += (double) *ysrc.f;
	  } while (advanceLoop(&ysrcinfo, &ysrc),
		   advanceLoop(&xsrcinfo, &xsrc) < xsrcinfo.naxes);
	  memcpy(xsrcinfo.coords, save, xsrcinfo.ndim*sizeof(int32_t));
	  memcpy(ysrcinfo.coords, save, ysrcinfo.ndim*sizeof(int32_t));
	  xsrc = xsrc0;
          ysrc = ysrc0;
	  xmean /= n;
          ymean /= n;
	  cov = 0.0;
	  do {
	    xtemp = ((double) *xsrc.f - xmean);
            ytemp = ((double) *ysrc.f - ymean);
	    cov += xtemp*ytemp;
	  } while ((done = (advanceLoop(&ysrcinfo, &ysrc),
			    advanceLoop(&xsrcinfo, &xsrc))) < xsrcinfo.naxes);
	  switch (outtype) {
	    case LUX_FLOAT:
	      *trgt.f++ = (float) (cov/n2);
	      break;
	    case LUX_DOUBLE:
	      *trgt.d++ = (cov/n2);
	      break;
	  }
	} while (done < xsrcinfo.rndim);
        break;
      case LUX_DOUBLE:
	do {
	  xmean = 0.0;
          ymean = 0.0;
	  memcpy(save, xsrcinfo.coords, xsrcinfo.ndim*sizeof(int32_t));
	  xsrc0 = xsrc;
          ysrc0 = ysrc;
	  do {
	    xmean += (double) *xsrc.d;
            ymean += (double) *ysrc.d;
	  } while (advanceLoop(&ysrcinfo, &ysrc),
		   advanceLoop(&xsrcinfo, &xsrc) < xsrcinfo.naxes);
	  memcpy(xsrcinfo.coords, save, xsrcinfo.ndim*sizeof(int32_t));
	  memcpy(ysrcinfo.coords, save, ysrcinfo.ndim*sizeof(int32_t));
	  xsrc = xsrc0;
          ysrc = ysrc0;
	  xmean /= n;
          ymean /= n;
	  cov = 0.0;
	  do {
	    xtemp = ((double) *xsrc.d - xmean);
            ytemp = ((double) *ysrc.d - ymean);
	    cov += xtemp*ytemp;
	  } while ((done = (advanceLoop(&ysrcinfo, &ysrc),
			    advanceLoop(&xsrcinfo, &xsrc))) < xsrcinfo.naxes);
          *trgt.d++ = (cov/n2);
	} while (done < xsrcinfo.rndim);
        break;
    }
  }
  
  switch (outtype) {
  case LUX_FLOAT:
    lastmean.f = xmean;
    break;
  case LUX_DOUBLE:
    lastmean.d = xmean;
    break;
  }
  scalar_type(lastmean_sym) = scalar_type(lastsdev_sym) = outtype;
  return result;
}
/*------------------------------------------------------------------------- */
int32_t sdev(int32_t narg, int32_t ps[], int32_t sq)
/* f(<x> [, <mode>, WEIGHTS=<weights>, /SAMPLE, /POPULATION, /KEEPDIMS, */
/*         /DOUBLE]) */
/* with f = SDEV (sq = 1) or VARIANCE (sq = 0) */
/* Returns the standard deviation or variance of <x>, which must be a */
/* numerical array.  By default, returns the sample deviation or variance */
/* of the whole array <x>. */
/* <mode>: if it is a scalar, then returns a result for each run along the */
/*    dimension indicated by <mode>.  If it is an array with the same number */
/*    of elements as <x>, then each element of <mode> identifies the integer */
/*    class that the corresponding element of <x> belongs to, and a result */
/*    is returned for each class. */
/* <weights>: the weight of each element of <x>.  This parameter must be a */
/*    numerical array with the same dimensions as <x>.  If <weights> is */
/*    specified, then /SAMPLE is ignored. */
/* /SAMPLE: return the sample standard deviation or variance. */
/* /POPULATION: return the population standard deviation or variance. */
/* /KEEPDIMS: if this keyword is specified, then the dimension(s) along */
/*    which the calculations of the deviations or variances are performed */
/*    are set to 1 in the result.  If this keyword is not specified, then */
/*    any dimensions equal to 1 in the result are omitted, and if only a */
/*    single value is returned then this is returned as a scalar. */
/* /DOUBLE: by default, the result is DOUBLE only if <x> is DOUBLE or */
/*    CDOUBLE, and the result is FLOAT otherwise.  If this keyword is set, */
/*    then the result is always DOUBLE. */
/* LS 19 July 2000 */
{
  int32_t	result, n, i, done, n2, save[MAX_DIMS], outtype, haveWeights;
  pointer	src, trgt, src0, trgt0, weight, weight0;
  double	mean, sdev, temp, nn;
  doubleComplex	cmean;
  loopInfo	srcinfo, trgtinfo, winfo;
  extern scalar	lastsdev, lastmean;
  extern int32_t	lastsdev_sym, lastmean_sym;
  
  /* return values by class? */
  if (narg > 1 && ps[1] && symbolIsNumericalArray(ps[1])
      && symbolIsNumericalArray(ps[0])
      && array_size(ps[0]) == array_size(ps[1]))
    return index_sdev(narg, ps, sq);

  if (internalMode & 4) 	/* /DOUBLE */
    outtype = LUX_DOUBLE;
  else if (isComplexType(symbol_type(ps[0])))
    outtype = (symbol_type(ps[0]) == LUX_CFLOAT)? LUX_FLOAT: LUX_DOUBLE;
  else
    outtype = (symbol_type(ps[0]) == LUX_DOUBLE)? LUX_DOUBLE: LUX_FLOAT;

  if (narg > 2 && ps[2]) {	/* have <weights> */
    if (!symbolIsNumericalArray(ps[2])	/* but it's not a numerical array */
	|| array_size(ps[2]) != array_size(ps[0])) /* or has wrong size */
      return cerror(INCMP_ARG, ps[2]);
    for (i = 0; i < array_num_dims(ps[2]) - 1; i++) /* check the dimensions */
      if (array_dims(ps[2])[i] != array_dims(ps[0])[i])
	return cerror(INCMP_DIMS, ps[2]);
    haveWeights = 1;
  } else
    haveWeights = 0;
    
  /* set up for traversing the data, and create an output symbol, too */
  if (standardLoop(ps[0], (narg > 1 && ps[1])? ps[1]: 0,
		   SL_COMPRESSALL /* omit all axis dimensions from result */
		   | (narg > 1 && ps[1]? 0: SL_ALLAXES)
		   | SL_EXACT
		   | SL_EACHCOORD /* need all coordinates */
		   | SL_UNIQUEAXES /* no duplicate axes allowed */
		   | SL_AXESBLOCK /* treat axes as a block */
		   | ((internalMode & 2)? SL_ONEDIMS: 0), /* omit -> 1 */
		   outtype, &srcinfo, &src, &result, &trgtinfo, &trgt) < 0)
    return LUX_ERROR;

  /* set up for traversing the weights, too */
  if (haveWeights) {
    /* we make sure that <weights> has the same data type as <x> -- except */
    /* that <weights> is never complex */
    haveWeights = lux_converts[realType(array_type(ps[0]))](1, &ps[2]);
    if (standardLoop(haveWeights, ps[1],
		     (ps[1]? 0: SL_ALLAXES) | SL_EACHCOORD | SL_AXESBLOCK,
		     0, &winfo, &weight, NULL, NULL, NULL) < 0)
      return LUX_ERROR;
  }
		      
  if (!srcinfo.naxes) {
    srcinfo.naxes++;
    if (haveWeights)
      winfo.naxes++;
  }

  trgt0 = trgt;

  /* Knuth (the Art of Computer Programming, Volume 2: Seminumerical
     Algorithms) gives the following algorithm for calculating the
     standard deviation with just one pass through the data and the
     least round-off error and guaranteed nonnegative variance:

     M₁ = x₁
     S₁ = 0
     M_k = M_{k-1} + (x_k - M_{k-1})/k
     S_k = S_{k-1} + (x_k - M_{k-1})(x_k - M_k)
     σ = √(S_k/(n - 1))

     TODO: Extend this algorithm to weighted data */

  /* we make two passes: one to get the average, and then one to calculate */
  /* the standard deviation; this way we limit truncation and roundoff */
  /* errors */
  if (haveWeights) {
    switch (symbol_type(ps[0])) {
      case LUX_BYTE:
	do {
	  mean = 0.0;
	  memcpy(save, srcinfo.coords, srcinfo.ndim*sizeof(int32_t));
	  src0 = src;
	  weight0 = weight;
	  nn = 0;
	  do {
	    mean += (double) *src.b * *weight.b;
	    nn += (double) *weight.b;
	  }
	  while (advanceLoop(&winfo, &weight),
		 advanceLoop(&srcinfo, &src) < srcinfo.naxes);
	  memcpy(srcinfo.coords, save, srcinfo.ndim*sizeof(int32_t));
	  src = src0;
	  weight = weight0;
	  mean /= (nn? nn: 1);
	  sdev = 0.0;
	  do {
	    temp = ((double) *src.b - mean);
	    sdev += temp*temp* *weight.b;
	  } while ((done = (advanceLoop(&winfo, &weight),
			    advanceLoop(&srcinfo, &src))) < srcinfo.naxes);
	  if (!nn)
	    nn = 1;
	  sdev /= nn;
	  switch (outtype) {
	    case LUX_FLOAT:
	      *trgt.f++ = (float) sdev;
	      break;
	    case LUX_DOUBLE:
	      *trgt.d++ = sdev;
	      break;
	  }
	} while (done < srcinfo.rndim);
	break;
      case LUX_WORD:
	do {
	  mean = 0.0;
	  memcpy(save, srcinfo.coords, srcinfo.ndim*sizeof(int32_t));
	  src0 = src;
	  weight0 = weight;
	  nn = 0;
	  do {
	    mean += (double) *src.w * *weight.w;
	    nn += (double) *weight.w;
	  }
	  while (advanceLoop(&winfo, &weight),
		 advanceLoop(&srcinfo, &src) < srcinfo.naxes);
	  memcpy(srcinfo.coords, save, srcinfo.ndim*sizeof(int32_t));
	  src = src0;
	  weight = weight0;
	  mean /= (nn? nn: 1);
	  sdev = 0.0;
	  do {
	    temp = ((double) *src.w - mean);
	    sdev += temp*temp* *weight.w;
	  } while ((done = (advanceLoop(&winfo, &weight),
			    advanceLoop(&srcinfo, &src))) < srcinfo.naxes);
	  if (!nn)
	    nn = 1;
	  sdev /= nn;
	  switch (outtype) {
	    case LUX_FLOAT:
	      *trgt.f++ = (float) sdev;
	      break;
	    case LUX_DOUBLE:
	      *trgt.d++ = sdev;
	      break;
	  }
	} while (done < srcinfo.rndim);
	break;
      case LUX_LONG:
	do {
	  mean = 0.0;
	  memcpy(save, srcinfo.coords, srcinfo.ndim*sizeof(int32_t));
	  src0 = src;
	  weight0 = weight;
	  nn = 0;
	  do {
	    mean += (double) *src.l * *weight.l;
	    nn += (double) *weight.l;
	  }
	  while (advanceLoop(&winfo, &weight),
		 advanceLoop(&srcinfo, &src) < srcinfo.naxes);
	  memcpy(srcinfo.coords, save, srcinfo.ndim*sizeof(int32_t));
	  src = src0;
	  weight = weight0;
	  mean /= (nn? nn: 1);
	  sdev = 0.0;
	  do {
	    temp = ((double) *src.l - mean);
	    sdev += temp*temp* *weight.l;
	  } while ((done = (advanceLoop(&winfo, &weight),
			    advanceLoop(&srcinfo, &src))) < srcinfo.naxes);
	  if (!nn)
	    nn = 1;
	  sdev /= nn;
	  switch (outtype) {
	    case LUX_FLOAT:
	      *trgt.f++ = (float) sdev;
	      break;
	    case LUX_DOUBLE:
	      *trgt.d++ = sdev;
	      break;
	  }
	} while (done < srcinfo.rndim);
	break;
      case LUX_FLOAT:
	do {
	  mean = 0.0;
	  memcpy(save, srcinfo.coords, srcinfo.ndim*sizeof(int32_t));
	  src0 = src;
	  weight0 = weight;
	  nn = 0;
	  do {
	    mean += (double) *src.f * *weight.f;
	    nn += (double) *weight.f;
	  }
	  while (advanceLoop(&winfo, &weight),
		 advanceLoop(&srcinfo, &src) < srcinfo.naxes);
	  memcpy(srcinfo.coords, save, srcinfo.ndim*sizeof(int32_t));
	  src = src0;
	  weight = weight0;
	  mean /= (nn? nn: 1);
	  sdev = 0.0;
	  do {
	    temp = ((double) *src.f - mean);
	    sdev += temp*temp* *weight.f;
	  } while ((done = (advanceLoop(&winfo, &weight),
			    advanceLoop(&srcinfo, &src))) < srcinfo.naxes);
	  if (!nn)
	    nn = 1;
	  sdev /= nn;
	  switch (outtype) {
	    case LUX_FLOAT:
	      *trgt.f++ = (float) sdev;
	      break;
	    case LUX_DOUBLE:
	      *trgt.d++ = sdev;
	      break;
	  }
	} while (done < srcinfo.rndim);
	break;
      case LUX_DOUBLE:
	do {
	  mean = 0.0;
	  memcpy(save, srcinfo.coords, srcinfo.ndim*sizeof(int32_t));
	  src0 = src;
	  weight0 = weight;
	  nn = 0;
	  do {
	    mean += (double) *src.d * *weight.d;
	    nn += (double) *weight.d;
	  }
	  while (advanceLoop(&winfo, &weight),
		 advanceLoop(&srcinfo, &src) < srcinfo.naxes);
	  memcpy(srcinfo.coords, save, srcinfo.ndim*sizeof(int32_t));
	  src = src0;
	  weight = weight0;
	  mean /= (nn? nn: 1);
	  sdev = 0.0;
	  do {
	    temp = ((double) *src.d - mean);
	    sdev += temp*temp* *weight.d;
	  } while ((done = (advanceLoop(&winfo, &weight),
			    advanceLoop(&srcinfo, &src))) < srcinfo.naxes);
	  if (!nn)
	    nn = 1;
	  sdev /= nn;
	  switch (outtype) {
	    case LUX_FLOAT:
	      *trgt.f++ = (float) sdev;
	      break;
	    case LUX_DOUBLE:
	      *trgt.d++ = sdev;
	      break;
	  }
	} while (done < srcinfo.rndim);
	break;
    }
    zapTemp(haveWeights);	/* delete if it is a temp */
  } else {
    n = 1;			/* initialize number of values per sdev */
    for (i = 0; i < srcinfo.naxes; i++)
      n *= srcinfo.rdims[i];
    n2 = (internalMode & 1)? n: n - 1; /* sample or population sdev */
    if (!n2)
      return luxerror("Single values have no sample standard deviation", ps[0]);

    switch (symbol_type(ps[0])) {
      case LUX_BYTE:
	do {
	  mean = 0.0;
	  memcpy(save, srcinfo.coords, srcinfo.ndim*sizeof(int32_t));
	  src0 = src;
	  do
	    mean += (double) *src.b;
	  while (advanceLoop(&srcinfo, &src) < srcinfo.naxes);
	  memcpy(srcinfo.coords, save, srcinfo.ndim*sizeof(int32_t));
	  src = src0;
	  mean /= n;
	  sdev = 0.0;
	  do {
	    temp = ((double) *src.b - mean);
	    sdev += temp*temp;
	  } while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
	  switch (outtype) {
	    case LUX_FLOAT:
	      *trgt.f++ = (float) (sdev/n2);
	      break;
	    case LUX_DOUBLE:
	      *trgt.d++ = (sdev/n2);
	      break;
	  }
	} while (done < srcinfo.rndim);
	break;
      case LUX_WORD:
	do {
	  mean = 0.0;
	  memcpy(save, srcinfo.coords, srcinfo.ndim*sizeof(int32_t));
	  src0 = src;
	  do
	    mean += (double) *src.w;
	  while (advanceLoop(&srcinfo, &src) < srcinfo.naxes);
	  memcpy(srcinfo.coords, save, srcinfo.ndim*sizeof(int32_t));
	  src = src0;
	  mean /= n;
	  sdev = 0.0;
	  do {
	    temp = ((double) *src.w - mean);
	    sdev += temp*temp;
	  } while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
	  switch (outtype) {
	    case LUX_FLOAT:
	      *trgt.f++ = (float) (sdev/n2);
	      break;
	    case LUX_DOUBLE:
	      *trgt.d++ = (sdev/n2);
	      break;
	  }
	} while (done < srcinfo.rndim);
	break;
      case LUX_LONG:
	do {
	  mean = 0.0;
	  memcpy(save, srcinfo.coords, srcinfo.ndim*sizeof(int32_t));
	  src0 = src;
	  do
	    mean += (double) *src.l;
	  while (advanceLoop(&srcinfo, &src) < srcinfo.naxes);
	  memcpy(srcinfo.coords, save, srcinfo.ndim*sizeof(int32_t));
	  src = src0;
	  mean /= n;
	  sdev = 0.0;
	  do {
	    temp = ((double) *src.l - mean);
	    sdev += temp*temp;
	  } while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
	  switch (outtype) {
	    case LUX_FLOAT:
	      *trgt.f++ = (float) (sdev/n2);
	      break;
	    case LUX_DOUBLE:
	      *trgt.d++ = (sdev/n2);
	      break;
	  }
	} while (done < srcinfo.rndim);
	break;
      case LUX_FLOAT:
	do {
	  mean = 0.0;
	  memcpy(save, srcinfo.coords, srcinfo.ndim*sizeof(int32_t));
	  src0 = src;
	  do
	    mean += (double) *src.f;
	  while (advanceLoop(&srcinfo, &src) < srcinfo.naxes);
	  memcpy(srcinfo.coords, save, srcinfo.ndim*sizeof(int32_t));
	  src = src0;
	  mean /= n;
	  sdev = 0.0;
	  do {
	    temp = ((double) *src.f - mean);
	    sdev += temp*temp;
	  } while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
	  switch (outtype) {
	    case LUX_FLOAT:
	      *trgt.f++ = (float) (sdev/n2);
	      break;
	    case LUX_DOUBLE:
	      *trgt.d++ = (sdev/n2);
	      break;
	  }
	} while (done < srcinfo.rndim);
	break;
      case LUX_DOUBLE:
	do {
	  mean = 0.0;
	  memcpy(save, srcinfo.coords, srcinfo.ndim*sizeof(int32_t));
	  src0 = src;
	  do
	    mean += (double) *src.d;
	  while (advanceLoop(&srcinfo, &src) < srcinfo.naxes);
	  memcpy(srcinfo.coords, save, srcinfo.ndim*sizeof(int32_t));
	  src = src0;
	  mean /= n;
	  sdev = 0.0;
	  do {
	    temp = ((double) *src.d - mean);
	    sdev += temp*temp;
	  } while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
	  *trgt.d++ = sdev/n2;
	} while (done < srcinfo.rndim);
	break;
      case LUX_CFLOAT:
	do {
	  cmean.real = cmean.imaginary = 0.0;
	  memcpy(save, srcinfo.coords, srcinfo.ndim*sizeof(int32_t));
	  src0 = src;
	  do {
	    cmean.real += (double) src.cf->real;
	    cmean.imaginary += (double) src.cf->imaginary;
	  } while (advanceLoop(&srcinfo, &src) < srcinfo.naxes);
	  memcpy(srcinfo.coords, save, srcinfo.ndim*sizeof(int32_t));
	  src = src0;
	  cmean.real /= n;
	  cmean.imaginary /= n;
	  sdev = 0.0;
	  do {
	    temp = src.cf->real - cmean.real;
	    sdev += temp*temp;
	    temp = src.cf->imaginary - cmean.imaginary;
	    sdev += temp*temp;
	  } while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
	  switch (outtype) {
	    case LUX_FLOAT:
	      *trgt.f++ = (float) (sdev/n2);
	      break;
	    case LUX_DOUBLE:
	      *trgt.d++ = (sdev/n2);
	      break;
	  }
	} while (done < srcinfo.rndim);
	break;
      case LUX_CDOUBLE:
	do {
	  cmean.real = cmean.imaginary = 0.0;
	  memcpy(save, srcinfo.coords, srcinfo.ndim*sizeof(int32_t));
	  src0 = src;
	  do {
	    cmean.real += (double) src.cd->real;
	    cmean.imaginary += (double) src.cd->imaginary;
	  } while (advanceLoop(&srcinfo, &src) < srcinfo.naxes);
	  memcpy(srcinfo.coords, save, srcinfo.ndim*sizeof(int32_t));
	  src = src0;
	  cmean.real /= n;
	  cmean.imaginary /= n;
	  sdev = 0.0;
	  do {
	    temp = src.cd->real - cmean.real;
	    sdev += temp*temp;
	    temp = src.cd->imaginary - cmean.imaginary;
	    sdev += temp*temp;
	  } while ((done = advanceLoop(&srcinfo, &src)) < srcinfo.naxes);
	  *trgt.d++ = sdev/n2;
	} while (done < srcinfo.rndim);
	break;
    }
  }
  
  if (sq) { 			/* standard deviation rather than variance */
    switch (outtype) {
      case LUX_FLOAT:
	n = trgt.f - trgt0.f;
	while (n--) {
	  *trgt0.f = sqrt(*trgt0.f);
	  trgt0.f++;
	}
	switch (symbol_type(ps[0])) {
	  case LUX_CFLOAT: case LUX_CDOUBLE:
	    lastmean.f = cmean.real;
	    break;
	  default:
	    lastmean.f = mean;
	    break;
	}
	lastsdev.f = trgt0.f[-1];
	break;
      case LUX_DOUBLE:
	n = trgt.d - trgt0.d;
	while (n--) {
	  *trgt0.d = sqrt(*trgt0.d);
	  trgt0.d++;
	}
	switch (symbol_type(ps[0])) {
	  case LUX_CFLOAT: case LUX_CDOUBLE:
	    lastmean.d = cmean.real;
	    break;
	  default:
	    lastmean.d = mean;
	    break;
	}
	lastsdev.d = trgt0.d[-1];
	break;
    } 
  } else {			/* wanted variance */
    switch (outtype) {
      case LUX_FLOAT:
	lastsdev.f = sqrt(trgt.f[-1]);
	switch (symbol_type(ps[0])) {
	  case LUX_CFLOAT: case LUX_CDOUBLE:
	    lastmean.f = cmean.real;
	    break;
	  default:
	    lastmean.f = mean;
	    break;
	}
	break;
      case LUX_DOUBLE:
	lastsdev.d = sqrt(trgt.d[-1]);
	switch (symbol_type(ps[0])) {
	  case LUX_CFLOAT: case LUX_CDOUBLE:
	    lastmean.d = cmean.real;
	    break;
	  default:
	    lastmean.d = mean;
	    break;
	}
	break;
    }
  }
  scalar_type(lastmean_sym) = scalar_type(lastsdev_sym) = outtype;
  return result;
}
/*------------------------------------------------------------------------- */
int32_t lux_sdev(int32_t narg, int32_t ps[])
     /* return standard deviation */
{
  return sdev(narg, ps, 1);
}
/*------------------------------------------------------------------------- */
int32_t lux_variance(int32_t narg, int32_t ps[])
     /* returns variance */
{
  return sdev(narg, ps, 0);
}
/*------------------------------------------------------------------------- */
/*
Algorithm for calculating the standard deviation with very little
roundoff error, with one pass through the data, and with guaranteed
non-negative variance.

Knuth, The Art of Computer Programming, Volume 2: Seminumerical
Algorithms, gives the following algorithm for unweighted data:

M₁ = x₁
S₁ = 0
M_k = M_{k-1} + (x_k - M_{k-1})/k
S_k = S_{k-1} + (x_k - M_{k-1})*(x_k - M_k)
s = √(S_k/(k - 1))

Example: x₁ = 2, x₂ = 1, x₃ = 1
M₁ = 2, S₁ = 0
M₂ = 2 + (1 - 2)/2 = 3/2
S₂ = 0 + (1 - 2)*(1 - 3/2) = 1/2
M₃ = 3/2 + (1 - 3/2)/3 = 4/3
S₃ = 1/2 + (1 - 3/2)*(1 - 4/3) = 2/3
s = √(2/3/(3 - 1)) = ⅓√3

I find the following extension to weighted data:

M₁ = x₁
W₁ = w₁
S₁ = 0
W_k = W_{k-1} + w_k
M_k = M_{k-1} + (x_k - M_{k-1})/W_k
S_k = S_{k-1} + (x_k - M_{k-1})*(x_k - M_k)*w_k
s = √(S_k/(W_k - 1))

Example: x₁ = 2, x₂ = 1, x₃ = 1, w₁ = w₂ = w₃ = 1
M₁ = 2, S₁ = 0, W₁ = 1, X₁ = 1
W₂ = 1 + 1 = 2
M₂ = 2 + (1 - 2)/2 = 3/2
S₂ = 0 + (1 - 2)*(1 - 3/2)*1 = 1/2
W₃ = 2 + 1 = 3
M₃ = 3/2 + (1 - 3/2)/3 = 4/3
S₃ = 1/2 + (1 - 3/2)*(1 - 4/3)*1 = 2/3
s = √(2/3/(3 - 1)) = √(1/3) = ⅓√3

Example: x₁ = 2, x₂ = 1, w₁ = 1, w₂ = 2
M₁ = 2, S₁ = 0, W₁ = 1
W₂ = 1 + 2 = 3
M₂ = 2 + (1 - 2)/3 = 4/3
S₂ = 0 + (1 - 2)*(1 - 4/3)*2 = 2/3
s = √(2/3/(3 - 1)) = √(1/3) = ⅓√3
 */
/*------------------------------------------------------------------------- */
int32_t lux_swab(int32_t narg, int32_t ps[])
/*swap bytes in an array or even an scalar */
/*this is a subroutine and swaps the input symbol (not a copy) */
{
  int32_t	iq, n, nd, j;
  pointer q1;
  array	*h;
  int32_t	swapb(char *, int32_t);

  while (narg--)
  { iq = *ps++;						/*just 1 argument */
    /*check if legal to change this symbol */
    if ( iq <= nFixed ) return cerror(CST_LHS, iq);
    type = sym[iq].type;
    switch (sym[iq].class)	{
    case 1:						/*scalar case */
	n = lux_type_size[type];
	q1.l = &sym[iq].spec.scalar.l;
	break;
      case LUX_SCAL_PTR:
	n = lux_type_size[type];
	q1 = scal_ptr_pointer(iq);
	break;
      case 4:						/*array case */
	n = lux_type_size[type];
	h = (array *) sym[iq].spec.array.ptr;
	q1.l = (int32_t *) ((char *)h + sizeof(array));
	nd = h->ndim;
	for (j=0;j<nd;j++) n *= h->dims[j]; 		/*# of elements */
	break;
      default:
	return cerror(ILL_CLASS, iq);
      }
    n = n - n%2;					/*force even */
    swapb( (char *) q1.l, n); }
  return 1;
}
/*------------------------------------------------------------------------- */
int32_t lux_wait(int32_t narg, int32_t ps[])
/* wait the specified # of seconds (floating point) */
{
  float	xq;
  struct timeval	tval;

  xq = float_arg(ps[0]);
  xq = MAX(0.0, xq);
  tval.tv_sec = (uint32_t) xq;
  xq = (xq - tval.tv_sec) * 1.e6;
  tval.tv_usec = (uint32_t) xq;
  select(0, NULL, NULL, NULL,&tval);
  return LUX_OK;
}
/*---------------------------------------------------------*/
int32_t lux_esmooth(int32_t narg, int32_t ps[])
  /* Exponential Asymmetric Smoothing */
  /* Syntax: Y = ESMOOTH( X [, AXIS, ORDER] ) */
{
  int32_t	iq, axis, result_sym, outtype, type, m, ndim, *dims, xdims[8],
  	i, tally[8], n, step[8], done;
  float	damping, width;
  array	*h;
  pointer	src, trgt;
  scalar	sum, weight;
  extern float	float_arg(int32_t);

  if (narg > 2) { iq = ps[2];  axis = int_arg(ps[1]); }
  else { iq = ps[1];  axis = -1; }
  if (narg > 1) width = float_arg(iq); else width = 3;
  if (width == 0) width = 1e-10;
  iq = ps[0];
  if (iq < 0) return iq;	/* some previous error */
  if (sym[iq].class == LUX_SCAL_PTR) iq = dereferenceScalPointer(iq);
  switch (sym[iq].class)
  { case LUX_SCALAR:		/* just return value */
      if (isFreeTemp(iq)) result_sym = iq;
      else result_sym = scalar_scratch_copy(iq);
      break;
    case LUX_ARRAY:
      outtype = type = sym[iq].type;
      if (type < LUX_FLOAT) { outtype = LUX_FLOAT;  iq = lux_float(1, &iq); }
      h = HEAD(iq);
      src.l = LPTR(h);
      GET_SIZE(m, h->dims, h->ndim);
      if (axis >= 0)
      { dims = h->dims;  ndim = h->ndim;
	if (axis >= ndim) return cerror(ILL_DIM, ps[1]); }
      else
      { dims = &m;  ndim = 1;  axis = 0; }
      memcpy(xdims, dims, sizeof(int32_t)*ndim); /* copy dimensions */
      result_sym = array_clone(iq, outtype);
      h = HEAD(result_sym);
      trgt.l = LPTR(h);
				/* set up for walk through array */
      for (i = 0; i < ndim; i++) tally[i] = 1;
      n = *step = lux_type_size[outtype];
      for (i = 1; i < ndim; i++) step[i] = (n *= dims[i - 1]);
      if (axis)
      { n = *step; *step = step[axis]; step[axis] = n;
	n = *xdims; *xdims = xdims[axis]; xdims[axis] = n; }
      for (i = ndim - 1; i; i--) step[i] -= step[i - 1]*xdims[i - 1];
      damping = exp(-1./width);
      sum.d = weight.d = 0.0;
      do			/* main loop */
      { switch (outtype)
	{ case LUX_FLOAT:
	    sum.f = sum.f*damping + *src.f; 
	    weight.f = weight.f*damping + 1.0;
	    *trgt.f = sum.f/weight.f;  break;
	  case LUX_DOUBLE:
	    sum.d = sum.d*damping + *src.d;
	    weight.d = weight.d*damping + 1.0;
	    *trgt.d = sum.d/weight.d;  break; }
	src.b += *step;  trgt.b += *step;
	for (i = 0; i < ndim; i++)
	{ if (tally[i]++ != xdims[i])
	  { done = 0;
	    if (i == 1) sum.d = weight.d = 0.0;
	    break; }
	  tally[i] = 1;  done = 1;
	  src.b += step[i + 1];
	  trgt.b += step[i + 1]; }
      } while (!done);
      break;
    default:
      return cerror(ILL_TYPE, iq);
    }
  return result_sym;
}
/*------------------------------------------------------------------------- */
void esmooth_asymmetric(double *srcdata, size_t srccount, size_t srcstride,
			double width,
			double *tgtdata, size_t tgtcount, size_t tgtstride)
{
  double damping = exp(-1.0/width);
  double sum = 0.0;
  double weight = 0.0;
  int32_t i;

  assert(tgtcount >= srccount);

  /*
    a = exp(-1/w)
    y[i]   = x[i] + x[i-1]*a + x[i-2]*a² + …
    y[i+1] = x[i+1] + x[i]*a + x[i-1]*a² + …
           = x[i+1] + y[i]*a
  */

  for (i = 0; i < srccount; i++) {
    sum = sum*damping + *srcdata;
    weight = weight*damping + 1;
    *tgtdata = sum/weight;
    srcdata += srcstride;
    tgtdata += tgtstride;
  }
}
/* i>D*;i?D1;rD& */
BIND(esmooth_asymmetric, v_sddsd_iDaD1rDq_012, f, ESMOOTH1, 1, 2, NULL);
/*------------------------------------------------------------------------- */
void esmooth_symmetric(double *srcdata, size_t srccount, size_t srcstride,
		       double width,
		       double *tgtdata, size_t tgtcount, size_t tgtstride)
{
  double damping = exp(-1.0/width);
  double sum = 0.0;
  double weight = 0.0;
  int32_t i;

  assert(tgtcount >= srccount);

  /*
    a = exp(-1/w)
    z[i]   = … + x[i-2]*a² + x[i-1]*a + x[i] + x[i+1]*a + x[i+2]*a² + …
    z[i+1] = … + x[i-1]*a² + x[i]*a + x[i+1] + x[i+2]*a + x[i+3]*a² + …
           = … + x[i-2]*a³ + x[i-1]*a² + x[i]*a + x[i+1] + x[i+2]*a + …

    z₊[i] = x[i+1]*a + x[i+2]*a² + …
    z₋[i] = … + x[i-2]*a² + x[i-1]*a
    z[i] = z₋[i] + x[i] + z₊[i]
    z[i+1] = z₋[i+1] + x[i+1] + z₊[i+1]

    z₋[i+1] = … + x[i-1]*a² + x[i]*a
            = z‐[i]*a + x[i]*a
    z₊[i+1] = x[i+2]*a + x[i+3]*a² + …
            = z₊[i]/a - x[i+1]

	    TODO: adjust below
  */

  for (i = 0; i < srccount; i++) {
    sum = sum*damping + *srcdata;
    weight = weight*damping + 1;
    *tgtdata = sum/weight;
    srcdata += srcstride;
    tgtdata += tgtstride;
  }
  sum = 0;
  weight = 0;
  for (i = 0; i < srccount; i++) {
    srcdata -= srcstride;
    tgtdata -= tgtstride;
    sum = sum*damping + *srcdata;
    weight = weight*damping + 1;
    *tgtdata += (sum - *srcdata)/weight;
  }
}
/* i>D*;i?D1;rD& */
BIND(esmooth_symmetric, v_sddsd_iDaD1rDq_012, f, ESMOOTH2, 1, 2, NULL);
/*------------------------------------------------------------------------- */
void vargsmoothkernel(double width, int32_t nx, int32_t *n2, int32_t *ng, double **gkern,
		      double *gsum, double **partial)
/* makes sure a gaussian kernel of FWHM <width> and kernel size at
 most <nx> is returned in <*gkern>.  The kernel size is returned in
 <*ng>, which is equal to 2*<*n2> + 1.  The sum of the kernel values
 is returned in <*gsum>.  A list of normalization factors for partial
 sums is returned at <*partial>.  When first called, <*gkern> must be
 NULL to indicate that no kernel space is available yet.  On each next
 call you must specify the same values for <*gkern>, <*n2>, <*ng>, and
 <*gsum> as were returned the previous time, unless the kernel space
 was properly disposed of earlier.  Kernel space disposal is done by
 specifying an <nx> of zero.  Then the current kernel space is
 released.  LS 25apr97 */
{
  int32_t	n, nm;
  double	w, *pt3, *pt2;
  double	xq, wq, dq;
  static double	ow = 0.0;
  static int32_t	maxng = 0;

  if (!nx) {			/* release kernel space */
    Free(*gkern);
    ow = 0.0;
    maxng = 0;
    return; }

  w = 0.6005612*width;
  if (w != ow) {		/* need new kernel */
    ow = w;			/* remember current width */
    /* calculate smoothing kernel size */
    *n2 = 4*w;
    if (*n2 < 1)
      *n2 = 1;			/* must not be smaller than one */
    else if (*n2 > nx)
      *n2 = nx;			/* must not be bigger than axis dimension */
    *ng = 2* *n2 + 1;		/* smoothing kernel size */
    nm = 3* *n2 + 2;		/* gkern plus partial */
    /* now need to find memory for smoothing kernel */
    if (nm > maxng) {	/* bigger than available memory */
				/* so get more */
      *gkern = (double *) (*gkern? Realloc(*gkern, nm*sizeof(double)):
			   Malloc(nm*sizeof(double)));
      maxng = nm; }		/* remember current size */
    *partial = *gkern + *ng;	/* partial sums */
    /* now get ready to calculate kernel values */
    n = *ng;			/* number of elements */
    dq = 1.0/w;			/* step size */
    wq = -*n2*dq;		/* start value for gaussian */
    pt3 = *gkern;		/* start of kernel */
    pt2 = *partial;		/* start of list of partial sums */
    *gsum = 0.0;		/* initial sum of kernel values */
    while (n--) {
      xq = exp(-wq*wq);		/* gaussian value */
      *pt3++ = xq;		/* put in kernel */
      *gsum += xq;		/* update kernel sum */
      if (n <= *n2)
	*pt2++ = *gsum;		/* partial sum */
      wq += dq; }		/* next ordinate */
    pt2 = *partial;
    for (n = nx/2; n <= *n2; n++) /* correct for limited size */
      pt2[n] = pt2[nx - 1 - n] = pt2[n]	+ (pt2[nx - 1 - n] - *gsum);
    for (n = 0; n <= *n2; n++) { /* partial sum - normalization factors */
      *pt2 = *gsum/ *pt2;
      pt2++; }
  }
  /* all done */
}
/*------------------------------------------------------------------------- */
int32_t lux_gsmooth(int32_t narg, int32_t ps[])
 /* smooth input array with a gaussian*/
 /* ps[0] is input array and ps[1] is fwhm width */
 /* Addition LS 13aug94:  user may supply kernel
      GSMOOTH(x, kernel)
    kernel must have odd size;  even-sized kernels are truncated
    the center of the kernel is the central element */
 /* Addition LS 6mar95:  user may select dimension to smooth
      GSMOOTH(x [[,axis],width])
      GSMOOTH(x [,axis] [,KERNEL=kernel]) */
/* <kernel> specified -> user-specified kernel */
/* keyword /ALL selects all axes, if no <axis> is specified */
/* treatment near the edges is governed by keywords /BALANCED and /FULLNORM: */
/* keyword /FULLNORM always normalized with the full sum of weights; it */
/* assumes that there is all-zero data beyond the data edges and counts */
/* those. */
/* keyword /BALANCED reduces the width of the included data so that the */
/* weighted and included data is always balanced around the central point: */
/* this way a smoothed version of a straight line is also a straight line. */
/* LS 13aug97 */
{
  extern	int32_t scrat[NSCRAT];
  float	sum, *pt3, wq, sumg, xq;
  int32_t	n, nWidth;
  pointer	src, trgt, widths, gkern, pt1;
  int32_t	j, n2, ng, i2, i, ik, id, k;
  int32_t	iq, nx, mem = 0, dgkern;
  float	width, gsum;
  char	haveKernel = 0;
  int32_t	stride, result, loop, mode = 0;
  loopInfo	srcinfo, trgtinfo;

  iq = 0;
  if (narg >= 3 && ps[2])	/* have <axes> */
    iq = ps[1];			/* <axes> */
  else if (internalMode & 8)	/* /ALL */
    mode = SL_ALLAXES;		/* all axes specified implicitly */
  else
    mode = SL_TAKEONED;		/* no axes -> treat as 1D */

  if (standardLoop(ps[0], iq, mode | SL_SAMEDIMS | SL_EXACT | SL_EACHROW,
		   LUX_FLOAT, &srcinfo, &src, &result, &trgtinfo, &trgt) < 0)
    return LUX_ERROR;

  nx = srcinfo.rdims[0];	/* number of data points in selected
				   dimension */

  if (narg >= 4) {		/* have <kernel> */
    /* check if it is numerical and ensure that it is LUX_FLOAT */
    if (getNumerical(ps[3], LUX_FLOAT, &ng, &gkern, GN_UPDATE | GN_EXACT,
		     NULL, NULL) < 0)
      return LUX_ERROR;
    if (ng < 3)
      return luxerror("GSMOOTH - kernel must have at least 3 elements",
		   ps[narg - 1]);
    if (ng % 2 == 0) {	/* even kernel size */
      puts("GSMOOTH - truncated kernel to odd size");
      ng--;
    }
    n2 = (ng - 1)/2;
    haveKernel = 1;
    iq = ps[2]? ps[2]: ps[1];	/* <width> */
  } else
    iq = (narg == 1)? 0: ps[narg - 1]; /* <width> */

  if (iq) {			/* have <width> */
    /* check that it is numerical and ensure that it is LUX_FLOAT */
    if (getNumerical(iq, LUX_FLOAT, &nWidth, &widths,
		     GN_UPDATE | GN_EXACT, NULL, NULL) < 0)
      return LUX_ERROR;
    for (i = 0; i < nWidth; i++)
      if (widths.f[i] < 0)
	return luxerror("Need non-negative width(s)", iq);
  } else {
    nWidth = 0;
    width = 1;
  }
  
  if (!srcinfo.naxes) {
    if (nWidth > 1)
      return luxerror("No axis -> 1D, but more than one width??", ps[1]);
    srcinfo.naxes++;
  }
  
  iq = ps[0];			/* <data> */
  for (loop = 0; loop < srcinfo.naxes; loop++) {
    nx = srcinfo.rdims[0];
    stride = srcinfo.step[0];
    if (!haveKernel) {		/* construct gaussian kernel */
      width = nWidth? 0.6005612 * *widths.f: 1.8; /*  fwhm to gaussian*/
      if (nWidth > 1)
	widths.f++;
      
      if (width >= 0.25) {
	/* set up the gaussian convolution kernel */
	n2 = MIN( 4*width, nx - 1);/* maximum half-length */
	/*	n2 = MAX( n2, 1);*/	/* minimum half-length */
	ng = 2 * n2 + 1;	/* total length */
	if (mem) {
	  if (ng > mem)
	    gkern.f = (float *) realloc(gkern.f, (mem = ng)*sizeof(float));
	} else if (ng*sizeof(float) <= NSCRAT)
	  gkern.f = (float *) scrat;
	else {
	  mem = ng;
	  gkern.f = (float *) malloc(ng*sizeof(float));
	}
	n = ng;			/* number of kernel points to do */
	wq = (float) (-n2);	/* first x */
	pt3 = gkern.f;		/* target */
	gsum = 0.0;		/* initialize sum */
	while (n--) {
	  sum = wq/width;	/* reduced x */
	  wq++;			/* increase x */
	  xq = exp(-sum*sum);	/* kernel value */
	  *pt3++ = xq;		/* store */
	  gsum += xq;		/* sum */
	}
      }
    } else {			/* have a kernel already */
      if (internalMode & 1) {	/* /NORMALIZE: normalization desired */
	if ((internalMode & 2)	/* /FULLNORM */
	    && ng > 2*nx + 1) {
	  n = 2*nx + 1;
	  pt3 = gkern.f + ng/2 - nx;
	} else {
	  n = ng;
	  pt3 = gkern.f;
	}
	gsum = 0.0;
	while (n--)
	  gsum += *pt3++;
      } else
	gsum = 1.0;
    }
    if (ng > 2*nx + 1) {	/* kernel bigger than data */
      dgkern = ng/2 - nx;
      gkern.f += dgkern;
      ng = 2*nx + 1;
    } else
      dgkern = 0;

    /* check for trivial cases, just return original */
    if (width < 0.25) {
      n = result;
      result = iq;		/* return original */
      iq = n;
    } else switch (symbol_type(iq)) {
      /*
	We divide the calculations into three zones, based on the
	coverage of the kernel elements by the data elements.  In zone
	1, data element 0 is the first one included, and not all
	kernel elements have a corresponding data element.  In zone 2,
	all kernel elements have a corresponding data element.  In
	zone 3, data element 0 is not the first one included, and not
	all kernel elements have a corresponding data element.

	Below, various examples are shown.  In each example, the
	values of nx (number of data elements) and ng (number of
	kernel elements -- always odd, and ng = 2*n2 + 1 <= 2*nx + 1)
	is shown at the top, then a line of kernel coordinates
	(relative to the center), and then lines where the data
	element indices are shown that correspond to the kernel
	element in the same column.  At the end of each line, the zone
	that the calculation belongs to is shown after a colon (:).
	Above the row of minus signs (-) the unbalanced case is shown,
	below that row the balanced case is shown.  It appears that
	all that matters is (1) whether nx is less than, or not less
	than ng; and (2) whether nx is even or odd.  For all possible
	combinations of these an example is shown.

    nx = 3 < ng = 5    nx = 4 < ng = 5   nx = 5 = ng       nx = 6 > ng = 5
    -2 -1  0 +1 +2     -2 -1  0 +1 +2    -2 -1  0 +1 +2    -2 -1  0 +1 +2
           0  1  2: 1         0  1  2: 1        0  1  2: 1        0  1  2: 1
        0  1  2   : 1      0  1  2  3: 1     0  1  2  3: 1     0  1  2  3: 1
     0  1  2      : 1   0  1  2  3   : 1  0  1  2  3  4: 2  0  1  2  3  4: 2
    -----------------   1  2  3      : 3  1  2  3  4   : 3  1  2  3  4  5: 2
           0      : 1  -----------------  2  3  4      : 3  2  3  4  5   : 3
        0  1  2   : 1         0      : 1 -----------------  3  4  5      : 3
           2      : 3      0  1  2   : 1        0      : 1 -----------------
                           1  2  3   : 3     0  1  2   : 1        0      : 1
                              3      : 3  0  1  2  3  4: 2     0  1  2   : 1
                                             2  3  4   : 3  0  1  2  3  4: 2
                                                4      : 3  1  2  3  4  5: 2
                                                               3  4  5   : 3
                                                                  5      : 3

    nx = 4 < ng = 7          nx = 5 < ng = 7          nx = 6 < ng = 7
    -3 -2 -1  0 +1 +2 +3     -3 -2 -1  0 +1 +2 +3     -3 -2 -1  0 +1 +2 +3
              0  1  2  3: 1            0  1  2  3: 1            0  1  2  3: 1
           0  1  2  3   : 1         0  1  2  3  4: 1         0  1  2  3  4: 1
        0  1  2  3      : 1      0  1  2  3  4   : 1      0  1  2  3  4  5: 1
     0  1  2  3         : 1   0  1  2  3  4      : 1   0  1  2  3  4  5   : 1
    -----------------------   1  2  3  4         : 3   1  2  3  4  5      : 3
              0         : 1  -----------------------   2  3  4  5         : 3
           0  1  2      : 1            0         : 1  -----------------------
           1  2  3      : 3         0  1  2      : 1            0         : 1
              3         : 3      0  1  2  3  4   : 1         0  1  2      : 1
                                    2  3  4      : 3      0  1  2  3  4   : 1
                                       4         : 3      1  2  3  4  5   : 3
                                                             3  4  5      : 3
                                                                5         : 3

    nx = 7 = ng              nx = 8 > ng = 7          nx = 9 > ng = 7
    -3 -2 -1  0 +1 +2 +3     -3 -2 -1  0 +1 +2 +3     -3 -2 -1  0 +1 +2 +3
              0  1  2  3: 1            0  1  2  3: 1            0  1  2  3: 1
           0  1  2  3  4: 1         0  1  2  3  4: 1         0  1  2  3  4: 1
        0  1  2  3  4  5: 1      0  1  2  3  4  5: 1      0  1  2  3  4  5: 1
     0  1  2  3  4  5  6: 2   0  1  2  3  4  5  6: 2   0  1  2  3  4  5  6: 2
     1  2  3  4  5  6   : 3   1  2  3  4  5  6  7: 2   1  2  3  4  5  6  7: 2
     2  3  4  5  6      : 3   2  3  4  5  6  7   : 3   2  3  4  5  6  7  8: 2
     3  4  5  6         : 3   3  4  5  6  7      : 3   3  4  5  6  7  8   : 3
    -----------------------   4  5  6  7         : 3   4  5  6  7  8      : 3
              0         : 1  -----------------------   5  6  7  8         : 3
           0  1  2      : 1            0         : 1  -----------------------
        0  1  2  3  4   : 1         0  1  2      : 1            0         : 1
     0  1  2  3  4  5  6: 2      0  1  2  3  4   : 1         0  1  2      : 1
        2  3  4  5  6   : 3   0  1  2  3  4  5  6: 2      0  1  2  3  4   : 1
           4  5  6      : 3   1  2  3  4  5  6  7: 2   0  1  2  3  4  5  6: 2
              6         : 3      3  4  5  6  7   : 3   1  2  3  4  5  6  7: 2
                                    5  6  7      : 3   2  3  4  5  6  7  8: 2
                                       7         : 3      4  5  6  7  8   : 3
                                                             6  7  8      : 3
                                                                8         : 3
	For unbalanced smoothing, we draw the following conclusions:

	(1) for nx < ng, the number of calculations in zone 1 is equal
	to n2 + 1; (2) for nx >= ng, the number of calculations in
	zone 1 is equal to n2; (3) the number of calculations in zone
	2 is equal to the greater of nx + 1 - ng and 0; (4) for nx <
	ng, the number of calculations in zone 3 is equal to nx - n2 -
	1; (5) for nx >= ng, the number of calculations in zone 3 is
	equal to n2;

	(6) the number of included data points in the first
	calculation in zone 1 is equal to n2; it increases by 1 every
	next calculation until a total of nx is reached; (7) the
	number of included data points in zone 2 is equal to ng; (8)
	if nx < ng, then the number of included data points in the
	first calculation in zone 3 is equal to nx - 1; it decreases
	by 1 every next calculation; (9) if nx >= ng, then the number
	of included data points in the first calculation in zone 3 is
	equal to ng - 1; it decreases by 1 every next calculation

	(10) the index of the first included kernel element in the
	first calculation in zone 1 is equal to n2; it decreases by 1
	every next calculation; (11) the index of the first included
	kernel element in all calculations in zones 2 and 3 is equal
	to 0.

	(12) the index of the first included data point is equal to 0
	for all calculations in zone 1; (13) the index of the first
	included data point for the first calculation in zone 2 is
	equal to 0; it increases by 1 every next calculation through
	the end of zone 3; (14) for nx < ng, the index of the first
	included data point for the first calculation in zone 3 is
	equal to 1; it increases by 1 every next calculation; (15) for
	nx >= ng, the index of the first included data point for the
	first calculation in zone 3 is equal to nx - ng + 1; it
	increases by 1 every next calculation.

	For balanced smoothing, the conclusions are:

	(1) for nx < ng, the number of calculations in zone 1 is equal
	to (nx + 1)/2; (2) for nx >= ng, the number of calculations in
	zone 1 is equal to n2; (3) the number of calculations in zone
	2 is equal to the greater of nx - ng + 1 and 0; (4) for nx <
	ng, the number of calculations in zone 3 is equal to nx/2; (5)
	for nx >= ng, the number of calculations in zone 3 is equal to
	n2;

        (6) the number of included data points in the first
        calculation in zone 1 is equal to 1; it increases by 2 every
        next calculation; (7) the number of included data points in
        all calculations in zone 2 is equal to ng; (8) for nx < ng,
        the number of included data points in the first calculation in
        zone 3 is equal to the greatest odd number less than nx; it
        decreases by 2 every next calculation; (9) for nx >= ng, the
        number of included data points in the first calculation in
        zone 3 is equal to ng - 2; it decreases by 2 every next
        calculation;

	(10) the index of the first included kernel element in the
	first calculation in zone 1 is equal to n2; it decreases by 1
	every next calculation; (11) the index of the first included
	kernel element in all calculations in zone 2 is equal to 0;
	(12) for nx < ng, the index of the first included kernel
	element in the first calculation in zone 3 is equal to (ng -
	nx)/2 + 1; it increases by 1 every next calculation; (13) for
	nx >= ng, the index of the first included kernel element in
	the first calculation in zone 3 is equal to 1; it increases by
	1 every next calculation;

	(14) the index of the first included data point for all
	calculations in zone 1 is equal to 0; (15) the index of the
	first included data point for the first calculation in zone 2
	is equal to 0; it increases by 1 every next calculation; (16)
	for nx < ng, the index of the first included data point for
	the first calculation in zone 3 is equal to 2 if nx is odd, or
	1 if nx is even; it increases by 2 every next calculation (17)
	for nx >= ng, the index of the first included data point for
	the first calculation in zone 3 is equal to nx - ng + 2; it
	increases by 2 every next calculation. */
      case LUX_BYTE:
	do {		/* main loop */
	  /* zone 1: left edge */
	  ik = n2;		/* index into kernel */
	  if (internalMode & 4) {/* /BALANCED */
	    k = (nx < ng)? (nx + 1)/2: n2; /* # calculations */
	    i2 = 1;		/* number of points to sum */
	  } else {		/* not balanced */
	    k = n2 + (nx < ng);
	    i2 = n2;
	  }
	  for (j = 0; j < k; j++) {
	    sum = sumg = 0.0;
	    pt3 = gkern.f + ik; /* pointer into kernel */
	    pt1.b = src.b;	/* pointer into source */
	    n = i2;
	    while (n--) {
	      sum += (float) *pt1.b * *pt3;
	      pt1.b += stride;
	      sumg += *pt3++;
	    }
	    *trgt.f = sum / ((internalMode & 2)? gsum: sumg);
	    trgt.f += stride;
	    ik--;
	    if (internalMode & 4) /* /BALANCED */
	      i2 += 2;		/* add new points in pairs */
	    else 		/* unbalanced */
	      if (i2 < nx) 
		i2++;		/* add one more */
	  }
	  /* zone 2 */
	  k = nx + 1 - ng;	/* number of calculations in zone 2 */
	  for (j = 0; j < k; j++) {
	    sum = 0.0;
	    pt1.b = src.b + stride*j; /* pointer into source */
	    pt3 = gkern.f;	/* pointer to start of kernel */
	    n = ng;
	    while (n--) {
	      sum += (float) *pt1.b * *pt3++;
	      pt1.b += stride;
	    }
	    *trgt.f = sum / gsum;
	    trgt.f += stride;
	  }
	  /* zone 3: right edge */
	  if (internalMode & 4) { /* /BALANCED */
	    if (nx < ng) {
	      k = nx/2;
	      ik = (ng - nx)/2 + 1;
	      id = 2 - (nx % 2 == 0);
	      i2 = nx - 1 - (nx % 2 == 1);
	    } else {
	      k = n2;
	      ik = 1;
	      id = nx - ng + 2;
	      i2 = ng - 2;
	    }
	  } else {		/* unbalanced */
	    ik = 0;
	    if (nx < ng) {
	      k = nx - n2 - 1;
	      id = 1;
	      i2 = nx - 1;
	    } else {
	      k = n2;
	      id = nx - ng + 1;
	      i2 = ng - 1;
	    }
	  }
	  for (j = 0; j < k; j++) {
	    sum = sumg = 0.0;
	    pt1.b = src.b + id*stride; /* pointer into source */
	    pt3 = gkern.f + ik;	/* pointer into kernel */
	    n = i2;
	    while (n--) {
	      sum += (float) *pt1.b * *pt3;
	      pt1.b += stride;
	      sumg += *pt3++;
	    }
	    *trgt.f = sum / ((internalMode & 2)? gsum: sumg);
	    trgt.f += stride;
	    if (internalMode & 4) { /* /BALANCED */
	      id += 2;
	      i2 -= 2;
	      ik++;
	    } else {		/* unbalanced */
	      id++;
	      i2--;
	    }
	  }
	  src.b += nx*stride;
	} while (advanceLoop(&trgtinfo, &trgt),
		 advanceLoop(&srcinfo, &src) < srcinfo.rndim);
	break;
      case LUX_WORD:
	do {		/* main loop */
	  /* zone 1: left edge */
	  ik = n2;		/* index into kernel */
	  if (internalMode & 4) {/* /BALANCED */
	    k = (nx < ng)? (nx + 1)/2: n2; /* # calculations */
	    i2 = 1;		/* number of points to sum */
	  } else {		/* not balanced */
	    k = n2 + (nx < ng);
	    i2 = n2;
	  }
	  for (j = 0; j < k; j++) {
	    sum = sumg = 0.0;
	    pt3 = gkern.f + ik; /* pointer into kernel */
	    pt1.w = src.w;	/* pointer into source */
	    n = i2;
	    while (n--) {
	      sum += (float) *pt1.w * *pt3;
	      pt1.w += stride;
	      sumg += *pt3++;
	    }
	    *trgt.f = sum / ((internalMode & 2)? gsum: sumg);
	    trgt.f += stride;
	    ik--;
	    if (internalMode & 4) /* /BALANCED */
	      i2 += 2;		/* add new points in pairs */
	    else 		/* unbalanced */
	      if (i2 < nx) 
		i2++;		/* add one more */
	  }
	  /* zone 2 */
	  k = nx + 1 - ng;	/* number of calculations in zone 2 */
	  for (j = 0; j < k; j++) {
	    sum = 0.0;
	    pt1.w = src.w + stride*j; /* pointer into source */
	    pt3 = gkern.f;	/* pointer to start of kernel */
	    n = ng;
	    while (n--) {
	      sum += (float) *pt1.w * *pt3++;
	      pt1.w += stride;
	    }
	    *trgt.f = sum / gsum;
	    trgt.f += stride;
	  }
	  /* zone 3: right edge */
	  if (internalMode & 4) { /* /BALANCED */
	    if (nx < ng) {
	      k = nx/2;
	      ik = (ng - nx)/2 + 1;
	      id = 2 - (nx % 2 == 0);
	      i2 = nx - 1 - (nx % 2 == 1);
	    } else {
	      k = n2;
	      ik = 1;
	      id = nx - ng + 2;
	      i2 = ng - 2;
	    }
	  } else {		/* unbalanced */
	    ik = 0;
	    if (nx < ng) {
	      k = nx - n2 - 1;
	      id = 1;
	      i2 = nx - 1;
	    } else {
	      k = n2;
	      id = nx - ng + 1;
	      i2 = ng - 1;
	    }
	  }
	  for (j = 0; j < k; j++) {
	    sum = sumg = 0.0;
	    pt1.w = src.w + id*stride; /* pointer into source */
	    pt3 = gkern.f + ik;	/* pointer into kernel */
	    n = i2;
	    while (n--) {
	      sum += (float) *pt1.w * *pt3;
	      pt1.w += stride;
	      sumg += *pt3++;
	    }
	    *trgt.f = sum / ((internalMode & 2)? gsum: sumg);
	    trgt.f += stride;
	    if (internalMode & 4) { /* /BALANCED */
	      id += 2;
	      i2 -= 2;
	      ik++;
	    } else {		/* unbalanced */
	      id++;
	      i2--;
	    }
	  }
	  src.w += nx*stride;
	} while (advanceLoop(&trgtinfo, &trgt),
		 advanceLoop(&srcinfo, &src) < srcinfo.rndim);
	break;
      case LUX_LONG:
	do {		/* main loop */
	  /* zone 1: left edge */
	  ik = n2;		/* index into kernel */
	  if (internalMode & 4) {/* /BALANCED */
	    k = (nx < ng)? (nx + 1)/2: n2; /* # calculations */
	    i2 = 1;		/* number of points to sum */
	  } else {		/* not balanced */
	    k = n2 + (nx < ng);
	    i2 = n2;
	  }
	  for (j = 0; j < k; j++) {
	    sum = sumg = 0.0;
	    pt3 = gkern.f + ik; /* pointer into kernel */
	    pt1.l = src.l;	/* pointer into source */
	    n = i2;
	    while (n--) {
	      sum += (float) *pt1.l * *pt3;
	      pt1.l += stride;
	      sumg += *pt3++;
	    }
	    *trgt.f = sum / ((internalMode & 2)? gsum: sumg);
	    trgt.f += stride;
	    ik--;
	    if (internalMode & 4) /* /BALANCED */
	      i2 += 2;		/* add new points in pairs */
	    else 		/* unbalanced */
	      if (i2 < nx) 
		i2++;		/* add one more */
	  }
	  /* zone 2 */
	  k = nx + 1 - ng;	/* number of calculations in zone 2 */
	  for (j = 0; j < k; j++) {
	    sum = 0.0;
	    pt1.l = src.l + stride*j; /* pointer into source */
	    pt3 = gkern.f;	/* pointer to start of kernel */
	    n = ng;
	    while (n--) {
	      sum += (float) *pt1.l * *pt3++;
	      pt1.l += stride;
	    }
	    *trgt.f = sum / gsum;
	    trgt.f += stride;
	  }
	  /* zone 3: right edge */
	  if (internalMode & 4) { /* /BALANCED */
	    if (nx < ng) {
	      k = nx/2;
	      ik = (ng - nx)/2 + 1;
	      id = 2 - (nx % 2 == 0);
	      i2 = nx - 1 - (nx % 2 == 1);
	    } else {
	      k = n2;
	      ik = 1;
	      id = nx - ng + 2;
	      i2 = ng - 2;
	    }
	  } else {		/* unbalanced */
	    ik = 0;
	    if (nx < ng) {
	      k = nx - n2 - 1;
	      id = 1;
	      i2 = nx - 1;
	    } else {
	      k = n2;
	      id = nx - ng + 1;
	      i2 = ng - 1;
	    }
	  }
	  for (j = 0; j < k; j++) {
	    sum = sumg = 0.0;
	    pt1.l = src.l + id*stride; /* pointer into source */
	    pt3 = gkern.f + ik;	/* pointer into kernel */
	    n = i2;
	    while (n--) {
	      sum += (float) *pt1.l * *pt3;
	      pt1.l += stride;
	      sumg += *pt3++;
	    }
	    *trgt.f = sum / ((internalMode & 2)? gsum: sumg);
	    trgt.f += stride;
	    if (internalMode & 4) { /* /BALANCED */
	      id += 2;
	      i2 -= 2;
	      ik++;
	    } else {		/* unbalanced */
	      id++;
	      i2--;
	    }
	  }
	  src.l += nx*stride;
	} while (advanceLoop(&trgtinfo, &trgt),
		 advanceLoop(&srcinfo, &src) < srcinfo.rndim);
	break;
      case LUX_FLOAT:
	do {		/* main loop */
	  /* zone 1: left edge */
	  ik = n2;		/* index into kernel */
	  if (internalMode & 4) {/* /BALANCED */
	    k = (nx < ng)? (nx + 1)/2: n2; /* # calculations */
	    i2 = 1;		/* number of points to sum */
	  } else {		/* not balanced */
	    k = n2 + (nx < ng);
	    i2 = n2;
	  }
	  for (j = 0; j < k; j++) {
	    sum = sumg = 0.0;
	    pt3 = gkern.f + ik; /* pointer into kernel */
	    pt1.f = src.f;	/* pointer into source */
	    n = i2;
	    while (n--) {
	      sum += (float) *pt1.f * *pt3;
	      pt1.f += stride;
	      sumg += *pt3++;
	    }
	    *trgt.f = sum / ((internalMode & 2)? gsum: sumg);
	    trgt.f += stride;
	    ik--;
	    if (internalMode & 4) /* /BALANCED */
	      i2 += 2;		/* add new points in pairs */
	    else 		/* unbalanced */
	      if (i2 < nx) 
		i2++;		/* add one more */
	  }
	  /* zone 2 */
	  k = nx + 1 - ng;	/* number of calculations in zone 2 */
	  for (j = 0; j < k; j++) {
	    sum = 0.0;
	    pt1.f = src.f + stride*j; /* pointer into source */
	    pt3 = gkern.f;	/* pointer to start of kernel */
	    n = ng;
	    while (n--) {
	      sum += (float) *pt1.f * *pt3++;
	      pt1.f += stride;
	    }
	    *trgt.f = sum / gsum;
	    trgt.f += stride;
	  }
	  /* zone 3: right edge */
	  if (internalMode & 4) { /* /BALANCED */
	    if (nx < ng) {
	      k = nx/2;
	      ik = (ng - nx)/2 + 1;
	      id = 2 - (nx % 2 == 0);
	      i2 = nx - 1 - (nx % 2 == 1);
	    } else {
	      k = n2;
	      ik = 1;
	      id = nx - ng + 2;
	      i2 = ng - 2;
	    }
	  } else {		/* unbalanced */
	    ik = 0;
	    if (nx < ng) {
	      k = nx - n2 - 1;
	      id = 1;
	      i2 = nx - 1;
	    } else {
	      k = n2;
	      id = nx - ng + 1;
	      i2 = ng - 1;
	    }
	  }
	  for (j = 0; j < k; j++) {
	    sum = sumg = 0.0;
	    pt1.f = src.f + id*stride; /* pointer into source */
	    pt3 = gkern.f + ik;	/* pointer into kernel */
	    n = i2;
	    while (n--) {
	      sum += (float) *pt1.f * *pt3;
	      pt1.f += stride;
	      sumg += *pt3++;
	    }
	    *trgt.f = sum / ((internalMode & 2)? gsum: sumg);
	    trgt.f += stride;
	    if (internalMode & 4) { /* /BALANCED */
	      id += 2;
	      i2 -= 2;
	      ik++;
	    } else {		/* unbalanced */
	      id++;
	      i2--;
	    }
	  }
	  src.f += nx*stride;
	} while (advanceLoop(&trgtinfo, &trgt),
		 advanceLoop(&srcinfo, &src) < srcinfo.rndim);
	break;
      case LUX_DOUBLE:
	do {		/* main loop */
	  /* zone 1: left edge */
	  ik = n2;		/* index into kernel */
	  if (internalMode & 4) {/* /BALANCED */
	    k = (nx < ng)? (nx + 1)/2: n2; /* # calculations */
	    i2 = 1;		/* number of points to sum */
	  } else {		/* not balanced */
	    k = n2 + (nx < ng);
	    i2 = n2;
	  }
	  for (j = 0; j < k; j++) {
	    sum = sumg = 0.0;
	    pt3 = gkern.f + ik; /* pointer into kernel */
	    pt1.d = src.d;	/* pointer into source */
	    n = i2;
	    while (n--) {
	      sum += (float) *pt1.d * *pt3;
	      pt1.d += stride;
	      sumg += *pt3++;
	    }
	    *trgt.f = sum / ((internalMode & 2)? gsum: sumg);
	    trgt.f += stride;
	    ik--;
	    if (internalMode & 4) /* /BALANCED */
	      i2 += 2;		/* add new points in pairs */
	    else 		/* unbalanced */
	      if (i2 < nx) 
		i2++;		/* add one more */
	  }
	  /* zone 2 */
	  k = nx + 1 - ng;	/* number of calculations in zone 2 */
	  for (j = 0; j < k; j++) {
	    sum = 0.0;
	    pt1.d = src.d + stride*j; /* pointer into source */
	    pt3 = gkern.f;	/* pointer to start of kernel */
	    n = ng;
	    while (n--) {
	      sum += (float) *pt1.d * *pt3++;
	      pt1.d += stride;
	    }
	    *trgt.f = sum / gsum;
	    trgt.f += stride;
	  }
	  /* zone 3: right edge */
	  if (internalMode & 4) { /* /BALANCED */
	    if (nx < ng) {
	      k = nx/2;
	      ik = (ng - nx)/2 + 1;
	      id = 2 - (nx % 2 == 0);
	      i2 = nx - 1 - (nx % 2 == 1);
	    } else {
	      k = n2;
	      ik = 1;
	      id = nx - ng + 2;
	      i2 = ng - 2;
	    }
	  } else {		/* unbalanced */
	    ik = 0;
	    if (nx < ng) {
	      k = nx - n2 - 1;
	      id = 1;
	      i2 = nx - 1;
	    } else {
	      k = n2;
	      id = nx - ng + 1;
	      i2 = ng - 1;
	    }
	  }
	  for (j = 0; j < k; j++) {
	    sum = sumg = 0.0;
	    pt1.d = src.d + id*stride; /* pointer into source */
	    pt3 = gkern.f + ik;	/* pointer into kernel */
	    n = i2;
	    while (n--) {
	      sum += (float) *pt1.d * *pt3;
	      pt1.d += stride;
	      sumg += *pt3++;
	    }
	    *trgt.f = sum / ((internalMode & 2)? gsum: sumg);
	    trgt.f += stride;
	    if (internalMode & 4) { /* /BALANCED */
	      id += 2;
	      i2 -= 2;
	      ik++;
	    } else {		/* unbalanced */
	      id++;
	      i2--;
	    }
	  }
	  src.d += nx*stride;
	} while (advanceLoop(&trgtinfo, &trgt),
		 advanceLoop(&srcinfo, &src) < srcinfo.rndim);
	break;
    }
    if (nextLoops(&srcinfo, &trgtinfo)) {
      if (isFreeTemp(iq) && symbol_type(iq) == LUX_FLOAT) {
	n = result;
	result = iq;
	iq = n;
      } else {			/* need new temp for next result */
	iq = result;
	result = array_clone(iq, LUX_FLOAT);
	srcinfo.stride = lux_type_size[LUX_FLOAT];
      }
      src.b = array_data(iq);
      trgt.b = array_data(result);
    }
  }
  if (mem)
    free(gkern.f - dgkern);
  return result;
}
/*------------------------------------------------------------------------- */
int32_t lux_lower(narg,ps)					/*lower function */
/* convert all letters in a string to lower case */
int32_t narg,ps[];
{
register  uint8_t *p1, *p2;
register  int32_t  mq;
int32_t	result_sym;
if ( sym[ ps[0] ].class != 2 ) return cerror(NEED_STR, *ps);
mq = sym[ps[0]].spec.array.bstore - 1;
result_sym = string_scratch(mq);		/*for resultant string */
p1 = (uint8_t *) sym[ps[0] ].spec.array.ptr;
p2 = (uint8_t *) sym[result_sym].spec.array.ptr;
while (mq--)
 *p2++ = isupper( (int32_t) *p1 ) ? (uint8_t) tolower( (int32_t) *p1++) : *p1++;
*p2 = 0;					/*ending null */
return result_sym;
}
/*------------------------------------------------------------------------- */
int32_t lux_upper(narg,ps)					/*upper function */
/* convert all letters in a string to upper case */
int32_t narg,ps[];
{
register  uint8_t *p1, *p2;
register  int32_t  mq;
int32_t	result_sym;
if ( sym[ ps[0] ].class != 2 ) return cerror(NEED_STR, *ps);
mq = sym[ps[0]].spec.array.bstore - 1;
result_sym = string_scratch(mq);		/*for resultant string */
p1 = (uint8_t *) sym[ps[0] ].spec.array.ptr;
p2 = (uint8_t *) sym[result_sym].spec.array.ptr;
while (mq--)
 *p2++ = islower( (int32_t) *p1 ) ? (uint8_t) toupper( (int32_t) *p1++) : *p1++;
*p2 = 0;					/*ending null */
return result_sym;
}
/*------------------------------------------------------------------------- */
int32_t lux_strpos(int32_t narg, int32_t ps[]) /*STRPOS function */
/* returns index of sub string if found */
     /* added index - LS 11jun97 */
{
  register char *p1, *p2, *p3;
  int32_t	result_sym, index;

  if (symbol_class(ps[0]) != LUX_STRING)
    return cerror(NEED_STR, *ps);
  if (symbol_class(ps[1]) != LUX_STRING)
    return cerror(NEED_STR, ps[1]);
  result_sym = scalar_scratch(LUX_LONG); /*for resultant position */
  scalar_value(result_sym).l = -1;

  p1 = string_value(ps[0]);
  p2 = string_value(ps[1]);
  if (narg > 2)
  { index = int_arg(ps[2]);	/* start index */
    if (index < 0)		/* start seeking before string start */
      index = 0; 		/* so start at beginning of string */
    else if (index >= string_size(ps[0])) /* seek beyond string */
      return result_sym; }	/* so return -1 */
  else
    index = 0;
  p3 = strstr(p1 + index, p2);
  if (p3 != NULL)
    scalar_value(result_sym).l = p3 - p1;
  return result_sym;
}
/*------------------------------------------------------------------------- */
int32_t lux_strcount(narg,ps)				/*STRCOUNT function */
/*  count # of occurences of a substring  */
int32_t narg,ps[];
{
register char *p1, *p2, *p3;
int32_t	result_sym, n;
if ( sym[ ps[0] ].class != 2 ) return cerror(NEED_STR, ps[0]);
if ( sym[ ps[1] ].class != 2 ) return cerror(NEED_STR, ps[1]);
result_sym = scalar_scratch(2);		/*for resultant position */
n = 0;
p1 = (char *) sym[ps[0] ].spec.array.ptr;
p2 = (char *) sym[ps[1] ].spec.array.ptr;
while ( (p3 = strstr( p1, p2)) != NULL) {
n++;   p1 =  p3 + sym[ps[1]].spec.array.bstore - 1; }
sym[result_sym].spec.scalar.l = n;
return result_sym;
}
/*------------------------------------------------------------------------- */
int32_t lux_strreplace(narg,ps)			/*STRREPLACE function */
/* replace a substring nc times */
int32_t narg,ps[];
{
register char *p1, *p2, *p3, *p4;
char	*pq;
int32_t	result_sym, n, nc, nr, mq, ns;
if ( sym[ ps[0] ].class != 2 ) return cerror(NEED_STR, ps[0]);
if ( sym[ ps[1] ].class != 2 ) return cerror(NEED_STR, ps[1]);
if ( sym[ ps[2] ].class != 2 ) return cerror(NEED_STR, ps[2]);
nc = 0;
if (narg >= 4 ) nc = int_arg( ps[3] );
p1 = (char *) sym[ps[0] ].spec.array.ptr;
p2 = (char *) sym[ps[1] ].spec.array.ptr;
p3 = (char *) sym[ps[2] ].spec.array.ptr;
/* do in 2 passes, first count the number of target substrings up to nc */
n= 0;
pq = p1;
while ( (p4 = strstr( p1, p2)) != NULL) {
n++;   p1 =  p4 + sym[ps[1]].spec.array.bstore - 1;
if ( n >= nc && nc > 0) break;	}
		/* n is the count to replace, compute storage */
nr = sym[ps[2]].spec.array.bstore - 1;
ns = sym[ps[1]].spec.array.bstore - 1;
mq = sym[ps[0]].spec.array.bstore - 1 + n * (nr - ns);
result_sym = string_scratch(mq);		/*for resultant string */
p4 = (char *) sym[ result_sym ].spec.array.ptr;
p1 = pq;
while ( n-- ) { pq = strstr( p1, p2);
				/*transfer any before the target */
nc = ( pq - p1 );
while (nc--)  *p4++ = *p1++;
pq = p3; nc = nr;
while (nc--)  *p4++ = *pq++;
p1 += ns;
}
				/*leftovers */
nc = mq - (p4 - (char *) sym[ result_sym ].spec.array.ptr);
if (nc > 0) { while (nc--) { *p4++ = *p1++; } }
*p4 = '\0';
return result_sym;
}
/*------------------------------------------------------------------------- */
int32_t lux_strpbrk(int32_t narg, int32_t ps[])
 /* returns a string from s1 starting with the first char from s2
 found in s1, if none found, an empty string */
{
  char *p1, *p2, *p3, *p4;
  int32_t  mq, off;
  int32_t    result_sym;

  if (!symbolIsStringScalar(ps[0]))
    return cerror(NEED_STR, ps[0]);
  if (!symbolIsStringScalar(ps[1]))
    return cerror(NEED_STR, ps[1]);
  p1 = string_value(ps[0]);
  p2 = string_value(ps[1]);
  p3 = strpbrk(p1, p2);
  mq = 0; off = (p3 - p1);
  if ( p3 != NULL ) mq = string_size(ps[0]) - off;
  result_sym = string_scratch(mq);               /*for resultant string */
  p4 = string_value(result_sym);
  if (mq != 0)  bcopy( p3, p4, mq );
  *(p4 + mq) = 0;
  return result_sym;
}
/*------------------------------------------------------------------------- */
int32_t lux_strloc(narg,ps)					/*STRLOC function */
/* returns rest of source starting at object string
 */
int32_t narg,ps[];
{
register char *p1, *p2, *p3;
register  int32_t  mq, off;
int32_t	result_sym;
if ( sym[ ps[0] ].class != 2 ) return cerror(NEED_STR, ps[0]);
if ( sym[ ps[1] ].class != 2 ) return cerror(NEED_STR, ps[1]);
p1 = (char *) sym[ps[0] ].spec.array.ptr;
p2 = (char *) sym[ps[1] ].spec.array.ptr;
p3 = strstr( p1, p2);
mq = 0; off = (p3 - p1);
if ( p3 != NULL ) mq = sym[ps[0]].spec.array.bstore - 1 - off;
result_sym = string_scratch(mq);		/*for resultant string */
p3 = (char *) sym[result_sym].spec.array.ptr;
memcpy(p3, p1 + off, mq);
*(p3 + mq) = 0;
return result_sym;
}
/*------------------------------------------------------------------------- */
int32_t lux_strskp(narg,ps)					/*STRSKP function */
/*  returns rest of source starting AFTER object string
 */
int32_t narg,ps[];
{
register char *p1, *p2, *p3;
register  int32_t  mq, off;
int32_t	result_sym;
if ( sym[ ps[0] ].class != 2 ) return cerror(NEED_STR, ps[0]);
if ( sym[ ps[1] ].class != 2 ) return cerror(NEED_STR, ps[1]);
p1 = (char *) sym[ps[0] ].spec.array.ptr;
p2 = (char *) sym[ps[1] ].spec.array.ptr;
p3 = strstr( p1, p2);
mq = 0; off = (p3 - p1) + sym[ps[1]].spec.array.bstore - 1;
if ( p3 != NULL )
 mq = sym[ps[0]].spec.array.bstore - 1 - off;
if (mq < 0)  mq = 0; 
result_sym = string_scratch(mq);		/*for resultant string */
p3 = (char *) sym[result_sym].spec.array.ptr;
if (mq != 0)  memcpy(p3, p1 + off, mq);
*(p3 + mq) = 0;
return result_sym;
}
/*------------------------------------------------------------------------- */
int32_t lux_skipc(narg,ps)					/*SKIPC function */
/* skip over characters */
int32_t narg,ps[];
{
register char *p1, *p2, *p3;
int32_t  mq, off;
int32_t	result_sym;
if ( sym[ ps[0] ].class != 2 ) return cerror(NEED_STR, ps[0]);
if ( sym[ ps[1] ].class != 2 ) return cerror(NEED_STR, ps[1]);
p1 = (char *) sym[ps[0] ].spec.array.ptr;
p2 = (char *) sym[ps[1] ].spec.array.ptr;
off = strspn( p1, p2);
mq = sym[ps[0]].spec.array.bstore - 1 - off;
if (mq < 0)  mq = 0;
result_sym = string_scratch(mq);		/*for resultant string */
p3 = (char *) sym[result_sym].spec.array.ptr;
if (mq != 0)  memcpy(p3, p1 + off, mq);
*(p3 + mq) = 0;
return result_sym;
}
/*------------------------------------------------------------------------- */
int32_t lux_strsub(narg,ps)				/*STRSUB function */
/* return a substring */
int32_t narg,ps[];
{
register char *p1, *p3;
int32_t  mq, off, len;
int32_t	result_sym;
if ( sym[ ps[0] ].class != 2 ) return cerror(NEED_STR, ps[0]);
p1 = (char *) sym[ps[0] ].spec.array.ptr;
off = int_arg( ps[1] );
len = int_arg( ps[2] );
if (len < 0 ) len = 0;
mq = sym[ps[0]].spec.array.bstore - 1 - off;
mq = len <= mq ? len : mq;
result_sym = string_scratch(mq);		/*for resultant string */
p3 = (char *) sym[result_sym].spec.array.ptr;
if (mq != 0)  memcpy(p3, p1 + off, mq);
*(p3 + mq) = 0;
return result_sym;
}
/*------------------------------------------------------------------------- */
int32_t lux_strtrim(int32_t narg, int32_t ps[]) /*STRTRIM function */
/* trim trailing blanks and tabs and any control chars */
     /* older version took off last OK char, too - fixed.  LS 7may97 */
{
  register char *p1, *p2, *p3;
  int32_t  mq;
  int32_t	result_sym;
  
  if (symbol_class(ps[0]) != LUX_STRING)
    return cerror(NEED_STR, *ps);
  p1 = string_value(ps[0]);
  mq = string_size(ps[0]);

  if (mq) {
    p2 = p1 + mq;		/*just after end */
    while (1) {
      --p2;
      if (p2 < p1 || *p2 > ' ')
	break;
    }
    mq = p2 - p1 + 1;
  }
  result_sym = string_scratch(mq);		/*for resultant string */
  p3 = string_value(result_sym);
  if (mq)
    memcpy(p3, p1, mq);
  p3[mq] = 0;
  return result_sym;
}
/*------------------------------------------------------------------------- */
int32_t lux_istring(narg,ps)				/* ISTRING function */
/* string in I format */
int32_t narg,ps[];
{
char	s[6], *p3;
int32_t  iq, mq, n, k, mode;
int32_t	result_sym;
iq = ps[0];
switch (sym[iq].class)	{
default: return cerror(NO_SCAL, iq);
case 1:							/*scalar case */
k = int_arg(iq);
}
n=10;
mode = 0;
if ( narg > 1 ) n = int_arg( ps[1] );
if ( narg > 2 ) mode = int_arg( ps[2] );
switch (mode) {
default:
case 0: sprintf(s, "%%%1dd", n); break;
case 1: sprintf(s, "%%0%1dd", n); break;
case 2: sprintf(s, "%%1d"); break;
}
/* s now has the  control string for the formatting, use line to store
	result since length can be uncertain for some cases */
sprintf(templine, s, k);
mq = strlen(templine);
/*printf("mq = %d, s = %s, line = %s\n",mq,s,templine);*/
result_sym = string_scratch(mq);		/*for resultant string */
p3 = (char *) sym[result_sym].spec.array.ptr;
memcpy(p3, templine, mq);
*(p3 + mq) = 0;
return result_sym;
}
/*------------------------------------------------------------------------- */
int32_t lux_fstring(int32_t narg, int32_t ps[])			/* FSTRING function */
/* string from scalar, C-format */
/* syntax:  FSTRING(format, arg1, arg2, ...)
   the argument is modified to suit the data type expected by the format.
   LS 24apr93 */
{
 extern	int32_t	column;
 char	*p3;
 int32_t	col, mq;
 int32_t	type_formatted_ascii(int32_t, int32_t [], FILE *);

 col = column;
 column = 0;
 type_formatted_ascii(narg, ps, NULL);
 column = col;
 mq = strlen(curScrat);
 result_sym = string_scratch(mq); /* for resultant string */
 p3 = string_value(result_sym);
 memcpy(p3, curScrat, mq);
 p3[mq] = '\0';
 return result_sym;
}
/*------------------------------------------------------------------------- */
int32_t lux_string(int32_t narg, int32_t ps[])
 /* converts to LUX_STRING
   LS 24apr93 21sep98 8sep99 */
{
  scalar	value;
  char	*p;
  int32_t	result, n, nel;
  pointer	q;
  extern char	*fmt_integer, *fmt_float;

  if (narg > 1)			/* FSTRING */
    return lux_fstring(narg, ps);

  switch (symbol_class(ps[0])) {
    case LUX_SCALAR:
      switch (scalar_type(ps[0])) {
	case LUX_BYTE:
	  value.l = scalar_value(ps[0]).b;
	  break;
	case LUX_WORD:
	  value.l = scalar_value(ps[0]).w;
	  break;
	case LUX_LONG:
	  value.l = scalar_value(ps[0]).l;
	  break;
	case LUX_FLOAT:
	  value.d = scalar_value(ps[0]).f;
	  break;
	case LUX_DOUBLE:
	  value.d = scalar_value(ps[0]).d;
	  break;
	default:
	  return cerror(ILL_TYPE, ps[0]);
      }
      switch (scalar_type(ps[0])) {
	case LUX_BYTE: case LUX_WORD: case LUX_LONG:
	  sprintf(curScrat, fmt_integer, value.l);
	  break;
	case LUX_FLOAT: case LUX_DOUBLE:
	  sprintf(curScrat, fmt_float, value.d);
	  break;
      }
      p = curScrat;
      while (isspace((uint8_t) *p)) /* skip initial whitespace */
	p++;
      result = string_scratch(strlen(p));
      strcpy(string_value(result), p);
      break;
    case LUX_STRING:
      return ps[0];
    case LUX_ARRAY:
      if (array_type(ps[0]) == LUX_STRING_ARRAY) {
	/* we transform string arrays into strings, just stringing */
	/* everything together */
	n = 0;
	nel = array_size(ps[0]);
	q.sp = array_data(ps[0]);
	while (nel--)
	  n += strlen(*q.sp++);
	result = string_scratch(n);
	p = string_value(result);
	nel = array_size(ps[0]);
	q.sp -= nel;
	while (nel--) {
	  n = strlen(*q.sp);
	  if (*q.sp)
	    memcpy(p, *q.sp, n);
	  p += n;
	  q.sp++;
	}
	*p = '\0';		/* terminate */
      } else
	return cerror(ILL_TYPE, ps[0]);
      break;
    default:
      return cerror(ILL_CLASS, ps[0]);
  }
  return result;
}
/*------------------------------------------------------------------------- */
int32_t lux_strlen(int32_t narg, int32_t ps[])/*strlen function */
/* length, not counting null; duplicates num_elem but insists on string */
{
  int32_t	result_sym, *l, n;
  pointer	p;

  switch (symbol_class(ps[0])) {
    case LUX_STRING:
      result_sym = scalar_scratch(LUX_LONG);
      scalar_value(result_sym).l = string_size(ps[0]);
      break;
    case LUX_ARRAY:
      if (array_type(ps[0]) == LUX_STRING_ARRAY) {
	result_sym = array_scratch(LUX_LONG, array_num_dims(ps[0]),
				   array_dims(ps[0]));
	l = array_data(result_sym);
	p.sp = array_data(ps[0]);
	n = array_size(ps[0]);
	while (n--)
	  *l++ = strlen(*p.sp++);
      } else
	return cerror(ILL_TYPE, ps[0]);
      break;
    default:
      return cerror(ILL_CLASS, ps[0]);
  }
  return result_sym;
}
/*------------------------------------------------------------------------- */
int32_t lux_decomp(int32_t narg, int32_t ps[])/*decomp routine */
		/*replaces a square matrix with it's lu decomposition */
{
  int32_t	iq, nd, nx;
  array	*h;
  pointer q1;
  int32_t	lux_replace(int32_t, int32_t);

if ( narg != 1 ) return cerror(WRNG_N_ARG, 0);
iq = ps[0];
if (isFreeTemp(iq)) return cerror(RET_ARG_NO_ATOM, iq);
CK_ARR(iq, -1);
h = (array *) sym[iq].spec.array.ptr;
nd = h->ndim;	nx = h->dims[0];
if ( nd != 2 || nx != h->dims[1] ) return cerror(NEED_2D_SQ_ARR, iq);
						/*must be 2-D and square */
if  (sym[iq].type < 4 )  iq = lux_float(1, &iq);
if (iq != ps[0] ) lux_replace(ps[0], iq);
iq = ps[0];
h = (array *) sym[iq].spec.array.ptr;
q1.f = (float *) ((char *)h + sizeof(array));
		/*could be either float or double, support both */
switch ( sym[iq].type ) {
case 3:
  f_decomp( q1.f, nx, nx );	break;
case 4:
  d_decomp( q1.d, nx, nx );	break;
}
return 1;
}
/*------------------------------------------------------------------------- */
int32_t lux_dsolve(narg,ps)					/*decomp routine */
		/*solves a linear system given lu decomp and rhs */
int32_t narg,ps[];
{
int32_t	iq, jq, nd, nx, toptype, j, outer;
array	*h;
pointer q1, q2;
if ( narg != 2 ) return cerror(WRNG_N_ARG, 0);
iq = ps[0];				/*the lu decomp matrix */		
CK_ARR(iq, 1);
toptype = sym[iq].type;
h = (array *) sym[iq].spec.array.ptr;
nd = h->ndim;	nx = h->dims[0];
if ( nd != 2 || nx != h->dims[1] ) return cerror(NEED_2D_SQ_ARR, iq);
					/*must be 2-D and square */
jq = ps[1];				/*rhs side (may be more than one)*/
if (isFreeTemp(jq)) return cerror(RET_ARG_NO_ATOM, jq);
toptype = toptype > (int32_t) sym[jq].type ? toptype :  sym[iq].type;
if (toptype < 3) toptype = 3;
h = (array *) sym[jq].spec.array.ptr;
nd = h->ndim;	if ( h->dims[0] != nx) return cerror(INCMP_LU_RHS, iq);
outer = 1;
if (nd > 1) for(j=1;j<nd;j++) outer *= h->dims[j];
				/*check type and upgrade as necessary */
switch (toptype) {
case 3:
 iq = lux_float( 1, &iq); jq = lux_float( 1, &jq); break;
case 4:
 iq = lux_double( 1, &iq); jq = lux_double( 1, &jq); break;
}
if (jq != ps[1] ) lux_replace(ps[1], jq);	jq = ps[1];
h = (array *) sym[iq].spec.array.ptr;
q1.l = (int32_t *) ((char *)h + sizeof(array));
h = (array *) sym[jq].spec.array.ptr;
q2.l = (int32_t *) ((char *)h + sizeof(array));
while (outer--) {
		/*could be either float or double, support both */
switch ( toptype ) {
case 3:
f_solve( q1.f, q2.f, nx, nx ); q2.f += nx;	break;
case 4:
d_solve( q1.d, q2.d, nx, nx ); q2.d += nx;	break;
}
}
return 1;
}
/*------------------------------------------------------------------------- */
int32_t lux_pit(int32_t narg, int32_t ps[])	/*pit function */
			/* polynomial fit, CALL IS C=PIT([X],Y,[NPOW])*/
{
  double	*a, *fbase, *cfbase;
  pointer qxbase,qybase;
  int32_t	npow, symx, symy, nd, nxq, outer, outerx, dim[2], cfsym;
  int32_t	j, toptype;
  array	*h;
  int32_t	setxpit(int32_t, int32_t),
    clux_pit(pointer *, pointer *, int32_t, int32_t, double *, double *, double *,
	     int32_t, int32_t);

/* depending on # of nargs, we may have to default x or npow, narg=2 case
	is ambiguous until we get class */
npow = 0;	symx = -1;
switch (narg) {
case 3:
symx = ps[0];	symy = ps[1];	npow = int_arg( ps[2] ); break;
case 2:
/* two possibilities, determined by class of ps[1] */
if ( sym[ps[1]].class == 4 ) { 		/* the (x,y) case */
	symx = ps[0];	symy = ps[1]; }
	else { symy = ps[0];	npow = int_arg( ps[1] ); }
break;
case 1:			/* just the y vector */
symy = ps[0];
break;
default: return cerror(WRNG_N_ARG, 0);
}
if (npow < 1 || npow >10 ) return cerror(ILL_POWER, 0);	/*check power */
CK_ARR(symy, -1);
	/* if either y or x (if x specified) is double, do a double calc. */
toptype = sym[symy].type < 3 ? 3 : sym[symy].type;
h = (array *) sym[symy].spec.array.ptr;
	/*array may be upgraded but y dimensions will be same, get now */
nd = h->ndim;	nxq = h->dims[0];	outer = 1;
if (nd > 1) for(j=1;j<nd;j++) outer *= h->dims[j];
					/* check x if specified */
if (symx > 0) { CK_ARR(symx, -1); 
 toptype = (int32_t) sym[symx].type > toptype ? sym[symx].type : toptype;
 h = (array *) sym[symx].spec.array.ptr;
			/* a specified x must match y in some regards */
 nd = h->ndim;	if (h->dims[0] != nxq) return cerror(INCMP_DIMS, symy);
 outerx = 1;
 if (nd > 1) for(j=1;j<nd;j++) outerx *= h->dims[j];
 if (outerx != 1 && outerx != outer ) return cerror(INCMP_DIMS, symy);
} else {				/* make a fake x array */
symx = setxpit(toptype, nxq); outerx = 1;
}
				/* check types and upgrade as necessary */
switch (toptype) {
case 3:
 symy = lux_float( 1, &symy); symx = lux_float( 1, &symx); break;
case 4:
 symy = lux_double( 1, &symy); symx = lux_double( 1, &symx); break;
}
h = (array *) sym[symy].spec.array.ptr;
qybase.l = (int32_t *) ((char *)h + sizeof(array));
h = (array *) sym[symx].spec.array.ptr;
qxbase.l = (int32_t *) ((char *)h + sizeof(array));
			/* get the various matrices and vectors needed */
dim[0] = npow + 1;	dim[1] = outer;
if (outer > 1) cfsym = array_scratch(4, 2, dim);	/*always double */		
	else cfsym = array_scratch(4, 1, dim);
h = (array *) sym[cfsym].spec.array.ptr;
cfbase = (double*) ((char *)h + sizeof(array));
							/*scratch array */
allocate(fbase, nxq, double);
allocate(a, (npow + 1)*(npow + 1), double);
	/*loop over the number of individual fits to do */
while (outer--) { 
	/* ready to start actual calculation, all done in clux_pit */
clux_pit( &qxbase, &qybase, nxq, npow, a, cfbase, fbase, toptype, outerx);
cfbase += npow+1;
}					/*end of while (outer--) loop */
Free(a);  Free(fbase);
return cfsym;
}
/*------------------------------------------------------------------------- */
int32_t clux_pit(pointer *qxbase, pointer *qybase, int32_t nxq, int32_t npow, double *a,
	     double *cfbase, double *fbase, int32_t toptype, int32_t outerx)
/* internal routine used by lux_pit, lux_trend, and lux_detrend */
{
double	sum, *f, *cf;
pointer qx,qy;
int32_t	i, n, ib, ie, k;
cf = cfbase;	f = fbase;	qy.l = qybase->l;  qx.l = qxbase->l;
switch (toptype) {
case 3:
/*
printf("float case\n");
for (k=0;k<nxq;k++) printf("k, x(k), y(k) %d %f %f\n",k,*(qx.f+k),*(qy.f+k));
*/
n = nxq;  while (n--) *f++ = *qx.f++;
*a = nxq;
for (k=0;k < (2*npow);k++) { sum = 0.0; f = fbase; n = nxq; qx.l = qxbase->l;
	while (n--) {sum += *f;  *f = (*f) * (*qx.f++); f++; }
	ib = (k+1-npow) > 0 ? (k+1-npow) : 0;		/*max of */
	ie = (k+1) > (npow) ? (npow) : (k+1);		/*min of */
	for (i=ib;i<=ie;i++) {
	*( a + i + (npow+1)*(ie+ib-i) ) = sum;
	}
}
							/* now the rhs */
f = fbase; n = nxq;   while (n--) *f++ = *qy.f++;
for (k=0;k <= (npow);k++) { sum = 0.0; f = fbase; n = nxq; qx.l = qxbase->l;
	while (n--) {sum += *f;  *f = (*f) * (*qx.f++); f++; }
	*cf++ = sum; }
qybase->f += nxq;  if (outerx != 1) qxbase->f += nxq; break;
case 4:
/*
printf("double case\n");
for (k=0;k<nxq;k++) printf("k, x(k), y(k) %d %f %f\n",k,*(qx.d+k),*(qy.d+k));
*/
n = nxq;  while (n--) *f++ = *qx.d++;
*a = nxq;
for (k=0;k < (2*npow);k++) { sum = 0.0; f = fbase; n = nxq; qx.l = qxbase->l;
	while (n--) {sum += *f;  *f = (*f) * (*qx.d++); f++; }
	ib = (k+1-npow) > 0 ? (k+1-npow) : 0;		/*max of */
	ie = (k+1) > (npow) ? (npow) : (k+1);		/*min of */
	for (i=ib;i<=ie;i++) {
	*( a + i + (npow+1)*(ie+ib-i) ) = sum;
	}
}
							/* now the rhs */
f = fbase; n = nxq;   while (n--) *f++ = *qy.d++;
for (k=0;k <= (npow);k++) { sum = 0.0; f = fbase; n = nxq; qx.l = qxbase->l;
	while (n--) {sum += *f;  *f = (*f) * (*qx.d++); f++; }
	*cf++ = sum; }
qybase->d += nxq;  if (outerx != 1) qxbase->d += nxq; break;
}

d_decomp( a, npow+1, npow+1);
d_solve(a, cfbase, npow+1, npow+1);

/*for (k=0;k<=npow;k++) printf("k, cf(k) %d %f\n",k,*(cfbase+k));*/
return 1;
}
/*------------------------------------------------------------------------- */
int32_t setxpit(int32_t type, int32_t n)	/* used by several routines to set up
				   an x array */
			/*consisting of {0,1/n,2/n,...,(n-1)/n} */
			/* type must be 3 or 4 */
{
  register pointer qx;
  register float	del;
  register double	ddel;
  int32_t	nsym, nx;
  array	*h;
  nx = n;
  nsym = array_scratch(type, 1, &nx );
  h = (array *) sym[nsym].spec.array.ptr;
  qx.d = (double *) ((char *)h + sizeof(array));
  switch (type) {
  case 3:
    del = 1.0 / nx;  *qx.f = 0.0;  while (--nx) *(qx.f+1) = *qx.f++ + del; break;
  case 4:
    ddel = 1.0/ nx;  *qx.d = 0.0;  while (--nx) *(qx.d+1) = *qx.d++ + ddel; break;
  }
  qx.l = (int32_t *) ((char *)h + sizeof(array));
  return nsym;
}
/*------------------------------------------------------------------------- */
int32_t lux_detrend(int32_t narg, int32_t ps[])/*detrend function */
	/* detrend using a polynomial fit, CALL IS DT=detrend(Y,[NPOW])*/
{
  int32_t	lux_trend(int32_t, int32_t []);
			/* just call trend  with the detrend flag set */
  detrend_flag = 1;
  return lux_trend(narg, ps);
}
/*------------------------------------------------------------------------- */
int32_t lux_trend(int32_t narg, int32_t ps[]) /*trend function */
	/* trend using a polynomial fit, CALL IS T=trend(Y,[NPOW])*/
{
  register double	*a;
  double	*fbase, *cfbase;
  pointer qxbase,qybase,qzbase;
  int32_t	npow, symx, symy, nd, nxq, outer;
  int32_t	toptype, result_sym;
  int32_t	clux_poly(double *cfbase, pointer *qxbase, int32_t nxq, int32_t npow,
		  pointer *qzbase, int32_t toptype);

  /* second argument is optional, default is linear fit (npow = 1) */
  npow = 1;
  switch (narg)
  { case 2:
      npow = int_arg(ps[1]);
    case 1:
      symy = ps[0];
      break;
    default:
      return cerror(WRNG_N_ARG, 0); }
  if (npow < 1 || npow > 10)
    return cerror(ILL_POWER, ps[1]); /*check power */
  CK_ARR(symy, 1);
  toptype = symbol_type(symy) < LUX_FLOAT ? LUX_FLOAT : symbol_type(symy);
  /*array may be upgraded but y dimensions will be same, get now */
  nd = array_num_dims(symy);
  nxq = array_dims(symy)[0];
  outer = array_size(symy)/nxq;
						/* make a fake x array */
  symx = setxpit(toptype, nxq);
				/* check type and upgrade as necessary */
  switch (toptype)
  { case LUX_FLOAT:
      symy = lux_float(1, &symy);
      symx = lux_float(1, &symx);
      break;
    case LUX_DOUBLE:
      symy = lux_double(1, &symy);
      symx = lux_double(1, &symx);
      break; }
  qybase.l = array_data(symy);
  qxbase.l = array_data(symx);
							/*get result sym */
  result_sym = array_clone(symy, toptype);	

  qzbase.l = array_data(result_sym);
			/* get the various matrices and vectors needed */
			/*unlike pit, the coeffs. are not in a symbol */
  allocate(cfbase, npow + 1, double);
  allocate(fbase, nxq, double);
  allocate(a, (npow + 1)*(npow + 1), double);
	/*loop over the number of individual fits to do */
  while (outer--)
		/* get the poly fit coefficients, done in clux_pit */
  { clux_pit(&qxbase, &qybase, nxq, npow, a, cfbase, fbase, toptype, 1);
		/*cfbase has coeffs., use scratch x to get trend */
    clux_poly(cfbase, &qxbase, nxq, npow, &qzbase, toptype);
		/* check if this is actually a detrend call */
    if (detrend_flag)
		/* a detrend means subtracting the fit from the data
			and returning the result rather than the fit */
    { switch (toptype)
      { case LUX_FLOAT:
	  qybase.f -= nxq;
	  qzbase.f -= nxq;
	  nd = nxq;
	  /* note: cannot use *qzbase.f++ = *qbase.f++ - *qzbase.f */
	  /* because the order in which the assignment and the updating */
	  /* of qzbase.f are done is not defined in the C standard. */
	  /* LS 18jun97 */
	  while (nd--) 
	  { *qzbase.f = *qybase.f++ - *qzbase.f;
	    qzbase.f++; }
	  break;
	case LUX_DOUBLE:
	  qybase.d -= nxq;
	  qzbase.d -= nxq;
	  nd = nxq;
	  while (nd--)
	  { *qzbase.d = *qybase.d++ - *qzbase.d;
	    qzbase.d++; }
	  break; }
    }
  }					/*end of while (outer--) loop */
  Free(a);
  Free(fbase);
  Free(cfbase);
  detrend_flag = 0;
  return result_sym;
}
/*------------------------------------------------------------------------- */
int32_t clux_poly(double *cfbase, pointer *qxbase, int32_t nxq, int32_t npow,
	      pointer *qzbase, int32_t toptype)
/* internal routine used by lux_trend, and lux_detrend */
{
double	sum, *cf, xq;
pointer qz,qx;
int32_t	n, m;
cf = cfbase;	qz.l = qzbase->l;  qx.l = qxbase->l;
switch (toptype) {
case 3:
n = nxq;
switch (npow) {
case 0:	while (n--) *qz.f++ = *cfbase;	break;
default:
while (n--) { cf = cfbase + npow;  /*point to last coeff. */
  xq = *qx.f++;		sum = (*cf--) * xq;	m = npow - 1;
  	while (m-- > 0) sum = ( sum + *cf--) * xq;
	*qz.f++ = sum + *cf;	}
}
qzbase->f += nxq; break;
case 4:
n = nxq;
switch (npow) {
case 0:	while (n--) *qz.d++ = *cfbase;	break;
default:
n = nxq;  while (n--) { cf = cfbase + npow;  /*point to last coeff. */
  xq = *qx.d++;		sum = (*cf--) * xq;	m = npow - 1;
  	while (m-- > 0) sum = ( sum + *cf--) * xq;
	*qz.d++ = sum + *cf;	}
}
qzbase->d += nxq; break;
}
return 1;
}
/*------------------------------------------------------------------------- */
int32_t lux_poly(int32_t narg, int32_t ps[])
 /* y = poly(x,c) */
 /* x: data values at which to evaluate the polynomials */
 /* c: coefficients of the polynomial */
{
  int32_t result, ndata, ncoeff, datasym, coeffsym, i;
  uint8_t outtype;
  pointer data, tgt, coeff;

  if (!symbolIsNumerical(ps[0])
      || !isNumericalType(symbol_type(ps[0])))
    return luxerror("Need a real scalar or array", ps[0]);
  if (!symbolIsRealArray(ps[1]))
    return luxerror("Need a real array", ps[1]);
  datasym = (symbol_type(ps[0]) < LUX_FLOAT)? lux_float(1, &ps[0]): ps[0];
  coeffsym = (symbol_type(ps[1]) < LUX_FLOAT)? lux_float(1, &ps[1]): ps[1];
  outtype = combinedType(symbol_type(datasym), array_type(coeffsym));
  if (getNumerical(datasym, outtype, &ndata, &data, GN_UPGRADE, &result, &tgt)
      != LUX_OK)
    return LUX_ERROR;
  coeff.v = array_data(coeffsym);
  ncoeff = array_size(coeffsym);

  switch (symbol_type(datasym)) {
  case LUX_FLOAT:
    switch (symbol_type(coeffsym)) {
    case LUX_FLOAT:
      while (ndata--) {
	*tgt.f = coeff.f[ncoeff - 1];
	for (i = ncoeff - 2; i >= 0; i--)
	  *tgt.f = *tgt.f * *data.f + coeff.f[i];
	tgt.f++;  data.f++;
      }
      break;
    case LUX_DOUBLE:
      while (ndata--) {
	*tgt.d = coeff.d[ncoeff - 1];
	for (i = ncoeff - 2; i >= 0; i--)
	  *tgt.d = *tgt.d * *data.f + coeff.d[i];
	tgt.d++;  data.f++;
      }
      break;
    default:
      assert(0);
    }
    break;
  case LUX_DOUBLE:
    switch (symbol_type(coeffsym)) {
    case LUX_FLOAT:
      while (ndata--) {
	*tgt.d = coeff.f[ncoeff - 1];
	for (i = ncoeff - 2; i >= 0; i--)
	  *tgt.d = *tgt.d * *data.d + coeff.f[i];
	tgt.d++;  data.d++;
      }
      break;
    case LUX_DOUBLE:
      while (ndata--) {
	*tgt.d = coeff.d[ncoeff - 1];
	for (i = ncoeff - 2; i >= 0; i--)
	  *tgt.d = *tgt.d * *data.d + coeff.d[i];
	tgt.d++;  data.d++;
      }
      break;
    default:
      assert(0);
    }
    break;
  default:
    assert(0);
  }
  if (coeffsym != ps[1])
    zap(coeffsym);
  if (datasym != ps[0])
    zap(datasym);
  return result;
}
/*------------------------------------------------------------------------- */
int32_t lux_strtok(int32_t narg, int32_t ps[])
/* mimics the C strtok function.
 STRTOK( [<s1>,] <s2>)
 all specified arguments must be strings.  If <s1> is specified and not
 equal to an empty string then its contents are installed as new string
 to be tokenized.  <s2> is a string containing the characters
 that are token separators in the current call.  The next token in
 the remainder of the currently installed string that is delimited by
 characters from <s2> is returned.  If there were no non-delimiter
 characters left in the remainder of the current string, then the
 remainder of the current string is returned as a token.  If there was no
 more of the current string left, then the current string is discarded
 and an empty string is returned.  If no string is currently installed,
 then an error is generated.  LS 19sep98 */
{
  char	*s1, *s2, *s;
  int32_t	iq;
  static char	*string = NULL;

  if (symbol_class(ps[0]) != LUX_STRING)
    return cerror(NEED_STR, ps[0]);
  if (narg == 2) {
    if (symbol_class(ps[1]) != LUX_STRING)
      return cerror(NEED_STR, ps[1]);
    s1 = string_value(ps[0]);
    if (*s1 == '\0')		/* empty string */
      s1 = NULL;
    s2 = string_value(ps[1]);
  } else { 			/* only one arg */
    s1 = NULL;
    s2 = string_value(ps[0]);
  }

  if (s1) { 			/* install a new string */
    string = realloc(string, strlen(s1) + 1);
    if (!string)
      return cerror(ALLOC_ERR, 0);
    strcpy(string, s1);
    s = strtok(s1,s2);
  } else			/* continue with old one */
    s = strtok(NULL, s2);

  if (s) {			/* have something to return */
    iq = string_scratch(strlen(s));
    if (iq < 0)
      return LUX_ERROR;
    strcpy(string_value(iq), s);
  } else {			/* return an empty string */
    iq = string_scratch(0);
    *string_value(iq) = '\0';
  }
  
  return iq;
}
/*------------------------------------------------------------------------- */
void ksmooth(loopInfo *srcinfo, loopInfo *trgtinfo, float *kernel, int32_t nkernel)
{
  int32_t	nx, n2, dataindex, kernelindex, di, ki, npoints, ncalc, np, i, stride;
  float	sum, norm;
  pointer	src, trgt;

  src.v = srcinfo->data->v;
  trgt.v = trgtinfo->data->v;
  nx = srcinfo->rdims[0];
  n2 = (nkernel - 1)/2;
  stride = srcinfo->rsinglestep[0];
  switch (srcinfo->type) {
    case LUX_BYTE:
      /* zone 1: starts with dataindex 0 */
      if (internalMode & 1) {	/* /BALANCED */
	ncalc = (nx + 1)/2;
	if (ncalc > n2 + 1)
	  ncalc = n2 + 1;
	npoints = 1;	
      } else {
	ncalc = n2 + 1;
	if (ncalc > nx)
	  ncalc = nx;
	npoints = ncalc;
      }
      dataindex = 0;
      kernelindex = n2*stride;
      while (ncalc--) {
	np = npoints;
	sum = 0.0;
	norm = 0;
	di = dataindex;
	ki = kernelindex;
	while (np--) {
	  sum += src.b[di]*kernel[ki];
	  norm += kernel[ki];
	  di += stride;
	  ki += stride;
	}
	*trgt.f = norm? sum/norm: 0;
	trgt.f += stride;
	if (kernelindex)
	  kernelindex -= stride;
	if (internalMode & 1)
	  npoints += 2;
	else if (npoints < nx)
	  npoints++;
      }
      /* zone2: data covers whole kernel */
      if (nx > nkernel) {
	ncalc = nx - nkernel;
	dataindex = stride;
	kernelindex = 0;
	npoints = nkernel;
	norm = 0;
	for (i = 0; i < npoints; i++)
	  norm += kernel[i];
	norm = norm? 1.0/norm: 0;
	while (ncalc--) {
	  np = npoints;
	  di = dataindex;
	  ki = kernelindex;
	  sum = 0.0;
	  while (np--) {
	    sum += src.b[di]*kernel[ki];
	    di += stride;
	    ki += stride;
	  }
	  *trgt.f = sum*norm;
	  trgt.f += stride;
	  dataindex += stride;
	}
      }
      /* zone 3: through last data element */
      if (nx > nkernel - 3) {
	if (internalMode & 1) {
	  ncalc = nx/2;
	  if (ncalc > n2)
	    ncalc = n2;
	  npoints = 2*ncalc - 1;
	  dataindex = (nx - 2*ncalc + 1)*stride;
	  kernelindex = (n2 - ncalc + 1)*stride;
	} else {
	  ncalc = n2;
	  if (ncalc > nx - 4)
	    ncalc = nx - 4;
	  dataindex = nx - nkernel + 1;
	  if (dataindex < 1)
	    dataindex = 1;
	  dataindex *= stride;
	  kernelindex = 0;
	  npoints = nkernel - 1;
	}
	while (ncalc--) {
	  np = npoints;
	  di = dataindex;
	  ki = kernelindex;
	  sum = 0.0;
	  norm = 0.0;
	  while (np--) {
	    sum += src.b[di]*kernel[ki];
	    norm += kernel[ki];
	    di += stride;
	    ki += stride;
	  }
	  *trgt.f = norm? sum/norm: 0;
	  trgt.f += stride;
	  if (internalMode & 1) {
	    dataindex += 2;
	    kernelindex += stride;
	    npoints -= 2;
	  } else {
	    dataindex += stride;
	    npoints--;
	  }
	}
      }
      break;
    case LUX_WORD:
      /* zone 1: starts with dataindex 0 */
      if (internalMode & 1) {	/* /BALANCED */
	ncalc = (nx + 1)/2;
	if (ncalc > n2 + 1)
	  ncalc = n2 + 1;
	npoints = 1;	
      } else {
	ncalc = n2 + 1;
	if (ncalc > nx)
	  ncalc = nx;
	npoints = ncalc;
      }
      dataindex = 0;
      kernelindex = n2*stride;
      while (ncalc--) {
	np = npoints;
	sum = 0.0;
	norm = 0;
	di = dataindex;
	ki = kernelindex;
	while (np--) {
	  sum += src.w[di]*kernel[ki];
	  norm += kernel[ki];
	  di += stride;
	  ki += stride;
	}
	*trgt.f = norm? sum/norm: 0;
	trgt.f += stride;
	if (kernelindex)
	  kernelindex -= stride;
	if (internalMode & 1)
	  npoints += 2;
	else if (npoints < nx)
	  npoints++;
      }
      /* zone2: data covers whole kernel */
      if (nx > nkernel) {
	ncalc = nx - nkernel;
	dataindex = stride;
	kernelindex = 0;
	npoints = nkernel;
	norm = 0;
	for (i = 0; i < npoints; i++)
	  norm += kernel[i];
	norm = norm? 1.0/norm: 0;
	while (ncalc--) {
	  np = npoints;
	  di = dataindex;
	  ki = kernelindex;
	  sum = 0.0;
	  while (np--) {
	    sum += src.w[di]*kernel[ki];
	    di += stride;
	    ki += stride;
	  }
	  *trgt.f = sum*norm;
	  trgt.f += stride;
	  dataindex += stride;
	}
      }
      /* zone 3: through last data element */
      if (nx > nkernel - 3) {
	if (internalMode & 1) {
	  ncalc = nx/2;
	  if (ncalc > n2)
	    ncalc = n2;
	  npoints = 2*ncalc - 1;
	  dataindex = (nx - 2*ncalc + 1)*stride;
	  kernelindex = (n2 - ncalc + 1)*stride;
	} else {
	  ncalc = n2;
	  if (ncalc > nx - 4)
	    ncalc = nx - 4;
	  dataindex = nx - nkernel + 1;
	  if (dataindex < 1)
	    dataindex = 1;
	  dataindex *= stride;
	  kernelindex = 0;
	  npoints = nkernel - 1;
	}
	while (ncalc--) {
	  np = npoints;
	  di = dataindex;
	  ki = kernelindex;
	  sum = 0.0;
	  norm = 0.0;
	  while (np--) {
	    sum += src.w[di]*kernel[ki];
	    norm += kernel[ki];
	    di += stride;
	    ki += stride;
	  }
	  *trgt.f = norm? sum/norm: 0;
	  trgt.f += stride;
	  if (internalMode & 1) {
	    dataindex += 2;
	    kernelindex += stride;
	    npoints -= 2;
	  } else {
	    dataindex += stride;
	    npoints--;
	  }
	}
      }
      break;
    case LUX_LONG:
      /* zone 1: starts with dataindex 0 */
      if (internalMode & 1) {	/* /BALANCED */
	ncalc = (nx + 1)/2;
	if (ncalc > n2 + 1)
	  ncalc = n2 + 1;
	npoints = 1;	
      } else {
	ncalc = n2 + 1;
	if (ncalc > nx)
	  ncalc = nx;
	npoints = ncalc;
      }
      dataindex = 0;
      kernelindex = n2*stride;
      while (ncalc--) {
	np = npoints;
	sum = 0.0;
	norm = 0;
	di = dataindex;
	ki = kernelindex;
	while (np--) {
	  sum += src.l[di]*kernel[ki];
	  norm += kernel[ki];
	  di += stride;
	  ki += stride;
	}
	*trgt.f = norm? sum/norm: 0;
	trgt.f += stride;
	if (kernelindex)
	  kernelindex -= stride;
	if (internalMode & 1)
	  npoints += 2;
	else if (npoints < nx)
	  npoints++;
      }
      /* zone2: data covers whole kernel */
      if (nx > nkernel) {
	ncalc = nx - nkernel;
	dataindex = stride;
	kernelindex = 0;
	npoints = nkernel;
	norm = 0;
	for (i = 0; i < npoints; i++)
	  norm += kernel[i];
	norm = norm? 1.0/norm: 0;
	while (ncalc--) {
	  np = npoints;
	  di = dataindex;
	  ki = kernelindex;
	  sum = 0.0;
	  while (np--) {
	    sum += src.l[di]*kernel[ki];
	    di += stride;
	    ki += stride;
	  }
	  *trgt.f = sum*norm;
	  trgt.f += stride;
	  dataindex += stride;
	}
      }
      /* zone 3: through last data element */
      if (nx > nkernel - 3) {
	if (internalMode & 1) {
	  ncalc = nx/2;
	  if (ncalc > n2)
	    ncalc = n2;
	  npoints = 2*ncalc - 1;
	  dataindex = (nx - 2*ncalc + 1)*stride;
	  kernelindex = (n2 - ncalc + 1)*stride;
	} else {
	  ncalc = n2;
	  if (ncalc > nx - 4)
	    ncalc = nx - 4;
	  dataindex = nx - nkernel + 1;
	  if (dataindex < 1)
	    dataindex = 1;
	  dataindex *= stride;
	  kernelindex = 0;
	  npoints = nkernel - 1;
	}
	while (ncalc--) {
	  np = npoints;
	  di = dataindex;
	  ki = kernelindex;
	  sum = 0.0;
	  norm = 0.0;
	  while (np--) {
	    sum += src.l[di]*kernel[ki];
	    norm += kernel[ki];
	    di += stride;
	    ki += stride;
	  }
	  *trgt.f = norm? sum/norm: 0;
	  trgt.f += stride;
	  if (internalMode & 1) {
	    dataindex += 2;
	    kernelindex += stride;
	    npoints -= 2;
	  } else {
	    dataindex += stride;
	    npoints--;
	  }
	}
      }
      break;
    case LUX_FLOAT:
      /* zone 1: starts with dataindex 0 */
      if (internalMode & 1) {	/* /BALANCED */
	ncalc = (nx + 1)/2;
	if (ncalc > n2 + 1)
	  ncalc = n2 + 1;
	npoints = 1;	
      } else {
	ncalc = n2 + 1;
	if (ncalc > nx)
	  ncalc = nx;
	npoints = ncalc;
      }
      dataindex = 0;
      kernelindex = n2*stride;
      while (ncalc--) {
	np = npoints;
	sum = 0.0;
	norm = 0;
	di = dataindex;
	ki = kernelindex;
	while (np--) {
	  sum += src.f[di]*kernel[ki];
	  norm += kernel[ki];
	  di += stride;
	  ki += stride;
	}
	*trgt.f = norm? sum/norm: 0;
	trgt.f += stride;
	if (kernelindex)
	  kernelindex -= stride;
	if (internalMode & 1)
	  npoints += 2;
	else if (npoints < nx)
	  npoints++;
      }
      /* zone2: data covers whole kernel */
      if (nx > nkernel) {
	ncalc = nx - nkernel;
	dataindex = stride;
	kernelindex = 0;
	npoints = nkernel;
	norm = 0;
	for (i = 0; i < npoints; i++)
	  norm += kernel[i];
	norm = norm? 1.0/norm: 0;
	while (ncalc--) {
	  np = npoints;
	  di = dataindex;
	  ki = kernelindex;
	  sum = 0.0;
	  while (np--) {
	    sum += src.f[di]*kernel[ki];
	    di += stride;
	    ki += stride;
	  }
	  *trgt.f = sum*norm;
	  trgt.f += stride;
	  dataindex += stride;
	}
      }
      /* zone 3: through last data element */
      if (nx > nkernel - 3) {
	if (internalMode & 1) {
	  ncalc = nx/2;
	  if (ncalc > n2)
	    ncalc = n2;
	  npoints = 2*ncalc - 1;
	  dataindex = (nx - 2*ncalc + 1)*stride;
	  kernelindex = (n2 - ncalc + 1)*stride;
	} else {
	  ncalc = n2;
	  if (ncalc > nx - 4)
	    ncalc = nx - 4;
	  dataindex = nx - nkernel + 1;
	  if (dataindex < 1)
	    dataindex = 1;
	  dataindex *= stride;
	  kernelindex = 0;
	  npoints = nkernel - 1;
	}
	while (ncalc--) {
	  np = npoints;
	  di = dataindex;
	  ki = kernelindex;
	  sum = 0.0;
	  norm = 0.0;
	  while (np--) {
	    sum += src.f[di]*kernel[ki];
	    norm += kernel[ki];
	    di += stride;
	    ki += stride;
	  }
	  *trgt.f = norm? sum/norm: 0;
	  trgt.f += stride;
	  if (internalMode & 1) {
	    dataindex += 2;
	    kernelindex += stride;
	    npoints -= 2;
	  } else {
	    dataindex += stride;
	    npoints--;
	  }
	}
      }
      break;
    case LUX_DOUBLE:
      /* zone 1: starts with dataindex 0 */
      if (internalMode & 1) {	/* /BALANCED */
	ncalc = (nx + 1)/2;
	if (ncalc > n2 + 1)
	  ncalc = n2 + 1;
	npoints = 1;	
      } else {
	ncalc = n2 + 1;
	if (ncalc > nx)
	  ncalc = nx;
	npoints = ncalc;
      }
      dataindex = 0;
      kernelindex = n2*stride;
      while (ncalc--) {
	np = npoints;
	sum = 0.0;
	norm = 0;
	di = dataindex;
	ki = kernelindex;
	while (np--) {
	  sum += src.d[di]*kernel[ki];
	  norm += kernel[ki];
	  di += stride;
	  ki += stride;
	}
	*trgt.f = norm? sum/norm: 0;
	trgt.f += stride;
	if (kernelindex)
	  kernelindex -= stride;
	if (internalMode & 1)
	  npoints += 2;
	else if (npoints < nx)
	  npoints++;
      }
      /* zone2: data covers whole kernel */
      if (nx > nkernel) {
	ncalc = nx - nkernel;
	dataindex = stride;
	kernelindex = 0;
	npoints = nkernel;
	norm = 0;
	for (i = 0; i < npoints; i++)
	  norm += kernel[i];
	norm = norm? 1.0/norm: 0;
	while (ncalc--) {
	  np = npoints;
	  di = dataindex;
	  ki = kernelindex;
	  sum = 0.0;
	  while (np--) {
	    sum += src.d[di]*kernel[ki];
	    di += stride;
	    ki += stride;
	  }
	  *trgt.f = sum*norm;
	  trgt.f += stride;
	  dataindex += stride;
	}
      }
      /* zone 3: through last data element */
      if (nx > nkernel - 3) {
	if (internalMode & 1) {
	  ncalc = nx/2;
	  if (ncalc > n2)
	    ncalc = n2;
	  npoints = 2*ncalc - 1;
	  dataindex = (nx - 2*ncalc + 1)*stride;
	  kernelindex = (n2 - ncalc + 1)*stride;
	} else {
	  ncalc = n2;
	  if (ncalc > nx - 4)
	    ncalc = nx - 4;
	  dataindex = nx - nkernel + 1;
	  if (dataindex < 1)
	    dataindex = 1;
	  dataindex *= stride;
	  kernelindex = 0;
	  npoints = nkernel - 1;
	}
	while (ncalc--) {
	  np = npoints;
	  di = dataindex;
	  ki = kernelindex;
	  sum = 0.0;
	  norm = 0.0;
	  while (np--) {
	    sum += src.d[di]*kernel[ki];
	    norm += kernel[ki];
	    di += stride;
	    ki += stride;
	  }
	  *trgt.f = norm? sum/norm: 0;
	  trgt.f += stride;
	  if (internalMode & 1) {
	    dataindex += 2;
	    kernelindex += stride;
	    npoints -= 2;
	  } else {
	    dataindex += stride;
	    npoints--;
	  }
	}
      }
      break;
  default:
    break;
  }
  src.b += nx*stride*srcinfo->stride;
  srcinfo->data->v = src.v;
  trgtinfo->data->v = trgt.v;
}
/*------------------------------------------------------------------------- */
int32_t lux_ksmooth(int32_t narg, int32_t ps[])
/* y = KSMOOTH(x, kernel [, axis] [, /BALANCED]) */
{
  loopInfo	srcinfo, trgtinfo;
  pointer	src, trgt;
  int32_t	result, iq, nkernel;
  float	*kernel;

  if (!symbolIsNumericalArray(ps[1])) /* <kernel> must be a numerical array */
    return cerror(NEED_NUM_ARR, ps[1]);
  nkernel = array_size(ps[1]);
  if ((nkernel % 2) == 0)
    nkernel--;			/* we require an odd number of elements in
				   the kernel, so we ignore the last one */
  if (!nkernel)
    return luxerror("Need at least 2 elements in the kernel", ps[1]);
    
  iq = lux_float(1, ps + 1);	/* make sure <kernel> is FLOAT */
  kernel = array_data(iq);

  if (standardLoop(ps[0], narg > 2? ps[2]: 0,
		   SL_ONEAXIS | SL_SAMEDIMS | SL_EACHROW,
		   LUX_FLOAT, &srcinfo, &src, &result, &trgtinfo, &trgt) < 0)
    return LUX_ERROR;

  do {
    do {
      ksmooth(&srcinfo, &trgtinfo, kernel, nkernel);
    } while (advanceLoop(&trgtinfo, &trgt), 
	     advanceLoop(&srcinfo, &src) < srcinfo.rndim);
  } while (nextLoops(&srcinfo, &trgtinfo));

  return result;
}
/*------------------------------------------------------------------------- */
int32_t lux_crosscorr(int32_t narg, int32_t ps[])
/* y = CROSSCORR(x1, x2 [,mode]) */
/* returns the cross-correlation coefficient between <x1> and <x2>, */
/* along the axis indicated by <mode> (if scalar), or as a function of */
/* class as specified in <mode> (if an array of the same size as <x1> and */
/* <x2>).  LS 8apr99 */
{
  int32_t	type, outtype, i, iq1, iq2, result, n, save[MAX_DIMS], done;
  double	meanx, meany, kx, ky, pxy, tempx, tempy;
  doubleComplex	cmeanx, cmeany, cpxy, ctempx, ctempy;
  pointer	src1, src2, src1save, src2save, trgt;
  loopInfo	srcinfo1, srcinfo2, trgtinfo;

  if (!symbolIsNumericalArray(ps[0]))
    return cerror(NEED_NUM_ARR, ps[0]);
  if (!symbolIsNumericalArray(ps[1]))
    return cerror(NEED_NUM_ARR, ps[1]);

  if (array_size(ps[0]) != array_size(ps[1])
      || array_num_dims(ps[0]) != array_num_dims(ps[1]))
    return cerror(INCMP_ARG, ps[1]);

  for (i = 0; i < array_num_dims(ps[0]); i++)
    if (array_dims(ps[0])[i] != array_dims(ps[1])[i])
      return cerror(INCMP_ARG, ps[1]);

  type = combinedType(symbol_type(ps[0]), symbol_type(ps[1]));
  if (isComplexType(type))
    outtype = type;
  else
    outtype = (type == LUX_DOUBLE)? LUX_DOUBLE: LUX_FLOAT;
  
  iq1 = (array_type(ps[0]) == type)? ps[0]: lux_converts[type](1, ps);
  iq2 = (array_type(ps[1]) == type)? ps[1]: lux_converts[type](1, ps + 1);

  if (standardLoop(iq1, narg > 2? ps[2]: 0,
		   (ps[2]? 0: SL_ALLAXES) | SL_COMPRESSALL | SL_EXACT
		   | SL_EACHCOORD | SL_UNIQUEAXES | SL_AXESBLOCK
		   | ((internalMode & 1)? SL_ONEDIMS: 0),
		   outtype, &srcinfo1, &src1, &result, &trgtinfo, &trgt) < 0
      || standardLoop(iq2, narg > 2? ps[2]: 0,
		      (ps[2]? 0: SL_ALLAXES) | SL_COMPRESSALL | SL_EXACT
		      | SL_EACHCOORD | SL_UNIQUEAXES | SL_AXESBLOCK
		      | ((internalMode & 1)? SL_ONEDIMS: 0),
		      outtype, &srcinfo2, &src2, NULL, NULL, NULL) < 0)
    return LUX_ERROR;

  if (!srcinfo1.naxes)
    srcinfo1.naxes++;

  n = 1;			/* initialize # values per calculation */
  for (i = 0; i < srcinfo1.naxes; i++)
    n *= srcinfo1.rdims[i];

  switch (symbol_type(ps[0])) {
    case LUX_BYTE:
      do {
	meanx = meany = 0.0;
	memcpy(save, srcinfo1.coords, srcinfo1.ndim*sizeof(int32_t));
	src1save = src1;
	src2save = src2;
	do {
	  meanx += (double) *src1.b;
	  meany += (double) *src2.b;
	} while (advanceLoop(&srcinfo2, &src2),
		 advanceLoop(&srcinfo1, &src1) < srcinfo1.naxes);
	memcpy(srcinfo1.coords, save, srcinfo1.ndim*sizeof(int32_t));
	src1 = src1save;
	src2 = src2save;
	meanx /= n;
	meany /= n;
	pxy = kx = ky = 0.0;
	do {
	  tempx = ((double) *src1.b - meanx);
	  tempy = ((double) *src2.b - meany);
	  kx += tempx*tempx;
	  ky += tempy*tempy;
	  pxy += tempx*tempy;
	} while ((done = (advanceLoop(&srcinfo2, &src2),
			  advanceLoop(&srcinfo1, &src1))) < srcinfo1.naxes);
	tempx = kx*ky;
	*trgt.f++ = tempx? pxy/sqrt(tempx): 0.0;
      } while (done < srcinfo1.rndim);
      break;
    case LUX_WORD:
      do {
	meanx = meany = 0.0;
	memcpy(save, srcinfo1.coords, srcinfo1.ndim*sizeof(int32_t));
	src1save = src1;
	src2save = src2;
	do {
	  meanx += (double) *src1.w;
	  meany += (double) *src2.w;
	} while (advanceLoop(&srcinfo2, &src2),
		 advanceLoop(&srcinfo1, &src1) < srcinfo1.naxes);
	memcpy(srcinfo1.coords, save, srcinfo1.ndim*sizeof(int32_t));
	src1 = src1save;
	src2 = src2save;
	meanx /= n;
	meany /= n;
	pxy = kx = ky = 0.0;
	do {
	  tempx = ((double) *src1.w - meanx);
	  tempy = ((double) *src2.w - meany);
	  kx += tempx*tempx;
	  ky += tempy*tempy;
	  pxy += tempx*tempy;
	} while ((done = (advanceLoop(&srcinfo2, &src2),
			  advanceLoop(&srcinfo1, &src1))) < srcinfo1.naxes);
	tempx = kx*ky;
	*trgt.f++ = tempx? pxy/sqrt(tempx): 0.0;
      } while (done < srcinfo1.rndim);
      break;
    case LUX_LONG:
      do {
	meanx = meany = 0.0;
	memcpy(save, srcinfo1.coords, srcinfo1.ndim*sizeof(int32_t));
	src1save = src1;
	src2save = src2;
	do {
	  meanx += (double) *src1.l;
	  meany += (double) *src2.l;
	} while (advanceLoop(&srcinfo2, &src2),
		 advanceLoop(&srcinfo1, &src1) < srcinfo1.naxes);
	memcpy(srcinfo1.coords, save, srcinfo1.ndim*sizeof(int32_t));
	src1 = src1save;
	src2 = src2save;
	meanx /= n;
	meany /= n;
	pxy = kx = ky = 0.0;
	do {
	  tempx = ((double) *src1.l - meanx);
	  tempy = ((double) *src2.l - meany);
	  kx += tempx*tempx;
	  ky += tempy*tempy;
	  pxy += tempx*tempy;
	} while ((done = (advanceLoop(&srcinfo2, &src2),
			  advanceLoop(&srcinfo1, &src1))) < srcinfo1.naxes);
	tempx = kx*ky;
	*trgt.f++ = tempx? pxy/sqrt(tempx): 0.0;
      } while (done < srcinfo1.rndim);
      break;
    case LUX_FLOAT:
      do {
	meanx = meany = 0.0;
	memcpy(save, srcinfo1.coords, srcinfo1.ndim*sizeof(int32_t));
	src1save = src1;
	src2save = src2;
	do {
	  meanx += (double) *src1.f;
	  meany += (double) *src2.f;
	} while (advanceLoop(&srcinfo2, &src2),
		 advanceLoop(&srcinfo1, &src1) < srcinfo1.naxes);
	memcpy(srcinfo1.coords, save, srcinfo1.ndim*sizeof(int32_t));
	src1 = src1save;
	src2 = src2save;
	meanx /= n;
	meany /= n;
	pxy = kx = ky = 0.0;
	do {
	  tempx = ((double) *src1.f - meanx);
	  tempy = ((double) *src2.f - meany);
	  kx += tempx*tempx;
	  ky += tempy*tempy;
	  pxy += tempx*tempy;
	} while ((done = (advanceLoop(&srcinfo2, &src2),
			  advanceLoop(&srcinfo1, &src1))) < srcinfo1.naxes);
	tempx = kx*ky;
	*trgt.f++ = tempx? pxy/sqrt(tempx): 0.0;
      } while (done < srcinfo1.rndim);
      break;
    case LUX_DOUBLE:
      do {
	meanx = meany = 0.0;
	memcpy(save, srcinfo1.coords, srcinfo1.ndim*sizeof(int32_t));
	src1save = src1;
	src2save = src2;
	do {
	  meanx += (double) *src1.d;
	  meany += (double) *src2.d;
	} while (advanceLoop(&srcinfo2, &src2),
		 advanceLoop(&srcinfo1, &src1) < srcinfo1.naxes);
	memcpy(srcinfo1.coords, save, srcinfo1.ndim*sizeof(int32_t));
	src1 = src1save;
	src2 = src2save;
	meanx /= n;
	meany /= n;
	pxy = kx = ky = 0.0;
	do {
	  tempx = ((double) *src1.d - meanx);
	  tempy = ((double) *src2.d - meany);
	  kx += tempx*tempx;
	  ky += tempy*tempy;
	  pxy += tempx*tempy;
	} while ((done = (advanceLoop(&srcinfo2, &src2),
			  advanceLoop(&srcinfo1, &src1))) < srcinfo1.naxes);
	tempx = kx*ky;
	*trgt.d++ = tempx? pxy/sqrt(tempx): 0.0;
      } while (done < srcinfo1.rndim);
      break;
    case LUX_CFLOAT:
      do {
	cmeanx.real = cmeanx.imaginary = cmeany.real = cmeany.imaginary = 0.0;
	memcpy(save, srcinfo1.coords, srcinfo1.ndim*sizeof(int32_t));
	src1save = src1;
	src2save = src2;
	do {
	  cmeanx.real += (double) src1.cf->real;
	  cmeanx.imaginary += (double) src1.cf->imaginary;
	  cmeany.real += (double) src2.cf->real;
	  cmeany.imaginary += (double) src2.cf->imaginary;
	} while (advanceLoop(&srcinfo2, &src2),
		 advanceLoop(&srcinfo1, &src1) < srcinfo1.naxes);
	memcpy(srcinfo1.coords, save, srcinfo1.ndim*sizeof(int32_t));
	src1 = src1save;
	src2 = src2save;
	cmeanx.real /= n;
	cmeanx.imaginary /= n;
	cmeany.real /= n;
	cmeany.imaginary /= n;
	cpxy.real = cpxy.imaginary = kx = ky = 0.0;
	do {
	  ctempx.real = ((double) src1.cf->real - cmeanx.real);
	  ctempy.real = ((double) src2.cf->real - cmeany.real);
	  ctempx.imaginary = ((double) src1.cf->imaginary - cmeanx.imaginary);
	  ctempy.imaginary = ((double) src2.cf->imaginary - cmeany.imaginary);
	  kx += ctempx.real*ctempx.real + ctempx.imaginary*ctempx.imaginary;
	  ky += ctempy.real*ctempy.real + ctempy.imaginary*ctempy.imaginary;
	  cpxy.real += ctempx.real*ctempy.real
	    + ctempx.imaginary*ctempy.imaginary;
	  cpxy.imaginary += ctempx.imaginary*ctempy.real
	    - ctempx.real*ctempy.imaginary;
	} while ((done = (advanceLoop(&srcinfo2, &src2),
			  advanceLoop(&srcinfo1, &src1))) < srcinfo1.naxes);
	tempx = kx*ky;
	if (tempx) {
	  tempx = sqrt(tempx);
	  trgt.cf->real = cpxy.real/tempx;
	  trgt.cf->imaginary = cpxy.imaginary/tempx;
	} else
	  trgt.cf->real = trgt.cf->imaginary = 0.0;
	trgt.cf++;
      } while (done < srcinfo1.rndim);
      break;
    case LUX_CDOUBLE:
      do {
	cmeanx.real = cmeanx.imaginary = cmeany.real = cmeany.imaginary = 0.0;
	memcpy(save, srcinfo1.coords, srcinfo1.ndim*sizeof(int32_t));
	src1save = src1;
	src2save = src2;
	do {
	  cmeanx.real += (double) src1.cd->real;
	  cmeanx.imaginary += (double) src1.cd->imaginary;
	  cmeany.real += (double) src2.cd->real;
	  cmeany.imaginary += (double) src2.cd->imaginary;
	} while (advanceLoop(&srcinfo2, &src2),
		 advanceLoop(&srcinfo1, &src1) < srcinfo1.naxes);
	memcpy(srcinfo1.coords, save, srcinfo1.ndim*sizeof(int32_t));
	src1 = src1save;
	src2 = src2save;
	cmeanx.real /= n;
	cmeanx.imaginary /= n;
	cmeany.real /= n;
	cmeany.imaginary /= n;
	cpxy.real = cpxy.imaginary = kx = ky = 0.0;
	do {
	  ctempx.real = ((double) src1.cd->real - cmeanx.real);
	  ctempy.real = ((double) src2.cd->real - cmeany.real);
	  ctempx.imaginary = ((double) src1.cd->imaginary - cmeanx.imaginary);
	  ctempy.imaginary = ((double) src2.cd->imaginary - cmeany.imaginary);
	  kx += ctempx.real*ctempx.real + ctempx.imaginary*ctempx.imaginary;
	  ky += ctempy.real*ctempy.real + ctempy.imaginary*ctempy.imaginary;
	  cpxy.real += ctempx.real*ctempy.real
	    + ctempx.imaginary*ctempy.imaginary;
	  cpxy.imaginary += ctempx.imaginary*ctempy.real
	    - ctempx.real*ctempy.imaginary;
	} while ((done = (advanceLoop(&srcinfo2, &src2),
			  advanceLoop(&srcinfo1, &src1))) < srcinfo1.naxes);
	tempx = kx*ky;
	if (tempx) {
	  tempx = sqrt(tempx);
	  trgt.cd->real = cpxy.real/tempx;
	  trgt.cd->imaginary = cpxy.imaginary/tempx;
	} else
	  trgt.cd->real = trgt.cd->imaginary = 0.0;
	trgt.cd++;
      } while (done < srcinfo1.rndim);
      break;
  }
  return result;
}
/*------------------------------------------------------------------------- */
void wait_sec(float xq) /* for internal use */
{
  struct timeval	tval;

  xq = MAX(0.0, xq);
  tval.tv_sec = (uint32_t) xq;
  xq = (xq - tval.tv_sec) * 1.e6;
  tval.tv_usec = (uint32_t) xq;
  select(0, NULL, NULL, NULL,&tval);
}
/*------------------------------------------------------------------------- */
