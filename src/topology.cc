/* This is file topology.cc.

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
// File topology.c
// LUX routine for identifying and labeling disjoint areas in a bitmap.
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include "action.hh"

//------------------------------------------------------------------
#define SEEK_MAXIMUM    (LUX_DOUBLE + 1)
int32_t segment_2d(int32_t narg, int32_t ps[])
/* does image segmentation on two-dimensional arrays.
   it is assumed that's what <x> is!
   Y = SEGMENT(x [, sign]) */
{
  LoopInfo      srcinfo, trgtinfo;
  Pointer       src, trgt;
  Scalar        value;
  int32_t       nx, ny, n, result, sign;

  if (standardLoop(ps[0], 0, SL_ALLAXES | SL_SAMEDIMS | SL_EXACT, LUX_INT32,
                   &srcinfo, &src, &result, &trgtinfo, &trgt) == LUX_ERROR)
    return LUX_ERROR;

  sign = (narg > 1 && ps[1])? int_arg(ps[1]): 1;

  nx = srcinfo.dims[0];
  ny = srcinfo.dims[1];

  // top row: always zero
  zerobytes(trgt.l, nx*sizeof(int32_t));
  trgt.l += nx;
  src.ui8 += nx*lux_type_size[array_type(ps[0])];
  ny -= 2;
  if (sign >= 0) {              // seek hill-like objects
    switch (array_type(ps[0])) {
      case LUX_INT8:
        while (ny--) {          // all rows except for bottom one
          *trgt.l++ = 0;
          src.ui8++;
          n = nx - 2;
          while (n--) {
            value.i16 = *src.ui8 * 2;
            *trgt.l++ = ((int16_t) src.ui8[-1] + (int16_t) src.ui8[1] < value.i16
                         && (int16_t) src.ui8[nx] + (int16_t) src.ui8[-nx] < value.i16
                         && (int16_t) src.ui8[1 + nx] + (int16_t) src.ui8[-1 - nx]
                         < value.i16
                         && (int16_t) src.ui8[1 - nx] + (int16_t) src.ui8[-1 + nx]
                         < value.i16);
            src.ui8++;
          }
          *trgt.l++ = 0;
          src.ui8++;
        }
        break;
      case LUX_INT16:
        while (ny--) {          // all rows except for bottom one
          *trgt.l++ = 0;
          src.i16++;
          n = nx - 2;
          while (n--) {
            value.l = (int32_t) *src.i16 * 2;
            *trgt.l++ = ((int32_t) src.i16[-1] + (int32_t) src.i16[1] < value.l
                         && (int32_t) src.i16[nx] + (int32_t) src.i16[-nx] < value.l
                         && (int32_t) src.i16[1 + nx] + (int32_t) src.i16[-1 - nx]
                            < value.l
                         && (int32_t) src.i16[1 - nx] + (int32_t) src.i16[-1 + nx]
                            < value.l);
            src.i16++;
          }
          *trgt.l++ = 0;
          src.i16++;
        }
        break;
      case LUX_INT32:
        while (ny--) {          // all rows except for bottom one
          *trgt.l++ = 0;
          src.l++;
          n = nx - 2;
          while (n--) {
            value.l = *src.l * 2;
            *trgt.l++ = (src.l[-1] + src.l[1] < value.l
                         && src.l[nx] + src.l[-nx] < value.l
                         && src.l[1 + nx] + src.l[-1 - nx] < value.l
                         && src.l[1 - nx] + src.l[-1 + nx] < value.l);
            src.l++;
          }
          *trgt.l++ = 0;
          src.l++;
        }
        break;
      case LUX_INT64:
        while (ny--) {          // all rows except for bottom one
          *trgt.l++ = 0;
          src.q++;
          n = nx - 2;
          while (n--) {
            value.q = *src.q * 2;
            *trgt.l++ = (src.q[-1] + src.q[1] < value.q
                         && src.q[nx] + src.q[-nx] < value.q
                         && src.q[1 + nx] + src.q[-1 - nx] < value.q
                         && src.q[1 - nx] + src.q[-1 + nx] < value.q);
            src.q++;
          }
          *trgt.l++ = 0;
          src.q++;
        }
        break;
      case LUX_FLOAT:
        while (ny--) {          // all rows except for bottom one
          *trgt.l++ = 0;
          src.f++;
          n = nx - 2;
          while (n--) {
            value.f = *src.f * 2;
            *trgt.l++ = (src.f[-1] + src.f[1] < value.f
                         && src.f[nx] + src.f[-nx] < value.f
                         && src.f[1 + nx] + src.f[-1 - nx] < value.f
                         && src.f[1 - nx] + src.f[-1 + nx] < value.f);
            src.f++;
          }
          *trgt.l++ = 0;
          src.f++;
        }
        break;
      case LUX_DOUBLE:
        while (ny--) {          // all rows except for bottom one
          *trgt.l++ = 0;
          src.d++;
          n = nx - 2;
          while (n--) {
            value.d = *src.d * 2;
            *trgt.l++ = (src.d[-1] + src.d[1] < value.d
                         && src.d[nx] + src.d[-nx] < value.d
                         && src.d[1 + nx] + src.d[-1 - nx] < value.d
                         && src.d[1 - nx] + src.d[-1 + nx] < value.d);
            src.d++;
          }
          *trgt.l++ = 0;
          src.d++;
        }
        break;
    }
  } else {                      // seek valley-like objects
    switch (array_type(ps[0])) {
      case LUX_INT8:
        while (ny--) {          // all rows except for bottom one
          *trgt.l++ = 0;
          src.ui8++;
          n = nx - 2;
          while (n--) {
            value.ui8 = *src.ui8 * (short) 2;
            *trgt.l++ = (src.ui8[-1] + src.ui8[1] > value.ui8
                         && src.ui8[nx] + src.ui8[-nx] > value.ui8
                         && src.ui8[1 + nx] + src.ui8[-1 - nx] > value.ui8
                         && src.ui8[1 - nx] + src.ui8[-1 + nx] > value.ui8);
            src.ui8++;
          }
          *trgt.l++ = 0;
          src.ui8++;
        }
        break;
      case LUX_INT16:
        while (ny--) {          // all rows except for bottom one
          *trgt.l++ = 0;
          src.i16++;
          n = nx - 2;
          while (n--) {
            value.i16 = *src.i16 * 2;
            *trgt.l++ = (src.i16[-1] + src.i16[1] > value.i16
                         && src.i16[nx] + src.i16[-nx] > value.i16
                         && src.i16[1 + nx] + src.i16[-1 - nx] > value.i16
                         && src.i16[1 - nx] + src.i16[-1 + nx] > value.i16);
            src.i16++;
          }
          *trgt.l++ = 0;
          src.i16++;
        }
        break;
      case LUX_INT32:
        while (ny--) {          // all rows except for bottom one
          *trgt.l++ = 0;
          src.l++;
          n = nx - 2;
          while (n--) {
            value.l = *src.l * 2;
            *trgt.l++ = (src.l[-1] + src.l[1] > value.l
                         && src.l[nx] + src.l[-nx] > value.l
                         && src.l[1 + nx] + src.l[-1 - nx] > value.l
                         && src.l[1 - nx] + src.l[-1 + nx] > value.l);
            src.l++;
          }
          *trgt.l++ = 0;
          src.l++;
        }
        break;
      case LUX_INT64:
        while (ny--) {          // all rows except for bottom one
          *trgt.l++ = 0;
          src.q++;
          n = nx - 2;
          while (n--) {
            value.q = *src.q * 2;
            *trgt.l++ = (src.q[-1] + src.q[1] > value.q
                         && src.q[nx] + src.q[-nx] > value.q
                         && src.q[1 + nx] + src.q[-1 - nx] > value.q
                         && src.q[1 - nx] + src.q[-1 + nx] > value.q);
            src.q++;
          }
          *trgt.l++ = 0;
          src.q++;
        }
        break;
      case LUX_FLOAT:
        while (ny--) {          // all rows except for bottom one
          *trgt.l++ = 0;
          src.f++;
          n = nx - 2;
          while (n--) {
            value.f = *src.f * 2;
            *trgt.l++ = (src.f[-1] + src.f[1] > value.f
                         && src.f[nx] + src.f[-nx] > value.f
                         && src.f[1 + nx] + src.f[-1 - nx] > value.f
                         && src.f[1 - nx] + src.f[-1 + nx] > value.f);
            src.f++;
          }
          *trgt.l++ = 0;
          src.f++;
        }
        break;
      case LUX_DOUBLE:
        while (ny--) {          // all rows except for bottom one
          *trgt.l++ = 0;
          src.d++;
          n = nx - 2;
          while (n--) {
            value.d = *src.d * 2;
            *trgt.l++ = (src.d[-1] + src.d[1] > value.d
                         && src.d[nx] + src.d[-nx] > value.d
                         && src.d[1 + nx] + src.d[-1 - nx] > value.d
                         && src.d[1 - nx] + src.d[-1 + nx] > value.d);
            src.d++;
          }
          *trgt.l++ = 0;
          src.d++;
        }
        break;
    }
  }
  zerobytes(trgt.l, nx*sizeof(int32_t));
  return result;
}
//-------------------------------------------------------------------------
int32_t segment_general(int32_t narg, int32_t ps[])
/* Y = SEGMENT(X) curvature-based segmentation in multiple dimensions
   SEGMENT(x [, sign, DIAGONAL=diagonal, /DEGREE])
   <x>: data
   <sign>: sign of objects to look for, positive integer -> hill-like,
    negative integer -> valley-like.  If zero, then +1 is assumed.
   /DEGREE: returns number of OK curvatures per data element
   LS 18may95 4aug97 */
{
  int32_t       result, sign, degree, n, i, *offset, k, j, ok, *edge, nok;
  Scalar        value;
  Pointer       src, trgt, srcl, srcr;
  LoopInfo      srcinfo, trgtinfo;

  // gather info about ps[0] and prepare a return symbol
  if (standardLoop(ps[0], 0, SL_ALLAXES | SL_SAMEDIMS | SL_EXACT,
                   LUX_INT32, &srcinfo, &src, &result, &trgtinfo, &trgt) < 0)
    return LUX_ERROR;
  if (symbol_class(ps[0]) != LUX_ARRAY)
    return cerror(ILL_CLASS, ps[0]);

  sign = (narg > 1 && ps[1])? int_arg(ps[1]): 1;
  sign = (sign >= 0)? 1: 0;     // 1 -> hill-like, 0 -> valley-like
  degree = (internalMode & 1);

  // treat DIAGONAL
  n = prepareDiagonals(narg > 2? ps[2]: 0, &srcinfo, 2, &offset, &edge, NULL,
                       NULL);
  if (n == LUX_ERROR)
    return LUX_ERROR;

  // set the edges to zero
  for (i = 0; i < 2*srcinfo.ndim; i++) {
    if (edge[i]) {
      trgtinfo.rearrangeEdgeLoop(NULL, i);
      do
        *trgt.l = 0;
      while (trgtinfo.advanceLoop(&trgt) < trgtinfo.ndim - 1);
    }
  }

  // set up the range of coordinates that is to be checked
  for (i = 0; i < srcinfo.ndim; i++)
    edge[2*i + 1] = srcinfo.dims[i] - 1 - edge[2*i + 1];
  srcinfo.subdataLoop(edge);
  trgtinfo.subdataLoop(edge);

  free(edge);                   // clean up

  // now do the loop work
  if (sign && !degree && (internalMode & 2) == 0)
                // standard form - make as fast as possible
    switch (array_type(ps[0])) {
      case LUX_INT8:
        do {
          value.i16 = 2 * (int16_t) *src.ui8;
          for (j = 0; j < n; j++) {     /* all directions */
            k = offset[j];
            srcl.ui8 = src.ui8 + k;
            srcr.ui8 = src.ui8 - k;
            if (value.i16 <= (int16_t) *srcl.ui8 + (int16_t) *srcr.ui8)
              break;
          }
          *trgt.l = (j == n);
        } while (trgtinfo.advanceLoop(&trgt),
                 srcinfo.advanceLoop(&src) < srcinfo.ndim);
        break;
      case LUX_INT16:
        do {
          value.l = 2 * *src.i16;
          for (j = 0; j < n; j++) {     // all directions
            k = offset[j];
            srcl.i16 = src.i16 + k;
            srcr.i16 = src.i16 - k;
            if (value.l <= (int32_t) *srcl.i16 + (int32_t) *srcr.i16)
              break;
          }
          *trgt.l = (j == n);
        } while (trgtinfo.advanceLoop(&trgt),
                 srcinfo.advanceLoop(&src) < srcinfo.ndim);
        break;
      case LUX_INT32:
        do {
          value.l = 2 * *src.l;
          for (j = 0; j < n; j++) {     // all directions
            k = offset[j];
            srcl.l = src.l + k;
            srcr.l = src.l - k;
            if (value.l <= *srcl.l + *srcr.l)
              break;
          }
          *trgt.l = (j == n);
        } while (trgtinfo.advanceLoop(&trgt),
                 srcinfo.advanceLoop(&src) < srcinfo.ndim);
        break;
      case LUX_INT64:
        do {
          value.q = 2 * *src.q;
          for (j = 0; j < n; j++) {     // all directions
            k = offset[j];
            srcl.q = src.q + k;
            srcr.q = src.q - k;
            if (value.q <= *srcl.q + *srcr.q)
              break;
          }
          *trgt.l = (j == n);
        } while (trgtinfo.advanceLoop(&trgt),
                 srcinfo.advanceLoop(&src) < srcinfo.ndim);
        break;
      case LUX_FLOAT:
        do {
          value.f = 2 * *src.f;
          for (j = 0; j < n; j++) {     // all directions
            k = offset[j];
            srcl.f = src.f + k;
            srcr.f = src.f - k;
            if (value.f <= *srcl.f + *srcr.f)
              break;
          }
          *trgt.l = (j == n);
        } while (trgtinfo.advanceLoop(&trgt),
                 srcinfo.advanceLoop(&src) < srcinfo.ndim);
        break;
      case LUX_DOUBLE:
        do {
          value.d = 2 * *src.d;
          for (j = 0; j < n; j++) {     // all directions
            k = offset[j];
            srcl.d = src.d + k;
            srcr.d = src.d - k;
            if (value.d <= *srcl.d + *srcr.d)
              break;
          }
          *trgt.l = (j == n);
        } while (trgtinfo.advanceLoop(&trgt),
                 srcinfo.advanceLoop(&src) < srcinfo.ndim);
        break;
    } else                              // general case - a bit slower
      switch (array_type(ps[0])) {
        case LUX_INT8:
          do {
            nok = 1 - degree;
            value.ui8 = 2 * *src.ui8;
            for (j = 0; j < n; j++) { // all directions
              k = offset[j];
              srcl.ui8 = src.ui8 + k;
              srcr.ui8 = src.ui8 - k;
              ok = (sign && value.ui8 > *srcl.ui8 + *srcr.ui8)
                || (!sign && value.ui8 < *srcl.ui8 + *srcr.ui8);
              if (degree)
                nok += ok;
              else if (!ok) {
                nok = 0;
                break;
              }
            }
            *trgt.l = nok;
          } while (trgtinfo.advanceLoop(&trgt),
                   srcinfo.advanceLoop(&src) < srcinfo.ndim);
          break;
        case LUX_INT16:
          do {
            nok = 1 - degree;
            value.i16 = 2 * *src.i16;
            for (j = 0; j < n; j++) { // all directions
              k = offset[j];
              srcl.i16 = src.i16 + k;
              srcr.i16 = src.i16 - k;
              ok = (sign && value.i16 > *srcl.i16 + *srcr.i16)
                || (!sign && value.i16 < *srcl.i16 + *srcr.i16);
              if (degree)
                nok += ok;
              else if (!ok) {
                nok = 0;
                break;
              }
            }
            *trgt.l = nok;
          } while (trgtinfo.advanceLoop(&trgt),
                   srcinfo.advanceLoop(&src) < srcinfo.ndim);
          break;
        case LUX_INT32:
          do {
            nok = 1 - degree;
            value.l = 2 * *src.l;
            for (j = 0; j < n; j++) { // all directions
              k = offset[j];
              srcl.l = src.l + k;
              srcr.l = src.l - k;
              ok = (sign && value.l > *srcl.l + *srcr.l)
                || (!sign && value.l < *srcl.l + *srcr.l);
              if (degree)
                nok += ok;
              else if (!ok) {
                nok = 0;
                break;
              }
            }
            *trgt.l = nok;
          } while (trgtinfo.advanceLoop(&trgt),
                   srcinfo.advanceLoop(&src) < srcinfo.ndim);
          break;
        case LUX_INT64:
          do {
            nok = 1 - degree;
            value.q = 2 * *src.q;
            for (j = 0; j < n; j++) { // all directions
              k = offset[j];
              srcl.q = src.q + k;
              srcr.q = src.q - k;
              ok = (sign && value.q > *srcl.q + *srcr.q)
                || (!sign && value.q < *srcl.q + *srcr.q);
              if (degree)
                nok += ok;
              else if (!ok) {
                nok = 0;
                break;
              }
            }
            *trgt.l = nok;
          } while (trgtinfo.advanceLoop(&trgt),
                   srcinfo.advanceLoop(&src) < srcinfo.ndim);
          break;
        case LUX_FLOAT:
          do {
            nok = 1 - degree;
            value.f = 2 * *src.f;
            for (j = 0; j < n; j++) { // all directions
              k = offset[j];
              srcl.f = src.f + k;
              srcr.f = src.f - k;
              ok = (sign && value.f > *srcl.f + *srcr.f)
                || (!sign && value.f < *srcl.f + *srcr.f);
              if (degree)
                nok += ok;
              else if (!ok) {
                nok = 0;
                break;
              }
            }
            *trgt.l = nok;
          } while (trgtinfo.advanceLoop(&trgt),
                   srcinfo.advanceLoop(&src) < srcinfo.ndim);
          break;
        case LUX_DOUBLE:
          do {
            nok = 1 - degree;
            value.d = 2 * *src.d;
            for (j = 0; j < n; j++) { // all directions
              k = offset[j];
              srcl.d = src.d + k;
              srcr.d = src.d - k;
              ok = (sign && value.d > *srcl.d + *srcr.d)
                || (!sign && value.d < *srcl.d + *srcr.d);
              if (degree)
                nok += ok;
              else if (!ok) {
                nok = 0;
                break;
              }
            }
            *trgt.l = nok;
          } while (trgtinfo.advanceLoop(&trgt),
                   srcinfo.advanceLoop(&src) < srcinfo.ndim);
          break;
      }

  free(offset);
  return result;
}
//-------------------------------------------------------------------------
int32_t lux_segment(int32_t narg, int32_t ps[])
{
  if (narg <= 2 && (internalMode == 0) && symbolIsNumericalArray(ps[0])
      && array_num_dims(ps[0]) == 2)
    return segment_2d(narg, ps); // fast special-purpose routine
  else
    return segment_general(narg, ps); // general routine
}
//-------------------------------------------------------------------------
#define DEG22_5         (M_PI/8)
int32_t lux_segment_dir(int32_t narg, int32_t ps[])
/* y = SEGMENTDIR(<im>, <phi> [,<sign>])
   segmentates image <im> only one-dimensionally in the direction <phi>
   (radians measured counterclockwise from the positive x-axis).
   LS 9nov98 */
{
  LoopInfo      srcinfo, trgtinfo;
  Pointer       src, trgt;
  double        *angle, s, c, a;
  Scalar        value;
  int32_t       nx, ny, n, result, sign, class_id;
  int32_t       off[4];

  if (!symbolIsNumericalArray(ps[0])
      || array_num_dims(ps[0]) != 2)
    return cerror(NEED_2D_ARR, ps[0]);

  if (!symbolIsNumericalArray(ps[1])
      || array_size(ps[1]) != array_size(ps[0]))
    return cerror(INCMP_ARG, ps[1]);

  if (standardLoop(ps[0], 0, SL_ALLAXES | SL_SAMEDIMS | SL_EXACT, LUX_INT32,
                   &srcinfo, &src, &result, &trgtinfo, &trgt) == LUX_ERROR)
    return LUX_ERROR;

  n = lux_double(1, &ps[1]);
  angle = (double*) array_data(n);

  sign = (narg > 1 && ps[2])? int_arg(ps[2]): 1;

  nx = srcinfo.dims[0];
  ny = srcinfo.dims[1];
  off[0] = nx + 1;
  off[1] = 1;
  off[2] = nx - 1;
  off[3] = nx;

  // top row: always zero
  zerobytes(trgt.ui8, nx*sizeof(int32_t));
  trgt.l += nx;
  angle += nx;
  src.ui8 += nx*lux_type_size[array_type(ps[0])];
  ny -= 2;
  if (sign >= 0) {              // seek hill-like objects
    switch (array_type(ps[0])) {
      case LUX_INT8:
        while (ny--) {          // all rows except for bottom one
          *trgt.l++ = 0;
          angle++;
          src.ui8++;
          n = nx - 2;
          while (n--) {
            value.ui8 = *src.ui8 * (short) 2;
            a = *angle++ + DEG22_5;
            s = sin(a);
            c = cos(a);
            class_id = ((s*c > 0) << 1) + (s*c*(2*c*c - 1) > 0);
            *trgt.l++ = (src.ui8[off[class_id]] + src.ui8[-off[class_id]] < value.ui8);
            src.ui8++;
          }
          *trgt.l++ = 0;
          src.ui8++;
          angle++;
        }
        break;
      case LUX_INT16:
        while (ny--) {          // all rows except for bottom one
          *trgt.l++ = 0;
          angle++;
          src.i16++;
          n = nx - 2;
          while (n--) {
            value.i16 = *src.i16 * (short) 2;
            a = *angle++ + DEG22_5;
            s = sin(a);
            c = cos(a);
            class_id = ((s*c > 0) << 1) + (s*c*(2*c*c - 1) > 0);
            *trgt.l++ = (src.i16[off[class_id]] + src.i16[-off[class_id]] < value.i16);
            src.i16++;
          }
          *trgt.l++ = 0;
          src.i16++;
          angle++;
        }
        break;
      case LUX_INT32:
        while (ny--) {          // all rows except for bottom one
          *trgt.l++ = 0;
          angle++;
          src.l++;
          n = nx - 2;
          while (n--) {
            value.l = *src.l * (short) 2;
            a = *angle++ + DEG22_5;
            s = sin(a);
            c = cos(a);
            class_id = ((s*c > 0) << 1) + (s*c*(2*c*c - 1) > 0);
            *trgt.l++ = (src.l[off[class_id]] + src.l[-off[class_id]] < value.l);
            src.l++;
          }
          *trgt.l++ = 0;
          src.l++;
          angle++;
        }
        break;
      case LUX_INT64:
        while (ny--) {          // all rows except for bottom one
          *trgt.l++ = 0;
          angle++;
          src.q++;
          n = nx - 2;
          while (n--) {
            value.q = *src.q * (short) 2;
            a = *angle++ + DEG22_5;
            s = sin(a);
            c = cos(a);
            class_id = ((s*c > 0) << 1) + (s*c*(2*c*c - 1) > 0);
            *trgt.l++ = (src.q[off[class_id]] + src.q[-off[class_id]] < value.q);
            src.q++;
          }
          *trgt.l++ = 0;
          src.q++;
          angle++;
        }
        break;
      case LUX_FLOAT:
        while (ny--) {          // all rows except for bottom one
          *trgt.l++ = 0;
          angle++;
          src.f++;
          n = nx - 2;
          while (n--) {
            value.f = *src.f * (short) 2;
            a = *angle++ + DEG22_5;
            s = sin(a);
            c = cos(a);
            class_id = ((s*c > 0) << 1) + (s*c*(2*c*c - 1) > 0);
            *trgt.l++ = (src.f[off[class_id]] + src.f[-off[class_id]] < value.f);
            src.f++;
          }
          *trgt.l++ = 0;
          src.f++;
          angle++;
        }
        break;
      case LUX_DOUBLE:
        while (ny--) {          // all rows except for bottom one
          *trgt.l++ = 0;
          angle++;
          src.d++;
          n = nx - 2;
          while (n--) {
            value.d = *src.d * (short) 2;
            a = *angle++ + DEG22_5;
            s = sin(a);
            c = cos(a);
            class_id = ((s*c > 0) << 1) + (s*c*(2*c*c - 1) > 0);
            *trgt.l++ = (src.d[off[class_id]] + src.d[-off[class_id]] < value.d);
            src.d++;
          }
          *trgt.l++ = 0;
          src.d++;
          angle++;
        }
        break;
    }
  } else {                      // seek valley-like objects
    switch (array_type(ps[0])) {
      case LUX_INT8:
        while (ny--) {          // all rows except for bottom one
          *trgt.l++ = 0;
          src.ui8++;
          angle++;
          n = nx - 2;
          while (n--) {
            value.ui8 = *src.ui8 * (short) 2;
            a = *angle++ + DEG22_5;
            s = sin(a);
            c = cos(a);
            class_id = ((s*c > 0)<<1) + (s*c*(2*c*c - 1) > 0);
            *trgt.l++ = (src.ui8[off[class_id]] + src.ui8[-off[class_id]] > value.ui8);
            src.ui8++;
          }
          *trgt.l++ = 0;
          angle++;
          src.ui8++;
        }
        break;
      case LUX_INT16:
        while (ny--) {          // all rows except for bottom one
          *trgt.l++ = 0;
          src.i16++;
          angle++;
          n = nx - 2;
          while (n--) {
            value.i16 = *src.i16 * (short) 2;
            a = *angle++ + DEG22_5;
            s = sin(a);
            c = cos(a);
            class_id = ((s*c > 0)<<1) + (s*c*(2*c*c - 1) > 0);
            *trgt.l++ = (src.i16[off[class_id]] + src.i16[-off[class_id]] > value.i16);
            src.i16++;
          }
          *trgt.l++ = 0;
          angle++;
          src.i16++;
        }
        break;
      case LUX_INT32:
        while (ny--) {          // all rows except for bottom one
          *trgt.l++ = 0;
          src.l++;
          angle++;
          n = nx - 2;
          while (n--) {
            value.l = *src.l * (short) 2;
            a = *angle++ + DEG22_5;
            s = sin(a);
            c = cos(a);
            class_id = ((s*c > 0)<<1) + (s*c*(2*c*c - 1) > 0);
            *trgt.l++ = (src.l[off[class_id]] + src.l[-off[class_id]] > value.l);
            src.l++;
          }
          *trgt.l++ = 0;
          angle++;
          src.l++;
        }
        break;
      case LUX_INT64:
        while (ny--) {          // all rows except for bottom one
          *trgt.l++ = 0;
          src.q++;
          angle++;
          n = nx - 2;
          while (n--) {
            value.q = *src.q * (short) 2;
            a = *angle++ + DEG22_5;
            s = sin(a);
            c = cos(a);
            class_id = ((s*c > 0)<<1) + (s*c*(2*c*c - 1) > 0);
            *trgt.l++ = (src.q[off[class_id]] + src.q[-off[class_id]] > value.q);
            src.q++;
          }
          *trgt.l++ = 0;
          angle++;
          src.q++;
        }
        break;
      case LUX_FLOAT:
        while (ny--) {          // all rows except for bottom one
          *trgt.l++ = 0;
          src.f++;
          angle++;
          n = nx - 2;
          while (n--) {
            value.f = *src.f * (short) 2;
            a = *angle++ + DEG22_5;
            s = sin(a);
            c = cos(a);
            class_id = ((s*c > 0)<<1) + (s*c*(2*c*c - 1) > 0);
            *trgt.l++ = (src.f[off[class_id]] + src.f[-off[class_id]] > value.f);
            src.f++;
          }
          *trgt.l++ = 0;
          angle++;
          src.f++;
        }
        break;
      case LUX_DOUBLE:
        while (ny--) {          // all rows except for bottom one
          *trgt.l++ = 0;
          src.d++;
          angle++;
          n = nx - 2;
          while (n--) {
            value.d = *src.d * (short) 2;
            a = *angle++ + DEG22_5;
            s = sin(a);
            c = cos(a);
            class_id = ((s*c > 0)<<1) + (s*c*(2*c*c - 1) > 0);
            *trgt.l++ = (src.d[off[class_id]] + src.d[-off[class_id]] > value.d);
            src.d++;
          }
          *trgt.l++ = 0;
          angle++;
          src.d++;
        }
        break;
    }
  }
  zerobytes(trgt.l, nx*sizeof(int32_t));
  return result;
}
//-------------------------------------------------------------------------
int32_t lux_max_dir(int32_t narg, int32_t ps[])
/* y = MAXDIR(<im>, <phi> [,<sign>])
   returns 1s at the locations of local extremes in image <im> only
   one-dimensionally in the direction <phi> (radians measured
   counterclockwise from the positive x-axis).  LS 9nov98 */
{
  LoopInfo      srcinfo, trgtinfo;
  Pointer       src, trgt;
  double        *angle, s, c, a;
  Scalar        value;
  int32_t       nx, ny, n, result, sign, class_id;
  int32_t       off[4];

  if (!symbolIsNumericalArray(ps[0])
      || array_num_dims(ps[0]) != 2)
    return cerror(NEED_2D_ARR, ps[0]);

  if (!symbolIsNumericalArray(ps[1])
      || array_size(ps[1]) != array_size(ps[0]))
    return cerror(INCMP_ARG, ps[1]);

  if (standardLoop(ps[0], 0, SL_ALLAXES | SL_SAMEDIMS | SL_EXACT, LUX_INT8,
                   &srcinfo, &src, &result, &trgtinfo, &trgt) == LUX_ERROR)
    return LUX_ERROR;

  n = lux_double(1, &ps[1]);
  angle = (double*) array_data(n);

  sign = (narg > 1 && ps[2])? int_arg(ps[2]): 1;

  nx = srcinfo.dims[0];
  ny = srcinfo.dims[1];
  off[0] = nx + 1;
  off[1] = 1;
  off[2] = nx - 1;
  off[3] = nx;

  // top row: always zero
  zerobytes(trgt.ui8, nx);
  trgt.ui8 += nx;
  angle += nx;
  src.ui8 += nx*lux_type_size[array_type(ps[0])];
  ny -= 2;
  if (sign >= 0) {              // seek hill-like objects
    switch (array_type(ps[0])) {
      case LUX_INT8:
        while (ny--) {          // all rows except for bottom one
          *trgt.ui8++ = 0;
          angle++;
          src.ui8++;
          n = nx - 2;
          while (n--) {
            value.ui8 = *src.ui8;
            a = *angle++ + DEG22_5;
            s = sin(a);
            c = cos(a);
            class_id = ((s*c > 0) << 1) + (s*c*(2*c*c - 1) > 0);
            *trgt.ui8++ = (value.ui8 > src.ui8[off[class_id]]
                         && value.ui8 > src.ui8[-off[class_id]]);
            src.ui8++;
          }
          *trgt.ui8++ = 0;
          src.ui8++;
          angle++;
        }
        break;
      case LUX_INT16:
        while (ny--) {          // all rows except for bottom one
          *trgt.ui8++ = 0;
          angle++;
          src.i16++;
          n = nx - 2;
          while (n--) {
            value.i16 = *src.i16;
            a = *angle++ + DEG22_5;
            s = sin(a);
            c = cos(a);
            class_id = ((s*c > 0) << 1) + (s*c*(2*c*c - 1) > 0);
            *trgt.ui8++ = (value.i16 > src.i16[off[class_id]]
                         && value.i16 > src.i16[-off[class_id]]);
            src.i16++;
          }
          *trgt.ui8++ = 0;
          src.i16++;
          angle++;
        }
        break;
      case LUX_INT32:
        while (ny--) {          // all rows except for bottom one
          *trgt.ui8++ = 0;
          angle++;
          src.l++;
          n = nx - 2;
          while (n--) {
            value.l = *src.l;
            a = *angle++ + DEG22_5;
            s = sin(a);
            c = cos(a);
            class_id = ((s*c > 0) << 1) + (s*c*(2*c*c - 1) > 0);
            *trgt.ui8++ = (value.l > src.l[off[class_id]]
                         && value.l > src.l[-off[class_id]]);
            src.l++;
          }
          *trgt.ui8++ = 0;
          src.l++;
          angle++;
        }
        break;
      case LUX_INT64:
        while (ny--) {          // all rows except for bottom one
          *trgt.ui8++ = 0;
          angle++;
          src.q++;
          n = nx - 2;
          while (n--) {
            value.q = *src.q;
            a = *angle++ + DEG22_5;
            s = sin(a);
            c = cos(a);
            class_id = ((s*c > 0) << 1) + (s*c*(2*c*c - 1) > 0);
            *trgt.ui8++ = (value.q > src.q[off[class_id]]
                         && value.q > src.q[-off[class_id]]);
            src.q++;
          }
          *trgt.ui8++ = 0;
          src.q++;
          angle++;
        }
        break;
      case LUX_FLOAT:
        while (ny--) {          // all rows except for bottom one
          *trgt.ui8++ = 0;
          angle++;
          src.f++;
          n = nx - 2;
          while (n--) {
            value.f = *src.f;
            a = *angle++ + DEG22_5;
            s = sin(a);
            c = cos(a);
            class_id = ((s*c > 0) << 1) + (s*c*(2*c*c - 1) > 0);
            *trgt.ui8++ = (value.f > src.f[off[class_id]]
                         && value.f > src.f[-off[class_id]]);
            src.f++;
          }
          *trgt.ui8++ = 0;
          src.f++;
          angle++;
        }
        break;
      case LUX_DOUBLE:
        while (ny--) {          // all rows except for bottom one
          *trgt.ui8++ = 0;
          angle++;
          src.d++;
          n = nx - 2;
          while (n--) {
            value.d = *src.d;
            a = *angle++ + DEG22_5;
            s = sin(a);
            c = cos(a);
            class_id = ((s*c > 0) << 1) + (s*c*(2*c*c - 1) > 0);
            *trgt.ui8++ = (value.d > src.d[off[class_id]]
                         && value.d > src.d[-off[class_id]]);
            src.d++;
          }
          *trgt.ui8++ = 0;
          src.d++;
          angle++;
        }
        break;
    }
  } else {                      // seek valley-like objects
    switch (array_type(ps[0])) {
      case LUX_INT8:
        while (ny--) {          // all rows except for bottom one
          *trgt.ui8++ = 0;
          src.ui8++;
          angle++;
          n = nx - 2;
          while (n--) {
            value.ui8 = *src.ui8;
            a = *angle++ + DEG22_5;
            s = sin(a);
            c = cos(a);
            class_id = ((s*c > 0)<<1) + (s*c*(2*c*c - 1) > 0);
            *trgt.ui8++ = (value.ui8 < src.ui8[off[class_id]]
                         && value.ui8 < src.ui8[-off[class_id]]);
            src.ui8++;
          }
          *trgt.ui8++ = 0;
          angle++;
          src.ui8++;
        }
        break;
      case LUX_INT16:
        while (ny--) {          // all rows except for bottom one
          *trgt.ui8++ = 0;
          src.i16++;
          angle++;
          n = nx - 2;
          while (n--) {
            value.i16 = *src.i16;
            a = *angle++ + DEG22_5;
            s = sin(a);
            c = cos(a);
            class_id = ((s*c > 0)<<1) + (s*c*(2*c*c - 1) > 0);
            *trgt.ui8++ = (value.i16 < src.i16[off[class_id]]
                         && value.i16 < src.i16[-off[class_id]]);
            src.i16++;
          }
          *trgt.ui8++ = 0;
          angle++;
          src.i16++;
        }
        break;
      case LUX_INT32:
        while (ny--) {          // all rows except for bottom one
          *trgt.ui8++ = 0;
          src.l++;
          angle++;
          n = nx - 2;
          while (n--) {
            value.l = *src.l;
            a = *angle++ + DEG22_5;
            s = sin(a);
            c = cos(a);
            class_id = ((s*c > 0)<<1) + (s*c*(2*c*c - 1) > 0);
            *trgt.ui8++ = (value.l < src.l[off[class_id]]
                         && value.l < src.l[-off[class_id]]);
            src.l++;
          }
          *trgt.ui8++ = 0;
          angle++;
          src.l++;
        }
        break;
      case LUX_INT64:
        while (ny--) {          // all rows except for bottom one
          *trgt.ui8++ = 0;
          src.q++;
          angle++;
          n = nx - 2;
          while (n--) {
            value.q = *src.q;
            a = *angle++ + DEG22_5;
            s = sin(a);
            c = cos(a);
            class_id = ((s*c > 0)<<1) + (s*c*(2*c*c - 1) > 0);
            *trgt.ui8++ = (value.q < src.q[off[class_id]]
                         && value.q < src.q[-off[class_id]]);
            src.q++;
          }
          *trgt.ui8++ = 0;
          angle++;
          src.q++;
        }
        break;
      case LUX_FLOAT:
        while (ny--) {          // all rows except for bottom one
          *trgt.ui8++ = 0;
          src.f++;
          angle++;
          n = nx - 2;
          while (n--) {
            value.f = *src.f;
            a = *angle++ + DEG22_5;
            s = sin(a);
            c = cos(a);
            class_id = ((s*c > 0)<<1) + (s*c*(2*c*c - 1) > 0);
            *trgt.ui8++ = (value.f < src.f[off[class_id]]
                         && value.f < src.f[-off[class_id]]);
            src.f++;
          }
          *trgt.ui8++ = 0;
          angle++;
          src.f++;
        }
        break;
      case LUX_DOUBLE:
        while (ny--) {          // all rows except for bottom one
          *trgt.ui8++ = 0;
          src.d++;
          angle++;
          n = nx - 2;
          while (n--) {
            value.d = *src.d;
            a = *angle++ + DEG22_5;
            s = sin(a);
            c = cos(a);
            class_id = ((s*c > 0)<<1) + (s*c*(2*c*c - 1) > 0);
            *trgt.ui8++ = (value.d < src.d[off[class_id]]
                         && value.d < src.d[-off[class_id]]);
            src.d++;
          }
          *trgt.ui8++ = 0;
          angle++;
          src.d++;
        }
        break;
    }
  }
  zerobytes(trgt.ui8, nx);
  return result;
}
//-------------------------------------------------------------------------
#define N_DIRECTIONS    8
#define STACKBLOCK      256
#define EDGE            -1
#define MARKED          -2
#define EDGEMARKED      -3
int32_t area_2d(int32_t narg, int32_t ps[])
/* AREA,image
 assigns labels to contiguous sets of non-zeros in the bitmap <image>.
 <image> must be a 2D LONG array. */
{
  int32_t       *ptr, *ptr0, *ptr1, *ptrend, nx, ny, n, areaNumber, *ptr2,
    offsets[8], **stack, **stack0, **stackend, nStack, onEdge, ix = 0, iy = 0,
    ix2, iy2, direction;
  int32_t       rcoords[8][2] = {
    { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 },
    { -1, 0 }, { -1, -1 }, { 0, -1 }, { 1, -1 }
  };

  nx = array_dims(ps[0])[0];    // width
  ny = array_dims(ps[0])[1];    // height

  /* offsets[] lists the offsets (in elements) to one of the nearest
     neighbors in all accessible directions */
  offsets[0] = 1;
  offsets[1] = 1 + nx;
  offsets[2] = nx;
  offsets[3] = nx - 1;
  offsets[4] = -1;
  offsets[5] = -1 - nx;
  offsets[6] = -nx;
  offsets[7] = -nx + 1;

  ptr0 = ptr = (int32_t*) array_data(ps[0]); // data start
  ptrend = ptr0 + array_size(ps[0]); // points one beyond data end

  /* The numbers in the input data <image> are interpreted as follows:
     Values equal to zero or greater than one indicate positions that
     are ignored.  Ones indicate positions that are to be labeled.
     Negative values are reserved for indicating intermediate results
     during execution of the subroutine: If negative values are
     present in <image> upon entry into the routine, then unexpected
     results may be obtained.

     Unfortunately, we cannot treat the data elements in a linear
     fashion, because the segments may not be convex in shape, so the
     intersection of a segment and any curve may have disjoint parts.
     When we find one element of a new segment, we have to find all
     other elements of the same segment before looking for another
     segment.

     When we walk through the data, trying to trace out a new segment,
     we must be careful not to run across one of the edges of the
     image.  One method to ensure this is to check for each and every
     considered element if it is on an edge, but this is inefficient
     because the checks have to be performed for all data elements
     even when the edges cover only a very small fraction of the
     image.  We do it differently: We ensure that the algorithm cannot
     run across any edges by substituting a distinct negative value
     (not confusable with other intermediate values) for any 1 found
     on the edges of the data volume.  Then checking for data on an
     edge is reduced to checking for that negative value. */

  ptr = ptr0;                   // the first data value
  areaNumber = 2;               // the label for the next segment

  // Look at the edges
  n = nx - 1;
  while (n--) {
    if (*ptr == 1)              // treatable data
      *ptr = EDGE;
    ptr++;
  }
  n = ny - 1;
  while (n--) {
    if (*ptr == 1)
      *ptr = EDGE;
    ptr += nx;
  }
  n = nx - 1;
  while (n--) {
    if (*ptr == 1)
      *ptr = EDGE;
    ptr--;
  }
  n = ny - 1;
  while (n--) {
    if (*ptr == 1)
      *ptr = EDGE;
    ptr -= nx;
  }
  /* Data values equal to 1 or EDGE are taken to indicate elements
     that need to be assigned labels. */

  // prepare a stack
  nStack = STACKBLOCK;
  stack = stack0 = (int32_t**) malloc(STACKBLOCK*sizeof(int32_t *));
  if (!stack0)                  // allocation failed
    return cerror(ALLOC_ERR, 0);
  stackend = stack0 + nStack;   // pointer to one beyond end of stack

  ptr1 = ptr0;
  do {                          // still more to do
    while (*ptr1 != 1
           && *ptr1 != EDGE
           && ptr1 < ptrend)    /* current element need not be treated
                                   and there are still more to go */
      ptr1++;

    if (ptr1 == ptrend)                 // we're all done
      break;

    /* We have found a new segment to treat.  We deal with each
       segment through a width-first algorithm, as follows: For each
       found segment element (i.e., data element that contains a 1 or
       EDGE and needs to be assigned a label) connections in eight (or
       less for EDGE) directions must be checked.  We check each
       direction in turn, and push the directions in which we find
       other segment elements onto a stack.  Then we assign a label to
       the current position and start treating the elements that are
       on the stack.  If the stack becomes empty, then we are done
       with the current segment.

       We use a width-first rather than a depth-first method because
       in the former method we only need check once if the current
       position is on an edge. */

    onEdge = (*ptr1 == EDGE);   // 1 -> on edge; 0 -> in interior
    if (onEdge) {               /* current position is on an edge:
                                   we need the coordinates for edge testing */
      ix = (ptr1 - ptr0) % nx;
      iy = (ptr1 - ptr0)/nx;
    }

    ptr = ptr1;
    while (1) {
      if (onEdge) {
        for (direction = 0; direction < N_DIRECTIONS; direction++) {
          // now we must check if the neighbor is across an edge
          ix2 = ix + rcoords[direction][0];
          iy2 = iy + rcoords[direction][1];
          if (ix2 < 0 || ix2 >= nx || iy2 < 0 || iy2 >= ny)
            // across the edge: continue with next direction
            continue;

          // the current direction does not lead across an edge
          ptr2 = ptr + offsets[direction]; // the neighboring position
          if (*ptr2 == 1 || *ptr2 == EDGE) { // neighbor must be treated
            *stack++ = ptr;     // place neighbor position on stack
            if (stack == stackend) { // need to enlarge the stack
              stack0 = (int32_t**) realloc(stack0, (nStack + STACKBLOCK)*sizeof(int32_t *));
              if (!stack0)      // allocation failed
                return cerror(ALLOC_ERR, 0);
              /* the call to realloc() may have moved the whole stack to
                 a different memory location; we ensure that <stack>
                 points at the same stack item as before the reallocation */
              stack = stack0 + nStack;
              nStack += STACKBLOCK; // update stack size
              stackend = stack0 + nStack; // and stack end pointer
            }
            *ptr2 = (*ptr2 == 1)? MARKED: EDGEMARKED;
          }
        }
      } else {                  // not on an edge
        for (direction = 0; direction < N_DIRECTIONS; direction++) {
          ptr2 = ptr + offsets[direction]; // the neighboring position
          if (*ptr2 == 1 || *ptr2 == EDGE) { // neighbor must be treated
            *stack++ = ptr2;    // place neighbor position on stack
            if (stack == stackend) { // need to enlarge the stack
              stack0 = (int32_t**) realloc(stack0, (nStack + STACKBLOCK)*sizeof(int32_t *));
              if (!stack0)      // allocation failed
                return cerror(ALLOC_ERR, 0);
              /* the call to realloc() may have moved the whole stack to
                 a different memory location; we ensure that <stack>
                 points at the same stack item as before the reallocation */
              stack = stack0 + nStack;
              nStack += STACKBLOCK; // update stack size
              stackend = stack0 + nStack;       // and stack end pointer
            }
            *ptr2 = (*ptr2 == 1)? MARKED: EDGEMARKED;
          }
        }
      }
      /* We have checked all directions for one position: assign a
         label number and pop next position, if any, from stack */

      *ptr = areaNumber;        // assign label
      if (stack == stack0)      // stack is empty: all done with this area
        break;
      // stack is not yet empty: pop last position from stack
      ptr = *--stack;
    }
    // we are done with the current segment: update label
    areaNumber++;
  } while (1);
  free(stack0);
  return 1;
}
//-------------------------------------------------------------------------
int32_t area_general(int32_t narg, int32_t ps[], int32_t isFunction)
/* AREA,bitmap[,seeds,numbers,diagonal]
 <bitmap> is assumed to be a LONG array. */
