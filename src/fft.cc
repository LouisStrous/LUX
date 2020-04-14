/* This is file fft.cc.

Copyright 2013-2014 Louis Strous

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
// LUX's FFT routines
// Based on FFTPACK version 4 by Paul N. Swarztrauber (April 1985)
// Copied from the Netlib CDROM (2nd edition)
/* Translated from fortran to C using f2c + manual changes
   by Louis Strous (15 June 1998) */

// FFTPACK intro (describing the fortran version; We do not use all of
// the routines in FFTPACK, and we only include the description of
// the ones we use):

/*

                      FFTPACK

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

                  version 4  april 1985

     a package of fortran subprograms for the fast fourier
      transform of periodic and other symmetric sequences

                         by

                  paul n swarztrauber

  national center for atmospheric research  boulder,colorado 80307

   which is sponsored by the national science foundation

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *


this package consists of programs which perform fast fourier
transforms for both complex and real periodic sequences and
certain other symmetric sequences that are listed below.

1.   rffti     initialize  rfftf and rfftb
2.   rfftf     forward transform of a real periodic sequence
3.   rfftb     backward transform of a real coefficient array

4.   ezffti    initialize ezfftf and ezfftb
5.   ezfftf    a simplified real periodic forward transform
6.   ezfftb    a simplified real periodic backward transform

7.   sinti     initialize sint
8.   sint      sine transform of a real odd sequence

9.   costi     initialize cost
10.  cost      cosine transform of a real even sequence

11.  sinqi     initialize sinqf and sinqb
12.  sinqf     forward sine transform with odd wave numbers
13.  sinqb     unnormalized inverse of sinqf

14.  cosqi     initialize cosqf and cosqb
15.  cosqf     forward cosine transform with odd wave numbers
16.  cosqb     unnormalized inverse of cosqf

17.  cffti     initialize cfftf and cfftb
18.  cfftf     forward transform of a complex periodic sequence
19.  cfftb     unnormalized inverse of cfftf


******************************************************************

subroutine rffti(n,wsave)

  ****************************************************************

subroutine rffti initializes the array wsave which is used in
both rfftf and rfftb. the prime factorization of n together with
a tabulation of the trigonometric functions are computed and
stored in wsave.

input parameter

n       the length of the sequence to be transformed.

output parameter

wsave   a work array which must be dimensioned at least 2*n+15.
        the same work array can be used for both rfftf and rfftb
        as long as n remains unchanged. different wsave arrays
        are required for different values of n. the contents of
        wsave must not be changed between calls of rfftf or rfftb.

******************************************************************

subroutine rfftf(n,r,wsave)

******************************************************************

subroutine rfftf computes the fourier coefficients of a real
perodic sequence (fourier analysis). the transform is defined
below at output parameter r.

input parameters

n       the length of the array r to be transformed.  the method
        is most efficient when n is a product of small primes.
        n may change so long as different work arrays are provided

r       a real array of length n which contains the sequence
        to be transformed

wsave   a work array which must be dimensioned at least 2*n+15.
        in the program that calls rfftf. the wsave array must be
        initialized by calling subroutine rffti(n,wsave) and a
        different wsave array must be used for each different
        value of n. this initialization does not have to be
        repeated so long as n remains unchanged thus subsequent
        transforms can be obtained faster than the first.
        the same wsave array can be used by rfftf and rfftb.


output parameters

r       r(1) = the sum from i=1 to i=n of r(i)

        if n is even set l =n/2   , if n is odd set l = (n+1)/2

          then for k = 2,...,l

             r(2*k-2) = the sum from i = 1 to i = n of

                  r(i)*cos((k-1)*(i-1)*2*pi/n)

             r(2*k-1) = the sum from i = 1 to i = n of

                 -r(i)*sin((k-1)*(i-1)*2*pi/n)

        if n is even

             r(n) = the sum from i = 1 to i = n of

                  (-1)**(i-1)*r(i)

 *****  note
             this transform is unnormalized since a call of rfftf
             followed by a call of rfftb will multiply the input
             sequence by n.

wsave   contains results which must not be destroyed between
        calls of rfftf or rfftb.


******************************************************************

subroutine rfftb(n,r,wsave)

******************************************************************

subroutine rfftb computes the real perodic sequence from its
fourier coefficients (fourier synthesis). the transform is defined
below at output parameter r.

input parameters

n       the length of the array r to be transformed.  the method
        is most efficient when n is a product of small primes.
        n may change so long as different work arrays are provided

r       a real array of length n which contains the sequence
        to be transformed

wsave   a work array which must be dimensioned at least 2*n+15.
        in the program that calls rfftb. the wsave array must be
        initialized by calling subroutine rffti(n,wsave) and a
        different wsave array must be used for each different
        value of n. this initialization does not have to be
        repeated so long as n remains unchanged thus subsequent
        transforms can be obtained faster than the first.
        the same wsave array can be used by rfftf and rfftb.


output parameters

r       for n even and for i = 1,...,n

             r(i) = r(1)+(-1)**(i-1)*r(n)

                  plus the sum from k=2 to k=n/2 of

                   2.*r(2*k-2)*cos((k-1)*(i-1)*2*pi/n)

                  -2.*r(2*k-1)*sin((k-1)*(i-1)*2*pi/n)

        for n odd and for i = 1,...,n

             r(i) = r(1) plus the sum from k=2 to k=(n+1)/2 of

                  2.*r(2*k-2)*cos((k-1)*(i-1)*2*pi/n)

                 -2.*r(2*k-1)*sin((k-1)*(i-1)*2*pi/n)

 *****  note
             this transform is unnormalized since a call of rfftf
             followed by a call of rfftb will multiply the input
             sequence by n.

wsave   contains results which must not be destroyed between
        calls of rfftb or rfftf.


******************************************************************

subroutine ezffti(n,wsave)

******************************************************************

subroutine ezffti initializes the array wsave which is used in
both ezfftf and ezfftb. the prime factorization of n together with
a tabulation of the trigonometric functions are computed and
stored in wsave.

input parameter

n       the length of the sequence to be transformed.

output parameter

wsave   a work array which must be dimensioned at least 3*n+15.
        the same work array can be used for both ezfftf and ezfftb
        as long as n remains unchanged. different wsave arrays
        are required for different values of n.


******************************************************************

subroutine ezfftf(n,r,azero,a,b,wsave)

******************************************************************

subroutine ezfftf computes the fourier coefficients of a real
perodic sequence (fourier analysis). the transform is defined
below at output parameters azero,a and b. ezfftf is a simplified
but slower version of rfftf.

input parameters

n       the length of the array r to be transformed.  the method
        is must efficient when n is the product of small primes.

r       a real array of length n which contains the sequence
        to be transformed. r is not destroyed.


wsave   a work array which must be dimensioned at least 3*n+15.
        in the program that calls ezfftf. the wsave array must be
        initialized by calling subroutine ezffti(n,wsave) and a
        different wsave array must be used for each different
        value of n. this initialization does not have to be
        repeated so long as n remains unchanged thus subsequent
        transforms can be obtained faster than the first.
        the same wsave array can be used by ezfftf and ezfftb.

output parameters

azero   the sum from i=1 to i=n of r(i)/n

a,b     for n even b(n/2)=0. and a(n/2) is the sum from i=1 to
        i=n of (-1)**(i-1)*r(i)/n

        for n even define kmax=n/2-1
        for n odd  define kmax=(n-1)/2

        then for  k=1,...,kmax

             a(k) equals the sum from i=1 to i=n of

                  2./n*r(i)*cos(k*(i-1)*2*pi/n)

             b(k) equals the sum from i=1 to i=n of

                  2./n*r(i)*sin(k*(i-1)*2*pi/n)


******************************************************************

subroutine ezfftb(n,r,azero,a,b,wsave)

******************************************************************

subroutine ezfftb computes a real perodic sequence from its
fourier coefficients (fourier synthesis). the transform is
defined below at output parameter r. ezfftb is a simplified
but slower version of rfftb.

input parameters

n       the length of the output array r.  the method is most
        efficient when n is the product of small primes.

azero   the constant fourier coefficient

a,b     arrays which contain the remaining fourier coefficients
        these arrays are not destroyed.

        the length of these arrays depends on whether n is even or
        odd.

        if n is even n/2    locations are required
        if n is odd (n-1)/2 locations are required

wsave   a work array which must be dimensioned at least 3*n+15.
        in the program that calls ezfftb. the wsave array must be
        initialized by calling subroutine ezffti(n,wsave) and a
        different wsave array must be used for each different
        value of n. this initialization does not have to be
        repeated so long as n remains unchanged thus subsequent
        transforms can be obtained faster than the first.
        the same wsave array can be used by ezfftf and ezfftb.


output parameters

r       if n is even define kmax=n/2
        if n is odd  define kmax=(n-1)/2

        then for i=1,...,n

             r(i)=azero plus the sum from k=1 to k=kmax of

             a(k)*cos(k*(i-1)*2*pi/n)+b(k)*sin(k*(i-1)*2*pi/n)

********************* complex notation **************************

        for j=1,...,n

        r(j) equals the sum from k=-kmax to k=kmax of

             c(k)*exp(i*k*(j-1)*2*pi/n)

        where

             c(k) = .5*cmplx(a(k),-b(k))   for k=1,...,kmax

             c(-k) = conjg(c(k))

             c(0) = azero

                  and i=sqrt(-1)

*************** amplitude - phase notation ***********************

        for i=1,...,n

        r(i) equals azero plus the sum from k=1 to k=kmax of

             alpha(k)*cos(k*(i-1)*2*pi/n+beta(k))

        where

             alpha(k) = sqrt(a(k)*a(k)+b(k)*b(k))

             cos(beta(k))=a(k)/alpha(k)

             sin(beta(k))=-b(k)/alpha(k)

******************************************************************

subroutine cffti(n,wsave)

******************************************************************

subroutine cffti initializes the array wsave which is used in
both cfftf and cfftb. the prime factorization of n together with
a tabulation of the trigonometric functions are computed and
stored in wsave.

input parameter

n       the length of the sequence to be transformed

output parameter

wsave   a work array which must be dimensioned at least 4*n+15
        the same work array can be used for both cfftf and cfftb
        as long as n remains unchanged. different wsave arrays
        are required for different values of n. the contents of
        wsave must not be changed between calls of cfftf or cfftb.

******************************************************************

subroutine cfftf(n,c,wsave)

******************************************************************

subroutine cfftf computes the forward complex discrete fourier
transform (the fourier analysis). equivalently , cfftf computes
the fourier coefficients of a complex periodic sequence.
the transform is defined below at output parameter c.

the transform is not normalized. to obtain a normalized transform
the output must be divided by n. otherwise a call of cfftf
followed by a call of cfftb will multiply the sequence by n.

the array wsave which is used by subroutine cfftf must be
initialized by calling subroutine cffti(n,wsave).

input parameters


n      the length of the complex sequence c. the method is
       more efficient when n is the product of small primes. n

c      a complex array of length n which contains the sequence

wsave   a real work array which must be dimensioned at least 4n+15
        in the program that calls cfftf. the wsave array must be
        initialized by calling subroutine cffti(n,wsave) and a
        different wsave array must be used for each different
        value of n. this initialization does not have to be
        repeated so long as n remains unchanged thus subsequent
        transforms can be obtained faster than the first.
        the same wsave array can be used by cfftf and cfftb.

output parameters

c      for j=1,...,n

           c(j)=the sum from k=1,...,n of

                 c(k)*exp(-i*(j-1)*(k-1)*2*pi/n)

                       where i=sqrt(-1)

wsave   contains initialization calculations which must not be
        destroyed between calls of subroutine cfftf or cfftb

******************************************************************

subroutine cfftb(n,c,wsave)

******************************************************************

subroutine cfftb computes the backward complex discrete fourier
transform (the fourier synthesis). equivalently , cfftb computes
a complex periodic sequence from its fourier coefficients.
the transform is defined below at output parameter c.

a call of cfftf followed by a call of cfftb will multiply the
sequence by n.

the array wsave which is used by subroutine cfftb must be
initialized by calling subroutine cffti(n,wsave).

input parameters


n      the length of the complex sequence c. the method is
       more efficient when n is the product of small primes.

c      a complex array of length n which contains the sequence

wsave   a real work array which must be dimensioned at least 4n+15
        in the program that calls cfftb. the wsave array must be
        initialized by calling subroutine cffti(n,wsave) and a
        different wsave array must be used for each different
        value of n. this initialization does not have to be
        repeated so long as n remains unchanged thus subsequent
        transforms can be obtained faster than the first.
        the same wsave array can be used by cfftf and cfftb.

output parameters

c      for j=1,...,n

           c(j)=the sum from k=1,...,n of

                 c(k)*exp(i*(j-1)*(k-1)*2*pi/n)

                       where i=sqrt(-1)

wsave   contains initialization calculations which must not be
        destroyed between calls of subroutine cfftf or cfftb
****************************************************************** */

/* modifications of f2c output, by Louis Strous:
   integer -> int32_t
   real    -> float
   remove underscores from routine names
   from K&R to ANSI declarations
   make all static variables dynamic
   replace all external declarations by local declarations
   make a second copy with doubles rather than floats
   replace "int32_t *ifac" by "float *ifac" or "double *ifac" to fix
     mismatch of data types in some function calls
   replace "(doublereal)" by "(float)" because the "doublereals" are stored
     in floats anyway
 */

//------------------------------------------------------------------------
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <math.h>
#include "action.hh"

int32_t ezfft1(int32_t *n, float *wa, float *ifac)
{
    // Initialized data

  int32_t ntryh[4] = { 4,2,3,5 };
  float tpi = (float)6.28318530717959;

  // System generated locals
  int32_t i__1, i__2, i__3;

  // Builtin functions
  //  double cos(double), sin(double);

  // Local variables
  float argh;
  int32_t ntry, i, j, k1, l1, l2, ib, ii, nf, ip, nl, is, nq, nr;
  float ch1, sh1;
  int32_t ido, ipm;
  float dch1, ch1h, arg1, dsh1;
  int32_t nfm1;

    // Parameter adjustments
    --ifac;
    --wa;

    // Function Body
    nl = *n;
    nf = 0;
    j = 0;
L101:
    ++j;
    if (j - 4 <= 0) {
	goto L102;
    } else {
	goto L103;
    }
L102:
    ntry = ntryh[j - 1];
    goto L104;
L103:
    ntry += 2;
L104:
    nq = nl / ntry;
    nr = nl - ntry * nq;
    if (nr != 0) {
	goto L101;
    } else {
	goto L105;
    }
L105:
    ++nf;
    ifac[nf + 2] = ntry;
    nl = nq;
    if (ntry != 2) {
	goto L107;
    }
    if (nf == 1) {
	goto L107;
    }
    i__1 = nf;
    for (i = 2; i <= i__1; ++i) {
	ib = nf - i + 2;
	ifac[ib + 2] = ifac[ib + 1];
// L106:
    }
    ifac[3] = 2;
L107:
    if (nl != 1) {
	goto L104;
    }
    ifac[1] = *n;
    ifac[2] = nf;
    argh = tpi / (float) (*n);
    is = 0;
    nfm1 = nf - 1;
    l1 = 1;
    if (nfm1 == 0) {
	return 0;
    }
    i__1 = nfm1;
    for (k1 = 1; k1 <= i__1; ++k1) {
	ip = ifac[k1 + 2];
	l2 = l1 * ip;
	ido = *n / l2;
	ipm = ip - 1;
	arg1 = (float) l1 * argh;
	ch1 = (float)1.;
	sh1 = (float)0.;
	dch1 = cos(arg1);
	dsh1 = sin(arg1);
	i__2 = ipm;
	for (j = 1; j <= i__2; ++j) {
	    ch1h = dch1 * ch1 - dsh1 * sh1;
	    sh1 = dch1 * sh1 + dsh1 * ch1;
	    ch1 = ch1h;
	    i = is + 2;
	    wa[i - 1] = ch1;
	    wa[i] = sh1;
	    if (ido < 5) {
		goto L109;
	    }
	    i__3 = ido;
	    for (ii = 5; ii <= i__3; ii += 2) {
		i += 2;
		wa[i - 1] = ch1 * wa[i - 3] - sh1 * wa[i - 2];
		wa[i] = ch1 * wa[i - 2] + sh1 * wa[i - 3];
// L108:
	    }
L109:
	    is += ido;
// L110:
	}
	l1 = l2;
// L111:
    }
    return 0;
} // ezfft1
//------------------------------------------------------------------------
int32_t ezfftb(int32_t *n, float *r, float *azero, float *a, float *b, float *wsave)
{
  // System generated locals
  int32_t i__1;

  // Local variables
  int32_t i;
  int32_t rfftb(int32_t *n, float *r, float *wsave);
  int32_t ns2;

    // Parameter adjustments
    --wsave;
    --b;
    --a;
    --r;

    // Function Body
    if ((i__1 = *n - 2) < 0) {
	goto L101;
    } else if (i__1 == 0) {
	goto L102;
    } else {
	goto L103;
    }
L101:
    r[1] = *azero;
    return 0;
L102:
    r[1] = *azero + a[1];
    r[2] = *azero - a[1];
    return 0;
L103:
    ns2 = (*n - 1) / 2;
    i__1 = ns2;
    for (i = 1; i <= i__1; ++i) {
	r[i * 2] = a[i] * (float).5;
	r[(i << 1) + 1] = b[i] * (float)-.5;
// L104:
    }
    r[1] = *azero;
    if (*n % 2 == 0) {
	r[*n] = a[ns2 + 1];
    }
    rfftb(n, &r[1], &wsave[*n + 1]);
    return 0;
} // ezfftb
//------------------------------------------------------------------------
int32_t ezfftf(int32_t *n, float *r, float *azero, float *a, float *b, float *wsave)
{
  // System generated locals
  int32_t i__1;

  // Local variables
  int32_t i;
  int32_t rfftf(int32_t *n, float *r, float *wsave);
  float cf;
  int32_t ns2;
  float cfm;
  int32_t ns2m;


//                       VERSION 3  JUNE 1979

    // Parameter adjustments
    --wsave;
    --b;
    --a;
    --r;

    // Function Body
    if ((i__1 = *n - 2) < 0) {
	goto L101;
    } else if (i__1 == 0) {
	goto L102;
    } else {
	goto L103;
    }
L101:
    *azero = r[1];
    return 0;
L102:
    *azero = (r[1] + r[2]) * (float).5;
    a[1] = (r[1] - r[2]) * (float).5;
    return 0;
L103:
    i__1 = *n;
    for (i = 1; i <= i__1; ++i) {
	wsave[i] = r[i];
// L104:
    }
    rfftf(n, &wsave[1], &wsave[*n + 1]);
    cf = (float)2. / (float) (*n);
    cfm = -(float)cf;
    *azero = cf * (float).5 * wsave[1];
    ns2 = (*n + 1) / 2;
    ns2m = ns2 - 1;
    i__1 = ns2m;
    for (i = 1; i <= i__1; ++i) {
	a[i] = cf * wsave[i * 2];
	b[i] = cfm * wsave[(i << 1) + 1];
// L105:
    }
    if (*n % 2 == 1) {
	return 0;
    }
    a[ns2] = cf * (float).5 * wsave[*n];
    b[ns2] = (float)0.;
    return 0;
} // ezfftf
//------------------------------------------------------------------------
int32_t ezffti(int32_t *n, float *wsave)
{
  int32_t ezfft1(int32_t *n, float *wa, float *ifac);

  // Parameter adjustments
  --wsave;

    // Function Body
    if (*n == 1) {
	return 0;
    }
    ezfft1(n, &wsave[(*n << 1) + 1], &wsave[*n * 3 + 1]);
    return 0;
} // ezffti
//------------------------------------------------------------------------
int32_t radb2(int32_t *ido, int32_t *l1, float *cc, float *ch, float *wa1)
{
  // System generated locals
  int32_t cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

  // Local variables
  int32_t i, k, ic;
  float ti2, tr2;
  int32_t idp2;
  
  // Parameter adjustments
  ch_dim1 = *ido;
  ch_dim2 = *l1;
  ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
  ch -= ch_offset;
  cc_dim1 = *ido;
  cc_offset = cc_dim1 * 3 + 1;
  cc -= cc_offset;
  --wa1;

    // Function Body
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ch[(k + ch_dim2) * ch_dim1 + 1] = cc[((k << 1) + 1) * cc_dim1 + 1] + 
		cc[*ido + ((k << 1) + 2) * cc_dim1];
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = cc[((k << 1) + 1) * cc_dim1 
		+ 1] - cc[*ido + ((k << 1) + 2) * cc_dim1];
// L101:
    }
    if ((i__1 = *ido - 2) < 0) {
	goto L107;
    } else if (i__1 == 0) {
	goto L105;
    } else {
	goto L102;
    }
L102:
    idp2 = *ido + 2;
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    ic = idp2 - i;
	    ch[i - 1 + (k + ch_dim2) * ch_dim1] = cc[i - 1 + ((k << 1) + 1) * 
		    cc_dim1] + cc[ic - 1 + ((k << 1) + 2) * cc_dim1];
	    tr2 = cc[i - 1 + ((k << 1) + 1) * cc_dim1] - cc[ic - 1 + ((k << 1)
		     + 2) * cc_dim1];
	    ch[i + (k + ch_dim2) * ch_dim1] = cc[i + ((k << 1) + 1) * cc_dim1]
		     - cc[ic + ((k << 1) + 2) * cc_dim1];
	    ti2 = cc[i + ((k << 1) + 1) * cc_dim1] + cc[ic + ((k << 1) + 2) * 
		    cc_dim1];
	    ch[i - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i - 2] * tr2 - 
		    wa1[i - 1] * ti2;
	    ch[i + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i - 2] * ti2 + wa1[i 
		    - 1] * tr2;
// L103:
	}
// L104:
    }
    if (*ido % 2 == 1) {
	return 0;
    }
L105:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ch[*ido + (k + ch_dim2) * ch_dim1] = cc[*ido + ((k << 1) + 1) * 
		cc_dim1] + cc[*ido + ((k << 1) + 1) * cc_dim1];
	ch[*ido + (k + (ch_dim2 << 1)) * ch_dim1] = -(float)(cc[((k << 1)
		 + 2) * cc_dim1 + 1] + cc[((k << 1) + 2) * cc_dim1 + 1]);
// L106:
    }
L107:
    return 0;
} // radb2
//------------------------------------------------------------------------
int32_t radb3(int32_t *ido, int32_t *l1, float *cc, float *ch, float *wa1, float *wa2)
{
  // Initialized data
  
  float taur = (float)-.5;
  float taui = (float).866025403784439;

  // System generated locals
  int32_t cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

  // Local variables
  int32_t i, k, ic;
  float ci2, ci3, di2, di3, cr2, cr3, dr2, dr3, ti2, tr2;
  int32_t idp2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = (cc_dim1 << 2) + 1;
    cc -= cc_offset;
    --wa1;
    --wa2;

    // Function Body
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	tr2 = cc[*ido + (k * 3 + 2) * cc_dim1] + cc[*ido + (k * 3 + 2) * 
		cc_dim1];
	cr2 = cc[(k * 3 + 1) * cc_dim1 + 1] + taur * tr2;
	ch[(k + ch_dim2) * ch_dim1 + 1] = cc[(k * 3 + 1) * cc_dim1 + 1] + tr2;
	ci3 = taui * (cc[(k * 3 + 3) * cc_dim1 + 1] + cc[(k * 3 + 3) * 
		cc_dim1 + 1]);
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = cr2 - ci3;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 1] = cr2 + ci3;
// L101:
    }
    if (*ido == 1) {
	return 0;
    }
    idp2 = *ido + 2;
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    ic = idp2 - i;
	    tr2 = cc[i - 1 + (k * 3 + 3) * cc_dim1] + cc[ic - 1 + (k * 3 + 2) 
		    * cc_dim1];
	    cr2 = cc[i - 1 + (k * 3 + 1) * cc_dim1] + taur * tr2;
	    ch[i - 1 + (k + ch_dim2) * ch_dim1] = cc[i - 1 + (k * 3 + 1) * 
		    cc_dim1] + tr2;
	    ti2 = cc[i + (k * 3 + 3) * cc_dim1] - cc[ic + (k * 3 + 2) * 
		    cc_dim1];
	    ci2 = cc[i + (k * 3 + 1) * cc_dim1] + taur * ti2;
	    ch[i + (k + ch_dim2) * ch_dim1] = cc[i + (k * 3 + 1) * cc_dim1] + 
		    ti2;
	    cr3 = taui * (cc[i - 1 + (k * 3 + 3) * cc_dim1] - cc[ic - 1 + (k *
		     3 + 2) * cc_dim1]);
	    ci3 = taui * (cc[i + (k * 3 + 3) * cc_dim1] + cc[ic + (k * 3 + 2) 
		    * cc_dim1]);
	    dr2 = cr2 - ci3;
	    dr3 = cr2 + ci3;
	    di2 = ci2 + cr3;
	    di3 = ci2 - cr3;
	    ch[i - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i - 2] * dr2 - 
		    wa1[i - 1] * di2;
	    ch[i + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i - 2] * di2 + wa1[i 
		    - 1] * dr2;
	    ch[i - 1 + (k + ch_dim2 * 3) * ch_dim1] = wa2[i - 2] * dr3 - wa2[
		    i - 1] * di3;
	    ch[i + (k + ch_dim2 * 3) * ch_dim1] = wa2[i - 2] * di3 + wa2[i - 
		    1] * dr3;
// L102:
	}
// L103:
    }
    return 0;
} // radb3
//------------------------------------------------------------------------
int32_t radb4(int32_t *ido, int32_t *l1, float *cc, float *ch, float *wa1,
	  float *wa2, float *wa3)
{
  // Initialized data

  float sqrt2 = (float)1.414213562373095;

  // System generated locals
  int32_t cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

  // Local variables
  int32_t i, k, ic;
  float ci2, ci3, ci4, cr2, cr3, cr4, ti1, ti2, ti3, ti4, tr1, tr2, 
    tr3, tr4;
  int32_t idp2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = cc_dim1 * 5 + 1;
    cc -= cc_offset;
    --wa1;
    --wa2;
    --wa3;

    // Function Body
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	tr1 = cc[((k << 2) + 1) * cc_dim1 + 1] - cc[*ido + ((k << 2) + 4) * 
		cc_dim1];
	tr2 = cc[((k << 2) + 1) * cc_dim1 + 1] + cc[*ido + ((k << 2) + 4) * 
		cc_dim1];
	tr3 = cc[*ido + ((k << 2) + 2) * cc_dim1] + cc[*ido + ((k << 2) + 2) *
		 cc_dim1];
	tr4 = cc[((k << 2) + 3) * cc_dim1 + 1] + cc[((k << 2) + 3) * cc_dim1 
		+ 1];
	ch[(k + ch_dim2) * ch_dim1 + 1] = tr2 + tr3;
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = tr1 - tr4;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 1] = tr2 - tr3;
	ch[(k + (ch_dim2 << 2)) * ch_dim1 + 1] = tr1 + tr4;
// L101:
    }
    if ((i__1 = *ido - 2) < 0) {
	goto L107;
    } else if (i__1 == 0) {
	goto L105;
    } else {
	goto L102;
    }
L102:
    idp2 = *ido + 2;
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    ic = idp2 - i;
	    ti1 = cc[i + ((k << 2) + 1) * cc_dim1] + cc[ic + ((k << 2) + 4) * 
		    cc_dim1];
	    ti2 = cc[i + ((k << 2) + 1) * cc_dim1] - cc[ic + ((k << 2) + 4) * 
		    cc_dim1];
	    ti3 = cc[i + ((k << 2) + 3) * cc_dim1] - cc[ic + ((k << 2) + 2) * 
		    cc_dim1];
	    tr4 = cc[i + ((k << 2) + 3) * cc_dim1] + cc[ic + ((k << 2) + 2) * 
		    cc_dim1];
	    tr1 = cc[i - 1 + ((k << 2) + 1) * cc_dim1] - cc[ic - 1 + ((k << 2)
		     + 4) * cc_dim1];
	    tr2 = cc[i - 1 + ((k << 2) + 1) * cc_dim1] + cc[ic - 1 + ((k << 2)
		     + 4) * cc_dim1];
	    ti4 = cc[i - 1 + ((k << 2) + 3) * cc_dim1] - cc[ic - 1 + ((k << 2)
		     + 2) * cc_dim1];
	    tr3 = cc[i - 1 + ((k << 2) + 3) * cc_dim1] + cc[ic - 1 + ((k << 2)
		     + 2) * cc_dim1];
	    ch[i - 1 + (k + ch_dim2) * ch_dim1] = tr2 + tr3;
	    cr3 = tr2 - tr3;
	    ch[i + (k + ch_dim2) * ch_dim1] = ti2 + ti3;
	    ci3 = ti2 - ti3;
	    cr2 = tr1 - tr4;
	    cr4 = tr1 + tr4;
	    ci2 = ti1 + ti4;
	    ci4 = ti1 - ti4;
	    ch[i - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i - 2] * cr2 - 
		    wa1[i - 1] * ci2;
	    ch[i + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i - 2] * ci2 + wa1[i 
		    - 1] * cr2;
	    ch[i - 1 + (k + ch_dim2 * 3) * ch_dim1] = wa2[i - 2] * cr3 - wa2[
		    i - 1] * ci3;
	    ch[i + (k + ch_dim2 * 3) * ch_dim1] = wa2[i - 2] * ci3 + wa2[i - 
		    1] * cr3;
	    ch[i - 1 + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i - 2] * cr4 - 
		    wa3[i - 1] * ci4;
	    ch[i + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i - 2] * ci4 + wa3[i 
		    - 1] * cr4;
// L103:
	}
// L104:
    }
    if (*ido % 2 == 1) {
	return 0;
    }
L105:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ti1 = cc[((k << 2) + 2) * cc_dim1 + 1] + cc[((k << 2) + 4) * cc_dim1 
		+ 1];
	ti2 = cc[((k << 2) + 4) * cc_dim1 + 1] - cc[((k << 2) + 2) * cc_dim1 
		+ 1];
	tr1 = cc[*ido + ((k << 2) + 1) * cc_dim1] - cc[*ido + ((k << 2) + 3) *
		 cc_dim1];
	tr2 = cc[*ido + ((k << 2) + 1) * cc_dim1] + cc[*ido + ((k << 2) + 3) *
		 cc_dim1];
	ch[*ido + (k + ch_dim2) * ch_dim1] = tr2 + tr2;
	ch[*ido + (k + (ch_dim2 << 1)) * ch_dim1] = sqrt2 * (tr1 - ti1);
	ch[*ido + (k + ch_dim2 * 3) * ch_dim1] = ti2 + ti2;
	ch[*ido + (k + (ch_dim2 << 2)) * ch_dim1] = -(float)sqrt2 * (tr1 
		+ ti1);
// L106:
    }
L107:
    return 0;
} // radb4
//------------------------------------------------------------------------
int32_t radb5(int32_t *ido, int32_t *l1, float *cc, float *ch, float *wa1,
	  float *wa2, float *wa3, float *wa4)
{
  // Initialized data
  
  float tr11 = (float).309016994374947;
  float ti11 = (float).951056516295154;
  float tr12 = (float)-.809016994374947;
  float ti12 = (float).587785252292473;
  
  // System generated locals
  int32_t cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;
  
  // Local variables
  int32_t i, k, ic;
  float ci2, ci3, ci4, ci5, di3, di4, di5, di2, cr2, cr3, cr5, cr4, 
    ti2, ti3, ti4, ti5, dr3, dr4, dr5, dr2, tr2, tr3, tr4, tr5;
  int32_t idp2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = cc_dim1 * 6 + 1;
    cc -= cc_offset;
    --wa1;
    --wa2;
    --wa3;
    --wa4;

    // Function Body
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ti5 = cc[(k * 5 + 3) * cc_dim1 + 1] + cc[(k * 5 + 3) * cc_dim1 + 1];
	ti4 = cc[(k * 5 + 5) * cc_dim1 + 1] + cc[(k * 5 + 5) * cc_dim1 + 1];
	tr2 = cc[*ido + (k * 5 + 2) * cc_dim1] + cc[*ido + (k * 5 + 2) * 
		cc_dim1];
	tr3 = cc[*ido + (k * 5 + 4) * cc_dim1] + cc[*ido + (k * 5 + 4) * 
		cc_dim1];
	ch[(k + ch_dim2) * ch_dim1 + 1] = cc[(k * 5 + 1) * cc_dim1 + 1] + tr2 
		+ tr3;
	cr2 = cc[(k * 5 + 1) * cc_dim1 + 1] + tr11 * tr2 + tr12 * tr3;
	cr3 = cc[(k * 5 + 1) * cc_dim1 + 1] + tr12 * tr2 + tr11 * tr3;
	ci5 = ti11 * ti5 + ti12 * ti4;
	ci4 = ti12 * ti5 - ti11 * ti4;
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = cr2 - ci5;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 1] = cr3 - ci4;
	ch[(k + (ch_dim2 << 2)) * ch_dim1 + 1] = cr3 + ci4;
	ch[(k + ch_dim2 * 5) * ch_dim1 + 1] = cr2 + ci5;
// L101:
    }
    if (*ido == 1) {
	return 0;
    }
    idp2 = *ido + 2;
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    ic = idp2 - i;
	    ti5 = cc[i + (k * 5 + 3) * cc_dim1] + cc[ic + (k * 5 + 2) * 
		    cc_dim1];
	    ti2 = cc[i + (k * 5 + 3) * cc_dim1] - cc[ic + (k * 5 + 2) * 
		    cc_dim1];
	    ti4 = cc[i + (k * 5 + 5) * cc_dim1] + cc[ic + (k * 5 + 4) * 
		    cc_dim1];
	    ti3 = cc[i + (k * 5 + 5) * cc_dim1] - cc[ic + (k * 5 + 4) * 
		    cc_dim1];
	    tr5 = cc[i - 1 + (k * 5 + 3) * cc_dim1] - cc[ic - 1 + (k * 5 + 2) 
		    * cc_dim1];
	    tr2 = cc[i - 1 + (k * 5 + 3) * cc_dim1] + cc[ic - 1 + (k * 5 + 2) 
		    * cc_dim1];
	    tr4 = cc[i - 1 + (k * 5 + 5) * cc_dim1] - cc[ic - 1 + (k * 5 + 4) 
		    * cc_dim1];
	    tr3 = cc[i - 1 + (k * 5 + 5) * cc_dim1] + cc[ic - 1 + (k * 5 + 4) 
		    * cc_dim1];
	    ch[i - 1 + (k + ch_dim2) * ch_dim1] = cc[i - 1 + (k * 5 + 1) * 
		    cc_dim1] + tr2 + tr3;
	    ch[i + (k + ch_dim2) * ch_dim1] = cc[i + (k * 5 + 1) * cc_dim1] + 
		    ti2 + ti3;
	    cr2 = cc[i - 1 + (k * 5 + 1) * cc_dim1] + tr11 * tr2 + tr12 * tr3;
	    ci2 = cc[i + (k * 5 + 1) * cc_dim1] + tr11 * ti2 + tr12 * ti3;
	    cr3 = cc[i - 1 + (k * 5 + 1) * cc_dim1] + tr12 * tr2 + tr11 * tr3;
	    ci3 = cc[i + (k * 5 + 1) * cc_dim1] + tr12 * ti2 + tr11 * ti3;
	    cr5 = ti11 * tr5 + ti12 * tr4;
	    ci5 = ti11 * ti5 + ti12 * ti4;
	    cr4 = ti12 * tr5 - ti11 * tr4;
	    ci4 = ti12 * ti5 - ti11 * ti4;
	    dr3 = cr3 - ci4;
	    dr4 = cr3 + ci4;
	    di3 = ci3 + cr4;
	    di4 = ci3 - cr4;
	    dr5 = cr2 + ci5;
	    dr2 = cr2 - ci5;
	    di5 = ci2 - cr5;
	    di2 = ci2 + cr5;
	    ch[i - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i - 2] * dr2 - 
		    wa1[i - 1] * di2;
	    ch[i + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i - 2] * di2 + wa1[i 
		    - 1] * dr2;
	    ch[i - 1 + (k + ch_dim2 * 3) * ch_dim1] = wa2[i - 2] * dr3 - wa2[
		    i - 1] * di3;
	    ch[i + (k + ch_dim2 * 3) * ch_dim1] = wa2[i - 2] * di3 + wa2[i - 
		    1] * dr3;
	    ch[i - 1 + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i - 2] * dr4 - 
		    wa3[i - 1] * di4;
	    ch[i + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i - 2] * di4 + wa3[i 
		    - 1] * dr4;
	    ch[i - 1 + (k + ch_dim2 * 5) * ch_dim1] = wa4[i - 2] * dr5 - wa4[
		    i - 1] * di5;
	    ch[i + (k + ch_dim2 * 5) * ch_dim1] = wa4[i - 2] * di5 + wa4[i - 
		    1] * dr5;
// L102:
	}
