/* This is file fun4.cc.

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
// File fun4.c
// Various LUX functions.
/// \file
#include "config.h"
#include <cassert>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "action.hh"
#include "install.hh"

#define BI_CUBIC_SMOOTH         4
#define BI_CUBIC        3

extern  int32_t         lastmin_sym, lastmax_sym;
extern unsigned long    *pixels;
int32_t         maxregridsize = 2048, stretchmark, tvsmt, badmatch, stretch_clip = 19;
int32_t         islit, dstype, itmax = 20, sort_flag = 0;
float   gwid, xoffset, yoffset, xyres;
int32_t         resample_type = BI_CUBIC_SMOOTH;
void match_1(int32_t *p1, int32_t *p2, int32_t nxa, int32_t nxb, int32_t nya, int32_t nyb, int32_t nx,
             int32_t ny, float *gwx, float *gwy),
  gwind0(float *gwx, float *gwy, float gwid, int32_t nxa, int32_t nxb, int32_t nya,
         int32_t nyb),
  unbias(void *m1, void *m2, int32_t nxa, int32_t nxb, int32_t nya, int32_t nyb,
         int32_t nxs, int32_t nys, float *gx, float *gy, float *av1, float *av2,
         float *cx, float *cy, float *cxx, float *cxy, float *cyy,
         int32_t idelx, int32_t idely);
float averag(void *m, int32_t nxa, int32_t nxb, int32_t nya, int32_t nyb, int32_t nxs, int32_t nys,
             int32_t idx, int32_t idy, float *gx, float *gy),
  resid(int32_t *m1, int32_t *m2, int32_t idx, int32_t idy, int32_t nxa, int32_t nxb, int32_t nya,
        int32_t nyb, int32_t nxs, int32_t nys, int32_t ndmx, float *gx, float *gy,
        float bs);
//-------------------------------------------------------------------------
int32_t lux_gridmatch(ArgumentCount narg, Symbol ps[])/* gridmatch function */               
/* the call is offsets = gridmatch(m1,m2,gx,gy,dx,dy,gwid,mode)
        where   m1 = reference input image
        m2 = image to compare with m1, m1 and m2 must be same size
        gx = array of x gridpoints
        gy = array of y gridpoints, gx and gy must have same size
        dx and dy are the window size, and gwid is the gaussian mask width

        if mode is specified and unequal to 0, then the quality parameter
        is also returned in result (third "coordinate")  LS 14sep92
*/
{
 int32_t        type, nx, ny, nxg, nyg, dx, dy, dim[3];
 int32_t        result_sym, nc, i1, i2, j1, j2, dx2, dy2, mode;
 int32_t        *gx, *gy;
 float  *out, *gwx, *gwy;
 Pointer p1, p2;

 // <m1> must be a 2D numerical array
 if (!symbolIsNumericalArray(ps[0]))
   return cerror(NEED_NUM_ARR, ps[0]);
 if (array_num_dims(ps[0]) != 2)
   return cerror(NEED_2D_ARR, ps[0]);

 nx = array_dims(ps[0])[0];     // <m1> dimensions
 ny = array_dims(ps[0])[1];

 // <m2> must be a 2D numerical array with the same dimensions as <m1>
 if (!symbolIsNumericalArray(ps[1]))
   return cerror(NEED_NUM_ARR, ps[1]);
 if (array_num_dims(ps[1]) != 2)
   return cerror(NEED_2D_ARR, ps[1]);
 if (array_dims(ps[1])[0] != nx
     || array_dims(ps[1])[1] != ny)
   return cerror(INCMP_ARG, ps[1]);

 // <gx> must be a 2D numerical array
 if (!symbolIsNumericalArray(ps[2]))
   return cerror(NEED_NUM_ARR, ps[2]);
 if (array_num_dims(ps[0]) != 2)
   return cerror(NEED_2D_ARR, ps[2]);

 nxg = array_dims(ps[2])[0];    // <gx> dimensions
 nyg = array_dims(ps[2])[1];

 // <gy> must be a 2D numerical array with the same dimensions as <m1>
 if (!symbolIsNumericalArray(ps[3]))
   return cerror(NEED_NUM_ARR, ps[3]);
 if (array_num_dims(ps[3]) != 2)
   return cerror(NEED_2D_ARR, ps[3]);
 if (array_dims(ps[3])[0] != nxg
     || array_dims(ps[3])[1] != nyg)
   return cerror(INCMP_ARG, ps[3]);

 // we upgrade <m1> and <m2> to the greater of their data types
 type = array_type(ps[0]);
 if (array_type(ps[1]) > type) {
   type = array_type(ps[1]);
   p1.i32 = (int32_t*) array_data(lux_converts[type](1, &ps[0]));
   p2.i32 = (int32_t*) array_data(ps[1]);
 } else {
   p1.i32 = (int32_t*) array_data(ps[0]);
   p2.i32 = (int32_t*) array_data(lux_converts[type](1, &ps[1]));
 }
 dstype = type;

 // we convert <gx> and <gy> to LUX_INT32
 gx = (int32_t*) array_data(lux_long(1, &ps[2]));
 gy = (int32_t*) array_data(lux_long(1, &ps[3]));

 // now get the window size and mask width
 dx = int_arg(ps[4]);
 dy = int_arg(ps[5]);
 gwid = float_arg(ps[6]);
 mode = (narg > 7)? int_arg(ps[7]): 0; // LS 14sep92

 // now prepare the output symbol
 dim[0] = 2;
 dim[1] = nxg;
 dim[2] = nyg;
 if (mode)
   dim[0] = 3;                  // addition LS 14sep92

 result_sym = array_scratch(LUX_FLOAT, 3, dim);
 out = (float*) array_data(result_sym);

 // following converted from macro stretch.mar
 // try to use scratch for gaussian mask arrays (should work)
 if ((nx + ny) <= NSCRAT)
   gwx = (float *) scrat;
 else
   gwx = (float *) malloc((nx + ny)*sizeof(float));
 gwy = gwx + nx;
 nc = nxg*nyg;
 dx2 = dx/2;
 dy2 = dy/2;
 badmatch = 0;
 while (nc--) {                 // loop over grid points and get rectangles
   i1 = *gx - dx2;
   if (i1 < 0)
     i1 = 0;
   i1++;                        // fortran indices
   i2 = *gx++ + dx2;
   if (i2 > nx)
     i2 = nx;
   j1 = *gy - dy2;
   if (j1 < 0)
     j1 = 0;
   j1++;                        // fortran indices
   j2 = *gy++ + dy2;
   if (j2 > ny)
     j2 = ny;
   xoffset = yoffset = 0;
   // convert range to C (0 base) indices, do separately while debugging
   i1--;
   i2--;
   j1--;
   j2--;
   // get gaussian masks
   gwind0(gwx, gwy, gwid, i1, i2, j1, j2);
   match_1(p1.i32, p2.i32, i1, i2, j1, j2, nx, ny, gwx, gwy);
   *out++ = xoffset;
   *out++ = yoffset;
   if (mode != 0)
     *out++ = xyres;            // addition LS 14sep92
 }
 if (gwx != (float *) scrat)
   free(gwx);                   // free space if it was allocated
 return         result_sym;
}
 //-------------------------------------------------------------------------
void match_1(int32_t *p1, int32_t *p2, int32_t nxa, int32_t nxb, int32_t nya, int32_t nyb, int32_t nx,
            int32_t ny, float *gwx, float *gwy)
 // note that xoffset and yoffset are zeroed before we get in here
{
  int32_t       idelx, idely, i, j, k, ndmx=1000, done[9];
  int32_t       di, dj, in, jn, iter, dd, badflag;
  float         av1, av2, cx, cy,cxx,cxy,cyy, avdif, t, res[9], buf[9], t1, t2;
  void  getmin(float *, float *, float *);

  for (i = 0; i < 9; i++)
    done[i] = 0;
  idelx = rint(xoffset);
  idely = rint(yoffset);
  unbias(p1, p2, nxa, nxb, nya, nyb, nx, ny, gwx, gwy, &av1, &av2, &cx, &cy,
         &cxx, &cxy, &cyy, idelx, idely);
  /* look at a 3x3 matrix of residuals centered at 0 offset, find the location
     of the minimum, if not at center, then look at new 3x3 centered
     on the edge minimum; repeat until min at center */
  iter = itmax;
  badflag = 0;
  while (iter--) {
    for (k = 0; k < 9; k++) {
      if (done[k] == 0) {
        i = idelx + (k % 3) - 1;
        j = idely + (k / 3) - 1;
        avdif = av2 + i*cx + j*cy + i*i*cxx + i*j*cxy + j*j*cyy - av1;
        res[k] = resid(p1, p2, i, j, nxa, nxb, nya, nyb, nx, ny, ndmx,
                       gwx, gwy, avdif);
      }
    }
    t = res[4];
    i = 4;                      // if we're in an all-zero area, then
                                // return a zero. LS 28aug2000
    for (k=0;k<9;k++)
      if (res[k] < t) {
        t = res[k];
        i = k;
      }
    if (t < 0) {                // added LS 19feb95
      if (internalMode & 1)
        printf("match - ran out of data at edge\n");
      badflag = 1;
      break;
    }
    idelx += (i % 3) - 1;
    idely += (i / 3) - 1;
    // check if we have gone too far
    if (ABS(idelx) > stretch_clip || ABS(idely) > stretch_clip) {
      if (internalMode & 1)
        printf("match - stretch_clip exceeded\n");
      badflag = 1;
      break;
    }
    if (i == 4)
      break;                    // done if it is the center one
    // not in center, shuffle what we have to put the edge min in center
    di = (i % 3) - 1;
    dj = (i / 3) - 1;
    dd = dj*3 + di;
    for (k = 0; k < 9; k++) {
      in = k%3 + di;
      jn = k/3 + dj;
      if (in >= 0 && jn >= 0 && in < 3 && jn < 3) { // in range
        done[k] = 1;
        buf[k] = res[k + dd];
      } else
        done[k] = 0;            // not in range, mark undone
    }
    for (k = 0;k < 9; k++)
      res[k] = buf[k];          // put back in res array
  } // end of iter while
                                // done or reached itmax, which ?
  if (iter <= 0) {
    if (internalMode & 1)
      printf("match - exceeded maximum iteration\n");
    badflag = 1;
  }
  if (badflag) {
    if (internalMode & 1)
      printf("cell index range = %d %d %d %d\n", nxa, nxb, nya, nyb);
    badmatch++;
    return;
  }
                                        // must have been OK so far
  getmin(res, &t1, &t2);
  xoffset = idelx + t1;
  yoffset = idely + t2;
  xyres = res[4];
  return;
}
//-------------------------------------------------------------------------
void gwind0(float *gwx, float *gwy, float gwid, int32_t nxa, int32_t nxb, int32_t nya,
            int32_t nyb)
{
  float         wid, xcen, ycen, xq;
  int32_t       i;

  wid = gwid*0.6005612;                 // from FWHM to decay scale
  if (wid > 0) {
    xcen = (nxa + nxb)/2;
    ycen = (nya + nyb)/2;
    for (i = nxa; i <= nxb; i++) {
      xq = (i - xcen)/wid;
      gwx[i] = exp(-(xq*xq));
    }
    for (i = nya; i <= nyb; i++) {
      xq = (i - ycen)/wid;
      gwy[i] = exp(-(xq*xq));
    }
  } else {
    for (i = nxa; i <= nxb; i++)
      gwx[i] = 1.0;
    for (i = nya; i <= nyb; i++)
      gwy[i] = 1.0;
  }
}
//-------------------------------------------------------------------------
void unbias(void *m1, void *m2, int32_t nxa, int32_t nxb, int32_t nya, int32_t nyb,
            int32_t nxs, int32_t nys, float *gx, float *gy, float *av1, float *av2,
            float *cx, float *cy, float *cxx, float *cxy, float *cyy,
            int32_t idelx, int32_t idely)
{
  float         t0, t1, t2, t3, t4, t5;

  /*  find weighted means of m1 & m2 over the window
      sets up quadratic fit to average of m2 as a fcn. of offsets */
  *av1 = averag(m1, nxa, nxb, nya, nyb, nxs, nys, 0, 0, gx, gy);
  t0 = averag(m2, nxa, nxb, nya, nyb, nxs, nys, idelx, idely, gx, gy);
  t1 = averag(m2, nxa, nxb, nya, nyb, nxs, nys, idelx + 1, idely, gx, gy);
  t2 = averag(m2, nxa, nxb, nya, nyb, nxs, nys, idelx - 1, idely, gx, gy);
  t3 = averag(m2, nxa, nxb, nya, nyb, nxs, nys, idelx, idely + 1, gx, gy);
  t4 = averag(m2, nxa, nxb, nya, nyb, nxs, nys, idelx, idely - 1, gx, gy);
  t5 = averag(m2, nxa, nxb, nya, nyb, nxs, nys, idelx + 1, idely + 1, gx, gy);
  *av2 = t0;
  *cx = 0.5*(t1 - t2);
  *cy = 0.5*(t3 - t4);
  *cxx = 0.5*(t1 - 2*t0 + t2);
  *cyy = 0.5*(t3 - 2*t0 + t4);
  *cxy = t5 + t0 - t1 - t3;
}
//-------------------------------------------------------------------------
float averag(void *m, int32_t nxa, int32_t nxb, int32_t nya, int32_t nyb, int32_t nxs, int32_t nys,
             int32_t idx, int32_t idy, float *gx, float *gy)
