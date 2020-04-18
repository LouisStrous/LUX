/* This is file subsc.cc.

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
// File subsc.c
// LUX routines dealing with subscripts.
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdlib.h>
#include <string.h>
#include "action.hh"
#include "install.hh"
#include <limits.h>

extern int32_t  redim_warn_flag, range_warn_flag;
int32_t         lux_assoc_output(int32_t iq, int32_t jq, int32_t offsym, int32_t axsym),
  lux_file_output(int32_t iq, int32_t jq, int32_t offsym, int32_t axsym),
  lux_gmap(int32_t narg, int32_t ps[], Symboltype new_type), lux_assoc_input(int32_t, int32_t []),
  simple_reverse(int32_t *p1, int32_t *p2, int32_t n, int32_t type), getBody(int32_t),
  string_sub(int32_t, int32_t []);
//-------------------------------------------------------------------------
int32_t lux_inserter(int32_t narg, int32_t ps[])
/* syntax: insert,target,source [,origin]
   inserts <source> in <target>, starting at <target> coordinates
   <origin> (one or more).
   INSERT,t,s is equivalent to INSERT,t,s,0,0,...
   INSERT,t,s,n is equivalent to t(n) = s, with <t> and <s> regarded as
   1D arrays.   LS 10nov92 16sep98 22mar99 */
{
  LoopInfo      trgtinfo;
  Pointer       src, trgt;
  int32_t       *dims, ndim, i, offset[2*MAX_DIMS], doff, daxes, nelem;

  switch (symbol_class(ps[0])) {
    case LUX_ASSOC:
      doff = (narg > 2)? ps[2]: 0;
      daxes = (narg > 3)? ps[3]: 0;
      return lux_assoc_output(ps[0], ps[1], doff, daxes);
    case LUX_FILEMAP:
      doff = (narg > 2)? ps[2]: 0;
      daxes = (narg > 3)? ps[3]: 0;
      return lux_file_output(ps[0], ps[1], doff, daxes);
  }

  if (standardLoop(ps[0], 0,
                   (narg == 3? SL_TAKEONED: SL_ALLAXES) | SL_EACHROW,
                   LUX_INT8, &trgtinfo, &trgt, NULL, NULL, NULL) == LUX_ERROR)
    return LUX_ERROR;           // something wrong with the target

  if (numerical(ps[1], &dims, &ndim, &nelem, &src) == LUX_ERROR) // source
    return LUX_ERROR;

  if (narg > 3) {
    if (trgtinfo.ndim != ndim)
      return cerror(INCMP_ARG, ps[1]);
    if (narg != ndim + 2)
      return luxerror("Need one offset coordinate for each target dimension",
                   ps[2]);
    for (i = 0; i < ndim; i++)
      if (symbolIsRealScalar(ps[i + 2])) {
        offset[2*i] = int_arg(ps[i + 2]);
        offset[2*i + 1] = offset[2*i] + dims[i] - 1;
        if (offset[2*i] < 0 || offset[2*i + 1] >= trgtinfo.dims[i])
          return luxerror("Source extends beyond target in dimension %d",
                       ps[i + 2], i);
      } else
        return cerror(ILL_CLASS, ps[i + 2]);
  } else {
    if (symbolIsRealScalar(ps[2])) {
      offset[0] = int_arg(ps[2]);
      offset[1] = offset[0] + nelem - 1;
      if (offset[0] < 0 || offset[1] >= trgtinfo.nelem)
        return luxerror("Source extends beyond target", ps[2]);
    } else
      return cerror(ILL_CLASS, ps[2]);
  }

  trgtinfo.subdataLoop(offset);
  do {
    nelem = trgtinfo.rdims[0];
    switch (trgtinfo.type) {
      case LUX_INT8:
        switch (symbol_type(ps[1])) {
          case LUX_INT8:
            while (nelem--)
              *trgt.b++ = *src.b++;
            break;
          case LUX_INT16:
            while (nelem--)
              *trgt.b++ = *src.w++;
            break;
          case LUX_INT32:
            while (nelem--)
              *trgt.b++ = *src.l++;
            break;
          case LUX_INT64:
            while (nelem--)
              *trgt.b++ = *src.q++;
            break;
          case LUX_FLOAT:
            while (nelem--)
              *trgt.b++ = *src.f++;
            break;
          case LUX_DOUBLE:
            while (nelem--)
              *trgt.b++ = *src.d++;
            break;
          default:
            return cerror(ILL_TYPE, ps[1]);
        }
        break;
      case LUX_INT16:
        switch (symbol_type(ps[1])) {
          case LUX_INT8:
            while (nelem--)
              *trgt.w++ = *src.b++;
            break;
          case LUX_INT16:
            while (nelem--)
              *trgt.w++ = *src.w++;
            break;
          case LUX_INT32:
            while (nelem--)
              *trgt.w++ = *src.l++;
            break;
          case LUX_INT64:
            while (nelem--)
              *trgt.w++ = *src.q++;
            break;
          case LUX_FLOAT:
            while (nelem--)
              *trgt.w++ = *src.f++;
            break;
          case LUX_DOUBLE:
            while (nelem--)
              *trgt.w++ = *src.d++;
            break;
          default:
            return cerror(ILL_TYPE, ps[1]);
        }
        break;
      case LUX_INT32:
        switch (symbol_type(ps[1])) {
          case LUX_INT8:
            while (nelem--)
              *trgt.l++ = *src.b++;
            break;
          case LUX_INT16:
            while (nelem--)
              *trgt.l++ = *src.w++;
            break;
          case LUX_INT32:
            while (nelem--)
              *trgt.l++ = *src.l++;
            break;
          case LUX_INT64:
            while (nelem--)
              *trgt.l++ = *src.q++;
            break;
          case LUX_FLOAT:
            while (nelem--)
              *trgt.l++ = *src.f++;
            break;
          case LUX_DOUBLE:
            while (nelem--)
              *trgt.l++ = *src.d++;
            break;
          default:
            return cerror(ILL_TYPE, ps[1]);
        }
        break;
      case LUX_INT64:
        switch (symbol_type(ps[1])) {
          case LUX_INT8:
            while (nelem--)
              *trgt.q++ = *src.b++;
            break;
          case LUX_INT16:
            while (nelem--)
              *trgt.q++ = *src.w++;
            break;
          case LUX_INT32:
            while (nelem--)
              *trgt.q++ = *src.l++;
            break;
          case LUX_INT64:
            while (nelem--)
              *trgt.q++ = *src.q++;
            break;
          case LUX_FLOAT:
            while (nelem--)
              *trgt.q++ = *src.f++;
            break;
          case LUX_DOUBLE:
            while (nelem--)
              *trgt.q++ = *src.d++;
            break;
          default:
            return cerror(ILL_TYPE, ps[1]);
        }
        break;
      case LUX_FLOAT:
        switch (symbol_type(ps[1])) {
          case LUX_INT8:
            while (nelem--)
              *trgt.f++ = *src.b++;
            break;
          case LUX_INT16:
            while (nelem--)
              *trgt.f++ = *src.w++;
            break;
          case LUX_INT32:
            while (nelem--)
              *trgt.f++ = *src.l++;
            break;
          case LUX_INT64:
            while (nelem--)
              *trgt.f++ = *src.q++;
            break;
          case LUX_FLOAT:
            while (nelem--)
              *trgt.f++ = *src.f++;
            break;
          case LUX_DOUBLE:
            while (nelem--)
              *trgt.f++ = *src.d++;
            break;
          default:
            return cerror(ILL_TYPE, ps[1]);
        }
        break;
      case LUX_DOUBLE:
        switch (symbol_type(ps[1])) {
          case LUX_INT8:
            while (nelem--)
              *trgt.d++ = *src.b++;
            break;
          case LUX_INT16:
            while (nelem--)
              *trgt.d++ = *src.w++;
            break;
          case LUX_INT32:
            while (nelem--)
              *trgt.d++ = *src.l++;
            break;
          case LUX_INT64:
            while (nelem--)
              *trgt.d++ = *src.l++;
            break;
          case LUX_FLOAT:
            while (nelem--)
              *trgt.d++ = *src.f++;
            break;
          case LUX_DOUBLE:
            while (nelem--)
              *trgt.d++ = *src.d++;
            break;
          default:
            return cerror(ILL_TYPE, ps[1]);
        }
        break;
      default:
        return cerror(ILL_TYPE, ps[0]);
    }
  } while (trgtinfo.advanceLoop(&trgt) < trgtinfo.rndim);
  return LUX_OK;
}
//-------------------------------------------------------------------------
int32_t lux_smap(int32_t narg, int32_t ps[])
 // convert type (and class) to a string
{
  register int32_t n;
  register Pointer q1,q3;
  int32_t       nsym, nd, j, n1;
  int32_t       result_sym, size;

  nsym = ps[0];                                 //only one argument
  if (symbol_class(nsym) == LUX_STRING)
    return nsym;                //already a string
  //switch on the class
  switch (symbol_class(nsym))
  { case LUX_SCAL_PTR:          //scalar ptr
      n = 1;
      q1.b = scal_ptr_pointer(nsym).b;
      break;
    case LUX_SCALAR:            //scalar
      n = 1;
      q1.l = &scalar_value(nsym).l;
      break;
    case LUX_ARRAY:                     //array
      q1.l = (int32_t *) array_data(nsym);
      nd = array_num_dims(nsym);
      n = array_size(nsym);
      break;
    default:
      return cerror(ILL_CLASS, nsym);
    }
  n *= size = lux_type_size[symbol_type(nsym)];
  if (internalMode & 1)         // /TRUNCATE
  { j = 0;
    while (j < n && *q1.s)
    { j++;
      q1.s++; }
    q1.s -= j;
    n = j; }
  else if (internalMode & 2 && nd > 1) // /ARRAY
  { result_sym = array_scratch(LUX_TEMP_STRING, nd - 1, array_dims(nsym) + 1);
    q3.sp = (char**) array_data(result_sym);
    n1 = array_dims(nsym)[0]*size;
    j = n/n1;
    while (j--)
    { *q3.sp = (char *) Malloc(n1 + 1);
      memcpy(*q3.sp, q1.b, n1);
      q1.b += n1;
      (*q3.sp)[n1] = '\0';
      q3.sp++; }
    return result_sym; }
  result_sym = string_scratch(n);
  q3.s = string_value(result_sym);
  memcpy(q3.s, q1.b, n);
  q3.s[n] = '\0';
  return result_sym;
}
//-------------------------------------------------------------------------
int32_t lux_bmap(int32_t narg,int32_t ps[])
// convert type to uint8_t without changing memory contents
{
  return lux_gmap(narg, ps, LUX_INT8);
}
//-------------------------------------------------------------------------
int32_t lux_wmap(int32_t narg, int32_t ps[])
// convert type to uint8_t without changing memory contents
{
  return lux_gmap(narg, ps, LUX_INT16);
}
//-------------------------------------------------------------------------
int32_t lux_lmap(int32_t narg, int32_t ps[])
// convert type to long without changing memory contents
{
  return lux_gmap(narg, ps, LUX_INT32);
}
//-------------------------------------------------------------------------
int32_t lux_int64map(int32_t narg, int32_t ps[])
// convert type to long without changing memory contents
{
  return lux_gmap(narg, ps, LUX_INT64);
}
//-------------------------------------------------------------------------
int32_t lux_fmap(int32_t narg, int32_t ps[])
// convert type to float without changing memory contents
{
  return lux_gmap(narg, ps, LUX_FLOAT);
}
//-------------------------------------------------------------------------
int32_t lux_dmap(int32_t narg, int32_t ps[])
// convert type to double without changing memory contents
{
  return lux_gmap(narg, ps, LUX_DOUBLE);
}
//-------------------------------------------------------------------------
int32_t lux_cfmap(int32_t narg, int32_t ps[])
// convert type to cfloat without changing memory contents
{
  return lux_gmap(narg, ps, LUX_CFLOAT);
}
//-------------------------------------------------------------------------
int32_t lux_cdmap(int32_t narg, int32_t ps[])
// convert type to cdouble without changing memory contents
{
  return lux_gmap(narg, ps, LUX_CDOUBLE);
}
//-------------------------------------------------------------------------
int32_t lux_gmap(int32_t narg, int32_t ps[], Symboltype new_type)
                // general part for map routines
{
  register Pointer q1,q3;
  int32_t       nsym, nd, n, nn;
  int32_t       result_sym;
  Symboltype type;

  nsym = ps[0];                                 // only one argument
  type = symbol_type(nsym);
  // check if already the desired type
  if (type == new_type)
    return nsym;
  nd = 0;
  // switch on the class
  switch (symbol_class(nsym)) {
    case LUX_SCAL_PTR:
      n = lux_type_size[type];
      q1.b = scal_ptr_pointer(nsym).b;
      break;
    case LUX_SCALAR:
      n = lux_type_size[type];
      q1.l = &scalar_value(nsym).l;
      break;
    case LUX_CSCALAR:
      n = lux_type_size[type];
      q1.cf = complex_scalar_data(nsym).cf;
      break;
    case LUX_STRING:
      n = string_size(nsym);
      q1.s = string_value(nsym);
      break;
    case LUX_ARRAY: case LUX_CARRAY:
      nd = array_num_dims(nsym);
      n = array_size(nsym)*lux_type_size[type];
      q1.l = (int32_t*) array_data(nsym);
      /* n must be a multiple of type size or the inner dimension is not
         compatible */
      if ((array_dims(nsym)[0]*lux_type_size[type]) % lux_type_size[new_type])
        return luxerror("Array dimension 0 is incompatible with desired type",
                     ps[0]);
      break;
    default:
      return cerror(ILL_CLASS, nsym);
  }

  if (n % lux_type_size[new_type])      /* the new data type does not evenly divide
                                   the old data size */
    return luxerror("Data size is incompatible with desired type", ps[0]);

  // n is # of bytes to transfer, get # of elements
  nn = n/lux_type_size[new_type];
  if (nn <= 1) {                // at most one, make scalar
    result_sym = scalar_scratch(new_type);
    if (isComplexType(new_type)) {
      q3.cf = complex_scalar_data(result_sym).cf;
      symbol_class(result_sym) = LUX_CSCALAR;
    } else {
      q3.l = &scalar_value(result_sym).l;
      symbol_class(result_sym) = LUX_SCALAR;
    }
  } else {
    if (nd == 0) {              // input was not an array; output will be
      nd = 1;
      result_sym = array_scratch(new_type, nd, &nn);
    } else {                    /* input was an array; duplicate and then
                                   modify */
      result_sym = array_clone(nsym, type);
      array_type(result_sym) = new_type;
      if (isComplexType(new_type))
        symbol_class(result_sym) = LUX_CARRAY;
      else
        symbol_class(result_sym) = LUX_ARRAY;

         /*already know that inner dim is a proper multiple so safe to
                 do the following */
      array_dims(result_sym)[0] =
        (array_dims(result_sym)[0]*lux_type_size[type])/lux_type_size[new_type];
    }
    q3.l = (int32_t*) array_data(result_sym);
  }
  if (n > 0)
    memcpy(q3.b, q1.b, n);
  return result_sym;
}
 //-------------------------------------------------------------------------
