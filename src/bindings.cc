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
  Functions and macros for binding LUX functions
    and subroutines to C (library) functions.

    This file defines many binding functions.  The name of a binding
    functions is built up as follows:

    \verbatim
    'lux_<returntype>_<paramtypes>_<stdargspec>_<ptrspec>_<sf>_'
    \endverbatim

    A descriptive suffix may be appended to disambiguate names for
    binding functions that would otherwise be equal.

    \par returntype

    \c <returntype> is an encoding of the return type of the bound C
    function.  \c void is encoded as `v`, \c double as `d`, \c int32_t
    as `i`, <tt>double*</tt> as `dp`, <tt>double (*)[3][4]</tt> as
    `dp34`.

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
    - removing `>`, `;`, and all but the first `i`, all but the first
      `o`, and all but the first `r` (but not what's after those later
      `i` or `o` or `r`).
    - changing `&` to `q`, `+` to `p`, `-` to `m`, `*` to `a`.
    - changing `[...]` to `c`, `{...}` to `x`.
    - changing `#` to `o`.

    A repeat of a particular unit (from a variable type specification
    like `D` up to but not including the next one; for a repeat count
    of at least 3, or if the abbreviated version is shorter than the
    full version) is indicated by appending `T` and the repeat count.
    For example, `DqDqDqDq` gets abbreviated to `DqT4`.

    Multiple standard_args() format specifications can lead to the
    same \c <stdargspec>.

    So,

    \verbatim
    lux_d_dT3i_iD3rDq_X_X_
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
    lux_d_dT3i_iD3rDq_0001_2_X_
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
    lux_d_dT3i_iD3rDq_0001_2_f_
    \endverbatim

    binds the C function to a LUX function.

*/
#include "luxdefs.hh"
#include "error.hh"
#include <math.h>
#include <obstack.h>
#include "bindings.hh"
#define obstack_chunk_alloc malloc
#define obstack_chunk_free free

extern int32_t internalMode;

int32_t standard_args(int32_t, int32_t [], char const *, Pointer **,
                      loopInfo **);
int32_t setAxes(loopInfo *, int32_t, int32_t *, int32_t);
int32_t advanceLoop(loopInfo *, Pointer *);

struct obstack *registered_functions = NULL, *registered_subroutines = NULL;