// finds weighted average of array m over the block defined
{
  Pointer p;
  int32_t       nxc, nxd, nyc, nyd, i, j, jj;
  float         sum, sumg, sumx, sumgx;

  p.i32 = (int32_t*) m;
  // fix limits so sum doesn't run off edge of image
  if (nxa + idx < 0)
    nxc = -idx;
  else
    nxc = nxa;
  if (nya + idy < 0)
    nyc = -idy;
  else
    nyc = nya;
  if (nxb + idx > nxs)
    nxd = nxs - idx;
  else
    nxd = nxb;
  if (nyb + idy > nys)
    nyd = nys - idy;
  else
    nyd = nyb;
  sum = sumg = sumgx = 0.0;
  for (i = nxc; i < nxd; i++)
    sumgx += gx[i];
  // weighted sum in window, note switch on case before inner loop
  for (j = nyc; j < nyd; j++) {
    sumx = 0.0;
    jj = idx + nxs*(j + idy);
    switch (dstype) {
      case LUX_INT8:
        for (i = nxc; i < nxd; i++)
          sumx += gx[i]*p.ui8[i + jj];
        break;
      case LUX_INT16:
        for (i = nxc; i < nxd; i++)
          sumx += gx[i]*p.i16[i + jj];
        break;
      case LUX_INT32:
        for (i = nxc; i < nxd; i++)
          sumx += gx[i]*p.i32[i + jj];
        break;
      case LUX_INT64:
        for (i = nxc; i < nxd; i++)
          sumx += gx[i]*p.i64[i + jj];
        break;
      case LUX_FLOAT:
        for (i = nxc; i < nxd; i++)
          sumx += gx[i]*p.f[i + jj];
        break;
      case LUX_DOUBLE:
        for (i = nxc; i < nxd; i++)
          sumx += gx[i]*p.d[i + jj];
        break;
    }
    sum += gy[j]*sumx;
    sumg += gy[j]*sumgx;
  } // end of j loop
  return sum/sumg;
}
//-------------------------------------------------------------------------
float resid(int32_t *m1, int32_t *m2, int32_t idx, int32_t idy, int32_t nxa, int32_t nxb, int32_t nya,
            int32_t nyb, int32_t nxs, int32_t nys, int32_t ndmx, float *gx, float *gy,
            float bs)
{
  int32_t     nxc, nxd, nyc, nyd, nx, ny;
  Pointer   m1p, m2p;
  float   *p1, *p2, *ps;
  float   sum, sumx, t, ndmx2;
  int32_t     i, j;
  float   sumg;
  static  int32_t     mxc, mxd, myc, myd;
  static  float   gsum;

  //set up limits
  nxc = nxa;
  if (nxc + idx < 0)
    nxc = -idx;
  nyc = nya;
  if (nyc + idy < 0)
    nyc = -idy;
  nxd = nxb;
  if (nxd + idx >= nxs)
    nxd = nxs - idx - 1;
  nyd = nyb;
  if (nyd + idy >= nys)
    nyd = nys - idy - 1;
  sum = sumg = 0.0;

  nx = nxd - nxc + 1;
  p2 = gy + nyc;
  ps = gx + nxc;

  if (nxc != mxc || nxd != mxd || nyc != myc || nyd != myd) {
    /*sum gaussians over rectangle to get normalization
      (only if limits change)*/
    j = nyd - nyc + 1;
    if (j <= 0 || nxd - nxc + 1 <= 0)
      return -1;                // added 19feb95 LS
    //printf("in resid, j,nx,*ps = %d %d %e\n",j,nx,*ps);
    while (j) {
      i = nx;
      p1 = ps;
      while (i) {
        sumg += (*p1++) * (*p2);
        i--;
      }
      p2++;
      j--;
    }
    gsum = sumg;
    mxc = nxc;
    mxd = nxd;
    myc = nyc;
    myd = nyd;
  } else
    sumg = gsum;

  //printf("nxc, nxd, nyc, nyd = %d %d %d %d\n",nxc, nxd, nyc, nyd);
  //get start of m1 and m2 and the increments
  m1p.i32 = m1;
  m2p.i32 = m2;
  //printf("m1p.ui8, m2p.ui8 = %x %x\n",m1p.ui8, m2p.ui8);
  m1p.ui8 = m1p.ui8 + lux_type_size[dstype] * ((nyc * nxs ) + nxc);
  m2p.ui8 = m2p.ui8 + lux_type_size[dstype] * (( (nyc+ idy) * nxs ) + nxc + idx);
  ny = nxs - nx; //residual increment after inner loop
  p2 = gy + nyc;
  //printf("m1p.ui8, m2p.ui8 = %x %x, ny %d, bs %e\n",m1p.ui8, m2p.ui8,ny,bs);

  // now the loop to compute the residual
  // with switch on type of array
  j = nyd - nyc +1;
  ndmx2 = ndmx * ndmx;
  switch (dstype) {
    case LUX_INT8:
      while (j) {
        i = nx;
        p1 = ps;
        sumx = 0.0;
        while (i) {
          t = ((float) *m1p.ui8++) - ((float) *m2p.ui8++);
          t = t + bs;
          t = t*t;
          t = MIN(t, ndmx2);
          sumx += (*p1++) * t;
          i--;
        }
        sum += (*p2++) * sumx;
        m1p.ui8 += ny;
        m2p.ui8 += ny;
        j--;
      }
      break;
    case LUX_INT16:
      while (j) {
        i = nx;
        p1 = ps;
        sumx = 0.0;
        while (i) {
          t = ((float) *m1p.i16++) - ((float) *m2p.i16++);
          t = t + bs;
          t = t*t;
          t = MIN(t, ndmx2);
          sumx += (*p1++) * t;
          i--;
        }
        sum += (*p2++) * sumx;
        m1p.i16 += ny;
        m2p.i16 += ny;
        j--;
      }
      break;
    case LUX_INT32:
      while (j) {
        i = nx;
        p1 = ps;
        sumx = 0.0;
        while (i) {
          t = ((float) *m1p.i32++) - ((float) *m2p.i32++);
          t = t + bs;
          t = t*t;
          t = MIN(t, ndmx2);
          sumx += (*p1++) * t;
          i--;
        }
        sum += (*p2++) * sumx;
        m1p.i32 += ny;
        m2p.i32 += ny;
        j--;
      }
      break;
    case LUX_INT64:
      while (j) {
        i = nx;
        p1 = ps;
        sumx = 0.0;
        while (i) {
          t = ((float) *m1p.i64++) - ((float) *m2p.i64++);
          t = t + bs;
          t = t*t;
          t = MIN(t, ndmx2);
          sumx += (*p1++) * t;
          i--;
        }
        sum += (*p2++) * sumx;
        m1p.i64 += ny;
        m2p.i64 += ny;
        j--;
      }
      break;
    case LUX_FLOAT:
      while (j) {
        i = nx;
        p1 = ps;
        sumx = 0.0;
        while (i) {
          t = ((float) *m1p.f++) - ((float) *m2p.f++);
          t = t + bs;
          t = t*t;
          t = MIN(t, ndmx2);
          sumx += (*p1++) * t;
          i--;
        }
        sum += (*p2++) * sumx;
        m1p.f += ny;
        m2p.f += ny;
        j--;
      }
      break;
    case LUX_DOUBLE:
      while (j) {
        i = nx;
        p1 = ps;
        sumx = 0.0;
        while (i) {
          t = ((float) *m1p.d++) - ((float) *m2p.d++);
          t = t + bs;
          t = t*t;
          t = MIN(t, ndmx2);
          sumx += (*p1++) * t;
          i--;
        }
        sum += (*p2++) * sumx;
        m1p.d += ny;
        m2p.d += ny;
        j--;
      }
      break;
  }                             // end of type switch
  //return normalized residual
  return sum/sumg;
}
//-------------------------------------------------------------------------
int32_t lux_stretch(ArgumentCount narg, Symbol ps[])// stretch function
/* the call is MS = STRETCH( M2, DELTA)
   where M2 is the original array to be destretched, MS is the result, and
   DELTA is a displacement grid as generated by GRIDMATCH */
{
  int32_t       iq, n, m, nxg, nyg, result_sym, jy, j1, j2, nm1, nm2, mm1, mm2;
  int32_t       nxgm, nygm, ix, iy, i1, i2, i3, i4, j3, j4, jx;
  float         xd, yd, xinc, yinc, y, x, xs, dy, dy1;
  float         dx, dx1, dx0, dx2, dx3, dx4, fn, fm, xq;
  float         w1, w2, w3, w4, xl, yl, c1, c2, c3, c4, b1, b2, b3, b4;
  Pointer base, out, bb;
  float         *jpbase, *jbase, *xgbase;
  Symboltype type;

  iq = ps[0];                   // <image>
  if (!symbolIsNumericalArray(iq) || array_num_dims(iq) != 2)
    return cerror(NEED_2D_ARR, iq);
  type = array_type(iq);
  base.i32 = (int32_t*) array_data(iq);
  n = array_dims(iq)[0];
  m = array_dims(iq)[1];
  nm1 = n - 1;
  mm1 = m - 1;
  nm2 = n -2;
  mm2 = m -2;
  fn = n;
  fm = m;
                                        // make the output array
  result_sym = array_clone(iq, type);
  out.i32 = (int32_t*) array_data(result_sym);
                                // second arg must be a displacement grid
  iq = ps[1];
  if (!symbolIsNumericalArray(iq))
    return cerror(NEED_ARR, iq);
  if (array_num_dims(iq) != 3
      || array_dims(iq)[0] != 2)
    return cerror(BAD_GRID, 0);
  iq = lux_float(1, &ps[1]);    // ensure that it is FLOAT
  xgbase = (float*) array_data(iq);
  nxg = array_dims(iq)[1];
  nyg = array_dims(iq)[2];
  nxgm = nxg - 1;
  nygm = nyg - 1;
        // linearly interpolate the displacement grid values over array
        // similar to regrid3 in inner part
  xd = (float) n/nxg;
  xinc = 1.0/xd;
  xs = xinc + (xd - 1.0)/(2.0*xd);
  yd = (float) m/nyg;
  yinc = 1.0/yd;
  y = yinc + (yd - 1.0)/(2.0*yd);
  for (iy = 0; iy < m; iy++) {
    x = xs;
    jy = y;
    dy = y - jy;
    dy1 = 1.0 - dy;
    if (jy < 1)
      j1 = j2 = 0;
    else if (jy >= nyg)
      j1 = j2 = nygm;
    else {
      j1 = jy - 1;
      j2 = j1 + 1;
    }
    jbase  = xgbase + j1 * 2 * nxg;
    jpbase = xgbase + j2 * 2 * nxg;
    for (ix = 0; ix < n; ix++) {
      jx = x;
      dx = x - jx;
      dx1 = 1.0 - dx;
      if (jx < 1)
        i1 = i2 = 0;
      else if (jx >= nxg)
        i1 = i2 = nxgm;
      else {
        i1 = jx - 1;
        i2 = i1 + 1;
      }
      w1 = dy1*dx1;
      w2 = dy1*dx;
      w3 = dy*dx1;
      w4 = dy*dx;
      i1 = 2*i1;
      i2 = 2*i2;
      xl = w1 * *(jbase+i1) + w2 * *(jbase+i2) + w3 * *(jpbase+i1)
        + w4 * *(jpbase+i2) + (float) ix;
      i1 += 1;  i2 += 1;
      yl = w1 * *(jbase+i1) + w2 * *(jbase+i2) + w3 * *(jpbase+i1)
        + w4 * *(jpbase+i2) + (float) iy;

      // xl, yl is the place, now do a cubic interpolation for value

      i2 = xl;
      j2 = yl;
      if (i2 >= 1 && i2 < nm2) {// normal interior
        i1 = i2 - 1;
        i3 = i2 + 1;
        i4 = i2 + 2;
        dx0 = xl - i2;
      } else {                  // edge cases
        i2 = MIN(i2, nm1);
        i2 = MAX(i2, 0);
        xq = MIN(xl, fn-1.0);
        xq = MAX(xq, 0);
        dx0 = xq - i2;
        i1 = MIN(i2-1, nm1);
        i1 = MAX(i1, 0);
        i3 = MIN(i2+1, nm1);
        i3 = MAX(i3, 0);
        i4 = MIN(i2+2, nm1);
        i1 = MAX(i4, 0);
      }
      dx1 = 1.0 - dx0;
      dx2 = -dx0*0.5;
      dx3 = dx0*dx2;
      dx4 = 3.0*dx0* dx3;
      c1 = dx2*dx1*dx1;
      c2 = 1.0 - dx4 + 5.0*dx3;
      c3 = dx4 - (dx2 + 4.0*dx3);
      c4 = dx3*dx1;
      if (j2 >= 1 && j2 < mm2) { // normal interior
        j1 = j2 - 1;
        j3 = j2 + 1;
        j4 = j2 + 2;
        dx0 = yl - j2;
      } else {                  // edge cases
        j2 = MIN(j2, mm1);
        j2 = MAX( j2, 0);
        xq = MIN(yl, fm-1.0);
        xq = MAX(xq, 0);
        dx0 = xq - j2;
        j1 = MIN(j2 - 1, mm1);
        j1 = MAX(j1, 0);
        j3 = MIN(j2 + 1, mm1);
        j3 = MAX(j3, 0);
        j4 = MIN(j2 + 2, mm1);
        j1 = MAX(j4, 0);
      }
      dx1 = 1.0 - dx0;
      dx2 = -dx0*0.5;
      dx3 = dx0*dx2;
      dx4 = 3.0*dx0*dx3;
      b1 = dx2*dx1*dx1;
      b2 = 1.0 - dx4 + 5.0*dx3;
      b3 = dx4 - (dx2 + 4.0*dx3);
      b4 = dx3*dx1;
      // low corner offset
      iq = i1 + j1*n;
      i2 = i2 - i1;
      i3 = i3 - i1;
      i4 = i4 - i1;
      j4 = (j4 - j3)*n;
      j3 = (j3 - j2)*n;
      j2 = (j2 - j1)*n;
      switch (type) {
        case LUX_INT8:
          bb.ui8 = base.ui8 + iq;
          xq = b1*(c1*(float) *(bb.ui8) + c2* (float) *(bb.ui8 + i2)
                   + c3 * (float) *(bb.ui8+i3) + c4 * (float) *(bb.ui8+i4));
          bb.ui8 += j2;
          xq += b2*(c1 * *(bb.ui8) + c2 * *(bb.ui8+i2)
                    + c3 * *(bb.ui8+i3) + c4 * *(bb.ui8+i4));
          bb.ui8 += j3;
          xq += b3*(c1 * *(bb.ui8) + c2 * *(bb.ui8+i2)
                    + c3 * *(bb.ui8+i3) + c4 * *(bb.ui8+i4));
          bb.ui8 += j4;
          xq += b4*(c1 * *(bb.ui8) + c2 * *(bb.ui8+i2)
                    + c3 * *(bb.ui8+i3) + c4 * *(bb.ui8+i4));
          *out.ui8++ = xq;
          break;
        case LUX_INT16:
          bb.i16 = base.i16 + iq;
          xq = b1*(c1 * *(bb.i16) + c2 * *(bb.i16+i2)
                   + c3 * *(bb.i16+i3) + c4 * *(bb.i16+i4));
          bb.i16 += j2;
          xq += b2*(c1 * *(bb.i16) + c2 * *(bb.i16+i2)
                    + c3 * *(bb.i16+i3) + c4 * *(bb.i16+i4));
          bb.i16 += j3;
          xq += b3*(c1 * *(bb.i16) + c2 * *(bb.i16+i2)
                    + c3 * *(bb.i16+i3) + c4 * *(bb.i16+i4));
          bb.i16 += j4;
          xq += b4*(c1 * *(bb.i16) + c2 * *(bb.i16+i2)
                    + c3 * *(bb.i16+i3) + c4 * *(bb.i16+i4));
          *out.i16++ = xq;
          break;
        case LUX_INT32:
          bb.i32 = base.i32+iq;
          xq = b1*(c1 * *(bb.i32) + c2 * *(bb.i32+i2)
                   + c3 * *(bb.i32+i3) + c4 * *(bb.i32+i4));
          bb.i32 += j2;
          xq += b2*(c1 * *(bb.i32) + c2 * *(bb.i32+i2)
                    + c3 * *(bb.i32+i3) + c4 * *(bb.i32+i4));
          bb.i32 += j3;
          xq += b3*(c1 * *(bb.i32) + c2 * *(bb.i32+i2)
                    + c3 * *(bb.i32+i3) + c4 * *(bb.i32+i4));
          bb.i32 += j4;
          xq += b4*(c1 * *(bb.i32) + c2 * *(bb.i32+i2)
                    + c3 * *(bb.i32+i3) + c4 * *(bb.i32+i4));
          *out.i32++ = xq;
          break;
        case LUX_INT64:
          bb.i64 = base.i64+iq;
          xq = b1*(c1 * *(bb.i64) + c2 * *(bb.i64+i2)
                   + c3 * *(bb.i64+i3) + c4 * *(bb.i64+i4));
          bb.i64 += j2;
          xq += b2*(c1 * *(bb.i64) + c2 * *(bb.i64+i2)
                    + c3 * *(bb.i64+i3) + c4 * *(bb.i64+i4));
          bb.i64 += j3;
          xq += b3*(c1 * *(bb.i64) + c2 * *(bb.i64+i2)
                    + c3 * *(bb.i64+i3) + c4 * *(bb.i64+i4));
          bb.i64 += j4;
          xq += b4*(c1 * *(bb.i64) + c2 * *(bb.i64+i2)
                    + c3 * *(bb.i64+i3) + c4 * *(bb.i64+i4));
          *out.i64++ = xq;
          break;
        case LUX_FLOAT:
          bb.f = base.f+iq;
          xq = b1*(c1 * *(bb.f) + c2 * *(bb.f+i2)
                   + c3 * *(bb.f+i3) + c4 * *(bb.f+i4));
          bb.f += j2;
          xq += b2*(c1 * *(bb.f) + c2 * *(bb.f+i2)
                    + c3 * *(bb.f+i3) + c4 * *(bb.f+i4));
          bb.f += j3;
          xq += b3*(c1 * *(bb.f) + c2 * *(bb.f+i2)
                    + c3 * *(bb.f+i3) + c4 * *(bb.f+i4));
          bb.f += j4;
          xq += b4*(c1 * *(bb.f) + c2 * *(bb.f+i2)
                    + c3 * *(bb.f+i3) + c4 * *(bb.f+i4));
          *out.f++ = xq;
          break;
        case LUX_DOUBLE:
          bb.d = base.d+iq;
          xq = b1*(c1 * *(bb.d) + c2 * *(bb.d+i2)
                   + c3 * *(bb.d+i3) + c4 * *(bb.d+i4));
          bb.d += j2;
          xq += b2*(c1 * *(bb.d) + c2 * *(bb.d+i2)
                    + c3 * *(bb.d+i3) + c4 * *(bb.d+i4));
          bb.d += j3;
          xq += b3*(c1 * *(bb.d) + c2 * *(bb.d+i2)
                    + c3 * *(bb.d+i3) + c4 * *(bb.d+i4));
          bb.d += j4;
          xq += b4*(c1 * *(bb.d) + c2 * *(bb.d+i2)
                    + c3 * *(bb.d+i3) + c4 * *(bb.d+i4));
          *out.d++ = xq;
          break;
      }
      x += xinc;
    }
    y += yinc;
  }
  return result_sym;
}
//-------------------------------------------------------------------------
void getmin(float *, float *, float *);
int32_t lux_getmin9(ArgumentCount narg, Symbol ps[])/* getmin9 function */           
// local minimum for a 3x3 array
{
  int32_t       iq;
  float         *p, x0, y0;
                                        //first arg must be a 3x3 array
  iq = ps[0];
  if (symbol_class(iq) != LUX_ARRAY)
    return cerror(NEED_3x3_ARR, iq);
  if (array_num_dims(iq) != 2   // must be a 3x3 array
      || array_dims(iq)[0] != 3
      || array_dims(iq)[1] != 3)
    return cerror(NEED_3x3_ARR, iq);
  iq = lux_float(1, &iq);       // ensure FLOAT type
  p = (float *) array_data(iq);
  getmin(p, &x0, &y0);
  if (redef_scalar(ps[1], LUX_FLOAT, &x0) != 1)
    return cerror(ALLOC_ERR, ps[1]);
  if (redef_scalar(ps[2], LUX_FLOAT, &y0) != 1)
    return cerror(ALLOC_ERR, ps[2]);
  return 1;
}
//-------------------------------------------------------------------------
int32_t         getmin2(float *, float *, float *);
int32_t lux_getmin2(ArgumentCount narg, Symbol ps[])/* getmin2 function */           
// local minimum for a 3x3 array
{
  int32_t       iq;
  float         *p, x0, y0;
                                        //first arg must be a 3x3 array
  iq = ps[0];
  if (symbol_class(iq) != LUX_ARRAY)
    return cerror(NEED_3x3_ARR, iq);
  if (array_num_dims(iq) != 2   // must be a 3x3 array
      || array_dims(iq)[0] != 3
      || array_dims(iq)[1] != 3)
    return cerror(NEED_3x3_ARR, iq);
  iq = lux_float(1, &iq);       // ensure FLOAT type
  p = (float *) array_data(iq);
  getmin2(p, &x0, &y0);
  if (redef_scalar(ps[1], LUX_FLOAT, &x0) != 1)
    return cerror(ALLOC_ERR, ps[1]);
  if (redef_scalar(ps[2], LUX_FLOAT, &y0) != 1)
    return cerror(ALLOC_ERR, ps[2]);
  return 1;
}
//-------------------------------------------------------------------------
int32_t getminSYM(float *p, float *x0, float *y0)
// return extremum of least-squares fit of quadratic surface to 3x3 grid
// LS 21mar95
{
  float         f1, f2, f3, f4, f5, f6, f7, f8, f9;
  float         dx, dy, dxx, dyy, dxy, d;

  f1 = *p++;    f2 = *p++;      f3 = *p++;
  f4 = *p++;    f5 = *p++;      f6 = *p++;
  f7 = *p++;    f8 = *p++;      f9 = *p++;
  dx = (f3 + f6 + f9 - f1 - f4 - f7)*2;
  dy = (f7 + f8 + f9 - f1 - f2 - f3)*2;
  dxx = (f3 + f6 + f9 + f1 + f4 + f7 - 2*(f2 + f5 + f8))*4;
  dyy = (f1 + f2 + f3 + f7 + f8 + f9 - 2*(f4 + f5 + f6))*4;
  dxy = (f1 + f9 - f3 - f7)*3;
  d = dxx*dyy - dxy*dxy;
  *x0 = (dxy*dy - dyy*dx)/d;
  *y0 = (dxy*dx - dxx*dy)/d;
  return 1;
}
//-------------------------------------------------------------------------
void getmin(float *p, float *x0, float *y0)
{
  float         f11, f12, f13, f21, f22, f23, f31, f32, f33;
  float         fx, fy, t, fxx, fyy, fxy;
                                // find the min, p points to a 3x3 array
  f11 = *p++;   f21 = *p++;     f31 = *p++;
  f12 = *p++;   f22 = *p++;     f32 = *p++;
  f13 = *p++;   f23 = *p++;     f33 = *p++;

  fx = 0.5*(f32 - f12);
  fy = 0.5*(f23 - f21);
  t = 2.0*f22;
  fxx = f32 + f12 - t;
  fyy = f23 + f21 - t;
  // find in which quadrant the minimum lies
  if (f33 < f11) {
    if (f33 < f31) {
      if (f33 < f13) fxy = f33+f22-f32-f23; else fxy = f23+f12-f22-f13;
    } else {
      if (f31 < f13) fxy = f32+f21-f31-f22; else fxy = f23+f12-f22-f13;
    }
  } else {
    if (f11 < f31) {
      if (f11 < f13) fxy = f22+f11-f21-f12; else fxy = f23+f12-f22-f13;
    } else {
      if (f31 < f13) fxy = f32+f21-f31-f22; else fxy = f23+f12-f22-f13;
    }
  }
  t = fxx*fyy - fxy*fxy;
  if (!t) {                     // bail out if we're going to divide by
                                // zero.  LS 28aug2000
    *x0 = 0.0;
    *y0 = 0.0;
    return;
  }
  t = -1.0/t;
  *x0 = t*(fx*fyy - fy*fxy);
  *y0 = t*(fy*fxx - fx*fxy);
  if (ABS(*x0) >= 0.75 || ABS(*y0) >= 0.75) {
    *x0 = -fx/fxx;
    *y0 = -fy/fyy;
  }
}
//-------------------------------------------------------------------------
int32_t getmin2(float *p, float *x0, float *y0)
{
  float         a, b, c, d, e, det;

  a = (p[0] - 2*p[1] + p[2] + p[3] - 2*p[4] + p[5] + p[6] - 2*p[7] + p[8])/6;
  b = (p[0] + p[1] + p[2] - 2*(p[3] + p[4] + p[5]) + p[6] + p[7] + p[8])/6;
  c = (p[0] - p[2] - p[6] + p[8])/4;
  d = (-p[0] + p[2] - p[3] + p[5] - p[6] + p[8])/6;
  e = (-p[0] - p[1] - p[2] + p[6] + p[7] + p[8])/6;
  det = c*c - 4*a*b;
  det = det? 1.0/det: 0.0;
  *x0 = (2*b*d - c*e)*det;
  *y0 = (2*a*e - c*d)*det;
  return 1;
}
//------------------------------------------------------------------------- / A

/// A template type representing a function that acts as a backend to
/// interpolate_2d().  The function should produce an interpolated value based
/// on the provided arguments.
///
/// The input has \a nix by \a niy elements.  The location of the point at which
/// the interpolated value is desired has coordinates (\a ix + \a tx/(2*\a nox),
/// \a iy + \a ty/(2*\a noy)).
///
/// \tparam T is the type of the data to interpolate.
///
/// \param[in] in points at the input value nearest the desired location.
///
/// \param ix is the 0-based x index of the input value pointed at by \a in.
///
/// \param tx is the x coordinate of the desired location, relative to the
/// center of the element pointed at by \a in.
///
/// \param iy is the 0-based y index of the input value pointed at by \a in.
///
/// \param ty is the y coordinate of the desired location, relative to the
/// center of the input value.
///
/// \param nix is the input element count in the x direction.
///
/// \param niy is the input element count in the y direction.
///
/// \param nox is the output element count in the x direction.
///
/// \param noy is the output element count in the y direction.
template<typename T>
using Interpolation2DBackend
= T (*)(const T* in, int ix, int tx, int iy, int ty, int nix, int niy,
        int nox, int noy);

