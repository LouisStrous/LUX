/* This is file bindings.cc.

Copyright 2013-2017 Louis Strous

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

/** \file

    Functions and macros for binding LUX functions and subroutines to
    C (library) functions.

    This file defines many binding functions.  The name of a binding
    function is built up as follows:

    \verbatim
    'lux_<returntype>_<paramtypes>_<stdargspec>_<ptrspec>_<sf>_'
    \endverbatim

    A descriptive suffix may be appended to disambiguate names for
    binding functions that would otherwise be equal.

    \par returntype

    \c <returntype> is an encoding of the return type of the bound C
    function.  \c void is encoded as `v`, \c double as `d`, \c int32_t
    as `i`.  A pointer to a type gets a `p` appended, so a `double *`
    is encoded as `dp`.  A pointer to a `double` array of 3 by 2
    elements is encoded as `dp32`.

    So,

    \verbatim
    lux_d_X_X_X_X_
    \endverbatim

    binds a C function that returns a \c double.

    \par paramtypes

    \c <paramtypes> is the concatenated encoding of the parameter
    types of the bound C function.  The encoding is the same as for \c
    <returntype>, with the following extensions: Encoding `sd` refers
    to three adjacent parameters <tt>double* data, size_t count,
    size_t stride</tt> that describe a \c double vector slice.  A
    repeat of a particular parameter type (for a repeat count of at
    least 3) is indicated by appending \c T and the repeat count.  For
    example, <tt>double, double, double</tt> is encoded as `dT3`.

    So,

    \verbatim
    lux_d_dT3i_X_X_X_
    \endverbatim

    binds a C function that takes three \c double arguments and an \c
    int32_t argument and returns a \c double.

    \par stdargspec

    \c <stdargspec> is an encoding of the arguments type specification
    in the most elaborate call to standard_args() for this binding.
    The encoding is obtained from the arguments type specification by:

    - removing `;`, `?`, and all data type specifications (including
      `>`).

    - changing `&` to `q`, `+` to `p`, `-` to `m`, `*` to `a`, `=` to
      `k`.

    - changing `[...]` to `c`, `{...}` to `x`.

    - changing `#` to `o`.

    A repeat of a particular unit (from a `i`/`o`/`r` up to but not
    including the next one; for a repeat count of at least 3, or if
    the abbreviated version is shorter than the full version) is
    indicated by appending `T` and the repeat count.  For example,
    `iqiqiqiq` gets abbreviated to `iqT4`.

    Multiple standard_args() format specifications can lead to the
    same \c <stdargspec>.

    So,

    \verbatim
    lux_d_dT3i_i3rq_X_X_
    \endverbatim

    may be for a LUX function that takes a one-dimensional
    three-element array and a scalar for arguments and returns a
    one-dimensional three-element `double` array.

    \par ptrspec

    \c <ptrspec> is the concatenation of the pointer index that is
    used for each of the bound function's parameters.  If the bound
    function's return value is stored using a pointer/info index, then
    that pointer index is concatenated at the end, after `_`.  If a
    particular bound function's parameter does not depend on a
    pointer/info index, then `z` is specified for it.  3 or more
    adjacent indices are replaced by `<first-index>T<last-index>`.

    So,

    \verbatim
    lux_d_dT3i_i3rq_0001_2_X_
    \endverbatim

    indicates that the first three arguments to the bound C function
    are associated with the \c stdargspec format's argument with index
    0 (i.e., the first one), the next argument to the bound C function
    is associated with index 1, and the bound C function's return
    value is associated with index 2.

    \par sf

    \c <sf> is `s` if the binding is to a LUX function, or `s` if to a
    LUX subroutine.

    So,

    \verbatim
    lux_d_dT3i_i3rq_0001_2_f_
    \endverbatim

    binds the C function to a LUX function.

*/
#include "luxdefs.hh"
#include "action.hh"
#include "error.hh"
#include <math.h>
#include <obstack.h>
#include "bindings.hh"

//* Define which memory allocation routine to use for obstacks.
#define obstack_chunk_alloc malloc

//* Define which memory freeing routine to use for obstacks.
#define obstack_chunk_free free

struct obstack *registered_functions = NULL, *registered_subroutines = NULL;

//-----------------------------------------------------------------------
/// Bind a C++ pointer-count-stride function to a LUX function of type
/// `iD*;rD&` or `iD*;iL*;rD&`, or subroutine of type `iD*` or
/// `iD*;iL*`.
///
/// The LUX function or subroutine arguments are:
///
/// 1. an input scalar or array of arbitrary dimensions, converted to
/// `double`.  For the subroutine, this argument is modified, so is an
/// output argument, too, and must be a named variable.
///
/// 2. optionally, an input scalar or array of arbitrary dimensions,
/// converted to `int32` and treated as an axis parameter.
///
/// The mode argument `/allaxes` (`internalMode & 1`) is equivalent to
/// setting the second argument equal to an array listing all axes of
/// the first argument.
///
/// The LUX function returns a `double` value with the same dimensions
/// as the input argument.  It first copies the first argument to the
/// return value, and then acts as if the LUX subroutine were called
/// on the copy and the second argument.
///
/// The C++ function is called schematically like this:
///
/// \verbatim
/// f(i0, i0.dims[0], i0.step[0]);
/// advance(i0);
/// \endverbatim
int32_t lux_ivarl_copy_eachaxis_(int32_t narg, int32_t ps[],
                                 int32_t (*f)(double *, size_t count,
                                              size_t stride),
                                 int32_t isfunction)
// copy input to output, apply function to output, go through each
// axis separately
{
  Pointer *ptrs, ptrs0, ptrsr;
  LoopInfo *infos;
  int32_t iq, iret;
  int32_t *axes, naxes, oneaxis[1] = { 0 }, allaxes;

  StandardArguments_RAII sa;
  switch (narg) {
  case 1:                       // source
    if ((iq = sa.set(narg, ps, isfunction? "iD*;rD&": "iD*",
                     &ptrs, &infos)) < 0)
      return LUX_ERROR;
    axes = oneaxis;
    naxes = 1;
    setAxes(&infos[0], 0, NULL, SL_EACHROW);
    if (isfunction) {
      iret = 1;
      setAxes(&infos[iret], 0, NULL, SL_EACHROW);
    } else
      iret = 0;
    break;
  case 2:                       // source, axes
    if ((iq = sa.set(narg, ps, isfunction? "iD*;iL*;rD&": "iD*;iL*",
                     &ptrs, &infos)) < 0)
      return LUX_ERROR;
    axes = ptrs[1].l;
    naxes = infos[1].nelem;
    if (setAxes(&infos[0], infos[1].nelem, ptrs[1].l,
                SL_EACHROW | SL_UNIQUEAXES) < 0)
      return LUX_ERROR;
    if (isfunction) {
      iret = 2;
      if (setAxes(&infos[iret], infos[1].nelem, ptrs[1].l,
                  SL_EACHROW | SL_UNIQUEAXES) < 0)
        return LUX_ERROR;
    } else
      iret = 0;
    break;
  }

  if (internalMode & 1) {	// /allaxes
    naxes = infos[0].ndim;
    axes = (int32_t*) malloc(naxes*sizeof(int32_t));
    allaxes = 1;
    int32_t i;
    for (i = 0; i < naxes; i++)
      axes[i] = i;
  } else
    allaxes = 0;

  ptrs0 = ptrs[0];
  if (isfunction) {
    // copy input to output, then we can treat the function case as
    // the subroutine case
    memcpy(ptrs[iret].d, ptrs[0].d, infos[0].nelem*sizeof(double));
    ptrsr = ptrs[iret];
  }
  int32_t iaxis;
  for (iaxis = 0; iaxis < naxes; iaxis++) {
    setAxes(&infos[0], 1, &axes[iaxis], SL_EACHROW);
    ptrs[0] = ptrs0;
    if (isfunction) {
      setAxes(&infos[iret], 1, &axes[iaxis], SL_EACHROW);
      ptrs[iret] = ptrsr;
    }
    do {
      f(ptrs[iret].d, infos[iret].rdims[0], infos[iret].rsinglestep[0]);
      ptrs[iret].d += infos[iret].rsinglestep[0]*infos[iret].rdims[0];
    } while (advanceLoop(&infos[iret], &ptrs[iret]) < infos[iret].rndim);
  }
  if (allaxes)
    free(axes);
  return iq;
}
//-----------------------------------------------------------------------
double call_split_times(double t1, double t2,
                        double(*f)(double, double, double, double))
{
  double uta, utb, tta, ttb;
  uta = floor(t1);
  utb = t1 - uta;
  tta = floor(t2);
  ttb = t2 - ttb;
  return f(uta, utb, tta, ttb);
}
//-----------------------------------------------------------------------
/** Bind a C function to a LUX function of type `i>L*;iD&;iD&;rD&`.

    The LUX function arguments are:

    1. an input scalar or array of arbitrary dimensions, converted to
    `int32` if its type is less than that.

    2. two more input arguments with the same dimensions as the first
    one, converted to `double`.

    The LUX function returns a `double` array with the same dimensions
    as the first argument.

    The C function is schematically called like this:

    \verbatim
    *r++ = f(*i0++, *i1++, *i2++)
    \endverbatim

    @param [in] narg number of symbols in \p ps
    @param [in] ps array of argument symbols
    @param [in] f pointer to C function to bind
    @return the symbol containing the result of the function call
 */
int32_t
lux_d_dT3_iaiqiqrq_012_f_(int32_t narg, int32_t ps[],
                          double (*f)(double, double, double))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "i>L*;iD&;iD&;rD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  switch (infos[0].type) {
  case LUX_INT32:
    while (infos[1].nelem--)
      *ptrs[3].d++ = f((double) *ptrs[0].l++, *ptrs[1].d++, *ptrs[2].d++);
    break;
  case LUX_INT64:
    while (infos[1].nelem--)
      *ptrs[3].d++ = f((double) *ptrs[0].q++, *ptrs[1].d++, *ptrs[2].d++);
    break;
  case LUX_FLOAT:
    while (infos[1].nelem--)
      *ptrs[3].d++ = f((double) *ptrs[0].f++, *ptrs[1].d++, *ptrs[2].d++);
    break;
  case LUX_DOUBLE:
    while (infos[1].nelem--)
      *ptrs[3].d++ = f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++);
    break;
  default:
    break;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