// L103:
    }
    return 0;
} // radb5
//------------------------------------------------------------------------
int32_t radbg(int32_t *ido, int32_t *ip, int32_t *l1, int32_t *idl1, float *cc, float *c1,
	  float *c2, float *ch, float *ch2, float *wa)
{
  // Initialized data

  float tpi = (float)6.28318530717959;

  // System generated locals
  int32_t ch_dim1, ch_dim2, ch_offset, cc_dim1, cc_dim2, cc_offset, c1_dim1,
    c1_dim2, c1_offset, c2_dim1, c2_offset, ch2_dim1, ch2_offset, 
    i__1, i__2, i__3;

  // Builtin functions
  double cos(double), sin(double);

  // Local variables
  int32_t idij, ipph, i, j, k, l, j2, ic, jc, lc, ik, is;
  float dc2, ai1, ai2, ar1, ar2, ds2;
  int32_t nbd;
  float dcp, arg, dsp, ar1h, ar2h;
  int32_t idp2, ipp2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    c1_dim1 = *ido;
    c1_dim2 = *l1;
    c1_offset = c1_dim1 * (c1_dim2 + 1) + 1;
    c1 -= c1_offset;
    cc_dim1 = *ido;
    cc_dim2 = *ip;
    cc_offset = cc_dim1 * (cc_dim2 + 1) + 1;
    cc -= cc_offset;
    ch2_dim1 = *idl1;
    ch2_offset = ch2_dim1 + 1;
    ch2 -= ch2_offset;
    c2_dim1 = *idl1;
    c2_offset = c2_dim1 + 1;
    c2 -= c2_offset;
    --wa;

    // Function Body
    arg = tpi / (float) (*ip);
    dcp = cos(arg);
    dsp = sin(arg);
    idp2 = *ido + 2;
    nbd = (*ido - 1) / 2;
    ipp2 = *ip + 2;
    ipph = (*ip + 1) / 2;
    if (*ido < *l1) {
	goto L103;
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i = 1; i <= i__2; ++i) {
	    ch[i + (k + ch_dim2) * ch_dim1] = cc[i + (k * cc_dim2 + 1) * 
		    cc_dim1];
// L101:
	}
// L102:
    }
    goto L106;
L103:
    i__1 = *ido;
    for (i = 1; i <= i__1; ++i) {
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    ch[i + (k + ch_dim2) * ch_dim1] = cc[i + (k * cc_dim2 + 1) * 
		    cc_dim1];
// L104:
	}
// L105:
    }
L106:
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	j2 = j + j;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    ch[(k + j * ch_dim2) * ch_dim1 + 1] = cc[*ido + (j2 - 2 + k * 
		    cc_dim2) * cc_dim1] + cc[*ido + (j2 - 2 + k * cc_dim2) * 
		    cc_dim1];
	    ch[(k + jc * ch_dim2) * ch_dim1 + 1] = cc[(j2 - 1 + k * cc_dim2) *
		     cc_dim1 + 1] + cc[(j2 - 1 + k * cc_dim2) * cc_dim1 + 1];
// L107:
	}
// L108:
    }
    if (*ido == 1) {
	goto L116;
    }
    if (nbd < *l1) {
	goto L112;
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    i__3 = *ido;
	    for (i = 3; i <= i__3; i += 2) {
		ic = idp2 - i;
		ch[i - 1 + (k + j * ch_dim2) * ch_dim1] = cc[i - 1 + ((j << 1)
			 - 1 + k * cc_dim2) * cc_dim1] + cc[ic - 1 + ((j << 1)
			 - 2 + k * cc_dim2) * cc_dim1];
		ch[i - 1 + (k + jc * ch_dim2) * ch_dim1] = cc[i - 1 + ((j << 
			1) - 1 + k * cc_dim2) * cc_dim1] - cc[ic - 1 + ((j << 
			1) - 2 + k * cc_dim2) * cc_dim1];
		ch[i + (k + j * ch_dim2) * ch_dim1] = cc[i + ((j << 1) - 1 + 
			k * cc_dim2) * cc_dim1] - cc[ic + ((j << 1) - 2 + k * 
			cc_dim2) * cc_dim1];
		ch[i + (k + jc * ch_dim2) * ch_dim1] = cc[i + ((j << 1) - 1 + 
			k * cc_dim2) * cc_dim1] + cc[ic + ((j << 1) - 2 + k * 
			cc_dim2) * cc_dim1];
// L109:
	    }
// L110:
	}
// L111:
    }
    goto L116;
L112:
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    ic = idp2 - i;
	    i__3 = *l1;
	    for (k = 1; k <= i__3; ++k) {
		ch[i - 1 + (k + j * ch_dim2) * ch_dim1] = cc[i - 1 + ((j << 1)
			 - 1 + k * cc_dim2) * cc_dim1] + cc[ic - 1 + ((j << 1)
			 - 2 + k * cc_dim2) * cc_dim1];
		ch[i - 1 + (k + jc * ch_dim2) * ch_dim1] = cc[i - 1 + ((j << 
			1) - 1 + k * cc_dim2) * cc_dim1] - cc[ic - 1 + ((j << 
			1) - 2 + k * cc_dim2) * cc_dim1];
		ch[i + (k + j * ch_dim2) * ch_dim1] = cc[i + ((j << 1) - 1 + 
			k * cc_dim2) * cc_dim1] - cc[ic + ((j << 1) - 2 + k * 
			cc_dim2) * cc_dim1];
		ch[i + (k + jc * ch_dim2) * ch_dim1] = cc[i + ((j << 1) - 1 + 
			k * cc_dim2) * cc_dim1] + cc[ic + ((j << 1) - 2 + k * 
			cc_dim2) * cc_dim1];
// L113:
	    }
// L114:
	}
// L115:
    }
L116:
    ar1 = (float)1.;
    ai1 = (float)0.;
    i__1 = ipph;
    for (l = 2; l <= i__1; ++l) {
	lc = ipp2 - l;
	ar1h = dcp * ar1 - dsp * ai1;
	ai1 = dcp * ai1 + dsp * ar1;
	ar1 = ar1h;
	i__2 = *idl1;
	for (ik = 1; ik <= i__2; ++ik) {
	    c2[ik + l * c2_dim1] = ch2[ik + ch2_dim1] + ar1 * ch2[ik + (
		    ch2_dim1 << 1)];
	    c2[ik + lc * c2_dim1] = ai1 * ch2[ik + *ip * ch2_dim1];
// L117:
	}
	dc2 = ar1;
	ds2 = ai1;
	ar2 = ar1;
	ai2 = ai1;
	i__2 = ipph;
	for (j = 3; j <= i__2; ++j) {
	    jc = ipp2 - j;
	    ar2h = dc2 * ar2 - ds2 * ai2;
	    ai2 = dc2 * ai2 + ds2 * ar2;
	    ar2 = ar2h;
	    i__3 = *idl1;
	    for (ik = 1; ik <= i__3; ++ik) {
		c2[ik + l * c2_dim1] += ar2 * ch2[ik + j * ch2_dim1];
		c2[ik + lc * c2_dim1] += ai2 * ch2[ik + jc * ch2_dim1];
// L118:
	    }
// L119:
	}
// L120:
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	i__2 = *idl1;
	for (ik = 1; ik <= i__2; ++ik) {
	    ch2[ik + ch2_dim1] += ch2[ik + j * ch2_dim1];
// L121:
	}
// L122:
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    ch[(k + j * ch_dim2) * ch_dim1 + 1] = c1[(k + j * c1_dim2) * 
		    c1_dim1 + 1] - c1[(k + jc * c1_dim2) * c1_dim1 + 1];
	    ch[(k + jc * ch_dim2) * ch_dim1 + 1] = c1[(k + j * c1_dim2) * 
		    c1_dim1 + 1] + c1[(k + jc * c1_dim2) * c1_dim1 + 1];
// L123:
	}
// L124:
    }
    if (*ido == 1) {
	goto L132;
    }
    if (nbd < *l1) {
	goto L128;
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    i__3 = *ido;
	    for (i = 3; i <= i__3; i += 2) {
		ch[i - 1 + (k + j * ch_dim2) * ch_dim1] = c1[i - 1 + (k + j * 
			c1_dim2) * c1_dim1] - c1[i + (k + jc * c1_dim2) * 
			c1_dim1];
		ch[i - 1 + (k + jc * ch_dim2) * ch_dim1] = c1[i - 1 + (k + j *
			 c1_dim2) * c1_dim1] + c1[i + (k + jc * c1_dim2) * 
			c1_dim1];
		ch[i + (k + j * ch_dim2) * ch_dim1] = c1[i + (k + j * c1_dim2)
			 * c1_dim1] + c1[i - 1 + (k + jc * c1_dim2) * c1_dim1]
			;
		ch[i + (k + jc * ch_dim2) * ch_dim1] = c1[i + (k + j * 
			c1_dim2) * c1_dim1] - c1[i - 1 + (k + jc * c1_dim2) * 
			c1_dim1];
// L125:
	    }
// L126:
	}
// L127:
    }
    goto L132;
L128:
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    i__3 = *l1;
	    for (k = 1; k <= i__3; ++k) {
		ch[i - 1 + (k + j * ch_dim2) * ch_dim1] = c1[i - 1 + (k + j * 
			c1_dim2) * c1_dim1] - c1[i + (k + jc * c1_dim2) * 
			c1_dim1];
		ch[i - 1 + (k + jc * ch_dim2) * ch_dim1] = c1[i - 1 + (k + j *
			 c1_dim2) * c1_dim1] + c1[i + (k + jc * c1_dim2) * 
			c1_dim1];
		ch[i + (k + j * ch_dim2) * ch_dim1] = c1[i + (k + j * c1_dim2)
			 * c1_dim1] + c1[i - 1 + (k + jc * c1_dim2) * c1_dim1]
			;
		ch[i + (k + jc * ch_dim2) * ch_dim1] = c1[i + (k + j * 
			c1_dim2) * c1_dim1] - c1[i - 1 + (k + jc * c1_dim2) * 
			c1_dim1];
// L129:
	    }
// L130:
	}
// L131:
    }
L132:
    if (*ido == 1) {
	return 0;
    }
    i__1 = *idl1;
    for (ik = 1; ik <= i__1; ++ik) {
	c2[ik + c2_dim1] = ch2[ik + ch2_dim1];
// L133:
    }
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    c1[(k + j * c1_dim2) * c1_dim1 + 1] = ch[(k + j * ch_dim2) * 
		    ch_dim1 + 1];
// L134:
	}
// L135:
    }
    if (nbd > *l1) {
	goto L139;
    }
    is = -(*ido);
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
	is += *ido;
	idij = is;
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    idij += 2;
	    i__3 = *l1;
	    for (k = 1; k <= i__3; ++k) {
		c1[i - 1 + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1] * ch[i 
			- 1 + (k + j * ch_dim2) * ch_dim1] - wa[idij] * ch[i 
			+ (k + j * ch_dim2) * ch_dim1];
		c1[i + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1] * ch[i + (
			k + j * ch_dim2) * ch_dim1] + wa[idij] * ch[i - 1 + (
			k + j * ch_dim2) * ch_dim1];
// L136:
	    }
// L137:
	}
// L138:
    }
    goto L143;
L139:
    is = -(*ido);
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
	is += *ido;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    idij = is;
	    i__3 = *ido;
	    for (i = 3; i <= i__3; i += 2) {
		idij += 2;
		c1[i - 1 + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1] * ch[i 
			- 1 + (k + j * ch_dim2) * ch_dim1] - wa[idij] * ch[i 
			+ (k + j * ch_dim2) * ch_dim1];
		c1[i + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1] * ch[i + (
			k + j * ch_dim2) * ch_dim1] + wa[idij] * ch[i - 1 + (
			k + j * ch_dim2) * ch_dim1];
// L140:
	    }
// L141:
	}
// L142:
    }
L143:
    return 0;
} // radbg
//------------------------------------------------------------------------
int32_t radf2(int32_t *ido, int32_t *l1, float *cc, float *ch, float *wa1)
{
  // System generated locals
  int32_t ch_dim1, ch_offset, cc_dim1, cc_dim2, cc_offset, i__1, i__2;

  // Local variables
  int32_t i, k, ic;
  float ti2, tr2;
  int32_t idp2;

  // Parameter adjustments
    ch_dim1 = *ido;
    ch_offset = ch_dim1 * 3 + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_dim2 = *l1;
    cc_offset = cc_dim1 * (cc_dim2 + 1) + 1;
    cc -= cc_offset;
    --wa1;

    // Function Body
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ch[((k << 1) + 1) * ch_dim1 + 1] = cc[(k + cc_dim2) * cc_dim1 + 1] + 
		cc[(k + (cc_dim2 << 1)) * cc_dim1 + 1];
	ch[*ido + ((k << 1) + 2) * ch_dim1] = cc[(k + cc_dim2) * cc_dim1 + 1] 
		- cc[(k + (cc_dim2 << 1)) * cc_dim1 + 1];
// L101:
    }
    if ((i__1 = *ido - 2) < 0) {
	goto L107;
    } else if (i__1 == 0) {
	goto L105;
    } else {
	goto L102;
    }
L102:
    idp2 = *ido + 2;
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    ic = idp2 - i;
	    tr2 = wa1[i - 2] * cc[i - 1 + (k + (cc_dim2 << 1)) * cc_dim1] + 
		    wa1[i - 1] * cc[i + (k + (cc_dim2 << 1)) * cc_dim1];
	    ti2 = wa1[i - 2] * cc[i + (k + (cc_dim2 << 1)) * cc_dim1] - wa1[i 
		    - 1] * cc[i - 1 + (k + (cc_dim2 << 1)) * cc_dim1];
	    ch[i + ((k << 1) + 1) * ch_dim1] = cc[i + (k + cc_dim2) * cc_dim1]
		     + ti2;
	    ch[ic + ((k << 1) + 2) * ch_dim1] = ti2 - cc[i + (k + cc_dim2) * 
		    cc_dim1];
	    ch[i - 1 + ((k << 1) + 1) * ch_dim1] = cc[i - 1 + (k + cc_dim2) * 
		    cc_dim1] + tr2;
	    ch[ic - 1 + ((k << 1) + 2) * ch_dim1] = cc[i - 1 + (k + cc_dim2) *
		     cc_dim1] - tr2;
// L103:
	}
// L104:
    }
    if (*ido % 2 == 1) {
	return 0;
    }
L105:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ch[((k << 1) + 2) * ch_dim1 + 1] = -(float)cc[*ido + (k + (
		cc_dim2 << 1)) * cc_dim1];
	ch[*ido + ((k << 1) + 1) * ch_dim1] = cc[*ido + (k + cc_dim2) * 
		cc_dim1];
// L106:
    }
L107:
    return 0;
} // radf2
//------------------------------------------------------------------------
int32_t radf3(int32_t *ido, int32_t *l1, float *cc, float *ch, float *wa1, float *wa2)
{
  // Initialized data
  
  float taur = (float)-.5;
  float taui = (float).866025403784439;
  
  // System generated locals
  int32_t ch_dim1, ch_offset, cc_dim1, cc_dim2, cc_offset, i__1, i__2;
  
  // Local variables
  int32_t i, k, ic;
  float ci2, di2, di3, cr2, dr2, dr3, ti2, ti3, tr2, tr3;
  int32_t idp2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_offset = (ch_dim1 << 2) + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_dim2 = *l1;
    cc_offset = cc_dim1 * (cc_dim2 + 1) + 1;
    cc -= cc_offset;
    --wa1;
    --wa2;

    // Function Body
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	cr2 = cc[(k + (cc_dim2 << 1)) * cc_dim1 + 1] + cc[(k + cc_dim2 * 3) * 
		cc_dim1 + 1];
	ch[(k * 3 + 1) * ch_dim1 + 1] = cc[(k + cc_dim2) * cc_dim1 + 1] + cr2;
	ch[(k * 3 + 3) * ch_dim1 + 1] = taui * (cc[(k + cc_dim2 * 3) * 
		cc_dim1 + 1] - cc[(k + (cc_dim2 << 1)) * cc_dim1 + 1]);
	ch[*ido + (k * 3 + 2) * ch_dim1] = cc[(k + cc_dim2) * cc_dim1 + 1] + 
		taur * cr2;
// L101:
    }
    if (*ido == 1) {
	return 0;
    }
    idp2 = *ido + 2;
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    ic = idp2 - i;
	    dr2 = wa1[i - 2] * cc[i - 1 + (k + (cc_dim2 << 1)) * cc_dim1] + 
		    wa1[i - 1] * cc[i + (k + (cc_dim2 << 1)) * cc_dim1];
	    di2 = wa1[i - 2] * cc[i + (k + (cc_dim2 << 1)) * cc_dim1] - wa1[i 
		    - 1] * cc[i - 1 + (k + (cc_dim2 << 1)) * cc_dim1];
	    dr3 = wa2[i - 2] * cc[i - 1 + (k + cc_dim2 * 3) * cc_dim1] + wa2[
		    i - 1] * cc[i + (k + cc_dim2 * 3) * cc_dim1];
	    di3 = wa2[i - 2] * cc[i + (k + cc_dim2 * 3) * cc_dim1] - wa2[i - 
		    1] * cc[i - 1 + (k + cc_dim2 * 3) * cc_dim1];
	    cr2 = dr2 + dr3;
	    ci2 = di2 + di3;
	    ch[i - 1 + (k * 3 + 1) * ch_dim1] = cc[i - 1 + (k + cc_dim2) * 
		    cc_dim1] + cr2;
	    ch[i + (k * 3 + 1) * ch_dim1] = cc[i + (k + cc_dim2) * cc_dim1] + 
		    ci2;
	    tr2 = cc[i - 1 + (k + cc_dim2) * cc_dim1] + taur * cr2;
	    ti2 = cc[i + (k + cc_dim2) * cc_dim1] + taur * ci2;
	    tr3 = taui * (di2 - di3);
	    ti3 = taui * (dr3 - dr2);
	    ch[i - 1 + (k * 3 + 3) * ch_dim1] = tr2 + tr3;
	    ch[ic - 1 + (k * 3 + 2) * ch_dim1] = tr2 - tr3;
	    ch[i + (k * 3 + 3) * ch_dim1] = ti2 + ti3;
	    ch[ic + (k * 3 + 2) * ch_dim1] = ti3 - ti2;
// L102:
	}
// L103:
    }
    return 0;
} // radf3
//------------------------------------------------------------------------
int32_t radf4(int32_t *ido, int32_t *l1, float *cc, float *ch, float *wa1,
	  float *wa2, float *wa3)
{
  // Initialized data
  
  float hsqt2 = (float).7071067811865475;
  
  // System generated locals
  int32_t cc_dim1, cc_dim2, cc_offset, ch_dim1, ch_offset, i__1, i__2;

  // Local variables
  int32_t i, k, ic;
  float ci2, ci3, ci4, cr2, cr3, cr4, ti1, ti2, ti3, ti4, tr1, tr2, 
    tr3, tr4;
  int32_t idp2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_offset = ch_dim1 * 5 + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_dim2 = *l1;
    cc_offset = cc_dim1 * (cc_dim2 + 1) + 1;
    cc -= cc_offset;
    --wa1;
    --wa2;
    --wa3;

    // Function Body
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	tr1 = cc[(k + (cc_dim2 << 1)) * cc_dim1 + 1] + cc[(k + (cc_dim2 << 2))
		 * cc_dim1 + 1];
	tr2 = cc[(k + cc_dim2) * cc_dim1 + 1] + cc[(k + cc_dim2 * 3) * 
		cc_dim1 + 1];
	ch[((k << 2) + 1) * ch_dim1 + 1] = tr1 + tr2;
	ch[*ido + ((k << 2) + 4) * ch_dim1] = tr2 - tr1;
	ch[*ido + ((k << 2) + 2) * ch_dim1] = cc[(k + cc_dim2) * cc_dim1 + 1] 
		- cc[(k + cc_dim2 * 3) * cc_dim1 + 1];
	ch[((k << 2) + 3) * ch_dim1 + 1] = cc[(k + (cc_dim2 << 2)) * cc_dim1 
		+ 1] - cc[(k + (cc_dim2 << 1)) * cc_dim1 + 1];
// L101:
    }
    if ((i__1 = *ido - 2) < 0) {
	goto L107;
    } else if (i__1 == 0) {
	goto L105;
    } else {
	goto L102;
    }
L102:
    idp2 = *ido + 2;
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    ic = idp2 - i;
	    cr2 = wa1[i - 2] * cc[i - 1 + (k + (cc_dim2 << 1)) * cc_dim1] + 
		    wa1[i - 1] * cc[i + (k + (cc_dim2 << 1)) * cc_dim1];
	    ci2 = wa1[i - 2] * cc[i + (k + (cc_dim2 << 1)) * cc_dim1] - wa1[i 
		    - 1] * cc[i - 1 + (k + (cc_dim2 << 1)) * cc_dim1];
	    cr3 = wa2[i - 2] * cc[i - 1 + (k + cc_dim2 * 3) * cc_dim1] + wa2[
		    i - 1] * cc[i + (k + cc_dim2 * 3) * cc_dim1];
	    ci3 = wa2[i - 2] * cc[i + (k + cc_dim2 * 3) * cc_dim1] - wa2[i - 
		    1] * cc[i - 1 + (k + cc_dim2 * 3) * cc_dim1];
	    cr4 = wa3[i - 2] * cc[i - 1 + (k + (cc_dim2 << 2)) * cc_dim1] + 
		    wa3[i - 1] * cc[i + (k + (cc_dim2 << 2)) * cc_dim1];
	    ci4 = wa3[i - 2] * cc[i + (k + (cc_dim2 << 2)) * cc_dim1] - wa3[i 
		    - 1] * cc[i - 1 + (k + (cc_dim2 << 2)) * cc_dim1];
	    tr1 = cr2 + cr4;
	    tr4 = cr4 - cr2;
	    ti1 = ci2 + ci4;
	    ti4 = ci2 - ci4;
	    ti2 = cc[i + (k + cc_dim2) * cc_dim1] + ci3;
	    ti3 = cc[i + (k + cc_dim2) * cc_dim1] - ci3;
	    tr2 = cc[i - 1 + (k + cc_dim2) * cc_dim1] + cr3;
	    tr3 = cc[i - 1 + (k + cc_dim2) * cc_dim1] - cr3;
	    ch[i - 1 + ((k << 2) + 1) * ch_dim1] = tr1 + tr2;
	    ch[ic - 1 + ((k << 2) + 4) * ch_dim1] = tr2 - tr1;
	    ch[i + ((k << 2) + 1) * ch_dim1] = ti1 + ti2;
	    ch[ic + ((k << 2) + 4) * ch_dim1] = ti1 - ti2;
	    ch[i - 1 + ((k << 2) + 3) * ch_dim1] = ti4 + tr3;
	    ch[ic - 1 + ((k << 2) + 2) * ch_dim1] = tr3 - ti4;
	    ch[i + ((k << 2) + 3) * ch_dim1] = tr4 + ti3;
	    ch[ic + ((k << 2) + 2) * ch_dim1] = tr4 - ti3;
// L103:
	}
// L104:
    }
    if (*ido % 2 == 1) {
	return 0;
    }
L105:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ti1 = -(float)hsqt2 * (cc[*ido + (k + (cc_dim2 << 1)) * cc_dim1] 
		+ cc[*ido + (k + (cc_dim2 << 2)) * cc_dim1]);
	tr1 = hsqt2 * (cc[*ido + (k + (cc_dim2 << 1)) * cc_dim1] - cc[*ido + (
		k + (cc_dim2 << 2)) * cc_dim1]);
	ch[*ido + ((k << 2) + 1) * ch_dim1] = tr1 + cc[*ido + (k + cc_dim2) * 
		cc_dim1];
	ch[*ido + ((k << 2) + 3) * ch_dim1] = cc[*ido + (k + cc_dim2) * 
		cc_dim1] - tr1;
	ch[((k << 2) + 2) * ch_dim1 + 1] = ti1 - cc[*ido + (k + cc_dim2 * 3) *
		 cc_dim1];
	ch[((k << 2) + 4) * ch_dim1 + 1] = ti1 + cc[*ido + (k + cc_dim2 * 3) *
		 cc_dim1];
// L106:
    }
L107:
    return 0;
} // radf4
//------------------------------------------------------------------------
int32_t radf5(int32_t *ido, int32_t *l1, float *cc, float *ch, float *wa1,
	  float *wa2, float *wa3, float *wa4)
{
  // Initialized data
  
  float tr11 = (float).309016994374947;
  float ti11 = (float).951056516295154;
  float tr12 = (float)-.809016994374947;
  float ti12 = (float).587785252292473;
  
  // System generated locals
  int32_t cc_dim1, cc_dim2, cc_offset, ch_dim1, ch_offset, i__1, i__2;
  
  // Local variables
  int32_t i, k, ic;
  float ci2, di2, ci4, ci5, di3, di4, di5, ci3, cr2, cr3, dr2, dr3, 
    dr4, dr5, cr5, cr4, ti2, ti3, ti5, ti4, tr2, tr3, tr4, tr5;
  int32_t idp2;
  
    // Parameter adjustments
    ch_dim1 = *ido;
    ch_offset = ch_dim1 * 6 + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_dim2 = *l1;
    cc_offset = cc_dim1 * (cc_dim2 + 1) + 1;
    cc -= cc_offset;
    --wa1;
    --wa2;
    --wa3;
    --wa4;

    // Function Body
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	cr2 = cc[(k + cc_dim2 * 5) * cc_dim1 + 1] + cc[(k + (cc_dim2 << 1)) * 
		cc_dim1 + 1];
	ci5 = cc[(k + cc_dim2 * 5) * cc_dim1 + 1] - cc[(k + (cc_dim2 << 1)) * 
		cc_dim1 + 1];
	cr3 = cc[(k + (cc_dim2 << 2)) * cc_dim1 + 1] + cc[(k + cc_dim2 * 3) * 
		cc_dim1 + 1];
	ci4 = cc[(k + (cc_dim2 << 2)) * cc_dim1 + 1] - cc[(k + cc_dim2 * 3) * 
		cc_dim1 + 1];
	ch[(k * 5 + 1) * ch_dim1 + 1] = cc[(k + cc_dim2) * cc_dim1 + 1] + cr2 
		+ cr3;
	ch[*ido + (k * 5 + 2) * ch_dim1] = cc[(k + cc_dim2) * cc_dim1 + 1] + 
		tr11 * cr2 + tr12 * cr3;
	ch[(k * 5 + 3) * ch_dim1 + 1] = ti11 * ci5 + ti12 * ci4;
	ch[*ido + (k * 5 + 4) * ch_dim1] = cc[(k + cc_dim2) * cc_dim1 + 1] + 
		tr12 * cr2 + tr11 * cr3;
	ch[(k * 5 + 5) * ch_dim1 + 1] = ti12 * ci5 - ti11 * ci4;
// L101:
    }
    if (*ido == 1) {
	return 0;
    }
    idp2 = *ido + 2;
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    ic = idp2 - i;
	    dr2 = wa1[i - 2] * cc[i - 1 + (k + (cc_dim2 << 1)) * cc_dim1] + 
		    wa1[i - 1] * cc[i + (k + (cc_dim2 << 1)) * cc_dim1];
	    di2 = wa1[i - 2] * cc[i + (k + (cc_dim2 << 1)) * cc_dim1] - wa1[i 
		    - 1] * cc[i - 1 + (k + (cc_dim2 << 1)) * cc_dim1];
	    dr3 = wa2[i - 2] * cc[i - 1 + (k + cc_dim2 * 3) * cc_dim1] + wa2[
		    i - 1] * cc[i + (k + cc_dim2 * 3) * cc_dim1];
	    di3 = wa2[i - 2] * cc[i + (k + cc_dim2 * 3) * cc_dim1] - wa2[i - 
		    1] * cc[i - 1 + (k + cc_dim2 * 3) * cc_dim1];
	    dr4 = wa3[i - 2] * cc[i - 1 + (k + (cc_dim2 << 2)) * cc_dim1] + 
		    wa3[i - 1] * cc[i + (k + (cc_dim2 << 2)) * cc_dim1];
	    di4 = wa3[i - 2] * cc[i + (k + (cc_dim2 << 2)) * cc_dim1] - wa3[i 
		    - 1] * cc[i - 1 + (k + (cc_dim2 << 2)) * cc_dim1];
	    dr5 = wa4[i - 2] * cc[i - 1 + (k + cc_dim2 * 5) * cc_dim1] + wa4[
		    i - 1] * cc[i + (k + cc_dim2 * 5) * cc_dim1];
	    di5 = wa4[i - 2] * cc[i + (k + cc_dim2 * 5) * cc_dim1] - wa4[i - 
		    1] * cc[i - 1 + (k + cc_dim2 * 5) * cc_dim1];
	    cr2 = dr2 + dr5;
	    ci5 = dr5 - dr2;
	    cr5 = di2 - di5;
	    ci2 = di2 + di5;
	    cr3 = dr3 + dr4;
	    ci4 = dr4 - dr3;
	    cr4 = di3 - di4;
	    ci3 = di3 + di4;
	    ch[i - 1 + (k * 5 + 1) * ch_dim1] = cc[i - 1 + (k + cc_dim2) * 
		    cc_dim1] + cr2 + cr3;
	    ch[i + (k * 5 + 1) * ch_dim1] = cc[i + (k + cc_dim2) * cc_dim1] + 
		    ci2 + ci3;
	    tr2 = cc[i - 1 + (k + cc_dim2) * cc_dim1] + tr11 * cr2 + tr12 * 
		    cr3;
	    ti2 = cc[i + (k + cc_dim2) * cc_dim1] + tr11 * ci2 + tr12 * ci3;
	    tr3 = cc[i - 1 + (k + cc_dim2) * cc_dim1] + tr12 * cr2 + tr11 * 
		    cr3;
	    ti3 = cc[i + (k + cc_dim2) * cc_dim1] + tr12 * ci2 + tr11 * ci3;
	    tr5 = ti11 * cr5 + ti12 * cr4;
	    ti5 = ti11 * ci5 + ti12 * ci4;
	    tr4 = ti12 * cr5 - ti11 * cr4;
	    ti4 = ti12 * ci5 - ti11 * ci4;
	    ch[i - 1 + (k * 5 + 3) * ch_dim1] = tr2 + tr5;
	    ch[ic - 1 + (k * 5 + 2) * ch_dim1] = tr2 - tr5;
	    ch[i + (k * 5 + 3) * ch_dim1] = ti2 + ti5;
	    ch[ic + (k * 5 + 2) * ch_dim1] = ti5 - ti2;
	    ch[i - 1 + (k * 5 + 5) * ch_dim1] = tr3 + tr4;
	    ch[ic - 1 + (k * 5 + 4) * ch_dim1] = tr3 - tr4;
	    ch[i + (k * 5 + 5) * ch_dim1] = ti3 + ti4;
	    ch[ic + (k * 5 + 4) * ch_dim1] = ti4 - ti3;
// L102:
	}
// L103:
    }
    return 0;
} // radf5
//------------------------------------------------------------------------
int32_t radfg(int32_t *ido, int32_t *ip, int32_t *l1, int32_t *idl1, float *cc, float *c1,
	  float *c2, float *ch, float *ch2, float *wa)
{
  // Initialized data

  float tpi = (float)6.28318530717959;
  
  // System generated locals
  int32_t ch_dim1, ch_dim2, ch_offset, cc_dim1, cc_dim2, cc_offset, c1_dim1,
    c1_dim2, c1_offset, c2_dim1, c2_offset, ch2_dim1, ch2_offset, 
    i__1, i__2, i__3;
  
  // Builtin functions
  double cos(double), sin(double);

  // Local variables
  int32_t idij, ipph, i, j, k, l, j2, ic, jc, lc, ik, is;
  float dc2, ai1, ai2, ar1, ar2, ds2;
  int32_t nbd;
  float dcp, arg, dsp, ar1h, ar2h;
  int32_t idp2, ipp2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    c1_dim1 = *ido;
    c1_dim2 = *l1;
    c1_offset = c1_dim1 * (c1_dim2 + 1) + 1;
    c1 -= c1_offset;
    cc_dim1 = *ido;
    cc_dim2 = *ip;
    cc_offset = cc_dim1 * (cc_dim2 + 1) + 1;
    cc -= cc_offset;
    ch2_dim1 = *idl1;
    ch2_offset = ch2_dim1 + 1;
    ch2 -= ch2_offset;
    c2_dim1 = *idl1;
    c2_offset = c2_dim1 + 1;
    c2 -= c2_offset;
    --wa;

    // Function Body
    arg = tpi / (float) (*ip);
    dcp = cos(arg);
    dsp = sin(arg);
    ipph = (*ip + 1) / 2;
    ipp2 = *ip + 2;
    idp2 = *ido + 2;
    nbd = (*ido - 1) / 2;
    if (*ido == 1) {
	goto L119;
    }
    i__1 = *idl1;
    for (ik = 1; ik <= i__1; ++ik) {
	ch2[ik + ch2_dim1] = c2[ik + c2_dim1];
// L101:
    }
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    ch[(k + j * ch_dim2) * ch_dim1 + 1] = c1[(k + j * c1_dim2) * 
		    c1_dim1 + 1];
// L102:
	}
// L103:
    }
    if (nbd > *l1) {
	goto L107;
    }
    is = -(*ido);
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
	is += *ido;
	idij = is;
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    idij += 2;
	    i__3 = *l1;
	    for (k = 1; k <= i__3; ++k) {
		ch[i - 1 + (k + j * ch_dim2) * ch_dim1] = wa[idij - 1] * c1[i 
			- 1 + (k + j * c1_dim2) * c1_dim1] + wa[idij] * c1[i 
			+ (k + j * c1_dim2) * c1_dim1];
		ch[i + (k + j * ch_dim2) * ch_dim1] = wa[idij - 1] * c1[i + (
			k + j * c1_dim2) * c1_dim1] - wa[idij] * c1[i - 1 + (
			k + j * c1_dim2) * c1_dim1];
// L104:
	    }
// L105:
	}
// L106:
    }
    goto L111;
L107:
    is = -(*ido);
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
	is += *ido;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    idij = is;
	    i__3 = *ido;
	    for (i = 3; i <= i__3; i += 2) {
		idij += 2;
		ch[i - 1 + (k + j * ch_dim2) * ch_dim1] = wa[idij - 1] * c1[i 
			- 1 + (k + j * c1_dim2) * c1_dim1] + wa[idij] * c1[i 
			+ (k + j * c1_dim2) * c1_dim1];
		ch[i + (k + j * ch_dim2) * ch_dim1] = wa[idij - 1] * c1[i + (
			k + j * c1_dim2) * c1_dim1] - wa[idij] * c1[i - 1 + (
			k + j * c1_dim2) * c1_dim1];
// L108:
	    }
// L109:
	}
// L110:
    }
L111:
    if (nbd < *l1) {
	goto L115;
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    i__3 = *ido;
	    for (i = 3; i <= i__3; i += 2) {
		c1[i - 1 + (k + j * c1_dim2) * c1_dim1] = ch[i - 1 + (k + j * 
			ch_dim2) * ch_dim1] + ch[i - 1 + (k + jc * ch_dim2) * 
			ch_dim1];
		c1[i - 1 + (k + jc * c1_dim2) * c1_dim1] = ch[i + (k + j * 
			ch_dim2) * ch_dim1] - ch[i + (k + jc * ch_dim2) * 
			ch_dim1];
		c1[i + (k + j * c1_dim2) * c1_dim1] = ch[i + (k + j * ch_dim2)
			 * ch_dim1] + ch[i + (k + jc * ch_dim2) * ch_dim1];
		c1[i + (k + jc * c1_dim2) * c1_dim1] = ch[i - 1 + (k + jc * 
			ch_dim2) * ch_dim1] - ch[i - 1 + (k + j * ch_dim2) * 
			ch_dim1];
// L112:
	    }
// L113:
	}
// L114:
    }
    goto L121;
L115:
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    i__3 = *l1;
	    for (k = 1; k <= i__3; ++k) {
		c1[i - 1 + (k + j * c1_dim2) * c1_dim1] = ch[i - 1 + (k + j * 
			ch_dim2) * ch_dim1] + ch[i - 1 + (k + jc * ch_dim2) * 
			ch_dim1];
		c1[i - 1 + (k + jc * c1_dim2) * c1_dim1] = ch[i + (k + j * 
			ch_dim2) * ch_dim1] - ch[i + (k + jc * ch_dim2) * 
			ch_dim1];
		c1[i + (k + j * c1_dim2) * c1_dim1] = ch[i + (k + j * ch_dim2)
			 * ch_dim1] + ch[i + (k + jc * ch_dim2) * ch_dim1];
		c1[i + (k + jc * c1_dim2) * c1_dim1] = ch[i - 1 + (k + jc * 
			ch_dim2) * ch_dim1] - ch[i - 1 + (k + j * ch_dim2) * 
			ch_dim1];
// L116:
	    }
// L117:
	}
// L118:
    }
    goto L121;
L119:
    i__1 = *idl1;
    for (ik = 1; ik <= i__1; ++ik) {
	c2[ik + c2_dim1] = ch2[ik + ch2_dim1];
// L120:
    }
L121:
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    c1[(k + j * c1_dim2) * c1_dim1 + 1] = ch[(k + j * ch_dim2) * 
		    ch_dim1 + 1] + ch[(k + jc * ch_dim2) * ch_dim1 + 1];
	    c1[(k + jc * c1_dim2) * c1_dim1 + 1] = ch[(k + jc * ch_dim2) * 
		    ch_dim1 + 1] - ch[(k + j * ch_dim2) * ch_dim1 + 1];