/// A 2D interpolation backend that does nearest-neighbor interpolation.
///
/// The input has \a nix by \a niy elements.  The location of the point at which
/// the interpolated value is desired has coordinates (\a ix + \a tx/(2*\a nox),
/// \a iy + \a ty/(2*\a noy)).
///
/// \tparam T is the type of the data to interpolate.
///
/// \param[in] in points at the input value nearest the desired location.
///
/// \param ix is the 0-based x index of the input value pointed at by \a in.
///
/// \param tx is the x coordinate of the desired location, relative to the
/// center of the element pointed at by \a in.
///
/// \param iy is the 0-based y index of the input value pointed at by \a in.
///
/// \param ty is the y coordinate of the desired location, relative to the
/// center of the input value.
///
/// \param nix is the input element count in the x direction.
///
/// \param niy is the input element count in the y direction.
///
/// \param nox is the output element count in the x direction.
///
/// \param noy is the output element count in the y direction.
///
/// \returns the value pointed at by \a in, which is the input value
/// interpolated (in nearest-neighbor fashion) at the indicated position.
template<typename T>
inline T
nearest_neighbor_2d_interpolation_backend
(const T* in, int ix, int tx, int iy, int ty, int nix, int niy, int nox,
 int noy) {
  return *in;
}

/// A 2D interpolation backend that does bilinear interpolation.
///
/// The input has \a nix by \a niy elements.  The location of the point at which
/// the interpolated value is desired has coordinates (\a ix + \a tx/(2*\a nox),
/// \a iy + \a ty/(2*\a noy)).
///
/// \tparam T is the type of the data to interpolate.
///
/// \param[in] in points at the input value nearest the desired location.
///
/// \param ix is the 0-based x index of the input value pointed at by \a in.
///
/// \param tx is the x coordinate of the desired location, relative to the
/// center of the element pointed at by \a in.
///
/// \param iy is the 0-based y index of the input value pointed at by \a in.
///
/// \param ty is the y coordinate of the desired location, relative to the
/// center of the input value.
///
/// \param nix is the input element count in the x direction.
///
/// \param niy is the input element count in the y direction.
///
/// \param nox is the output element count in the x direction.
///
/// \param noy is the output element count in the y direction.
///
/// \returns the value pointed at by \a in, which is the input value
/// interpolated (in nearest-neighbor fashion) at the indicated position.
template<typename T>
T
bilinear_2d_interpolation_backend(const T* in, int ix, int tx, int iy, int ty,
                                  int nix, int niy, int nox, int noy) {
  auto nox2 = 2*nox;
  auto noy2 = 2*noy;

  // x = ix + tx/nox2
  // y = iy + ty/noy2

  if (ix == nix - 1) {          // beyond the high x edge
    --in;                       // start at the previous x
    tx += nox2;                 // shift target forward one x element
  }
  if (iy == niy - 1) {          // beyond the high y edge
    in -= nix;                  // start at the previous y
    ty += noy2;                 // shift the target forward one y element
  }
  if (tx < 0 && ix > 0) {
    --in;
    tx += nox2;
  }
  if (ty < 0 && iy > 0) {
    in -= nix;
    ty += noy2;
  }

  auto ux = nox2 - tx;
  auto uy = noy2 - ty;

  // The bilinear result is((in[0]*ux + in[1]*tx)*uy + (in[nix]*ux + in[nix +
  // 1]*tx)*ty)/(nox2*noy2) of which both the numerator and denominator can
  // easily overflow if T is an integer type.  We rewrite in terms of
  // differences with the base value:
  //
  // T v00 = in[0]
  // T d10 = in[1] - in[0];
  // T d01 = in[nix] - in[0];
  // T d11 = in[nix + 1] - in[0];
  // T v = (((v00*ux + (v00 + d10)*tx)*uy
  //        + ((v00 + d01)*ux + (v00 + d11)*tx)*ty))
  //       /(nox2*noy2)
  //   = v00 + (d10*tx*uy + d01*ux*ty + d11*tx*ty)/(nox2*noy2)
  //
  // This is already better, but if T is an unsigned type then d10, d01, d11
  // cannot express negative differences, and the numerator and denominator of
  // the ratio can still both overflow.  We'll keep it simple and calculate the
  // bit that gets added to v00 in a floating-point type.

  auto d10 = static_cast<typename std::common_type<T, float>::type>
    (in[1]) - in[0];
  auto d01 = static_cast<typename std::common_type<T, float>::type>
    (in[nix]) - in[0];
  auto d11 = static_cast<typename std::common_type<T, float>::type>
    (in[nix + 1]) - in[0];

  return in[0] + static_cast<T>((d10*tx*uy + ty*(d01*ux + d11*tx))/(nox2*noy2));
}

/// Does two-dimensional interpolation.
///
/// \tparam T is the type of the data values to interpolate.
///
/// \tparam interpolate points at the backend interpolation function which
/// produces the interpolated value at the desired location.
///
/// \param[in] vin points at the beginning of the input data.  The input data
/// values are expected to be of type \a T, but for convenience this pointer is
/// a pointer to void.
///
/// \param[out] vout points at the beginning of the output data.  The
/// interpolated values are written there.  The output data values are of type
/// \a T, but for convenience this pointer is a pointer to void.  The memory
/// area is assumed to have been allocated already and to be large enough to
/// hold the results.
///
/// \param[in] indims points at the 2 dimensions of the input data.  The
/// dimensions are expected to be positive.
///
/// \param[in] outdims points at the 2 dimensions of the output data.  The
/// dimensions are expected to be positive.
template<typename T, Interpolation2DBackend<T> interpolate>
void
interpolate_2d(const void* vin, void* vout, const int* indims,
               const int* outdims) {
  auto in = static_cast<const T*>(vin);
  auto out = static_cast<T*>(vout);
  auto& nix = indims[0];
  auto& niy = indims[1];
  auto& nox = outdims[0];
  auto& noy = outdims[1];

  auto qrx = std::div(nix, 2*nox);
  auto qry = std::div(niy, 2*noy);

  auto qx2 = 2*qrx.quot;
  auto qy2 = 2*qry.quot;

  auto rx2 = 2*qrx.rem;
  auto ry2 = 2*qry.rem;

  auto nox2 = 2*nox;
  auto noy2 = 2*noy;

  auto ix = qrx.quot;
  auto ix0 = ix;
  auto iy = qry.quot;

  int tx = qrx.rem - nox;
  int ty = qry.rem - noy;

  auto ip = &in[ix + iy*nix];
  auto op = &out[0];

  auto opendx = op + nox;
  auto opendy = op + nox*noy;

  auto cy = qy2*nix;
  while (op < opendy) {
    while (op < opendx) {
      *op = interpolate(ip, ix, tx, iy, ty, nix, niy, nox, noy);
      ix += qx2;
      ip += qx2;
      tx += rx2;
      if (tx >= nox) {
        ++ip;
        ++ix;
        tx -= nox2;
      }
      ++op;
    }
    ip += cy;
    ix = ix0;
    ty += ry2;
    if (ty >= noy) {
      ++iy;
      ty -= noy2;
    } else {
      ip -= nix;
    }
    opendx += nox;
  }
}

/// Does two-dimensional image expansion by an arbitrary positive factor for
/// each dimension.
///
/// \param source is the number of the LUX symbol that holds the image to be
/// expanded.
///
/// \param factor_x is the factor by which to expand the image in the x
/// direction (the first dimension).  The expanded image's x dimension is the
/// rounded-down product of the source image's x dimension and \a factor_x.  The
/// expansion factor passed to the interpolation backend is the ratio of the
/// output image's to the input image's x dimension.
///
/// \param factor_y is the factor by which to expand the image in the y
/// direction (the second dimension).  It gets treated similarly to \a factor_x.
///
/// \param method indicates which interpolation method to use.  Supported values
/// are 0 for nearest-neighbor interpolation, or 1 for bilinear interpolation.
///
/// \returns the LUX symbol number of the expanded image.
int32_t
expandImage(int32_t source, float factor_x, float factor_y, int32_t method)
{
  assert(factor_x > 0);
  assert(factor_y > 0);
  if (!symbolIsNumericalArray(source)
      || array_num_dims(source) != 2)
    return cerror(NEED_2D_ARR, source);

  int32_t* dimsin = array_dims(source);
  int32_t dimsout[2];
  dimsout[0] = dimsin[0]*factor_x;
  if (dimsout[0] < 1)
    dimsout[0] = 1;
  dimsout[1] = dimsin[1]*factor_y;
  if (dimsout[1] < 1)
    dimsout[1] = 1;

  int32_t out = array_scratch(array_type(source), 2, dimsout);

  switch (method) {
  default:                       // nearest neighbor
    switch (array_type(source)) {
    case LUX_INT8:
      interpolate_2d<uint8_t,
                     nearest_neighbor_2d_interpolation_backend>
        (array_data(source), array_data(out), array_dims(source),
         array_dims(out));
      break;
    case LUX_INT16:
      interpolate_2d<int16_t,
                     nearest_neighbor_2d_interpolation_backend>
        (array_data(source), array_data(out), array_dims(source),
         array_dims(out));
      break;
    case LUX_INT32:
      interpolate_2d<int32_t,
                     nearest_neighbor_2d_interpolation_backend>
        (array_data(source), array_data(out), array_dims(source),
         array_dims(out));
      break;
    case LUX_INT64:
      interpolate_2d<int64_t,
                     nearest_neighbor_2d_interpolation_backend>
        (array_data(source), array_data(out), array_dims(source),
         array_dims(out));
      break;
    case LUX_FLOAT:
      interpolate_2d<float,
                     nearest_neighbor_2d_interpolation_backend>
        (array_data(source), array_data(out), array_dims(source),
         array_dims(out));
      break;
    case LUX_DOUBLE:
      interpolate_2d<double,
                     nearest_neighbor_2d_interpolation_backend>
        (array_data(source), array_data(out), array_dims(source),
         array_dims(out));
      break;
    default:
      return cerror(ILL_TYPE, source);
    }
    break;
  case 1:                      // bilinear
    switch (array_type(source)) {
    case LUX_INT8:
      interpolate_2d<uint8_t,
                     bilinear_2d_interpolation_backend>
        (array_data(source), array_data(out), array_dims(source),
         array_dims(out));
      break;
    case LUX_INT16:
      interpolate_2d<int16_t,
                     bilinear_2d_interpolation_backend>
        (array_data(source), array_data(out), array_dims(source),
         array_dims(out));
      break;
    case LUX_INT32:
      interpolate_2d<int32_t,
                     bilinear_2d_interpolation_backend>
        (array_data(source), array_data(out), array_dims(source),
         array_dims(out));
      break;
    case LUX_INT64:
      interpolate_2d<int64_t,
                     bilinear_2d_interpolation_backend>
        (array_data(source), array_data(out), array_dims(source),
         array_dims(out));
      break;
    case LUX_FLOAT:
      interpolate_2d<float,
                     bilinear_2d_interpolation_backend>
        (array_data(source), array_data(out), array_dims(source),
         array_dims(out));
      break;
    case LUX_DOUBLE:
      interpolate_2d<double,
                     bilinear_2d_interpolation_backend>
        (array_data(source), array_data(out), array_dims(source),
         array_dims(out));
      break;
    default:
      return cerror(ILL_TYPE, source);
    }
    break;
  }
  return out;
}
//-------------------------------------------------------------------------
int32_t lux_expand(ArgumentCount narg, Symbol ps[])
{
  float         sx, sy;
  int32_t       smt;

  sx = float_arg(ps[1]);
  if (sx <= 0)
    return luxerror("Magnification factor %f should be positive", ps[1]);
  if (narg > 2)
    sy = float_arg(ps[2]);
  else
    sy = sx;
  if (sy <= 0)
    return luxerror("Magnification factor %f should be positive", ps[2]);
  if (sx == 1 && sy == 1)       // nothing to do
    return ps[0];
  if (narg > 3)
    smt = int_arg(ps[3]);
  else switch (internalMode & 3) {
  case 1: case 3:               // /SMOOTH
    smt = 1;
    break;
  case 2:
    smt = 0;                    // /NEAREST
    break;
  case 0:                       // none: use !TVSMT
    smt = tvsmt;
    break;
  }
  return expandImage(ps[0], sx, sy, smt);
}
//-------------------------------------------------------------------------
// some variables common to several routines
static int32_t regridtypeflag, stretchmark_flag,
  nm1, mm1, nm2, mm2, n;
static Symboltype regrid_type;
static float    fnm1, fnm5, fmm1, fmm5, xl, yl;
static Pointer  base, out;
int32_t regrid_common(int32_t, int32_t []);

void bicubic_f(void)    // internal routine for single pixel
 {
 /* used by all (most?) routines that do bi-cubic interpolations, runs
 a little slower than the originals, perhaps because of the overhead
 in the call or other adjustments made */
 /*    MODIFIED 1-26-84 TO USE LOWER NOISE CUBIC INTERPOLATION FOUND IN
      S.K. PARK & R.A. SCHOWENGERDT, COMP. VIS. & IM. PROC., VOL. 23,
      P. 258, 1983:  USES THEIR FORMULA WITH ALPHA = -0.5
*/
 int32_t        i1, i2, i3, i4, j1, j2, j3, j4, iq;
 float  c1, c2, c3, c4, b1, b2, b3, b4, dx0, dx1, dx2, dx3, dx4, xq;
 /* the location is in xl, yl; base is the pointer to array; out is
 pointer to output; both are unions */
 Pointer bb;
 i2 = (int32_t) xl;             j2 = (int32_t) yl;
 if ( i2 >= 1 && i2 < nm2 ) {           // normal interior
         dx0 = xl - i2; i1 = i2 - 1; i2 = 1; i3 = 2; i4 = 3;     }
         else {                                 // edge cases
         // check if stretchmarks required
         if (stretchmark_flag == 0) {
          if ( xl < -0.5 || xl > fnm5) {
           switch (regrid_type) {
           case 0: *out.ui8++ = 0; break;
           case 1: *out.i16++ = 0; break;
           case 2: *out.i32++ = 0; break;
           case 3: *out.f++ = 0; break;
           case 4: *out.d++ = 0; break;
           }
          return;
          }
         }
         i2 = MIN(i2, nm1);     i2 = MAX( i2, 0);
         xq = MIN(xl, fnm1);    xq = MAX(xq, 0);
         dx0 = xq - i2;
 //     printf("dx0 = %f\n",dx0);
         i1 = MIN(i2-1, nm1);   i1 = MAX( i1, 0);
         i3 = MIN(i2+1, nm1);   // i3 = MAX( i3, 0);
         i4 = MIN(i2+2, nm1);   // i4 = MAX( i4, 0);
         i2 = i2 - i1;  i3 = i3 - i1;   i4 = i4 - i1;
         }
 dx1 = 1.0 - dx0;  dx2 = -dx0 * 0.5;  dx3 = dx0 * dx2;
 dx4 = 3. * dx0 * dx3;
 c1 = dx2*dx1*dx1; c2 = 1.-dx4+5.*dx3; c3 = dx4-(dx2+4.*dx3); c4 = dx3*dx1;
 // printf("i2,j2,xl,yl= %d %d %f %f\n",i2,j2,xl,yl);
 if ( j2 >= 1 && j2 < mm2 ) {           // normal interior
         j1 = j2 - 1; dx0 = yl - j2; j3 = n; j2 = n; j4 = n;    }
         else {                                 // edge cases
         // check if stretchmarks required
         if (stretchmark_flag == 0) {
          if ( yl < -0.5 || yl > fmm5) {
           switch (regrid_type) {
           case 0: *out.ui8++ = 0; break;
           case 1: *out.i16++ = 0; break;
           case 2: *out.i32++ = 0; break;
           case 3: *out.f++ = 0; break;
           case 4: *out.d++ = 0; break;
           }
          return;
          }
         }       j2 = MIN(j2, mm1);     j2 = MAX( j2, 0);
         xq = MIN(yl, fmm1);    xq = MAX(xq, 0);
         dx0 = xq - j2;
 //     printf("dy0 = %f, yl = %f, j2 = %d\n",dx0,yl,j2);
         j1 = MIN(j2-1, mm1);   j1 = MAX( j1, 0);
         j3 = MIN(j2+1, mm1);   // j3 = MAX( j3, 0);
         j4 = MIN(j2+2, mm1);   // j4 = MAX( j4, 0);
         j4 = (j4 - j3) * n;  j3 = (j3 - j2) * n;  j2 = (j2 - j1) *n;
         }
 dx1 = 1.0 - dx0;  dx2 = -dx0 * 0.5;  dx3 = dx0 * dx2;
 dx4 = 3. * dx0 * dx3;
 b1 = dx2*dx1*dx1; b2 = 1.-dx4+5.*dx3; b3 = dx4-(dx2+4.*dx3); b4 = dx3*dx1;
 // low corner offset
 // printf("index j1, j2, j3, j4 = %d %d %d %d\n", j1, j2, j3, j4);
 iq = i1 + j1 * n;
 // printf("offset j1, j2, j3, j4 = %d %d %d %d\n", j1, j2, j3, j4);
 switch (regrid_type) {
 case 0:
 bb.ui8 = base.ui8+iq;
 xq = b1*(c1 * *(bb.ui8) + c2 * *(bb.ui8+i2)+ c3 * *(bb.ui8+i3) + c4 * *(bb.ui8+i4));
 bb.ui8 += j2;
 xq += b2*(c1 * *(bb.ui8) + c2 * *(bb.ui8+i2)+ c3 * *(bb.ui8+i3) + c4 * *(bb.ui8+i4));
 bb.ui8 += j3;
 xq += b3*(c1 * *(bb.ui8) + c2 * *(bb.ui8+i2)+ c3 * *(bb.ui8+i3) + c4 * *(bb.ui8+i4));
 bb.ui8 += j4;
 xq += b4*(c1 * *(bb.ui8) + c2 * *(bb.ui8+i2)+ c3 * *(bb.ui8+i3) + c4 * *(bb.ui8+i4));
 // uint8_t arrays need to be range restricted, too many simple minds out there
 xq = MAX( 0, MIN( 255, xq));
 // also we need to round rather than truncate, taking that extra care
 *out.ui8++ = rint(xq); break;
 case 1:
 bb.i16 = base.i16+iq;
 xq = b1*(c1 * *(bb.i16) + c2 * *(bb.i16+i2)+ c3 * *(bb.i16+i3) + c4 * *(bb.i16+i4));
 bb.i16 += j2;
 xq += b2*(c1 * *(bb.i16) + c2 * *(bb.i16+i2)+ c3 * *(bb.i16+i3) + c4 * *(bb.i16+i4));
 bb.i16 += j3;
 xq += b3*(c1 * *(bb.i16) + c2 * *(bb.i16+i2)+ c3 * *(bb.i16+i3) + c4 * *(bb.i16+i4));
 bb.i16 += j4;
 xq += b4*(c1 * *(bb.i16) + c2 * *(bb.i16+i2)+ c3 * *(bb.i16+i3) + c4 * *(bb.i16+i4));
 // also we need to round rather than truncate, taking that extra care
 *out.i16++ = rint(xq); break;
 case 2:
 bb.i32 = base.i32+iq;
 xq = b1*(c1 * *(bb.i32) + c2 * *(bb.i32+i2)+ c3 * *(bb.i32+i3) + c4 * *(bb.i32+i4));
 bb.i32 += j2;
 xq += b2*(c1 * *(bb.i32) + c2 * *(bb.i32+i2)+ c3 * *(bb.i32+i3) + c4 * *(bb.i32+i4));
 bb.i32 += j3;
 xq += b3*(c1 * *(bb.i32) + c2 * *(bb.i32+i2)+ c3 * *(bb.i32+i3) + c4 * *(bb.i32+i4));
 bb.i32 += j4;
 xq += b4*(c1 * *(bb.i32) + c2 * *(bb.i32+i2)+ c3 * *(bb.i32+i3) + c4 * *(bb.i32+i4));
 // also we need to round rather than truncate, taking that extra care
 *out.i32++ = rint(xq); break;
 case 3:
 bb.f = base.f+iq;
 xq = b1*(c1 * *(bb.f) + c2 * *(bb.f+i2)+ c3 * *(bb.f+i3) + c4 * *(bb.f+i4));
 bb.f += j2;
 xq += b2*(c1 * *(bb.f) + c2 * *(bb.f+i2)+ c3 * *(bb.f+i3) + c4 * *(bb.f+i4));
 bb.f += j3;
 xq += b3*(c1 * *(bb.f) + c2 * *(bb.f+i2)+ c3 * *(bb.f+i3) + c4 * *(bb.f+i4));
 bb.f += j4;
 xq += b4*(c1 * *(bb.f) + c2 * *(bb.f+i2)+ c3 * *(bb.f+i3) + c4 * *(bb.f+i4));
 *out.f++ = xq; break;
 case 4:
 bb.d = base.d+iq;
 xq = b1*(c1 * *(bb.d) + c2 * *(bb.d+i2)+ c3 * *(bb.d+i3) + c4 * *(bb.d+i4));
 bb.d += j2;
 xq += b2*(c1 * *(bb.d) + c2 * *(bb.d+i2)+ c3 * *(bb.d+i3) + c4 * *(bb.d+i4));
 bb.d += j3;
 xq += b3*(c1 * *(bb.d) + c2 * *(bb.d+i2)+ c3 * *(bb.d+i3) + c4 * *(bb.d+i4));
 bb.d += j4;
 xq += b4*(c1 * *(bb.d) + c2 * *(bb.d+i2)+ c3 * *(bb.d+i3) + c4 * *(bb.d+i4));
 *out.d++ = xq; break;
 }
 return;
 }