/** Register a LUX function so it can be used in LUX programs.

 \param [in] f points to the C function that corresponds to the LUX
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
/*-----------------------------------------------------------------------*/
/** Register a LUX subroutine so it can be used in LUX programs.

 \param [in] f points to the C function that corresponds to the LUX
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
/*-----------------------------------------------------------------------*/
int32_t lux_ivarl_copy_eachaxis_(int32_t narg, int32_t ps[], int32_t (*f)(double *, size_t count, size_t stride), int32_t isfunction)
/* copy input to output, apply function to output, go through each axis separately */
{
  Pointer *ptrs, ptrs0, ptrsr;
  loopInfo *infos;
  int32_t iq, iret;
  int32_t *axes, naxes, oneaxis[1] = { 0 }, allaxes;

  switch (narg) {
  case 1:                       /* source */
    if ((iq = standard_args(narg, ps, isfunction? "i>D*;rD&": "i>D*", &ptrs, &infos)) < 0)
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
  case 2:                       /* source, axes */
    if ((iq = standard_args(narg, ps, isfunction? "i>D*;iL*;rD&": "i>D*;iL*", &ptrs, &infos)) < 0)
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

  if (internalMode & 1) {	/* /ALLAXES */
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
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_d_dT4_iDaDarDq_0011_2_f_(int32_t narg, int32_t ps[], double (*f)(double, double, double, double))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;rD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    double uta, utb, tta, ttb;
    uta = floor(*ptrs[0].d);
    utb = *ptrs[0].d++ - uta;
    tta = floor(*ptrs[1].d);
    ttb = *ptrs[1].d++ - tta;
    *ptrs[2].d++ = f(uta, utb, tta, ttb);
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_d_dT4_iDaT4rDq_0T3_4_f_(int32_t narg, int32_t ps[], double (*f)(double, double, double, double))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;rD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  while (infos[0].nelem--)
    *ptrs[4].d++ = f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++);
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_d_dT4_iLaDaDarDq_0z12_f_(int32_t narg, int32_t ps[], double (*f)(double, double, double, double))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>L*;i>D*;i>D*;r>D&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  switch (infos[0].type) {
  case LUX_INT32:
    while (infos[1].nelem--)
      *ptrs[3].d++ = f((double) *ptrs[0].l++, 0.0, *ptrs[1].d++, *ptrs[2].d++);
    break;
  case LUX_INT64:
    while (infos[1].nelem--)
      *ptrs[3].d++ = f((double) *ptrs[0].q++, 0.0, *ptrs[1].d++, *ptrs[2].d++);
    break;
  case LUX_FLOAT:
    while (infos[1].nelem--)
      *ptrs[3].d++ = f((double) *ptrs[0].f++, 0.0, *ptrs[1].d++, *ptrs[2].d++);
    break;
  case LUX_DOUBLE:
    while (infos[1].nelem--)
      *ptrs[3].d++ = f(*ptrs[0].d++, 0.0, *ptrs[1].d++, *ptrs[2].d++);
    break;
  default:
    break;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_d_dT4dp3_iDaDaDp3p3arDq_00112_3_f_(int32_t narg, int32_t ps[], double (*f)(double, double, double, double, double (*)[3]))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D+3,+3*;rD&", &ptrs, &infos)) < 0)
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
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_d_dT6_iLaDaD1T3rDq_0z1T4_5_f_(int32_t narg, int32_t ps[], double (*f)(double, double, double, double, double, double))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>L*;i>D*;i>D1;i>D1;i>D1;r>D&", &ptrs, &infos)) < 0)
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
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_d_d_iDarDq_0_1_f_(int32_t narg, int32_t ps[], double (*f)(double))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;rD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;

  while (infos[0].nelem--)
    *ptrs[1].d++ = f(*ptrs[0].d++);
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
/** Bind a C function \p f to a LUX function with one at-least-double
    array input parameter, one scalar at-least-double scalar input
    parameter, and a double array return parameter with the same
    dimensions as the first parameter.  Function \p f is called for
    each element of the first parameter.

    Standard arguments <tt>"i>D*;i>D1;rD&"</tt>.

    @param [in] narg number of symbols in \p ps
    @param [in] ps array of argument symbols
    @param [in] f pointer to C function to bind
    @return the symbol containing the result of the function call
 */
int32_t lux_d_dd_iDaD1rDq_01_2_f_(int32_t narg, int32_t ps[], double (*f)(double, double))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D1;rD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  while (infos[0].nelem--)
    *ptrs[2].d++ = f(*ptrs[0].d++, *ptrs[1].d);
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
/** Bind a C function to a LUX function of type `iD*;iD#;rD`.

    The LUX function arguments are:
    1. an input array, converted to `double`
    2. a scalar or an array with the same number of elements as the
       first one

    The LUX function returns a `double` array with the same dimensions
    as the first argument.

    The C function is called for each element of the first array
    combined with the scalar or corresponding element of the second
    array.

    @param [in] narg number of symbols in \p ps
    @param [in] ps array of argument symbols
    @param [in] f pointer to C function to bind
    @return the symbol containing the result of the function call
 */
int32_t lux_d_dd_iDaDbrDq_01_2_f_(int32_t narg, int32_t ps[], double (*f)(double, double))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D#;rD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  if (infos[1].nelem > 1) {
    /* same number of elements in parameters 0 and 1 */
    while (infos[0].nelem--)
      *ptrs[2].d++ = f(*ptrs[0].d++, *ptrs[1].d++);
  } else {
    /* parameter 1 is a scalar */
    while (infos[0].nelem--)
      *ptrs[2].d++ = f(*ptrs[0].d++, *ptrs[1].d);
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_d_dd_iDarDq_0z_1_f_(int32_t narg, int32_t ps[], double (*f)(double, double))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;rD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  while (infos[0].nelem--)
    *ptrs[1].d++ = f(*ptrs[0].d++, 0.0);
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_d_dd_iLarDq_0z_1_f_(int32_t narg, int32_t ps[], double (*f)(double, double))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>L*;rD&", &ptrs, &infos)) < 0)
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
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_d_dp3d_iD33aDmmarDmmq_01_2_f_(int32_t narg, int32_t ps[], double (*f)(double (*)[3], double))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D3,3*;i>D-,-*;rD-,-&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  double (*rnpb)[3] = (double (*)[3]) ptrs[0].d;
  while (infos[1].nelem--) {
    *ptrs[2].d++ = f(rnpb, *ptrs[1].d++);
    rnpb += 3;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_d_dp_iD3arDm3q_0_1_f_(int32_t narg, int32_t ps[], double (*f)(double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D3*;rD-3&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  int32_t nelem = infos[0].nelem/3;
  while (nelem--) {
    *ptrs[1].d++ = f(ptrs[0].d);
    ptrs[0].d += 3;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_d_dpdp_iD3aD3qrDm3q_01_2_f_(int32_t narg, int32_t ps[], double (*f)(double *, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D3*;i>D3&;rD-3&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  while (infos[2].nelem--) {
    *ptrs[2].d++ = f(ptrs[0].d, ptrs[1].d);
    ptrs[0].d += 3;
    ptrs[1].d += 3;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_d_sd_iDaLarDxq_000_2_f_(int32_t narg, int32_t ps[], double (*f)(double *, size_t count, size_t stride))
{
  Pointer *ptrs, ptrs0, ptrsr;
  loopInfo *infos;
  int32_t iq, iret;
  int32_t *axes, naxes, oneaxis[1] = { 0 }, allaxes;

  switch (narg) {
  case 1:                       /* source */
    if ((iq = standard_args(narg, ps, "i>D*;rD-&", &ptrs, &infos)) < 0)
      return LUX_ERROR;
    axes = oneaxis;
    naxes = 1;
    setAxes(&infos[0], 0, NULL, SL_EACHROW);
    iret = 1;
    break;
  case 2:                       /* source, axes */
    if ((iq = standard_args(narg, ps, "i>D*;iL*;rD{-}&", &ptrs, &infos)) < 0)
      return LUX_ERROR;
    axes = ptrs[1].l;
    naxes = infos[1].nelem;
    if (setAxes(&infos[0], infos[1].nelem, ptrs[1].l,
		SL_EACHROW | SL_UNIQUEAXES) < 0)
      return LUX_ERROR;
    iret = 2;
    break;
  }

  if (internalMode & 1) {	/* /ALLAXES */
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
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_i_dT10dpT6_iDaT8oDqT6_0T6z7z8T13_s_(int32_t narg, int32_t ps[], int32_t (*f)(double, double, double, double, double, double, double, double, double, double, double *, double *, double *, double *, double *, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;i>D*;i>D*;i>D*;i>D*;oD&;oD&;oD&;oD&;oD&;oD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++, *ptrs[4].d++,
      *ptrs[5].d++, *ptrs[6].d++, 0.0, *ptrs[7].d++, 0.0, ptrs[8].d++,
      ptrs[9].d++, ptrs[10].d++, ptrs[11].d++, ptrs[12].d++, ptrs[13].d++);
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_i_dT3dpdp_iDaDarDq_0z122_f_(int32_t narg, int32_t ps[], int32_t (*f)(double, double, double, double *, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;rD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    double ut1, ut2;
    f(*ptrs[0].d++, 0.0, *ptrs[1].d++, &ut1, &ut2);
    *ptrs[2].d++ = ut1 + ut2;
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_i_dT5dp_iD3aD1D1rD3q_120003_f_(int32_t narg, int32_t ps[], int32_t (*f)(double, double, double, double, double, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D3*;iD1;iD1;rD3&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  size_t nelem = infos[0].nelem/3;
  while (nelem--) {
    f(*ptrs[1].d, *ptrs[2].d, ptrs[0].d[0], ptrs[0].d[1], ptrs[0].d[2], ptrs[3].d);
    ptrs[0].d += 3;
    ptrs[3].d += 3;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_i_dT6dp3_iDaT6oDp2p3q_0T6_f_(int32_t narg, int32_t ps[], int32_t (*f)(double, double, double, double, double, double, double [2][3]))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;i>D*;i>D*;oD+2,+3&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++, *ptrs[4].d++,
      *ptrs[5].d++, (double (*)[3]) ptrs[6].d);
    ptrs[6].d += 6;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_i_dddp3dp3_iLaoDp2p3qT2_0z12_s_(int32_t narg, int32_t ps[], int32_t (*f)(double, double, double (*)[3], double (*)[3]))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>L*;oD+2,+3&;oD+2,+3&", &ptrs, &infos)) < 0)
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
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_i_dddpdp_iDarDq_0z11_f_(int32_t narg, int32_t ps[], int32_t (*f)(double, double, double *, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;rD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    double tt1, tt2;
    f(*ptrs[0].d++, 0.0, &tt1, &tt2);
    *ptrs[1].d++ = tt1 + tt2;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_i_ddidp3_iDaL1rDp3p2q_0z12_f_(int32_t narg, int32_t ps[], int32_t (*f)(double, double, int32_t, double (*)[3]))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;iL1;rD+3,+2&", &ptrs, &infos)) < 0)
    return LUX_ERROR;

  while (infos[0].nelem--) {
    f(*ptrs[0].d++, 0.0, *ptrs[1].l, (double (*)[3]) ptrs[2].d);
    ptrs[2].d += 6;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_i_dp3dpT6_iD23aoDm2m3aDcT5_0T6_s_(int32_t narg, int32_t ps[], int32_t (*f)(double [2][3], double *, double *, double *, double *, double *, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D2,3*;oD-2-3*;oD[-];oD[-];oD[-];oD[-];oD[-]", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  size_t nelem = infos[0].nelem/6;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, ptrs[1].d++, ptrs[2].d++, ptrs[3].d++,
      ptrs[4].d++, ptrs[5].d++, ptrs[6].d++);
    ptrs[0].d += 6;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_i_dpiT3dp_iDaiLiLrDq_00T3_f_(int32_t narg, int32_t ps[], int32_t (*f)(double *, int32_t, int32_t, int32_t, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;iL;iL;rD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  size_t nrepeat = infos[0].nelem/infos[0].rdims[0];
  while (nrepeat--) {
    f(ptrs[0].d, infos[0].rdims[0], *ptrs[1].l, *ptrs[2].l, ptrs[3].d);
    ptrs[0].d += infos[0].rdims[0];
    ptrs[3].d += infos[0].rdims[0];
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_i_dpiidp_iDaLrDq_00T2_f_(int32_t narg, int32_t ps[], int32_t (*f)(double *, int32_t, int32_t, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;iL;rD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  size_t nrepeat = infos[0].nelem/infos[0].rdims[0];
  while (nrepeat--) {
    f(ptrs[0].d, infos[0].rdims[0], *ptrs[1].l, ptrs[2].d);
    ptrs[0].d += infos[0].rdims[0];
    ptrs[2].d += infos[0].rdims[0];
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_i_idT3dp_iD3aL1rDq_10002_f_(int32_t narg, int32_t ps[], int32_t (*f)(int32_t, double, double, double, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D3*;iL1;rD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  size_t nelem = infos[0].nelem/3;
  while (nelem--) {
    f(*ptrs[1].l, ptrs[0].d[0], ptrs[0].d[1], ptrs[0].d[2], ptrs[2].d);
    ptrs[0].d += 3;
    ptrs[2].d += 3;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
/** Bind a C function to a LUX function.

    The C function accepts an `int32_t` and four pointers to `double`
    and returns an `int32_t`.

    The LUX function requires a single-element input value (converted
    to `int32`), and an array (converted to `double`) with dimensions
    3 and any, and returns a `double` array with the same dimensions
    as the second argument.

    Standard arguments format <tt>"iL;iD3*;rD[-]"</tt>

    Function \p f is called once per 3 elements of both the 2nd input
    parameter and the return symbol.

    @param [in] narg number of symbols in \p ps
    @param [in] ps array of argument symbols
    @param [in] f pointer to C function to bind
    @return the symbol containing the result of the function call
 */
int32_t lux_i_idpT4_iL1D3arDcq_0T222_f_(int32_t narg, int32_t ps[], int32_t (*f)(int32_t, double *, double *, double *, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "iL;iD3*;rD[-]", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  size_t nelem = infos[1].nelem/3;
  while (nelem--) {
    f(*ptrs[0].l, ptrs[1].d, &ptrs[2].d[0], &ptrs[2].d[1], &ptrs[2].d[2]);
    ptrs[1].d += 3;
    ptrs[2].d += 3;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_i_idpdp_iLarDp2q_011_f_(int32_t narg, int32_t ps[], int32_t (*f)(int32_t, double *, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "iL*;rD+2&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    f(*ptrs[0].l++, &ptrs[1].d[0], &ptrs[1].d[1]);
    ptrs[1].d += 2;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_i_sd_iDaLa_000_s_(int32_t narg, int32_t ps[], int32_t (*f)(double *, size_t count, size_t stride))
{
  return lux_ivarl_copy_eachaxis_(narg, ps, f, 0);
}
/*-----------------------------------------------------------------------*/
int32_t lux_i_sd_iDaLarDq_000_f_(int32_t narg, int32_t ps[], int32_t (*f)(double *, size_t count, size_t stride))
{
  return lux_ivarl_copy_eachaxis_(narg, ps, f, 1);
}
/*-----------------------------------------------------------------------*/
int32_t lux_i_sdddsd_iDaLDDrDq_000T333_f_(int32_t narg, int32_t ps[], int32_t (*f)
		    (double *, size_t srcount, size_t srcstride,
		     double par1, double par2,
		     double *, size_t tgtcount, size_t tgtstride))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq, ipar1, ipar2, iret;

  switch (narg) {
  case 2:                       /* source, param1; (param2 = 0) */
    if ((iq = standard_args(narg, ps, "i>D*;i>D;rD&", &ptrs, &infos)) < 0)
      return LUX_ERROR;
    ipar1 = 1;
    ipar2 = -1;
    iret = 2;
    setAxes(&infos[0], 0, NULL, SL_EACHROW);
    setAxes(&infos[iret], 0, NULL, SL_EACHROW);
    break;
  case 3:                       /* source, param1, param2 */
    if ((iq = standard_args(narg, ps, "i>D*;i>D;i>D;rD&", &ptrs, &infos)) < 0)
      return LUX_ERROR;
    ipar1 = 1;
    ipar2 = 2;
    iret = 3;
    setAxes(&infos[0], 0, NULL, SL_EACHROW);
    setAxes(&infos[iret], 0, NULL, SL_EACHROW);
    break;
  case 4:                       /* source, axis, param1, param2 */
    if ((iq = standard_args(narg, ps, "i>D*;iL;i>D;i>D;rD&", &ptrs, &infos)) < 0)
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
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_iddipT3dp_iLarDp3q_0z1111_f_(int32_t narg, int32_t ps[], int32_t (*f)(double, double, int32_t *, int32_t *, int32_t *, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq, y, m, d;
  double fd;

  if ((iq = standard_args(narg, ps, "i>L*;r>D+3&", &ptrs, &infos)) < 0)
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
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dT3d3_iD3DcqrDcq_0T2_f_(int32_t narg, int32_t ps[], void (*f)(double *, double *, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D3*;i>D[-]&;rD[-]&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  size_t nelem = infos[0].nelem/3;
  while (nelem--) {
    f(ptrs[0].d, ptrs[1].d, ptrs[2].d);
    ptrs[0].d += 3;
    ptrs[1].d += 3;
    ptrs[2].d += 3;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dT3d3_iD3aD1D1rD3q_120333_f_(int32_t narg, int32_t ps[], int32_t (*f)(double, double, double *, double *, double *, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D3*;i>D1;i>D1;rD3&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  size_t nelem = infos[0].nelem/3;
  while (nelem--) {
    f(*ptrs[1].d, *ptrs[2].d, ptrs[0].d, &ptrs[3].d[0], &ptrs[3].d[1],
      &ptrs[3].d[2]);
    ptrs[0].d += 3;
    ptrs[3].d += 3;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
/** Bind a C function to a LUX function.

    The C function accepts three `double` values and a pointer to an
    array of 3 `double` values and returns `void`.

    The LUX function requires three numerical scalar or array
    arguments (converted to `double`) with the same dimensions and
    returns a `double` array with the same dimensions as the input
    arguments but with dimensions 3 and 3 prefixed.

    Standard arguments format <tt>"iD*;iD&;iD&;rD+3,+3"</tt>.

    @param [in] narg number of symbols in \p ps
    @param [in] ps array of argument symbols
    @param [in] f pointer to C function to bind
    @return the symbol containing the result of the function call

    \todo Check if advancing through the return value works OK if the
    first argument has more than one element.
 */
int32_t lux_v_dT3dp3_iDaDqDqrDp3p3_0T3_f_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double (*)[3]))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "iD*;iD&;iD&;rD+3,+3",
                          &ptrs, &infos)) < 0)
    return LUX_ERROR;
  do {
    f(*ptrs[0].d, *ptrs[1].d, *ptrs[2].d, (double (*)[3]) ptrs[3].d);
  } while (advanceLoop(&infos[0], &ptrs[3]),
           advanceLoop(&infos[1], &ptrs[2]),
           advanceLoop(&infos[2], &ptrs[1]),
           advanceLoop(&infos[3], &ptrs[0]) < infos[0].rndim);
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dT3dp3_iDaT3rDp3p3q_0T3_f_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double (*)[3]))
{
  int32_t iq;
  double (*r)[3];
  Pointer *ptrs;
  loopInfo *infos;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;rD+3,+3&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  r = (double (*)[3]) ptrs[3].d;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, r);
    r += 3;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dT3dp_iDaT3rDp3q_0T3_f_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double [3]))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;rD+3&", &ptrs, &infos)) < 0)
    return LUX_ERROR;

  while (infos[0].nelem--) {
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, ptrs[3].d);
    ptrs[3].d += 3;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dT4dp3_iDaT3rDp3p3q_0z1T3_f_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double, double (*)[3]))
{
  int32_t iq;
  double (*tgt)[3];
  Pointer *ptrs;
  loopInfo *infos;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;rD+3,+3&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  tgt = (double (*)[3]) ptrs[3].d;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, 0.0, *ptrs[1].d++, *ptrs[2].d++, tgt);
    tgt += 3;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dT4dp3_iDaT4rDp3p3q_0T4_f_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double, double (*)[3]))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;rD+3,+3&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  double (*r)[3] = (double (*)[3]) ptrs[4].d;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++, r);
    r += 3;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dT4dpT3_iDaDaoDqT3_0z1z2T4_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double, double *, double *, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;oD&;oD&;oD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, 0.0, *ptrs[1].d++, 0.0, ptrs[2].d++, ptrs[3].d++, ptrs[4].d++);
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dT4dpT4_iDaDaDoDqT4_0T2z3T6_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double, double *, double *, double *, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D;oD&;oD&;oD&;oD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, 0.0, ptrs[3].d++,
      ptrs[4].d++, ptrs[5].d++, ptrs[6].d++);
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dT4dpdp3T5_iDaT3oDqDp3p3qT5_0z1T8_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double, double *, double (*)[3], double (*)[3], double (*)[3], double (*)[3], double (*)[3]))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;oD&;oD+3,+3&;oD+3,+3&;oD+3,+3&;oD+3,+3&;oD+3,+3&", &ptrs, &infos)) < 0)
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
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dT4dpdp_iDaT3oDqDq_0T2z34_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double, double *, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;oD&;oD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, 0.0, ptrs[3].d++, ptrs[4].d++);
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dT4dpdp_iDaT4oDqDq_0T5_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double, double *, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;oD&;oD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++,
      ptrs[4].d++, ptrs[5].d++);
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dT6dp3_iDaT4rDp3p3q_0z11T4_f_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double, double, double, double (*)[3]))
{
  int32_t iq;
  Pointer *ptrs;
  loopInfo *infos;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;rD+3,+3&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    double d, t;
    d = floor(*ptrs[1].d - 0.5) + 0.5;
    t = *ptrs[1].d - d;
    f(*ptrs[0].d++, 0.0, d, t, *ptrs[2].d++, *ptrs[3].d++, (double (*)[3]) ptrs[4].d);
    ptrs[4].d += 9;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dT6dp3_iDaT6oDp2p3q_0T6_f_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double, double, double, double [2][3]))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;i>D*;i>D*;oD+2,+3&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++, *ptrs[4].d++,
      *ptrs[5].d++, (double (*)[3]) ptrs[6].d);
    ptrs[6].d += 6;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dT6dpT6_iDaT6oDqT6_0T11_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double, double, double, double *, double *, double *, double *, double *, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;i>D*;i>D*;oD&;oD&;oD&;oD&;oD&;oD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, *ptrs[1].d++, *ptrs[2].d++, *ptrs[3].d++,
      *ptrs[4].d++, *ptrs[5].d++, ptrs[6].d++, ptrs[7].d++,
      ptrs[8].d++, ptrs[9].d++, ptrs[10].d++, ptrs[11].d++);
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dT8dp3_iDaT5rDp3p3q_0zzz1T5_f_(int32_t narg, int32_t ps[], void (*f)(double, double, double, double, double, double, double, double, double (*)[3]))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;i>D*;i>D*;i>D*;rD+3,+3&",
                          &ptrs, &infos)) < 0)
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
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dddp3T3_iLaoD33oDp3p3qDcq_0z1T3_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double (*)[3], double (*)[3], double (*)[3]))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>L*;oD3,3;oD+3,+3&;oD[-]&", &ptrs, &infos)) < 0)
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
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dddp3_iDarDp3p3q_0z1_f_(int32_t narg, int32_t ps[], void (*f)(double, double, double (*)[3]))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;rD+3,+3&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  double (*r)[3] = (double (*)[3]) ptrs[1].d;

  while (infos[0].nelem--) {
    f(*ptrs[0].d++, 0.0, r);
    r += 3;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dddp3dp3_iDaiDp3p3arDcq_0z12_f_(int32_t narg, int32_t ps[], void (*f)(double, double, double (*)[3], double (*)[3]))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D+3,+3*;rD[-]&", &ptrs, &infos)) < 0)
    return LUX_ERROR;

  double (*r1)[3] = (double (*)[3]) ptrs[1].d;
  double (*r2)[3] = (double (*)[3]) ptrs[2].d;

  while (infos[0].nelem--) {
    f(*ptrs[0].d++, 0.0, r1, r2);
    r1 += 3;
    r2 += 3;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dddpT16_iDaoDqT16_0z1T16_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&;oD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, 0.0, ptrs[1].d++, ptrs[2].d++, ptrs[3].d++, ptrs[4].d++,
      ptrs[5].d++, ptrs[6].d++, ptrs[7].d++, ptrs[8].d++, ptrs[9].d++,
      ptrs[10].d++, ptrs[11].d++, ptrs[12].d++, ptrs[13].d++, ptrs[14].d++,
      ptrs[15].d++, ptrs[16].d++);
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dddpT3_iDaoDqT3_0z1T3_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double *, double *, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;oD&;oD&;oD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, 0.0, ptrs[1].d++, ptrs[2].d++, ptrs[3].d++);
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dddpT3dp3T5_iDaoDqT3Dp3p3qT5_0z1T8_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double *, double *, double *, double (*)[3], double (*)[3], double (*)[3], double (*)[3], double (*)[3]))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;oD&;oD&;oD&;oD+3,+3&;oD+3,+3&;oD+3,+3&;oD+3,+3&;oD+3,+3&", &ptrs, &infos)) < 0)
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
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dddpT4_iDaoDqT4_0z1T4_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double *, double *, double *, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;oD&;oD&;oD&;oD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, 0.0, ptrs[1].d++, ptrs[2].d++, ptrs[3].d++, ptrs[4].d++);
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dddp_iDaDarDp3q_0T2_f_(int32_t narg, int32_t ps[], void (*f)(double, double, double [3]))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;i>D*;rD+3&", &ptrs, &infos)) < 0)
    return LUX_ERROR;

  while (infos[0].nelem--) {
    f(*ptrs[0].d++, *ptrs[1].d++, ptrs[2].d);
    ptrs[2].d += 3;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dddpdp_iDaoDqDq_0z12_s_(int32_t narg, int32_t ps[], void (*f)(double, double, double *, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;oD&;oD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  while (infos[0].nelem--)
    f(*ptrs[0].d++, 0.0, ptrs[1].d++, ptrs[2].d++);
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_ddp3_iD1D33_01_s_(int32_t narg, int32_t ps[], void (*f)(double, double [3][3]))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D1;i>D3,3", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, (double (*)[3]) ptrs[1].d);
    ptrs[1].d += 9;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
/** Bind a C function \p f (with one double argument and two double
    Pointer arguments returning void) to a LUX function with one
    scalar at-least-double input parameter and two double output
    parameters that get the same dimensions as the input parameter.
    The function is called once for each element of the input
    parameter.

    Standard arguments <tt>"i>D*;oD&;oD&"</tt>.

    @param [in] narg number of symbols in \p ps
    @param [in] ps array of argument symbols
    @param [in] f Pointer to C function to bind
    @return the symbol containing the result of the function call
 */
int32_t lux_v_ddpdp_iDaoDqDq_012_s_(int32_t narg, int32_t ps[], void (*f)(double, double*, double*))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;oD&;oD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    f(*ptrs[0].d++, ptrs[1].d++, ptrs[2].d++);
  }
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dp3T3_iD23aD23qoDcq_0T2_f_(int32_t narg, int32_t ps[], void (*f)(double [2][3], double [2][3], double [2][3]))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D2,3*;i>D2,3&;oD[-]&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  size_t nelem = infos[0].nelem/6;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, (double (*)[3]) ptrs[1].d,
      (double (*)[3]) ptrs[2].d);
    ptrs[0].d += 6;
    ptrs[1].d += 6;
    ptrs[2].d += 6;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dp3T3_iD33aDaoDc_0T2_f_(int32_t narg, int32_t ps[], void (*f)(double [3][3], double [2][3], double [2][3]))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D3,3*;iD*;oD[-]", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  size_t nelem = infos[0].nelem/9;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, (double (*)[3]) ptrs[1].d,
      (double (*)[3]) ptrs[2].d);
    ptrs[0].d += 9;
    ptrs[1].d += 6;
    ptrs[2].d += 6;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dp3T3_iD33aDarDq_0T2_f_(int32_t narg, int32_t ps[], void (*f)(double (*)[3], double (*)[3], double (*)[3]))
{
  int32_t iq;
  Pointer *ptrs;
  loopInfo *infos;
  size_t nelem;

  if ((iq = standard_args(narg, ps, "i>D3,3*;i>D*;rD&", &ptrs, &infos)) < 0)
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
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dp3_rD33_0_f_(int32_t narg, int32_t ps[], void (*f)(double (*)[3]))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "rD3,3", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  f((double (*)[3]) ptrs[0].d);
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dp3ddp3dp3_iD33aDmmaDarDq_0T3_f_(int32_t narg, int32_t ps[], void (*f)(double (*)[3], double, double (*)[3], double (*)[3]))
{
  int32_t iq;
  Pointer *ptrs, era;
  loopInfo *infos;
  size_t nelem;

  if ((iq = standard_args(narg, ps, "i>D3,3*;i>D-,-*;i>D*;rD&", &ptrs, &infos)) < 0)
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
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dp3dp3_iD33arDq_01_f_(int32_t narg, int32_t ps[], void (*f)(double [3][3], double [3][3]))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D3,3*;rD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;

  size_t nelem = infos[0].nelem/9;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, (double (*)[3]) ptrs[1].d);
    ptrs[0].d += 9;
    ptrs[1].d += 9;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dp3dp3dp_iD23aD23qoDcm3q_0T2_f_(int32_t narg, int32_t ps[], void (*f)(double [2][3], double [2][3], double [2]))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D2,3*;i>D2,3&;oD[-],-3&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  size_t nelem = infos[0].nelem/6;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, (double (*)[3]) ptrs[1].d,
      (double *) ptrs[2].d);
    ptrs[0].d += 6;
    ptrs[1].d += 6;
    ptrs[2].d += 2;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dp3dpT6_iD23aoDm2m3aDcT5_0T6_s_(int32_t narg, int32_t ps[], void (*f)(double [2][3], double *, double *, double *, double *, double *, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D2,3*;oD-2-3*;oD[-];oD[-];oD[-];oD[-];oD[-]", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  size_t nelem = infos[0].nelem/6;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, ptrs[1].d++, ptrs[2].d++, ptrs[3].d++,
      ptrs[4].d++, ptrs[5].d++, ptrs[6].d++);
    ptrs[0].d += 6;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dp3dp_iD33arDm3q_01_f_(int32_t narg, int32_t ps[], void (*f)(double [3][3], double [3]))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D3,3*;rD-3&", &ptrs, &infos)) < 0)
    return LUX_ERROR;

  size_t nelem = infos[0].nelem/9;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, ptrs[1].d);
    ptrs[0].d += 9;
    ptrs[1].d += 3;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dp3dp_oD33D3_01_s_(int32_t narg, int32_t ps[], void (*f)(double (*)[3], double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "oD3,3;oD3", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  f((double (*)[3]) ptrs[0].d, (double *) ptrs[1].d);
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dp3dpdp_iD23aoDm2m3qDcq_0T2_s_(int32_t narg, int32_t ps[], void (*f)(double [2][3], double *, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D2,3*;oD-2-3&;oD[-]&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  size_t nelem = infos[0].nelem/6;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, ptrs[1].d++, ptrs[2].d++);
    ptrs[0].d += 6;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dp3dpdp_iD33aDm3arDcq_0T2_f_(int32_t narg, int32_t ps[], void (*f)(double [3][3], double [3], double [3]))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq, step;

  iq = LUX_ERROR;
  if ((iq = standard_args(narg, ps, "i>D3,3*;i>D-3*;rD[-]&", &ptrs, &infos))
      > 0)
    step = 9;
  else if ((iq = standard_args(narg, ps, "i>D3,3;i>D-3*;rD[-]&", &ptrs, &infos))
           > 0)
    step = 0;
  if (iq > 0) {
    size_t nelem = infos[1].nelem/3;
    while (nelem--) {
      f((double (*)[3]) ptrs[0].d, ptrs[1].d, ptrs[2].d);
      ptrs[0].d += step;
      ptrs[1].d += 3;
      ptrs[2].d += 3;
    }
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dp3dpdp_iD33aoDm3m3qT2_0T2_s_(int32_t narg, int32_t ps[], void (*f)(double (*)[3], double *, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D3,3*;oD-3,-3&;oD-3,-3&", &ptrs, &infos)) < 0)
    return LUX_ERROR;

  size_t nelem = infos[0].nelem/9;
  while (nelem--) {
    f((double (*)[3]) ptrs[0].d, ptrs[1].d++, ptrs[2].d++);
    ptrs[0].d += 9;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dpT3_iD3aoDm3qoDq_0T2_s_(int32_t narg, int32_t ps[], void (*f)(double *, double *, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D3*;oD-3&;oD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  size_t nelem = infos[0].nelem/3;
  while (nelem--) {
    f(ptrs[0].d, ptrs[1].d++, ptrs[2].d);
    ptrs[0].d += 3;
    ptrs[2].d += 3;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dpT3_iD3arDm3p2q_011_f_(int32_t narg, int32_t ps[], void (*f)(double *, double *, double *))
{
  int32_t iq;
  Pointer *ptrs;
  loopInfo *infos;

  if ((iq = standard_args(narg, ps, "i>D3*;r>D-3+2&", &ptrs, &infos)) < 0)
    return LUX_ERROR;

  size_t nelem = infos[0].nelem/3;
  while (nelem--) {
    f(ptrs[0].d, &ptrs[1].d[0], &ptrs[1].d[1]);
    ptrs[0].d += 3;
    ptrs[1].d += 2;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dpT3_rD3_000_f_(int32_t narg, int32_t ps[], void (*f)(double *, double *, double *))
{
  Pointer *tgts;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "rD3", &tgts, NULL)) < 0)
    return LUX_ERROR;
  f(&tgts[0].d[0], &tgts[0].d[1], &tgts[0].d[2]);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dpT4_iD3arD3q_0111_f_(int32_t narg, int32_t ps[], void (*f)(double *, double *, double *, double *))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D3*;rD3&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  int32_t nelem = infos[0].nelem/3;
  while (nelem--) {
    f(ptrs[0].d, &ptrs[1].d[0], &ptrs[1].d[1], &ptrs[1].d[2]);
    ptrs[0].d += 3;
    ptrs[1].d += 3;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_dpdp3_iD3arDp3q_01_f_(int32_t narg, int32_t ps[], void (*f)(double [3], double [3][3]))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D3*;rD+3&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  while (infos[0].nelem--) {
    f(ptrs[0].d, (double (*)[3]) ptrs[1].d);
    ptrs[0].d += 3;
    ptrs[1].d += 9;
  }
  free(ptrs);
  free(infos);
  return iq;
}
/*-----------------------------------------------------------------------*/
int32_t lux_v_sddsd_iDaD1rDq_012_f_(int32_t narg, int32_t ps[], void (*f)(double *, size_t, size_t, double, double *, size_t, size_t))
{
  Pointer *ptrs;
  loopInfo *infos;
  int32_t iq;

  if ((iq = standard_args(narg, ps, "i>D*;iD1?;rD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  double width = ptrs[1].d? ptrs[1].d[0]: 3;
  f(&ptrs[0].d[0], infos[0].dims[0], 1, width, &ptrs[2].d[0], infos[2].dims[0], 1);
  free(ptrs);
  free(infos);
  return iq;
}