void rearr(int32_t x[], int32_t rear[], int32_t nd)
 //rearrange subscript vectors
// added rear[] and nd parameters and scratch[] variable declaration to make
// lux_subsc_func recursive LS 30mar94
{
  static int32_t scratch[MAX_DIMS];
  int32_t       k;
  for (k=0;k<nd;k++) scratch[k] = x[ rear[k] ];
  for (k=0;k<nd;k++) x[k] = scratch[k];
}
//-------------------------------------------------------------------------
#define rint(value) ((int32_t) (value + ((value >= 0)? 0.5: -0.5)))
int32_t lux_reverse(int32_t narg, int32_t ps[])
/* y = REVERSE(x [, axes, center] [, /ZERO])
 reverse <x> in the indicated <axes>, around the specified <center>.
 if any elements would have to reverse into <y> from outside <x>, then
 set those values to zero if /ZERO is specified, or leave the original
 elements intact otherwise.  LS 11jan99 */
{
  LoopInfo      srcinfo, trgtinfo;
  Pointer       src, trgt, src2, trgt2, center;
  Scalar        value;
  int32_t       result, stride, i, iq, w, n, inplace;

  if (symbolIsNumerical(ps[0])) {
    if (standardLoop(ps[0], (narg > 1 && ps[1])? ps[1]: 0,
                     ((narg > 1 && ps[1])? 0: SL_TAKEONED) |
                     SL_UNIQUEAXES | SL_KEEPTYPE | SL_EACHROW,
                     LUX_INT8, &srcinfo, &src, &result, &trgtinfo, &trgt)
        == LUX_ERROR)
      return LUX_ERROR;

    if (narg > 2 && ps[2]) {    // <center>
      if (numerical(ps[2], NULL, NULL, &i, NULL) == LUX_ERROR)
        return LUX_ERROR;
      if (i != srcinfo.naxes)
        return cerror(INCMP_ARG, ps[2]);
      iq = lux_float(1, ps + 2);
      numerical(iq, NULL, NULL, NULL, &center);
    } else
      center.v = NULL;

    inplace = 0;                // 1 -> in-place reversal

    do {
      if (center.f) {
        /* NOTE: in gcc, rint() appears to be a macro; at least,
           rint(*center.f++) advanced center.f by two units instead of just
           one.  LS 11jan99 */
        w = rint(2**center.f);
        center.f++;
      } else
        w = srcinfo.rdims[0] - 1;
      stride = srcinfo.rsinglestep[0]*srcinfo.stride;
      if (w >= 0 && w < 2*(srcinfo.rdims[0] - 1)) // it's within bounds
        do {
          src2 = src;
          trgt2 = trgt;
          n = w - srcinfo.rdims[0] + 1;
          if (n >= 0) {                 // reversal at the far end
            i = n;
            if (inplace) {      // in-place reversal
              src.b += i*stride;
              if (internalMode & 1) // /ZERO
                while (i--) {
                  zerobytes(trgt.b, srcinfo.stride);
                  trgt.b += stride;
                }
            } else {            // target is different from source
              if ((internalMode & 1) == 0)
                while (i--) {
                  memcpy(trgt.b, src.b, srcinfo.stride);
                  trgt.b += stride;
                  src.b += stride;
                }
              else {            // /ZERO
                src.b += i*stride;
                while (i--) {
                  zerobytes(trgt.b, srcinfo.stride);
                  trgt.b += stride;
                } // end of while (i--)
              }         // end of if ((internalMode & 1) == 0) else
            } // end of if (inplace) else
            trgt.b = trgt2.b + stride*srcinfo.rdims[0];
          } else {              // n < 0 -> reversing at the near end
            trgt.b += srcinfo.rdims[0]*stride;
            src.b += srcinfo.rdims[0]*stride;
            n = -n;
            i = n;
            if (inplace) {
              if (internalMode & 1) // /ZERO
                while (i--) {
                  trgt.b -= stride;
                  zerobytes(trgt.b, srcinfo.stride);
                } // end of while (i--)
              else
                trgt.b -= i*stride;
            } else {            // not in place
              if ((internalMode & 1) == 0)
                while (i--) {
                  trgt.b -= stride;
                  src.b -= stride;
                  memcpy(trgt.b, src.b, srcinfo.stride);
                } // end of while (i--)
              else                      // /ZERO
                while (i--) {
                  trgt.b -= stride;
                  zerobytes(trgt.b, srcinfo.stride);
                } // end of while (i--)
            } // end of if (inplace) else
            src.b = src2.b;
          } // end of if (n >= 0) else
          i = srcinfo.rdims[0] - n;
          if (inplace) {
            i = i/2;
            while (i--) {
              trgt.b -= stride;
              memcpy(&value.b, trgt.b, srcinfo.stride);
              memcpy(trgt.b, src.b, srcinfo.stride);
              memcpy(src.b, &value.b, srcinfo.stride);
              src.b += stride;
            } // end of while (i--)
          } else                // not in place
            while (i--) {
              trgt.b -= stride;
              memcpy(trgt.b, src.b, srcinfo.stride);
              src.b += stride;
            } // end of while (i--)
          src.b = src2.b + stride*srcinfo.rdims[0];
          trgt.b = trgt2.b + stride*srcinfo.rdims[0];
        } while (trgtinfo.advanceLoop(&trgt),
                 srcinfo.advanceLoop(&src) < srcinfo.rndim);
      else {                    // center is out of bounds
        if (internalMode & 1) {         // /ZERO
          zerobytes(trgt.b, trgtinfo.nelem*trgtinfo.stride);
          return result;
        }
        if (!inplace)
          memcpy(trgt.b, src.b, trgtinfo.nelem*trgtinfo.stride);
      }         // end of if (w >= 0 && w < 2*(srcinfo.rdims[0] - 1)) else
      // "source" of next cycle was "target" of previous one
      srcinfo.data0 = trgtinfo.data0;
      inplace = 1;
    } while (nextLoops(&srcinfo, &trgtinfo));
  } else
    switch (symbol_class(ps[0])) {
      case LUX_STRING:
        src.s = string_value(ps[0]);
        n = string_size(ps[0]);
        result = string_scratch(n);
        trgt.s = string_value(result) + n;
        *--trgt.s = '\0';       // end of the string
        while (n--)
          *trgt.s-- = *src.s++;
        break;
      default:
        return cerror(ILL_CLASS, ps[0]);
  }
  return result;
}
//-------------------------------------------------------------------------
int32_t expandListArguments(int32_t *narg, int32_t *ps[], int32_t list, int32_t indx, int32_t freeList)
/* put the elements of the LIST/STRUCT <list> into <*ps> at
   index <indx>, replacing the symbol currently at that position.
   Recursively replaces LIST/STRUCTs in the expanded parts of the
   argument list, too.  Returns the number of expanded LIST/STRUCTURE
   elements.
   LS 14jun98 */
{
  int32_t       n, i, nlist, j, iq, k;

  nlist = 1;
  iq = -list;
  for (j = 0; j < nlist; j++) {
    if (iq < 0)
      iq = -iq;
    else
      iq = (*ps)[indx + j];
    switch (symbol_class(iq)) {
      case LUX_CLIST: case LUX_CPLIST:
        n = clist_num_symbols(iq);
        *narg += n - 1;                 /* minus one because the slot now occupied
                                 by <list> can be occupied by its first
                                 element instead */
        *ps = (int32_t*) realloc(*ps, *narg*sizeof(int32_t)); // more memory as needed
        if (!*ps)               // some allocation error occurred
          return cerror(ALLOC_ERR, iq);
        /* move the stuff that is after <list> to make room for the elements
           of <list> */
        memmove(*ps + indx + n, *ps + indx + 1,
                (*narg - indx - n)*sizeof(int32_t));
        // now insert the elements of <iq>
        for (i = 0; i < n; i++) {
          k = (*ps)[indx + i] = clist_symbols(iq)[i];
          /* we must update the context if the list is a free variable,
             otherwise the ex-members of the list are zapped when the
             list itself is.  However, we must leave CLISTs and LISTs
             alone because otherwise they are not zapped when their
             encompassing CLIST or LIST is. */
          if (freeList
              && symbol_class(k) != LUX_LIST
              && symbol_class(k) != LUX_CLIST
              && symbol_class(k) != LUX_CPLIST
              && symbol_context(k) == iq)
            symbol_context(k) = -compileLevel;
        }
        nlist += n - 1;
        j--;
        break;
      case LUX_LIST:
        n = list_num_symbols(iq);
        *narg += n - 1;
        *ps = (int32_t*) realloc(*ps, *narg*sizeof(int32_t));
        if (!*ps)
          return cerror(ALLOC_ERR, iq);
        memmove(*ps + indx + n, *ps + indx, (*narg - indx - n)*sizeof(int32_t));
        for (i = 0; i < n; i++) {
          k = (*ps)[indx + i] = list_symbol(iq, i);
          if (freeList
              && symbol_class(k) != LUX_LIST
              && symbol_class(k) != LUX_CLIST
              && symbol_class(k) != LUX_CPLIST
              && symbol_context(k) == iq)
            symbol_context(k) = -compileLevel;
        }
        nlist += n - 1;
        j--;
        break;
    }
  }
  return nlist;                         // done
}
 //-------------------------------------------------------------------------
int32_t treatListArguments(int32_t *narg, int32_t *ps[], int32_t offset)
// expands appropriate LIST/STRUCT arguments in the argument list
/* this routine takes argument list <*ps> and checks if any of the
   <*narg> arguments is a compact list with one element which is itself
   a compact list.  Such arguments are replaced by the contents of the
   contained compact list -- recursively, so that no compact lists with
   just a single element feature in the final result.  For example,
   suppose that the argument list is equal to  x, y, {{a, b, c}}, z;
   i.e., has 4 arguments.  Then this routine replaces that argument list
   by x, y, a, b, c, z: six arguments.  LS 28oct98 */
/* after this routine is through, the treated compact list is no longer
   part of the argument list, and, if it is a temporary variable,
   would therefore not be appropriately deleted.  We must explicitly zap
   the list if it is a temporary variable.  When a compact list is
   zapped, its temporary-variable elements get zapped, too, unless
   we assign them to a different context. */
{
  int32_t       i, iq, n, i1, i2, list;

  if (offset >= 0) {
    i1 = offset;
    i2 = 0;
  } else {
    i1 = 0;
    i2 = offset;
  }
  for (i = i1; i < *narg + i2; ) {      // check all arguments
    if ((*ps)[i])               // there is an argument in this slot
      switch (symbol_class((*ps)[i])) {
        case LUX_CLIST: case LUX_CPLIST:
          if (clist_num_symbols((*ps)[i]) == 1) { /* one element -> may be
                                                appropriate */
            list = (*ps)[i];
            iq = clist_symbols(list)[0];
            if (iq)             // the one element is not empty
              switch (symbol_class(iq)) {
                case LUX_CLIST: case LUX_LIST: case LUX_CPLIST:
                  n = expandListArguments(narg, ps, iq, i, isFreeTemp(list));
                  if (isFreeTemp(list))
                    /* we must delete this <list> symbol because it is
                       now no longer part of the argument list and
                       will therefore not be zapped through that
                       membership. */
                    zap(list);
                  if (n >= 0)
                    i += n;
                  else
                    return n;   // some error
                  break;
                default:
                  i++;
              }
            else
              i++;
          } else
            i++;
          break;
        case LUX_LIST:
          if (list_num_symbols((*ps)[i]) == 1) {
            list = (*ps)[i];
            iq = list_symbol((*ps)[i], 0);
            if (iq)
              switch (symbol_class(iq)) {
                case LUX_CLIST: case LUX_LIST: case LUX_CPLIST:
                  n = expandListArguments(narg, ps, iq, i, isFreeTemp(list));
                  if (isFreeTemp(list))
                    zap(list);
                  if (n >= 0)
                    i += n;
                  else
                    return n;   // some error
                  break;
                default:
                  i++;
              }
            else
              i++;
          } else
            i++;
          break;
        default:
          i++;
          break;
      } else
        i++;
  }
  return 1;
}
//-------------------------------------------------------------------------
int32_t expandListArguments_w(int32_t *narg, int16_t *ps[], int32_t list, int32_t indx,
                          int32_t freeList)