/// Bind a C++ function to a LUX function of type
/// `iD*;iD&;iD&;iD&;rD&`.
///
/// The LUX function arguments are:
///
/// 1. an input scalar or array of arbitrary dimensions , converted to
/// `double`.
///
/// 2. four more input arguments with the same dimensions as the first
/// one, converted to `double`.
///
/// The LUX function returns a `double` array with the same dimensions
/// as the first argument.
///
/// The C++ function is schematically called like this:
///
/// \verbatim
/// *r++ = f(*i0++, *i1++, *i2++, *i3++)
/// \endverbatim
///
/// @param [in] narg number of symbols in \p ps
/// @param [in] ps array of argument symbols
/// @param [in] f pointer to C function to bind
/// @return the symbol containing the result of the function call
int32_t
lux_d_dT4_iaiqT3rq_0T3_4_f_(int32_t narg, int32_t ps[],
                            double (*f)(double, double, double, double))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iD&;iD&;iD&;rD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--)
    *ptrs[4].d++ = f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++);
  return sa.result();
}
//-----------------------------------------------------------------------
/// Bind a C++ function to a LUX function of type `iD*;iD&;rD&`.
///
/// The LUX function arguments are:
///
/// 1. an input scalar or array of arbitrary dimensions, converted to
/// `double`.
///
/// 2. an input argument with the same dimensions as the first one,
/// converted to `double`.
///
/// The LUX function returns a `double` array with the same dimensions
/// as the first argument.
///
/// The C++ function is schematically called like this:
///
/// \verbatim
/// *r++ = f(*i0++, *i1++)
/// \endverbatim
///
/// @param [in] narg number of symbols in \p ps
/// @param [in] ps array of argument symbols
/// @param [in] f pointer to C function to bind
/// @return the symbol containing the result of the function call
int32_t
lux_d_dT4_iaiqrq_01_2_f_(int32_t narg, int32_t ps[],
                         double (*f)(double, double, double, double))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iD&;rD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    *ptrs[2].d++ = call_split_times(*ptrs[0].d++, *ptrs[1].d++, f);
  }
  return sa.result();
}
//-----------------------------------------------------------------------
/// Bind a C++ function to a LUX function of type
/// `iD*;iD&;iD+3,+3&;rD&`.
///
/// The LUX function arguments are:
///
/// 1. an input scalar or array of arbitrary dimensions, converted to
/// `double`.
///
/// 2. an input argument with the same dimensions as the first one,
/// converted to `double`.
///
/// 3. an input array with the same dimensions as the first one, but
/// with two dimensions equal to 3 prefixed, converted to `double`.
///
/// The LUX function returns a `double` array with the same dimensions
/// as the first argument.
///
/// The C++ function is schematically called like this:
///
/// \verbatim
/// *r++ = f(floor(*i0), *i0 - floor(*i0),
///          floor(*i1), *i1 - floor(*i1),
///          i2);
/// i0++;
/// i1++;
/// i2 += 9;
/// \endverbatim
///
/// @param [in] narg number of symbols in \p ps
/// @param [in] ps array of argument symbols
/// @param [in] f pointer to C function to bind
/// @return the symbol containing the result of the function call
int32_t
lux_d_dT4dp33_iaiqip3p3qrq_00112_3_f_(int32_t narg, int32_t ps[],
                                      double (*f)(double, double, double,
                                                  double, double [3][3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iD&;iD+3,+3&;rD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  double (*rnpb)[3] = (double (*)[3]) ptrs[2].d;
  while (infos[0].nelem--) {
    double uta, utb, tta, ttb;
    uta = floor(*ptrs[0].d);
    utb = *ptrs[0].d++ - uta;
    tta = floor(*ptrs[1].d);
    ttb = *ptrs[1].d++ - tta;
    *ptrs[3].d++ = f(uta, utb, tta, ttb, rnpb);
    rnpb += 3;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
/// Bind a C++ function to a LUX function of type
/// `i>L*;iD;iD;iD;iD;rD&`.
///
/// The LUX function arguments are:
///
/// 1. an input scalar or array of arbitrary dimensions, converted to
/// `int32` if it is less than that.
///
/// 2. four input scalars, converted to `double`.
///
/// The LUX function returns a `double` array with the same dimensions
/// as the first argument.
///
/// The C++ function is schematically called like this:
///
/// \verbatim
/// *r++ = f(*i0++, 0, *i1, *i2, *i3, *i4)
/// \endverbatim
///
/// @param [in] narg number of symbols in \p ps
/// @param [in] ps array of argument symbols
/// @param [in] f pointer to C++ function to bind
/// @return the symbol containing the result of the function call
int32_t
lux_d_dT6_iaiT4rq_0z1T4_5_f_(int32_t narg, int32_t ps[],
                             double (*f)(double, double, double, double,
                                         double, double))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "i>L*;iD;iD;iD;iD;rD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;

  switch (infos[0].type) {
  case LUX_INT32:
    while (infos[0].nelem--)
      *ptrs[5].d++ = f((double) *ptrs[0].l++, 0.0, *ptrs[1].d, *ptrs[2].d,
                       *ptrs[3].d, *ptrs[4].d);
    break;
  case LUX_INT64:
    while (infos[0].nelem--)
      *ptrs[5].d++ = f((double) *ptrs[0].q++, 0.0, *ptrs[1].d, *ptrs[2].d,
                       *ptrs[3].d, *ptrs[4].d);
    break;
  case LUX_FLOAT:
    while (infos[0].nelem--)
      *ptrs[5].d++ = f((double) *ptrs[0].f++, 0.0, *ptrs[1].d, *ptrs[2].d,
                       *ptrs[3].d, *ptrs[4].d);
    break;
  case LUX_DOUBLE:
    while (infos[0].nelem--)
      *ptrs[5].d++ = f(*ptrs[0].d++, 0.0, *ptrs[1].d, *ptrs[2].d,
                       *ptrs[3].d, *ptrs[4].d);
    break;
  default:
    break;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
/// Bind a C++ function to a LUX function of type `iD*;rD&`.
///
/// The LUX function arguments are:
///
/// 1. an input scalar or array of arbitrary dimensions, converted to
/// `double`.
///
/// The LUX function returns a `double` array with the same dimensions
/// as the first argument.
///
/// The C++ function is schematically called like this:
///
/// \verbatim
/// *r++ = f(*i0++)
/// \endverbatim
///
/// @param [in] narg number of symbols in \p ps
/// @param [in] ps array of argument symbols
/// @param [in] f pointer to C++ function to bind
/// @return the symbol containing the result of the function call
int32_t
lux_d_d_iarq_0_1_f_(int32_t narg, int32_t ps[], double (*f)(double))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;rD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;

  while (infos[0].nelem--)
    *ptrs[1].d++ = f(*ptrs[0].d++);
  return sa.result();
}
//-----------------------------------------------------------------------
/// Bind a C++ function to a LUX function of type `iD*;iD#;rD&`.
///
/// The LUX function arguments are:
///
/// 1. an input scalar or array of arbitrary dimensions, converted to
/// `double`.
///
/// 2. a scalar or an array with the same number of elements as (but
/// possibly a different dimensional structure than) the first one,
/// converted to `double`.
///
/// The LUX function returns a `double` array with the same dimensions
/// as the first argument.
///
/// The C++ function is called schematically like this:
///
/// \verbatim
/// *r++ = f(*i0++, *i1++)
/// \endverbatim
///
/// @param [in] narg number of symbols in \p ps
/// @param [in] ps array of argument symbols
/// @param [in] f pointer to C++ function to bind
/// @return the symbol containing the result of the function call
int32_t
lux_d_dd_iaibrq_01_2_f_(int32_t narg, int32_t ps[], double (*f)(double, double))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iD#;rD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  if (infos[1].nelem > 1) {
    // same number of elements in parameters 0 and 1
    while (infos[0].nelem--)
      *ptrs[2].d++ = f(*ptrs[0].d++, *ptrs[1].d++);
  } else {
    // parameter 1 is a scalar
    while (infos[0].nelem--)
      *ptrs[2].d++ = f(*ptrs[0].d++, *ptrs[1].d);
  }
  return sa.result();
}
//-----------------------------------------------------------------------
/// Bind a C++ function to a LUX function of type `iD*;iD&;rD&`.
///
/// The LUX function arguments are:
///
/// 1. an input scalar or array of arbitrary dimensions, converted to
/// `double`.
///
/// 2. an input argument with the same dimensions as the first one,
/// converted to `double`.
///
/// The LUX function returns a `double` array with the same dimensions
/// as the first argument.
///
/// The C++ function is schematically called like this:
///
/// \verbatim
/// *r++ = f(*i0++, *i1++)
/// \endverbatim
///
/// @param [in] narg number of symbols in \p ps
/// @param [in] ps array of argument symbols
/// @param [in] f pointer to C++ function to bind
/// @return the symbol containing the result of the function call
int32_t
lux_d_dd_iaiqrq_01_2_f_(int32_t narg, int32_t ps[], double (*f)(double, double))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iD&;rD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    *ptrs[2].d++ = f(*ptrs[0].d++, *ptrs[1].d++);
  }
  return sa.result();
}
//-----------------------------------------------------------------------
/// Bind a C++ function to a LUX function of type `i>L*;rD&`.
///
/// The LUX function arguments are:
///
/// 1. an input scalar or array of arbitrary dimensions, converted to
/// `int32` if it is less than that.
///
/// The LUX function returns a `double` array with the same dimensions
/// as the first argument.
///
/// The C++ function is called schematically like this:
///
/// \verbatim
/// *r++ = f(*i0++, 0)
/// \endverbatim
///
/// @param [in] narg number of symbols in \p ps
/// @param [in] ps array of argument symbols
/// @param [in] f pointer to C++ function to bind
/// @return the symbol containing the result of the function call
int32_t
lux_d_dd_iarq_0z_1_f_(int32_t narg, int32_t ps[], double (*f)(double, double))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "i>L*;rD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;

  switch (infos[0].type) {
  case LUX_INT32:
    while (infos[1].nelem--)
      *ptrs[1].d++ = f((double) *ptrs[0].l++, 0.0);
    break;
  case LUX_INT64:
    while (infos[1].nelem--)
      *ptrs[1].d++ = f((double) *ptrs[0].q++, 0.0);
    break;
  case LUX_FLOAT:
    while (infos[1].nelem--)
      *ptrs[1].d++ = f((double) *ptrs[0].f++, 0.0);
    break;
  case LUX_DOUBLE:
    while (infos[1].nelem--)
      *ptrs[1].d++ = f(*ptrs[0].d++, 0.0);
    break;
  default:
    break;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
/// Bind a C++ function to a LUX function of type `iD3,3*;iD-,-&;rD[-]&`.
///
/// The LUX function arguments are:
///
/// 1. an input array with the first two dimensions equal to 3, and
/// arbitrary following dimensions, converted to `double`.
///
/// 2. an input array with the same dimensions as the first argument
/// except that it omits the first two dimensions, converted to
/// `double`.
///
/// The LUX function returns a `double` array with the same dimensions
/// as the second argument.
///
/// The C++ function is called schematically like this:
///
/// \verbatim
/// *r++ = f(i0, *i1++);
/// i0 += 9;
/// \endverbatim
///
/// @param [in] narg number of symbols in \p ps
/// @param [in] ps array of argument symbols
/// @param [in] f pointer to C++ function to bind
/// @return the symbol containing the result of the function call
int32_t
lux_d_dp33d_i33aimmqrcq_01_2_f_(int32_t narg, int32_t ps[],
                                double (*f)(double [3][3], double))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD3,3*;iD-,-&;rD[-]&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  double (*rnpb)[3] = (double (*)[3]) ptrs[0].d;
  while (infos[1].nelem--) {
    *ptrs[2].d++ = f(rnpb, *ptrs[1].d++);
    rnpb += 3;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
/// Bind a C++ function to a LUX function of type `iD3*;rD-3&`.
///
/// The LUX function arguments are:
///
/// 1. an input array with the first dimension equal to 3, and
/// arbitrary following dimensions, converted to `double`.
///
/// The LUX function returns a `double` array with the same dimensions
/// as the first argument, except that it omits the first dimension.
///
/// The C++ function is called schematically like this:
///
/// \verbatim
/// *r++ = f(i0);
/// i0 += 3;
/// \endverbatim
///
/// @param [in] narg number of symbols in \p ps
/// @param [in] ps array of argument symbols
/// @param [in] f pointer to C++ function to bind
/// @return the symbol containing the result of the function call
int32_t
lux_d_dp3_i3arm3q_0_1_f_(int32_t narg, int32_t ps[], double (*f)(double [3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD3*;rD-3&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  int32_t nelem = infos[0].nelem/3;
  while (nelem--) {
    *ptrs[1].d++ = f(ptrs[0].d);
    ptrs[0].d += 3;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
/// Bind a C++ function to a LUX function of type `iD3*;iD&;rD-3&`.
///
/// The LUX function arguments are:
///
/// 1. an input array with the first dimension equal to 3, and
/// arbitrary following dimensions, converted to `double`.
///
/// 2. an input array with the same dimensions as the first one,
/// converted to `double`.
///
/// The LUX function returns a `double` array with the same dimensions
/// as the first argument, except that it omits the first dimension.
///
/// The C++ function is called schematically like this:
///
/// \verbatim
/// *r++ = f(i0, i1);
/// i0 += 3;
/// i1 += 3;
/// \endverbatim
///
/// @param [in] narg number of symbols in \p ps
/// @param [in] ps array of argument symbols
/// @param [in] f pointer to C++ function to bind
/// @return the symbol containing the result of the function call
int32_t
lux_d_dp3dp3_i3aiqrm3q_01_2_f_(int32_t narg, int32_t ps[],
                               double (*f)(double [3], double [3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD3*;iD&;rD-3&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[2].nelem--) {
    *ptrs[2].d++ = f(ptrs[0].d, ptrs[1].d);
    ptrs[0].d += 3;
    ptrs[1].d += 3;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
/// Bind a C++ function to a LUX function of type `iD*;rD-&` or
/// `iD*;iL*;rD{-}&`.
///
/// The LUX function arguments are:
///
/// 1. an input scalar or array of arbitrary dimensions, converted to
/// `double`.
///
/// 2. an optional input scalar or array of arbitrary dimensions,
/// converted to `int32`, which acts as an axis specification.
///
/// The LUX function returns a `double` array with the same dimensions
/// as the first argument, except that it omits the dimensions
/// indicated by the axis parameter, or omits the first dimension if
/// no axis parameter was specified.
///
/// The C++ function is called schematically like this:
///
/// \verbatim
/// *r++ = f(i0, i0.dims[0], i0.step[0]);
/// advance(i0);
/// advance(r);
/// \endverbatim
///
/// @param [in] narg number of symbols in \p ps
/// @param [in] ps array of argument symbols
/// @param [in] f pointer to C++ function to bind
/// @return the symbol containing the result of the function call
int32_t
lux_d_sd_iaiarxq_000_2_f_(int32_t narg, int32_t ps[],
                          double (*f)(double *, size_t count, size_t stride))
{
  Pointer *ptrs, ptrs0, ptrsr;
  LoopInfo *infos;
  int32_t iq, iret;
  int32_t *axes, naxes, oneaxis[1] = { 0 }, allaxes;

  StandardArguments_RAII sa;
  switch (narg) {
  case 1:                       // source
    if ((iq = sa.set(narg, ps, "iD*;rD-&", &ptrs, &infos)) < 0)
      return LUX_ERROR;
    axes = oneaxis;
    naxes = 1;
    setAxes(&infos[0], 0, NULL, SL_EACHROW);
    iret = 1;
    break;
  case 2:                       // source, axes
    if ((iq = sa.set(narg, ps, "iD*;iL*;rD{-}&", &ptrs, &infos)) < 0)
      return LUX_ERROR;
    axes = ptrs[1].l;
    naxes = infos[1].nelem;
    if (setAxes(&infos[0], infos[1].nelem, ptrs[1].l,
                SL_EACHROW | SL_UNIQUEAXES) < 0)
      return LUX_ERROR;
    iret = 2;
    break;
  }

  if (internalMode & 1) {       // /ALLAXES
    naxes = infos[0].ndim;
    axes = (int32_t*) malloc(naxes*sizeof(int32_t));
    allaxes = 1;
    int32_t i;
    for (i = 0; i < naxes; i++)
      axes[i] = i;
  } else
    allaxes = 0;  ptrs0 = ptrs[0];
  ptrsr = ptrs[iret];
  int32_t iaxis;
  for (iaxis = 0; iaxis < naxes; iaxis++) {
    setAxes(&infos[0], 1, &axes[iaxis], SL_EACHROW);
    ptrs[0] = ptrs0;
    ptrs[iret] = ptrsr;
    do {
      *ptrs[iret].d = f(ptrs[0].d, infos[0].rdims[0], infos[0].rsinglestep[0]);
      ptrs[0].d += infos[0].rsinglestep[1];
    } while (advanceLoop(&infos[iret], &ptrs[iret]),
             advanceLoop(&infos[0], &ptrs[0]) < infos[0].rndim);
  }
  if (allaxes)
    free(axes);
  return iq;
}
//-----------------------------------------------------------------------
/// Bind a C++ function to a LUX subroutine of type
/// `iD*;iD&;iD&;iD&;iD&;iD&;iD&;iD&;oD&;oD&;oD&;oD&;oD&;oD&`.
///
/// The LUX subroutine arguments are:
///
/// 1. an input scalar or array of arbitrary dimensions, converted to
/// `double`.
///
/// 2. seven arguments of the same dimensions as the first one,
/// converted to `double`.
///
/// 3. six output arguments changed to `double` and to the same
/// dimensions as the first input argument.
///
/// The C++ function is called schematically like this:
///
/// \verbatim
/// f(*i0++, *i1++, *i2++, *i3++, *i4++, *i5++, *i6++, 0, *i7++,
///   o8++, o9++, o10++, o11++, o12++, o13++)
/// \endverbatim
///
/// @param [in] narg number of symbols in \p ps
/// @param [in] ps array of argument symbols
/// @param [in] f pointer to C++ function to bind
/// @return the symbol containing the result of the function call
int32_t
lux_i_dT10dpT6_iaiqT7oqT6_0T6z7z8T13_s_(int32_t narg, int32_t ps[],
                                        int32_t (*f)(double, double, double,
                                                     double, double, double,
                                                     double, double, double,
                                                     double, double *, double *,
                                                     double *, double *,
                                                     double *, double *))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iD&;iD&;iD&;iD&;iD&;iD&;iD&;oD&"
                            ";oD&;oD&;oD&;oD&;oD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++, *ptrs[4].d++,
      *ptrs[5].d++, *ptrs[6].d++, 0.0, *ptrs[7].d++, 0.0, ptrs[8].d++,
      ptrs[9].d++, ptrs[10].d++, ptrs[11].d++, ptrs[12].d++, ptrs[13].d++);
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD*;iD&;rD&`.
//
// The LUX function arguments are:
//
// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.
//
// 2. an input argument of the same dimensions as the first one,
// converted to `double`.
//
// The LUX function returns a `double` array with the same dimensions
// as the first argument.
//
// The C++ function is called schematically like this:
//
// \verbatim
// f(*i0++, 0, *i1++, &a, &b);
// *r++ = a + b;
// \endverbatim
//
// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_i_dT3dpdp_iaiqrq_0z122_f_(int32_t narg, int32_t ps[],
                              int32_t (*f)(double, double, double, double *,
                                           double *))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iD&;rD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    double ut1, ut2;
    f(*ptrs[0].d++, 0.0, *ptrs[1].d++, &ut1, &ut2);
    *ptrs[2].d++ = ut1 + ut2;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
/// Bind a C++ function to a LUX function of type `iD3*;iD;iD;rD&`.
///
/// The LUX function arguments are:
///
/// 1. an input array with 3 elements in its first dimension, followed
/// by arbitrary other dimensions, converted to `double`.
///
/// 2. a scalar or single-element array, converted to `double`.
///
/// 3. a scalar or single-element array, converted to `double`.
///
/// The LUX function returns a `double` array with the same dimensions
/// as the first argument.
///
/// The C++ function is called schematically like this:
///
/// \verbatim
/// f(*i1, *i2, i0[0], i0[1], i0[2], r);
/// i0 += 3;
/// r += 3;
/// \endverbatim
///
/// @param [in] narg number of symbols in \p ps
/// @param [in] ps array of argument symbols
/// @param [in] f pointer to C++ function to bind
/// @return the symbol containing the result of the function call
int32_t
lux_i_dT5dp3_i3aiirq_120003_f_(int32_t narg, int32_t ps[],
                               int32_t (*f)(double, double, double, double,
                                            double, double [3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD3*;iD;iD;rD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  size_t nelem = infos[0].nelem/3;
  while (nelem--) {
    f(*ptrs[1].d, *ptrs[2].d, ptrs[0].d[0], ptrs[0].d[1], ptrs[0].d[2],
      ptrs[3].d);
    ptrs[0].d += 3;
    ptrs[3].d += 3;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
/// Bind a C++ function to a LUX function of type
/// `iD*;iD&;iD&;iD&;iD&;iD&;oD+2,+3&`.
///
/// The LUX function arguments are:
///
/// 1. an input scalar or array of arbitrary dimensions, converted to
/// `double`.
///
/// 2. five input arguments with the same dimensions as the first one,
/// converted to `double`.
///
/// The LUX function returns a `double` array with the same dimensions
/// as the first argument, except that dimensions with 2 and 3
/// elements are prefixed.
///
/// The C++ function is called schematically like this:
///
/// \verbatim
/// f(*i0++, *i1++, *i2++, *i3++, *i4++, *i5++, r);
/// r += 6;
/// \endverbatim
///
/// @param [in] narg number of symbols in \p ps
/// @param [in] ps array of argument symbols
/// @param [in] f pointer to C++ function to bind
/// @return the symbol containing the result of the function call
int32_t
lux_i_dT6dp23_iaiqT5op2p3q_0T6_f_(int32_t narg, int32_t ps[],
                                  int32_t (*f)(double, double, double, double,
                                               double, double, double [2][3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iD&;iD&;iD&;iD&;iD&;oD+2,+3&",
                            &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++, *ptrs[4].d++,
      *ptrs[5].d++, (double (*)[3]) ptrs[6].d);
    ptrs[6].d += 6;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX subroutine of type `i>L*;oD+2,+3&;oD[-]&`.

// The LUX function arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `int32` if it is less.

// 2. an output argument with the same dimensions as the first one,
// except that dimensions equal to 2 and 3 are prefixed.

// 3. an output argument with the same dimensions as the previous
// one, converted to `double`.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, 0, o1, o2);
// o1 += 6;
// o2 += 6;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_i_dddp23T2_iaop2p3qocq_0z12_s_(int32_t narg, int32_t ps[],
                                   int32_t (*f)(double, double, double [2][3],
                                                double [2][3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "i>L*;oD+2,+3&;oD[-]&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  double (*pvh)[3] = (double (*)[3]) ptrs[1].d;
  double (*pvb)[3] = (double (*)[3]) ptrs[2].d;

  switch (infos[0].type) {
  case LUX_INT32:
    while (infos[0].nelem--) {
      f((double) *ptrs[0].l++, 0.0, pvh, pvb);
      pvh += 2;
      pvb += 2;
    }
    break;
  case LUX_INT64:
    while (infos[0].nelem--) {
      f((double) *ptrs[0].q++, 0.0, pvh, pvb);
      pvh += 2;
      pvb += 2;
    }
    break;
  case LUX_FLOAT:
    while (infos[0].nelem--) {
      f((double) *ptrs[0].f++, 0.0, pvh, pvb);
      pvh += 2;
      pvb += 2;
    }
    break;
  case LUX_DOUBLE:
    while (infos[0].nelem--) {
      f(*ptrs[0].d++, 0.0, pvh, pvb);
      pvh += 2;
      pvb += 2;
    }
    break;
  default:
    break;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD*;rD&`.

// The LUX function arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// The LUX function returns a `double` array with the same dimensions
// as the first argument.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, 0, &a, &b);
// *r++ = a + b;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_i_dddpdp_iarq_0z11_f_(int32_t narg, int32_t ps[],
                          int32_t (*f)(double, double, double *, double *))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;rD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    double tt1, tt2;
    f(*ptrs[0].d++, 0.0, &tt1, &tt2);
    *ptrs[1].d++ = tt1 + tt2;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD*;iL;rD+3,+2&`.

// The LUX function arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// 2. an input scalar or single-element array, converted to `int32`.

// The LUX function returns a `double` array with the same dimensions
// as the first argument, except that dimensions equal to 3 and 2 are
// prefixed.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, 0, *i1, r);
// r += 6;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_i_ddidp23_iairp3p2q_0z12_f_(int32_t narg, int32_t ps[],
                                int32_t (*f)(double, double, int32_t,
                                             double [2][3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iL;rD+3,+2&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;

  while (infos[0].nelem--) {
    f(*ptrs[0].d++, 0.0, *ptrs[1].l, (double (*)[3]) ptrs[2].d);
    ptrs[2].d += 6;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `i>L*;rD+3&`.

// The LUX function arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `int32` if it is less than that.

// The LUX function returns a `double` array with the same dimensions
// as the argument, except that a single dimension with 3 elements is
// prefixed.

// The C++ function is called schematically like this:

// \verbatim
// if (f(*i0++, 0, &y, &m, &d, &fd)) {
//   *r[0] = *r[1] = *r[2] = 0;
// }
// else {
//   *r[0] = y;
//   *r[1] = m;
//   *r[2] = d + fd;
// }
// r += 3;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_i_ddipT3dp_iarp3q_0z1111_f_(int32_t narg, int32_t ps[],
                                int32_t (*f)(double, double, int32_t *,
                                             int32_t *, int32_t *, double *))
{
  Pointer *ptrs;
  LoopInfo *infos;
  int32_t y, m, d;
  double fd;

  StandardArguments_RAII sa(narg, ps, "i>L*;rD+3&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;

  switch (infos[0].type) {
  case LUX_INT32:
    while (infos[0].nelem--) {
      if (f((double) *ptrs[0].l++, 0.0, &y, &m, &d, &fd)) {
        ptrs[1].d[0] = 0.0;
        ptrs[1].d[1] = 0.0;
        ptrs[1].d[2] = 0.0;
      } else {
        ptrs[1].d[0] = (double) y;
        ptrs[1].d[1] = (double) m;
        ptrs[1].d[2] = (double) d + fd;
      }
      ptrs[1].d += 3;
    }
    break;
  case LUX_INT64:
    while (infos[0].nelem--) {
      if (f((double) *ptrs[0].q++, 0.0, &y, &m, &d, &fd)) {
        ptrs[1].d[0] = 0.0;
        ptrs[1].d[1] = 0.0;
        ptrs[1].d[2] = 0.0;
      } else {
        ptrs[1].d[0] = (double) y;
        ptrs[1].d[1] = (double) m;
        ptrs[1].d[2] = (double) d + fd;
      }
      ptrs[1].d += 3;
    }
    break;
  case LUX_FLOAT:
    while (infos[0].nelem--) {
      if (f((double) *ptrs[0].f++, 0.0, &y, &m, &d, &fd)) {
        ptrs[1].d[0] = 0.0;
        ptrs[1].d[1] = 0.0;
        ptrs[1].d[2] = 0.0;
      } else {
        ptrs[1].d[0] = (double) y;
        ptrs[1].d[1] = (double) m;
        ptrs[1].d[2] = (double) d + fd;
      }
      ptrs[1].d += 3;
    }
    break;
  case LUX_DOUBLE:
    while (infos[0].nelem--) {
      if (f(*ptrs[0].d++, 0.0, &y, &m, &d, &fd)) {
        ptrs[1].d[0] = 0.0;
        ptrs[1].d[1] = 0.0;
        ptrs[1].d[2] = 0.0;
      } else {
        ptrs[1].d[0] = (double) y;
        ptrs[1].d[1] = (double) m;
        ptrs[1].d[2] = (double) d + fd;
      }
      ptrs[1].d += 3;
    }
    break;
  default:
    return cerror(ILL_ARG, ps[0]);
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX subroutine of type
// `iD2,3*;oD-2-3&;oD[-]&;oD[-]&;oD[-]&;oD[-]&;oD[-]&`.

// The LUX function arguments are:

// 1. an input array with 2 and 3 elements in the first two
// dimensions, and arbitrary following dimensions, converted to
// `double`.

// 2. an output argument changed to be `double` and to have the same
// dimensions as the first one, except that the first two dimensions
// are omitted.

// 3. five output arguments changed similarly to the first output
// argument.

// The C++ function is called schematically like this:

// \verbatim
// f(i0, i1++, i2++, i3++, i4++, i5++, i6++);
// i0 += 6;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_i_dp23dpT6_i23aom2m3qomqT5_0T6_s_(int32_t narg, int32_t ps[],
                                      int32_t (*f)(double [2][3], double *,
                                                   double *, double *, double *,
                                                   double *, double *))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps,
                            "iD2,3*;oD-2-3&;oD[-]&;oD[-]&;oD[-]&;oD[-]&;oD[-]&",
                            &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  size_t nelem = infos[0].nelem/6;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, ptrs[1].d++, ptrs[2].d++, ptrs[3].d++,
      ptrs[4].d++, ptrs[5].d++, ptrs[6].d++);
    ptrs[0].d += 6;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD*;iL;iL;rD&`

// The LUX function arguments are:

// 1. an input scalar or array with arbitrary dimensions, converted
// to `double`.

// 2. an input scalar or single-element array, converted to `int32`.

// 3. an input scalar or single-element array, converted to `int32`.

// The LUX function returns a `double` scalar or array with the same
// dimensions as the first argument.

// The C++ function is called schematically like this:

// \verbatim
// f(i0, i0.dims[0], *i1, *i2, r);
// i0 += i0.dims[0];
// r += i0.dims[0];
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_i_dpiT3dp_iaiirq_00T3_f_(int32_t narg, int32_t ps[],
                             int32_t (*f)(double *, int32_t, int32_t,
                                          int32_t, double *))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iL;iL;rD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  size_t nrepeat = infos[0].nelem/infos[0].rdims[0];
  while (nrepeat--) {
    f(ptrs[0].d, infos[0].rdims[0], *ptrs[1].l, *ptrs[2].l, ptrs[3].d);
    ptrs[0].d += infos[0].rdims[0];
    ptrs[3].d += infos[0].rdims[0];
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD*;iL;rD&`.

// The LUX function arguments are:

// 1. an input scalar or array with arbitrary dimensions, converted
// to `double`.

// 2. an input scalar or single-element array, converted to `int32`.

// The LUX function returns a `double` scalar or array with the same
// dimensions as the first argument.

// The C++ function is called schematically like this:

// \verbatim
// f(i0, i0.dims[0], *i1, r);
// i0 += i0.dims[0];
// r += i0.dims[0];
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_i_dpiidp_iairq_00T2_f_(int32_t narg, int32_t ps[],
                           int32_t (*f)(double *, int32_t, int32_t, double *))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iL;rD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  size_t nrepeat = infos[0].nelem/infos[0].rdims[0];
  while (nrepeat--) {
    f(ptrs[0].d, infos[0].rdims[0], *ptrs[1].l, ptrs[2].d);
    ptrs[0].d += infos[0].rdims[0];
    ptrs[2].d += infos[0].rdims[0];
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD3*;iL;rD&`.

// The LUX function arguments are:

// 1. an input array with 3 elements in its first dimension, and
// arbitrary following dimensions, converted to `double`.

// 2. an input scalar or single-element array, converted to `int32`.

// The LUX function returns a `double` scalar or array with the same
// dimensions as the first argument.

// The C++ function is called schematically like this:

// \verbatim
// f(*i1, i0[0], i0[1], i0[2], r);
// i0 += 3;
// r += 3;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_i_idT3dp3_i3airq_10002_f_(int32_t narg, int32_t ps[],
                              int32_t (*f)(int32_t, double, double, double,
                                           double [3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD3*;iL;rD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  size_t nelem = infos[0].nelem/3;
  while (nelem--) {
    f(*ptrs[1].l, ptrs[0].d[0], ptrs[0].d[1], ptrs[0].d[2], ptrs[2].d);
    ptrs[0].d += 3;
    ptrs[2].d += 3;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iL;iD3*;rD[-]&`.

// The LUX function arguments are:

// 1. an input scalar or single-element array, converted to `int32`.

// 2. an input array with 3 elements in its first dimension, and
// arbitrary following dimensions, converted to `double`.

// The LUX function returns a `double` scalar or array with the same
// dimensions as the second argument.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0, i1, &r[0], &r[1], &r[2]);
// i1 += 3;
// r += 3;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_i_idp3dpT3_ii3arcq_0T222_f_(int32_t narg, int32_t ps[],
                                int32_t (*f)(int32_t, double [3], double *,
                                             double *, double *))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iL;iD3*;rD[-]&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  size_t nelem = infos[1].nelem/3;
  while (nelem--) {
    f(*ptrs[0].l, ptrs[1].d, &ptrs[2].d[0], &ptrs[2].d[1], &ptrs[2].d[2]);
    ptrs[1].d += 3;
    ptrs[2].d += 3;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iL*;rD+2&`.

// The LUX function arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `int32`.

// The LUX function returns a `double` array with the same dimensions
// as the argument, except that a single dimension with 2 elements is
// prefixed.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, &r[0], &r[1]);
// r += 2;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_i_idpdp_iarp2q_011_f_(int32_t narg, int32_t ps[],
                          int32_t (*f)(int32_t, double *, double *))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iL*;rD+2&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    f(*ptrs[0].l++, &ptrs[1].d[0], &ptrs[1].d[1]);
    ptrs[1].d += 2;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C pointer-count-stride function to a LUX subroutine of type
// `iD*` or `iD*;iL*`.

// The LUX subroutine arguments are:

// 1. an `double` input/output scalar or array of arbitrary
// dimensions.  It must be a named variable.

// 2. optionally, an input scalar or array of arbitrary dimensions,
// converted to `int32` and treated as an axis parameter.

// The mode argument `/allaxes` (`internalMode & 1`) is equivalent to
// setting the second argument equal to an array listing all axes of
// the first argument.

// The C++ function is called schematically like this:

// \verbatim
// f(i0, i0.dims[0], i0.step[0]);
// advance(i0);
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_i_sd_iaia_000_s_(int32_t narg, int32_t ps[], int32_t (*f)(double *, size_t count, size_t stride))
{
  return lux_ivarl_copy_eachaxis_(narg, ps, f, 0);
}
//-----------------------------------------------------------------------
// Bind a C pointer-count-stride function to a LUX function of type
// `iD*;rD&` or `iD*;iL*;rD&`.

// The LUX function arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// 2. optionally, an input scalar or array of arbitrary dimensions,
// converted to `int32` and treated as an axis parameter.

// The mode argument `/allaxes` (`internalMode & 1`) is equivalent to
// setting the second argument equal to an array listing all axes of
// the first argument.

// The LUX function returns a `double` value with the same dimensions
// as the input argument.  It first copies the first argument to the
// return value, and then applies the C++ function to the return value.

// The C++ function is called schematically like this:

// \verbatim
// copy(i0, r);                // take into account stride
// f(r, r.dims[0], r.step[0]);
// advance(r);
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_i_sd_iaiarq_000_f_(int32_t narg, int32_t ps[], int32_t (*f)(double *, size_t count, size_t stride))
{
  return lux_ivarl_copy_eachaxis_(narg, ps, f, 1);
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD*;iL?;iD;iD?;rD&`.

// The LUX function arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `int32`.

// The LUX function returns a `double` array with the same dimensions
// as the argument, except that a single dimension with 2 elements is
// prefixed.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, &r[0], &r[1]);
// r += 2;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_i_sdddsd_iaiiirq_000T333_f_(int32_t narg, int32_t ps[],
                                int32_t (*f)(double *, size_t srcount,
                                             size_t srcstride, double par1,
                                             double par2, double *,
                                             size_t tgtcount, size_t tgtstride))
{
  Pointer *ptrs;
  LoopInfo *infos;
  int32_t iq, ipar1, ipar2, iret;

  StandardArguments_RAII sa;
  switch (narg) {
  case 2:                       // source, param1; (param2 = 0)
    if ((iq = sa.set(narg, ps, "iD*;iD;rD&", &ptrs, &infos)) < 0)
      return LUX_ERROR;
    ipar1 = 1;
    ipar2 = -1;
    iret = 2;
    setAxes(&infos[0], 0, NULL, SL_EACHROW);
    setAxes(&infos[iret], 0, NULL, SL_EACHROW);
    break;
  case 3:                       // source, param1, param2
    if ((iq = sa.set(narg, ps, "iD*;iD;iD;rD&", &ptrs, &infos)) < 0)
      return LUX_ERROR;
    ipar1 = 1;
    ipar2 = 2;
    iret = 3;
    setAxes(&infos[0], 0, NULL, SL_EACHROW);
    setAxes(&infos[iret], 0, NULL, SL_EACHROW);
    break;
  case 4:                       // source, axis, param1, param2
    if ((iq = sa.set(narg, ps, "iD*;iL;iD;iD;rD&", &ptrs, &infos)) < 0)
      return LUX_ERROR;
    ipar1 = 2;
    ipar2 = 3;
    iret = 4;
    setAxes(&infos[0], infos[1].nelem, ptrs[1].l, SL_EACHROW);
    setAxes(&infos[iret], infos[1].nelem, ptrs[1].l, SL_EACHROW);
    break;
  }
  do {
    f(ptrs[0].d, infos[0].rdims[0], infos[0].rsinglestep[0],
      *ptrs[ipar1].d, (ipar2 >= 0? *ptrs[ipar2].d: 0.0),
      ptrs[iret].d, infos[0].rdims[0], infos[iret].rsinglestep[0]);
    ptrs[0].d += infos[0].rsinglestep[1];
    ptrs[iret].d += infos[iret].rsinglestep[1];
  } while (advanceLoop(&infos[0], &ptrs[0]),
           advanceLoop(&infos[iret], &ptrs[iret])
           < infos[iret].rndim);
  return iq;
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD*;iD&;iD&;rD+3,+3&`.

// The LUX function arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// 2. a input array of the same dimensions as the first one,
// converted to `double`.

// 3. a input array of the same dimensions as the first one,
// converted to `double`.

// The LUX function returns a `double` array with the same dimensions
// as the first argument, except that two dimensions with 3 element
// are prefixed.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, *i1++, *i2++, r);
// r += 9;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dT3dp33_iaiqiqrp3p3q_0T3_f_(int32_t narg, int32_t ps[],
                                  void (*f)(double, double, double,
                                            double [3][3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iD&;iD&;rD+3,+3&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  do {
    f(*ptrs[0].d, *ptrs[1].d, *ptrs[2].d, (double (*)[3]) ptrs[3].d);
  } while (advanceLoop(&infos[0], &ptrs[3]),
           advanceLoop(&infos[1], &ptrs[2]),
           advanceLoop(&infos[2], &ptrs[1]),
           advanceLoop(&infos[3], &ptrs[0]) < infos[0].rndim);
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD*;iD&;iD&;rD+3&`.

// The LUX function arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// 2. a input array of the same dimensions as the first one,
// converted to `double`.

// 3. a input array of the same dimensions as the first one,
// converted to `double`.

// The LUX function returns a `double` array with the same dimensions
// as the first argument, except that one dimension with 3 elements
// is prefixed.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, *i1++, *i2++, r);
// r += 3;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dT3dp3_iaiqiqrp3q_0T3_f_(int32_t narg, int32_t ps[],
                               void (*f)(double, double, double, double [3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iD&;iD&;rD+3&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;

  while (infos[0].nelem--) {
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, ptrs[3].d);
    ptrs[3].d += 3;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD*;iD&;iD&;iD&;rD+3,+3&`.

// The LUX function arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// 2. a input array of the same dimensions as the first one,
// converted to `double`.

// 3. a input array of the same dimensions as the first one,
// converted to `double`.

// 4. a input array of the same dimensions as the first one,
// converted to `double`.

// The LUX function returns a `double` array with the same dimensions
// as the first argument, except that two dimensions with 3 elements
// each are prefixed.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, *i1++, *i2++, *i3++, r);
// r += 9;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dT4dp33_iaiqT3rp3p3q_0T4_f_(int32_t narg, int32_t ps[],
                                  void (*f)(double, double, double,
                                            double, double [3][3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iD&;iD&;iD&;rD+3,+3&",
                            &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  double (*r)[3] = (double (*)[3]) ptrs[4].d;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++, r);
    r += 3;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD*;iD&;iD&;rD+3,+3&`.

// The LUX function arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// 2. a input array of the same dimensions as the first one,
// converted to `double`.

// 3. a input array of the same dimensions as the first one,
// converted to `double`.

// The LUX function returns a `double` array with the same dimensions
// as the first argument, except that two dimensions with 3 elements
// each are prefixed.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, 0, *i1++, *i2++, r);
// r += 9;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dT4dp33_iaiqiqrp3p3q_0z1T3_f_(int32_t narg, int32_t ps[],
                                    void (*f)(double, double, double,
                                              double, double [3][3]))
{
  double (*tgt)[3];
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iD&;iD&;rD+3,+3&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  tgt = (double (*)[3]) ptrs[3].d;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, 0.0, *ptrs[1].d++, *ptrs[2].d++, tgt);
    tgt += 3;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX subroutine of type `iD*;iD&;oD&;oD&;oD&`.

// The LUX subroutine arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// 2. a input array of the same dimensions as the first one,
// converted to `double`.

// 3. an output variable changed to be a `double` array of the same
// dimensions as the first argument.

// 4. an output variable changed to be a `double` array of the same
// dimensions as the first argument.

// 5. an output variable changed to be a `double` array of the same
// dimensions as the first argument.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, 0, *i1++, 0, o2++, o3++, o4++)
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dT4dpT3_iaiqoqT3_0z1z2T4_s_(int32_t narg, int32_t ps[],
                                  void (*f)(double, double, double, double,
                                            double *, double *, double *))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iD&;oD&;oD&;oD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, 0.0, *ptrs[1].d++, 0.0, ptrs[2].d++, ptrs[3].d++,
      ptrs[4].d++);
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX subroutine of type
// `iD*;iD&;iD&;oD&;oD&;oD&;oD&`.

// The LUX subroutine arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// 2. two input arrays of the same dimensions as the first one,
// converted to `double`.

// 3. four output variables changed to be `double` arrays of the same
// dimensions as the first argument.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, *i1++, *i2++, 0, o3++, o4++, o5++, o6++)
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dT4dpT4_iaiqiqoqT4_0T2z3T6_s_(int32_t narg, int32_t ps[],
                                    void (*f)(double, double, double, double,
                                              double *, double *, double *,
                                              double *))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iD&;iD&;oD&;oD&;oD&;oD&",
                            &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, 0.0, ptrs[3].d++,
      ptrs[4].d++, ptrs[5].d++, ptrs[6].d++);
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX subroutine of type
// `iD*;iD&;iD&;oD&;oD+3,+3&;oD[-]&;oD[-]&;oD[-]&;oD[-]&`.

// The LUX subroutine arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// 2. two input arrays of the same dimensions as the first one,
// converted to `double`.

// 3. one output variable changed to be a `double` array of the same
// dimensions as the first argument.

// 3. five output variables changed to be `double` arrays of the same
// dimensions as the first argument, except that two dimensions equal
// to 3 each are prefixed.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, 0, *i1++, *i2++, i3++, o4, o5, o6, o7, o8);
// o4 += 9;
// o5 += 9;
// o6 += 9;
// o7 += 9;
// o8 += 9;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dT4dpdp3T5_iaiqiqoqop3p3qocqT4_0z1T8_s_(int32_t narg, int32_t ps[],
                                              void (*f)(double, double, double,
                                                        double, double *,
                                                        double [3][3],
                                                        double [3][3],
                                                        double [3][3],
                                                        double [3][3],
                                                        double [3][3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iD&;iD&;oD&;oD+3,+3&;oD[-]&;"
                            "oD[-]&;oD[-]&;oD[-]&",
                            &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, 0.0, *ptrs[1].d++, *ptrs[2].d++, ptrs[3].d++,
      (double (*)[3]) ptrs[4].d, (double (*)[3]) ptrs[5].d,
      (double (*)[3]) ptrs[6].d, (double (*)[3]) ptrs[7].d,
      (double (*)[3]) ptrs[8].d);
    ptrs[4].d += 9;
    ptrs[5].d += 9;
    ptrs[6].d += 9;
    ptrs[7].d += 9;
    ptrs[8].d += 9;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX subroutine of type
// `iD*;iD&;iD&;iD&;oD&;oD&`.

// The LUX subroutine arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// 2. three input arrays of the same dimensions as the first one,
// converted to `double`.

// 3. two output variables changed to be `double` arrays of the same
// dimensions as the first argument.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, *i1++, *i2++, *i3++, o4++, o5++);
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dT4dpdp_iaiqT3oqoq_0T5_s_(int32_t narg, int32_t ps[],
                                void (*f)(double, double, double, double,
                                          double *, double *))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iD&;iD&;iD&;oD&;oD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++,
      ptrs[4].d++, ptrs[5].d++);
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX subroutine of type
// `iD*;iD&;iD&;oD&;oD&`.

// The LUX subroutine arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// 2. two input arrays of the same dimensions as the first one,
// converted to `double`.

// 3. two output variables changed to be `double` arrays of the same
// dimensions as the first argument.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, *i1++, *i2++, 0, o3++, o4++);
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dT4dpdp_iaiqiqoqoq_0T2z34_s_(int32_t narg, int32_t ps[],
                                   void (*f)(double, double, double, double,
                                             double *, double *))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iD&;iD&;oD&;oD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, 0.0, ptrs[3].d++, ptrs[4].d++);
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type
// `iD*;iD&;iD&;iD&;iD&;iD&;rD+2,+3&`.

// The LUX function arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// 2. five more input scalars or arrays of the same dimensions as the
// first one, converted to `double`.

// The LUX function returns a `double` array with the same dimensions
// as the first argument, except that dimensions with 2 and 3
// elements are prefixed.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, *i1++, *i2++, *i3++, *i4++, *i5++, r);
// r += 6;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dT6dp23_iaiqT5op2p3q_0T6_f_(int32_t narg, int32_t ps[],
                                  void (*f)(double, double, double, double,
                                            double, double, double [2][3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iD&;iD&;iD&;iD&;iD&;oD+2,+3&",
                            &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++, *ptrs[4].d++,
      *ptrs[5].d++, (double (*)[3]) ptrs[6].d);
    ptrs[6].d += 6;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD*;iD&;iD&;iD&;rD+3,+3&`.

// The LUX function arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// 2. three input scalars or arrays of the same dimensions as the
// first one, converted to `double`.

// The LUX function returns a `double` array with the same dimensions
// as the first argument, except that two dimensions with 3 elements
// each are prefixed.

// The C++ function is called schematically like this:

// \verbatim
// d = floor(*i1 - 0.5) + 0.5;
// t = *i1++ - d;
// f(*i0++, 0, d, t, *i2++, *i3++, r);
// r += 9;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dT6dp33_iaiqT3rp3p3q_0z11T4_f_(int32_t narg, int32_t ps[],
                                     void (*f)(double, double, double,
                                               double, double, double,
                                               double [3][3]))
{
  Pointer *ptrs;
  LoopInfo* infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iD&;iD&;iD&;rD+3,+3&",
                            &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    double d, t;
    d = floor(*ptrs[1].d - 0.5) + 0.5;
    t = *ptrs[1].d++ - d;
    f(*ptrs[0].d++, 0.0, d, t, *ptrs[2].d++, *ptrs[3].d++,
      (double (*)[3]) ptrs[4].d);
    ptrs[4].d += 9;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX subroutine of type
// `iD*;iD&;iD&;iD&;iD&;iD&;oD&;oD&;oD&;oD&;oD&;oD&`.

// The LUX subroutine arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// 2. five more input arrays of the same dimensions as the first one,
// converted to `double`.

// 3. six output variables changed to be `double` arrays of the same
// dimensions as the first argument.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, *i1++, *i2++, *i3++, *i4++, *i5++,
//   o6++, o7++, o8++, o9++, o10++, o11++);
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dT6dpT6_iaiqT5oqT6_0T11_s_(int32_t narg, int32_t ps[],
                                 void (*f)(double, double, double, double,
                                           double, double, double *,
                                           double *, double *, double *,
                                           double *, double *))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps,
                            "iD*;iD&;iD&;iD&;iD&;iD&;oD&;oD&;oD&;oD&;oD&;oD&",
                            &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++,
      *ptrs[4].d++, *ptrs[5].d++, ptrs[6].d++, ptrs[7].d++,
      ptrs[8].d++, ptrs[9].d++, ptrs[10].d++, ptrs[11].d++);
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type
// `iD*;iD&;iD&;iD&;iD&;rD+3,+3&`.

// The LUX function arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// 2. four more input scalars or arrays of the same dimensions as the
// first one, converted to `double`.

// The LUX function returns a `double` array with the same dimensions
// as the first argument, except that two dimensions of 3 elements
// each are prefixed.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, 0, 0, 0, *i1++, *i2++, *i3++, *i4++, r);
// r += 9;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dT8dp33_iaiqT4rp3p3q_0zzz1T5_f_(int32_t narg, int32_t ps[],
                                      void (*f)(double, double, double,
                                                double, double, double,
                                                double, double, double [3][3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iD&;iD&;iD&;iD&;rD+3,+3&",
                            &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  double *jd = ptrs[0].d;
  double *dpsi = ptrs[1].d;
  double *deps = ptrs[2].d;
  double *xp = ptrs[3].d;
  double *yp = ptrs[4].d;
  double (*rc2t)[3] = (double (*)[3]) ptrs[5].d;
  size_t nelem = infos[0].nelem;
  while (nelem--) {
    f(*jd++, 0, 0, 0, *dpsi++, *deps++, *xp++, *yp++, rc2t);
    rc2t += 3;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD*;iD+3,+3&;rD[-]&`.

// The LUX function arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// 2. an input array of the same dimensions as the first one, except
// that two dimensions of 3 element each are prefixed, converted to
// `double`.

// The LUX function returns a `double` array with the same dimensions
// as the last argument.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, 0, i1, r);
// i1 += 9;
// r += 9;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dddp33T2_iaip3p3qrcq_0z12_f_(int32_t narg, int32_t ps[],
                                   void (*f)(double, double, double [3][3],
                                             double [3][3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iD+3,+3&;rD[-]&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;

  double (*r1)[3] = (double (*)[3]) ptrs[1].d;
  double (*r2)[3] = (double (*)[3]) ptrs[2].d;

  while (infos[0].nelem--) {
    f(*ptrs[0].d++, 0.0, r1, r2);
    r1 += 3;
    r2 += 3;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX subroutine of type
// `i>L*;oD3,3;oD+3,+3&;oD[-]&`.

// The LUX subroutine arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `int32` if it is less than that.

// 2. an output variable changed to be a `double` array of 3 by 3
// elements.

// 3. two output variables changed to be `double` arrays of the same
// dimensions as the first argument, except that two dimensions of 3
// elements each are prefixed.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, 0, o1, o2, o3);
// o2 += 9;
// o3 += 9;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dddp33T3_iao33op3p3qocq_0z1T3_s_(int32_t narg, int32_t ps[],
                                       void (*f)(double, double, double [3][3],
                                                 double [3][3], double [3][3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "i>L*;oD3,3;oD+3,+3&;oD[-]&",
                            &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;

  switch (infos[0].type) {
  case LUX_INT32:
    while (infos[0].nelem--) {
      f((double) *ptrs[0].l++, 0.0, (double (*)[3]) ptrs[1].d,
        (double (*)[3]) ptrs[2].d, (double (*)[3]) ptrs[3].d);
      ptrs[2].d += 9;
      ptrs[3].d += 9;
    }
    break;
  case LUX_INT64:
    while (infos[0].nelem--) {
      f((double) *ptrs[0].q++, 0.0, (double (*)[3]) ptrs[1].d,
        (double (*)[3]) ptrs[2].d, (double (*)[3]) ptrs[3].d);
      ptrs[2].d += 9;
      ptrs[3].d += 9;
    }
    break;
  case LUX_FLOAT:
    while (infos[0].nelem--) {
      f((double) *ptrs[0].f++, 0.0, (double (*)[3]) ptrs[1].d,
        (double (*)[3]) ptrs[2].d, (double (*)[3]) ptrs[3].d);
      ptrs[2].d += 9;
      ptrs[3].d += 9;
    }
    break;
  case LUX_DOUBLE:
    while (infos[0].nelem--) {
      f(*ptrs[0].d++, 0.0, (double (*)[3]) ptrs[1].d,
        (double (*)[3]) ptrs[2].d, (double (*)[3]) ptrs[3].d);
      ptrs[2].d += 9;
      ptrs[3].d += 9;
    }
    break;
  default:
    break;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD*;rD+3,+3&`.

// The LUX function arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// The LUX function returns a `double` array with the same dimensions
// as the first argument, except that two dimensions of 3 elements
// each are prefixed.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, 0, r);
// r += 9;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dddp33_iarp3p3q_0z1_f_(int32_t narg, int32_t ps[],
                             void (*f)(double, double, double [3][3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;rD+3,+3&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  double (*r)[3] = (double (*)[3]) ptrs[1].d;

  while (infos[0].nelem--) {
    f(*ptrs[0].d++, 0.0, r);
    r += 3;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD*;iD&;rD+3&`.

// The LUX function arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// 2. an input array of the same dimensions as the first one,
// converted to `double`.

// The LUX function returns a `double` array with the same dimensions
// as the first argument, except that a dimension equal to 3 is
// prefixed.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, *i1++, r)
// r += 3;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dddp3_iaiqrp3q_0T2_f_(int32_t narg, int32_t ps[],
                            void (*f)(double, double, double [3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iD&;rD+3&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;

  while (infos[0].nelem--) {
    f(*ptrs[0].d++, *ptrs[1].d++, ptrs[2].d);
    ptrs[2].d += 3;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD3*;iD;iD;rD&`.

// The LUX function arguments are:

// 1. an input array with 3 elements in its first dimension, and
// arbitrary following dimensions, converted to `double`.

// 2. a scalar or single-element input array, converted to `double`.

// 3. a scalar or single-element input array, converted to `double`.

// The LUX function returns a `double` array with the same dimensions
// as the first argument.

// The C++ function is called schematically like this:

// \verbatim
// f(*i1, *i2, i0, &r[0], &r[1], &r[2]);
// i0 += 3;
// r += 3;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dddp3dpT3_i3aiirq_120333_f_(int32_t narg, int32_t ps[],
                                  int32_t (*f)(double, double, double [3],
                                               double *, double *, double *))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD3*;iD;iD;rD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  size_t nelem = infos[0].nelem/3;
  while (nelem--) {
    f(*ptrs[1].d, *ptrs[2].d, ptrs[0].d, &ptrs[3].d[0], &ptrs[3].d[1],
      &ptrs[3].d[2]);
    ptrs[0].d += 3;
    ptrs[3].d += 3;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX subroutine of type
// `iD*;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&`.

// The LUX subroutine arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// 2. sixteen output variables changed to be `double` scalars or
// arrays of the same dimensions as the input argument.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, 0, o1++, o2++, o3++, o4++, o5++, o6++, o7++, o8++,
//   o9++, o10++, o11++, o12++, o13++, o14++, o15++, o16++)
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dddpT16_iaoqT16_0z1T16_s_(int32_t narg, int32_t ps[],
                                void (*f)(double, double, double *, double *,
                                          double *, double *, double *,
                                          double *, double *, double *,
                                          double *, double *, double *,
                                          double *, double *, double *,
                                          double *, double *))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;"
                            "oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&",
                            &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, 0.0, ptrs[1].d++, ptrs[2].d++, ptrs[3].d++, ptrs[4].d++,
      ptrs[5].d++, ptrs[6].d++, ptrs[7].d++, ptrs[8].d++, ptrs[9].d++,
      ptrs[10].d++, ptrs[11].d++, ptrs[12].d++, ptrs[13].d++, ptrs[14].d++,
      ptrs[15].d++, ptrs[16].d++);
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX subroutine of type `iD*;oD&;oD&;oD&`.

// The LUX subroutine arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// 2. three output variables changed to be `double` scalars or arrays
// of the same dimensions as the input argument.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, 0, o1++, o2++, o3++)
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dddpT3_iaoqT3_0z1T3_s_(int32_t narg, int32_t ps[],
                             void (*f)(double, double, double *, double *,
                                       double *))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;oD&;oD&;oD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, 0.0, ptrs[1].d++, ptrs[2].d++, ptrs[3].d++);
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX subroutine of type
// `iD*;oD&;oD&;oD&;oD+3,+3&;oD[-1]&;oD[-]&;oD[-]&;oD[-]&`.

// The LUX subroutine arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// 2. three output variables changed to be `double` scalars or arrays
// of the same dimensions as the input argument.

// 3. five output variables changed to be `double` arrays of the same
// dimensions as the input argument, except that two dimensions equal
// to 3 are prefixed.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, 0, o1++, o2++, o3++, o4, o5, o6, o7, o8);
// o4 += 9;
// o5 += 9;
// o6 += 9;
// o7 += 9;
// o8 += 9;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dddpT3dp33T5_iaoqT3op3p3qocqT4_0z1T8_s_(int32_t narg, int32_t ps[],
                                              void (*f)(double, double,
                                                        double *, double *,
                                                        double *, double [3][3],
                                                        double [3][3],
                                                        double [3][3],
                                                        double [3][3],
                                                        double [3][3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;oD&;oD&;oD&;oD+3,+3&;oD[-1]&;"
                            "oD[-]&;oD[-]&;oD[-]&",
                            &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, 0.0, ptrs[1].d++, ptrs[2].d++, ptrs[3].d++,
      (double (*)[3]) ptrs[4].d, (double (*)[3]) ptrs[5].d,
      (double (*)[3]) ptrs[6].d, (double (*)[3]) ptrs[7].d,
      (double (*)[3]) ptrs[8].d);
    ptrs[4].d += 9;
    ptrs[5].d += 9;
    ptrs[6].d += 9;
    ptrs[7].d += 9;
    ptrs[8].d += 9;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX subroutine of type
// `iD*;oD&;oD&;oD&;oD&`.

// The LUX subroutine arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// 2. four output variables changed to be `double` scalars or arrays
// of the same dimensions as the input argument.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, 0, o1++, o2++, o3++, o4++);
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dddpT4_iaoqT4_0z1T4_s_(int32_t narg, int32_t ps[],
                             void (*f)(double, double, double *, double *,
                                       double *, double *))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;oD&;oD&;oD&;oD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, 0.0, ptrs[1].d++, ptrs[2].d++, ptrs[3].d++, ptrs[4].d++);
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX subroutine of type `iD*;oD&;oD&`.

// The LUX subroutine arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// 2. two output variables changed to be `double` scalars or arrays
// of the same dimensions as the input argument.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, 0, o1++, o2++);
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dddpdp_iaoqoq_0z12_s_(int32_t narg, int32_t ps[],
                            void (*f)(double, double, double *, double *))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;oD&;oD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, 0.0, ptrs[1].d++, ptrs[2].d++);
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX subroutine of type `iD*;oD3,3&`.

// The LUX subroutine arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// 2. an output variable changed to be a `double` array of the same
// dimensions as the input argument, except that two dimensions equal
// to 3 each are prefixed.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, o1);
// o1 += 9;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_ddp33_iao33q_01_s_(int32_t narg, int32_t ps[],
                         void (*f)(double, double [3][3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;oD3,3&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, (double (*)[3]) ptrs[1].d);
    ptrs[1].d += 9;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX subroutine of type `iD*;oD&;oD&`.

// The LUX subroutine arguments are:

// 1. an input scalar or array of arbitrary dimensions, converted to
// `double`.

// 2. two output variables changed to be `double` arrays of the same
// dimensions as the input argument.

// The C++ function is called schematically like this:

// \verbatim
// f(*i0++, o1++, o2++);
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_ddpdp_iaoqoq_012_s_(int32_t narg, int32_t ps[],
                          void (*f)(double, double*, double*))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;oD&;oD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, ptrs[1].d++, ptrs[2].d++);
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD2,3*;iD&;rD=,-&`.

// The LUX function arguments are:

// 1. an input array with 2 and 3 elements in its first two
// dimensions, and arbitrary following dimensions, converted to
// `double`.

// 2. an input array with the same dimensions as the previous one,
// converted to `double`.

// The LUX function returns a `double` array with the same dimensions
// as the first input argument, except that the second dimension is
// omitted.

// The C++ function is called schematically like this:

// \verbatim
// f(i0, i1, r);
// i0 += 6;
// i1 += 6;
// r += 2;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dp23T2dp2_i23aiqrkmq_0T2_f_(int32_t narg, int32_t ps[],
                                  void (*f)(double [2][3],
                                            double [2][3], double [2]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD2,3*;iD&;rD=,-&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  size_t nelem = infos[0].nelem/6;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, (double (*)[3]) ptrs[1].d,
      (double *) ptrs[2].d);
    ptrs[0].d += 6;
    ptrs[1].d += 6;
    ptrs[2].d += 2;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD2,3*;iD&;rD&`.

// The LUX function arguments are:

// 1. an input array with 2 and 3 elements in the first two
// dimensions, and arbitrary following dimensions, converted to
// `double`.

// 2. an input array of the same dimensions as the first one,
// converted to `double`.

// The LUX function returns a `double` array with the same dimensions
// as the first argument.

// The C++ function is called schematically like this:

// \verbatim
// f(i0, i1, r)
// i0 += 6;
// i1 += 6;
// r += 6;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dp23T3_i23aiqrq_0T2_f_(int32_t narg, int32_t ps[],
                             void (*f)(double [2][3], double [2][3],
                                       double [2][3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD2,3*;iD&;rD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  size_t nelem = infos[0].nelem/6;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, (double (*)[3]) ptrs[1].d,
      (double (*)[3]) ptrs[2].d);
    ptrs[0].d += 6;
    ptrs[1].d += 6;
    ptrs[2].d += 6;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX subroutine of type
// `iD2,3*;oD-,-&;oD[-]&;oD[-]&;oD[-]&;oD[-]&;oD[-]&`.

// The LUX subroutine arguments are:

// 1. an input array with 2 and 3 elements in the first two
// dimensions, and arbitrary following dimensions, converted to
// `double`.

// 2. six output variables changed to be `double` arrays of the same
// dimensions as the input argument, except that the first two
// dimensions are omitted.

// The C++ function is called schematically like this:

// \verbatim
// f(i0, o1++, o2++, o3++, o4++, o5++, o6++);
// i0 += 6;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dp23dpT6_iD23aommqocqT5_0T6_s_(int32_t narg, int32_t ps[],
                                     void (*f)(double [2][3], double *,
                                               double *, double *, double *,
                                               double *, double *))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps,
                            "iD2,3*;oD-,-&;oD[-]&;oD[-]&;oD[-]&;oD[-]&;oD[-]&",
                            &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  size_t nelem = infos[0].nelem/6;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, ptrs[1].d++, ptrs[2].d++, ptrs[3].d++,
      ptrs[4].d++, ptrs[5].d++, ptrs[6].d++);
    ptrs[0].d += 6;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX subroutine of type
// `iD2,3*;oD-,-&;oD[-]&`.

// The LUX subroutine arguments are:

// 1. an input array with 2 and 3 elements in its first two
// dimensions, followed by arbitrary dimensions, converted to
// `double`.

// 2. two output variables changed to be `double` arrays with the
// same dimensions as the input argument, except that the first two
// dimensions are omitted.

// The C++ function is called schematically like this:

// \verbatim
// f(i0, o1++, o2++);
// i0 += 6;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dp23dpdp_i23aommqocq_0T2_s_(int32_t narg, int32_t ps[],
                                  void (*f)(double [2][3], double *, double *))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD2,3*;oD-,-&;oD[-]&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  size_t nelem = infos[0].nelem/6;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, ptrs[1].d++, ptrs[2].d++);
    ptrs[0].d += 6;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD3,3*;rD&`.

// The LUX function arguments are:

// 1. an input array with 3 elements in its first two dimensions, and
// arbitrary following dimensions, converted to `double`.

// The LUX function returns a `double` array with the same dimensions
// as the last two input arguments.

// The C++ function is called schematically like this:

// \verbatim
// f(i0, r);
// i0 += 9;
// r += 9;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dp33T2_i33arq_01_f_(int32_t narg, int32_t ps[],
                          void (*f)(double [3][3], double [3][3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD3,3*;rD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;

  size_t nelem = infos[0].nelem/9;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, (double (*)[3]) ptrs[1].d);
    ptrs[0].d += 9;
    ptrs[1].d += 9;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD3,3*;iD&;rD&`.

// The LUX function arguments are:

// 1. an input array with 3 elements in its first two dimensions, and
// arbitrary following dimensions, converted to `double`.

// 2. an input array of the same dimensions as the first one,
// converted to `double`.

// The LUX function returns a `double` array with the same dimensions
// as the first argument.

// The C++ function is called schematically like this:

// \verbatim
// f(i0, i1, r)
// i0 += 9;
// i1 += 9;
// r += 9;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dp33T3_i33aiqrq_0T2_f_(int32_t narg, int32_t ps[],
                             void (*f)(double [3][3], double [3][3],
                                       double [3][3]))
{
  Pointer *ptrs;
  LoopInfo *infos;
  size_t nelem;

  StandardArguments_RAII sa(narg, ps, "iD3,3*;iD&;rD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  double (*r1)[3] = (double (*)[3]) ptrs[0].d;
  double (*r2)[3] = (double (*)[3]) ptrs[1].d;
  double (*r3)[3] = (double (*)[3]) ptrs[2].d;
  nelem = infos[0].nelem/9;
  while (nelem--) {
    f(r1, r2, r3);
    r1 += 3;
    r2 += 3;
    r3 += 3;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `rD3,3`.

// The LUX function has no arguments.

// The LUX function returns a `double` array of 3 by 3 elements.

// The C++ function is called schematically like this:

// \verbatim
// f(r)
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dp33_r33_0_f_(int32_t narg, int32_t ps[], void (*f)(double [3][3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "rD3,3", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  f((double (*)[3]) ptrs[0].d);
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD3,3*;iD-,-&;iD&;rD&`.

// The LUX function arguments are:

// 1. an input array with 3 elements in its first two dimensions, and
// arbitrary following dimensions, converted to `double`.

// 2. an input scalar or array with the same dimensions as the first
// one, except that the first two dimensions are omitted, converted
// to `double`.

// 3. an input scalar or array with the same dimensions as the
// previous one, converted to `double`.

// The LUX function returns a `double` array with the same dimensions
// as the last two input arguments.

// The C++ function is called schematically like this:

// \verbatim
// f(i0, *i1++, i2, r);
// i0 += 9;
// i2 += 9;
// r += 9;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dp33ddp33T2_i33aimmqiqrq_0T3_f_(int32_t narg, int32_t ps[],
                                      void (*f)(double [3][3], double,
                                                double [3][3],
                                                double [3][3]))
{
  Pointer *ptrs, era;
  LoopInfo *infos;
  size_t nelem;

  StandardArguments_RAII sa(narg, ps, "iD3,3*;iD-,-&;iD&;rD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  era = ptrs[1];
  double (*rc2i)[3] = (double (*)[3]) ptrs[0].d;
  double (*rpom)[3] = (double (*)[3]) ptrs[2].d;
  double (*rc2t)[3] = (double (*)[3]) ptrs[3].d;
  nelem = infos[1].nelem;
  while (nelem--) {
    f(rc2i, *era.d++, rpom, rc2t);
    rc2i += 3;
    rpom += 3;
    rc2t += 3;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD3,3*;iD-3+2&;rD&`.

// The LUX function arguments are:

// 1. an input array with 3 elements in its first two dimensions, and
// arbitrary following dimensions, converted to `double`.

// 2. an input array of the same dimensions as the first one, except
// that the first dimension is 2 instead of 3, converted to `double`.

// The LUX function returns a `double` array with the same dimensions
// as the second argument.

// The C++ function is called schematically like this:

// \verbatim
// f(i0, i1, r)
// i0 += 9;
// i1 += 6;
// r += 6;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dp33dp23T2_i33aim3p2qrcq_0T2_f_(int32_t narg, int32_t ps[],
                                      void (*f)(double [3][3], double [2][3],
                                                double [2][3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD3,3*;iD-3+2&;rD[-]&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  size_t nelem = infos[0].nelem/9;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, (double (*)[3]) ptrs[1].d,
      (double (*)[3]) ptrs[2].d);
    ptrs[0].d += 9;
    ptrs[1].d += 6;
    ptrs[2].d += 6;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD3,3*;iD-&;rD[-]&`.

// The LUX function arguments are:

// 1. an input array with 3 elements in each of its first two
// dimensions, and arbitrary following dimensions, converted to
// `double`.

// 2. an input array with the same dimensions as the first one,
// except that the first dimension is omitted, converted to `double`.

// The LUX function returns a `double` array with the same dimensions
// as the second input argument.

// The C++ function is called schematically like this:

// \verbatim
// f(i0, i1, r);
// i0 += 9;
// i1 += 3;
// r += 3;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dp33dp3T2_i33aimqrcq_0T2_f_(int32_t narg, int32_t ps[],
                                  void (*f)(double [3][3], double [3],
                                            double [3]))
{
  Pointer *ptrs;
  LoopInfo *infos;
  int32_t step;

  StandardArguments_RAII sa(narg, ps, "iD3,3*;iD-&;rD[-]&", &ptrs, &infos);
  if (sa.result() < 0)
    return sa.result();
  size_t nelem = infos[1].nelem/3;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, ptrs[1].d, ptrs[2].d);
    ptrs[0].d += 9;
    ptrs[1].d += 3;
    ptrs[2].d += 3;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD3,3*;rD-&`.

// The LUX function arguments are:

// 1. an input array with 3 elements in each of its first two
// dimensions, and arbitrary following dimensions, converted to
// `double`.

// The LUX function returns a `double` array with the same dimensions
// as the first input argument, except that the first dimension is
// omitted.

// The C++ function is called schematically like this:

// \verbatim
// f(i0, r);
// i0 += 9;
// r += 3;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dp33dp3_i33armq_01_f_(int32_t narg, int32_t ps[],
                            void (*f)(double [3][3], double [3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD3,3*;rD-&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;

  size_t nelem = infos[0].nelem/9;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, ptrs[1].d);
    ptrs[0].d += 9;
    ptrs[1].d += 3;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX subroutine of type
// `iD3,3*;oD-,-&;oD[-]&`.

// The LUX subroutine arguments are:

// 1. an input array with 3 elements in its first two dimensions,
// followed by arbitrary dimensions, converted to `double`.

// 2. two output variables changed to be `double` arrays with the
// same dimensions as the input argument, except that the first two
// dimensions are omitted.

// The C++ function is called schematically like this:

// \verbatim
// f(i0, o1++, o2++);
// i0 += 9;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dp33dpdp_i33aommqocq_0T2_s_(int32_t narg, int32_t ps[],
                                  void (*f)(double [3][3], double *, double *))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD3,3*;oD-,-&;oD[-]&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;

  size_t nelem = infos[0].nelem/9;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, ptrs[1].d++, ptrs[2].d++);
    ptrs[0].d += 9;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD3*;iD&;rD&`.

// The LUX function arguments are:

// 1. an input array with 3 elements in its first dimension, and
// arbitrary following dimensions, converted to `double`.

// 2. an input array with the same dimensions as the first one,
// converted to `double`.

// The LUX function returns a `double` array with the same dimensions
// as the first argument.

// The C++ function is called schematically like this:

// \verbatim
// f(i0, i1, r);
// i0 += 3;
// i1 += 3;
// r += 3;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dp3T3_i3aiqrq_0T2_f_(int32_t narg, int32_t ps[],
                           void (*f)(double [3], double [3],
                                     double [3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD3*;iD&;rD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  size_t nelem = infos[0].nelem/3;
  while (nelem--) {
    f(ptrs[0].d, ptrs[1].d, ptrs[2].d);
    ptrs[0].d += 3;
    ptrs[1].d += 3;
    ptrs[2].d += 3;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD3*;rD+3&`.

// The LUX function arguments are:

// 1. an input array with 3 elements in its first dimension, and
// arbitrary following dimensions, converted to `double`.

// The LUX function returns a `double` array with the same dimensions
// as the input argument, except that a dimension with 3 elements is
// prefixed.

// The C++ function is called schematically like this:

// \verbatim
// f(i0, r);
// i0 += 3;
// r += 9;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dp3dp33_i3arp3q_01_f_(int32_t narg, int32_t ps[],
                            void (*f)(double [3], double [3][3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD3*;rD+3&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    f(ptrs[0].d, (double (*)[3]) ptrs[1].d);
    ptrs[0].d += 3;
    ptrs[1].d += 9;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD3*;rD&`.

// The LUX function arguments are:

// 1. an input array with 3 elements in its first dimension, and
// arbitrary following dimensions, converted to `double`.

// The LUX function returns a `double` array with the same dimensions
// as the input argument.

// The C++ function is called schematically like this:

// \verbatim
// f(i0, &r[0], &r[1], &r[2]);
// i0 += 3;
// r += 3;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dp3dpT3_i3arq_0111_f_(int32_t narg, int32_t ps[],
                            void (*f)(double [3], double *, double *, double *))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD3*;rD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  int32_t nelem = infos[0].nelem/3;
  while (nelem--) {
    f(ptrs[0].d, &ptrs[1].d[0], &ptrs[1].d[1], &ptrs[1].d[2]);
    ptrs[0].d += 3;
    ptrs[1].d += 3;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX subroutine of type `oD3,3;oD3`.

// The LUX subroutine arguments are:

// 1. an output variable changed to be a `double` array of 3 by 3
// elements.

// 2. an output variable changed to be a `double` array of 3
// elements.

// The C++ function is called schematically like this:

// \verbatim
// f(o0, o1);
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dp3dp_o33o3_01_s_(int32_t narg, int32_t ps[],
                        void (*f)(double [3][3], double [3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "oD3,3;oD3", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  f((double (*)[3]) ptrs[0].d, (double *) ptrs[1].d);
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX subroutine of type `iD3*;oD-&;oD&`.

// The LUX subroutine arguments are:

// 1. an input array with 3 elements in its first dimension, followed
// by arbitrary dimensions, converted to `double`.

// 2. an output variable changed to be `double` arrays with the same
// dimensions as the input argument, except that the first dimension
// is omitted.

// 3. an output variable changed to be `double` arrays with the same
// dimensions as the input argument.

// The C++ function is called schematically like this:

// \verbatim
// f(i0, o1++, o2);
// i0 += 3;
// o2 += 3;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dp3dpdp3_i3aomqoq_0T2_s_(int32_t narg, int32_t ps[],
                               void (*f)(double [3], double *, double [3]))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD3*;oD-&;oD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  size_t nelem = infos[0].nelem/3;
  while (nelem--) {
    f(ptrs[0].d, ptrs[1].d++, ptrs[2].d);
    ptrs[0].d += 3;
    ptrs[2].d += 3;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD3*;rD-3+2&`.

// The LUX function arguments are:

// 1. an input array with 3 elements in its first dimension, and
// arbitrary following dimensions, converted to `double`.

// The LUX function returns a `double` array with the same dimensions
// as the input argument, except that the first dimension has 2
// instead of 3 elements.

// The C++ function is called schematically like this:

// \verbatim
// f(i0, &r[0], &r[1]);
// i0 += 3;
// r += 2;
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dp3dpdp_i3arm3p2q_011_f_(int32_t narg, int32_t ps[],
                               void (*f)(double [3], double *, double *))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD3*;rD-3+2&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;

  size_t nelem = infos[0].nelem/3;
  while (nelem--) {
    f(ptrs[0].d, &ptrs[1].d[0], &ptrs[1].d[1]);
    ptrs[0].d += 3;
    ptrs[1].d += 2;
  }
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `rD3`.

// The LUX function has no arguments.

// The LUX function returns a `double` array with 3 elements.

// The C++ function is called schematically like this:

// \verbatim
// f(&r[0], &r[1], &r[2])
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_dpT3_r3_000_f_(int32_t narg, int32_t ps[],
                     void (*f)(double *, double *, double *))
{
  Pointer *tgts;

  StandardArguments_RAII sa(narg, ps, "rD3", &tgts, NULL);
  if (sa.result() < 0)
    return LUX_ERROR;
  f(&tgts[0].d[0], &tgts[0].d[1], &tgts[0].d[2]);
  return sa.result();
}
//-----------------------------------------------------------------------
// Bind a C++ function to a LUX function of type `iD*;iD?;rD&`.

// The LUX function arguments are:

// 1. an input scalar or array with arbitrary dimensions, converted
// to `double`.

// 2. an optional input scalar or single-element array, converted to
// `double`.

// The LUX function returns a `double` array with the same dimensions
// as the input argument.

// The C++ function is called schematically like this:

// \verbatim
// f(i0, i0.dims[0], 1, i1? *i1: 3, r, r.dims[0], 1));
// \endverbatim

// @param [in] narg number of symbols in \p ps
// @param [in] ps array of argument symbols
// @param [in] f pointer to C++ function to bind
// @return the symbol containing the result of the function call
int32_t
lux_v_sddsd_iairq_012_f_(int32_t narg, int32_t ps[],
                         void (*f)(double *, size_t, size_t, double,
                                   double *, size_t, size_t))
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments_RAII sa(narg, ps, "iD*;iD?;rD&", &ptrs, &infos);
  if (sa.result() < 0)
    return LUX_ERROR;
  double width = ptrs[1].d? ptrs[1].d[0]: 3;
  f(&ptrs[0].d[0], infos[0].dims[0], 1, width, &ptrs[2].d[0],
    infos[2].dims[0], 1);
  return sa.result();
}
//-----------------------------------------------------------------------
/** Register a LUX function so it can be used in LUX programs.

 \param [in] f points to the C++ function that corresponds to the LUX
 function.

 \param [in] name points to the name of the LUX function.

 \param [in] min_arg is the minimum number of arguments (excluding
 the `mode` argument) that the LUX function requires.

 \param [in] max_arg is the maximum number of arguments (excluding
 the `mode` argument) that the LUX function requires.

 \param [in] spec points to the specification of the keywords of
 the LUX function.

 The keyword string syntax is schematically as follows:

 \verbatim
 [*][+][-][|<defaultmode>|][%<offset>%][<key1>][:<key2>]...
 \endverbatim

 All components are optional.

 `*` indicates that the function generally yields a result that has
 the same number of elements as the (first) argument and that each
 element of the result depends only on the corresponding element of
 the argument.  Inclusion of a `*` may improve memory efficiency.

 `+` indicates that the usual argument evaluation before they are
 passed to tbe selected routine is suppressed.

 `-` indicates that arguments of class `LUX_UNUSED` should be removed
 instead of being converted to class `LUX_UNDEFINED`, so that it is as
 if the user didn't specify them.

 The `<defaultmode>` is the integer default value of global C variable
 `internalMode` before the mode keywords are treated.

 The `<offset>` is the argument offset of the first non-keyword
 argument and defaults to 0.  This is convenient when a routine has a
 variable number of ordinary arguments plus a fixed number of optional
 arguments.  In this way, the optional arguments can be put in fixed
 positions before the variable number of ordinary arguments, which
 simplifies the argument treatment.

 For example, with specification

 \verbatim
 %1%special
 \endverbatim

 the ordinary arguments (not identified in the call by a keyword) are
 placed in the final arguments list starting at position 1 instead of
 the default position 0, so the only way to get a value into position
 0 is by using the `special` keyword.

 The syntax if individual keyword specifications is either of the
 following:

 \verbatim
 [<number>[$]]<name>
 [#]<name>
 \endverbatim

 The `<name>` is the name of the keyword.

 If the keyword has no number in front of it, then it allows a regular
 argument to be defined by name.  The value specified for the keyword
 in a call to the function is entered into the final arguments list at
 the position indicated by the position of the key in the defining key
 list.

 For example, if specification

 \verbatim
 first:second
 \endverbatim

 is associated with LUX function `foo`, then `foo(second=5,first=3)`
 and `foo(second=5,3)` are equivalent to `foo(3,5)`, and
 `foo(second=5)` calls the function with the second parameter set to
 `5` and the first parameter not set at all.

 Keywords with a number in front of them are mode keywords.  If such a
 keyword is specified in a call of the LUX function or subroutine,
 then the associated number is merged into global C variable
 `internalMode` using the logical "or" operator, or gets removed from
 `internalMode` if the keyword is specified preceded by `no`.

 For example, with specification

 \verbatim
 8up:16left
 \endverbatim

 for LUX function `foo`, `foo(23,/up)` sets `internalMode` to 8, and
 `foo(23,/up,/left)` sets `internalMode` to 24.  Both calls pass 23
 into the function as the first and only argument.

 With specification

 \verbatim
 |8|8up:16left
 \endverbatim

 `internalMode` is preset to 8, so `foo(23,/up)` leaves `internalMode`
 the same, and `foo(23,/noup)` sets `internalMode` to 0, and
 `foo(23,/left)` sets `internalMode` to 24.

 If the number is followed by a dollar sign (`$`), then the keyword
 value is also entered into the arguments list passed into the C
 function.

 With specification

 \verbatim
 8up:16$left
 \endverbatim

 the call `foo(23,/left)` sets `internalMode` to 16 and passes
 arguments 16 and 23 into the function.

 A hash sign `#` before the keyword name says to suppress evaluation
 of the value before it is passed into the LUX function.
*/
void register_lux_f(int32_t (*f)(int32_t, int32_t []), char const* name,
                    int32_t min_arg, int32_t max_arg, char const* spec)
{
  internalRoutine ir;

  if (!registered_functions) {
    registered_functions = malloctype(obstack);
    obstack_init(registered_functions);
  }
  ir.name = name;
  ir.minArg = min_arg;
  ir.maxArg = max_arg;
  ir.ptr = f;
  ir.keys = spec;
  obstack_grow(registered_functions, &ir, sizeof(ir));
}
//-----------------------------------------------------------------------
/** Register a LUX subroutine so it can be used in LUX programs.

 \param [in] f points to the C++ function that corresponds to the LUX
 function.

 \param [in] name points to the name of the LUX function.

 \param [in] min_arg is the minimum number of arguments (excluding
 the `mode` argument) that the LUX function requires.

 \param [in] max_arg is the maximum number of arguments (excluding
 the `mode` argument) that the LUX function requires.

 \param [in] spec points to the specification of the keywords of
 the LUX function.

 The keywords specification format is the same as for register_lux_f().
*/
void register_lux_s(int32_t (*f)(int32_t, int32_t []), char const* name,
                    int32_t min_arg, int32_t max_arg, char const* spec)
{
  internalRoutine ir;

  if (!registered_subroutines) {
    registered_subroutines = malloctype(obstack);
    obstack_init(registered_subroutines);
  }
  ir.name = name;
  ir.minArg = min_arg;
  ir.maxArg = max_arg;
  ir.ptr = f;
  ir.keys = spec;
  obstack_grow(registered_subroutines, &ir, sizeof(ir));
}