//-------------------------------------------------------------------------
void bicubic_fc()       // internal routine for single pixel
{
  /* used by all (most?) routines that do bi-cubic interpolations, runs
     a little slower than the originals, perhaps because of the overhead
     in the call or other adjustments made */
  int32_t       i1, i2, i3, i4, j1, j2, j3, j4, iq;
  float         c1, c2, c3, c4, b1, b2, b3, b4, dx0, dx1, dx2, dx3, dx4, xq;
  Pointer bb;

  /* the location is in xl, yl; base is the pointer to array; out is
     pointer to output; both are unions */
  i2 = (int32_t) xl;
  j2 = (int32_t) yl;
  if (i2 >= 1 && i2 < nm2) {            // normal interior
    dx0 = xl - i2;              // Bx
    i1 = i2 - 1;
    i2 = 1;
    i3 = 2;
    i4 = 3;
  } else {                              // edge cases
    // check if stretchmarks required
    if (stretchmark_flag == 0) {
      if (xl < -0.5 || xl > fnm5) {
        switch (regrid_type) {
          case LUX_INT8:
            *out.ui8++ = 0;
            break;
          case LUX_INT16:
            *out.i16++ = 0;
            break;
          case LUX_INT32:
            *out.i32++ = 0;
            break;
          case LUX_INT64:
            *out.i64++ = 0;
            break;
          case LUX_FLOAT:
            *out.f++ = 0;
            break;
          case LUX_DOUBLE:
            *out.d++ = 0;
            break;
        }
        return;
      }
    }
    i2 = MIN(i2, nm1);
    i2 = MAX(i2, 0);
    xq = MIN(xl, fnm1);
    xq = MAX(xq, 0);
    dx0 = xq - i2;
    //  printf("dx0 = %f\n",dx0);
    i1 = MIN(i2-1, nm1);
    i1 = MAX(i1, 0);
    i3 = MIN(i2+1, nm1);        // i3 = MAX( i3, 0);
    i4 = MIN(i2+2, nm1);        // i4 = MAX( i4, 0);
    i2 = i2 - i1;
    i3 = i3 - i1;
    i4 = i4 - i1;
  }

  dx1 = 1.0 - dx0;              // Ax
  dx4 = -dx0*dx1;               // -Ax Bx
  c1 = dx4 * dx1;               // -Ax^2 Bx
  c4 = dx0 * dx4;               // -Ax Bx^2
  dx2 = dx0 * dx0;              // Bx^2
  dx3 = dx0 * dx2;              // Bx^3
  c2 = 1.-2.*dx2+dx3;           // 1 - 2 Bx^2 + Bx^3
  c3 = dx0*(1.0+dx0-dx2);       // Bx (1 + Bx - Bx^2)
  // printf("i2,j2,xl,yl= %d %d %f %f\n",i2,j2,xl,yl);
  if (j2 >= 1 && j2 < mm2) {            // normal interior
    j1 = j2 - 1;
    dx0 = yl - j2;              // By
    j3 = n;                     // NOTE: different than for i2, i3, i4
    j2 = n;
    j4 = n;
  } else {                              // edge cases
    // check if stretchmarks required
    if (stretchmark_flag == 0) {
      if (yl < -0.5 || yl > fmm5) {
        switch (regrid_type) {
          case LUX_INT8:
            *out.ui8++ = 0;
            break;
          case LUX_INT16:
            *out.i16++ = 0;
            break;
          case LUX_INT32:
            *out.i32++ = 0;
            break;
          case LUX_INT64:
            *out.i64++ = 0;
            break;
          case LUX_FLOAT:
            *out.f++ = 0;
            break;
          case LUX_DOUBLE:
            *out.d++ = 0;
            break;
        }
        return;
      }
    }
    j2 = MIN(j2, mm1);
    j2 = MAX(j2, 0);
    xq = MIN(yl, fmm1);
    xq = MAX(xq, 0);
    dx0 = xq - j2;
    //  printf("dy0 = %f, yl = %f, j2 = %d\n",dx0,yl,j2);
    j1 = MIN(j2-1, mm1);
    j1 = MAX(j1, 0);
    j3 = MIN(j2+1, mm1);        // j3 = MAX( j3, 0);
    j4 = MIN(j2+2, mm1);        // j4 = MAX( j4, 0);
    j4 = (j4 - j3) * n;                 // NOTE: j1 not multiplied by n
    j3 = (j3 - j2) * n;
    j2 = (j2 - j1) *n;
  }

  dx1 = 1.0 - dx0;              // Ay
  dx4 = -dx0*dx1;               // -Ay By
  b1 = dx4 * dx1;               // -Ay^2 By
  b4 = dx0 * dx4;               // -Ay By^2
  dx2 = dx0 * dx0;              // By^2
  dx3 = dx0 * dx2;              // By^3
  b2 = 1.-2.*dx2+dx3;           // 1 - 2 By^2 + By^3
  b3 = dx0*(1.0+dx0-dx2);       // By (1 + By - By^2)
  // low corner offset
  // printf("index j1, j2, j3, j4 = %d %d %d %d\n", j1, j2, j3, j4);
  iq = i1 + j1 * n;
  // printf("offset j1, j2, j3, j4 = %d %d %d %d\n", j1, j2, j3, j4);
  switch (regrid_type) {
    case LUX_INT8:
      bb.ui8 = base.ui8+iq;
      xq = b1*(c1*bb.ui8[0] + c2*bb.ui8[i2]+ c3*bb.ui8[i3] + c4*bb.ui8[i4]);
      bb.ui8 += j2;
      xq += b2*(c1*bb.ui8[0] + c2*bb.ui8[i2]+ c3*bb.ui8[i3] + c4*bb.ui8[i4]);
      bb.ui8 += j3;
      xq += b3*(c1*bb.ui8[0] + c2*bb.ui8[i2]+ c3*bb.ui8[i3] + c4*bb.ui8[i4]);
      bb.ui8 += j4;
      xq += b4*(c1*bb.ui8[0] + c2*bb.ui8[i2]+ c3*bb.ui8[i3] + c4*bb.ui8[i4]);
      // uint8_t arrays need to be range restricted, too many simple minds out there
      xq = MAX( 0, MIN( 255, xq));
      // also we need to round rather than truncate, taking that extra care
      *out.ui8++ = rint(xq);
      break;
    case LUX_INT16:
      bb.i16 = base.i16+iq;
      xq = b1*(c1*bb.i16[0] + c2*bb.i16[i2]+ c3*bb.i16[i3] + c4*bb.i16[i4]);
      bb.i16 += j2;
      xq += b2*(c1*bb.i16[0] + c2*bb.i16[i2]+ c3*bb.i16[i3] + c4*bb.i16[i4]);
      bb.i16 += j3;
      xq += b3*(c1*bb.i16[0] + c2*bb.i16[i2]+ c3*bb.i16[i3] + c4*bb.i16[i4]);
      bb.i16 += j4;
      xq += b4*(c1*bb.i16[0] + c2*bb.i16[i2]+ c3*bb.i16[i3] + c4*bb.i16[i4]);
      // also we need to round rather than truncate, taking that extra care
      *out.i16++ = rint(xq);
      break;
    case LUX_INT32:
      bb.i32 = base.i32+iq;
      xq = b1*(c1*bb.i32[0] + c2*bb.i32[i2]+ c3*bb.i32[i3] + c4*bb.i32[i4]);
      bb.i32 += j2;
      xq += b2*(c1*bb.i32[0] + c2*bb.i32[i2]+ c3*bb.i32[i3] + c4*bb.i32[i4]);
      bb.i32 += j3;
      xq += b3*(c1*bb.i32[0] + c2*bb.i32[i2]+ c3*bb.i32[i3] + c4*bb.i32[i4]);
      bb.i32 += j4;
      xq += b4*(c1*bb.i32[0] + c2*bb.i32[i2]+ c3*bb.i32[i3] + c4*bb.i32[i4]);
      // also we need to round rather than truncate, taking that extra care
      *out.i32++ = rint(xq);
      break;
    case LUX_INT64:
      bb.i64 = base.i64+iq;
      xq = b1*(c1*bb.i64[0] + c2*bb.i64[i2]+ c3*bb.i64[i3] + c4*bb.i64[i4]);
      bb.i64 += j2;
      xq += b2*(c1*bb.i64[0] + c2*bb.i64[i2]+ c3*bb.i64[i3] + c4*bb.i64[i4]);
      bb.i64 += j3;
      xq += b3*(c1*bb.i64[0] + c2*bb.i64[i2]+ c3*bb.i64[i3] + c4*bb.i64[i4]);
      bb.i64 += j4;
      xq += b4*(c1*bb.i64[0] + c2*bb.i64[i2]+ c3*bb.i64[i3] + c4*bb.i64[i4]);
      // also we need to round rather than truncate, taking that extra care
      *out.i64++ = rint(xq);
      break;
    case LUX_FLOAT:
      bb.f = base.f+iq;
      xq = b1*(c1*bb.f[0] + c2*bb.f[i2]+ c3*bb.f[i3] + c4*bb.f[i4]);
      bb.f += j2;
      xq += b2*(c1*bb.f[0] + c2*bb.f[i2]+ c3*bb.f[i3] + c4*bb.f[i4]);
      bb.f += j3;
      xq += b3*(c1*bb.f[0] + c2*bb.f[i2]+ c3*bb.f[i3] + c4*bb.f[i4]);
      bb.f += j4;
      xq += b4*(c1*bb.f[0] + c2*bb.f[i2]+ c3*bb.f[i3] + c4*bb.f[i4]);
      *out.f++ = xq;
      break;
    case LUX_DOUBLE:
      bb.d = base.d+iq;
      xq = b1*(c1*bb.d[0] + c2*bb.d[i2]+ c3*bb.d[i3] + c4*bb.d[i4]);
      bb.d += j2;
      xq += b2*(c1*bb.d[0] + c2*bb.d[i2]+ c3*bb.d[i3] + c4*bb.d[i4]);
      bb.d += j3;
      xq += b3*(c1*bb.d[0] + c2*bb.d[i2]+ c3*bb.d[i3] + c4*bb.d[i4]);
      bb.d += j4;
      xq += b4*(c1*bb.d[0] + c2*bb.d[i2]+ c3*bb.d[i3] + c4*bb.d[i4]);
      *out.d++ = xq;
      break;
  }
  return;
}
 //-------------------------------------------------------------------------
int32_t lux_regrid(ArgumentCount narg, Symbol ps[]) // regrid function
 // call is Y = REGRID( X, XG, YG, DX, DY)
 {
 regridtypeflag = 0;    // for a nearest neighbor regrid
 return regrid_common(narg,ps);
 }
 //-------------------------------------------------------------------------
int32_t lux_regrid3(ArgumentCount narg, Symbol ps[]) // regrid3 function
 // call is Y = REGRID3( X, XG, YG, DX, DY)
 /* similar to regrid but uses bicubic interpolation rather than nearest
  neighbor for pixel value, still uses bilinear for grid, also uses stretch
  marks for boundaries */
 {
 regridtypeflag = 1;    // for a bicubic with stretchmarks regrid
 stretchmark_flag = 1;
 return regrid_common(narg,ps);
 }
 //-------------------------------------------------------------------------
int32_t lux_regrid3ns(ArgumentCount narg, Symbol ps[]) // regrid3ns function
 // call is Y = REGRID3( X, XG, YG, DX, DY)
 /* similar to regrid but uses bicubic interpolation rather than nearest
  neighbor for pixel value, still uses bilinear for grid, without stretch
  marks for boundaries (hence the ns)*/
 {
 regridtypeflag = 1;    // for a bicubic with stretchmarks regrid
 stretchmark_flag = 0;
 return regrid_common(narg,ps);
 }
 //-------------------------------------------------------------------------