/* put the elements of the LIST/STRUCT <list> into <*ps> at
   index <indx>, replacing the symbol currently at that position.
   Recursively replaces LIST/STRUCTs in the expanded parts of the
   argument list, too.  Returns the number of expanded LIST/STRUCTURE
   elements.
   LS 14jun98 */
{
  int32_t       n, i, nlist, j, iq, k;

  nlist = 1;
  iq = -list;
  for (j = 0; j < nlist; j++) {
    if (iq < 0)
      iq = -iq;
    else
      iq = (*ps)[indx + j];
    switch (symbol_class(iq)) {
      case LUX_CLIST: case LUX_CPLIST:
        n = clist_num_symbols(iq);
        *narg += n - 1;                 /* minus one because the slot now occupied
                                 by <list> can be occupied by its first
                                 element instead */
        *ps = (int16_t*) realloc(*ps, *narg*sizeof(int16_t)); // more memory as needed
        if (!*ps)               // some allocation error occurred
          return cerror(ALLOC_ERR, iq);
        /* move the stuff that is after <list> to make room for the elements
           of <list> */
        memmove(*ps + indx + n, *ps + indx + 1,
                (*narg - indx - n)*sizeof(int16_t));
        // now insert the elements of <iq>
        for (i = 0; i < n; i++) {
          k = (*ps)[indx + i] = clist_symbols(iq)[i];
          /* we must update the context if the list is a free variable,
             otherwise the ex-members of the list are zapped when the
             list itself is.  However, we must leave CLISTs and LISTs
             alone because otherwise they are not zapped when their
             encompassing CLIST or LIST is. */
          if (freeList
              && symbol_class(k) != LUX_LIST
              && symbol_class(k) != LUX_CLIST
              && symbol_class(k) != LUX_CPLIST
              && symbol_context(k) == iq)
            symbol_context(k) = -compileLevel;
        }
        nlist += n - 1;
        j--;
        break;
      case LUX_LIST:
        n = list_num_symbols(iq);
        *narg += n - 1;
        *ps = (int16_t*) realloc(*ps, *narg*sizeof(int16_t));
        if (!*ps)
          return cerror(ALLOC_ERR, iq);
        memmove(*ps + indx + n, *ps + indx, (*narg - indx - n)*sizeof(int16_t));
        for (i = 0; i < n; i++) {
          k = (*ps)[indx + i] = list_symbol(iq, i);
          if (freeList
              && symbol_class(k) != LUX_LIST
              && symbol_class(k) != LUX_CLIST
              && symbol_class(k) != LUX_CPLIST
              && symbol_context(k) == iq)
            symbol_context(k) = -compileLevel;
        }
        nlist += n - 1;
        j--;
        break;
    }
  }
  return nlist;                         // done
}
//-------------------------------------------------------------------------
int32_t treatListArguments_w(int32_t *narg, int16_t *ps[], int32_t offset)
// expands appropriate LIST/STRUCT arguments in the argument list
/* this routine takes argument list <*ps> and checks if any of the
   <*narg> arguments is a compact list with one element which is itself
   a compact list.  Such arguments are replaced by the contents of the
   contained compact list -- recursively, so that no compact lists with
   just a single element feature in the final result.  For example,
   suppose that the argument list is equal to  x, y, {{a, b, c}}, z;
   i.e., has 4 arguments.  Then this routine replaces that argument list
   by x, y, a, b, c, z: six arguments.  LS 28oct98 */
/* after this routine is through, the treated compact list is no longer
   part of the argument list, and, if it is a temporary variable,
   would therefore not be appropriately deleted.  We must explicitly zap
   the list if it is a temporary variable.  When a compact list is
   zapped, its temporary-variable elements get zapped, too, unless
   we assign them to a different context. */
{
  int32_t       i, iq, n, i1, i2, list;

  if (offset >= 0) {
    i1 = offset;
    i2 = 0;
  } else {
    i1 = 0;
    i2 = offset;
  }
  for (i = i1; i < *narg + i2; ) {      // check all arguments
    if ((*ps)[i])               // there is an argument in this slot
      switch (symbol_class((*ps)[i])) {
        case LUX_CLIST: case LUX_CPLIST:
          if (clist_num_symbols((*ps)[i]) == 1) { /* one element -> may be
                                                appropriate */
            list = (*ps)[i];
            iq = clist_symbols(list)[0];
            if (iq)             // the one element is not empty
              switch (symbol_class(iq)) {
                case LUX_CLIST: case LUX_LIST: case LUX_CPLIST:
                  n = expandListArguments_w(narg, ps, iq, i, isFreeTemp(list));
                  if (isFreeTemp(list))
                    /* we must delete this <list> symbol because it is
                       now no longer part of the argument list and
                       will therefore not be zapped through that
                       membership. */
                    zap(list);
                  if (n >= 0)
                    i += n;
                  else
                    return n;   // some error
                  break;
                default:
                  i++;
              }
            else
              i++;
          } else
            i++;
          break;
        case LUX_LIST:
          if (list_num_symbols((*ps)[i]) == 1) {
            list = (*ps)[i];
            iq = list_symbol((*ps)[i], 0);
            if (iq)
              switch (symbol_class(iq)) {
                case LUX_CLIST: case LUX_LIST: case LUX_CPLIST:
                  n = expandListArguments_w(narg, ps, iq, i, isFreeTemp(list));
                  if (isFreeTemp(list))
                    zap(list);
                  if (n >= 0)
                    i += n;
                  else
                    return n;   // some error
                  break;
                default:
                  i++;
              }
            else
              i++;
          } else
            i++;
          break;
        default:
          i++;
          break;
      } else
        i++;
  }
  return 1;
}
//-------------------------------------------------------------------------
#define UNDEFINED       0
#define INNER   1
#define OUTER   2