// L122:
	}
// L123:
    }

    ar1 = (float)1.;
    ai1 = (float)0.;
    i__1 = ipph;
    for (l = 2; l <= i__1; ++l) {
	lc = ipp2 - l;
	ar1h = dcp * ar1 - dsp * ai1;
	ai1 = dcp * ai1 + dsp * ar1;
	ar1 = ar1h;
	i__2 = *idl1;
	for (ik = 1; ik <= i__2; ++ik) {
	    ch2[ik + l * ch2_dim1] = c2[ik + c2_dim1] + ar1 * c2[ik + (
		    c2_dim1 << 1)];
	    ch2[ik + lc * ch2_dim1] = ai1 * c2[ik + *ip * c2_dim1];
// L124:
	}
	dc2 = ar1;
	ds2 = ai1;
	ar2 = ar1;
	ai2 = ai1;
	i__2 = ipph;
	for (j = 3; j <= i__2; ++j) {
	    jc = ipp2 - j;
	    ar2h = dc2 * ar2 - ds2 * ai2;
	    ai2 = dc2 * ai2 + ds2 * ar2;
	    ar2 = ar2h;
	    i__3 = *idl1;
	    for (ik = 1; ik <= i__3; ++ik) {
		ch2[ik + l * ch2_dim1] += ar2 * c2[ik + j * c2_dim1];
		ch2[ik + lc * ch2_dim1] += ai2 * c2[ik + jc * c2_dim1];
// L125:
	    }
// L126:
	}
// L127:
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	i__2 = *idl1;
	for (ik = 1; ik <= i__2; ++ik) {
	    ch2[ik + ch2_dim1] += c2[ik + j * c2_dim1];
// L128:
	}
// L129:
    }

    if (*ido < *l1) {
	goto L132;
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i = 1; i <= i__2; ++i) {
	    cc[i + (k * cc_dim2 + 1) * cc_dim1] = ch[i + (k + ch_dim2) * 
		    ch_dim1];
// L130:
	}
// L131:
    }
    goto L135;
L132:
    i__1 = *ido;
    for (i = 1; i <= i__1; ++i) {
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    cc[i + (k * cc_dim2 + 1) * cc_dim1] = ch[i + (k + ch_dim2) * 
		    ch_dim1];
// L133:
	}
// L134:
    }
L135:
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	j2 = j + j;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    cc[*ido + (j2 - 2 + k * cc_dim2) * cc_dim1] = ch[(k + j * ch_dim2)
		     * ch_dim1 + 1];
	    cc[(j2 - 1 + k * cc_dim2) * cc_dim1 + 1] = ch[(k + jc * ch_dim2) *
		     ch_dim1 + 1];
// L136:
	}
// L137:
    }
    if (*ido == 1) {
	return 0;
    }
    if (nbd < *l1) {
	goto L141;
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	j2 = j + j;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    i__3 = *ido;
	    for (i = 3; i <= i__3; i += 2) {
		ic = idp2 - i;
		cc[i - 1 + (j2 - 1 + k * cc_dim2) * cc_dim1] = ch[i - 1 + (k 
			+ j * ch_dim2) * ch_dim1] + ch[i - 1 + (k + jc * 
			ch_dim2) * ch_dim1];
		cc[ic - 1 + (j2 - 2 + k * cc_dim2) * cc_dim1] = ch[i - 1 + (k 
			+ j * ch_dim2) * ch_dim1] - ch[i - 1 + (k + jc * 
			ch_dim2) * ch_dim1];
		cc[i + (j2 - 1 + k * cc_dim2) * cc_dim1] = ch[i + (k + j * 
			ch_dim2) * ch_dim1] + ch[i + (k + jc * ch_dim2) * 
			ch_dim1];
		cc[ic + (j2 - 2 + k * cc_dim2) * cc_dim1] = ch[i + (k + jc * 
			ch_dim2) * ch_dim1] - ch[i + (k + j * ch_dim2) * 
			ch_dim1];
// L138:
	    }
// L139:
	}
// L140:
    }
    return 0;
L141:
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	j2 = j + j;
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    ic = idp2 - i;
	    i__3 = *l1;
	    for (k = 1; k <= i__3; ++k) {
		cc[i - 1 + (j2 - 1 + k * cc_dim2) * cc_dim1] = ch[i - 1 + (k 
			+ j * ch_dim2) * ch_dim1] + ch[i - 1 + (k + jc * 
			ch_dim2) * ch_dim1];
		cc[ic - 1 + (j2 - 2 + k * cc_dim2) * cc_dim1] = ch[i - 1 + (k 
			+ j * ch_dim2) * ch_dim1] - ch[i - 1 + (k + jc * 
			ch_dim2) * ch_dim1];
		cc[i + (j2 - 1 + k * cc_dim2) * cc_dim1] = ch[i + (k + j * 
			ch_dim2) * ch_dim1] + ch[i + (k + jc * ch_dim2) * 
			ch_dim1];
		cc[ic + (j2 - 2 + k * cc_dim2) * cc_dim1] = ch[i + (k + jc * 
			ch_dim2) * ch_dim1] - ch[i + (k + j * ch_dim2) * 
			ch_dim1];
// L142:
	    }
// L143:
	}
// L144:
    }
    return 0;
} // radfg
//------------------------------------------------------------------------
int32_t rfftb(int32_t *n, float *r, float *wsave)
{
  int32_t rfftb1(int32_t *n, float *c, float *ch, float *wa, float *ifac);

  // Parameter adjustments
  --wsave;
  --r;

    // Function Body
    if (*n == 1) {
	return 0;
    }
    rfftb1(n, &r[1], &wsave[1], &wsave[*n + 1], &wsave[(*n << 1) + 1]);
    return 0;
} // rfftb
//------------------------------------------------------------------------
int32_t rfftb1(int32_t *n, float *c, float *ch, float *wa, float *ifac)
{
  // System generated locals
  int32_t i__1;

  // Local variables
  int32_t radb2(int32_t *ido, int32_t *l1, float *cc, float *ch, float *wa1),
    radb3(int32_t *ido, int32_t *l1, float *cc, float *ch, float *wa1, float *wa2),
    radb4(int32_t *ido, int32_t *l1, float *cc, float *ch, float *wa1,
	  float *wa2, float *wa3),
    radb5(int32_t *ido, int32_t *l1, float *cc, float *ch, float *wa1,
	  float *wa2, float *wa3, float *wa4);
  int32_t i;
  int32_t radbg(int32_t *ido, int32_t *ip, int32_t *l1, int32_t *idl1, float *cc, float *c1,
	  float *c2, float *ch, float *ch2, float *wa);
  int32_t k1, l1, l2, na, nf, ip, iw, ix2, ix3, ix4, ido, idl1;

    // Parameter adjustments
    --ifac;
    --wa;
    --ch;
    --c;

    // Function Body
    nf = ifac[2];
    na = 0;
    l1 = 1;
    iw = 1;
    i__1 = nf;
    for (k1 = 1; k1 <= i__1; ++k1) {
	ip = ifac[k1 + 2];
	l2 = ip * l1;
	ido = *n / l2;
	idl1 = ido * l1;
	if (ip != 4) {
	    goto L103;
	}
	ix2 = iw + ido;
	ix3 = ix2 + ido;
	if (na != 0) {
	    goto L101;
	}
	radb4(&ido, &l1, &c[1], &ch[1], &wa[iw], &wa[ix2], &wa[ix3]);
	goto L102;
L101:
	radb4(&ido, &l1, &ch[1], &c[1], &wa[iw], &wa[ix2], &wa[ix3]);
L102:
	na = 1 - na;
	goto L115;
L103:
	if (ip != 2) {
	    goto L106;
	}
	if (na != 0) {
	    goto L104;
	}
	radb2(&ido, &l1, &c[1], &ch[1], &wa[iw]);
	goto L105;
L104:
	radb2(&ido, &l1, &ch[1], &c[1], &wa[iw]);
L105:
	na = 1 - na;
	goto L115;
L106:
	if (ip != 3) {
	    goto L109;
	}
	ix2 = iw + ido;
	if (na != 0) {
	    goto L107;
	}
	radb3(&ido, &l1, &c[1], &ch[1], &wa[iw], &wa[ix2]);
	goto L108;
L107:
	radb3(&ido, &l1, &ch[1], &c[1], &wa[iw], &wa[ix2]);
L108:
	na = 1 - na;
	goto L115;
L109:
	if (ip != 5) {
	    goto L112;
	}
	ix2 = iw + ido;
	ix3 = ix2 + ido;
	ix4 = ix3 + ido;
	if (na != 0) {
	    goto L110;
	}
	radb5(&ido, &l1, &c[1], &ch[1], &wa[iw], &wa[ix2], &wa[ix3], &wa[ix4]
		);
	goto L111;
L110:
	radb5(&ido, &l1, &ch[1], &c[1], &wa[iw], &wa[ix2], &wa[ix3], &wa[ix4]
		);
L111:
	na = 1 - na;
	goto L115;
L112:
	if (na != 0) {
	    goto L113;
	}
	radbg(&ido, &ip, &l1, &idl1, &c[1], &c[1], &c[1], &ch[1], &ch[1], &
		wa[iw]);
	goto L114;
L113:
	radbg(&ido, &ip, &l1, &idl1, &ch[1], &ch[1], &ch[1], &c[1], &c[1], &
		wa[iw]);
L114:
	if (ido == 1) {
	    na = 1 - na;
	}
L115:
	l1 = l2;
	iw += (ip - 1) * ido;
// L116:
    }
    if (na == 0) {
	return 0;
    }
    i__1 = *n;
    for (i = 1; i <= i__1; ++i) {
	c[i] = ch[i];
// L117:
    }
    return 0;
} // rfftb1
//------------------------------------------------------------------------
int32_t rfftf(int32_t *n, float *r, float *wsave)
{
  int32_t rfftf1(int32_t *n, float *c, float *ch, float *wa, float *ifac);
  
  // Parameter adjustments
  --wsave;
  --r;
  
  // Function Body
  if (*n == 1) {
    return 0;
  }
  rfftf1(n, &r[1], &wsave[1], &wsave[*n + 1], &wsave[(*n << 1) + 1]);
  return 0;
} // rfftf
//------------------------------------------------------------------------
int32_t rfftf1(int32_t *n, float *c, float *ch, float *wa, float *ifac)
{
  // System generated locals
  int32_t i__1;

  // Local variables
  int32_t radf2(int32_t *ido, int32_t *l1, float *cc, float *ch, float *wa1),
    radf3(int32_t *ido, int32_t *l1, float *cc, float *ch, float *wa1, float *wa2),
    radf4(int32_t *ido, int32_t *l1, float *cc, float *ch, float *wa1,
	  float *wa2, float *wa3),
    radf5(int32_t *ido, int32_t *l1, float *cc, float *ch, float *wa1,
	  float *wa2, float *wa3, float *wa4);
  int32_t i;
  int32_t radfg(int32_t *ido, int32_t *ip, int32_t *l1, int32_t *idl1, float *cc, float *c1,
	  float *c2, float *ch, float *ch2, float *wa);
  int32_t k1, l1, l2, na, kh, nf, ip, iw, ix2, ix3, ix4, ido, idl1;

    // Parameter adjustments
    --ifac;
    --wa;
    --ch;
    --c;

    // Function Body
    nf = ifac[2];
    na = 1;
    l2 = *n;
    iw = *n;
    i__1 = nf;
    for (k1 = 1; k1 <= i__1; ++k1) {
	kh = nf - k1;
	ip = ifac[kh + 3];
	l1 = l2 / ip;
	ido = *n / l2;
	idl1 = ido * l1;
	iw -= (ip - 1) * ido;
	na = 1 - na;
	if (ip != 4) {
	    goto L102;
	}
	ix2 = iw + ido;
	ix3 = ix2 + ido;
	if (na != 0) {
	    goto L101;
	}
	radf4(&ido, &l1, &c[1], &ch[1], &wa[iw], &wa[ix2], &wa[ix3]);
	goto L110;
L101:
	radf4(&ido, &l1, &ch[1], &c[1], &wa[iw], &wa[ix2], &wa[ix3]);
	goto L110;
L102:
	if (ip != 2) {
	    goto L104;
	}
	if (na != 0) {
	    goto L103;
	}
	radf2(&ido, &l1, &c[1], &ch[1], &wa[iw]);
	goto L110;
L103:
	radf2(&ido, &l1, &ch[1], &c[1], &wa[iw]);
	goto L110;
L104:
	if (ip != 3) {
	    goto L106;
	}
	ix2 = iw + ido;
	if (na != 0) {
	    goto L105;
	}
	radf3(&ido, &l1, &c[1], &ch[1], &wa[iw], &wa[ix2]);
	goto L110;
L105:
	radf3(&ido, &l1, &ch[1], &c[1], &wa[iw], &wa[ix2]);
	goto L110;
L106:
	if (ip != 5) {
	    goto L108;
	}
	ix2 = iw + ido;
	ix3 = ix2 + ido;
	ix4 = ix3 + ido;
	if (na != 0) {
	    goto L107;
	}
	radf5(&ido, &l1, &c[1], &ch[1], &wa[iw], &wa[ix2], &wa[ix3], &wa[ix4]
		);
	goto L110;
L107:
	radf5(&ido, &l1, &ch[1], &c[1], &wa[iw], &wa[ix2], &wa[ix3], &wa[ix4]
		);
	goto L110;
L108:
	if (ido == 1) {
	    na = 1 - na;
	}
	if (na != 0) {
	    goto L109;
	}
	radfg(&ido, &ip, &l1, &idl1, &c[1], &c[1], &c[1], &ch[1], &ch[1], &
		wa[iw]);
	na = 1;
	goto L110;
L109:
	radfg(&ido, &ip, &l1, &idl1, &ch[1], &ch[1], &ch[1], &c[1], &c[1], &
		wa[iw]);
	na = 0;
L110:
	l2 = l1;
// L111:
    }
    if (na == 1) {
	return 0;
    }
    i__1 = *n;
    for (i = 1; i <= i__1; ++i) {
	c[i] = ch[i];
// L112:
    }
    return 0;
} // rfftf1
//------------------------------------------------------------------------
int32_t rffti(int32_t *n, float *wsave)
{
  int32_t rffti1(int32_t *n, float *wa, float *ifac);

  // Parameter adjustments
  --wsave;

    // Function Body
    if (*n == 1) {
	return 0;
    }
    rffti1(n, &wsave[*n + 1], &wsave[(*n << 1) + 1]);
    return 0;
} // rffti
//------------------------------------------------------------------------
int32_t rffti1(int32_t *n, float *wa, float *ifac)
{
  // Initialized data
  
  int32_t ntryh[4] = { 4,2,3,5 };
  
  // System generated locals
  int32_t i__1, i__2, i__3;
  
  // Builtin functions
  double cos(double), sin(double);
  
  // Local variables
  float argh;
  int32_t ntry, i, j;
  float argld;
  int32_t k1, l1, l2, ib;
  float fi;
  int32_t ld, ii, nf, ip, nl, is, nq, nr;
  float arg;
  int32_t ido, ipm;
  float tpi;
  int32_t nfm1;

    // Parameter adjustments
    --ifac;
    --wa;

    // Function Body
    nl = *n;
    nf = 0;
    j = 0;
L101:
    ++j;
    if (j - 4 <= 0) {
	goto L102;
    } else {
	goto L103;
    }
L102:
    ntry = ntryh[j - 1];
    goto L104;
L103:
    ntry += 2;
L104:
    nq = nl / ntry;
    nr = nl - ntry * nq;
    if (nr != 0) {
	goto L101;
    } else {
	goto L105;
    }
L105:
    ++nf;
    ifac[nf + 2] = ntry;
    nl = nq;
    if (ntry != 2) {
	goto L107;
    }
    if (nf == 1) {
	goto L107;
    }
    i__1 = nf;
    for (i = 2; i <= i__1; ++i) {
	ib = nf - i + 2;
	ifac[ib + 2] = ifac[ib + 1];
// L106:
    }
    ifac[3] = 2;
L107:
    if (nl != 1) {
	goto L104;
    }
    ifac[1] = *n;
    ifac[2] = nf;
    tpi = (float)6.28318530717959;
    argh = tpi / (float) (*n);
    is = 0;
    nfm1 = nf - 1;
    l1 = 1;
    if (nfm1 == 0) {
	return 0;
    }
    i__1 = nfm1;
    for (k1 = 1; k1 <= i__1; ++k1) {
	ip = ifac[k1 + 2];
	ld = 0;
	l2 = l1 * ip;
	ido = *n / l2;
	ipm = ip - 1;
	i__2 = ipm;
	for (j = 1; j <= i__2; ++j) {
	    ld += l1;
	    i = is;
	    argld = (float) ld * argh;
	    fi = (float)0.;
	    i__3 = ido;
	    for (ii = 3; ii <= i__3; ii += 2) {
		i += 2;
		fi += (float)1.;
		arg = fi * argld;
		wa[i - 1] = cos(arg);
		wa[i] = sin(arg);
// L108:
	    }
	    is += ido;
// L109:
	}
	l1 = l2;
// L110:
    }
    return 0;
} // rffti1
//------------------------------------------------------------------------
// Below follow routines for complex ffts
//------------------------------------------------------------------------
int32_t passf(int32_t *nac, int32_t *ido, int32_t *ip, int32_t *l1, int32_t *idl1,
	  float *cc, float *c1, float *c2, float *ch, float *ch2, float *wa)
{
    // System generated locals
    int32_t ch_dim1, ch_dim2, ch_offset, cc_dim1, cc_dim2, cc_offset, c1_dim1,
	     c1_dim2, c1_offset, c2_dim1, c2_offset, ch2_dim1, ch2_offset, 
	    i__1, i__2, i__3;

    // Local variables
    int32_t idij, idlj, idot, ipph, i__, j, k, l, jc, lc, ik, idj, 
	    idl, inc, idp;
    double wai, war;
    int32_t ipp2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    c1_dim1 = *ido;
    c1_dim2 = *l1;
    c1_offset = c1_dim1 * (c1_dim2 + 1) + 1;
    c1 -= c1_offset;
    cc_dim1 = *ido;
    cc_dim2 = *ip;
    cc_offset = cc_dim1 * (cc_dim2 + 1) + 1;
    cc -= cc_offset;
    ch2_dim1 = *idl1;
    ch2_offset = ch2_dim1 + 1;
    ch2 -= ch2_offset;
    c2_dim1 = *idl1;
    c2_offset = c2_dim1 + 1;
    c2 -= c2_offset;
    --wa;

    // Function Body
    idot = *ido / 2;
    ipp2 = *ip + 2;
    ipph = (*ip + 1) / 2;
    idp = *ip * *ido;

    if (*ido < *l1) {
	goto L106;
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    i__3 = *ido;
	    for (i__ = 1; i__ <= i__3; ++i__) {
		ch[i__ + (k + j * ch_dim2) * ch_dim1] = cc[i__ + (j + k * 
			cc_dim2) * cc_dim1] + cc[i__ + (jc + k * cc_dim2) * 
			cc_dim1];
		ch[i__ + (k + jc * ch_dim2) * ch_dim1] = cc[i__ + (j + k * 
			cc_dim2) * cc_dim1] - cc[i__ + (jc + k * cc_dim2) * 
			cc_dim1];
// L101:
	    }
// L102:
	}
// L103:
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + (k * cc_dim2 + 1) * 
		    cc_dim1];
// L104:
	}
// L105:
    }
    goto L112;
L106:
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *ido;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    i__3 = *l1;
	    for (k = 1; k <= i__3; ++k) {
		ch[i__ + (k + j * ch_dim2) * ch_dim1] = cc[i__ + (j + k * 
			cc_dim2) * cc_dim1] + cc[i__ + (jc + k * cc_dim2) * 
			cc_dim1];
		ch[i__ + (k + jc * ch_dim2) * ch_dim1] = cc[i__ + (j + k * 
			cc_dim2) * cc_dim1] - cc[i__ + (jc + k * cc_dim2) * 
			cc_dim1];
// L107:
	    }
// L108:
	}
// L109:
    }
    i__1 = *ido;
    for (i__ = 1; i__ <= i__1; ++i__) {
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + (k * cc_dim2 + 1) * 
		    cc_dim1];
// L110:
	}
// L111:
    }
L112:
    idl = 2 - *ido;
    inc = 0;
    i__1 = ipph;
    for (l = 2; l <= i__1; ++l) {
	lc = ipp2 - l;
	idl += *ido;
	i__2 = *idl1;
	for (ik = 1; ik <= i__2; ++ik) {
	    c2[ik + l * c2_dim1] = ch2[ik + ch2_dim1] + wa[idl - 1] * ch2[ik 
		    + (ch2_dim1 << 1)];
	    c2[ik + lc * c2_dim1] = -wa[idl] * ch2[ik + *ip * ch2_dim1];
// L113:
	}
	idlj = idl;
	inc += *ido;
	i__2 = ipph;
	for (j = 3; j <= i__2; ++j) {
	    jc = ipp2 - j;
	    idlj += inc;
	    if (idlj > idp) {
		idlj -= idp;
	    }
	    war = wa[idlj - 1];
	    wai = wa[idlj];
	    i__3 = *idl1;
	    for (ik = 1; ik <= i__3; ++ik) {
		c2[ik + l * c2_dim1] += war * ch2[ik + j * ch2_dim1];
		c2[ik + lc * c2_dim1] -= wai * ch2[ik + jc * ch2_dim1];
// L114:
	    }
// L115:
	}
// L116:
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	i__2 = *idl1;
	for (ik = 1; ik <= i__2; ++ik) {
	    ch2[ik + ch2_dim1] += ch2[ik + j * ch2_dim1];
// L117:
	}
// L118:
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *idl1;
	for (ik = 2; ik <= i__2; ik += 2) {
	    ch2[ik - 1 + j * ch2_dim1] = c2[ik - 1 + j * c2_dim1] - c2[ik + 
		    jc * c2_dim1];
	    ch2[ik - 1 + jc * ch2_dim1] = c2[ik - 1 + j * c2_dim1] + c2[ik + 
		    jc * c2_dim1];
	    ch2[ik + j * ch2_dim1] = c2[ik + j * c2_dim1] + c2[ik - 1 + jc * 
		    c2_dim1];
	    ch2[ik + jc * ch2_dim1] = c2[ik + j * c2_dim1] - c2[ik - 1 + jc * 
		    c2_dim1];
// L119:
	}
// L120:
    }
    *nac = 1;
    if (*ido == 2) {
	return 0;
    }
    *nac = 0;
    i__1 = *idl1;
    for (ik = 1; ik <= i__1; ++ik) {
	c2[ik + c2_dim1] = ch2[ik + ch2_dim1];
// L121:
    }
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    c1[(k + j * c1_dim2) * c1_dim1 + 1] = ch[(k + j * ch_dim2) * 
		    ch_dim1 + 1];
	    c1[(k + j * c1_dim2) * c1_dim1 + 2] = ch[(k + j * ch_dim2) * 
		    ch_dim1 + 2];
// L122:
	}
// L123:
    }
    if (idot > *l1) {
	goto L127;
    }
    idij = 0;
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
	idij += 2;
	i__2 = *ido;
	for (i__ = 4; i__ <= i__2; i__ += 2) {
	    idij += 2;
	    i__3 = *l1;
	    for (k = 1; k <= i__3; ++k) {
		c1[i__ - 1 + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1] * ch[
			i__ - 1 + (k + j * ch_dim2) * ch_dim1] + wa[idij] * 
			ch[i__ + (k + j * ch_dim2) * ch_dim1];
		c1[i__ + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1] * ch[i__ 
			+ (k + j * ch_dim2) * ch_dim1] - wa[idij] * ch[i__ - 
			1 + (k + j * ch_dim2) * ch_dim1];
// L124:
	    }
// L125:
	}
// L126:
    }
    return 0;
L127:
    idj = 2 - *ido;
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
	idj += *ido;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    idij = idj;
	    i__3 = *ido;
	    for (i__ = 4; i__ <= i__3; i__ += 2) {
		idij += 2;
		c1[i__ - 1 + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1] * ch[
			i__ - 1 + (k + j * ch_dim2) * ch_dim1] + wa[idij] * 
			ch[i__ + (k + j * ch_dim2) * ch_dim1];
		c1[i__ + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1] * ch[i__ 
			+ (k + j * ch_dim2) * ch_dim1] - wa[idij] * ch[i__ - 
			1 + (k + j * ch_dim2) * ch_dim1];
// L128:
	    }
// L129:
	}
// L130:
    }
    return 0;
} // passf
//------------------------------------------------------------------------
int32_t passf2(int32_t *ido, int32_t *l1, float *cc, float *ch, float *wa1)
{
    // System generated locals
    int32_t cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

    // Local variables
    int32_t i__, k;
    double ti2, tr2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = cc_dim1 * 3 + 1;
    cc -= cc_offset;
    --wa1;

    // Function Body
    if (*ido > 2) {
	goto L102;
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ch[(k + ch_dim2) * ch_dim1 + 1] = cc[((k << 1) + 1) * cc_dim1 + 1] + 
		cc[((k << 1) + 2) * cc_dim1 + 1];
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = cc[((k << 1) + 1) * cc_dim1 
		+ 1] - cc[((k << 1) + 2) * cc_dim1 + 1];
	ch[(k + ch_dim2) * ch_dim1 + 2] = cc[((k << 1) + 1) * cc_dim1 + 2] + 
		cc[((k << 1) + 2) * cc_dim1 + 2];
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 2] = cc[((k << 1) + 1) * cc_dim1 
		+ 2] - cc[((k << 1) + 2) * cc_dim1 + 2];
// L101:
    }
    return 0;
L102:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i__ = 2; i__ <= i__2; i__ += 2) {
	    ch[i__ - 1 + (k + ch_dim2) * ch_dim1] = cc[i__ - 1 + ((k << 1) + 
		    1) * cc_dim1] + cc[i__ - 1 + ((k << 1) + 2) * cc_dim1];
	    tr2 = cc[i__ - 1 + ((k << 1) + 1) * cc_dim1] - cc[i__ - 1 + ((k <<
		     1) + 2) * cc_dim1];
	    ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + ((k << 1) + 1) * 
		    cc_dim1] + cc[i__ + ((k << 1) + 2) * cc_dim1];
	    ti2 = cc[i__ + ((k << 1) + 1) * cc_dim1] - cc[i__ + ((k << 1) + 2)
		     * cc_dim1];
	    ch[i__ + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * ti2 - 
		    wa1[i__] * tr2;
	    ch[i__ - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * tr2 
		    + wa1[i__] * ti2;
// L103:
	}
// L104:
    }
    return 0;
} // passf2
//------------------------------------------------------------------------
int32_t passf3(int32_t *ido, int32_t *l1, float *cc, float *ch, float *wa1, float *wa2)
{
    // Initialized data

    double taur = (float)-.5;
    double taui = (float)-.866025403784439;

    // System generated locals
    int32_t cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

    // Local variables
    int32_t i__, k;
    double ci2, ci3, di2, di3, cr2, cr3, dr2, dr3, ti2, tr2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = (cc_dim1 << 2) + 1;
    cc -= cc_offset;
    --wa1;
    --wa2;

    // Function Body
    if (*ido != 2) {
	goto L102;
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	tr2 = cc[(k * 3 + 2) * cc_dim1 + 1] + cc[(k * 3 + 3) * cc_dim1 + 1];
	cr2 = cc[(k * 3 + 1) * cc_dim1 + 1] + taur * tr2;
	ch[(k + ch_dim2) * ch_dim1 + 1] = cc[(k * 3 + 1) * cc_dim1 + 1] + tr2;
	ti2 = cc[(k * 3 + 2) * cc_dim1 + 2] + cc[(k * 3 + 3) * cc_dim1 + 2];
	ci2 = cc[(k * 3 + 1) * cc_dim1 + 2] + taur * ti2;
	ch[(k + ch_dim2) * ch_dim1 + 2] = cc[(k * 3 + 1) * cc_dim1 + 2] + ti2;
	cr3 = taui * (cc[(k * 3 + 2) * cc_dim1 + 1] - cc[(k * 3 + 3) * 
		cc_dim1 + 1]);
	ci3 = taui * (cc[(k * 3 + 2) * cc_dim1 + 2] - cc[(k * 3 + 3) * 
		cc_dim1 + 2]);
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = cr2 - ci3;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 1] = cr2 + ci3;
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 2] = ci2 + cr3;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 2] = ci2 - cr3;
// L101:
    }
    return 0;
L102:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i__ = 2; i__ <= i__2; i__ += 2) {
	    tr2 = cc[i__ - 1 + (k * 3 + 2) * cc_dim1] + cc[i__ - 1 + (k * 3 + 
		    3) * cc_dim1];
	    cr2 = cc[i__ - 1 + (k * 3 + 1) * cc_dim1] + taur * tr2;
	    ch[i__ - 1 + (k + ch_dim2) * ch_dim1] = cc[i__ - 1 + (k * 3 + 1) *
		     cc_dim1] + tr2;
	    ti2 = cc[i__ + (k * 3 + 2) * cc_dim1] + cc[i__ + (k * 3 + 3) * 
		    cc_dim1];
	    ci2 = cc[i__ + (k * 3 + 1) * cc_dim1] + taur * ti2;
	    ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + (k * 3 + 1) * 
		    cc_dim1] + ti2;
	    cr3 = taui * (cc[i__ - 1 + (k * 3 + 2) * cc_dim1] - cc[i__ - 1 + (
		    k * 3 + 3) * cc_dim1]);
	    ci3 = taui * (cc[i__ + (k * 3 + 2) * cc_dim1] - cc[i__ + (k * 3 + 
		    3) * cc_dim1]);
	    dr2 = cr2 - ci3;
	    dr3 = cr2 + ci3;
	    di2 = ci2 + cr3;
	    di3 = ci2 - cr3;
	    ch[i__ + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * di2 - 
		    wa1[i__] * dr2;
	    ch[i__ - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * dr2 
		    + wa1[i__] * di2;
	    ch[i__ + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 1] * di3 - wa2[
		    i__] * dr3;
	    ch[i__ - 1 + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 1] * dr3 + 
		    wa2[i__] * di3;
// L103:
	}
// L104:
    }
    return 0;
} // passf3
//------------------------------------------------------------------------
int32_t passf4(int32_t *ido, int32_t *l1, float *cc, float *ch, float *wa1, float *wa2,
	   float *wa3)
{
    // System generated locals
    int32_t cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

    // Local variables
    int32_t i__, k;
    double ci2, ci3, ci4, cr2, cr3, cr4, ti1, ti2, ti3, ti4, tr1, tr2, 
	    tr3, tr4;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = cc_dim1 * 5 + 1;
    cc -= cc_offset;
    --wa1;
    --wa2;
    --wa3;

    // Function Body
    if (*ido != 2) {
	goto L102;
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ti1 = cc[((k << 2) + 1) * cc_dim1 + 2] - cc[((k << 2) + 3) * cc_dim1 
		+ 2];
	ti2 = cc[((k << 2) + 1) * cc_dim1 + 2] + cc[((k << 2) + 3) * cc_dim1 
		+ 2];
	tr4 = cc[((k << 2) + 2) * cc_dim1 + 2] - cc[((k << 2) + 4) * cc_dim1 
		+ 2];
	ti3 = cc[((k << 2) + 2) * cc_dim1 + 2] + cc[((k << 2) + 4) * cc_dim1 
		+ 2];
	tr1 = cc[((k << 2) + 1) * cc_dim1 + 1] - cc[((k << 2) + 3) * cc_dim1 
		+ 1];
	tr2 = cc[((k << 2) + 1) * cc_dim1 + 1] + cc[((k << 2) + 3) * cc_dim1 
		+ 1];
	ti4 = cc[((k << 2) + 4) * cc_dim1 + 1] - cc[((k << 2) + 2) * cc_dim1 
		+ 1];
	tr3 = cc[((k << 2) + 2) * cc_dim1 + 1] + cc[((k << 2) + 4) * cc_dim1 
		+ 1];
	ch[(k + ch_dim2) * ch_dim1 + 1] = tr2 + tr3;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 1] = tr2 - tr3;
	ch[(k + ch_dim2) * ch_dim1 + 2] = ti2 + ti3;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 2] = ti2 - ti3;
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = tr1 + tr4;
	ch[(k + (ch_dim2 << 2)) * ch_dim1 + 1] = tr1 - tr4;
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 2] = ti1 + ti4;
	ch[(k + (ch_dim2 << 2)) * ch_dim1 + 2] = ti1 - ti4;
// L101:
    }
    return 0;
L102:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i__ = 2; i__ <= i__2; i__ += 2) {
	    ti1 = cc[i__ + ((k << 2) + 1) * cc_dim1] - cc[i__ + ((k << 2) + 3)
		     * cc_dim1];
	    ti2 = cc[i__ + ((k << 2) + 1) * cc_dim1] + cc[i__ + ((k << 2) + 3)
		     * cc_dim1];
	    ti3 = cc[i__ + ((k << 2) + 2) * cc_dim1] + cc[i__ + ((k << 2) + 4)
		     * cc_dim1];
	    tr4 = cc[i__ + ((k << 2) + 2) * cc_dim1] - cc[i__ + ((k << 2) + 4)
		     * cc_dim1];
	    tr1 = cc[i__ - 1 + ((k << 2) + 1) * cc_dim1] - cc[i__ - 1 + ((k <<
		     2) + 3) * cc_dim1];
	    tr2 = cc[i__ - 1 + ((k << 2) + 1) * cc_dim1] + cc[i__ - 1 + ((k <<
		     2) + 3) * cc_dim1];
	    ti4 = cc[i__ - 1 + ((k << 2) + 4) * cc_dim1] - cc[i__ - 1 + ((k <<
		     2) + 2) * cc_dim1];
	    tr3 = cc[i__ - 1 + ((k << 2) + 2) * cc_dim1] + cc[i__ - 1 + ((k <<
		     2) + 4) * cc_dim1];
	    ch[i__ - 1 + (k + ch_dim2) * ch_dim1] = tr2 + tr3;
	    cr3 = tr2 - tr3;
	    ch[i__ + (k + ch_dim2) * ch_dim1] = ti2 + ti3;
	    ci3 = ti2 - ti3;
	    cr2 = tr1 + tr4;
	    cr4 = tr1 - tr4;
	    ci2 = ti1 + ti4;
	    ci4 = ti1 - ti4;
	    ch[i__ - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * cr2 
		    + wa1[i__] * ci2;
	    ch[i__ + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * ci2 - 
		    wa1[i__] * cr2;
	    ch[i__ - 1 + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 1] * cr3 + 
		    wa2[i__] * ci3;
	    ch[i__ + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 1] * ci3 - wa2[
		    i__] * cr3;
	    ch[i__ - 1 + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i__ - 1] * cr4 
		    + wa3[i__] * ci4;
	    ch[i__ + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i__ - 1] * ci4 - 
		    wa3[i__] * cr4;
// L103:
	}
// L104:
    }
    return 0;
} // passf4
//------------------------------------------------------------------------
int32_t passf5(int32_t *ido, int32_t *l1, float *cc, float *ch, float *wa1, float *wa2,
	   float *wa3, float *wa4)
{
    // Initialized data

    double tr11 = (float).309016994374947;
    double ti11 = (float)-.951056516295154;
    double tr12 = (float)-.809016994374947;
    double ti12 = (float)-.587785252292473;

    // System generated locals
    int32_t cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

    // Local variables
    int32_t i__, k;
    double ci2, ci3, ci4, ci5, di3, di4, di5, di2, cr2, cr3, cr5, cr4, 
	    ti2, ti3, ti4, ti5, dr3, dr4, dr5, dr2, tr2, tr3, tr4, tr5;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = cc_dim1 * 6 + 1;
    cc -= cc_offset;
    --wa1;
    --wa2;
    --wa3;
    --wa4;

    // Function Body
    if (*ido != 2) {
	goto L102;
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ti5 = cc[(k * 5 + 2) * cc_dim1 + 2] - cc[(k * 5 + 5) * cc_dim1 + 2];
	ti2 = cc[(k * 5 + 2) * cc_dim1 + 2] + cc[(k * 5 + 5) * cc_dim1 + 2];
	ti4 = cc[(k * 5 + 3) * cc_dim1 + 2] - cc[(k * 5 + 4) * cc_dim1 + 2];
	ti3 = cc[(k * 5 + 3) * cc_dim1 + 2] + cc[(k * 5 + 4) * cc_dim1 + 2];
	tr5 = cc[(k * 5 + 2) * cc_dim1 + 1] - cc[(k * 5 + 5) * cc_dim1 + 1];
	tr2 = cc[(k * 5 + 2) * cc_dim1 + 1] + cc[(k * 5 + 5) * cc_dim1 + 1];
	tr4 = cc[(k * 5 + 3) * cc_dim1 + 1] - cc[(k * 5 + 4) * cc_dim1 + 1];
	tr3 = cc[(k * 5 + 3) * cc_dim1 + 1] + cc[(k * 5 + 4) * cc_dim1 + 1];
	ch[(k + ch_dim2) * ch_dim1 + 1] = cc[(k * 5 + 1) * cc_dim1 + 1] + tr2 
		+ tr3;
	ch[(k + ch_dim2) * ch_dim1 + 2] = cc[(k * 5 + 1) * cc_dim1 + 2] + ti2 
		+ ti3;
	cr2 = cc[(k * 5 + 1) * cc_dim1 + 1] + tr11 * tr2 + tr12 * tr3;
	ci2 = cc[(k * 5 + 1) * cc_dim1 + 2] + tr11 * ti2 + tr12 * ti3;
	cr3 = cc[(k * 5 + 1) * cc_dim1 + 1] + tr12 * tr2 + tr11 * tr3;
	ci3 = cc[(k * 5 + 1) * cc_dim1 + 2] + tr12 * ti2 + tr11 * ti3;
	cr5 = ti11 * tr5 + ti12 * tr4;
	ci5 = ti11 * ti5 + ti12 * ti4;
	cr4 = ti12 * tr5 - ti11 * tr4;
	ci4 = ti12 * ti5 - ti11 * ti4;
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = cr2 - ci5;
	ch[(k + ch_dim2 * 5) * ch_dim1 + 1] = cr2 + ci5;
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 2] = ci2 + cr5;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 2] = ci3 + cr4;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 1] = cr3 - ci4;
	ch[(k + (ch_dim2 << 2)) * ch_dim1 + 1] = cr3 + ci4;
	ch[(k + (ch_dim2 << 2)) * ch_dim1 + 2] = ci3 - cr4;
	ch[(k + ch_dim2 * 5) * ch_dim1 + 2] = ci2 - cr5;