int32_t regrid_common(ArgumentCount narg, Symbol ps[])// with branches for type
{
  int32_t       iq, nx, ny, m, ng, mg, ns, ms, ngrun, dim[2];
  int32_t       iprun, jrun, jprun, ig, ic, jc, result_sym;
  int32_t       i, j, ind;
  float         fn, fm, yrun, ax, bx, cx, dx, ay, by, cy, dy, xq, beta, xinc, yinc,
    xl0, yl0;
  Pointer xgbase, ygbase, jpbase, jbase, ipbase;
                                 // first argument must be a 2-D array
  iq = ps[0];                    // <data>
  if (!symbolIsNumericalArray(iq))
    return cerror(NEED_NUM_ARR, iq);
  regrid_type = array_type(iq);         // data type
  base.i32 = (int32_t*) array_data(iq);   // pointer to start of data
                                         // we want a 2-D array only
  if (array_num_dims(iq) != 2)
    return cerror(NEED_2D_ARR, iq);
  n = array_dims(iq)[0];        // data dimension in x coordinate
  m = array_dims(iq)[1];        // data dimension in y coordinate
  if (n < 2 || m < 2)
    return cerror(NEED_NTRV_2D_ARR, iq);
  fn = (float) n;               // float versions of data dimensions
  fm = (float) m;
                                 // check xg and yg, must be the same size
  if (!symbolIsNumericalArray(ps[1]))
    return cerror(NEED_NUM_ARR, ps[1]);
                                         // we want a 2-D array only
  if (array_num_dims(ps[1]) != 2)
    return cerror(NEED_2D_ARR, ps[1]);
  ng = array_dims(ps[1])[0];    // x grid dimension in x direction
  mg = array_dims(ps[1])[1];    // x grid dimension in y direction
  if (ng < 2 || mg < 2)
    return cerror(NEED_NTRV_2D_ARR, ps[1]);
  iq = lux_float(1, &ps[1]);
  xgbase.i32 = (int32_t*) array_data(iq);         // x grid data

  if (!symbolIsNumericalArray(ps[2]))
    return cerror(NEED_NUM_ARR, ps[2]);
                                         // we want a 2-D array only
  if (array_num_dims(ps[2]) != 2)
    return cerror(NEED_2D_ARR, ps[2]);
  if (ng != array_dims(ps[2])[0]
      || mg != array_dims(ps[2])[1])
    return cerror(INCMP_ARG, ps[2]);
  iq = lux_float(1, &ps[2]);
  ygbase.i32 = (int32_t*) array_data(iq);         // y grid data
                         // get dx and dy which are put in ns and ms
  ns = int_arg(ps[3]);          // x scale
  ms = int_arg(ps[4]);          // y scale
  ngrun = ng;
                                         // generate the output array
  ng--;
  mg--;
  nx = ng * ns;
  ny = mg * ms;
  dim[0] = nx;                  // (grid x - 1)*nx
  dim[1] = ny;                  // (grid y - 1)*ny
  if (nx > maxregridsize || ny > maxregridsize) {
    printf("result array in REGRID would be %d by %d\n",nx,ny);
    printf("which exceeds current !maxregridsize (%d)\n",maxregridsize);
    return LUX_ERROR;
  }
  result_sym = array_scratch(regrid_type, 2, dim);
  jpbase.i32 = (int32_t*) array_data(result_sym); // output
  yrun = 1.0/ (float) ms;
  i = lux_type_size[regrid_type];
  // various increments, in bytes!
  iprun = ns * i;               // one grid cell row (in the output)
  jrun = ng * iprun;            // one output row
  jprun = ms * jrun;            // one grid cell height (in the output)
  ind = 0;
  // before any looping, branch on the type of regrid we are doing
  switch (regridtypeflag) {
    case 0:     // nearest neighbor regrid
                         // start 4 level loop
      while (mg--) {            // outer mg loop (over all grid y)
        ipbase.ui8 = jpbase.ui8;
        jpbase.ui8 = ipbase.ui8 + jprun;
        ig = ng;                // # grid cells horizontally
        j = ind;
                                // ig loop
        while (ig--) {          // over all grid x
                                // get the 8 grid values for this cell
          i = j;
          ax = xgbase.f[i];     // gx[ix,iy]  ix=ind % ng  iy=ind/ng
          ay = ygbase.f[i];
          i++;
          bx = xgbase.f[i] - ax; // gx[ix+1,iy] - gx[ix,iy]
          by = ygbase.f[i] - ay;
          i += ngrun;
          dx = xgbase.f[i] - ax; // gx[ix+1,iy+1] - gx[ix,iy]
          dy = ygbase.f[i] - ay;
          i--;
          cx = xgbase.f[i] - ax; // gx[ix,iy+1] - gx[ix,iy]
          cy = ygbase.f[i] - ay;
          dx = dx - bx - cx;    // gx[ix+1,iy+1] - gx[ix+1,iy]
                                // - gx[ix,iy+1] + gx[ix,iy]
          dy = dy - by - cy;
          j++;                  // to next grid cell
          xq = 1.0/(float) ns;  // 1/x scale
          bx *= xq;             // (gx[ix+1,iy]-gx[ix,iy])/scale_x
          by *= xq;
          dx *= xq*yrun;
          dy *= xq*yrun;
          cx *= yrun;
          cy *= yrun;
                                                 // setup for jc loop
          jbase.ui8 = ipbase.ui8;
          ipbase.ui8 = ipbase.ui8 + iprun;
          beta = 0.0;
          jc = ms;
          while (jc--) {
                                                 // setup for ic loop
            out.ui8 = jbase.ui8;
            jbase.ui8 = jbase.ui8 + jrun;
            xl = ax + beta*cx + 0.5;
            yl = ay + beta*cy + 0.5;
            xinc = (bx + beta*dx);
            yinc = (by + beta*dy);
            ic = ns;
                                         // type switch for inner loop
            switch (regrid_type) {
              case LUX_INT8:
                while (ic--) {
                  if (xl < 0 || xl >= fn || yl < 0 || yl >= fm)
                    *out.ui8++ = 0;
                  else
                    *out.ui8++ = *(base.ui8 + (int32_t) xl + n * (int32_t) yl);
                  xl += xinc;
                  yl += yinc;
                }
                break;
              case LUX_INT16:
                while (ic--) {
                  if (xl < 0 || xl >= fn || yl < 0 || yl >= fm)
                    *out.i16++ = 0;
                  else
                    *out.i16++ = *(base.i16 + (int32_t) xl + n * (int32_t) yl);
                  xl += xinc;
                  yl += yinc;
                }
                break;
              case LUX_INT32:
                while (ic--) {
                  if (xl < 0 || xl >= fn || yl < 0 || yl >= fm)
                    *out.i32++ = 0;
                  else
                    *out.i32++ = *(base.i32 + (int32_t) xl + n * (int32_t) yl);
                  xl += xinc;
                  yl += yinc;
                }
                break;
              case LUX_INT64:
                while (ic--) {
                  if (xl < 0 || xl >= fn || yl < 0 || yl >= fm)
                    *out.i64++ = 0;
                  else
                    *out.i64++ = *(base.i64 + (int32_t) xl + n * (int32_t) yl);
                  xl += xinc;
                  yl += yinc;
                }
                break;
              case LUX_FLOAT:
                while (ic--) {
                  if (xl < 0 || xl >= fn || yl < 0 || yl >= fm)
                    *out.f++ = 0;
                  else
                    *out.f++ = *(base.f + (int32_t) xl + n * (int32_t) yl);
                  xl += xinc;
                  yl += yinc;
                }
                break;
              case LUX_DOUBLE:
                while (ic--) {
                  if (xl < 0 || xl >= fn || yl < 0 || yl >= fm)
                    *out.d++ = 0;
                  else
                    *out.d++ = *(base.d + (int32_t) xl + n * (int32_t) yl);
                  xl += xinc;
                  yl += yinc;
                }
                break;
            } // end of switch on type for inner loop
            beta++;
          }
        }
        ind += ngrun;
      }
      break;                    // end of nearest neighbor regrid case

    case 1:
        // bicubic with or without stretchmarks case
      mm1 = m-1;
      nm1 = n-1;
      nm2 = n-2;
      mm2 = m-2;
      fnm1 = fn - 1.0;
      fnm5 = fn - 0.5;
      fmm1 = fm - 1.0;
      fmm5 = fm - 0.5;
      // start 4 level loop
      // printf("mg, ng = %d, %d\n", mg, ng);
      do {                      // outer mg loop
        ipbase.ui8 = jpbase.ui8;
        jpbase.ui8 = ipbase.ui8 + jprun;
        ig = ng;
        j = ind;
        // ig loop
        do {
                                 // get the 8 grid values for this cell
          i = j;
          // printf("i = %d\n",i);
          ax = xgbase.f[i];
          ay = ygbase.f[i];
          i++;
          bx = xgbase.f[i] - ax;
          by = ygbase.f[i] - ay;
          i += ngrun;
          dx = xgbase.f[i] - ax;
          dy = ygbase.f[i] - ay;
          i--;
          cx = xgbase.f[i] - ax;
          cy = ygbase.f[i] - ay;
          dx = dx - bx - cx;
          dy = dy - by - cy;
          j++;
          xq = 1.0/(float) ns;
          bx *= xq;
          by *= xq;
          dx *= xq*yrun;
          dy *= xq*yrun;
          cx *= yrun;
          cy *= yrun;
          // setup for jc loop
          jbase.ui8 = ipbase.ui8;
          ipbase.ui8 = ipbase.ui8 + iprun;
          beta = 0.0;
          jc = ms;
          while (jc--) {
            // setup for ic loop
            // some optimizer trouble here on Alpha, re-arrange
            yinc = (by + beta*dy);
            ic = 0;
            out.ui8 = jbase.ui8;
            jbase.ui8 = jbase.ui8 + jrun;
            xl0 = ax + beta*cx;
            yl0 = ay + beta*cy; // different from regrid
            xinc = (bx + beta*dx);
            // except for 2 lines (and some declarations), identical (?) to
            // regrid to this point
            // use stretch marks on boundaries for cubic interpolation
            do {
              switch (resample_type) {
                case BI_CUBIC_SMOOTH:
                  bicubic_f();
                  break;
                case BI_CUBIC:
                  bicubic_fc();
                  break;
              }
              // The following commented lines yield systematic bias
              // in the results when the image is more than a few hundred
              // pixels in either dimension, because of roundoff error.
              // Replaced them with the following uncommented lines.
              // LS 26jul2000
              /* xl += xinc;
                 yl += yinc; */
              xl = xl0 + xinc*(++ic);
              yl = yl0 + yinc*ic;
            } while (ic < ns);
            beta++;
          }
        } while (--ig > 0);
        ind += ngrun;
      } while (--mg > 0);
      break;            // end of bicubic with stretchmarks case
    default:
      return cerror(IMPOSSIBLE, 0);
  }
  return result_sym;
}
//-------------------------------------------------------------------------
int32_t lux_compress(ArgumentCount narg, Symbol ps[])
// COMPRESS(data [, axes], factors) compresses the data array by the
// integer <factors> in the indicated <axes>.  If no <axes> are
// specified and <factors> contains a single value, then that value
// is applied to all dimensions.  LS 30apr98
// Fixed overflow problem.  LS 19nov99
{
  int32_t       iq, nFac, *factors, outDims[MAX_DIMS], i,
    n, offset, div[MAX_DIMS], range[2*MAX_DIMS], nel;
  Pointer       src, trgt;
  char  allAxes;
  Scalar        sum;
  LoopInfo      srcinfo, trgtinfo, tmpinfo;
  int32_t       lux_indgen(int32_t, int32_t []);

  if (symbol_class(ps[0]) != LUX_ARRAY)
    return cerror(NEED_ARR, ps[0]); // data not an array

  iq = (narg > 2)? ps[2]: ps[1]; // <factor>
  switch (symbol_class(iq)) {
    case LUX_SCAL_PTR:
      iq = dereferenceScalPointer(iq);
      // fall-thru
    case LUX_SCALAR:
      iq = lux_long(1, &iq);    // ensure LONG
      nFac = 1;
      factors = &scalar_value(iq).i32;
      break;
    case LUX_ARRAY:
      iq = lux_long(1, &iq);    // ensure LONG
      nFac = array_size(iq);    // number of factors
      factors = (int32_t *) array_data(iq);     // pointer to factors
      break;
    default:
      return cerror(ILL_CLASS, narg > 2? ps[2]: ps[1]);         // illegal
  }

  for (i = 0; i < nFac; i++)    // check that factors are legal
    if (factors[i] < 1)
      return luxerror("Compression factors must be positive integers", ps[1]);

  if (narg >= 3) {              // have <axes>
    iq = ps[1];
    // number of factors must equal to number of axes
    if ((nFac == 1
         && symbol_class(iq) != LUX_SCALAR
         && (symbol_class(iq) != LUX_ARRAY || array_size(iq) != 1))
        || (nFac > 1
            && symbol_class(iq) != LUX_ARRAY
            && array_size(iq) != nFac))
      return cerror(INCMP_ARG, ps[1]);
    allAxes = 0;
  } else {                      // no particular axes specified
    if (nFac == 1) {            // compress all dimensions
      iq = 0;                   // all axes
      allAxes = 1;
    } else {
      iq = lux_indgen(1, &ps[1]); // default: INDGEN(<factors>)
      allAxes = 0;
    }
  }

  if (standardLoop(ps[0], iq,
                   (allAxes? SL_ALLAXES: 0) | SL_EACHCOORD | SL_UNIQUEAXES,
                   LUX_INT8, &srcinfo, &src, NULL, NULL, NULL) < 0)
    return LUX_ERROR;           // some error

  // gather all compression factors
  if (allAxes)
    for (i = 0; i < srcinfo.ndim; i++)
      div[i] = factors[0];
  else {
    for (i = 0; i < srcinfo.rndim; i++)
      div[i] = 1;
    for (i = 0; i < nFac; i++)
      div[srcinfo.axes[i]] = factors[i];
  }
  for (i = 0; i < srcinfo.ndim; i++)
    if (div[i] > srcinfo.dims[i])
      return luxerror("Compression factor %1d exceeds the %1d elements in dimension %1d", (narg > 3 && ps[2])? ps[2]: 0, div[i], srcinfo.dims[i], i);

  /* we treat the biggest part of the data that has dimensions that
   are multiples of the compression factors */
  for (i = 0; i < srcinfo.ndim; i++) {
    range[2*i] = 0;
    range[2*i + 1] = srcinfo.dims[i] - (srcinfo.dims[i] % div[i]) - 1;
  }
  srcinfo.subdataLoop(range);   // restrict treated part

  // create the output symbol
  memcpy(outDims, srcinfo.dims, srcinfo.ndim*sizeof(int32_t));
  if (allAxes)
    for (i = 0; i < srcinfo.ndim; i++) {
      outDims[i] = outDims[i]/factors[0];
      if (!outDims[i])
        outDims[i] = 1;
    }
  else
    for (i = 0; i < nFac; i++) {
      outDims[srcinfo.axes[i]] = outDims[srcinfo.axes[i]]/factors[i];
      if (!outDims[srcinfo.axes[i]]) // ensure result dimension >= 1
        outDims[srcinfo.axes[i]] = 1;
    }
  iq = array_scratch(symbol_type(ps[0]), srcinfo.ndim, outDims);
  trgt.i32 = (int32_t*) array_data(iq);
  zerobytes(trgt.ui8, array_size(iq)*lux_type_size[array_type(iq)]);

  trgtinfo.setupDimensionLoop(srcinfo.ndim, outDims, symbol_type(ps[0]),
                              srcinfo.naxes, srcinfo.axes, &trgt, SL_EACHCOORD);

  nel = 1;
  for (i = 0; i < srcinfo.rndim; i++)
    nel *= div[i];

  // set up for walk through subarea that gets compressed into a single
  // element
  tmpinfo = srcinfo;
  zerobytes(range, srcinfo.ndim*2*sizeof(int32_t));
  for (i = 0; i < srcinfo.ndim; i++)
    range[2*i + 1] = div[i] - 1;
  tmpinfo.subdataLoop(range);

  offset = 0;
  switch (array_type(iq)) {
    case LUX_INT8:
      do {
        sum.i32 = 0;
        do
          sum.i32 += (int32_t) src.ui8[offset];
        while (tmpinfo.advanceLoop(&src.ui8) < tmpinfo.rndim);
        *trgt.ui8 = sum.i32/nel;
        src.ui8 = (uint8_t *) tmpinfo.data0;
        n = trgtinfo.advanceLoop(&trgt.ui8);
        offset = 0;
        for (i = 0; i < trgtinfo.ndim; i++)
          offset += srcinfo.rsinglestep[i]*trgtinfo.coords[i]*div[srcinfo.raxes[i]];
      } while (n < trgtinfo.rndim);
      break;
    case LUX_INT16:
      do {
        sum.i32 = 0;
        do
          sum.i32 += (int32_t) src.i16[offset];
        while (tmpinfo.advanceLoop(&src.ui8) < tmpinfo.rndim);
        *trgt.i16 = sum.i32/nel;
        src.i16 = (int16_t *) tmpinfo.data0;
        n = trgtinfo.advanceLoop(&trgt.ui8);
        offset = 0;
        for (i = 0; i < trgtinfo.ndim; i++)
          offset += srcinfo.rsinglestep[i]*trgtinfo.coords[i]*div[srcinfo.raxes[i]];
      } while (n < trgtinfo.rndim);
      break;
    case LUX_INT32:
      do {
        sum.i32 = 0;
        do
          sum.i32 += src.i32[offset];
        while (tmpinfo.advanceLoop(&src.ui8) < tmpinfo.rndim);
        *trgt.i32 = sum.i32/nel;
        src.i32 = (int32_t *) tmpinfo.data0;
        n = trgtinfo.advanceLoop(&trgt.ui8);
        offset = 0;
        for (i = 0; i < trgtinfo.ndim; i++)
          offset += srcinfo.rsinglestep[i]*trgtinfo.coords[i]*div[srcinfo.raxes[i]];
      } while (n < trgtinfo.rndim);
      break;
    case LUX_INT64:
      do {
        sum.i64 = 0;
        do
          sum.i64 += src.i64[offset];
        while (tmpinfo.advanceLoop(&src.ui8) < tmpinfo.rndim);
        *trgt.i64 = sum.i64/nel;
        src.i64 = (int64_t*) tmpinfo.data0;
        n = trgtinfo.advanceLoop(&trgt.ui8);
        offset = 0;
        for (i = 0; i < trgtinfo.ndim; i++)
          offset += srcinfo.rsinglestep[i]*trgtinfo.coords[i]*div[srcinfo.raxes[i]];
      } while (n < trgtinfo.rndim);
      break;
    case LUX_FLOAT:
      do {
        sum.d = 0;
        do
          sum.d += (double) src.f[offset];
        while (tmpinfo.advanceLoop(&src.ui8) < tmpinfo.rndim);
        *trgt.f = sum.d/nel;
        src.f = (float *) tmpinfo.data0;
        n = trgtinfo.advanceLoop(&trgt.ui8);
        offset = 0;
        for (i = 0; i < trgtinfo.ndim; i++)
          offset += srcinfo.rsinglestep[i]*trgtinfo.coords[i]*div[srcinfo.raxes[i]];
      } while (n < trgtinfo.rndim);
      break;
    case LUX_DOUBLE:
      do {
        sum.d = 0;
        do
          sum.d += src.d[offset];
        while (tmpinfo.advanceLoop(&src.ui8) < tmpinfo.rndim);
        *trgt.d = sum.d/nel;
        src.d = (double *) tmpinfo.data0;
        n = trgtinfo.advanceLoop(&trgt.ui8);
        offset = 0;
        for (i = 0; i < trgtinfo.ndim; i++)
          offset += srcinfo.rsinglestep[i]*trgtinfo.coords[i]*div[srcinfo.raxes[i]];
      } while (n < trgtinfo.rndim);
      break;
  }
  return iq;
}
//-------------------------------------------------------------------------
int32_t lux_oldcompress(ArgumentCount narg, Symbol ps[]) // compress function
// compresses image data by an integer factor
//  xc = compress(x, cx, [cy])
{
  int32_t       n, iq, i, j, cx, cy, nx, ny, result_sym, dim[2], nd, nxx;
  Symboltype type;
  float         xq, fac;
  double        dq, dfac;
  Array *h;
  Pointer q1, q2, p, base;

  iq = ps[0];
  CK_ARR(iq, 1);
  type = sym[iq].type;
  h = (Array *) sym[iq].spec.array.ptr;
  q1.i32 = (int32_t *) ((char *)h + sizeof(Array));
  nd = h->ndim;
  // we want a 1 or 2-D array only
  if ( nd > 2 ) return cerror(NEED_1D_2D_ARR, iq);
  cx = int_arg( ps[1] );                cy = cx;
  if (narg > 2 )  cy = int_arg( ps[2] );
  // get resultant sizes, check for degenerates
  cx = MAX(cx , 1);     cy = MAX(cy , 1);       // otherwise we could bomb
  nx = h->dims[0] / cx;  if (nx < 1) { cx = h->dims[0];  nx = 1; }
  if (nd > 1) ny = h->dims[1]; else ny = 1;
  ny = ny / cy;  if (ny < 1) { cy = h->dims[1];  ny = 1; }
  dim[0] = nx;  dim[1] = ny;    nxx = h->dims[0];
  result_sym = array_scratch(type, nd, dim);
  h = (Array *) sym[result_sym].spec.array.ptr;
  q2.i32 = (int32_t *) ((char *)h + sizeof(Array));
  fac = 1.0 / ( (float) cx * (float) cy );
  n = nxx - cx;                                         // step bewteen lines
  switch (type)
  { case LUX_INT8:
      while (ny--)
      {         base.ui8 = q1.ui8;  iq = nx;
        while (iq--)
        { p.ui8 = base.ui8; xq = 0.0;
          for (j = 0; j < cy; j++)
          { for (i = 0; i < cx; i++) xq += *p.ui8++;
            p.ui8 += n; }
          *q2.ui8++ = (uint8_t) ( xq * fac );
          base.ui8 += cx; }
        q1.ui8 +=  nxx * cy; }
      break;
    case LUX_INT16:
      while (ny--)
      {         base.i16 = q1.i16;  iq = nx;
        while (iq--)
        { p.i16 = base.i16; xq = 0.0;
          for (j=0;j<cy;j++)
          { for (i=0;i<cx;i++) xq += *p.i16++; p.i16 += n; }
          *q2.i16++ = (short) ( xq * fac );
          base.i16 += cx; }
        q1.i16 +=  nxx * cy; }
      break;
    case LUX_INT32:
      while (ny--)
      {         base.i32 = q1.i32;  iq = nx;
        while (iq--)
        { p.i32 = base.i32; xq = 0.0;
          for (j=0;j<cy;j++)
          { for (i=0;i<cx;i++) xq += *p.i32++; p.i32 += n; }
          *q2.i32++ = (int32_t) ( xq * fac );
          base.i32 += cx; }
        q1.i32 +=  nxx * cy; }
      break;
    case LUX_INT64:
      while (ny--)
      {         base.i64 = q1.i64;  iq = nx;
        while (iq--)
        { p.i64 = base.i64; xq = 0.0;
          for (j=0;j<cy;j++)
          { for (i=0;i<cx;i++) xq += *p.i64++; p.i64 += n; }
          *q2.i64++ = xq * fac;
          base.i64 += cx; }
        q1.i64 +=  nxx * cy; }
      break;
    case LUX_FLOAT:
      while (ny--)
      {         base.f = q1.f;  iq = nx;
        while (iq--)
        { p.f = base.f; xq = 0.0;
          for (j=0;j<cy;j++)
          { for (i=0;i<cx;i++) xq += *p.f++; p.f += n; }
          *q2.f++ =  ( xq * fac );
          base.f += cx; }
        q1.f +=  nxx * cy; }
      break;
    case LUX_DOUBLE:
      dfac = 1.0 / ( (double) cx * (double) cy );
      while (ny--)
      {         base.d = q1.d;  iq = nx;
        while (iq--)
        { p.d = base.d; dq = 0.0;
          for (j=0;j<cy;j++)
          { for (i=0;i<cx;i++) dq += *p.d++; p.d += n; }
          *q2.d++ = (double) ( dq * dfac );
          base.d += cx; }
        q1.d +=  nxx * cy; }
      break;
    }
  return result_sym;
}
 //-------------------------------------------------------------------------