int32_t lux_subsc_func(int32_t narg, int32_t ps[])
// decipher and extract subscripted values from a symbol; supports arrays,
// strings, associated variables, file maps, and function pointers.
// rewritten LS 6aug96
{
  int32_t       nsym, i, n, iq, ndim, *dims, start[MAX_DIMS], j, nsum,
        size[MAX_DIMS], todim[MAX_DIMS], *index[MAX_DIMS], nelem, *iptr,
        trgtndim, trgtdims[MAX_DIMS], fromdim[MAX_DIMS], one = 1, offset0,
        stride[MAX_DIMS], offset, width, tally[MAX_DIMS], step[MAX_DIMS],
        noutdim, ps2[MAX_DIMS], narr, combineType;
  Symboltype type;
  Symbolclass class_id;
  int16_t       *ap;
  Pointer       src, trgt;
  wideScalar    value, item;
  uint8_t       subsc_type[MAX_DIMS], sum[MAX_DIMS];
  listElem      *le;
  FILE  *fp = NULL;
  int32_t       lux_subsc_subgrid(int32_t, int32_t []);
  int32_t lux_endian(int32_t, int32_t []);

  nsym = ps[--narg];
  /* now nsym is the source (subscripted) symbol and narg is the number
   of subscripts.  The first argument is ps[0] */

  /* we want to treat LUX_CLISTs and LUX_LISTs in the argument list
   not as single arguments but as sets of arguments.  I.e.,
   FOO(arg1, arg2) = FOO(arg1, {{ arg2a, arg2b }}) = FOO( arg1, arg2a, arg2b)
   if arg2 = { arg2a, arg2b }.  We require two sets of braces to
   distinguish the case where the list is to be interpreted as a set of
   arguments and the case where the list is itself an argument.
   LS 14jun98 */
  if (treatListArguments(&narg, &ps, 0) == LUX_ERROR) /* treat subscripts
                                                       but not target */
    return LUX_ERROR;           // some error

  if (internalMode & 8)                 // /SUBGRID
    return lux_subsc_subgrid(narg, ps);

  class_id = symbol_class(nsym);        // source symbol class

  switch (class_id) {           // switch on source class
    default:
      return cerror(SUBSC_NO_INDX, nsym);
    case LUX_CLIST: case LUX_CPLIST:
      nelem = clist_num_symbols(nsym);
      ndim = 1;
      dims = &nelem;
      break;
    case LUX_RANGE:
      nelem = 2;
      ndim = 1;
      dims = &nelem;
      break;
    case LUX_LIST:
      nelem = list_num_symbols(nsym);
      ndim = 1;
      dims = &nelem;
      break;
    case LUX_ASSOC:             //assoc. var. case
      // assoc variables come here but are handed off to files.c
      iq = lux_assoc_input(narg, ps);
      return iq;
    case LUX_FUNC_PTR:          //func. ptr. case
      {
        int32_t itype;

        if ((iq = func_ptr_routine_num(nsym)) > 0) { // user-defined
          if (symbol_class(iq) == LUX_DEFERRED_FUNC) {
            // it's a deferred function, so its body has not yet been
            // compiled.  Do it now.
            if (getBody(iq) < 0)        // some error during compilation
              return LUX_ERROR;
          }
          if (symbol_class(iq) != LUX_FUNCTION)
            return luxerror("Subscripted pointer to non-function!", nsym);
          itype = 1;            // user-defined
        } else {                        // internal function or routine
          iq = -iq;             // get function number
          if (iq >= nFunction)
            return luxerror("Pointer to non-existent internal function!", nsym);
          itype = 0;            // internal
        }
        // now create proper symbol to handle the situation
        getFreeTempExecutable(n);
        if (itype) {            // a user-defined function
          symbol_class(n) = LUX_USR_FUNC;
          usr_func_number(n) = iq;
        } else {                        // an internal function
          symbol_class(n) = LUX_INT_FUNC;
          int_func_number(n) = iq;
        }
        symbol_memory(n) = narg*sizeof(int16_t);
        if (narg) {             // arguments
          ALLOCATE(symbol_data(n), narg, int16_t *);
          ap = (int16_t*) symbol_data(n);
          while (narg--)
            *ap++ = *ps++;      // ap is int16_t* and ps is int32_t*, so we cannot
                                // just use memcpy().
        }
        iq = eval(n);
        zap(n);                         // delete temp executable
      }
      return iq;
    case LUX_STRING:            // string case
      iq = string_sub(narg, ps);
      return iq;
    case LUX_FILEMAP:
      if ((fp = fopen(expand_name(file_map_file_name(nsym), NULL), "r"))
          == NULL) {
        printf("File %s: ", expname);
        perror("System message");
        return cerror(ERR_OPEN, nsym);
      }
      ndim = file_map_num_dims(nsym);
      dims = file_map_dims(nsym);
      nelem = file_map_size(nsym);
      type = file_map_type(nsym);
      break;
    case LUX_ARRAY: case LUX_CARRAY:
      src.l = (int32_t *) array_data(nsym);
      ndim = array_num_dims(nsym);
      dims = array_dims(nsym);
      nelem = array_size(nsym);
      type = symbol_type(nsym);
      break;
    case LUX_CSCALAR:
      src.cf = complex_scalar_data(nsym).cf;
      ndim = 1;
      dims = &one;
      nelem = 1;
      type = complex_scalar_type(nsym);
      break;
    case LUX_SCALAR:
      src.l = &scalar_value(nsym).l;
      ndim = 1;
      dims = &one;
      nelem = 1;
      type = symbol_type(nsym);
      break;
    } //end of switch on class

  // OK argument lists:
  // one argument for each dimension
  // or just one argument
  /* or one argument for the first few dimensions, with /ZERO (4) or
       /ALL (32) */
  if (narg > ndim               // too many subscripts
      || ((internalMode & 36) == 0 // or /ZERO, /ALL not specified
          && narg > 1           // and more than 1 subscript
          && narg != ndim)) {   // and # subscripts unequal to # dims
    cerror(ILL_N_SUBSC, nsym);
    goto lux_subsc_1;
  }

  // only get here if source is a numerical or string array, a scalar,
  // a file map, or a list.

  if (narg == 1                         // one subscript
      && (internalMode & 0x60) == 0) { // no /ALL or /SEPARATE
                                // -> treat source as 1D
    dims = &nelem;
    ndim = 1;
  }

  // how do we combine different subscripts if they contain multiple values?
  // we can choose between inner-style, in which for each output element
  // we take the next value from all subscripts, and outer-style, in which
  // all possible combinations of values (taking one from each subscript)
  // are serviced.  For example, (X,Y,Z,/INNER), with X, Y, Z arrays with
  // 100 elements each, produces 100 values, whereas (X,Y,Z,/OUTER)
  // yields 100*100*100 = 1,000,000 values.
  switch (internalMode & 3) {
    case 0:
      combineType = UNDEFINED;  // wasn't explicitly specified
      break;
    case 1:                     // /INNER
      combineType = INNER;
      break;
    case 2:                     // /OUTER
      combineType = OUTER;
      break;
    case 3:                     // both /INNER and /OUTER
      luxerror("Specified incompatible /INNER and /OUTER", 0);
      goto lux_subsc_1;
  }

  if ((internalMode & 64)       // /SEPARATE
      && (narg > 1              // more than one argument
          || !symbolIsNumericalArray(ps[0]) // or not a numerical array
          || array_size(ps[0]) > ndim)) { // or wrong # dimensions
    luxerror("Improper subscript for /SEPARATE", ps[0]);
    goto lux_subsc_1;
  }

  nsum = narr = 0;
  // check all subscripts for legality as far as possible
  // collect the number of elements covered by each subscript (size),
  // the start index (start), the redirection dimension (todim),
  // the summation flag (sum), and the subscript type (LUX_ARRAY
  // or LUX_RANGE)
  for (i = 0; i < narg; i++) { // all subscripts
    iq = ps[i];                         // next subscript
    switch (symbol_class(iq)) {         // subscript class
      case LUX_SCAL_PTR: case LUX_SCALAR:
        start[i] = int_arg(iq);         // start index (scalar value)
        if (narg == 1)          // only a single subscript: address
                                // the array as if it were 1D
          width = nelem;
        else
          width = dims[i];
        if (start[i] < 0 || start[i] >= width) { // check if within range
          cerror(ILL_SUBSC, iq, start[i], width);
          goto lux_subsc_1;
        }
        size[i] = 1;            // size (1 element)
        todim[i] = -1;          // no associated redirection dimension
        sum[i] = 0;             // no summation
        subsc_type[i] = LUX_RANGE; // subscript type
        break;
      case LUX_ARRAY:
        iq = lux_long(1, &iq);  // get LONG indices
        n = size[i] = array_size(iq); // number of array elements
        if (internalMode & 64) { // /SEPARATE
          /* the elements of the current array are regarded as
           individual scalar subscripts rather than as members of a
           single array subscript.  If we get here, then we already
           know that the array is the only one, and has a number of
           elements not exceeding the number of dimensions of the source.
           LS 22jul98 */
          iptr = (int32_t*) array_data(iq);
          for (i = 0; i < n; i++) {
            if (*iptr < 0 || *iptr >= dims[i]) { // out of bounds
              cerror(ILL_SUBSC, ps[1], *iptr, dims[i]);
              goto lux_subsc_1;
            }
            start[i] = *iptr++;         // start index
            size[i] = 1;        // range size
            todim[i] = -1;      // no associated redirection dimension
            subsc_type[i] = LUX_RANGE; // subscript type
          }
          narg = n;
          i = narg;
        } else {                // no /SEPARATE
          if (n == 1) {                 // only one element: mimic scalar
            start[i] = *(int32_t *) array_data(iq);
            if (narg == 1)      // single subscript only
              width = nelem;
            else
              width = dims[i];
            if (start[i] < 0 || start[i] >= width) {
              cerror(ILL_SUBSC, iq, start[i], width);
              goto lux_subsc_1;
            }
            subsc_type[i] = LUX_RANGE;
          } else {              // multiple elements in this subscript
            iptr = index[i] = (int32_t *) array_data(iq);
            if (narg == 1)      // only a single subscript; addresses
                                // whole source array as if it were 1D
              width = nelem;
            else
              width = dims[i];
            while (n--) {       // check if all subscripts are within range
              if (*iptr < 0 || *iptr >= width) {
                cerror(ILL_SUBSC, iq, *iptr, width);
                goto lux_subsc_1;
              }
              iptr++;
            }
            subsc_type[i] = LUX_ARRAY;
            narr++;
            ps2[i] = iq;        // index symbol number
          }
          todim[i] = -1;
          sum[i] = 0;
          // if the subscript is a multi-dimensional array and no
          // subscript combination type was explicitly specified, then
          // we go for inner-style, to prevent accidental requests for
          // humongous output arrays.
          if (combineType == UNDEFINED && array_num_dims(iq) > 1)
            combineType = INNER;
        }
        break;
      case LUX_RANGE:
        if (!range_scalar(iq)) {
          // check for nested ranges, e.g., ((3:4):>5).
          j = iq;
          while (!range_scalar(j) && symbol_class(j) != LUX_ARRAY) {
            if (range_end(j) != LUX_ZERO) { // invalid nested range
              cerror(ILL_SUBSC_TYPE, iq);
              goto lux_subsc_1;
            }
            j = range_start(j);
          }
          // now take index information from symbol j and the redirection
          // information from symbol iq (which may be the same)
          switch (symbol_class(j)) {
            case LUX_ARRAY:
              j = lux_long(1, &j);      // ensure LONG indices
              n = size[i] = array_size(j);
              iptr = index[i] = (int32_t *) array_data(j);
              while (n--) {     // check if all subscripts are within range
                if (*iptr < 0 || *iptr >= dims[i]) {
                  cerror(ILL_SUBSC, iq, *iptr, dims[i]);
                  goto lux_subsc_1;
                }
                iptr++;
              }
              ps2[i] = j;       // index symbol number
              subsc_type[i] = LUX_ARRAY;
              narr++;
              break;
            case LUX_RANGE:
              j = convertRange(j); // convert to subscript pointer
              start[i] = subsc_ptr_start(j);
              if (start[i] < 0)         // count from end of range
                start[i] += dims[i];
              if (start[i] < 0 || start[i] >= dims[i]) { // outside range
                cerror(RANGE_START, iq, start[i], dims[i]);
                goto lux_subsc_1;
              }
              size[i] = subsc_ptr_end(j); // endpoint
              if (size[i] < 0)  // count from end of range
                size[i] += dims[i];
              if (size[i] < start[i] || size[i] >= dims[i]) { // outside
                cerror(RANGE_END, iq, size[i], dims[i]);
                goto lux_subsc_1;
              }
              size[i] += 1 - start[i]; // now we have the size
              subsc_type[i] = LUX_RANGE;
              break;
          }
          j = range_redirect(iq); // redirection symbol
          if (j >= 0) {                 // -1: no symbol specified
            j = eval(j);
            if (symbol_class(j) != LUX_SCALAR) { // must be a scalar
              cerror(ILL_REARRANGE, iq);
              goto lux_subsc_1;
            }
            j = int_arg(j);     // use the integer part
            if (j < 0) {        // negative axes don't exist
              cerror(ILL_REARRANGE, iq);
              goto lux_subsc_1;
            }
          }
          if (j >= ndim) {      // beyond range of dimensions
            cerror(ILL_REARRANGE, iq);
            goto lux_subsc_1;
          }
          todim[i] = j;                 // redirection, if any
          sum[i] = range_sum(iq); // summation, if any
          if (sum[i])           // this dimension is flagged for summation
            nsum++;             // count the number of summed dimensions
          else if (subsc_type[i] == LUX_ARRAY
                   && combineType == UNDEFINED
                   && array_num_dims(ps2[i]) > 1)
            // with multi-dimensional arrays for subscripts, we
            // use inner-style combination as the default, to prevent
            // accidental requests for giant return arrays.
            combineType = INNER;
          break;
        }
        // get here if it's a scalar range
        iq = convertRange(iq);  // convert to subscript pointer
        if (iq < 0)             // some error occurred
          return iq;
        // fall-thru
      case LUX_SUBSC_PTR:
        start[i] = subsc_ptr_start(iq);
        if (start[i] < 0)       // count from end of range
          start[i] += dims[i];
        if (start[i] < 0 || start[i] >= dims[i]) {
          cerror(RANGE_START, iq, start[i], dims[i]);
          goto lux_subsc_1;
        }
        size[i] = subsc_ptr_end(iq); // endpoint
        if (size[i] < 0)        // count from end of range
          size[i] += dims[i];
        if (size[i] < start[i] || size[i] >= dims[i]) {
          cerror(RANGE_END, iq, size[i], dims[i]);
          goto lux_subsc_1;
        }
        size[i] += 1 - start[i]; // now we have the size
        j = subsc_ptr_redirect(iq);
        if (j > ndim) {
          cerror(ILL_REARRANGE, iq);
          goto lux_subsc_1;
        }
        todim[i] = j;
        sum[i] = subsc_ptr_sum(iq);
        if (sum[i])
          nsum++;
        subsc_type[i] = LUX_RANGE;
        break;
      default:                  // some unsupported class
        cerror(ILL_SUBSC_TYPE, iq);
        goto lux_subsc_1;
      }
  }

  // if we get here and the combination type is still not specified,
  // then we are sure not to have multi-dimensional arrays for any
  // of the subscripts, and then we'll assume that the user wants
  // outer-style combination, compatible with ancient LUX practice
  // from the days when only ranges or scalars could be used in
  // multiple-coordinate subscripts.
  if (combineType == UNDEFINED)
    combineType = OUTER;

  if (narg != ndim)             // we have fewer subscripts than dimensions
                                // (narg > ndim was checked earlier)
    switch (internalMode & 36) {// /ALL or /ZERO?
      case 32:                  // /ALL: omitted dimensions are assumed (*)
        for (i = narg; i < ndim; i++) {
          start[i] = 0;
          size[i] = dims[i];
          todim[i] = -1;
          sum[i] = 0;
          subsc_type[i] = LUX_RANGE;
        }
        narg = ndim;
        break;
      case 4:                   // /ZERO: omitted dimensiions are assumed 0
        for (i = narg; i < ndim; i++) {
          start[i] = 0;
          size[i] = 1;
          todim[i] = -1;
          sum[i] = 0;
          subsc_type[i] = LUX_RANGE;
        }
        narg = ndim;
        break;
      default:                  // none of the above: error
        cerror(ILL_N_SUBSC, ps[0]);
        goto lux_subsc_1;
    }

  // now we are ready to start the extraction

  switch (class_id) {
    case LUX_CLIST:
      if (size[0] == 1)         // want only a single element
        return clist_symbols(nsym)[start[0]];
      // else we return a CLIST with the indicated number of elements
      iq = nextFreeTempVariable();
      if (iq == LUX_ERROR)
        goto lux_subsc_1;
      symbol_class(iq) = class_id;
      n = size[0]*sizeof(int16_t);
      ap = clist_symbols(iq) = (int16_t*) malloc(n);
      if (!clist_symbols(iq)) {
        cerror(ALLOC_ERR, iq);
        goto lux_subsc_1;
      }
      symbol_memory(iq) = n;
      switch (subsc_type[0]) {
        case LUX_RANGE:
          memcpy(ap, clist_symbols(nsym) + start[0], size[0]*sizeof(int16_t));
          break;
        case LUX_ARRAY:
          n = size[0];
          iptr = index[0];
          while (n--)
            *ap++ = clist_symbols(nsym)[*iptr++];
          break;
      }
      return iq;
    case LUX_CPLIST:
      if (size[0] == 1) {
        /* we want only a single element: if it is a named variable,
           then we make it a TRANSFER, otherwise we return it as is. */
        n = clist_symbols(nsym)[start[0]];
        if (symbolIsNamed(n)) {
          iq = nextFreeTempVariable();
          if (iq == LUX_ERROR)
            goto lux_subsc_1;
          symbol_class(iq) = LUX_TRANSFER;
          transfer_is_parameter(iq) = (Symboltype) 0;
          transfer_target(iq) = n;
        } else
          iq = n;
      } else {
        iq = nextFreeTempVariable();
        if (iq == LUX_ERROR)
          goto lux_subsc_1;
        symbol_class(iq) = class_id;
        n = size[0]*sizeof(int16_t);
        ap = clist_symbols(iq) = (int16_t*) malloc(n);
        if (!clist_symbols(iq)) {
          cerror(ALLOC_ERR, iq);
          goto lux_subsc_1;
        }
        symbol_memory(iq) = n;
        switch (subsc_type[0]) {
          case LUX_RANGE:
            memcpy(ap, clist_symbols(nsym) + start[0], size[0]*sizeof(int16_t));
            break;
          case LUX_ARRAY:
            n = size[0];
            iptr = index[0];
            while (n--)
              *ap++ = clist_symbols(nsym)[*iptr++];
            break;
        }
      }
      return iq;
    case LUX_LIST:
      if (size[0] == 1)         // want only a single element
        return list_symbols(nsym)[start[0]].value;
      // else we return a CLIST with the indicated number of elements
      iq = nextFreeTempVariable();
      if (iq == LUX_ERROR)
        goto lux_subsc_1;
      symbol_class(iq) = LUX_LIST;
      n = size[0]*sizeof(listElem);
      le = list_symbols(iq) = (listElem*) malloc(n);
      if (!list_symbols(iq)) {
        cerror(ALLOC_ERR, iq);
        goto lux_subsc_1;
      }
      symbol_memory(iq) = n;
      switch (subsc_type[0]) {
        case LUX_RANGE:
          memcpy(le, clist_symbols(nsym) + start[0], size[0]*sizeof(listElem));
          break;
        case LUX_ARRAY:
          n = size[0];
          iptr = index[0];
          while (n--)
            *le++ = list_symbols(nsym)[*iptr++];
          break;
      }
      return iq;
    case LUX_RANGE:
      if (size[0] == 1)
        return start[0]? range_end(nsym): range_start(nsym);
      // else we want both: return the range itself
      return nsym;
  }

  // we get here if the source is a numerical or string array or a
  // file map
  if (combineType == INNER) {
    // generalized inner product:
    // x([1,2],[3,4],/inner) = [x(1,3),x(2,4)]
    // the output is an array with the same dimensional structure
    // as the first array subscript of x, or a scalar.
    // the subscripts that are arrays must all have the same number
    // of elements, but the dimensional structures need not be the
    // same.  LS 29jan97
    noutdim = 0;                // number of output dimensions
    n = -1;                     // index of first non-scalar subscript
    offset0 = 0;                // default offset, includes scalar
                                // subscripts and range starts
    for (i = 0; i < narg; i++) {
      if (sum[i]) {
        luxerror("Sorry, array summation not implemented for /INNER",
              ps[i + 1]);
        goto lux_subsc_1;
      }
      if (todim[i] >= 0) {
        luxerror("Sorry, redirection not implemented for /INNER",
              ps[i + 1]);
        goto lux_subsc_1;
      }
      if (size[i] > 1) {        // multiple elements
        if (n >= 0) {           // already have an earlier multiple
          if (size[i] != size[n]) { // sizes do not agree
            cerror(INCMP_DIMS, ps[i + 1]);
            goto lux_subsc_1;
          }
        }
        else
          n = i;
      }
    }

    // now create output symbol
    if (n >= 0) {               // output array
      if (subsc_type[n] == LUX_ARRAY)
        iq = array_scratch(type, array_num_dims(ps[n]),
                           array_dims(ps[n]));
      else                      // must be a range
        iq = array_scratch(type, 1, &size[n]);
      nelem = array_size(iq);
      trgt.l = (int32_t *) array_data(iq);
    } else {
      iq = scalar_scratch(type);
      nelem = 1;
      trgt.l = &scalar_value(iq).l;
    }

    // for LUX_ARRAY subscripts the offset is calculated for each index;
    // for LUX_RANGE subscripts the offset is updated for each element
    offset0 = 0;
    width = lux_type_size[type];
    if ((nsum && class_id != LUX_FILEMAP)
        || (class_id == LUX_ARRAY && type == LUX_STRING_ARRAY))
      n = 1;
    else
      n = width;
    for (i = 0; i < narg; i++) {
      stride[i] = n;
      if (subsc_type[i] == LUX_RANGE)
        offset0 += start[i]*n;  // update initial offset
      n *= dims[i];
    }

    switch (class_id) {
      case LUX_ARRAY: case LUX_SCALAR:
        if (class_id == LUX_ARRAY && type == LUX_STRING_ARRAY) {
          for (j = 0; j < nelem; j++) {
            offset = offset0;
            for (i = 0; i < narg; i++)
              if (subsc_type[i] == LUX_ARRAY)
                offset += index[i][j]*stride[i];
              else if (size[i] > 1) // range
                offset0 += stride[i];
            if (!(*trgt.sp = (char*) malloc(strlen(src.sp[offset]) + 1))) {
              cerror(ALLOC_ERR, 0);
              goto lux_subsc_1;
            }
            strcpy(*trgt.sp++, src.sp[offset]);
          }
        } else {
          for (j = 0; j < nelem; j++) {
            offset = offset0;
            for (i = 0; i < narg; i++)
              if (subsc_type[i] == LUX_ARRAY)
                offset += index[i][j]*stride[i];
              else if (size[i] > 1) // range
                offset0 += stride[i];
            memcpy(trgt.b, src.b + offset, width);
            trgt.b += width;
          }
        }
        break;
      case LUX_FILEMAP:
        if (file_map_has_offset(nsym))
          offset0 += file_map_offset(nsym);
        for (j = 0; j < nelem; j++) {
          offset = offset0;
          for (i = 0; i < narg; i++)
            if (subsc_type[i] == LUX_ARRAY)
              offset += index[i][j]*stride[i];
            else if (size[i] > 1) // range
              offset0 += stride[i];
          if (fseek(fp, offset, SEEK_SET)) {
            perror("System message");
            cerror(POS_ERR, nsym);
            goto lux_subsc_1;
          }
          if (fread(trgt.b, width, 1, fp) != 1) {
            if (feof(fp))
              puts("Encountered EOF");
            else
              perror("System message");
            cerror(READ_ERR, nsym);
            goto lux_subsc_1;
          }
          trgt.b += width;
        }
        break;
      }
  } else {                              // generalized outer product:
    // x([1,2],[3,4],/outer) = x([1,2],[3,4]) =
    // [x(1,3),x(1,4),x(2,3),x(2,4)]
    // now have all the info about the subscripts.  figure out how big
    // the result is, and if redirections (if any) are legal.
    trgtndim = noutdim = 0;
    for (i = 0; i < MAX_DIMS; i++)
      fromdim[i] = -1;          // target dimension i comes from source
                                // dimension fromdim[i]

    for (i = 0; i < narg; i++) { // check all subscripts
      if ((size[i] > 1 && !sum[i]) // this dim yields more than one element
          || todim[i] >= 0) {   // or redirection
        if (subsc_type[i] == LUX_ARRAY) // index subscript
          n = array_num_dims(ps2[i]); // all array dims go to result
        else                    // range subscript
          n = 1;                // this subscript yields one dimension
        if (todim[i] >= 0) {    // redirection specified for this dim
          if (fromdim[todim[i]] >= 0) { // source dimension already claimed
            cerror(ILL_REARRANGE, ps[i + 1]);
            goto lux_subsc_1;
          } // end of if (fromdim[todim[i]] >= 0)
          fromdim[todim[i]] = i; // source of target dimension
        } // end of if (todim[i] >= 0)
        else                    // no redirection for this dimension
          fromdim[trgtndim] = i; // stays in current dimension
        step[i] = n;            // # dimensions in the current subscript
        noutdim += n;           // total number of output dimensions
        trgtndim++;             // total number of non-scalar subscripts
      }         // end of if (size[i] > 1 && !sum[i])
      else if (internalMode & 16) { // this yields a dimension equal
                                    // to 1, and we have /KEEPDIMS,
                                    // so we should keep that dimension
                                    // in the output, too
        if (todim[i] >= 0) {        // redirection specified for this dim
          if (fromdim[todim[i]] >= 0) {         // target dimension already claimed
            cerror(ILL_REARRANGE, ps[i + 1]);
            goto lux_subsc_1;
          } // end of if (fromdim[todim[i]] >= 0)
          fromdim[todim[i]] = i; // source of target dimension
        } // end of if (todim[i] >= 0)
        else                    // no redirection for this dimension
          fromdim[trgtndim] = i; // stays in current dimension
        step[i] = 1;            // adds 1 dimension
        noutdim++;              // total number of output dimensions
        trgtndim++;             // total number of non-scalar subscripts
      }
      // check that we haven't gotten too many dimensions
      if (noutdim >= MAX_DIMS) {
        luxerror("Too many dimensions in subscript return",
              ps[i + 1]);
        goto lux_subsc_1;
      } // end of if (noutdim + n >= MAX_DIMS)
    }

    // now create the output symbol
    n = 0;                      // current output dimension index
    if (noutdim) {              // need array: collect dimensions
      for (i = 0; i < trgtndim; i++) {
        j = fromdim[i];                 // (first) source dimension
        if (subsc_type[j] == LUX_ARRAY && size[j] > 1 && !sum[j]) {
          // a multi-dimensional array was specified as a subscript
          // without summation; copy the dimensions to the output symbol
          memcpy(trgtdims + n, array_dims(ps2[j]), step[j]*sizeof(int32_t));
          n += step[j];
        } else if (!sum[j])     // simple argument: one dimension extra
          trgtdims[n++] = size[j];
        else                    // summation: dimension of 1
          trgtdims[n++] = 1;
      }         // end of for (i = 0...)
      iq = array_scratch(type, noutdim, trgtdims);
      trgt.l = (int32_t *) array_data(iq);
    } // end of if (noutdim)
    else if (class_id == LUX_ARRAY && type == LUX_STRING_ARRAY) { // string from string array
      if (src.sp[start[0]]) {   // non-null string
        n = strlen(src.sp[start[0]]);
        iq = string_scratch(n);
        strcpy(string_value(iq), src.sp[start[0]]);
      } else {                  // null string
        iq = string_scratch(0);
        *string_value(iq) = '\0';
      }
      return iq;
    } else {                    // scalar output
      iq = scalar_scratch(type);
      if (isComplexType(type))
        trgt.cf = complex_scalar_data(iq).cf;
      else
        trgt.l = &scalar_value(iq).l;
    }

    // now we have the output symbol ready for use, with the data
    // to start at trgt.v.

    // everything is OK now for going through the target dimensional
    // structure, but if we're summing in any dimensions, then we must
    // also loop over the data to be summed, so we modify the info
    // accordingly.  We must put the dimensions to be summed over
    // at the beginning so that we can work on one sum at a time and
    // don't need to keep track of many intermediate values.
    if (nsum) {                         // we're summing
      if ((internalMode & 16) == 0) { // we're not keeping all dimensions
        // first we move the non-summed dimensions to the back
        for (i = trgtndim - 1; i >= 0; i--)
          fromdim[i + nsum] = fromdim[i];
        // then we add in the dimensions to be summed over
        n = 0;
        for (i = 0; i < narg; i++)
          if (sum[i])
            fromdim[n++] = i;
        // and finally we update the number of dimensions to loop over
        trgtndim += nsum;
      } else {                  // we're keeping all dimensions;
        // in this case we must only make sure that the dimensions to
        // be summed over are listed first.
        n = 0;
        for (i = 0; i < narg; i++)
          if (sum[fromdim[i]])  // first the summed dimensions
            stride[n++] = fromdim[i];
        for (i = 0; i < narg; i++)
          if (!sum[fromdim[i]])         // and then the non-summed ones
            stride[n++] = fromdim[i];
        memcpy(fromdim, stride, narg*sizeof(int32_t));
      }
    }
    // for LUX_ARRAY subscripts the offset is calculated for each index;
    // for LUX_RANGE subscripts the offset is updated for each element
    offset0 = 0;                // initial offset
    width = lux_type_size[type];
    if ((nsum && class_id != LUX_FILEMAP)
        || (class_id == LUX_ARRAY && type == LUX_STRING_ARRAY))
      n = 1;
    else
      n = width;
    for (i = 0; i < narg; i++) {
      stride[i] = n;            // step size for each dimension
      if (subsc_type[i] == LUX_RANGE)
        offset0 += start[i]*n;  // update initial offset
      tally[i] = 0;             // current coordinate
      n *= dims[i];
    }
    if (trgtndim) {             // more than one output value
      // we must rearrange the data in order of the target dimensions
      // first the number of elements in each target dimension
      for (i = 0; i < trgtndim; i++)
        ps2[i] = size[fromdim[i]];
      memcpy(size, ps2, trgtndim*sizeof(int32_t));
      // then the step sizes to go through each target dimension
      for (i = 0; i < trgtndim; i++)
        ps2[i] = stride[fromdim[i]];
      memcpy(stride, ps2, trgtndim*sizeof(int32_t));
      // then the subscript types
      for (i = 0; i < trgtndim; i++)
        ps2[i] = subsc_type[fromdim[i]];
      for (i = 0; i < trgtndim; i++)
        subsc_type[i] = ps2[i];
      // now we calculate the step size for each target dimensions,
      // including a full complement of all earlier dimensions
      if (subsc_type[0] == LUX_RANGE)
        step[0] = stride[0];
      else
        step[0] = 0;
      for (i = 1; i < trgtndim; i++)
        step[i] = (subsc_type[i] == LUX_RANGE? stride[i]: 0)
          - (subsc_type[i - 1] == LUX_RANGE? size[i - 1]*stride[i - 1]: 0);
    } // end of if (trgtndim)
    else                        // a scalar output
      step[0] = width;

    if (!nsum && step[0] == width && trgtndim && !step[1]) {
      // we don't need to read each value in turn, but can treat blocks
      // of values at a time, effectively reducing the number of
      // dimensions in the data
      width *= size[0];
      for (i = 1; i < trgtndim; i++) {
        if (step[i] || subsc_type[i] == LUX_ARRAY)
          break;
        width *= size[i];
      }
      // now <i> indicates the number of dimensions at the beginning
      // that can be treated as a single dimension.  We adjust the
      // info accordingly.
      memmove(step, step + i, (trgtndim - i)*sizeof(int32_t));
      memmove(size, size + i, (trgtndim - i)*sizeof(int32_t));
      memmove(stride, stride + i, (trgtndim - i)*sizeof(int32_t));
      memmove(subsc_type, subsc_type + i, trgtndim - i);
      memmove(fromdim, fromdim + i, (trgtndim - i)*sizeof(int32_t));
      step[0] += width;
      trgtndim -= i;
    }

    switch (class_id) {
      case LUX_ARRAY: case LUX_SCALAR:
      case LUX_CARRAY: case LUX_CSCALAR:
        if (class_id == LUX_ARRAY && type == LUX_STRING_ARRAY) {
          do
            { offset = offset0;
              for (i = 0; i < trgtndim; i++) // add LUX_ARRAY indices
                if (subsc_type[i] == LUX_ARRAY)
                  offset += index[fromdim[i]][tally[i]]*stride[i];
              if (src.sp[offset])
                *trgt.sp++ = strsave(src.sp[offset]);
              else
                *trgt.sp++ = NULL;
              for (i = 0; i < trgtndim; i++)
                { if (i)
                    tally[i - 1] = 0;
                  offset0 += step[i];   // update for LUX_RANGE subscripts
                  tally[i]++;
                  if (tally[i] != size[i])
                    break; }
            } while (i != trgtndim);
        } else {
          if (nsum) {           // have summation flag(s)
            zerobytes(&value.b, lux_type_size[LUX_CDOUBLE]); // initialize
            do {
              offset = offset0;
              for (i = 0; i < trgtndim; i++) // add LUX_ARRAY indices
                if (subsc_type[i] == LUX_ARRAY)
                  offset += index[fromdim[i]][tally[i]]*stride[i];
              switch (type) {
              case LUX_INT8:
                value.b += src.b[offset];
                break;
              case LUX_INT16:
                value.w += src.w[offset];
                break;
              case LUX_INT32:
                value.l += src.l[offset];
                break;
              case LUX_INT64:
                value.q += src.l[offset];
                break;
              case LUX_FLOAT:
                value.f += src.f[offset];
                break;
              case LUX_DOUBLE:
                value.d += src.d[offset];
                break;
              case LUX_CFLOAT:
                value.cf.real += src.cf[offset].real;
                value.cf.imaginary += src.cf[offset].imaginary;
                break;
              case LUX_CDOUBLE:
                value.cd.real += src.cd[offset].real;
                value.cd.imaginary += src.cd[offset].imaginary;
                break;
              }
              for (i = 0; i < trgtndim; i++) { // update coordinates
                if (i)
                  tally[i - 1] = 0;
                offset0 += step[i]; // update for LUX_RANGE subscripts
                tally[i]++;
                if (tally[i] != size[i]) // not yet done with this dimension
                  break;
                if (i == nsum - 1) { // done with this one
                  memcpy(trgt.b, &value.b, width); // store result
                  trgt.b += width;
                  value.d = 0.0;
                }
              } // end of for (i = 0)
            } while (i != trgtndim);
          } else                        // no summation: simple copy will do
            do {
              offset = offset0;
              for (i = 0; i < trgtndim; i++) // add LUX_ARRAY indices
                if (subsc_type[i] == LUX_ARRAY)
                  offset += index[fromdim[i]][tally[i]]*stride[i];
              memcpy(trgt.b, src.b + offset, width);
              trgt.b += width;
              for (i = 0; i < trgtndim; i++) {
                if (i)
                  tally[i - 1] = 0;
                offset0 += step[i]; // update for LUX_RANGE subscripts
                tally[i]++;
                if (tally[i] != size[i])
                  break;
              }
            } while (i != trgtndim);
        }
        break;
      case LUX_FILEMAP:
        if (file_map_has_offset(nsym))
          offset0 += file_map_offset(nsym);
        if (nsum)               // have summation flag(s)
        { value.d = 0.0;        // ASSUME THIS ZEROS FOR ALL DATA TYPES
          do
          { offset = offset0;
            for (i = 0; i < narg; i++)  // add LUX_ARRAY indices
              if (subsc_type[i] == LUX_ARRAY)
                offset += index[i][tally[i]]*stride[i];
            if (fseek(fp, offset, SEEK_SET)) {
              perror("System message");
              cerror(POS_ERR, nsym);
              goto lux_subsc_1;
            }
            if (fread(&item.b, width, 1, fp) != 1) {
              if (feof(fp))
                puts("Encountered EOF");
              else
                perror("System message");
              cerror(READ_ERR, iq);
              goto lux_subsc_1;
            }
            switch (type) {
              case LUX_INT8:
                value.b += item.b;
                break;
              case LUX_INT16:
                value.w += item.w;
                break;
              case LUX_INT32:
                value.l += item.l;
                break;
              case LUX_INT64:
                value.q += item.q;
                break;
              case LUX_FLOAT:
                value.f += item.f;
                break;
              case LUX_DOUBLE:
                value.d += item.d;
                break;
            }
            for (i = 0; i < trgtndim; i++) {
              if (i)
                tally[i - 1] = 0;
              offset0 += step[i]; // update for LUX_RANGE subscripts
              tally[i]++;
              if (tally[i] != size[i])
                break;
              if (i == nsum - 1) {
                memcpy(trgt.b, &value.b, width);
                trgt.b += width;
                value.d = 0.0;
              }
            }
          } while (i != trgtndim);
          fclose(fp);
        } else                  // no summation: simple copy will do
          do {
            offset = offset0;
            for (i = 0; i < narg; i++)  // add LUX_ARRAY indices
              if (subsc_type[i] == LUX_ARRAY)
                offset += index[i][tally[i]]*stride[i];
            if (fseek(fp, offset, SEEK_SET)) {
              perror("System message");
              cerror(POS_ERR, nsym);
              goto lux_subsc_1;
            }
            if (fread(trgt.b, width, 1, fp) != 1) {
              if (feof(fp))
                puts("Encountered EOF");
              else
                perror("System message");
              cerror(READ_ERR, nsym);
              goto lux_subsc_1;
            }
            trgt.b += width;
            for (i = 0; i < trgtndim; i++) {
              if (i)
                tally[i - 1] = 0;
              offset0 += step[i]; // update for LUX_RANGE subscripts
              tally[i]++;
              if (tally[i] != size[i])
                break;
            }
          } while (i != trgtndim);
        fclose(fp);
        break;
    }
  }
  int32_t lux_endian(int32_t, int32_t *);
  if (class_id == LUX_FILEMAP && file_map_swap(nsym))
    lux_endian(1, &iq);                 // Byte swap
  return iq;

  lux_subsc_1:
  if (fp)
    fclose(fp);
  return LUX_ERROR;
}                                               // end of lux_subsc
 //-------------------------------------------------------------------------