// L101:
    }
    return 0;
L102:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i__ = 2; i__ <= i__2; i__ += 2) {
	    ti5 = cc[i__ + (k * 5 + 2) * cc_dim1] - cc[i__ + (k * 5 + 5) * 
		    cc_dim1];
	    ti2 = cc[i__ + (k * 5 + 2) * cc_dim1] + cc[i__ + (k * 5 + 5) * 
		    cc_dim1];
	    ti4 = cc[i__ + (k * 5 + 3) * cc_dim1] - cc[i__ + (k * 5 + 4) * 
		    cc_dim1];
	    ti3 = cc[i__ + (k * 5 + 3) * cc_dim1] + cc[i__ + (k * 5 + 4) * 
		    cc_dim1];
	    tr5 = cc[i__ - 1 + (k * 5 + 2) * cc_dim1] - cc[i__ - 1 + (k * 5 + 
		    5) * cc_dim1];
	    tr2 = cc[i__ - 1 + (k * 5 + 2) * cc_dim1] + cc[i__ - 1 + (k * 5 + 
		    5) * cc_dim1];
	    tr4 = cc[i__ - 1 + (k * 5 + 3) * cc_dim1] - cc[i__ - 1 + (k * 5 + 
		    4) * cc_dim1];
	    tr3 = cc[i__ - 1 + (k * 5 + 3) * cc_dim1] + cc[i__ - 1 + (k * 5 + 
		    4) * cc_dim1];
	    ch[i__ - 1 + (k + ch_dim2) * ch_dim1] = cc[i__ - 1 + (k * 5 + 1) *
		     cc_dim1] + tr2 + tr3;
	    ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + (k * 5 + 1) * 
		    cc_dim1] + ti2 + ti3;
	    cr2 = cc[i__ - 1 + (k * 5 + 1) * cc_dim1] + tr11 * tr2 + tr12 * 
		    tr3;
	    ci2 = cc[i__ + (k * 5 + 1) * cc_dim1] + tr11 * ti2 + tr12 * ti3;
	    cr3 = cc[i__ - 1 + (k * 5 + 1) * cc_dim1] + tr12 * tr2 + tr11 * 
		    tr3;
	    ci3 = cc[i__ + (k * 5 + 1) * cc_dim1] + tr12 * ti2 + tr11 * ti3;
	    cr5 = ti11 * tr5 + ti12 * tr4;
	    ci5 = ti11 * ti5 + ti12 * ti4;
	    cr4 = ti12 * tr5 - ti11 * tr4;
	    ci4 = ti12 * ti5 - ti11 * ti4;
	    dr3 = cr3 - ci4;
	    dr4 = cr3 + ci4;
	    di3 = ci3 + cr4;
	    di4 = ci3 - cr4;
	    dr5 = cr2 + ci5;
	    dr2 = cr2 - ci5;
	    di5 = ci2 - cr5;
	    di2 = ci2 + cr5;
	    ch[i__ - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * dr2 
		    + wa1[i__] * di2;
	    ch[i__ + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * di2 - 
		    wa1[i__] * dr2;
	    ch[i__ - 1 + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 1] * dr3 + 
		    wa2[i__] * di3;
	    ch[i__ + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 1] * di3 - wa2[
		    i__] * dr3;
	    ch[i__ - 1 + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i__ - 1] * dr4 
		    + wa3[i__] * di4;
	    ch[i__ + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i__ - 1] * di4 - 
		    wa3[i__] * dr4;
	    ch[i__ - 1 + (k + ch_dim2 * 5) * ch_dim1] = wa4[i__ - 1] * dr5 + 
		    wa4[i__] * di5;
	    ch[i__ + (k + ch_dim2 * 5) * ch_dim1] = wa4[i__ - 1] * di5 - wa4[
		    i__] * dr5;
// L103:
	}
// L104:
    }
    return 0;
} // passf5
//------------------------------------------------------------------------
int32_t cfftf1(int32_t *n, float *c__, float *ch, float *wa, float *ifac)
{
    // System generated locals
    int32_t i__1;

    // Local variables
    int32_t idot, i__;
    int32_t passf(int32_t *, int32_t *, int32_t *, int32_t *, int32_t *, float *, float *, float *,
	      float *, float *, float *);
    int32_t k1, l1, l2, n2;
    int32_t passf2(int32_t *, int32_t *, float *, float *, float *),
      passf3(int32_t *, int32_t *, float *, float *, float *, float *),
      passf4(int32_t *, int32_t *, float *, float *, float *, float *, float *),
      passf5(int32_t *, int32_t *, float *, float *, float *, float *, float *,
	     float *);
    int32_t na, nf, ip, iw, ix2, ix3, ix4, nac, ido, idl1;

    // Parameter adjustments
    --ifac;
    --wa;
    --ch;
    --c__;

    // Function Body
    nf = ifac[2];
    na = 0;
    l1 = 1;
    iw = 1;
    i__1 = nf;
    for (k1 = 1; k1 <= i__1; ++k1) {
	ip = ifac[k1 + 2];
	l2 = ip * l1;
	ido = *n / l2;
	idot = ido + ido;
	idl1 = idot * l1;
	if (ip != 4) {
	    goto L103;
	}
	ix2 = iw + idot;
	ix3 = ix2 + idot;
	if (na != 0) {
	    goto L101;
	}
	passf4(&idot, &l1, &c__[1], &ch[1], &wa[iw], &wa[ix2], &wa[ix3]);
	goto L102;
L101:
	passf4(&idot, &l1, &ch[1], &c__[1], &wa[iw], &wa[ix2], &wa[ix3]);
L102:
	na = 1 - na;
	goto L115;
L103:
	if (ip != 2) {
	    goto L106;
	}
	if (na != 0) {
	    goto L104;
	}
	passf2(&idot, &l1, &c__[1], &ch[1], &wa[iw]);
	goto L105;
L104:
	passf2(&idot, &l1, &ch[1], &c__[1], &wa[iw]);
L105:
	na = 1 - na;
	goto L115;
L106:
	if (ip != 3) {
	    goto L109;
	}
	ix2 = iw + idot;
	if (na != 0) {
	    goto L107;
	}
	passf3(&idot, &l1, &c__[1], &ch[1], &wa[iw], &wa[ix2]);
	goto L108;
L107:
	passf3(&idot, &l1, &ch[1], &c__[1], &wa[iw], &wa[ix2]);
L108:
	na = 1 - na;
	goto L115;
L109:
	if (ip != 5) {
	    goto L112;
	}
	ix2 = iw + idot;
	ix3 = ix2 + idot;
	ix4 = ix3 + idot;
	if (na != 0) {
	    goto L110;
	}
	passf5(&idot, &l1, &c__[1], &ch[1], &wa[iw], &wa[ix2], &wa[ix3], &wa[
		ix4]);
	goto L111;
L110:
	passf5(&idot, &l1, &ch[1], &c__[1], &wa[iw], &wa[ix2], &wa[ix3], &wa[
		ix4]);
L111:
	na = 1 - na;
	goto L115;
L112:
	if (na != 0) {
	    goto L113;
	}
	passf(&nac, &idot, &ip, &l1, &idl1, &c__[1], &c__[1], &c__[1], &ch[1]
		, &ch[1], &wa[iw]);
	goto L114;
L113:
	passf(&nac, &idot, &ip, &l1, &idl1, &ch[1], &ch[1], &ch[1], &c__[1], 
		&c__[1], &wa[iw]);
L114:
	if (nac != 0) {
	    na = 1 - na;
	}
L115:
	l1 = l2;
	iw += (ip - 1) * idot;
// L116:
    }
    if (na == 0) {
	return 0;
    }
    n2 = *n + *n;
    i__1 = n2;
    for (i__ = 1; i__ <= i__1; ++i__) {
	c__[i__] = ch[i__];
// L117:
    }
    return 0;
} // cfftf1
//------------------------------------------------------------------------
int32_t cfftf(int32_t *n, float *c__, float *wsave)
{
  int32_t cfftf1(int32_t *, float *, float *, float *, float *);
  int32_t iw1, iw2;

  // Parameter adjustments
  --wsave;
  --c__;

  // Function Body
  if (*n == 1) {
    return 0;
  }
  iw1 = *n + *n + 1;
  iw2 = iw1 + *n + *n;
  cfftf1(n, &c__[1], &wsave[1], &wsave[iw1], &wsave[iw2]);
  return 0;
} // cfftf
//------------------------------------------------------------------------
int32_t passb(int32_t *nac, int32_t *ido, int32_t *ip, int32_t *l1, int32_t *idl1,
	  float *cc, float *c1, float *c2, float *ch, float *ch2, float *wa)
{
    // System generated locals
    int32_t ch_dim1, ch_dim2, ch_offset, cc_dim1, cc_dim2, cc_offset, c1_dim1,
	     c1_dim2, c1_offset, c2_dim1, c2_offset, ch2_dim1, ch2_offset, 
	    i__1, i__2, i__3;

    // Local variables
    int32_t idij, idlj, idot, ipph, i__, j, k, l, jc, lc, ik, idj, 
	    idl, inc, idp;
    double wai, war;
    int32_t ipp2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    c1_dim1 = *ido;
    c1_dim2 = *l1;
    c1_offset = c1_dim1 * (c1_dim2 + 1) + 1;
    c1 -= c1_offset;
    cc_dim1 = *ido;
    cc_dim2 = *ip;
    cc_offset = cc_dim1 * (cc_dim2 + 1) + 1;
    cc -= cc_offset;
    ch2_dim1 = *idl1;
    ch2_offset = ch2_dim1 + 1;
    ch2 -= ch2_offset;
    c2_dim1 = *idl1;
    c2_offset = c2_dim1 + 1;
    c2 -= c2_offset;
    --wa;

    // Function Body
    idot = *ido / 2;
    ipp2 = *ip + 2;
    ipph = (*ip + 1) / 2;
    idp = *ip * *ido;

    if (*ido < *l1) {
	goto L106;
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    i__3 = *ido;
	    for (i__ = 1; i__ <= i__3; ++i__) {
		ch[i__ + (k + j * ch_dim2) * ch_dim1] = cc[i__ + (j + k * 
			cc_dim2) * cc_dim1] + cc[i__ + (jc + k * cc_dim2) * 
			cc_dim1];
		ch[i__ + (k + jc * ch_dim2) * ch_dim1] = cc[i__ + (j + k * 
			cc_dim2) * cc_dim1] - cc[i__ + (jc + k * cc_dim2) * 
			cc_dim1];
// L101:
	    }
// L102:
	}
// L103:
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + (k * cc_dim2 + 1) * 
		    cc_dim1];
// L104:
	}
// L105:
    }
    goto L112;
L106:
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *ido;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    i__3 = *l1;
	    for (k = 1; k <= i__3; ++k) {
		ch[i__ + (k + j * ch_dim2) * ch_dim1] = cc[i__ + (j + k * 
			cc_dim2) * cc_dim1] + cc[i__ + (jc + k * cc_dim2) * 
			cc_dim1];
		ch[i__ + (k + jc * ch_dim2) * ch_dim1] = cc[i__ + (j + k * 
			cc_dim2) * cc_dim1] - cc[i__ + (jc + k * cc_dim2) * 
			cc_dim1];
// L107:
	    }
// L108:
	}
// L109:
    }
    i__1 = *ido;
    for (i__ = 1; i__ <= i__1; ++i__) {
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + (k * cc_dim2 + 1) * 
		    cc_dim1];
// L110:
	}
// L111:
    }
L112:
    idl = 2 - *ido;
    inc = 0;
    i__1 = ipph;
    for (l = 2; l <= i__1; ++l) {
	lc = ipp2 - l;
	idl += *ido;
	i__2 = *idl1;
	for (ik = 1; ik <= i__2; ++ik) {
	    c2[ik + l * c2_dim1] = ch2[ik + ch2_dim1] + wa[idl - 1] * ch2[ik 
		    + (ch2_dim1 << 1)];
	    c2[ik + lc * c2_dim1] = wa[idl] * ch2[ik + *ip * ch2_dim1];
// L113:
	}
	idlj = idl;
	inc += *ido;
	i__2 = ipph;
	for (j = 3; j <= i__2; ++j) {
	    jc = ipp2 - j;
	    idlj += inc;
	    if (idlj > idp) {
		idlj -= idp;
	    }
	    war = wa[idlj - 1];
	    wai = wa[idlj];
	    i__3 = *idl1;
	    for (ik = 1; ik <= i__3; ++ik) {
		c2[ik + l * c2_dim1] += war * ch2[ik + j * ch2_dim1];
		c2[ik + lc * c2_dim1] += wai * ch2[ik + jc * ch2_dim1];
// L114:
	    }
// L115:
	}
// L116:
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	i__2 = *idl1;
	for (ik = 1; ik <= i__2; ++ik) {
	    ch2[ik + ch2_dim1] += ch2[ik + j * ch2_dim1];
// L117:
	}
// L118:
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *idl1;
	for (ik = 2; ik <= i__2; ik += 2) {
	    ch2[ik - 1 + j * ch2_dim1] = c2[ik - 1 + j * c2_dim1] - c2[ik + 
		    jc * c2_dim1];
	    ch2[ik - 1 + jc * ch2_dim1] = c2[ik - 1 + j * c2_dim1] + c2[ik + 
		    jc * c2_dim1];
	    ch2[ik + j * ch2_dim1] = c2[ik + j * c2_dim1] + c2[ik - 1 + jc * 
		    c2_dim1];
	    ch2[ik + jc * ch2_dim1] = c2[ik + j * c2_dim1] - c2[ik - 1 + jc * 
		    c2_dim1];
// L119:
	}
// L120:
    }
    *nac = 1;
    if (*ido == 2) {
	return 0;
    }
    *nac = 0;
    i__1 = *idl1;
    for (ik = 1; ik <= i__1; ++ik) {
	c2[ik + c2_dim1] = ch2[ik + ch2_dim1];
// L121:
    }
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    c1[(k + j * c1_dim2) * c1_dim1 + 1] = ch[(k + j * ch_dim2) * 
		    ch_dim1 + 1];
	    c1[(k + j * c1_dim2) * c1_dim1 + 2] = ch[(k + j * ch_dim2) * 
		    ch_dim1 + 2];
// L122:
	}
// L123:
    }
    if (idot > *l1) {
	goto L127;
    }
    idij = 0;
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
	idij += 2;
	i__2 = *ido;
	for (i__ = 4; i__ <= i__2; i__ += 2) {
	    idij += 2;
	    i__3 = *l1;
	    for (k = 1; k <= i__3; ++k) {
		c1[i__ - 1 + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1] * ch[
			i__ - 1 + (k + j * ch_dim2) * ch_dim1] - wa[idij] * 
			ch[i__ + (k + j * ch_dim2) * ch_dim1];
		c1[i__ + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1] * ch[i__ 
			+ (k + j * ch_dim2) * ch_dim1] + wa[idij] * ch[i__ - 
			1 + (k + j * ch_dim2) * ch_dim1];
// L124:
	    }
// L125:
	}
// L126:
    }
    return 0;
L127:
    idj = 2 - *ido;
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
	idj += *ido;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    idij = idj;
	    i__3 = *ido;
	    for (i__ = 4; i__ <= i__3; i__ += 2) {
		idij += 2;
		c1[i__ - 1 + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1] * ch[
			i__ - 1 + (k + j * ch_dim2) * ch_dim1] - wa[idij] * 
			ch[i__ + (k + j * ch_dim2) * ch_dim1];
		c1[i__ + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1] * ch[i__ 
			+ (k + j * ch_dim2) * ch_dim1] + wa[idij] * ch[i__ - 
			1 + (k + j * ch_dim2) * ch_dim1];
// L128:
	    }
// L129:
	}
// L130:
    }
    return 0;
} // passb
//------------------------------------------------------------------------
int32_t passb2(int32_t *ido, int32_t *l1, float *cc, float *ch, float *wa1)
{
    // System generated locals
    int32_t cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

    // Local variables
    int32_t i__, k;
    double ti2, tr2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = cc_dim1 * 3 + 1;
    cc -= cc_offset;
    --wa1;

    // Function Body
    if (*ido > 2) {
	goto L102;
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ch[(k + ch_dim2) * ch_dim1 + 1] = cc[((k << 1) + 1) * cc_dim1 + 1] + 
		cc[((k << 1) + 2) * cc_dim1 + 1];
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = cc[((k << 1) + 1) * cc_dim1 
		+ 1] - cc[((k << 1) + 2) * cc_dim1 + 1];
	ch[(k + ch_dim2) * ch_dim1 + 2] = cc[((k << 1) + 1) * cc_dim1 + 2] + 
		cc[((k << 1) + 2) * cc_dim1 + 2];
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 2] = cc[((k << 1) + 1) * cc_dim1 
		+ 2] - cc[((k << 1) + 2) * cc_dim1 + 2];
// L101:
    }
    return 0;
L102:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i__ = 2; i__ <= i__2; i__ += 2) {
	    ch[i__ - 1 + (k + ch_dim2) * ch_dim1] = cc[i__ - 1 + ((k << 1) + 
		    1) * cc_dim1] + cc[i__ - 1 + ((k << 1) + 2) * cc_dim1];
	    tr2 = cc[i__ - 1 + ((k << 1) + 1) * cc_dim1] - cc[i__ - 1 + ((k <<
		     1) + 2) * cc_dim1];
	    ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + ((k << 1) + 1) * 
		    cc_dim1] + cc[i__ + ((k << 1) + 2) * cc_dim1];
	    ti2 = cc[i__ + ((k << 1) + 1) * cc_dim1] - cc[i__ + ((k << 1) + 2)
		     * cc_dim1];
	    ch[i__ + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * ti2 + 
		    wa1[i__] * tr2;
	    ch[i__ - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * tr2 
		    - wa1[i__] * ti2;
// L103:
	}
// L104:
    }
    return 0;
} // passb2
//------------------------------------------------------------------------
int32_t passb3(int32_t *ido, int32_t *l1, float *cc, float *ch, float *wa1, float *wa2)
{
    // Initialized data

    double taur = (float)-.5;
    double taui = (float).866025403784439;

    // System generated locals
    int32_t cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

    // Local variables
    int32_t i__, k;
    double ci2, ci3, di2, di3, cr2, cr3, dr2, dr3, ti2, tr2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = (cc_dim1 << 2) + 1;
    cc -= cc_offset;
    --wa1;
    --wa2;

    // Function Body
    if (*ido != 2) {
	goto L102;
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	tr2 = cc[(k * 3 + 2) * cc_dim1 + 1] + cc[(k * 3 + 3) * cc_dim1 + 1];
	cr2 = cc[(k * 3 + 1) * cc_dim1 + 1] + taur * tr2;
	ch[(k + ch_dim2) * ch_dim1 + 1] = cc[(k * 3 + 1) * cc_dim1 + 1] + tr2;
	ti2 = cc[(k * 3 + 2) * cc_dim1 + 2] + cc[(k * 3 + 3) * cc_dim1 + 2];
	ci2 = cc[(k * 3 + 1) * cc_dim1 + 2] + taur * ti2;
	ch[(k + ch_dim2) * ch_dim1 + 2] = cc[(k * 3 + 1) * cc_dim1 + 2] + ti2;
	cr3 = taui * (cc[(k * 3 + 2) * cc_dim1 + 1] - cc[(k * 3 + 3) * 
		cc_dim1 + 1]);
	ci3 = taui * (cc[(k * 3 + 2) * cc_dim1 + 2] - cc[(k * 3 + 3) * 
		cc_dim1 + 2]);
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = cr2 - ci3;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 1] = cr2 + ci3;
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 2] = ci2 + cr3;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 2] = ci2 - cr3;
// L101:
    }
    return 0;
L102:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i__ = 2; i__ <= i__2; i__ += 2) {
	    tr2 = cc[i__ - 1 + (k * 3 + 2) * cc_dim1] + cc[i__ - 1 + (k * 3 + 
		    3) * cc_dim1];
	    cr2 = cc[i__ - 1 + (k * 3 + 1) * cc_dim1] + taur * tr2;
	    ch[i__ - 1 + (k + ch_dim2) * ch_dim1] = cc[i__ - 1 + (k * 3 + 1) *
		     cc_dim1] + tr2;
	    ti2 = cc[i__ + (k * 3 + 2) * cc_dim1] + cc[i__ + (k * 3 + 3) * 
		    cc_dim1];
	    ci2 = cc[i__ + (k * 3 + 1) * cc_dim1] + taur * ti2;
	    ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + (k * 3 + 1) * 
		    cc_dim1] + ti2;
	    cr3 = taui * (cc[i__ - 1 + (k * 3 + 2) * cc_dim1] - cc[i__ - 1 + (
		    k * 3 + 3) * cc_dim1]);
	    ci3 = taui * (cc[i__ + (k * 3 + 2) * cc_dim1] - cc[i__ + (k * 3 + 
		    3) * cc_dim1]);
	    dr2 = cr2 - ci3;
	    dr3 = cr2 + ci3;
	    di2 = ci2 + cr3;
	    di3 = ci2 - cr3;
	    ch[i__ + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * di2 + 
		    wa1[i__] * dr2;
	    ch[i__ - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * dr2 
		    - wa1[i__] * di2;
	    ch[i__ + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 1] * di3 + wa2[
		    i__] * dr3;
	    ch[i__ - 1 + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 1] * dr3 - 
		    wa2[i__] * di3;
// L103:
	}
// L104:
    }
    return 0;
} // passb3
//------------------------------------------------------------------------
int32_t passb4(int32_t *ido, int32_t *l1, float *cc, float *ch, float *wa1, float *wa2,
	   float *wa3)
{
    // System generated locals
    int32_t cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

    // Local variables
    int32_t i__, k;
    double ci2, ci3, ci4, cr2, cr3, cr4, ti1, ti2, ti3, ti4, tr1, tr2, 
	    tr3, tr4;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = cc_dim1 * 5 + 1;
    cc -= cc_offset;
    --wa1;
    --wa2;
    --wa3;

    // Function Body
    if (*ido != 2) {
	goto L102;
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ti1 = cc[((k << 2) + 1) * cc_dim1 + 2] - cc[((k << 2) + 3) * cc_dim1 
		+ 2];
	ti2 = cc[((k << 2) + 1) * cc_dim1 + 2] + cc[((k << 2) + 3) * cc_dim1 
		+ 2];
	tr4 = cc[((k << 2) + 4) * cc_dim1 + 2] - cc[((k << 2) + 2) * cc_dim1 
		+ 2];
	ti3 = cc[((k << 2) + 2) * cc_dim1 + 2] + cc[((k << 2) + 4) * cc_dim1 
		+ 2];
	tr1 = cc[((k << 2) + 1) * cc_dim1 + 1] - cc[((k << 2) + 3) * cc_dim1 
		+ 1];
	tr2 = cc[((k << 2) + 1) * cc_dim1 + 1] + cc[((k << 2) + 3) * cc_dim1 
		+ 1];
	ti4 = cc[((k << 2) + 2) * cc_dim1 + 1] - cc[((k << 2) + 4) * cc_dim1 
		+ 1];
	tr3 = cc[((k << 2) + 2) * cc_dim1 + 1] + cc[((k << 2) + 4) * cc_dim1 
		+ 1];
	ch[(k + ch_dim2) * ch_dim1 + 1] = tr2 + tr3;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 1] = tr2 - tr3;
	ch[(k + ch_dim2) * ch_dim1 + 2] = ti2 + ti3;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 2] = ti2 - ti3;
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = tr1 + tr4;
	ch[(k + (ch_dim2 << 2)) * ch_dim1 + 1] = tr1 - tr4;
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 2] = ti1 + ti4;
	ch[(k + (ch_dim2 << 2)) * ch_dim1 + 2] = ti1 - ti4;
// L101:
    }
    return 0;
L102:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i__ = 2; i__ <= i__2; i__ += 2) {
	    ti1 = cc[i__ + ((k << 2) + 1) * cc_dim1] - cc[i__ + ((k << 2) + 3)
		     * cc_dim1];
	    ti2 = cc[i__ + ((k << 2) + 1) * cc_dim1] + cc[i__ + ((k << 2) + 3)
		     * cc_dim1];
	    ti3 = cc[i__ + ((k << 2) + 2) * cc_dim1] + cc[i__ + ((k << 2) + 4)
		     * cc_dim1];
	    tr4 = cc[i__ + ((k << 2) + 4) * cc_dim1] - cc[i__ + ((k << 2) + 2)
		     * cc_dim1];
	    tr1 = cc[i__ - 1 + ((k << 2) + 1) * cc_dim1] - cc[i__ - 1 + ((k <<
		     2) + 3) * cc_dim1];
	    tr2 = cc[i__ - 1 + ((k << 2) + 1) * cc_dim1] + cc[i__ - 1 + ((k <<
		     2) + 3) * cc_dim1];
	    ti4 = cc[i__ - 1 + ((k << 2) + 2) * cc_dim1] - cc[i__ - 1 + ((k <<
		     2) + 4) * cc_dim1];
	    tr3 = cc[i__ - 1 + ((k << 2) + 2) * cc_dim1] + cc[i__ - 1 + ((k <<
		     2) + 4) * cc_dim1];
	    ch[i__ - 1 + (k + ch_dim2) * ch_dim1] = tr2 + tr3;
	    cr3 = tr2 - tr3;
	    ch[i__ + (k + ch_dim2) * ch_dim1] = ti2 + ti3;
	    ci3 = ti2 - ti3;
	    cr2 = tr1 + tr4;
	    cr4 = tr1 - tr4;
	    ci2 = ti1 + ti4;
	    ci4 = ti1 - ti4;
	    ch[i__ - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * cr2 
		    - wa1[i__] * ci2;
	    ch[i__ + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * ci2 + 
		    wa1[i__] * cr2;
	    ch[i__ - 1 + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 1] * cr3 - 
		    wa2[i__] * ci3;
	    ch[i__ + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 1] * ci3 + wa2[
		    i__] * cr3;
	    ch[i__ - 1 + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i__ - 1] * cr4 
		    - wa3[i__] * ci4;
	    ch[i__ + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i__ - 1] * ci4 + 
		    wa3[i__] * cr4;
// L103:
	}
// L104:
    }
    return 0;
} // passb4
//------------------------------------------------------------------------
int32_t passb5(int32_t *ido, int32_t *l1, float *cc, float *ch, float *wa1, float *wa2,
	   float *wa3, float *wa4)
{
    // Initialized data

    double tr11 = (float).309016994374947;
    double ti11 = (float).951056516295154;
    double tr12 = (float)-.809016994374947;
    double ti12 = (float).587785252292473;

    // System generated locals
    int32_t cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

    // Local variables
    int32_t i__, k;
    double ci2, ci3, ci4, ci5, di3, di4, di5, di2, cr2, cr3, cr5, cr4, 
	    ti2, ti3, ti4, ti5, dr3, dr4, dr5, dr2, tr2, tr3, tr4, tr5;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = cc_dim1 * 6 + 1;
    cc -= cc_offset;
    --wa1;
    --wa2;
    --wa3;
    --wa4;

    // Function Body
    if (*ido != 2) {
	goto L102;
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ti5 = cc[(k * 5 + 2) * cc_dim1 + 2] - cc[(k * 5 + 5) * cc_dim1 + 2];
	ti2 = cc[(k * 5 + 2) * cc_dim1 + 2] + cc[(k * 5 + 5) * cc_dim1 + 2];
	ti4 = cc[(k * 5 + 3) * cc_dim1 + 2] - cc[(k * 5 + 4) * cc_dim1 + 2];
	ti3 = cc[(k * 5 + 3) * cc_dim1 + 2] + cc[(k * 5 + 4) * cc_dim1 + 2];
	tr5 = cc[(k * 5 + 2) * cc_dim1 + 1] - cc[(k * 5 + 5) * cc_dim1 + 1];
	tr2 = cc[(k * 5 + 2) * cc_dim1 + 1] + cc[(k * 5 + 5) * cc_dim1 + 1];
	tr4 = cc[(k * 5 + 3) * cc_dim1 + 1] - cc[(k * 5 + 4) * cc_dim1 + 1];
	tr3 = cc[(k * 5 + 3) * cc_dim1 + 1] + cc[(k * 5 + 4) * cc_dim1 + 1];
	ch[(k + ch_dim2) * ch_dim1 + 1] = cc[(k * 5 + 1) * cc_dim1 + 1] + tr2 
		+ tr3;
	ch[(k + ch_dim2) * ch_dim1 + 2] = cc[(k * 5 + 1) * cc_dim1 + 2] + ti2 
		+ ti3;
	cr2 = cc[(k * 5 + 1) * cc_dim1 + 1] + tr11 * tr2 + tr12 * tr3;
	ci2 = cc[(k * 5 + 1) * cc_dim1 + 2] + tr11 * ti2 + tr12 * ti3;
	cr3 = cc[(k * 5 + 1) * cc_dim1 + 1] + tr12 * tr2 + tr11 * tr3;
	ci3 = cc[(k * 5 + 1) * cc_dim1 + 2] + tr12 * ti2 + tr11 * ti3;
	cr5 = ti11 * tr5 + ti12 * tr4;
	ci5 = ti11 * ti5 + ti12 * ti4;
	cr4 = ti12 * tr5 - ti11 * tr4;
	ci4 = ti12 * ti5 - ti11 * ti4;
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = cr2 - ci5;
	ch[(k + ch_dim2 * 5) * ch_dim1 + 1] = cr2 + ci5;
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 2] = ci2 + cr5;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 2] = ci3 + cr4;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 1] = cr3 - ci4;
	ch[(k + (ch_dim2 << 2)) * ch_dim1 + 1] = cr3 + ci4;
	ch[(k + (ch_dim2 << 2)) * ch_dim1 + 2] = ci3 - cr4;
	ch[(k + ch_dim2 * 5) * ch_dim1 + 2] = ci2 - cr5;
// L101:
    }
    return 0;
