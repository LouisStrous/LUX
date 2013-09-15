/* This is file svd.c.

Copyright 2013 Louis Strous

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
#include <math.h>
#include "anadefs.h"
#include "action.h"

/* The following algorithm is due to Bryant Marks */
/* (bryant@sioux.stanford.edu) 3 April 1993 */

/* This SVD routine is based on pgs 30-48 of "Compact Numerical Methods
   for Computers" by J.C. Nash (1990), used to compute the pseudoinverse.
   Modifications include:
        Translation from Pascal to ANSI C.
        Array indexing from 0 rather than 1.
        Float replaced by Double everywhere.
        Support for the Matrix structure.
        I changed the array indexing so that the matricies (Float [][])
           could be replaced be a single list (Double *) for more
           efficient communication with Mathematica.
*/

#define TOLERANCE 1.0e-22

void SVD(Double *W, Double *Z, Int nRow, Int nCol)
{
  Int i, j, k, EstColRank, RotCount, SweepCount, slimit;
  Double eps, e2, tol, vt, p, h2, x0, y0, q, r, c0, s0, c2, d1, d2;
  eps = TOLERANCE;
  slimit = nCol/4;
  if (slimit < 6.0)
    slimit = 6;
  SweepCount = 0;
  e2 = 10.0*nRow*eps*eps;
  tol = eps*.1;
  EstColRank = nCol;
  for (i=0; i<nCol; i++)
    for (j=0; j<nCol; j++)
      {
        W[nCol*(nRow+i)+j] = 0.0;
        W[nCol*(nRow+i)+i] = 1.0;
      }
  RotCount = EstColRank*(EstColRank-1)/2;
  while (RotCount != 0 && SweepCount <= slimit)
    {
      RotCount = EstColRank*(EstColRank-1)/2;
      SweepCount++;
      for (j=0; j<EstColRank-1; j++)
        {
          for (k=j+1; k<EstColRank; k++)
            {
              p = q = r = 0.0;
              for (i=0; i<nRow; i++)
                {
                  x0 = W[nCol*i+j]; y0 = W[nCol*i+k];
                  p += x0*y0; q += x0*x0; r += y0*y0;
                }
              Z[nCol*j+j] = q; Z[nCol*k+k] = r;
              if (q >= r)
                {
                  if (q<=e2*Z[0] || fabs(p)<=tol*q) RotCount--;
                  else
                    {
                      p /= q; r = 1 - r/q; vt = sqrt(4*p*p+r*r);
                      c0 = sqrt(fabs(.5*(1+r/vt))); s0 = p/(vt*c0);
                      for (i=0; i<nRow+nCol; i++)
                        {
                          d1 = W[nCol*i+j]; d2 = W[nCol*i+k];
                          W[nCol*i+j] = d1*c0+d2*s0; W[nCol*i+k] = -d1*s0+d2*c0;
                        }
                    }
                }
              else
                {
                  p /= r; q = q/r-1; vt = sqrt(4*p*p+q*q);
                  s0 = sqrt(fabs(.5*(1-q/vt)));
                  if (p<0) s0 = -s0;
                  c0 = p/(vt*s0);
                  for (i=0; i<nRow+nCol; i++)
                    {
                      d1 = W[nCol*i+j]; d2 = W[nCol*i+k];
                      W[nCol*i+j] = d1*c0+d2*s0; W[nCol*i+k] = -d1*s0+d2*c0;
                    }
                }
            }
        }
      while (EstColRank>=3 && Z[nCol*(EstColRank-1)+(EstColRank-1)]<=Z[0]*tol+tol*tol)
        EstColRank--;
    }
#if DEBUG
  if (SweepCount > slimit)
    fprintf(stderr, "Sweeps = %d\n", SweepCount);
#endif
}

Int ana_svd(Int narg, Int ps[])
/* calculates the singular value decomposition of matrix A, such that */
/* A = U S V' */
/* SVD,A,U,S,V */
{
  Int	iq, nRow, nCol, *d, n;
  Double	*a, *z;
  pointer	src;

/*  void SVD(Double *W, Double *Z, Int nRow, Int nCol) */

  if (symbol_class(ps[0]) != ANA_ARRAY
      || array_num_dims(ps[0]) != 2)
    return cerror(NEED_2D_ARR, ps[0]);
  d = array_dims(ps[0]);
  if (d[0] > d[1])
    return anaerror("#columns > #rows in input matrix in SVD", ps[0]);
  a = (Double *) Malloc(d[0]*(d[1] + d[0])*sizeof(Double));
  if (!a)
    return cerror(ALLOC_ERR, 0);
  z = (Double *) Malloc(d[1]*sizeof(Double));
  n = d[0]*d[1];
  src = array_data(ps[0]);
  switch (symbol_type(ps[0])) {
    case ANA_BYTE:
      while (n--)
	*a++ = (Double) *src.b++;
      a -= n;
      break;
    case ANA_WORD:
      while (n--)
	*a++ = (Double) *src.w++;
      a -= n;
      break;
    case ANA_LONG:
      while (n--)
	*a++ = (Double) *src.l++;
      a -= n;
      break;
    case ANA_FLOAT:
      while (n--)
	*a++ = (Double) *src.f++;
      a -= n;
      break;
    case ANA_DOUBLE:
      while (n--)
	*a++ = (Double) *src.d++;
      a -= n;
      break;
    }rlof
  SVD(a, z, d[1], d[0]);

  if (ps[1])
    undefine(ps[1]);
  ps[1] = array_scratch(ANA_DOUBLE, 2, d);
  n = d[0]*d[1];
  memcpy(array_data(ps[1]), a, d[0]*d[1]);

  if (ps[2])
    undefine(ps[2]);
  ps[2] = array_scratch(ANA_DOUBLE, 1, d + 1);
  memcpy(array_data(ps[2]), z, d[1]);

  if (ps[3])
    undefine(ps[3]);
  d[1] = d[0];
  ps[3] = array_scratch(ANA_DOUBLE, 2, d);
  memcpy(array_data(ps[3]), a + n, d[0]*d[0]);
  
  Free(a);
  Free(z);
  return 1;
}