int32_t string_sub(int32_t narg, int32_t ps[])
// subscripts for strings, called by lux_subc
/* the string is in ps[narg]; narg indicates the number of subscripts.
 LS 19aug98 */
{
  // this is a subset of cases for arrays since strings are 1-D
  int32_t       iq, n, result_sym, ns, nsym, i;
  char  *p, *q;
  Pointer p2;

  if (narg != 1)                // only one subscript allowed
    return cerror(N_STR_DIMS_OVR, 0);
  nsym = ps[narg];              // the string
  p = string_value(nsym);
  n = string_size(nsym);
  iq = ps[0];                   // only one subscript
  switch (symbol_class(iq)) {
    case LUX_SCALAR:    //a scalar, simple, get a long (int32_t) version
      i = int_arg(iq);
      if (i < 0)
        i += n;         // (*-expr)
      if (i >= n || i < 0) {
        printf(" index: %1d;  string length: %1d\n", i, n);
        return cerror(SUBSC_RANGE, iq);
      }
                                /* pointers don't work too well for
                                   strings because various parts of code
                                   expect strings to be null terminated,
                                   so for now we just create a single
                                   element string, this means that an
                                   element can't be loaded in a read
                                   statement like an array element */
      // return  create_str_sub_ptr(nsym,p,i);
      result_sym = string_scratch(1);
      q = string_value(result_sym);
      *q++ = *(p + i);
      *q = 0;
      return result_sym;
    case LUX_ARRAY:
      // a string is created with length = # elements in array
      iq = lux_long(1, &iq);    // get a long version
      p2.l = (int32_t *) array_data(iq);
      ns = array_size(iq);
      result_sym = string_scratch(ns);
      q = string_value(result_sym);
      while (ns) {
        i = *p2.l++;
        if (i < 0) i += n;      // (*-expr)
        if ( i >= n || i < 0 )
        { cerror(SUBSC_RANGE, iq);
          printf(" index: %1d;  size: %1d\n", i, n);
          return -1; }
        *q++ = *( (p++) + i );
        ns--; }         *q = 0;
      return result_sym;
    case LUX_RANGE:
      iq = convertRange(iq);
    case LUX_SUBSC_PTR:                 // not all aspects supported for strings
      // info is directly at .spec.array.ptr now. LS
      i = subsc_ptr_start(iq);
      if (i < 0) i = n + i;
      if ( i >= n || i < 0 )
      { cerror(SUBSC_RANGE, iq);
        printf(" start index: %1d;  size: %1d\n", i, n);
        return -1; }
      ns = subsc_ptr_end(iq);
      if (ns < 0) ns = n + ns - i + 1; // number of chars
      else
      { if ( ns >= n || ns < i )
        { cerror(SUBSC_RANGE, iq);
          printf(" end index: %1d;  start index: %1d;  size: %1d\n", ns, i, n);
          return -1; }
        ns = ns - i + 1; }
                 // if the compression or rearrange are set, flag an error

      if (subsc_ptr_sum(iq) || subsc_ptr_redirect(iq) != -1)
      { cerror(ILL_W_STR, iq);
        return -1; }
      result_sym = string_scratch(ns);
      p += i;
      q = string_value(result_sym);
      while (ns) {
        *q++ = *p++;
        ns--; }
      *q = 0;
      return result_sym;
    }
  return 1;
}
 //-------------------------------------------------------------------------
