/* This is file strous3.cc.

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

/// \file

// File strous3.cc
// Various LUX routines by L. Strous

/// \defgroup luxroutines LUX routines
///
/// C++ functions that implement LUX subroutines and functions.  Typically, a
/// C++ function called `lux_X` implements a LUX subroutine or function named
/// `X`.
///
/// @{
/// @}

// configuration include

#include "config.h"

// standard includes

#include <cassert>
#include <ctype.h>
#include <errno.h>              // for errno
#include <float.h>
#include <functional>
#include <list>
#include <math.h>
#include <numeric>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// library includes

#include <gsl/gsl_sf_gamma.h>
#include <gslpp_sort.hh>        // for gsl_sort_index
#include <gslpp_sort_vector.hh> // for gsl_sort_vector
#include <gslpp_vector.hh>

// own includes

#include "Bytestack.hh"
#include "MonotoneInterpolation.hh"
#include "SSFC.hh"
#include "SolarSystemEphemerides.hh"
#include "action.hh"
#include "cdiv.hh"
#include "permutations.hh"

/// \ingroup luxroutines
///
/// Implements the `bisect` function in LUX.
///
///     y = bisect([x,] y, values [, axis=axis, pos=pos, width=width])
///
/// Depends on the Gnu Scientific Library.
///
/// \param[in] narg is the number of arguments.
///
/// \param[in,out] points at the LUX symbol number of the first argument.
///
/// \returns the LUX symbol number of the result.
int32_t lux_bisect(int32_t narg, int32_t ps[])
// y = BISECT([x,] y, values [, AXIS=axis, POS=pos, WIDTH=width])
// calculates bisector positions
// <axis> may only have a single dimension.
// The first dimension of <y> traces profiles to be checked for
// bisectors.
/* <values> contains the levels at which bisector positions are to be
   returned.
   <pos> contains the position (index to each profile) at which
   the search for each bisector should start.  (From such a position,
   the local minimum is sought, and bisectors are determined from there.)
   If <pos> is not defined, then bisectors are determined starting at
   the position of the absolute minimum in each profile.  <pos> must be
   a scalar.
   If <width> is specified, then the profile widths corresponding to the
   bisector positions are returned in it. */
