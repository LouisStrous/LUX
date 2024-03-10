/* This is file strous2.cc.

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
// File strous2.c
// Various LUX routines by L. Strous.
#if HAVE_CONFIG_H
#include "config.h"
#endif
#include "action.hh"
#include <algorithm>
#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <memory>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>             // for sbrk()

int32_t         minmax(int32_t *, int32_t, int32_t),
  lux_convert(int32_t, int32_t [], Symboltype, int32_t),
  copySym(int32_t), f_decomp(float *, int32_t, int32_t),
  copyToSym(int32_t, int32_t),
  f_solve(float *, float *, int32_t, int32_t);

int32_t d_decomp(double *, int32_t, int32_t),
  d_solve(double *, double *, int32_t, int32_t);
//---------------------------------------------------------
int32_t lux_noop(ArgumentCount narg, Symbol ps[])
     /* no operation - a dummy routine that provides an entry point
        for debuggers */
{
  return 1;
}
//---------------------------------------------------------
int32_t lux_zap(ArgumentCount narg, Symbol ps[])
// remove the specified symbols completely: i.e. leave no trace
// only allowed from main execution level, to prevent zapping of
// a variable which may be referenced later on in an already-compiled
// -and-now-executing piece of code
{
  int32_t       iq;
  void  cleanUpRoutine(int32_t context, char keepBase);

  if (curContext > 0)
    return luxerror("ZAP only allowed from main execution level!", 0);
  while (narg--) {
    if ((internalMode & 1) == 0
        || (symbol_class(*ps) != LUX_POINTER
            && symbol_class(*ps) != LUX_TRANSFER))
      iq = eval(*ps++);
    else
      iq = *ps++;
    if (symbol_class(iq) != LUX_FUNC_PTR)
      zap(iq);
    else {
      iq = func_ptr_routine_num(iq);
      if (iq < 0)               // an internal routine or function
        return luxerror("Cannot zap an internal routine or function!", *ps);
      cleanUpRoutine(iq, 0);
    }
  }
  return 1;
}
//---------------------------------------------------------
int32_t showstats(ArgumentCount narg, Symbol ps[])
{
  extern int16_t        listStack[], *listStackItem, curContext;
  extern int32_t        nNamedVariable, nTempVariable, markIndex,
                tempExecutableIndex, executeLevel,
                nSymbolStack, nExecutable, statementDepth, compileLevel;
  extern char   *firstbreak;
#if DEBUG
  extern int32_t        nAlloc;
  extern size_t         tSize;

  printf("NALL    TALLOC ");
#endif
  printf("       SZ IGN STD EXL CPL LST NVAR TVAR  EXE TEXE SSTK CTXT  TM\n");
#if DEBUG
  printf("%4d %9d ", nAlloc, tSize);
#endif
  printf("%9d %3d %3d %3d %3d %3d %4d %4d %4d %4d %4d %4d %3d\n",
         (char *) sbrk(0) - firstbreak,
         ignoreInput, statementDepth, executeLevel, compileLevel,
         listStackItem - listStack, nNamedVariable,
         nTempVariable, nExecutable,
         tempExecutableIndex - TEMP_EXE_START, nSymbolStack,
         curContext, markIndex - 1);
  return 1;
}
//---------------------------------------------------------
int32_t lux_system(ArgumentCount narg, Symbol ps[])
// pass a string to the operating system
{
  char  *name;

  name = string_arg(*ps);
  system(name);
  return LUX_OK;
}
//---------------------------------------------------------
int32_t lux_psum(ArgumentCount narg, Symbol ps[])
/* Y = PSUM(X, POWERS [,AXES, CLASS][,/ONEDIM,/NORMALIZE,/SINGLE]) returns a
   weighted sum of <x>. */
// The weights are equal to the coordinates in the dimensions given in
// <axes> to the integer powers given in <powers>.  <powers> and <axes>
// may be scalars or arrays and must have the same number of elements.
/* If <axes> is not specified, then INDGEN(POWERS) is assumed.
   Keyword */
/* /ONEDIM indicates that <x> should be treated as a 1D array.  LS
   8jan96 */
// If CLASS is specified, then the results are gathered separately
// for each class.  /NORMALIZE forces normalization of the results.
// LS 17feb97
// /SINGLE collects the results in a single scalar (unless CLASS is
// specified) LS 17mar97
{
  int32_t tally[MAX_DIMS], step[MAX_DIMS], *dims, done = 0, n, i, done2, done3,
        ndim, type, *axes, naxes, result_sym = 0, size, iq, iStep[MAX_DIMS];
  Pointer       src, trgt;
  int32_t       *exponent, nexponent, j, k, *temp, outdims[MAX_DIMS],
        in[MAX_DIMS], *index, nIndex, offset;
  double        value, factor, weight, bigweight, *bigweights;
  char  haveClass = '\0';
  extern Scalar         lastmin, lastmax;

  iq = lux_long(1, &ps[1]);     // integer exponents only
  switch (symbol_class(iq)) {   // POWERS
    case LUX_SCALAR:
      nexponent = 1;
      exponent = &scalar_value(iq).i32;
      break;
    case LUX_ARRAY:
      nexponent = array_size(iq);
      exponent = (int32_t *) array_data(iq);
      break;
    default:
      return cerror(ILL_CLASS, ps[1]);
  }
  temp = exponent;
  for (i = 0; i < nexponent; i++) {
    if (temp[i++] < 0)
      return luxerror("PSUM - Only nonnegative integer exponents allowed", ps[1]);
  }
  switch (symbol_class(*ps)) {  // X
    case LUX_SCALAR:
      ndim = 1;
      dims = &ndim;
      src.i32 = &scalar_value(*ps).i32;
      size = 1;
      break;
    case LUX_ARRAY:
      ndim = array_num_dims(*ps);
      dims = array_dims(*ps);
      src.i32 = (int32_t *) array_data(*ps);
      size = array_size(*ps);
      break;
    default:
      return cerror(ILL_CLASS, *ps);
  }
  if (internalMode & 1 && ndim > 1) { // /ONEDIM
    ndim = 1;
    dims = &size;
  }
  type = symbol_type(*ps);
  if (narg > 2) {
    iq = lux_long(1, &ps[2]);   // AXES
    switch (symbol_class(iq)) {
      case LUX_SCALAR:
        naxes = 1;
        axes = &scalar_value(iq).i32;
        break;
      case LUX_ARRAY:
        naxes = array_size(iq);
        axes = (int32_t *) array_data(iq);
        break;
      default:
        return cerror(ILL_CLASS, *ps);
    }
    if (naxes != nexponent)
      return cerror(INCMP_ARG, ps[2]);
    temp = axes;
    for (i = 0; i < naxes; i++) {
      if (temp[i] < 0 || temp[i] >= ndim)
        return cerror(ILL_DIM, ps[2]);
    }
    // determine the dimensions of the result
    for (i = 0; i < ndim; i++)
      in[i] = 0;
    for (i = 0; i < naxes; i++)
      in[axes[i]]++;
    for (i = 0; i < ndim; i++)
      if (in[i] > 1)
        return luxerror("A dimension was specified multiple times", ps[2]);
    j = 0;
    for (i = 0; i < ndim; i++)
      if (!in[i])               // no summing this dim, so is in result
        outdims[j++] = dims[i];
    if (narg > 3) {             // CLASS
      iq = lux_long(1, &ps[3]);         // integer classes
      haveClass = 1;
      index = (int32_t*) array_data(iq);
      nIndex = array_size(iq);
      if (nIndex != size)
        return cerror(INCMP_ARR, ps[3]);
      minmax(index, nIndex, LUX_INT32); // determine range of indices
      size = lastmax.i32 + 1;
      offset = 0;
      if (lastmin.i32 < 0)
        size += (offset = -lastmin.i32);
      result_sym = array_scratch(LUX_DOUBLE, 1, &size);
      trgt.i32 = (int32_t*) array_data(result_sym);
      zerobytes(trgt.ui8, size*sizeof(double));
      ALLOCATE(bigweights, size, double);
      zerobytes(bigweights, size*sizeof(double));
    }
  } else {                      // no AXES, assume INDGEN(POWERS)
    axes = NULL;
    naxes = nexponent;
    if (ndim > naxes)
      memcpy(outdims, dims + naxes, (ndim - naxes)*sizeof(int32_t));
  }
  if (!result_sym) {
    if (ndim == 1 || naxes == ndim || internalMode & 8) {
      result_sym = scalar_scratch(LUX_DOUBLE);
      trgt.ui8 = &scalar_value(result_sym).ui8;
    } else {
      n = 1;
      for (i = 0; i < ndim - naxes; i++)
        n *= outdims[i];
      result_sym = array_scratch(LUX_DOUBLE, 1, &n);
      array_num_dims(result_sym) = ndim - naxes;
      memcpy(array_dims(result_sym), outdims, (ndim - naxes)*sizeof(int32_t));
      trgt.i32 = (int32_t*) array_data(result_sym);
    }
  }

                                // initialize tally &c
  n = *step = lux_type_size[type];
  for (i = 1; i < ndim; i++)
    step[i] = (n *= dims[i - 1]);
  if (axes) {                   // swap dims so requested ones are first
    for (i = 0; i < naxes; i++) {
      outdims[i] = dims[axes[i]];
      tally[i] = step[axes[i]];
    }
    j = naxes;
    for (i = 0; i < ndim; i++)
      if (!in[i]) {
        outdims[j] = dims[i];
        tally[j++] = step[i];
      }
    memcpy(step, tally, ndim*sizeof(int32_t));
  } else
    memcpy(outdims, dims, ndim*sizeof(int32_t));
  for (i = ndim - 1; i; i--)
    step[i] -= step[i - 1]*outdims[i - 1];
  for (i = 0; i < ndim; i++)
    iStep[i] = step[i]/lux_type_size[type];
  for (i = 0; i < ndim; i++)
    tally[i] = 0;
  if (ndim == 1)
    done = 1;                   // only one go is needed
  *trgt.d = bigweight = 0;
  done2 = (internalMode & 6) && !haveClass && !(internalMode & 8);
  done3 = !haveClass && !(internalMode & 8);
  do {                        // loop over all elements
    switch (type) {
      case LUX_INT8:
        value = (double) *src.ui8;
        break;
      case LUX_INT16:
        value = (double) *src.i16;
        break;
      case LUX_INT32:
        value = (double) *src.i32;
        break;
      case LUX_INT64:
        value = (double) *src.i64;
        break;
      case LUX_FLOAT:
        value = (double) *src.f;
        break;
      case LUX_DOUBLE:
        value = *src.d;
        break; }
    weight = 1.0;
    for (j = 0; j < nexponent; j++)
      if (exponent[j]) {        // not equal to 0
        factor = tally[j];      // current jth coordinate
        if (exponent[j] % 2)
          weight *= factor;
        k = exponent[j]/2;
        while (k) {
          factor *= factor;
          if (k % 2)
            weight *= factor;
          k /= 2;
        }
      }
    if (haveClass) {
      if (internalMode & 2)     // /VNORMALIZE
        bigweights[*index] += value;
      else if (internalMode & 4) // /CNORMALIZE
        bigweights[*index] += weight;
      trgt.d[*index] += value*weight;
      index += iStep[0];
    } else {
      if (internalMode & 2)     // /VNORMALIZE
        bigweight += value;
      else if (internalMode & 4) // /CNORMALIZE
        bigweight += weight;
      *trgt.d += value*weight;
    }
    src.ui8 += step[0];
    for (i = 0; i < ndim; i++) {  // must another dimension be advanced?
      if (tally[i]++ != outdims[i] - 1) {
        done = 0;
        break;                  // not yet finished with this dimension
      }
      tally[i] = 0;
      done = 1;                         // done with this dimension anyway
      src.ui8 += step[i + 1];     // advance another dimension
      if (haveClass)
        index += iStep[i + 1];
    }
    if (i >= naxes) {
      if (done2) {
        if (bigweight)
          *trgt.d /= bigweight;
        else
          *trgt.d = 0.0;
      }
      if (done3 && !done)
        *++trgt.d = bigweight = 0.0;
    }
  } while (!done);
  if (haveClass) {
    for (i = 0; i < size; i++)
      if (bigweights[i])
        trgt.d[i] /= bigweights[i];
    free(bigweights);
  } else if (internalMode & 8 && internalMode & 6) // /SINGLE + /.NORMALIZE
    *trgt.d /= bigweight;
  return result_sym;
}
//---------------------------------------------------------
Pointer         multiCompData;
int32_t         multiCompNPoints, multiCompNCoord, multiCompType;
int32_t multiCompare(const void *arg1, const void *arg2)
// comparison function for use with lux_tolookup.  LS 9nov95
{
  int32_t       i1, i2, i;
  Scalar        d;

  i1 = *(int32_t *) arg1;
  i2 = *(int32_t *) arg2;
  switch (multiCompType) {
  case LUX_INT8:
    for (i = 0; i < multiCompNCoord; i++) {
      d.i32 = (int32_t) multiCompData.ui8[i1 + i*multiCompNPoints]
        - (int32_t) multiCompData.ui8[i2 + i*multiCompNPoints];
      if (d.i32)
        return d.i32;
    }
    break;
  case LUX_INT16:
    for (i = 0; i < multiCompNCoord; i++) {
      d.i32 = (int32_t) multiCompData.i16[i1 + i*multiCompNPoints]
        - (int32_t) multiCompData.i16[i2 + i*multiCompNPoints];
      if (d.i32)
        return d.i32;
    }
    break;
  case LUX_INT32:
    for (i = 0; i < multiCompNCoord; i++) {
      d.i32 = (int32_t) multiCompData.i32[i1 + i*multiCompNPoints]
        - (int32_t) multiCompData.i32[i2 + i*multiCompNPoints];
      if (d.i32)
        return d.i32;
    }
    break;
  case LUX_INT64:
    for (i = 0; i < multiCompNCoord; i++) {
      d.i64 = multiCompData.i64[i1 + i*multiCompNPoints]
        - multiCompData.i64[i2 + i*multiCompNPoints];
      if (d.i64)
        return d.i64 > 0? 1: -1;
    }
    break;
  case LUX_FLOAT:
    for (i = 0; i < multiCompNCoord; i++) {
      d.f =  multiCompData.f[i1 + i*multiCompNPoints]
        - multiCompData.f[i2 + i*multiCompNPoints];
      if (d.f)
        return d.f > 0? 1: -1;
    }
    break;
  case LUX_DOUBLE:
    for (i = 0; i < multiCompNCoord; i++) {
      d.d =  multiCompData.d[i1 + i*multiCompNPoints]
        - multiCompData.d[i2 + i*multiCompNPoints];
      if (d.d)
        return d.d > 0? 1: -1;
    }
    break;
  }
  return 0;
}
//---------------------------------------------------------
template<typename T>
struct MultiCompare
{
  MultiCompare(const T* data, Size coordinates_count, T tolerance = 0)
    : m_data(data), m_coordinates_count(coordinates_count),
      m_tolerance(tolerance)
  { }

  bool
  operator()(const Size& ilhs, const Size& irhs)
  {
    for (Size i = 0; i < m_coordinates_count; ++i)
    {
      auto lhs = m_data[i + ilhs*m_coordinates_count];
      auto rhs = m_data[i + irhs*m_coordinates_count];
      if (lhs < rhs)
        return true;
      else if (lhs > rhs)
        return false;
    }
    return false;
  }

  bool
  equals(Size ilhs, Size irhs)
  {
    for (Size i = 0; i < m_coordinates_count; ++i)
    {
      auto lhs = m_data[i + ilhs*m_coordinates_count];
      auto rhs = m_data[i + irhs*m_coordinates_count];
      if (lhs < rhs
          && rhs - lhs > m_tolerance)
        return false;
      if (lhs > rhs
          && lhs - rhs > m_tolerance)
        return false;
    }
    return true;
  }

  const T* m_data;
  Size m_coordinates_count;
  T m_tolerance;
};