int32_t lux_symclass(int32_t narg, int32_t ps[])
 //return the class of the argument symbol
 /* NOTE: scalar pointers (class 8) are returned as scalars (class 1)
    LS 13may92 */
{
  int32_t       nsym, nd, result_sym;

  if (internalMode & 2)
    nsym = int_arg(ps[0]);
  else
    nsym = (internalMode & 1)? eval(ps[0]): ps[0]; // target symbol
  result_sym = scalar_scratch(LUX_INT32);
  if (nsym < 0 || nsym >= NSYM)
    nd = LUX_UNUSED;
  else
    nd = symbol_class(nsym);
  if (nd == LUX_SCAL_PTR)
    nd = LUX_SCALAR;
  scalar_value(result_sym).l = nd;
  return result_sym;
}
 //-------------------------------------------------------------------------
int32_t lux_symdtype(int32_t narg, int32_t ps[])
 //return the type of the argument symbol
{
  int32_t       nsym, result_sym;

  nsym = ps[0];                                 //the target symbol is the first
  result_sym = scalar_scratch(LUX_INT32);
  int32_t t = sym[nsym].type;
  switch (sym[nsym].type) {
  case LUX_LSTRING: case LUX_STRING_ARRAY:
    t = LUX_TEMP_STRING;
    // no break
  default:
    sym[result_sym].spec.scalar.l = t;
  }
  return result_sym;
}
 //-------------------------------------------------------------------------
int32_t lux_num_elem(int32_t narg, int32_t ps[])
/* return the number of elements in the argument symbol, or the number
   contained within the specified dimensions.
   call:  NUM_ELEM(x [, dims]) */
{
  int32_t       nsym, n, j, result_sym, *dims, *axes, naxes, ndim, temp;

  nsym = ps[0];                                 //the target symbol is the first
  result_sym = scalar_scratch(LUX_INT32);
  if (narg > 1) {               // have <dims>
    temp = lux_long(1, ps + 1);         // ensure LONG
    switch (symbol_class(temp)) {
      case LUX_ARRAY:
        axes = (int32_t*) array_data(temp);
        naxes = array_size(temp);
        break;
      case LUX_SCALAR:
        axes = &scalar_value(temp).l;
        naxes = 1;
        break;
      default:
        zapTemp(temp);
        return cerror(ILL_CLASS, ps[1]);
    }
  } else
    naxes = 0;
  switch (symbol_class(nsym)) {         //switch on class
    case LUX_ARRAY: case LUX_CARRAY:
      if (naxes) {
        n = 1;
        dims = array_dims(nsym);
        ndim = array_num_dims(nsym);
        for (j = 0; j < naxes; j++) {
          if (axes[j] < 0 || axes[j] >= ndim)
            return cerror(SUBSC_RANGE, ps[1]);
          n *= dims[axes[j]];
        }
      } else
        n = array_size(nsym);
      break;
    case LUX_FILEMAP:
      n = 1;
      dims = file_map_dims(nsym);
      ndim = file_map_num_dims(nsym);
      if (naxes)
        for (j = 0; j < naxes; j++) {
          if (axes[j] < 0 || axes[j] >= ndim)
            return cerror(SUBSC_RANGE, ps[1]);
          n *= dims[axes[j]];
        }
      else
        for (j = 0; j < ndim; j++)
          n *= dims[j];
      break;
    case LUX_STRING:
      n = string_size(nsym);    //don't include the null
      break;
    case LUX_SCALAR: case LUX_SCAL_PTR: case LUX_CSCALAR:
      n = 1;                    //scalar, just return 1
      break;
    case LUX_CLIST: case LUX_CPLIST:
      n = clist_num_symbols(nsym);
      break;
    case LUX_LIST:
      n = list_num_symbols(nsym);
      break;
    default:
      return cerror(ILL_CLASS, nsym);
  }
  scalar_value(result_sym).l = n;
  if (naxes)
    zapTemp(temp);
  return result_sym;
}
//-------------------------------------------------------------------------
int32_t lux_num_dimen(int32_t narg, int32_t ps[])
 //return the number of dimensions in the argument symbol
{
  int32_t       nsym, nd, result_sym;

  array         *h;
  nsym = ps[0];                                 //the target symbol is the first
  result_sym = scalar_scratch(LUX_INT32);
  switch (symbol_class(nsym))   {                       //switch on class
    case LUX_ARRAY: case LUX_FILEMAP:
        h = (array *) sym[nsym].spec.array.ptr;
        nd = h->ndim;
        break;
      case LUX_STRING:
        nd = 1;                                         //return 1 for strings
        break;
      case LUX_SCALAR: nd = 0;                          //scalar, just return 0
        break;
      default:
        return cerror(ILL_CLASS, nsym);
  }
  sym[result_sym].spec.scalar.l = nd;
  return result_sym;
}
 //-------------------------------------------------------------------------
int32_t lux_dimen(int32_t narg, int32_t ps[])
// DIMEN(x [,axes]) returns the indicated dimensions of <x>.  If <x> is
// a scalar or a string, then a scalar 1 is returned.  If <x> has only
// one dimension, then that is returned in a scalar.  If <axes> is not
// specified, then all dimensions are returned.  LS 19may98
{
  int32_t       nAxes, iq, *out, i, ndim, *dims;
  Pointer       axes;

  if (narg > 1) {               // have <axes>
    if (!symbolIsNumerical(ps[1]))
      return cerror(ILL_TYPE, ps[1]);
    iq = lux_long(1, ps + 1);
    numerical(iq, NULL, NULL, &nAxes, &axes);
  } else
    nAxes = 0;

  switch (symbol_class(ps[0])) { // <x>
    case LUX_SCAL_PTR: case LUX_SCALAR: case LUX_STRING:
      for (i = 0; i < nAxes; i++)
        if (axes.l[i] != 0)
          return cerror(ILL_AXIS, ps[1], axes.l[i]);
      if (nAxes > 1) {
        iq = array_scratch(LUX_INT32, 1, &nAxes);
        out = (int32_t *) array_data(iq);
        while (nAxes--)
          *out++ = 1;
      } else {
        iq = scalar_scratch(LUX_INT32);
        scalar_value(iq).l = 1;
      }
      break;
    case LUX_ARRAY: case LUX_FILEMAP: case LUX_CARRAY:
      ndim = array_num_dims(ps[0]);
      dims = array_dims(ps[0]);
      for (i = 0; i < nAxes; i++)
        if (axes.l[i] < 0 || axes.l[i] >= ndim)
          return cerror(ILL_AXIS, ps[1], axes.l[i]);
      if (nAxes > 1) {
        iq = array_scratch(LUX_INT32, 1, &nAxes);
        out = (int32_t *) array_data(iq);
        while (nAxes--)
          *out++ = dims[*axes.l++];
      } else if (nAxes == 1) {
        iq = scalar_scratch(LUX_INT32);
        scalar_value(iq).l = dims[*axes.l];
      } else {                  // return all dimensions
        if (ndim > 1) {
          iq = array_scratch(LUX_INT32, 1, &ndim);
          memcpy((int32_t*) array_data(iq), dims, ndim*sizeof(int32_t));
        } else {
          iq = scalar_scratch(LUX_INT32);
          scalar_value(iq).l = dims[0];
        }
      }
      break;
    default:
      return cerror(ILL_CLASS, ps[0]);
  }
  return iq;
}
 //-------------------------------------------------------------------------
int32_t lux_redim_f(int32_t narg, int32_t ps[])
 // redimension an array, a function version, returns input symbol
{
  int32_t       result, n, *args;
  int32_t       lux_redim(int32_t, int32_t []);

  args = (int32_t*) Malloc(narg*sizeof(int32_t));
  if (!args)
    return LUX_ERROR;
  result = copySym(ps[0]);
  memcpy(args, ps, narg*sizeof(int32_t));
  args[0] = result;
  n = lux_redim(narg, args);
  Free(args);
  if (n == LUX_ERROR)
    zap(result);
  else
    n = result;
  return n;
}
 //-------------------------------------------------------------------------
int32_t lux_redim(int32_t narg, int32_t ps[])
     // redimension an array
     // NOTE: my array_size() calculates the number of elements in an
     // array from the size of the allocated memory and this assumes that
     // the array memory size fits the array data perfectly.  Shine's
     // version of lux_redim changed the dimensions without changing
     // the amount of allocated memory accordingly.  I've now changed
     // this behavior and I hope loose-fit memory does not cause problems
     // for me elsewhere.  LS 19may97
{
  extern int32_t        redim_warn_flag;
  int32_t       oldSize, newSize, ndim, dims[MAX_DIMS], i, iq, nsym;
  array         *data;

  nsym = ps[0];                         // the target symbol is the first
  if (symbol_class(nsym) != LUX_ARRAY) // not an array
    return cerror(NEED_ARR, nsym);
  // get old size
  oldSize = array_size(nsym);
  ndim = 0;
  for (i = 1; i < narg; i++)
    switch (symbol_class(ps[i])) {
      case LUX_SCALAR:
        if (ndim + 1> MAX_DIMS)
          return luxerror("Too many dimensions specified", ps[i]);
        iq = lux_long(1, &ps[i]); // ensure LONG
        dims[ndim++] = scalar_value(iq).l;
        break;
      case LUX_ARRAY:
        if (ndim + array_size(ps[i]) > MAX_DIMS)
          return luxerror("Too many dimensions specified", ps[i]);
        iq = lux_long(1, &ps[i]);
        memcpy(dims + ndim, (int32_t*) array_data(iq),
               array_size(iq)*sizeof(int32_t));
        ndim += array_size(iq);
        break;
      default:
        return cerror(ILL_CLASS, ps[1]);
    }
  newSize = 1;                  // calculate new size
  for (i = 0; i < ndim; i++)
    newSize *= dims[i];
  if (newSize > oldSize)
    return cerror(ARR_SMALL, nsym);
  else if (newSize < oldSize && redim_warn_flag)
    printf("WARNING: result from REDIM is smaller than original\n");
  memcpy(array_dims(nsym), dims, ndim*sizeof(int32_t));
  array_num_dims(nsym) = ndim;
  newSize = newSize*lux_type_size[array_type(nsym)] + sizeof(array);
  data = (array*) Realloc(array_header(nsym), newSize);
  if (!data)
    return luxerror("Resizing of array memory failed", nsym);
  array_header(nsym) = data;
  symbol_memory(nsym) = newSize;
  return 1;
}
 //-------------------------------------------------------------------------
