/* This is file decomp.cc.

Copyright 2013 Louis Strous, Richard Shine
Copyright 2014 Louis Strous

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
// File decomp.c
/* LUX routines for LU decomposition and solution of\n sets of
   linear equations. */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include "action.hh"
//--------------------------------------------------------------------------
int32_t d_decomp(double* x, int32_t n, int32_t nd)
/*translated from fortran anadecomp.for which may be easier to follow since
	it uses subscripts rather than the pointers used here */
//no pivoting in this version !, so diagonals must be != 0
{
register	double	sum1, sum2, *p1, *p2, *p3, *p4;
double	*qd, *q2, *q1, div;
int32_t	nq, mq, lq, k;
p1 = x;
div = 1.0 / *p1; nq = n-1; while (nq--) { p1 += nd; *p1 *= div; }
nq = n-1;	k = 1;		qd = x + 1 + nd; //qd is diagonal ptr
while (nq--) {						//outer loop
	//diagonal term
	sum1 = *qd;  mq = k;  p1 = p2 = qd;
	while (mq--) { p1 -= nd; p2--; sum1 -= (*p1) * (*p2); }
	*qd = sum1;
	q1 = q2 = qd; lq = n - k - 1;
	while (lq--) { q1 += nd;  q2 += 1;  mq = k;
	 sum1 = *q1;	sum2 = *q2;
	 p1 = p4 = qd; p2 = q1; p3 = q2;
	 while (mq--) { p1 -= nd; p2 -= 1; p3 -= nd; p4 -= 1; //inner
	 sum1 -= (*p1) * (*p2);  sum2 -= (*p3) * (*p4); }     //end of
	*q2 = sum2;  *q1 = sum1 / *qd;
	}					//end of middle loop
k++; qd += (nd + 1); }				//end of outer loop
return 1;			// added LS 5feb97
}
//--------------------------------------------------------------------------
int32_t f_decomp(float *x, int32_t n, int32_t nd)
/*translated from fortran anadecomp.for which may be easier to follow since
	it uses subscripts rather than the pointers used here */
//no pivoting in this version !, so diagonals must be != 0
{
  register	float	sum1, sum2, *p1, *p2, *p3, *p4;
  float		*qd, * q2, *q1, div;
  int32_t		nq, mq, lq, k;

  p1 = x;
  div = 1.0/ *p1;
  nq = n-1;
  while (nq--)
  { p1 += nd; *p1 *= div; }
  nq = n-1;	k = 1;		qd = x + 1 + nd; //qd is diagonal ptr
  while (nq--)			//outer loop
	//diagonal term
  { sum1 = *qd;  mq = k;  p1 = p2 = qd;
    while (mq--) { p1 -= nd; p2--; sum1 -= (*p1) * (*p2); }
    *qd = sum1;
    q1 = q2 = qd; lq = n - k - 1;
    while (lq--)
    { q1 += nd;  q2 += 1;  mq = k;
      sum1 = *q1;	sum2 = *q2;
      p1 = p4 = qd; p2 = q1; p3 = q2;
      while (mq--)
      { p1 -= nd; p2 -= 1; p3 -= nd; p4 -= 1; //inner
	sum1 -= (*p1) * (*p2);  sum2 -= (*p3) * (*p4); }     //end of
      *q2 = sum2;  *q1 = sum1 / *qd;
    }					//end of middle loop
    k++; qd += (nd + 1); }				//end of outer loop
return 1;			// added LS 5feb97
}
//--------------------------------------------------------------------------
int32_t f_solve(float *a, float *b, int32_t n, int32_t nd)
{
  register	float	sum, *p1, *p2;
  float		*qd, *q1;
  int32_t		nq, mq, k;

//printf("in f_solve\n");
  p1 = a;	q1 = b;
  *q1++ /= *p1;		      // changed from *q1++ = *q1 / *p1;  LS26jan95
  nq = n-1;	k = 1;		qd = a + 1 + nd; //qd is diagonal ptr
  while (nq--)
  { sum = 0.0;  mq = k;
    p1 = qd;	p2 = q1;
    while (mq--) { p1 -= nd; p2--; sum += (*p1) * (*p2); }
    *q1 = (*q1 - sum) / *qd;
    k++; qd += (nd + 1); q1++; }	
  nq = n-1;	k = n - 1;	qd = a + nd*nd -2;  q1 = b + n - 2;
  while (nq--)
  { sum = 0.0;  mq = n - k;
    p1 = qd;	p2 = q1;
    while (mq--) { p2++; sum += (*p1) * (*p2); p1 += nd; }
    *q1 = *q1 - sum;
    k--; qd -= (nd + 1); q1--; }	
return 1;			// added LS 5feb97
}
//--------------------------------------------------------------------------
int32_t d_solve(double* a, double* b, int32_t n, int32_t nd)
{
register	double	sum, *p1, *p2;
double	*qd, *q1;
int32_t	nq, mq, k;
//printf("in d_solve\n");
p1 = a;	q1 = b; *q1++ /= *p1;
nq = n-1;	k = 1;		qd = a + 1 + nd; //qd is diagonal ptr
while (nq--) { sum = 0.0;  mq = k;
	p1 = qd;	p2 = q1;
	while (mq--) { p1 -= nd; p2--; sum += (*p1) * (*p2); }
	*q1 = (*q1 - sum) / *qd;
k++; qd += (nd + 1); q1++; }	
nq = n-1;	k = n - 1;	qd = a + nd*nd -2;  q1 = b + n - 2;
while (nq--) { sum = 0.0;  mq = n - k;
	p1 = qd;	p2 = q1;
	while (mq--) { p2++; sum += (*p1) * (*p2); p1 += nd; }
	*q1 = *q1 - sum;
k--; qd -= (nd + 1); q1--; }	
return 1;			// added LS 5feb97
}
//--------------------------------------------------------------------------