L102:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i__ = 2; i__ <= i__2; i__ += 2) {
	    ti5 = cc[i__ + (k * 5 + 2) * cc_dim1] - cc[i__ + (k * 5 + 5) * 
		    cc_dim1];
	    ti2 = cc[i__ + (k * 5 + 2) * cc_dim1] + cc[i__ + (k * 5 + 5) * 
		    cc_dim1];
	    ti4 = cc[i__ + (k * 5 + 3) * cc_dim1] - cc[i__ + (k * 5 + 4) * 
		    cc_dim1];
	    ti3 = cc[i__ + (k * 5 + 3) * cc_dim1] + cc[i__ + (k * 5 + 4) * 
		    cc_dim1];
	    tr5 = cc[i__ - 1 + (k * 5 + 2) * cc_dim1] - cc[i__ - 1 + (k * 5 + 
		    5) * cc_dim1];
	    tr2 = cc[i__ - 1 + (k * 5 + 2) * cc_dim1] + cc[i__ - 1 + (k * 5 + 
		    5) * cc_dim1];
	    tr4 = cc[i__ - 1 + (k * 5 + 3) * cc_dim1] - cc[i__ - 1 + (k * 5 + 
		    4) * cc_dim1];
	    tr3 = cc[i__ - 1 + (k * 5 + 3) * cc_dim1] + cc[i__ - 1 + (k * 5 + 
		    4) * cc_dim1];
	    ch[i__ - 1 + (k + ch_dim2) * ch_dim1] = cc[i__ - 1 + (k * 5 + 1) *
		     cc_dim1] + tr2 + tr3;
	    ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + (k * 5 + 1) * 
		    cc_dim1] + ti2 + ti3;
	    cr2 = cc[i__ - 1 + (k * 5 + 1) * cc_dim1] + tr11 * tr2 + tr12 * 
		    tr3;
	    ci2 = cc[i__ + (k * 5 + 1) * cc_dim1] + tr11 * ti2 + tr12 * ti3;
	    cr3 = cc[i__ - 1 + (k * 5 + 1) * cc_dim1] + tr12 * tr2 + tr11 * 
		    tr3;
	    ci3 = cc[i__ + (k * 5 + 1) * cc_dim1] + tr12 * ti2 + tr11 * ti3;
	    cr5 = ti11 * tr5 + ti12 * tr4;
	    ci5 = ti11 * ti5 + ti12 * ti4;
	    cr4 = ti12 * tr5 - ti11 * tr4;
	    ci4 = ti12 * ti5 - ti11 * ti4;
	    dr3 = cr3 - ci4;
	    dr4 = cr3 + ci4;
	    di3 = ci3 + cr4;
	    di4 = ci3 - cr4;
	    dr5 = cr2 + ci5;
	    dr2 = cr2 - ci5;
	    di5 = ci2 - cr5;
	    di2 = ci2 + cr5;
	    ch[i__ - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * dr2 
		    - wa1[i__] * di2;
	    ch[i__ + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * di2 + 
		    wa1[i__] * dr2;
	    ch[i__ - 1 + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 1] * dr3 - 
		    wa2[i__] * di3;
	    ch[i__ + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 1] * di3 + wa2[
		    i__] * dr3;
	    ch[i__ - 1 + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i__ - 1] * dr4 
		    - wa3[i__] * di4;
	    ch[i__ + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i__ - 1] * di4 + 
		    wa3[i__] * dr4;
	    ch[i__ - 1 + (k + ch_dim2 * 5) * ch_dim1] = wa4[i__ - 1] * dr5 - 
		    wa4[i__] * di5;
	    ch[i__ + (k + ch_dim2 * 5) * ch_dim1] = wa4[i__ - 1] * di5 + wa4[
		    i__] * dr5;
// L103:
	}
// L104:
    }
    return 0;
} // passb5
//------------------------------------------------------------------------
int32_t cfftb1(int32_t *n, float *c__, float *ch, float *wa, float *ifac)
{
    // System generated locals
    int32_t i__1;

    // Local variables
    int32_t idot, i__;
    int32_t passb(int32_t *, int32_t *, int32_t *, int32_t *, int32_t *, float *, float *, float *,
	      float *, float *, float *); 
    int32_t k1, l1, l2, n2;
    int32_t passb2(int32_t *, int32_t *, float *, float *, float *),
      passb3(int32_t *, int32_t *, float *, float *, float *, float *),
      passb4(int32_t *, int32_t *, float *, float *, float *, float *, float *),
      passb5(int32_t *, int32_t *, float *, float *, float *, float *, float *,
	     float *);
    int32_t na, nf, ip, iw, ix2, ix3, ix4, nac, ido, idl1;

    // Parameter adjustments
    --ifac;
    --wa;
    --ch;
    --c__;

    // Function Body
    nf = ifac[2];
    na = 0;
    l1 = 1;
    iw = 1;
    i__1 = nf;
    for (k1 = 1; k1 <= i__1; ++k1) {
	ip = ifac[k1 + 2];
	l2 = ip * l1;
	ido = *n / l2;
	idot = ido + ido;
	idl1 = idot * l1;
	if (ip != 4) {
	    goto L103;
	}
	ix2 = iw + idot;
	ix3 = ix2 + idot;
	if (na != 0) {
	    goto L101;
	}
	passb4(&idot, &l1, &c__[1], &ch[1], &wa[iw], &wa[ix2], &wa[ix3]);
	goto L102;
L101:
	passb4(&idot, &l1, &ch[1], &c__[1], &wa[iw], &wa[ix2], &wa[ix3]);
L102:
	na = 1 - na;
	goto L115;
L103:
	if (ip != 2) {
	    goto L106;
	}
	if (na != 0) {
	    goto L104;
	}
	passb2(&idot, &l1, &c__[1], &ch[1], &wa[iw]);
	goto L105;
L104:
	passb2(&idot, &l1, &ch[1], &c__[1], &wa[iw]);
L105:
	na = 1 - na;
	goto L115;
L106:
	if (ip != 3) {
	    goto L109;
	}
	ix2 = iw + idot;
	if (na != 0) {
	    goto L107;
	}
	passb3(&idot, &l1, &c__[1], &ch[1], &wa[iw], &wa[ix2]);
	goto L108;
L107:
	passb3(&idot, &l1, &ch[1], &c__[1], &wa[iw], &wa[ix2]);
L108:
	na = 1 - na;
	goto L115;
L109:
	if (ip != 5) {
	    goto L112;
	}
	ix2 = iw + idot;
	ix3 = ix2 + idot;
	ix4 = ix3 + idot;
	if (na != 0) {
	    goto L110;
	}
	passb5(&idot, &l1, &c__[1], &ch[1], &wa[iw], &wa[ix2], &wa[ix3], &wa[
		ix4]);
	goto L111;
L110:
	passb5(&idot, &l1, &ch[1], &c__[1], &wa[iw], &wa[ix2], &wa[ix3], &wa[
		ix4]);
L111:
	na = 1 - na;
	goto L115;
L112:
	if (na != 0) {
	    goto L113;
	}
	passb(&nac, &idot, &ip, &l1, &idl1, &c__[1], &c__[1], &c__[1], &ch[1]
		, &ch[1], &wa[iw]);
	goto L114;
L113:
	passb(&nac, &idot, &ip, &l1, &idl1, &ch[1], &ch[1], &ch[1], &c__[1], 
		&c__[1], &wa[iw]);
L114:
	if (nac != 0) {
	    na = 1 - na;
	}
L115:
	l1 = l2;
	iw += (ip - 1) * idot;
// L116:
    }
    if (na == 0) {
	return 0;
    }
    n2 = *n + *n;
    i__1 = n2;
    for (i__ = 1; i__ <= i__1; ++i__) {
	c__[i__] = ch[i__];
// L117:
    }
    return 0;
} // cfftb1
//------------------------------------------------------------------------
int32_t cfftb(int32_t *n, float *c__, float *wsave)
{
  int32_t cfftb1(int32_t *, float *, float *, float *, float *);
    int32_t iw1, iw2;

    // Parameter adjustments
    --wsave;
    --c__;

    // Function Body
    if (*n == 1) {
	return 0;
    }
    iw1 = *n + *n + 1;
    iw2 = iw1 + *n + *n;
    cfftb1(n, &c__[1], &wsave[1], &wsave[iw1], &wsave[iw2]);
    return 0;
} // cfftb
//------------------------------------------------------------------------
int32_t cffti1(int32_t *n, float *wa, float *ifac)
{
    // Initialized data

    int32_t ntryh[4] = { 3,4,2,5 };

    // System generated locals
    int32_t i__1, i__2, i__3;

    // Builtin functions
    double cos(double), sin(double);

    // Local variables
    double argh;
    int32_t idot, ntry, i__, j;
    double argld;
    int32_t i1, k1, l1, l2, ib;
    double fi;
    int32_t ld, ii, nf, ip, nl, nq, nr;
    double arg;
    int32_t ido, ipm;
    double tpi;

    // Parameter adjustments
    --ifac;
    --wa;

    // Function Body
    nl = *n;
    nf = 0;
    j = 0;
L101:
    ++j;
    if (j - 4 <= 0) {
	goto L102;
    } else {
	goto L103;
    }
L102:
    ntry = ntryh[j - 1];
    goto L104;
L103:
    ntry += 2;
L104:
    nq = nl / ntry;
    nr = nl - ntry * nq;
    if (nr != 0) {
	goto L101;
    } else {
	goto L105;
    }
L105:
    ++nf;
    ifac[nf + 2] = ntry;
    nl = nq;
    if (ntry != 2) {
	goto L107;
    }
    if (nf == 1) {
	goto L107;
    }
    i__1 = nf;
    for (i__ = 2; i__ <= i__1; ++i__) {
	ib = nf - i__ + 2;
	ifac[ib + 2] = ifac[ib + 1];
// L106:
    }
    ifac[3] = 2;
L107:
    if (nl != 1) {
	goto L104;
    }
    ifac[1] = *n;
    ifac[2] = nf;
    tpi = (float)6.28318530717959;
    argh = tpi / (double) (*n);
    i__ = 2;
    l1 = 1;
    i__1 = nf;
    for (k1 = 1; k1 <= i__1; ++k1) {
	ip = ifac[k1 + 2];
	ld = 0;
	l2 = l1 * ip;
	ido = *n / l2;
	idot = ido + ido + 2;
	ipm = ip - 1;
	i__2 = ipm;
	for (j = 1; j <= i__2; ++j) {
	    i1 = i__;
	    wa[i__ - 1] = (float)1.;
	    wa[i__] = (float)0.;
	    ld += l1;
	    fi = (float)0.;
	    argld = (double) ld * argh;
	    i__3 = idot;
	    for (ii = 4; ii <= i__3; ii += 2) {
		i__ += 2;
		fi += (float)1.;
		arg = fi * argld;
		wa[i__ - 1] = cos(arg);
		wa[i__] = sin(arg);
// L108:
	    }
	    if (ip <= 5) {
		goto L109;
	    }
	    wa[i1 - 1] = wa[i__ - 1];
	    wa[i1] = wa[i__];
L109:
	    ;
	}
	l1 = l2;
// L110:
    }
    return 0;
} // cffti1
//------------------------------------------------------------------------
int32_t cffti(int32_t *n, float *wsave)
{
  int32_t cffti1(int32_t *, float *, float *);
  int32_t iw1, iw2;

    // Parameter adjustments
    --wsave;

    // Function Body
    if (*n == 1) {
	return 0;
    }
    iw1 = *n + *n + 1;
    iw2 = iw1 + *n + *n;
    cffti1(n, &wsave[iw1], &wsave[iw2]);
    return 0;
} // cffti
//------------------------------------------------------------------------

/* Below follow the double-precision forms of the above routines; adapted
   from the foregoing by Louis Strous 15 June 1998 */

//------------------------------------------------------------------------
int32_t ezfft1d(int32_t *n, double *wa, double *ifac)
{
    // Initialized data

  int32_t ntryh[4] = { 4,2,3,5 };
  double tpi = (double)6.28318530717959;

  // System generated locals
  int32_t i__1, i__2, i__3;

  // Builtin functions
  double cos(double), sin(double);

  // Local variables
  double argh;
  int32_t ntry, i, j, k1, l1, l2, ib, ii, nf, ip, nl, is, nq, nr;
  double ch1, sh1;
  int32_t ido, ipm;
  double dch1, ch1h, arg1, dsh1;
  int32_t nfm1;

    // Parameter adjustments
    --ifac;
    --wa;

    // Function Body
    nl = *n;
    nf = 0;
    j = 0;
L101:
    ++j;
    if (j - 4 <= 0) {
	goto L102;
    } else {
	goto L103;
    }
L102:
    ntry = ntryh[j - 1];
    goto L104;
L103:
    ntry += 2;
L104:
    nq = nl / ntry;
    nr = nl - ntry * nq;
    if (nr != 0) {
	goto L101;
    } else {
	goto L105;
    }
L105:
    ++nf;
    ifac[nf + 2] = ntry;
    nl = nq;
    if (ntry != 2) {
	goto L107;
    }
    if (nf == 1) {
	goto L107;
    }
    i__1 = nf;
    for (i = 2; i <= i__1; ++i) {
	ib = nf - i + 2;
	ifac[ib + 2] = ifac[ib + 1];
// L106:
    }
    ifac[3] = 2;
L107:
    if (nl != 1) {
	goto L104;
    }
    ifac[1] = *n;
    ifac[2] = nf;
    argh = tpi / (double) (*n);
    is = 0;
    nfm1 = nf - 1;
    l1 = 1;
    if (nfm1 == 0) {
	return 0;
    }
    i__1 = nfm1;
    for (k1 = 1; k1 <= i__1; ++k1) {
	ip = ifac[k1 + 2];
	l2 = l1 * ip;
	ido = *n / l2;
	ipm = ip - 1;
	arg1 = (double) l1 * argh;
	ch1 = (double)1.;
	sh1 = (double)0.;
	dch1 = cos(arg1);
	dsh1 = sin(arg1);
	i__2 = ipm;
	for (j = 1; j <= i__2; ++j) {
	    ch1h = dch1 * ch1 - dsh1 * sh1;
	    sh1 = dch1 * sh1 + dsh1 * ch1;
	    ch1 = ch1h;
	    i = is + 2;
	    wa[i - 1] = ch1;
	    wa[i] = sh1;
	    if (ido < 5) {
		goto L109;
	    }
	    i__3 = ido;
	    for (ii = 5; ii <= i__3; ii += 2) {
		i += 2;
		wa[i - 1] = ch1 * wa[i - 3] - sh1 * wa[i - 2];
		wa[i] = ch1 * wa[i - 2] + sh1 * wa[i - 3];
// L108:
	    }
L109:
	    is += ido;
// L110:
	}
	l1 = l2;
// L111:
    }
    return 0;
} // ezfft1
//------------------------------------------------------------------------
int32_t ezfftbd(int32_t *n, double *r, double *azero, double *a, double *b, double *wsave)
{
  // System generated locals
  int32_t i__1;

  // Local variables
  int32_t i;
  int32_t rfftbd(int32_t *n, double *r, double *wsave);
  int32_t ns2;

    // Parameter adjustments
    --wsave;
    --b;
    --a;
    --r;

    // Function Body
    if ((i__1 = *n - 2) < 0) {
	goto L101;
    } else if (i__1 == 0) {
	goto L102;
    } else {
	goto L103;
    }
L101:
    r[1] = *azero;
    return 0;
L102:
    r[1] = *azero + a[1];
    r[2] = *azero - a[1];
    return 0;
L103:
    ns2 = (*n - 1) / 2;
    i__1 = ns2;
    for (i = 1; i <= i__1; ++i) {
	r[i * 2] = a[i] * (double).5;
	r[(i << 1) + 1] = b[i] * (double)-.5;
// L104:
    }
    r[1] = *azero;
    if (*n % 2 == 0) {
	r[*n] = a[ns2 + 1];
    }
    rfftbd(n, &r[1], &wsave[*n + 1]);
    return 0;
} // ezfftb
//------------------------------------------------------------------------
int32_t ezfftfd(int32_t *n, double *r, double *azero, double *a, double *b, double *wsave)
{
  // System generated locals
  int32_t i__1;

  // Local variables
  int32_t i;
  int32_t rfftfd(int32_t *n, double *r, double *wsave);
  double cf;
  int32_t ns2;
  double cfm;
  int32_t ns2m;


//                       VERSION 3  JUNE 1979

    // Parameter adjustments
    --wsave;
    --b;
    --a;
    --r;

    // Function Body
    if ((i__1 = *n - 2) < 0) {
	goto L101;
    } else if (i__1 == 0) {
	goto L102;
    } else {
	goto L103;
    }
L101:
    *azero = r[1];
    return 0;
L102:
    *azero = (r[1] + r[2]) * (double).5;
    a[1] = (r[1] - r[2]) * (double).5;
    return 0;
L103:
    i__1 = *n;
    for (i = 1; i <= i__1; ++i) {
	wsave[i] = r[i];
// L104:
    }
    rfftfd(n, &wsave[1], &wsave[*n + 1]);
    cf = (double)2. / (double) (*n);
    cfm = -cf;
    *azero = cf * (double).5 * wsave[1];
    ns2 = (*n + 1) / 2;
    ns2m = ns2 - 1;
    i__1 = ns2m;
    for (i = 1; i <= i__1; ++i) {
	a[i] = cf * wsave[i * 2];
	b[i] = cfm * wsave[(i << 1) + 1];
// L105:
    }
    if (*n % 2 == 1) {
	return 0;
    }
    a[ns2] = cf * (double).5 * wsave[*n];
    b[ns2] = (double)0.;
    return 0;
} // ezfftf
//------------------------------------------------------------------------
int32_t ezfftid(int32_t *n, double *wsave)
{
  int32_t ezfft1d(int32_t *n, double *wa, double *ifac);

  // Parameter adjustments
  --wsave;

    // Function Body
    if (*n == 1) {
	return 0;
    }
    ezfft1d(n, &wsave[(*n << 1) + 1], &wsave[*n * 3 + 1]);
    return 0;
} // ezffti
//------------------------------------------------------------------------
int32_t radb2d(int32_t *ido, int32_t *l1, double *cc, double *ch, double *wa1)
{
  // System generated locals
  int32_t cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

  // Local variables
  int32_t i, k, ic;
  double ti2, tr2;
  int32_t idp2;
  
  // Parameter adjustments
  ch_dim1 = *ido;
  ch_dim2 = *l1;
  ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
  ch -= ch_offset;
  cc_dim1 = *ido;
  cc_offset = cc_dim1 * 3 + 1;
  cc -= cc_offset;
  --wa1;

    // Function Body
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ch[(k + ch_dim2) * ch_dim1 + 1] = cc[((k << 1) + 1) * cc_dim1 + 1] + 
		cc[*ido + ((k << 1) + 2) * cc_dim1];
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = cc[((k << 1) + 1) * cc_dim1 
		+ 1] - cc[*ido + ((k << 1) + 2) * cc_dim1];
// L101:
    }
    if ((i__1 = *ido - 2) < 0) {
	goto L107;
    } else if (i__1 == 0) {
	goto L105;
    } else {
	goto L102;
    }
L102:
    idp2 = *ido + 2;
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    ic = idp2 - i;
	    ch[i - 1 + (k + ch_dim2) * ch_dim1] = cc[i - 1 + ((k << 1) + 1) * 
		    cc_dim1] + cc[ic - 1 + ((k << 1) + 2) * cc_dim1];
	    tr2 = cc[i - 1 + ((k << 1) + 1) * cc_dim1] - cc[ic - 1 + ((k << 1)
		     + 2) * cc_dim1];
	    ch[i + (k + ch_dim2) * ch_dim1] = cc[i + ((k << 1) + 1) * cc_dim1]
		     - cc[ic + ((k << 1) + 2) * cc_dim1];
	    ti2 = cc[i + ((k << 1) + 1) * cc_dim1] + cc[ic + ((k << 1) + 2) * 
		    cc_dim1];
	    ch[i - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i - 2] * tr2 - 
		    wa1[i - 1] * ti2;
	    ch[i + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i - 2] * ti2 + wa1[i 
		    - 1] * tr2;
// L103:
	}
// L104:
    }
    if (*ido % 2 == 1) {
	return 0;
    }
L105:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ch[*ido + (k + ch_dim2) * ch_dim1] = cc[*ido + ((k << 1) + 1) * 
		cc_dim1] + cc[*ido + ((k << 1) + 1) * cc_dim1];
	ch[*ido + (k + (ch_dim2 << 1)) * ch_dim1] = -(double)(cc[((k << 1)
		 + 2) * cc_dim1 + 1] + cc[((k << 1) + 2) * cc_dim1 + 1]);
// L106:
    }
L107:
    return 0;
} // radb2
//------------------------------------------------------------------------
int32_t radb3d(int32_t *ido, int32_t *l1, double *cc, double *ch, double *wa1, double *wa2)
{
  // Initialized data
  
  double taur = (double)-.5;
  double taui = (double).866025403784439;

  // System generated locals
  int32_t cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

  // Local variables
  int32_t i, k, ic;
  double ci2, ci3, di2, di3, cr2, cr3, dr2, dr3, ti2, tr2;
  int32_t idp2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = (cc_dim1 << 2) + 1;
    cc -= cc_offset;
    --wa1;
    --wa2;

    // Function Body
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	tr2 = cc[*ido + (k * 3 + 2) * cc_dim1] + cc[*ido + (k * 3 + 2) * 
		cc_dim1];
	cr2 = cc[(k * 3 + 1) * cc_dim1 + 1] + taur * tr2;
	ch[(k + ch_dim2) * ch_dim1 + 1] = cc[(k * 3 + 1) * cc_dim1 + 1] + tr2;
	ci3 = taui * (cc[(k * 3 + 3) * cc_dim1 + 1] + cc[(k * 3 + 3) * 
		cc_dim1 + 1]);
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = cr2 - ci3;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 1] = cr2 + ci3;
// L101:
    }
    if (*ido == 1) {
	return 0;
    }
    idp2 = *ido + 2;
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    ic = idp2 - i;
	    tr2 = cc[i - 1 + (k * 3 + 3) * cc_dim1] + cc[ic - 1 + (k * 3 + 2) 
		    * cc_dim1];
	    cr2 = cc[i - 1 + (k * 3 + 1) * cc_dim1] + taur * tr2;
	    ch[i - 1 + (k + ch_dim2) * ch_dim1] = cc[i - 1 + (k * 3 + 1) * 
		    cc_dim1] + tr2;
	    ti2 = cc[i + (k * 3 + 3) * cc_dim1] - cc[ic + (k * 3 + 2) * 
		    cc_dim1];
	    ci2 = cc[i + (k * 3 + 1) * cc_dim1] + taur * ti2;
	    ch[i + (k + ch_dim2) * ch_dim1] = cc[i + (k * 3 + 1) * cc_dim1] + 
		    ti2;
	    cr3 = taui * (cc[i - 1 + (k * 3 + 3) * cc_dim1] - cc[ic - 1 + (k *
		     3 + 2) * cc_dim1]);
	    ci3 = taui * (cc[i + (k * 3 + 3) * cc_dim1] + cc[ic + (k * 3 + 2) 
		    * cc_dim1]);
	    dr2 = cr2 - ci3;
	    dr3 = cr2 + ci3;
	    di2 = ci2 + cr3;
	    di3 = ci2 - cr3;
	    ch[i - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i - 2] * dr2 - 
		    wa1[i - 1] * di2;
	    ch[i + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i - 2] * di2 + wa1[i 
		    - 1] * dr2;
	    ch[i - 1 + (k + ch_dim2 * 3) * ch_dim1] = wa2[i - 2] * dr3 - wa2[
		    i - 1] * di3;
	    ch[i + (k + ch_dim2 * 3) * ch_dim1] = wa2[i - 2] * di3 + wa2[i - 
		    1] * dr3;
// L102:
	}
// L103:
    }
    return 0;
} // radb3
//------------------------------------------------------------------------
int32_t radb4d(int32_t *ido, int32_t *l1, double *cc, double *ch, double *wa1,
	  double *wa2, double *wa3)
{
  // Initialized data

  double sqrt2 = (double)1.414213562373095;

  // System generated locals
  int32_t cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

  // Local variables
  int32_t i, k, ic;
  double ci2, ci3, ci4, cr2, cr3, cr4, ti1, ti2, ti3, ti4, tr1, tr2, 
    tr3, tr4;
  int32_t idp2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = cc_dim1 * 5 + 1;
    cc -= cc_offset;
    --wa1;
    --wa2;
    --wa3;

    // Function Body
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	tr1 = cc[((k << 2) + 1) * cc_dim1 + 1] - cc[*ido + ((k << 2) + 4) * 
		cc_dim1];
	tr2 = cc[((k << 2) + 1) * cc_dim1 + 1] + cc[*ido + ((k << 2) + 4) * 
		cc_dim1];
	tr3 = cc[*ido + ((k << 2) + 2) * cc_dim1] + cc[*ido + ((k << 2) + 2) *
		 cc_dim1];
	tr4 = cc[((k << 2) + 3) * cc_dim1 + 1] + cc[((k << 2) + 3) * cc_dim1 
		+ 1];
	ch[(k + ch_dim2) * ch_dim1 + 1] = tr2 + tr3;
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = tr1 - tr4;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 1] = tr2 - tr3;
	ch[(k + (ch_dim2 << 2)) * ch_dim1 + 1] = tr1 + tr4;
// L101:
    }
    if ((i__1 = *ido - 2) < 0) {
	goto L107;
    } else if (i__1 == 0) {
	goto L105;
    } else {
	goto L102;
    }
L102:
    idp2 = *ido + 2;
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    ic = idp2 - i;
	    ti1 = cc[i + ((k << 2) + 1) * cc_dim1] + cc[ic + ((k << 2) + 4) * 
		    cc_dim1];
	    ti2 = cc[i + ((k << 2) + 1) * cc_dim1] - cc[ic + ((k << 2) + 4) * 
		    cc_dim1];
	    ti3 = cc[i + ((k << 2) + 3) * cc_dim1] - cc[ic + ((k << 2) + 2) * 
		    cc_dim1];
	    tr4 = cc[i + ((k << 2) + 3) * cc_dim1] + cc[ic + ((k << 2) + 2) * 
		    cc_dim1];
	    tr1 = cc[i - 1 + ((k << 2) + 1) * cc_dim1] - cc[ic - 1 + ((k << 2)
		     + 4) * cc_dim1];
	    tr2 = cc[i - 1 + ((k << 2) + 1) * cc_dim1] + cc[ic - 1 + ((k << 2)
		     + 4) * cc_dim1];
	    ti4 = cc[i - 1 + ((k << 2) + 3) * cc_dim1] - cc[ic - 1 + ((k << 2)
		     + 2) * cc_dim1];
	    tr3 = cc[i - 1 + ((k << 2) + 3) * cc_dim1] + cc[ic - 1 + ((k << 2)
		     + 2) * cc_dim1];
	    ch[i - 1 + (k + ch_dim2) * ch_dim1] = tr2 + tr3;
	    cr3 = tr2 - tr3;
	    ch[i + (k + ch_dim2) * ch_dim1] = ti2 + ti3;
	    ci3 = ti2 - ti3;
	    cr2 = tr1 - tr4;
	    cr4 = tr1 + tr4;
	    ci2 = ti1 + ti4;
	    ci4 = ti1 - ti4;
	    ch[i - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i - 2] * cr2 - 
		    wa1[i - 1] * ci2;
	    ch[i + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i - 2] * ci2 + wa1[i 
		    - 1] * cr2;
	    ch[i - 1 + (k + ch_dim2 * 3) * ch_dim1] = wa2[i - 2] * cr3 - wa2[
		    i - 1] * ci3;
	    ch[i + (k + ch_dim2 * 3) * ch_dim1] = wa2[i - 2] * ci3 + wa2[i - 
		    1] * cr3;
	    ch[i - 1 + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i - 2] * cr4 - 
		    wa3[i - 1] * ci4;
	    ch[i + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i - 2] * ci4 + wa3[i 
		    - 1] * cr4;
// L103:
	}
// L104:
    }
    if (*ido % 2 == 1) {
	return 0;
    }
L105:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ti1 = cc[((k << 2) + 2) * cc_dim1 + 1] + cc[((k << 2) + 4) * cc_dim1 
		+ 1];
	ti2 = cc[((k << 2) + 4) * cc_dim1 + 1] - cc[((k << 2) + 2) * cc_dim1 
		+ 1];
	tr1 = cc[*ido + ((k << 2) + 1) * cc_dim1] - cc[*ido + ((k << 2) + 3) *
		 cc_dim1];
	tr2 = cc[*ido + ((k << 2) + 1) * cc_dim1] + cc[*ido + ((k << 2) + 3) *
		 cc_dim1];
	ch[*ido + (k + ch_dim2) * ch_dim1] = tr2 + tr2;
	ch[*ido + (k + (ch_dim2 << 1)) * ch_dim1] = sqrt2 * (tr1 - ti1);
	ch[*ido + (k + ch_dim2 * 3) * ch_dim1] = ti2 + ti2;
	ch[*ido + (k + (ch_dim2 << 2)) * ch_dim1] = -(double)sqrt2 * (tr1 
		+ ti1);
// L106:
    }
L107:
    return 0;
} // radb4
//------------------------------------------------------------------------
int32_t radb5d(int32_t *ido, int32_t *l1, double *cc, double *ch, double *wa1,
	  double *wa2, double *wa3, double *wa4)
{
  // Initialized data
  
  double tr11 = (double).309016994374947;
  double ti11 = (double).951056516295154;
  double tr12 = (double)-.809016994374947;
  double ti12 = (double).587785252292473;
  
  // System generated locals
  int32_t cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;
  
  // Local variables
  int32_t i, k, ic;
  double ci2, ci3, ci4, ci5, di3, di4, di5, di2, cr2, cr3, cr5, cr4, 
    ti2, ti3, ti4, ti5, dr3, dr4, dr5, dr2, tr2, tr3, tr4, tr5;
  int32_t idp2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = cc_dim1 * 6 + 1;
    cc -= cc_offset;
    --wa1;
    --wa2;
    --wa3;
    --wa4;

    // Function Body
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ti5 = cc[(k * 5 + 3) * cc_dim1 + 1] + cc[(k * 5 + 3) * cc_dim1 + 1];
	ti4 = cc[(k * 5 + 5) * cc_dim1 + 1] + cc[(k * 5 + 5) * cc_dim1 + 1];
	tr2 = cc[*ido + (k * 5 + 2) * cc_dim1] + cc[*ido + (k * 5 + 2) * 
		cc_dim1];
	tr3 = cc[*ido + (k * 5 + 4) * cc_dim1] + cc[*ido + (k * 5 + 4) * 
		cc_dim1];
	ch[(k + ch_dim2) * ch_dim1 + 1] = cc[(k * 5 + 1) * cc_dim1 + 1] + tr2 
		+ tr3;
	cr2 = cc[(k * 5 + 1) * cc_dim1 + 1] + tr11 * tr2 + tr12 * tr3;
	cr3 = cc[(k * 5 + 1) * cc_dim1 + 1] + tr12 * tr2 + tr11 * tr3;
	ci5 = ti11 * ti5 + ti12 * ti4;
	ci4 = ti12 * ti5 - ti11 * ti4;
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = cr2 - ci5;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 1] = cr3 - ci4;
	ch[(k + (ch_dim2 << 2)) * ch_dim1 + 1] = cr3 + ci4;
	ch[(k + ch_dim2 * 5) * ch_dim1 + 1] = cr2 + ci5;
// L101:
    }
    if (*ido == 1) {
	return 0;
    }
    idp2 = *ido + 2;
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    ic = idp2 - i;
	    ti5 = cc[i + (k * 5 + 3) * cc_dim1] + cc[ic + (k * 5 + 2) * 
		    cc_dim1];
	    ti2 = cc[i + (k * 5 + 3) * cc_dim1] - cc[ic + (k * 5 + 2) * 
		    cc_dim1];
	    ti4 = cc[i + (k * 5 + 5) * cc_dim1] + cc[ic + (k * 5 + 4) * 
		    cc_dim1];
	    ti3 = cc[i + (k * 5 + 5) * cc_dim1] - cc[ic + (k * 5 + 4) * 
		    cc_dim1];
	    tr5 = cc[i - 1 + (k * 5 + 3) * cc_dim1] - cc[ic - 1 + (k * 5 + 2) 
		    * cc_dim1];
	    tr2 = cc[i - 1 + (k * 5 + 3) * cc_dim1] + cc[ic - 1 + (k * 5 + 2) 
		    * cc_dim1];
	    tr4 = cc[i - 1 + (k * 5 + 5) * cc_dim1] - cc[ic - 1 + (k * 5 + 4) 
		    * cc_dim1];
	    tr3 = cc[i - 1 + (k * 5 + 5) * cc_dim1] + cc[ic - 1 + (k * 5 + 4) 
		    * cc_dim1];
	    ch[i - 1 + (k + ch_dim2) * ch_dim1] = cc[i - 1 + (k * 5 + 1) * 
		    cc_dim1] + tr2 + tr3;
	    ch[i + (k + ch_dim2) * ch_dim1] = cc[i + (k * 5 + 1) * cc_dim1] + 
		    ti2 + ti3;
	    cr2 = cc[i - 1 + (k * 5 + 1) * cc_dim1] + tr11 * tr2 + tr12 * tr3;
	    ci2 = cc[i + (k * 5 + 1) * cc_dim1] + tr11 * ti2 + tr12 * ti3;
	    cr3 = cc[i - 1 + (k * 5 + 1) * cc_dim1] + tr12 * tr2 + tr11 * tr3;
	    ci3 = cc[i + (k * 5 + 1) * cc_dim1] + tr12 * ti2 + tr11 * ti3;
	    cr5 = ti11 * tr5 + ti12 * tr4;
	    ci5 = ti11 * ti5 + ti12 * ti4;
	    cr4 = ti12 * tr5 - ti11 * tr4;
	    ci4 = ti12 * ti5 - ti11 * ti4;
	    dr3 = cr3 - ci4;
	    dr4 = cr3 + ci4;
	    di3 = ci3 + cr4;
	    di4 = ci3 - cr4;
	    dr5 = cr2 + ci5;
	    dr2 = cr2 - ci5;
	    di5 = ci2 - cr5;
	    di2 = ci2 + cr5;
	    ch[i - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i - 2] * dr2 - 
		    wa1[i - 1] * di2;
	    ch[i + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i - 2] * di2 + wa1[i 
		    - 1] * dr2;
	    ch[i - 1 + (k + ch_dim2 * 3) * ch_dim1] = wa2[i - 2] * dr3 - wa2[
		    i - 1] * di3;
	    ch[i + (k + ch_dim2 * 3) * ch_dim1] = wa2[i - 2] * di3 + wa2[i - 
		    1] * dr3;
	    ch[i - 1 + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i - 2] * dr4 - 
		    wa3[i - 1] * di4;
	    ch[i + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i - 2] * di4 + wa3[i 
		    - 1] * dr4;
	    ch[i - 1 + (k + ch_dim2 * 5) * ch_dim1] = wa4[i - 2] * dr5 - wa4[
		    i - 1] * di5;
	    ch[i + (k + ch_dim2 * 5) * ch_dim1] = wa4[i - 2] * di5 + wa4[i - 
		    1] * dr5;
// L102:
	}
// L103:
    }
    return 0;
} // radb5
//------------------------------------------------------------------------
int32_t radbgd(int32_t *ido, int32_t *ip, int32_t *l1, int32_t *idl1, double *cc, double *c1,
	  double *c2, double *ch, double *ch2, double *wa)
{
  // Initialized data

  double tpi = (double)6.28318530717959;

  // System generated locals
  int32_t ch_dim1, ch_dim2, ch_offset, cc_dim1, cc_dim2, cc_offset, c1_dim1,
    c1_dim2, c1_offset, c2_dim1, c2_offset, ch2_dim1, ch2_offset, 
    i__1, i__2, i__3;

  // Builtin functions
  double cos(double), sin(double);

  // Local variables
  int32_t idij, ipph, i, j, k, l, j2, ic, jc, lc, ik, is;
  double dc2, ai1, ai2, ar1, ar2, ds2;
  int32_t nbd;
  double dcp, arg, dsp, ar1h, ar2h;
  int32_t idp2, ipp2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    c1_dim1 = *ido;
    c1_dim2 = *l1;
    c1_offset = c1_dim1 * (c1_dim2 + 1) + 1;
    c1 -= c1_offset;
    cc_dim1 = *ido;
    cc_dim2 = *ip;
    cc_offset = cc_dim1 * (cc_dim2 + 1) + 1;
    cc -= cc_offset;
    ch2_dim1 = *idl1;
    ch2_offset = ch2_dim1 + 1;
    ch2 -= ch2_offset;
    c2_dim1 = *idl1;
    c2_offset = c2_dim1 + 1;
    c2 -= c2_offset;
    --wa;

    // Function Body
    arg = tpi / (double) (*ip);
    dcp = cos(arg);
    dsp = sin(arg);
    idp2 = *ido + 2;
    nbd = (*ido - 1) / 2;
    ipp2 = *ip + 2;
    ipph = (*ip + 1) / 2;
    if (*ido < *l1) {
	goto L103;
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i = 1; i <= i__2; ++i) {
	    ch[i + (k + ch_dim2) * ch_dim1] = cc[i + (k * cc_dim2 + 1) * 
		    cc_dim1];
// L101:
	}
// L102:
    }
    goto L106;
L103:
    i__1 = *ido;
    for (i = 1; i <= i__1; ++i) {
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    ch[i + (k + ch_dim2) * ch_dim1] = cc[i + (k * cc_dim2 + 1) * 
		    cc_dim1];
// L104:
	}
// L105:
    }
L106:
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	j2 = j + j;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    ch[(k + j * ch_dim2) * ch_dim1 + 1] = cc[*ido + (j2 - 2 + k * 
		    cc_dim2) * cc_dim1] + cc[*ido + (j2 - 2 + k * cc_dim2) * 
		    cc_dim1];
	    ch[(k + jc * ch_dim2) * ch_dim1 + 1] = cc[(j2 - 1 + k * cc_dim2) *
		     cc_dim1 + 1] + cc[(j2 - 1 + k * cc_dim2) * cc_dim1 + 1];
// L107:
	}
// L108:
    }
    if (*ido == 1) {
	goto L116;
    }
    if (nbd < *l1) {
	goto L112;
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    i__3 = *ido;
	    for (i = 3; i <= i__3; i += 2) {
		ic = idp2 - i;
		ch[i - 1 + (k + j * ch_dim2) * ch_dim1] = cc[i - 1 + ((j << 1)
			 - 1 + k * cc_dim2) * cc_dim1] + cc[ic - 1 + ((j << 1)
			 - 2 + k * cc_dim2) * cc_dim1];
		ch[i - 1 + (k + jc * ch_dim2) * ch_dim1] = cc[i - 1 + ((j << 
			1) - 1 + k * cc_dim2) * cc_dim1] - cc[ic - 1 + ((j << 
			1) - 2 + k * cc_dim2) * cc_dim1];
		ch[i + (k + j * ch_dim2) * ch_dim1] = cc[i + ((j << 1) - 1 + 
			k * cc_dim2) * cc_dim1] - cc[ic + ((j << 1) - 2 + k * 
			cc_dim2) * cc_dim1];
		ch[i + (k + jc * ch_dim2) * ch_dim1] = cc[i + ((j << 1) - 1 + 
			k * cc_dim2) * cc_dim1] + cc[ic + ((j << 1) - 2 + k * 
			cc_dim2) * cc_dim1];
// L109:
	    }
// L110:
	}
// L111:
    }
    goto L116;
L112:
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    ic = idp2 - i;
	    i__3 = *l1;
	    for (k = 1; k <= i__3; ++k) {
		ch[i - 1 + (k + j * ch_dim2) * ch_dim1] = cc[i - 1 + ((j << 1)
			 - 1 + k * cc_dim2) * cc_dim1] + cc[ic - 1 + ((j << 1)
			 - 2 + k * cc_dim2) * cc_dim1];
		ch[i - 1 + (k + jc * ch_dim2) * ch_dim1] = cc[i - 1 + ((j << 
			1) - 1 + k * cc_dim2) * cc_dim1] - cc[ic - 1 + ((j << 
			1) - 2 + k * cc_dim2) * cc_dim1];
		ch[i + (k + j * ch_dim2) * ch_dim1] = cc[i + ((j << 1) - 1 + 
			k * cc_dim2) * cc_dim1] - cc[ic + ((j << 1) - 2 + k * 
			cc_dim2) * cc_dim1];
		ch[i + (k + jc * ch_dim2) * ch_dim1] = cc[i + ((j << 1) - 1 + 
			k * cc_dim2) * cc_dim1] + cc[ic + ((j << 1) - 2 + k * 
			cc_dim2) * cc_dim1];
// L113:
	    }
// L114:
	}
// L115:
    }
L116:
    ar1 = (double)1.;
    ai1 = (double)0.;
    i__1 = ipph;
    for (l = 2; l <= i__1; ++l) {
	lc = ipp2 - l;
	ar1h = dcp * ar1 - dsp * ai1;
	ai1 = dcp * ai1 + dsp * ar1;
	ar1 = ar1h;
	i__2 = *idl1;
	for (ik = 1; ik <= i__2; ++ik) {
	    c2[ik + l * c2_dim1] = ch2[ik + ch2_dim1] + ar1 * ch2[ik + (
		    ch2_dim1 << 1)];
	    c2[ik + lc * c2_dim1] = ai1 * ch2[ik + *ip * ch2_dim1];
// L117:
	}
	dc2 = ar1;
	ds2 = ai1;
	ar2 = ar1;
	ai2 = ai1;
	i__2 = ipph;
	for (j = 3; j <= i__2; ++j) {
	    jc = ipp2 - j;
	    ar2h = dc2 * ar2 - ds2 * ai2;
	    ai2 = dc2 * ai2 + ds2 * ar2;
	    ar2 = ar2h;
	    i__3 = *idl1;
	    for (ik = 1; ik <= i__3; ++ik) {
		c2[ik + l * c2_dim1] += ar2 * ch2[ik + j * ch2_dim1];
		c2[ik + lc * c2_dim1] += ai2 * ch2[ik + jc * ch2_dim1];
// L118:
	    }
// L119:
	}
// L120:
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	i__2 = *idl1;
	for (ik = 1; ik <= i__2; ++ik) {
	    ch2[ik + ch2_dim1] += ch2[ik + j * ch2_dim1];
// L121:
	}
// L122:
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    ch[(k + j * ch_dim2) * ch_dim1 + 1] = c1[(k + j * c1_dim2) * 
		    c1_dim1 + 1] - c1[(k + jc * c1_dim2) * c1_dim1 + 1];
	    ch[(k + jc * ch_dim2) * ch_dim1 + 1] = c1[(k + j * c1_dim2) * 
		    c1_dim1 + 1] + c1[(k + jc * c1_dim2) * c1_dim1 + 1];
// L123:
	}