int32_t lux_concat_list(int32_t narg, int32_t ps[])
// concatenation involving LUX_CLIST or LUX_LIST. LS 15jun98
{
  int32_t       result, i, iq, nelem = 0, j, indx;
  uint8_t       isStruct = 0;

  if ((result = nextFreeTempVariable()) == LUX_ERROR)
    return LUX_ERROR;

  for (i = 0; i < narg; i++) {
    iq = ps[i];
    switch (symbol_class(iq)) {
      case LUX_CLIST:
        nelem += clist_num_symbols(iq);
        break;
      case LUX_LIST:
        nelem += list_num_symbols(iq);
        isStruct = 1;
        break;
      case LUX_UNUSED: case LUX_UNDEFINED:
        break;
      default:
        nelem++;
        break;
    }
  }
  // we now have the number of elements of the resulting STRUCT or LIST

  if (isStruct) {               // result is an LUX_LIST
    symbol_class(result) = LUX_LIST;
    list_symbols(result) = (listElem*) Malloc(nelem*sizeof(listElem));
    if (!list_symbols(result))
      return cerror(ALLOC_ERR, 0);
    symbol_memory(result) = nelem*sizeof(listElem);
  } else {
    symbol_class(result) = LUX_CLIST;
    clist_symbols(result) = (int16_t*) Malloc(nelem*sizeof(int16_t));
    if (!clist_symbols(result))
      return cerror(ALLOC_ERR, 0);
    symbol_memory(result) = nelem*sizeof(int16_t);
  }

  indx = 0;
  if (isStruct) {               // result is a structure
    for (i = 0; i < narg; i++) {
      iq = ps[i];
      switch (symbol_class(iq)) {
        case LUX_CLIST:
          nelem = clist_num_symbols(iq);
          for (j = 0; j < nelem; j++) {
            list_key(result, indx) = NULL;
            list_symbol(result, indx) =
              copyEvalSym(clist_symbols(iq)[j]);
            if (list_symbol(result, indx) == LUX_ERROR)
              return LUX_ERROR;
            embed(list_symbol(result, indx), result);
            indx++;
          }
          break;
        case LUX_LIST:
          nelem = list_num_symbols(iq);
          for (j = 0; j < nelem; j++) {
            list_key(result, indx) =
              list_key(iq, j)? strsave(list_key(iq, j)): NULL;
            list_symbol(result, indx) =
              copyEvalSym(list_symbol(iq, j));
            if (list_symbol(result, indx) == LUX_ERROR)
              return LUX_ERROR;
            embed(list_symbol(result, indx), result);
            indx++;
          }
          break;
        case LUX_UNUSED: case LUX_UNDEFINED:
          break;                // ignore these
        default:
          list_key(result, indx) = NULL;
          list_symbol(result, indx) = copyEvalSym(iq);
          if (list_symbol(result, indx) == LUX_ERROR)
            return LUX_ERROR;
          embed(list_symbol(result, indx), result);
          indx++;
          break;
      }
    }
  } else {                      // result is a list: no structs
    for (i = 0; i < narg; i++) {
      iq = ps[i];
      switch (symbol_class(iq)) {
        case LUX_CLIST:
          nelem = clist_num_symbols(iq);
          for (j = 0; j < nelem; j++) {
            clist_symbols(result)[indx] =
              copyEvalSym(clist_symbols(iq)[j]);
            if (clist_symbols(result)[indx] == LUX_ERROR)
              return LUX_ERROR;
            embed(clist_symbols(result)[indx], result);
            indx++;
          }
          break;
        case LUX_UNUSED: case LUX_UNDEFINED:
          break;                // ignore these
        default:
          clist_symbols(result)[indx] = copyEvalSym(iq);
          if (clist_symbols(result)[indx] == LUX_ERROR)
            return LUX_ERROR;
          embed(clist_symbols(result)[indx], result);
          indx++;
          break;
      }         // end switch (symbol_class(iq))
    } // end for (i = 0; ...)
  }
  return result;                // everything OK
}
 //-------------------------------------------------------------------------
int32_t lux_concat(int32_t narg, int32_t ps[])
 /* lux's concatenation function, insisted on by idl users, but works a bit
         differently
 there are 2 modes
 mode 1 - has a single argument, we return a variable which has one extra
          dimension which has a length of one; i.e., a scalar becomes a
          vector of length 1, a vector of length n becomes an array of nx1
 mode 2 - 2 or more arguments, result combines the last dimension
         all dimensions but the last must be of the same size */
 // C version 11/2/91, r shine
 /* allow undefined arguments (if at least one argument is defined)
    to work things like x=[x,...] without x initialization
    LS 22feb93 */
// allow concatenation of LISTs and STRUCTs LS 7feb95
{
  Pointer q1,q2;
  int32_t       nd, j, i, dim[MAX_DIMS], nundef = 0;
  int32_t       iq, nsym, mq, topnd = 0, sflag = 0, n, nq;
  Symboltype toptype = LUX_INT8;
  Scalar        temp;

  if (narg <= 0)
    return cerror(WRNG_N_ARG, 0);
  if (narg == 1) {              // one argument case
    if ((iq = ps[0]) <= 0)      // some error
      return iq;
    switch (symbol_class(iq)) {
      case LUX_SCAL_PTR:
        iq = dereferenceScalPointer(iq);
        // fall-thru to SCALAR case
      case LUX_SCALAR:          //scalar case
        // create an array of length 1
        dim[0] = 1;
        nsym = array_scratch(scalar_type(iq), 1, dim);
        if (nsym == LUX_ERROR)
          return LUX_ERROR;
        memcpy((char*) array_data(nsym),
               &scalar_value(iq).l,
               lux_type_size[scalar_type(iq)]);
        return nsym;
      case LUX_CSCALAR:                 // complex scalar case LS 4aug98
        dim[0] = 1;
        nsym = array_scratch(complex_scalar_type(iq), 1, dim);
        if (nsym == LUX_ERROR)
          return LUX_ERROR;
        memcpy((char*) complex_array_data(nsym),
               (char*) complex_scalar_data(iq).cf,
               lux_type_size[complex_scalar_type(nsym)]);
        return nsym;
      case LUX_STRING:  //string, turn into string array.  LS 21apr93
        dim[0] = 1;
        nsym = array_scratch(LUX_TEMP_STRING, 1, dim);
        q1.sp = (char **) array_data(nsym);
        *q1.sp = strsave(string_value(iq));
        return nsym;
      case LUX_ARRAY: case LUX_CARRAY: //array
        // make a copy with an additional dimension (of size 1) added

        if (array_num_dims(iq) == MAX_DIMS) // already at max number of
                                            // dimensions
          return cerror(N_DIMS_OVR, iq);
        nd = array_num_dims(iq);
        memcpy(dim, array_dims(iq), nd*sizeof(int32_t));
        dim[nd++] = 1;          // extra dimension
        nsym = array_scratch(array_type(iq), nd, dim);
        if (array_type(nsym) == LUX_STRING_ARRAY) { // string array
          q1.v = array_data(iq);
          q2.v = array_data(nsym);
          n = array_size(iq);
          while (n--)
            if (*q1.sp)                 // actual string there
              *q2.sp++ = strsave(*q1.sp++);
            else                // empty string
              *q2.sp++ = 0;
          return nsym;
        }
        // if we get here it is a numerical array
        mq = symbol_memory(iq) - sizeof(array);// actual data memory
        memcpy((char*) array_data(nsym), (char*) array_data(iq), mq);
        return nsym;
      default:
        return cerror(ILL_CLASS, iq);
    } //end of switch
  } else {                      // 2 or more arguments
    // first a quick look at all the symbols
    mq = 0;                     // total number of elements
    for (i = 0; i < narg; i++) {
      if ((iq = ps[i]) <= 0)    // some error
        return iq;
      switch (symbol_class(iq))         {
        case LUX_SCALAR: case LUX_SCAL_PTR: case LUX_CSCALAR: //scalar case
          mq += 1;              // one element
          break;
        case LUX_ARRAY: case LUX_CARRAY: // array; real or complex
          mq += array_size(iq);
          if (array_num_dims(iq) > topnd) // max number of dimensions
            topnd = array_num_dims(iq);
          if (array_type(iq) == LUX_STRING_ARRAY)
            sflag += 1;                 // string array
          break;
        case LUX_STRING:        // string
          mq++;                         // one string
          sflag++;              // string flag
          break;
        case LUX_UNDEFINED:     // undefined: ignore
          nundef++;
          break;
        case LUX_CLIST: case LUX_LIST:  // added LS 7feb95
          return lux_concat_list(narg, ps);
        default:
          return cerror(ILL_CLASS, iq);
      }
      toptype = combinedType(toptype, symbol_type(iq));
    }   // end of for
    // now run through again
    if (!mq)
      return cerror(ILL_CLASS, 0); // all were undefined
    if (sflag) {
      if (sflag + nundef < narg) // some but not all were strings
        return cerror(ILL_CMB_S_NON_S, 0);
      toptype = LUX_STRING_ARRAY;
    }
    if (topnd == 0) {           // no arrays
      dim[0] = mq;
      nsym = array_scratch(toptype, 1, dim);
      n = lux_type_size[toptype];
      q2.b = (uint8_t*) array_data(nsym);
      for (i = 0; i < narg; i++) {
        iq = ps[i];
        switch (symbol_class(iq)) {
          case LUX_SCAL_PTR:
            iq = dereferenceScalPointer(iq);
            // fall-thru to LUX_SCALAR
          case LUX_SCALAR:
            memcpy(&temp.b, &scalar_value(iq).b,
                   lux_type_size[scalar_type(iq)]);
            if (scalar_type(iq) != toptype)
              convertPointer(&temp, scalar_type(iq), toptype);
            memcpy(q2.b, &temp.b, n);
            q2.b += n;          // add the right Byte count for each element
            break;
          case LUX_CSCALAR:
            memcpy(q2.b, complex_scalar_data(iq).cf, n);
            q2.b += n;
            break;
          case LUX_STRING:
            *q2.sp++ = strsave(string_value(iq));
            break;
          case LUX_UNDEFINED:   // ignore
            break;
          default:              //just in case
            return cerror(ILL_CLASS, iq);
        }
      }
      return nsym;
    }
    // arrays, the only possiblity left
    // all component arrays must have the same dimensions, except that
    // the last dimension (from the dimensions list of the result)
    // may be missing.
    // the arguments are either all numerical or all strings
    n = 0;                      // flag: result dimensions list not yet
                                // defined
    nq = 0;
    for (i = 0; i < narg; i++) {
      iq = ps[i];
      switch (symbol_class(iq)) {
        case LUX_ARRAY: case LUX_CARRAY:
          if (array_num_dims(iq) < topnd - 1) // not enough dimensions
            return cerror(INCMP_DIMS, iq);
          if (n) {              // have result dimensions list
            mq = (array_num_dims(iq) == topnd)?
              array_dims(iq)[topnd - 1]: 1; // last dimension
            // check all dimensions but last one
            if (internalMode & 1) { // /SLOPPY: # elements must match
              if (array_size(iq)/mq != n) // no match
                return cerror(INCMP_DIMS, iq);
            } else {            // each dimension must match
              for (j = 0; j < topnd - 1; j++)
                if (array_dims(iq)[j] != dim[j])
                  return cerror(INCMP_DIMS, iq);
            }
            nq += mq;           // collect total in loose dimension
          } else {              // create result dimensions list
            memcpy(dim, array_dims(iq), array_num_dims(iq)*sizeof(int32_t));
            // we count the last dimension separately: all the others
            // must match, but the last one may vary between components
            nq = (array_num_dims(iq) == topnd)? dim[topnd - 1]: 1;
            n = array_size(iq)/nq; // number of elements in all but the
                                   // loose dimension
          }
          break;
        case LUX_SCAL_PTR: case LUX_SCALAR: case LUX_STRING: case LUX_CSCALAR:
          // NOTE: there was a serious bug here that showed up in statements
          // such as X = [6, X] if X is an array: the resulting X would
          // have the same number of elements as the old one but the
          // extra element would apparently be stored, leading to
          // illegal memory access.  LS fixed 12may98: add if (!n)
          // statement, and nq = 0 before this switch statement.
          if (topnd > 1)
            return cerror(INCMP_DIMS, iq);
          if (!n)
            n = 1;
          nq++;                         // count this one in the last dimension
          break;
        case LUX_UNDEFINED:     // ignore
          break;
      }         // switch (symbol_class(iq))
    } // for (i = 0; ...)
    dim[topnd - 1] = nq;        // final value for the last dimension

    // get the resultant array
    nsym = array_scratch(toptype, topnd, dim);
    q2.v = array_data(nsym);

    // now just load everybody in, converting type as necessary
    for (i = 0; i < narg; i++) {
      iq = ps[i];
      n = lux_type_size[symbol_type(iq)];
      switch (symbol_class(iq))         {
        case LUX_SCALAR:        //scalar case
          q1.b = &scalar_value(iq).b;
          n = 1;
          break;
        case LUX_SCAL_PTR:
          q1.b = scal_ptr_pointer(iq).b;
          n = 1;
          break;
        case LUX_ARRAY: case LUX_CARRAY: // array
          n = array_size(iq);
          q1.b = (uint8_t*) array_data(iq);
          break;
        case LUX_CSCALAR:
          q1 = complex_scalar_data(iq);
          n = 1;
          break;
        case LUX_STRING:        // string
          n = 1;
          q1.s = string_value(iq);
          break;
        case LUX_UNDEFINED:     // ignore
          n = 0;
          break;
      }         // switch (symbol_class(iq))
      if (n)
        switch (toptype) {
        case LUX_INT8:
          while (n--)
            *q2.b++ = *q1.b++;
          break;
        case LUX_INT16:
          switch (symbol_type(iq)) {
          case LUX_INT8:
            while (n--)
              *q2.w++ = (int16_t) *q1.b++;
            break;
          case LUX_INT16:
            while (n--)
              *q2.w++ = *q1.w++;
            break;
          }
          break;
        case LUX_INT32:
          switch (symbol_type(iq)) {
          case LUX_INT8:
            while (n--)
              *q2.l++ = (int32_t) *q1.b++;
            break;
          case LUX_INT16:
            while (n--)
              *q2.l++ = (int32_t) *q1.w++;
            break;
          case LUX_INT32:
            while (n--)
              *q2.l++ = *q1.l++;
            break;
          }
          break;
        case LUX_INT64:
          switch (symbol_type(iq)) {
          case LUX_INT8:
            while (n--)
              *q2.q++ = (int64_t) *q1.b++;
            break;
          case LUX_INT16:
            while (n--)
              *q2.q++ = (int64_t) *q1.w++;
            break;
          case LUX_INT32:
            while (n--)
              *q2.q++ = (int64_t) *q1.l++;
            break;
          case LUX_INT64:
            while (n--)
              *q2.q++ = *q1.q++;
            break;
          }
          break;
        case LUX_FLOAT:
          switch (symbol_type(iq)) {
          case LUX_INT8:
            while (n--)
              *q2.f++ = (float) *q1.b++;
            break;
          case LUX_INT16:
            while (n--)
              *q2.f++ = (float) *q1.w++;
            break;
          case LUX_INT32:
            while (n--)
              *q2.f++ = (float) *q1.l++;
            break;
          case LUX_INT64:
            while (n--)
              *q2.f++ = (float) *q1.q++;
            break;
          case LUX_FLOAT:
            while (n--)
              *q2.f++ = *q1.f++;
            break;
          }
          break;
        case LUX_DOUBLE:
          switch (symbol_type(iq)) {
          case LUX_INT8:
            while (n--)
              *q2.d++ = (double) *q1.b++;
            break;
          case LUX_INT16:
            while (n--)
              *q2.d++ = (double) *q1.w++;
            break;
          case LUX_INT32:
            while (n--)
              *q2.d++ = (double) *q1.l++;
            break;
          case LUX_INT64:
            while (n--)
              *q2.d++ = (double) *q1.q++;
            break;
          case LUX_FLOAT:
            while (n--)
              *q2.d++ = (double) *q1.f++;
            break;
          case LUX_DOUBLE:
            while (n--)
              *q2.d++ = *q1.d++;
            break;
          }
          break;
        case LUX_CFLOAT:
          switch (symbol_type(iq)) {
          case LUX_INT8:
            while (n--) {
              q2.cf->real = *q1.b++;
              q2.cf++->imaginary = 0.0;
            }
            break;
          case LUX_INT16:
            while (n--) {
              q2.cf->real = *q1.w++;
              q2.cf++->imaginary = 0.0;
            }
            break;
          case LUX_INT32:
            while (n--) {
              q2.cf->real = *q1.l++;
              q2.cf++->imaginary = 0.0;
            }
            break;
          case LUX_INT64:
            while (n--) {
              q2.cf->real = *q1.q++;
              q2.cf++->imaginary = 0.0;
            }
            break;
          case LUX_FLOAT:
            while (n--) {
              q2.cf->real = *q1.f++;
              q2.cf++->imaginary = 0.0;
            }
            break;
          case LUX_CFLOAT:
            while (n--) {
              q2.cf->real = q1.cf->real;
              q2.cf++->imaginary = q1.cf++->imaginary;
            }
            break;
          }
          break;
        case LUX_CDOUBLE:
          switch (symbol_type(iq)) {
          case LUX_INT8:
            while (n--) {
              q2.cd->real = *q1.b++;
              q2.cd++->imaginary = 0.0;
            }
            break;
          case LUX_INT16:
            while (n--) {
              q2.cd->real = *q1.w++;
              q2.cd++->imaginary = 0.0;
            }
            break;
          case LUX_INT32:
            while (n--) {
              q2.cd->real = *q1.l++;
              q2.cd++->imaginary = 0.0;
            }
            break;
          case LUX_INT64:
            while (n--) {
              q2.cd->real = *q1.q++;
              q2.cd++->imaginary = 0.0;
            }
            break;
          case LUX_FLOAT:
            while (n--) {
              q2.cd->real = *q1.f++;
              q2.cd++->imaginary = 0.0;
            }
            break;
          case LUX_DOUBLE:
            while (n--) {
              q2.cd->real = *q1.d++;
              q2.cd++->imaginary = 0.0;
            }
            break;
          case LUX_CFLOAT:
            while (n--) {
              q2.cd->real = q1.cf->real;
              q2.cd++->imaginary = q1.cf++->imaginary;
            }
            break;
          case LUX_CDOUBLE:
            while (n--) {
              q2.cd->real = q1.cd->real;
              q2.cd++->imaginary = q1.cd++->imaginary;
            }
            break;
          }
          break;
        case LUX_TEMP_STRING: case LUX_LSTRING: case LUX_STRING_ARRAY:
          if (symbol_class(iq) == LUX_STRING)
            *q2.sp = strsave(q1.s);
          else                  // string array
            while (n--) {
              *q2.sp++ = *q1.sp? strsave(*q1.sp): NULL;
              q1.sp++;
            }
          break;
        } // switch (toptype)
    } // for (i = 0; ...)
    return nsym;
  } // end of if else 2-or-more args
}                                               //end of lux_concat
//-------------------------------------------------------------------------
int32_t lux_isscalar(int32_t narg, int32_t ps[])
// return 1 if symbol is a scalar, 0 otherwise
{ return (symbol_class(*ps) == LUX_SCALAR)? 1: 4; }
//-------------------------------------------------------------------------
int32_t lux_isarray(int32_t narg, int32_t ps[])
// return 1 if symbol is an array, 0 otherwise
{ return (symbol_class(*ps) == LUX_ARRAY)? 1: 4; }
//-------------------------------------------------------------------------
int32_t lux_isstring(int32_t narg, int32_t ps[])
// return 1 if symbol is a string, 0 otherwise
{ return (symbol_class(*ps) == LUX_STRING)? 1: 4; }
//-------------------------------------------------------------------------
int32_t lux_subsc_subgrid(int32_t narg, int32_t ps[])
/* like x(subsc,/inner) but with linear interpolation for fractional
   coordinates.  LS 19jun97 */