/* identifies distinct areas with values equal to 1 in a bitmap.
   Syntax:  AREA,image[,seeds,NUMBERS=numbers,DIAGONAL=diagonal]
   All elements of each distinct area are replaced by a number (larger than
   1) which labels the area. <flag> indicates whether the call is a
   function call (non-zero) or a subroutine call (zero).
   Arrays of arbitrary dimension are allowed.
   If <seeds> is specified, then only the areas in which the seeds
   (indices to the image, after conversion to LONG) lie are identified.
   If <numbers> is a scalar, then the area numbers start with the
   specified value.  If <numbers> is an array, then it must have the
   same size as <seeds>, and contains area numbers for the areas
   in which the seeds lie.  If <numbers> is undefined, then area
   numbers start at 2.  If <numbers> has at most one element, then
   the area numbers are incremented by one for each new area.
   Note that this routine uses negative numbers to indicate intermediate
   results, so pre-existing negative numbers in the data may lead to
   faulty results.
   LS 4feb93 6aug97 */
     // Strategy: find data point with value equal to 1.  Then check all
     // allowed directions in order.  If another data point is encountered
     // with value equal to 1, then remember how many directions were
     // already checked for the current point and treat the new point.
     // If all allowed directions are treated without finding any new
     // untreated points, then assign area number and resume treating
     // last partially-treated point.