// L124:
    }
    if (*ido == 1) {
	goto L132;
    }
    if (nbd < *l1) {
	goto L128;
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    i__3 = *ido;
	    for (i = 3; i <= i__3; i += 2) {
		ch[i - 1 + (k + j * ch_dim2) * ch_dim1] = c1[i - 1 + (k + j * 
			c1_dim2) * c1_dim1] - c1[i + (k + jc * c1_dim2) * 
			c1_dim1];
		ch[i - 1 + (k + jc * ch_dim2) * ch_dim1] = c1[i - 1 + (k + j *
			 c1_dim2) * c1_dim1] + c1[i + (k + jc * c1_dim2) * 
			c1_dim1];
		ch[i + (k + j * ch_dim2) * ch_dim1] = c1[i + (k + j * c1_dim2)
			 * c1_dim1] + c1[i - 1 + (k + jc * c1_dim2) * c1_dim1]
			;
		ch[i + (k + jc * ch_dim2) * ch_dim1] = c1[i + (k + j * 
			c1_dim2) * c1_dim1] - c1[i - 1 + (k + jc * c1_dim2) * 
			c1_dim1];
// L125:
	    }
// L126:
	}
// L127:
    }
    goto L132;
L128:
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    i__3 = *l1;
	    for (k = 1; k <= i__3; ++k) {
		ch[i - 1 + (k + j * ch_dim2) * ch_dim1] = c1[i - 1 + (k + j * 
			c1_dim2) * c1_dim1] - c1[i + (k + jc * c1_dim2) * 
			c1_dim1];
		ch[i - 1 + (k + jc * ch_dim2) * ch_dim1] = c1[i - 1 + (k + j *
			 c1_dim2) * c1_dim1] + c1[i + (k + jc * c1_dim2) * 
			c1_dim1];
		ch[i + (k + j * ch_dim2) * ch_dim1] = c1[i + (k + j * c1_dim2)
			 * c1_dim1] + c1[i - 1 + (k + jc * c1_dim2) * c1_dim1]
			;
		ch[i + (k + jc * ch_dim2) * ch_dim1] = c1[i + (k + j * 
			c1_dim2) * c1_dim1] - c1[i - 1 + (k + jc * c1_dim2) * 
			c1_dim1];
// L129:
	    }
// L130:
	}
// L131:
    }
L132:
    if (*ido == 1) {
	return 0;
    }
    i__1 = *idl1;
    for (ik = 1; ik <= i__1; ++ik) {
	c2[ik + c2_dim1] = ch2[ik + ch2_dim1];
// L133:
    }
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    c1[(k + j * c1_dim2) * c1_dim1 + 1] = ch[(k + j * ch_dim2) * 
		    ch_dim1 + 1];
// L134:
	}
// L135:
    }
    if (nbd > *l1) {
	goto L139;
    }
    is = -(*ido);
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
	is += *ido;
	idij = is;
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    idij += 2;
	    i__3 = *l1;
	    for (k = 1; k <= i__3; ++k) {
		c1[i - 1 + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1] * ch[i 
			- 1 + (k + j * ch_dim2) * ch_dim1] - wa[idij] * ch[i 
			+ (k + j * ch_dim2) * ch_dim1];
		c1[i + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1] * ch[i + (
			k + j * ch_dim2) * ch_dim1] + wa[idij] * ch[i - 1 + (
			k + j * ch_dim2) * ch_dim1];
// L136:
	    }
// L137:
	}
// L138:
    }
    goto L143;
L139:
    is = -(*ido);
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
	is += *ido;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    idij = is;
	    i__3 = *ido;
	    for (i = 3; i <= i__3; i += 2) {
		idij += 2;
		c1[i - 1 + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1] * ch[i 
			- 1 + (k + j * ch_dim2) * ch_dim1] - wa[idij] * ch[i 
			+ (k + j * ch_dim2) * ch_dim1];
		c1[i + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1] * ch[i + (
			k + j * ch_dim2) * ch_dim1] + wa[idij] * ch[i - 1 + (
			k + j * ch_dim2) * ch_dim1];
// L140:
	    }
// L141:
	}
// L142:
    }
L143:
    return 0;
} // radbg
//------------------------------------------------------------------------
int32_t radf2d(int32_t *ido, int32_t *l1, double *cc, double *ch, double *wa1)
{
  // System generated locals
  int32_t ch_dim1, ch_offset, cc_dim1, cc_dim2, cc_offset, i__1, i__2;

  // Local variables
  int32_t i, k, ic;
  double ti2, tr2;
  int32_t idp2;

  // Parameter adjustments
    ch_dim1 = *ido;
    ch_offset = ch_dim1 * 3 + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_dim2 = *l1;
    cc_offset = cc_dim1 * (cc_dim2 + 1) + 1;
    cc -= cc_offset;
    --wa1;

    // Function Body
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ch[((k << 1) + 1) * ch_dim1 + 1] = cc[(k + cc_dim2) * cc_dim1 + 1] + 
		cc[(k + (cc_dim2 << 1)) * cc_dim1 + 1];
	ch[*ido + ((k << 1) + 2) * ch_dim1] = cc[(k + cc_dim2) * cc_dim1 + 1] 
		- cc[(k + (cc_dim2 << 1)) * cc_dim1 + 1];
// L101:
    }
    if ((i__1 = *ido - 2) < 0) {
	goto L107;
    } else if (i__1 == 0) {
	goto L105;
    } else {
	goto L102;
    }
L102:
    idp2 = *ido + 2;
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    ic = idp2 - i;
	    tr2 = wa1[i - 2] * cc[i - 1 + (k + (cc_dim2 << 1)) * cc_dim1] + 
		    wa1[i - 1] * cc[i + (k + (cc_dim2 << 1)) * cc_dim1];
	    ti2 = wa1[i - 2] * cc[i + (k + (cc_dim2 << 1)) * cc_dim1] - wa1[i 
		    - 1] * cc[i - 1 + (k + (cc_dim2 << 1)) * cc_dim1];
	    ch[i + ((k << 1) + 1) * ch_dim1] = cc[i + (k + cc_dim2) * cc_dim1]
		     + ti2;
	    ch[ic + ((k << 1) + 2) * ch_dim1] = ti2 - cc[i + (k + cc_dim2) * 
		    cc_dim1];
	    ch[i - 1 + ((k << 1) + 1) * ch_dim1] = cc[i - 1 + (k + cc_dim2) * 
		    cc_dim1] + tr2;
	    ch[ic - 1 + ((k << 1) + 2) * ch_dim1] = cc[i - 1 + (k + cc_dim2) *
		     cc_dim1] - tr2;
// L103:
	}
// L104:
    }
    if (*ido % 2 == 1) {
	return 0;
    }
L105:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ch[((k << 1) + 2) * ch_dim1 + 1] = -(double)cc[*ido + (k + (
		cc_dim2 << 1)) * cc_dim1];
	ch[*ido + ((k << 1) + 1) * ch_dim1] = cc[*ido + (k + cc_dim2) * 
		cc_dim1];
// L106:
    }
L107:
    return 0;
} // radf2
//------------------------------------------------------------------------
int32_t radf3d(int32_t *ido, int32_t *l1, double *cc, double *ch, double *wa1, double *wa2)
{
  // Initialized data
  
  double taur = (double)-.5;
  double taui = (double).866025403784439;
  
  // System generated locals
  int32_t ch_dim1, ch_offset, cc_dim1, cc_dim2, cc_offset, i__1, i__2;
  
  // Local variables
  int32_t i, k, ic;
  double ci2, di2, di3, cr2, dr2, dr3, ti2, ti3, tr2, tr3;
  int32_t idp2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_offset = (ch_dim1 << 2) + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_dim2 = *l1;
    cc_offset = cc_dim1 * (cc_dim2 + 1) + 1;
    cc -= cc_offset;
    --wa1;
    --wa2;

    // Function Body
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	cr2 = cc[(k + (cc_dim2 << 1)) * cc_dim1 + 1] + cc[(k + cc_dim2 * 3) * 
		cc_dim1 + 1];
	ch[(k * 3 + 1) * ch_dim1 + 1] = cc[(k + cc_dim2) * cc_dim1 + 1] + cr2;
	ch[(k * 3 + 3) * ch_dim1 + 1] = taui * (cc[(k + cc_dim2 * 3) * 
		cc_dim1 + 1] - cc[(k + (cc_dim2 << 1)) * cc_dim1 + 1]);
	ch[*ido + (k * 3 + 2) * ch_dim1] = cc[(k + cc_dim2) * cc_dim1 + 1] + 
		taur * cr2;
// L101:
    }
    if (*ido == 1) {
	return 0;
    }
    idp2 = *ido + 2;
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    ic = idp2 - i;
	    dr2 = wa1[i - 2] * cc[i - 1 + (k + (cc_dim2 << 1)) * cc_dim1] + 
		    wa1[i - 1] * cc[i + (k + (cc_dim2 << 1)) * cc_dim1];
	    di2 = wa1[i - 2] * cc[i + (k + (cc_dim2 << 1)) * cc_dim1] - wa1[i 
		    - 1] * cc[i - 1 + (k + (cc_dim2 << 1)) * cc_dim1];
	    dr3 = wa2[i - 2] * cc[i - 1 + (k + cc_dim2 * 3) * cc_dim1] + wa2[
		    i - 1] * cc[i + (k + cc_dim2 * 3) * cc_dim1];
	    di3 = wa2[i - 2] * cc[i + (k + cc_dim2 * 3) * cc_dim1] - wa2[i - 
		    1] * cc[i - 1 + (k + cc_dim2 * 3) * cc_dim1];
	    cr2 = dr2 + dr3;
	    ci2 = di2 + di3;
	    ch[i - 1 + (k * 3 + 1) * ch_dim1] = cc[i - 1 + (k + cc_dim2) * 
		    cc_dim1] + cr2;
	    ch[i + (k * 3 + 1) * ch_dim1] = cc[i + (k + cc_dim2) * cc_dim1] + 
		    ci2;
	    tr2 = cc[i - 1 + (k + cc_dim2) * cc_dim1] + taur * cr2;
	    ti2 = cc[i + (k + cc_dim2) * cc_dim1] + taur * ci2;
	    tr3 = taui * (di2 - di3);
	    ti3 = taui * (dr3 - dr2);
	    ch[i - 1 + (k * 3 + 3) * ch_dim1] = tr2 + tr3;
	    ch[ic - 1 + (k * 3 + 2) * ch_dim1] = tr2 - tr3;
	    ch[i + (k * 3 + 3) * ch_dim1] = ti2 + ti3;
	    ch[ic + (k * 3 + 2) * ch_dim1] = ti3 - ti2;
// L102:
	}
// L103:
    }
    return 0;
} // radf3
//------------------------------------------------------------------------
int32_t radf4d(int32_t *ido, int32_t *l1, double *cc, double *ch, double *wa1,
	  double *wa2, double *wa3)
{
  // Initialized data
  
  double hsqt2 = (double).7071067811865475;
  
  // System generated locals
  int32_t cc_dim1, cc_dim2, cc_offset, ch_dim1, ch_offset, i__1, i__2;

  // Local variables
  int32_t i, k, ic;
  double ci2, ci3, ci4, cr2, cr3, cr4, ti1, ti2, ti3, ti4, tr1, tr2, 
    tr3, tr4;
  int32_t idp2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_offset = ch_dim1 * 5 + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_dim2 = *l1;
    cc_offset = cc_dim1 * (cc_dim2 + 1) + 1;
    cc -= cc_offset;
    --wa1;
    --wa2;
    --wa3;

    // Function Body
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	tr1 = cc[(k + (cc_dim2 << 1)) * cc_dim1 + 1] + cc[(k + (cc_dim2 << 2))
		 * cc_dim1 + 1];
	tr2 = cc[(k + cc_dim2) * cc_dim1 + 1] + cc[(k + cc_dim2 * 3) * 
		cc_dim1 + 1];
	ch[((k << 2) + 1) * ch_dim1 + 1] = tr1 + tr2;
	ch[*ido + ((k << 2) + 4) * ch_dim1] = tr2 - tr1;
	ch[*ido + ((k << 2) + 2) * ch_dim1] = cc[(k + cc_dim2) * cc_dim1 + 1] 
		- cc[(k + cc_dim2 * 3) * cc_dim1 + 1];
	ch[((k << 2) + 3) * ch_dim1 + 1] = cc[(k + (cc_dim2 << 2)) * cc_dim1 
		+ 1] - cc[(k + (cc_dim2 << 1)) * cc_dim1 + 1];
// L101:
    }
    if ((i__1 = *ido - 2) < 0) {
	goto L107;
    } else if (i__1 == 0) {
	goto L105;
    } else {
	goto L102;
    }
L102:
    idp2 = *ido + 2;
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    ic = idp2 - i;
	    cr2 = wa1[i - 2] * cc[i - 1 + (k + (cc_dim2 << 1)) * cc_dim1] + 
		    wa1[i - 1] * cc[i + (k + (cc_dim2 << 1)) * cc_dim1];
	    ci2 = wa1[i - 2] * cc[i + (k + (cc_dim2 << 1)) * cc_dim1] - wa1[i 
		    - 1] * cc[i - 1 + (k + (cc_dim2 << 1)) * cc_dim1];
	    cr3 = wa2[i - 2] * cc[i - 1 + (k + cc_dim2 * 3) * cc_dim1] + wa2[
		    i - 1] * cc[i + (k + cc_dim2 * 3) * cc_dim1];
	    ci3 = wa2[i - 2] * cc[i + (k + cc_dim2 * 3) * cc_dim1] - wa2[i - 
		    1] * cc[i - 1 + (k + cc_dim2 * 3) * cc_dim1];
	    cr4 = wa3[i - 2] * cc[i - 1 + (k + (cc_dim2 << 2)) * cc_dim1] + 
		    wa3[i - 1] * cc[i + (k + (cc_dim2 << 2)) * cc_dim1];
	    ci4 = wa3[i - 2] * cc[i + (k + (cc_dim2 << 2)) * cc_dim1] - wa3[i 
		    - 1] * cc[i - 1 + (k + (cc_dim2 << 2)) * cc_dim1];
	    tr1 = cr2 + cr4;
	    tr4 = cr4 - cr2;
	    ti1 = ci2 + ci4;
	    ti4 = ci2 - ci4;
	    ti2 = cc[i + (k + cc_dim2) * cc_dim1] + ci3;
	    ti3 = cc[i + (k + cc_dim2) * cc_dim1] - ci3;
	    tr2 = cc[i - 1 + (k + cc_dim2) * cc_dim1] + cr3;
	    tr3 = cc[i - 1 + (k + cc_dim2) * cc_dim1] - cr3;
	    ch[i - 1 + ((k << 2) + 1) * ch_dim1] = tr1 + tr2;
	    ch[ic - 1 + ((k << 2) + 4) * ch_dim1] = tr2 - tr1;
	    ch[i + ((k << 2) + 1) * ch_dim1] = ti1 + ti2;
	    ch[ic + ((k << 2) + 4) * ch_dim1] = ti1 - ti2;
	    ch[i - 1 + ((k << 2) + 3) * ch_dim1] = ti4 + tr3;
	    ch[ic - 1 + ((k << 2) + 2) * ch_dim1] = tr3 - ti4;
	    ch[i + ((k << 2) + 3) * ch_dim1] = tr4 + ti3;
	    ch[ic + ((k << 2) + 2) * ch_dim1] = tr4 - ti3;
// L103:
	}
// L104:
    }
    if (*ido % 2 == 1) {
	return 0;
    }
L105:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ti1 = -(double)hsqt2 * (cc[*ido + (k + (cc_dim2 << 1)) * cc_dim1] 
		+ cc[*ido + (k + (cc_dim2 << 2)) * cc_dim1]);
	tr1 = hsqt2 * (cc[*ido + (k + (cc_dim2 << 1)) * cc_dim1] - cc[*ido + (
		k + (cc_dim2 << 2)) * cc_dim1]);
	ch[*ido + ((k << 2) + 1) * ch_dim1] = tr1 + cc[*ido + (k + cc_dim2) * 
		cc_dim1];
	ch[*ido + ((k << 2) + 3) * ch_dim1] = cc[*ido + (k + cc_dim2) * 
		cc_dim1] - tr1;
	ch[((k << 2) + 2) * ch_dim1 + 1] = ti1 - cc[*ido + (k + cc_dim2 * 3) *
		 cc_dim1];
	ch[((k << 2) + 4) * ch_dim1 + 1] = ti1 + cc[*ido + (k + cc_dim2 * 3) *
		 cc_dim1];
// L106:
    }
L107:
    return 0;
} // radf4
//------------------------------------------------------------------------
int32_t radf5d(int32_t *ido, int32_t *l1, double *cc, double *ch, double *wa1,
	  double *wa2, double *wa3, double *wa4)
{
  // Initialized data
  
  double tr11 = (double).309016994374947;
  double ti11 = (double).951056516295154;
  double tr12 = (double)-.809016994374947;
  double ti12 = (double).587785252292473;
  
  // System generated locals
  int32_t cc_dim1, cc_dim2, cc_offset, ch_dim1, ch_offset, i__1, i__2;
  
  // Local variables
  int32_t i, k, ic;
  double ci2, di2, ci4, ci5, di3, di4, di5, ci3, cr2, cr3, dr2, dr3, 
    dr4, dr5, cr5, cr4, ti2, ti3, ti5, ti4, tr2, tr3, tr4, tr5;
  int32_t idp2;
  
    // Parameter adjustments
    ch_dim1 = *ido;
    ch_offset = ch_dim1 * 6 + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_dim2 = *l1;
    cc_offset = cc_dim1 * (cc_dim2 + 1) + 1;
    cc -= cc_offset;
    --wa1;
    --wa2;
    --wa3;
    --wa4;

    // Function Body
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	cr2 = cc[(k + cc_dim2 * 5) * cc_dim1 + 1] + cc[(k + (cc_dim2 << 1)) * 
		cc_dim1 + 1];
	ci5 = cc[(k + cc_dim2 * 5) * cc_dim1 + 1] - cc[(k + (cc_dim2 << 1)) * 
		cc_dim1 + 1];
	cr3 = cc[(k + (cc_dim2 << 2)) * cc_dim1 + 1] + cc[(k + cc_dim2 * 3) * 
		cc_dim1 + 1];
	ci4 = cc[(k + (cc_dim2 << 2)) * cc_dim1 + 1] - cc[(k + cc_dim2 * 3) * 
		cc_dim1 + 1];
	ch[(k * 5 + 1) * ch_dim1 + 1] = cc[(k + cc_dim2) * cc_dim1 + 1] + cr2 
		+ cr3;
	ch[*ido + (k * 5 + 2) * ch_dim1] = cc[(k + cc_dim2) * cc_dim1 + 1] + 
		tr11 * cr2 + tr12 * cr3;
	ch[(k * 5 + 3) * ch_dim1 + 1] = ti11 * ci5 + ti12 * ci4;
	ch[*ido + (k * 5 + 4) * ch_dim1] = cc[(k + cc_dim2) * cc_dim1 + 1] + 
		tr12 * cr2 + tr11 * cr3;
	ch[(k * 5 + 5) * ch_dim1 + 1] = ti12 * ci5 - ti11 * ci4;
// L101:
    }
    if (*ido == 1) {
	return 0;
    }
    idp2 = *ido + 2;
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    ic = idp2 - i;
	    dr2 = wa1[i - 2] * cc[i - 1 + (k + (cc_dim2 << 1)) * cc_dim1] + 
		    wa1[i - 1] * cc[i + (k + (cc_dim2 << 1)) * cc_dim1];
	    di2 = wa1[i - 2] * cc[i + (k + (cc_dim2 << 1)) * cc_dim1] - wa1[i 
		    - 1] * cc[i - 1 + (k + (cc_dim2 << 1)) * cc_dim1];
	    dr3 = wa2[i - 2] * cc[i - 1 + (k + cc_dim2 * 3) * cc_dim1] + wa2[
		    i - 1] * cc[i + (k + cc_dim2 * 3) * cc_dim1];
	    di3 = wa2[i - 2] * cc[i + (k + cc_dim2 * 3) * cc_dim1] - wa2[i - 
		    1] * cc[i - 1 + (k + cc_dim2 * 3) * cc_dim1];
	    dr4 = wa3[i - 2] * cc[i - 1 + (k + (cc_dim2 << 2)) * cc_dim1] + 
		    wa3[i - 1] * cc[i + (k + (cc_dim2 << 2)) * cc_dim1];
	    di4 = wa3[i - 2] * cc[i + (k + (cc_dim2 << 2)) * cc_dim1] - wa3[i 
		    - 1] * cc[i - 1 + (k + (cc_dim2 << 2)) * cc_dim1];
	    dr5 = wa4[i - 2] * cc[i - 1 + (k + cc_dim2 * 5) * cc_dim1] + wa4[
		    i - 1] * cc[i + (k + cc_dim2 * 5) * cc_dim1];
	    di5 = wa4[i - 2] * cc[i + (k + cc_dim2 * 5) * cc_dim1] - wa4[i - 
		    1] * cc[i - 1 + (k + cc_dim2 * 5) * cc_dim1];
	    cr2 = dr2 + dr5;
	    ci5 = dr5 - dr2;
	    cr5 = di2 - di5;
	    ci2 = di2 + di5;
	    cr3 = dr3 + dr4;
	    ci4 = dr4 - dr3;
	    cr4 = di3 - di4;
	    ci3 = di3 + di4;
	    ch[i - 1 + (k * 5 + 1) * ch_dim1] = cc[i - 1 + (k + cc_dim2) * 
		    cc_dim1] + cr2 + cr3;
	    ch[i + (k * 5 + 1) * ch_dim1] = cc[i + (k + cc_dim2) * cc_dim1] + 
		    ci2 + ci3;
	    tr2 = cc[i - 1 + (k + cc_dim2) * cc_dim1] + tr11 * cr2 + tr12 * 
		    cr3;
	    ti2 = cc[i + (k + cc_dim2) * cc_dim1] + tr11 * ci2 + tr12 * ci3;
	    tr3 = cc[i - 1 + (k + cc_dim2) * cc_dim1] + tr12 * cr2 + tr11 * 
		    cr3;
	    ti3 = cc[i + (k + cc_dim2) * cc_dim1] + tr12 * ci2 + tr11 * ci3;
	    tr5 = ti11 * cr5 + ti12 * cr4;
	    ti5 = ti11 * ci5 + ti12 * ci4;
	    tr4 = ti12 * cr5 - ti11 * cr4;
	    ti4 = ti12 * ci5 - ti11 * ci4;
	    ch[i - 1 + (k * 5 + 3) * ch_dim1] = tr2 + tr5;
	    ch[ic - 1 + (k * 5 + 2) * ch_dim1] = tr2 - tr5;
	    ch[i + (k * 5 + 3) * ch_dim1] = ti2 + ti5;
	    ch[ic + (k * 5 + 2) * ch_dim1] = ti5 - ti2;
	    ch[i - 1 + (k * 5 + 5) * ch_dim1] = tr3 + tr4;
	    ch[ic - 1 + (k * 5 + 4) * ch_dim1] = tr3 - tr4;
	    ch[i + (k * 5 + 5) * ch_dim1] = ti3 + ti4;
	    ch[ic + (k * 5 + 4) * ch_dim1] = ti4 - ti3;
// L102:
	}
// L103:
    }
    return 0;
} // radf5
//------------------------------------------------------------------------
int32_t radfgd(int32_t *ido, int32_t *ip, int32_t *l1, int32_t *idl1, double *cc, double *c1,
	  double *c2, double *ch, double *ch2, double *wa)
{
  // Initialized data

  double tpi = (double)6.28318530717959;
  
  // System generated locals
  int32_t ch_dim1, ch_dim2, ch_offset, cc_dim1, cc_dim2, cc_offset, c1_dim1,
    c1_dim2, c1_offset, c2_dim1, c2_offset, ch2_dim1, ch2_offset, 
    i__1, i__2, i__3;
  
  // Builtin functions
  double cos(double), sin(double);

  // Local variables
  int32_t idij, ipph, i, j, k, l, j2, ic, jc, lc, ik, is;
  double dc2, ai1, ai2, ar1, ar2, ds2;
  int32_t nbd;
  double dcp, arg, dsp, ar1h, ar2h;
  int32_t idp2, ipp2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    c1_dim1 = *ido;
    c1_dim2 = *l1;
    c1_offset = c1_dim1 * (c1_dim2 + 1) + 1;
    c1 -= c1_offset;
    cc_dim1 = *ido;
    cc_dim2 = *ip;
    cc_offset = cc_dim1 * (cc_dim2 + 1) + 1;
    cc -= cc_offset;
    ch2_dim1 = *idl1;
    ch2_offset = ch2_dim1 + 1;
    ch2 -= ch2_offset;
    c2_dim1 = *idl1;
    c2_offset = c2_dim1 + 1;
    c2 -= c2_offset;
    --wa;

    // Function Body
    arg = tpi / (double) (*ip);
    dcp = cos(arg);
    dsp = sin(arg);
    ipph = (*ip + 1) / 2;
    ipp2 = *ip + 2;
    idp2 = *ido + 2;
    nbd = (*ido - 1) / 2;
    if (*ido == 1) {
	goto L119;
    }
    i__1 = *idl1;
    for (ik = 1; ik <= i__1; ++ik) {
	ch2[ik + ch2_dim1] = c2[ik + c2_dim1];
// L101:
    }
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    ch[(k + j * ch_dim2) * ch_dim1 + 1] = c1[(k + j * c1_dim2) * 
		    c1_dim1 + 1];
// L102:
	}
// L103:
    }
    if (nbd > *l1) {
	goto L107;
    }
    is = -(*ido);
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
	is += *ido;
	idij = is;
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    idij += 2;
	    i__3 = *l1;
	    for (k = 1; k <= i__3; ++k) {
		ch[i - 1 + (k + j * ch_dim2) * ch_dim1] = wa[idij - 1] * c1[i 
			- 1 + (k + j * c1_dim2) * c1_dim1] + wa[idij] * c1[i 
			+ (k + j * c1_dim2) * c1_dim1];
		ch[i + (k + j * ch_dim2) * ch_dim1] = wa[idij - 1] * c1[i + (
			k + j * c1_dim2) * c1_dim1] - wa[idij] * c1[i - 1 + (
			k + j * c1_dim2) * c1_dim1];
// L104:
	    }
// L105:
	}
// L106:
    }
    goto L111;
L107:
    is = -(*ido);
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
	is += *ido;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    idij = is;
	    i__3 = *ido;
	    for (i = 3; i <= i__3; i += 2) {
		idij += 2;
		ch[i - 1 + (k + j * ch_dim2) * ch_dim1] = wa[idij - 1] * c1[i 
			- 1 + (k + j * c1_dim2) * c1_dim1] + wa[idij] * c1[i 
			+ (k + j * c1_dim2) * c1_dim1];
		ch[i + (k + j * ch_dim2) * ch_dim1] = wa[idij - 1] * c1[i + (
			k + j * c1_dim2) * c1_dim1] - wa[idij] * c1[i - 1 + (
			k + j * c1_dim2) * c1_dim1];
// L108:
	    }
// L109:
	}
// L110:
    }
L111:
    if (nbd < *l1) {
	goto L115;
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    i__3 = *ido;
	    for (i = 3; i <= i__3; i += 2) {
		c1[i - 1 + (k + j * c1_dim2) * c1_dim1] = ch[i - 1 + (k + j * 
			ch_dim2) * ch_dim1] + ch[i - 1 + (k + jc * ch_dim2) * 
			ch_dim1];
		c1[i - 1 + (k + jc * c1_dim2) * c1_dim1] = ch[i + (k + j * 
			ch_dim2) * ch_dim1] - ch[i + (k + jc * ch_dim2) * 
			ch_dim1];
		c1[i + (k + j * c1_dim2) * c1_dim1] = ch[i + (k + j * ch_dim2)
			 * ch_dim1] + ch[i + (k + jc * ch_dim2) * ch_dim1];
		c1[i + (k + jc * c1_dim2) * c1_dim1] = ch[i - 1 + (k + jc * 
			ch_dim2) * ch_dim1] - ch[i - 1 + (k + j * ch_dim2) * 
			ch_dim1];
// L112:
	    }
// L113:
	}
// L114:
    }
    goto L121;
L115:
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    i__3 = *l1;
	    for (k = 1; k <= i__3; ++k) {
		c1[i - 1 + (k + j * c1_dim2) * c1_dim1] = ch[i - 1 + (k + j * 
			ch_dim2) * ch_dim1] + ch[i - 1 + (k + jc * ch_dim2) * 
			ch_dim1];
		c1[i - 1 + (k + jc * c1_dim2) * c1_dim1] = ch[i + (k + j * 
			ch_dim2) * ch_dim1] - ch[i + (k + jc * ch_dim2) * 
			ch_dim1];
		c1[i + (k + j * c1_dim2) * c1_dim1] = ch[i + (k + j * ch_dim2)
			 * ch_dim1] + ch[i + (k + jc * ch_dim2) * ch_dim1];
		c1[i + (k + jc * c1_dim2) * c1_dim1] = ch[i - 1 + (k + jc * 
			ch_dim2) * ch_dim1] - ch[i - 1 + (k + j * ch_dim2) * 
			ch_dim1];
// L116:
	    }
// L117:
	}
// L118:
    }
    goto L121;
L119:
    i__1 = *idl1;
    for (ik = 1; ik <= i__1; ++ik) {
	c2[ik + c2_dim1] = ch2[ik + ch2_dim1];
// L120:
    }
L121:
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    c1[(k + j * c1_dim2) * c1_dim1 + 1] = ch[(k + j * ch_dim2) * 
		    ch_dim1 + 1] + ch[(k + jc * ch_dim2) * ch_dim1 + 1];
	    c1[(k + jc * c1_dim2) * c1_dim1 + 1] = ch[(k + jc * ch_dim2) * 
		    ch_dim1 + 1] - ch[(k + j * ch_dim2) * ch_dim1 + 1];
// L122:
	}
// L123:
    }

    ar1 = (double)1.;
    ai1 = (double)0.;
    i__1 = ipph;
    for (l = 2; l <= i__1; ++l) {
	lc = ipp2 - l;
	ar1h = dcp * ar1 - dsp * ai1;
	ai1 = dcp * ai1 + dsp * ar1;
	ar1 = ar1h;
	i__2 = *idl1;
	for (ik = 1; ik <= i__2; ++ik) {
	    ch2[ik + l * ch2_dim1] = c2[ik + c2_dim1] + ar1 * c2[ik + (
		    c2_dim1 << 1)];
	    ch2[ik + lc * ch2_dim1] = ai1 * c2[ik + *ip * c2_dim1];
// L124:
	}
	dc2 = ar1;
	ds2 = ai1;
	ar2 = ar1;
	ai2 = ai1;
	i__2 = ipph;
	for (j = 3; j <= i__2; ++j) {
	    jc = ipp2 - j;
	    ar2h = dc2 * ar2 - ds2 * ai2;
	    ai2 = dc2 * ai2 + ds2 * ar2;
	    ar2 = ar2h;
	    i__3 = *idl1;
	    for (ik = 1; ik <= i__3; ++ik) {
		ch2[ik + l * ch2_dim1] += ar2 * c2[ik + j * c2_dim1];
		ch2[ik + lc * ch2_dim1] += ai2 * c2[ik + jc * c2_dim1];
// L125:
	    }
// L126:
	}
// L127:
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	i__2 = *idl1;
	for (ik = 1; ik <= i__2; ++ik) {
	    ch2[ik + ch2_dim1] += c2[ik + j * c2_dim1];
// L128:
	}
// L129:
    }

    if (*ido < *l1) {
	goto L132;
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i = 1; i <= i__2; ++i) {
	    cc[i + (k * cc_dim2 + 1) * cc_dim1] = ch[i + (k + ch_dim2) * 
		    ch_dim1];
// L130:
	}
// L131:
    }
    goto L135;
L132:
    i__1 = *ido;
    for (i = 1; i <= i__1; ++i) {
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    cc[i + (k * cc_dim2 + 1) * cc_dim1] = ch[i + (k + ch_dim2) * 
		    ch_dim1];
// L133:
	}
// L134:
    }
L135:
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	j2 = j + j;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    cc[*ido + (j2 - 2 + k * cc_dim2) * cc_dim1] = ch[(k + j * ch_dim2)
		     * ch_dim1 + 1];
	    cc[(j2 - 1 + k * cc_dim2) * cc_dim1 + 1] = ch[(k + jc * ch_dim2) *
		     ch_dim1 + 1];
// L136:
	}
// L137:
    }
    if (*ido == 1) {
	return 0;
    }
    if (nbd < *l1) {
	goto L141;
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	j2 = j + j;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    i__3 = *ido;
	    for (i = 3; i <= i__3; i += 2) {
		ic = idp2 - i;
		cc[i - 1 + (j2 - 1 + k * cc_dim2) * cc_dim1] = ch[i - 1 + (k 
			+ j * ch_dim2) * ch_dim1] + ch[i - 1 + (k + jc * 
			ch_dim2) * ch_dim1];
		cc[ic - 1 + (j2 - 2 + k * cc_dim2) * cc_dim1] = ch[i - 1 + (k 
			+ j * ch_dim2) * ch_dim1] - ch[i - 1 + (k + jc * 
			ch_dim2) * ch_dim1];
		cc[i + (j2 - 1 + k * cc_dim2) * cc_dim1] = ch[i + (k + j * 
			ch_dim2) * ch_dim1] + ch[i + (k + jc * ch_dim2) * 
			ch_dim1];
		cc[ic + (j2 - 2 + k * cc_dim2) * cc_dim1] = ch[i + (k + jc * 
			ch_dim2) * ch_dim1] - ch[i + (k + j * ch_dim2) * 
			ch_dim1];
// L138:
	    }
// L139:
	}
// L140:
    }
    return 0;
L141:
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	j2 = j + j;
	i__2 = *ido;
	for (i = 3; i <= i__2; i += 2) {
	    ic = idp2 - i;
	    i__3 = *l1;
	    for (k = 1; k <= i__3; ++k) {
		cc[i - 1 + (j2 - 1 + k * cc_dim2) * cc_dim1] = ch[i - 1 + (k 
			+ j * ch_dim2) * ch_dim1] + ch[i - 1 + (k + jc * 
			ch_dim2) * ch_dim1];
		cc[ic - 1 + (j2 - 2 + k * cc_dim2) * cc_dim1] = ch[i - 1 + (k 
			+ j * ch_dim2) * ch_dim1] - ch[i - 1 + (k + jc * 
			ch_dim2) * ch_dim1];
		cc[i + (j2 - 1 + k * cc_dim2) * cc_dim1] = ch[i + (k + j * 
			ch_dim2) * ch_dim1] + ch[i + (k + jc * ch_dim2) * 
			ch_dim1];
		cc[ic + (j2 - 2 + k * cc_dim2) * cc_dim1] = ch[i + (k + jc * 
			ch_dim2) * ch_dim1] - ch[i + (k + j * ch_dim2) * 
			ch_dim1];
// L142:
	    }
// L143:
	}