template<typename T>
Symbol
tolookup_action(ArgumentCount narg, Symbol ps[])
{
  Symbol iq = *ps;              // <src>

  Size nd = array_num_dims(iq);
  Size* dims = array_dims(iq);

  Size coords_per_point;
  if (internalMode & 1)         // /one: <src> is interpreted as one-dimensional
    coords_per_point = 1;
  else              // <src>(*,0,0,...) are the coordinates of the first element
    coords_per_point = nd > 1? dims[0]: 1;
  Size points_count = array_size(iq)/coords_per_point;

  auto src = static_cast<const T*>(array_data(iq));

  T tolerance;
  if (narg >= 4 && ps[3])
    tolerance = get_scalar_value<T>(ps[3]);
  else
    tolerance = 0;
  MultiCompare<T> mc{src, coords_per_point, tolerance };

  auto order = std::vector<Size>(points_count);
  std::iota(&order[0], &order[points_count], 0);
  std::sort(&order[0], &order[points_count], mc);
  // now <src>(*,order[i]) are in ascending order (defined by mc)

  // count number of unique members
  Size n = 1;
  for (Size i = 0; i < points_count - 1; ++i)
    if (!mc.equals(order[i], order[i + 1]))
      n++;

  // create symbols for <index> and <list>
  if (coords_per_point > 1)
  {
    redef_array(ps[2], LUX_INT32, nd - 1, dims + 1); // <index>

    Size outdims[2];
    outdims[0] = coords_per_point;
    outdims[1] = n;
    redef_array(ps[1], lux_symboltype_for_type<T>, 2, outdims); // <list>
  }
  else
  {
    redef_array(ps[2], LUX_INT32, nd, dims);               // <index>
    redef_array(ps[1], lux_symboltype_for_type<T>, 1, &n); // <list>
  }
  auto index = static_cast<int32_t*>(array_data(ps[2]));
  auto list = static_cast<T*>(array_data(ps[1]));

  // the first one is always unique
  memcpy(list, src + order[0]*coords_per_point, coords_per_point*sizeof(T));
  Size curIndex;
  index[order[0]] = curIndex = 0;
  for (Size i = 1; i < points_count; i++)
  {
    if (!mc.equals(order[i - 1], order[i])) // unequal
    {
      index[order[i]] = ++curIndex;
      memcpy(list + curIndex*coords_per_point,
             src + order[i]*coords_per_point, coords_per_point*sizeof(T));
    } else
      index[order[i]] = curIndex;
  }
  return 1;
}

Symbol
lux_tolookup(ArgumentCount narg, Symbol ps[])
// tolookup,src,list,index,tolerance rewrites <src> as <list(index)>, with
// <list> a list of unique members of <src> (sorted in ascending order) and
// <index> ranging between 0 and num_elem(list) - 1.  <tolerance> is the
// tolerance for detecting uniqueness.  If <src> is multidimensional, then its
// last dimension is taken to run along a vector and <list> will be a list of
// vectors.  Keyword /one signals treatment of <src> as a big 1D vector.  LS
// 9nov95 7may96 16oct97
{
  if (symbol_class(*ps) != LUX_ARRAY)
    return cerror(NEED_ARR, *ps);
  switch (array_type(*ps))
  {
    case LUX_INT8:
      return tolookup_action<uint8_t>(narg, ps);
    case LUX_INT16:
      return tolookup_action<int16_t>(narg, ps);
    case LUX_INT32:
      return tolookup_action<int32_t>(narg, ps);
    case LUX_INT64:
      return tolookup_action<int64_t>(narg, ps);
    case LUX_FLOAT:
      return tolookup_action<float>(narg, ps);
    case LUX_DOUBLE:
      return tolookup_action<double>(narg, ps);
  }
  return LUX_ONE;
}
REGISTER(tolookup, s, tolookup, 2, 4, "1one");
//---------------------------------------------------------
Pointer         src;
Symboltype type;
int32_t mcmp(const void *x1, const void *x2)
// auxilliary for lux_medianfilter
{
  extern Pointer        src;
  Scalar        d1, d2;

  switch (type)
  { case LUX_INT8:
      d1.ui8 = src.ui8[*(int32_t *) x1];
      d2.ui8 = src.ui8[*(int32_t *) x2];
      return d1.ui8 < d2.ui8? -1: (d1.ui8 > d2.ui8? 1: 0);
    case LUX_INT16:
      d1.i16 = src.i16[*(int32_t *) x1];
      d2.i16 = src.i16[*(int32_t *) x2];
      return d1.i16 < d2.i16? -1: (d1.i16 > d2.i16? 1: 0);
    case LUX_INT32:
      d1.i32 = src.i32[*(int32_t *) x1];
      d2.i32 = src.i32[*(int32_t *) x2];
      return d1.i32 < d2.i32? -1: (d1.i32 > d2.i32? 1: 0);
    case LUX_INT64:
      d1.i64 = src.i64[*(int32_t *) x1];
      d2.i64 = src.i64[*(int32_t *) x2];
      return d1.i64 < d2.i64? -1: (d1.i64 > d2.i64? 1: 0);
    case LUX_FLOAT:
      d1.f = src.f[*(int32_t *) x1];
      d2.f = src.f[*(int32_t *) x2];
      return d1.f < d2.f? -1: (d1.f > d2.f? 1: 0);
    case LUX_DOUBLE:
      d1.d = src.d[*(int32_t *) x1];
      d2.d = src.d[*(int32_t *) x2];
      return d1.d < d2.d? -1: (d1.d > d2.d? 1: 0); }
  return 1;
}
//---------------------------------------------------------
int32_t cmp(const void *x1, const void *x2)
{
  switch (type) {
    case LUX_INT8:
      return *(uint8_t *) x1 < *(uint8_t *) x2? -1:
        (*(uint8_t *) x1 > *(uint8_t *) x2? 1: 0);
    case LUX_INT16:
      return *(int16_t *) x1 < *(int16_t *) x2? -1:
        (*(int16_t *) x1 > *(int16_t *) x2? 1: 0);
    case LUX_INT32:
      return *(int32_t *) x1 < *(int32_t *) x2? -1:
        (*(int32_t *) x1 > *(int32_t *) x2? 1: 0);
    case LUX_INT64:
      return *(int64_t *) x1 < *(int64_t *) x2? -1:
        (*(int64_t *) x1 > *(int64_t *) x2? 1: 0);
    case LUX_FLOAT:
      return *(float *) x1 < *(float *) x2? -1:
        (*(float *) x1 > *(float *) x2? 1: 0);
    case LUX_DOUBLE:
      return *(double *) x1 < *(double *) x2? -1:
        (*(double *) x1 > *(double *) x2? 1: 0);
  }
  return 1;                     // or some compilers complain
}
//---------------------------------------------------------
int32_t lux_orderfilter(ArgumentCount narg, Symbol ps[])
// Applies an ordinal filter to a data set
/* syntax:  Y = ORDFILTER([ORDER=ORDER,] X [[,AXES],WIDTH,
                /MEDIAN,/MINIMUM,/MAXIMUM]) */
// LS 23oct98
{
  int32_t       output, width, w, med, nelem, range[2*MAX_DIMS], *index,
    *offset, i, j;
  float         order;
  LoopInfo      srcinfo, trgtinfo, tmpinfo;
  // we use a global pointer src
  Pointer       trgt, tmpsrc;

  if (!ps[1])                   // no <data>
    return luxerror("Need data array", 0);

  if (narg > 2) {               // have <width>
    width = int_arg((narg == 4)? ps[3]: ps[2]);
    if (width < 3)
      return ps[1];             // nothing to do
  } else
    width = 3;
  w = (width - 1)/2;
  width = 2*w + 1;              // ensure that <width> is odd

  if (ps[0]) {                  // have <order>
    order = float_arg(ps[0]);
    if (order < 0.0 || order > 1.0)
      return luxerror("Order fraction must be between 0 and 1, was %f",
                   ps[0], order);
  } else
    order = -1;                         // flag that we didn't have one

  if (standardLoop(ps[1], (narg == 4)? ps[2]: 0,
                   (narg != 4? SL_ALLAXES: 0) | SL_UNIQUEAXES
                   | SL_AXESBLOCK | SL_KEEPTYPE, LUX_INT8,
                   &srcinfo, &src, &output, &trgtinfo, &trgt) == LUX_ERROR)
    return LUX_ERROR;

  // we calculate the number of elements that go into each sorting
  nelem = 1;
  for (i = 0; i < srcinfo.naxes; i++)
    nelem *= width;
  if (order != -1)              // we had an explicit <order>
    med = (int32_t) (order*(nelem - 0.0001)); /* the index in the sorted
                                           data values of the value to be
                                           returned. */
  else switch (internalMode) {
    case 0: case 1:             // /MEDIAN or default
      med = (nelem - 1)/2;
      break;
    case 2:
      med = 0;
      break;
    case 3:
      med = nelem - 1;
      break;
    default:
      return luxerror("Illegal keyword combination", 0);
  }

  // we do not treat data that is on an edge.  Rather than checking
  // for each data point if it is on an edge, we do a plain copy of
  // the data and then treat just the interior points.
  memcpy(trgt.ui8, src.ui8, srcinfo.stride*srcinfo.nelem);

  // we calculate the offsets of the elements to be considered
  for (i = 0; i < srcinfo.naxes; i++)
    range[i] = width;
  tmpsrc = src;                         // just to be safe
  tmpinfo.setupDimensionLoop(srcinfo.naxes, range, LUX_INT32, srcinfo.naxes,
                             NULL, &tmpsrc, SL_EACHCOORD);

  index = (int32_t*) malloc(nelem*sizeof(int32_t));
  offset = (int32_t*) malloc(nelem*sizeof(int32_t));
  if (!index || !offset)
    return cerror(ALLOC_ERR, 0);

  i = 0;
  do {
    offset[i] = 0;
    for (j = 0; j < srcinfo.naxes; j++)
      offset[i] += srcinfo.rsinglestep[j]*(tmpinfo.coords[j] - w);
    i++;
  } while (tmpinfo.advanceLoop(&tmpsrc.ui8) < tmpinfo.ndim);

  /* now we arrange to select only the interior area.
     we start by specifying ranges comprising the whole data set */
  for (i = 0; i < srcinfo.ndim; i++) {
    range[2*i] = 0;
    range[2*i + 1] = srcinfo.dims[i] - 1;
  }
  // and then we modify those specified in <axis>
  for (i = 0; i < srcinfo.naxes; i++) {
    range[2*srcinfo.axes[i]] = w;
    range[2*srcinfo.axes[i] + 1] -= w;
  }
  // and then we modify the loops to reflect the reduced ranges
  srcinfo.subdataLoop(range);
  trgtinfo.subdataLoop(range);

  type = srcinfo.type;          // so mcmp() knows what type it is

  // now we do the actual filtering
  do {
    memcpy(index, offset, nelem*sizeof(int32_t));
    qsort(index, nelem, sizeof(*index), mcmp);
    switch (type) {
      case LUX_INT8:
        *trgt.ui8 = src.ui8[index[med]];
        break;
      case LUX_INT16:
        *trgt.i16 = src.i16[index[med]];
        break;
      case LUX_INT32:
        *trgt.i32 = src.i32[index[med]];
        break;
      case LUX_INT64:
        *trgt.i64 = src.i64[index[med]];
        break;
      case LUX_FLOAT:
        *trgt.f = src.f[index[med]];
        break;
      case LUX_DOUBLE:
        *trgt.d = src.d[index[med]];
        break;
    }
  } while (trgtinfo.advanceLoop(&trgt.ui8),
           srcinfo.advanceLoop(&src.ui8) < srcinfo.ndim);

  free(index);
  free(offset);
  return output;
}
//---------------------------------------------------------
int32_t lux_medfilter(ArgumentCount narg, Symbol ps[])
     // median filter
{
  internalMode = 1;
  return lux_orderfilter(narg, ps);
}
//---------------------------------------------------------
int32_t lux_quantile(ArgumentCount narg, Symbol ps[])
// QUANTILE([<order> ,] <data> [, <axes>])
{
  int32_t       output, t, nelem, i;
  double        order, f;
  LoopInfo      srcinfo, trgtinfo;
  // we use a global pointer src
  Pointer       trgt, tmp, tmp0;

  if (!ps[1])                   // no <data>
    return luxerror("Need data array", 0);

  if (ps[0]) {                  // have <order>
    order = double_arg(ps[0]);
    if (order < 0.0 || order > 1.0)
      return luxerror("Order fraction must be between 0 and 1, was %g",
                   ps[0], order);
  } else
    order = -1;                         // flag that we didn't have one

  if (standardLoop(ps[1], (narg > 2)? ps[2]: 0,
                   (narg <= 2? SL_ALLAXES: 0) | SL_UNIQUEAXES | SL_AXESBLOCK
                   | SL_KEEPTYPE | SL_COMPRESSALL, LUX_INT8, &srcinfo,
                   &src, &output, &trgtinfo, &trgt) == LUX_ERROR)
    return LUX_ERROR;

  // we calculate the number of elements that go into each sorting
  nelem = 1;
  for (i = 0; i < srcinfo.naxes; i++)
    nelem *= srcinfo.rdims[i];
  if (order == -1) {
    switch (internalMode) {
    case 0: case 1:             // /MEDIAN or default
      order = 0.5;
      break;
    case 2:                     // min
      order = 0;
      break;
    case 3:                     // max
      order = 1;
      break;
    default:
      return luxerror("Illegal keyword combination", 0);
    }
  }
  {
    double target = order*(nelem - 1);
    t = floor(target);
    f = target - t;
  }

  type = srcinfo.type;          // so mcmp() knows what type it is

  tmp0.ui8 = (uint8_t*) malloc(nelem*srcinfo.stride);
  if (!tmp0.ui8)
    return cerror(ALLOC_ERR, 0);

  // now we do the actual filtering
  do {
    tmp.ui8 = tmp0.ui8;
    do {
      memcpy(tmp.ui8, src.ui8, srcinfo.stride);
      tmp.ui8 += srcinfo.stride;
    } while (srcinfo.advanceLoop(&src.ui8) < srcinfo.naxes);
    qsort(tmp0.ui8, nelem, srcinfo.stride, cmp);
    switch (type) {
      case LUX_INT8:
        *trgt.ui8 = tmp0.ui8[t];
        if (f) {
          // the quantile doesn't correspond to a specific element; interpolate
          // linearly between the next lower and higher elements
          *trgt.ui8 +=
            static_cast<uint8_t>((tmp0.ui8[t + 1] - tmp0.ui8[t])*f);
        }
        break;
      case LUX_INT16:
        *trgt.i16 = tmp0.i16[t];
        if (f) {
          *trgt.i16 +=
            static_cast<int16_t>((tmp0.i16[t + 1] - tmp0.i16[t])*f);
        }
        break;
      case LUX_INT32:
        *trgt.i32 = tmp0.i32[t];
        if (f) {
          *trgt.i32 +=
            static_cast<int32_t>((tmp0.i32[t + 1] - tmp0.i32[t])*f);
        }
        break;
      case LUX_INT64:
        *trgt.i64 = tmp0.i64[t];
        if (f) {
          *trgt.i64 +=
            static_cast<int64_t>((tmp0.i64[t + 1] - tmp0.i64[t])*f);
        }
        break;
      case LUX_FLOAT:
        *trgt.f = tmp0.f[t];
        if (f) {
          *trgt.f +=
            static_cast<float>((tmp0.f[t + 1] - tmp0.f[t])*f);
        }
        break;
      case LUX_DOUBLE:
        *trgt.d = tmp0.d[t];
        if (f) {
          *trgt.d += (tmp0.d[t + 1] - tmp0.d[t])*f;
        }
        break;
    }
  } while (trgtinfo.advanceLoop(&trgt.ui8) < trgtinfo.rndim);

  free(tmp0.ui8);
  return output;
}
//---------------------------------------------------------
int32_t lux_median(ArgumentCount narg, Symbol ps[])
     // median filter
{
  internalMode = 1;
  return lux_quantile(narg, ps);
}
//---------------------------------------------------------
int32_t lux_minfilter(ArgumentCount narg, Symbol ps[])
     // minimum filter