/* <diagonal>: one element per dimension.  0: do not check the dimension.
   1: only allow connections to neighbors that share a face in this dimension
   2: allow connections to neighbors that share a face or a vertex in this
      dimension
 LS 17jun98, 5sep98 */
{
  int32_t       iq, *dims, ndim, nelem, nSeed, nNumber, nDirection, *seed, *number,
    i, *rcoord, *offset, j, nStack, **stack, **stack0, areaNumber,
    direction, *edge;
  int32_t       *ptr0, *ptr, *ptrend, **stackend, *ptr1, onEdge, ix, ix2, *ptr2;
  Pointer       src;
  LoopInfo      srcinfo;

  if (!symbolIsNumericalArray(ps[0])) // not a numerical array
    return cerror(NEED_NUM_ARR, ps[0]);
  iq = lux_long(1, ps);                 // ensure LONG
  if (standardLoop(iq, 0, SL_ALLAXES | SL_EACHCOORD, LUX_INT8, &srcinfo, &src,
                   NULL, NULL, NULL) == LUX_ERROR)
    return LUX_ERROR;

  nSeed = nNumber = nDirection = 0; // defaults
  seed = NULL;
  if (narg > 1 && ps[1]) {      // have <seeds>
    if (symbol_class(ps[1]) != LUX_ARRAY) // must be ARRAY
      return cerror(ILL_CLASS, ps[1]);
    iq = lux_long(1, &ps[1]);   // ensure LONG
    seed = (int32_t*) array_data(iq);   // seed indices
    nSeed = array_size(iq);     // number of seeds
  }

  number = NULL;                // default: no <number>
  if (narg > 2 && ps[2]) {      // have <number>
    iq = ps[2];
    if (numerical(iq, NULL, NULL, &nNumber, NULL) < 0) // get info
      return cerror(ILL_CLASS, iq);
    if (nNumber != 1 && nNumber != nSeed) // one element per seed, or
                                          // a single number
      return cerror(INCMP_ARG, ps[2]);
    iq = lux_long(1, &iq);      // ensure LONG
    number = (int32_t*) array_data(iq);         // numbers
    nNumber = (nNumber == nSeed)? 1: 0;
  }

  // treat DIAGONAL
  nDirection = prepareDiagonals(narg > 3? ps[3]: 0, &srcinfo, 1, &offset,
                                &edge, &rcoord, NULL);
  if (nDirection == LUX_ERROR)
    return LUX_ERROR;
  if (!nDirection)
    return luxerror("No directions satisfy the requirements", ps[3]);

  nelem = array_size(ps[0]);    // number of elements in array
  if (nSeed) {
    for (i = 0; i < nSeed; i++)
      if (seed[i] < 0 || seed[i] >= nelem) { // check if in range
        free(offset);
        free(edge);
        return luxerror("Seed position %1d (index %1d) outside of the data",
                     seed[i], i);
      }
  }

  /* treat all elements that are on an edge: if they are equal to 1, then
     set them to EDGE */
  for (i = 0; i < 2*srcinfo.ndim; i++) {
    if (edge[i]) {
      srcinfo.rearrangeEdgeLoop(NULL, i);
      do
        if (*src.l == 1)
          *src.l = EDGE;
      while (srcinfo.advanceLoop(&src) < srcinfo.ndim - 1);
    }
  }
  free(edge);

  srcinfo.rearrangeDimensionLoop();

  // get space for temporary positions.  estimate how much is needed.
  // if this is too little, then more will be allocated on the fly.
  // find greatest dimension
  dims = array_dims(iq);
  ndim = array_num_dims(iq);
  nStack = dims[0];
  for (j = 1; j < ndim; j++)
    if (dims[j] > nStack)
      nStack = dims[j];
  nStack *= nDirection;         // times number of directions
  // get space for temporary positions
  stack = stack0 = (int32_t **) malloc(nStack*sizeof(int32_t *));
  if (!stack) {
    free(offset);
    free(rcoord);
    return cerror(ALLOC_ERR, 0);
  }

  if (number) {                         // have specific area numbers to assign
    areaNumber = *number;       // first one
    number += nNumber;          // zero if scalar <number>, 1 if array
  } else
    areaNumber = 2;             // default start value

  ptr0 = ptr = src.l;
  ptrend = ptr0 + array_size(ps[0]);

  ptr = ptr0;
  stackend = stack0 + nStack;

  ptr1 = ptr0;
  do {                          // still more to do
    if (seed) {
      while (nSeed) {
        if (*seed < 0 || *seed >= nelem)
          return luxerror("Index (%1d) out of bounds", ps[1], *seed);
        if (ptr0[*seed] == 1 || ptr0[*seed] == EDGE)
          break;
        seed++;
        nSeed--;
      }
      if (nSeed <= 0)
        break;                  // all done
      ptr1 = ptr0 + *seed++;
      nSeed--;
    } else {
      while (*ptr1 != 1
             && *ptr1 != EDGE
             && ptr1 < ptrend)  /* current element need not be treated
                                   and there are still more to go */
        ptr1++;

      if (ptr1 == ptrend)       // we're all done
        break;
    }

    /* We have found a new segment to treat.  We deal with each
       segment through a width-first algorithm, as follows: For each
       found segment element (i.e., data element that contains a 1 or
       EDGE and needs to be assigned a label) connections in eight (or
       less for EDGE) directions must be checked.  We check each
       direction in turn, and push the directions in which we find
       other segment elements onto a stack.  Then we assign a label to
       the current position and start treating the elements that are
       on the stack.  If the stack becomes empty, then we are done
       with the current segment.

       We use a width-first rather than a depth-first method because
       in the former method we only need check once if the current
       position is on an edge. */

    ptr = ptr1;
    while (1) {
      onEdge = (*ptr == EDGE || *ptr == EDGEMARKED); /* 1 -> on edge;
                                                        0 -> in interior */
      if (onEdge) {             /* current position is on an edge:
                                   we need the coordinates for edge testing */
        ix = ptr - ptr0;        // index
        for (i = 0; i < ndim; i++) {
          srcinfo.coords[i] = ix % srcinfo.dims[i];
          ix /= srcinfo.dims[i];
        } // end of for (i = 0,...)
        for (direction = 0; direction < nDirection; direction++) {
          // now we must check if the neighbor is across an edge
          for (i = 0; i < ndim; i++) {
            ix2 = srcinfo.coords[i] + rcoord[i + direction*ndim];
            if (ix2 < 0 || ix2 >= dims[i]) // over the egde
              break;
          } // end of for (i = 0,...)
          if (i < ndim)                 // over the edge
            continue;

          // the current direction does not lead across an edge
          ptr2 = ptr + offset[direction]; // the neighboring position
          if (*ptr2 == 1 || *ptr2 == EDGE) { // neighbor must be treated
            *stack++ = ptr2;    // place neighbor position on stack
            if (stack == stackend) { // need to enlarge the stack
              stack0 = (int32_t**) realloc(stack0, (nStack + STACKBLOCK)*sizeof(int32_t *));
              if (!stack0)      // allocation failed
                return cerror(ALLOC_ERR, 0);
              /* the call to realloc() may have moved the whole stack to
                 a different memory location; we ensure that <stack>
                 points at the same stack item as before the reallocation */
              stack = stack0 + nStack;
              nStack += STACKBLOCK; // update stack size
              stackend = stack0 + nStack; // and stack end pointer
            } // end of if (stack == stackend)
            *ptr2 = (*ptr2 == 1)? MARKED: EDGEMARKED; // mark it
          } // end of if (*ptr2 == 1 | *ptr2 == EDGE)
        } // end of for (direction = 0; direction < nDirection; direction++)
      } else {                  // not on an edge
        for (direction = 0; direction < nDirection; direction++) {
          ptr2 = ptr + offset[direction]; // the neighboring position
          if (*ptr2 == 1 || *ptr2 == EDGE) { // neighbor must be treated
            *stack++ = ptr2;    // place neighbor position on stack
            if (stack == stackend) { // need to enlarge the stack
              stack0 = (int32_t**) realloc(stack0, (nStack + STACKBLOCK)*sizeof(int32_t *));
              if (!stack0)      // allocation failed
                return cerror(ALLOC_ERR, 0);
              /* the call to realloc() may have moved the whole stack to
                 a different memory location; we ensure that <stack>
                 points at the same stack item as before the reallocation */
              stack = stack0 + nStack;
              nStack += STACKBLOCK; // update stack size
              stackend = stack0 + nStack;       // and stack end pointer
            } // end of if (stack == stackend)
            *ptr2 = (*ptr2 == 1)? MARKED: EDGEMARKED;
          } // end of if (*ptr2 == 1 || *ptr2 == EDGE)
        } // end of for (direction = 0; ...)
      }         // end of if (onEdge) else
      /* We have checked all directions for one position: assign a
         label number and pop next position, if any, from stack */

      *ptr = areaNumber;        // assign label
      if (stack == stack0)      // stack is empty: all done with this area
        break;
      // stack is not yet empty: pop last position from stack
      ptr = *--stack;

    } // end of while (1)
    // we are done with the current segment: update label
    if (nNumber) {
      areaNumber = *number++;
      nNumber--;
    } else
      areaNumber++;
  } while (1);

  free(rcoord);
  free(offset);
  free(stack0);
  return 1;
}
//-------------------------------------------------------------------------
int32_t lux_area(int32_t narg, int32_t ps[])
// AREA,bitmap [, SEED=<seed>, NUMBERS=<numbers>, DIAGONAL=<diagonal>]
{
  if (!symbolIsNumericalArray(ps[0]) || array_type(ps[0]) != LUX_INT32)
    return luxerror("Need LONG array", ps[0]);

  if (array_num_dims(ps[0]) == 2
      && narg == 1)
    return area_2d(narg, ps);

  return area_general(narg, ps, 0);
}
//----------------------------------------------------------------------
#define SEEK_MAXIMUM    (LUX_DOUBLE + 1)
int32_t area2_2d(int32_t narg, int32_t ps[])
/* AREA2,bitmap,data [,sign]
 assigns numerical labels to those elements of <bitmap> that have values
 equal to 1.  A particular label is assigned to all eligible elements that
 are connected in nearest-neighbor fashion through vertices or edes.
 It is assumed that (1) <bitmap> is a 2D LONG array; and (2) <data>
 is a 2D array with the same dimensions as <bitmap>.  LS 5sep98 */
{
  int32_t       *ptr, *ptr0, *ptr1, *ptrend, nx, ny, n, areaNumber, *ptr2,
    offset[8], **stack, **stack0, **stackend, nStack, onEdge, ix = 0, iy = 0,
    ix2, iy2, direction, maximum, j;
  int32_t       stride, type;
  int32_t       rcoords[8][2] = { { 1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 },
                          { -1, -1 }, { 0, -1 }, { 1, -1 }, { -1, 0 } };
  Pointer       dataptr0, dataptr, dataptr2;

  nx = array_dims(ps[0])[0];    // width
  ny = array_dims(ps[0])[1];    // height
  dataptr0.l = (int32_t*) array_data(ps[1]);

  type = array_type(ps[1]);
  stride = lux_type_size[type];

  maximum = (narg > 2 && ps[2])? (int_arg(ps[2]) >= 0 ? 1: 0): 1;
  maximum *= SEEK_MAXIMUM;

  /* offsets[] lists the offsets (in elements) to one of the nearest
     neighbors in all accessible directions */
  offset[0] = 1;
  offset[1] = -1 + nx;
  offset[2] = nx;
  offset[3] = nx + 1;
  offset[4] = -nx - 1;
  offset[5] = -nx;
  offset[6] = 1 - nx;
  offset[7] = -1;

  ptr0 = ptr = (int32_t*) array_data(ps[0]); // bitmap start
  ptrend = ptr0 + array_size(ps[0]); // points one beyond bitmap end

  /* The numbers in the input <bitmap> are interpreted as follows:
     Values equal to zero or greater than one indicate positions that
     are ignored.  Ones indicate positions that are to be labeled.
     Negative values are reserved for indicating intermediate results
     during execution of the subroutine: If negative values are
     present in <bitmap> upon entry into the routine, then unexpected
     results may be obtained.

     Unfortunately, we cannot treat the data elements in a linear
     fashion, because the segments may not be convex in shape, so the
     intersection of a segment and any curve may have disjoint parts.
     When we find one element of a new segment, we have to find all
     other elements of the same segment before looking for another
     segment.

     When we walk through the data, trying to trace out a new segment,
     we must be careful not to run across one of the edges of the
     image.  One method to ensure this is to check for each and every
     considered element if it is on an edge, but this is inefficient
     because the checks have to be performed for all data elements
     even when the edges cover only a very small fraction of the
     image.  We do it differently: We ensure that the algorithm cannot
     run across any edges by substituting a distinct negative value
     (not confusable with other intermediate values) for any 1 found
     on the edges of the data volume.  Then checking for data on an
     edge is reduced to checking for that negative value. */

  ptr = ptr0;                   // the first data value
  areaNumber = 2;               // the label for the next segment

  // Look at the edges
  n = nx - 1;
  while (n--) {
    if (*ptr == 1)              // treatable data
      *ptr = EDGE;
    ptr++;
  }
  n = ny - 1;
  while (n--) {
    if (*ptr == 1)
      *ptr = EDGE;
    ptr += nx;
  }
  n = nx - 1;
  while (n--) {
    if (*ptr == 1)
      *ptr = EDGE;
    ptr--;
  }
  n = ny - 1;
  while (n--) {
    if (*ptr == 1)
      *ptr = EDGE;
    ptr -= nx;
  }
  /* Data values equal to 1 or EDGE are taken to indicate elements
     that need to be assigned labels. */

  // prepare a stack
  nStack = STACKBLOCK;
  stack = stack0 = (int32_t**) malloc(STACKBLOCK*sizeof(int32_t *));
  if (!stack0)                  // allocation failed
    return cerror(ALLOC_ERR, 0);
  stackend = stack0 + nStack;   // pointer to one beyond end of stack

  ptr1 = ptr0;
  do {                          // still more to do
    while (*ptr1 != 1
           && *ptr1 != EDGE
           && ptr1 < ptrend)    /* current element need not be treated
                                   and there are still more to go */
      ptr1++;

    if (ptr1 == ptrend)                 // we're all done
      break;

    /* We have found a new segment to treat.  Those elements that are
       connected to a single appropriate local extreme through
       steepest ascent/descent paths get the same numerical label.
       This means that different appropriate local extremes will be
       assigned to differently labeled areas.  It also means that in
       some cases the assignment of a particular element to one or
       another segment may depend on which element of the segment(s)
       happens to be seen first.

       We arrange this as follows: From the just discovered position,
       walk in steepest ascent/descent fashion to the associated local
       extreme in <data> but only through data elements that are
       flagged as part of some segment (i.e., that have a bitmap value
       equal to 1).  Then we start assigning labels as for the case
       where no data values are inspected.  We use this restricted
       sense of "local extreme" because it is possible for a segment
       to contain no local extreme when taking into account all
       neighbors whether they are segment elements or not.

       We deal with each segment through a width-first algorithm, as
       follows: For each found segment element (i.e., data element
       that contains a 1 or EDGE and needs to be assigned a label)
       connections in eight (or less for EDGE) directions must be
       checked.  We check each direction in turn, and push the
       directions in which we find other segment elements onto a
       stack.  Then we assign a label to the current position and
       start treating the elements that are on the stack.  If the
       stack becomes empty, then we are done with the current segment.

       We use a width-first rather than a depth-first method because
       in the former method we only need check once if the current
       position is on an edge.  */

    // We walk to the nearest extreme of the sought kind
    ptr = ptr1;
    dataptr.ui8 = dataptr0.ui8 + (ptr1 - ptr0)*stride; // corresponding data
    while (1) {
      j = -1;
      dataptr2 = dataptr;
      if (*ptr == EDGE) {       // get the coordinates
        iy = ptr - ptr0;
        ix = iy % nx;
        iy /= nx;
      }
      for (direction = 0; direction < N_DIRECTIONS; direction++) {
        if (ptr[offset[direction]] != 1
            && ptr[offset[direction]] != EDGE) // not active
          continue;
        if (*ptr == EDGE) {
          // now we must check if the neighbor is across an edge
          ix2 = ix + rcoords[direction][0];
          iy2 = iy + rcoords[direction][1];
          if (ix2 < 0 || ix2 >= nx || iy2 < 0 || iy2 >= ny)
            // across the edge: continue with next direction
            continue;
        }
        switch (maximum + type) {
          case SEEK_MAXIMUM + LUX_INT8:
            if (dataptr.ui8[offset[direction]] > *dataptr2.ui8) {
              j = direction;
              dataptr2.ui8 = dataptr.ui8 + offset[direction];
            }
            break;
          case SEEK_MAXIMUM + LUX_INT16:
            if (dataptr.i16[offset[direction]] > *dataptr2.i16) {
              j = direction;
              dataptr2.i16 = dataptr.i16 + offset[direction];
            }
            break;
          case SEEK_MAXIMUM + LUX_INT32:
            if (dataptr.l[offset[direction]] > *dataptr2.l) {
              j = direction;
              dataptr2.l = dataptr.l + offset[direction];
            }
            break;
          case SEEK_MAXIMUM + LUX_INT64:
            if (dataptr.q[offset[direction]] > *dataptr2.q) {
              j = direction;
              dataptr2.q = dataptr.q + offset[direction];
            }
            break;
          case SEEK_MAXIMUM + LUX_FLOAT:
            if (dataptr.f[offset[direction]] > *dataptr2.f) {
              j = direction;
              dataptr2.f = dataptr.f + offset[direction];
            }
            break;
          case SEEK_MAXIMUM + LUX_DOUBLE:
            if (dataptr.d[offset[direction]] > *dataptr2.d) {
              j = direction;
              dataptr2.d = dataptr.d + offset[direction];
            }
            break;
          case LUX_INT8:
            if (dataptr.ui8[offset[direction]] < *dataptr2.ui8) {
              j = direction;
              dataptr2.ui8 = dataptr.ui8 + offset[direction];
            }
            break;
          case LUX_INT16:
            if (dataptr.i16[offset[direction]] < *dataptr2.i16) {
              j = direction;
              dataptr2.i16 = dataptr.i16 + offset[direction];
            }
            break;
          case LUX_INT32:
            if (dataptr.l[offset[direction]] < *dataptr2.l) {
              j = direction;
              dataptr2.l = dataptr.l + offset[direction];
            }
            break;
          case LUX_INT64:
            if (dataptr.q[offset[direction]] < *dataptr2.q) {
              j = direction;
              dataptr2.q = dataptr.q + offset[direction];
            }
            break;
          case LUX_FLOAT:
            if (dataptr.f[offset[direction]] < *dataptr2.f) {
              j = direction;
              dataptr2.f = dataptr.f + offset[direction];
            }
            break;
          case LUX_DOUBLE:
            if (dataptr.d[offset[direction]] < *dataptr2.d) {
              j = direction;
              dataptr2.d = dataptr.d + offset[direction];
            }
            break;
        }
      }
      if (j != -1) {            // not yet found the extreme
        // update pointers
        ptr += offset[j];       // to bitmap
        dataptr.ui8 += offset[j]*stride; // to data
      } else
        break;
    }

    /* We're at the position of the nearest desired extreme.
     Now we start identifying and assigning labels to the elements
     of the current segment.  We make sure to only walk downhill
     if we were looking for a maximum, or uphill if we were looking
     for a minimum. */

    onEdge = (*ptr == EDGE);    // 1 -> on edge; 0 -> in interior
    if (onEdge) {               /* current position is on an edge:
                                   we need the coordinates for edge testing */
      ix = (ptr - ptr0) % nx;
      iy = (ptr - ptr0)/nx;
    }

    while (1) {
      if (onEdge) {
        for (direction = 0; direction < N_DIRECTIONS; direction++) {
          // now we must check if the neighbor is across an edge
          ix2 = ix + rcoords[direction][0];
          iy2 = iy + rcoords[direction][1];
          if (ix2 < 0 || ix2 >= nx || iy2 < 0 || iy2 >= ny)
            // across the edge: continue with next direction
            continue;

          // the current direction does not lead across an edge
          ptr2 = ptr + offset[direction]; // the neighboring position
          if (*ptr2 != 1 && *ptr2 != EDGE)
            continue;
          switch (maximum + type) {
            case SEEK_MAXIMUM + LUX_INT8:
              if (dataptr.ui8[offset[direction]] >= *dataptr.ui8)
                continue;
              break;
            case SEEK_MAXIMUM + LUX_INT16:
              if (dataptr.i16[offset[direction]] >= *dataptr.i16)
                continue;
              break;
            case SEEK_MAXIMUM + LUX_INT32:
              if (dataptr.l[offset[direction]] >= *dataptr.l)
                continue;
              break;
            case SEEK_MAXIMUM + LUX_INT64:
              if (dataptr.q[offset[direction]] >= *dataptr.q)
                continue;
              break;
            case SEEK_MAXIMUM + LUX_FLOAT:
              if (dataptr.f[offset[direction]] >= *dataptr.f)
                continue;
              break;
            case SEEK_MAXIMUM + LUX_DOUBLE:
              if (dataptr.d[offset[direction]] >= *dataptr.d)
                continue;
              break;
            case LUX_INT8:
              if (dataptr.ui8[offset[direction]] <= *dataptr.ui8)
                continue;
              break;
            case LUX_INT16:
              if (dataptr.i16[offset[direction]] <= *dataptr.i16)
                continue;
              break;
            case LUX_INT32:
              if (dataptr.l[offset[direction]] <= *dataptr.l)
                continue;
              break;
            case LUX_INT64:
              if (dataptr.q[offset[direction]] <= *dataptr.q)
                continue;
              break;
            case LUX_FLOAT:
              if (dataptr.f[offset[direction]] <= *dataptr.f)
                continue;
              break;
            case LUX_DOUBLE:
              if (dataptr.d[offset[direction]] <= *dataptr.d)
                continue;
              break;
          }

          // the current position is accepted

          *stack++ = ptr;       // place neighbor position on stack
          if (stack == stackend) { // need to enlarge the stack
            stack0 = (int32_t**) realloc(stack0, (nStack + STACKBLOCK)*sizeof(int32_t *));
            if (!stack0)        // allocation failed
              return cerror(ALLOC_ERR, 0);
            /* the call to realloc() may have moved the whole stack to
               a different memory location; we ensure that <stack>
               points at the same stack item as before the reallocation */
            stack = stack0 + nStack;
            nStack += STACKBLOCK; // update stack size
            stackend = stack0 + nStack; // and stack end pointer
          }
          *ptr2 = (*ptr2 == 1)? MARKED: EDGEMARKED;
        }
      } else {                  // not on an edge
        for (direction = 0; direction < N_DIRECTIONS; direction++) {
          ptr2 = ptr + offset[direction]; // the neighboring position
          if (*ptr2 != 1 && *ptr2 != EDGE)
            continue;
          switch (maximum + type) {
            case SEEK_MAXIMUM + LUX_INT8:
              if (dataptr.ui8[offset[direction]] >= *dataptr.ui8)
                continue;
              break;
            case SEEK_MAXIMUM + LUX_INT16:
              if (dataptr.i16[offset[direction]] >= *dataptr.i16)
                continue;
              break;
            case SEEK_MAXIMUM + LUX_INT32:
              if (dataptr.l[offset[direction]] >= *dataptr.l)
                continue;
              break;
            case SEEK_MAXIMUM + LUX_INT64:
              if (dataptr.q[offset[direction]] >= *dataptr.q)
                continue;
              break;
            case SEEK_MAXIMUM + LUX_FLOAT:
              if (dataptr.f[offset[direction]] >= *dataptr.f)
                continue;
              break;
            case SEEK_MAXIMUM + LUX_DOUBLE:
              if (dataptr.d[offset[direction]] >= *dataptr.d)
                continue;
              break;
            case LUX_INT8:
              if (dataptr.ui8[offset[direction]] <= *dataptr.ui8)
                continue;
              break;
            case LUX_INT16:
              if (dataptr.i16[offset[direction]] <= *dataptr.i16)
                continue;
              break;
            case LUX_INT32:
              if (dataptr.l[offset[direction]] <= *dataptr.l)
                continue;
              break;
            case LUX_INT64:
              if (dataptr.q[offset[direction]] <= *dataptr.q)
                continue;
              break;
            case LUX_FLOAT:
              if (dataptr.f[offset[direction]] <= *dataptr.f)
                continue;
              break;
            case LUX_DOUBLE:
              if (dataptr.d[offset[direction]] <= *dataptr.d)
                continue;
              break;
          }

          // the current position is accepted

          *stack++ = ptr2;      // place neighbor position on stack
          if (stack == stackend) { // need to enlarge the stack
            stack0 = (int32_t**) realloc(stack0, (nStack + STACKBLOCK)*sizeof(int32_t *));
            if (!stack0)        // allocation failed
              return cerror(ALLOC_ERR, 0);
            /* the call to realloc() may have moved the whole stack to
               a different memory location; we ensure that <stack>
               points at the same stack item as before the reallocation */
            stack = stack0 + nStack;
            nStack += STACKBLOCK; // update stack size
            stackend = stack0 + nStack;         // and stack end pointer
          }
          *ptr2 = (*ptr2 == 1)? MARKED: EDGEMARKED;
        }
      }

      /* We have checked all directions for one position: assign a
         label number and pop next position, if any, from stack */

      *ptr = areaNumber;        // assign label
      if (stack == stack0)      // stack is empty: all done with this area
        break;
      // stack is not yet empty: pop last position from stack
      ptr = *--stack;
      dataptr.ui8 = dataptr0.ui8 + (ptr - ptr0)*stride; // corresponding data
    }
    // we are done with the current segment: update label
    areaNumber++;
  } while (1);
  free(stack0);
  return 1;
}
//----------------------------------------------------------------------
int32_t area2_general(int32_t narg, int32_t ps[])
/* AREA,bitmap,data[,seed,numbers,diagonal,sign]
 <bitmap> is assumed to be a LONG array. */
/* identifies distinct areas with values equal to 1 in a bitmap.
   Syntax:  AREA,image,data,[,seeds,NUMBERS=numbers,DIAGONAL=diagonal]
   All elements of each distinct area are replaced by a number (larger than
   1) which labels the area. <flag> indicates whether the call is a
   function call (non-zero) or a subroutine call (zero).
   Arrays of arbitrary dimension are allowed.
   If <seeds> is specified, then only the areas in which the seeds
   (indices to the image, after conversion to LONG) lie are identified.
   If <numbers> is a scalar, then the area numbers start with the
   specified value.  If <numbers> is an array, then it must have the
   same size as <seeds>, and contains area numbers for the areas
   in which the seeds lie.  If <numbers> is undefined, then area
   numbers start at 2.  If <numbers> has at most one element, then
   the area numbers are incremented by one for each new area.
   Note that this routine uses negative numbers to indicate intermediate
   results, so pre-existing negative numbers in the data may lead to
   faulty results.
   LS 4feb93 6aug97 */
     // Strategy: find data point with value equal to 1.  Then check all
     // allowed directions in order.  If another data point is encountered
     // with value equal to 1, then remember how many directions were
     // already checked for the current point and treat the new point.
     // If all allowed directions are treated without finding any new
     // untreated points, then assign area number and resume treating
     // last partially-treated point.