int32_t lux_sort(ArgumentCount narg, Symbol ps[])
//sort contents of a copy of an array
{
  int32_t       iq, result_sym, n, nloop, step;
  Symboltype type;
  Pointer       p;
  char  sortType;

  // SGI cc does not accept combination of (int32_t, uint8_t *) and (int32_t, int16_t *)
  // functions in one array of function pointers, not even if the function
  // pointer array is defined as (int32_t, void *).
  void sort_b(int32_t, uint8_t*);
  void sort_w(int32_t, int16_t*);
  void sort_l(int32_t, int32_t*);
  void sort_int64(int64_t, int64_t*);
  void sort_f(int32_t, float*);
  void sort_d(int32_t, double*);
  void sort_s(int32_t, char**);
  void shell_b(int32_t, uint8_t*);
  void shell_w(int32_t, int16_t*);
  void shell_l(int32_t, int32_t*);
  void shell_int64(int64_t, int64_t*);
  void shell_f(int32_t, float*);
  void shell_d(int32_t, double*);
  void shell_s(int32_t, char**);
  int32_t       lux_replace(int32_t, int32_t);

  iq = ps[0];
  if (!symbolIsArray(iq))
    return cerror(NEED_ARR, iq);
  type = array_type(iq);
  if (type >= LUX_CFLOAT || type == LUX_TEMP_STRING || type == LUX_LSTRING)
    return cerror(ILL_TYPE, iq, typeName(type));
  if (type == LUX_STRING_ARRAY)
    type = (Symboltype) (LUX_DOUBLE + 1); // so we get the right sort function
  result_sym = array_clone(iq, type);
  /* make a copy of original array, we sort the copy. Note that if
     original is already a temp, there is no copy and it is sorted in place */
  if (lux_replace(result_sym, iq) != 1)
    return cerror(IMPOSSIBLE, 0);
  p.i32 = (int32_t*) array_data(result_sym);
  n = array_size(result_sym);
  if (n <= 1)                   // nothing to sort
    return result_sym;
  if (internalMode & 4) {       // along 0th dimension
    nloop = n/array_dims(result_sym)[0];
    n = array_dims(result_sym)[0];
  } else
    nloop = 1;
  switch (internalMode & ~4) {
    default:
      luxerror("Cannot select multiple sort methods!  Default is used.", 0);
    case 0:                     // default method
      sortType = sort_flag? 1: 0;
      break;
    case 1:                     // heap sort
      sortType = 0;
      break;
    case 2:                     // shell sort
      sortType = 1;
      break;
  }
  step = n*lux_type_size[type];
  if (isStringType(type))
    type = LUX_TEMP_STRING;
  /*note - the default uses Press etal's heap sort, experiments with
    shell sort indicated it was slower for all cases that reliable
    times could be obtained on (about n>=10000) */
  switch (sortType) {
  case 0:                       // heap sort
    switch (type) {
    case LUX_INT8:
      for ( ; nloop--; p.ui8 += step)
        sort_b(n, p.ui8);
      break;
    case LUX_INT16:
      for ( ; nloop--; p.ui8 += step)
        sort_w(n, p.i16);
      break;
    case LUX_INT32:
      for ( ; nloop--; p.ui8 += step)
        sort_l(n, p.i32);
      break;
    case LUX_INT64:
      for ( ; nloop--; p.ui8 += step)
        sort_int64(n, p.i64);
      break;
    case LUX_FLOAT:
      for ( ; nloop--; p.ui8 += step)
        sort_f(n, p.f);
      break;
    case LUX_DOUBLE:
      for ( ; nloop--; p.ui8 += step)
        sort_d(n, p.d);
      break;
    case LUX_TEMP_STRING:
      for ( ; nloop--; p.ui8 += step)
        sort_s(n, p.sp);
      break;
    default:
      return cerror(ILL_TYPE, ps[0]);
    }
    break;
  case 1:                       // shell sort
    switch (type) {
    case LUX_INT8:
      for ( ; nloop--; p.ui8 += step)
        shell_b(n, p.ui8);
      break;
    case LUX_INT16:
      for ( ; nloop--; p.ui8 += step)
        shell_w(n, p.i16);
      break;
    case LUX_INT32:
      for ( ; nloop--; p.ui8 += step)
        shell_l(n, p.i32);
      break;
    case LUX_INT64:
      for ( ; nloop--; p.ui8 += step)
        shell_int64(n, p.i64);
      break;
    case LUX_FLOAT:
      for ( ; nloop--; p.ui8 += step)
        shell_f(n, p.f);
      break;
    case LUX_DOUBLE:
      for ( ; nloop--; p.ui8 += step)
        shell_d(n, p.d);
      break;
    case LUX_TEMP_STRING:
      for ( ; nloop--; p.ui8 += step)
        shell_s(n, p.sp);
      break;
    default:
      return cerror(ILL_TYPE, ps[0]);
    }
    break;
  }
  return result_sym;
}
 //-------------------------------------------------------------------------
int32_t lux_index(ArgumentCount narg, Symbol ps[])
 // construct a sorted index table for an array
 // the index is returned as a long array of the same size
 // uses heap sort only
{
  int32_t       iq, type, result_sym, n, nloop, step1, step2, nloop2;
  Pointer       p, q;
  void invertPermutation(int32_t *, int32_t);
  void indexx_b(int32_t, uint8_t*, int32_t*);
  void indexx_w(int32_t, int16_t*, int32_t*);
  void indexx_l(int32_t, int32_t*, int32_t*);
  void indexx_int64(int64_t, int64_t*, int64_t*);
  void indexx_f(int32_t, float*, int32_t*);
  void indexx_d(int32_t, double*, int32_t*);
  void indexx_s(int32_t, char**, int32_t*);

  iq = ps[0];
  if (symbol_class(iq) != LUX_ARRAY)
    return cerror(NEED_ARR, iq);
  type = symbol_type(iq);
  if (type >= LUX_CFLOAT)
    return cerror(ILL_TYPE, iq, typeName(type));
  q.i32 = (int32_t *) array_data(iq);
  n = array_size(iq);
  if (n <= 1)                   // nothing to sort
    return LUX_ZERO;
  result_sym = array_clone(iq, LUX_INT32);
  p.i32 = (int32_t *) array_data(result_sym);
  if (internalMode & 1) {       // along 0th dimension
    nloop = n/array_dims(iq)[0];
    n = array_dims(iq)[0];
  } else
    nloop = 1;
  nloop2 = nloop;
  step1 = n*lux_type_size[type];
  step2 = n*sizeof(int32_t);
  if (isStringType(type))
    type = LUX_TEMP_STRING;
  switch (type) {
  case LUX_INT8:
    for ( ; nloop--; q.ui8 += step1, p.ui8 += step2)
      indexx_b(n, q.ui8, p.i32);
    break;
  case LUX_INT16:
    for ( ; nloop--; q.ui8 += step1, p.ui8 += step2)
      indexx_w(n, q.i16, p.i32);
    break;
  case LUX_INT32:
    for ( ; nloop--; q.ui8 += step1, p.ui8 += step2)
      indexx_l(n, q.i32, p.i32);
    break;
  case LUX_INT64:
    for ( ; nloop--; q.ui8 += step1, p.ui8 += step2)
      indexx_int64(n, q.i64, p.i64);
    break;
  case LUX_FLOAT:
    for ( ; nloop--; q.ui8 += step1, p.ui8 += step2)
      indexx_f(n, q.f, p.i32);
    break;
  case LUX_DOUBLE:
    for ( ; nloop--; q.ui8 += step1, p.ui8 += step2)
      indexx_d(n, q.d, p.i32);
    break;
  case LUX_TEMP_STRING:
    for ( ; nloop--; q.ui8 += step1, p.ui8 += step2)
      indexx_s(n, q.sp, p.i32);
    break;
  default:
    return cerror(ILL_TYPE, ps[0]);
  }
  if (internalMode & 2) {       // reverse index to get list of ranks
    p.i32 = (int32_t *) array_data(result_sym); // back to start of data
    // we reverse the list by moving each number x to the position
    // indicated at p.i32[x], and moving the old value at that
    // position in the same fashion until we've completed a cycle
    // and returned to the first x.  There may be more than one
    // cycles in the data, so we keep track of the number we've
    // already shifted to determine if we're all done.  LS 6may96
    while (nloop2--) {
      invertPermutation(p.i32, n);
      p.i32 += n;
    }
  }
  return result_sym;
}
 //-------------------------------------------------------------------------
static char const* zoomtemp = "$ZOOM_TEMP";
int32_t zoomer2(uint8_t *ain, int32_t n, int32_t m, int32_t *symout, int32_t *nx2, int32_t *ny2,
            int32_t sym_flag)
// internal zoom2
// zoom a uint8_t array by 2
// requires the original address, some size info
// creates a symbol $zoom_temp to allow re-use of memory for some calls
// RAS
{
 int32_t        ns, ms, dim[2], j, i, ns4, result_sym, leftover;
 uint8_t        *pin, *pout, *poutbase, tmp;
 int32_t        *p1, *p2, k;
#ifdef __alpha
 long   *p8;
 int32_t        alpha_flag = 0;
#endif

 pin = ain;

 // for current testing on alpha, n%4 must be 0
#ifdef __alpha
 if (n % 4 == 0)
   alpha_flag = 1;
#endif
                                // compute new ns and ms
 ns = n * 2;                    // output width
 ms = m * 2;                    // output height
 leftover = (4 - ns % 4) % 4;
 ns = ns + leftover;            // thus ns is 0 mod 4
 *nx2 = ns;                     // return output width
 *ny2 = ms;                     // return output height
 dim[0] = ns;
 dim[1] = ms;
 /* depending on the state of sym_flag, we create a temporary symbol
    or use $zoom_temp, the latter is for tvplanezoom */
 if (sym_flag) {
   result_sym = array_scratch(LUX_INT8, 2, dim);
   if (result_sym < 0)
     return LUX_ERROR;
 } else {
   result_sym = findVarName(zoomtemp, 0);
   if (redef_array(result_sym, LUX_INT8, 2, dim) != 1)
     return LUX_ERROR;
 }
 *symout = result_sym;

 pout = poutbase = (uint8_t*) array_data(result_sym);

 // first load every other line
 //t1 = systime();
#ifdef __alpha
 if (alpha_flag) {              /* this is a faster method if we have every
                                   row aligned I*4 */
   p8 = (long *) poutbase;
   p1 = (int32_t *) pin;
   for (j = 0; j < m; j++) {
     for (i = 0; i < n/4; i++) {
       register unsigned long pix = (long) *p1++;
       register unsigned long  zoom_reg = 0;

       zoom_reg += pix & 0xff;
       pix = pix << 8;
       zoom_reg += pix & 0xffff00;
       pix = pix << 8;
       zoom_reg += pix & 0xffff000000;
       pix = pix << 8;
       zoom_reg += pix & 0xffff0000000000;
       pix = pix << 8;
       zoom_reg += pix & 0xff00000000000000;
       *p8++ = zoom_reg;
     }
     p8 += n/4;
   }
 } else {                       // otherwise we do it the plain way
   for (j = 0; j < m; j++) {
     for (i = 0; i < n; i++) {
       *pout++ = *pin;
       *pout++ = *pin;
       pin++;
     }
     if (leftover) {
       k = leftover;
       tmp = *(pin - 1);
       while (k--)
         *pout++ = tmp;
     }
     pout += ns;
   }
 }
#else
 // for non-alpha, always the plain way
 for (j = 0; j < m; j++) {
   for (i = 0; i < n; i++) {
     *pout++ = *pin;
     *pout++ = *pin;
     pin++;
   }
   if (leftover) {
     k = leftover;
     tmp = *(pin - 1);
     while (k--)
       *pout++ = tmp;
   }
   pout += ns;
 }
#endif

 //t2 = systime();
 // now dupe the lines, this is why we wanted ns%4 = 0
 ns4 = ns/4;
 p1 = (int32_t *) poutbase;
 p2 = p1 + ns4;
 for (j = 0; j < m; j++) {
   for (i = 0; i < ns4; i++) {
     *p2++ = *p1++;
   }
   p1 += ns4;
   p2 += ns4;
 }
 return 1;
}
//-------------------------------------------------------------------------
int32_t zoomer3(uint8_t *ain, int32_t n, int32_t m, int32_t *symout, int32_t *nx2, int32_t *ny2,
            int32_t sym_flag) // internal zoom3
 // zoom a uint8_t array by 3
 // requires the original address, some size info
 // creates a symbol $zoom_temp to allow re-use of memory for some calls
// RAS
{
  int32_t       ns, ms, dim[2], j, i, ns4, result_sym, leftover;
  uint8_t       *pin, *pout, *poutbase, tmp;
  int32_t       *p1, *p2, *p3, nst2, tmpint, k, nsd2;

  pin = ain;
                                // compute new ns and ms
  ns = n*3;
  ms = m*3;
  leftover = (4 - ns % 4) % 4;
  ns = ns + leftover;   // pad to even multiple of 4 for I*4 transfers
  *nx2 = ns;
  *ny2 = ms;
  dim[0] = ns;
  dim[1] = ms;
  nst2 = 2*ns;
  /* depending on the state of sym_flag, we create a temporary symbol
     or use $zoom_temp, the latter is for tvplanezoom */
  if (sym_flag) {
    result_sym = array_scratch(LUX_INT8, 2, dim);
    if (result_sym < 0)
      return LUX_ERROR;
  } else {
    result_sym = findVarName(zoomtemp, 0);
    if (redef_array(result_sym, LUX_INT8, 2, dim) != 1)
      return LUX_ERROR;
  }
  *symout = result_sym;
  pout = poutbase = (uint8_t*) array_data(result_sym);

  // first load every third line
  //t1 = systime();
  for (j = 0; j < m; j++) {
    for (i = 0; i < n; i++) {
      tmp = *pin++;
      *pout++ = tmp;
      *pout++ = tmp;
      *pout++ = tmp;
    }
    if (leftover) {
      k = leftover;
      tmp = *(pin - 1);
      while (k--)
        *pout++ = tmp;
    }
    pout += nst2;
  }

  //t2 = systime();
  // now dupe the lines, this is why we needed ns%4 = 0
  ns4 = ns/4;
  nsd2 = ns/2;
  p1 = (int32_t *) poutbase;
  p2 = p1 + ns4;
  p3 = p2 + ns4;
  for (j = 0; j < m; j++) {
    for (i = 0; i < ns4; i++) {
      tmpint = *p1++;
      *p2++ = tmpint;
      *p3++ = tmpint;
    }
    p1 += nsd2;
    p2 += nsd2;
    p3 += nsd2;
  }
  return 1;
}
//-------------------------------------------------------------------------
int32_t zoomer4(uint8_t *ain, int32_t n, int32_t m, int32_t *symout, int32_t *nx2, int32_t *ny2,
            int32_t sym_flag) // internal zoom4
// zoom a uint8_t array by 4
// requires the original address, some size info
// creates a symbol $zoom_temp to allow re-use of memory for some calls
// RAS
{
  int32_t       ns, ms, dim[2], j, i, ns4, result_sym;
  uint8_t       *pin, *pout, *poutbase, tmp;
  int32_t       *p1, *p2, *p3, *p4, nst3, tmpint, nsd2;

  pin = ain;
                                // compute new ns and ms
  ns = n*4;
  ms = m*4;                     // no leftovers for x4
  *nx2 = ns;
  *ny2 = ms;
  dim[0] = ns;
  dim[1] = ms;
  nst3 = 3*ns;
  /* depending on the state of sym_flag, we create a temporary symbol
     or use $zoom_temp, the latter is for tvplanezoom */
  if (sym_flag) {
    result_sym = array_scratch(LUX_INT8, 2, dim);
    if (result_sym < 0)
      return LUX_ERROR;
  } else {
    result_sym = findVarName(zoomtemp, 0);
    if (redef_array(result_sym, LUX_INT8, 2, dim) != 1)
      return LUX_ERROR;
  }
  *symout = result_sym;
  pout = poutbase = (uint8_t*) array_data(result_sym);

  // first load every fourth line
  //t1 = systime();
  for (j = 0; j < m; j++) {
    for (i = 0; i < n; i++) {
      tmp = *pin++;
      *pout++ = tmp;
      *pout++ = tmp;
      *pout++ = tmp;
      *pout++ = tmp;
    }
    pout += nst3;
  }

  //t2 = systime();
  // now dupe the lines, this is why we needed ns%4 = 0
  ns4 = ns/4;
  nsd2 = 3*ns4;
  p1 = (int32_t *) poutbase;
  p2 = p1 + ns4;
  p3 = p2 + ns4;
  p4 = p3 + ns4;
  for (j = 0; j < m; j++) {
    for (i = 0; i < ns4; i++) {
      tmpint = *p1++;
      *p2++ = tmpint;
      *p3++ = tmpint;
      *p4++ = tmpint;
    }
    p1 += nsd2;
    p2 += nsd2;
    p3 += nsd2;
    p4 += nsd2;
  }
 return 1;
}
//-------------------------------------------------------------------------
int32_t zoomer8(uint8_t *ain, int32_t n, int32_t m, int32_t *symout, int32_t *nx2, int32_t *ny2,
            int32_t sym_flag) // internal zoom8
// zoom a uint8_t array by 8
// requires the original address, some size info
// creates a symbol $zoom_temp to allow re-use of memory for some calls
// RAS
{
  int32_t       ns, ms, dim[2], j, i, result_sym;
  uint8_t       *pin, *poutbase;
  union         { uint8_t       bb[4];   int32_t  ii; } tmp;
  int32_t       *p1, *p2, delta;

  pin = ain;
                                // compute new ns and ms
  ns = n*8;
  ms = m*8;                     // no leftovers for x8
  *nx2 = ns;
  *ny2 = ms;
  dim[0] = ns;
  dim[1] = ms;
  /* depending on the state of sym_flag, we create a temporary symbol
     or use $zoom_temp, the latter is for tvplanezoom */
  if (sym_flag) {
    result_sym = array_scratch(LUX_INT8, 2, dim);
    if (result_sym < 0)
      return LUX_ERROR;
  } else {
    result_sym = findVarName(zoomtemp, 0);
    if (redef_array(result_sym, LUX_INT8, 2, dim) != 1)
      return LUX_ERROR;
  }
  *symout = result_sym;
  poutbase = (uint8_t*) array_data(result_sym);
  p1 = (int32_t *) poutbase;
  delta = 2*(n - 1);

  //t1 = systime();
  for (j = 0; j < m; j++) {
    for (i = 0; i < n; i++) {
      tmp.bb[0] = tmp.bb[1] = tmp.bb[2] = tmp.bb[3] = *pin++;
      *p1++ = tmp.ii;
      *p1++ = tmp.ii;
      p2 = p1 + delta;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      p2 = p2 + delta;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      p2 = p2 + delta;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      p2 = p2 + delta;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      p2 = p2 + delta;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      p2 = p2 + delta;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      p2 = p2 + delta;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
    }
    p1 = p2;
  }
  return 1;
}
//-------------------------------------------------------------------------
int32_t zoomer16(uint8_t *ain, int32_t n, int32_t m, int32_t *symout, int32_t *nx2, int32_t *ny2,
             int32_t sym_flag) // internal zoom16
// zoom a uint8_t array by 16
// requires the original address, some size info
// creates a symbol $zoom_temp to allow re-use of memory for some calls
// RAS
{
  int32_t       ns, ms, dim[2], j, i, result_sym;
  uint8_t       *pin, *poutbase;
  union         { uint8_t       bb[4];   int32_t  ii; } tmp;
  int32_t       *p1, *p2, delta;

  pin = ain;
                                // compute new ns and ms
  ns = n*16;
  ms = m*16;                    // no leftovers for x8
  *nx2 = ns;
  *ny2 = ms;
  dim[0] = ns;
  dim[1] = ms;
  /* depending on the state of sym_flag, we create a temporary symbol
     or use $zoom_temp, the latter is for tvplanezoom */
  if (sym_flag) {
    result_sym = array_scratch(LUX_INT8, 2, dim);
    if (result_sym < 0)
      return LUX_ERROR;
  } else {
    result_sym = findVarName(zoomtemp, 0);
    if (redef_array(result_sym, LUX_INT8, 2, dim) != 1)
      return LUX_ERROR;
  }
  *symout = result_sym;
  poutbase = (uint8_t*) array_data(result_sym);
  p1 = (int32_t *) poutbase;
  delta = 4*(n - 1);

  //t1 = systime();
  for (j = 0; j < m; j++) {
    for (i = 0; i < n; i++) {
      tmp.bb[0] = tmp.bb[1] = tmp.bb[2] = tmp.bb[3] = *pin++;
      *p1++ = tmp.ii;
      *p1++ = tmp.ii;
      *p1++ = tmp.ii;
      *p1++ = tmp.ii;
      p2 = p1 + delta;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      p2 = p2 + delta;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      p2 = p2 + delta;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      p2 = p2 + delta;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      p2 = p2 + delta;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      p2 = p2 + delta;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      p2 = p2 + delta;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      p2 = p2 + delta;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      p2 = p2 + delta;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      p2 = p2 + delta;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      p2 = p2 + delta;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      p2 = p2 + delta;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      p2 = p2 + delta;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      p2 = p2 + delta;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      p2 = p2 + delta;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
      *p2++ = tmp.ii;
    }
    p1 = p2;
  }
  return 1;
}
//-------------------------------------------------------------------------
int32_t compress2(uint8_t *ain, int32_t n, int32_t m, int32_t *symout, int32_t *nx2, int32_t *ny2,
              int32_t sym_flag) // internal compress2