// LS 7may98
{
#if HAVE_LIBGSL
  int32_t       result, iq, pos, nLev, outDims[MAX_DIMS], step,
    lev, xSym, ySym, vSym, il, ir;
  double        xl, xr, min, minpos, max, maxpos, x1l, x2l, x1r, x2r;
  Pointer       src, trgt, level, ptr, rightedge, left, width, x;
  csplineInfo   cspl;
  LoopInfo      srcinfo;

  if (!symbolIsNumericalArray(ps[0]))
    return cerror(NEED_NUM_ARR, ps[0]);

  cspl = empty_cubic_spline();

  if (narg > 2 && ps[2]) {
    xSym = ps[0];
    ySym = ps[1];
    vSym = ps[2];
  } else {
    xSym = 0;
    ySym = ps[0];
    vSym = ps[1];
  }

  if (standardLoop(ySym, (narg > 3 && ps[3])? ps[3]: 0,
                   SL_COMPRESS | SL_ONEAXIS | SL_NEGONED | SL_SRCUPGRADE
                   | SL_EACHROW | SL_AXISCOORD, LUX_FLOAT,
                   &srcinfo, &src, NULL, NULL, NULL) < 0) // <data>, <axis>
    return LUX_ERROR;           // some error

  if (xSym) {
    if (!symbolIsNumericalArray(xSym))
      return cerror(NEED_NUM_ARR, xSym);
    if (array_size(xSym) != srcinfo.rdims[0])
      return cerror(INCMP_ARG, ySym);
    iq = lux_converts[srcinfo.type](1, &xSym);
    x.f = (float*) array_data(iq);
  } else
    x.f = NULL;

  if (!symbolIsNumerical(vSym)) // <levels>
    return LUX_ERROR;
  iq = lux_converts[srcinfo.type](1, &vSym); // ensure proper type
  numerical(iq, NULL, NULL, &nLev, &level);

  if (narg > 4 && ps[4]) {      // have <pos>
    pos = int_arg(ps[4]);
    if (pos < 0 || pos >= srcinfo.rdims[0])
      return luxerror("Index out of range", ps[4]);
  } else                                // no <pos>
    pos = -1;

  // create output symbol
  if (nLev > 1) {
    outDims[0] = nLev;
    memcpy(outDims + 1, srcinfo.dims, srcinfo.raxes[0]*sizeof(int32_t));
    memcpy(outDims + srcinfo.raxes[0] + 1, srcinfo.dims + srcinfo.raxes[0] + 1,
           (srcinfo.ndim - srcinfo.raxes[0] - 1)*sizeof(int32_t));
    result = array_scratch(srcinfo.type, srcinfo.ndim, outDims);
    if (narg > 5 && ps[5])      // have <width>
      if (to_scratch_array(ps[5], srcinfo.type, srcinfo.ndim, outDims)
          == LUX_ERROR)
        return LUX_ERROR;
  } else {
    if (srcinfo.ndim > 1) {
      memcpy(outDims, srcinfo.dims, srcinfo.raxes[0]*sizeof(int32_t));
      memcpy(outDims + srcinfo.raxes[0], srcinfo.dims + srcinfo.raxes[0] + 1,
             (srcinfo.ndim - srcinfo.raxes[0] - 1)*sizeof(int32_t));
      result = array_scratch(srcinfo.type, srcinfo.ndim - 1, outDims);
      if (narg > 5 && ps[5])    // have <width>
        if (to_scratch_array(ps[5], srcinfo.type, srcinfo.ndim, outDims)
            == LUX_ERROR)
          return LUX_ERROR;
    } else {                    // only one return value
      result = scalar_scratch(srcinfo.type);
      if (narg > 5 && ps[5]) {  // have <width>
        undefine(ps[5]);
        symbol_class(ps[5]) = LUX_SCALAR;
        scalar_type(ps[5]) = srcinfo.type;
      }         
    }
  }
  if (result < 0)
    return LUX_ERROR;
  switch (symbol_class(result)) {
    case LUX_ARRAY:
      trgt.f = (float *) array_data(result);
      if (narg > 5 && ps[5])
        width.f = (float *) array_data(ps[5]);
      else
        width.f = NULL;
      break;
    case LUX_SCALAR:
      trgt.f = &scalar_value(result).f;
      if (narg > 5 && ps[5])
        width.f = &scalar_value(ps[5]).f;
      else
        width.f = NULL;
      break;
  }

  step = srcinfo.step[0];

  // now do the work
  switch (srcinfo.type) {
    case LUX_FLOAT:
      do {
        rightedge.f = src.f + step*(srcinfo.rdims[0] - 1);
        if (pos >= 0) {
          ptr.f = src.f + pos*step; // start position
          // now seek the local minimum
          if (ptr.f > src.f && ptr.f[-step] < *ptr.f)
            while (ptr.f > src.f && ptr.f[-step] < *ptr.f)
              ptr.f -= step;
          else
            while (ptr.f < rightedge.f && ptr.f[step] < *ptr.f)
              ptr.f += step;
        } else {                // find absolute minimum
          ptr.f = left.f = src.f;
          do {
            left.f += step;
            if (*left.f < *ptr.f)
              ptr.f = left.f;
          } while (left.f < rightedge.f);
        }

        // install table for cubic spline interpolation
        cubic_spline_tables(x.f, srcinfo.type, 1,
                            src.f, srcinfo.type, step,
                            srcinfo.rdims[0], 0, 0,
                            &cspl);

        /* the cubic spline may dip below the lower of the surrounding
         specified data points: we must find the local minimum in
         the cubic spline. */
        ir = (ptr.f - src.f)/step;
        il = ir - 1;
        if (x.f) {
          x1l = x.f[ir - 1];
          x1r = x2l = x.f[ir];
          x2r = x.f[ir + 1];
        } else {
          x1l = ir - 1;
          x1r = x2l = ir;
          x2r = ir + 1;
        }
        find_cspline_extremes(x1l, x2r, &minpos, &min, NULL, NULL, &cspl);

        // the levels are assumed to be sorted in ascending order
        for (lev = 0; lev < nLev; lev++) {
          if (min > level.f[lev]) {
            *trgt.f++ = -1.0;
            if (width.f != NULL)
              *width.f++ = 0.0;
          } else {
            if (*ptr.f > level.f[lev]) { // the current level is above
              /* the minimum of the spline fit, but below the local minimum of
               the tabular points */
              xl = find_cspline_value(level.f[lev], x1l, minpos, &cspl);
              xr = find_cspline_value(level.f[lev], minpos, x2r, &cspl);
              *trgt.f = (xl + xr)/2;
              if (width.f)
                *width.f++ = xr - xl;
            } else {
              if (il >= 0) {
                do {
                  find_cspline_extremes(x1l, x2l, NULL, NULL, &maxpos,
                                        &max, &cspl);
                  if (max < level.f[lev]) {
                    il--;
                    if (x.f) {
                      x1l = x.f[il];
                      x2l = x.f[il + 1];
                    } else {
                      x1l = il;
                      x2l = il + 1;
                    }
                  } else
                    break;
                } while (il >= 0);
                if (il >= 0)
                  xl = find_cspline_value(level.f[lev], maxpos, x2l, &cspl);
                else
                  xl = -DBL_MAX;
              } else
                xl = -DBL_MAX;

              if (ir < srcinfo.rdims[0]) { // not yet at edge
                do {
                  find_cspline_extremes(x1r, x2r, NULL, NULL, &maxpos,
                                        &max, &cspl);
                  if (max < level.f[lev]) {
                    ir++;
                    if (x.f) {
                      x1r = x.f[ir];
                      x2r = x.f[ir + 1];
                    } else {
                      x1r = ir;
                      x2r = ir + 1;
                    }
                  } else
                    break;
                } while (ir < srcinfo.rdims[0]);
                if (ir < srcinfo.rdims[0])
                  xr = find_cspline_value(level.f[lev], x1r, maxpos, &cspl);
                else
                  xr = -DBL_MAX;        // flag: at edge
              } else
                xr = -DBL_MAX;

              if (xl > -DBL_MAX && xr > -DBL_MAX) { // not at edge
                *trgt.f = (xl + xr)/2;
                if (width.f)
                  *width.f++ = xr - xl;
              } else {
                if (width.f)
                  *width.f++ = 0;
                *trgt.f = -1;   // not found
              }
            }
            trgt.f++;
          } // end of if (*ptr.f > level.f[lev]) else
        } // end of for (lev = 0; ...)
        src.f += step*srcinfo.rdims[0];
      } while (srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
      break;
    case LUX_DOUBLE:
      do {
        rightedge.d = src.d + step*(srcinfo.rdims[0] - 1);
        if (pos >= 0) {
          ptr.d = src.d + pos*step; // start position
          // now seek the local minimum
          if (ptr.d > src.d && ptr.d[-step] < *ptr.d)
            while (ptr.d > src.d && ptr.d[-step] < *ptr.d)
              ptr.d -= step;
          else
            while (ptr.d < rightedge.d && ptr.d[step] < *ptr.d)
              ptr.d += step;
        } else {                // find absolute minimum
          ptr.d = left.d = src.d;
          do {
            left.d += step;
            if (*left.d < *ptr.d)
              ptr.d = left.d;
          } while (left.d < rightedge.d);
        }

        // install table for cubic spline interpolation
        cubic_spline_tables(x.d, srcinfo.type, 1,
                            src.d, srcinfo.type, step,
                            srcinfo.rdims[0], 0, 0,
                            &cspl);

        /* the cubic spline may dip below the lower of the surrounding
         specified data points: we must find the local minimum in
         the cubic spline. */
        ir = (ptr.d - src.d)/step;
        il = ir - 1;
        if (x.d) {
          x1l = x.d[ir - 1];
          x1r = x2l = x.d[ir];
          x2r = x.d[ir + 1];
        } else {
          x1l = ir - 1;
          x1r = x2l = ir;
          x2r = ir + 1;
        }
        find_cspline_extremes(x1l, x2r, &minpos, &min, NULL, NULL, &cspl);

        // the levels are assumed to be sorted in ascending order
        for (lev = 0; lev < nLev; lev++) {
          if (min > level.d[lev]) {
            *trgt.d++ = -1.0;
            if (width.d != NULL)
              *width.d++ = 0.0;
          } else {
            if (*ptr.d > level.d[lev]) { // the current level is above
              /* the minimum of the spline fit, but below the local minimum of
               the tabular points */
              xl = find_cspline_value(level.d[lev], x1l, minpos, &cspl);
              xr = find_cspline_value(level.d[lev], minpos, x2r, &cspl);
              *trgt.d = (xl + xr)/2;
              if (width.d)
                *width.d++ = xr - xl;
            } else {
              if (il >= 0) {
                do {
                  find_cspline_extremes(x1l, x2l, NULL, NULL, &maxpos,
                                        &max, &cspl);
                  if (max < level.d[lev]) {
                    il--;
                    if (x.d) {
                      x1l = x.d[il];
                      x2l = x.d[il + 1];
                    } else {
                      x1l = il;
                      x2l = il + 1;
                    }
                  } else
                    break;
                } while (il >= 0);
                if (il >= 0)
                  xl = find_cspline_value(level.d[lev], maxpos, x2l, &cspl);
                else
                  xl = -DBL_MAX;
              } else
                xl = -DBL_MAX;

              if (ir < srcinfo.rdims[0]) { // not yet at edge
                do {
                  find_cspline_extremes(x1r, x2r, NULL, NULL, &maxpos,
                                        &max, &cspl);
                  if (max < level.d[lev]) {
                    ir++;
                    if (x.d) {
                      x1r = x.d[ir];
                      x2r = x.d[ir + 1];
                    } else {
                      x1r = ir;
                      x2r = ir + 1;
                    }
                  } else
                    break;
                } while (ir < srcinfo.rdims[0]);
                if (ir < srcinfo.rdims[0])
                  xr = find_cspline_value(level.d[lev], x1r, maxpos, &cspl);
                else
                  xr = -DBL_MAX;        // flag: at edge
              } else
                xr = -DBL_MAX;

              if (xl > -DBL_MAX && xr > -DBL_MAX) { // not at edge
                *trgt.d = (xl + xr)/2;
                if (width.d)
                  *width.d++ = xr - xl;
              } else {
                if (width.d)
                  *width.d++ = 0;
                *trgt.d = -1;   // not found
              }
            }
            trgt.d++;
          } // end of if (*ptr.d > level.d[lev]) else
        } // end of for (lev = 0; ...)
        src.d += step*srcinfo.rdims[0];
      } while (srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
      break;
  default:
    break;
  }
  cleanup_cubic_spline_tables(&cspl);
  return result;
#else
  return cerror(NOSUPPORT, 0, "BISECT", "libgsl");
#endif
}
//---------------------------------------------------------------------
static int32_t cmp0(const void *a, const void *b)
{
  struct c { double v; int32_t i32; } aa, bb;
  int32_t d;

  aa = *(struct c *) a;
  bb = *(struct c *) b;
  d = aa.i32 - bb.i32;
  if (d)
    return d;
  else {
    double dl;
    dl = aa.v - bb.v;
    if (dl < 0)
      return -1;
    if (dl > 0)
      return +1;
  }
  return 0;
}
//---------------------------------------------------------------------
/// \ingroup luxroutines
///
/// Implements the `cspline_find` function in LUX.
///
///     y = cspline_find([x,] y, values [, axis=axis, pos=pos, width=width])
///
/// Depends on the Gnu Scientific Library.
///
/// \param[in] narg is the number of arguments.
///
/// \param[in,out] points at the LUX symbol number of the first argument.
///
/// \returns the LUX symbol number of the result.
int32_t lux_cspline_find(int32_t narg, int32_t ps[])
// z = CSPLINE_FIND(y, levels [, AXIS=axis, INDEX=index])
/* locates positions where a certain value gets attained, using cubic
   splines
   <y> = data values
   <levels> = the values to look for in the y coordinate
   <axis> = the single axis (of <y>) along which to look
   <index> = the indices into <z> where the data for each level begins

   The result <z> has its 1st dimension equal to the number of
   dimensions in <y> and its 2nd dimension equal to the number of
   found locations.  The locations are sorted by the level and secondarily
   by the coordinates.  The locations for level <level(i)> begin at
   index <index(i)> and run up to but not including index <index(i+1)>. */
// LS 2009-08-09
{
#if HAVE_LIBGSL
  int32_t       result, iq, nLev, lev, ySym, vSym, i, step, *index, j;
  Pointer       src, level;
  csplineInfo   cspl;
  LoopInfo      srcinfo;
  Bytestack b;
  struct c { double v; int32_t l; int32_t c; } *c;
  int32_t csize;

  ySym = ps[0];                 // <y>
  vSym = ps[1];                 // <values>

  if (!symbolIsNumericalArray(ySym))
    return cerror(NEED_NUM_ARR, ySym);

  cspl = empty_cubic_spline();

  if (standardLoop(ySym, (narg > 2 && ps[2])? ps[2]: LUX_ZERO,
                   SL_NEGONED | SL_ONEAXIS | SL_SRCUPGRADE
                   | SL_AXISCOORD, LUX_FLOAT,
                   &srcinfo, &src, NULL, NULL, NULL) < 0) // <data>, <axis>
    return LUX_ERROR;           // some error

  if (!symbolIsNumerical(vSym)) // <levels>
    return LUX_ERROR;
  iq = lux_converts[srcinfo.type](1, &vSym); // ensure proper type

  numerical(iq, NULL, NULL, &nLev, &level);

  if (narg > 3 && ps[3]) {      // <index>
    if (to_scratch_array(ps[3], LUX_INT32, 1, &nLev) == LUX_ERROR)
      return LUX_ERROR;
    index = (int32_t *) array_data(ps[3]);
    memset(index, 0, srcinfo.ndim*sizeof(int32_t));
  } else
    index = NULL;

  // we don't know beforehand how many output values there will be
  b = Bytestack_create();       // so store them on a Byte stack

  step = srcinfo.rsinglestep[0];

  /* we'll store the data as follows on the Byte stack:
     1. the found location in the target dimension (double)
     2. the index of the level of which this is the location (int32_t)
     3. the (one or more) coordinates of the location (int32_t) */
  {
    struct c cc;
    csize = (uint8_t *) &cc.c - (uint8_t *) &cc.v + srcinfo.ndim*sizeof(int32_t);
  }
  c = (struct c*) malloc(csize);

  // now do the work
  switch (srcinfo.type) {
    case LUX_FLOAT:
      do {
        // install table for cubic spline interpolation
        cubic_spline_tables(NULL, srcinfo.type, 1,
                            src.f, srcinfo.type, step,
                            srcinfo.rdims[0], 0, 0,
                            &cspl);
        // the levels are assumed to be sorted in ascending order
        do {
          if (!srcinfo.coords[0]) {
            for (lev = 0; lev < nLev && *src.f > level.f[lev]; lev++) ;
            // now level.f[lev - 1] <= *src.f < level.f[lev]
          } else {
            double z;

            if (lev > 0 && *src.f < level.f[lev - 1]) {
              /* passed a target level going down: determine &
                 remember detailed location */
              z = find_cspline_value(level.f[lev - 1],
                                     srcinfo.coords[0] - 1,
                                     srcinfo.coords[0],
                                     &cspl);
              c->v = z;                 // store the found location
              c->l = --lev;     // and the level index
              // and after that the coordinates
              for (j = 0; j < srcinfo.ndim; j++)
                (&c->c)[srcinfo.raxes[j]] = srcinfo.coords[j];
              Bytestack_push_data(b, c, (uint8_t *) c + csize);
            } else if (lev < nLev && *src.f >= level.f[lev]) {
              /* passed a target level going up: determine &
                 remember detailed location */
              z = find_cspline_value(level.f[lev],
                                     srcinfo.coords[0] - 1,
                                     srcinfo.coords[0],
                                     &cspl);
              c->v = z;
              c->l = lev++;
              for (j = 0; j < srcinfo.ndim; j++)
                (&c->c)[srcinfo.raxes[j]] = srcinfo.coords[j];
              Bytestack_push_data(b, c, (uint8_t *) c + csize);
            }
          }
        } while ((i = srcinfo.advanceLoop(&src.ui8)) == 0);
      } while (i < srcinfo.rndim);
      break;
  case LUX_DOUBLE:
      do {
        // install table for cubic spline interpolation
        cubic_spline_tables(NULL, srcinfo.type, 1,
                            src.d, srcinfo.type, step,
                            srcinfo.rdims[0], 0, 0,
                            &cspl);
        // the levels are assumed to be sorted in ascending order
        do {
          if (!srcinfo.coords[0]) {
            for (lev = 0; lev < nLev && *src.d > level.d[lev]; lev++) ;
            // now level.d[lev - 1] <= *src.d < level.d[lev]
          } else {
            double z;

            if (lev > 0 && *src.d < level.d[lev - 1]) {
              /* passed a target level going down: determine &
                 remember detailed location */
              z = find_cspline_value(level.d[lev - 1],
                                     srcinfo.coords[0] - 1,
                                     srcinfo.coords[0],
                                     &cspl);
              c->v = z;
              c->l = --lev;
              for (j = 0; j < srcinfo.ndim; j++)
                (&c->c)[srcinfo.raxes[j]] = srcinfo.coords[j];
              Bytestack_push_data(b, c, (uint8_t *) c + csize);
            } else if (lev < nLev && *src.d >= level.d[lev]) {
              /* passed a target level going up: determine &
                 remember detailed location */
              z = find_cspline_value(level.d[lev],
                                     srcinfo.coords[0] - 1,
                                     srcinfo.coords[0],
                                     &cspl);
              c->v = z;
              c->l = lev++;
              for (j = 0; j < srcinfo.ndim; j++)
                (&c->c)[srcinfo.raxes[j]] = srcinfo.coords[j];
              Bytestack_push_data(b, c, (uint8_t *) c + csize);
            }
          }
        } while ((i = srcinfo.advanceLoop(&src.ui8)) == 0);
      } while (i < srcinfo.rndim);
    break;
  default:
    break;
  }
  cleanup_cubic_spline_tables(&cspl);

  /* now we have <n> positions in the order in which we found them.
     We want to sort them first by the level and then by the position,
     so that we first have the positions for the first level, then
     those for the second level, and so on. */
  {
    int32_t n;
    struct c *d;

    n = Bytestack_bytes(b, 0)/csize; // number of found data points
    if (n > 0) {
      Bytestack_index bi;
      union {
        struct c *c;
        uint8_t *b;
      } q;

      d = (struct c *) Bytestack_peek(b, 0); // beginning of data
      qsort(d, n, csize, cmp0);

      // create output symbol
      bi = Bytestack_top(NULL);
      if (srcinfo.ndim > 1)
        Bytestack_push_var(NULL, srcinfo.ndim);
      Bytestack_push_var(NULL, n);
      result = array_scratch(LUX_DOUBLE, (srcinfo.ndim > 1? 2: 1),
                             (int32_t *) Bytestack_pop(NULL, bi));
      src.d = (double*) array_data(result);
      q.c = d;
      for (i = 0; i < n; i++) {
        int32_t j;
        for (j = 0; j < srcinfo.ndim; j++) {
          if (j == srcinfo.raxes[0])
            *src.d++ = q.c->v;
          else
            *src.d++ = ((&q.c->c)[j]);
        }
        q.b += csize;
      }
      if (index) {
        index[0] = 0;
        if (nLev > 1) {
          lev = 0;
          for (i = 0; i < n; i++) {
            while (d->l != lev)
              index[++lev] = i;
            d = (struct c *) ((uint8_t *)d + csize);
          }
        }
      }
    } else
      result = LUX_MINUS_ONE;
  }

  Bytestack_delete(b);
  free(c);

  return result;
#else
  return cerror(NOSUPPORT, 0, "CSPLINE_FIND", "libgsl");
#endif
}
//----------------------------------------------------------------
/// \ingroup luxroutines
///
/// Implements the `monotoneinterpolate` function in LUX.
///
///     yi = monotoneinterpolate(x,y,xnew[,/none,/circle,/square,/wide])
///
/// \param[in] narg is the number of arguments.
///
/// \param[in,out] points at the LUX symbol number of the first argument.
///
/// \returns the LUX symbol number of the result.
int32_t lux_monotone_interpolation(int32_t narg, int32_t ps[])
{
  Pointer *ptrs;
  LoopInfo* infos;

  StandardArguments sa(narg, ps, "i>D*;i>D&;i>D*;rD[2]&",
                            &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;

  MonotoneInterpolation::MonotoneMethodSelection method;
  switch (internalMode) {
  case 1:
    method = MonotoneInterpolation::MonotoneMethodSelection::NONE;
    break;
  case 2:
    method = MonotoneInterpolation::MonotoneMethodSelection::CIRCLE;
    break;
  case 4:
    method = MonotoneInterpolation::MonotoneMethodSelection::SQUARE;
    break;
  case 8:
    method = MonotoneInterpolation::MonotoneMethodSelection::WIDE;
    break;
  default:
    method = MonotoneInterpolation::MonotoneMethodSelection::FULL;
    break;
  }

  MonotoneInterpolation mi(infos[0].nelem, ptrs[0].d, ptrs[1].d, method);

  size_t nelem = infos[2].nelem;
  while (nelem--) {
    *ptrs[3].d++ = mi.interpolate(*ptrs[2].d++);
  }

  return sa.result();
}
REGISTER(monotone_interpolation, f, monotoneinterpolate, 3, 3, "1none:2circle:4square:8wide:16full");
//--------------------------------------------------------------------------
#ifdef WORDS_BIGENDIAN
#define SYNCH_OK        0xaaaa5555
#define SYNCH_REVERSE   0x5555aaaa
#else
#define SYNCH_OK        0x5555aaaa
#define SYNCH_REVERSE   0xaaaa5555
#endif
int32_t lux_fitskey(int32_t narg, int32_t ps[])
// FITSKEY(file, key) returns the value associated with the string <key>
/* in FITS file <file>.  If <file> is a string, then it is taken as the
   name of the FITS file.  If <file> is a scalar, then its (integer) value
   is taken as the lun on which the FITS file is assumed to be opened.
   If a lun is specified, then the file pointer is left in the same position
   it was found in.
   Returns 0 if (1) the file cannot be opened */
// for reading, (2) the file is not a FITS file (does not have "SIMPLE "
// as its first keyword), (3) the key is not a string, or (4) the key has
// more than 8 characters.  If the keyword is not found, then a 0 is
// returned.  The key is transformed to all uppercase before it is sought
// in the file.  LS 4jun98
{
  char  *file, *key, *key2, *scr, mustclose, ok;
  int32_t       n, n2, i, evalString(char *, int32_t), ptr, iq, i0;
  Symboltype type;
  Pointer       p;
  Scalar        value;
  FILE  *fp;
  void  read_a_number(char **buf, Scalar *value, Symboltype *type);

  switch (symbol_class(ps[0])) {
    case LUX_STRING:
      file = expand_name(string_value(ps[0]), NULL); // full file name
      fp = fopen(file, "r");
      if (!fp)
        return LUX_ZERO;
      mustclose = 1;            // must close the file again when done
      break;
    case LUX_SCALAR:
      i = int_arg(ps[0]);
      if (i < 0 || i >= MAXFILES)
        return LUX_ZERO;
      fp = lux_file[i];
      if (!fp)
        return LUX_ZERO;
      ptr = ftell(fp);          // current file pointer position
      fseek(fp, 0, SEEK_SET);
      mustclose = 0;            // leave file open when done
      break;
    default:
      return LUX_ZERO;
  }
  if (symbol_class(ps[1]) != LUX_STRING) { // <key> must be a string
    if (mustclose)
      fclose(fp);
    else
      fseek(fp, ptr, SEEK_SET);
    return LUX_ZERO;
  }
  n = string_size(ps[1]);
  if (n > 8) {
    if (mustclose)
      fclose(fp);
    else
      fseek(fp, ptr, SEEK_SET);
    return LUX_ZERO;
  }

  key = string_value(ps[1]);
  n2 = (n < 8)? n + 1: 8;       // because we add a space if it fits
  key2 = (char*) malloc(n2 + 1);
  strcpy(key2, key);
  if (n2 != n) {                // add a space at the end to prevent
                                // partial matches
    key2[n] = ' ';
    key2[n + 1] = '\0';
  }
  for (key = key2; *key; key++)         // make all uppercase
    *key = toupper(*key);

  scr = curScrat;
  scr = fgets(scr, 9, fp);

  ok = 1;
  i0 = 0;                       // default offset is 0
  if (!scr)
    ok = 0;
  else if (strncmp(scr, "SIMPLE  ", 8)) { // no expected FITS key
    // we'll accept an FZ file if its has a FITS header
    p.s = scr;
    if (*p.i32 == SYNCH_OK) {     // have an FZ file.  OK header?
      fseek(fp, 256, SEEK_SET); // to start of header
      scr = fgets(scr, 9, fp);
      if (!scr || strncmp(scr, "SIMPLE  ", 8)) // not OK
        ok = 0;
      else                      // we have an FZ FITS header: adjust offset
        i0 = 256;
    }
  }
  if (!ok) {                    // something was wrong
    if (mustclose)
      fclose(fp);
    else
      fseek(fp, ptr, SEEK_SET);
    free(key2);
    return LUX_ZERO;
  }
  i = 0;

  do {
    if (fseek(fp, i*80 + i0, SEEK_SET)) {
      if (mustclose)
        fclose(fp);
      else
        fseek(fp, ptr, SEEK_SET);
      free(key2);
      return LUX_ZERO;
    }
    i++;
    scr = fgets(scr, 80, fp);
    if (!scr) {
      if (mustclose)
        fclose(fp);
      else
        fseek(fp, ptr, SEEK_SET);
      free(key2);
      return LUX_ZERO;
    }
  } while (strncmp(scr, "END ", 4) && strncmp(scr, key2, n2));

  if (mustclose)
    fclose(fp);
  else
    fseek(fp, ptr, SEEK_SET);
  free(key2);
  if (!strncmp(scr, "END ", 4))         // found end of header but not the keyword
    return LUX_ZERO;

  /* The FITS rules say that a comment is introduced by a forward slash.
     String constants are enclosed in single quotes ('' inside a string refers
     to a literal single quote), so we must look for the first forward slash
     outside of a string.  LS 14jan99 */
  // if /COMMENT is specified, then we must return the comment value;
  // otherwise the data value.  Return comments always as strings.
  // LS 26may99
  scr += 9;                     // beginning of data value
  n = 0;
  while (isspace((int32_t) *scr))
    scr++;
  key = scr;
  while (*key) {
    switch (*key) {
      case '\'':                // literal text string
        n = !n;                         // toggle string status
        break;
      case '/':                         // maybe FITS comment
        if (!n) {               // not in a literal text string
          if (internalMode & 1) { // /COMMENT
            scr = key + 1;      // start reading here
            while (isspace((int32_t) *scr))
              scr++;            // skip initial whitespace
            key = scr + strlen(scr) - 1; // skip final whitespace
            while (key > scr && isspace((int32_t) *key))
              key--;
            key[1] = '\0';
            iq = string_scratch(strlen(scr));
            strcpy(string_value(iq), scr);
            return iq;
          } else {
            while (key > scr
                   && isspace((int32_t) key[-1])) // skip trailing spaces
              key--;
            *key-- = '\0';      // terminate data value
          }
        }
        break;
    }
    key++;
  }
  // the data may be numerical, or a quote-enclosed string, or the values
  // T for true or F for false.  We return non-quoted non-numerical text
  // as a string
  key = scr;
  switch (*key) {
    case '\'':                  // a string
      scr = ++key;
      while (*key && *key != '\'') // find the closing quote
        key++;
      *key = '\0';              // terminate
      break;
    case '-': case '+':                 // may be a number
      scr = key++;
      while (isspace((int32_t) *key)) // skip whitespace
        key++;
      if (!isdigit((int32_t) *key)) { // treat it as a string
        key += strlen(key);     // go to the end of the string
        while (key > scr
               && isspace((int32_t) key[-1])) // skip trailing whitespace
          key--;
        break;
      }
      // else we treat it as a number; fall through to the next case
    case '0': case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9':
      read_a_number(&scr, &value, &type);
      iq = scalar_scratch(type);
      switch (type) {
        case LUX_INT8:
          scalar_value(iq).ui8 = (uint8_t) value.i32;
          break;
        case LUX_INT16:
          scalar_value(iq).i16 = (int16_t) value.i32;
          break;
        case LUX_INT32:
          scalar_value(iq).i32 = value.i32;
          break;
        case LUX_INT64:
          scalar_value(iq).i64 = value.i32;
          break;
        case LUX_FLOAT:
          scalar_value(iq).f = (float) value.d;
          break;
        case LUX_CFLOAT:
          complex_scalar_data(iq).cf->real = 0.0;
          complex_scalar_data(iq).cf->imaginary = (float) value.d;
          break;
        case LUX_CDOUBLE:
          complex_scalar_data(iq).cd->real = 0.0;
          complex_scalar_data(iq).cd->imaginary = value.d;
          break;
      }
      return iq;
      break;
  }

  iq = string_scratch(strlen(scr));
  strcpy(string_value(iq), scr); // copy string
  return iq;
}
//--------------------------------------------------------------------------
#define LEFT    0
#define DOWN    1
#define RIGHT   2
#define UP      3
#define CENTER  4
#define DONE    5

int32_t sign(float x)
{
  if (x > 0)
    return 1;
  else if (x < 0)
    return -1;
  else
    return 0;
}
//--------------------------------------------------------------------
int32_t sgnclass(float x)
{
  if (x > 0)
    return 2;
  else if (x < 0)
    return 0;
  else
    return 1;
}
//--------------------------------------------------------------------
int32_t traverseElement(float xin, float yin, float vx, float vy,
                    float *xout, float *yout)
/* if you start at position (<xin>,<yin>), with 0 <= <xin>,<yin> <= 1,
   and move in the direction given by (<vx>,<vy>), then this routine
   determines which pixel boundary you cross (UP, DOWN, LEFT, RIGHT,
   or CENTER if <vx> = <vy> = 0) and what the coordinates are.  The
   new coordinates are returned in <*xout> and <*yout> and the pixel
   boundary code is the return value of the routine.  LS 17oct98 */
// We must make sure that the edges are treated properly, too.
// We do that by moving the edges outward a tiny bit so that our
// data point <xin>,<yin> is never actually one an edge.
{
  if (vx > 0) {                         // to the right
    /* first we check for UP.  The vector that separates UP from RIGHT
       has coordinates (1 - xin, 1 - yin); rotated counterclockwise
       over 90 degrees this becomes (yin - 1, 1 - xin): if the inner
       product of the velocity vector (vx,vy) with this vector is
       positive, then we are going UP.  The vector that separates
       RIGHT from DOWN is (1-xin,-yin), which leads to (yin, 1-xin). */
    if (vx*(yin - 1 - FLT_EPSILON) + vy*(1 + FLT_EPSILON - xin) > 0) {
      // going UP
      *xout = vy? xin + (1 - yin)/vy*vx: 1;
      *yout = 1;
      return UP;
    } else if (vx*(yin + FLT_EPSILON) + vy*(1 + FLT_EPSILON - xin) > 0) {
      // we're going RIGHT
      *xout = 1;
      *yout = vx? yin + (1 - xin)/vx*vy: 0;
      return RIGHT;
    } else {
      *xout = vy? xin - yin/vy*vx: 1;
      *yout = 0;
      return DOWN;
    }
  }

  if (vx < 0) {                 // to the left
    // first we check for DOWN.  The vector that separates DOWN from LEFT
    // has coordinates (-xin,-yin); rotated counterclockwise over 90
    // degrees this becomes (yin,-xin): if the inner product of the velocity
    // vector (vx,vy) with this vector is positive, then we are going DOWN.
    // The vector that separates LEFT from UP is (-xin,1-yin), which leads
    // to (yin-1,-xin).
    if (vx*(yin + FLT_EPSILON) - vy*(xin + FLT_EPSILON) > 0) {  // DOWN
      *xout = vy? xin - yin/vy*vx: 0;
      *yout = 0;
      return DOWN;
    } else if (vx*(yin + FLT_EPSILON - 1) - vy*(xin + FLT_EPSILON) > 0) {
      // LEFT
      *xout = 0;
      *yout = vx? yin - xin/vx*vy: 1;
      return LEFT;
    } else {                    // UP
      *xout = vy? xin + (1 - yin)/vy*vx: 0;
      *yout = 1;
      return UP;
    }
  }

  if (vy > 0) {                 // straight up
    *xout = xin;
    *yout = 1;
    return UP;
  }

  if (vy < 0) {                 // straight down
    *xout = xin;
    *yout = 0;
    return DOWN;
  }
  // no movement at all
  *xout = xin;
  *yout = yin;
  return CENTER;
}
//--------------------------------------------------------------------
#define FACTOR  (0.886226925)   // 0.5*sqrt(pi)
int32_t lux_dir_smooth(int32_t narg, int32_t ps[])
/* Y = DSMOOTH(<data>,<vx>,<vy> [, /TWOSIDED, /ONESIDED, /BOXCAR, /GAUSSIAN,
               /NORMALIZE])
   smooths 2D image <data> in the direction indicated by the
   angle <vx> and <vy>, over a distance indicated by the magnitude of vector
   <v>.  The vector <vx>,<vy> indicates a flow field.  Data along the
   flow line through the data point under consideration is weighted into
   a result for the data point under consideration.  /TWOSIDED (the default)
   indicates that data on both sides of the data point under consideration
   must be weighted into the result.  /ONESIDED means that only data on
   the side indicated by the direction of the vector field at the point
   under consideration must be included.  /BOXCAR (default) indicates that
   boxcar smoothing is to be used; /GAUSSIAN selects gaussian smoothing
   instead.

LS 9nov98 */
{
  int32_t       iq, nx, ny, ix, iy, c, index, rindex, count, twosided, total,
    gaussian, iq0, di, straight;
  float         x1, y1, x2, y2, *vx0, *vy0, value, vx, vy, s, s0, ds, dslimit,
    weight, ws, s1;
  Pointer       src, trgt, src0;
  LoopInfo      srcinfo, trgtinfo;

  iq0 = ps[0];                  // data
  if (symbol_class(iq0) != LUX_ARRAY // not an array
      || array_num_dims(iq0) != 2) // or doesn't have 2 dimensions
    return cerror(NEED_2D_ARR, iq0);
  iq0 = lux_float(1, &iq0);
  nx = array_dims(iq0)[0];
  ny = array_dims(iq0)[1];

  iq = ps[1];                   // vx
  if (symbol_class(iq) != LUX_ARRAY
      || array_num_dims(iq) != 2
      || array_dims(iq)[0] != nx
      || array_dims(iq)[1] != ny)
    return cerror(INCMP_ARG, iq);
  iq = lux_float(1, &iq);
  vx0 = (float*) array_data(iq);

  iq = ps[2];                   // vy
  if (symbol_class(iq) != LUX_ARRAY
      || array_num_dims(iq) != 2
      || array_dims(iq)[0] != nx
      || array_dims(iq)[1] != ny)
    return cerror(INCMP_ARG, iq);
  iq = lux_float(1, &iq);
  vy0 = (float*) array_data(iq);

  if (standardLoop(iq0, 0, SL_ALLAXES | SL_SAMEDIMS | SL_EXACT | SL_EACHCOORD,
                   LUX_FLOAT, &srcinfo, &src, &iq, &trgtinfo, &trgt) < 0)
    return LUX_ERROR;
  src0.f = src.f;

  twosided = ((internalMode & 1) == 0); // /TWOSIDED
  total = (internalMode & 4);   // /TOTAL
  gaussian = (internalMode & 2); // /GAUSSIAN
  straight = (internalMode & 16); // /STRAIGHT

  if (!gaussian)                // boxcar
    do {
      count = twosided + 1;
      /* we work up to the desired smoothing width by measuring the path
         lengths in each pixel and adding them up until we reach the
         desired path length.  This scheme backfires when the velocities
         at some point circle the common corner of four pixels: then the
         algorithm can keep circling that point in very small steps for
         a very long time.  If many small steps follow one another in
         succession, then we are probably in such a situation and it is
         then unlikely that we'll get out of it any time soon.  We guard
         against this by maintaining an exponentially weighted average
         of the past steps and quitting if the weighted average gets too
         small.  We use an exponential decay scale of 2 steps and
         a limit value of 0.2. */
      dslimit = 1.0;            // current weighted average of step sizes
      value = 0.0;
      *trgt.f = 0.0;
      while (count--) {
        rindex = 0;             // index relative to current start location
        ix = srcinfo.coords[0];         // x pixel coordinate
        iy = srcinfo.coords[1];         // y pixel coordinate
        index = src.f - src0.f;         // index relative to data start

        x1 = 0.5;               // x coordinate in pixel (between 0 and 1)
        y1 = 0.5;               // y coordinate in pixel (between 0 and 1)
        vx = vx0[index];        // x velocity
        vy = vy0[index];        // y velocity
        if (count) {
          vx = -vx;
          vy = -vy;
        }

        s0 = 0.5*hypot(vx, vy); // length indicates smoothing width
        s1 = s0;
        s = 0.0;
        
        while (s < s1) {
          c = traverseElement(x1, y1, vx, vy, &x2, &y2);
          // calculate distance inside the current pixel
          x1 -= x2;
          y1 -= y2;
          ds = hypot(x1, y1);
          if (s + ds > s1)
            ds = s0 - s;
          dslimit = 0.5*(dslimit + ds);
          if (dslimit < 0.2) {  // we fear a semi-infinite loop here
            value += src.f[rindex]*(s1 - s);
            s = s1;             // we break it off
            continue;
          }
          switch (c) {
            case UP:
              if (iy == ny - 1) { // already at top
                value += src.f[rindex]*ds;
                s += ds;
                s1 = s;
                continue;
              }
              di = nx;
              x1 = x2;
              y1 = 0.0;
              iy++;
              break;
            case RIGHT:
              if (ix == nx - 1) { // already at right edge
                value += src.f[rindex]*ds;
                s += ds;
                s1 = s;
                continue;
              }
              di = 1;
              x1 = 0.0;
              y1 = y2;
              ix++;
              break;
            case DOWN:
              if (iy == 0) {    // already at bottom
                value += src.f[rindex]*ds;
                s += ds;
                s1 = s;
                continue;
              }
              di = -nx;
              x1 = x2;
              y1 = 1.0;
              iy--;
              break;
            case LEFT:
              if (ix == 0) {    // already at left edge
                value += src.f[rindex]*ds;
                s += ds;
                s1 = s;
                continue;
              }
              di = -1;
              x1 = 1.0;
              y1 = y2;
              ix--;
              break;
            case CENTER:
              value += src.f[rindex]*(s1 - s);
              s1 = s;
              continue;
          } // end of switch (c)
          value += src.f[rindex]*ds;
          index += di;
          rindex += di;
          s += ds;
          if (!straight) {
            vx = vx0[index];
            vy = vy0[index];
            if (count) {
              vx = -vx;
              vy = -vy;
            }
          } // end of if (straight)
        } // end of while (s < s0)
      } // end of while (count--)
      if (!total) {
        value /= s1;
        if (twosided)
          value *= 0.5;
      }
      *trgt.f = value;
    } while (trgtinfo.advanceLoop(&trgt.ui8),
             srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
  else                          // gaussian smoothing
    do {
      count = twosided + 1;
      /* we work up to the desired smoothing width by measuring the path
         lengths in each pixel and adding them up until we reach the
         desired path length.  This scheme backfires when the velocities
         at some point circle the common corner of four pixels: then the
         algorithm can keep circling that point in very small steps for
         a very long time.  If many small steps follow one another in
         succession, then we are probably in such a situation and it is
         then unlikely that we'll get out of it any time soon.  We guard
         against this by maintaining an exponentially weighted average
         of the past steps and quitting if the weighted average gets too
         small.  We use an exponential decay scale of 2 steps and
         a limit value of 0.2. */
      dslimit = 1.0;            // current weighted average of step sizes
      value = 0.0;
      ws = 0.0;
      while (count--) {
        rindex = 0;             // index relative to current start location
        ix = srcinfo.coords[0];         // x pixel coordinate
        iy = srcinfo.coords[1];         // y pixel coordinate
        index = src.f - src0.f;         // index relative to data start

        x1 = 0.5;               // x coordinate in pixel (between 0 and 1)
        y1 = 0.5;               // y coordinate in pixel (between 0 and 1)
        vx = vx0[index];        // x velocity
        vy = vy0[index];        // y velocity
        if (count) {
          vx = -vx;
          vy = -vy;
        }

        s0 = 0.6005612*hypot(vx, vy); // smoothing width
        s = 0.0;
        s1 = 4*s0;
        
        while (s < s1) {
          c = traverseElement(x1, y1, vx, vy, &x2, &y2);
          // calculate distance inside the current pixel
          x1 -= x2;
          y1 -= y2;
          ds = hypot(x1, y1);
          if (s + ds > s1)
            ds = s1 - s;
        /* we must determine the weight to assign to the contributions.
           if gsmooth is used in one dimension, then the weights of the
           contributions are equal to exp(-(x/s0)^2) where x is the
           integer index of the data point relative to the point under
           consideration.  However, if this routine is used to mimic gsmooth,
           then s is only equal to 0.5 when x = +1 is reached, so
           using exp(-(s/s0)^2) with s the total distance from the central
           point does not yield good results.  We can't just add 0.5 to
           s from the beginning either, because we do want a weight of 1
           for the first included data. */
          weight = s? (s + 0.5)/s0: 0.0;
          weight = exp(-weight*weight);
          dslimit = 0.5*(dslimit + ds);
          if (dslimit < 0.2) {  // we fear a semi-infinite loop here
            ds = 0.5;
            if (s + ds > s1)
              ds = s1 - s;
          }
          switch (c) {
            case UP:
              if (iy == ny - 1) { // already at top
                value += src.f[rindex]*weight*ds;
                ws += weight*ds;
                s += ds;
                s1 = s;
                continue;
              }
              di = nx;
              x1 = x2;
              y1 = 0.0;
              iy++;
              break;
            case RIGHT:
              if (ix == nx - 1) { // already at right edge
                value += src.f[rindex]*weight*ds;
                ws += weight*ds;
                s += ds;
                s1 = s;
                continue;
              }
              di = 1;
              x1 = 0.0;
              y1 = y2;
              ix++;
              break;
            case DOWN:
              if (iy == 0) {    // already at bottom
                value += src.f[rindex]*weight*ds;
                ws += weight*ds;
                s += ds;
                s1 = s;
                continue;
              }
              di = -nx;
              x1 = x2;
              y1 = 1.0;
              iy--;
              break;
            case LEFT:
              if (ix == 0) {    // already at left edge
                value += src.f[rindex]*weight*ds;
                ws += weight*ds;
                s += ds;
                s1 = s;
                continue;
              }
              di = -1;
              x1 = 1.0;
              y1 = y2;
              ix--;
              break;
            case CENTER:
              di = 0;
              break;
          } // end of switch (c)
          value += src.f[rindex]*weight*ds;
          rindex += di;
          index += di;
          ws += weight*ds;
          s += ds;
          if (!straight) {
            vx = vx0[index];
            vy = vy0[index];
            if (count) {
              vx = -vx;
              vy = -vy;
            } // end of if (count)
          } // end of if (straight)
        } // end of while (d < DONE)
      } // end of while (count--)
      if (!total)
        value /= ws;
      *trgt.f = value;
    } while (trgtinfo.advanceLoop(&trgt.ui8),
             srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
  return iq;
}
//--------------------------------------------------------------------
int32_t lux_dir_smooth2(int32_t narg, int32_t ps[])
/* Y = LSMOOTH(<data>,<vx>,<vy>)
   smooths 2D image <data> in the direction indicated by the
   angle <vx> and <vy>, over a distance indicated by the magnitude of vector
   <v>. */
{
  int32_t       iq, nx, ny, ix, iy, c, index, rindex, count, twosided, normalize,
    gaussian, iq0, di, straight;
  float         x1, y1, x2, y2, *vx0, *vy0, vx, vy, s, s0, ds, dslimit,
    weight, ws, s1, norm;
  Pointer       src, trgt, src0;
  LoopInfo      srcinfo, trgtinfo;

  iq0 = ps[0];                  // data
  if (symbol_class(iq0) != LUX_ARRAY // not an array
      || array_num_dims(iq0) != 2) // or doesn't have 2 dimensions
    return cerror(NEED_2D_ARR, iq0);
  iq0 = lux_float(1, &iq0);
  nx = array_dims(iq0)[0];
  ny = array_dims(iq0)[1];

  iq = ps[1];                   // vx
  if (symbol_class(iq) != LUX_ARRAY
      || array_num_dims(iq) != 2
      || array_dims(iq)[0] != nx
      || array_dims(iq)[1] != ny)
    return cerror(INCMP_ARG, iq);
  iq = lux_float(1, &iq);
  vx0 = (float*) array_data(iq);

  iq = ps[2];                   // vy
  if (symbol_class(iq) != LUX_ARRAY
      || array_num_dims(iq) != 2
      || array_dims(iq)[0] != nx
      || array_dims(iq)[1] != ny)
    return cerror(INCMP_ARG, iq);
  iq = lux_float(1, &iq);
  vy0 = (float*) array_data(iq);

  if (standardLoop(iq0, 0, SL_ALLAXES | SL_SAMEDIMS | SL_EXACT | SL_EACHCOORD,
                   LUX_FLOAT, &srcinfo, &src, &iq, &trgtinfo, &trgt) < 0)
    return LUX_ERROR;
  src0.f = src.f;

  twosided = ((internalMode & 1) == 0); // /TWOSIDED
  normalize = (internalMode & 4)? 1: 0;         // /NORMALIZE
  gaussian = (internalMode & 2)? 1: 0; // /GAUSSIAN
  straight = (internalMode & 8);

  zerobytes(trgt.f, array_size(iq)*sizeof(float)); // set to zero

  if (!gaussian) {              // boxcar
    if (!normalize)
      norm = 1.0;
    do {
      count = twosided + 1;
      /* we work up to the desired smoothing width by measuring the path
         lengths in each pixel and adding them up until we reach the
         desired path length.  This scheme backfires when the velocities
         at some point circle the common corner of four pixels: then the
         algorithm can keep circling that point in very small steps for
         a very long time.  If many small steps follow one another in
         succession, then we are probably in such a situation and it is
         then unlikely that we'll get out of it any time soon.  We guard
         against this by maintaining an exponentially weighted average
         of the past steps and quitting if the weighted average gets too
         small.  We use an exponential decay scale of 2 steps and
         a limit value of 0.2. */
      dslimit = 1.0;            // current weighted average of step sizes
      while (count--) {
        rindex = 0;             // index relative to current start location
        ix = srcinfo.coords[0];         // x pixel coordinate
        iy = srcinfo.coords[1];         // y pixel coordinate
        index = src.f - src0.f;         // index relative to data start

        x1 = 0.5;               // x coordinate in pixel (between 0 and 1)
        y1 = 0.5;               // y coordinate in pixel (between 0 and 1)
        vx = vx0[index];        // x velocity
        vy = vy0[index];        // y velocity
        if (count) {
          vx = -vx;
          vy = -vy;
        }

        s0 = 0.5*hypot(vx, vy); // smoothing width
        s1 = s0;
        s = 0.0;
        if (normalize)
          norm = s0? 0.5/s0: 1.0;
        
        while (s < s1) {
          c = traverseElement(x1, y1, vx, vy, &x2, &y2);
          // calculate distance inside the current pixel
          x1 -= x2;
          y1 -= y2;
          ds = hypot(x1, y1);
          if (s + ds > s1)
            ds = s0 - s;
          dslimit = 0.5*(dslimit + ds);
          if (dslimit < 0.2) {  // we fear a semi-infinite loop here
            trgt.f[rindex] += *src.f*(s1 - s)*norm;
            s = s1;             // we break it off
            continue;
          }
          switch (c) {
            case UP:
              if (iy == ny - 1) { // already at top
                trgt.f[rindex] += *src.f*ds*norm;
                s += ds;
                s1 = s;
                continue;
              }
              di = nx;
              x1 = x2;
              y1 = 0.0;
              iy++;
              break;
            case RIGHT:
              if (ix == nx - 1) { // already at right edge
                trgt.f[rindex] += *src.f*ds*norm;
                s += ds;
                s1 = s;
                continue;
              }
              di = 1;
              x1 = 0.0;
              y1 = y2;
              ix++;
              break;
            case DOWN:
              if (iy == 0) {    // already at bottom
                trgt.f[rindex] += *src.f*ds*norm;
                s += ds;
                s1 = s;
                continue;
              }
              di = -nx;
              x1 = x2;
              y1 = 1.0;
              iy--;
              break;
            case LEFT:
              if (ix == 0) {    // already at left edge
                trgt.f[rindex] += *src.f*ds*norm;
                s += ds;
                s1 = s;
                continue;
              }
              di = -1;
              x1 = 1.0;
              y1 = y2;
              ix--;
              break;
            case CENTER:
              trgt.f[rindex] += *src.f*(s1 - s)*norm;
              s1 = s;
              continue;
          } // end of switch (c)
          trgt.f[rindex] += *src.f*ds*norm;
          index += di;
          rindex += di;
          s += ds;
          if (!straight) {
            vx = vx0[index];
            vy = vy0[index];
            if (count) {
              vx = -vx;
              vy = -vy;
            }
          }
        } // end of while (s < s0)
      } // end of while (count--)
    } while (trgtinfo.advanceLoop(&trgt.ui8),
             srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
  } else {                      // gaussian smoothing
    norm = 0.5*M_2_SQRTPI;
    do {
      count = twosided + 1;
      /* we work up to the desired smoothing width by measuring the path
         lengths in each pixel and adding them up until we reach the
         desired path length.  This scheme backfires when the velocities
         at some point circle the common corner of four pixels: then the
         algorithm can keep circling that point in very small steps for
         a very long time.  If many small steps follow one another in
         succession, then we are probably in such a situation and it is
         then unlikely that we'll get out of it any time soon.  We guard
         against this by maintaining an exponentially weighted average
         of the past steps and quitting if the weighted average gets too
         small.  We use an exponential decay scale of 2 steps and
         a limit value of 0.2. */
      dslimit = 1.0;            // current weighted average of step sizes
      ws = 0.0;
      while (count--) {
        rindex = 0;             // index relative to current start location
        ix = srcinfo.coords[0];         // x pixel coordinate
        iy = srcinfo.coords[1];         // y pixel coordinate
        index = src.f - src0.f;         // index relative to data start

        x1 = 0.5;               // x coordinate in pixel (between 0 and 1)
        y1 = 0.5;               // y coordinate in pixel (between 0 and 1)
        vx = vx0[index];        // x velocity
        vy = vy0[index];        // y velocity
        if (count) {
          vx = -vx;
          vy = -vy;
        }

        s0 = 0.6005612*hypot(vx, vy);   // smoothing width
        s = 0.0;
        s1 = 4*s0;

        if (normalize)
          norm = s0? (0.5*M_2_SQRTPI)/s0: (0.5*M_2_SQRTPI);
        
        while (s < s1) {
          c = traverseElement(x1, y1, vx, vy, &x2, &y2);
          // calculate distance inside the current pixel
          x1 -= x2;
          y1 -= y2;
          ds = hypot(x1, y1);
          if (s + ds > s1)
            ds = s1 - s;
        /* we must determine the weight to assign to the contributions.
           if gsmooth is used in one dimension, then the weights of the
           contributions are equal to exp(-(x/s0)^2) where x is the
           integer index of the data point relative to the point under
           consideration.  However, if this routine is used to mimic gsmooth,
           then s is only equal to 0.5 when x = +1 is reached, so
           using exp(-(s/s0)^2) with s the total distance from the central
           point does not yield good results.  We can't just add 0.5 to
           s from the beginning either, because we do want a weight of 1
           for the first included data. */
          weight = s? (s + 0.5)/s0: 0.0;
          weight = exp(-weight*weight);
          dslimit = 0.5*(dslimit + ds);
          if (dslimit < 0.2) {  // we fear a semi-infinite loop here
            ds = 0.5;
            if (s + ds > s1)
              ds = s1 - s;
          }
          switch (c) {
            case UP:
              if (iy == ny - 1) { // already at top
                trgt.f[rindex] += *src.f*ds*weight*norm;
                ws += weight*ds;
                s += ds;
                s1 = s;
                continue;
              }
              di = nx;
              x1 = x2;
              y1 = 0.0;
              iy++;
              break;
            case RIGHT:
              if (ix == nx - 1) { // already at right edge
                trgt.f[rindex] += *src.f*ds*weight*norm;
                ws += weight*ds;
                s += ds;
                s1 = s;
                continue;
              }
              di = 1;
              x1 = 0.0;
              y1 = y2;
              ix++;
              break;
            case DOWN:
              if (iy == 0) {    // already at bottom
                trgt.f[rindex] += *src.f*ds*weight*norm;
                ws += weight*ds;
                s += ds;
                s1 = s;
                continue;
              }
              di = -nx;
              x1 = x2;
              y1 = 1.0;
              iy--;
              break;
            case LEFT:
              if (ix == 0) {    // already at left edge
                trgt.f[rindex] += *src.f*ds*weight*norm;
                ws += weight*ds;
                s += ds;
                s1 = s;
                continue;
              }
              di = -1;
              x1 = 1.0;
              y1 = y2;
              ix--;
              break;
            case CENTER:
              di = 0;
              break;
          } // end of switch (c)
          trgt.f[rindex] += *src.f*weight*ds*norm;
          rindex += di;
          index += di;
          ws += weight*ds;
          s += ds;
          if (!straight) {
            vx = vx0[index];
            vy = vy0[index];
            if (count) {
              vx = -vx;
              vy = -vy;
            }
          }
        } // end of while (d < DONE)
      } // end of while (count--)
    } while (trgtinfo.advanceLoop(&trgt.ui8),
             srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
  }
  return iq;
}
//--------------------------------------------------------------------
int32_t lux_trajectory(int32_t narg, int32_t ps[])
/* TRAJECTORY,<gx>,<gy>,<vx>,<vy>[,<n>][,<ox>,<oy>])
  Takes the positions indicated by <gx>,<gy> and advances them for <n>
  time steps according to the velocity field in <vx>,<vy>.  The first
  four arguments must be real arrays, and <n> must be a scalar and
  defaults to 1.  <gx> and <gy> must have the same number of elements.
  <vx> must have two dimensions.  <vy> must have the same dimensions
  as <vx>.  The first element of <vx> is assumed to reside at <gx>,<gy>
  coordinates equal to (0,0); the second element at (1,0), etcetera.
  The results are returned in <ox> and <oy> and get the
  dimensions of <gx> plus one extra dimension equal to <n> at the beginning
  (except if <n> is equal to 1).  <n> must be positive (after
  transformation to a LONG scalar).  If <n> exceeds 1, then <ox> and <oy>
  must be specified.  Otherwise, if <ox> and <oy> are not specified,
  then the results are stored in <gx> and <gy>.  LS 15-16dec99
*/
 /* the following argument lists are valid:
     gx,gy, v
     gx,gy,vx,vy
     gx,gy, v,ox,oy
     gx,gy, v, n,ox,oy
     gx,gy,vx,vy,ox,oy
     gx,gy,vx,vy, n,ox,oy
  */
{
  int32_t       iq, nx, ny, ix, iy, c, index, di, n, i, dims[MAX_DIMS],
    ngrid, dv;
  Symboltype type;
  float         x1, y1, x2, y2, vx, vy, s, s0, ds, dslimit, s1;
  Pointer       gx, gy, vx0, vy0, ox, oy;
  int32_t lux_convert(int32_t, int32_t [], Symboltype, int32_t);

  // we treat all arguments.
  if (!symbolIsRealArray(ps[0]))// <gx> must be a real array
    return cerror(ILL_CLASS, ps[0]);
  ngrid = array_size(ps[0]);    // number of grid points
  if (!symbolIsRealArray(ps[1]) || // <gy> must be a real array
      array_size(ps[1]) != ngrid) // with the same size as <gx>
    return cerror(INCMP_ARG, ps[1]);

  // we keep track of the greatest data type
  type = array_type(ps[0]);
  if (array_type(ps[1]) > type)
    type = array_type(ps[1]);

  if (!symbolIsRealArray(ps[2])) // <vx> must be a real array
    return cerror(NEED_ARR, ps[2]);
  if (array_type(ps[2]) > type)
    type = array_type(ps[2]);

  if (narg == 3 || narg == 5
      || (narg == 6 && symbolIsScalar(ps[3]))) {
    /* ps[2] is <v>; it must have 3 dimensions and the first one
     must have 2 elements. */
    if (array_num_dims(ps[2]) != 3)
      return luxerror("Need 3 dimensions here", ps[2]);
    if (array_dims(ps[2])[0] != 2)
      return luxerror("Need 2 elements in first dimension here", ps[2]);
    nx = array_dims(ps[2])[1];
    ny = array_dims(ps[2])[2];
  } else {
    /* ps[2] is <vx>, ps[3] is <vy>; both must have 2 dimensions and
       the dimensions of <vx> must equal those of <vy>. */
    if (!symbolIsRealArray(ps[3]))
      return cerror(NEED_ARR, ps[3]);
    if (array_num_dims(ps[2]) != 2)
      return cerror(NEED_2D_ARR, ps[2]);
    if (array_num_dims(ps[3]) != 2)
      return cerror(NEED_2D_ARR, ps[3]);
    nx = array_dims(ps[2])[0];
    ny = array_dims(ps[2])[1];
    if (array_size(ps[2]) != array_size(ps[3])
        || array_dims(ps[3])[0] != nx)
      return cerror(INCMP_ARG, ps[3]);
    if (array_type(ps[3]) > type)
      type = array_type(ps[3]);
  }

  // check for <n>
  if (narg == 7) {
    if (!symbolIsScalar(ps[4]))
      return luxerror("Need a scalar here", ps[4]);
    n = int_arg(ps[4]);
    if (n < 1)
      return cerror(ILL_ARG, ps[4]);
  } else if (narg == 6 && symbolIsScalar(ps[3])) {
    if (!symbolIsScalar(ps[3]))
      return luxerror("Need a scalar here", ps[3]);
    n = int_arg(ps[3]);
    if (n < 1)
      return cerror(ILL_ARG, ps[3]);
  } else
    n = 1;

  // check for <ox> and <oy>; prepare output variables
  if (narg >= 5) {              // have <ox> and <oy>
    // we prepare the output symbols
    i = 0;
    if (n > 1)
      dims[i++] = n;
    memcpy(dims + i, array_dims(ps[0]), array_num_dims(ps[0])*sizeof(int32_t));
    i += array_num_dims(ps[0]);
    to_scratch_array(ps[narg - 2], type, i, dims);
    ox.v = array_data(ps[narg - 2]);
    to_scratch_array(ps[narg - 1], type, i, dims);
    oy.v = array_data(ps[narg - 1]);
  } else {                      // use <gx> and <gy> for <ox> and <oy>
    int32_t lux_convert(int32_t, int32_t [], Symboltype, int32_t);
    lux_convert(2, ps, type, 0);
    ox.v = array_data(ps[0]);
    oy.v = array_data(ps[1]);
  }

  // now we do any promotion to the highest type and get pointers to
  // the data
  iq = lux_converts[type](1, ps);
  if (iq == LUX_ERROR)
    return LUX_ERROR;
  gx.v = array_data(iq);
  iq = lux_converts[type](1, ps + 1);
  if (iq == LUX_ERROR)
    return LUX_ERROR;
  gy.v = array_data(iq);
  if (narg == 3 || narg == 5
      || (narg == 6 && symbolIsScalar(ps[3]))) { // <v>
    iq = lux_converts[type](1, ps + 2);
    if (iq == LUX_ERROR)
      return LUX_ERROR;
    vx0.v = array_data(iq);
    vy0.ui8 = vx0.ui8 + lux_type_size[type];
    dv = 2;
  } else {
    iq = lux_converts[type](1, ps + 2);
    if (iq == LUX_ERROR)
      return LUX_ERROR;
    vx0.v = array_data(iq);
    iq = lux_converts[type](1, ps + 3);
    if (iq == LUX_ERROR)
      return LUX_ERROR;
    vy0.v = array_data(iq);
    dv = 1;
  }

  while (ngrid--) {                     // all grid points
    /* we work up to the desired distance by measuring the path
       lengths in each pixel and adding them up.  This scheme backfires
       when the velocities
       at some point circle the common corner of four pixels: then the
       algorithm can keep circling that point in very small steps for
       a very long time.  If many small steps follow one another in
       succession, then we are probably in such a situation and it is
       then unlikely that we'll get out of it any time soon.  We guard
       against this by maintaining an exponentially weighted average
       of the past steps and quitting if the weighted average gets too
       small.  We use an exponential decay scale of 2 steps and
       a limit value of 0.2. */
    for (i = 0; i < n; i++) {
      dslimit = 1.0;            // current weighted average of step sizes
      if (i) {
        x1 = x2;
        y1 = y2;
      } else {
        switch (type) {
          case LUX_INT8:
            ix = (int32_t) *gx.ui8;       // x pixel coordinate
            iy = (int32_t) *gy.ui8;       // y pixel coordinate
            x1 = (double) *gx.ui8++ - ix;
            y1 = (double) *gy.ui8++ - iy;
            break;
          case LUX_INT16:
            ix = (int32_t) *gx.i16;       // x pixel coordinate
            iy = (int32_t) *gy.i16;       // y pixel coordinate
            x1 = (double) *gx.i16++ - ix;
            y1 = (double) *gy.i16++ - iy;
            break;
          case LUX_INT32:
            ix = (int32_t) *gx.i32;       // x pixel coordinate
            iy = (int32_t) *gy.i32;       // y pixel coordinate
            x1 = (double) *gx.i32++ - ix;
            y1 = (double) *gy.i32++ - iy;
            break;
          case LUX_INT64:
            ix = (int32_t) *gx.i64;       // x pixel coordinate
            iy = (int32_t) *gy.i64;       // y pixel coordinate
            x1 = (double) *gx.i64++ - ix;
            y1 = (double) *gy.i64++ - iy;
            break;
          case LUX_FLOAT:
            ix = (int32_t) *gx.f;       // x pixel coordinate
            iy = (int32_t) *gy.f;       // y pixel coordinate
            x1 = (double) *gx.f++ - ix;
            y1 = (double) *gy.f++ - iy;
            break;
          case LUX_DOUBLE:
            ix = (int32_t) *gx.d;       // x pixel coordinate
            iy = (int32_t) *gy.d;       // y pixel coordinate
            x1 = (double) *gx.d++ - ix;
            y1 = (double) *gy.d++ - iy;
            break;
        }
        if (ix < 0 || ix > nx - 1 || iy < 0 || iy > ny - 1) { // out of range
          zerobytes(ox.ui8, lux_type_size[type]);
          zerobytes(oy.ui8, lux_type_size[type]);
          ox.ui8 += lux_type_size[type];
          oy.ui8 += lux_type_size[type];
          continue;
        }
      }

      index = ix + iy*nx;       // index relative to data start

      switch (type) {
        case LUX_INT8:
          vx = (double) vx0.ui8[index*dv]; // x velocity
          vy = (double) vy0.ui8[index*dv]; // y velocity
          break;
        case LUX_INT16:
          vx = (double) vx0.i16[index*dv]; // x velocity
          vy = (double) vy0.i16[index*dv]; // y velocity
          break;
        case LUX_INT32:
          vx = (double) vx0.i32[index*dv]; // x velocity
          vy = (double) vy0.i32[index*dv]; // y velocity
          break;
        case LUX_INT64:
          vx = (double) vx0.i64[index*dv]; // x velocity
          vy = (double) vy0.i64[index*dv]; // y velocity
          break;
        case LUX_FLOAT:
          vx = (double) vx0.f[index*dv]; // x velocity
          vy = (double) vy0.f[index*dv]; // y velocity
          break;
        case LUX_DOUBLE:
          vx = (double) vx0.d[index*dv]; // x velocity
          vy = (double) vy0.d[index*dv]; // y velocity
          break;
      }

      s0 = 0.5*hypot(vx, vy);   // length indicates desired distance
      s1 = s0;
      s = 0.0;

      while (s < s1) {          // go the distance
        c = traverseElement(x1, y1, vx, vy, &x2, &y2);
        // calculate distance inside the current pixel
        x1 -= x2;
        y1 -= y2;
        ds = hypot(x1, y1);     // distance in the current pixel
        if (s + ds > s1) {      // we went a bit too far
          x2 -= x1*(s0 - s - ds)/ds;
          y2 -= y1*(s0 - s - ds)/ds;
          c = CENTER;
        }
        dslimit = 0.5*(dslimit + ds);
        if (dslimit < 0.2) {    // we fear a semi-infinite loop here
          s = s1;               // we break it off
          continue;
        }
        switch (c) {
          case UP:
            if (iy == ny - 1) { // we're already at the top
              s += ds;
              s1 = s;
              continue;
            }
            di = nx;
            x1 = x2;
            y1 = 0.0;
            iy++;
            break;
          case RIGHT:
            if (ix == nx - 1) { // already at right edge
              s += ds;
              s1 = s;
              continue;
            }
            di = 1;
            x1 = 0.0;
            y1 = y2;
            ix++;
            break;
          case DOWN:
            if (iy == 0) {      // already at bottom
              s += ds;
              s1 = s;
              continue;
            }
            di = -nx;
            x1 = x2;
            y1 = 1.0;
            iy--;
            break;
          case LEFT:
            if (ix == 0) {      // already at left edge
              s += ds;
              s1 = s;
              continue;
            }
            di = -1;
            x1 = 1.0;
            y1 = y2;
            ix--;
            break;
          case CENTER:
            s1 = s;
            continue;
        } // end of switch (c)
        index += di;
        s += ds;
        switch (type) {
          case LUX_INT8:
            vx = (double) vx0.ui8[index*dv];
            vy = (double) vy0.ui8[index*dv];
            break;
          case LUX_INT16:
            vx = (double) vx0.i16[index*dv];
            vy = (double) vy0.i16[index*dv];
            break;
          case LUX_INT32:
            vx = (double) vx0.i32[index*dv];
            vy = (double) vy0.i32[index*dv];
            break;
          case LUX_INT64:
            vx = (double) vx0.i64[index*dv];
            vy = (double) vy0.i64[index*dv];
            break;
          case LUX_FLOAT:
            vx = (double) vx0.f[index*dv];
            vy = (double) vy0.f[index*dv];
            break;
          case LUX_DOUBLE:
            vx = (double) vx0.d[index*dv];
            vy = (double) vy0.d[index*dv];
            break;
        }
      } // end of while (s < s0)

      // if we get here then we are at the desired spot, or we have overshot
      // it a bit.  We adjust.

      switch (type) {
        case LUX_INT8:
          *ox.ui8++ = ix + x2;
          *oy.ui8++ = iy + y2;
          break;
        case LUX_INT16:
          *ox.i16++ = ix + x2;
          *oy.i16++ = iy + y2;
          break;
        case LUX_INT32:
          *ox.i32++ = ix + x2;
          *oy.i32++ = iy + y2;
          break;
        case LUX_INT64:
          *ox.i64++ = ix + x2;
          *oy.i64++ = iy + y2;
          break;
        case LUX_FLOAT:
          *ox.f++ = ix + x2;
          *oy.f++ = iy + y2;
          break;
        case LUX_DOUBLE:
          *ox.d++ = ix + x2;
          *oy.d++ = iy + y2;
          break;
      }
    } // end of for (i = 0; i < n; i++)
  } // end of while (ngrid--)
  return LUX_OK;
}
//--------------------------------------------------------------------
void legendre(double x, int32_t lmax, double *values)
// calculates the values of the associate Legendre polynomials
// P_l^m(x) at ordinate <x> for all <l> from 0 through <lmax> and all <m>
// from 0 through <l>
// we store the results in predefined <values>, in the following order:
// l 0 1 1 2 2 2 3 3 3 3
// m 0 0 1 0 1 2 0 1 2 3
// based on the following (recursion) relations:
// (l - m) P_l^m = x (2 l - 1) P_{l-1}^m - (l + m - 1) P_{l-2}^m
// P_m^m = (-1)^m (2 m - 1)!! (1 - x^2)^{m/2}
// (with n!! the product of all *odd* values between 1 and n)
// P_{m+1}^m = x (2 m + 1) P_m^m
{
  int32_t       l, m, j1, j2, j3, j4;
  double        z, *p, v1, v2;

  zerobytes(values, (lmax + 1)*(lmax + 2)*sizeof(double)/2);

  p = values;
  z = sqrt(1 - x*x);
  // first we calculate P_l^l for all <l>
  v1 = *p = 1.0;                // P_0^0
  p += 2;
  //    l 0 1 1 2 2 2 3 3 3 3 4 4 4 4 4
  //    m 0 0 1 0 1 2 0 1 2 3 0 1 2 3 4
  // done + +

  // now p points at P_1_1
  j1 = 1;
  for (l = 1; l <= lmax; l++) {
    v1 = *p = -v1*j1*z;
    j1 += 2;
    p += l + 2;
  }
  //    l 0 1 1 2 2 2 3 3 3 3 4 4 4 4 4
  //    m 0 0 1 0 1 2 0 1 2 3 0 1 2 3 4
  // done + + +     +       +         +

  // then we do the other combinations
  // the index of P_l^m is equal to l(l + 1)/2 + m
  for (m = 0; m < lmax; m++) {
    v2 = values[(m + 1)*(m + 2)/2 - 1]; // P_m^m
    v1 = values[(m + 2)*(m + 3)/2 - 2] = x*(2*m + 1)*v2; // P_{m+1}^m
    p = values + (m + 3)*(m + 4)/2 - 3;         // points at P_{m+2}^m
    j1 = 2*m + 3;
    j2 = j1 - 2;
    j3 = 2;
    j4 = m + 2;
    for (l = m + 2; l <= lmax; l++) {
      *p = (x*j1*v1 - j2*v2)/j3;
      v2 = v1;
      v1 = *p;
      j1 += 2;
      j2++;
      j3++;
      j4++;
      p += j4;
    }
  }
}
//--------------------------------------------------------------------
void spherical_harmonics(double x, int32_t lmax, double *values)
// calculates the values of the normalized associate Legendre polynomials
// y_l^m(x) at ordinate <x> for all <l> from 0 through <lmax> and all <m>
// from 0 through <l>.  The normalization is such that
// the normalized spherical harmonics are equal to
// Y_l^m(theta,phi) = y_l^m(cos(theta)) exp(i m phi).
// we store the results in predefined <values>, in the following order:
// l 0 1 1 2 2 2 3 3 3 3
// m 0 0 1 0 1 2 0 1 2 3
// NOTE: doesn't seem quite OK yet
{
  int32_t       l, m, j4;
  double        z, *p, v1, v2, w1, w2, w3, w4, w5, w6;

  p = values;
  z = sqrt(1 - x*x);
  // first we calculate y_l^l for all <l>
  v1 = *p = 0.25*M_2_SQRTPI;    // P_0^0
  p += 2;
  //    l 0 1 1 2 2 2 3 3 3 3 4 4 4 4 4
  //    m 0 0 1 0 1 2 0 1 2 3 0 1 2 3 4
  // done + +

  // now p points at y_1_1
  // y_{m+1}^{m+1} = -y_m^m z sqrt((2*m + 3)/(2*m + 1))
  w1 = 3;
  w2 = 2;
  for (l = 1; l <= lmax; l++) {
    v1 = *p = -v1*z*sqrt(w1/w2);
    w1 += 2;
    w2 += 2;
    p += l + 2;
  }
  //    l 0 1 1 2 2 2 3 3 3 3 4 4 4 4 4
  //    m 0 0 1 0 1 2 0 1 2 3 0 1 2 3 4
  // done + + +     +       +         +

  // then we do the other combinations
  // the index of y_l^m is equal to l(l + 1)/2 + m
  // y_{m+1}^m = y_m^m x sqrt(2m + 3)
  // y_l^m = x y_{l-1}^m sqrt((4l^2 - 1)/(l^2 - m^2))
  //         - y_{l-2}^m sqrt((2l + 1)/(2l - 3)*((l - 1)^2 - m^2)/(l^2 - m^2))
  for (m = 0; m < lmax; m++) {
    v2 = values[(m + 1)*(m + 2)/2 - 1]; // P_m^m
    v1 = values[(m + 2)*(m + 3)/2 - 2] = x*sqrt(2*m + 3)*v2; // P_{m+1}^m
    p = values + (m + 3)*(m + 4)/2 - 3;         // points at P_{m+2}^m
    j4 = m + 2;
    w2 = m*m;
    w3 = 2*(m + 2) + 1;
    w4 = 2*(m + 2) - 3;
    w5 = (m + 2) - 1;
    for (l = m + 2; l <= lmax; l++) {
      w1 = l*l;
      w6 = w1 - w2;
      *p = x*v1*sqrt((4*w1 - 1)/w6) - v2*sqrt(w3*(w5*w5 - w2)/(w4*w6));
      v2 = v1;
      v1 = *p;
      w3 += 2;
      w4 += 2;
      w5++;
      j4++;
      p += j4;
    }
  }
}
//--------------------------------------------------------------------
int32_t lux_legendre(int32_t narg, int32_t ps[])
// LEGENDRE(x, lmax)
{
  double        x, *values;
  int32_t       lmax, out, n;

  x = double_arg(ps[0]);
  if (x < -1 || x > 1)
    return luxerror("Illegal ordinate %g (not between -1 and +1)", ps[0], x);
  lmax = int_arg(ps[1]);
  if (lmax < 0)
    return luxerror("Illegal maximum order %d (must be nonnegative)", ps[1], lmax);
  n = ((lmax + 1)*(lmax + 2))/2;
  out = array_scratch(LUX_DOUBLE, 1, &n);
  values = (double*) array_data(out);
  if (internalMode & 1)
    spherical_harmonics(x, lmax, values);
  else
    legendre(x, lmax, values);
  return out;
}
//--------------------------------------------------------------------
int32_t lux_enhanceimage(int32_t narg, int32_t ps[])
/* ENHANCEIMAGE(<x> [, <part>, <tgt>, /SYMMETRIC]) enhances an image
  <x>.  The first dimension of <x> is assumed to select between color
  channels.  <x> is assumed to contain BYTE values between 0 and 255,
  inclusive.  <part> specifies which fraction of the full correction
  is to be applied; it defaults to 1.  <tgt> specifies at which grey
  level fraction the median of the enhanced image is expected to end
  up (if <part> is 1); it defaults to 100/256.  /SYMMETRIC means to
  apply enhancements from both the low and high ends; /NOSYMMETRIC
  means to enhance only from the low end.  LS 2006jun15 */
{
  Pointer src, tgt;
  int32_t ndim, *dims, nhist, *hist, nelem, i, result;
  float target, part;
  float a, b;
  float *m;

  if (!symbolIsNumericalArray(ps[0]))
    return cerror(NEED_NUM_ARR, ps[0]);
  if (symbol_type(ps[0]) != LUX_INT8)
    return luxerror("Need BYTE array", ps[0]);
  numerical(ps[0], &dims, &ndim, &nelem, &src);
  if (ndim < 2)
    return luxerror("Need 2 or more dimensions", ps[0]);
  part = (narg > 1 && ps[1])? float_arg(ps[1]): 1;
  target = (narg > 2 && ps[2])? float_arg(ps[2]): 100.0/256;

  nhist = 256*dims[0];
  hist = (int32_t*) calloc(nhist, sizeof(*hist));
  m = (float*) malloc(nhist*sizeof(*m));
  if (!hist || !m) {
    free(hist);
    return cerror(ALLOC_ERR, 0);
  }
  result = array_clone(ps[0], LUX_INT8);
  if (result == LUX_ERROR) {
    free(hist);
    free(m);
    return result;
  }
  tgt.ui8 = (uint8_t*) array_data(result);
  for (i = 0; i < nelem; i += dims[0]) {
    int32_t j, x = 0;

    for (j = 0; j < dims[0]; j++) // over all color channels
      x += *src.ui8++;
    hist[x]++;
  }
  src.ui8 = (uint8_t*) array_data(ps[0]);
  for (i = 1; i < nhist; i++)   // calculate running sum
    hist[i] += hist[i - 1];
  a = 2 - 4*target;
  b = 1 - a;
  m[0] = 1.0;
  for (i = 1; i < nhist; i++) {         // calculate adjustment factors
    float q;

    q = (float) hist[i]*dims[0]/nelem;
    m[i] = q*(a*q + b)/i*nhist;
    if (m[i] < 1 && !(internalMode & 1))
      m[i] = 1;
    if (part != 1)
      m[i] = part*m[i] + 1 - part;
  }
  for (i = 0; i < nelem; i += dims[0]) { // calculate adjusted image
    int32_t j, x = 0;

    for (j = 0; j < dims[0]; j++)
      x += src.ui8[j];
    for (j = 0; j < dims[0]; j++) {
      int32_t y;

      y = *src.ui8++ * m[x];
      if (y > 255)
        y = 255;
      *tgt.ui8++ = y;
    }
  }
  free(m);
  free(hist);
  return result;
}
//--------------------------------------------------------------------
int32_t lux_hamming(int32_t narg, int32_t ps[]) {
  int32_t nelem, nelem2, ndim, *dims, result, i, type, nr2isarray;
  Pointer src, src2, tgt;

  if (!symbolIsNumerical(ps[0]))
    return luxerror("Need a numerical argument", ps[0]);
  if (!symbolIsInteger(ps[0]))
    return cerror(NEED_INT_ARG, ps[0]);
  numerical(ps[0], &dims, &ndim, &nelem, &src);
  type = symbol_type(ps[0]);

  if (narg >= 2) {
    if (!symbolIsNumerical(ps[1]))
      return luxerror("Need a numerical argument", ps[1]);
    if (!symbolIsInteger(ps[1]))
      return cerror(NEED_INT_ARG, ps[1]);
    if (symbol_type(ps[1]) != type)
      return luxerror("Data type is different from previous argument", ps[1]);
    numerical(ps[1], NULL, NULL, &nelem2, &src2);
    if (nelem2 != nelem && nelem2 != 1)
      return cerror(INCMP_ARG, ps[1]);
    nr2isarray = (nelem2 > 1);
  }

  if (symbolIsScalar(ps[0])) {
    result = scalar_scratch(LUX_INT32);
    tgt.i32 = &scalar_value(result).i32;
  } else {
    result = array_scratch(LUX_INT32, ndim, dims);
    tgt.i32 = (int32_t*) array_data(result);
  }

  if (narg == 1) {
    for (i = 0; i < nelem; i++) {
      uint32_t dist = 0, val;
      switch (type) {
      case LUX_INT8:
        val = *src.ui8++;
        while (val) {
          ++dist;
          val &= val - 1;
        }
        *tgt.i32++ = dist;
        break;
      case LUX_INT16:
        val = *src.i16++;
        while (val) {
          ++dist;
          val &= val - 1;
        }
        *tgt.i32++ = dist;
        break;
      case LUX_INT32:
        val = *src.i32++;
        while (val) {
          ++dist;
          val &= val - 1;
        }
        *tgt.i32++ = dist;
        break;
      case LUX_INT64:
        val = *src.i64++;
        while (val) {
          ++dist;
          val &= val - 1;
        }
        *tgt.i32++ = dist;
        break;
      }
    }
  } else {
    for (i = 0; i < nelem; i++) {
      uint32_t dist = 0, val;
      switch (type) {
      case LUX_INT8:
        val = *src.ui8++ ^ *src2.ui8;
        if (nr2isarray)
          src2.ui8++;
        while (val) {
          ++dist;
          val &= val - 1;
        }
        *tgt.i32++ = dist;
        break;
      case LUX_INT16:
        val = *src.i16++ ^ *src2.i16;
        if (nr2isarray)
          src2.i16++;
        while (val) {
          ++dist;
          val &= val - 1;
        }
        *tgt.i32++ = dist;
        break;
      case LUX_INT32:
        val = *src.i32++ ^ *src2.i32;
        if (nr2isarray)
          src2.i32++;
        while (val) {
          ++dist;
          val &= val - 1;
        }
        *tgt.i32++ = dist;
        break;
      case LUX_INT64:
        val = *src.i64++ ^ *src2.i64;
        if (nr2isarray)
          src2.i64++;
        while (val) {
          ++dist;
          val &= val - 1;
        }
        *tgt.i32++ = dist;
        break;
      }
    }
  }
  return result;
}
//--------------------------------------------------------------------
double vhypot(int32_t n, double arg1, double arg2, ...)
{
  double arg = hypot(arg1, arg2);
  if (n < 3)
    return arg;
  va_list ap;
  va_start(ap, arg2);
  n -= 2;
  while (n--)
    arg = hypot(arg, va_arg(ap, double));
  va_end(ap);
  return arg;
}
//--------------------------------------------------------------------
double hypota(int32_t n, double *x)
{
  if (n < 1)
    return 0.0;
  double arg = *x++;
  while (--n)
    arg = hypot(arg, *x++);
  return arg;
}
//--------------------------------------------------------------------
int32_t compare_doubles(const void *a, const void *b)
{
  const double *da = (const double *) a;
  const double *db = (const double *) b;
  return (*da > *db) - (*da < *db);
}
int32_t runord_d(double *data, int32_t n, int32_t width, int32_t ord, double *result)
{
  int32_t i;

  if (n < 1 || width < 1) {
    errno = EDOM;
    return -1;
  }
  double *temp = (double*) malloc(width*sizeof(double));
  if (!temp) {
    errno = ENOMEM;
    return -1;
  }
  if (width > n)
    width = n;
  if (ord < 0)
    ord = 0;
  if (ord > width - 1)
    ord = width - 1;
  int32_t o = width/2;
  for (i = 0; i < n - width; i++) {
    memcpy(temp, data + i, width*sizeof(double));
    qsort(temp, width, sizeof(double), compare_doubles);
    result[o + i] = temp[ord];
  }
  for ( ; i < n - o; i++)
    result[i] = result[i - 1];
  for (i = 0; i < o; i++)
    result[i] = result[o];
  free(temp);
  return 0;
}
BIND(runord_d, i_dpiT3dp_iaiirq_00T3, f, runord, 3, 3, NULL);
//--------------------------------------------------------------------
int32_t runmax_d(double *data, int32_t n, int32_t width, double *result)
{
  return runord_d(data, n, width, width - 1, result);
}
BIND(runmax_d, i_dpiidp_iairq_00T2, f, RUNMAX, 2, 2, NULL);
//--------------------------------------------------------------------
int32_t runmin_d(double *data, int32_t n, int32_t width, double *result)
{
  return runord_d(data, n, width, 0, result);
}
BIND(runmin_d, i_dpiidp_iairq_00T2, f, RUNMIN, 2, 2, NULL);
//--------------------------------------------------------------------
/*
  Returns <x> such that <x> = <cur> (mod <period) and
  <average> - <period>/2 <= <x> - <prev> < <average> + <period>/2
 */
double unmod(double cur, double prev, double period, double average)
{
  if (!period)
    return cur;
  return cur + period*ceil((prev - cur + average)/period - 0.5);
}
//--------------------------------------------------------------------
int32_t unmod_slice_d(double *srcptr, size_t srccount, size_t srcstride,
                  double period, double average,
                  double *tgtptr, size_t tgtcount, size_t tgtstride)
{
  int32_t i;

  if (!period || !srcptr || srccount < 1 || !tgtptr) {
    errno = EDOM;
    return 1;
  }
  *tgtptr = *srcptr;
  tgtptr += tgtstride;
  srcptr += srcstride;
  for (i = 1; i < srccount; i++) {
    *tgtptr = unmod(*srcptr, tgtptr[-tgtstride], period, average);
    tgtptr += tgtstride;
    srcptr += srcstride;
  }
  return 0;
}
BIND(unmod_slice_d, i_sdddsd_iaiiirq_000T333, f, unmod, 2, 4, ":axis:period:average");
//--------------------------------------------------------------------
double hypot_stride(double *data, size_t count, size_t stride)
{
  double result = 0.0;
  while (count-- > 0) {
    result = hypot(result, *data);
    data += stride;
  }
  return result;
}
BIND(hypot_stride, d_sd_iaiarxq_000_2, f, hypot, 1, 2, ":axis");
//--------------------------------------------------------------------
int32_t approximately_equal(double a, double b, double eps)
{
  return fabs(a - b) <= (fabs(a) < fabs(b)? fabs(b): fabs(a))*DBL_EPSILON*eps;
}
//--------------------------------------------------------------------
int32_t approximately_equal_z(doubleComplex a, doubleComplex b, double eps)
{
  double md = hypot(a.real - b.real, a.imaginary - b.imaginary);
  double ma = hypot(a.real, a.imaginary);
  double mb = hypot(b.real, b.imaginary);
  return md <= (ma < mb? mb: ma)*DBL_EPSILON*eps;
}
//--------------------------------------------------------------------
int32_t essentially_equal(double a, double b, double eps)
{
  return fabs(a - b) <= (fabs(a) > fabs(b)? fabs(b): fabs(a))*DBL_EPSILON*eps;
}
//--------------------------------------------------------------------
int32_t essentially_equal_z(doubleComplex a, doubleComplex b, double eps)
{
  double md = hypot(a.real - b.real, a.imaginary - b.imaginary);
  double ma = hypot(a.real, a.imaginary);
  double mb = hypot(b.real, b.imaginary);
  return md <= (ma > mb? mb: ma)*DBL_EPSILON*eps;
}
//--------------------------------------------------------------------
int32_t approximately_equal_f(float a, float b, float eps)
{
  return fabs(a - b) <= (fabs(a) < fabs(b)? fabs(b): fabs(a))*FLT_EPSILON*eps;
}
//--------------------------------------------------------------------
int32_t approximately_equal_z_f(floatComplex a, floatComplex b, float eps)
{
  float md = hypot(a.real - b.real, a.imaginary - b.imaginary);
  float ma = hypot(a.real, a.imaginary);
  float mb = hypot(b.real, b.imaginary);
  return md <= (ma < mb? mb: ma)*FLT_EPSILON*eps;
}
//--------------------------------------------------------------------
int32_t essentially_equal_f(float a, float b, float eps)
{
  return fabs(a - b) <= (fabs(a) > fabs(b)? fabs(b): fabs(a))*FLT_EPSILON*eps;
}
//--------------------------------------------------------------------
int32_t essentially_equal_z_f(floatComplex a, floatComplex b, float eps)
{
  float md = hypot(a.real - b.real, a.imaginary - b.imaginary);
  float ma = hypot(a.real, a.imaginary);
  float mb = hypot(b.real, b.imaginary);
  return md <= (ma > mb? mb: ma)*FLT_EPSILON*eps;
}
//--------------------------------------------------------------------
// SSFCTOPOLAR(<x> [, <level>]) interprets each element of <x> as an
// SSFC (Sierpiński Surface-Filling Coordinate) and returns the
// corresponding latitude, longitude, and precision for it, all
// measured in radians.  The precision is a measure for the angular
// size of the area indicated by the SSFC at that level.
//
// If <level> has fewer elements than <x> has, then the last element
// of <level> is used for the remaining elements of <x>.
//
// If <x> is of an integer type, then its values are interpreted as
// SSFC indices at the indicated <level>, and are expected to be
// between 0 and 2^<level> - 1, inclusive.  If no <level> is
// specified, then it is taken to be the least number that
// accommodates the greatest <x>, such that MAX(<x>) LT 2^<level>.
//
// If <x> is of a floating-point type, then its values are taken MOD
// 1, and are interpreted as decimal SSFCs.  If <level> is not
// specified, then it is taken equal to the number of bits in <x>,
// minus 1.
int32_t lux_ssfc_to_polar(int32_t narg, int32_t ps[]) {
  if (!symbolIsNumerical(ps[0]))
    return luxerror("Need a numerical argument", ps[0]);
  if (narg > 1 && !symbolIsInteger(ps[1]))
    return cerror(NEED_INT_ARG, ps[1]);

  Pointer* ptrs;
  LoopInfo* infos;
  int32_t iq;
  StandardArguments sa;
  if (symbolIsInteger(ps[0])) {
    if ((iq = sa.set(narg, ps, "iL*;iL*?;rD+3&", &ptrs, &infos)) < 0)
      return LUX_ERROR;
  } else {
    if ((iq = sa.set(narg, ps, "iD*;iL*?;rD+3&", &ptrs, &infos)) < 0)
      return LUX_ERROR;
  }

  int32_t default_level = 8*lux_type_size[symbol_type(ps[0])] - 1;

  size_t ssfc_nelem = infos[0].nelem;
  size_t level_nelem = infos[1].nelem;
  if (!level_nelem) {
    switch (infos[0].type) {
    case LUX_INT32:
      {
        int32_t max = std::accumulate(ptrs[0].i32, ptrs[0].i32 + infos[0].nelem,
                                      (int32_t) 0,
                                      [](int32_t a, int32_t b) {
                                        return std::max(a, b);
                                      });
        default_level = ceil(log2(max));
      }
      break;
    case LUX_INT64:
      {
        int64_t max = std::accumulate(ptrs[0].i64, ptrs[0].i64 + infos[0].nelem,
                                      (int64_t) 0,
                                      [](int64_t a, int64_t b) {
                                        return std::max(a, b);
                                      });
        default_level = ceil(log2(max));
      }
      break;
    case LUX_DOUBLE:
      // accept default level
      break;
    }
    ptrs[1].i32 = &default_level;
    level_nelem = 1;
  }

  while (ssfc_nelem--) {
    SSFC ssfc;
    switch (infos[0].type) {
    case LUX_INT32:
      ssfc.set_ssfc(*ptrs[0].i32++, *ptrs[1].i32);
      break;
    case LUX_INT64:
      ssfc.set_ssfc(*ptrs[0].i64++, *ptrs[1].i32);
      break;
    case LUX_DOUBLE:
      ssfc.set_ssfc(*ptrs[0].d++, *ptrs[1].i32);
      break;
    }
    *ptrs[2].d++ = ssfc.get_latitude_rad();
    *ptrs[2].d++ = ssfc.get_longitude_rad();
    *ptrs[2].d++ = ssfc.get_precision_rad();
    if (level_nelem > 1) {
      --level_nelem;
      ++ptrs[1].i32;
    }
  }

  return iq;
}
REGISTER(ssfc_to_polar, f, ssfctopolar, 1, 2, 0);
//--------------------------------------------------------------------
// POLARTOSSFC(<coords>, [<level>]) converts polar coordinates into
// SSFC (Sierpiński Surface-Filling Coordinate).  <coords> is
// expected to have at least 2 elements in its first dimension, which
// are interpreted as the latitude and longitude, measured in
// radians.
//
// If <level> is specified, then the returned value is the SSFC index
// at the indicated <level>, and is a nonnegative integer less than
// 2^<level>.  Otherwise, the returned value is the decimal SSFC
// coordinate, as a nonnegative double-precision value less than 1.
// If <level> has fewer elements than <coords> has, then the last
// element of <level> is used for the remaining <coords>.
int32_t lux_polar_to_ssfc(int32_t narg, int32_t ps[]) {
  if (!symbolIsNumerical(ps[0]))
    return luxerror("Need a numerical argument", ps[0]);
  if (narg > 1 && !symbolIsInteger(ps[1]))
    return cerror(NEED_INT_ARG, ps[1]);

  Pointer* ptrs;
  LoopInfo* infos;
  int32_t iq;
  int32_t level;
  StandardArguments sa;
  if (narg > 1) {
    if ((iq = sa.set(narg, ps, "iD>2,*;iL;rL-,&", &ptrs, &infos)) < 0)
      return LUX_ERROR;
    level = *ptrs[1].i32;
    if (level < 3)
      level = 3;
    else if (level > 64)
      level = 64;
    if (level > 31) {
      iq = lux_int64(1, &iq);
      numerical(iq, NULL, NULL, NULL, &ptrs[2]);
      infos[2].type = LUX_INT64;
    }
  } else {
    if ((iq = sa.set(narg, ps, "iD>2,*;rD-&", &ptrs, &infos)) < 0)
      return LUX_ERROR;
    level = floor(log(std::numeric_limits<double>::epsilon())/log(2));
  }

  size_t points_count = infos[0].nelem/infos[0].dims[0];
  if (narg > 1) {               // return SSFC index at indicated level
    while (points_count--) {
      SSFC ssfc(ptrs[0].d[0], ptrs[0].d[1], level);
      if (level > 31) {
        *ptrs[2].i64++ = static_cast<int64_t>(ssfc.get_bits());
      } else {
        *ptrs[2].i32++ = static_cast<int32_t>(ssfc.get_bits());
      }
      ptrs[0].d += infos[0].dims[0];
    }
  } else {                      // return decimal SSFC
    while (points_count--) {
      SSFC ssfc(ptrs[0].d[0], ptrs[0].d[1], level);
      *ptrs[1].d++ = static_cast<double>(ssfc.get_bits())/pow(2,level);
      ptrs[0].d += infos[0].dims[0];
    }
  }

  return iq;
}
REGISTER(polar_to_ssfc, f, polartossfc, 1, 2, 0);

/// A description of a span of (consecutive) elements.
struct Span {
  int32_t location;     //!< the location of the beginning of the span.
  int32_t size;         //!< the count of elements in the span.
};

/// Finds the location and length of consecutive spans of elements for
/// which a condition is non-zero.
///
/// \param[in] first is an iterator pointing at the first condition
/// value.
///
/// \param[in] last is an iterator pointing at one past the last
/// condition value.
///
/// \param[in] cycle says whether or not to treat the condition values
/// as cyclical.  If yes, then the condition values repeat themselves.
///
/// \returns a vector describing the found spans.
///
/// If all condition values are non-zero, then returns a single span
/// beginning at 0.  If \a cycle is true, the first and the last
/// condition values are non-zero, and at least one condition value is
/// zero, then the first element is included in the last reported
/// span.  For example, if the condition values are
///
///     1 1 0 1 1 0 0 1
///
/// and if \a cycle is false, then reports a span of length 2
/// beginning at 0, a span of length 2 beginning at 3, and a span of
/// length 1 beginning at 7.  If \a cycle is true, then reports a span
/// of length 2 beginning at 3 and a span of length 3 beginning at 7.
/// That last span includes the elements at positions 7, 0, and 1.
template<class Iterator,
         class T = typename std::iterator_traits<Iterator>::value_type>
void
findspans(const Iterator first, const Iterator last, bool cycle,
          std::vector<Span>& spans)
{
  spans.clear();
  if (last != first) {
    int inspan = 0;
    int location;
    int index = 0;
    for (auto it = first; it != last; ++it) {
      if (*it++ != T()) {
        if (!inspan)
          location = index;
        ++inspan;
      } else if (inspan) {
        spans.push_back({location, inspan});
        inspan = 0;
      }
      ++index;
    }
    if (inspan > 0) {
      if (cycle) {
        if (inspan < index) {   // not all elements meet the condition
          for (auto it = first; *it != T(); ++it)
            ++inspan;
          if (spans.size() > 0
              && spans[0].location == 0)
            // Already had a span with index 0 in it.  Will be
            // replaced by the current span, so remove the earlier
            // one.
            spans.erase(spans.begin());
        }
      }
      spans.push_back({location, inspan});
    }
  }
}

// findspans(<condition>[, /cycle])
//
// Returns the location and length of spans of consecutive elements
// of <condition> for which the condition is non-zero.  If /cycle is
// specified, then the data is assumed to repeat itself, and a span
// may consist of some elements at the end of the data plus some
// elements at the beginning of the data.
//
// If any spans are found, then returns a 2-dimensional array with 2
// elements in its 2nd dimension and one element in its 1st dimension
// for each found span.  If we call the returned array <r>, then
// <r(*,0)> holds the index of the first member of each span, and
// <r(*,1)> holds the length of that span.
//
// If no spans are found, then return scalar -1.
//
// LS 2020-04-29
int32_t lux_findspans(int32_t narg, int32_t ps[]) {
  int32_t nelem;
  Pointer src;
  if (numerical(ps[0], NULL, NULL, &nelem, &src) < 0)
    return LUX_ERROR;

  std::vector<Span> spans;
  bool cycle = (internalMode & 1) != 0;
  switch (symbol_type(ps[0])) {
  case LUX_INT8:
    findspans(src.ui8, src.ui8 + nelem, cycle, spans);
    break;
  case LUX_INT16:
    findspans(src.i16, src.i16 + nelem, cycle, spans);
    break;
  case LUX_INT32:
    findspans(src.i32, src.i32 + nelem, cycle, spans);
    break;
  case LUX_INT64:
    findspans(src.i64, src.i64 + nelem, cycle, spans);
    break;
  case LUX_FLOAT:
    findspans(src.f, src.f + nelem, cycle, spans);
    break;
  case LUX_DOUBLE:
    findspans(src.d, src.d + nelem, cycle, spans);
    break;
  default:
    return cerror(ILL_TYPE, ps[0]);
  }

  int32_t result;
  if (spans.size() == 0)
    result = LUX_MINUS_ONE;
  else {
    int dims[2];
    dims[0] = spans.size();
    dims[1] = 2;
    result = array_scratch(LUX_INT32, 2, dims);
    int32_t* tgt = static_cast<int32_t*>(array_data(result));
    for (auto it = spans.cbegin(); it != spans.cend(); ++it)
      *tgt++ = it->location;
    for (auto it = spans.cbegin(); it != spans.cend(); ++it)
      *tgt++ = it->size;
  }
  return result;
}
REGISTER(findspans, f, findspans, 1, 1, "1cycle");

template<class T>
int32_t
convert_vector_to_lux_array(const std::vector<T>& data, int32_t lux_symbol)
{
  // convert <data> to a one-dimensional LUX array
  int32_t size = static_cast<int32_t>(data.size());
  Symboltype t = lux_symboltype_for_type<T>;
  auto result = to_scratch_array(lux_symbol, t, 1, &size);
  if (result < 0)
    return result;
  Pointer tgt;
  tgt.v = array_data(lux_symbol);
  switch (t) {
  case LUX_INT8:
    for (auto it = data.begin(); it != data.end(); ++it)
      *tgt.ui8++ = *it;
    break;
  case LUX_INT16:
    for (auto it = data.begin(); it != data.end(); ++it)
      *tgt.i16++ = *it;
    break;
  case LUX_INT32:
    for (auto it = data.begin(); it != data.end(); ++it)
      *tgt.i32++ = *it;
    break;
  case LUX_INT64:
    for (auto it = data.begin(); it != data.end(); ++it)
      *tgt.i64++ = *it;
    break;
  case LUX_FLOAT:
    for (auto it = data.begin(); it != data.end(); ++it)
      *tgt.f++ = *it;
    break;
  case LUX_DOUBLE:
    for (auto it = data.begin(); it != data.end(); ++it)
      *tgt.d++ = *it;
    break;
  default:
    return cerror(ILL_TYPE, lux_symbol);
  }
  return result;
}

template<typename Integer,
         std::enable_if_t<std::is_integral<Integer>::value, bool> = true>
Integer
nn_mod(Integer a, Integer b)
{
  Integer result = a%b;
  if (result < 0)
    result += std::abs(b);
  return result;
}

template<typename Float,
         std::enable_if_t<std::is_floating_point<Float>::value, bool> = true>
Float
nn_mod(Float a, Float b)
{
  Float result = fmod(a, b);
  if (result < 0)
    result += std::abs(b);
  return result;
}

/// Returns a nonnegative common divisor if not less than a specified
/// minimum value and (for floating-point data types) within a
/// roundoff error tolerance.
///
/// \param[in] a is one of the two values to process.
///
/// \param[in] b is the other of the two values to process.
///
/// \param[in] min_value is the lower threshold for the intermediate
/// value in searching for a common divisor.  It defaults to 0.
///
/// \param[in] tolerance specifies the nonnegative tolerance for
/// accepting a match.  It defaults to
/// 3*std::numeric_limits<T>::epsilon, which is 0 for integer data
/// types.
///
/// \returns the detected common divisor, or 0 if none could be found
/// that meets the specified conditions.
///
/// The common divisor is sought using a modified version of the
/// Euclidean algorithm.  The modifications are:
///
/// - If it is detected that the common divisor cannot be greater than
///   or equal to \a min_value, then returns 0.
///
/// - If the ratio between the mismatch and the candidate common
///   divisor is not greater in magnitude than the tolerance, then
///   that common divisor is returned.
///
/// For floating-point data types, the Euclidean algorithm isn't
/// guaranteed to terminate, so then having a non-zero \a min_value
/// and a \a tolerance comfortably greater than
/// std::numeric_limits<T>::epsilon is a good idea.
template<class T>
T
truncated_gcd(T a, T b, T min_value = 0,
              T tolerance = 3*std::numeric_limits<T>::epsilon)
{
  if (a == 0 || b == 0)
    return 0;
  if (a < 0)
    a = -a;
  if (b < 0)
    b = -b;
  if (a < b)
    std::swap(a, b);
  // now a >= b > 0
  T r;
  while (1) {
    r = nn_mod(a, b);
    if (r <= tolerance*b)       // match
      return b;
    if (r < min_value)          // no match possible
      return 0;
    a = b;
    b = r;
  }
}

/// A comparison function object to allow indirect sorting of a constant vector
/// by rearranging a vector of indexes into the constant vector.
///
///     std::vector<T> v = { 3, 7, 5 };     // vector to sort indirectly
///     std::vector<int> ix = { 0, 1, 2 };  // vector of indexes into <v>
///     Index_less ixless(v);
///     std::sort(ix.begin(), ix.end(), ixless);
///     // now v[ix[i]] for i = 0 .. 2 produces 3, 5, 7
template<class T>
struct Index_less {

  /// Constructor.
  ///
  /// \param[in] v is the vector to sort indirectly.  It is not modified.
  explicit Index_less(const std::vector<T>& v) : m_v(v) { }

  /// Is the element at index \p a less than the element at index \p b?
  ///
  /// \param[in] a is the index of the left-hand element.
  ///
  /// \param[in] b is the index of the right-hand element.
  ///
  /// \returns `true` if \a is less than \b, otherwise `false`.
  bool operator()(int a, int b) const { return m_v[a] < m_v[b]; }

  const std::vector<T>& m_v;    //!< The vector of elements to sort indirectly.
};

/// Categorizes values by their lowest common factors.
///
/// \param[in] first is an iterator pointing at the first value.
///
/// \param[in] last is an iterator pointing at one past the last
/// value.
///
/// \param[in] tolerance is the maximum tolerance for accepting
/// near-common factors.  This parameter is used only for
/// floating-point values.
///
/// \param[out] category receives the category number for each value.
/// Its previous contents are lost.
///
/// \param[out] factor receives the common factors.  Its previous
/// contents are lost.
///
/// The detection of common factors uses a modification of the
/// Euclidean algorithm, see truncated_gcd().
///
/// If we regard the input values between \a first (inclusive) and \a
/// last (exclusive) as members of an array `x`, then the lowest
/// common factor for `x[i]` is `factors[category[i]]`.  For
/// example, if the integer values are
///
///     8 5 6 3
///
/// then \a category ends up as
///
///     0 1 0 2
///
/// and \a dfactors as
///
///     2 5 3
///
/// so the `factors[category[i]]` are
///
///     2 5 2 3
template<class Iterator,
         class T = typename std::iterator_traits<Iterator>::value_type>
void
find_common_factor_category(const Iterator first, const Iterator last,
                            T tolerance, std::vector<int32_t>& categories,
                            std::vector<T>& factors)
{
  categories.clear();
  factors.clear();

  // We compare each value with the factors seen so far.  For the
  // first value there aren't any such factors yet, so accept it
  // outright.
  auto it = first;
  factors.push_back(*it);
  categories.push_back(0);   // element 0 is associated with factor[0]
  ++it;

  // We iterate over the remaining values.  For each one, we compare it with all
  // factors seen so far, from small to large.  Vector <categories> relies on
  // the order of elements in vector <factors>, so we must not sort vector
  // <factors> itself.  Instead, we maintain a vector of indexes into <factors>
  // that produce the factors from small to large.
  std::vector<int> factor_indexes;
  factor_indexes.push_back(0);

  for ( ; it != last; ++it) {
    bool found_one = false;
    for (int i = 0; i < factor_indexes.size(); ++i) {
      auto fi = factor_indexes[i];
      auto f = factors[fi];

      // truncated_gcd() stops (returns 0) when common factor cannot
      // be greater than 3rd argument
      T g = truncated_gcd(*it, f, factors[factor_indexes[0]], tolerance);
      if (g == f) { // multiple of existing factor
        categories.push_back(fi);
        found_one = true;
        break;                  // done
      } else if (g != 0) {
        // The common factor differs from <f> so must be less than <f>.  We try
        // known factors in ascending order, so the detected common factor must
        // not be in the list yet.  We replace <f> with <g>.
        factors[fi] = g;
        // keep <factor_indexes> sorted
        Index_less<T> fil(factors);
        std::sort(factor_indexes.begin(), factor_indexes.end(), fil);
        categories.push_back(fi);
        found_one = true;
        break;
      }
    }
    if (!found_one) {
      // none of the current factors match, so the current value is an
      // entirely new factor
      auto fi = factors.size();
      factors.push_back(*it);
      factor_indexes.push_back(fi);
      // keep factor_index sorted
      std::sort(factor_indexes.begin(), factor_indexes.end(),
                Index_less<T>(factors));
      categories.push_back(fi);
    }
  }
}

// commonfactors, <x>, <factors>, <categories> [, <tol>]
//
// Assigns the values of <x> to categories depending on factors that
// they have in common.  <tol> is the tolerance for detecting the
// common factors (for floating-point <x>).  The found common factors
// are returned in <factors> if specified.  The category that each
// value of <x> belongs to is returned in <categories> if specified.
// <factors>(<categories)> has the same dimensions as <x> and provides
// the common factor for each element of <x>.
int32_t lux_commonfactors(int32_t narg, int32_t ps[]) {

  // the input values
  int32_t nelem;
  Pointer src;
  if (numerical(ps[0], NULL, NULL, &nelem, &src) < 0)
    return LUX_ERROR;

  double tol;
  if (narg >= 3 && ps[3]) {     // <tol>
    tol = double_arg(ps[3]);
  } else {
    tol = 0.01;
  }

  std::vector<int32_t> categories;
  switch (symbol_type(ps[0])) {
  case LUX_INT8:
    {
      std::vector<uint8_t> factors;
      find_common_factor_category(src.ui8, src.ui8 + nelem, uint8_t(0),
                                  categories, factors);
      convert_vector_to_lux_array(factors, ps[1]);
    }
    break;
  case LUX_INT16:
    {
      std::vector<int16_t> factors;
      find_common_factor_category(src.i16, src.i16 + nelem, int16_t(0),
                                  categories, factors);
      convert_vector_to_lux_array(factors, ps[1]);
    }
    break;
  case LUX_INT32:
    {
      std::vector<int32_t> factors;
      find_common_factor_category(src.i32, src.i32 + nelem, int32_t(0),
                                  categories, factors);
      convert_vector_to_lux_array(factors, ps[1]);
    }
    break;
  case LUX_INT64:
    {
      std::vector<int64_t> factors;
      find_common_factor_category(src.i64, src.i64 + nelem, int64_t(0),
                                  categories, factors);
      convert_vector_to_lux_array(factors, ps[1]);
    }
    break;
  case LUX_FLOAT:
    {
      std::vector<float> factors;
      find_common_factor_category(src.f, src.f + nelem, float(tol),
                                  categories, factors);
      convert_vector_to_lux_array(factors, ps[1]);
    }
    break;
  case LUX_DOUBLE:
    {
      std::vector<double> factors;
      find_common_factor_category(src.d, src.d + nelem, tol, categories,
                                  factors);
      convert_vector_to_lux_array(factors, ps[1]);
    }
    break;
  default:
    return cerror(ILL_TYPE, ps[0]);
  }
  convert_vector_to_lux_array(categories, ps[2]);
  return LUX_OK;
}
REGISTER(commonfactors, s, commonfactors, 3, 4, NULL);

#if HAVE_LIBGSL
struct PermutationChangeInterval
{
  double tmin;
  double tmax;
  size_t change_count;
  int object1;
  int object2;
};

/// Returns the absolute difference between two values of the same type, even if
/// that type is unsigned.
///
/// \tparam T is the data type.  It must support `operator-()` and
/// `operator<()`.
///
/// \param x1 is one value to compare.
///
/// \param x2 is the other value to compare.
///
/// \returns the absolute difference.
template<typename T>
T abs_diff(T x1, T x2)
{
  return x1 < x2? x2 - x1: x1 - x2;
}

struct PermutationChange {
  double t;
  int object1;
  int object2;
  size_t final_permutation;
};

/// Locate values of the independent coordinate (here denoted `t`) at which the
/// circular rank permutation of objects changes.
///
/// The used algorithm may fail to locate some relevant values of `t` if some of
/// the objects pass each other more than once during the searched interval.
///
/// \param f is a function object or a pointer to a function of `t` that
/// produces the positions ("angles") that define the permutation, one position
/// for each object.
///
/// \param period is the maximum value at which the positions wrap around.  The
/// positions must be between 0 (inclusive) and \a period (exclusive).
///
/// \param tmin is the beginning of the interval of `t` to search.
///
/// \param tmax is the end of the interval of `t` to search.
///
/// \returns a vector of PermutationChange that reports the values of `t` at
/// which the permutation changes, and the 0-based indexes (in ascending order)
/// of the two objects that switch places at that time.
template<typename FuncType>
std::vector<PermutationChange>
find_permutation_changes(FuncType f, double period, double tmin, double tmax)
{
  std::vector<PermutationChange> ts;

  std::vector<double> vmin = f(tmin);

  if (vmin.size() < 2)          // cannot detect changes
    return ts;

  std::vector<double> vmax = f(tmax);
  size_t dtot = permutation_distance_circular_rank(vmin, vmax);
  if (!dtot) {
    return ts;                  // no permutation changes
  }
  if (dtot > 0) {
    // locate intervals with a single permutation change

    std::list<PermutationChangeInterval> intervals;
    PermutationChangeInterval pci;
    pci.tmin         = tmin;
    pci.tmax         = tmax;
    pci.change_count = dtot;
    intervals.push_back(pci);

    while (1) {
      bool bad = false;
      for (auto it = intervals.begin(); it != intervals.end(); ) {
        switch (it->change_count) {
        case 0:                 // no permutation change: remove
          it = intervals.erase(it);
          break;
        case 1:                 // single permutation change: ok
          ++it;
          break;
        default:                // too many permutation changes: split
          pci.tmin = it->tmin;
          pci.tmax = (it->tmin + it->tmax)/2;
          it->tmin = pci.tmax;
          vmin = f(pci.tmin);
          vmax = f(pci.tmax);
          pci.change_count = permutation_distance_circular_rank(vmin, vmax);
          if (pci.change_count > 0) {
            intervals.insert(it, pci);
            if (pci.change_count != 1)
              bad = true;
            std::swap(vmin, vmax);
            vmax = f(it->tmax);
            it->change_count = permutation_distance_circular_rank(vmin, vmax);
            if (it->change_count == 0) {
              it = intervals.erase(it);
            } else if (it->change_count != 1) {
              bad = true;
              ++it;
            }
          } else {
            bad = true;
            ++it;
          }
          break;
        }
      }
      if (!bad)
        break;
    }
    // Now intervals is a list in which every element has change_count == 1.
    // Figure out which two objects switch places during each interval
    for (auto it = intervals.begin(); it != intervals.end(); ++it) {
      vmin = f(it->tmin);
      std::vector<size_t> rmin = sort_rank(vmin);
      rotate_ranks_greatest_at_end(rmin);

      vmax = f(it->tmax);
      std::vector<size_t> rmax = sort_rank(vmax);
      rotate_ranks_greatest_at_end(rmax);

      // with three elements there are only two distinct circular permutations,
      // so it is impossible to say which two elements switched places.

      // with four or more elements, if the switching pair does not include the
      // last element (case A):

      // (Note that all but the last of the elements may be shuffled randomly
      // compared to the example.)

      // 0 - 1 - 2 - 3
      // 1 - 0 - 2 - 3
      // only two items differ; they differ by 1

      // if the switching pair consists of the largest two ranks (case B):

      // 0 - 1 - 2 - 3
      // 0 - 1 - 3 - 2 gets rotated to
      // 1 - 2 - 0 - 3
      // the last item is the same, one item differs by n − 1, the rest differs
      // by 1

      // if the switching pair consists of the greatest and smallest ranks (case
      // C):

      // 0 - 1 - 2 - 3
      // 3 - 1 - 2 - 0 gets rotated to
      // 2 - 0 - 1 - 3
      // the last item is the same, one item differs by n − 1, the rest differs
      // by 1

      if (rmin.size() > 2) {
        size_t ix;
        for (ix = 0; ix != rmin.size(); ++ix) {
          if (abs_diff(rmin[ix], rmax[ix]) > 1) {
            break;
          }
        }
        if (ix == rmin.size()) { // case A
          for (ix = 0; ix != rmin.size(); ++ix) {
            if (abs_diff(rmin[ix], rmax[ix]) == 1) {
              it->object1 = ix;
              break;
            }
          }
          for (++ix; ix != rmin.size(); ++ix) {
            if (abs_diff(rmin[ix], rmax[ix]) == 1) {
              it->object2 = ix;
              break;
            }
          }
        } else {                // case B or C
          it->object1 = ix;
          it->object2 = rmin.size() - 1;
        }
      } else {
        // 2 items
        // \todo can we handle this case better?
        return ts;
      } // end if (rmin.size() > 2)

      // Now we know which two items switched places.  We locate the time of
      // conjunction based on the difference in their positions.

      auto getd = [&](double t) -> double {
        auto v = f(t);
        return fasmod(v[it->object1] - v[it->object2], period);
      };

      // determine the final permutation number
      size_t pmax = permutation_number_circular_rank(vmax);

      auto dmin = getd(it->tmin);
      auto dmax = getd(it->tmax);
      size_t count = 0;
      while (1) {
        // the switch occurs sometime between tmin and tmax
        auto test = (dmax*it->tmin - dmin*it->tmax)
                   /(dmax          - dmin         );
        if (test < it->tmin || test > it->tmax) {
          // the estimate is outside the bracket; bisect instead
          test = (it->tmin + it->tmax)/2;
        }
        if (++count > 20) {
          // we're likely in some kind of loop; abort
          ts.push_back({test, it->object1, it->object2, pmax});
          break;
        }
        auto dest = getd(test);
        if (dmin*dest < 0) {
          // the target time is between tmin and test
          if (test == it->tmax) {
            // we cannot improve anymore
            ts.push_back({test, it->object1, it->object2, pmax});
            break;
          }
          // see if we can tighten tmin, too
          auto test2 = 2*test - it->tmax;
          if (test2 > it->tmin) {
            auto dest2 = getd(test2);
            if (dest2*dmin > 0) {
              it->tmin = test2;
              dmin = dest2;
            }
          }
          it->tmax = test;
          dmax = dest;
        } else if (dest*dmax < 0) {
          // the target time is between test and tmax
          if (test == it->tmin) {
            // we cannot improve anymore
            ts.push_back({test, it->object1, it->object2, pmax});
            break;
          }
          // see if we can tighten tmax, too
          auto test2 = 2*test - it->tmin;
          if (test2 < it->tmax) {
            auto dest2 = getd(test2);
            if (dest2*dmax > 0) {
              it->tmax = test2;
              dmax = dest2;
            }
          }
          it->tmin = test;
          dmin = dest;
        } else {
          // we cannot improve anymore
          ts.push_back({test, it->object1, it->object2, pmax});
          break;
        }
        if ((it->tmax - it->tmin) < std::max(it->tmax, it->tmin)
            * 2*std::numeric_limits<double>::epsilon()) {
          // we likely cannot improve anymore
          ts.push_back({test, it->object1, it->object2, pmax});
          break;
        }
      }
    } // end for (auto it = intervals.begin(); it != intervals.end(); ++it)
  } // end if (dtot > 0)
  return ts;
}

/// Implements the LUX function
///
///     jds = planetpermutationchanges(jd1, jd2, planets)
///
/// which calculates Julian Day numbers at which any two of the planets
/// (specified as for LUX function `astron`) have the same ICRS ecliptic
/// longitude.
///
/// \param narg is the number of LUX arguments
///
/// \param ps points at the sequence of LUX arguments
///
/// \returns the LUX symbol representing the result.
int32_t
lux_planetpermutationchanges(int32_t narg, int32_t ps[])
{
  double t1 = double_arg(ps[0]);
  double t2 = double_arg(ps[1]);
  int32_t nelem;
  Pointer src;
  if (numerical(ps[2], NULL, NULL, &nelem, &src) < 0) {
    return LUX_ERROR;
  }

  SolarSystemEphemerides sse;
  sse
    .set_coordinate_system(SolarSystemEphemerides::CoordinateSystem::Ecliptic);

  auto gl = [&](double JD){
    std::vector<double> l(nelem);
    std::transform(src.i32, src.i32 + nelem, l.begin(),
                   [&](int32_t o){
                     auto sso = static_cast<SolarSystemObject>(o);
                     auto polar = sse.polar_geocentric(JD, sso);
                     return polar[0];
                   });
    return l;
  };

  std::vector<PermutationChange> pc
    = find_permutation_changes(gl, 2*M_PI, t1, t2);

  int32_t size = pc.size();
  int32_t iq;
  if (size > 0) {
    int32_t dims[2];
    dims[0] = 4;
    dims[1] = size;
    iq = array_scratch(LUX_DOUBLE, 2, dims);
    Pointer tgt;
    tgt.v = array_data(iq);
    for (auto it = pc.begin(); it != pc.end(); ++it) {
      *tgt.d++ = it->t;
      *tgt.d++ = it->object1;
      *tgt.d++ = it->object2;
      *tgt.d++ = it->final_permutation;
    }
  } else {
    iq = LUX_MINUS_ONE;
  }
  return iq;
}
REGISTER(planetpermutationchanges, f, planetpermutationchanges, 3, 3, NULL, HAVE_LIBGSL);
#endif

// iD;iL*?;rL{1} → iiarx
// lux_i_sd_iiarx_<ptrspec>_f_
#if HAVE_LIBGSL
/// Implements the LUX `permutationnumber` function.
///
///     value = permutationnumber(data [, /rank, /index]
///                               [, /linear, /circular])
///     value = permutationnumber(data, period [, /rank, /index]) ; semicircular
///
/// \param[in] narg is the argument count.
///
/// \param[in,out] ps points at the array of LUX symbol numbers.
///
/// \returns the LUX symbol number representing the result.
int32_t
lux_permutationnumber(int32_t narg, int32_t ps[])
{
  int32_t icount;
  Pointer src;
  if (numerical(ps[0], NULL, NULL, &icount, &src) == LUX_ERROR)
    return LUX_ERROR;
  size_t count = (size_t) icount;

  // int32_t and uint32_t are suitable for (linear) n not exceeding 12, and
  // int64_t and uint64_t go up to 20.

  if (narg >= 2)
    internalMode = 0;

  int permutation_is_circular = ((internalMode & 2) != 0);

  int32_t result;
  if (count <= 12 + permutation_is_circular) {
    // int32_t is wide enough
    result = scalar_scratch(LUX_INT32);
  } else if (count <= 20 + permutation_is_circular) {
    // int64_t is wide enough
    result = scalar_scratch(LUX_INT64);
  } else {
    return luxerror("Cannot store permutation numbers for permutations "
                    "of more than 20 (linear, semicircular) or 21 (circular) "
                    "elements.", ps[0]);
  }

  Pointer tgt;
  tgt.i32 = &scalar_value(result).i32;

  if (narg >= 2) {
    // semicircular permutation
    size_t pn;
    switch (symbol_type(ps[0])) {
    case LUX_INT8:
      pn = permutation_number_semicircular_rank
        (src.ui8, count, get_scalar_value<uint8_t>(ps[1]));
      break;
    case LUX_INT16:
      pn = permutation_number_semicircular_rank
        (src.i16, count, get_scalar_value<int16_t>(ps[1]));
      break;
    case LUX_INT32:
      pn = permutation_number_semicircular_rank
        (src.i32, count, get_scalar_value<int32_t>(ps[1]));
      break;
    case LUX_INT64:
      pn = permutation_number_semicircular_rank
        (src.i64, count, get_scalar_value<int64_t>(ps[1]));
      break;
    case LUX_FLOAT:
      pn = permutation_number_semicircular_rank
        (src.f, count, get_scalar_value<float>(ps[1]));
      break;
    case LUX_DOUBLE:
      pn = permutation_number_semicircular_rank
        (src.d, count, get_scalar_value<double>(ps[1]));
      break;
    }
    switch (scalar_type(result)) {
    case LUX_INT32:
      *tgt.i32 = pn;
      break;
    case LUX_INT64:
      *tgt.i64 = pn;
      break;
    }
  } else if (permutation_is_circular) {
    // circular permutation

    // We first determine the ranks of the values (0 = least, 1 = next higher,
    // etc.).  If the input values are (equivalent to) indexes that sort the
    // elements into ascending order, then sort_index() returns the
    // corresponding 0-based ranks.  If the input values are values, then we
    // need to call sort_index() once to find the indexes, and then once more to
    // invert the indexes to find ranks.
    std::vector<size_t> r;
    switch (symbol_type(ps[0])) {
    case LUX_INT8:
      r = sort_index(src.ui8, 1, count);
      break;
    case LUX_INT16:
      r = sort_index(src.i16, 1, count);
      break;
    case LUX_INT32:
      r = sort_index(src.i32, 1, count);
      break;
    case LUX_FLOAT:
      r = sort_index(src.f, 1, count);
      break;
    case LUX_DOUBLE:
      r = sort_index(src.d, 1, count);
      break;
    }

    if ((internalMode & 1) == 0) {
      // the input values were ranks, not indexes.  r now contains indexes.
      // Invert them to get ranks.
      r = sort_index(r);
    }

    // Now we can calculate the permutation numbers from the ranks, which always
    // have the same data type (size_t).

    auto pn = permutation_number_circular_from_ranks(r);
    switch (scalar_type(result)) {
    case LUX_INT32:
      *tgt.i32 = pn;
      break;
    case LUX_INT64:
      *tgt.i64 = pn;
      break;
    }
  } else {
    // linear permutation

    size_t pn;
    if (internalMode & 1) {
      // by index
      switch (symbol_type(ps[0])) {
      case LUX_INT8:
        pn = permutation_number_linear_index(src.ui8, count);
        break;
      case LUX_INT16:
        pn = permutation_number_linear_index(src.i16, count);
        break;
      case LUX_INT32:
        pn = permutation_number_linear_index(src.i32, count);
        break;
      case LUX_INT64:
        pn = permutation_number_linear_index(src.i64, count);
        break;
      case LUX_FLOAT:
        pn = permutation_number_linear_index(src.f, count);
        break;
      case LUX_DOUBLE:
        pn = permutation_number_linear_index(src.d, count);
        break;
      }
    } else {
      // by rank
      switch (symbol_type(ps[0])) {
      case LUX_INT8:
        pn = permutation_number_linear_rank(src.ui8, count);
        break;
      case LUX_INT16:
        pn = permutation_number_linear_rank(src.i16, count);
        break;
      case LUX_INT32:
        pn = permutation_number_linear_rank(src.i32, count);
        break;
      case LUX_INT64:
        pn = permutation_number_linear_rank(src.i64, count);
        break;
      case LUX_FLOAT:
        pn = permutation_number_linear_rank(src.f, count);
        break;
      case LUX_DOUBLE:
        pn = permutation_number_linear_rank(src.d, count);
        break;
      }
    }

    switch (scalar_type(result)) {
    case LUX_INT32:
      *tgt.i32 = pn;
      break;
    case LUX_INT64:
      *tgt.i64 = pn;
      break;
    }
  }

  return result;
}
REGISTER(permutationnumber, f, permutationnumber, 1, 2, "0rank:1index:0linear:2circular", HAVE_LIBGSL);
#endif

/// Implements the `permutation` function in LUX.
///
/// `    p = permutation(pn, n [, /circular])`
///
/// \param[in] narg is the number of arguments.
///
/// \param[in,out] points at the LUX symbol number of the first argument.
///
/// \returns the LUX symbol number of the result.
int32_t
lux_permutation(int32_t narg, int32_t ps[])
{
  auto ipn = int_arg(ps[0]);
  if (ipn < 0)
    return luxerror("Permutation number must be a nonnegative integer", ps[0]);
  auto icount = int_arg(ps[1]);
  if (icount < 1)
    return luxerror("Permutation count must be a positive integer", ps[1]);

  int32_t result = array_scratch(LUX_INT32, 1, &icount);
  int32_t* tgt = (int32_t*) array_data(result);

  std::vector<size_t> p;
  if (internalMode & 2) {       // circular permutation
    p = permutation_circular_rank(ipn, icount);
  } else {
    p = permutation_linear_rank(ipn, icount);
  }
  if (internalMode & 1) {       // index-based permutation
    p = sort_index(p);          // convert ranks to indexes
  }
  if (p.empty())
    return luxerror("Permutation number is too large for the element count",
                    ps[0]);
  for (int i = 0; i < icount; ++i)
    *tgt++ = (int32_t) p[i];

  return result;
}
REGISTER(permutation, f, permutation, 2, 2, "0rank:1index:0linear:2circular", HAVE_LIBGSL);

#if HAVE_LIBGSL
/// \ingroup luxroutines
///
/// Implements the `permutationdistance` function in LUX.
///
///     d = permutationdistance(p1, p2 [, /rank, /index, /linear, /circular])
///
/// \param[in] narg is the number of arguments.
///
/// \param[in,out] points at the LUX symbol number of the first argument.
///
/// \returns the LUX symbol number of the result.
///
int32_t
lux_permutationdistance(int32_t narg, int32_t ps[])
{
  int32_t iq1 = ps[0];
  int32_t iq2 = ps[1];
  auto ct = combinedType(symbol_type(iq1), symbol_type(iq2));
  if (symbol_type(iq1) != ct)
    iq1 = lux_converts[ct](1, &iq1);
  if (symbol_type(iq2) != ct)
    iq2 = lux_converts[ct](1, &iq2);

  int32_t count1;
  int32_t count2;
  Pointer src1;
  Pointer src2;
  if (numerical(iq1, NULL, NULL, &count1, &src1) < 0
      || numerical(iq2, NULL, NULL, &count2, &src2) < 0)
    return LUX_ERROR;
  if (count1 != count2) {
    return luxerror("Must have the same number of elements "
                    "as the previous argument",
                    ps[1]);
  }
  int32_t iq = scalar_scratch(LUX_INT32);
  if (internalMode & 2) {       // circular permutations
    if (internalMode & 1) {     // based on index
      switch (ct) {
      case LUX_INT8:
        scalar_value(iq).i32
          = permutation_distance_circular_index(count1, src1.ui8, src2.ui8);
        break;
      case LUX_INT16:
        scalar_value(iq).i32
          = permutation_distance_circular_index(count1, src1.i16, src2.i16);
        break;
      case LUX_INT32:
        scalar_value(iq).i32
          = permutation_distance_circular_index(count1, src1.i32, src2.i32);
        break;
      case LUX_FLOAT:
        scalar_value(iq).i32
          = permutation_distance_circular_index(count1, src1.f, src2.f);
        break;
      case LUX_DOUBLE:
        scalar_value(iq).i32
          = permutation_distance_circular_index(count1, src1.d, src2.d);
        break;
      }
    } else {                    // based on rank
      switch (ct) {
      case LUX_INT8:
        scalar_value(iq).i32
          = permutation_distance_circular_rank(count1, src1.ui8, src2.ui8);
        break;
      case LUX_INT16:
        scalar_value(iq).i32
          = permutation_distance_circular_rank(count1, src1.i16, src2.i16);
        break;
      case LUX_INT32:
        scalar_value(iq).i32
          = permutation_distance_circular_rank(count1, src1.i32, src2.i32);
        break;
      case LUX_FLOAT:
        scalar_value(iq).i32
          = permutation_distance_circular_rank(count1, src1.f, src2.f);
        break;
      case LUX_DOUBLE:
        scalar_value(iq).i32
          = permutation_distance_circular_rank(count1, src1.d, src2.d);
        break;
      }
    }
  } else {                      // linear permutations
    if (internalMode & 1) {     // based on index
      switch (ct) {
      case LUX_INT8:
        scalar_value(iq).i32
          = permutation_distance_linear_index(count1, src1.ui8, src2.ui8);
        break;
      case LUX_INT16:
        scalar_value(iq).i32
          = permutation_distance_linear_index(count1, src1.i16, src2.i16);
        break;
      case LUX_INT32:
        scalar_value(iq).i32
          = permutation_distance_linear_index(count1, src1.i32, src2.i32);
        break;
      case LUX_FLOAT:
        scalar_value(iq).i32
          = permutation_distance_linear_index(count1, src1.f, src2.f);
        break;
      case LUX_DOUBLE:
        scalar_value(iq).i32
          = permutation_distance_linear_index(count1, src1.d, src2.d);
        break;
      }
    } else {                    // based on rank
      switch (ct) {
      case LUX_INT8:
        scalar_value(iq).i32
          = permutation_distance_linear_rank(count1, src1.ui8, src2.ui8);
        break;
      case LUX_INT16:
        scalar_value(iq).i32
          = permutation_distance_linear_rank(count1, src1.i16, src2.i16);
        break;
      case LUX_INT32:
        scalar_value(iq).i32
          = permutation_distance_linear_rank(count1, src1.i32, src2.i32);
        break;
      case LUX_FLOAT:
        scalar_value(iq).i32
          = permutation_distance_linear_rank(count1, src1.f, src2.f);
        break;
      case LUX_DOUBLE:
        scalar_value(iq).i32
          = permutation_distance_linear_rank(count1, src1.d, src2.d);
        break;
      }
    }
  }
  return iq;
}
REGISTER(permutationdistance, f, permutationdistance, 2, 2, "0rank:1index:0linear:2circular", HAVE_LIBGSL);
#endif

#if HAVE_LIBGSL
/// \ingroup luxroutines
///
/// Implements the `factorial` function in LUX.
///
///     f = factorial(x)
///
/// Depends on the Gnu Scientific Library.
///
/// \param[in] narg is the number of arguments.
///
/// \param[in,out] points at the LUX symbol number of the first argument.
///
/// \returns the LUX symbol number of the result.i
int32_t
lux_factorial(int32_t narg, int32_t ps[])
{
  int32_t count;
  int32_t* dims;
  int32_t ndim;
  Pointer src;
  if (numerical(ps[0], &dims, &ndim, &count, &src) < 0)
    return LUX_ERROR;

  int32_t iq;
  double* tgt;
  if (count == 1) {
    iq = scalar_scratch(LUX_DOUBLE);
    tgt = &scalar_value(iq).d;
  } else {
    iq = array_scratch(LUX_DOUBLE, ndim, dims);
    tgt = (double*) array_data(iq);
  }
  switch (symbol_type(ps[0])) {
  case LUX_INT8:
    for (int i = 0; i < count; ++i) {
      *tgt = gsl_sf_fact(*src.ui8);
      ++src.ui8;
    }
    break;
  case LUX_INT16:
    for (int i = 0; i < count; ++i) {
      *tgt = *src.i16 >= 0? gsl_sf_fact(*src.i16): 0;
      ++src.i16;
    }
    break;
  case LUX_INT32:
    for (int i = 0; i < count; ++i) {
      *tgt = *src.i32 >= 0? gsl_sf_fact(*src.i32): 0;
      ++src.i32;
    }
    break;
  case LUX_FLOAT:
    for (int i = 0; i < count; ++i) {
      *tgt = *src.f >= 0? gsl_sf_fact(*src.f): 0;
      ++src.f;
    }
    break;
  case LUX_DOUBLE:
    for (int i = 0; i < count; ++i) {
      *tgt = *src.d >= 0? gsl_sf_fact(*src.f): 0;
      ++src.d;
    }
    break;
  }
  return iq;
}
REGISTER(factorial, f, factorial, 1, 1, NULL, HAVE_LIBGSL);
#endif

/// This template function provides the main action for function
/// lux_decompose_2d_median3().  It decomposes a source image into an image of
/// the same dimensions that shows only the small scale details (referred to
/// below as the first output image) and a smaller image that shows only the
/// large scale details (the second output image).
///
/// The source image is divided into tiles of 3 by 3 elements, except that the
/// tiles at the right-hand and bottom edges can be smaller if the dimensions of
/// the source image aren't divisible by 3.  The second output image is composed
/// of the median value of all tiles.  The first output image contains the
/// difference between source image tiles and the median value of those tiles.
///
/// \tparam T is the numerical data type of the data values.
///
/// \param[in] vin points at the start of the source image values.
///
/// \param[in] dims holds the dimensions of the source image, which are also
/// the dimensions of the first output image.
///
/// \param[out] vout1 points at the start of the memory area where the values of
/// the first output image get written.  The memory area must be large enough to
/// hold `dims[0]*dims[1]` data values.
///
/// \param[out] vout2 points at the start of the memory area where the values of
/// the second output image get written.  The memory area must be large enough
/// to hold `ceil(dims[0]/3.0)*ceil(dims[1]/3.0)` elements.
template<typename T>
void
lux_decompose_2d_median3_action(const void* vin, int32_t dims[2], void* vout1,
                                void* vout2)
{
  const T* in = static_cast<const T*>(vin);
  T* out1 = static_cast<T*>(vout1);
  T* out2 = static_cast<T*>(vout2);
  const int side = 3;
  T data[side*side];

  int rightedge = dims[0]%side;
  int bottomedge = dims[1]%side;

  // We process the input tile by tile, where each tile has side*side elements
  // -- except perhaps at the right-hand and lower edges.  First we handle the
  // rows of tiles for which we have the full height.
  for (int y = 0; y < dims[1] - dims[1]%side; y += side) {
    // in each row, we first handle the tiles that are complete (have side*side
    // elements).
    for (int x = 0; x < dims[0] - dims[0]%side; x += side) {
      // copy input values from the current tile into the temporary array
      for (int doff = 0, inoff = 0;
           doff < side*side;
           doff += side, inoff += dims[0]) {
        memcpy(data + doff, in + inoff, side*sizeof(*in));
      }

      // determine the median: sort, then take the middle element
      std::sort(&data[0], &data[9]);
      auto v = data[(side*side)/2];

      // fill out1 with the input values minus the median
      for (int y2 = 0, inoff = 0; y2 < side; ++y2, inoff += dims[0]) {
        for (int x2 = 0; x2 < side; ++x2) {
          out1[inoff + x2] = in[inoff + x2] - v;
        }
      }
      in += side;
      out1 += side;

      // store the median in out2
      *out2++ = v;
    }
    if (rightedge) {
      // We have less than a full tile (only rightedge elements wide) at the
      // right-hand edge.  Copy input values from the current tile into the
      // temporary array.
      for (int doff = 0, inoff = 0;
           doff < side*rightedge;
           doff += rightedge, inoff += dims[0]) {
        memcpy(data + doff, in + inoff, rightedge*sizeof(*in));
      }

      // Determine the median: sort, then take the middle element.  If the
      // number of elements is even then we take one of the elements that are
      // closest to the middle.
      std::sort(&data[0], &data[side*rightedge]);
      T v = data[(side*rightedge)/2]; // (near) median

      // fill out1 with the input values minus the median
      for (int y2 = 0, inoff = 0; y2 < side; ++y2, inoff += dims[0]) {
        for (int x2 = 0; x2 < rightedge; ++x2) {
          out1[inoff + x2] = in[inoff + x2] - v;
        }
      }
      in += rightedge;
      out1 += rightedge;

      // store the median in out2
      *out2++ = v;
    }
    in += 2*dims[0];
    out1 += 2*dims[0];
  }
  if (bottomedge) {
    // We have less than full tiles (only bottomedge elements tall) in the
    // bottom row.  We first handle the tiles that have full width.
    for (int x = 0; x < dims[0] - dims[0]%side; x += side) {
      // copy input values from the current tile into the temporary array
      for (int doff = 0, inoff = 0;
           doff < side*bottomedge;
           doff += side, inoff += dims[0]) {
        memcpy(data + doff, in + inoff, side*sizeof(*in));
      }

      // determine the median: sort, then take the middle element
      std::sort(&data[0], &data[9]);
      auto v = data[(side*bottomedge)/2];

      // fill out1 with the input values minus the median
      for (int y2 = 0, inoff = 0; y2 < bottomedge; ++y2, inoff += dims[0]) {
        for (int x2 = 0; x2 < side; ++x2) {
          out1[inoff + x2] = in[inoff + x2] - v;
        }
      }
      in += side;
      out1 += side;

      // store the median in out2
      *out2++ = v;
    }
    if (rightedge) {
      // The very last tile is both shorter and narrower (only rightedge
      // elements wide, only bottomedge elements tall).  Copy input values from
      // the current tile into the temporary array.
      for (int doff = 0, inoff = 0;
           doff < bottomedge*rightedge;
           doff += rightedge, inoff += dims[0]) {
        memcpy(data + doff, in + inoff, rightedge*sizeof(*in));
      }

      // Determine the median: sort, then take the middle element.  If the
      // number of elements is even then we take one of the elements that are
      // closest to the middle.
      std::sort(&data[0], &data[bottomedge*rightedge]);
      T v = data[(bottomedge*rightedge)/2]; // (near) median

      // fill out1 with the input values minus the median
      for (int y2 = 0, inoff = 0; y2 < bottomedge; ++y2, inoff += dims[0]) {
        for (int x2 = 0; x2 < rightedge; ++x2) {
          out1[inoff + x2] = in[inoff + x2] - v;
        }
      }
      // store the median in out2
      *out2 = v;
    }
  }
}

int32_t
lux_decompose_2d_median3(int32_t narg, int32_t ps[])
{
  int32_t in = ps[0];
  if (!symbolIsNumericalArray(in)
      || array_num_dims(in) != 2)
    return cerror(NEED_2D_ARR, in);

  int32_t dims[2];
  memcpy(dims, array_dims(in), sizeof(dims)); // copy 2 int32_t

  int32_t out1 = ps[1];
  to_scratch_array(out1, symbol_type(in), sizeof(dims)/sizeof(dims[0]), dims);

  int32_t out2 = ps[2];
  // out2's dimension is ceil(n/3) where n is in's corresponding dimension
  for (int i = 0; i < sizeof(dims)/sizeof(dims[0]); ++i)
    dims[i] = dims[i]/3 + (dims[i]%3 != 0);
  to_scratch_array(out2, symbol_type(in), sizeof(dims)/sizeof(dims[0]), dims);

  switch (array_type(in)) {
  case LUX_INT8:
    lux_decompose_2d_median3_action<uint8_t>(array_data(in),
                                             array_dims(in),
                                             array_data(out1),
                                             array_data(out2));
    break;
  case LUX_INT16:
    lux_decompose_2d_median3_action<int16_t>(array_data(in),
                                             array_dims(in),
                                             array_data(out1),
                                             array_data(out2));
    break;
  case LUX_INT32:
    lux_decompose_2d_median3_action<int32_t>(array_data(in),
                                             array_dims(in),
                                             array_data(out1),
                                             array_data(out2));
    break;
  case LUX_INT64:
    lux_decompose_2d_median3_action<int64_t>(array_data(in),
                                             array_dims(in),
                                             array_data(out1),
                                             array_data(out2));
  case LUX_FLOAT:
    lux_decompose_2d_median3_action<float>(array_data(in),
                                           array_dims(in),
                                           array_data(out1),
                                           array_data(out2));
    break;
  case LUX_DOUBLE:
    lux_decompose_2d_median3_action<double>(array_data(in),
                                            array_dims(in),
                                            array_data(out1),
                                            array_data(out2));
    break;
  default:
    return cerror(ILL_TYPE, 0);
  }
  return LUX_OK;
}
REGISTER(decompose_2d_median3, s, decompose2dmedian3, 3, 3, NULL);

/// This template function provides the main action for function
/// lux_compose_2d(), which does the opposite of lux_decompose_2d_median3().
/// Conceptually, it expands the second source image by a factor of 3 in both
/// dimensions, chops off the right-hand and bottom edges as neeeded to make it
/// have the same size as the first source image, and then adds it to the first
/// source image to produce the target image.
///
/// \tparam T is the numerical data type of the data values.
///
/// \param[in] vout points at the start of the memory area where the values of
/// the output image get written.  The memory area must be large enough to hold
/// `dims[0]*dims[1]` data values.
///
/// \param[in] dims holds the dimensions of the first source image, which are
/// also the dimensions of the output image.
///
/// \param[out] vin1 points at the start of the data values of the first input
/// image.
///
/// \param[out] vin2 points at the start of the data values of the second input
/// image.  It must hold `ceil(dims[0]/3.0)` by `ceil(dims[1]/3.0)` elements.
template<typename T>
void
lux_compose_2d_action(void* vout, int32_t dimsout[2], const void* vin1,
                      const void* vin2)
{
  T* out = static_cast<T*>(vout);
  const T* in1 = static_cast<const T*>(vin1);
  const T* in2 = static_cast<const T*>(vin2);
  const int side = 3;
  T data[side*side];

  int rightedge = dimsout[0]%side;
  int bottomedge = dimsout[1]%side;

  // We process the output tile by tile, where each tile has side*side elements
  // -- except perhaps at the right-hand and lower edges.  First we handle the
  // rows of tiles for which we have the full height.
  for (int y = 0; y < dimsout[1] - dimsout[1]%side; y += side) {
    // in each row, we first handle the tiles that are complete (have side*side
    // elements).
    for (int x = 0; x < dimsout[0] - dimsout[0]%side; x += side) {
      // fill out with the sum of the relevant input values
      for (int y2 = 0, inoff = 0; y2 < side; ++y2, inoff += dimsout[0]) {
        for (int x2 = 0; x2 < side; ++x2) {
          out[inoff + x2] = in1[inoff + x2] + *in2;
        }
      }
      out += side;
      in1 += side;
      ++in2;
    }
    if (rightedge) {
      // We have less than a full tile (only rightedge elements wide) at the
      // right-hand edge.
      for (int y2 = 0, inoff = 0; y2 < side; ++y2, inoff += dimsout[0]) {
        for (int x2 = 0; x2 < rightedge; ++x2) {
          out[inoff + x2] = in1[inoff + x2] + *in2;
        }
      }
      out+= rightedge;
      in1 += rightedge;
      ++in2;
    }
    out+= 2*dimsout[0];
    in1 += 2*dimsout[0];
  }
  if (bottomedge) {
    // We have less than full tiles (only bottomedge elements tall) in the
    // bottom row.  We first handle the tiles that have full width.
    for (int x = 0; x < dimsout[0] - dimsout[0]%side; x += side) {
      for (int y2 = 0, inoff = 0; y2 < bottomedge; ++y2, inoff += dimsout[0]) {
        for (int x2 = 0; x2 < side; ++x2) {
          out[inoff + x2] = in1[inoff + x2] + *in2;
        }
      }
      out += side;
      in1+= side;
      ++in2;
    }
    if (rightedge) {
      // The very last tile is both shorter and narrower (only rightedge
      // elements wide, only bottomedge elements tall).
      for (int y2 = 0, inoff = 0; y2 < bottomedge; ++y2, inoff += dimsout[0]) {
        for (int x2 = 0; x2 < rightedge; ++x2) {
          out[inoff + x2] = in1[inoff + x2] + *in2;
        }
      }
    }
  }
}

int32_t
lux_compose_2d(int32_t narg, int32_t ps[])
{
  int32_t in1 = ps[1];
  if (!symbolIsNumericalArray(in1)
      || array_num_dims(in1) != 2)
    return cerror(NEED_2D_ARR, in1);

  int32_t dims[2];
  memcpy(dims, array_dims(in1), sizeof(dims));

  int32_t in2 = ps[2];
  if (!symbolIsNumericalArray(in2)
      || array_num_dims(in2) != 2)
    return cerror(NEED_2D_ARR, in2);

  // in1 and in2 must have the same data type
  if (symbol_type(ps[2]) != symbol_type(ps[1]))
    return cerror(INCMP_ARG, in2);

  // in2's dimension must be ceil(n/3) where n is in1's corresponding dimension
  int32_t* dims2 = array_dims(in2);
  for (int i = 0; i < sizeof(dims)/sizeof(dims[0]); ++i)
    if (dims2[i] != dims[i]/3 + (dims[i]%3 != 0))
      return cerror(INCMP_DIMS, in2, in1);

  int32_t out = ps[0];
  to_scratch_array(out, symbol_type(in1), sizeof(dims)/sizeof(dims[0]), dims);

  switch (array_type(in1)) {
  case LUX_INT8:
    lux_compose_2d_action<uint8_t>(array_data(out),
                                   array_dims(in1),
                                   array_data(in1),
                                   array_data(in2));
    break;
  case LUX_INT16:
    lux_compose_2d_action<int16_t>(array_data(out),
                                   array_dims(in1),
                                   array_data(in1),
                                   array_data(in2));
    break;
  case LUX_INT32:
    lux_compose_2d_action<int32_t>(array_data(out),
                                   array_dims(in1),
                                   array_data(in1),
                                   array_data(in2));
    break;
  case LUX_INT64:
    lux_compose_2d_action<int64_t>(array_data(out),
                                   array_dims(in1),
                                   array_data(in1),
                                   array_data(in2));
  case LUX_FLOAT:
    lux_compose_2d_action<float>(array_data(out),
                                 array_dims(in1),
                                 array_data(in1),
                                 array_data(in2));
    break;
  case LUX_DOUBLE:
    lux_compose_2d_action<double>(array_data(out),
                                  array_dims(in1),
                                  array_data(in1),
                                  array_data(in2));
    break;
  default:
    return cerror(ILL_TYPE, 0);
  }
  return LUX_OK;
}
REGISTER(compose_2d, s, compose2d, 3, 3, NULL);

/// A template function to divide data using cdiv(), iterating over the data via
/// LoopInfo functionality.  The division produces the quotient and nonnegative
/// remainder of dividing the numerator by the denominator.
///
/// \tparam T is the type of the data values
///
/// \param infos points at the 3 relevant LoopInfo instances corresponding to \a
/// num, \a denom, and \a tgt.
///
/// \param[in] num points at the numerators.
///
/// \param[in] denom is the denominator.
///
/// \param[out] tgt points at the memory where to write the results.  There must
/// be enough memory available there.
template<typename T>
void
lux_div_action(LoopInfo* infos, T* num, T denom, T* tgt)
{
  do {
    auto result = cdiv(*num, denom);
    *tgt++ = result.quot;
    *tgt++ = result.rem;
  } while (infos[0].advanceLoop(&num) < infos[0].rndim);
}

/// A template function to divide data using cdiv(), iterating over the data via
/// LoopInfo functionality.  The division produces the quotient and nonnegative
/// remainder of dividing the numerator by the denominator.
///
/// \tparam T is the type of the data values
///
/// \param infos points at the 3 relevant LoopInfo instances corresponding to \a
/// num, \a denom, and \a tgt.
///
/// \param[in] num points at the numerators.
///
/// \param[in] denom points at the the denominators.  There must be as many of
/// them as there are numerators.
///
/// \param[out] tgt points at the memory where to write the results.  There must
/// be enough memory available there.
template<typename T>
void
lux_div_action(LoopInfo* infos, T* num, T* denom, T* tgt)
{
  do {
    auto result = cdiv(*num, *denom);
    *tgt++ = result.quot;
    *tgt++ = result.rem;
  } while (infos[1].advanceLoop(&denom),
           infos[0].advanceLoop(&num) < infos[0].rndim);
}

/// Implements LUX function `div`.
int32_t
lux_div(int32_t narg, int32_t ps[])
{
  StandardArguments sa;
  int32_t iq;
  Pointer* ptrs;
  LoopInfo* infos;
  if ((iq = sa.set(narg, ps, "i^*;i^#;r^+2&", &ptrs, &infos)) < 0)
    return LUX_ERROR;

  if (infos[1].nelem == 1) {
    switch (infos[0].type) {
    case LUX_INT8:
      lux_div_action(infos, ptrs[0].ui8, *ptrs[1].ui8, ptrs[2].ui8);
      break;
    case LUX_INT16:
      lux_div_action(infos, ptrs[0].i16, *ptrs[1].i16, ptrs[2].i16);
      break;
    case LUX_INT32:
      lux_div_action(infos, ptrs[0].i32, *ptrs[1].i32, ptrs[2].i32);
      break;
    case LUX_INT64:
      lux_div_action(infos, ptrs[0].i64, *ptrs[1].i64, ptrs[2].i64);
      break;
    default:
      return cerror(ILL_TYPE, ps[0]);
    }
  } else {
    switch (infos[0].type) {
    case LUX_INT8:
      lux_div_action(infos, ptrs[0].ui8, ptrs[1].ui8, ptrs[2].ui8);
      break;
    case LUX_INT16:
      lux_div_action(infos, ptrs[0].i16, ptrs[1].i16, ptrs[2].i16);
      break;
    case LUX_INT32:
      lux_div_action(infos, ptrs[0].i32, ptrs[1].i32, ptrs[2].i32);
      break;
    case LUX_INT64:
      lux_div_action(infos, ptrs[0].i64, ptrs[1].i64, ptrs[2].i64);
      break;
    default:
      return cerror(ILL_TYPE, ps[0]);
    }
  }

  return iq;
}
REGISTER(div, f, div, 2, 2, NULL);