/* <diagonal>: one element per dimension.  0: do not check the dimension.
   1: only allow connections to neighbors that share a face in this dimension
   2: allow connections to neighbors that share a face or a vertex in this
      dimension
 LS 17jun98, 5sep98 */
{
  int32_t       iq, *dims, ndim, nelem, nSeed, nNumber, nDirection, *seed, *number,
    i, *rcoord, *offset, j, nStack, **stack, **stack0, areaNumber,
    direction, stride, maximum, type, *edge;
  int32_t       *ptr0, *ptr, *ptrend, **stackend, *ptr1, onEdge, ix, ix2, *ptr2;
  Pointer       src, dataptr0, dataptr, dataptr2;
  LoopInfo      srcinfo;

  if (!symbolIsNumericalArray(ps[0])) // not a numerical array
    return cerror(NEED_NUM_ARR, ps[0]);
  iq = lux_long(1, ps);                 // ensure LONG
  if (standardLoop(iq, 0, SL_ALLAXES | SL_EACHCOORD, LUX_INT8, &srcinfo, &src,
                   NULL, NULL, NULL) == LUX_ERROR)
    return LUX_ERROR;

  ptr0 = src.l;

  if (!symbolIsNumericalArray(ps[1]))
    return cerror(NEED_NUM_ARR, ps[1]);
  if (array_size(ps[1]) != array_size(iq)) // must have same size
    return cerror(INCMP_ARG, ps[1]);
  dataptr0.l = (int32_t*) array_data(ps[1]);
  type = array_type(ps[1]);
  stride = lux_type_size[type];
  maximum = (narg > 5 && ps[5])? (int_arg(ps[5]) >= 0 ? 1: 0): 1;
  maximum *= SEEK_MAXIMUM;

  nSeed = nNumber = nDirection = 0;             // defaults
  seed = NULL;
  if (narg > 2 && ps[2]) {      // have <seeds>
    if (symbol_class(ps[2]) != LUX_ARRAY) // must be ARRAY
      return cerror(ILL_CLASS, ps[2]);
    iq = lux_long(1, &ps[2]);   // ensure LONG
    seed = (int32_t*) array_data(iq);   // seed indices
    nSeed = array_size(iq);     // number of seeds
  }

  nelem = array_size(ps[0]);
  if (nSeed) {
    for (i = 0; i < nSeed; i++)
      if (seed[i] < 0 || seed[i] >= nelem) // check if in range
        return luxerror("Seed position %1d (index %1d) outside of the data",
                     ps[2], seed[i], i);
  }

  number = NULL;                // default: no <number>
  if (narg > 3 && ps[3]) {      // have <number>
    iq = ps[3];
    if (numerical(iq, NULL, NULL, &nNumber, NULL) < 0) // get info
      return cerror(ILL_CLASS, iq);
    if (nNumber != 1 && nNumber != nSeed) // one element per seed, or
                                          // a single number
      return cerror(INCMP_ARG, ps[3]);
    iq = lux_long(1, &iq);      // ensure LONG
    number = (int32_t*) array_data(iq);         // numbers
    nNumber = (nNumber == nSeed)? 1: 0;
  }

  nDirection = prepareDiagonals(narg > 4? ps[4]: 0, &srcinfo, 1, &offset,
                                &edge, &rcoord, NULL);
  if (nDirection == LUX_ERROR)
    return LUX_ERROR;
  if (!nDirection)
    return luxerror("No directions satisfy the requirements", ps[4]);

  // we mark all data to be treated that is on an edge
  for (i = 0; i < 2*srcinfo.rndim; i++) {
    srcinfo.rearrangeEdgeLoop(NULL, i);
    do {
      if (*src.l == 1)
        *src.l = EDGE;
    } while (srcinfo.advanceLoop(&src) < srcinfo.rndim - 1);
  }
  free(edge);

  srcinfo.rearrangeDimensionLoop();

  // prepare a stack
  nStack = STACKBLOCK;
  stack = stack0 = (int32_t**) malloc(STACKBLOCK*sizeof(int32_t *));
  if (!stack0) {
    free(offset);
    free(rcoord);
    return cerror(ALLOC_ERR, 0);
  }
  stackend = stack0 + nStack;

  if (number) {                         // have specific area numbers to assign
    areaNumber = *number;       // first one
    number += nNumber;          // zero if scalar <number>, 1 if array
  } else
    areaNumber = 2;             // default start value

  ptrend = ptr0 + array_size(ps[0]);

  ptr = ptr0;
  stackend = stack0 + nStack;

  ndim = srcinfo.rndim;
  dims = srcinfo.dims;

  ptr1 = ptr0;
  do {                          // still more to do
    if (seed) {
      while (nSeed) {
        if (ptr0[*seed] == 1 || ptr0[*seed] == EDGE)
          break;
        seed++;
        nSeed--;
      }
      if (nSeed <= 0)
        break;                  // all done
      ptr1 = ptr0 + *seed++;
      nSeed--;
    } else {
      while (*ptr1 != 1
             && *ptr1 != EDGE
             && ptr1 < ptrend)  /* current element need not be treated
                                   and there are still more to go */
        ptr1++;

      if (ptr1 == ptrend)       // we're all done
        break;
    }

    /* We have found a new segment to treat.  We deal with each
       segment through a width-first algorithm, as follows: For each
       found segment element (i.e., data element that contains a 1 or
       EDGE and needs to be assigned a label) connections in all allowed
       directions must be checked.  We check each
       direction in turn, and push the directions in which we find
       other segment elements onto a stack.  Then we assign a label to
       the current position and start treating the elements that are
       on the stack.  If the stack becomes empty, then we are done
       with the current segment.

       We use a width-first rather than a depth-first method because
       in the former method we only need check once if the current
       position is on an edge. */

    // We walk to the nearest extreme of the sought kind
    ptr = ptr1;
    dataptr.ui8 = dataptr0.ui8 + (ptr1 - ptr0)*stride; // corresponding data
    while (1) {
      j = -1;
      dataptr2 = dataptr;
      onEdge = (*ptr == EDGE);
      if (onEdge) {
        // now we must check if the neighbor is across an edge
        ix = ptr - ptr0;        // index
        for (i = 0; i < ndim; i++) {
          srcinfo.coords[i] = ix % srcinfo.dims[i];
          ix /= srcinfo.dims[i];
        }
      }

      for (direction = 0; direction < nDirection; direction++) {
        if (ptr[offset[direction]] != 1
            && ptr[offset[direction]] != EDGE) // not active
          continue;
        if (*ptr == EDGE) {
          for (i = 0; i < ndim; i++) {
            ix2 = srcinfo.coords[i] + rcoord[i + direction*ndim];
            if (ix2 < 0 || ix2 >= dims[i])
              // across the edge: continue with next direction
              continue;
          }
        }
        switch (maximum + type) {
          case SEEK_MAXIMUM + LUX_INT8:
            if (dataptr.ui8[offset[direction]] > *dataptr2.ui8) {
              j = direction;
              dataptr2.ui8 = dataptr.ui8 + offset[direction];
            }
            break;
          case SEEK_MAXIMUM + LUX_INT16:
            if (dataptr.i16[offset[direction]] > *dataptr2.i16) {
              j = direction;
              dataptr2.i16 = dataptr.i16 + offset[direction];
            }
            break;
          case SEEK_MAXIMUM + LUX_INT32:
            if (dataptr.l[offset[direction]] > *dataptr2.l) {
              j = direction;
              dataptr2.l = dataptr.l + offset[direction];
            }
            break;
          case SEEK_MAXIMUM + LUX_INT64:
            if (dataptr.q[offset[direction]] > *dataptr2.q) {
              j = direction;
              dataptr2.q = dataptr.q + offset[direction];
            }
            break;
          case SEEK_MAXIMUM + LUX_FLOAT:
            if (dataptr.f[offset[direction]] > *dataptr2.f) {
              j = direction;
              dataptr2.f = dataptr.f + offset[direction];
            }
            break;
          case SEEK_MAXIMUM + LUX_DOUBLE:
            if (dataptr.d[offset[direction]] > *dataptr2.d) {
              j = direction;
              dataptr2.d = dataptr.d + offset[direction];
            }
            break;
          case LUX_INT8:
            if (dataptr.ui8[offset[direction]] < *dataptr2.ui8) {
              j = direction;
              dataptr2.ui8 = dataptr.ui8 + offset[direction];
            }
            break;
          case LUX_INT16:
            if (dataptr.i16[offset[direction]] < *dataptr2.i16) {
              j = direction;
              dataptr2.i16 = dataptr.i16 + offset[direction];
            }
            break;
          case LUX_INT32:
            if (dataptr.l[offset[direction]] < *dataptr2.l) {
              j = direction;
              dataptr2.l = dataptr.l + offset[direction];
            }
            break;
          case LUX_INT64:
            if (dataptr.q[offset[direction]] < *dataptr2.q) {
              j = direction;
              dataptr2.q = dataptr.q + offset[direction];
            }
            break;
          case LUX_FLOAT:
            if (dataptr.f[offset[direction]] < *dataptr2.f) {
              j = direction;
              dataptr2.f = dataptr.f + offset[direction];
            }
            break;
          case LUX_DOUBLE:
            if (dataptr.d[offset[direction]] < *dataptr2.d) {
              j = direction;
              dataptr2.d = dataptr.d + offset[direction];
            }
            break;
        }
      }
      if (j != -1) {            // not yet found the extreme
        // update pointers
        ptr += offset[j];       // to bitmap
        dataptr.ui8 += offset[j]*stride; // to data
      } else
        break;
    }

    /* We're at the position of the nearest desired extreme.
     Now we start identifying and assigning labels to the elements
     of the current segment.  We make sure to only walk downhill
     if we were looking for a maximum, or uphill if we were looking
     for a minimum. */

    onEdge = (*ptr == EDGE);    // 1 -> on edge; 0 -> in interior
    if (onEdge) {               /* current position is on an edge:
                                   we need the coordinates for edge testing */
      ix = ptr - ptr0;          // index
      for (i = 0; i < ndim; i++) {
        srcinfo.coords[i] = ix % srcinfo.dims[i];
        ix /= srcinfo.dims[i];
      }
    }

    while (1) {
      if (onEdge) {
        for (direction = 0; direction < nDirection; direction++) {
          // now we must check if the neighbor is across an edge
          for (i = 0; i < ndim; i++) {
            ix2 = srcinfo.coords[i] + rcoord[i + direction*ndim];
            if (ix2 < 0 || ix2 >= dims[i]) // over the egde
              continue;
          }

          // the current direction does not lead across an edge
          ptr2 = ptr + offset[direction]; // the neighboring position
          if (*ptr2 != 1 && *ptr2 != EDGE)
            continue;
          switch (maximum + type) {
            case SEEK_MAXIMUM + LUX_INT8:
              if (dataptr.ui8[offset[direction]] >= *dataptr.ui8)
                continue;
              break;
            case SEEK_MAXIMUM + LUX_INT16:
              if (dataptr.i16[offset[direction]] >= *dataptr.i16)
                continue;
              break;
            case SEEK_MAXIMUM + LUX_INT32:
              if (dataptr.l[offset[direction]] >= *dataptr.l)
                continue;
              break;
            case SEEK_MAXIMUM + LUX_INT64:
              if (dataptr.q[offset[direction]] >= *dataptr.q)
                continue;
              break;
            case SEEK_MAXIMUM + LUX_FLOAT:
              if (dataptr.f[offset[direction]] >= *dataptr.f)
                continue;
              break;
            case SEEK_MAXIMUM + LUX_DOUBLE:
              if (dataptr.d[offset[direction]] >= *dataptr.d)
                continue;
              break;
            case LUX_INT8:
              if (dataptr.ui8[offset[direction]] <= *dataptr.ui8)
                continue;
              break;
            case LUX_INT16:
              if (dataptr.i16[offset[direction]] <= *dataptr.i16)
                continue;
              break;
            case LUX_INT32:
              if (dataptr.l[offset[direction]] <= *dataptr.l)
                continue;
              break;
            case LUX_INT64:
              if (dataptr.q[offset[direction]] <= *dataptr.q)
                continue;
              break;
            case LUX_FLOAT:
              if (dataptr.f[offset[direction]] <= *dataptr.f)
                continue;
              break;
            case LUX_DOUBLE:
              if (dataptr.d[offset[direction]] <= *dataptr.d)
                continue;
              break;
          }

          // the current position is accepted

          *stack++ = ptr;       // place neighbor position on stack
          if (stack == stackend) { // need to enlarge the stack
            stack0 = (int32_t**) realloc(stack0, (nStack + STACKBLOCK)*sizeof(int32_t *));
            if (!stack0)        // allocation failed
              return cerror(ALLOC_ERR, 0);
            /* the call to realloc() may have moved the whole stack to
               a different memory location; we ensure that <stack>
               points at the same stack item as before the reallocation */
            stack = stack0 + nStack;
            nStack += STACKBLOCK; // update stack size
            stackend = stack0 + nStack; // and stack end pointer
          }
          *ptr2 = (*ptr2 == 1)? MARKED: EDGEMARKED;
        }
      } else {                  // not on an edge
        for (direction = 0; direction < nDirection; direction++) {
          ptr2 = ptr + offset[direction]; // the neighboring position
          if (*ptr2 != 1 && *ptr2 != EDGE)
            continue;
          switch (maximum + type) {
            case SEEK_MAXIMUM + LUX_INT8:
              if (dataptr.ui8[offset[direction]] >= *dataptr.ui8)
                continue;
              break;
            case SEEK_MAXIMUM + LUX_INT16:
              if (dataptr.i16[offset[direction]] >= *dataptr.i16)
                continue;
              break;
            case SEEK_MAXIMUM + LUX_INT32:
              if (dataptr.l[offset[direction]] >= *dataptr.l)
                continue;
              break;
            case SEEK_MAXIMUM + LUX_INT64:
              if (dataptr.q[offset[direction]] >= *dataptr.q)
                continue;
              break;
            case SEEK_MAXIMUM + LUX_FLOAT:
              if (dataptr.f[offset[direction]] >= *dataptr.f)
                continue;
              break;
            case SEEK_MAXIMUM + LUX_DOUBLE:
              if (dataptr.d[offset[direction]] >= *dataptr.d)
                continue;
              break;
            case LUX_INT8:
              if (dataptr.ui8[offset[direction]] <= *dataptr.ui8)
                continue;
              break;
            case LUX_INT16:
              if (dataptr.i16[offset[direction]] <= *dataptr.i16)
                continue;
              break;
            case LUX_INT32:
              if (dataptr.l[offset[direction]] <= *dataptr.l)
                continue;
              break;
            case LUX_INT64:
              if (dataptr.q[offset[direction]] <= *dataptr.q)
                continue;
              break;
            case LUX_FLOAT:
              if (dataptr.f[offset[direction]] <= *dataptr.f)
                continue;
              break;
            case LUX_DOUBLE:
              if (dataptr.d[offset[direction]] <= *dataptr.d)
                continue;
              break;
          }

          // the current position is accepted

          *stack++ = ptr2;      // place neighbor position on stack
          if (stack == stackend) { // need to enlarge the stack
            stack0 = (int32_t**) realloc(stack0, (nStack + STACKBLOCK)*sizeof(int32_t *));
            if (!stack0)        // allocation failed
              return cerror(ALLOC_ERR, 0);
            /* the call to realloc() may have moved the whole stack to
               a different memory location; we ensure that <stack>
               points at the same stack item as before the reallocation */
            stack = stack0 + nStack;
            nStack += STACKBLOCK; // update stack size
            stackend = stack0 + nStack;         // and stack end pointer
          }
          *ptr2 = (*ptr2 == 1)? MARKED: EDGEMARKED;
        }
      }
      /* We have checked all directions for one position: assign a
         label number and pop next position, if any, from stack */

      *ptr = areaNumber;        // assign label
      if (stack == stack0)      // stack is empty: all done with this area
        break;
      // stack is not yet empty: pop last position from stack
      ptr = *--stack;
      dataptr.ui8 = dataptr0.ui8 + (ptr - ptr0)*stride; // corresponding data
    }
    // we are done with the current segment: update label
    if (nNumber) {
      areaNumber = *number++;
      nNumber--;
    } else
      areaNumber++;
  } while (1);

  free(rcoord);
  free(offset);
  free(stack0);
  return 1;
}
//----------------------------------------------------------------------
int32_t lux_area2(int32_t narg, int32_t ps[])
{
  if (!symbolIsNumericalArray(ps[0]) || array_type(ps[0]) != LUX_INT32)
    return luxerror("Need LONG array", ps[0]);
  if (!symbolIsNumericalArray(ps[1]))
    return cerror(NEED_ARR, ps[1]);
  if (array_size(ps[0]) != array_size(ps[1]))
    return cerror(INCMP_ARG, ps[1]);

  if (array_num_dims(ps[0]) == 2
      && narg == 2)
    return area2_2d(narg, ps);

  return area2_general(narg, ps);
}
//----------------------------------------------------------------------
int32_t lux_basin(int32_t narg, int32_t ps[])
/* Returns a basin map derived from altitude map <im>
   Syntax:  y = basin(im [,/DIFFERENCE,/SINK,/NUMBER])
   <mode>:  1 /NUMBER -> assign different number to each basin, starting
              with number 1 and increasing in steps of one.
            2 /SINK -> assign index to <im> of the final sink corresponding
              to each pixel.  Similar to 1, but with different labels for
              each basin
            4 /DIFFERENCE -> return index offset of steepest-descent
              neighbor for each pixel
   LS 19feb93 9may97 */
{
 int32_t        iq, result_sym, nx, ny, col, row, *wsh, nx0;
 int32_t        mode, i, n, nsinks, *code, n0;
 register uint8_t       loc;
 uint8_t        end, locs[3];
 array  *h;
 register float         *alt, min0, min1 = 0.0, min2 = 0.0, min;

 iq = ps[0];                            // altitude map
 CK_ARR(iq,1);
 iq = lux_float(1, &iq);                // make float for easy programming
 h = HEAD(iq);
 if (h->ndim != 2)
   return cerror(NEED_2D_ARR, iq);
 nx = h->dims[0];               // dimensions
 ny = h->dims[1];
 alt = (float *) LPTR(h);       // altitudes (i.e. data)
 result_sym = array_clone(iq, LUX_INT32);
 h = HEAD(result_sym);
 wsh = LPTR(h);                         // result map
 switch (internalMode & 7) {
 case 1:                        // /NUMBER
   mode = 0;
   break;
 case 2:                        // /SINK
   mode = 1;
   break;
 case 4:                        // /DIFFERENCE
   mode = 2;
   break;
 default:
   mode = 0;
   break;
 }

 // strategy: we compare the data in units of a column of 3 elements
 // centered on the element of interest.  min2 will contain the minimum
 // value found in such a unit, and locs[2] the index of the location
 // of the minimum in the unit: 0 for the bottom (lowest y) element,
 // 1 for the middle element, and 2 for the top (highest y) element.
 // we shift the min/locs data to the left and repeat, until we have
 // collected data on a block of 3 by 3 elements.  Then we determine
 // which of those nine elements is the lowest one and assign an
 // according offset to the central element.

 col = row = end = nsinks = 0;
 while (!end) {                         // all points
   if (!col) {                  // at left edge
     min1 = min2 = FLT_MAX;
     locs[1] = locs[2] = 0;     // flags bottom element as minimum
     if (row) {                         // not in bottom row
       min2 = alt[-nx];
       locs[2] = 0;             // flags bottom element as minimum
     }
     if (*alt < min2) {
       min2 = *alt;
       locs[2] = 1;             // flags middle element as minimum
     }
     if (row < ny - 1 && alt[nx] < min2) {
       min2 = alt[nx];
       locs[2] = 2;             // flags top element as minimum
     }
     alt++;                     // move one spot to the right
   }
   min0 = min1;                         // shift minimum values and flags
   min1 = min2;
   locs[0] = locs[1];
   locs[1] = locs[2];
   min2 = FLT_MAX;
   if (row) {                   // not in bottom row
     min2 = alt[-nx];           // comparison value from next lower row
     locs[2] = 0;               // flags bottom element as minimum
   }
   if (*alt < min2) {           // middle element is smaller
     min2 = *alt;               // update minimum
     locs[2] = 1;               // flags middle element as minimum
   }
   if (row < ny - 1 && alt[nx] < min2) { // not in top row and
                                         // top element is smaller
     min2 = alt[nx];            // update minimum
     locs[2] = 2;               // flag top element as minimum
   }

   // strategy: now min2 / locs[2] contain info on the current unit
   // (column 2), min1 / locs[1] on the previous unit (column 1),
   // and min0 / locs[0] on two units back (column 0).  All in all
   // we now have info on a block of 3 by 3 elements.  We determine
   // which one of the nine elements is the lowest one

   min = min0;                  // info from column 0
   loc = 0;                     // flag left column as lowest one
   if (min1 < min) {            // middle column has smaller minimum
     min = min1;                // update minimum
     loc = 1;                   // flag middle column as lowest one
   }
   if (min2 < min) {            // right column has smaller minimum
     min = min2;                // update minimum
     loc = 2;                   // flag right column as lowest one
   }
   if (!(*wsh++ = loc - 1 + (locs[loc] - 1)*nx)) // offset from middle
                                // element to lowest neighbor
     nsinks++;                  // if no offset, then we found a sink
   col++;                       // go to next element
   alt++;
   if (col == nx - 1) {                 // at right edge
     min = min1;
     loc = 1;
     if (min2 < min) {
       min = min2;
       loc = 2;
     }
     if (!(*wsh++ = loc - 2 + (locs[loc] - 1)*nx))
       nsinks++;
     col = 0;
     row++;
     if (row == ny)
       end = 1;
   }
 }
 switch (mode) {
 case 0: case 1:
   wsh = LPTR(h);               // back to start of result map
   n = n0 = nx*ny;
   row = col = 0;
   if (!(code = (int32_t *) malloc(sizeof(int32_t)*nsinks)))
     return cerror(ALLOC_ERR, 0);
   if (ny > nx)
     nx = ny;
   nx += 2;                     // we label the sinks with a number;
                                // nx is the number for the next sink
   i = nx;
   nx0 = nx - 1;
   do {
     col = row;
     while (wsh[col] && wsh[col] < nx) // no sink or old track
       col += wsh[col];                 // current offset
     if (!wsh[col]) {           // sink
       code[i - nx] = col;      // result for code i
       wsh[col] = iq = i++;
       n--;
     } else
       iq = wsh[col];           // iq = current code
     col = row++;               // start again, enter code
     while (wsh[col] < nx) {    // no old track
       ny = wsh[col];
       wsh[col] = iq;
       col += ny;
       n--;
     }
   } while (n);
   if (mode == 1)
     // now put in offset of sinks in stead of codes
     while (n0--) {
       *wsh = code[*wsh - nx];
       wsh++;
     } else while (n0--) {
       *wsh = *wsh - nx0;
       wsh++;
     }
   free(code);
   break;
 }
 return result_sym;
}
//----------------------------------------------------------------------
int32_t lux_basin2(int32_t narg, int32_t ps[])
/* Returns a basin map derived from "altitude" map <data>
   Syntax:  y = basin(data [, sign, /DIFFERENCE,/SINK,/NUMBER])
      <data>: data array
      <sign>: sign of sought basins: +1 -> "mountains", -1 -> "basins"
              defaults to -1.
   /NUMBER -> assign different number to each basin, starting
              with number 1 and increasing in steps of one.
   /SINK -> assign index to <im> of the final sink corresponding
              to each pixel.  Similar to 1, but with different labels for
              each basin
   /DIFFERENCE -> return index offset of steepest-descent
              neighbor for each pixel
   LS 19feb93 9may97 24jun98 */
{
  int32_t       result, mode, n, i, j, k, *offsets, *rcoords, edge = 0,
    mini, loc[3], nel, label = 0, sign, maxi = 0;
  Pointer       src, trgt, trgt0;
  Scalar        min[3], max[3];
  extern struct boundsStruct    bounds;
  LoopInfo      srcinfo, trgtinfo;

  if (narg == 1 && symbolIsNumericalArray(ps[0]) && array_num_dims(ps[0]) == 2)
    return lux_basin(narg, ps);         // use old (but faster) routine

  // check <data>, create output symbol, and prepare for walk through
  // <data>
  if (symbol_type(ps[0]) != LUX_FLOAT)
    return luxerror("Sorry, must be FLOAT at the moment", ps[0]);

  if (standardLoop(ps[0], 0, SL_ALLAXES | SL_SAMEDIMS | SL_EACHCOORD | SL_EXACT,
                   LUX_INT32,
                   &srcinfo, &src, &result, &trgtinfo, &trgt) < 0)
    return LUX_ERROR;
  trgt0 = trgt;
  nel = array_size(ps[0]);

  if (narg > 1 && ps[1]) {      // SIGN
    sign = int_arg(ps[1]);
    sign = (sign < 0)? 0: 0x20;
  } else
    sign = 0;

  // treat keyword switches
  switch (internalMode & 7) {
    case 1:
      mode = 0;                         // /NUMBER
      break;
    case 2:
      mode = 1;                         // /SINK
      break;
    case 4:
      mode = 2;                         // /DIFFERENCE
      break;
    default:
      mode = 0;
      break;
  }

  // we calculate the index offsets for all directions we want to
  // investigate, which means all directions with the first dimension
  // equal to +1.
  srcinfo.coords[0] = 1;                // set first dimension equal to +1
  // calculate the number of nearest-neighbor data points with their
  // first dimension equal to +1
  n = 1;
  for (i = 1; i < srcinfo.ndim; i++)
    n *= 3;
  offsets = (int32_t *) malloc(n*sizeof(int32_t));
  rcoords = (int32_t *) malloc(n*(srcinfo.ndim - 1)*sizeof(int32_t));
  if (!offsets || !rcoords)
    return cerror(ALLOC_ERR, 0);
  for (k = 0; k < n; k++) {
    offsets[k] = 1;
    for (j = 1; j < srcinfo.ndim; j++)
                                // calculate index offset
      offsets[k] += srcinfo.coords[j]*srcinfo.singlestep[srcinfo.raxes[j]];
    memcpy(rcoords + k*(srcinfo.ndim - 1), &srcinfo.coords[1],
           (srcinfo.ndim - 1)*sizeof(int32_t));
    for (j = 1; j < srcinfo.ndim; j++) { // to next direction: don't change
                                // the first dimension
      srcinfo.coords[j]++;
      if (srcinfo.coords[j] < 2)
        break;
      for (i = 1; i <= j; i++)
        srcinfo.coords[i] = -1;
    }
  }
  zerobytes(srcinfo.coords, srcinfo.ndim*sizeof(int32_t)); // back to zeros

  mini = 2;
  switch (symbol_type(ps[0]) | sign) {
    case LUX_INT8:
      min[0].ui8 = min[1].ui8 = bounds.max.ui8;
      do {
        min[2].ui8 = min[1].ui8;
        min[1].ui8 = min[0].ui8;
        loc[2] = loc[1];
        loc[1] = loc[0];
        min[0].ui8 = bounds.max.ui8;
        mini++;
        if (!srcinfo.coords[0]) {       // at left edge
          src.ui8--;
          min[2].ui8 = min[1].ui8 = bounds.max.ui8;
          mini = 3;
          edge = 0;
          for (i = 1; i < srcinfo.ndim; i++)
            if (!srcinfo.coords[i]
                || srcinfo.coords[i] == srcinfo.dims[i] - 1) {
              edge = 1;
              break;
            }
          if (edge)
            for (i = 0; i < n; i++) { // all directions
              for (j = 0; j < srcinfo.ndim - 1; j++) {
                // check if across edge
                k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
                if (k < 0 || k == srcinfo.dims[j + 1])
                  break;
              }
              if (j == srcinfo.ndim - 1) {      // not across any other edge
                if (src.ui8[offsets[i]] < min[0].ui8) { // update
                  min[0].ui8 = src.ui8[offsets[i]];
                  loc[0] = i;
                }
              }
            } else for (i = 0; i < n; i++) {
              if (src.ui8[offsets[i]] < min[0].ui8) { // update
                min[0].ui8 = src.ui8[offsets[i]];
                loc[0] = i;
              }
            }
          if (mini < 3) {       // previous lowest is now in 1 or 2
            if (min[0].ui8 < min[mini].ui8)
              mini = 0;
          } else {              // previous lowest is too far away: find new
            if (min[0].ui8 < min[1].ui8) {
              if (min[0].ui8 < min[2].ui8)
                mini = 0;
              else
                mini = 2;
            } else {
              if (min[1].ui8 < min[2].ui8)
                mini = 1;
              else
                mini = 2;
            }
          }
          src.ui8++;              // back to current position
          min[2].ui8 = min[1].ui8;
          min[1].ui8 = min[0].ui8;
          loc[2] = loc[1];
          loc[1] = loc[0];
          min[0].ui8 = bounds.max.ui8;
          mini++;
        }

        if (srcinfo.coords[0] == srcinfo.dims[0] - 1) { // at right edge
          if (mini == 3) {
            if (min[1].ui8 < min[2].ui8)
              mini = 1;
            else
              mini = 2;
          }
        } else {
          if (edge)
            for (i = 0; i < n; i++) {   // all directions
              for (j = 0; j < srcinfo.ndim - 1; j++) {
                // check if across edge
                k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
                if (k < 0 || k == srcinfo.dims[j + 1])
                  break;
              }
              if (j == srcinfo.ndim - 1) {      // not across any edge
                if (src.ui8[offsets[i]] < min[0].ui8) { // update
                  min[0].ui8 = src.ui8[offsets[i]];
                  loc[0] = i;
                }
              }
            }
          else for (i = 0; i < n; i++)
            if (src.ui8[offsets[i]] < min[0].ui8) { // update
              min[0].ui8 = src.ui8[offsets[i]];
              loc[0] = i;
            }
          if (mini < 3) {       // previous lowest is now in 1 or 2
            if (min[0].ui8 < min[mini].ui8)
              mini = 0;
          } else {              // previous lowest is too far away: find new
            if (min[0].ui8 < min[1].ui8) {
              if (min[0].ui8 < min[2].ui8)
                mini = 0;
              else
                mini = 2;
            } else {
              if (min[1].ui8 < min[2].ui8)
                mini = 1;
              else
                mini = 2;
            }
          }
        }
        // now min[mini].ui8 is the lowest value
        *trgt.l = -loc[mini] - n*mini;
      } while (trgtinfo.advanceLoop(&trgt),
               srcinfo.advanceLoop(&src) < srcinfo.rndim);
      break;
    case LUX_INT16:
      min[0].i16 = min[1].i16 = bounds.max.i16;
      do {
        min[2].i16 = min[1].i16;
        min[1].i16 = min[0].i16;
        loc[2] = loc[1];
        loc[1] = loc[0];
        min[0].i16 = bounds.max.i16;
        mini++;
        if (!srcinfo.coords[0]) {       // at left edge
          src.i16--;
          min[2].i16 = min[1].i16 = bounds.max.i16;
          mini = 3;
          edge = 0;
          for (i = 1; i < srcinfo.ndim; i++)
            if (!srcinfo.coords[i]
                || srcinfo.coords[i] == srcinfo.dims[i] - 1) {
              edge = 1;
              break;
            }
          if (edge)
            for (i = 0; i < n; i++) { // all directions
              for (j = 0; j < srcinfo.ndim - 1; j++) {
                // check if across edge
                k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
                if (k < 0 || k == srcinfo.dims[j + 1])
                  break;
              }
              if (j == srcinfo.ndim - 1) {      // not across any other edge
                if (src.i16[offsets[i]] < min[0].i16) { // update
                  min[0].i16 = src.i16[offsets[i]];
                  loc[0] = i;
                }
              }
            } else for (i = 0; i < n; i++) {
              if (src.i16[offsets[i]] < min[0].i16) { // update
                min[0].i16 = src.i16[offsets[i]];
                loc[0] = i;
              }
            }
          if (mini < 3) {       // previous lowest is now in 1 or 2
            if (min[0].i16 < min[mini].i16)
              mini = 0;
          } else {              // previous lowest is too far away: find new
            if (min[0].i16 < min[1].i16) {
              if (min[0].i16 < min[2].i16)
                mini = 0;
              else
                mini = 2;
            } else {
              if (min[1].i16 < min[2].i16)
                mini = 1;
              else
                mini = 2;
            }
          }
          src.i16++;              // back to current position
          min[2].i16 = min[1].i16;
          min[1].i16 = min[0].i16;
          loc[2] = loc[1];
          loc[1] = loc[0];
          min[0].i16 = bounds.max.i16;
          mini++;
        }

        if (srcinfo.coords[0] == srcinfo.dims[0] - 1) { // at right edge
          if (mini == 3) {
            if (min[1].i16 < min[2].i16)
              mini = 1;
            else
              mini = 2;
          }
        } else {
          if (edge)
            for (i = 0; i < n; i++) {   // all directions
              for (j = 0; j < srcinfo.ndim - 1; j++) {
                // check if across edge
                k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
                if (k < 0 || k == srcinfo.dims[j + 1])
                  break;
              }
              if (j == srcinfo.ndim - 1) {      // not across any edge
                if (src.i16[offsets[i]] < min[0].i16) { // update
                  min[0].i16 = src.i16[offsets[i]];
                  loc[0] = i;
                }
              }
            }
          else for (i = 0; i < n; i++)
            if (src.i16[offsets[i]] < min[0].i16) { // update
              min[0].i16 = src.i16[offsets[i]];
              loc[0] = i;
            }
          if (mini < 3) {       // previous lowest is now in 1 or 2
            if (min[0].i16 < min[mini].i16)
              mini = 0;
          } else {              // previous lowest is too far away: find new
            if (min[0].i16 < min[1].i16) {
              if (min[0].i16 < min[2].i16)
                mini = 0;
              else
                mini = 2;
            } else {
              if (min[1].i16 < min[2].i16)
                mini = 1;
              else
                mini = 2;
            }
          }
        }
        // now min[mini].i16 is the lowest value
        *trgt.l = -loc[mini] - n*mini;
      } while (trgtinfo.advanceLoop(&trgt),
               srcinfo.advanceLoop(&src) < srcinfo.rndim);
      break;
    case LUX_INT32:
      min[0].l = min[1].l = bounds.max.l;
      do {
        min[2].l = min[1].l;
        min[1].l = min[0].l;
        loc[2] = loc[1];
        loc[1] = loc[0];
        min[0].l = bounds.max.l;
        mini++;
        if (!srcinfo.coords[0]) {       // at left edge
          src.l--;
          min[2].l = min[1].l = bounds.max.l;
          mini = 3;
          edge = 0;
          for (i = 1; i < srcinfo.ndim; i++)
            if (!srcinfo.coords[i]
                || srcinfo.coords[i] == srcinfo.dims[i] - 1) {
              edge = 1;
              break;
            }
          if (edge)
            for (i = 0; i < n; i++) { // all directions
              for (j = 0; j < srcinfo.ndim - 1; j++) {
                // check if across edge
                k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
                if (k < 0 || k == srcinfo.dims[j + 1])
                  break;
              }
              if (j == srcinfo.ndim - 1) {      // not across any other edge
                if (src.l[offsets[i]] < min[0].l) { // update
                  min[0].l = src.l[offsets[i]];
                  loc[0] = i;
                }
              }
            } else for (i = 0; i < n; i++) {
              if (src.l[offsets[i]] < min[0].l) { // update
                min[0].l = src.l[offsets[i]];
                loc[0] = i;
              }
            }
          if (mini < 3) {       // previous lowest is now in 1 or 2
            if (min[0].l < min[mini].l)
              mini = 0;
          } else {              // previous lowest is too far away: find new
            if (min[0].l < min[1].l) {
              if (min[0].l < min[2].l)
                mini = 0;
              else
                mini = 2;
            } else {
              if (min[1].l < min[2].l)
                mini = 1;
              else
                mini = 2;
            }
          }
          src.l++;              // back to current position
          min[2].l = min[1].l;
          min[1].l = min[0].l;
          loc[2] = loc[1];
          loc[1] = loc[0];
          min[0].l = bounds.max.l;
          mini++;
        }

        if (srcinfo.coords[0] == srcinfo.dims[0] - 1) { // at right edge
          if (mini == 3) {
            if (min[1].l < min[2].l)
              mini = 1;
            else
              mini = 2;
          }
        } else {
          if (edge)
            for (i = 0; i < n; i++) {   // all directions
              for (j = 0; j < srcinfo.ndim - 1; j++) {
                // check if across edge
                k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
                if (k < 0 || k == srcinfo.dims[j + 1])
                  break;
              }
              if (j == srcinfo.ndim - 1) {      // not across any edge
                if (src.l[offsets[i]] < min[0].l) { // update
                  min[0].l = src.l[offsets[i]];
                  loc[0] = i;
                }
              }
            }
          else for (i = 0; i < n; i++)
            if (src.l[offsets[i]] < min[0].l) { // update
              min[0].l = src.l[offsets[i]];
              loc[0] = i;
            }
          if (mini < 3) {       // previous lowest is now in 1 or 2
            if (min[0].l < min[mini].l)
              mini = 0;
          } else {              // previous lowest is too far away: find new
            if (min[0].l < min[1].l) {
              if (min[0].l < min[2].l)
                mini = 0;
              else
                mini = 2;
            } else {
              if (min[1].l < min[2].l)
                mini = 1;
              else
                mini = 2;
            }
          }
        }
        // now min[mini].l is the lowest value
        *trgt.l = -loc[mini] - n*mini;
      } while (trgtinfo.advanceLoop(&trgt),
               srcinfo.advanceLoop(&src) < srcinfo.rndim);
      break;
    case LUX_INT64:
      min[0].q = min[1].q = bounds.max.q;
      do {
        min[2].q = min[1].q;
        min[1].q = min[0].q;
        loc[2] = loc[1];
        loc[1] = loc[0];
        min[0].q = bounds.max.q;
        mini++;
        if (!srcinfo.coords[0]) {       // at left edge
          src.q--;
          min[2].q = min[1].q = bounds.max.q;
          mini = 3;
          edge = 0;
          for (i = 1; i < srcinfo.ndim; i++)
            if (!srcinfo.coords[i]
                || srcinfo.coords[i] == srcinfo.dims[i] - 1) {
              edge = 1;
              break;
            }
          if (edge)
            for (i = 0; i < n; i++) { // all directions
              for (j = 0; j < srcinfo.ndim - 1; j++) {
                // check if across edge
                k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
                if (k < 0 || k == srcinfo.dims[j + 1])
                  break;
              }
              if (j == srcinfo.ndim - 1) {      // not across any other edge
                if (src.q[offsets[i]] < min[0].q) { // update
                  min[0].q = src.q[offsets[i]];
                  loc[0] = i;
                }
              }
            } else for (i = 0; i < n; i++) {
              if (src.q[offsets[i]] < min[0].q) { // update
                min[0].q = src.q[offsets[i]];
                loc[0] = i;
              }
            }
          if (mini < 3) {       // previous lowest is now in 1 or 2
            if (min[0].q < min[mini].q)
              mini = 0;
          } else {              // previous lowest is too far away: find new
            if (min[0].q < min[1].q) {
              if (min[0].q < min[2].q)
                mini = 0;
              else
                mini = 2;
            } else {
              if (min[1].q < min[2].q)
                mini = 1;
              else
                mini = 2;
            }
          }
          src.q++;              // back to current position
          min[2].q = min[1].q;
          min[1].q = min[0].q;
          loc[2] = loc[1];
          loc[1] = loc[0];
          min[0].q = bounds.max.q;
          mini++;
        }

        if (srcinfo.coords[0] == srcinfo.dims[0] - 1) { // at right edge
          if (mini == 3) {
            if (min[1].q < min[2].q)
              mini = 1;
            else
              mini = 2;
          }
        } else {
          if (edge)
            for (i = 0; i < n; i++) {   // all directions
              for (j = 0; j < srcinfo.ndim - 1; j++) {
                // check if across edge
                k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
                if (k < 0 || k == srcinfo.dims[j + 1])
                  break;
              }
              if (j == srcinfo.ndim - 1) {      // not across any edge
                if (src.q[offsets[i]] < min[0].q) { // update
                  min[0].q = src.q[offsets[i]];
                  loc[0] = i;
                }
              }
            }
          else for (i = 0; i < n; i++)
            if (src.q[offsets[i]] < min[0].q) { // update
              min[0].q = src.q[offsets[i]];
              loc[0] = i;
            }
          if (mini < 3) {       // previous lowest is now in 1 or 2
            if (min[0].q < min[mini].q)
              mini = 0;
          } else {              // previous lowest is too far away: find new
            if (min[0].q < min[1].q) {
              if (min[0].q < min[2].q)
                mini = 0;
              else
                mini = 2;
            } else {
              if (min[1].q < min[2].q)
                mini = 1;
              else
                mini = 2;
            }
          }
        }
        // now min[mini].q is the lowest value
        *trgt.l = -loc[mini] - n*mini;
      } while (trgtinfo.advanceLoop(&trgt),
               srcinfo.advanceLoop(&src) < srcinfo.rndim);
      break;
    case LUX_FLOAT:
      min[0].f = min[1].f = bounds.max.f;
      do {
        min[2].f = min[1].f;
        min[1].f = min[0].f;
        loc[2] = loc[1];
        loc[1] = loc[0];
        min[0].f = bounds.max.f;
        mini++;
        if (!srcinfo.coords[0]) {       // at left edge
          src.f--;
          min[2].f = min[1].f = bounds.max.f;
          mini = 3;
          edge = 0;
          for (i = 1; i < srcinfo.ndim; i++)
            if (!srcinfo.coords[i]
                || srcinfo.coords[i] == srcinfo.dims[i] - 1) {
              edge = 1;
              break;
            }
          if (edge)
            for (i = 0; i < n; i++) { // all directions
              for (j = 0; j < srcinfo.ndim - 1; j++) {
                // check if across edge
                k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
                if (k < 0 || k == srcinfo.dims[j + 1])
                  break;
              }
              if (j == srcinfo.ndim - 1) {      // not across any other edge
                if (src.f[offsets[i]] < min[0].f) { // update
                  min[0].f = src.f[offsets[i]];
                  loc[0] = i;
                }
              }
            } else for (i = 0; i < n; i++) {
              if (src.f[offsets[i]] < min[0].f) { // update
                min[0].f = src.f[offsets[i]];
                loc[0] = i;
              }
            }
          if (mini < 3) {       // previous lowest is now in 1 or 2
            if (min[0].f < min[mini].f)
              mini = 0;
          } else {              // previous lowest is too far away: find new
            if (min[0].f < min[1].f) {
              if (min[0].f < min[2].f)
                mini = 0;
              else
                mini = 2;
            } else {
              if (min[1].f < min[2].f)
                mini = 1;
              else
                mini = 2;
            }
          }
          src.f++;              // back to current position
          min[2].f = min[1].f;
          min[1].f = min[0].f;
          loc[2] = loc[1];
          loc[1] = loc[0];
          min[0].f = bounds.max.f;
          mini++;
        }

        if (srcinfo.coords[0] == srcinfo.dims[0] - 1) { // at right edge
          if (mini == 3) {
            if (min[1].f < min[2].f)
              mini = 1;
            else
              mini = 2;
          }
        } else {
          if (edge)
            for (i = 0; i < n; i++) {   // all directions
              for (j = 0; j < srcinfo.ndim - 1; j++) {
                // check if across edge
                k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
                if (k < 0 || k == srcinfo.dims[j + 1])
                  break;
              }
              if (j == srcinfo.ndim - 1) {      // not across any edge
                if (src.f[offsets[i]] < min[0].f) { // update
                  min[0].f = src.f[offsets[i]];
                  loc[0] = i;
                }
              }
            }
          else for (i = 0; i < n; i++)
            if (src.f[offsets[i]] < min[0].f) { // update
              min[0].f = src.f[offsets[i]];
              loc[0] = i;
            }
          if (mini < 3) {       // previous lowest is now in 1 or 2
            if (min[0].f < min[mini].f)
              mini = 0;
          } else {              // previous lowest is too far away: find new
            if (min[0].f < min[1].f) {
              if (min[0].f < min[2].f)
                mini = 0;
              else
                mini = 2;
            } else {
              if (min[1].f < min[2].f)
                mini = 1;
              else
                mini = 2;
            }
          }
        }
        // now min[mini].f is the lowest value
        *trgt.l = -loc[mini] - n*mini;
      } while (trgtinfo.advanceLoop(&trgt),
               srcinfo.advanceLoop(&src) < srcinfo.rndim);
      break;
    case LUX_DOUBLE:
      min[0].d = min[1].d = bounds.max.d;
      do {
        min[2].d = min[1].d;
        min[1].d = min[0].d;
        loc[2] = loc[1];
        loc[1] = loc[0];
        min[0].d = bounds.max.d;
        mini++;
        if (!srcinfo.coords[0]) {       // at left edge
          src.d--;
          min[2].d = min[1].d = bounds.max.d;
          mini = 3;
          edge = 0;
          for (i = 1; i < srcinfo.ndim; i++)
            if (!srcinfo.coords[i]
                || srcinfo.coords[i] == srcinfo.dims[i] - 1) {
              edge = 1;
              break;
            }
          if (edge)
            for (i = 0; i < n; i++) { // all directions
              for (j = 0; j < srcinfo.ndim - 1; j++) {
                // check if across edge
                k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
                if (k < 0 || k == srcinfo.dims[j + 1])
                  break;
              }
              if (j == srcinfo.ndim - 1) {      // not across any other edge
                if (src.d[offsets[i]] < min[0].d) { // update
                  min[0].d = src.d[offsets[i]];
                  loc[0] = i;
                }
              }
            } else for (i = 0; i < n; i++) {
              if (src.d[offsets[i]] < min[0].d) { // update
                min[0].d = src.d[offsets[i]];
                loc[0] = i;
              }
            }
          if (mini < 3) {       // previous lowest is now in 1 or 2
            if (min[0].d < min[mini].d)
              mini = 0;
          } else {              // previous lowest is too far away: find new
            if (min[0].d < min[1].d) {
              if (min[0].d < min[2].d)
                mini = 0;
              else
                mini = 2;
            } else {
              if (min[1].d < min[2].d)
                mini = 1;
              else
                mini = 2;
            }
          }
          src.d++;              // back to current position
          min[2].d = min[1].d;
          min[1].d = min[0].d;
          loc[2] = loc[1];
          loc[1] = loc[0];
          min[0].d = bounds.max.d;
          mini++;
        }

        if (srcinfo.coords[0] == srcinfo.dims[0] - 1) { // at right edge
          if (mini == 3) {
            if (min[1].d < min[2].d)
              mini = 1;
            else
              mini = 2;
          }
        } else {
          if (edge)
            for (i = 0; i < n; i++) {   // all directions
              for (j = 0; j < srcinfo.ndim - 1; j++) {
                // check if across edge
                k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
                if (k < 0 || k == srcinfo.dims[j + 1])
                  break;
              }
              if (j == srcinfo.ndim - 1) {      // not across any edge
                if (src.d[offsets[i]] < min[0].d) { // update
                  min[0].d = src.d[offsets[i]];
                  loc[0] = i;
                }
              }
            }
          else for (i = 0; i < n; i++)
            if (src.d[offsets[i]] < min[0].d) { // update
              min[0].d = src.d[offsets[i]];
              loc[0] = i;
            }
          if (mini < 3) {       // previous lowest is now in 1 or 2
            if (min[0].d < min[mini].d)
              mini = 0;
          } else {              // previous lowest is too far away: find new
            if (min[0].d < min[1].d) {
              if (min[0].d < min[2].d)
                mini = 0;
              else
                mini = 2;
            } else {
              if (min[1].d < min[2].d)
                mini = 1;
              else
                mini = 2;
            }
          }
        }
        // now min[mini].d is the lowest value
        *trgt.l = -loc[mini] - n*mini;
      } while (trgtinfo.advanceLoop(&trgt),
               srcinfo.advanceLoop(&src) < srcinfo.rndim);
      break;

    case LUX_INT8 | 0x20:
      max[0].ui8 = max[1].ui8 = bounds.min.ui8;
      do {
        max[2].ui8 = max[1].ui8;
        max[1].ui8 = max[0].ui8;
        loc[2] = loc[1];
        loc[1] = loc[0];
        max[0].ui8 = bounds.min.ui8;
        maxi++;
        if (!srcinfo.coords[0]) {       // at left edge
          src.ui8--;
          max[2].ui8 = max[1].ui8 = bounds.min.ui8;
          maxi = 3;
          edge = 0;
          for (i = 1; i < srcinfo.ndim; i++)
            if (!srcinfo.coords[i]
                || srcinfo.coords[i] == srcinfo.dims[i] - 1) {
              edge = 1;
              break;
            }
          if (edge)
            for (i = 0; i < n; i++) { // all directions
              for (j = 0; j < srcinfo.ndim - 1; j++) {
                // check if across edge
                k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
                if (k < 0 || k == srcinfo.dims[j + 1])
                  break;
              }
              if (j == srcinfo.ndim - 1) {      // not across any other edge
                if (src.ui8[offsets[i]] > max[0].ui8) { // update
                  max[0].ui8 = src.ui8[offsets[i]];
                  loc[0] = i;
                }
              }
            } else for (i = 0; i < n; i++) {
              if (src.ui8[offsets[i]] > max[0].ui8) { // update
                max[0].ui8 = src.ui8[offsets[i]];
                loc[0] = i;
              }
            }
          if (maxi < 3) {       // previous highest is now in 1 or 2
            if (max[0].ui8 > max[maxi].ui8)
              maxi = 0;
          } else {              // previous highest is too far away: find new
            if (max[0].ui8 > max[1].ui8) {
              if (max[0].ui8 > max[2].ui8)
                maxi = 0;
              else
                maxi = 2;
            } else {
              if (max[1].ui8 > max[2].ui8)
                maxi = 1;
              else
                maxi = 2;
            }
          }
          src.ui8++;              // back to current position
          max[2].ui8 = max[1].ui8;
          max[1].ui8 = max[0].ui8;
          loc[2] = loc[1];
          loc[1] = loc[0];
          max[0].ui8 = bounds.min.ui8;
          maxi++;
        }

        if (srcinfo.coords[0] == srcinfo.dims[0] - 1) { // at right edge
          if (maxi == 3) {
            if (max[1].ui8 > max[2].ui8)
              maxi = 1;
            else
              maxi = 2;
          }
        } else {
          if (edge)
            for (i = 0; i < n; i++) {   // all directions
              for (j = 0; j < srcinfo.ndim - 1; j++) {
                // check if across edge
                k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
                if (k < 0 || k == srcinfo.dims[j + 1])
                  break;
              }
              if (j == srcinfo.ndim - 1) {      // not across any edge
                if (src.ui8[offsets[i]] > max[0].ui8) { // update
                  max[0].ui8 = src.ui8[offsets[i]];
                  loc[0] = i;
                }
              }
            }
          else for (i = 0; i < n; i++)
            if (src.ui8[offsets[i]] > max[0].ui8) { // update
              max[0].ui8 = src.ui8[offsets[i]];
              loc[0] = i;
            }
          if (maxi < 3) {       // previous highest is now in 1 or 2
            if (max[0].ui8 > max[maxi].ui8)
              maxi = 0;
          } else { // previous highest is too far away: find new
            if (max[0].ui8 > max[1].ui8) {
              if (max[0].ui8 > max[2].ui8)
                maxi = 0;
              else
                maxi = 2;
            } else {
              if (max[1].ui8 > max[2].ui8)
                maxi = 1;
              else
                maxi = 2;
            }
          }
        }
        // now max[maxi].ui8 is the highest value
        *trgt.l = -loc[maxi] - n*maxi;
      } while (trgtinfo.advanceLoop(&trgt),
               srcinfo.advanceLoop(&src) < srcinfo.rndim);
      break;
    case LUX_INT16 | 0x20:
      max[0].i16 = max[1].i16 = bounds.min.i16;
      do {
        max[2].i16 = max[1].i16;
        max[1].i16 = max[0].i16;
        loc[2] = loc[1];
        loc[1] = loc[0];
        max[0].i16 = bounds.min.i16;
        maxi++;
        if (!srcinfo.coords[0]) {       // at left edge
          src.i16--;
          max[2].i16 = max[1].i16 = bounds.min.i16;
          maxi = 3;
          edge = 0;
          for (i = 1; i < srcinfo.ndim; i++)
            if (!srcinfo.coords[i]
                || srcinfo.coords[i] == srcinfo.dims[i] - 1) {
              edge = 1;
              break;
            }
          if (edge)
            for (i = 0; i < n; i++) { // all directions
              for (j = 0; j < srcinfo.ndim - 1; j++) {
                // check if across edge
                k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
                if (k < 0 || k == srcinfo.dims[j + 1])
                  break;
              }
              if (j == srcinfo.ndim - 1) {      // not across any other edge
                if (src.i16[offsets[i]] > max[0].i16) { // update
                  max[0].i16 = src.i16[offsets[i]];
                  loc[0] = i;
                }
              }
            } else for (i = 0; i < n; i++) {
              if (src.i16[offsets[i]] > max[0].i16) { // update
                max[0].i16 = src.i16[offsets[i]];
                loc[0] = i;
              }
            }
          if (maxi < 3) {       // previous highest is now in 1 or 2
            if (max[0].i16 > max[maxi].i16)
              maxi = 0;
          } else { // previous highest is too far away: find new
            if (max[0].i16 > max[1].i16) {
              if (max[0].i16 > max[2].i16)
                maxi = 0;
              else
                maxi = 2;
            } else {
              if (max[1].i16 > max[2].i16)
                maxi = 1;
              else
                maxi = 2;
            }
          }
          src.i16++;              // back to current position
          max[2].i16 = max[1].i16;
          max[1].i16 = max[0].i16;
          loc[2] = loc[1];
          loc[1] = loc[0];
          max[0].i16 = bounds.min.i16;
          maxi++;
        }

        if (srcinfo.coords[0] == srcinfo.dims[0] - 1) { // at right edge
          if (maxi == 3) {
            if (max[1].i16 > max[2].i16)
              maxi = 1;
            else
              maxi = 2;
          }
        } else {
          if (edge)
            for (i = 0; i < n; i++) {   // all directions
              for (j = 0; j < srcinfo.ndim - 1; j++) {
                // check if across edge
                k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
                if (k < 0 || k == srcinfo.dims[j + 1])
                  break;
              }
              if (j == srcinfo.ndim - 1) {      // not across any edge
                if (src.i16[offsets[i]] > max[0].i16) { // update
                  max[0].i16 = src.i16[offsets[i]];
                  loc[0] = i;
                }
              }
            }
          else for (i = 0; i < n; i++)
            if (src.i16[offsets[i]] > max[0].i16) { // update
              max[0].i16 = src.i16[offsets[i]];
              loc[0] = i;
            }
          if (maxi < 3) {       // previous highest is now in 1 or 2
            if (max[0].i16 > max[maxi].i16)
              maxi = 0;
          } else { // previous highest is too far away: find new
            if (max[0].i16 > max[1].i16) {
              if (max[0].i16 > max[2].i16)
                maxi = 0;
              else
                maxi = 2;
            } else {
              if (max[1].i16 > max[2].i16)
                maxi = 1;
              else
                maxi = 2;
            }
          }
        }
        // now max[maxi].i16 is the highest value
        *trgt.l = -loc[maxi] - n*maxi;
      } while (trgtinfo.advanceLoop(&trgt),
               srcinfo.advanceLoop(&src) < srcinfo.rndim);
      break;
    case LUX_INT32 | 0x20:
      max[0].l = max[1].l = bounds.min.l;
      do {
        max[2].l = max[1].l;
        max[1].l = max[0].l;
        loc[2] = loc[1];
        loc[1] = loc[0];
        max[0].l = bounds.min.l;
        maxi++;
        if (!srcinfo.coords[0]) {       // at left edge
          src.l--;
          max[2].l = max[1].l = bounds.min.l;
          maxi = 3;
          edge = 0;
          for (i = 1; i < srcinfo.ndim; i++)
            if (!srcinfo.coords[i]
                || srcinfo.coords[i] == srcinfo.dims[i] - 1) {
              edge = 1;
              break;
            }
          if (edge)
            for (i = 0; i < n; i++) { // all directions
              for (j = 0; j < srcinfo.ndim - 1; j++) {
                // check if across edge
                k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
                if (k < 0 || k == srcinfo.dims[j + 1])
                  break;
              }
              if (j == srcinfo.ndim - 1) {      // not across any other edge
                if (src.l[offsets[i]] > max[0].l) { // update
                  max[0].l = src.l[offsets[i]];
                  loc[0] = i;
                }
              }
            } else for (i = 0; i < n; i++) {
              if (src.l[offsets[i]] > max[0].l) { // update
                max[0].l = src.l[offsets[i]];
                loc[0] = i;
              }
            }
          if (maxi < 3) {       // previous highest is now in 1 or 2
            if (max[0].l > max[maxi].l)
              maxi = 0;
          } else { // previous highest is too far away: find new
            if (max[0].l > max[1].l) {
              if (max[0].l > max[2].l)
                maxi = 0;
              else
                maxi = 2;
            } else {
              if (max[1].l > max[2].l)
                maxi = 1;
              else
                maxi = 2;
            }
          }
          src.l++;              // back to current position
          max[2].l = max[1].l;
          max[1].l = max[0].l;
          loc[2] = loc[1];
          loc[1] = loc[0];
          max[0].l = bounds.min.l;
          maxi++;
        }

        if (srcinfo.coords[0] == srcinfo.dims[0] - 1) { // at right edge
          if (maxi == 3) {
            if (max[1].l > max[2].l)
              maxi = 1;
            else
              maxi = 2;
          }
        } else {
          if (edge)
            for (i = 0; i < n; i++) {   // all directions
              for (j = 0; j < srcinfo.ndim - 1; j++) {
                // check if across edge
                k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
                if (k < 0 || k == srcinfo.dims[j + 1])
                  break;
              }
              if (j == srcinfo.ndim - 1) {      // not across any edge
                if (src.l[offsets[i]] > max[0].l) { // update
                  max[0].l = src.l[offsets[i]];
                  loc[0] = i;
                }
              }
            }
          else for (i = 0; i < n; i++)
            if (src.l[offsets[i]] > max[0].l) { // update
              max[0].l = src.l[offsets[i]];
              loc[0] = i;
            }
          if (maxi < 3) {       // previous highest is now in 1 or 2
            if (max[0].l > max[maxi].l)
              maxi = 0;
          } else { // previous highest is too far away: find new
            if (max[0].l > max[1].l) {
              if (max[0].l > max[2].l)
                maxi = 0;
              else
                maxi = 2;
            } else {
              if (max[1].l > max[2].l)
                maxi = 1;
              else
                maxi = 2;
            }
          }
        }
        // now max[maxi].l is the highest value
        *trgt.l = -loc[maxi] - n*maxi;
      } while (trgtinfo.advanceLoop(&trgt),
               srcinfo.advanceLoop(&src) < srcinfo.rndim);
      break;
    case LUX_INT64 | 0x20:
      max[0].q = max[1].q = bounds.min.q;
      do {
        max[2].q = max[1].q;
        max[1].q = max[0].q;
        loc[2] = loc[1];
        loc[1] = loc[0];
        max[0].q = bounds.min.q;
        maxi++;
        if (!srcinfo.coords[0]) {       // at left edge
          src.q--;
          max[2].q = max[1].q = bounds.min.q;
          maxi = 3;
          edge = 0;
          for (i = 1; i < srcinfo.ndim; i++)
            if (!srcinfo.coords[i]
                || srcinfo.coords[i] == srcinfo.dims[i] - 1) {
              edge = 1;
              break;
            }
          if (edge)
            for (i = 0; i < n; i++) { // all directions
              for (j = 0; j < srcinfo.ndim - 1; j++) {
                // check if across edge
                k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
                if (k < 0 || k == srcinfo.dims[j + 1])
                  break;
              }
              if (j == srcinfo.ndim - 1) {      // not across any other edge
                if (src.q[offsets[i]] > max[0].q) { // update
                  max[0].q = src.q[offsets[i]];
                  loc[0] = i;
                }
              }
            } else for (i = 0; i < n; i++) {
              if (src.q[offsets[i]] > max[0].q) { // update
                max[0].q = src.q[offsets[i]];
                loc[0] = i;
              }
            }
          if (maxi < 3) {       // previous highest is now in 1 or 2
            if (max[0].q > max[maxi].q)
              maxi = 0;
          } else { // previous highest is too far away: find new
            if (max[0].q > max[1].q) {
              if (max[0].q > max[2].q)
                maxi = 0;
              else
                maxi = 2;
            } else {
              if (max[1].q > max[2].q)
                maxi = 1;
              else
                maxi = 2;
            }
          }
          src.q++;              // back to current position
          max[2].q = max[1].q;
          max[1].q = max[0].q;
          loc[2] = loc[1];
          loc[1] = loc[0];
          max[0].q = bounds.min.q;
          maxi++;
        }

        if (srcinfo.coords[0] == srcinfo.dims[0] - 1) { // at right edge
          if (maxi == 3) {
            if (max[1].q > max[2].q)
              maxi = 1;
            else
              maxi = 2;
          }
        } else {
          if (edge)
            for (i = 0; i < n; i++) {   // all directions
              for (j = 0; j < srcinfo.ndim - 1; j++) {
                // check if across edge
                k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
                if (k < 0 || k == srcinfo.dims[j + 1])
                  break;
              }
              if (j == srcinfo.ndim - 1) {      // not across any edge
                if (src.q[offsets[i]] > max[0].q) { // update
                  max[0].q = src.q[offsets[i]];
                  loc[0] = i;
                }
              }
            }
          else for (i = 0; i < n; i++)
            if (src.q[offsets[i]] > max[0].q) { // update
              max[0].q = src.q[offsets[i]];
              loc[0] = i;
            }
          if (maxi < 3) {       // previous highest is now in 1 or 2
            if (max[0].q > max[maxi].q)
              maxi = 0;
          } else { // previous highest is too far away: find new
            if (max[0].q > max[1].q) {
              if (max[0].q > max[2].q)
                maxi = 0;
              else
                maxi = 2;
            } else {
              if (max[1].q > max[2].q)
                maxi = 1;
              else
                maxi = 2;
            }
          }
        }
        // now max[maxi].q is the highest value
        *trgt.l = -loc[maxi] - n*maxi;
      } while (trgtinfo.advanceLoop(&trgt),
               srcinfo.advanceLoop(&src) < srcinfo.rndim);
      break;
    case LUX_FLOAT | 0x20:
      max[0].f = max[1].f = bounds.min.f;
      do {
        max[2].f = max[1].f;
        max[1].f = max[0].f;
        loc[2] = loc[1];
        loc[1] = loc[0];
        max[0].f = bounds.min.f;
        maxi++;
        if (!srcinfo.coords[0]) {       // at left edge
          src.f--;
          max[2].f = max[1].f = bounds.min.f;
          maxi = 3;
          edge = 0;
          for (i = 1; i < srcinfo.ndim; i++)
            if (!srcinfo.coords[i]
                || srcinfo.coords[i] == srcinfo.dims[i] - 1) {
              edge = 1;
              break;
            }
          if (edge)
            for (i = 0; i < n; i++) { // all directions
              for (j = 0; j < srcinfo.ndim - 1; j++) {
                // check if across edge
                k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
                if (k < 0 || k == srcinfo.dims[j + 1])
                  break;
              }
              if (j == srcinfo.ndim - 1) {      // not across any other edge
                if (src.f[offsets[i]] > max[0].f) { // update
                  max[0].f = src.f[offsets[i]];
                  loc[0] = i;
                }
              }
            } else for (i = 0; i < n; i++) {
              if (src.f[offsets[i]] > max[0].f) { // update
                max[0].f = src.f[offsets[i]];
                loc[0] = i;
              }
            }
          if (maxi < 3) {       // previous highest is now in 1 or 2
            if (max[0].f > max[maxi].f)
              maxi = 0;
          } else { // previous highest is too far away: find new
            if (max[0].f > max[1].f) {
              if (max[0].f > max[2].f)
                maxi = 0;
              else
                maxi = 2;
            } else {
              if (max[1].f > max[2].f)
                maxi = 1;
              else
                maxi = 2;
            }
          }
          src.f++;              // back to current position
          max[2].f = max[1].f;
          max[1].f = max[0].f;
          loc[2] = loc[1];
          loc[1] = loc[0];
          max[0].f = bounds.min.f;
          maxi++;
        }

        if (srcinfo.coords[0] == srcinfo.dims[0] - 1) { // at right edge
          if (maxi == 3) {
            if (max[1].f > max[2].f)
              maxi = 1;
            else
              maxi = 2;
          }
        } else {
          if (edge)
            for (i = 0; i < n; i++) {   // all directions
              for (j = 0; j < srcinfo.ndim - 1; j++) {
                // check if across edge
                k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
                if (k < 0 || k == srcinfo.dims[j + 1])
                  break;
              }
              if (j == srcinfo.ndim - 1) {      // not across any edge
                if (src.f[offsets[i]] > max[0].f) { // update
                  max[0].f = src.f[offsets[i]];
                  loc[0] = i;
                }
              }
            }
          else for (i = 0; i < n; i++)
            if (src.f[offsets[i]] > max[0].f) { // update
              max[0].f = src.f[offsets[i]];
              loc[0] = i;
            }
          if (maxi < 3) {       // previous highest is now in 1 or 2
            if (max[0].f > max[maxi].f)
              maxi = 0;
          } else { // previous highest is too far away: find new
            if (max[0].f > max[1].f) {
              if (max[0].f > max[2].f)
                maxi = 0;
              else
                maxi = 2;
            } else {
              if (max[1].f > max[2].f)
                maxi = 1;
              else
                maxi = 2;
            }
          }
        }
        // now max[maxi].f is the highest value
        *trgt.l = -loc[maxi] - n*maxi;
      } while (trgtinfo.advanceLoop(&trgt),
               srcinfo.advanceLoop(&src) < srcinfo.rndim);
      break;
    case LUX_DOUBLE | 0x20:
      max[0].d = max[1].d = bounds.min.d;
      do {
        max[2].d = max[1].d;
        max[1].d = max[0].d;
        loc[2] = loc[1];
        loc[1] = loc[0];
        max[0].d = bounds.min.d;
        maxi++;
        if (!srcinfo.coords[0]) {       // at left edge
          src.d--;
          max[2].d = max[1].d = bounds.min.d;
          maxi = 3;
          edge = 0;
          for (i = 1; i < srcinfo.ndim; i++)
            if (!srcinfo.coords[i]
                || srcinfo.coords[i] == srcinfo.dims[i] - 1) {
              edge = 1;
              break;
            }
          if (edge)
            for (i = 0; i < n; i++) { // all directions
              for (j = 0; j < srcinfo.ndim - 1; j++) {
                // check if across edge
                k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
                if (k < 0 || k == srcinfo.dims[j + 1])
                  break;
              }
              if (j == srcinfo.ndim - 1) {      // not across any other edge
                if (src.d[offsets[i]] > max[0].d) { // update
                  max[0].d = src.d[offsets[i]];
                  loc[0] = i;
                }
              }
            } else for (i = 0; i < n; i++) {
              if (src.d[offsets[i]] > max[0].d) { // update
                max[0].d = src.d[offsets[i]];
                loc[0] = i;
              }
            }
          if (maxi < 3) {       // previous highest is now in 1 or 2
            if (max[0].d > max[maxi].d)
              maxi = 0;
          } else { // previous highest is too far away: find new
            if (max[0].d > max[1].d) {
              if (max[0].d > max[2].d)
                maxi = 0;
              else
                maxi = 2;
            } else {
              if (max[1].d > max[2].d)
                maxi = 1;
              else
                maxi = 2;
            }
          }
          src.d++;              // back to current position
          max[2].d = max[1].d;
          max[1].d = max[0].d;
          loc[2] = loc[1];
          loc[1] = loc[0];
          max[0].d = bounds.min.d;
          maxi++;
        }

        if (srcinfo.coords[0] == srcinfo.dims[0] - 1) { // at right edge
          if (maxi == 3) {
            if (max[1].d > max[2].d)
              maxi = 1;
            else
              maxi = 2;
          }
        } else {
          if (edge)
            for (i = 0; i < n; i++) {   // all directions
              for (j = 0; j < srcinfo.ndim - 1; j++) {
                // check if across edge
                k = srcinfo.coords[j + 1] + rcoords[(srcinfo.ndim - 1)*i + j];
                if (k < 0 || k == srcinfo.dims[j + 1])
                  break;
              }
              if (j == srcinfo.ndim - 1) {      // not across any edge
                if (src.d[offsets[i]] > max[0].d) { // update
                  max[0].d = src.d[offsets[i]];
                  loc[0] = i;
                }
              }
            }
          else for (i = 0; i < n; i++)
            if (src.d[offsets[i]] > max[0].d) { // update
              max[0].d = src.d[offsets[i]];
              loc[0] = i;
            }
          if (maxi < 3) {       // previous highest is now in 1 or 2
            if (max[0].d > max[maxi].d)
              maxi = 0;
          } else { // previous highest is too far away: find new
            if (max[0].d > max[1].d) {
              if (max[0].d > max[2].d)
                maxi = 0;
              else
                maxi = 2;
            } else {
              if (max[1].d > max[2].d)
                maxi = 1;
              else
                maxi = 2;
            }
          }
        }
        // now max[maxi].d is the highest value
        *trgt.l = -loc[maxi] - n*maxi;
      } while (trgtinfo.advanceLoop(&trgt),
               srcinfo.advanceLoop(&src) < srcinfo.rndim);
      break;
  }

  trgt.f = trgt0.f;             // back to the start
  switch (mode) {
    case 0:                     // /NUMBER: assign subsequent numbers
      label = 1;
      // fall-thru
    case 1:                     // /SINK: assign label equal to sink index
      while (nel--) {
        if (*trgt.l <= 0) {     // still needs treatment
          i = 0;
          do {
            j = -trgt.l[i];
            if (j >= 0) {
              j = offsets[j % n] - (j/n);       // 0 -> found minimum
              if (j)
                i += j;
            } else
              break;
          } while (j);
          if (j)                // found one already treated: copy number
            *trgt.l = trgt.l[i];
          else {                // found sink: assign number & copy
            mini = mode? trgt.l + i - trgt0.l: label++;
            i = 0;
            do {
              j = -trgt.l[i];
              if (j >= 0) {
                trgt.l[i] = mini;
                j = offsets[j % n] - (j/n);
                if (j)
                  i += j;
              }
            } while (j > 0);
          }
        }
        trgt.l++;
      }
      break;
    case 2:                     // /DIFFERENCE: index difference
      while (nel--) {
        i = -*trgt.l;
        *trgt.l++ = offsets[i % n] - (i/n);
      }
      break;
  }
  free(rcoords);
  free(offsets);
  return result;
}
//----------------------------------------------------------------------
int32_t lux_extreme_general(int32_t narg, int32_t ps[])
/* Y = ESEGMENT(X) seeks positions of local extremes
   ESEGMENT(x [, sign, DIAGONAL=diagonal, THRESHOLD=threshold])
   <x>: data
   <sign>: sign of objects to look for, positive integer -> maximum,
    negative integer -> minimum.  If zero, then +1 is assumed.
   <threshold>: threshold for acceptance (defaults to zero)
   returns number of OK extremes per data element
   LS 18may95 4aug97 16nov98 2aug99 */
{
  int32_t       result, sign, n, i, *offset, k, j, nElem, edge,
        *diagonal, nDiagonal, n1, n2, nDoDim, i1, i2, n0, haveThreshold;
  double        zero = 0.0;
  Pointer       src, trgt, srcl, srcr, t;
  LoopInfo      srcinfo, trgtinfo;

  // gather info about ps[0] and prepare a return symbol
  if (standardLoop(ps[0], 0, SL_ALLAXES | SL_SAMEDIMS | SL_EXACT | SL_EACHROW,
                   LUX_INT8, &srcinfo, &src, &result, &trgtinfo, &trgt) < 0)
    return LUX_ERROR;
  if (symbol_class(ps[0]) != LUX_ARRAY)
    return cerror(ILL_CLASS, ps[0]);

  sign = (narg > 1 && ps[1])? int_arg(ps[1]): 1;
  sign = (sign >= 0)? 1: 0;     // 1 -> hill-like, 0 -> valley-like

  if (narg > 2 && ps[2]) {      // have <diagonal>
    if (symbol_class(ps[2]) != LUX_ARRAY)
      return cerror(NEED_ARR, ps[2]);
    if (array_size(ps[2]) != srcinfo.ndim)
      return cerror(INCMP_ARG, ps[2]);
    i = lux_long(1, &ps[2]);    // ensure LONG
    diagonal = (int32_t*) array_data(i);
    nDiagonal = nDoDim = 0;
    for (i = 0; i < srcinfo.ndim; i++)
      if (diagonal[i]) {
        nDoDim++;               /* # dimensions that are considered
                                 at all */
        if (diagonal[i] != 1)
          nDiagonal++;          /* # dimensions that allow diagonal
                                 links */
      }
  } else {
    diagonal = NULL;
    nDiagonal = nDoDim = srcinfo.ndim;
  }

  if (narg > 3 && ps[3]) {      // have threshold
    if (symbolIsRealScalar(ps[3])) { // it's valid
      i = lux_converts[symbol_type(ps[0])](1, ps + 3);
      t.ui8 = &scalar_value(i).ui8;
      haveThreshold = 1;
    } else
      return cerror(ILL_CLASS, ps[3]);
  } else {
    t.d = &zero;
    haveThreshold = 0;
  }

  /* now calculate the number of directions to treat;
     equal to (3^nDiagonal - 1)/2 + nDoDim - nDiagonal */
  n = 1;
  for (i = 0; i < nDiagonal; i++)
    n *= 3;
  n = (n - 1)/2 + nDoDim - nDiagonal;

  offset = (int32_t *) malloc(n*sizeof(int32_t)); // offsets to elements to be
                                          // investigated
  if (!offset)
    return cerror(ALLOC_ERR, 0);

  // calculate offsets to elements to be investigated
  // we only need to treat n directions
  // see local_extreme() for more info
  for (i = 0; i < srcinfo.ndim; i++)
    srcinfo.coords[i] = 0;
  srcinfo.coords[0] = 1;

  n0 = n1 = 0;
  n2 = 1;                       // defaults for when diagonal == 0
  for (k = 0; k < n; ) {
    if (diagonal) {
      n0 = n1 = n2 = 0;
      for (i = 0; i < srcinfo.ndim; i++)
        if (srcinfo.coords[i])
          switch (diagonal[i]) {
            case 0:
              n0++;
              break;
            case 1:
              n1++;
              break;
            case 2:
              n2++;
              break;
          }
    }
    if (!n0 && ((n2 && !n1) || (n1 == 1 && !n2))) {
      // OK: treat this direction
      offset[k] = 0;
      for (j = 0; j < srcinfo.ndim; j++)
        offset[k] += srcinfo.rsinglestep[j]*srcinfo.coords[j];
      k++;
    }
    /* go to next direction.  If this algorithm were allowed to run
     all the way to (-1,0,0), then it would have cycled through all
     directions accessible from (0,0,0).  However, in this case we
     want only one member of each pair of diametrically opposite
     directions, because we compare the central element with the
     elements adjacent to it on both (diametrically opposite) sides.
     Fortunately, the last half of the complete cycle of this
     algorithm is the negative mirror image of the first half, so
     we can limit ourselves to the correct half by stopping when
     we have reached the correct number of directions: this is
     taken care of by the "for (k = 0; k < n; )" statement.  LS 19jun98 */
    for (j = 0; j < srcinfo.ndim; j++) {
      srcinfo.coords[j]++;
      if (srcinfo.coords[j] <= 1)
        break;
      for (i = 0; i <= j; i++)
        srcinfo.coords[i] = -1;
    }
  }

  zerobytes(srcinfo.coords, srcinfo.ndim*sizeof(int32_t));
  nElem = srcinfo.dims[0];

  if (!diagonal || diagonal[0]) {
    i1 = 1;
    i2 = nElem - 1;
  } else {
    i1 = 0;
    i2 = nElem;
  }
  // now do the loop work
  if (sign && (internalMode & 2) == 0 && !diagonal && !haveThreshold)
                // standard form - make as fast as possible
    switch (array_type(ps[0])) {
      case LUX_INT8:
        do {
          for (edge = srcinfo.ndim - 1; edge; edge--)
            if (!srcinfo.coords[edge]
                || srcinfo.coords[edge] == srcinfo.dims[edge] - 1)
              break;            // at edge
          if (edge) {
            zerobytes(trgt.ui8, nElem);
            trgt.ui8 += nElem;
            src.ui8 += nElem;
          } else {
            *trgt.ui8++ = 0;              // left edge
            src.ui8++;
            for (i = i1; i < i2; i++) { // center data points
              *trgt.ui8 = 0;
              for (j = 0; j < n; j++) {         /* all directions */
                k = offset[j];
                srcl.ui8 = src.ui8 + k;
                srcr.ui8 = src.ui8 - k;
                *trgt.ui8 += (*src.ui8 > *srcl.ui8 && *src.ui8 > *srcr.ui8);
              }
              src.ui8++;
              trgt.ui8++;
            }
            *trgt.ui8++ = 0;              // right edge
            src.ui8++;
          }
        } while (trgtinfo.advanceLoop(&trgt),
                 srcinfo.advanceLoop(&src) < srcinfo.ndim);
        break;
      case LUX_INT16:
        do {
          for (edge = srcinfo.ndim - 1; edge; edge--)
            if (!srcinfo.coords[edge]
                || srcinfo.coords[edge] == srcinfo.dims[edge] - 1)
              break;            // at edge
          if (edge) {
            zerobytes(trgt.ui8, nElem);
            trgt.ui8 += nElem;
            src.i16 += nElem;
          } else {
            *trgt.ui8++ = 0;              // left edge
            src.i16++;
            for (i = i1; i < i2; i++) { // center data points
              *trgt.ui8 = 0;
              for (j = 0; j < n; j++) {         /* all directions */
                k = offset[j];
                srcl.i16 = src.i16 + k;
                srcr.i16 = src.i16 - k;
                *trgt.ui8 += (*src.i16 > *srcl.i16 && *src.i16 > *srcr.i16);
              }
              src.i16++;
              trgt.ui8++;
            }
            *trgt.ui8++ = 0;              // right edge
            src.i16++;
          }
        } while (trgtinfo.advanceLoop(&trgt),
                 srcinfo.advanceLoop(&src) < srcinfo.ndim);
        break;
      case LUX_INT32:
        do {
          for (edge = srcinfo.ndim - 1; edge; edge--)
            if (!srcinfo.coords[edge]
                || srcinfo.coords[edge] == srcinfo.dims[edge] - 1)
              break;            // at edge
          if (edge) {
            zerobytes(trgt.ui8, nElem);
            trgt.ui8 += nElem;
            src.l += nElem;
          } else {
            *trgt.ui8++ = 0;              // left edge
            src.l++;
            for (i = i1; i < i2; i++) { // center data points
              *trgt.ui8 = 0;
              for (j = 0; j < n; j++) {         /* all directions */
                k = offset[j];
                srcl.l = src.l + k;
                srcr.l = src.l - k;
                *trgt.ui8 += (*src.l > *srcl.l && *src.l > *srcr.l);
              }
              src.l++;
              trgt.ui8++;
            }
            *trgt.ui8++ = 0;              // right edge
            src.l++;
          }
        } while (trgtinfo.advanceLoop(&trgt),
                 srcinfo.advanceLoop(&src) < srcinfo.ndim);
        break;
      case LUX_INT64:
        do {
          for (edge = srcinfo.ndim - 1; edge; edge--)
            if (!srcinfo.coords[edge]
                || srcinfo.coords[edge] == srcinfo.dims[edge] - 1)
              break;            // at edge
          if (edge) {
            zerobytes(trgt.ui8, nElem);
            trgt.ui8 += nElem;
            src.q += nElem;
          } else {
            *trgt.ui8++ = 0;              // left edge
            src.q++;
            for (i = i1; i < i2; i++) { // center data points
              *trgt.ui8 = 0;
              for (j = 0; j < n; j++) {         // all directions
                k = offset[j];
                srcl.q = src.q + k;
                srcr.q = src.q - k;
                *trgt.ui8 += (*src.q > *srcl.q && *src.q > *srcr.q);
              }
              src.q++;
              trgt.ui8++;
            }
            *trgt.ui8++ = 0;              // right edge
            src.q++;
          }
        } while (trgtinfo.advanceLoop(&trgt),
                 srcinfo.advanceLoop(&src) < srcinfo.ndim);
        break;
      case LUX_FLOAT:
        do {
          for (edge = srcinfo.ndim - 1; edge; edge--)
            if (!srcinfo.coords[edge]
                || srcinfo.coords[edge] == srcinfo.dims[edge] - 1)
              break;            // at edge
          if (edge) {
            zerobytes(trgt.ui8, nElem);
            trgt.ui8 += nElem;
            src.f += nElem;
          } else {
            *trgt.ui8++ = 0;              // left edge
            src.f++;
            for (i = i1; i < i2; i++) { // center data points
              *trgt.ui8 = 0;
              for (j = 0; j < n; j++) {         // all directions
                k = offset[j];
                srcl.f = src.f + k;
                srcr.f = src.f - k;
                *trgt.ui8 += (*src.f > *srcl.f && *src.f > *srcr.f);
              }
              src.f++;
              trgt.ui8++;
            }
            *trgt.ui8++ = 0;              // right edge
            src.f++;
          }
        } while (trgtinfo.advanceLoop(&trgt),
                 srcinfo.advanceLoop(&src) < srcinfo.ndim);
        break;
      case LUX_DOUBLE:
        do {
          for (edge = srcinfo.ndim - 1; edge; edge--)
            if (!srcinfo.coords[edge]
                || srcinfo.coords[edge] == srcinfo.dims[edge] - 1)
              break;            // at edge
          if (edge) {
            zerobytes(trgt.ui8, nElem);
            trgt.ui8 += nElem;
            src.d += nElem;
          } else {
            *trgt.ui8++ = 0;              // left edge
            src.d++;
            for (i = i1; i < i2; i++) { // center data points
              *trgt.ui8 = 0;
              for (j = 0; j < n; j++) {         /* all directions */
                k = offset[j];
                srcl.d = src.d + k;
                srcr.d = src.d - k;
                *trgt.ui8 += (*src.d > *srcl.d && *src.d > *srcr.d);
              }
              src.d++;
              trgt.ui8++;
            }
            *trgt.ui8++ = 0;              // right edge
            src.d++;
          }
        } while (trgtinfo.advanceLoop(&trgt),
                 srcinfo.advanceLoop(&src) < srcinfo.ndim);
        break;
    } else                              // general case - a bit slower
      switch (array_type(ps[0])) {
        case LUX_INT8:
          do {
            for (edge = srcinfo.ndim - 1; edge; edge--)
              if ((!diagonal || diagonal[edge])
                  && (!srcinfo.coords[edge]
                      || srcinfo.coords[edge] == srcinfo.dims[edge] - 1))
                break;          // at edge
            if (edge) {
              zerobytes(trgt.ui8, nElem);
              trgt.ui8 += nElem;
              src.ui8 += nElem;
            } else {
              if (!diagonal || diagonal[0]) {
                *trgt.ui8++ = 0; // left edge
                src.ui8++;
              }
              for (i = i1; i < i2; i++) { // center data points
                *trgt.ui8 = 0;
                for (j = 0; j < n; j++) { /* all directions */
                  k = offset[j];
                  srcl.ui8 = src.ui8 + k;
                  srcr.ui8 = src.ui8 - k;
                  *trgt.ui8 += ((sign && *src.ui8 > *srcl.ui8 + *t.ui8
                               && *src.ui8 > *srcr.ui8 + *t.ui8)
                              || (!sign && *src.ui8 < *srcl.ui8 - *t.ui8
                                  && *src.ui8 < *srcr.ui8 - *t.ui8));
                }
                trgt.ui8++;
                src.ui8++;
              }
              if (!diagonal || diagonal[0]) {
                *trgt.ui8++ = 0;          // right edge
                src.ui8++;
              }
            }
          } while (trgtinfo.advanceLoop(&trgt),
                   srcinfo.advanceLoop(&src) < srcinfo.ndim);
          break;
        case LUX_INT16:
          do {
            for (edge = srcinfo.ndim - 1; edge; edge--)
              if ((!diagonal || diagonal[edge])
                  && (!srcinfo.coords[edge]
                      || srcinfo.coords[edge] == srcinfo.dims[edge] - 1))
                break;          // at edge
            if (edge) {
              zerobytes(trgt.ui8, nElem);
              trgt.ui8 += nElem;
              src.i16 += nElem;
            } else {
              if (!diagonal || diagonal[0]) {
                *trgt.ui8++ = 0; // left edge
                src.i16++;
              }
              for (i = i1; i < i2; i++) { // center data points
                *trgt.ui8 = 0;
                for (j = 0; j < n; j++) { /* all directions */
                  k = offset[j];
                  srcl.i16 = src.i16 + k;
                  srcr.i16 = src.i16 - k;
                  *trgt.ui8 += ((sign && *src.i16 > *srcl.i16 + *t.i16
                               && *src.i16 > *srcr.i16 + *t.i16)
                              || (!sign && *src.i16 < *srcl.i16 - *t.i16
                                  && *src.i16 < *srcr.i16 - *t.i16));
                }
                trgt.ui8++;
                src.i16++;
              }
              if (!diagonal || diagonal[0]) {
                *trgt.ui8++ = 0;          // right edge
                src.i16++;
              }
            }
          } while (trgtinfo.advanceLoop(&trgt),
                   srcinfo.advanceLoop(&src) < srcinfo.ndim);
          break;
        case LUX_INT32:
          do {
            for (edge = srcinfo.ndim - 1; edge; edge--)
              if ((!diagonal || diagonal[edge])
                  && (!srcinfo.coords[edge]
                      || srcinfo.coords[edge] == srcinfo.dims[edge] - 1))
                break;          // at edge
            if (edge) {
              zerobytes(trgt.ui8, nElem);
              trgt.ui8 += nElem;
              src.l += nElem;
            } else {
              if (!diagonal || diagonal[0]) {
                *trgt.ui8++ = 0; // left edge
                src.l++;
              }
              for (i = i1; i < i2; i++) { // center data points
                *trgt.ui8 = 0;
                for (j = 0; j < n; j++) { /* all directions */
                  k = offset[j];
                  srcl.l = src.l + k;
                  srcr.l = src.l - k;
                  *trgt.ui8 += ((sign && *src.l > *srcl.l + *t.l
                               && *src.l > *srcr.l + *t.l)
                              || (!sign && *src.l < *srcl.l - *t.l
                                  && *src.l < *srcr.l - *t.l));
                }
                trgt.ui8++;
                src.l++;
              }
              if (!diagonal || diagonal[0]) {
                *trgt.ui8++ = 0;          // right edge
                src.l++;
              }
            }
          } while (trgtinfo.advanceLoop(&trgt),
                   srcinfo.advanceLoop(&src) < srcinfo.ndim);
          break;
        case LUX_INT64:
          do {
            for (edge = srcinfo.ndim - 1; edge; edge--)
              if ((!diagonal || diagonal[edge])
                  && (!srcinfo.coords[edge]
                      || srcinfo.coords[edge] == srcinfo.dims[edge] - 1))
                break;          // at edge
            if (edge) {
              zerobytes(trgt.ui8, nElem);
              trgt.ui8 += nElem;
              src.q += nElem;
            } else {
              if (!diagonal || diagonal[0]) {
                *trgt.ui8++ = 0; // left edge
                src.q++;
              }
              for (i = i1; i < i2; i++) { // center data points
                *trgt.ui8 = 0;
                for (j = 0; j < n; j++) { // all directions
                  k = offset[j];
                  srcl.q = src.q + k;
                  srcr.q = src.q - k;
                  *trgt.ui8 += ((sign && *src.q > *srcl.q + *t.q
                               && *src.q > *srcr.q + *t.q)
                              || (!sign && *src.q < *srcl.q - *t.q
                                  && *src.q < *srcr.q - *t.q));
                }
                trgt.ui8++;
                src.q++;
              }
              if (!diagonal || diagonal[0]) {
                *trgt.ui8++ = 0;          // right edge
                src.q++;
              }
            }
          } while (trgtinfo.advanceLoop(&trgt),
                   srcinfo.advanceLoop(&src) < srcinfo.ndim);
          break;
        case LUX_FLOAT:
          do {
            for (edge = srcinfo.ndim - 1; edge; edge--)
              if ((!diagonal || diagonal[edge])
                  && (!srcinfo.coords[edge]
                      || srcinfo.coords[edge] == srcinfo.dims[edge] - 1))
                break;          // at edge
            if (edge) {
              zerobytes(trgt.ui8, nElem);
              trgt.ui8 += nElem;
              src.f += nElem;
            } else {
              if (!diagonal || diagonal[0]) {
                *trgt.ui8++ = 0; // left edge
                src.f++;
              }
              for (i = i1; i < i2; i++) { // center data points
                *trgt.ui8 = 0;
                for (j = 0; j < n; j++) { /* all directions */
                  k = offset[j];
                  srcl.f = src.f + k;
                  srcr.f = src.f - k;
                  *trgt.ui8 += ((sign && *src.f > *srcl.f + *t.f
                               && *src.f > *srcr.f + *t.f)
                              || (!sign && *src.f < *srcl.f - *t.f
                                  && *src.f < *srcr.f - *t.f));
                }
                trgt.ui8++;
                src.f++;
              }
              if (!diagonal || diagonal[0]) {
                *trgt.ui8++ = 0;          // right edge
                src.f++;
              }
            }
          } while (trgtinfo.advanceLoop(&trgt),
                   srcinfo.advanceLoop(&src) < srcinfo.ndim);
          break;
        case LUX_DOUBLE:
          do {
            for (edge = srcinfo.ndim - 1; edge; edge--)
              if ((!diagonal || diagonal[edge])
                  && (!srcinfo.coords[edge]
                      || srcinfo.coords[edge] == srcinfo.dims[edge] - 1))
                break;          // at edge
            if (edge) {
              zerobytes(trgt.ui8, nElem);
              trgt.ui8 += nElem;
              src.d += nElem;
            } else {
              if (!diagonal || diagonal[0]) {
                *trgt.ui8++ = 0; // left edge
                src.d++;
              }
              for (i = i1; i < i2; i++) { // center data points
                *trgt.ui8 = 0;
                for (j = 0; j < n; j++) { /* all directions */
                  k = offset[j];
                  srcl.d = src.d + k;
                  srcr.d = src.d - k;
                  *trgt.ui8 += ((sign && *src.d > *srcl.d + *t.d
                               && *src.d > *srcr.d + *t.d)
                              || (!sign && *src.d < *srcl.d - *t.d
                                  && *src.d < *srcr.d - *t.d));
                }
                trgt.ui8++;
                src.d++;
              }
              if (!diagonal || diagonal[0]) {
                *trgt.ui8++ = 0;          // right edge
                src.d++;
              }
            }
          } while (trgtinfo.advanceLoop(&trgt),
                   srcinfo.advanceLoop(&src) < srcinfo.ndim);
          break;
      }

  free(offset);
  return result;
}
//----------------------------------------------------------------------
int32_t lux_inpolygon(int32_t narg, int32_t ps[])
/* INPOLYGON(x,y,lx,ly) returns the indices of those points <x,y> that
 lie within the polyhon defined by points <lx,ly>.  LS 24nov98 */
{
  int32_t       n, np, result, iq, type, *trgt, temptype, i, count, *trgt0, j;
  Pointer       x, y, lx, ly;
  Scalar        thisx, thisy, yc;

  if (!symbolIsNumericalArray(ps[0]))
    return cerror(ILL_CLASS, ps[0]);
  n = array_size(ps[0]);
  if (!symbolIsNumericalArray(ps[1])
      || array_size(ps[1]) != n)
    return cerror(INCMP_ARG, ps[1]);
  if (!symbolIsNumericalArray(ps[2]))
    return cerror(ILL_CLASS, ps[2]);
  np = array_size(ps[2]);
  if (!symbolIsNumericalArray(ps[3])
      || array_size(ps[3]) != np)
    return cerror(INCMP_ARG, ps[3]);

  result = array_clone(ps[0], LUX_INT32);
  array_num_dims(result) = 1;
  array_dims(result)[0] = n;
  trgt = trgt0 = (int32_t*) array_data(result);

  type = array_type(ps[0]);
  x.v = array_data(ps[0]);
  if (array_type(ps[1]) != type)
    iq = lux_converts[type](1, &ps[1]);
  else
    iq = ps[1];
  y.v = array_data(iq);

  temptype = (type == LUX_DOUBLE)? LUX_DOUBLE: LUX_FLOAT;

  if (array_type(ps[2]) != temptype)
    iq = lux_converts[temptype](1, &ps[2]);
  else
    iq = ps[2];
  lx.v = array_data(iq);
  if (array_type(ps[3]) != temptype)
    iq = lux_converts[temptype](1, &ps[3]);
  else
    iq = ps[3];
  ly.v = array_data(iq);

  j = 0;
  while (n--) {
    switch (type) {
      case LUX_INT8:
        thisx.f = (float) *x.ui8++;
        thisy.f = (float) *y.ui8++;
        break;
      case LUX_INT16:
        thisx.f = (float) *x.i16++;
        thisy.f = (float) *y.i16++;
        break;
      case LUX_INT32:
        thisx.f = (float) *x.l++;
        thisy.f = (float) *y.l++;
        break;
      case LUX_INT64:
        thisx.f = *x.q++;
        thisy.f = *y.q++;
        break;
      case LUX_FLOAT:
        thisx.f = *x.f++;
        thisy.f = *y.f++;
        break;
      case LUX_DOUBLE:
        thisx.d = *x.d++;
        thisy.d = *y.d++;
        break;
    }
    switch (temptype) {
      case LUX_FLOAT:
        count = 0;
        for (i = 0; i < np - 1; i++)
          if ((thisx.f >= lx.f[i] || thisx.f >= lx.f[i + 1])
              && (thisx.f < lx.f[i] || thisx.f < lx.f[i + 1])) {
            yc.f = (ly.f[i]*(lx.f[i + 1] - thisx.f)
                    + ly.f[i + 1]*(thisx.f - lx.f[i]))/(lx.f[i + 1] - lx.f[i]);
            count += (yc.f > thisy.f);
          }
        if ((thisx.f >= lx.f[np - 1] || thisx.f >= lx.f[0])
            && (thisx.f < lx.f[np - 1] || thisx.f < lx.f[0])) {
          yc.f = (ly.f[np - 1]*(lx.f[0] - thisx.f)
                  + ly.f[0]*(thisx.f - lx.f[np - 1]))/(lx.f[0] - lx.f[np - 1]);
          count += (yc.f > thisy.f);
        }
        if (count % 2)
          *trgt++ = j;
        break;
      case LUX_DOUBLE:
        count = 0;
        for (i = 0; i < np - 1; i++)
          if ((thisx.d >= lx.d[i] || thisx.d >= lx.d[i + 1])
              && (thisx.d < lx.d[i] || thisx.d < lx.d[i + 1])) {
            yc.d = (ly.d[i]*(lx.d[i + 1] - thisx.d)
                    + ly.d[i + 1]*(thisx.d - lx.d[i]))/(lx.d[i + 1] - lx.d[i]);
            count += (yc.d > thisy.d);
          }
        if ((thisx.d >= lx.d[np - 1] || thisx.d >= lx.d[0])
            && (thisx.d < lx.d[np - 1] || thisx.d < lx.d[0])) {
          yc.d = (ly.d[np - 1]*(lx.d[0] - thisx.d)
                  + ly.d[0]*(thisx.d - lx.d[np - 1]))/(lx.d[0] - lx.d[np - 1]);
          count += (yc.d > thisy.d);
        }
        if (count % 2)
          *trgt++ = j;
        break;
    }
    j++;
  }
  n = trgt - trgt0;
  if (n) {
    symbol_memory(result) = sizeof(array) + lux_type_size[temptype]*n;
    symbol_data(result) = realloc(symbol_data(result), symbol_memory(result));
    array_dims(result)[0] = n;
  } else {
    undefine(result);
    symbol_class(result) = LUX_SCALAR;
    scalar_type(result) = LUX_INT32;
    scalar_value(result).l = -1;
  }
  return result;
}
//----------------------------------------------------------------------
int32_t         *ptr1, *ptr2;
int32_t ac_compare(const void *arg1, const void *arg2)
{
  int32_t       i1, i2, d;

  i1 = *(int32_t *) arg1;
  i2 = *(int32_t *) arg2;
  d = ptr1[i1] - ptr1[i2];
  return d? d: ptr2[i1] - ptr2[i2];
}
//----------------------------------------------------------------------
int32_t intcmp(const void *arg1, const void *arg2)
{
  return *(int32_t *) arg1 - *(int32_t *) arg2;
}
//----------------------------------------------------------------------
int32_t lux_area_connect(int32_t narg, int32_t ps[])
/* AREACONNECT(im1, im2 [, compact] [, /RAW]) determines connections
 between areas in segmented images <im1> and <im2>.  If /RAW is specified,
 then the raw links
 are returned in the form of a 2D array with 3 elements in its second
 dimension.  If that array is called r, then r(*,0) contains area
 numbers in <im1>, r(*,1) area numbers in <im2>, and r(*,2) the number
 of pixels that have those area numbers in <im1> and <im2>.  By
 default (<raw> not zero) the topological classes of the areas are
 reported in variables $app (appearing in <im2>), $disapp
 (disappearing from <im1>), $split1 (splitting in <im1>), $split2
 (split products in <im2>), $merge1 (merge components in <im1>),
 $merge2 (merge product in <im2>), and $stay (remaining topologically
 the same), and the area numbers in <im2> are modified so that
 topologically connected areas have the same area number in <im2> as
 in <im1>, as far as possible. */
{
  char  raw = 0, compact = 1;
  uint8_t       *flags;
  int32_t       n, n2, dims[2], i, result, *order, *ptr, i2, v1, v2,
    max1, max2, *ptr0, j, i0, *out1, *out2, *order2, *list,
    qapp, qdisapp, qmerge1, qmerge2, qmerge1list, qsplit1, qsplit2,
    qsplit2list, qstay1, qstay2;

  raw = (internalMode & 1);
  if (narg > 2 && ps[2])
    compact = int_arg(ps[2]);

  if (!symbolIsNumericalArray(ps[0]))
    return cerror(NEED_NUM_ARR, ps[0]);
  if (!symbolIsNumericalArray(ps[1]))
    return cerror(NEED_NUM_ARR, ps[1]);
  n = array_num_dims(ps[0]);
  for (i = 0; i < n; i++)
    if (array_dims(ps[0])[i] != array_dims(ps[1])[i])
      return cerror(INCMP_DIMS, ps[1]);
  if (array_type(ps[0]) != LUX_INT32)
    return luxerror("Need a LONG array", ps[0]);
  if (array_type(ps[1]) != LUX_INT32)
    return luxerror("Need a LONG array", ps[1]);
  ptr1 = (int32_t*) array_data(ps[0]);
  ptr2 = (int32_t*) array_data(ps[1]);
  n = array_size(ps[0]);

  // we sort the entries by *ptr1 value and secondarily by *ptr2 value
  order = (int32_t*) malloc(n*sizeof(int32_t));
  if (!order)
    return cerror(ALLOC_ERR, 0);
  for (i = 0; i < n; i++)
    order[i] = i;
  qsort(order, n, sizeof(int32_t), ac_compare); // sort
  // now ptr1[order[...]] is in ascending order, and
  // ptr2[order[...]] is in ascending order for each fixed value of
  // ptr1[order[...]]

  /* we need to count how many unique entries there are, and we want to
     exclude pixels with zeros in both images */
  // first we skip all entries (pixels) with zeros in both images
  for (i = 0; i < n; i++)
    if (ptr1[order[i]] || ptr2[order[i]])
      break;

  if (i == n)                   // there aren't any entries that are
                                // non-zero in both images
    n2 = 0;
  else {                        // we do have some non-zero entries
    n2 = 1;                     // count the first one
    for (i2 = i + 1; i2 < n; i2++)
      // if the current entry is unequal to the previous one, then it is
      // unique
      if (ac_compare(order + i2 - 1, order + i2))
        n2++;                   // count the unique ones
  }

  if (n2) {                     // we have some non-zero entries
    // prepare an output array
    dims[0] = n2;               // number of non-zero entries
    dims[1] = 3;                // three output numbers per entry
    result = array_scratch(LUX_INT32, 2, dims);
    ptr = ptr0 = (int32_t*) array_data(result); // output pointer
    // we store the area numbers in both images, count the number of
    // pixels with that combination, and keep track of the maximum
    // included area numbers in both images.
    max1 = ptr[0] = ptr1[order[i]];
    max2 = ptr[n2] = ptr2[order[i]];
    ptr[2*n2] = 1;              // start with one entry
    for (i2 = i + 1; i2 < n; i2++) { //
      v1 = ptr1[order[i2]];     // image 1 area number
      v2 = ptr2[order[i2]];     // image 2 area number
      if (v1 > max1)
        max1 = v1;              // track maximum value in image 1
      if (v2 > max2)
        max2 = v2;              // track maximum value in image 2
      if (ac_compare(order + i2 - 1, order + i2)) {
        // this entry is unequal to the previous one, so it is a new one
        ptr++;                  // move to next slot
        ptr[0] = v1;            // store area numbers
        ptr[n2] = v2;
        ptr[2*n2] = 1;          // and start counting entries at 1
      } else
        ptr[2*n2]++;            // it's the same as the previous one,
                                // so we need only update the count
    }
    free(order);                // no longer needed
  } else {                      // no overlapping segments at all
    free(order);                // no longer needed
    return LUX_MINUS_ONE;
  }

  if (raw)                      // we're done
    return result;

  // we now have a list of connections between areas in image 1 and
  // areas in image 2, that is sorted in ascending order first of
  // area number in image 1 and then, within each group with the
  // same image-1 area number, by area number in image 2.
  // For what follows, it is convenient to also have the ordering
  // by image 2 first and image 1 last.  We get that one now.
  order = (int32_t *) malloc(n2*sizeof(int32_t));
  order2 = (int32_t *) malloc(n2*sizeof(int32_t));
  if (!order || !order2)
    return cerror(ALLOC_ERR, 0);
  for (i = 0; i < n2; i++)
    order[i] = i;
  ptr1 = ptr0 + n2;
  ptr2 = ptr0;
  qsort(order, n2, sizeof(int32_t), ac_compare);
  // now ptr1[order] is in ascending order of image-2 area numbers
  // and ptr2[order] in ascending order of image-1 area numbers for
  // each set of constant image-2 area numbers.
  ptr1 = ptr0;                  // image-1 numbers
  ptr2 = ptr0 + n2;             // image-2 numbers
  for (i = 0; i < n2; i++)      // get the reverse ordering
    order2[order[i]] = i;

  // Those areas in image 1 that have more than one connection to
  // non-background areas in image 2 are splitting up.  We find them.
  i = j = v1 = v2 = 0;
  // seek the next all-non-zero connection
  while (i < n2) {              // not at the end yet
    if (!ptr1[i] || !ptr2[i]) {         // background (0) in either image
      i++;                      // skip
      continue;
    }
    if (ptr1[i] != j)           // a new image-1 area
      j = ptr1[i++];
    else {                      // the second hit on this image-1 area
      v1++;                     // increment count of hits in image1
      v2++;                     // and hits in image2
      for ( ; i < n2 && ptr1[i] == j; i++) // count image-2 areas
        if (ptr2[i] != ptr2[i - 1])
          v2++;
    }
  }

  // now v1 counts the number of image-1 areas that are splitting up,
  // and v2 counts the total number of image-2 areas that correspond
  // to the splitting image-1 areas.

  // we prepare output symbols for the results

  // we store the image-1 area numbers of splitters in $SPLIT1
  // and the corresponding image-2 area numbers in $SPLIT2.
  // we store index values in $SPLIT2_INDEX such that the image-2 areas
  // that correspond to image-1 area $SPLIT1(I) are given by
  // $SPLIT2($SPLIT2_INDEX(I):$SPLIT2_INDEX(I+1)-1).
  qsplit1 = findVarName("$SPLIT1", 0);
  if (qsplit1 == LUX_ERROR) {
    free(order);
    free(order2);
    return LUX_ERROR;
  }
  if (v1) {                     // we have <v1> splitters to store
    if (to_scratch_array(qsplit1, LUX_INT32, 1, &v1) == LUX_ERROR) {
      free(order);
      free(order2);
      return LUX_ERROR;
    }
    out1 = (int32_t*) array_data(qsplit1);
  } else {                      // no splitters to store: output scalar -1
    to_scalar(qsplit1, LUX_INT32);
    scalar_value(qsplit1).l = -1;
  }
  qsplit2list = findVarName("$SPLIT2_INDEX", 0);
  if (qsplit2list == LUX_ERROR) {
    free(order);
    free(order2);
    return LUX_ERROR;
  }
  if (v1) {
    v1++;                       // add one extra element to point just
    // beyond the end of the $SPLIT2 array, so that
    // $SPLIT2($SPLIT2_INDEX(I):$SPLIT2_INDEX(I+1)-1) works even for
    // the last valid value of I.
    if (to_scratch_array(qsplit2list, LUX_INT32, 1, &v1) == LUX_ERROR) {
      free(order);
      free(order2);
      return LUX_ERROR;
    }
    list = (int32_t*) array_data(qsplit2list);
    v1--;                       // return to previous value
  } else {
    to_scalar(qsplit2list, LUX_INT32);
    scalar_value(qsplit2list).l = -1;
  }
  qsplit2 = findVarName("$SPLIT2", 0);
  if (qsplit2 == LUX_ERROR) {
    free(order);
    free(order2);
    return LUX_ERROR;
  }
  if (v2) {
    if (to_scratch_array(qsplit2, LUX_INT32, 1, &v2) == LUX_ERROR) {
      free(order);
      free(order2);
      return LUX_ERROR;
    }
    out2 = (int32_t*) array_data(qsplit2);
  } else {
    to_scalar(qsplit2, LUX_INT32);
    scalar_value(qsplit2).l = -1;
  }

  // now write the area numbers to the output variables.
  i = j = v1 = v2 = 0;
  while (i < n2) {              // not yet at end
    if (!ptr1[i] || !ptr2[i]) {         // seek non-zero values in both images
      i++;
      continue;
    }
    if (ptr1[i] != j)           // a new image-1 area
      j = ptr1[i++];            // remember this area number
    else {                      // the second hit on this image-1 area
      *out1++ = ptr1[i];        // store image-1 area number
      *out2++ = ptr2[i - 1];    // we must not forget the first hit: store
                                // its image-2 value
      *list++ = v2++;           // store the index
      for ( ; i < n2 && ptr1[i] == j; i++) // count image-2 areas
        if (ptr2[i] != ptr2[i - 1]) {
          *out2++ = ptr2[i];
          v2++;
        }
    }
  }
  // we now add one element at the end of the $SPLIT2_INDEX array that
  // points to one beyond the end of the $SPLIT2 array
  if (v2)
    *list = v2;

  // Those areas in image 2 that have more than one connection to
  // non-background areas in image 1 have undergone merging.  We find them.
  i = j = v1 = v2 = 0;
  // seek the next all-non-zero connection
  while (i < n2) {
    if (!ptr1[order[i]] || !ptr2[order[i]]) {
      i++;
      continue;
    }
    if (ptr2[order[i]] != j)    // a new image-2 area
      j = ptr2[order[i++]];
    else {                      // the second hit on this image-2 area
      v1++;                     // increment count
      v2++;
      for ( ; i < n2 && ptr2[order[i]] == j; i++) // count image-1 areas
        if (ptr1[order[i]] != ptr1[order[i - 1]])
          v2++;
    }
  }

  qmerge2 = findVarName("$MERGE2", 0);
  if (qmerge2 == LUX_ERROR) {
    free(order);
    free(order2);
    return LUX_ERROR;
  }
  if (v1) {
    if (to_scratch_array(qmerge2, LUX_INT32, 1, &v1) == LUX_ERROR) {
      free(order);
      free(order2);
      return LUX_ERROR;
    }
    out1 = (int32_t*) array_data(qmerge2);
  } else {
    to_scalar(qmerge2, LUX_INT32);
    scalar_value(qmerge2).l = -1;
  }
  qmerge1list = findVarName("$MERGE1_INDEX", 0);
  if (qmerge1list == LUX_ERROR) {
    free(order);
    free(order2);
    return LUX_ERROR;
  }
  if (v1) {
    v1++;
    if (to_scratch_array(qmerge1list, LUX_INT32, 1, &v1) == LUX_ERROR) {
      free(order);
      free(order2);
      return LUX_ERROR;
    }
    list = (int32_t*) array_data(qmerge1list);
    v1--;
  } else {
    to_scalar(qmerge1list, LUX_INT32);
    scalar_value(qmerge1list).l = -1;
  }
  qmerge1 = findVarName("$MERGE1", 0);
  if (qmerge1 == LUX_ERROR) {
    free(order);
    free(order2);
    return LUX_ERROR;
  }
  if (v2) {
    if (to_scratch_array(qmerge1, LUX_INT32, 1, &v2) == LUX_ERROR) {
      free(order);
      free(order2);
      return LUX_ERROR;
    }
    out2 = (int32_t*) array_data(qmerge1);
  } else {
    to_scalar(qmerge1, LUX_INT32);
    scalar_value(qmerge1).l = -1;
  }

  i = j = v1 = v2 = 0;
  // seek the next all-non-zero connection
  while (i < n2) {
    if (!ptr1[order[i]] || !ptr2[order[i]]) {
      i++;
      continue;
    }
    if (ptr2[order[i]] != j)    // a new image-2 area
      j = ptr2[order[i++]];
    else {                      // the second hit on this image-2 area
      *out1++ = ptr2[order[i]];
      *out2++ = ptr1[order[i - 1]];
      *list++ = v2++;
      for ( ; i < n2 && ptr2[order[i]] == j; i++) // count image-1 areas
        if (ptr1[order[i]] != ptr1[order[i - 1]]) {
          *out2++ = ptr1[order[i]];
          v2++;
        }
    }
  }
  if (v2)
    *list = v2;

  /* The areas in image 2 that have no counterparts in image 1 have just
     appeared.  They have only a single entry in the list. */
  // First we skip the links with zero in image 2
  for (i = 0; i < n2; i++)
    if (ptr2[order[i]])
      break;
  i0 = i;
  n = 0;
  for ( ; i < n2 - 1; i++)
    if (ptr1[order[i]] || ptr2[order[i + 1]] == ptr2[order[i]])
      continue;
    else
      n++;
  // check the last one separately
  if (!ptr1[order[n2 - 1]] && ptr2[order[n2 - 2]] != ptr2[order[n2 - 1]])
    n++;

  qapp = findVarName("$APP", 0);
  if (qapp == LUX_ERROR) {
    free(order);
    free(order2);
    return LUX_ERROR;
  }
  if (n) {
    if (to_scratch_array(qapp, LUX_INT32, 1, &n) == LUX_ERROR) {
      free(order);
      free(order2);
      return LUX_ERROR;
    }
    ptr = (int32_t*) array_data(qapp);
    for (i = i0 ; i < n2 - 1; i++)
      if (ptr1[order[i]] || ptr2[order[i + 1]] == ptr2[order[i]])
        continue;
      else
        *ptr++ = ptr2[order[i]];
    // check the last one separately
    if (!ptr1[order[n2 - 1]] && ptr2[order[n2 - 2]] != ptr2[order[n2 - 1]])
      *ptr++ = ptr2[order[n2 - 1]];
  } else {
    to_scalar(qapp, LUX_INT32);
    scalar_value(qapp).l = -1;
  }

  /* The areas in image 1 that have no counterparts in image 2 have just
     disappeared.  They have only a single entry in the list. */
  // First we skip the links with zero in image 1
  for (i = 0; i < n2; i++)
    if (ptr1[i])
      break;
  i0 = i;
  n = 0;
  for ( ; i < n2 - 1; i++)
    if (ptr2[i] || ptr1[i + 1] == ptr1[i])
      continue;
    else
      n++;
  // check the last one separately
  if (!ptr2[n2 - 1] && ptr1[n2 - 2] != ptr1[n2 - 1])
    n++;

  qdisapp = findVarName("$DISAPP", 0);
  if (qdisapp == LUX_ERROR) {
    free(order);
    free(order2);
    return LUX_ERROR;
  }
  if (n) {
    if (to_scratch_array(qdisapp, LUX_INT32, 1, &n) == LUX_ERROR) {
      free(order);
      free(order2);
      return LUX_ERROR;
    }
    ptr = (int32_t*) array_data(qdisapp);
    for (i = i0 ; i < n2 - 1; i++)
      if (ptr2[i] || ptr1[i + 1] == ptr1[i])
        continue;
      else
        *ptr++ = ptr1[i];
    // check the last one separately
    if (!ptr2[n2 - 1] && ptr1[n2 - 2] != ptr1[n2 - 1])
      *ptr++ = ptr1[n2 - 1];
  } else {
    to_scalar(qdisapp, LUX_INT32);
    scalar_value(qdisapp).l = -1;
  }

  /* Those regions that neither appear nor disappear, split, or merge,
     are "stayers".  These can have only a single entry in the list
     with non-zero area numbers in both images */
  n = 0;
  for (i = 0; i < n2; i++) {
    if (!ptr1[i] || !ptr2[i])
      // must have image-1 and image-2 area numbers unequal to zero
      continue;
    if (i < n2 - 1 && ptr1[i + 1] == ptr1[i]) {
      // must have only a single entry with non-zero image-2 for any
      // given image-1.  If a second such entry exists, then it follows
      // immediately upon the first one, because of the ordering of the
      // entries.  We found one such, so we can skip the rest of this
      // image-1 area number.
      j = ptr1[i++];
      while (i < n2 - 1 && ptr1[i + 1] == j)
        i++;
      continue;
    }
    // now we must check the same thing for the image-2 area
    j = order2[i];              // now order[order2[j]] == i
    if (j < n2 - 1
        && ptr2[order[j + 1]] == ptr2[order[j]]
        && ptr1[order[j + 1]])
      // found a seocnd entry for the image-2 area
      continue;
    if (j > 0
        && ptr2[order[j - 1]] == ptr2[order[j]]
        && ptr1[order[j - 1]])
      // found a seocnd entry for the image-2 area
      continue;
    n++;
  }

  qstay1 = findVarName("$STAY1", 0);
  if (qstay1 == LUX_ERROR) {
    free(order);
    free(order2);
    return LUX_ERROR;
  }
  if (n) {
    if (to_scratch_array(qstay1, LUX_INT32, 1, &n) == LUX_ERROR) {
      free(order);
      free(order2);
      return LUX_ERROR;
    }
    out1 = (int32_t*) array_data(qstay1);
  } else {
    to_scalar(qstay1, LUX_INT32);
    scalar_value(qstay1).l = -1;
  }

  qstay2 = findVarName("$STAY2", 0);
  if (qstay2 == LUX_ERROR) {
    free(order);
    free(order2);
    return LUX_ERROR;
  }
  if (n) {
    if (to_scratch_array(qstay2, LUX_INT32, 1, &n) == LUX_ERROR) {
      free(order);
      free(order2);
      return LUX_ERROR;
    }
    out2 = (int32_t*) array_data(qstay2);
  } else {
    to_scalar(qstay2, LUX_INT32);
    scalar_value(qstay2).l = -1;
  }

  if (n) {
    for (i = 0; i < n2; i++) {
      if (!ptr1[i] || !ptr2[i])
        // must have image-1 and image-2 area numbers unequal to zero
        continue;
      if (i < n2 - 1 && ptr1[i + 1] == ptr1[i]) {
        // must have only a single entry with non-zero image-2 for any
        // given image-1.  If a second such entry exists, then it follows
        // immediately upon the first one, because of the ordering of the
        // entries.  We found one such, so we can skip the rest of this
        // image-1 area number.
        j = ptr1[i++];
        while (i < n2 - 1 && ptr1[i + 1] == j)
          i++;
        continue;
      }
      // now we must check the same thing for the image-2 area
      j = order2[i];            // now order[order2[j]] == i
      if (j < n2 - 1
          && ptr2[order[j + 1]] == ptr2[order[j]]
          && ptr1[order[j + 1]])
        // found a seocnd entry for the image-2 area
        continue;
      if (j > 0
          && ptr2[order[j - 1]] == ptr2[order[j]]
          && ptr1[order[j - 1]])
        // found a seocnd entry for the image-2 area
        continue;
      *out1++ = ptr1[i];
      *out2++ = ptr2[i];
    }
  }

  // now we do any requested area number compacting.  We only modify
  // the <im2> area numbers, and related global variables -- if
  // <im2> is a named variable.
  // if <compact> is equal to zero or negative,
  // then nothing is modified.  Otherwise, the <im2> numbers of staying
  // areas are set equal to the corresponding numbers from <im1>

  if (compact > 0 && symbolProperName(ps[1])) {
    // we need sorted lists of all <im1> and <im2> area numbers
    out2 = order2;
    // skip the <im2> zeros
    for (i = 0; i < n2; i++)
      if (ptr2[order[i]])
        break;
    if (i == n2) {              // don't have any
      free(order);
      free(order2);
      return result;            // so we're done
    }
    j = 0;
    for ( ; i < n2; i++)
      if (ptr2[order[i]] != j)  // a new area number
        *out2++ = j = ptr2[order[i]];
    n = n2;
    n2 = out2 - order2;                 // the number of them
    // now <order2> contains a sorted list of image-2 numbers
    // we go for the <im1> areas
    out1 = order;
    for (i = 0; i < n; i++)
      if (ptr1[i])
        break;
    if (i == n)                         // don't have any
      n = 0;
    else {
      j = 0;
      for ( ; i < n; i++)
        if (ptr1[i] != j)       // a new area number
          *out1++ = j = ptr1[i];
      n = out1 - order;         // the number of them
    }

    // we keep a list of flags to indicate which image-1 numbers
    // have already been assigned to some image-2 areas
    flags = (uint8_t*) malloc(n);
    if (!flags) {
      free(order);
      free(order2);
      return cerror(ALLOC_ERR, 0);
    }
    zerobytes(flags, n);

    // we store the replacement numbers in <ptr0>
    ptr0 = (int32_t*) malloc(n2*sizeof(int32_t));
    if (!ptr0) {
      free(order);
      free(order2);
      free(flags);
      return cerror(ALLOC_ERR, 0);
    }
    zerobytes(ptr0, n2*sizeof(int32_t));
    // we start with stayers.
    if (symbolIsArray(qstay1)) {
      out1 = (int32_t*) array_data(qstay1);
      out2 = (int32_t*) array_data(qstay2);
      j = array_size(qstay1);
      while (j--) {
        // seek the encountered image-2 number in the list
        ptr = (int32_t*) bsearch(out2++, order2, n2, sizeof(int32_t), intcmp);
        ptr0[ptr - order2] = *out1; // replacement number
        ptr = (int32_t*) bsearch(out1++, order, n, sizeof(int32_t), intcmp);
        flags[ptr - order] = 1; // flag use of this number
      }
    }
    // now we work on the splitters; the image-1 number is assigned to
    // the first of the image-2 counterparts that does not yet have
    // a new assigned number.
    if (symbolIsArray(qsplit1)) {
      out1 = (int32_t*) array_data(qsplit1);
      out2 = (int32_t*) array_data(qsplit2);
      list = (int32_t*) array_data(qsplit2list);
      j = array_size(qsplit1);
      while (j--) {             // all pre-split areas
        i = list[1] - list[0];  // the number of post-split areas
        list++;
        // corresponding to the current pre-split
        while (i--) {           // all corresponding post-split areas
          ptr = (int32_t*) bsearch(out2++, order2, n2, sizeof(int32_t), intcmp);
          if (!ptr0[ptr - order2]) { // not yet assigned
            ptr0[ptr - order2] = *out1;
            ptr = (int32_t*) bsearch(out1, order, n, sizeof(int32_t), intcmp);
            flags[ptr - order] = 1; // flag use of this number
            break;
          }
        }
        if (i)                  // done before all post-splits were treated
          out2 += i;
        out1++;                         // done with this one
      }
    }
    // and finally the mergers; the image-1 number of the first available
    // pre-merger area is assigned to the post-merger area, if that one
    // has not already been assigned to.  A given area may be part of
    // splitting and merging processes at the same time, so we must
    // be careful.
    if (symbolIsArray(qmerge1)) {
      out1 = (int32_t*) array_data(qmerge1);
      out2 = (int32_t*) array_data(qmerge2);
      list = (int32_t*) array_data(qmerge1list);
      j = array_size(qmerge2);
      while (j--) {             // all post-merge areas
        i = list[1] - list[0];  // the number of pre-merge areas
        list++;
        // corresponding to the current post-merge
        ptr = (int32_t*) bsearch(out2, order2, n2, sizeof(int32_t), intcmp);
        if (ptr0[ptr - order2]) {       // already assigned to
          out2++;
          out1 += i;
          continue;
        }
        while (i--) {           // all corresponding pre-merge areas
          ptr = (int32_t*) bsearch(out1, order, n, sizeof(int32_t), intcmp);
          if (!flags[ptr - order]) { // not yet assigned
            flags[ptr - order] = 1; // flag use of this number
            ptr = (int32_t*) bsearch(out2, order2, n2, sizeof(int32_t), intcmp);
            ptr0[ptr - order2] = *out1;
            break;
          }
        }
        if (i)                  // done before all pre-merges were treated
          out1 += i;
        out1++;
        out2++;                         // done with this one
      }
    }
    // we must still assign new numbers to all image-2 areas that have not
    // yet received any.  How this is dealt with depends on the value of
    // <compact>.
    //  < 1: no new numbers
    // == 1: contiguous numbers, starting at one greater than the
    //       maximum image-1 number
    // == 2: smallest available numbers no smaller than 2
    //  > 2: contiguous numbers, starting at one greater than
    //       the value of <compact>.
    if (compact == 2) {                 // we assign new numbers that are as small
                                // as possible yet still unused
      // we locate the first available number at least equal to 2
      j = 2;
      i2 = 0;
      while (order[i2] < j && i2 <= n)
        i2++;
      if (i2 > n) {             // we've reached the end of the range of
                                // numbers used in image 1
        j = order[n - 1];
        for (i = 0; i < n2; i++)
          if (!ptr0[i])
            ptr0[i] = ++j;
      } else {
        for (i = 0; i < n2; i++)
          if (!ptr0[i]) {       // not yet assigned to
            while (j == order[i2]
                   && i2 <= n) {
              j++;
              i2++;
            }
            if (i2 > n)
              break;
            ptr0[i] = j++;
          }
        if (i < n2)             // we reached the end of the image-1 range
          for ( ; i < n2; i++)
            if (!ptr0[i])
              ptr0[i] = j++;
      }
    } else {                    // we assign new numbers starting at one
      // greater than the greatest image-1 number (if compact == 1) or
      // starting at one greater than the value of <compact> (if
      // compact > 1).
      if (compact == 1 || compact < order[n - 1])
        j = order[n - 1];
      else
        j = compact;
      for (i = 0; i < n2; i++)
        if (!ptr0[i])           // not yet assigned to
          ptr0[i] = ++j;
      if (compact > 1 && symbolProperName(ps[2])) {
        // store highest assigned number in <compact>, if it is a
        // named variable (and not an expression)
        to_scalar(ps[2], LUX_INT32);
        scalar_value(ps[2]).l = j;
      }
    }
    free(flags);

    // now we have the new numbers, and we modify the image-2 numbers and
    // relevant global variables accordingly.
    if (symbolIsArray(ps[1])) {
      list = (int32_t*) array_data(ps[1]);      // <im2>
      i2 = array_size(ps[1]);
      j = -1;
      i = 0;
      while (i2--) {
        if (*list > 0) {
          if (*list == j)
            *list = i;
          else {
            j = *list;
            ptr = (int32_t*) bsearch(list, order2, n2, sizeof(int32_t), intcmp);
            *list = i = ptr0[ptr - order2];
          }
        }
        list++;
      }
    }
    // modify the return value
    if (symbolIsArray(result)) {
      list = (int32_t*) array_data(result);
      i2 = array_size(result)/3; // we only want to change the image-2
                                 // numbers, i.e. 1/3rd of the whole.
      list += i2;               // move to start of image-2 numbers
      while (i2--) {
        if (*list > 0) {
          ptr = (int32_t*) bsearch(list, order2, n2, sizeof(int32_t), intcmp);
          *list = ptr0[ptr - order2];
        }
        list++;
      }
    }
    // modify $APP
    if (symbolIsArray(qapp)) {
      list = (int32_t*) array_data(qapp);
      i2 = array_size(qapp);
      while (i2--) {
        if (*list > 0) {
          ptr = (int32_t*) bsearch(list, order2, n2, sizeof(int32_t), intcmp);
          *list = ptr0[ptr - order2];
        }
        list++;
      }
    }
    // modify $STAY2
    if (symbolIsArray(qstay2)) {
      list = (int32_t*) array_data(qstay2);
      i2 = array_size(qstay2);
      while (i2--) {
        if (*list > 0) {
          ptr = (int32_t*) bsearch(list, order2, n2, sizeof(int32_t), intcmp);
          *list = ptr0[ptr - order2];
        }
        list++;
      }
    }
    // modify $SPLIT2
    if (symbolIsArray(qsplit2)) {
      list = (int32_t*) array_data(qsplit2);
      i2 = array_size(qsplit2);
      while (i2--) {
        if (*list > 0) {
          ptr = (int32_t*) bsearch(list, order2, n2, sizeof(int32_t), intcmp);
          *list = ptr0[ptr - order2];
        }
        list++;
      }
    }
    // modify $MERGE2
    if (symbolIsArray(qmerge2)) {
      list = (int32_t*) array_data(qmerge2);
      i2 = array_size(qmerge2);
      while (i2--) {
        if (*list > 0) {
          ptr = (int32_t*) bsearch(list, order2, n2, sizeof(int32_t), intcmp);
          *list = ptr0[ptr - order2];
        }
        list++;
      }
    }
    // all done with this
    free(ptr0);
  } // end of if (compact > 0)

  free(order);
  free(order2);

  return result;
}
//----------------------------------------------------------------------