// Applies a running minimum filter to a data set
// syntax:  Y = MINFILTER(X [[,AXIS], WIDTH])
// LS 31dec95 30jul97
{
  Symboltype    type;
  int32_t       n, result, i, offset1, offset2, stride,         w1, j, ww, loop, iq, three=3,
    nWidth;
  Pointer       src, trgt, width;
  Scalar        value;
  LoopInfo      srcinfo, trgtinfo;

  if (standardLoop(ps[0], narg > 2? ps[1]: 0,
                   SL_SAMEDIMS | SL_UPGRADE | SL_EACHROW, LUX_INT8,
                   &srcinfo, &src, &result, &trgtinfo, &trgt) < 0)
    return LUX_ERROR;
  if (narg > 1) {               // <width>
    // check that it is numerical and ensure that it is LUX_INT32
    if (getNumerical(ps[narg - 1], LUX_INT32, &nWidth, &width,
                     GN_UPDATE | GN_UPGRADE, NULL, NULL) < 0)
      return LUX_ERROR;
    if (nWidth > 1 && nWidth != srcinfo.naxes)
      return luxerror("Number of widths must be 1 or equal to number of axes",
                   ps[2]);
    for (i = 0; i < nWidth; i++)
      if (width.i32[i] < 1)
        return luxerror("Width(s) must be positive", ps[2]);
  } else {
    width.i32 = &three;
    nWidth = 1;
  }
  iq = ps[0];                   // data
  type = symbol_type(iq);
  if (!srcinfo.naxes)
    srcinfo.naxes++;
  for (loop = 0; loop < srcinfo.naxes; loop++) {
    ww = (*width.i32 > srcinfo.rdims[0])? srcinfo.rdims[0]: *width.i32;
    if (nWidth > 1)
      width.i32++;
    stride = srcinfo.step[0];
    offset2 = -stride*ww;
    offset1 = offset2 + stride;
    w1 = (ww + 1)/2;
    switch (type) {
      default:
        return cerror(ILL_TYPE, ps[0]);
      case LUX_INT8:
        do {
          value.ui8 = bounds.max.ui8;       // initialize
          for (i = 0; i < ww; i++) { // do the left edge
            if (*src.ui8 < value.ui8)
              value.ui8 = *src.ui8;
            src.ui8 += stride;
          }
          for (i = 0; i < w1; i++) {
            *trgt.ui8 = value.ui8;
            trgt.ui8 += stride;
          }
          for (i = ww; i < srcinfo.rdims[0]; i++) { // middle part
            if (*src.ui8 < value.ui8) {
              value.ui8 = *src.ui8;
              src.ui8 += stride;
            } else if (src.ui8[offset2] == value.ui8) {
              src.ui8 += offset1;
              value.ui8 = *src.ui8;
              src.ui8 += stride;
              for (j = 1; j < ww; j++) {
                if (*src.ui8 < value.ui8)
                value.ui8 = *src.ui8;
                src.ui8 += stride;
              }
            } else
              src.ui8 += stride;
            *trgt.ui8 = value.ui8;
            trgt.ui8 += stride;
          }
          for (i = w1; i < ww; i++) { // right edge
            *trgt.ui8 = value.ui8;
            trgt.ui8 += stride;
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
      case LUX_INT16:
        do {
          value.i16 = bounds.max.i16;       // initialize
          for (i = 0; i < ww; i++) { // do the left edge
            if (*src.i16 < value.i16)
              value.i16 = *src.i16;
            src.i16 += stride;
          }
          for (i = 0; i < w1; i++) {
            *trgt.i16 = value.i16;
            trgt.i16 += stride;
          }
          for (i = ww; i < srcinfo.rdims[0]; i++) { // middle part
            if (*src.i16 < value.i16) {
              value.i16 = *src.i16;
              src.i16 += stride;
            } else if (src.i16[offset2] == value.i16) {
              src.i16 += offset1;
              value.i16 = *src.i16;
              src.i16 += stride;
              for (j = 1; j < ww; j++) {
                if (*src.i16 < value.i16)
                value.i16 = *src.i16;
                src.i16 += stride;
              }
            } else
              src.i16 += stride;
            *trgt.i16 = value.i16;
            trgt.i16 += stride;
          }
          for (i = w1; i < ww; i++) { // right edge
            *trgt.i16 = value.i16;
            trgt.i16 += stride;
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
      case LUX_INT32:
        do {
          value.i32 = bounds.max.i32;       // initialize
          for (i = 0; i < ww; i++) { // do the left edge
            if (*src.i32 < value.i32)
              value.i32 = *src.i32;
            src.i32 += stride;
          }
          for (i = 0; i < w1; i++) {
            *trgt.i32 = value.i32;
            trgt.i32 += stride;
          }
          for (i = ww; i < srcinfo.rdims[0]; i++) { // middle part
            if (*src.i32 < value.i32) {
              value.i32 = *src.i32;
              src.i32 += stride;
            } else if (src.i32[offset2] == value.i32) {
              src.i32 += offset1;
              value.i32 = *src.i32;
              src.i32 += stride;
              for (j = 1; j < ww; j++) {
                if (*src.i32 < value.i32)
                value.i32 = *src.i32;
                src.i32 += stride;
              }
            } else
              src.i32 += stride;
            *trgt.i32 = value.i32;
            trgt.i32 += stride;
          }
          for (i = w1; i < ww; i++) { // right edge
            *trgt.i32 = value.i32;
            trgt.i32 += stride;
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
      case LUX_INT64:
        do {
          value.i64 = bounds.max.i64;       // initialize
          for (i = 0; i < ww; i++) { // do the left edge
            if (*src.i64 < value.i64)
              value.i64 = *src.i64;
            src.i64 += stride;
          }
          for (i = 0; i < w1; i++) {
            *trgt.i64 = value.i64;
            trgt.i64 += stride;
          }
          for (i = ww; i < srcinfo.rdims[0]; i++) { // middle part
            if (*src.i64 < value.i64) {
              value.i64 = *src.i64;
              src.i64 += stride;
            } else if (src.i64[offset2] == value.i64) {
              src.i64 += offset1;
              value.i64 = *src.i64;
              src.i64 += stride;
              for (j = 1; j < ww; j++) {
                if (*src.i64 < value.i64)
                value.i64 = *src.i64;
                src.i64 += stride;
              }
            } else
              src.i64 += stride;
            *trgt.i64 = value.i64;
            trgt.i64 += stride;
          }
          for (i = w1; i < ww; i++) { // right edge
            *trgt.i64 = value.i64;
            trgt.i64 += stride;
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
      case LUX_FLOAT:
        do {
          value.f = bounds.max.f;       // initialize
          for (i = 0; i < ww; i++) { // do the left edge
            if (*src.f < value.f)
              value.f = *src.f;
            src.f += stride;
          }
          for (i = 0; i < w1; i++) {
            *trgt.f = value.f;
            trgt.f += stride;
          }
          for (i = ww; i < srcinfo.rdims[0]; i++) { // middle part
            if (*src.f < value.f) {
              value.f = *src.f;
              src.f += stride;
            } else if (src.f[offset2] == value.f) {
              src.f += offset1;
              value.f = *src.f;
              src.f += stride;
              for (j = 1; j < ww; j++) {
                if (*src.f < value.f)
                value.f = *src.f;
                src.f += stride;
              }
            } else
              src.f += stride;
            *trgt.f = value.f;
            trgt.f += stride;
          }
          for (i = w1; i < ww; i++) { // right edge
            *trgt.f = value.f;
            trgt.f += stride;
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
      case LUX_DOUBLE:
        do {
          value.d = bounds.max.d;       // initialize
          for (i = 0; i < ww; i++) { // do the left edge
            if (*src.d < value.d)
              value.d = *src.d;
            src.d += stride;
          }
          for (i = 0; i < w1; i++) {
            *trgt.d = value.d;
            trgt.d += stride;
          }
          for (i = ww; i < srcinfo.rdims[0]; i++) { // middle part
            if (*src.d < value.d) {
              value.d = *src.d;
              src.d += stride;
            } else if (src.d[offset2] == value.d) {
              src.d += offset1;
              value.d = *src.d;
              src.d += stride;
              for (j = 1; j < ww; j++) {
                if (*src.d < value.d)
                value.d = *src.d;
                src.d += stride;
              }
            } else
              src.d += stride;
            *trgt.d = value.d;
            trgt.d += stride;
          }
          for (i = w1; i < ww; i++) { // right edge
            *trgt.d = value.d;
            trgt.d += stride;
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
    }
    if (loop < srcinfo.naxes - 1) {
      nextLoops(&srcinfo, &trgtinfo);
      if (isFreeTemp(iq)) {     // can use iq to hold next result
        n = result;
        result = iq;
        iq = n;
      } else {                  // need new temp for next result
        iq = result;
        result = array_clone(iq, type);
      }
      src.ui8 = (uint8_t*) array_data(iq);
      trgt.ui8 = (uint8_t*) array_data(result);
    }
  }
  return result;
}
//---------------------------------------------------------
int32_t lux_maxfilter(ArgumentCount narg, Symbol ps[])
// maximum filter
// Applies a running maximum filter to a data set
// syntax:  Y = MAXFILTER(X [[,AXIS], WIDTH])
// LS 31dec95 30jul97
{
  Symboltype    type;
  int32_t n, result, i, offset1, offset2, stride, w1, j, ww, loop,
    iq, three=3, nWidth;
  Pointer       src, trgt, width;
  Scalar        value;
  LoopInfo      srcinfo, trgtinfo;

  if (standardLoop(ps[0], narg > 2? ps[1]: 0,
                   SL_SAMEDIMS | SL_UPGRADE | SL_EACHROW, LUX_INT8,
                   &srcinfo, &src, &result, &trgtinfo, &trgt) < 0)
    return LUX_ERROR;
  if (narg > 1) {               // <width>
    // check that it is numerical and ensure that it is LUX_INT32
    if (getNumerical(ps[narg - 1], LUX_INT32, &nWidth, &width,
                     GN_UPGRADE | GN_UPDATE, NULL, NULL) < 0)
      return LUX_ERROR;
    if (nWidth > 1 && nWidth != srcinfo.naxes)
      return luxerror("Number of widths must be 1 or equal to number of axes",
                   ps[2]);
    for (i = 0; i < nWidth; i++)
      if (width.i32[i] < 1)
        return luxerror("Width(s) must be positive", ps[2]);
  } else {
    width.i32 = &three;
    nWidth = 1;
  }
  iq = ps[0];                   // data
  type = symbol_type(iq);
  if (!srcinfo.naxes)
    srcinfo.naxes++;
  for (loop = 0; loop < srcinfo.naxes; loop++) {
    ww = (*width.i32 > srcinfo.rdims[0])? srcinfo.rdims[0]: *width.i32;
    if (nWidth > 1)
      width.i32++;
    stride = srcinfo.step[0];
    offset2 = -stride*ww;
    offset1 = offset2 + stride;
    w1 = (ww + 1)/2;
    switch (type) {
      default:
        return cerror(ILL_TYPE, ps[0]);
      case LUX_INT8:
        do {
          value.ui8 = bounds.min.ui8;       // initialize
          for (i = 0; i < ww; i++) { // do the left edge
            if (*src.ui8 > value.ui8)
              value.ui8 = *src.ui8;
            src.ui8 += stride;
          }
          for (i = 0; i < w1; i++) {
            *trgt.ui8 = value.ui8;
            trgt.ui8 += stride;
          }
          for (i = ww; i < srcinfo.rdims[0]; i++) { // middle part
            if (*src.ui8 > value.ui8) {
              value.ui8 = *src.ui8;
              src.ui8 += stride;
            } else if (src.ui8[offset2] == value.ui8) {
              src.ui8 += offset1;
              value.ui8 = *src.ui8;
              src.ui8 += stride;
              for (j = 1; j < ww; j++) {
                if (*src.ui8 > value.ui8)
                value.ui8 = *src.ui8;
                src.ui8 += stride;
              }
            } else
              src.ui8 += stride;
            *trgt.ui8 = value.ui8;
            trgt.ui8 += stride;
          }
          for (i = w1; i < ww; i++) { // right edge
            *trgt.ui8 = value.ui8;
            trgt.ui8 += stride;
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
      case LUX_INT16:
        do {
          value.i16 = bounds.min.i16;       // initialize
          for (i = 0; i < ww; i++) { // do the left edge
            if (*src.i16 > value.i16)
              value.i16 = *src.i16;
            src.i16 += stride;
          }
          for (i = 0; i < w1; i++) {
            *trgt.i16 = value.i16;
            trgt.i16 += stride;
          }
          for (i = ww; i < srcinfo.rdims[0]; i++) { // middle part
            if (*src.i16 > value.i16) {
              value.i16 = *src.i16;
              src.i16 += stride;
            } else if (src.i16[offset2] == value.i16) {
              src.i16 += offset1;
              value.i16 = *src.i16;
              src.i16 += stride;
              for (j = 1; j < ww; j++) {
                if (*src.i16 > value.i16)
                value.i16 = *src.i16;
                src.i16 += stride;
              }
            } else
              src.i16 += stride;
            *trgt.i16 = value.i16;
            trgt.i16 += stride;
          }
          for (i = w1; i < ww; i++) { // right edge
            *trgt.i16 = value.i16;
            trgt.i16 += stride;
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
      case LUX_INT32:
        do {
          value.i32 = bounds.min.i32;       // initialize
          for (i = 0; i < ww; i++) { // do the left edge
            if (*src.i32 > value.i32)
              value.i32 = *src.i32;
            src.i32 += stride;
          }
          for (i = 0; i < w1; i++) {
            *trgt.i32 = value.i32;
            trgt.i32 += stride;
          }
          for (i = ww; i < srcinfo.rdims[0]; i++) { // middle part
            if (*src.i32 > value.i32) {
              value.i32 = *src.i32;
              src.i32 += stride;
            } else if (src.i32[offset2] == value.i32) {
              src.i32 += offset1;
              value.i32 = *src.i32;
              src.i32 += stride;
              for (j = 1; j < ww; j++) {
                if (*src.i32 > value.i32)
                value.i32 = *src.i32;
                src.i32 += stride;
              }
            } else
              src.i32 += stride;
            *trgt.i32 = value.i32;
            trgt.i32 += stride;
          }
          for (i = w1; i < ww; i++) { // right edge
            *trgt.i32 = value.i32;
            trgt.i32 += stride;
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
      case LUX_INT64:
        do {
          value.i64 = bounds.min.i64;       // initialize
          for (i = 0; i < ww; i++) { // do the left edge
            if (*src.i64 > value.i64)
              value.i64 = *src.i64;
            src.i64 += stride;
          }
          for (i = 0; i < w1; i++) {
            *trgt.i64 = value.i64;
            trgt.i64 += stride;
          }
          for (i = ww; i < srcinfo.rdims[0]; i++) { // middle part
            if (*src.i64 > value.i64) {
              value.i64 = *src.i64;
              src.i64 += stride;
            } else if (src.i64[offset2] == value.i64) {
              src.i64 += offset1;
              value.i64 = *src.i64;
              src.i64 += stride;
              for (j = 1; j < ww; j++) {
                if (*src.i64 > value.i64)
                value.i64 = *src.i64;
                src.i64 += stride;
              }
            } else
              src.i64 += stride;
            *trgt.i64 = value.i64;
            trgt.i64 += stride;
          }
          for (i = w1; i < ww; i++) { // right edge
            *trgt.i64 = value.i64;
            trgt.i64 += stride;
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
      case LUX_FLOAT:
        do {
          value.f = bounds.min.f;       // initialize
          for (i = 0; i < ww; i++) { // do the left edge
            if (*src.f > value.f)
              value.f = *src.f;
            src.f += stride;
          }
          for (i = 0; i < w1; i++) {
            *trgt.f = value.f;
            trgt.f += stride;
          }
          for (i = ww; i < srcinfo.rdims[0]; i++) { // middle part
            if (*src.f > value.f) {
              value.f = *src.f;
              src.f += stride;
            } else if (src.f[offset2] == value.f) {
              src.f += offset1;
              value.f = *src.f;
              src.f += stride;
              for (j = 1; j < ww; j++) {
                if (*src.f > value.f)
                value.f = *src.f;
                src.f += stride;
              }
            } else
              src.f += stride;
            *trgt.f = value.f;
            trgt.f += stride;
          }
          for (i = w1; i < ww; i++) { // right edge
            *trgt.f = value.f;
            trgt.f += stride;
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
      case LUX_DOUBLE:
        do {
          value.d = bounds.min.d;       // initialize
          for (i = 0; i < ww; i++) { // do the left edge
            if (*src.d > value.d)
              value.d = *src.d;
            src.d += stride;
          }
          for (i = 0; i < w1; i++) {
            *trgt.d = value.d;
            trgt.d += stride;
          }
          for (i = ww; i < srcinfo.rdims[0]; i++) { // middle part
            if (*src.d > value.d) {
              value.d = *src.d;
              src.d += stride;
            } else if (src.d[offset2] == value.d) {
              src.d += offset1;
              value.d = *src.d;
              src.d += stride;
              for (j = 1; j < ww; j++) {
                if (*src.d > value.d)
                value.d = *src.d;
                src.d += stride;
              }
            } else
              src.d += stride;
            *trgt.d = value.d;
            trgt.d += stride;
          }
          for (i = w1; i < ww; i++) { // right edge
            *trgt.d = value.d;
            trgt.d += stride;
          }
        } while (trgtinfo.advanceLoop(&trgt.ui8),
                 srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
        break;
    }
    if (loop < srcinfo.naxes - 1) {
      nextLoops(&srcinfo, &trgtinfo);
      if (isFreeTemp(iq)) {     // can use iq to hold next result
        n = result;
        result = iq;
        iq = n;
      } else {                  // need new temp for next result
        iq = result;
        result = array_clone(iq, type);
      }
      src.ui8 = (uint8_t*) array_data(iq);
      trgt.ui8 = (uint8_t*) array_data(result);
    }
  }
  return result;
}
//---------------------------------------------------------
int32_t lux_distarr(ArgumentCount narg, Symbol ps[])
// DISTARR(dims[,center,stretch]) returns a FLOAT array of dimensions <dims>
// wherein each element contains the distance of the coordinates of
// that element to the point given in <center>, after stretching each
// coordinate by the appropriate factor from <stretch>.  <dims>, <center>,
// and <stretch> must have the same number of elements.  LS 9jan97 22may97
{
  int32_t       iq, *dims, ndim, i, result, done;
  float         *center, temp, temptot, zerocenter[MAX_DIMS], *stretch,
        onestretch[MAX_DIMS];
  Pointer       trgt;
  LoopInfo      trgtinfo;

  iq = lux_long(1, &ps[0]);     // DIMS
  switch (symbol_class(iq))
  { case LUX_ARRAY:
      dims = (int32_t *) array_data(iq);
      ndim = array_size(iq);
      break;
    case LUX_SCALAR:
      dims = &scalar_value(iq).i32;
      ndim = 1;
      break;
    default:
      return cerror(ILL_CLASS, ps[0]); }
  if (narg > 1)
  { iq = lux_float(1, &ps[1]);  // CENTER
    switch (symbol_class(iq))
    { case LUX_ARRAY:
        if (array_size(iq) != ndim)
          return cerror(INCMP_ARG, ps[1]);
        center = (float *) array_data(iq);
        break;
      case LUX_SCALAR:
        if (ndim != 1)
          return cerror(INCMP_ARG, ps[1]);
        center = &scalar_value(iq).f;
        break;
      default:
        return cerror(ILL_CLASS, ps[1]); }
  } else
  { zerobytes(zerocenter, ndim*sizeof(float));
    center = zerocenter; }
  if (narg > 2)
  { iq = lux_float(1, &ps[2]);  // STRETCH
    switch (symbol_class(iq))
    { case LUX_ARRAY:
        if (array_size(iq) != ndim)
          return cerror(INCMP_ARG, ps[2]);
        stretch = (float *) array_data(iq);
        break;
      case LUX_SCALAR:
        if (ndim != 1)
          return cerror(INCMP_ARG, ps[2]);
        stretch = &scalar_value(iq).f;
        break;
      default:
        return cerror(ILL_CLASS, ps[2]); }
  }
  else
  { for (i = 0; i < ndim; i++)
      onestretch[i] = 1.0;
    stretch = onestretch; }

                                // check output dimensions
  for (i = 0; i < ndim; i++)
    if (dims[i] <= 0)
      return cerror(ILL_DIM, ps[0]);
  result = array_scratch(LUX_FLOAT, ndim, dims);
  if (result == LUX_ERROR)      // some error
    return LUX_ERROR;
  trgt.f = (float *) array_data(result);
  trgtinfo.setupDimensionLoop(ndim, dims, LUX_FLOAT, ndim, NULL, &trgt,
                              SL_EACHCOORD);
  // initialize walk through array
  temptot = 0.0;
  for (i = 1; i < ndim; i++)
  { temp = center[i]*stretch[i];
    temptot += temp*temp; }
  do
  { temp = (trgtinfo.coords[0] - center[0])*stretch[0];
    *trgt.f = temp*temp + temptot;
    *trgt.f = sqrt((double) *trgt.f); // distance
    done = trgtinfo.advanceLoop(&trgt.ui8);
    if (done && done < ndim)
    { temptot = 0.0;
      for (i = 1; i < ndim; i++)
      { temp = (trgtinfo.coords[i] - center[i])*stretch[i];
        temptot += temp*temp; }
    }
  } while (done < ndim);
  return result;
}
//---------------------------------------------------------
int32_t lux_multisieve(ArgumentCount narg, Symbol ps[])
/*  MULTISIEVE, x, y, list, index
    returns <list> and <index> such that
    <x(list(index(j-1):index(j)-1))> EQ <y(j)>
    for all j
    LS 14apr97 - 15apr97 */
{
  Symboltype    maxType;
  Pointer       xData, yData;
  int32_t       nx, ny, *listData, *indexData,
    n, *temp, nTemp, iq, nnTemp, noMatch, ix, i, match, *base, step,
    *reallocbase, nMatch, *number, j;

  if (getSimpleNumerical(ps[0], &xData, &nx) < 0)
    return -1;                  // some error occurred
  if (getSimpleNumerical(ps[1], &yData, &ny) < 0)
    return -1;

  // make arguments have same data type
  maxType = symbol_type(ps[0]);
  if (symbol_type(ps[1]) > maxType) { // need to upgrade ps[0]
    maxType = symbol_type(ps[1]);
    iq = lux_convert(1, ps, maxType, 1);
    getSimpleNumerical(iq, &yData, &ny);
  } else if (symbol_type(ps[1]) < maxType) { // upgrade ps[1]
    maxType = symbol_type(ps[0]);
    iq = lux_convert(1, ps + 1, maxType, 1);
    getSimpleNumerical(iq, &xData, &nx);
  }

  nTemp = 1000;                         // start with 1000 elements
  nnTemp = nTemp - 2;
  ALLOCATE(temp, nTemp, int32_t);
  base = temp;
  step = lux_type_size[maxType];

  n = ny + 1;
  if (redef_array(ps[3], LUX_INT32, 1, &n) < 0) // <indices>
    return -1;
  indexData = (int32_t*) array_data(ps[3]);
  indexData[0] = 0;
  indexData++;
  zerobytes((char *) indexData, ny*sizeof(int32_t));

  // get results
  // we maintain a list of matches as follows: the list consists of a
  // set of ranges of matches.  Each range is specified by first the
  // start index in <x>, then the number of matches that follow in the
  // current range, and then the matches for that range.  if fewer than
  // three no-matches follow the last match in the current range, then
  // the no-matches and the current match are entered in the current
  // range, otherwise a new range is started.  This scheme ensures that
  // the list gets at most two elements longer than <x> itself.
  // Example: if <x> = 1 2 3 7 1 6 7 5 7 5 and <y> = 1 5 7 then
  // the matches are as follows:
  //   x     1 2 3 3 7 1 6 7 5 7 5
  //   match 0 - - - 2 0 - 2 1 2 1
  // and the temporary match list comes out as follows:
  //   0 1 0 4 7 2 0 -1 2 1 2 1
  noMatch = 999;
  ix = nMatch = 0;
  *temp = 0;                    // no matches so far
  while (1)
  { while (nnTemp > 3 && ix < nx)
    { match = -1;               // default: no match
      switch (maxType)
      { case LUX_INT8:
          for (i = 0; i < ny; i++)
            if (*xData.ui8 == yData.ui8[i])
            { match = i;
              indexData[match]++; // count
              break; }
          break;
        case LUX_INT16:
          for (i = 0; i < ny; i++)
            if (*xData.i16 == yData.i16[i])
            { match = i;
              indexData[match]++; // count
              break; }
          break;
        case LUX_INT32:
          for (i = 0; i < ny; i++)
            if (*xData.i32 == yData.i32[i])
            { match = i;
              indexData[match]++; // count
              break; }
          break;
        case LUX_INT64:
          for (i = 0; i < ny; i++)
            if (*xData.i64 == yData.i64[i])
            { match = i;
              indexData[match]++; // count
              break; }
          break;
        case LUX_FLOAT:
          for (i = 0; i < ny; i++)
            if (*xData.f == yData.f[i])
            { match = i;
              indexData[match]++; // count
              break; }
          break;
        case LUX_DOUBLE:
          for (i = 0; i < ny; i++)
            if (*xData.d == yData.d[i])
            { match = i;
              indexData[match]++; // count
              break; }
          break;
        }
      if (match >= 0) {         // have a match
        nMatch++;
        switch (noMatch)
        { case 2:               // two no-matches before
            temp[++(*temp)] = -1;
            nnTemp--;
            // fall-thru
          case 1:               // one no-match before
            temp[++(*temp)] = -1;
            nnTemp--;
            noMatch = 0;
            // fall-thru
          case 0:               // previous one was a match, too
            temp[++(*temp)] = match;
            nnTemp--;
            break;
          default:              // three or more no-matches before:
                                // start a new range
            temp += *temp + 1;
            *temp++ = ix;
            *temp = 1;
            temp[*temp] = match;
            nnTemp -= 3;
            noMatch = 0;
            break; }
      }
      else ++noMatch;
      xData.ui8 += step;
      ix++; }
    if (ix != nx)               // more to do
    { nTemp += 1000;
      reallocbase = (int32_t*) Realloc(base, nTemp*sizeof(int32_t));
      if (reallocbase == NULL)
      { free(base);
        return cerror(ALLOC_ERR, 0); }
      temp = (temp - base) + reallocbase;
      base = reallocbase;
      nnTemp += 1000; }
    else break; }

  if (!nMatch)                  // no matches at all: return -1
  { undefine(ps[2]);
    symbol_class(ps[2]) = LUX_SCALAR;
    scalar_type(ps[2]) = LUX_INT32;
    scalar_value(ps[2]).i32 = -1; }
  else
  { if (redef_array(ps[2], LUX_INT32, 1, &nMatch) < 0) // <list>
      return -1;                // some error
    listData = (int32_t*) array_data(ps[2]);

    temp = indexData;
    n = 0;
    for (i = 0; i < ny; i++) {
      n += indexData[i];
      indexData[i] = n;
    }
    ix = indexData[ny - 1];

    number = (int32_t*) Malloc(ny*sizeof(int32_t));
    if (!number) {
      free(base);
      return cerror(ALLOC_ERR, 0);
    }
    zerobytes((char *) number, ny*sizeof(int32_t));

    temp = base + 1;
    while (ix) {
      step = *temp++;           // offset
      n = *temp++;              // number
      for (j = 0; j < n; j++) {
        if (*temp < 0)          // no match
        { temp++;
          continue; }
        ix--;
        i = *temp? indexData[*temp - 1]: 0;
        listData[number[*temp] + i] = step + j;
        number[*temp++]++;
      }
    }

    free(number);
  }
  free(base);
  return 1;
}
//---------------------------------------------------------
int32_t lux_temp(ArgumentCount narg, Symbol ps[])
// experimental routine: returns temp copy of symbol if it is
// a named symbol
{
  int32_t       result;

  if (*ps >= NAMED_START && *ps < NAMED_END)
  { result = nextFreeTempVariable();
    return copyToSym(result, *ps); }
  else return *ps;
}
//---------------------------------------------------------
int32_t shift(ArgumentCount narg, Symbol ps[], int32_t isFunction)
/* SHIFT(x[,axes],dist[,BLANK=blank][,/TRANSLATE]) shifts the data in
 <x> over distances <dist> (a scalar or an array containing one
 integer number per dimension) along the indicated <axes> (a scalar or
 an array).  If /TRANSLATE and/or <blank> are specified, then the
 shifting is non-circular, and then elements in the result for which
 no corresponding data was present in <x> is filled with the value of
 <blank>, or 0 by default.  If /TRANSLATE and <blank> are not
 specified, then the shifting is circular, so that elements that shift
 out of the field of view at one edge reappear at the opposite edge,
 and no elements are lost.
  LS 7may97 6sep2000 */
{
  int32_t       axesSym, offset, i, step, j, distSym, iq, *ptr;
  Pointer       src, trgt, shift, src0, trgt0, blank;
  char  *tmp, *tmp0 = NULL;
  LoopInfo      srcinfo, trgtinfo;
  int32_t       lux_indgen(int32_t, int32_t []);
  double        zero = 0.0;

  if (!symbolIsNumericalArray(ps[0]))
    return cerror(NEED_ARR, ps[0]);

  if (narg == 1 || (narg == 4 && !ps[1] && !ps[2])) {
    // SHIFT(x)
    // create <dist> = DIMEN(<x>)/2
    iq = array_num_dims(ps[0]);
    distSym = array_scratch(LUX_INT32, 1, &iq);
    ptr = (int32_t*) array_data(distSym);
    memcpy(ptr, array_dims(ps[0]), iq*sizeof(int32_t));
    for (i = 0; i < iq; i++) {
      *ptr = *ptr/2;
      ptr++;
    }
    axesSym = lux_indgen(1, &distSym); // create <axes> = INDGEN(<dist>)
  } else if (narg == 2 || (narg == 4 && !ps[2])) {
    // SHIFT(x,dist)
    axesSym = lux_indgen(1, ps + 1); // <axes> = INDGEN(<dist>)
    distSym = ps[1];            // <dist>
  } else if (narg > 2 && ps[2]) {
    // SHIFT(x,axes,dist)
    axesSym = ps[1];
    distSym = ps[2];
  }
  if (narg > 3 && ps[3]) {      // <blank>
    if (!symbolIsScalar(ps[3]))
      return cerror(NEED_SCAL, ps[3]);
    iq = lux_converts[array_type(ps[0])](1, ps + 3);
    blank.i32 = &scalar_value(iq).i32;
    internalMode |= 1;          // /TRANSLATE
  } else
    blank.d = &zero;

  if (isFunction) {
    if (standardLoop(ps[0], axesSym,
                     SL_UPGRADE | SL_UNIQUEAXES | SL_EACHROW | SL_AXISCOORD,
                     LUX_INT8, &srcinfo, &src, &iq, &trgtinfo, &trgt) < 0)
      return LUX_ERROR;
  } else {
    if (standardLoop(ps[0], axesSym,
                     SL_UPGRADE | SL_UNIQUEAXES | SL_EACHROW | SL_AXISCOORD,
                     LUX_INT8, &srcinfo, &src, NULL, NULL, NULL) < 0)
      return LUX_ERROR;
    trgtinfo = srcinfo;
    trgtinfo.data = &trgt;
    trgt = src;
  }
  src0 = src;
  trgt0 = trgt;
  if (numerical(distSym, NULL, NULL, &i, NULL) < 0) // distance
    return LUX_ERROR;
  if (i != srcinfo.naxes)       // must have one distance for each axis
    return cerror(INCMP_ARG, distSym);
  i = lux_long(1, &distSym);    // ensure LONG
  numerical(i, NULL, NULL, NULL, &shift);
  if (internalMode & 1) {       // non-circular shift
    for (i = 0; i < srcinfo.naxes; i++)
      if (shift.i32[i] >= srcinfo.rdims[0] || shift.i32[i] <= -srcinfo.rdims[0]) {
        // the requested shift distance is so great that no part of the
        // old image remains in the field of view
        zerobytes(trgt.v, srcinfo.nelem*lux_type_size[srcinfo.type]);
        return isFunction? iq: LUX_OK;
      }
    do {
      i = *shift.i32++;
      offset = i*srcinfo.stride;
      step = srcinfo.step[0]*srcinfo.stride;
      src = src0;
      trgt = trgt0;
      tmp0 = (char*) realloc(tmp0, srcinfo.rdims[0]*srcinfo.stride);
      if (!tmp0)
        return cerror(ALLOC_ERR, 0);
      if (i > 0)
        for (j = 0; j < i; j++)
          memcpy(tmp0 + j*srcinfo.stride, blank.ui8, srcinfo.stride);
      else
        for (j = 0; j < -i; j++)
          memcpy(tmp0 + (srcinfo.rdims[0] + i + j)*srcinfo.stride,
                 blank.ui8, srcinfo.stride);
      do {
        // we assemble the shifted elements in some temporary storage
        if (offset > 0) {       // we're shifting forward
          tmp = tmp0 + offset;
          for (j = 0; j < srcinfo.rdims[0] - i; j++) {
            memcpy(tmp, src.ui8, srcinfo.stride);
            tmp += srcinfo.stride;
            src.ui8 += step;
          }
          src.ui8 += i*step;
        } else {                // we're shifting backward
          tmp = tmp0;
          src.ui8 -= step*i;
          for (j = -i; j < srcinfo.rdims[0]; j++) {
            memcpy(tmp, src.ui8, srcinfo.stride);
            tmp += srcinfo.stride;
            src.ui8 += step;
          }
        }
        // and we move the finished elements to their final location
        tmp = tmp0;
        for (j = 0; j < srcinfo.rdims[0]; j++) {
          memcpy(trgt.ui8, tmp, srcinfo.stride);
          tmp += srcinfo.stride;
          trgt.ui8 += step;
        }
      } while (trgtinfo.advanceLoop(&trgt.ui8),
               srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
      src0.ui8 = trgt0.ui8;                 // take result of previous loop as source
      // of next one
    } while (nextLoops(&srcinfo, &trgtinfo));
  } else                        // circular shift
    do {
      i = *shift.i32++;           // shift distance in current dimension
      // reduce shift distance to interval (0:srcinfo.rdims[0]-1)
      if (i < 0)
        i += srcinfo.rdims[0]*(1 - i/srcinfo.rdims[0]);
      if (i >= srcinfo.rdims[0])
        i -= srcinfo.rdims[0]*(i/srcinfo.rdims[0]);
      offset = i*srcinfo.stride;
      step = srcinfo.step[0]*srcinfo.stride;
      src = src0;
      trgt = trgt0;
      tmp0 = (char*) realloc(tmp0, srcinfo.rdims[0]*srcinfo.stride);
      if (!tmp0)
        return cerror(ALLOC_ERR, 0);
      do {
        // we assemble the shifted elements in some temporary storage
        // first the elements that get shifted forward
        tmp = tmp0 + offset;
        for (j = 0; j < srcinfo.rdims[0] - i; j++) {
          memcpy(tmp, src.ui8, srcinfo.stride);
          tmp += srcinfo.stride;
          src.ui8 += step;
        }
        // then the elements that get shifted backward
        tmp = tmp0;
        for (; j < srcinfo.rdims[0]; j++) {
          memcpy(tmp, src.ui8, srcinfo.stride);
          tmp += srcinfo.stride;
          src.ui8 += step;
        }
        // finally, we move the finished elements to their final location
        tmp = tmp0;
        for (j = 0; j < srcinfo.rdims[0]; j++) {
          memcpy(trgt.ui8, tmp, srcinfo.stride);
          tmp += srcinfo.stride;
          trgt.ui8 += step;
        }
      } while (trgtinfo.advanceLoop(&trgt.ui8),
               srcinfo.advanceLoop(&src.ui8) < srcinfo.rndim);
      src0.ui8 = trgt0.ui8;                 // take result of previous loop as source
      // of next one
    } while (nextLoops(&srcinfo, &trgtinfo));
  free(tmp0);
  return isFunction? iq: LUX_OK;
}
//---------------------------------------------------------
int32_t lux_shift_f(ArgumentCount narg, Symbol ps[])
{
  return shift(narg, ps, 1);
}
//---------------------------------------------------------
int32_t lux_shift(ArgumentCount narg, Symbol ps[])
{
  return shift(narg, ps, 0);
}
//---------------------------------------------------------
int32_t lux_swaphalf(ArgumentCount narg, Symbol ps[])
// SWAPHALF,d  swaps elements of <d> such that new coordinate x_i' is
// related to old coordinate x_i according to
// x_i' = (x_i + n_i/2) mod n_i  where n_i is the size of <d> in
// dimension i.  Such swapping is useful in conjunction with
// Fourier and Hartley transforms which (at least in LUX) return
// data such that wavevector 0 ends up in result element 0.  After
// swapping with SWAPHALF, wavevector 0 is in the middle of the
// array (at coordinates n_i/2) and the length of any wavevector
// is given by its cartesian distance to the middle of the array.
// LS 21may97
{
  int32_t       i, dims[MAX_DIMS], ndim, tally[MAX_DIMS], step[MAX_DIMS],
        n, width, jump;
  char  *src;
  Scalar        temp;

  if (symbol_class(*ps) != LUX_ARRAY)
    return cerror(NEED_ARR, *ps);
  src = (char *) array_data(*ps);
  ndim = array_num_dims(*ps);
  memcpy(dims, array_dims(*ps), ndim*sizeof(int32_t));
  for (i = 0; i < ndim; i++)
    if (dims[i] % 2 == 1)
      return luxerror("SWAPHALF deals only with even-dimension arrays", *ps);

  n = width = lux_type_size[array_type(*ps)];
  for (i = 0; i < ndim; i++) {
    tally[i] = 0;
    n *= dims[i];
    step[i] = n;
  }

  dims[ndim - 1] /= 2;

  jump = 0;
  for (i = 0; i < ndim; i++)
    jump += step[i];
  jump /= 2;

  do {
    memcpy(&temp.ui8, src, width);
    memcpy(src, src + jump, width);
    memcpy(src + jump, &temp.ui8, width);
    src += width;
    for (i = 0; i < ndim; i++)
      if (++(tally[i]) == dims[i]) {
        tally[i] = 0;
        jump += step[i];
      } else {
        if (tally[i] == dims[i]/2 && i != ndim - 1)
          jump -= step[i];
        break;
      }
  } while (i < ndim);
  return 1;
}
//---------------------------------------------------------
int32_t lux_equivalence(ArgumentCount narg, Symbol ps[])
// returns equivalence classes.  If <x1> and <x2> are equivalence relations
// such that for all <i> elements <x1(i)> and <x2(i)> are in the same class
// then <y> = EQUIVALENCE(<x1>,<x2>) returns in each <y(i)> the smallest
// element of <x1> and <x2> that is in the same class as elements <x(i)>
// and <y(i)>.  LS 30jul97
{
  int32_t       result, n, *x1, *x2, *class_id, *trgt, mx, mn, nClass, i, a1, a2;

  if (symbol_class(ps[0]) != LUX_ARRAY)
    return cerror(ILL_CLASS, ps[0]);
  if (symbol_class(ps[1]) != LUX_ARRAY)
    return cerror(ILL_CLASS, ps[1]);
  n = array_size(ps[0]);
  if (n != array_size(ps[1]))
    return cerror(INCMP_ARR, ps[2]);
  x1 = (int32_t *) array_data(lux_long(1, ps));
  x2 = (int32_t *) array_data(lux_long(1, ps + 1));
  result = array_clone(ps[0], LUX_INT32);
  trgt = (int32_t *) array_data(result);
  mx = INT32_MIN;
  mn = INT32_MAX;
  for (i = 0; i < n; i++)
  { if (x1[i] < mn)
      mn = x1[i];
    else if (x1[i] > mx)
      mx = x1[i];
    if (x2[i] < mn)
      mn = x2[i];
    else if (x2[i] > mx)
      mx = x2[i]; }
  nClass = mx - mn + 1;
  class_id = (int32_t *) Malloc(nClass*sizeof(int32_t));
  if (!class_id)
    return cerror(ALLOC_ERR, 0);
  for (i = 0; i < nClass; i++)
    class_id[i] = i;
  for (i = 0; i < n; i++)
  { a1 = x1[i] - mn;
    a2 = x2[i] - mn;
    while (class_id[a1] != a1)  // find most remote "ancestor"
      a1 = class_id[a1];
    while (class_id[a2] != a2)
      a2 = class_id[a2];
    if (a1 < a2)                // reclassify
      class_id[a2] = a1;
    else
      class_id[a1] = a2; }
  for (i = 0; i < nClass; i++)
  { a1 = class_id[i];
    while (class_id[a1] != a1)
      a1 = class_id[a1];
    class_id[i] = a1; }
  for (i = 0; i < n; i++)
    *trgt++ = class_id[x1[i] - mn] + mn;
  free(class_id);
  return result;
}
//---------------------------------------------------------
int32_t local_extrema(ArgumentCount narg, Symbol ps[], int32_t code)
/* local extreme search in multiple dimensions
   (x [, diagonal], /DEGREE, /SUBGRID])
   <x>: data
   <sign>: sign of objects to look for, positive integer -> hill-like,
    negative integer -> valley-like.  If zero, then +1 is assumed.
   /DEGREE: returns number of OK directions per data element
   code: 0 -> return minimum values, 1 -> return minimum locations,
         2 -> return maximum values, 3 -> return maximum locations
   LS 18may95 4aug97 */
{
  int32_t       result, sign, degree, n, i, *offset, k, j, *edge,
    *diagonal, nDiagonal, index, subgrid, ready, done;
  Pointer       src, trgt, srcl, srcr;
  LoopInfo      srcinfo, trgtinfo;

  if (!symbolIsNumericalArray(ps[0]))
    return cerror(NEED_ARR, ps[0]);
  degree = (internalMode & 1);
  if ((degree
       && standardLoop(ps[0], 0,
                       SL_ALLAXES | SL_SAMEDIMS | SL_EXACT | SL_EACHCOORD,
                       LUX_INT32, &srcinfo, &src, &result, &trgtinfo, &trgt)
       < 0)
      || standardLoop(ps[0], 0, SL_ALLAXES | SL_EACHCOORD, LUX_INT8,
                      &srcinfo, &src, NULL, NULL, NULL) < 0)
    return LUX_ERROR;

  subgrid = (internalMode & 2);
  sign = (code & 2);            // 1 -> seek maxima

  n = prepareDiagonals(narg > 1? ps[1]: 0, &srcinfo, 2, &offset, &edge, NULL,
                       &diagonal);
  if (n == LUX_ERROR)
    return LUX_ERROR;

  std::vector<size_t> hit_locations;

  // we exclude the selected edges
  for (i = 0; i < srcinfo.ndim; i++)
    if (edge[2*i + 1]) {
      edge[2*i + 1] = srcinfo.dims[i] - 2;
      if (edge[2*i + 1] < 0)
        edge[2*i + 1] = 0;
    }
  if (degree) {
    // we set the edges to zero
    for (i = 0; i < trgtinfo.ndim; i++)
      if (edge[i]) {
        trgtinfo.rearrangeEdgeLoop(NULL, i);
        do
          *trgt.i32 = 0;
        while (trgtinfo.advanceLoop(&trgt.ui8) < trgtinfo.ndim - 1);
      }
    // and exclude the edges from further dealings
    trgtinfo.subdataLoop(edge);
  }
  srcinfo.subdataLoop(edge);
  free(edge);

  // now do the loop work
  switch (array_type(ps[0])) {
    case LUX_INT8:
      do {
        int ok_count = 1 - degree;
        for (j = 0; j < n; j++) {       // all directions
          k = offset[j];
          srcl.ui8 = src.ui8 + k;
          srcr.ui8 = src.ui8 - k;
          bool ok = (sign && *src.ui8 >= *srcl.ui8 && *src.ui8 > *srcr.ui8)
            || (!sign && *src.ui8 <= *srcl.ui8 && *src.ui8 < *srcr.ui8);
          if (degree)
            ok_count += ok;
          else if (!ok) {
            ok_count = 0;
            break;
          }     // end if (!ok)
        } // end for (j = 0; j < n; j++)
        if (degree)
          *trgt.i32 = ok_count;
        else if (ok_count)
          hit_locations.push_back(src.ui8 - (uint8_t *) srcinfo.data0);
        done = degree? (trgtinfo.advanceLoop(&trgt.ui8),
                        srcinfo.advanceLoop(&src.ui8)):
          srcinfo.advanceLoop(&src.ui8);
      } while (done < srcinfo.ndim);
      break;
    case LUX_INT16:
      do {
        int ok_count = 1 - degree;
        for (j = 0; j < n; j++) {       // all directions
          k = offset[j];
          srcl.i16 = src.i16 + k;
          srcr.i16 = src.i16 - k;
          bool ok = (sign && *src.i16 >= *srcl.i16 && *src.i16 > *srcr.i16)
            || (!sign && *src.i16 <= *srcl.i16 && *src.i16 < *srcr.i16);
          if (degree)
            ok_count += ok;
          else if (!ok) {
            ok_count = 0;
            break;
          }     // end if (!ok)
        } // end for (j = 0; j < n; j++)
        if (degree)
          *trgt.i32 = ok_count;
        else if (ok_count)
          hit_locations.push_back(src.i16 - (int16_t *) srcinfo.data0);
        done = degree? (trgtinfo.advanceLoop(&trgt.ui8),
                        srcinfo.advanceLoop(&src.ui8)):
          srcinfo.advanceLoop(&src.ui8);
      } while (done < srcinfo.ndim);
      break;
    case LUX_INT32:
      do {
        int ok_count = 1 - degree;
        for (j = 0; j < n; j++) {       // all directions
          k = offset[j];
          srcl.i32 = src.i32 + k;
          srcr.i32 = src.i32 - k;
          bool ok = (sign && *src.i32 >= *srcl.i32 && *src.i32 > *srcr.i32)
            || (!sign && *src.i32 <= *srcl.i32 && *src.i32 < *srcr.i32);
          if (degree)
            ok_count += ok;
          else if (!ok) {
            ok_count = 0;
            break;
          }     // end if (!ok)
        } // end for (j = 0; j < n; j++)
        if (degree)
          *trgt.i32 = ok_count;
        else if (ok_count)
          hit_locations.push_back(src.i32 - (int32_t *) srcinfo.data0);
        done = degree? (trgtinfo.advanceLoop(&trgt.ui8),
                        srcinfo.advanceLoop(&src.ui8)):
          srcinfo.advanceLoop(&src.ui8);
      } while (done < srcinfo.ndim);
      break;
    case LUX_INT64:
      do {
        int ok_count = 1 - degree;
        for (j = 0; j < n; j++) {       // all directions
          k = offset[j];
          srcl.i64 = src.i64 + k;
          srcr.i64 = src.i64 - k;
          bool ok = (sign && *src.i64 >= *srcl.i64 && *src.i64 > *srcr.i64)
            || (!sign && *src.i64 <= *srcl.i64 && *src.i64 < *srcr.i64);
          if (degree)
            ok_count += ok;
          else if (!ok) {
            ok_count = 0;
            break;
          }     // end if (!ok)
        } // end for (j = 0; j < n; j++)
        if (degree)
          *trgt.i64 = ok_count;
        else if (ok_count)
          hit_locations.push_back(src.i64 - (int64_t *) srcinfo.data0);
        done = degree? (trgtinfo.advanceLoop(&trgt.ui8),
                        srcinfo.advanceLoop(&src.ui8)):
          srcinfo.advanceLoop(&src.ui8);
      } while (done < srcinfo.ndim);
      break;
    case LUX_FLOAT:
      do {
        int ok_count = 1 - degree;
        for (j = 0; j < n; j++) {       // all directions
          k = offset[j];
          srcl.f = src.f + k;
          srcr.f = src.f - k;
          bool ok = (sign && *src.f >= *srcl.f && *src.f > *srcr.f)
            || (!sign && *src.f <= *srcl.f && *src.f < *srcr.f);
          if (degree)
            ok_count += ok;
          else if (!ok) {
            ok_count = 0;
            break;
          }     // end if (!ok)
        } // end for (j = 0; j < n; j++)
        if (degree)
          *trgt.i32 = ok_count;
        else if (ok_count)
          hit_locations.push_back(src.f - (float *) srcinfo.data0);
        done = degree? (trgtinfo.advanceLoop(&trgt.ui8),
                        srcinfo.advanceLoop(&src.ui8)):
          srcinfo.advanceLoop(&src.ui8);
      } while (done < srcinfo.ndim);
      break;
    case LUX_DOUBLE:
      do {
        int ok_count = 1 - degree;
        for (j = 0; j < n; j++) {       // all directions
          k = offset[j];
          srcl.d = src.d + k;
          srcr.d = src.d - k;
          bool ok = (sign && *src.d >= *srcl.d && *src.d > *srcr.d)
            || (!sign && *src.d <= *srcl.d && *src.d < *srcr.d);
          if (degree)
            ok_count += ok;
          else if (!ok) {
            ok_count = 0;
            break;
          }     // end if (!ok)
        } // end for (j = 0; j < n; j++)
        if (degree)
          *trgt.i32 = ok_count;
        else if (ok_count)
          hit_locations.push_back(src.d - (double *) srcinfo.data0);
        done = degree? (trgtinfo.advanceLoop(&trgt.ui8),
                        srcinfo.advanceLoop(&src.ui8)):
          srcinfo.advanceLoop(&src.ui8);
      } while (done < srcinfo.ndim);
      break;
  }

  auto hitit = hit_locations.begin();
  std::vector<float> grad2;
  std::vector<float> grad;
  std::vector<float> hessian;

  nDiagonal = 0;
  if (!degree) {
    n = hit_locations.size();       // number of found extrema
    if (!n)                     // none found
      return LUX_MINUS_ONE;
    switch (code) {
      case 0: case 2:           // find values
        if (subgrid)
          result = array_scratch(LUX_FLOAT, 1, &n);
        else
          result = array_scratch(symbol_type(ps[0]), 1, &n);
        trgt.v = array_data(result);
        break;
      case 1: case 3:           // find positions
        if (!(internalMode & 4) && !subgrid) {  // not /COORDS, not /SUBGRID
          srcinfo.coords[0] = n;        // number of found extrema
          result = array_scratch(LUX_INT32, 1, srcinfo.coords);
          trgt.i32 = (int32_t *) array_data(result);
        } else {
          srcinfo.coords[0] = srcinfo.ndim; // # dimensions in the data
          srcinfo.coords[1] = n;        // number of found extrema
          result = array_scratch(subgrid? LUX_FLOAT: LUX_INT32, 2,
                                 srcinfo.coords);
          trgt.i32 = (int32_t *) array_data(result);
        }
        break;
    }
    if (result < 0)
      return LUX_ERROR;                 // some error
    if (subgrid) {
      /* count the number of dimensions that allow diagonal links:
         the quadratic fits involve only those dimensions */
      if (!diagonal)
        nDiagonal = srcinfo.ndim;
      else {
        for (i = 0; i < srcinfo.ndim; i++)
          if (diagonal[i])
            nDiagonal++;
      }         // end of if (!diagonal) else
      if (nDiagonal) {
        grad2.resize(nDiagonal);
        grad.resize(nDiagonal);
        hessian.resize(nDiagonal*nDiagonal);
        k = 0;
        for (i = 0; i < srcinfo.ndim; i++)
          if (!diagonal || diagonal[i])
            srcinfo.step[k] = srcinfo.step[i]; // participating dimensions
      }         // end of if (nDiagonal)

      src.i32 = (int32_t *) array_data(ps[0]); // data
      float value;
      while (n--) {             // all found extrema
        index = k = *hitit++;         // index of extremum
        for (i = 0; i < srcinfo.ndim; i++) { // calculate coordinates
          srcinfo.coords[i] = k % srcinfo.dims[i];
          k /= srcinfo.dims[i];
        } // end of for (i = 0;...)
        switch (symbol_type(ps[0])) {
          case LUX_INT8:
            srcl.ui8 = src.ui8 + index;
            value = (float) *srcl.ui8;
            for (i = 0; i < nDiagonal; i++)
              grad[i] = grad2[i]
                = ((float) srcl.ui8[srcinfo.step[i]]
                   - (float) srcl.ui8[-srcinfo.step[i]])/2;
            ready = 1;  // zero gradient?
            for (i = 0; i < nDiagonal; i++)
              if (grad[i] != 0) {
                ready = 0;
                break;
              } // end of if (grad[i] != 0)
            if (ready)  // yes, zero gradient
              break;
            for (i = 0; i < nDiagonal; i++)
              for (j = 0; j <= i; j++)
                if (i == j)
                  hessian[i + i*nDiagonal]
                    = (float) srcl.ui8[srcinfo.step[i]]
                    + (float) srcl.ui8[-srcinfo.step[i]] - 2*value;
                else
                  hessian[i + j*nDiagonal] = hessian[j + i*nDiagonal]
                    = ((float) srcl.ui8[srcinfo.step[i] + srcinfo.step[j]]
                       + (float) srcl.ui8[-srcinfo.step[i] - srcinfo.step[j]]
                       - (float) srcl.ui8[srcinfo.step[i] - srcinfo.step[j]]
                       - (float) srcl.ui8[srcinfo.step[j]
                                       - srcinfo.step[i]])/4;
            break;
          case LUX_INT16:
            srcl.i16 = src.i16 + index;
            value = (float) *srcl.i16;
            for (i = 0; i < nDiagonal; i++)
              grad[i] = grad2[i]
                = ((float) srcl.i16[srcinfo.step[i]]
                   - (float) srcl.i16[-srcinfo.step[i]])/2;
            ready = 1;  // zero gradient?
            for (i = 0; i < nDiagonal; i++)
              if (grad[i] != 0) {
                ready = 0;
                break;
              } // end of if (grad[i] != 0)
            if (ready)  // yes, zero gradient
              break;
            for (i = 0; i < nDiagonal; i++)
              for (j = 0; j <= i; j++)
                if (i == j)
                  hessian[i + i*nDiagonal]
                    = (float) srcl.i16[srcinfo.step[i]]
                    + (float) srcl.i16[-srcinfo.step[i]] - 2*value;
                else
                  hessian[i + j*nDiagonal] = hessian[j + i*nDiagonal]
                    = ((float) srcl.i16[srcinfo.step[i] + srcinfo.step[j]]
                       + (float) srcl.i16[-srcinfo.step[i] - srcinfo.step[j]]
                       - (float) srcl.i16[srcinfo.step[i] - srcinfo.step[j]]
                       - (float) srcl.i16[srcinfo.step[j]
                                       - srcinfo.step[i]])/4;
            break;
          case LUX_INT32:
            srcl.i32 = src.i32 + index;
            value = (float) *srcl.i32;
            for (i = 0; i < nDiagonal; i++)
              grad[i] = grad2[i]
                = ((float) srcl.i32[srcinfo.step[i]]
                   - (float) srcl.i32[-srcinfo.step[i]])/2;
            ready = 1;  // zero gradient?
            for (i = 0; i < nDiagonal; i++)
              if (grad[i] != 0) {
                ready = 0;
                break;
              } // end of if (grad[i] != 0)
            if (ready)  // yes, zero gradient
              break;
            for (i = 0; i < nDiagonal; i++)
              for (j = 0; j <= i; j++)
                if (i == j)
                  hessian[i + i*nDiagonal]
                    = (float) srcl.i32[srcinfo.step[i]]
                    + (float) srcl.i32[-srcinfo.step[i]] - 2*value;
                else
                  hessian[i + j*nDiagonal] = hessian[j + i*nDiagonal]
                    = ((float) srcl.i32[srcinfo.step[i] + srcinfo.step[j]]
                       + (float) srcl.i32[-srcinfo.step[i] - srcinfo.step[j]]
                       - (float) srcl.i32[srcinfo.step[i] - srcinfo.step[j]]
                       - (float) srcl.i32[srcinfo.step[j]
                                       - srcinfo.step[i]])/4;
            break;
          case LUX_INT64:
            srcl.i64 = src.i64 + index;
            value = (float) *srcl.i64;
            for (i = 0; i < nDiagonal; i++)
              grad[i] = grad2[i]
                = ((float) srcl.i64[srcinfo.step[i]]
                   - (float) srcl.i64[-srcinfo.step[i]])/2;
            ready = 1;  // zero gradient?
            for (i = 0; i < nDiagonal; i++)
              if (grad[i] != 0) {
                ready = 0;
                break;
              } // end of if (grad[i] != 0)
            if (ready)  // yes, zero gradient
              break;
            for (i = 0; i < nDiagonal; i++)
              for (j = 0; j <= i; j++)
                if (i == j)
                  hessian[i + i*nDiagonal]
                    = (float) srcl.i64[srcinfo.step[i]]
                    + (float) srcl.i64[-srcinfo.step[i]] - 2*value;
                else
                  hessian[i + j*nDiagonal] = hessian[j + i*nDiagonal]
                    = ((float) srcl.i64[srcinfo.step[i] + srcinfo.step[j]]
                       + (float) srcl.i64[-srcinfo.step[i] - srcinfo.step[j]]
                       - (float) srcl.i64[srcinfo.step[i] - srcinfo.step[j]]
                       - (float) srcl.i64[srcinfo.step[j]
                                       - srcinfo.step[i]])/4;
            break;
          case LUX_FLOAT:
            srcl.f = src.f + index;
            value = (float) *srcl.f;
            for (i = 0; i < nDiagonal; i++)
              grad[i] = grad2[i]
                = ((float) srcl.f[srcinfo.step[i]]
                   - (float) srcl.f[-srcinfo.step[i]])/2;
            ready = 1;  // zero gradient?
            for (i = 0; i < nDiagonal; i++)
              if (grad[i] != 0) {
                ready = 0;
                break;
              } // end of if (grad[i] != 0)
            if (ready)  // yes, zero gradient
              break;
            for (i = 0; i < nDiagonal; i++)
              for (j = 0; j <= i; j++)
                if (i == j)
                  hessian[i + i*nDiagonal]
                    = (float) srcl.f[srcinfo.step[i]]
                    + (float) srcl.f[-srcinfo.step[i]] - 2*value;
                else
                  hessian[i + j*nDiagonal] = hessian[j + i*nDiagonal]
                    = ((float) srcl.f[srcinfo.step[i] + srcinfo.step[j]]
                       + (float) srcl.f[-srcinfo.step[i] - srcinfo.step[j]]
                       - (float) srcl.f[srcinfo.step[i] - srcinfo.step[j]]
                       - (float) srcl.f[srcinfo.step[j]
                                       - srcinfo.step[i]])/4;
            break;
          case LUX_DOUBLE:
            srcl.d = src.d + index;
            value = (float) *srcl.d;
            for (i = 0; i < nDiagonal; i++)
              grad[i] = grad2[i]
                = ((float) srcl.d[srcinfo.step[i]]
                   - (float) srcl.d[-srcinfo.step[i]])/2;
            ready = 1;  // zero gradient?
            for (i = 0; i < nDiagonal; i++)
              if (grad[i] != 0) {
                ready = 0;
                break;
              } // end of if (grad[i] != 0)
            if (ready)  // yes, zero gradient
              break;
            for (i = 0; i < nDiagonal; i++)
              for (j = 0; j <= i; j++)
                if (i == j)
                  hessian[i + i*nDiagonal]
                    = (float) srcl.d[srcinfo.step[i]]
                    + (float) srcl.d[-srcinfo.step[i]] - 2*value;
                else
                  hessian[i + j*nDiagonal] = hessian[j + i*nDiagonal]
                    = ((float) srcl.d[srcinfo.step[i] + srcinfo.step[j]]
                       + (float) srcl.d[-srcinfo.step[i] - srcinfo.step[j]]
                       - (float) srcl.d[srcinfo.step[i] - srcinfo.step[j]]
                       - (float) srcl.d[srcinfo.step[j]
                                       - srcinfo.step[i]])/4;
            break;
        } // end of switch (type)
        if (ready)
          for (i = 0; i < nDiagonal; i++)
            grad2[i] = 0.0;
        else {          // solve hessian.x = gradient
          f_decomp(hessian.data(), nDiagonal, nDiagonal);
          ready = 1;
          for (i = nDiagonal*nDiagonal - 1; i >= 0; i -= nDiagonal + 1)
            if (!hessian[i]) { // zero determinant, indeterminate fit
              ready = 0;
              for (i = 0; i < nDiagonal; i++)
                grad2[i] = 0.0;
              break;
            }   // end of if (!hessian[i])
          if (ready) {
            f_solve(hessian.data(), grad2.data(), nDiagonal, nDiagonal);
            for (i = 0; i < nDiagonal; i++)
              value -= 0.5*grad2[i]*grad[i];
          } // end of if (ready)
        } // end of if (ready) else
        if (code & 1) {                 // find position
          k = 0;
          for (i = 0; i < srcinfo.ndim; i++)
            *trgt.f++ = srcinfo.coords[i]
              - ((!diagonal || diagonal[i])? grad2[k++]: 0);
        } else                  // find value
          *trgt.f++ = value;
      }         // end of while (n--)
    } else {                    // subgrid == 0
      if (code & 1) {           // positions
        while (n--) {
          index = *hitit++;
          if (internalMode & 4)         // /COORDS
            for (i = 0; i < srcinfo.ndim; i++) {
              *trgt.i32++ = index % srcinfo.dims[i];
              index /= srcinfo.dims[i];
            } // end of for (i = 0;...)
          else
            *trgt.i32++ = index;
        } // end of while (n--)
      } else {                  // values
        src.v = srcinfo.data0; // reset to start of data
        while (n--) {
          index = *hitit++;
          switch (symbol_type(ps[0])) {
            case LUX_INT8:
              *trgt.ui8++ = src.ui8[index];
              break;
            case LUX_INT16:
              *trgt.i16++ = src.i16[index];
              break;
            case LUX_INT32:
              *trgt.i32++ = src.i32[index];
              break;
            case LUX_INT64:
              *trgt.i64++ = src.i64[index];
              break;
            case LUX_FLOAT:
              *trgt.f++ = src.f[index];
              break;
            case LUX_DOUBLE:
              *trgt.d++ = src.d[index];
              break;
          } // end of switch (symbol_type(ps[0]))
        } // end of while (n--)
      } // end of if (code & 1)
    } // end of if (subgrid)
  } // end of if (!degree)

  free(offset);
  return result;
}
//---------------------------------------------------------
int32_t lux_find_maxloc(ArgumentCount narg, Symbol ps[])
{
  return local_extrema(narg, ps, 3);
}
//---------------------------------------------------------
int32_t lux_find_minloc(ArgumentCount narg, Symbol ps[])
{
  return local_extrema(narg, ps, 1);
}
//---------------------------------------------------------
int32_t lux_find_max(ArgumentCount narg, Symbol ps[])
{
  return local_extrema(narg, ps, 2);
}
//---------------------------------------------------------
int32_t lux_find_min(ArgumentCount narg, Symbol ps[])
{
  return local_extrema(narg, ps, 0);
}
//---------------------------------------------------------
int32_t lux_replace_values(ArgumentCount narg, Symbol ps[])
/* REPLACE,x,src,trgt
  replaces all occurrences in <x> of each element of <src> by
  the corresponding element of <trgt>.  <src> must be in ascending order!
  LS 20feb98
*/
{
  int32_t       nData, nSrc, nTrgt, type, low, mid, high, ps1, ps2;
  Pointer       data, src, trgt;
  Scalar        v;

  ps1 = (symbol_type(ps[1]) == symbol_type(ps[0]))? ps[1]:
    lux_converts[symbol_type(ps[0])](1, &ps[1]);
  ps2 = (symbol_type(ps[2]) == symbol_type(ps[0]))? ps[2]:
    lux_converts[symbol_type(ps[0])](1, &ps[2]);

  if (numerical(ps[0], NULL, NULL, &nData, &data) < 0
      || numerical(ps1, NULL, NULL, &nSrc, &src) < 0
      || numerical(ps2, NULL, NULL, &nTrgt, &trgt) < 0)
    return LUX_ERROR;
  if (nTrgt != nSrc) // src and trgt must have same number of elements
    return cerror(INCMP_ARG, ps[2]);

  type = symbol_type(ps[0]);

  switch (type) {
  case LUX_INT8:
    mid = 0;
    v.ui8 = *data.ui8 + 1;
    while (nData--) {           // all data points
      if (*data.ui8 != v.ui8) { // we added this check because we
        // hope there is coherence in the data so that the next data
        // value has a fair chance of being equal to the previous one.
        // In that case we can skip the binary search.
        low = 0;
        high = nSrc - 1;
        v.ui8 = *data.ui8;
        while (low <= high) {
          mid = (low + high)/2;
          if (v.ui8 < src.ui8[mid])
            high = mid - 1;
          else if (v.ui8 > src.ui8[mid])
            low = mid + 1;
          else {
            mid = -mid - 1;     // flag that we found it
            break;
          }
        }
      }
      if (mid < 0)              // found it
        *data.ui8 = trgt.ui8[-mid - 1];
      data.ui8++;
    }
    break;
  case LUX_INT16:
    mid = 0;
    v.i16 = *data.i16 + 1;
    while (nData--) {           // all data points
      if (*data.i16 != v.i16) { // we added this check because we
        // hope there is coherence in the data so that the next data
        // value has a fair chance of being equal to the previous one.
        // In that case we can skip the binary search.
        low = 0;
        high = nSrc - 1;
        v.i16 = *data.i16;
        while (low <= high) {
          mid = (low + high)/2;
          if (v.i16 < src.i16[mid])
            high = mid - 1;
          else if (v.i16 > src.i16[mid])
            low = mid + 1;
          else {
            mid = -mid - 1;     // flag that we found it
            break;
          }
        }
      }
      if (mid < 0)              // found it
        *data.i16 = trgt.i16[-mid - 1];
      data.i16++;
    }
    break;
  case LUX_INT32:
    mid = 0;
    v.i32 = *data.i32 + 1;
    while (nData--) {           // all data points
      if (*data.i32 != v.i32) { // we added this check because we
        // hope there is coherence in the data so that the next data
        // value has a fair chance of being equal to the previous one.
        // In that case we can skip the binary search.
        low = 0;
        high = nSrc - 1;
        v.i32 = *data.i32;
        while (low <= high) {
          mid = (low + high)/2;
          if (v.i32 < src.i32[mid])
            high = mid - 1;
          else if (v.i32 > src.i32[mid])
            low = mid + 1;
          else {
            mid = -mid - 1;     // flag that we found it
            break;
          }
        }
      }
      if (mid < 0)              // found it
        *data.i32 = trgt.i32[-mid - 1];
      data.i32++;
    }
    break;
  case LUX_INT64:
    mid = 0;
    v.i64 = *data.i64 + 1;
    while (nData--) {           // all data points
      if (*data.i64 != v.i64) { // we added this check because we
        // hope there is coherence in the data so that the next data
        // value has a fair chance of being equal to the previous one.
        // In that case we can skip the binary search.
        low = 0;
        high = nSrc - 1;
        v.i64 = *data.i64;
        while (low <= high) {
          mid = (low + high)/2;
          if (v.i64 < src.i64[mid])
            high = mid - 1;
          else if (v.i64 > src.i64[mid])
            low = mid + 1;
          else {
            mid = -mid - 1;     // flag that we found it
            break;
          }
        }
      }
      if (mid < 0)              // found it
        *data.i64 = trgt.i64[-mid - 1];
      data.i64++;
    }
    break;
  case LUX_FLOAT:
    mid = 0;
    v.f = *data.f + 1;
    while (nData--) {           // all data points
      if (*data.f != v.f) { // we added this check because we
        // hope there is coherence in the data so that the next data
        // value has a fair chance of being equal to the previous one.
        // In that case we can skip the binary search.
        low = 0;
        high = nSrc - 1;
        v.f = *data.f;
        while (low <= high) {
          mid = (low + high)/2;
          if (v.f < src.f[mid])
            high = mid - 1;
          else if (v.f > src.f[mid])
            low = mid + 1;
          else {
            mid = -mid - 1;     // flag that we found it
            break;
          }
        }
      }
      if (mid < 0)              // found it
        *data.f = trgt.f[-mid - 1];
      data.f++;
    }
    break;
  case LUX_DOUBLE:
    mid = 0;
    v.d = *data.d + 1;
    while (nData--) {           // all data points
      if (*data.d != v.d) { // we added this check because we
        // hope there is coherence in the data so that the next data
        // value has a fair chance of being equal to the previous one.
        // In that case we can skip the binary search.
        low = 0;
        high = nSrc - 1;
        v.d = *data.d;
        while (low <= high) {
          mid = (low + high)/2;
          if (v.d < src.d[mid])
            high = mid - 1;
          else if (v.d > src.d[mid])
            low = mid + 1;
          else {
            mid = -mid - 1;     // flag that we found it
            break;
          }
        }
      }
      if (mid < 0)              // found it
        *data.d = trgt.d[-mid - 1];
      data.d++;
    }
    break;
  }
  return LUX_ONE;
}
//---------------------------------------------------------
int32_t lux_lsq(ArgumentCount narg, Symbol ps[])
/* linear least squares fit.
   a = LSQ(x,y[,w,COV=cov,ERR=err,CHISQ=chisq,/FORMAL])
   The model is that y = a(0)*x(...,0) + a(1)*x(...,1) + ... + error term
   The parameters <a> are those that minimize the weighted rms of the
   error term. <w> contains (optional) weights to assign to each of the <x>
   (if 1D), or a covariance matrix for the <y> (if multi-dim -- NOT YET).
   In <cov> is returned the covariance matrix of the <a>, in <err> the
   weighted residual error, in <chisq> the chi-square.  If /FORMAL is
   specified, then the covariances are multiplied by <err>^2 (suitable
   when no actual variances for <y> are known -- then it is assumed that
   <w> contains numbers of datapoints only [usually 1]).

   LS 4may98

Theory:
    V = (X'WX)^-1     covariance matrix of <a>
    a = VX'Wy         parameters <a>
    chi2 = y'Wy - a'V^-1a  chi-square
    err = sqrt(chi2/total(W))  residual error
   where W is C^-1, C is the covariance matrix of <y>.
   */
{
  int32_t       sx, sy, sw, *dims, ndim, nPar, i, nData, result, j, k, dw,
    newDims[MAX_DIMS];
  Symboltype type;
  float         onef = 1.0;
  double        oned = 1.0, errv, f, chi2;
  Scalar        value;
  Pointer       px, py, pl, pr, p1, p2, pw, pc, err, chisq;
  int32_t       f_decomp(float *, int32_t, int32_t), d_decomp(double *, int32_t, int32_t),
    f_solve(float *, float *, int32_t, int32_t),
    d_solve(double *, double *, int32_t, int32_t);

  sx = ps[0];                   // <x>: independent data
  if (symbol_class(sx) != LUX_ARRAY)
    return cerror(ILL_CLASS, sx);
  if (!symbolIsNumerical(sx))
    return cerror(ILL_TYPE, sx);
  dims = array_dims(sx);
  ndim = array_num_dims(sx);

  sy = ps[1];                   // <y>: dependent data
  if (symbol_class(sy) != LUX_ARRAY)
    return cerror(ILL_CLASS, sy);
  if (!symbolIsNumerical(sy))
    return cerror(ILL_TYPE, sy);
  if (array_num_dims(sy) == ndim) // must have same # dims as <x>...
    nPar = 1;                   // number of parameters to fit
  else if (array_num_dims(sy) == ndim - 1) // ... or one less
    nPar = dims[ndim - 1];
  else
    return cerror(INCMP_ARG, sy);
  nData = array_size(sy);       // number of data points

  for (i = 0; i < array_num_dims(sy); i++) // dimensions must match
    if (array_dims(sy)[i] != dims[i])
      return cerror(INCMP_ARG, sy);

  narg -= 2;
  ps += 2;

  if (narg > 0 && *ps) {                // have <w>: weights (if 1D)
    sw = *ps;
    if (symbol_class(sw) != LUX_ARRAY)
      return cerror(ILL_CLASS, sw);
    if (!symbolIsNumerical(sw))
      return cerror(ILL_TYPE, sw);
    if (array_num_dims(sw) != array_num_dims(sy))
      return cerror(INCMP_ARG, sw);
    for (i = 0; i < array_num_dims(sw); i++)
      if (array_dims(sy)[i] != array_dims(sw)[i])
        return cerror(INCMP_ARG, sw);
    dw = 1;
  } else
    dw = 0;

  type = LUX_FLOAT;             /* determine hightest data type: all will
                                 be upgraded to that */
  if (array_type(sx) > type)
    type = array_type(sx);
  if (array_type(sy) > type)
    type = array_type(sy);
  if (dw && array_type(sw) > type)
    type = array_type(sw);
  switch (type) {
    case LUX_FLOAT:
      if (array_type(sx) < LUX_FLOAT)
        sx = lux_float(1, &sx);
      if (array_type(sy) < LUX_FLOAT)
        sy = lux_float(1, &sy);
      if (dw && array_type(sw) < LUX_FLOAT)
        sw = lux_float(1, &sw);
      break;
    case LUX_DOUBLE:
      if (array_type(sx) < LUX_DOUBLE)
        sx = lux_double(1, &sx);
      if (array_type(sy) < LUX_DOUBLE)
        sy = lux_double(1, &sy);
      if (dw && array_type(sw) < LUX_DOUBLE)
        sw = lux_double(1, &sw);
      break;
  }

  if (!dw)                      // no <w>: default weights equal to 1
    switch (type) {
      case LUX_FLOAT:
        pw.f = &onef;
        break;
      case LUX_DOUBLE:
        pw.d = &oned;
        break;
    }
  else
    pw.f = (float *) array_data(sw);

  narg--; ps++;
  if (narg > 0 && *ps) {        // <cov>: place to return covariances
    newDims[0] = newDims[1] = nPar;
    to_scratch_array(*ps, type, 2, newDims); // make into suitable array
    pc.f = (float *) array_data(*ps);
    zerobytes(pc.f, nPar*nPar*lux_type_size[type]);
    switch (type) {             // fill with identity matrix
      case LUX_FLOAT:
        for (i = 0; i < nPar*nPar; i += nPar + 1)
          pc.f[i] = 1;
        break;
      case LUX_DOUBLE:
        for (i = 0; i < nPar*nPar; i += nPar + 1)
          pc.d[i] = 1;
        break;
    }
  } else
    pc.f = NULL;                // no covariance return

  narg--; ps++;
  if (narg > 0 && *ps) {        // <err>: place to return residual error
    undefine(*ps);              // make into suitable scalar
    symbol_class(*ps) = LUX_SCALAR;
    symbol_type(*ps) = type;
    err.ui8 = &scalar_value(*ps).ui8;
  } else
    err.f = NULL;               // no residual error return

  narg--; ps++;
  if (narg > 0 && *ps) {        // <chisq>: place to return chi-square
    undefine(*ps);
    symbol_class(*ps) = LUX_SCALAR;
    symbol_type(*ps) = type;
    chisq.ui8 = &scalar_value(*ps).ui8;
  } else
    chisq.f = NULL;

  px.f = (float *) array_data(sx); // x data
  py.f = (float *) array_data(sy); // y data

  // now get some scratch space for intermediate results
  pl.f = (float *) malloc(nPar*nPar*lux_type_size[type]);
  zerobytes(pl.f, nPar*nPar*lux_type_size[type]);

  result = array_scratch(type, 1, &nPar); // return symbol: parameters <a>
  pr.f = (float *) array_data(result);
  zerobytes(pr.f, nPar*lux_type_size[type]);

  // now do the calculations
  switch (type) {
    case LUX_FLOAT:
      // get (X'WX) in pl
      for (i = 0; i < nPar; i++) {
        p1.f = px.f + i*nData;
        for (j = 0; j < nPar; j++) {
          p2.f = px.f + j*nData;
          for (k = 0; k < nData; k++) {
            *pl.f += p1.f[k]*p2.f[k]* *pw.f;
            pw.f += dw;
          }
          pl.f++;
          pw.f -= dw*nData;     // back to the start
        }
      }
      pl.f -= nPar*nPar;        // back to the start
      // get (X'Wy) in pr
      for (i = 0; i < nPar; i++) {
        for (j = 0; j < nData; j++) {
          *pr.f += *px.f++ * *py.f++ * *pw.f;
          pw.f += dw;
        }
        pr.f++;
        py.f -= nData;          // back to the start
        pw.f -= dw*nData;
      }
      px.f -= nData*nPar;       // back to the start
      pr.f -= nPar;
      f_decomp(pl.f, nPar, nPar); // LU decomposition of pl
      f_solve(pl.f, pr.f, nPar, nPar); // solve: now pr has parameters <a>
      if (chisq.f != NULL || err.f != NULL
          || (pc.f != NULL && (internalMode & 1))) { // need chi-square
        chi2 = f = 0.0;
        for (i = 0; i < nData; i++) {
          value.f = py.f[i];
          for (j = 0; j < nPar; j++)
            value.f -= pr.f[j]*px.f[j*nData];
          chi2 += value.f*value.f**pw.f;
          f += *pw.f;           // collect weights for normalization
          px.f++;
          pw.f += dw;
        }
        if (chisq.f != NULL)
          *chisq.f = (float) chi2;
        errv = sqrt(chi2/f);    // normalize and get rms
        if (err.f != NULL)
          *err.f = (float) errv; // store
      } else
        chi2 = 1.0;             // don't calculate: assume 1.
      if ((internalMode & 1) == 0)
        chi2 = 1.0;
      if (pc.f != NULL) {       // get covariances
        for (i = 0; i < nPar; i++) {
          f_solve(pl.f, pc.f, nPar, nPar); /* now pc contains
                                              the covariances */
          pc.f += nPar;
        }
        pc.f -= nPar*nPar;      // back to the start
        if (internalMode & 1) {
          if (chisq.f != NULL)
            *chisq.f = f - 1;
          chi2 = chi2/(f - 1);
          for (i = 0; i < nPar*nPar; i++)
            pc.f[i] *= chi2;
        }
        if (internalMode & 2) {
          /* we transform the covariance matrix so its diagonal contains
             standard errors, one triangle contains covariances, and the other
             triangle contains correlation coefficients */
          for (i = 0; i < nPar; i++)
            for (j = 0; j < i; j++)
              pc.f[i + j*nPar] /= sqrt(pc.f[i*(nPar + 1)]*pc.f[j*(nPar + 1)]);
          for (i = 0; i < nPar; i++)
            pc.f[i + i*nPar] = sqrt(pc.f[i + i*nPar]);
        }
      }
      break;
    case LUX_DOUBLE:
      // get (X'WX) in pl
      for (i = 0; i < nPar; i++) {
        p1.d = px.d + i*nData;
        for (j = 0; j < nPar; j++) {
          p2.d = px.d + j*nData;
          for (k = 0; k < nData; k++) {
            *pl.d += p1.d[k]*p2.d[k]* *pw.d;
            pw.d += dw;
          }
          pl.d++;
          pw.d -= dw*nData;     // back to the start
        }
      }
      pl.d -= nPar*nPar;        // back to the start
      // get (X'Wy) in pr
      for (i = 0; i < nPar; i++) {
        for (j = 0; j < nData; j++) {
          *pr.d += *px.d++ * *py.d++ * *pw.d;
          pw.d += dw;
        }
        pr.d++;
        py.d -= nData;          // back to the start
        pw.d -= dw*nData;
      }
      px.d -= nData*nPar;       // back to the start
      pr.d -= nPar;
      d_decomp(pl.d, nPar, nPar); // LU decomposition of pl
      d_solve(pl.d, pr.d, nPar, nPar); // solve: now pr has parameters <a>
      if (chisq.d != NULL || err.d != NULL
          || (pc.d != NULL && (internalMode & 1))) { // need chi-square
        chi2 = f = 0.0;
        for (i = 0; i < nData; i++) {
          value.d = py.d[i];
          for (j = 0; j < nPar; j++)
            value.d -= pr.d[j]*px.d[j*nData];
          chi2 += value.d*value.d**pw.d;
          f += *pw.d;           // collect weights for normalization
          px.d++;
          pw.d += dw;
        }
        if (chisq.d != NULL)
          *chisq.d = (float) chi2;
        errv = sqrt(chi2/f);    // normalize and get rms
        if (err.d != NULL)
          *err.d = (float) errv; // store
      } else
        chi2 = 1.0;             // don't calculate: assume 1.
      if ((internalMode & 1) == 0)
        chi2 = 1.0;
      if (pc.d != NULL) {       // get covariances
        for (i = 0; i < nPar; i++) {
          d_solve(pl.d, pc.d, nPar, nPar); /* now pc contains
                                              the covariances */
          pc.d += nPar;
        }
        pc.d -= nPar*nPar;      // back to the start
        if (internalMode & 1) {
          if (chisq.f != NULL)
            *chisq.d = f - 1;
          chi2 = chi2/(f - 1);
          for (i = 0; i < nPar*nPar; i++)
            pc.d[i] *= chi2;
        }
        if (internalMode & 2) {
          /* we transform the covariance matrix so its diagonal contains
             standard errors, one triangle contains covariances, and the other
             triangle contains correlation coefficients */
          for (i = 0; i < nPar; i++)
            for (j = 0; j < i; j++)
              pc.d[i + j*nPar] /= sqrt(pc.d[i*(nPar + 1)]*pc.d[j*(nPar + 1)]);
          for (i = 0; i < nPar; i++)
            pc.d[i + i*nPar] = sqrt(pc.d[i + i*nPar]);
        }
      }
      break;
  }

  free(pl.f);
  return result;
}
//---------------------------------------------------------
int32_t         lux_indgen(int32_t, int32_t []);
int32_t lux_lsq2(ArgumentCount narg, Symbol ps[])
/* linear least squares fit.
   a = LLSQ(x,y[,axis,FWHM=fwhm,WEIGHTS=w,COV=cov,ERR=err,CHISQ=chisq,
       /FORMAL,/REDUCE])
   The model is that y = a(0)*x(...,0) + a(1)*x(...,1) + ... + error term
   The parameters <a> are those that minimize the weighted rms of the
   error term. <w> contains (optional) weights to assign to each of the <x>
   (if 1D), or a covariance matrix for the <y> (if multi-dim -- NOT YET).
   In <cov> is returned the covariance matrix of the <a>, in <err> the
   weighted residual error, in <chisq> the chi-square.  If /FORMAL is
   specified, then the covariances are multiplied by <err>^2 (suitable
   when no actual variances for <y> are known -- then it is assumed that
   <w> contains numbers of datapoints only [usually 1]).

   LS 4may98

Theory:
    V = (X'WX)^-1     covariance matrix of <a>
    a = VX'Wy         parameters <a>
    chi2 = y'Wy - a'V^-1a  chi-square
    err = sqrt(chi2/total(W))  residual error
   where W is C^-1, C is the covariance matrix of <y>.
   */

// LSQ2(x, y [, axis, w])
{
  int32_t       i, j, k, nPar, nData, result, dims[MAX_DIMS], ndim, dw, ncc,
    axisSym, nRepeat, stride, bigstride, ysym, nkernel, n, i1, i2, cc, dk;
  Symboltype type;
  float         onef = 1.0;
  double oned = 1.0, chi2, f, errv, fwhm, norm;
  Scalar        value;
  Pointer       px, py, pl, pr, p2, pw, pc, err, chisq, pk;
  LoopInfo      xinfo, yinfo, p2info, winfo;

  if (isComplexType(symbol_type(ps[0])))
    return cerror(NO_COMPLEX, ps[0]);
  if (isComplexType(symbol_type(ps[1])))
    return cerror(NO_COMPLEX, ps[1]);
  if (!symbolIsRealArray(ps[0])) // <x>
    return cerror(NEED_NUM_ARR, ps[0]);
  if (!symbolIsRealArray(ps[1])) // <y>
    return cerror(NEED_NUM_ARR, ps[1]);

  // we allow the user to specify zero or one axes.  If none are specified,
  // then <y> and the corresponding dimensions of <x> are regarded as
  // one-dimensional.
  if (narg > 2 && ps[2]) {      // <axes>
    if (!symbolIsRealScalar(ps[2]))
      return cerror(NEED_SCAL, ps[2]);
    axisSym = ps[2];
  } else {                      // default: all <y> axes
    axisSym = array_num_dims(ps[1]);
    axisSym = array_scratch(LUX_INT32, 1, &axisSym);
    axisSym = lux_indgen(1, &axisSym);
  }

  type = combinedType(array_type(ps[0]), array_type(ps[1]));
  if (narg >= 4 && ps[4])
    type = combinedType(type, array_type(ps[4]));
  if (type < LUX_FLOAT)
    type = LUX_FLOAT;

  if (standardLoop(ps[0], axisSym, SL_SRCUPGRADE | SL_AXESBLOCK,
                   type, &xinfo, &px, NULL, NULL, NULL) < 0
      || standardLoop(ps[1], axisSym, SL_SRCUPGRADE | SL_AXESBLOCK,
                      type, &yinfo, &py, NULL, NULL, NULL) < 0)
    return LUX_ERROR;

  ysym = ps[1];

  // <x> must have all the dimensions of <y>, and may have one more
  // dimension at the end
  if (xinfo.ndim != yinfo.ndim && xinfo.ndim != yinfo.ndim + 1)
    return cerror(INCMP_ARG, ps[1]);
  for (i = 0; i < yinfo.ndim; i++)
    if (xinfo.dims[i] != yinfo.dims[i])
      return cerror(INCMP_ARG, ps[1]);

  // find the number of parameters to fit
  if (xinfo.ndim == yinfo.ndim)
    nPar = 1;
  else
    nPar = xinfo.dims[xinfo.ndim - 1];

  // determine the number of data points that go in each fit
  nData = 1;
  for (i = 0; i < yinfo.naxes; i++)
    nData *= yinfo.rdims[i];

  nRepeat = yinfo.nelem/nData;

  ps += 3;
  narg -= 3;

  if (narg > 0 && *ps) {        // have <fhwm>
    if (!symbolIsRealScalar(*ps))
      return cerror(NEED_SCAL, *ps);
    fwhm = double_arg(*ps);
    if (fwhm < 0)
      fwhm = 0.0;
  } else
    fwhm = 0.0;

  ps++; narg--;
  if (narg > 0 && *ps) {        // have <w>: weights
    if (!symbolIsRealArray(*ps))
      return cerror(ILL_CLASS, *ps);
    if (array_num_dims(*ps) != array_num_dims(ysym))
      return cerror(INCMP_ARG, *ps);
    for (i = 0; i < array_num_dims(*ps); i++)
      if (array_dims(ysym)[i] != array_dims(*ps)[i])
        return cerror(INCMP_ARG, *ps);
    standardLoop(*ps, axisSym, SL_SRCUPGRADE | SL_AXESBLOCK,
                 type, &winfo, &pw, NULL, NULL, NULL);
    dw = 1;
  } else {
    switch (type) {
      case LUX_FLOAT:
        pw.f = &onef;
        break;
      case LUX_DOUBLE:
        pw.d = &oned;
        break;
    }
    dw = 0;
  }

  k = fwhm? 0: yinfo.naxes;

  ps++; narg--;
  if (narg > 0 && *ps) {        // <cov>: place to return covariances
    if (yinfo.ndim + 2 - k >= MAX_DIMS)
      return luxerror("Too many dimensions for COV", 0);
    dims[0] = dims[1] = nPar;
    memcpy(dims + 2, yinfo.rdims + k, (yinfo.ndim - k)*sizeof(int32_t));
    ndim = 2 + yinfo.ndim - k;
    if (to_scratch_array(*ps, type, ndim, dims) == LUX_ERROR)
      return LUX_ERROR;
    pc.f = (float *) array_data(*ps);
    zerobytes(pc.f, array_size(*ps)*lux_type_size[type]);
    j = fwhm? nRepeat: yinfo.nelem;
    switch (type) {             // fill with identity matrix
      case LUX_FLOAT:
        while (j--) {
          for (i = 0; i < nPar*nPar; i += nPar + 1)
            pc.f[i] = 1;
          pc.f += nPar*nPar;
        }
        break;
      case LUX_DOUBLE:
        while (j--) {
          for (i = 0; i < nPar*nPar; i += nPar + 1)
            pc.d[i] = 1;
          pc.d += nPar*nPar;
        }
        break;
    }
    pc.f = (float*) array_data(*ps);
  } else
    pc.f = NULL;                // no covariance return

  narg--; ps++;
  if (narg > 0 && *ps) {        // <err>: place to return residual error
    if (nRepeat == 1 && !fwhm) { // a scalar will do
      undefine(*ps);            // make into suitable scalar
      symbol_class(*ps) = LUX_SCALAR;
      symbol_type(*ps) = type;
      err.ui8 = &scalar_value(*ps).ui8;
    } else {                    // we need an array
      ndim = yinfo.ndim - k;
      memcpy(dims, yinfo.rdims + k, ndim*sizeof(int32_t));
      if (to_scratch_array(*ps, type, ndim, dims) == LUX_ERROR)
        return LUX_ERROR;
      err.ui8 = (uint8_t*) array_data(*ps);
    }
  } else
    err.f = NULL;               // no residual error return

  narg--; ps++;
  if (narg > 0 && *ps) {        // <chisq>: place to return chi-square
    if (nRepeat == 1 && !fwhm) { // a scalar will do
      undefine(*ps);            // make into suitable scalar
      symbol_class(*ps) = LUX_SCALAR;
      symbol_type(*ps) = type;
      chisq.ui8 = &scalar_value(*ps).ui8;
    } else {                    // we need an array
      ndim = yinfo.ndim - k;
      memcpy(dims, yinfo.rdims + k, ndim*sizeof(int32_t));
      if (to_scratch_array(*ps, type, ndim, dims) == LUX_ERROR)
        return LUX_ERROR;
      chisq.ui8 = (uint8_t*) array_data(*ps);
    }
  } else
    chisq.f = NULL;             // no residual error return

  // prepare the return symbol
  dims[0] = nPar;
  memcpy(dims + 1, yinfo.rdims + k,
         (yinfo.ndim - k)*sizeof(int32_t));
  ndim = 1 + yinfo.ndim - k;
  result = array_scratch(type, ndim, dims);
  if (result == LUX_ERROR)
    return LUX_ERROR;
  pr.f = (float*) array_data(result);

  // prepare some scratch space
  pl.f = (float*) malloc(nPar*nPar*lux_type_size[type]);
  if (!pl.f)
    return cerror(ALLOC_ERR, 0);

  // prepare smoothing kernel
  if (fwhm) {
    nkernel = MAX(MIN(2*fwhm, nData), nPar);
    pk.f = (float*) malloc((2*nkernel + 1)*lux_type_size[type]);
    errv = 0.600561204/fwhm;
    f = -nkernel*errv;
    for (i = 0; i < 2*nkernel + 1; i++) {
      *pk.f++ = exp(-f*f);
      f += errv;
    }
    pk.f -= 2*nkernel + 1;
    dk = 1;
  } else {
    switch (type) {
      case LUX_FLOAT:
        pk.f = &onef;
        break;
      case LUX_DOUBLE:
        pk.d = &oned;
        break;
    }
    dk = 0;
  }

  // now do the work
  stride = yinfo.rsinglestep[0];
  bigstride = stride*nData;
  n = nData;
  ncc = fwhm? nData: 1;
  do {
    for (cc = 0; cc < ncc; cc++) {
      zerobytes(pl.f, nPar*nPar*lux_type_size[type]);
      zerobytes(pr.f, nPar*lux_type_size[type]);
      p2info = xinfo;
      p2info.data = &p2;
      p2.f = px.f;

      if (fwhm) {
        i1 = MAX(cc - nkernel, 0);
        i2 = MIN(cc + nkernel, nData - 1);
        n = i2 - i1 + 1;
        bigstride = stride*n;
        px.f += i1;
        py.f += i1;
        p2.f += i1;
        if (dw)
          pw.f += i1;
        pk.f += nkernel - cc + i1;
      }

      switch (type) {
        case LUX_FLOAT:
          // get (X'X) in pl
          do {
            do {
              for (i = 0; i < n; i++) {
                *pl.f += *px.f * *p2.f * *pw.f * *pk.f;
                px.f += stride;
                p2.f += stride;
                if (dw)
                  pw.f += stride;
                pk.f += dk;
              }
              px.f -= bigstride;
              p2.f -= bigstride;
              if (dw)
                pw.f -= bigstride;
              pk.f -= n*dk;
              pl.f++;
            } while (xinfo.moveLoop(yinfo.rndim, 1) <= yinfo.rndim);
            xinfo.moveLoop(yinfo.rndim, -nPar);
          } while (p2info.moveLoop(yinfo.rndim, 1) <= yinfo.rndim);
          p2info.moveLoop(yinfo.rndim, -nPar);
          pl.f -= nPar*nPar;    // back to the start

          // get (X'y) in pr
          j = yinfo.naxes - 1;
          do {
            for (i = 0; i < n; i++) {
              *pr.f += *px.f * *py.f * *pw.f * *pk.f;
              px.f += stride;
              py.f += stride;
              if (dw)
                pw.f += stride;
              pk.f += dk;
            }
            pr.f++;
            px.f -= bigstride;
            py.f -= bigstride;
            if (dw)
              pw.f -= bigstride;
            pk.f -= n*dk;
          } while (xinfo.moveLoop(yinfo.rndim, 1) <= yinfo.rndim);
          xinfo.moveLoop(yinfo.rndim, -nPar);
          pr.f -= nPar;                 // back to the start
          f_decomp(pl.f, nPar, nPar); // LU decomposition of pl
          f_solve(pl.f, pr.f, nPar, nPar); // solve: pr gets parameters <a>

          if (chisq.f != NULL || err.f != NULL
              || (pc.f != NULL && (internalMode & 1))) { // need chi-square
            chi2 = f = norm = 0.0;
            // calculate the deviations from the fit
            k = nData*nRepeat;
            for (i = 0; i < n; i++) {
              value.d = *py.f;
              for (j = 0; j < nPar; j++)
                value.d -= pr.f[j]*px.f[j*k];
              chi2 += value.d*value.d * *pw.f * *pk.f;
              norm += *pk.f;
              f += *pw.f * *pk.f;
              px.f += stride;
              py.f += stride;
              if (dw)
                pw.f += stride;
              pk.f += dk;
            }
            px.f -= bigstride;
            py.f -= bigstride;
            if (dw)
              pw.f -= bigstride;
            pk.f -= n*dk;
            if (chisq.f != NULL)
              *chisq.f++ = chi2/norm;
            errv = sqrt(chi2/f);
            if (err.f)
              *err.f++ = errv;
          } else
            chi2 = 1.0;
          if ((internalMode & 1) == 0)
            chi2 = 1.0;
          if (pc.f) {           // get covariances
            for (i = 0; i < nPar; i++) {
              f_solve(pl.f, pc.f, nPar, nPar);
              pc.f += nPar;
            }
            pc.f -= nPar*nPar;  // back to the start
            if (internalMode & 1) {
              if (chisq.f != NULL)
                *chisq.f = f - 1;
              chi2 = chi2/(f - 1);
              for (i = 0; i < nPar*nPar; i++)
                pc.f[i] *= chi2;
            }
            if (internalMode & 2) {
              /* we transform the covariance matrix so its diagonal contains
                 standard errors, one triangle contains covariances, and
                 the other triangle contains correlation coefficients */
              for (i = 0; i < nPar; i++)
                for (j = 0; j < i; j++)
                  pc.f[i + j*nPar] /= sqrt(pc.f[i*(nPar + 1)]*pc.f[j*(nPar + 1)]);
              for (i = 0; i < nPar; i++)
                pc.f[i + i*nPar] = sqrt(pc.f[i + i*nPar]);
              pc.f += nPar*nPar;
            }
          }
          pr.f += nPar;                 // get ready for the next one
          break;
        case LUX_DOUBLE:
          // get (X'X) in pl
          do {
            do {
              for (i = 0; i < n; i++) {
                *pl.d += *px.d * *p2.d * *pw.d * *pk.d;
                px.d += stride;
                p2.d += stride;
                if (dw)
                  pw.d += stride;
                pk.d += dk;
              }
              px.d -= bigstride;
              p2.d -= bigstride;
              if (dw)
                pw.d -= bigstride;
              pk.d -= n*dk;
              pl.d++;
            } while (xinfo.moveLoop(yinfo.rndim, 1) <= yinfo.rndim);
            xinfo.moveLoop(yinfo.rndim, -nPar);
          } while (p2info.moveLoop(yinfo.rndim, 1) <= yinfo.rndim);
          p2info.moveLoop(yinfo.rndim, -nPar);
          pl.d -= nPar*nPar;    // back to the start

          // get (X'y) in pr
          j = yinfo.naxes - 1;
          do {
            for (i = 0; i < n; i++) {
              *pr.d += *px.d * *py.d * *pw.d * *pk.d;
              px.d += stride;
              py.d += stride;
              if (dw)
                pw.d += stride;
              pk.d += dk;
            }
            pr.d++;
            px.d -= bigstride;
            py.d -= bigstride;
            if (dw)
              pw.d -= bigstride;
            pk.d -= n*dk;
          } while (xinfo.moveLoop(yinfo.rndim, 1) <= yinfo.rndim);
          xinfo.moveLoop(yinfo.rndim, -nPar);
          pr.d -= nPar;                 // back to the start
          d_decomp(pl.d, nPar, nPar); // LU decomposition of pl
          d_solve(pl.d, pr.d, nPar, nPar); // solve: pr gets parameters <a>

          if (chisq.d != NULL || err.d != NULL
              || (pc.d != NULL && (internalMode & 1))) { // need chi-square
            chi2 = f = norm = 0.0;
            // calculate the deviations from the fit
            k = nData*nRepeat;
            for (i = 0; i < n; i++) {
              value.d = *py.d;
              for (j = 0; j < nPar; j++)
                value.d -= pr.d[j]*px.d[j*k];
              chi2 += value.d*value.d * *pw.d * *pk.d;
              norm += *pk.d;
              f += *pw.d * *pk.d;
              px.d += stride;
              py.d += stride;
              if (dw)
                pw.d += stride;
              pk.d += dk;
            }
            px.d -= bigstride;
            py.d -= bigstride;
            if (dw)
              pw.d -= bigstride;
            pk.d -= n*dk;
            if (chisq.d != NULL)
              *chisq.d++ = chi2/norm;
            errv = sqrt(chi2/f);
            if (err.d)
              *err.d++ = errv;
          } else
            chi2 = 1.0;
          if ((internalMode & 1) == 0)
            chi2 = 1.0;
          if (pc.d) {           // get covariances
            for (i = 0; i < nPar; i++) {
              d_solve(pl.d, pc.d, nPar, nPar);
              pc.d += nPar;
            }
            pc.d -= nPar*nPar;  // back to the start
            if (internalMode & 1) {
              if (chisq.d != NULL)
                *chisq.d = f - 1;
              chi2 = chi2/(f - 1);
              for (i = 0; i < nPar*nPar; i++)
                pc.d[i] *= chi2;
            }
            if (internalMode & 2) {
              /* we transform the covariance matrix so its diagonal contains
                 standard errors, one triangle contains covariances, and
                 the other triangle contains correlation coefficients */
              for (i = 0; i < nPar; i++)
                for (j = 0; j < i; j++)
                  pc.d[i + j*nPar] /= sqrt(pc.d[i*(nPar + 1)]*pc.d[j*(nPar + 1)]);
              for (i = 0; i < nPar; i++)
                pc.d[i + i*nPar] = sqrt(pc.d[i + i*nPar]);
              pc.d += nPar*nPar;
            }
          }
          pr.d += nPar;                 // get ready for the next one
          break;
      }         // end of switch (type)

      // restore
      if (fwhm) {
        px.f -= i1;
        py.f -= i1;
        p2.f -= i1;
        if (dw)
          pw.f -= i1;
        pk.f -= nkernel - cc + i1;
      }
    } // end of for (cc = ...)
    yinfo.moveLoop(yinfo.naxes, 1);
  } while (xinfo.moveLoop(yinfo.naxes, 1) < yinfo.rndim);
  free(pl.f);
  return result;
}
//---------------------------------------------------------
int32_t lux_runprod(ArgumentCount narg, Symbol ps[])
// RUNPROD( data [, axis]) returns a running product of <data>
// along the indicated <axis>.  LS 13jul98
{
  int32_t       n, result;
  Scalar        value;
  Pointer       src, trgt;
  LoopInfo      srcinfo, trgtinfo;

  if (standardLoop(ps[0], narg > 1? ps[1]: 0,
                   SL_SAMEDIMS | SL_UPGRADE | SL_ONEAXIS | SL_NEGONED,
                   LUX_INT8,
                   &srcinfo, &src, &result, &trgtinfo, &trgt) == LUX_ERROR)
    return LUX_ERROR;

  switch (srcinfo.type) {
    case LUX_INT8:
      value.ui8 = 1;
      do {
        value.ui8 *= *src.ui8;
        *trgt.ui8 = value.ui8;
        n = trgtinfo.advanceLoop(&trgt.ui8), srcinfo.advanceLoop(&src.ui8);
        if (n)
          value.ui8 = 1;
      } while (n < srcinfo.rndim);
      break;
    case LUX_INT16:
      value.i16 = 1;
      do {
        value.i16 *= *src.i16;
        *trgt.i16 = value.i16;
        n = trgtinfo.advanceLoop(&trgt.ui8), srcinfo.advanceLoop(&src.ui8);
        if (n)
          value.i16 = 1;
      } while (n < srcinfo.rndim);
      break;
    case LUX_INT32:
      value.i32 = 1;
      do {
        value.i32 *= *src.i32;
        *trgt.i32 = value.i32;
        n = trgtinfo.advanceLoop(&trgt.ui8), srcinfo.advanceLoop(&src.ui8);
        if (n)
          value.i32 = 1;
      } while (n < srcinfo.rndim);
      break;
    case LUX_INT64:
      value.i64 = 1;
      do {
        value.i64 *= *src.i64;
        *trgt.i64 = value.i64;
        n = trgtinfo.advanceLoop(&trgt.ui8), srcinfo.advanceLoop(&src.ui8);
        if (n)
          value.i64 = 1;
      } while (n < srcinfo.rndim);
      break;
    case LUX_FLOAT:
      value.f = 1;
      do {
        value.f *= *src.f;
        *trgt.f = value.f;
        n = trgtinfo.advanceLoop(&trgt.ui8), srcinfo.advanceLoop(&src.ui8);
        if (n)
          value.f = 1;
      } while (n < srcinfo.rndim);
      break;
    case LUX_DOUBLE:
      value.d = 1;
      do {
        value.d *= *src.d;
        *trgt.d = value.d;
        n = trgtinfo.advanceLoop(&trgt.ui8), srcinfo.advanceLoop(&src.ui8);
        if (n)
          value.d = 1;
      } while (n < srcinfo.rndim);
      break;
  default:
    break;
  }
  return result;
}
//---------------------------------------------------------
