/* File decomp.c */
/* ANA routines for LU decomposition and solution of\n sets of
   linear equations. */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include "action.h"
static char rcsid[] __attribute__ ((unused)) =
 "$Id: decomp.c,v 4.0 2001/02/07 20:36:58 strous Exp $";
/*--------------------------------------------------------------------------*/
Int d_decomp(x, n, nd)
/*translated from fortran anadecomp.for which may be easier to follow since
	it uses subscripts rather than the pointers used here */
/*no pivoting in this version !, so diagonals must be != 0 */
Double	*x;
Int	n, nd;
{
register	Double	sum1, sum2, *p1, *p2, *p3, *p4;
Double	*qd, *q2, *q1, div;
Int	nq, mq, lq, k;
p1 = x;
div = 1.0 / *p1; nq = n-1; while (nq--) { p1 += nd; *p1 *= div; }
nq = n-1;	k = 1;		qd = x + 1 + nd; /*qd is diagonal ptr */
while (nq--) {						/*outer loop */
	/*diagonal term */
	sum1 = *qd;  mq = k;  p1 = p2 = qd;
	while (mq--) { p1 -= nd; p2--; sum1 -= (*p1) * (*p2); }
	*qd = sum1;
	q1 = q2 = qd; lq = n - k - 1;
	while (lq--) { q1 += nd;  q2 += 1;  mq = k;
	 sum1 = *q1;	sum2 = *q2;
	 p1 = p4 = qd; p2 = q1; p3 = q2;
	 while (mq--) { p1 -= nd; p2 -= 1; p3 -= nd; p4 -= 1; /*inner */
	 sum1 -= (*p1) * (*p2);  sum2 -= (*p3) * (*p4); }     /*end of */
	*q2 = sum2;  *q1 = sum1 / *qd;
	}					/*end of middle loop */
k++; qd += (nd + 1); }				/*end of outer loop */
return 1;			/* added LS 5feb97 */
}
/*--------------------------------------------------------------------------*/
Int f_decomp(Float *x, Int n, Int nd)
/*translated from fortran anadecomp.for which may be easier to follow since
	it uses subscripts rather than the pointers used here */
/*no pivoting in this version !, so diagonals must be != 0 */
{
  register	Float	sum1, sum2, *p1, *p2, *p3, *p4;
  Float		*qd, * q2, *q1, div;
  Int		nq, mq, lq, k;

  p1 = x;
  div = 1.0/ *p1;
  nq = n-1;
  while (nq--)
  { p1 += nd; *p1 *= div; }
  nq = n-1;	k = 1;		qd = x + 1 + nd; /*qd is diagonal ptr */
  while (nq--)			/*outer loop */
	/*diagonal term */
  { sum1 = *qd;  mq = k;  p1 = p2 = qd;
    while (mq--) { p1 -= nd; p2--; sum1 -= (*p1) * (*p2); }
    *qd = sum1;
    q1 = q2 = qd; lq = n - k - 1;
    while (lq--)
    { q1 += nd;  q2 += 1;  mq = k;
      sum1 = *q1;	sum2 = *q2;
      p1 = p4 = qd; p2 = q1; p3 = q2;
      while (mq--)
      { p1 -= nd; p2 -= 1; p3 -= nd; p4 -= 1; /*inner */
	sum1 -= (*p1) * (*p2);  sum2 -= (*p3) * (*p4); }     /*end of */
      *q2 = sum2;  *q1 = sum1 / *qd;
    }					/*end of middle loop */
    k++; qd += (nd + 1); }				/*end of outer loop */
return 1;			/* added LS 5feb97 */
}
/*--------------------------------------------------------------------------*/
Int f_solve(Float *a, Float *b, Int n, Int nd)
{
  register	Float	sum, *p1, *p2;
  Float		*qd, *q1;
  Int		nq, mq, k;

/*printf("in f_solve\n");*/
  p1 = a;	q1 = b;
  *q1++ /= *p1;		      /* changed from *q1++ = *q1 / *p1;  LS26jan95*/
  nq = n-1;	k = 1;		qd = a + 1 + nd; /*qd is diagonal ptr */
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
return 1;			/* added LS 5feb97 */
}
/*--------------------------------------------------------------------------*/
Int d_solve(a, b, n, nd)
Double	*a,*b;
Int	n, nd;
{
register	Double	sum, *p1, *p2;
Double	*qd, *q1;
Int	nq, mq, k;
/*printf("in d_solve\n");*/
p1 = a;	q1 = b; *q1++ /= *p1;
nq = n-1;	k = 1;		qd = a + 1 + nd; /*qd is diagonal ptr */
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
return 1;			/* added LS 5feb97 */
}
/*--------------------------------------------------------------------------*/