// compress a uint8_t array by 2
// requires the original address, some size info
// creates a symbol $zoom_temp to allow re-use of memory for some calls
// RAS
{
  int32_t       ns, ms, dim[2], result_sym;
  uint8_t       *pin, *pout, *poutbase, *p1, *p2;
  int32_t       nx, ny, xq, nskip;

  pin = ain;
                                        // compute new ns and ms
  ns = n/2;
  ms = m/2;
  *nx2 = ns;
  *ny2 = ms;
  dim[0] = ns;
  dim[1] = ms;
  /* depending on the state of sym_flag, we create a temporary symbol
     or use $zoom_temp, the latter is for tvplanezoom */
  if (sym_flag) {
    result_sym = array_scratch(LUX_INT8, 2, dim);
    if (result_sym < 0)
      return LUX_ERROR;
  } else {
    result_sym = findVarName(zoomtemp, 0);
    if (redef_array(result_sym, LUX_INT8, 2, dim) != 1)
      return LUX_ERROR;
  }
  *symout = result_sym;
  pout = poutbase = (uint8_t*) array_data(result_sym);

  ny = ms;
  nskip = n + n%2;
  p1 = pin;
  p2 = p1 + n;
  while (ny--) {
    nx = ns;
    while (nx--) {
      // sum the 4 pixels here
      xq = *p1++;
      xq += *p2++;
      xq += *p1++;
      xq += *p2++;
      *pout++ = (uint8_t) (xq/4);
    }
    p1 += nskip;
    p2 += nskip;
  }
  return 1;
}
//-------------------------------------------------------------------------
int32_t compress4(uint8_t *ain, int32_t n, int32_t m, int32_t *symout, int32_t *nx2, int32_t *ny2,
              int32_t sym_flag) // internal compress4
// compress a uint8_t array by 4
// requires the original address, some size info
// creates a symbol $zoom_temp to allow re-use of memory for some calls
// RAS
{
 int32_t        ns, ms, dim[2], result_sym;
 uint8_t        *pin, *pout, *poutbase, *p1, *p2, *p3, *p4;
 int32_t        nx, ny, xq, nskip;

 pin = ain;
                                        // compute new ns and ms
 ns = n/4;
 ms = m/4;
 *nx2 = ns;
 *ny2 = ms;
 dim[0] = ns;
 dim[1] = ms;
 /* depending on the state of sym_flag, we create a temporary symbol
 or use $zoom_temp, the latter is for tvplanezoom */
 if (sym_flag) {
   result_sym = array_scratch(LUX_INT8, 2, dim);
   if (result_sym < 0)
     return LUX_ERROR;
 } else {
   result_sym = findVarName(zoomtemp, 0);
   if (redef_array(result_sym, LUX_INT8, 2, dim) != 1)
     return LUX_ERROR;
 }
 *symout = result_sym;
 pout = poutbase = (uint8_t*) array_data(result_sym);

 ny = ms;
 nskip = 3* n + n%4;
 p1 = pin;
 p2 = p1 + n;
 p3 = p2 + n;
 p4 = p3 + n;
 while (ny--) {
   nx = ns;
   while (nx--) {
     // sum the 16 pixels here
     xq = *p1++;
     xq += *p2++;
     xq += *p3++;
     xq += *p4++;
     xq += *p1++;
     xq += *p2++;
     xq += *p3++;
     xq += *p4++;
     xq += *p1++;
     xq += *p2++;
     xq += *p3++;
     xq += *p4++;
     xq += *p1++;
     xq += *p2++;
     xq += *p3++;
     xq += *p4++;
     *pout++ = (uint8_t) (xq/16);
   }
   p1 += nskip;
   p2 += nskip;
   p3 += nskip;
   p4 += nskip;
 }
 return 1;
}
//-------------------------------------------------------------------------
void shift_bicubic(float dx, int32_t nx, int32_t ny, int32_t inc, int32_t dline,
                   Pointer base, Pointer out, int32_t type)
 // internal routine for shift in one direction
{
  int32_t       i1, i2, i3, i4, k;
  float         c1, c2, c3, c4, dx0, dx1, dx2, dx3, dx4;
  double        cd1, cd2, cd3, cd4, ddx0, ddx1, ddx2, ddx3, ddx4;
  int32_t       nzone2, nzone3, nzone4, nz2;
  int32_t       nz3, nz4, rflag;

  nm1 = nx -1; rflag = 0;
  // get the fraction shift and the integer shift
  if (dx<0) {
    /* handle negative offsets by flipping and starting at end of array, this
       allows in and out array to always be the same without overwrites */
    rflag = nm1*inc;  // not a flag but an offset
    inc = - inc;
    dx = -dx;
  }
  i2 = (int32_t) dx;
  dx0 = dx - i2;

  // 3 zones, zone 1 gone with reversal technique
  nzone2 = nzone3 = nzone4 = 0;
  if (i2 == 0) {
    nzone3 = nzone4 = 1;  nzone2 = nx - 2;
  }
  if (i2 > 0) {
    nzone4 =  MIN(i2 + 1, nx);
    if (nzone4 < nx) nzone3 = 1; else nzone3 = 0;
    nzone2 = nx - nzone4 - nzone3;
  }
  // get the first 4 values we will need, start at (i2-1)>0
  i2 = MIN(i2, nm1);    i2 = MAX( i2, 0);
  i1 = MIN(i2-1, nm1);  i1 = MAX( i1, 0);
  i3 = MIN(i2+1, nm1);
  i4 = MIN(i3+1, nm1);
  i1 = i1*inc;  i2 = i2*inc;  i3 = i3*inc;  i4 = i4*inc;
  if (type < 4) {
    // coefficients are done in F*4 unless we need a F*8 result
    dx1 = 1.0 - dx0;
    switch (resample_type) {
      case BI_CUBIC_SMOOTH:

        dx2 = -dx0 * 0.5;  dx3 = dx0 * dx2;
        dx4 = 3. * dx0 * dx3;
        c1 = dx2*dx1*dx1; c2 = 1.-dx4+5.*dx3; c3 = dx4-(dx2+4.*dx3); c4 = dx3*dx1;
        break;
        
      case BI_CUBIC:
        dx4 = -dx0*dx1;  c1 = dx4 * dx1;  c4 = dx0 * dx4;
        dx2 = dx0 * dx0; dx3 = dx0 * dx2; c2 = 1.-2.*dx2+dx3; c3 = dx0*(1.0+dx0-dx2);
        break;
    }


  } else {
    // the F*8 case, use alternate coefficients
    ddx0 = (double) dx0;
    ddx1 = 1.0 - ddx0;
    switch (resample_type) {
      case BI_CUBIC_SMOOTH:
        
        ddx2 = -ddx0 * 0.5;  ddx3 = ddx0 * ddx2;
        ddx4 = 3. * ddx0 * ddx3;
        cd1 = ddx2*ddx1*ddx1; cd2 = 1.-ddx4+5.*ddx3; cd3 = ddx4-(ddx2+4.*ddx3);
        cd4 = ddx3*ddx1;
        break;
        
      case BI_CUBIC:
        ddx4 = -ddx0*ddx1;  cd1 = ddx4 * ddx1;  cd4 = ddx0 * ddx4;
        ddx2 = ddx0 * ddx0; ddx3 = ddx0 * ddx2; cd2 = 1.-2.*ddx2+ddx3;
        cd3 = ddx0*(1.0+ddx0-ddx2);
        break;
    }


  }
  switch (type) {
    case LUX_INT8:      // I*1 image
    {
      float     z4, z1, z2, z3, yq;
      uint8_t   *p, *q;
      for (k=0;k<ny;k++) {
        p = base.ui8 + k*dline + rflag;
        q = out.ui8 + k*dline + rflag;
        z1 = (float) *(p + i1);         z2 = (float) *(p + i2);
        z3 = (float) *(p + i3);         z4 = (float) *(p + i4);
        p = p + i4;
        nz2 = nzone2;  nz3 = nzone3;  nz4 = nzone4;
        
        if (nz2) {
          yq = c1*z1 +c2*z2 + c3*z3 + c4*z4;
          // uint8_t arrays need to be range restricted
          yq = MAX( 0, MIN( 255.0, yq));
          *q = (uint8_t) yq;
          q = q + inc;
          nz2--;
          while (nz2--) {
            p = p + inc;
            z1 = z2;  z2 = z3;  z3 = z4;
            z4 = *p;
            yq = c1*z1 +c2*z2 + c3*z3 + c4*z4;
            yq = MAX( 0, MIN( 255.0, yq));
            *q = (uint8_t) yq;
            q = q + inc;
          }
        }
        if (nz3) {
          z1 = z2;  z2 = z3;  z3 = z4;
          yq = c1*z1 +c2*z2 + c3*z3 + c4*z4;
          yq = MAX( 0, MIN( 255.0, yq));
          *q = (uint8_t) yq;
          q = q + inc;
        }
        z4 = MAX( 0, MIN( 255.0, z4));
        while (nz4--) { *q = (uint8_t) z4;  q = q + inc; }
      }
    }
     break;
    case LUX_INT16:
    {
      float     z4, z1, z2, z3, yq;
      short     *p, *q;
      for (k=0;k<ny;k++) {
        p = base.i16 + k*dline + rflag;
        q = out.i16  + k*dline + rflag;
        z1 = (float) *(p + i1);         z2 = (float) *(p + i2);
        z3 = (float) *(p + i3);         z4 = (float) *(p + i4);
        p = p + i4;
        nz2 = nzone2;  nz3 = nzone3;  nz4 = nzone4;
        
        if (nz2) {
          yq = c1*z1 +c2*z2 + c3*z3 + c4*z4;
          *q = (short) yq;
          q = q + inc;
          nz2--;
          while (nz2--) {
            p = p + inc;
            z1 = z2;  z2 = z3;  z3 = z4;
            z4 = *p;
            yq = c1*z1 +c2*z2 + c3*z3 + c4*z4;
            *q = (short) yq;
            q = q + inc;
          }
        }
        if (nz3) {
          z1 = z2;  z2 = z3;  z3 = z4;
          yq = c1*z1 +c2*z2 + c3*z3 + c4*z4;
          *q = (short) yq;
          q = q + inc;
        }
        while (nz4--) { *q = (short) z4;  q = q + inc; }
      }
    }
     break;
    case LUX_INT32:
    {
      float     z4, z1, z2, z3, yq;
      int32_t   *p, *q;
      for (k=0;k<ny;k++) {
        p = base.i32 + k*dline + rflag;
        q = out.i32  + k*dline + rflag;
        z1 = (float) *(p + i1);         z2 = (float) *(p + i2);
        z3 = (float) *(p + i3);         z4 = (float) *(p + i4);
        p = p + i4;
        nz2 = nzone2;  nz3 = nzone3;  nz4 = nzone4;

        if (nz2) {
          yq = c1*z1 +c2*z2 + c3*z3 + c4*z4;
          *q = (int32_t) yq;
          q = q + inc;
          nz2--;
          while (nz2--) {
            p = p + inc;
            z1 = z2;  z2 = z3;  z3 = z4;
            z4 = *p;
            yq = c1*z1 +c2*z2 + c3*z3 + c4*z4;
            *q = (int32_t) yq;
            q = q + inc;
          }
        }
        if (nz3) {
          z1 = z2;  z2 = z3;  z3 = z4;
          yq = c1*z1 +c2*z2 + c3*z3 + c4*z4;
          *q = (int32_t) yq;
          q = q + inc;
        }
        while (nz4--) { *q = (int32_t) z4;  q = q + inc; }
      }
    }
     break;
    case LUX_INT64:
    {
      double    z4, z1, z2, z3, yq;
      int64_t   *p, *q;
      for (k=0;k<ny;k++) {
        p = base.i64 + k*dline + rflag;
        q = out.i64  + k*dline + rflag;
        z1 = *(p + i1);         z2 = *(p + i2);
        z3 = *(p + i3);         z4 = *(p + i4);
        p = p + i4;
        nz2 = nzone2;  nz3 = nzone3;  nz4 = nzone4;

        if (nz2) {
          yq = c1*z1 +c2*z2 + c3*z3 + c4*z4;
          *q = yq;
          q = q + inc;
          nz2--;
          while (nz2--) {
            p = p + inc;
            z1 = z2;  z2 = z3;  z3 = z4;
            z4 = *p;
            yq = c1*z1 +c2*z2 + c3*z3 + c4*z4;
            *q = yq;
            q = q + inc;
          }
        }
        if (nz3) {
          z1 = z2;  z2 = z3;  z3 = z4;
          yq = c1*z1 +c2*z2 + c3*z3 + c4*z4;
          *q = yq;
          q = q + inc;
        }
        while (nz4--) { *q = z4;  q = q + inc; }
      }
    }
     break;
    case LUX_FLOAT:     // floating F*4
    {
      float     z4, z1, z2, z3, yq;
      float     *p, *q;
      for (k=0;k<ny;k++) {
        p = base.f + k*dline + rflag;
        q = out.f + k*dline + rflag;
        z1 = *(p + i1);         z2 = *(p + i2);         z3 = *(p + i3);
        z4 = *(p + i4);
        p = p + i4;
        nz2 = nzone2;  nz3 = nzone3;  nz4 = nzone4;
        
        if (nz2) {
          yq = c1*z1 +c2*z2 + c3*z3 + c4*z4;
          *q = yq;
          q = q + inc;
          nz2--;
          while (nz2--) {
            p = p + inc;
            z1 = z2;  z2 = z3;  z3 = z4;
            z4 = *p;
            yq = c1*z1 +c2*z2 + c3*z3 + c4*z4;
            *q = yq;
            q = q + inc;
          }
        }
        if (nz3) {
          z1 = z2;  z2 = z3;  z3 = z4;
          yq = c1*z1 +c2*z2 + c3*z3 + c4*z4;
          *q = yq;
          q = q + inc;
        }
        while (nz4--) { *q = z4;  q = q + inc; }
      }
    }
     break;

    case LUX_DOUBLE:    // F*8, double precision
    {
      double    z4, z1, z2, z3, yq;
      double    *p, *q;
      for (k=0;k<ny;k++) {
        p = base.d + k*dline + rflag;
        q = out.d + k*dline + rflag;
        z1 = *(p + i1);         z2 = *(p + i2);         z3 = *(p + i3);
        z4 = *(p + i4);
        p = p + i4;
        nz2 = nzone2;  nz3 = nzone3;  nz4 = nzone4;
        
        if (nz2) {
          yq = cd1*z1 +cd2*z2 + cd3*z3 + cd4*z4;
          *q = yq;
          q = q + inc;
          nz2--;
          while (nz2--) {
            p = p + inc;
            z1 = z2;  z2 = z3;  z3 = z4;
            z4 = *p;
            yq = cd1*z1 +cd2*z2 + cd3*z3 + cd4*z4;
            *q = yq;
            q = q + inc;
          }
        }
        if (nz3) {
          z1 = z2;  z2 = z3;  z3 = z4;
          yq = cd1*z1 +cd2*z2 + cd3*z3 + cd4*z4;
          *q = yq;
          q = q + inc;
        }
        while (nz4--) { *q = z4;  q = q + inc; }
      }
    }
     break;
  } return;
}
 //-------------------------------------------------------------------------
int32_t lux_shift3(ArgumentCount narg, Symbol ps[])  // shift3, bicubic image shift
 // y = shift3(x, dx, dy)
 /* the direction of the shift is such that a positive dx moves the image
 to the left (toward smaller x) and likewise in y. Hence the original (dx,dy)
 becomes (0,0). Note that this is the opposite of the direction for rotate3
 when it is used only for a shift */
{
  int32_t       nx, ny, iq, nd, n, nb, result_sym;
  float         dx, dy;
  Pointer       base, out;
  void  shift_bicubic(float, int32_t, int32_t, int32_t, int32_t, Pointer, Pointer, int32_t);

  iq = ps[0];
  if (!symbolIsNumericalArray(iq))
    return cerror(NEED_NUM_ARR, iq);
  regrid_type = array_type(iq);
  base.i32 = (int32_t*) array_data(iq);
                                         // we want a 2-D array only
  nd = array_num_dims(iq);
  if (nd > 2)
    return luxerror("Need 1D or 2D array", iq);
  nx = array_dims(iq)[0];
  if (nd == 2)
    ny = array_dims(iq)[1];
  else
    ny = 1;
  if (nx < 2)
    return luxerror("Need at least 2 elements in 1st dimension", iq);
  n = array_size(iq);
  nb = n*lux_type_size[regrid_type];            // # of bytes
  // get the shifts
  if (float_arg_stat(ps[1], &dx) != LUX_OK)
    return LUX_ERROR;
  dy = 0.0;
  if (narg > 2 && float_arg_stat(ps[2], &dy) != LUX_OK)
    return LUX_ERROR;

  result_sym = array_clone(iq, regrid_type);
  out.i32 = (int32_t*) array_data(result_sym);

  // first the shifts in x
  if (dx == 0.0)                // just copy over the original
    bcopy((char *) base.ui8, (char *) out.ui8, nb);
  else
    shift_bicubic(dx, nx, ny, 1, nx, base, out, regrid_type);
  // if dy is zip, we are done
  if (dy != 0.0)
    shift_bicubic(dy, ny, nx, nx, 1, out, out, regrid_type);

  return result_sym;
}
//-------------------------------------------------------------------------
void interpolate(void *srcv, int32_t type, float xsrc, float ysrc, int32_t nsx,
                 int32_t nsy, void *trgtv)