// L144:
    }
    return 0;
} // radfg
//------------------------------------------------------------------------
int32_t rfftbd(int32_t *n, double *r, double *wsave)
{
  int32_t rfftb1d(int32_t *n, double *c, double *ch, double *wa, double *ifac);

  // Parameter adjustments
  --wsave;
  --r;

    // Function Body
    if (*n == 1) {
	return 0;
    }
    rfftb1d(n, &r[1], &wsave[1], &wsave[*n + 1], &wsave[(*n << 1) + 1]);
    return 0;
} // rfftb
//------------------------------------------------------------------------
int32_t rfftb1d(int32_t *n, double *c, double *ch, double *wa, double *ifac)
{
  // System generated locals
  int32_t i__1;

  // Local variables
  int32_t radb2d(int32_t *ido, int32_t *l1, double *cc, double *ch, double *wa1),
    radb3d(int32_t *ido, int32_t *l1, double *cc, double *ch, double *wa1, double *wa2),
    radb4d(int32_t *ido, int32_t *l1, double *cc, double *ch, double *wa1,
	  double *wa2, double *wa3),
    radb5d(int32_t *ido, int32_t *l1, double *cc, double *ch, double *wa1,
	  double *wa2, double *wa3, double *wa4);
  int32_t i;
  int32_t radbgd(int32_t *ido, int32_t *ip, int32_t *l1, int32_t *idl1, double *cc, double *c1,
	  double *c2, double *ch, double *ch2, double *wa);
  int32_t k1, l1, l2, na, nf, ip, iw, ix2, ix3, ix4, ido, idl1;

    // Parameter adjustments
    --ifac;
    --wa;
    --ch;
    --c;

    // Function Body
    nf = ifac[2];
    na = 0;
    l1 = 1;
    iw = 1;
    i__1 = nf;
    for (k1 = 1; k1 <= i__1; ++k1) {
	ip = ifac[k1 + 2];
	l2 = ip * l1;
	ido = *n / l2;
	idl1 = ido * l1;
	if (ip != 4) {
	    goto L103;
	}
	ix2 = iw + ido;
	ix3 = ix2 + ido;
	if (na != 0) {
	    goto L101;
	}
	radb4d(&ido, &l1, &c[1], &ch[1], &wa[iw], &wa[ix2], &wa[ix3]);
	goto L102;
L101:
	radb4d(&ido, &l1, &ch[1], &c[1], &wa[iw], &wa[ix2], &wa[ix3]);
L102:
	na = 1 - na;
	goto L115;
L103:
	if (ip != 2) {
	    goto L106;
	}
	if (na != 0) {
	    goto L104;
	}
	radb2d(&ido, &l1, &c[1], &ch[1], &wa[iw]);
	goto L105;
L104:
	radb2d(&ido, &l1, &ch[1], &c[1], &wa[iw]);
L105:
	na = 1 - na;
	goto L115;
L106:
	if (ip != 3) {
	    goto L109;
	}
	ix2 = iw + ido;
	if (na != 0) {
	    goto L107;
	}
	radb3d(&ido, &l1, &c[1], &ch[1], &wa[iw], &wa[ix2]);
	goto L108;
L107:
	radb3d(&ido, &l1, &ch[1], &c[1], &wa[iw], &wa[ix2]);
L108:
	na = 1 - na;
	goto L115;
L109:
	if (ip != 5) {
	    goto L112;
	}
	ix2 = iw + ido;
	ix3 = ix2 + ido;
	ix4 = ix3 + ido;
	if (na != 0) {
	    goto L110;
	}
	radb5d(&ido, &l1, &c[1], &ch[1], &wa[iw], &wa[ix2], &wa[ix3], &wa[ix4]
		);
	goto L111;
L110:
	radb5d(&ido, &l1, &ch[1], &c[1], &wa[iw], &wa[ix2], &wa[ix3], &wa[ix4]
		);
L111:
	na = 1 - na;
	goto L115;
L112:
	if (na != 0) {
	    goto L113;
	}
	radbgd(&ido, &ip, &l1, &idl1, &c[1], &c[1], &c[1], &ch[1], &ch[1], &
		wa[iw]);
	goto L114;
L113:
	radbgd(&ido, &ip, &l1, &idl1, &ch[1], &ch[1], &ch[1], &c[1], &c[1], &
		wa[iw]);
L114:
	if (ido == 1) {
	    na = 1 - na;
	}
L115:
	l1 = l2;
	iw += (ip - 1) * ido;
// L116:
    }
    if (na == 0) {
	return 0;
    }
    i__1 = *n;
    for (i = 1; i <= i__1; ++i) {
	c[i] = ch[i];
// L117:
    }
    return 0;
} // rfftb1
//------------------------------------------------------------------------
int32_t rfftfd(int32_t *n, double *r, double *wsave)
{
  int32_t rfftf1d(int32_t *n, double *c, double *ch, double *wa, double *ifac);
  
  // Parameter adjustments
  --wsave;
  --r;
  
  // Function Body
  if (*n == 1) {
    return 0;
  }
  rfftf1d(n, &r[1], &wsave[1], &wsave[*n + 1], &wsave[(*n << 1) + 1]);
  return 0;
} // rfftf
//------------------------------------------------------------------------
int32_t rfftf1d(int32_t *n, double *c, double *ch, double *wa, double *ifac)
{
  // System generated locals
  int32_t i__1;

  // Local variables
  int32_t radf2d(int32_t *ido, int32_t *l1, double *cc, double *ch, double *wa1),
    radf3d(int32_t *ido, int32_t *l1, double *cc, double *ch, double *wa1, double *wa2),
    radf4d(int32_t *ido, int32_t *l1, double *cc, double *ch, double *wa1,
	  double *wa2, double *wa3),
    radf5d(int32_t *ido, int32_t *l1, double *cc, double *ch, double *wa1,
	  double *wa2, double *wa3, double *wa4);
  int32_t i;
  int32_t radfgd(int32_t *ido, int32_t *ip, int32_t *l1, int32_t *idl1, double *cc, double *c1,
	  double *c2, double *ch, double *ch2, double *wa);
  int32_t k1, l1, l2, na, kh, nf, ip, iw, ix2, ix3, ix4, ido, idl1;

    // Parameter adjustments
    --ifac;
    --wa;
    --ch;
    --c;

    // Function Body
    nf = ifac[2];
    na = 1;
    l2 = *n;
    iw = *n;
    i__1 = nf;
    for (k1 = 1; k1 <= i__1; ++k1) {
	kh = nf - k1;
	ip = ifac[kh + 3];
	l1 = l2 / ip;
	ido = *n / l2;
	idl1 = ido * l1;
	iw -= (ip - 1) * ido;
	na = 1 - na;
	if (ip != 4) {
	    goto L102;
	}
	ix2 = iw + ido;
	ix3 = ix2 + ido;
	if (na != 0) {
	    goto L101;
	}
	radf4d(&ido, &l1, &c[1], &ch[1], &wa[iw], &wa[ix2], &wa[ix3]);
	goto L110;
L101:
	radf4d(&ido, &l1, &ch[1], &c[1], &wa[iw], &wa[ix2], &wa[ix3]);
	goto L110;
L102:
	if (ip != 2) {
	    goto L104;
	}
	if (na != 0) {
	    goto L103;
	}
	radf2d(&ido, &l1, &c[1], &ch[1], &wa[iw]);
	goto L110;
L103:
	radf2d(&ido, &l1, &ch[1], &c[1], &wa[iw]);
	goto L110;
L104:
	if (ip != 3) {
	    goto L106;
	}
	ix2 = iw + ido;
	if (na != 0) {
	    goto L105;
	}
	radf3d(&ido, &l1, &c[1], &ch[1], &wa[iw], &wa[ix2]);
	goto L110;
L105:
	radf3d(&ido, &l1, &ch[1], &c[1], &wa[iw], &wa[ix2]);
	goto L110;
L106:
	if (ip != 5) {
	    goto L108;
	}
	ix2 = iw + ido;
	ix3 = ix2 + ido;
	ix4 = ix3 + ido;
	if (na != 0) {
	    goto L107;
	}
	radf5d(&ido, &l1, &c[1], &ch[1], &wa[iw], &wa[ix2], &wa[ix3], &wa[ix4]
		);
	goto L110;
L107:
	radf5d(&ido, &l1, &ch[1], &c[1], &wa[iw], &wa[ix2], &wa[ix3], &wa[ix4]
		);
	goto L110;
L108:
	if (ido == 1) {
	    na = 1 - na;
	}
	if (na != 0) {
	    goto L109;
	}
	radfgd(&ido, &ip, &l1, &idl1, &c[1], &c[1], &c[1], &ch[1], &ch[1], &
		wa[iw]);
	na = 1;
	goto L110;
L109:
	radfgd(&ido, &ip, &l1, &idl1, &ch[1], &ch[1], &ch[1], &c[1], &c[1], &
		wa[iw]);
	na = 0;
L110:
	l2 = l1;
// L111:
    }
    if (na == 1) {
	return 0;
    }
    i__1 = *n;
    for (i = 1; i <= i__1; ++i) {
	c[i] = ch[i];
// L112:
    }
    return 0;
} // rfftf1
//------------------------------------------------------------------------
int32_t rfftid(int32_t *n, double *wsave)
{
  int32_t rffti1d(int32_t *n, double *wa, double *ifac);

  // Parameter adjustments
  --wsave;

    // Function Body
    if (*n == 1) {
	return 0;
    }
    rffti1d(n, &wsave[*n + 1], &wsave[(*n << 1) + 1]);
    return 0;
} // rffti
//------------------------------------------------------------------------
int32_t rffti1d(int32_t *n, double *wa, double *ifac)
{
  // Initialized data
  
  int32_t ntryh[4] = { 4,2,3,5 };
  
  // System generated locals
  int32_t i__1, i__2, i__3;
  
  // Builtin functions
  double cos(double), sin(double);
  
  // Local variables
  double argh;
  int32_t ntry, i, j;
  double argld;
  int32_t k1, l1, l2, ib;
  double fi;
  int32_t ld, ii, nf, ip, nl, is, nq, nr;
  double arg;
  int32_t ido, ipm;
  double tpi;
  int32_t nfm1;

    // Parameter adjustments
    --ifac;
    --wa;

    // Function Body
    nl = *n;
    nf = 0;
    j = 0;
L101:
    ++j;
    if (j - 4 <= 0) {
	goto L102;
    } else {
	goto L103;
    }
L102:
    ntry = ntryh[j - 1];
    goto L104;
L103:
    ntry += 2;
L104:
    nq = nl / ntry;
    nr = nl - ntry * nq;
    if (nr != 0) {
	goto L101;
    } else {
	goto L105;
    }
L105:
    ++nf;
    ifac[nf + 2] = ntry;
    nl = nq;
    if (ntry != 2) {
	goto L107;
    }
    if (nf == 1) {
	goto L107;
    }
    i__1 = nf;
    for (i = 2; i <= i__1; ++i) {
	ib = nf - i + 2;
	ifac[ib + 2] = ifac[ib + 1];
// L106:
    }
    ifac[3] = 2;
L107:
    if (nl != 1) {
	goto L104;
    }
    ifac[1] = *n;
    ifac[2] = nf;
    tpi = (double)6.28318530717959;
    argh = tpi / (double) (*n);
    is = 0;
    nfm1 = nf - 1;
    l1 = 1;
    if (nfm1 == 0) {
	return 0;
    }
    i__1 = nfm1;
    for (k1 = 1; k1 <= i__1; ++k1) {
	ip = ifac[k1 + 2];
	ld = 0;
	l2 = l1 * ip;
	ido = *n / l2;
	ipm = ip - 1;
	i__2 = ipm;
	for (j = 1; j <= i__2; ++j) {
	    ld += l1;
	    i = is;
	    argld = (double) ld * argh;
	    fi = (double)0.;
	    i__3 = ido;
	    for (ii = 3; ii <= i__3; ii += 2) {
		i += 2;
		fi += (double)1.;
		arg = fi * argld;
		wa[i - 1] = cos(arg);
		wa[i] = sin(arg);
// L108:
	    }
	    is += ido;
// L109:
	}
	l1 = l2;
// L110:
    }
    return 0;
} // rffti1
//------------------------------------------------------------------------
// Below follow routines for complex ffts
//------------------------------------------------------------------------
int32_t passfd(int32_t *nac, int32_t *ido, int32_t *ip, int32_t *l1, int32_t *idl1,
	  double *cc, double *c1, double *c2, double *ch, double *ch2, double *wa)
{
    // System generated locals
    int32_t ch_dim1, ch_dim2, ch_offset, cc_dim1, cc_dim2, cc_offset, c1_dim1,
	     c1_dim2, c1_offset, c2_dim1, c2_offset, ch2_dim1, ch2_offset, 
	    i__1, i__2, i__3;

    // Local variables
    int32_t idij, idlj, idot, ipph, i__, j, k, l, jc, lc, ik, idj, 
	    idl, inc, idp;
    double wai, war;
    int32_t ipp2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    c1_dim1 = *ido;
    c1_dim2 = *l1;
    c1_offset = c1_dim1 * (c1_dim2 + 1) + 1;
    c1 -= c1_offset;
    cc_dim1 = *ido;
    cc_dim2 = *ip;
    cc_offset = cc_dim1 * (cc_dim2 + 1) + 1;
    cc -= cc_offset;
    ch2_dim1 = *idl1;
    ch2_offset = ch2_dim1 + 1;
    ch2 -= ch2_offset;
    c2_dim1 = *idl1;
    c2_offset = c2_dim1 + 1;
    c2 -= c2_offset;
    --wa;

    // Function Body
    idot = *ido / 2;
    ipp2 = *ip + 2;
    ipph = (*ip + 1) / 2;
    idp = *ip * *ido;

    if (*ido < *l1) {
	goto L106;
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    i__3 = *ido;
	    for (i__ = 1; i__ <= i__3; ++i__) {
		ch[i__ + (k + j * ch_dim2) * ch_dim1] = cc[i__ + (j + k * 
			cc_dim2) * cc_dim1] + cc[i__ + (jc + k * cc_dim2) * 
			cc_dim1];
		ch[i__ + (k + jc * ch_dim2) * ch_dim1] = cc[i__ + (j + k * 
			cc_dim2) * cc_dim1] - cc[i__ + (jc + k * cc_dim2) * 
			cc_dim1];
// L101:
	    }
// L102:
	}
// L103:
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + (k * cc_dim2 + 1) * 
		    cc_dim1];
// L104:
	}
// L105:
    }
    goto L112;
L106:
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *ido;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    i__3 = *l1;
	    for (k = 1; k <= i__3; ++k) {
		ch[i__ + (k + j * ch_dim2) * ch_dim1] = cc[i__ + (j + k * 
			cc_dim2) * cc_dim1] + cc[i__ + (jc + k * cc_dim2) * 
			cc_dim1];
		ch[i__ + (k + jc * ch_dim2) * ch_dim1] = cc[i__ + (j + k * 
			cc_dim2) * cc_dim1] - cc[i__ + (jc + k * cc_dim2) * 
			cc_dim1];
// L107:
	    }
// L108:
	}
// L109:
    }
    i__1 = *ido;
    for (i__ = 1; i__ <= i__1; ++i__) {
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + (k * cc_dim2 + 1) * 
		    cc_dim1];
// L110:
	}
// L111:
    }
L112:
    idl = 2 - *ido;
    inc = 0;
    i__1 = ipph;
    for (l = 2; l <= i__1; ++l) {
	lc = ipp2 - l;
	idl += *ido;
	i__2 = *idl1;
	for (ik = 1; ik <= i__2; ++ik) {
	    c2[ik + l * c2_dim1] = ch2[ik + ch2_dim1] + wa[idl - 1] * ch2[ik 
		    + (ch2_dim1 << 1)];
	    c2[ik + lc * c2_dim1] = -wa[idl] * ch2[ik + *ip * ch2_dim1];
// L113:
	}
	idlj = idl;
	inc += *ido;
	i__2 = ipph;
	for (j = 3; j <= i__2; ++j) {
	    jc = ipp2 - j;
	    idlj += inc;
	    if (idlj > idp) {
		idlj -= idp;
	    }
	    war = wa[idlj - 1];
	    wai = wa[idlj];
	    i__3 = *idl1;
	    for (ik = 1; ik <= i__3; ++ik) {
		c2[ik + l * c2_dim1] += war * ch2[ik + j * ch2_dim1];
		c2[ik + lc * c2_dim1] -= wai * ch2[ik + jc * ch2_dim1];
// L114:
	    }
// L115:
	}
// L116:
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	i__2 = *idl1;
	for (ik = 1; ik <= i__2; ++ik) {
	    ch2[ik + ch2_dim1] += ch2[ik + j * ch2_dim1];
// L117:
	}
// L118:
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *idl1;
	for (ik = 2; ik <= i__2; ik += 2) {
	    ch2[ik - 1 + j * ch2_dim1] = c2[ik - 1 + j * c2_dim1] - c2[ik + 
		    jc * c2_dim1];
	    ch2[ik - 1 + jc * ch2_dim1] = c2[ik - 1 + j * c2_dim1] + c2[ik + 
		    jc * c2_dim1];
	    ch2[ik + j * ch2_dim1] = c2[ik + j * c2_dim1] + c2[ik - 1 + jc * 
		    c2_dim1];
	    ch2[ik + jc * ch2_dim1] = c2[ik + j * c2_dim1] - c2[ik - 1 + jc * 
		    c2_dim1];
// L119:
	}
// L120:
    }
    *nac = 1;
    if (*ido == 2) {
	return 0;
    }
    *nac = 0;
    i__1 = *idl1;
    for (ik = 1; ik <= i__1; ++ik) {
	c2[ik + c2_dim1] = ch2[ik + ch2_dim1];
// L121:
    }
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    c1[(k + j * c1_dim2) * c1_dim1 + 1] = ch[(k + j * ch_dim2) * 
		    ch_dim1 + 1];
	    c1[(k + j * c1_dim2) * c1_dim1 + 2] = ch[(k + j * ch_dim2) * 
		    ch_dim1 + 2];
// L122:
	}
// L123:
    }
    if (idot > *l1) {
	goto L127;
    }
    idij = 0;
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
	idij += 2;
	i__2 = *ido;
	for (i__ = 4; i__ <= i__2; i__ += 2) {
	    idij += 2;
	    i__3 = *l1;
	    for (k = 1; k <= i__3; ++k) {
		c1[i__ - 1 + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1] * ch[
			i__ - 1 + (k + j * ch_dim2) * ch_dim1] + wa[idij] * 
			ch[i__ + (k + j * ch_dim2) * ch_dim1];
		c1[i__ + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1] * ch[i__ 
			+ (k + j * ch_dim2) * ch_dim1] - wa[idij] * ch[i__ - 
			1 + (k + j * ch_dim2) * ch_dim1];
// L124:
	    }
// L125:
	}
// L126:
    }
    return 0;
L127:
    idj = 2 - *ido;
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
	idj += *ido;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    idij = idj;
	    i__3 = *ido;
	    for (i__ = 4; i__ <= i__3; i__ += 2) {
		idij += 2;
		c1[i__ - 1 + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1] * ch[
			i__ - 1 + (k + j * ch_dim2) * ch_dim1] + wa[idij] * 
			ch[i__ + (k + j * ch_dim2) * ch_dim1];
		c1[i__ + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1] * ch[i__ 
			+ (k + j * ch_dim2) * ch_dim1] - wa[idij] * ch[i__ - 
			1 + (k + j * ch_dim2) * ch_dim1];
// L128:
	    }
// L129:
	}
// L130:
    }
    return 0;
} // passf
//------------------------------------------------------------------------
int32_t passf2d(int32_t *ido, int32_t *l1, double *cc, double *ch, double *wa1)
{
    // System generated locals
    int32_t cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

    // Local variables
    int32_t i__, k;
    double ti2, tr2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = cc_dim1 * 3 + 1;
    cc -= cc_offset;
    --wa1;

    // Function Body
    if (*ido > 2) {
	goto L102;
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ch[(k + ch_dim2) * ch_dim1 + 1] = cc[((k << 1) + 1) * cc_dim1 + 1] + 
		cc[((k << 1) + 2) * cc_dim1 + 1];
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = cc[((k << 1) + 1) * cc_dim1 
		+ 1] - cc[((k << 1) + 2) * cc_dim1 + 1];
	ch[(k + ch_dim2) * ch_dim1 + 2] = cc[((k << 1) + 1) * cc_dim1 + 2] + 
		cc[((k << 1) + 2) * cc_dim1 + 2];
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 2] = cc[((k << 1) + 1) * cc_dim1 
		+ 2] - cc[((k << 1) + 2) * cc_dim1 + 2];
// L101:
    }
    return 0;
L102:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i__ = 2; i__ <= i__2; i__ += 2) {
	    ch[i__ - 1 + (k + ch_dim2) * ch_dim1] = cc[i__ - 1 + ((k << 1) + 
		    1) * cc_dim1] + cc[i__ - 1 + ((k << 1) + 2) * cc_dim1];
	    tr2 = cc[i__ - 1 + ((k << 1) + 1) * cc_dim1] - cc[i__ - 1 + ((k <<
		     1) + 2) * cc_dim1];
	    ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + ((k << 1) + 1) * 
		    cc_dim1] + cc[i__ + ((k << 1) + 2) * cc_dim1];
	    ti2 = cc[i__ + ((k << 1) + 1) * cc_dim1] - cc[i__ + ((k << 1) + 2)
		     * cc_dim1];
	    ch[i__ + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * ti2 - 
		    wa1[i__] * tr2;
	    ch[i__ - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * tr2 
		    + wa1[i__] * ti2;
// L103:
	}
// L104:
    }
    return 0;
} // passf2
//------------------------------------------------------------------------
int32_t passf3d(int32_t *ido, int32_t *l1, double *cc, double *ch, double *wa1, double *wa2)
{
    // Initialized data

    double taur = (double)-.5;
    double taui = (double)-.866025403784439;

    // System generated locals
    int32_t cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

    // Local variables
    int32_t i__, k;
    double ci2, ci3, di2, di3, cr2, cr3, dr2, dr3, ti2, tr2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = (cc_dim1 << 2) + 1;
    cc -= cc_offset;
    --wa1;
    --wa2;

    // Function Body
    if (*ido != 2) {
	goto L102;
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	tr2 = cc[(k * 3 + 2) * cc_dim1 + 1] + cc[(k * 3 + 3) * cc_dim1 + 1];
	cr2 = cc[(k * 3 + 1) * cc_dim1 + 1] + taur * tr2;
	ch[(k + ch_dim2) * ch_dim1 + 1] = cc[(k * 3 + 1) * cc_dim1 + 1] + tr2;
	ti2 = cc[(k * 3 + 2) * cc_dim1 + 2] + cc[(k * 3 + 3) * cc_dim1 + 2];
	ci2 = cc[(k * 3 + 1) * cc_dim1 + 2] + taur * ti2;
	ch[(k + ch_dim2) * ch_dim1 + 2] = cc[(k * 3 + 1) * cc_dim1 + 2] + ti2;
	cr3 = taui * (cc[(k * 3 + 2) * cc_dim1 + 1] - cc[(k * 3 + 3) * 
		cc_dim1 + 1]);
	ci3 = taui * (cc[(k * 3 + 2) * cc_dim1 + 2] - cc[(k * 3 + 3) * 
		cc_dim1 + 2]);
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = cr2 - ci3;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 1] = cr2 + ci3;
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 2] = ci2 + cr3;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 2] = ci2 - cr3;
// L101:
    }
    return 0;
L102:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i__ = 2; i__ <= i__2; i__ += 2) {
	    tr2 = cc[i__ - 1 + (k * 3 + 2) * cc_dim1] + cc[i__ - 1 + (k * 3 + 
		    3) * cc_dim1];
	    cr2 = cc[i__ - 1 + (k * 3 + 1) * cc_dim1] + taur * tr2;
	    ch[i__ - 1 + (k + ch_dim2) * ch_dim1] = cc[i__ - 1 + (k * 3 + 1) *
		     cc_dim1] + tr2;
	    ti2 = cc[i__ + (k * 3 + 2) * cc_dim1] + cc[i__ + (k * 3 + 3) * 
		    cc_dim1];
	    ci2 = cc[i__ + (k * 3 + 1) * cc_dim1] + taur * ti2;
	    ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + (k * 3 + 1) * 
		    cc_dim1] + ti2;
	    cr3 = taui * (cc[i__ - 1 + (k * 3 + 2) * cc_dim1] - cc[i__ - 1 + (
		    k * 3 + 3) * cc_dim1]);
	    ci3 = taui * (cc[i__ + (k * 3 + 2) * cc_dim1] - cc[i__ + (k * 3 + 
		    3) * cc_dim1]);
	    dr2 = cr2 - ci3;
	    dr3 = cr2 + ci3;
	    di2 = ci2 + cr3;
	    di3 = ci2 - cr3;
	    ch[i__ + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * di2 - 
		    wa1[i__] * dr2;
	    ch[i__ - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * dr2 
		    + wa1[i__] * di2;
	    ch[i__ + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 1] * di3 - wa2[
		    i__] * dr3;
	    ch[i__ - 1 + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 1] * dr3 + 
		    wa2[i__] * di3;
// L103:
	}
// L104:
    }
    return 0;
} // passf3
//------------------------------------------------------------------------
int32_t passf4d(int32_t *ido, int32_t *l1, double *cc, double *ch, double *wa1, double *wa2,
	   double *wa3)
{
    // System generated locals
    int32_t cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

    // Local variables
    int32_t i__, k;
    double ci2, ci3, ci4, cr2, cr3, cr4, ti1, ti2, ti3, ti4, tr1, tr2, 
	    tr3, tr4;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = cc_dim1 * 5 + 1;
    cc -= cc_offset;
    --wa1;
    --wa2;
    --wa3;

    // Function Body
    if (*ido != 2) {
	goto L102;
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ti1 = cc[((k << 2) + 1) * cc_dim1 + 2] - cc[((k << 2) + 3) * cc_dim1 
		+ 2];
	ti2 = cc[((k << 2) + 1) * cc_dim1 + 2] + cc[((k << 2) + 3) * cc_dim1 
		+ 2];
	tr4 = cc[((k << 2) + 2) * cc_dim1 + 2] - cc[((k << 2) + 4) * cc_dim1 
		+ 2];
	ti3 = cc[((k << 2) + 2) * cc_dim1 + 2] + cc[((k << 2) + 4) * cc_dim1 
		+ 2];
	tr1 = cc[((k << 2) + 1) * cc_dim1 + 1] - cc[((k << 2) + 3) * cc_dim1 
		+ 1];
	tr2 = cc[((k << 2) + 1) * cc_dim1 + 1] + cc[((k << 2) + 3) * cc_dim1 
		+ 1];
	ti4 = cc[((k << 2) + 4) * cc_dim1 + 1] - cc[((k << 2) + 2) * cc_dim1 
		+ 1];
	tr3 = cc[((k << 2) + 2) * cc_dim1 + 1] + cc[((k << 2) + 4) * cc_dim1 
		+ 1];
	ch[(k + ch_dim2) * ch_dim1 + 1] = tr2 + tr3;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 1] = tr2 - tr3;
	ch[(k + ch_dim2) * ch_dim1 + 2] = ti2 + ti3;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 2] = ti2 - ti3;
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = tr1 + tr4;
	ch[(k + (ch_dim2 << 2)) * ch_dim1 + 1] = tr1 - tr4;
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 2] = ti1 + ti4;
	ch[(k + (ch_dim2 << 2)) * ch_dim1 + 2] = ti1 - ti4;
// L101:
    }
    return 0;
L102:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i__ = 2; i__ <= i__2; i__ += 2) {
	    ti1 = cc[i__ + ((k << 2) + 1) * cc_dim1] - cc[i__ + ((k << 2) + 3)
		     * cc_dim1];
	    ti2 = cc[i__ + ((k << 2) + 1) * cc_dim1] + cc[i__ + ((k << 2) + 3)
		     * cc_dim1];
	    ti3 = cc[i__ + ((k << 2) + 2) * cc_dim1] + cc[i__ + ((k << 2) + 4)
		     * cc_dim1];
	    tr4 = cc[i__ + ((k << 2) + 2) * cc_dim1] - cc[i__ + ((k << 2) + 4)
		     * cc_dim1];
	    tr1 = cc[i__ - 1 + ((k << 2) + 1) * cc_dim1] - cc[i__ - 1 + ((k <<
		     2) + 3) * cc_dim1];
	    tr2 = cc[i__ - 1 + ((k << 2) + 1) * cc_dim1] + cc[i__ - 1 + ((k <<
		     2) + 3) * cc_dim1];
	    ti4 = cc[i__ - 1 + ((k << 2) + 4) * cc_dim1] - cc[i__ - 1 + ((k <<
		     2) + 2) * cc_dim1];
	    tr3 = cc[i__ - 1 + ((k << 2) + 2) * cc_dim1] + cc[i__ - 1 + ((k <<
		     2) + 4) * cc_dim1];
	    ch[i__ - 1 + (k + ch_dim2) * ch_dim1] = tr2 + tr3;
	    cr3 = tr2 - tr3;
	    ch[i__ + (k + ch_dim2) * ch_dim1] = ti2 + ti3;
	    ci3 = ti2 - ti3;
	    cr2 = tr1 + tr4;
	    cr4 = tr1 - tr4;
	    ci2 = ti1 + ti4;
	    ci4 = ti1 - ti4;
	    ch[i__ - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * cr2 
		    + wa1[i__] * ci2;
	    ch[i__ + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * ci2 - 
		    wa1[i__] * cr2;
	    ch[i__ - 1 + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 1] * cr3 + 
		    wa2[i__] * ci3;
	    ch[i__ + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 1] * ci3 - wa2[
		    i__] * cr3;
	    ch[i__ - 1 + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i__ - 1] * cr4 
		    + wa3[i__] * ci4;
	    ch[i__ + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i__ - 1] * ci4 - 
		    wa3[i__] * cr4;
// L103:
	}
// L104:
    }
    return 0;
} // passf4
//------------------------------------------------------------------------
int32_t passf5d(int32_t *ido, int32_t *l1, double *cc, double *ch, double *wa1, double *wa2,
	   double *wa3, double *wa4)
{
    // Initialized data

    double tr11 = (double).309016994374947;
    double ti11 = (double)-.951056516295154;
    double tr12 = (double)-.809016994374947;
    double ti12 = (double)-.587785252292473;

    // System generated locals
    int32_t cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

    // Local variables
    int32_t i__, k;
    double ci2, ci3, ci4, ci5, di3, di4, di5, di2, cr2, cr3, cr5, cr4, 
	    ti2, ti3, ti4, ti5, dr3, dr4, dr5, dr2, tr2, tr3, tr4, tr5;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = cc_dim1 * 6 + 1;
    cc -= cc_offset;
    --wa1;
    --wa2;
    --wa3;
    --wa4;

    // Function Body
    if (*ido != 2) {
	goto L102;
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ti5 = cc[(k * 5 + 2) * cc_dim1 + 2] - cc[(k * 5 + 5) * cc_dim1 + 2];
	ti2 = cc[(k * 5 + 2) * cc_dim1 + 2] + cc[(k * 5 + 5) * cc_dim1 + 2];
	ti4 = cc[(k * 5 + 3) * cc_dim1 + 2] - cc[(k * 5 + 4) * cc_dim1 + 2];
	ti3 = cc[(k * 5 + 3) * cc_dim1 + 2] + cc[(k * 5 + 4) * cc_dim1 + 2];
	tr5 = cc[(k * 5 + 2) * cc_dim1 + 1] - cc[(k * 5 + 5) * cc_dim1 + 1];
	tr2 = cc[(k * 5 + 2) * cc_dim1 + 1] + cc[(k * 5 + 5) * cc_dim1 + 1];
	tr4 = cc[(k * 5 + 3) * cc_dim1 + 1] - cc[(k * 5 + 4) * cc_dim1 + 1];
	tr3 = cc[(k * 5 + 3) * cc_dim1 + 1] + cc[(k * 5 + 4) * cc_dim1 + 1];
	ch[(k + ch_dim2) * ch_dim1 + 1] = cc[(k * 5 + 1) * cc_dim1 + 1] + tr2 
		+ tr3;
	ch[(k + ch_dim2) * ch_dim1 + 2] = cc[(k * 5 + 1) * cc_dim1 + 2] + ti2 
		+ ti3;
	cr2 = cc[(k * 5 + 1) * cc_dim1 + 1] + tr11 * tr2 + tr12 * tr3;
	ci2 = cc[(k * 5 + 1) * cc_dim1 + 2] + tr11 * ti2 + tr12 * ti3;
	cr3 = cc[(k * 5 + 1) * cc_dim1 + 1] + tr12 * tr2 + tr11 * tr3;
	ci3 = cc[(k * 5 + 1) * cc_dim1 + 2] + tr12 * ti2 + tr11 * ti3;
	cr5 = ti11 * tr5 + ti12 * tr4;
	ci5 = ti11 * ti5 + ti12 * ti4;
	cr4 = ti12 * tr5 - ti11 * tr4;
	ci4 = ti12 * ti5 - ti11 * ti4;
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = cr2 - ci5;
	ch[(k + ch_dim2 * 5) * ch_dim1 + 1] = cr2 + ci5;
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 2] = ci2 + cr5;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 2] = ci3 + cr4;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 1] = cr3 - ci4;
	ch[(k + (ch_dim2 << 2)) * ch_dim1 + 1] = cr3 + ci4;
	ch[(k + (ch_dim2 << 2)) * ch_dim1 + 2] = ci3 - cr4;
	ch[(k + ch_dim2 * 5) * ch_dim1 + 2] = ci2 - cr5;
// L101:
    }
    return 0;
L102:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i__ = 2; i__ <= i__2; i__ += 2) {
	    ti5 = cc[i__ + (k * 5 + 2) * cc_dim1] - cc[i__ + (k * 5 + 5) * 
		    cc_dim1];
	    ti2 = cc[i__ + (k * 5 + 2) * cc_dim1] + cc[i__ + (k * 5 + 5) * 
		    cc_dim1];
	    ti4 = cc[i__ + (k * 5 + 3) * cc_dim1] - cc[i__ + (k * 5 + 4) * 
		    cc_dim1];
	    ti3 = cc[i__ + (k * 5 + 3) * cc_dim1] + cc[i__ + (k * 5 + 4) * 
		    cc_dim1];
	    tr5 = cc[i__ - 1 + (k * 5 + 2) * cc_dim1] - cc[i__ - 1 + (k * 5 + 
		    5) * cc_dim1];
	    tr2 = cc[i__ - 1 + (k * 5 + 2) * cc_dim1] + cc[i__ - 1 + (k * 5 + 
		    5) * cc_dim1];
	    tr4 = cc[i__ - 1 + (k * 5 + 3) * cc_dim1] - cc[i__ - 1 + (k * 5 + 
		    4) * cc_dim1];
	    tr3 = cc[i__ - 1 + (k * 5 + 3) * cc_dim1] + cc[i__ - 1 + (k * 5 + 
		    4) * cc_dim1];
	    ch[i__ - 1 + (k + ch_dim2) * ch_dim1] = cc[i__ - 1 + (k * 5 + 1) *
		     cc_dim1] + tr2 + tr3;
	    ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + (k * 5 + 1) * 
		    cc_dim1] + ti2 + ti3;
	    cr2 = cc[i__ - 1 + (k * 5 + 1) * cc_dim1] + tr11 * tr2 + tr12 * 
		    tr3;
	    ci2 = cc[i__ + (k * 5 + 1) * cc_dim1] + tr11 * ti2 + tr12 * ti3;
	    cr3 = cc[i__ - 1 + (k * 5 + 1) * cc_dim1] + tr12 * tr2 + tr11 * 
		    tr3;
	    ci3 = cc[i__ + (k * 5 + 1) * cc_dim1] + tr12 * ti2 + tr11 * ti3;
	    cr5 = ti11 * tr5 + ti12 * tr4;
	    ci5 = ti11 * ti5 + ti12 * ti4;
	    cr4 = ti12 * tr5 - ti11 * tr4;
	    ci4 = ti12 * ti5 - ti11 * ti4;
	    dr3 = cr3 - ci4;
	    dr4 = cr3 + ci4;
	    di3 = ci3 + cr4;
	    di4 = ci3 - cr4;
	    dr5 = cr2 + ci5;
	    dr2 = cr2 - ci5;
	    di5 = ci2 - cr5;
	    di2 = ci2 + cr5;
	    ch[i__ - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * dr2 
		    + wa1[i__] * di2;
	    ch[i__ + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * di2 - 
		    wa1[i__] * dr2;
	    ch[i__ - 1 + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 1] * dr3 + 
		    wa2[i__] * di3;
	    ch[i__ + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 1] * di3 - wa2[
		    i__] * dr3;
	    ch[i__ - 1 + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i__ - 1] * dr4 
		    + wa3[i__] * di4;
	    ch[i__ + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i__ - 1] * di4 - 
		    wa3[i__] * dr4;
	    ch[i__ - 1 + (k + ch_dim2 * 5) * ch_dim1] = wa4[i__ - 1] * dr5 + 
		    wa4[i__] * di5;
	    ch[i__ + (k + ch_dim2 * 5) * ch_dim1] = wa4[i__ - 1] * di5 - wa4[
		    i__] * dr5;
// L103:
	}
// L104:
    }
    return 0;
} // passf5
//------------------------------------------------------------------------
int32_t cfftf1d(int32_t *n, double *c__, double *ch, double *wa, double *ifac)
{
    // System generated locals
    int32_t i__1;

    // Local variables
    int32_t idot, i__;
    int32_t passfd(int32_t *, int32_t *, int32_t *, int32_t *, int32_t *, double *, double *, double *,
	      double *, double *, double *);
    int32_t k1, l1, l2, n2;
    int32_t passf2d(int32_t *, int32_t *, double *, double *, double *),
      passf3d(int32_t *, int32_t *, double *, double *, double *, double *),
      passf4d(int32_t *, int32_t *, double *, double *, double *, double *, double *),
      passf5d(int32_t *, int32_t *, double *, double *, double *, double *, double *,
	     double *);
    int32_t na, nf, ip, iw, ix2, ix3, ix4, nac, ido, idl1;

    // Parameter adjustments
    --ifac;
    --wa;
    --ch;
    --c__;

    // Function Body
    nf = ifac[2];
    na = 0;
    l1 = 1;
    iw = 1;
    i__1 = nf;
    for (k1 = 1; k1 <= i__1; ++k1) {
	ip = ifac[k1 + 2];
	l2 = ip * l1;
	ido = *n / l2;
	idot = ido + ido;
	idl1 = idot * l1;
	if (ip != 4) {
	    goto L103;
	}
	ix2 = iw + idot;
	ix3 = ix2 + idot;
	if (na != 0) {
	    goto L101;
	}
	passf4d(&idot, &l1, &c__[1], &ch[1], &wa[iw], &wa[ix2], &wa[ix3]);
	goto L102;
L101:
	passf4d(&idot, &l1, &ch[1], &c__[1], &wa[iw], &wa[ix2], &wa[ix3]);
L102:
	na = 1 - na;
	goto L115;
L103:
	if (ip != 2) {
	    goto L106;
	}
	if (na != 0) {
	    goto L104;
	}
	passf2d(&idot, &l1, &c__[1], &ch[1], &wa[iw]);
	goto L105;
L104:
	passf2d(&idot, &l1, &ch[1], &c__[1], &wa[iw]);
L105:
	na = 1 - na;
	goto L115;
L106:
	if (ip != 3) {
	    goto L109;
	}
	ix2 = iw + idot;
	if (na != 0) {
	    goto L107;
	}
	passf3d(&idot, &l1, &c__[1], &ch[1], &wa[iw], &wa[ix2]);
	goto L108;
L107:
	passf3d(&idot, &l1, &ch[1], &c__[1], &wa[iw], &wa[ix2]);
L108:
	na = 1 - na;
	goto L115;
L109:
	if (ip != 5) {
	    goto L112;
	}
	ix2 = iw + idot;
	ix3 = ix2 + idot;
	ix4 = ix3 + idot;
	if (na != 0) {
	    goto L110;
	}
	passf5d(&idot, &l1, &c__[1], &ch[1], &wa[iw], &wa[ix2], &wa[ix3], &wa[
		ix4]);
	goto L111;
L110:
	passf5d(&idot, &l1, &ch[1], &c__[1], &wa[iw], &wa[ix2], &wa[ix3], &wa[
		ix4]);
L111:
	na = 1 - na;
	goto L115;
L112:
	if (na != 0) {
	    goto L113;
	}
	passfd(&nac, &idot, &ip, &l1, &idl1, &c__[1], &c__[1], &c__[1], &ch[1]
		, &ch[1], &wa[iw]);
	goto L114;
L113:
	passfd(&nac, &idot, &ip, &l1, &idl1, &ch[1], &ch[1], &ch[1], &c__[1], 
		&c__[1], &wa[iw]);
L114:
	if (nac != 0) {
	    na = 1 - na;
	}
L115:
	l1 = l2;
	iw += (ip - 1) * idot;
// L116:
    }
    if (na == 0) {
	return 0;
    }
    n2 = *n + *n;
    i__1 = n2;
    for (i__ = 1; i__ <= i__1; ++i__) {
	c__[i__] = ch[i__];
// L117:
    }
    return 0;
} // cfftf1
//------------------------------------------------------------------------
int32_t cfftfd(int32_t *n, double *c__, double *wsave)
{
  int32_t cfftf1d(int32_t *, double *, double *, double *, double *);
  int32_t iw1, iw2;

  // Parameter adjustments
  --wsave;
  --c__;

  // Function Body
  if (*n == 1) {
    return 0;
  }
  iw1 = *n + *n + 1;
  iw2 = iw1 + *n + *n;
  cfftf1d(n, &c__[1], &wsave[1], &wsave[iw1], &wsave[iw2]);
  return 0;
} // cfftf
//------------------------------------------------------------------------
int32_t passbd(int32_t *nac, int32_t *ido, int32_t *ip, int32_t *l1, int32_t *idl1,
	  double *cc, double *c1, double *c2, double *ch, double *ch2, double *wa)
{
    // System generated locals
    int32_t ch_dim1, ch_dim2, ch_offset, cc_dim1, cc_dim2, cc_offset, c1_dim1,
	     c1_dim2, c1_offset, c2_dim1, c2_offset, ch2_dim1, ch2_offset, 
	    i__1, i__2, i__3;

    // Local variables
    int32_t idij, idlj, idot, ipph, i__, j, k, l, jc, lc, ik, idj, 
	    idl, inc, idp;
    double wai, war;
    int32_t ipp2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    c1_dim1 = *ido;
    c1_dim2 = *l1;
    c1_offset = c1_dim1 * (c1_dim2 + 1) + 1;
    c1 -= c1_offset;
    cc_dim1 = *ido;
    cc_dim2 = *ip;
    cc_offset = cc_dim1 * (cc_dim2 + 1) + 1;
    cc -= cc_offset;
    ch2_dim1 = *idl1;
    ch2_offset = ch2_dim1 + 1;
    ch2 -= ch2_offset;
    c2_dim1 = *idl1;
    c2_offset = c2_dim1 + 1;
    c2 -= c2_offset;
    --wa;

    // Function Body
    idot = *ido / 2;
    ipp2 = *ip + 2;
    ipph = (*ip + 1) / 2;
    idp = *ip * *ido;

    if (*ido < *l1) {
	goto L106;
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    i__3 = *ido;
	    for (i__ = 1; i__ <= i__3; ++i__) {
		ch[i__ + (k + j * ch_dim2) * ch_dim1] = cc[i__ + (j + k * 
			cc_dim2) * cc_dim1] + cc[i__ + (jc + k * cc_dim2) * 
			cc_dim1];
		ch[i__ + (k + jc * ch_dim2) * ch_dim1] = cc[i__ + (j + k * 
			cc_dim2) * cc_dim1] - cc[i__ + (jc + k * cc_dim2) * 
			cc_dim1];
// L101:
	    }
// L102:
	}
// L103:
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + (k * cc_dim2 + 1) * 
		    cc_dim1];