/* the target (subscripted) symbol is in ps[narg]; narg indicates the
   number of subscripts.  This is somewhat non-standard use of ps/narg!
   LS 19aug98 */
{
  Symbolclass class_id;
  uint8_t       type;
  int32_t       ndim, *dims, i, subsc[MAX_DIMS], nSubsc, iq, j, mid,
    stride[MAX_DIMS], index, tally[MAX_DIMS], step[2][MAX_DIMS];
  float         *coord[MAX_DIMS], d[MAX_DIMS];
  Scalar        value, cvalue;
  Pointer       src, out;

  class_id = (Symbolclass) symbol_class(ps[narg]); // class of target symbol
  switch (class_id) {
    default:
      return cerror(ILL_CLASS, ps[narg]);
    case LUX_ARRAY:             // only numerical LUX_ARRAY is allowed
      type = array_type(ps[narg]);
      if (type > LUX_DOUBLE)    // string array
        return cerror(ILL_TYPE, ps[narg]);
      src.l = (int32_t*) array_data(ps[narg]);
      ndim = array_num_dims(ps[narg]);
      dims = array_dims(ps[narg]);
      break;
  }

  if (narg != ndim)
    return luxerror("Incorrect number of subscripts", ps[narg]);

  for (i = 0; i < ndim; i++) { // all subscripts
    /* the subscripts must all be numerical with the same number
       of elements */
    iq = ps[i];
    switch (symbol_class(iq)) {
      case LUX_ARRAY:
        if (symbol_type(iq) > LUX_DOUBLE)
          return cerror(ILL_TYPE, iq);
        if (i) {
          if (array_size(iq) != nSubsc)
            return cerror(INCMP_DIMS, iq);
        } else
          nSubsc = array_size(iq);
        subsc[i] = lux_float(1, &iq);
        coord[i] = (float*) array_data(subsc[i]);
        break;
      case LUX_SCALAR:
        if (i) {
          if (nSubsc != 1)
            return cerror(INCMP_DIMS, iq);
        } else
          nSubsc = 1;
        subsc[i] = lux_float(1, &iq);
        coord[i] = &scalar_value(subsc[i]).f;
        break;
      default:
        return cerror(ILL_CLASS, iq);
    }
  }

  // derive step sizes for moving through the various dimensions
  stride[0] = 1;
  for (i = 1; i < ndim; i++)
    stride[i] = stride[i - 1]*dims[i - 1];

  for (i = 0; i < ndim; i++) {
    step[1][i] = stride[i];
    step[0][i] = -stride[i];
  }

  /* now create output symbol: FLOAT or DOUBLE, and with same dimensions
   as first subscript */
  if (symbol_class(subsc[0]) == LUX_ARRAY) {
    iq = array_scratch(type == LUX_DOUBLE? LUX_DOUBLE: LUX_FLOAT,
                       array_num_dims(subsc[0]), array_dims(subsc[0]));
    out.f = (float*) array_data(iq);
  } else {
    iq = scalar_scratch(type == LUX_DOUBLE? LUX_DOUBLE: LUX_FLOAT);
    out.f = &scalar_value(iq).f;
  }

  for (i = 0; i < ndim; i++)
    tally[i] = 0;

  // now start interpolating
  while (nSubsc--) {
    /* first find central point around which interpolation
       will be performed */
    index = 0;
    for (i = 0; i < ndim; i++) {
      if (*coord[i] < 0)
        mid = 0;
      else if (*coord[i] >= dims[i] - 1)
        mid = dims[i] - 2;
      else
        mid = (int32_t) *coord[i];
      d[i] = mid + 1 - *coord[i]; // interpolation distance
      index += mid*stride[i];
      coord[i]++;
    }

    value.d = 0.0;

    // now do the interpolating
    switch (type) {
      case LUX_INT8:
        do {
          cvalue.f = (float) src.b[index];
          for (i = 0; i < ndim; i++)
            cvalue.f *= d[i];
          for (j = 0; j < ndim; j++) {
            tally[j] ^= 1;
            d[j] = 1 - d[j];
            index += step[tally[j]][j];
            if (tally[j])
              break;
          }
          value.f += cvalue.f;
        } while (j < ndim);
        *out.f++ = value.f;
        break;
      case LUX_INT16:
        do {
          cvalue.f = (float) src.w[index];
          for (i = 0; i < ndim; i++)
            cvalue.f *= d[i];
          for (j = 0; j < ndim; j++) {
            tally[j] ^= 1;
            d[j] = 1 - d[j];
            index += step[tally[j]][j];
            if (tally[j])
              break;
          }
          value.f += cvalue.f;
        } while (j < ndim);
        *out.f++ = value.f;
        break;
      case LUX_INT32:
        do {
          cvalue.f = (float) src.l[index];
          for (i = 0; i < ndim; i++)
            cvalue.f *= d[i];
          for (j = 0; j < ndim; j++) {
            tally[j] ^= 1;
            d[j] = 1 - d[j];
            index += step[tally[j]][j];
            if (tally[j])
              break;
          }
          value.f += cvalue.f;
        } while (j < ndim);
        *out.f++ = value.f;
        break;
      case LUX_INT64:
        do {
          cvalue.d = src.q[index];
          for (i = 0; i < ndim; i++)
            cvalue.d *= d[i];
          for (j = 0; j < ndim; j++) {
            tally[j] ^= 1;
            d[j] = 1 - d[j];
            index += step[tally[j]][j];
            if (tally[j])
              break;
          }
          value.d += cvalue.d;
        } while (j < ndim);
        *out.f++ = value.d;
        break;
      case LUX_FLOAT:
        do {
          cvalue.f = src.f[index];
          for (i = 0; i < ndim; i++)
            cvalue.f *= d[i];
          for (j = 0; j < ndim; j++) {
            tally[j] ^= 1;
            d[j] = 1 - d[j];
            index += step[tally[j]][j];
            if (tally[j])
              break;
          }
          value.f += cvalue.f;
        } while (j < ndim);
        *out.f++ = value.f;
        break;
      case LUX_DOUBLE:
        do {
          cvalue.d = src.d[index];
          for (i = 0; i < ndim; i++)
            cvalue.d *= d[i];
          for (j = 0; j < ndim; j++) {
            tally[j] ^= 1;
            d[j] = 1 - d[j];
            index += step[tally[j]][j];
            if (tally[j])
              break;
          }
          value.d += cvalue.d;
        } while (j < ndim);
        *out.d++ = value.d;
        break;
    }
  }
  return iq;
}
//-------------------------------------------------------------------------
int32_t extractNumerical(Pointer src, Pointer trgt, Symboltype type,
                         int32_t ndim, int32_t *dims, int32_t *coords,
                         int32_t nAxes, int32_t *axes)
{
  LoopInfo      info;

  info.setupDimensionLoop(ndim, dims, type, nAxes, axes, &src,
                          SL_EACHCOORD);

  info.subdataLoop(coords);

  do {
    memcpy(trgt.b, src.b, info.stride);
    src.b += info.stride;
  } while (info.advanceLoop(&trgt) < info.rndim);
  return 1;
}
//-------------------------------------------------------------------------
int32_t lux_roll(int32_t narg, int32_t ps[])
// ROLL(array, target) rearranges the dimensions of <array> according to
// <target>.  If <target> is a scalar, then the dimensions list is shifted
// cyclically over that number of elements.  If <target> is a numerical
// array, then it is taken to contain the indices of the new dimensions.
// For example, if <array> has dimensions 2 by 3 by 4, then
// ROLL(array,1) is equivalent to array(>1,>2,>0) (a 4 by 2 by 3 array),
// ROLL(array,-1) is equivalent to array(>2,>0,>1) (a 3 by 4 by 2 array),
// and ROLL(array,[2,1,0]) is equivalent to array(>2,*,>0) (a 4 by 3 by 2
// array).  If <target> is an array, then DIMEN(array)(target) is equal to
// DIMEN(ROLL(array,target)).  LS 12sep2000
{
  int32_t       nd, result, temp, iq, i;
  LoopInfo      srcinfo, trgtinfo;
  Pointer       src, trgt;

  if (!symbolIsNumericalArray(ps[0]))
    return cerror(NEED_ARR, ps[0]);

  if (symbolIsScalar(ps[1])) {  // <target> is a scalar: roll the dimension
                                // list of <array>
    nd = array_num_dims(ps[0]);
    result = -int_arg(ps[1]);
    if (result < 0)
      result += ((-result/nd) + 1)*nd;
    if (result > nd)
      result -= (result/nd)*nd;
    // we must construct an array containing the target list of dimensions
    iq = array_scratch(LUX_INT32, 1, &nd);
    src.l = (int32_t*) array_data(iq);
    for (i = 0; i < nd; i++) {
      *src.l = i + result;
      if (*src.l >= nd)
        *src.l -= nd;
      src.l++;
    }
    temp = 1;
  } else if (symbolIsNumericalArray(ps[1])) {
    iq = ps[1];
    nd = array_size(ps[1]);
    if (nd != array_num_dims(ps[0]))
      return cerror(INCMP_ARG, ps[1]);
    temp = 0;
  } else
    return cerror(ILL_CLASS, ps[1]);

  if (standardLoop(ps[0], iq, SL_KEEPTYPE | SL_AXESBLOCK, LUX_INT8, &srcinfo,
                   &src, &result, &trgtinfo, &trgt) == LUX_ERROR)
    return LUX_ERROR;

  memcpy(array_dims(result), srcinfo.rdims, nd*sizeof(int32_t));

  if (temp)
    zap(iq);                    // no longer needed

  do {
    memcpy(trgt.b, src.b, srcinfo.stride);
    trgt.b += srcinfo.stride;
  } while (srcinfo.advanceLoop(&src) < srcinfo.rndim);

  return result;
}
//-------------------------------------------------------------------------