// does bicubic interpolation.
// <srcv>: pointer to source array
// <type>: data type of source array, either LUX_FLOAT or LUX_DOUBLE
// <xsrc>, <ysrc>: coordinates at which interpolation is to be performed
// <nsx>, <nsy>: dimensions of the source array
// <trgtv>: pointer to the location where the interpolated value is to be
//         stored, in data type <type>
// Interpolation at integer coordinates yields one exact value from the
// source array.
// LS 4 August 2000
{
  Pointer       src, trgt;
  int32_t       ix, iy, i;
  float         px1, px2, px3, px4, py1, py2, py3, py4, bx, by, ax, ay;

  src.v = srcv;
  trgt.v = trgtv;
  ix = (int32_t) xsrc;
  iy = (int32_t) ysrc;
  if (ix < 1)
    ix = 1;
  else if (ix > nsx - 3)
    ix = nsx - 3;
  if (iy < 1)
    iy = 1;
  else if (iy > nsy - 3)
    iy = nsy - 3;
  bx = xsrc - ix;
  by = ysrc - iy;
  ax = 1 - bx;
  ay = 1 - by;
  px1 = -0.5*bx*ax*ax;
  px2 = 1 + bx*bx*(-2.5 + 1.5*bx);
  px3 = 1 + ax*ax*(-2.5 + 1.5*ax);
  px4 = -0.5*ax*bx*bx;
  py1 = -0.5*by*ay*ay;
  py2 = 1 + by*by*(-2.5 + 1.5*by);
  py3 = 1 + ay*ay*(-2.5 + 1.5*ay);
  py4 = -0.5*ay*by*by;
  i = ix + iy*nsx;
  switch (type) {
    case LUX_INT8:
      *trgt.ui8 = py1*(px1*src.ui8[i - nsx - 1] + px2*src.ui8[i - nsx]
                     + px3*src.ui8[i - nsx + 1] + px4*src.ui8[i - nsx + 2])
        + py2*(px1*src.ui8[i - 1] + px2*src.ui8[i] + px3*src.ui8[i + 1]
               + px4*src.ui8[i + 2])
        + py3*(px1*src.ui8[i + nsx - 1] + px2*src.ui8[i + nsx]
               + px3*src.ui8[i + nsx + 1] + px4*src.ui8[i + nsx + 2])
        + py4*(px1*src.ui8[i + 2*nsx - 1] + px2*src.ui8[i + 2*nsx]
               + px3*src.ui8[i + 2*nsx + 1] + px4*src.ui8[i + 2*nsx + 2]);
      break;
    case LUX_INT16:
      *trgt.i16 = py1*(px1*src.i16[i - nsx - 1] + px2*src.i16[i - nsx]
                     + px3*src.i16[i - nsx + 1] + px4*src.i16[i - nsx + 2])
        + py2*(px1*src.i16[i - 1] + px2*src.i16[i] + px3*src.i16[i + 1]
               + px4*src.i16[i + 2])
        + py3*(px1*src.i16[i + nsx - 1] + px2*src.i16[i + nsx]
               + px3*src.i16[i + nsx + 1] + px4*src.i16[i + nsx + 2])
        + py4*(px1*src.i16[i + 2*nsx - 1] + px2*src.i16[i + 2*nsx]
               + px3*src.i16[i + 2*nsx + 1] + px4*src.i16[i + 2*nsx + 2]);
      break;
    case LUX_INT32:
      *trgt.i32 = py1*(px1*src.i32[i - nsx - 1] + px2*src.i32[i - nsx]
                     + px3*src.i32[i - nsx + 1] + px4*src.i32[i - nsx + 2])
        + py2*(px1*src.i32[i - 1] + px2*src.i32[i] + px3*src.i32[i + 1]
               + px4*src.i32[i + 2])
        + py3*(px1*src.i32[i + nsx - 1] + px2*src.i32[i + nsx]
               + px3*src.i32[i + nsx + 1] + px4*src.i32[i + nsx + 2])
        + py4*(px1*src.i32[i + 2*nsx - 1] + px2*src.i32[i + 2*nsx]
               + px3*src.i32[i + 2*nsx + 1] + px4*src.i32[i + 2*nsx + 2]);
      break;
    case LUX_INT64:
      *trgt.i64 = py1*(px1*src.i64[i - nsx - 1] + px2*src.i64[i - nsx]
                     + px3*src.i64[i - nsx + 1] + px4*src.i64[i - nsx + 2])
        + py2*(px1*src.i64[i - 1] + px2*src.i64[i] + px3*src.i64[i + 1]
               + px4*src.i64[i + 2])
        + py3*(px1*src.i64[i + nsx - 1] + px2*src.i64[i + nsx]
               + px3*src.i64[i + nsx + 1] + px4*src.i64[i + nsx + 2])
        + py4*(px1*src.i64[i + 2*nsx - 1] + px2*src.i64[i + 2*nsx]
               + px3*src.i64[i + 2*nsx + 1] + px4*src.i64[i + 2*nsx + 2]);
      break;
    case LUX_FLOAT:
      *trgt.f = py1*(px1*src.f[i - nsx - 1] + px2*src.f[i - nsx]
                     + px3*src.f[i - nsx + 1] + px4*src.f[i - nsx + 2])
        + py2*(px1*src.f[i - 1] + px2*src.f[i] + px3*src.f[i + 1]
               + px4*src.f[i + 2])
        + py3*(px1*src.f[i + nsx - 1] + px2*src.f[i + nsx]
               + px3*src.f[i + nsx + 1] + px4*src.f[i + nsx + 2])
        + py4*(px1*src.f[i + 2*nsx - 1] + px2*src.f[i + 2*nsx]
               + px3*src.f[i + 2*nsx + 1] + px4*src.f[i + 2*nsx + 2]);
      break;
    case LUX_DOUBLE:
      *trgt.d = py1*(px1*src.d[i - nsx - 1] + px2*src.d[i - nsx]
                     + px3*src.d[i - nsx + 1] + px4*src.d[i - nsx + 2])
        + py2*(px1*src.d[i - 1] + px2*src.d[i] + px3*src.d[i + 1]
               + px4*src.d[i + 2])
        + py3*(px1*src.d[i + nsx - 1] + px2*src.d[i + nsx]
               + px3*src.d[i + nsx + 1] + px4*src.d[i + nsx + 2])
        + py4*(px1*src.d[i + 2*nsx - 1] + px2*src.d[i + 2*nsx]
               + px3*src.d[i + 2*nsx + 1] + px4*src.d[i + 2*nsx + 2]);
      break;
  } // end of switch (type)
  return;
}
//-------------------------------------------------------------------------
int32_t lux_regridls(ArgumentCount narg, Symbol ps[])
// REGRIDLS(<data>,<gx>,<gy>,<nx>,<ny>)
{
  int32_t       nx, ny, result, dims[2], ngx, ngy, gx, gy, s, t, nsx, nsy,
    step;
  Symboltype type;
  float         xsrc, ysrc, *gridx, *gridy, sx, sy, tx, ty, stx, sty, xsrc0, ysrc0;
  Pointer       src, trgt;

  // <data>
  if (!symbolIsRealArray(ps[0])
      || array_num_dims(ps[0]) != 2)
    return cerror(NEED_2D_ARR, ps[0]);

  // <gx>
  if (!symbolIsRealArray(ps[1])
      || array_num_dims(ps[1]) != 2)
    return cerror(NEED_2D_ARR, ps[1]);

  // <gy>
  if (!symbolIsRealArray(ps[2])
      || array_num_dims(ps[2]) != 2
      || array_dims(ps[2])[0] != array_dims(ps[1])[0]
      || array_dims(ps[2])[1] != array_dims(ps[2])[1])
    return cerror(INCMP_ARG, ps[2]);

  // <nx>
  if (!symbolIsRealScalar(ps[3]))
    return cerror(NEED_REAL_SCAL, ps[3]);

  // <ny>
  if (!symbolIsRealScalar(ps[4]))
    return cerror(NEED_REAL_SCAL, ps[4]);

  // get pointers and info
  src.v = array_data(ps[0]);
  type = array_type(ps[0]);
  gridx = (float*) array_data(lux_float(1, &ps[1]));
  gridy = (float*) array_data(lux_float(1, &ps[2]));
  nx = int_arg(ps[3]);
  ny = int_arg(ps[4]);
  ngx = array_dims(ps[2])[0];
  ngy = array_dims(ps[2])[1];
  nsx = array_dims(ps[0])[0];
  nsy = array_dims(ps[0])[1];
  if (nx < 1)
    return cerror(NEED_POS_ARG, ps[3]);
  if (ny < 1)
    return cerror(NEED_POS_ARG, ps[4]);
  step = lux_type_size[type];

  // generate output symbol
  dims[0] = nx*(ngx - 1);
  dims[1] = ny*(ngy - 1);
  result = array_scratch(type, 2, dims);
  if (result == LUX_ERROR)
    return LUX_ERROR;
  trgt.v = array_data(result);

  // Theory: the user provides a grid of control points <gx>,<gy>
  // that identify locations in the source coordinate system.  The
  // control points define cells in the source image.  The user also
  // provides the dimensions <nx>, <ny> of a grid of interior points
  // that is to be interpolated nicely in each cell.  The source image
  // is interpolated at each of these interior points, and the resulting
  // value goes to the output.

  // We use linear interpolation to find the interior points, and then
  // bicubic interpolation to get the data values.  We find the locations
  // of the interior points as follows:
  // Suppose that the corner points of an interior cell are given by
  // the vectors <v0>, <v1>, <v2>, and <v3>.  We look for a mapping
  // of coordinates (s,t) to source image coordinate vector <x>, with
  // <s> ranging between 0 and <nx> - 1, and <t> between 0 and <ny> - 1,
  // such that (0,0) corresponds to <v0>, (nx,0) to <v1>, (0,ny) to <v2>,
  // and (nx,ny) to <v3>, and the mapping should be linear in <s> and in
  // <t>.  The following mapping satisfies these requirements:
  // x = (v1 - v0)*s/nx + (v2 - v0)*t/ny + (v3 - v2 - v1 + v0)*s*t/(nx*ny)

  for (gy = 0; gy < ngy - 1; gy++) { // all grid cell rows
    for (gx = 0; gx < ngx - 1; gx++) { // all grid cells in each row
      // for each grid cell we calculate <nx> by <ny> interior points
      sx = (gridx[1] - gridx[0])/nx;
      sy = (gridy[1] - gridy[0])/nx;
      tx = (gridx[ngx] - gridx[0])/ny;
      ty = (gridy[ngx] - gridy[0])/ny;
      stx = (gridx[ngx + 1] - gridx[ngx] - gridx[1] + gridx[0])/(nx*ny);
      sty = (gridy[ngx + 1] - gridy[ngx] - gridy[1] + gridy[0])/(nx*ny);
      for (t = 0; t < ny; t++) {
        xsrc0 = tx*t + gridx[0];
        ysrc0 = ty*t + gridy[0];
        for (s = 0; s < nx; s++) {
          xsrc = xsrc0 + s*(sx + stx*t);
          ysrc = ysrc0 + s*(sy + sty*t);
          // now that we've found the source image location, we must
          // interpolate the image value there.
          interpolate(src.v, type, xsrc, ysrc, nsx, nsy, trgt.v);
          trgt.ui8 += step;
        } // end of for (s = 0; s < nx; s++)
      }         // end of for (t = 0; t < ny; t++)
      gridx++;
      gridy++;
    } // end of for (gx = 0; gx < ngx; gx++)
  } // end of for (gy = 0; gy < ngy; gy++)
  return result;
}
//-------------------------------------------------------------------------
int32_t bigger235(int32_t x)
// returns a number not less than <x> that can be expressed as
// the product of integer powers of 2, 3, and 5 only.  For <x> less than
// or equal to 6480, the result is guaranteed to be the smallest number
// meeting the criteria.  For greater <x>, the returned value may not be
// the smallest possible one.  LS 2 August 2000
{

  static int32_t table[] = {
    1, 2, 3, 4, 5, 6, 8, 9, 10, 12, 15, 16, 18, 20, 24, 25, 27, 30,
    32, 36, 40, 45, 48, 50, 54, 60, 64, 72, 75, 80, 81, 90, 96, 100,
    108, 120, 125, 128, 135, 144, 150, 160, 162, 180, 192, 200, 216,
    225, 240, 243, 250, 256, 270, 288, 300, 320, 324, 360, 375, 384,
    400, 405, 432, 450, 480, 486, 500, 512, 540, 576, 600, 625, 640,
    648, 675, 720, 729, 750, 768, 800, 810, 864, 900, 960, 972, 1000,
    1024, 1080, 1125, 1152, 1200, 1215, 1250, 1280, 1296, 1350, 1440,
    1458, 1500, 1536, 1600, 1620, 1728, 1800, 1875, 1920, 1944, 2000,
    2025, 2048, 2160, 2187, 2250, 2304, 2400, 2430, 2500, 2560, 2592,
    2700, 2880, 2916, 3000, 3072, 3125, 3200, 3240, 3375, 3456, 3600,
    3645, 3750, 3840, 3888, 4000, 4050, 4096, 4320, 4374, 4500, 4608,
    4800, 4860, 5000, 5120, 5184, 5400, 5625, 5760, 5832, 6000, 6075,
    6144, 6250, 6400, 6480
  }, n = sizeof(table)/sizeof(int32_t);
  int32_t       ilo, ihi, imid;
  int32_t       fac = 1, fac2, x2;

  if (x < 1)
    return 1;

  if (x > 6480) {
    while (x % 2 == 0) {
      fac *= 2;
      x /= 2;
    }
    while (x % 3 == 0) {
      fac *= 3;
      x /= 3;
    }
    while (x % 5 == 0) {
      fac *= 5;
      x /= 5;
    }
    // now all factors 2, 3, and 5 have been transferred from <x> to <fac>.
    if (x > 6480) {
      do {
        x2 = ++x;
        fac2 = 1;
        while (x2 % 2 == 0) {
          fac2 *= 2;
          x2 /= 2;
        }
        while (x2 % 3 == 0) {
          fac2 *= 3;
          x2 /= 3;
        }
        while (x2 % 5 == 0) {
          fac2 *= 5;
          x2 /= 5;
        }
      } while (x2 > 6480);
      fac *= fac2;
      x = x2;
    }
  }

  ilo = 0;
  ihi = n - 1;
  while (ilo <= ihi) {
    imid = (ilo + ihi)/2;
    if (x > table[imid])
      ilo = imid + 1;
    else if (x < table[imid])
      ihi = imid - 1;
    else                        // found in table
      return x*fac;
  }

  x = table[ilo]*fac;

  return x;
}
//-------------------------------------------------------------------------
int32_t lux_bigger235(ArgumentCount narg, Symbol ps[])
{
  int32_t       iq, *src, *trgt, n, result;

  switch (symbol_class(ps[0])) {
    case LUX_ARRAY:
      iq = lux_long(1, ps);
      src = (int32_t*) array_data(iq);
      n = array_size(iq);
      result = array_clone(iq, LUX_INT32);
      trgt = (int32_t*) array_data(result);
      break;
    case LUX_SCALAR:
      iq = lux_long(1, ps);
      src = &scalar_value(iq).i32;
      n = 1;
      result = scalar_scratch(LUX_INT32);
      trgt = &scalar_value(result).i32;
      break;
    default:
      return cerror(ILL_CLASS, ps[0]);
  }

  while (n--)
    *trgt++ = bigger235(*src++);

  return result;
}
//-------------------------------------------------------------------------
#if HAVE_LIBGSL
int32_t single_fft(Pointer data, int32_t n, int32_t type, int32_t back)
// type = LUX_FLOAT or LUX_DOUBLE
{
  int32_t gsl_fft(double *, size_t, size_t);
  int32_t gsl_fft_back(double *, size_t, size_t);
  double *ddata;
  if (type == LUX_DOUBLE)
    ddata = data.d;
  else
    ddata = (double*) malloc(n*sizeof(double));
  if (back)
    gsl_fft_back(ddata, n, 1);
  else
    gsl_fft(ddata, n, 1);
  if (type != LUX_DOUBLE) {
    int32_t i;
    for (i = 0; i < n; i++)
      data.f[i] = ddata[i];
    free(ddata);
  }
  return 0;
}
#endif
//-------------------------------------------------------------------------
int32_t lux_cartesian_to_polar(ArgumentCount narg, Symbol ps[])
// y = CTOP(x [, x0, y0])
{
#if HAVE_LIBGSL
  int32_t       nx, ny, result, dims[2], r, rmax, n, az, step, i;
  Symboltype type;
  float         x0, y0, x, y, daz;
  Pointer       src, trgt;

  // <x>
  if (!symbolIsNumericalArray(ps[0]) || array_num_dims(ps[0]) != 2)
    return cerror(NEED_2D_ARR, ps[0]);
  nx = array_dims(ps[0])[0];
  ny = array_dims(ps[0])[1];
  src.v = array_data(ps[0]);
  type = array_type(ps[0]);
  step = lux_type_size[type];

  if (type != LUX_FLOAT && type != LUX_DOUBLE)
    return luxerror("CTOP() works only on FLOAT or DOUBLE arrays", ps[0]);

  x0 = (narg > 1 && ps[1])? float_arg(ps[1]): (nx - 1)*0.5; // <x0>
  y0 = (narg > 2 && ps[2])? float_arg(ps[2]): (ny - 1)*0.5; // <y0>

  // we determine for which values of the radius we have complete coverage
  rmax = (x0 >= 0 && x0 < nx && y0 >= 0 && y0 < ny)?
    floor(MIN(MIN(MIN(x0, nx - x0), y0), ny - y0)): 0;

  // how many complete circles do we have?
  if (!rmax)
    return luxerror("No circles around (%g,%g) fall completely within the image",
                 ps[0], x0, y0);

  dims[0] = bigger235((int32_t) ceil(rmax*TWOPI));      // #pts for biggest circle
  dims[1] = rmax + 1;
  result = array_scratch(type, 2, dims);
  if (result == LUX_ERROR)
    return LUX_ERROR;
  trgt.v = array_data(result);

  for (r = 0; r <= rmax; r++) {
    n = bigger235((int32_t) ceil(r*TWOPI));
    daz = TWOPI/n;
    for (az = 0; az < n; az++) {
      x = x0 + r*cos(az*daz);
      y = y0 + r*sin(az*daz);
      interpolate(src.v, type, x, y, nx, ny, trgt.v);
      trgt.ui8 += step;
    }
    trgt.ui8 -= n*step;
    single_fft(trgt, n, type, 0);
    switch (type) {
      case LUX_FLOAT:
        for (i = 0; i < n; i++)
          trgt.f[i] /= n;
        break;
      case LUX_DOUBLE:
        for (i = 0; i < n; i++)
          trgt.d[i] /= n;
        break;
    }
    trgt.ui8 += n*step;
    if (n < dims[0]) {
      zerobytes(trgt.ui8, (dims[0] - n)*step);
      trgt.ui8 += (dims[0] - n)*step;
    }
    trgt.ui8 -= dims[0]*step;
    single_fft(trgt, dims[0], type, 1);
    trgt.ui8 += dims[0]*step;
  }
  return result;
#else
  return cerror(NOSUPPORT, 0, "CTOP", "libgsl");
#endif
}
//-------------------------------------------------------------------------
int32_t lux_polar_to_cartesian(ArgumentCount narg, Symbol ps[])
// y = PTOC(x [, nx, ny, x0, y0])
{
#if HAVE_LIBGSL
  int32_t       nx, ny, result, dims[2], step, ix, iy;
  Symboltype type;
  float         x0, y0, daz, dx, dy, az, r;
  Pointer       src, trgt;
  int32_t       single_fft(Pointer src, int32_t n, int32_t type, int32_t backwards);

  // <x>
  if (!symbolIsNumericalArray(ps[0]) || array_num_dims(ps[0]) != 2)
    return cerror(NEED_2D_ARR, ps[0]);
  src.v = array_data(ps[0]);
  type = array_type(ps[0]);
  step = lux_type_size[type];
  nx = array_dims(ps[0])[0];
  ny = array_dims(ps[0])[1];

  if (type != LUX_FLOAT && type != LUX_DOUBLE)
    return luxerror("PTOC() works only on FLOAT or DOUBLE arrays", ps[0]);

  dims[0] = (narg > 1 && ps[1])? int_arg(ps[1]): 2*(ny - 1);
  dims[1] = (narg > 2 && ps[2])? int_arg(ps[2]): 2*(ny - 1);
  x0 = (narg > 3 && ps[3])? float_arg(ps[3]): ny - 1.5; // <x0>
  y0 = (narg > 4 && ps[4])? float_arg(ps[4]): ny - 1.5; // <y0>

  result = array_scratch(type, 2, dims);
  if (result == LUX_ERROR)
    return LUX_ERROR;
  trgt.v = array_data(result);

  daz = nx/TWOPI;
  for (iy = 0; iy < dims[1]; iy++) {
    for (ix = 0; ix < dims[0]; ix++) {
      dx = ix - x0;
      dy = iy - y0;
      r = hypot(dx,dy);
      az = ((int32_t) floor(atan2(dy,dx)*daz + 0.5) + nx) % nx;
      if (r < ny)
        interpolate(src.v, type, az, r, nx, ny, trgt.v);
      else
        zerobytes(trgt.v, step);
      trgt.ui8 += step;
    }
  }
  return result;
#else
  return cerror(NOSUPPORT, 0, "PTOC", "libgsl");
#endif
}
//-------------------------------------------------------------------------