// L104:
	}
// L105:
    }
    goto L112;
L106:
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *ido;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    i__3 = *l1;
	    for (k = 1; k <= i__3; ++k) {
		ch[i__ + (k + j * ch_dim2) * ch_dim1] = cc[i__ + (j + k * 
			cc_dim2) * cc_dim1] + cc[i__ + (jc + k * cc_dim2) * 
			cc_dim1];
		ch[i__ + (k + jc * ch_dim2) * ch_dim1] = cc[i__ + (j + k * 
			cc_dim2) * cc_dim1] - cc[i__ + (jc + k * cc_dim2) * 
			cc_dim1];
// L107:
	    }
// L108:
	}
// L109:
    }
    i__1 = *ido;
    for (i__ = 1; i__ <= i__1; ++i__) {
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + (k * cc_dim2 + 1) * 
		    cc_dim1];
// L110:
	}
// L111:
    }
L112:
    idl = 2 - *ido;
    inc = 0;
    i__1 = ipph;
    for (l = 2; l <= i__1; ++l) {
	lc = ipp2 - l;
	idl += *ido;
	i__2 = *idl1;
	for (ik = 1; ik <= i__2; ++ik) {
	    c2[ik + l * c2_dim1] = ch2[ik + ch2_dim1] + wa[idl - 1] * ch2[ik 
		    + (ch2_dim1 << 1)];
	    c2[ik + lc * c2_dim1] = wa[idl] * ch2[ik + *ip * ch2_dim1];
// L113:
	}
	idlj = idl;
	inc += *ido;
	i__2 = ipph;
	for (j = 3; j <= i__2; ++j) {
	    jc = ipp2 - j;
	    idlj += inc;
	    if (idlj > idp) {
		idlj -= idp;
	    }
	    war = wa[idlj - 1];
	    wai = wa[idlj];
	    i__3 = *idl1;
	    for (ik = 1; ik <= i__3; ++ik) {
		c2[ik + l * c2_dim1] += war * ch2[ik + j * ch2_dim1];
		c2[ik + lc * c2_dim1] += wai * ch2[ik + jc * ch2_dim1];
// L114:
	    }
// L115:
	}
// L116:
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	i__2 = *idl1;
	for (ik = 1; ik <= i__2; ++ik) {
	    ch2[ik + ch2_dim1] += ch2[ik + j * ch2_dim1];
// L117:
	}
// L118:
    }
    i__1 = ipph;
    for (j = 2; j <= i__1; ++j) {
	jc = ipp2 - j;
	i__2 = *idl1;
	for (ik = 2; ik <= i__2; ik += 2) {
	    ch2[ik - 1 + j * ch2_dim1] = c2[ik - 1 + j * c2_dim1] - c2[ik + 
		    jc * c2_dim1];
	    ch2[ik - 1 + jc * ch2_dim1] = c2[ik - 1 + j * c2_dim1] + c2[ik + 
		    jc * c2_dim1];
	    ch2[ik + j * ch2_dim1] = c2[ik + j * c2_dim1] + c2[ik - 1 + jc * 
		    c2_dim1];
	    ch2[ik + jc * ch2_dim1] = c2[ik + j * c2_dim1] - c2[ik - 1 + jc * 
		    c2_dim1];
// L119:
	}
// L120:
    }
    *nac = 1;
    if (*ido == 2) {
	return 0;
    }
    *nac = 0;
    i__1 = *idl1;
    for (ik = 1; ik <= i__1; ++ik) {
	c2[ik + c2_dim1] = ch2[ik + ch2_dim1];
// L121:
    }
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    c1[(k + j * c1_dim2) * c1_dim1 + 1] = ch[(k + j * ch_dim2) * 
		    ch_dim1 + 1];
	    c1[(k + j * c1_dim2) * c1_dim1 + 2] = ch[(k + j * ch_dim2) * 
		    ch_dim1 + 2];
// L122:
	}
// L123:
    }
    if (idot > *l1) {
	goto L127;
    }
    idij = 0;
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
	idij += 2;
	i__2 = *ido;
	for (i__ = 4; i__ <= i__2; i__ += 2) {
	    idij += 2;
	    i__3 = *l1;
	    for (k = 1; k <= i__3; ++k) {
		c1[i__ - 1 + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1] * ch[
			i__ - 1 + (k + j * ch_dim2) * ch_dim1] - wa[idij] * 
			ch[i__ + (k + j * ch_dim2) * ch_dim1];
		c1[i__ + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1] * ch[i__ 
			+ (k + j * ch_dim2) * ch_dim1] + wa[idij] * ch[i__ - 
			1 + (k + j * ch_dim2) * ch_dim1];
// L124:
	    }
// L125:
	}
// L126:
    }
    return 0;
L127:
    idj = 2 - *ido;
    i__1 = *ip;
    for (j = 2; j <= i__1; ++j) {
	idj += *ido;
	i__2 = *l1;
	for (k = 1; k <= i__2; ++k) {
	    idij = idj;
	    i__3 = *ido;
	    for (i__ = 4; i__ <= i__3; i__ += 2) {
		idij += 2;
		c1[i__ - 1 + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1] * ch[
			i__ - 1 + (k + j * ch_dim2) * ch_dim1] - wa[idij] * 
			ch[i__ + (k + j * ch_dim2) * ch_dim1];
		c1[i__ + (k + j * c1_dim2) * c1_dim1] = wa[idij - 1] * ch[i__ 
			+ (k + j * ch_dim2) * ch_dim1] + wa[idij] * ch[i__ - 
			1 + (k + j * ch_dim2) * ch_dim1];
// L128:
	    }
// L129:
	}
// L130:
    }
    return 0;
} // passb
//------------------------------------------------------------------------
int32_t passb2d(int32_t *ido, int32_t *l1, double *cc, double *ch, double *wa1)
{
    // System generated locals
    int32_t cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

    // Local variables
    int32_t i__, k;
    double ti2, tr2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = cc_dim1 * 3 + 1;
    cc -= cc_offset;
    --wa1;

    // Function Body
    if (*ido > 2) {
	goto L102;
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ch[(k + ch_dim2) * ch_dim1 + 1] = cc[((k << 1) + 1) * cc_dim1 + 1] + 
		cc[((k << 1) + 2) * cc_dim1 + 1];
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = cc[((k << 1) + 1) * cc_dim1 
		+ 1] - cc[((k << 1) + 2) * cc_dim1 + 1];
	ch[(k + ch_dim2) * ch_dim1 + 2] = cc[((k << 1) + 1) * cc_dim1 + 2] + 
		cc[((k << 1) + 2) * cc_dim1 + 2];
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 2] = cc[((k << 1) + 1) * cc_dim1 
		+ 2] - cc[((k << 1) + 2) * cc_dim1 + 2];
// L101:
    }
    return 0;
L102:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i__ = 2; i__ <= i__2; i__ += 2) {
	    ch[i__ - 1 + (k + ch_dim2) * ch_dim1] = cc[i__ - 1 + ((k << 1) + 
		    1) * cc_dim1] + cc[i__ - 1 + ((k << 1) + 2) * cc_dim1];
	    tr2 = cc[i__ - 1 + ((k << 1) + 1) * cc_dim1] - cc[i__ - 1 + ((k <<
		     1) + 2) * cc_dim1];
	    ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + ((k << 1) + 1) * 
		    cc_dim1] + cc[i__ + ((k << 1) + 2) * cc_dim1];
	    ti2 = cc[i__ + ((k << 1) + 1) * cc_dim1] - cc[i__ + ((k << 1) + 2)
		     * cc_dim1];
	    ch[i__ + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * ti2 + 
		    wa1[i__] * tr2;
	    ch[i__ - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * tr2 
		    - wa1[i__] * ti2;
// L103:
	}
// L104:
    }
    return 0;
} // passb2
//------------------------------------------------------------------------
int32_t passb3d(int32_t *ido, int32_t *l1, double *cc, double *ch, double *wa1, double *wa2)
{
    // Initialized data

    double taur = (double)-.5;
    double taui = (double).866025403784439;

    // System generated locals
    int32_t cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

    // Local variables
    int32_t i__, k;
    double ci2, ci3, di2, di3, cr2, cr3, dr2, dr3, ti2, tr2;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = (cc_dim1 << 2) + 1;
    cc -= cc_offset;
    --wa1;
    --wa2;

    // Function Body
    if (*ido != 2) {
	goto L102;
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	tr2 = cc[(k * 3 + 2) * cc_dim1 + 1] + cc[(k * 3 + 3) * cc_dim1 + 1];
	cr2 = cc[(k * 3 + 1) * cc_dim1 + 1] + taur * tr2;
	ch[(k + ch_dim2) * ch_dim1 + 1] = cc[(k * 3 + 1) * cc_dim1 + 1] + tr2;
	ti2 = cc[(k * 3 + 2) * cc_dim1 + 2] + cc[(k * 3 + 3) * cc_dim1 + 2];
	ci2 = cc[(k * 3 + 1) * cc_dim1 + 2] + taur * ti2;
	ch[(k + ch_dim2) * ch_dim1 + 2] = cc[(k * 3 + 1) * cc_dim1 + 2] + ti2;
	cr3 = taui * (cc[(k * 3 + 2) * cc_dim1 + 1] - cc[(k * 3 + 3) * 
		cc_dim1 + 1]);
	ci3 = taui * (cc[(k * 3 + 2) * cc_dim1 + 2] - cc[(k * 3 + 3) * 
		cc_dim1 + 2]);
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = cr2 - ci3;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 1] = cr2 + ci3;
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 2] = ci2 + cr3;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 2] = ci2 - cr3;
// L101:
    }
    return 0;
L102:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i__ = 2; i__ <= i__2; i__ += 2) {
	    tr2 = cc[i__ - 1 + (k * 3 + 2) * cc_dim1] + cc[i__ - 1 + (k * 3 + 
		    3) * cc_dim1];
	    cr2 = cc[i__ - 1 + (k * 3 + 1) * cc_dim1] + taur * tr2;
	    ch[i__ - 1 + (k + ch_dim2) * ch_dim1] = cc[i__ - 1 + (k * 3 + 1) *
		     cc_dim1] + tr2;
	    ti2 = cc[i__ + (k * 3 + 2) * cc_dim1] + cc[i__ + (k * 3 + 3) * 
		    cc_dim1];
	    ci2 = cc[i__ + (k * 3 + 1) * cc_dim1] + taur * ti2;
	    ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + (k * 3 + 1) * 
		    cc_dim1] + ti2;
	    cr3 = taui * (cc[i__ - 1 + (k * 3 + 2) * cc_dim1] - cc[i__ - 1 + (
		    k * 3 + 3) * cc_dim1]);
	    ci3 = taui * (cc[i__ + (k * 3 + 2) * cc_dim1] - cc[i__ + (k * 3 + 
		    3) * cc_dim1]);
	    dr2 = cr2 - ci3;
	    dr3 = cr2 + ci3;
	    di2 = ci2 + cr3;
	    di3 = ci2 - cr3;
	    ch[i__ + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * di2 + 
		    wa1[i__] * dr2;
	    ch[i__ - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * dr2 
		    - wa1[i__] * di2;
	    ch[i__ + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 1] * di3 + wa2[
		    i__] * dr3;
	    ch[i__ - 1 + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 1] * dr3 - 
		    wa2[i__] * di3;
// L103:
	}
// L104:
    }
    return 0;
} // passb3
//------------------------------------------------------------------------
int32_t passb4d(int32_t *ido, int32_t *l1, double *cc, double *ch, double *wa1, double *wa2,
	   double *wa3)
{
    // System generated locals
    int32_t cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

    // Local variables
    int32_t i__, k;
    double ci2, ci3, ci4, cr2, cr3, cr4, ti1, ti2, ti3, ti4, tr1, tr2, 
	    tr3, tr4;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = cc_dim1 * 5 + 1;
    cc -= cc_offset;
    --wa1;
    --wa2;
    --wa3;

    // Function Body
    if (*ido != 2) {
	goto L102;
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ti1 = cc[((k << 2) + 1) * cc_dim1 + 2] - cc[((k << 2) + 3) * cc_dim1 
		+ 2];
	ti2 = cc[((k << 2) + 1) * cc_dim1 + 2] + cc[((k << 2) + 3) * cc_dim1 
		+ 2];
	tr4 = cc[((k << 2) + 4) * cc_dim1 + 2] - cc[((k << 2) + 2) * cc_dim1 
		+ 2];
	ti3 = cc[((k << 2) + 2) * cc_dim1 + 2] + cc[((k << 2) + 4) * cc_dim1 
		+ 2];
	tr1 = cc[((k << 2) + 1) * cc_dim1 + 1] - cc[((k << 2) + 3) * cc_dim1 
		+ 1];
	tr2 = cc[((k << 2) + 1) * cc_dim1 + 1] + cc[((k << 2) + 3) * cc_dim1 
		+ 1];
	ti4 = cc[((k << 2) + 2) * cc_dim1 + 1] - cc[((k << 2) + 4) * cc_dim1 
		+ 1];
	tr3 = cc[((k << 2) + 2) * cc_dim1 + 1] + cc[((k << 2) + 4) * cc_dim1 
		+ 1];
	ch[(k + ch_dim2) * ch_dim1 + 1] = tr2 + tr3;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 1] = tr2 - tr3;
	ch[(k + ch_dim2) * ch_dim1 + 2] = ti2 + ti3;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 2] = ti2 - ti3;
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = tr1 + tr4;
	ch[(k + (ch_dim2 << 2)) * ch_dim1 + 1] = tr1 - tr4;
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 2] = ti1 + ti4;
	ch[(k + (ch_dim2 << 2)) * ch_dim1 + 2] = ti1 - ti4;
// L101:
    }
    return 0;
L102:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i__ = 2; i__ <= i__2; i__ += 2) {
	    ti1 = cc[i__ + ((k << 2) + 1) * cc_dim1] - cc[i__ + ((k << 2) + 3)
		     * cc_dim1];
	    ti2 = cc[i__ + ((k << 2) + 1) * cc_dim1] + cc[i__ + ((k << 2) + 3)
		     * cc_dim1];
	    ti3 = cc[i__ + ((k << 2) + 2) * cc_dim1] + cc[i__ + ((k << 2) + 4)
		     * cc_dim1];
	    tr4 = cc[i__ + ((k << 2) + 4) * cc_dim1] - cc[i__ + ((k << 2) + 2)
		     * cc_dim1];
	    tr1 = cc[i__ - 1 + ((k << 2) + 1) * cc_dim1] - cc[i__ - 1 + ((k <<
		     2) + 3) * cc_dim1];
	    tr2 = cc[i__ - 1 + ((k << 2) + 1) * cc_dim1] + cc[i__ - 1 + ((k <<
		     2) + 3) * cc_dim1];
	    ti4 = cc[i__ - 1 + ((k << 2) + 2) * cc_dim1] - cc[i__ - 1 + ((k <<
		     2) + 4) * cc_dim1];
	    tr3 = cc[i__ - 1 + ((k << 2) + 2) * cc_dim1] + cc[i__ - 1 + ((k <<
		     2) + 4) * cc_dim1];
	    ch[i__ - 1 + (k + ch_dim2) * ch_dim1] = tr2 + tr3;
	    cr3 = tr2 - tr3;
	    ch[i__ + (k + ch_dim2) * ch_dim1] = ti2 + ti3;
	    ci3 = ti2 - ti3;
	    cr2 = tr1 + tr4;
	    cr4 = tr1 - tr4;
	    ci2 = ti1 + ti4;
	    ci4 = ti1 - ti4;
	    ch[i__ - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * cr2 
		    - wa1[i__] * ci2;
	    ch[i__ + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * ci2 + 
		    wa1[i__] * cr2;
	    ch[i__ - 1 + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 1] * cr3 - 
		    wa2[i__] * ci3;
	    ch[i__ + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 1] * ci3 + wa2[
		    i__] * cr3;
	    ch[i__ - 1 + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i__ - 1] * cr4 
		    - wa3[i__] * ci4;
	    ch[i__ + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i__ - 1] * ci4 + 
		    wa3[i__] * cr4;
// L103:
	}
// L104:
    }
    return 0;
} // passb4
//------------------------------------------------------------------------
int32_t passb5d(int32_t *ido, int32_t *l1, double *cc, double *ch, double *wa1, double *wa2,
	   double *wa3, double *wa4)
{
    // Initialized data

    double tr11 = (double).309016994374947;
    double ti11 = (double).951056516295154;
    double tr12 = (double)-.809016994374947;
    double ti12 = (double).587785252292473;

    // System generated locals
    int32_t cc_dim1, cc_offset, ch_dim1, ch_dim2, ch_offset, i__1, i__2;

    // Local variables
    int32_t i__, k;
    double ci2, ci3, ci4, ci5, di3, di4, di5, di2, cr2, cr3, cr5, cr4, 
	    ti2, ti3, ti4, ti5, dr3, dr4, dr5, dr2, tr2, tr3, tr4, tr5;

    // Parameter adjustments
    ch_dim1 = *ido;
    ch_dim2 = *l1;
    ch_offset = ch_dim1 * (ch_dim2 + 1) + 1;
    ch -= ch_offset;
    cc_dim1 = *ido;
    cc_offset = cc_dim1 * 6 + 1;
    cc -= cc_offset;
    --wa1;
    --wa2;
    --wa3;
    --wa4;

    // Function Body
    if (*ido != 2) {
	goto L102;
    }
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	ti5 = cc[(k * 5 + 2) * cc_dim1 + 2] - cc[(k * 5 + 5) * cc_dim1 + 2];
	ti2 = cc[(k * 5 + 2) * cc_dim1 + 2] + cc[(k * 5 + 5) * cc_dim1 + 2];
	ti4 = cc[(k * 5 + 3) * cc_dim1 + 2] - cc[(k * 5 + 4) * cc_dim1 + 2];
	ti3 = cc[(k * 5 + 3) * cc_dim1 + 2] + cc[(k * 5 + 4) * cc_dim1 + 2];
	tr5 = cc[(k * 5 + 2) * cc_dim1 + 1] - cc[(k * 5 + 5) * cc_dim1 + 1];
	tr2 = cc[(k * 5 + 2) * cc_dim1 + 1] + cc[(k * 5 + 5) * cc_dim1 + 1];
	tr4 = cc[(k * 5 + 3) * cc_dim1 + 1] - cc[(k * 5 + 4) * cc_dim1 + 1];
	tr3 = cc[(k * 5 + 3) * cc_dim1 + 1] + cc[(k * 5 + 4) * cc_dim1 + 1];
	ch[(k + ch_dim2) * ch_dim1 + 1] = cc[(k * 5 + 1) * cc_dim1 + 1] + tr2 
		+ tr3;
	ch[(k + ch_dim2) * ch_dim1 + 2] = cc[(k * 5 + 1) * cc_dim1 + 2] + ti2 
		+ ti3;
	cr2 = cc[(k * 5 + 1) * cc_dim1 + 1] + tr11 * tr2 + tr12 * tr3;
	ci2 = cc[(k * 5 + 1) * cc_dim1 + 2] + tr11 * ti2 + tr12 * ti3;
	cr3 = cc[(k * 5 + 1) * cc_dim1 + 1] + tr12 * tr2 + tr11 * tr3;
	ci3 = cc[(k * 5 + 1) * cc_dim1 + 2] + tr12 * ti2 + tr11 * ti3;
	cr5 = ti11 * tr5 + ti12 * tr4;
	ci5 = ti11 * ti5 + ti12 * ti4;
	cr4 = ti12 * tr5 - ti11 * tr4;
	ci4 = ti12 * ti5 - ti11 * ti4;
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 1] = cr2 - ci5;
	ch[(k + ch_dim2 * 5) * ch_dim1 + 1] = cr2 + ci5;
	ch[(k + (ch_dim2 << 1)) * ch_dim1 + 2] = ci2 + cr5;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 2] = ci3 + cr4;
	ch[(k + ch_dim2 * 3) * ch_dim1 + 1] = cr3 - ci4;
	ch[(k + (ch_dim2 << 2)) * ch_dim1 + 1] = cr3 + ci4;
	ch[(k + (ch_dim2 << 2)) * ch_dim1 + 2] = ci3 - cr4;
	ch[(k + ch_dim2 * 5) * ch_dim1 + 2] = ci2 - cr5;
// L101:
    }
    return 0;
L102:
    i__1 = *l1;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *ido;
	for (i__ = 2; i__ <= i__2; i__ += 2) {
	    ti5 = cc[i__ + (k * 5 + 2) * cc_dim1] - cc[i__ + (k * 5 + 5) * 
		    cc_dim1];
	    ti2 = cc[i__ + (k * 5 + 2) * cc_dim1] + cc[i__ + (k * 5 + 5) * 
		    cc_dim1];
	    ti4 = cc[i__ + (k * 5 + 3) * cc_dim1] - cc[i__ + (k * 5 + 4) * 
		    cc_dim1];
	    ti3 = cc[i__ + (k * 5 + 3) * cc_dim1] + cc[i__ + (k * 5 + 4) * 
		    cc_dim1];
	    tr5 = cc[i__ - 1 + (k * 5 + 2) * cc_dim1] - cc[i__ - 1 + (k * 5 + 
		    5) * cc_dim1];
	    tr2 = cc[i__ - 1 + (k * 5 + 2) * cc_dim1] + cc[i__ - 1 + (k * 5 + 
		    5) * cc_dim1];
	    tr4 = cc[i__ - 1 + (k * 5 + 3) * cc_dim1] - cc[i__ - 1 + (k * 5 + 
		    4) * cc_dim1];
	    tr3 = cc[i__ - 1 + (k * 5 + 3) * cc_dim1] + cc[i__ - 1 + (k * 5 + 
		    4) * cc_dim1];
	    ch[i__ - 1 + (k + ch_dim2) * ch_dim1] = cc[i__ - 1 + (k * 5 + 1) *
		     cc_dim1] + tr2 + tr3;
	    ch[i__ + (k + ch_dim2) * ch_dim1] = cc[i__ + (k * 5 + 1) * 
		    cc_dim1] + ti2 + ti3;
	    cr2 = cc[i__ - 1 + (k * 5 + 1) * cc_dim1] + tr11 * tr2 + tr12 * 
		    tr3;
	    ci2 = cc[i__ + (k * 5 + 1) * cc_dim1] + tr11 * ti2 + tr12 * ti3;
	    cr3 = cc[i__ - 1 + (k * 5 + 1) * cc_dim1] + tr12 * tr2 + tr11 * 
		    tr3;
	    ci3 = cc[i__ + (k * 5 + 1) * cc_dim1] + tr12 * ti2 + tr11 * ti3;
	    cr5 = ti11 * tr5 + ti12 * tr4;
	    ci5 = ti11 * ti5 + ti12 * ti4;
	    cr4 = ti12 * tr5 - ti11 * tr4;
	    ci4 = ti12 * ti5 - ti11 * ti4;
	    dr3 = cr3 - ci4;
	    dr4 = cr3 + ci4;
	    di3 = ci3 + cr4;
	    di4 = ci3 - cr4;
	    dr5 = cr2 + ci5;
	    dr2 = cr2 - ci5;
	    di5 = ci2 - cr5;
	    di2 = ci2 + cr5;
	    ch[i__ - 1 + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * dr2 
		    - wa1[i__] * di2;
	    ch[i__ + (k + (ch_dim2 << 1)) * ch_dim1] = wa1[i__ - 1] * di2 + 
		    wa1[i__] * dr2;
	    ch[i__ - 1 + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 1] * dr3 - 
		    wa2[i__] * di3;
	    ch[i__ + (k + ch_dim2 * 3) * ch_dim1] = wa2[i__ - 1] * di3 + wa2[
		    i__] * dr3;
	    ch[i__ - 1 + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i__ - 1] * dr4 
		    - wa3[i__] * di4;
	    ch[i__ + (k + (ch_dim2 << 2)) * ch_dim1] = wa3[i__ - 1] * di4 + 
		    wa3[i__] * dr4;
	    ch[i__ - 1 + (k + ch_dim2 * 5) * ch_dim1] = wa4[i__ - 1] * dr5 - 
		    wa4[i__] * di5;
	    ch[i__ + (k + ch_dim2 * 5) * ch_dim1] = wa4[i__ - 1] * di5 + wa4[
		    i__] * dr5;
// L103:
	}
// L104:
    }
    return 0;
} // passb5
//------------------------------------------------------------------------
int32_t cfftb1d(int32_t *n, double *c__, double *ch, double *wa, double *ifac)
{
    // System generated locals
    int32_t i__1;

    // Local variables
    int32_t idot, i__;
    int32_t passbd(int32_t *, int32_t *, int32_t *, int32_t *, int32_t *, double *, double *, double *,
	      double *, double *, double *); 
    int32_t k1, l1, l2, n2;
    int32_t passb2d(int32_t *, int32_t *, double *, double *, double *),
      passb3d(int32_t *, int32_t *, double *, double *, double *, double *),
      passb4d(int32_t *, int32_t *, double *, double *, double *, double *, double *),
      passb5d(int32_t *, int32_t *, double *, double *, double *, double *, double *,
	     double *);
    int32_t na, nf, ip, iw, ix2, ix3, ix4, nac, ido, idl1;

    // Parameter adjustments
    --ifac;
    --wa;
    --ch;
    --c__;

    // Function Body
    nf = ifac[2];
    na = 0;
    l1 = 1;
    iw = 1;
    i__1 = nf;
    for (k1 = 1; k1 <= i__1; ++k1) {
	ip = ifac[k1 + 2];
	l2 = ip * l1;
	ido = *n / l2;
	idot = ido + ido;
	idl1 = idot * l1;
	if (ip != 4) {
	    goto L103;
	}
	ix2 = iw + idot;
	ix3 = ix2 + idot;
	if (na != 0) {
	    goto L101;
	}
	passb4d(&idot, &l1, &c__[1], &ch[1], &wa[iw], &wa[ix2], &wa[ix3]);
	goto L102;
L101:
	passb4d(&idot, &l1, &ch[1], &c__[1], &wa[iw], &wa[ix2], &wa[ix3]);
L102:
	na = 1 - na;
	goto L115;
L103:
	if (ip != 2) {
	    goto L106;
	}
	if (na != 0) {
	    goto L104;
	}
	passb2d(&idot, &l1, &c__[1], &ch[1], &wa[iw]);
	goto L105;
L104:
	passb2d(&idot, &l1, &ch[1], &c__[1], &wa[iw]);
L105:
	na = 1 - na;
	goto L115;
L106:
	if (ip != 3) {
	    goto L109;
	}
	ix2 = iw + idot;
	if (na != 0) {
	    goto L107;
	}
	passb3d(&idot, &l1, &c__[1], &ch[1], &wa[iw], &wa[ix2]);
	goto L108;
L107:
	passb3d(&idot, &l1, &ch[1], &c__[1], &wa[iw], &wa[ix2]);
L108:
	na = 1 - na;
	goto L115;
L109:
	if (ip != 5) {
	    goto L112;
	}
	ix2 = iw + idot;
	ix3 = ix2 + idot;
	ix4 = ix3 + idot;
	if (na != 0) {
	    goto L110;
	}
	passb5d(&idot, &l1, &c__[1], &ch[1], &wa[iw], &wa[ix2], &wa[ix3], &wa[
		ix4]);
	goto L111;
L110:
	passb5d(&idot, &l1, &ch[1], &c__[1], &wa[iw], &wa[ix2], &wa[ix3], &wa[
		ix4]);
L111:
	na = 1 - na;
	goto L115;
L112:
	if (na != 0) {
	    goto L113;
	}
	passbd(&nac, &idot, &ip, &l1, &idl1, &c__[1], &c__[1], &c__[1], &ch[1]
		, &ch[1], &wa[iw]);
	goto L114;
L113:
	passbd(&nac, &idot, &ip, &l1, &idl1, &ch[1], &ch[1], &ch[1], &c__[1], 
		&c__[1], &wa[iw]);
L114:
	if (nac != 0) {
	    na = 1 - na;
	}
L115:
	l1 = l2;
	iw += (ip - 1) * idot;
// L116:
    }
    if (na == 0) {
	return 0;
    }
    n2 = *n + *n;
    i__1 = n2;
    for (i__ = 1; i__ <= i__1; ++i__) {
	c__[i__] = ch[i__];
// L117:
    }
    return 0;
} // cfftb1
//------------------------------------------------------------------------
int32_t cfftbd(int32_t *n, double *c__, double *wsave)
{
  int32_t cfftb1d(int32_t *, double *, double *, double *, double *);
    int32_t iw1, iw2;

    // Parameter adjustments
    --wsave;
    --c__;

    // Function Body
    if (*n == 1) {
	return 0;
    }
    iw1 = *n + *n + 1;
    iw2 = iw1 + *n + *n;
    cfftb1d(n, &c__[1], &wsave[1], &wsave[iw1], &wsave[iw2]);
    return 0;
} // cfftb
//------------------------------------------------------------------------
int32_t cffti1d(int32_t *n, double *wa, double *ifac)
{
    // Initialized data

    int32_t ntryh[4] = { 3,4,2,5 };

    // System generated locals
    int32_t i__1, i__2, i__3;

    // Builtin functions
    double cos(double), sin(double);

    // Local variables
    double argh;
    int32_t idot, ntry, i__, j;
    double argld;
    int32_t i1, k1, l1, l2, ib;
    double fi;
    int32_t ld, ii, nf, ip, nl, nq, nr;
    double arg;
    int32_t ido, ipm;
    double tpi;

    // Parameter adjustments
    --ifac;
    --wa;

    // Function Body
    nl = *n;
    nf = 0;
    j = 0;
L101:
    ++j;
    if (j - 4 <= 0) {
	goto L102;
    } else {
	goto L103;
    }
L102:
    ntry = ntryh[j - 1];
    goto L104;
L103:
    ntry += 2;
L104:
    nq = nl / ntry;
    nr = nl - ntry * nq;
    if (nr != 0) {
	goto L101;
    } else {
	goto L105;
    }
L105:
    ++nf;
    ifac[nf + 2] = ntry;
    nl = nq;
    if (ntry != 2) {
	goto L107;
    }
    if (nf == 1) {
	goto L107;
    }
    i__1 = nf;
    for (i__ = 2; i__ <= i__1; ++i__) {
	ib = nf - i__ + 2;
	ifac[ib + 2] = ifac[ib + 1];
// L106:
    }
    ifac[3] = 2;
L107:
    if (nl != 1) {
	goto L104;
    }
    ifac[1] = *n;
    ifac[2] = nf;
    tpi = (double)6.28318530717959;
    argh = tpi / (double) (*n);
    i__ = 2;
    l1 = 1;
    i__1 = nf;
    for (k1 = 1; k1 <= i__1; ++k1) {
	ip = ifac[k1 + 2];
	ld = 0;
	l2 = l1 * ip;
	ido = *n / l2;
	idot = ido + ido + 2;
	ipm = ip - 1;
	i__2 = ipm;
	for (j = 1; j <= i__2; ++j) {
	    i1 = i__;
	    wa[i__ - 1] = (double)1.;
	    wa[i__] = (double)0.;
	    ld += l1;
	    fi = (double)0.;
	    argld = (double) ld * argh;
	    i__3 = idot;
	    for (ii = 4; ii <= i__3; ii += 2) {
		i__ += 2;
		fi += (double)1.;
		arg = fi * argld;
		wa[i__ - 1] = cos(arg);
		wa[i__] = sin(arg);
// L108:
	    }
	    if (ip <= 5) {
		goto L109;
	    }
	    wa[i1 - 1] = wa[i__ - 1];
	    wa[i1] = wa[i__];
L109:
	    ;
	}
	l1 = l2;
// L110:
    }
    return 0;
} // cffti1
//------------------------------------------------------------------------
int32_t cfftid(int32_t *n, double *wsave)
{
  int32_t cffti1d(int32_t *, double *, double *);
  int32_t iw1, iw2;

    // Parameter adjustments
    --wsave;

    // Function Body
    if (*n == 1) {
	return 0;
    }
    iw1 = *n + *n + 1;
    iw2 = iw1 + *n + *n;
    cffti1d(n, &wsave[iw1], &wsave[iw2]);
    return 0;
} // cffti
//------------------------------------------------------------------------
