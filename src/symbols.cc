/* This is file symbols.c.

Copyright 2013 Louis Strous, Richard Shine

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
/* LUX routines dealing with the creation of LUX symbols. */
#include "config.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include "install.hh"
#include "action.hh"
#include "editor.hh"		/* for BUFSIZE */
extern "C" {
#include "visualclass.h"
}
#if HAVE_LIBX11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

void	zerobytes(void *, int32_t), updateIndices(void), symdumpswitch(int32_t, int32_t);
#if HAVE_LIBX11
void	xsynchronize(int32_t);
#endif
int32_t	installString(char const*), fixContext(int32_t, int32_t), lux_replace(int32_t, int32_t);
char *fmttok(char *);
int32_t Sprintf_tok(char *, ...);
/*-----------------------------------------------------*/
/* combinedType(type1,type2) returns the type that a value based on
   the combination of the two specified types should get.  The
   combined type is as wide as the widest of the two types.  For
   complex types, count the width of one of the two components.  If a
   complex type is involved, then the combined type is complex.
   Otherwise, if a float type is involved, then the combined type is
   float.  Otherwise, the combined type is integer.

       B  W  L  Q  F  D CF CD
   B   B  W  L  Q  F  D CF CD
   W   W  W  L  Q  F  D CF CD
   L   L  L  L  Q  F  D CF CD
   Q   Q  Q  Q  Q  D  D CD CD
   F   F  F  F  D  F  D CF CD
   D   D  D  D  D  D  D CD CD
   CF CF CF CF CD CF CD CF CD
   CD CD CD CD CD CD CD CD CD
   w   1  2  4  8  4  8  4  8
*/
Symboltype combinedType(Symboltype type1, Symboltype type2)
{
  const Symboltype N = (Symboltype) 0;
  static Symboltype const combined[LUX_NO_SYMBOLTYPE][LUX_NO_SYMBOLTYPE] =
    {
      {    LUX_INT8,   LUX_INT16,    LUX_INT32,   LUX_INT64,  LUX_FLOAT,  LUX_DOUBLE, N, N, N,  LUX_CFLOAT, LUX_CDOUBLE }, /* LUX_INT8 */
      {   LUX_INT16,   LUX_INT16,    LUX_INT32,   LUX_INT64,  LUX_FLOAT,  LUX_DOUBLE, N, N, N,  LUX_CFLOAT, LUX_CDOUBLE }, /* LUX_INT16 */
      {   LUX_INT32,   LUX_INT32,    LUX_INT32,   LUX_INT64,  LUX_FLOAT,  LUX_DOUBLE, N, N, N,  LUX_CFLOAT, LUX_CDOUBLE }, /* LUX_INT32 */
      {   LUX_INT64,   LUX_INT64,    LUX_INT64,   LUX_INT64, LUX_DOUBLE,  LUX_DOUBLE, N, N, N, LUX_CDOUBLE, LUX_CDOUBLE }, /* LUX_INT64 */
      {   LUX_FLOAT,   LUX_FLOAT,   LUX_FLOAT,  LUX_DOUBLE,   LUX_FLOAT,  LUX_DOUBLE, N, N, N,  LUX_CFLOAT, LUX_CDOUBLE }, /* LUX_FLOAT */
      {  LUX_DOUBLE,  LUX_DOUBLE,  LUX_DOUBLE,  LUX_DOUBLE,  LUX_DOUBLE,  LUX_DOUBLE, N, N, N, LUX_CDOUBLE, LUX_CDOUBLE }, /* LUX_DOUBLE */
      {           N,           N,           N,           N,           N,           N, N, N, N,           N,           N }, /* not numerical */
      {           N,           N,           N,           N,           N,           N, N, N, N,           N,           N }, /* not numerical */
      {           N,           N,           N,           N,           N,           N, N, N, N,           N,           N }, /* not numerical */
      {  LUX_CFLOAT,  LUX_CFLOAT,  LUX_CFLOAT, LUX_CDOUBLE,  LUX_CFLOAT, LUX_CDOUBLE, N, N, N,  LUX_CFLOAT, LUX_CDOUBLE }, /* LUX_CFLOAT */
      { LUX_CDOUBLE, LUX_CDOUBLE, LUX_CDOUBLE, LUX_CDOUBLE, LUX_CDOUBLE, LUX_CDOUBLE, N, N, N, LUX_CDOUBLE, LUX_CDOUBLE }, /* LUX_CDOUBLE */
    };
  assert(type1 >= 0 && type1 < LUX_NO_SYMBOLTYPE);
  assert(type2 >= 0 && type2 < LUX_NO_SYMBOLTYPE);
  return combined[type1][type2];
}
/*-----------------------------------------------------*/
void embed(int32_t target, int32_t context)
/* gives <target> the specified <context>, if it is a contextless */
/* temporary.  LS 15jun98 */
{
  if (target < 0)
    target = -target;
  if (target >= NAMED_END || !symbolProperName(target)) {
    symbol_context(target) = context;
    if (context >= NAMED_END
	&& symbol_line(context) > symbol_line(target))
      symbol_line(context) = symbol_line(target);
  }
}
/*-----------------------------------------------------*/
int32_t structPtrTarget(int32_t symbol)
/* returns number of symbol that LIST_PTR <symbol> points at */
{
  int32_t	base, index = -1, i, n;
  char	*key;
  
  base = list_ptr_target(symbol); /* the enveloping structure */
  if (base < 0) {		/* numerical label */
    base = -base;
    index = list_ptr_tag_number(symbol);
  } else
    key = list_ptr_tag_string(symbol);
  if (base >= NSYM)
    return cerror(BAD_STRUCT_KEY, symbol);
  base = transfer(base);
  switch (symbol_class(base)) {	/* what kind of envelope? */
    case LUX_RANGE:
      /* if the indicated range start or end contains a (*-...) entry,
       then the negative of its value is returned */
      if (index < 0 || index > 1) /* bad label */
	return cerror(BAD_STRUCT_KEY, symbol);
      n = index? range_end(base): range_start(base);
      if (n < 0)
	n = -n;
      return n;
    case LUX_CLIST:
      if (index < 0 || index >= clist_num_symbols(base))
	return cerror(BAD_STRUCT_KEY, symbol);
      return clist_symbols(base)[index];
    case LUX_LIST:
      if (index < 0) {		/* need to match the key */
	for (i = 0; i < (n = list_num_symbols(base)); i++) {
	  if (!strcmp(key, list_key(base,i)))
	    break;
	}
	if (i < n)
	  index = i;
      }
      if (index < 0 || index >= n)
	return cerror(BAD_STRUCT_KEY, symbol);
      return list_symbol(base,index);
    case LUX_SUBROUTINE:
    case LUX_FUNCTION:
      if (index < 0) {		/* need to match the key */
	i = installString(key);
	n = findVar(i, base);
	freeString(i);
      }
      if (index >= 0 || n < 0)
	return cerror(BAD_STRUCT_KEY, symbol);
      return transfer(n);
    default:
      return luxerror("Pointer to non-embedding variable.", symbol);
  }
}
/*-----------------------------------------------------*/
int32_t transfer(int32_t symbol)
/* if symbol is a TRANSFER or POINTER, return symbol number of */
/* ultimate target */
{
  while (symbol_class(symbol) == LUX_POINTER
	 || symbol_class(symbol) == LUX_TRANSFER) {
    if (transfer_target(symbol) == symbol) {
      symbol_class(symbol) = LUX_UNDEFINED;
      printf("WARNING - pointer %s points at itself: make undefined\n",
	     symbolIdent(symbol, I_PARENT));
      return symbol;
    }
    symbol = transfer_target(symbol);
    if (symbol_class(symbol) == LUX_UNUSED)
      symbol = 0;
  }
  return symbol;
}
/*-----------------------------------------------------*/
int32_t lux_convert(int32_t, int32_t [], Symboltype, int32_t);
int32_t lux_convertsym(int32_t narg, int32_t ps[])
     /* Y = CONVERT(X, TYPE) returns a copy of X converted to data type */
     /* TYPE (according to #TYPE).  LS 1aug97 */
{
  int32_t	iq;
  Symboltype type;

  iq = ps[0];
  switch (symbol_class(ps[1])) {
    case LUX_SCAL_PTR:
      iq = dereferenceScalPointer(iq);
    case LUX_SCALAR:
      type = (Symboltype) int_arg(iq);
      break;
    default:
      return cerror(ILL_CLASS, ps[0]);
  }
  if (type < LUX_INT8 || type > LUX_DOUBLE)
    return cerror(ILL_TYPE, ps[1]);
  return lux_convert(1, ps, type, 1);
}
/*-----------------------------------------------------*/
int32_t scalar_scratch(Symboltype type)
/* returns a temporary scalar of the indicated type */
{
 int32_t	n;

 getFreeTempVariable(n);
 symbol_type(n) = type;
 symbol_line(n) = curLineNumber;
 if (type >= LUX_CFLOAT) {
   complex_scalar_memory(n) = lux_type_size[type];
   complex_scalar_data(n).f = (float*) malloc(complex_scalar_memory(n));
   if (!complex_scalar_data(n).f)
     return cerror(ALLOC_ERR, n);
   symbol_class(n) = LUX_CSCALAR;
 } else
   symbol_class(n) = LUX_SCALAR;
 return n;
}
/*-----------------------------------------------------*/
int32_t scalar_scratch_copy(int32_t nsym)
/* creates a temporary scalar which is a copy of <nsym> */
{
 int32_t	n;

 getFreeTempVariable(n);
 symbol_class(n) = LUX_SCALAR;
 sym[n].type = sym[nsym].type;
 sym[n].line = curLineNumber;
 memcpy(&sym[n].spec.scalar.b, &sym[nsym].spec.scalar.b, sizeof(double));
 return n;
}
/*-----------------------------------------------------*/
int32_t string_scratch(int32_t size)
/* returns a new temporary string with the indicated size
   (terminating null not counted) */
{
 int32_t	n;

 getFreeTempVariable(n);
 symbol_class(n) = LUX_STRING;
 sym[n].type = LUX_TEMP_STRING;
 sym[n].line = curLineNumber;
 if (size >= 0)
 { allocate(string_value(n), size + 1, char);
   sym[n].spec.array.bstore = size + 1; }
 else
 { string_value(n) = NULL;
   symbol_memory(n) = 0; }
 return n;
}
/*-----------------------------------------------------*/
int32_t to_scratch_array(int32_t n, Symboltype type, int32_t ndim, int32_t dims[])
/* modifies symbol <n> to an array of the specified type and dimensions */
/* if the type is LUX_TEMP_STRING or LUX_LSTRING, then the new */
/* array gets type LUX_STRING_ARRAY.  LS 28mar98 */
{
 size_t	size, i;
 float	fsize;
 array	*h;
 pointer	ptr;

 if (isStringType(type))
   type = LUX_STRING_ARRAY;
 if (!isLegalType(type))
   return cerror(ILL_TYPE, n, typeName(type));
 size = lux_type_size[type];
 /* we calculate the number of bytes requested for the data.
    This number may be greater than can be represented in an integer of
    type size_t -- which is the widest integer type available on the
    current system.  If you want so many bytes that the number doesn't
    fit even in a size_t variable, then you request too many bytes.
    C does not define a method for finding out if the result of an
    integer multiplication exceeds the greatest allowed integer number,
    so we must invent our own method. */
 fsize = size;
 for (i = 0; i < ndim; i++) {
   if (dims[i] <= 0)
     return luxerror("Illegal dimension size, %1d", 0, dims[i]);
   fsize *= dims[i];
   size *= dims[i];
 }
 size += sizeof(array);
 fsize += sizeof(array);
 if (fabs(((float) size)/fsize - 1) > 1e-3 || size > INT32_MAX)
   return luxerror("The number of bytes requested for the array\n(about %g) is too great", 0, fsize);
 undefine(n);
 symbol_class(n) = isComplexType(type)? LUX_CARRAY: LUX_ARRAY;
 array_type(n) = type;
 symbol_line(n) = curLineNumber;
 if (!(ptr.v = malloc(size))) {
   printf("requested %1d bytes in array_scratch\n", size);
   return cerror(ALLOC_ERR, 0);
 }
 array_header(n) = h = (array *) ptr.v;
 symbol_memory(n) = size;
 array_num_dims(n) = ndim;
 memcpy(array_dims(n), dims, ndim*sizeof(int32_t));
 h->c1 = h->c2 = h->nfacts = 0;
 h->facts = NULL;
 zerobytes(array_data(n), array_size(n)*lux_type_size[type]);
 return n;
}
/*-----------------------------------------------------*/
int32_t to_scalar(int32_t nsym, Symboltype type)
/* turns symbol <nsym> into a scalar or cscalar of the given <type> */
{
  undefine(nsym);
  symbol_class(nsym) = isRealType(type)? LUX_SCALAR: LUX_CSCALAR;
  symbol_type(nsym) = type;
  if (isComplexType(type)) {
    complex_scalar_data(nsym).cf = (floatComplex*) malloc(lux_type_size[type]);
    if (!complex_scalar_data(nsym).cf)
      return cerror(ALLOC_ERR, 0);
  }
  return LUX_OK;
}
/*-----------------------------------------------------*/
int32_t array_scratch(Symboltype type, int32_t ndim, int32_t dims[])
{
  int32_t	n;

  if (!isLegalType(type))
    return cerror(ILL_TYPE, 0, typeName(type));
  getFreeTempVariable(n);
  return to_scratch_array(n, type, ndim, dims);
}
/*-----------------------------------------------------*/
int32_t array_clone(int32_t symbol, Symboltype type)
/* returns a new temporary array of the indicated type,
  with the same structure as <symbol> */
{
 int32_t	n, size;
 float	fsize;
 array	*h, *hOld;
 void	*ptr;
 extern int32_t	pipeSym, pipeExec;

 size = ((symbol_memory(symbol) - sizeof(array))
	 / lux_type_size[array_type(symbol)]) * lux_type_size[type]
   + sizeof(array);
 if (lux_type_size[type] > lux_type_size[array_type(symbol)]) {
   fsize = ((float) (symbol_memory(symbol) - sizeof(array))
	    / lux_type_size[array_type(symbol)]) * lux_type_size[type]
     + sizeof(array);
   if (fsize != (float) size)
     return luxerror("The number of bytes requested for the array\n(about %g) is too great", 0, fsize);
 }
 if (!pipeExec
     && pipeSym
     && symbol_memory(pipeSym) == size) {
   n = pipeSym;
   pipeSym = 0;
   memcpy(array_header(n), array_header(symbol), sizeof(array));
 } else {
   getFreeTempVariable(n);
   symbol_class(n) = ((isRealType(type) || isStringType(type))?
		      LUX_ARRAY: LUX_CARRAY);
   symbol_line(n) = curLineNumber;
   if (!(ptr = malloc(size))) {
     printf("requested %1d bytes in array_clone\n", size);
     return cerror(ALLOC_ERR, 0);
   }
   array_header(n) = (array*) ptr;
   symbol_memory(n) = size;
   h = (array *) ptr;
   hOld = HEAD(symbol);
   h->ndim = hOld->ndim;
   memcpy(h->dims, hOld->dims, h->ndim*sizeof(int32_t));
   h->c1 = h->c2 = h->nfacts = 0;
   h->facts = NULL;
 }
 array_type(n) = type;
 return n;
}
/*-----------------------------------------------------*/
int32_t numerical_clone(int32_t iq, Symboltype type) {
  switch (symbol_class(iq)) {
  case LUX_ARRAY:
    return array_clone(iq, type);
  case LUX_SCALAR:
    return scalar_scratch(type);
  default:
    return luxerror("Need numerical argument", iq);
  }
}
/*-----------------------------------------------------*/
int32_t dereferenceScalPointer(int32_t nsym)
/* returns an ordinary LUX_SCALAR for a LUX_SCAL_PTR.  NOTE: assumes that
 <nsym> is a SCAL_PTR!  LS 31jul98 */
{
 int32_t	n;
 Symboltype type;
 pointer	ptr;

 type = scal_ptr_type(nsym);
 if (type == LUX_TEMP_STRING)
   n = string_scratch(string_size(nsym));
 else if (isFreeTemp(nsym))
   n = nsym;
 else
   n = nextFreeTempVariable();
 switch (type) {
   case LUX_TEMP_STRING:
     break;
   case LUX_CFLOAT: case LUX_CDOUBLE:
     symbol_class(n) = LUX_CSCALAR;
     complex_scalar_type(n) = type;
     complex_scalar_memory(n) = lux_type_size[type];
     complex_scalar_data(n).f = (float*) malloc(complex_scalar_memory(n));
     if (!complex_scalar_data(n).f)
       return cerror(ALLOC_ERR, n);
     break;
   default:
     symbol_class(n) = LUX_SCALAR;
     scalar_type(n) = type;
     break;
 }
 ptr = scal_ptr_pointer(nsym);
 switch (scal_ptr_type(nsym)) {
 case LUX_INT8:
   scalar_value(n).b = *ptr.b;
   break;
 case LUX_INT16:
   scalar_value(n).w = *ptr.w;
   break;
 case LUX_INT32:
   scalar_value(n).l = *ptr.l;
   break;
 case LUX_INT64:
   scalar_value(n).q = *ptr.q;
   break;
 case LUX_FLOAT:
   scalar_value(n).f = *ptr.f;
   break;
 case LUX_DOUBLE:
   scalar_value(n).d = *ptr.d;
   break;
 case LUX_CFLOAT:
   complex_scalar_data(n).cf->real = ptr.cf->real;
   complex_scalar_data(n).cf->imaginary = ptr.cf->imaginary;
   break;
 case LUX_CDOUBLE:
   complex_scalar_data(n).cd->real = ptr.cd->real;
   complex_scalar_data(n).cd->imaginary = ptr.cd->imaginary;
   break;
 case LUX_TEMP_STRING:
   string_value(n) = strsave(ptr.s);
   break;
 default:
   return cerror(ILL_CLASS, nsym);
 }
 return n;
}
/*-----------------------------------------------------*/
char *strsave(char const* str)
/* saves string <str> and returns address */
{
 char	*p;

 if (!(p = (char *) malloc(strlen(str) + 1))) {
   printf("strsave: ");
   cerror(ALLOC_ERR, 0);
   return NULL;
 }
 strcpy(p, str);
 return p;
}
/*-----------------------------------------------------*/
int32_t int_arg(int32_t nsym)
/* returns the 32-bit integer value of a scalar symbol */
{
 if (nsym < 0 || nsym >= NSYM) {
   cerror(ILL_SYM, 0, nsym, "int_arg");
   return 0;
 }
 if (symbol_class(nsym) == LUX_SCAL_PTR)
   nsym = dereferenceScalPointer(nsym);
 if (symbol_class(nsym) != LUX_SCALAR) {
   cerror(NO_SCAL, nsym);
   return 0;
 }
 switch (scalar_type(nsym)) {
   case LUX_INT8:
     return (int32_t) scalar_value(nsym).b;
   case LUX_INT16:
     return (int32_t) scalar_value(nsym).w;
   case LUX_INT32:
     return (int32_t) scalar_value(nsym).l;
   case LUX_INT64:
     return (int32_t) scalar_value(nsym).q;
   case LUX_FLOAT:
     return (int32_t) scalar_value(nsym).f;
   case LUX_DOUBLE:
     return (int32_t) scalar_value(nsym).d;
   default:
     return cerror(ILL_TYPE, nsym);
 }
}
/*-----------------------------------------------------*/
int32_t int_arg_stat(int32_t nsym, int32_t *value)
/* returns integer value of symbol <nsym>, if any, or an error */
{
 if (nsym < 0 || nsym >= NSYM)
   return cerror(ILL_SYM, 0, nsym, "int_arg_stat");
 if (symbol_class(nsym) == LUX_SCAL_PTR)
   nsym = dereferenceScalPointer(nsym);
 if (symbol_class(nsym) != LUX_SCALAR)
   return cerror(NO_SCAL, nsym);
 switch (scalar_type(nsym)) {
 case LUX_INT8:
   *value = (int32_t) scalar_value(nsym).b;
   break;
 case LUX_INT16:
   *value = (int32_t) scalar_value(nsym).w;
   break;
 case LUX_INT32:
   *value = (int32_t) scalar_value(nsym).l;
   break;
 case LUX_INT64:
   *value = (int32_t) scalar_value(nsym).q;
   break;
 case LUX_FLOAT:
   *value = (int32_t) scalar_value(nsym).f;
   break;
 case LUX_DOUBLE:
   *value = (int32_t) scalar_value(nsym).d;
   break;
 }
 return 1;			/* everything OK */
}
/*-----------------------------------------------------*/
float float_arg(int32_t nsym)
/* returns the float value of a scalar symbol */
{
  if (nsym < 0 || nsym >= NSYM)
    { cerror(ILL_SYM, 0, nsym, "float_arg");
      return 0; }
  if (symbol_class(nsym) == LUX_SCAL_PTR) nsym = dereferenceScalPointer(nsym);
  if (symbol_class(nsym) != LUX_SCALAR) { cerror(NO_SCAL, nsym);  return 0.0; }
  switch (scalar_type(nsym)) {
  case LUX_INT8:
    return (float) scalar_value(nsym).b;
  case LUX_INT16:
    return (float) scalar_value(nsym).w;
  case LUX_INT32:
    return (float) scalar_value(nsym).l;
  case LUX_INT64:
    return (float) scalar_value(nsym).q;
  case LUX_FLOAT:
    return (float) scalar_value(nsym).f;
  case LUX_DOUBLE:
    return (float) scalar_value(nsym).d;
  default:
    return cerror(ILL_TYPE, nsym);
  }
}
/*-----------------------------------------------------*/
int32_t float_arg_stat(int32_t nsym, float *value)
/* returns float value of symbol <nsym>, if any, or an error */
{
 if (nsym < 0 || nsym >= NSYM)
   return cerror(ILL_SYM, 0, nsym, "float_arg_stat");
 if (symbol_class(nsym) == LUX_SCAL_PTR)
   nsym = dereferenceScalPointer(nsym);
 if (symbol_class(nsym) != LUX_SCALAR)
   return cerror(NO_SCAL, nsym);
 switch (scalar_type(nsym)) {
   case LUX_INT8:
     *value = (float) scalar_value(nsym).b;
     break;
   case LUX_INT16:
     *value = (float) scalar_value(nsym).w;
     break;
   case LUX_INT32:
     *value = (float) scalar_value(nsym).l;
     break;
   case LUX_INT64:
     *value = (float) scalar_value(nsym).q;
     break;
   case LUX_FLOAT:
     *value = (float) scalar_value(nsym).f;
     break;
   case LUX_DOUBLE:
     *value = (float) scalar_value(nsym).d;
     break;
 }
 return LUX_OK;			/* everything OK */
}
/*-----------------------------------------------------*/
double double_arg(int32_t nsym)
/* returns the double value of a LUX_DOUBLE scalar symbol */
{
 if (nsym < 0 || nsym >= NSYM)
 { cerror(ILL_SYM, 0, nsym, "double_arg");
   return 0; }
 if (symbol_class(nsym) == LUX_SCAL_PTR) nsym = dereferenceScalPointer(nsym);
 if (symbol_class(nsym) != LUX_SCALAR) { cerror(NO_SCAL, nsym);  return 0.0; }
 switch (scalar_type(nsym))
 { case LUX_INT8:
     return (double) scalar_value(nsym).b;
   case LUX_INT16:
     return (double) scalar_value(nsym).w;
   case LUX_INT32:
     return (double) scalar_value(nsym).l;
   case LUX_INT64:
     return (double) scalar_value(nsym).q;
   case LUX_FLOAT:
     return (double) scalar_value(nsym).f;
   case LUX_DOUBLE:
     return (double) scalar_value(nsym).d;
   default:
     return cerror(ILL_TYPE, nsym); }
}
/*-----------------------------------------------------*/
int32_t double_arg_stat(int32_t nsym, double *value)
/* returns double value of symbol <nsym>, if any, or an error */
{
 if (nsym < 0 || nsym >= NSYM)
   return cerror(ILL_SYM, 0, nsym, "int_arg_stat");
 if (symbol_class(nsym) == LUX_SCAL_PTR)
   nsym = dereferenceScalPointer(nsym);
 if (symbol_class(nsym) != LUX_SCALAR)
   return cerror(NO_SCAL, nsym);
 switch (scalar_type(nsym)) {
   case LUX_INT8:
     *value = (double) scalar_value(nsym).b;
     break;
   case LUX_INT16:
     *value = (double) scalar_value(nsym).w;
     break;
   case LUX_INT32:
     *value = (double) scalar_value(nsym).l;
     break;
   case LUX_INT64:
     *value = (double) scalar_value(nsym).q;
     break;
   case LUX_FLOAT:
     *value = (double) scalar_value(nsym).f;
     break;
   case LUX_DOUBLE:
     *value = (double) scalar_value(nsym).d;
     break;
 }
 return LUX_OK;			/* everything OK */
}
/*-----------------------------------------------------*/
char *string_arg(int32_t nsym)
/* returns the string value of a string symbol, or NULL */
{
 if (nsym < 0 || nsym >= NSYM)
 { cerror(ILL_SYM, 0, nsym, "string_arg");
   return 0; }
 if (symbol_class(nsym) == LUX_SCAL_PTR) nsym = dereferenceScalPointer(nsym);
 if (symbol_class(nsym) != LUX_STRING) { cerror(NEED_STR, nsym);  return NULL; }
 return string_value(nsym);
}
/*-----------------------------------------------------*/
int32_t lux_byte(int32_t narg, int32_t ps[])
/* returns a LUX_INT8 version of the argument */
{
  return lux_convert(narg, ps, LUX_INT8, 1);
}
/*-----------------------------------------------------*/
int32_t lux_word(int32_t narg, int32_t ps[])
/* returns a LUX_INT16 version of the argument */
{
  return lux_convert(narg, ps, LUX_INT16, 1);
}
/*-----------------------------------------------------*/
int32_t lux_long(int32_t narg, int32_t ps[])
/* returns a LUX_INT32 version of the argument */
{
  return lux_convert(narg, ps, LUX_INT32, 1);
}
/*-----------------------------------------------------*/
int32_t lux_int64(int32_t narg, int32_t ps[])
/* returns a LUX_INT64 version of the argument */
{
  return lux_convert(narg, ps, LUX_INT64, 1);
}
/*-----------------------------------------------------*/
int32_t lux_floor(int32_t narg, int32_t ps[])
/* returns a LUX_INT32 version of the argument */
/* each float number is transformed into the next lower integer */
/* compare lux_long(), where each float number is transformed into the */
/* next integer closer to zero, and lux_rfix(), where each float is */
/* transformed into the closest integer.  LS 24may96 */
{
 int32_t	iq, result, n, temp, size, type;
 int32_t	value;
 pointer	src, trgt;

 iq = *ps;
 if (!symbolIsNumerical(iq)	/* not numerical */
     && !symbolIsStringScalar(iq))	/* and not a string either */
   return cerror(ILL_CLASS, iq); /* reject */
 if (isIntegerType(symbol_type(iq))) /* if it's already integer then we're done */
   return iq;
 temp = (isFreeTemp(iq))? 1: 0;
 type = symbol_type(iq);	/* gotta store now because "result" may */
				/* be same symbol as "iq" */
 switch (symbol_class(iq)) {
   case LUX_SCAL_PTR:		/* transform to scalar */
     iq = dereferenceScalPointer(iq);
     temp = (isFreeTemp(iq))? 1: 0;
   case LUX_SCALAR:
     if (temp)			/* can use iq to store the result */
       result = iq;
     else			/* need new scalar */
       result = scalar_scratch(LUX_INT32);
     trgt.b = &scalar_value(result).b;
     src.b = &scalar_value(iq).b;
     n = 1;
     break;
   case LUX_STRING:
     if (temp)
       result = iq;
     else
       result = scalar_scratch(LUX_INT32);
     value = (int32_t) floor(atof((char *) string_value(iq))); /* convert */
     if (temp) {
       free(string_value(iq));	/* change string to scalar: free up memory */
       symbol_class(result) = LUX_SCALAR;
       scalar_type(result) = LUX_INT32;
     }
     scalar_value(result).l = value;
     return result;
   case LUX_CSCALAR:
     result = scalar_scratch(LUX_INT32);
     temp = 0;
     n = 1;
     trgt.b = &scalar_value(result).b;
     src.cf = complex_scalar_data(iq).cf;
     break;
   case LUX_ARRAY:
     /* we can use the input symbol if it is free and if
	it has at least as much memory as we need */
     if (temp
	 && (int32_t) array_type(iq) >= LUX_INT32)
       result = iq;
     else {
       result = array_clone(iq, LUX_INT32);
       temp = 0;
     }
     n = array_size(result);
     trgt.v = array_data(result);
     src.v = array_data(iq);
     break;
   default:
     return cerror(ILL_CLASS, iq);
 }
	/* convert */
 size = n;
 switch (type) {
   case LUX_INT8:
     while (n--)
       *trgt.l++ = (int32_t) *src.b++;
     break;
   case LUX_INT16:
     while (n--)
       *trgt.l++ = (int32_t) *src.w++;
     break;
   case LUX_INT32:
     while (n--)
       *trgt.l++ = (int32_t) *src.l++;
     break;
   case LUX_INT64:
     while (n--)
       *trgt.l++ = (int32_t) *src.q++;
     break;
   case LUX_FLOAT:
     while (n--)
       *trgt.l++ = (int32_t) floor(*src.f++);
     break;
   case LUX_DOUBLE:
     while (n--)
       *trgt.l++ = (int32_t) floor(*src.d++);
     break;
   case LUX_CFLOAT:
     while (n--)
       *trgt.l++ = (int32_t) floor(src.cf++->real);
     break;
   case LUX_CDOUBLE:
     while (n--)
       *trgt.l++ = (int32_t) floor(src.cd++->real);
     break;
 }
 if (temp			/* we used input symbol to store results */
     && symbol_class(iq) == LUX_ARRAY /* it's an array */
     && array_type(iq) > LUX_INT32) { /* and bigger than we needed */
   symbol_memory(iq) = sizeof(array) + size*sizeof(int32_t);
   symbol_data(iq) = (array *) realloc(symbol_data(iq), symbol_memory(iq));
   if (!symbol_data(iq))	/* reallocation failed */
     return luxerror("Realloc() failed in lux_floor", 0);
 }
 symbol_type(result) = LUX_INT32;
 return result;
}
/*-----------------------------------------------------*/
int32_t lux_ceil(int32_t narg, int32_t ps[])
/* returns a LUX_INT32 version of the argument */
/* each float number is transformed into the next higher integer */
/* compare lux_long(), where each float number is transformed into the */
/* next integer closer to zero, and lux_rfix(), where each float is */
/* transformed into the closest integer.  LS 24may96 */
{
 int32_t	iq, result, n, temp, size, type;
 int32_t	value;
 pointer	src, trgt;

 iq = *ps;
 if (!symbolIsNumerical(iq)	/* not numerical */
     && !symbolIsStringScalar(iq))	/* and not a string either */
   return cerror(ILL_CLASS, iq); /* reject */
 if (isIntegerType(symbol_type(iq))) /* if it's already integer then we're done */
   return iq;
 temp = (isFreeTemp(iq))? 1: 0;
 type = symbol_type(iq);	/* gotta store now because "result" may */
				/* be same symbol as "iq" */
 switch (symbol_class(iq)) {
   case LUX_SCAL_PTR:		/* transform to scalar */
     iq = dereferenceScalPointer(iq);
     temp = (isFreeTemp(iq))? 1: 0;
   case LUX_SCALAR:
     if (temp)			/* can use iq to store the result */
       result = iq;
     else			/* need new scalar */
       result = scalar_scratch(LUX_INT32);
     trgt.b = &scalar_value(result).b;
     src.b = &scalar_value(iq).b;
     n = 1;
     break;
   case LUX_STRING:
     if (temp)
       result = iq;
     else
       result = scalar_scratch(LUX_INT32);
     value = (int32_t) ceil(atof((char *) string_value(iq))); /* convert */
     if (temp) {
       free(string_value(iq));	/* change string to scalar: free up memory */
       symbol_class(result) = LUX_SCALAR;
       scalar_type(result) = LUX_INT32;
     }
     scalar_value(result).l = value;
     return result;
   case LUX_CSCALAR:
     result = scalar_scratch(LUX_INT32);
     temp = 0;
     n = 1;
     trgt.b = &scalar_value(result).b;
     src.cf = complex_scalar_data(iq).cf;
     break;
   case LUX_ARRAY:
     /* we can use the input symbol if it is free and if
	it has at least as much memory as we need */
     if (temp
	 && (int32_t) array_type(iq) >= LUX_INT32)
       result = iq;
     else {
       result = array_clone(iq, LUX_INT32);
       temp = 0;
     }
     n = array_size(result);
     trgt.v = array_data(result);
     src.v = array_data(iq);
     break;
   default:
     return cerror(ILL_CLASS, iq);
 }
	/* convert */
 size = n;
 switch (type) {
   case LUX_INT8:
     while (n--)
       *trgt.l++ = (int32_t) *src.b++;
     break;
   case LUX_INT16:
     while (n--)
       *trgt.l++ = (int32_t) *src.w++;
     break;
   case LUX_INT32:
     while (n--)
       *trgt.l++ = (int32_t) *src.l++;
     break;
   case LUX_INT64:
     while (n--)
       *trgt.l++ = (int32_t) *src.q++;
     break;
   case LUX_FLOAT:
     while (n--)
       *trgt.l++ = (int32_t) ceil(*src.f++);
     break;
   case LUX_DOUBLE:
     while (n--)
       *trgt.l++ = (int32_t) ceil(*src.d++);
     break;
   case LUX_CFLOAT:
     while (n--)
       *trgt.l++ = (int32_t) ceil(src.cf++->real);
     break;
   case LUX_CDOUBLE:
     while (n--)
       *trgt.l++ = (int32_t) ceil(src.cd++->real);
     break;
 }
 if (temp			/* we used input symbol to store results */
     && symbol_class(iq) == LUX_ARRAY /* it's an array */
     && array_type(iq) > LUX_INT32) { /* and bigger than we needed */
   symbol_memory(iq) = sizeof(array) + size*sizeof(int32_t);
   symbol_data(iq) = (array *) realloc(symbol_data(iq), symbol_memory(iq));
   if (!symbol_data(iq))	/* reallocation failed */
     return luxerror("Realloc() failed in lux_floor", 0);
 }
 symbol_type(result) = LUX_INT32;
 return result;
}
/*-----------------------------------------------------*/
int32_t lux_float(int32_t narg, int32_t ps[])
/* returns a LUX_FLOAT version of the argument */
{
  return lux_convert(narg, ps, LUX_FLOAT, 1);
}
/*-----------------------------------------------------*/
int32_t lux_double(int32_t narg, int32_t ps[])
/* returns a LUX_DOUBLE version of the argument */
{
  return lux_convert(narg, ps, LUX_DOUBLE, 1);
}
/*-----------------------------------------------------*/
int32_t lux_cfloat(int32_t narg, int32_t ps[])
/* returns an LUX_CFLOAT version of the argument */
{
  return lux_convert(narg, ps, LUX_CFLOAT, 1);
}
/*-----------------------------------------------------*/
int32_t lux_cdouble(int32_t narg, int32_t ps[])
/* returns an LUX_CDOUBLE version of the argument */
{
  return lux_convert(narg, ps, LUX_CDOUBLE, 1);
}
/*-----------------------------------------------------*/
extern int32_t	nFixed;
int32_t lux_convert(int32_t narg, int32_t ps[], Symboltype totype, int32_t isFunc)
/* converts ps[0] to data type <totype>. */
/* we use this function in one of two modes: function mode (isFunc != 0)
   or subroutine mode (isFunc = 0).  In function mode, there can be
   only a single argument, so then <narg> == 1.  In that case,
   we generate a copy of ps[0] with data type <totype>, which may
   be one of LUX_INT8 ... LUX_DOUBLE LUX_CFLOAT LUX_CDOUBLE
   LUX_TEMP_STRING.  If ps[0] is a temporary variable, then we use it
   to store the results in, too.  In subroutine mode, each of the
   arguments must be a named, writeable symbol, and we convert each
   of them in-place.  LS 16dec99 */
{
  int32_t	iq, n, size, srcstep, trgtstep, temp, result;
  Symboltype type;
  char	do_realloc = 0;
  pointer	src, trgt;
  Scalar	value;
  extern char	*fmt_integer, *fmt_float, *fmt_complex;
  void	read_a_number(char **, Scalar *, Symboltype *);
  char *fmttok(char *);
  int32_t Sprintf_tok(char *, ...);

  while (narg--) {
    iq = *ps++;
    if (!symbolIsNumerical(iq)	/* not numerical */
	&& !symbolIsStringScalar(iq)	/* and not a string */
	&& !symbolIsStringArray(iq))	/* and not a string array either */
      return cerror(ILL_CLASS, iq); /* reject */
    if (!isFunc && (!symbolIsNamed(iq) || iq <= nFixed))
      return luxerror("Cannot modify symbol", iq);
    if (symbol_type(iq) == totype) { /* if it's already of the desired type */
				     /* then we're done */
      if (isFunc)
	return iq;
      continue;
    }
    type = symbol_type(iq);
    srcstep = lux_type_size[type];
    trgtstep = lux_type_size[totype];
    if (!isFunc || isFreeTemp(iq))
      result = iq;
    else {
      getFreeTempVariable(result);
    }
    switch (symbol_class(iq)) {	/* source */
      case LUX_SCALAR:
	if (isComplexType(totype)) {
	  value = scalar_value(iq);
	  src.b = &value.b;
	  complex_scalar_data(result).cf = (floatComplex*) malloc(trgtstep);
	  symbol_memory(result) = trgtstep;
	  if (!complex_scalar_data(result).cf)
	    return cerror(ALLOC_ERR, 0);
	  trgt.cf = complex_scalar_data(result).cf;
	  symbol_class(result) = LUX_CSCALAR;
	} else if (isStringType(totype)) {
	  src.b = &scalar_value(iq).b;
	  switch (type) {
	    case LUX_INT8:
	      fmttok(fmt_integer);
	      Sprintf_tok(curScrat, (int32_t) *src.b);
	      break;
	    case LUX_INT16:
	      fmttok(fmt_integer);
	      Sprintf_tok(curScrat, (int32_t) *src.w);
	      break;
	    case LUX_INT32:
	      fmttok(fmt_integer);
	      Sprintf_tok(curScrat, (int32_t) *src.l);
	      break;
	    case LUX_INT64:
	      fmttok(fmt_integer);
	      Sprintf_tok(curScrat, *src.q);
	      break;
	    case LUX_FLOAT:
	      fmttok(fmt_float);
	      Sprintf_tok(curScrat, (double) *src.f);
	      break;
	    case LUX_DOUBLE:
	      fmttok(fmt_float);
	      Sprintf_tok(curScrat, (double) *src.d);
	      break;
	  }
	  size = strlen(curScrat) + 1;
	  symbol_memory(result) = size;
	  symbol_class(result) = LUX_STRING;
	  symbol_type(result) = LUX_TEMP_STRING;
	  string_value(result) = (char*) malloc(size);
	  if (!string_value(result))
	    return cerror(ALLOC_ERR, 0);
	  memcpy(string_value(result), curScrat, size);
	  if (isFunc)
	    return result;
	  continue;
	} else {
	  symbol_class(result) = LUX_SCALAR;
	  src.b = &scalar_value(iq).b;
	  trgt.b = &scalar_value(result).b;
	}
	n = 1;
	break;
      case LUX_STRING:
	if (isIntegerType(totype))
	  value.l = atol(string_value(iq));
	else if (isRealType(totype))
	  value.d = atof(string_value(iq));
	else {			/* complex type */
	  src.s = string_value(iq);
	  read_a_number(&src.s, &value, NULL);
	}
	if (result == iq)
	  free(string_value(iq)); /* change string to scalar: free up memory */
	if (isRealType(totype)) {
	  symbol_class(result) = LUX_SCALAR;
	  trgt.b = &scalar_value(result).b;
	} else {			/* complex output */
	  symbol_class(result) = LUX_CSCALAR;
	  complex_scalar_data(result).cf = (floatComplex*) malloc(trgtstep);
	  if (!complex_scalar_data(result).cf)
	    return cerror(ALLOC_ERR, 0);
	  symbol_memory(result) = trgtstep;
	  if (!complex_scalar_data(result).cf)
	    return cerror(ALLOC_ERR, 0);
	  trgt.cf = complex_scalar_data(result).cf;
	}
	switch (totype) {
	  case LUX_INT8:
	    *trgt.b = value.l;
	    break;
	  case LUX_INT16:
	    *trgt.w = value.l;
	    break;
	  case LUX_INT32:
	    *trgt.l = value.l;
	    break;
	  case LUX_INT64:
	    *trgt.q = value.q;
	    break;
	  case LUX_FLOAT:
	    *trgt.f = value.d;
	    break;
	  case LUX_DOUBLE:
	    *trgt.d = value.d;
	    break;
	  case LUX_CFLOAT:
	    trgt.cf->real = value.d;
	    trgt.cf->imaginary = 0.0;
	    break;
	  case LUX_CDOUBLE:
	    trgt.cd->real = value.d;
	    trgt.cd->imaginary = 0.0;
	    break;
	}
	symbol_type(result) = totype;
	if (isFunc)
	  return result;
	continue;
      case LUX_CSCALAR:
	if (isRealType(totype)) {	/* to a real type */
	  switch (type) {
	    case LUX_CFLOAT:
	      value.d = complex_scalar_data(iq).cf->real;
	      break;
	    case LUX_CDOUBLE:
	      value.d = complex_scalar_data(iq).cd->real;
	      break;
	  }
	  if (result == iq)
	    free(complex_scalar_data(iq).cf);
	  symbol_class(result) = LUX_SCALAR;
	  switch (totype) {
	    case LUX_INT8:
	      scalar_value(result).b = (uint8_t) value.d;
	      break;
	    case LUX_INT16:
	      scalar_value(result).w = (int16_t) value.d;
	      break;
	    case LUX_INT32:
	      scalar_value(result).l = (int32_t) value.d;
	      break;
	    case LUX_INT64:
	      scalar_value(result).q = (int64_t) value.d;
	      break;
	    case LUX_FLOAT:
	      scalar_value(result).f = (float) value.d;
	      break;
	    case LUX_DOUBLE:
	      scalar_value(result).d = (double) value.d;
	      break;
	  }
	  scalar_type(result) = totype;
	  if (isFunc)
	    return result;
	  continue;
	} else if (isStringType(totype)) { /* to a string */
	  switch (type) {
	    case LUX_INT8:
	      fmttok(fmt_integer);
	      Sprintf_tok(curScrat, (int32_t) *src.b);
	      break;
	    case LUX_INT16:
	      fmttok(fmt_integer);
	      Sprintf_tok(curScrat, (int32_t) *src.w);
	      break;
	    case LUX_INT32:
	      fmttok(fmt_integer);
	      Sprintf_tok(curScrat, (int32_t) *src.l);
	      break;
	    case LUX_INT64:
	      fmttok(fmt_integer);
	      Sprintf_tok(curScrat, (int64_t) *src.q);
	      break;
	    case LUX_FLOAT:
	      fmttok(fmt_float);
	      Sprintf_tok(curScrat, (double) *src.f);
	      break;
	    case LUX_DOUBLE:
	      fmttok(fmt_float);
	      Sprintf_tok(curScrat, (double) *src.d);
	      break;
	  }
	  if (result == iq)
	    free(complex_scalar_data(result).cf);
	  size = strlen(curScrat) + 1;
	  symbol_memory(result) = size;
	  symbol_class(result) = LUX_STRING;
	  symbol_type(result) = LUX_TEMP_STRING;
	  string_value(result) = (char*) malloc(size);
	  if (!string_value(result))
	    return cerror(ALLOC_ERR, 0);
	  memcpy(string_value(result), curScrat, size);
	  if (isFunc)
	    return result;
	  continue;
	} else {			/* to a complex type */
	  if (iq == result) {
	    if (trgtstep <= srcstep)
	      do_realloc = 2;
	    else {
	      complex_scalar_data(result).cf = (floatComplex*)
		realloc(complex_scalar_data(result).cf, trgtstep);
	    }
	  } else
	    complex_scalar_data(result).cf = (floatComplex*) malloc(trgtstep);
	  if (!do_realloc && !complex_scalar_data(result).cf)
	    return cerror(ALLOC_ERR, 0);
	  symbol_memory(result) = trgtstep;
	  src.cf = complex_scalar_data(iq).cf;
	  trgt.cf = complex_scalar_data(result).cf;
	  symbol_class(result) = LUX_CSCALAR;
	  n = 1;
	}
	break;
      case LUX_ARRAY: case LUX_CARRAY:
	n = array_size(iq);
	size = n*trgtstep + sizeof(array); /* new size requirement */
	/* if the new size is greater than the old one, then we must */
	/* reallocate before we store the new numbers.  If the new size is */
	/* smaller than the old one, then we must reallocate after we store */
	/* the new numbers, because realloc() is not guaranteed to leave */
	/* any data beyond the new (smaller) size undisturbed and not to */
	/* move the smaller block to a new location.  LS 16dec99 */
	if (iq == result) {
	  if (size <= symbol_memory(iq))
	    do_realloc = 1;		/* need to reallocate afterwards */
	  else
	    array_header(result) = (array*) realloc(array_header(result), size);
	} else
	  array_header(result) = (array*) malloc(size);
	if (!do_realloc && !array_header(result))
	  return cerror(ALLOC_ERR, 0);
	if (iq != result)	/* copy dimensions &c */
	  memcpy(array_header(result), array_header(iq), sizeof(array));
	symbol_memory(result) = size;
	src.v = array_data(iq);
	trgt.v = array_data(result);
	if (isComplexType(totype))
	  symbol_class(result) = LUX_CARRAY;
	else
	  symbol_class(result) = LUX_ARRAY;
	break;
      default:
	return cerror(ILL_CLASS, iq);
    }

    if (trgtstep > srcstep) {
      /* we must start at the end, or else we'll overwrite stuff we */
      /* need later */
      trgt.b += (n - 1)*trgtstep;
      src.b += (n - 1)*srcstep;
      trgtstep = -trgtstep;
      srcstep = -srcstep;
    }

    /* convert */
    switch (type) {		/* source type */
      case LUX_INT8:
	switch (totype) {		/* target type */
	  case LUX_INT16:
	    while (n--) {
	      *trgt.w = (int16_t) *src.b;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_INT32:
	    while (n--) {
	      *trgt.l = (int32_t) *src.b;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_INT64:
	    while (n--) {
	      *trgt.q = *src.b;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_FLOAT:
	    while (n--) {
	      *trgt.f = (float) *src.b;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_DOUBLE:
	    while (n--) {
	      *trgt.d = (double) *src.b;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_CFLOAT:
	    while (n--) {
	      trgt.cf->real = (float) *src.b;
	      trgt.cf->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_CDOUBLE:
	    while (n--) {
	      trgt.cd->real = (double) *src.b;
	      trgt.cd->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_STRING_ARRAY:
	    fmttok(fmt_integer);
	    while (n--) {
	      Sprintf_tok(curScrat, (int32_t) *src.b);
	      temp = strlen(curScrat) + 1;
	      *trgt.sp = (char*) malloc(temp);
	      if (!*trgt.sp)
		return cerror(ALLOC_ERR, 0);
	      memcpy(*trgt.sp, curScrat, temp);
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	}
	break;
      case LUX_INT16:
	switch (totype) {		/* target type */
	  case LUX_INT8:
	    while (n--) {
	      *trgt.b = (int16_t) *src.w;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_INT32:
	    while (n--) {
	      *trgt.l = (int16_t) *src.w;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_INT64:
	    while (n--) {
	      *trgt.q = *src.w;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_FLOAT:
	    while (n--) {
	      *trgt.f = (int16_t) *src.w;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_DOUBLE:
	    while (n--) {
	      *trgt.d = (int16_t) *src.w;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_CFLOAT:
	    while (n--) {
	      trgt.cf->real = (float) *src.w;
	      trgt.cf->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_CDOUBLE:
	    while (n--) {
	      trgt.cd->real = (double) *src.w;
	      trgt.cd->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_STRING_ARRAY:
	    fmttok(fmt_integer);
	    while (n--) {
	      Sprintf_tok(curScrat, (int32_t) *src.w);
	      temp = strlen(curScrat) + 1;
	      *trgt.sp = (char*) malloc(temp);
	      if (!*trgt.sp)
		return cerror(ALLOC_ERR, 0);
	      memcpy(*trgt.sp, curScrat, temp);
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	}
	break;
      case LUX_INT32:
	switch (totype) {		/* target type */
	  case LUX_INT8:
	    while (n--) {
	      *trgt.b = (int32_t) *src.l;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_INT16:
	    while (n--) {
	      *trgt.w = (int32_t) *src.l;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_INT64:
	    while (n--) {
	      *trgt.q = *src.l;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_FLOAT:
	    while (n--) {
	      *trgt.f = (int32_t) *src.l;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_DOUBLE:
	    while (n--) {
	      *trgt.d = (int32_t) *src.l;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_CFLOAT:
	    while (n--) {
	      trgt.cf->real = (float) *src.l;
	      trgt.cf->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_CDOUBLE:
	    while (n--) {
	      trgt.cd->real = (double) *src.l;
	      trgt.cd->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_STRING_ARRAY:
	    fmttok(fmt_integer);
	    while (n--) {
	      Sprintf_tok(curScrat, (int32_t) *src.l);
	      temp = strlen(curScrat) + 1;
	      *trgt.sp = (char*) malloc(temp);
	      if (!*trgt.sp)
		return cerror(ALLOC_ERR, 0);
	      memcpy(*trgt.sp, curScrat, temp);
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	}
	break;
      case LUX_INT64:
	switch (totype) {		/* target type */
	  case LUX_INT8:
	    while (n--) {
	      *trgt.b = *src.q;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_INT16:
	    while (n--) {
	      *trgt.w = *src.q;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_INT32:
	    while (n--) {
	      *trgt.l = *src.q;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_FLOAT:
	    while (n--) {
	      *trgt.f = *src.q;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_DOUBLE:
	    while (n--) {
	      *trgt.d = *src.q;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_CFLOAT:
	    while (n--) {
	      trgt.cf->real = *src.q;
	      trgt.cf->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_CDOUBLE:
	    while (n--) {
	      trgt.cd->real = *src.q;
	      trgt.cd->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_STRING_ARRAY:
	    fmttok(fmt_integer);
	    while (n--) {
	      Sprintf_tok(curScrat, *src.q);
	      temp = strlen(curScrat) + 1;
	      *trgt.sp = (char*) malloc(temp);
	      if (!*trgt.sp)
		return cerror(ALLOC_ERR, 0);
	      memcpy(*trgt.sp, curScrat, temp);
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	}
	break;
      case LUX_FLOAT:
	switch (totype) {		/* target type */
	  case LUX_INT8:
	    while (n--) {
	      *trgt.b = (float) *src.f;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_INT16:
	    while (n--) {
	      *trgt.w = (float) *src.f;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_INT32:
	    while (n--) {
	      *trgt.l = (float) *src.f;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_INT64:
	    while (n--) {
	      *trgt.q = *src.f;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_DOUBLE:
	    while (n--) {
	      *trgt.d = (float) *src.f;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_CFLOAT:
	    while (n--) {
	      trgt.cf->real = (float) *src.f;
	      trgt.cf->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_CDOUBLE:
	    while (n--) {
	      trgt.cd->real = (double) *src.f;
	      trgt.cd->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_STRING_ARRAY:
	    fmttok(fmt_float);
	    while (n--) {
	      Sprintf_tok(curScrat, (double) *src.f);
	      temp = strlen(curScrat) + 1;
	      *trgt.sp = (char*) malloc(temp);
	      if (!*trgt.sp)
		return cerror(ALLOC_ERR, 0);
	      memcpy(*trgt.sp, curScrat, temp);
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	}
	break;
      case LUX_DOUBLE:
	switch (totype) {		/* target type */
	  case LUX_INT8:
	    while (n--) {
	      *trgt.b = (double) *src.d;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_INT16:
	    while (n--) {
	      *trgt.w = (double) *src.d;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_INT32:
	    while (n--) {
	      *trgt.l = (double) *src.d;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_INT64:
	    while (n--) {
	      *trgt.q = *src.d;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_FLOAT:
	    while (n--) {
	      *trgt.f = (double) *src.d;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_CFLOAT:
	    while (n--) {
	      trgt.cf->real = (float) *src.d;
	      trgt.cf->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_CDOUBLE:
	    while (n--) {
	      trgt.cd->real = (double) *src.d;
	      trgt.cd->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_STRING_ARRAY:
	    fmttok(fmt_float);
	    while (n--) {
	      Sprintf_tok(curScrat, (double) *src.d);
	      temp = strlen(curScrat) + 1;
	      *trgt.sp = (char*) malloc(temp);
	      if (!*trgt.sp)
		return cerror(ALLOC_ERR, 0);
	      memcpy(*trgt.sp, curScrat, temp);
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	}
	break;
      case LUX_CFLOAT:
	switch (totype) {
	  case LUX_INT8:
	    while (n--) {
	      *trgt.b = (uint8_t) src.cf->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_INT16:
	    while (n--) {
	      *trgt.w = (int16_t) src.cf->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_INT32:
	    while (n--) {
	      *trgt.l = (int32_t) src.cf->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_INT64:
	    while (n--) {
	      *trgt.q = src.cf->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_FLOAT:
	    while (n--) {
	      *trgt.f = (float) src.cf->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_DOUBLE:
	    while (n--) {
	      *trgt.d = (double) src.cf->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_CDOUBLE:
	    while (n--) {
	      trgt.cd->imaginary = src.cf->imaginary;
	      trgt.cd->real = src.cf->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_STRING_ARRAY:
	    while (n--) {
	      Sprintf(curScrat, fmt_complex, (double) src.cf->real,
		      (double) src.cf->imaginary);
	      temp = strlen(curScrat) + 1;
	      *trgt.sp = (char*) malloc(temp);
	      if (!*trgt.sp)
		return cerror(ALLOC_ERR, 0);
	      memcpy(*trgt.sp, curScrat, temp);
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	}
	break;
      case LUX_CDOUBLE:
	switch (totype) {
	  case LUX_INT8:
	    while (n--) {
	      *trgt.b = (uint8_t) src.cd->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_INT16:
	    while (n--) {
	      *trgt.w = (int16_t) src.cd->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_INT32:
	    while (n--) {
	      *trgt.l = (int32_t) src.cd->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_INT64:
	    while (n--) {
	      *trgt.q = src.cd->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_FLOAT:
	    while (n--) {
	      *trgt.f = (float) src.cd->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_DOUBLE:
	    while (n--) {
	      *trgt.d = (double) src.cd->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_CFLOAT:
	    while (n--) {
	      trgt.cf->imaginary = src.cd->imaginary;
	      trgt.cf->real = src.cd->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_STRING_ARRAY:
	    while (n--) {
	      Sprintf(curScrat, fmt_complex, (double) src.cd->real,
		      (double) src.cd->imaginary);
	      temp = strlen(curScrat) + 1;
	      *trgt.sp = (char*) malloc(temp);
	      if (!*trgt.sp)
		return cerror(ALLOC_ERR, 0);
	      memcpy(*trgt.sp, curScrat, temp);
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	}
	break;
      case LUX_STRING_ARRAY:	/* from string array */
	switch (totype) {
	  case LUX_INT8:
	    while (n--) {
	      value.b = atol(*src.sp);
	      if (iq == result && *src.sp)
		free(*src.sp);
	      *trgt.b = value.b;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_INT16:
	    while (n--) {
	      value.w = atol(*src.sp);
	      if (iq == result && *src.sp)
		free(*src.sp);
	      *trgt.w = value.w;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_INT32:
	    while (n--) {
	      value.l = atol(*src.sp);
	      if (iq == result && *src.sp)
		free(*src.sp);
	      *trgt.l = value.l;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_INT64:
	    while (n--) {
	      value.q = atoll(*src.sp);
	      if (iq == result && *src.sp)
		free(*src.sp);
	      *trgt.q = value.q;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_FLOAT:
	    while (n--) {
	      value.f = atof(*src.sp);
	      if (iq == result && *src.sp)
		free(*src.sp);
	      *trgt.f = value.f;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_DOUBLE:
	    while (n--) {
	      value.d = atof(*src.sp);
	      if (iq == result && *src.sp)
		free(*src.sp);
	      *trgt.d = value.d;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_CFLOAT:
	    while (n--) {
              Symboltype thistype;
	      read_a_number(src.sp, &value, &thistype);
	      if (iq == result && *src.sp)
		free(*src.sp);
              if (isIntegerType(thistype)) {
                trgt.cf->real = value.q;
              } else {
                trgt.cf->real = value.d;
	      }
	      trgt.cf->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case LUX_CDOUBLE:
	    while (n--) {
              Symboltype thistype;
	      read_a_number(src.sp, &value, &thistype);
	      if (iq == result && *src.sp)
		free(*src.sp);
              if (isIntegerType(thistype)) {
                trgt.cd->real = value.q;
              } else {
                trgt.cd->real = value.d;
              }
	      trgt.cd->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	}
	break;
    }

    symbol_type(result) = totype;

    switch (do_realloc) {
      case 1:
	array_header(result) = (array*) realloc(array_header(result), size);
	if (!array_header(result))
	  return cerror(ALLOC_ERR, 0);
	symbol_memory(result) = size;
	break;
      case 2:
	complex_scalar_data(result).cf = (floatComplex*)
	  realloc(complex_scalar_data(result).cf, size);
	if (!complex_scalar_data(result).cf)
	  return cerror(ALLOC_ERR, 0);
	symbol_memory(result) = size;
	break;
    }
  }
  return isFunc? result: LUX_OK;
}
/*-----------------------------------------------------*/
int32_t lux_byte_inplace(int32_t narg, int32_t ps[])
/* BYTE,x  converts <x> to LUX_INT8. */
{
  return lux_convert(narg, ps, LUX_INT8, 0);
}
/*-----------------------------------------------------*/
int32_t lux_word_inplace(int32_t narg, int32_t ps[])
/* WORD,x  converts <x> to LUX_INT16. */
{
  return lux_convert(narg, ps, LUX_INT16, 0);
}
/*-----------------------------------------------------*/
int32_t lux_long_inplace(int32_t narg, int32_t ps[])
/* LONG,x  converts <x> to LUX_INT32. */
{
  return lux_convert(narg, ps, LUX_INT32, 0);
}
/*-----------------------------------------------------*/
int32_t lux_int64_inplace(int32_t narg, int32_t ps[])
/* INT64,x  converts <x> to LUX_INT64. */
{
  return lux_convert(narg, ps, LUX_INT64, 0);
}
/*-----------------------------------------------------*/
int32_t lux_float_inplace(int32_t narg, int32_t ps[])
/* FLOAT,x  converts <x> to LUX_FLOAT. */
{
  return lux_convert(narg, ps, LUX_FLOAT, 0);
}
/*-----------------------------------------------------*/
int32_t lux_double_inplace(int32_t narg, int32_t ps[])
/* DOUBLE,x  converts <x> to LUX_DOUBLE. */
{
  return lux_convert(narg, ps, LUX_DOUBLE, 0);
}
/*-----------------------------------------------------*/
int32_t lux_cfloat_inplace(int32_t narg, int32_t ps[])
/* CFLOAT,x  converts <x> to LUX_CFLOAT. */
{
  return lux_convert(narg, ps, LUX_CFLOAT, 0);
}
/*-----------------------------------------------------*/
int32_t lux_cdouble_inplace(int32_t narg, int32_t ps[])
/* CDOUBLE,x  converts <x> to LUX_CDOUBLE. */
{
  return lux_convert(narg, ps, LUX_CDOUBLE, 0);
}
/*-----------------------------------------------------*/
int32_t lux_string_inplace(int32_t narg, int32_t ps[])
/* STRING,x  converts <x> into a string form. */
{
  return lux_convert(narg, ps, LUX_STRING_ARRAY, 0);
}
/*-----------------------------------------------------*/
int32_t get_dims(int32_t *num, int32_t *arg, int32_t *dims)
/* reads *num positive integers from subsequent arg[]s and puts them */
/* in dims[]; returns the actual number of dimensions that were read
 in *num. */
{
 int32_t	n, iq, size, *ptr, i;

 n = *num;
 while (n--) {
   iq = *arg++;
   if (symbol_class(iq) == LUX_ARRAY) {
     if (*num != 1)
       return cerror(ONLY_1_IF_ARR, iq);
     iq = lux_long(1, &iq);	/* ensure LONG */
     size = array_size(iq);
     if (size > MAX_DIMS)
       return cerror(N_DIMS_OVR, arg[-1]);
     ptr = (int32_t*) array_data(iq);
     *num = size;
     memcpy(dims, ptr, size*sizeof(int32_t));
     for (i = 0; i < size; i++)
       if (dims[i] <= 0)
	 return cerror(ILL_DIM, iq);
   } else if ((*dims++ = int_arg(iq)) <= 0)
     return cerror(ILL_DIM, iq);
 }
 return 1;
}
/*-----------------------------------------------------*/
int32_t create_sub_ptr(int32_t nsym, char *p, int32_t index)
/* creates a LUX_SCAL_PTR symbol pointing at the element with
  index <index> in the array <p> with the data type
  of symbol <nsym> */
{
 int32_t	iq;

 getFreeTempVariable(iq);
 symbol_class(iq) = LUX_SCAL_PTR;
 sym[iq].type = sym[nsym].type;
 sym[iq].line = curLineNumber;
 sym[iq].spec.general.ptr = p + index*lux_type_size[sym[nsym].type];
 return iq;
}
/*-----------------------------------------------------*/
int32_t redef_scalar(int32_t nsym, Symboltype ntype, void *val)
/* redefine symbol nsym to be a scalar of type ntype with value *val */
{
  wideScalar	*value;

  value = (wideScalar *) val;

  undefine(nsym);
  symbol_class(nsym) = LUX_SCALAR;
  symbol_type(nsym) = ntype;
  switch (ntype) {
    case LUX_INT8:
      scalar_value(nsym).b = value? value->b: 0;
      break;
    case LUX_INT16:
      scalar_value(nsym).w = value? value->w: 0;
      break;
    case LUX_INT32:
      scalar_value(nsym).l = value? value->l: 0;
      break;
    case LUX_INT64:
      scalar_value(nsym).q = value? value->q: 0;
      break;
    case LUX_FLOAT:
      scalar_value(nsym).f = value? value->f: 0.0;
      break;
    case LUX_DOUBLE:
      scalar_value(nsym).d = value? value->d: 0.0;
      break;
    case LUX_CFLOAT:
      complex_scalar_memory(nsym) = lux_type_size[LUX_CFLOAT];
      complex_scalar_data(nsym).cf = (floatComplex*) malloc(complex_scalar_memory(nsym));
      if (!complex_scalar_data(nsym).cf)
	return cerror(ALLOC_ERR, nsym);
      symbol_class(nsym) = LUX_CSCALAR;
      complex_scalar_data(nsym).cf->real = value? value->cf.real: 0.0;
      complex_scalar_data(nsym).cf->imaginary = value? value->cf.imaginary: 0.0;
      break;
    case LUX_CDOUBLE:
      complex_scalar_memory(nsym) = lux_type_size[LUX_CDOUBLE];
      complex_scalar_data(nsym).cd = (doubleComplex*) malloc(complex_scalar_memory(nsym));
      if (!complex_scalar_data(nsym).cd)
	return cerror(ALLOC_ERR, nsym);
      symbol_class(nsym) = LUX_CSCALAR;
      complex_scalar_data(nsym).cd->real = value? value->cd.real: 0.0;
      complex_scalar_data(nsym).cd->imaginary = value? value->cd.imaginary: 0.0;
      break;
  }
  return LUX_OK;
}
/*-----------------------------------------------------*/
int32_t redef_string(int32_t nsym, int32_t len)
/* redefine symbol nsym to be a string with length len (excluding \0) */
{
 undefine(nsym);
 symbol_class(nsym) = LUX_STRING;
 sym[nsym].type = LUX_TEMP_STRING;
 allocate(string_value(nsym), len + 1, char);
 sym[nsym].spec.array.bstore = len + 1;
 return 1;
}
/*-----------------------------------------------------*/
int32_t redef_array(int32_t nsym, Symboltype ntype, int32_t ndim, int32_t *dims)
/* redefines symbol nsym to be an array of the given type, number of
  dimensions, and dimensions; or a scalar if <ndim> == 0 */
{                                /*redefine a symbol i to an array */
  int32_t   mq, j;
  array	*h;
  pointer	p;

  if (ndim > MAX_DIMS)
    return cerror(N_DIMS_OVR, 0);
  if (ndim < 0)
    return cerror(ILL_DIM, 0);
  if (!ndim) {
    undefine(nsym);
    symbol_class(nsym) = LUX_SCALAR;
    scalar_type(nsym) = ntype;
    return 1;
  }
  mq = lux_type_size[ntype];
  for (j = 0; j < ndim; j++)
    mq *= dims[j];
  mq += sizeof(array); /*total memory required including header */
  /* before deleting, check the current size and use it if it matches,
  this avoids a lot of mallocing in loops */
  if (symbol_class(nsym) != LUX_ARRAY
      || mq != symbol_memory(nsym)) {
    undefine(nsym);
    symbol_memory(nsym) = mq;
    allocate(symbol_data(nsym), mq, char);
  }
  symbol_class(nsym) = LUX_ARRAY;
  array_type(nsym) = ntype;
  h = array_header(nsym);
  h->ndim = ndim;
  h->c1 = 0; h->c2 = 0; h->nfacts = 0;
  memcpy(h->dims, dims, ndim*sizeof(int32_t));
  h->facts = NULL;                      /* no known facts */
  if (ntype == LUX_STRING_ARRAY) {
    /* a string array: set all elements to NULL */
    mq = array_size(nsym);
    p.sp = (char**) array_data(nsym);
    while (mq--)
      *p.sp++ = NULL;
  }
  return 1;
}
/*-----------------------------------------------------*/
int32_t redef_array_extra_dims(int32_t tgt, int32_t src, Symboltype type, int32_t ndim, int32_t *dims)
{
  int32_t *srcdims, srcndim;
  int32_t tgtdims[MAX_DIMS];

  if (!symbolIsModifiable(tgt))
    return cerror(NEED_NAMED, tgt);
  if (!numerical(src, &srcdims, &srcndim, NULL, NULL))
    return cerror(NEED_NUM_ARR, src);
  if (symbolIsScalar(src))
    srcndim = 0;
  if (srcndim + ndim > MAX_DIMS)
    return cerror(N_DIMS_OVR, tgt);
  memcpy(tgtdims, dims, ndim*sizeof(*tgtdims));
  memcpy(tgtdims + ndim, srcdims, srcndim*sizeof(*tgtdims));
  ndim += srcndim;
  return redef_array(tgt, type, ndim, tgtdims);
}
/*-----------------------------------------------------*/
int32_t bytarr(int32_t narg, int32_t ps[])
/* create an array of I*1 elements */
{
 int32_t	dims[MAX_DIMS];

 if (get_dims(&narg, ps, dims) != 1)
   return LUX_ERROR;
 return array_scratch(LUX_INT8, narg, dims);
}
/*-----------------------------------------------------*/
int32_t intarr(int32_t narg, int32_t ps[])
/* create an array of I*2 elements */
{
 int32_t	dims[MAX_DIMS];

 if (get_dims(&narg, ps, dims) != 1)
   return LUX_ERROR;
 return array_scratch(LUX_INT16, narg, dims);
}
/*-----------------------------------------------------*/
int32_t lonarr(int32_t narg, int32_t ps[])
/* create an array of I*4 elements */
{
 int32_t	dims[MAX_DIMS];

 if (get_dims(&narg, ps, dims) != 1)
   return LUX_ERROR;
 return array_scratch(LUX_INT32, narg, dims);
}
/*-----------------------------------------------------*/
int32_t int64arr(int32_t narg, int32_t ps[])
/* create an array of 64-bit elements */
{
 int32_t	dims[MAX_DIMS];

 if (get_dims(&narg, ps, dims) != 1)
   return LUX_ERROR;
 return array_scratch(LUX_INT64, narg, dims);
}
/*-----------------------------------------------------*/
int32_t fltarr(int32_t narg, int32_t ps[])
/* create an array of F*4 elements */
{
 int32_t	dims[MAX_DIMS];

 if (get_dims(&narg, ps, dims) != 1)
   return LUX_ERROR;
 return array_scratch(LUX_FLOAT, narg, dims);
}
/*-----------------------------------------------------*/
int32_t dblarr(int32_t narg, int32_t ps[])
/* create an array of I*1 elements */
{
 int32_t	dims[MAX_DIMS];

 if (get_dims(&narg, ps, dims) != 1)
   return LUX_ERROR;
 return array_scratch(LUX_DOUBLE, narg, dims);
}
/*-----------------------------------------------------*/
int32_t cfltarr(int32_t narg, int32_t ps[])
/* create a CFLOAT array */
{
  int32_t	dims[MAX_DIMS];

  if (get_dims(&narg, ps, dims) != 1)
    return LUX_ERROR;
  return array_scratch(LUX_CFLOAT, narg, dims);
}
/*-----------------------------------------------------*/
int32_t cdblarr(int32_t narg, int32_t ps[])
/* create a CDOUBLE array */
{
  int32_t	dims[MAX_DIMS];

  if (get_dims(&narg, ps, dims) != 1)
    return LUX_ERROR;
  return array_scratch(LUX_CDOUBLE, narg, dims);
}
/*-----------------------------------------------------*/
int32_t strarr(int32_t narg, int32_t ps[])
/* create an array of NULL string pointers */
{
 int32_t	dims[MAX_DIMS], iq, size, n;
 char	**ptr;

 size = ps[0]? int_arg(ps[0]): 0;
 if (size < 0)
   return luxerror("Illegal negative string size", ps[0]);
 ps++;
 narg--;
 if (get_dims(&narg, ps, dims) != 1)
   return LUX_ERROR;
 iq = array_scratch(LUX_STRING_ARRAY, narg, dims);
 if (size) {			/* fill with specified size of whitespace */
   ptr = (char**) array_data(iq);
   n = array_size(iq);
   while (n--) {
     *ptr = (char*) malloc(size + 1);
     sprintf(*ptr, "%*s", size, " ");
     ptr++;
   }
 }
 return iq;
}
/*-----------------------------------------------------*/
int32_t show_routine(internalRoutine *table, int32_t tableLength, int32_t narg, int32_t ps[])
/* shows all routine names that contain a specified substring,
 or all of them */
{
 extern int32_t	uTermCol;
 int32_t	i, nOut = 0, nLine = uTermCol/16;
 char	*chars, *p;
 char	*name, **ptr;
 keyList	*keys;

 if (internalMode & 1)		/* /PARAMETERS */
 { if (symbol_class(*ps) != LUX_STRING)
     return cerror(NEED_STR, *ps);
   p = name = strsave(string_arg(*ps));
   while (*p)
   { *p = toupper(*p);
     p++; }
   i = findInternalName(name, table == subroutine? 1: 0);
   if (i < 0)
   { printf("Internal subroutine %s not found.\n", name);
     Free(name);
     return 1; }
   Free(name);
   printf("%s %s, (%1d:%1d) arguments\n",
	  table == subroutine? "subroutine": "function",
	  table[i].name, table[i].minArg, table[i].maxArg);
   keys = (keyList*) table[i].keys;
   if (keys)
   { printf("parameters: ");
     if (keys->suppressEval)
       putchar('+');
     if (keys->defaultMode)
       printf("|%1d|", keys->defaultMode);
     if (keys->offset)
       printf("%%%1d%%", keys->offset);
     ptr = keys->keys;
     while (*ptr)
     { printf(":%s", *ptr);
       ptr++; }
     putchar('\n'); }
   return 1; }
 if (narg) chars = string_arg(*ps); else chars = NULL;
 if (chars)
 { chars = strsave(chars);
   for (p = chars; *p; p++) *p = toupper(*p); }
 for (i = 0; i < tableLength; i++)
 { if (!chars || strstr(table[i].name, chars))
   { printf("%-16s", table[i].name);
     if (++nOut % nLine == 0) putchar('\n'); }
 }
 if (nOut % nLine) putchar('\n');
 Free(chars);
 return 1;
}
/*-----------------------------------------------------*/
int32_t lux_show_subr(int32_t narg, int32_t ps[])
{
 return show_routine(subroutine, nSubroutine, narg, ps);
}
/*-----------------------------------------------------*/
int32_t lux_show_func(int32_t narg, int32_t ps[])
{
 return show_routine(function, nFunction, narg, ps);
}
/*-----------------------------------------------------*/
int32_t lux_switch(int32_t narg, int32_t ps[])
/* switches identity of two symbols.  We cannot just swap the names
   because then the connection between a particular name and a
   particular symbol number is broken.  We must swap the values instead.
   LS 9nov98 9oct2010*/
{
 int32_t	sym1, sym2;
 symTableEntry	temp1, temp2;

 sym1 = ps[0];
 sym2 = ps[1];
 temp1 = sym[sym1];
 temp2 = sym[sym2];
 sym[sym2] = temp1;
 sym[sym1] = temp2;
 sym[sym1].xx = temp1.xx;
 sym[sym2].xx = temp2.xx;
 sym[sym1].line = temp1.line;
 sym[sym2].line = temp2.line;
 sym[sym1].context = temp1.context;
 sym[sym2].context = temp2.context;
 sym[sym1].exec = temp1.exec;
 sym[sym2].exec = temp2.exec;

#ifdef DONTIGNORE
 temp = sym[sym2].xx;		/* hash value for the name */
 tempcontext = sym[sym2].context; /* the context */
 tempexec = sym[sym2].exec;	/* the execution count */
 templine = sym[sym2].line;	/* the line number */
 tempSymbol = sym[sym2];	/* temporary save */
 sym[sym2] = sym[sym1];
 sym[sym2].xx = temp;
 sym[sym2].context = tempcontext;
 sym[sym2].line = templine;
 sym[sym2].exec = tempexec;
 temp = sym[sym1].xx;
 tempcontext = sym[sym1].context;
 templine = sym[sym1].line;
 tempexec = sym[sym1].exec;
 sym[sym1] = tempSymbol;
 sym[sym1].xx = temp;
 sym[sym1].context = tempcontext;
 sym[sym1].line = templine;
 sym[sym1].exec = tempexec;
#endif
 return 1;
}
/*-----------------------------------------------------*/
int32_t lux_array(int32_t narg, int32_t ps[])
/* create an array of the specified type and dimensions */
{
  int32_t	dims[MAX_DIMS];
  Symboltype type;

 narg--;
 type = (Symboltype) int_arg(*ps);
 if (!isLegalType(type))
   return cerror(ILL_TYPE, 0, type);
 if (isStringType(type))
   type = LUX_STRING_ARRAY;
 if (get_dims(&narg, ps + 1, dims) != 1)
   return LUX_ERROR;
 return array_scratch(type, narg, dims);
}
/*-----------------------------------------------------*/
int32_t lux_assoc(int32_t narg, int32_t ps[])
/* returns an associated variable.
  syntax: assoc(lun, array [, offset]) */
{
 int32_t	lun, result, iq, size;
 array	*h;

 lun = int_arg(*ps);
 iq = ps[1];
 CK_ARR(iq, 2);
 getFreeTempVariable(result);
 symbol_class(result) = LUX_ASSOC;
 sym[result].type = sym[iq].type;
 sym[result].line = curLineNumber;
 h = HEAD(iq);
 size = sizeof(array);
 if (narg >= 3) size += sizeof(int32_t);
 if (!(sym[result].spec.array.ptr = (array *) Malloc(size)))
   return cerror(ALLOC_ERR, iq);
 sym[result].spec.array.bstore = size;
 memcpy(sym[result].spec.array.ptr, h, sizeof(array));
 h = HEAD(result);
 h->c1 = lun;
 if (size != sizeof(array))
   set_assoc_has_offset(result);
 h->nfacts = 0;
 h->facts = NULL;
 if (assoc_has_offset(result))
   assoc_offset(result) = int_arg(ps[2]);
 return result;
}
/*-----------------------------------------------------*/
int32_t lux_rfix(int32_t narg, int32_t ps[])
/* returns an I*4 version of the argument, rounded to the nearest
 integer if necessary */
{
 int32_t	nsym, type, size, *trgt, i, result;
 pointer	src;
 double	temp;
 array	*h;

 nsym = *ps;
 if ((type = sym[nsym].type) == LUX_INT32) return nsym; /* already of proper type */
 switch (symbol_class(nsym))
 { case LUX_SCALAR:
     src.l = &sym[nsym].spec.scalar.l;  size = 1;
     result = scalar_scratch(LUX_INT32);  trgt = &sym[result].spec.scalar.l;  break;
   case LUX_STRING:
     result = scalar_scratch(LUX_INT32);  type = LUX_DOUBLE;
     temp = atof((char *) sym[nsym].spec.array.ptr);  src.d = &temp;
     size = 1;  trgt = &sym[result].spec.scalar.l;  break;
   case LUX_ARRAY:
     result = array_clone(nsym, LUX_INT32);  h = HEAD(nsym);
     GET_SIZE(size, h->dims, h->ndim);  src.l = LPTR(h);
     trgt = LPTR(HEAD(result));  break;
   default: return cerror(ILL_CLASS, nsym); }
 		/* now convert */
 switch (type)
 { case LUX_INT8:
     while (size--) *trgt++ = (int32_t) *src.b++;  break;
   case LUX_INT16:
     while (size--) *trgt++ = (int32_t) *src.w++;  break;
   case LUX_INT64:
     while (size--) *trgt++ = (int32_t) *src.q++;  break;
   case LUX_FLOAT:
     while (size--)
     { *trgt++ = (int32_t) (*src.f + ((*src.f >= 0)? 0.5: -0.5));
       src.f++; }
     break;
   case LUX_DOUBLE:
     while (size--)
     {*trgt++ = (int32_t) (*src.d + ((*src.d >= 0)? 0.5: -0.5));
      src.d++; }
     break; }
 return result;
}
/*-----------------------------------------------------*/
int32_t lux_echo(int32_t narg, int32_t ps[])
/* turn on echoing of input lines from non-keyboard sources */
{
 extern int32_t	echo;

 if (narg >= 1) echo = int_arg(*ps); else echo = 1;
 return 1;
}
/*-----------------------------------------------------*/
int32_t lux_noecho(int32_t narg, int32_t ps[])
/* turn on echoing of input lines from non-keyboard sources */
{
 extern int32_t	echo;

 echo = 0;
 return 1;
}
/*-----------------------------------------------------*/
int32_t lux_batch(int32_t narg, int32_t ps[])
/* turn on/off batch mode */
{
 extern char	batch;

 batch = (narg >= 1)? (int_arg(*ps)? 0: 1): 1;
 printf("%s batch mode.\n", batch? "Entering": "Leaving");
 return 1;
}
/*-----------------------------------------------------*/
FILE	*recordFile = NULL;
extern char	recording;
int32_t lux_record(int32_t narg, int32_t ps[])
 /* start/stop recording. */
 /* Syntax:  RECORD [,file] [,/RESET,/INPUT,/OUTPUT] */
{
  char	*file = NULL, mode, reset;

  if (narg)
    file = string_arg(*ps);
  mode = internalMode & 3;
  if (!mode)
    mode = 3;
  reset = internalMode & 4;
  if (reset) {			/* stop recording some or all */
    if (recordFile && recording == mode) {
      fclose(recordFile);
      recordFile = NULL;
    }
    recording &= ~mode;
    switch (recording) {
      case 0:
	puts("Stopped recording terminal I/O");
	break;
      case 1:
	puts("Now only recording terminal input");
	break;
      case 2:
	puts("Now only recording terminal output");
	break;
    }
  } else {			/* start recording */
    if (file) {			/* a file is specified */
      if (recordFile)		/* already recording something */
	fclose(recordFile);
      recordFile = fopen(expand_name(file, ".lux"), "a");  /* open new file */
    } else {
      if (!recordFile)
	recordFile = fopen("record.lux", "a");
    }
    recording |= mode;
    switch (recording) {
      case 1:
	puts("Recording terminal input");
	break;
      case 2:
	puts("Recording terminal output");
	break;
      case 3:
	puts("Recording terminal I/O");
    }
  }
  return 1;
}
/*-------------------------------------------------------------------------*/
int32_t step = 0;
int32_t lux_step(int32_t narg, int32_t ps[])
{
  int32_t	i = 1;

  if (narg) i = int_arg(*ps);
  if (i) { step = i;  printf("Commence stepping at level %d\n", step); }
  else { step = 0;  puts("No stepping"); }
  return 1;
}
/*-------------------------------------------------------------------------*/
int32_t lux_varname(int32_t narg, int32_t ps[])
/* returns the name of the variable */
{
  char const* name;
  int32_t	iq;

  name = varName(*ps);
  iq = string_scratch(strlen(name));
  strcpy((char *) sym[iq].spec.array.ptr, name);
  return iq;
}
/*-------------------------------------------------------------------------*/
int32_t namevar(int32_t symbol, int32_t safe)
/* if safe == 0: returns the variable that goes with the name string */
/* in the current context, or an error if not found. */
/* if safe == 1: as for safe == 0, but if not found and if at main level */
/* then such a variable is created at the main level. */
/* if safe == 2: as for safe == 0, but search main level only. */
/* if safe == 3: as for safe == 1, but search and create at main level. */
{
  char	*name;
  int32_t	iq, context;

  if (symbol_class(symbol) != LUX_STRING)
    return cerror(NEED_STR, symbol);
  name = string_value(symbol);
  strcpy(line, name);
  if (!isalpha((uint8_t) *name) && *name != '$' && *name != '!')
    return luxerror("Illegal symbol name: %s", symbol, name);
  for (name = line; *name; name++)
  { if (!isalnum((uint8_t) *name))
    return luxerror("Illegal symbol name: %s", symbol, name);
    *name = toupper(*name); }
  safe = safe & 3;
  context = (safe >= 2)? 0: curContext;
  iq = lookForVarName(line, context); /* seek symbol */
  if (iq < 0)
  { if ((safe & 1) == 0 || ((safe & 1) == 1 && context))
      return luxerror("Could not find variable %s %s %s\n", 0, line,
                   context? "in": "at",
		   context? varName(context): "main level");
    if ((safe & 1) == 1)	/* create at main level */
    { iq = installString(line);
      iq = findVar(iq, 0); }
  }
  return iq;
}
/*-------------------------------------------------------------------------*/
char const* keyName(internalRoutine *routine, int32_t number, int32_t index)
/* returns the name of the <index>th positional keyword of <routine> */
/* #<number>.  LS 19jan95 */
{
  keyList	*list;
  char		**keys;

  if (index < 0 || index >= routine[number].maxArg)
    return "(illegal)";
  list = (keyList *) routine[number].keys;
  keys = list->keys;
  if (index < 0) return "(unnamed)";	/* before first named key */
  while (*keys && index)
  { if (!isdigit((int32_t) **keys)) index--;
    keys++; }
  if (index) return "(unnamed)";	/* beyond last key */
  return *keys;
}
/*-------------------------------------------------------------------------*/
void checkErrno(void)
/* checks if errno has been set; displays appropriate message if so; */
/* resets errno */
{
  if (errno)
  { puts(symbolIdent(curSymbol, 1));
    perror("Mathematics error");
    errno = 0; }
}
/*-------------------------------------------------------------------------*/
char	allowPromptInInput = 1; /* default */
int32_t lux_set(int32_t narg, int32_t ps[])
/* SET[,/SHOWALLOC,/WHITEBACKGROUND,/INVIMCOORDS,/SET,/RESET,/ZOOM]
 governs aspects of the behaviour of various routines.  LS 11mar98 */
{
  extern int32_t	setup;
#if HAVE_LIBX11
  char	*string;
  char const* visualNames[] = { "StaticGray", "StaticColor", "TrueColor",
			   "GrayScale", "PseudoColor", "DirectColor",
			   "SG", "SC", "TC", "GS", "PC", "DC",
			   "GSL", "CSL", "CSI", "GDL", "CDL", "CDI" };
  int32_t	visualClassCode[] = { StaticGray, StaticColor, TrueColor,
			      GrayScale, PseudoColor, DirectColor };
  int32_t	setup_x_visual(int32_t), i;
  extern int32_t	connect_flag;
  extern Visual	*visual;
#endif

  if (narg) {
    if (*ps) {			/* VISUAL */
#if HAVE_LIBX11
      if (!symbolIsStringScalar(*ps))
	return cerror(NEED_STR, *ps);
      string = string_value(*ps);
      for (i = 0; i < 18; i++)
	if (!strcasecmp(string, visualNames[i])) /* found it */
	  break;
      if (i == 18) {		/* wasn't in the list */
	printf("Unknown visual name, \"%s\".\nSelect from:\n", string);
	for (i = 0; i < 12; i++)
	  printf("%s ", visualNames[i]);
	putchar('\n');
	return luxerror("Invalid visual", *ps);
      }
      i = visualClassCode[i % 6];
      if (connect_flag) {
	if (i == visualclass(visual))
	  return LUX_OK;	/* already use the selected visual class */
	else
	  return luxerror("Already using a %s visual.", 0,
                          visualNames[visualclass(visual)]);
      }
      if (setup_x_visual(i) == LUX_ERROR)
	return LUX_ERROR;
#else
      return luxerror("Need X11 package to set the visual", 0);
#endif
    }
  }
  if (internalMode & 1)		/* /SET: copy exactly */
    setup = internalMode >> 2;
  else if (internalMode & 2)	/* /RESET: remove selection */
    setup = (setup & ~(internalMode >> 2));
  else if (internalMode)	/* add selection */
    setup = setup | (internalMode >> 2);
  else {			/* show selection */
    if (setup) {
      puts("SETUP selections:");
      if (setup & 1)
	puts("/SHOWALLOC: Display information when allocating or freeing memory.");
      if (setup & 2)
	puts("/WHITEBACKGROUND: White default window background.");
      if (setup & 4)
	puts("/ULIMCOORDS: Image position origin in upper left-hand corner.");
      if (setup & 8)
	puts("/YREVERSEIMG: Image origin in upper left-hand corner.");
      if (setup & 16)
	puts("/OLDVERSION: Backward compatibility mode.");
#if HAVE_LIBX11
      if (setup & 32)
	puts("/ZOOM: Automatic image zoom.");
#endif
      if (setup & 256)
	puts("/ALLOWPROMPTS: Allow LUX prompts at the beginning of input lines.");
      if (setup & 512)
	puts("/XSYNCHRONIZE: Graphics output occurs immediately.");
      if (setup & 1024)
	puts("/PARSEWARN: Parser warnings.");
    } else if (!narg)
      puts("No SETUP selections.");
  }
  allowPromptInInput = (setup & 256)? 1: 0;
#if HAVE_LIBX11
  if (internalMode & 2048)
    xsynchronize(setup & 512? 1: 0);
#endif
  return LUX_ONE;
}
/*-------------------------------------------------------------------------*/
void zapTemp(int32_t symbol)
/* zaps <symbol> only if it is a temporary variable */
/* and if its context is equal to -compileLevel */
{
  if (symbol_context(symbol) == -compileLevel
      && ((symbol >= TEMPS_START && symbol < TEMPS_END)
	  || (symbol >= TEMP_EXE_START && symbol < TEMP_EXE_END)))
  { zap(symbol);
    updateIndices(); }
}
/*-------------------------------------------------------------------------*/
int32_t copyEvalSym(int32_t source)
  /* evaluates <source> and returns it in a temp */
{
  int32_t	result, target;

  result = eval(source);
  if (result < 0) return -1;	/* some error */
  if (result != source) zapTemp(source);
  if (isFreeTemp(result)) return result;
  target = nextFreeTempVariable();
  symbol_class(target) = LUX_UNDEFINED; /* else may get trouble when */
				  /* updateIndices() is called in */
				  /* lux_replace() */
  if (target < 0) return -1;	/* some error */
  if (lux_replace(target, result) < 0)
    return -1;			/* some error */
  return target;
}
/*-------------------------------------------------------------------------*/
int32_t (*lux_converts[])(int32_t, int32_t []) = {
  lux_byte, lux_word, lux_long, lux_int64, lux_float, lux_double, lux_string,
  lux_string, lux_string, lux_cfloat, lux_cdouble
};
int32_t getNumerical(int32_t iq, Symboltype minType, int32_t *n, pointer *src, char mode,
		 int32_t *result, pointer *trgt)
/* gets pointer to and size of the data in numerical argument <iq>.
   returns pointer in <*src>, number of elements in <*n>.

   If <trgt> is non-NULL, then a garbage-filled clone of <iq> is
   generated.  Its symbol number is returned in <result> and a pointer
   to its data in <trgt>.  Its data type is determined by <minType>
   and <mode>: If <mode> & GN_EXACT is non-zero, then its data type
   will be exactly equal to <minType>; otherwise if <mode> &
   GN_UPGRADE is non-zero and the data type of <iq> is less than
   <minType>, then its data type will be equal to <minType>; or
   otherwise it will be equal to the data type of <iq>.

   If <mode> & GN_UPDATE is non-zero, and if the data type of <iq> is
   not equal to that determined by <minType>, GN_EXACT, and
   GN_UPGRADE, then a copy of <iq> is generated that does have that
   data type, and a pointer to its data (rather than <iq>'s data) is
   returned in <src>.  If in addition <trgt> is equal to NULL and
   <result> unequal to NULL, then the symbol number of the updated
   copy of <iq> (or <iq> itself if no updating was required) is
   returned in <*result>.

   LS 19nov98 */
{
  Symboltype	type;

  if (!symbolIsNumerical(iq))
    return cerror(ILL_CLASS, iq);

  if (symbol_class(iq) == LUX_SCAL_PTR)
    iq = dereferenceScalPointer(iq);

  type = symbol_type(iq);
  if (((mode & GN_UPGRADE) && type < minType)
      || ((mode & GN_EXACT) && type != minType)) {
    if (mode & GN_UPDATE)
      iq = lux_converts[minType](1, &iq);
    type = minType;
  }

  switch (symbol_class(iq)) {
    case LUX_SCALAR:
      (*src).b = &scalar_value(iq).b;
      *n = 1;
      if (trgt) {
	*result = scalar_scratch(type);
	(*trgt).b = &scalar_value(*result).b;
      } else if (result)
	*result = iq;
      break;
    case LUX_ARRAY: case LUX_CARRAY:
      (*src).b = (uint8_t *) array_data(iq);
      *n = array_size(iq);
      if (trgt) {
	*result = array_clone(iq, type);
	(*trgt).b = (uint8_t *) array_data(*result);
      } else if (result)
	*result = iq;
      break;
    case LUX_CSCALAR:
      (*src).cf = complex_scalar_data(iq).cf;
      *n = 1;
      if (trgt) {
	*result = scalar_scratch(type);
	(*trgt).cf = complex_scalar_data(*result).cf;
      } else if (result)
	*result = iq;
      break;
    default:
      return cerror(ILL_CLASS, iq);
  }
  return 1;
}  
/*-------------------------------------------------------------------------*/
int32_t getSimpleNumerical(int32_t iq, pointer *data, int32_t *nelem)
/* returns pointer in <*data> to data in symbol <iq>, and in <*nelem> the
   number of data elements, and 1 as function return value, if <iq>
   is of numerical type.  Otherwise, returns -1 as function value, 0 in
   <*nelem>, and nothing in <*data>.  LS 14apr97 */
{
  switch (symbol_class(iq)) {
    case LUX_SCAL_PTR:
      iq = dereferenceScalPointer(iq);
    case LUX_SCALAR:
      (*data).b = &scalar_value(iq).b;
      *nelem = 1;
      break;
    case LUX_ARRAY:
      (*data).l = (int32_t*) array_data(iq);
      *nelem = array_size(iq);
      break;
    default:
      *nelem = 0;
      return cerror(ILL_CLASS, iq);
  }
  return 1;
}
/*-------------------------------------------------------------------------*/
int32_t file_map_size(int32_t symbol)
{
 int32_t	i, n, *p, size;

  p = file_map_dims(symbol);
  n = file_map_num_dims(symbol);
  size = 1;
  for (i = 0; i < n; i++)
    size *= *p++;
  return size;
}
/*-------------------------------------------------------------------------*/
int32_t lux_pointer(int32_t narg, int32_t ps[])
/* POINTER,pointer,target [,/FUNCTION, /SUBROUTINE, /INTERNAL, /MAIN] */
/* makes named variable */
/* <pointer> point at named variable <target>.  If <pointer> is a pointer */
/* already, then its old link is broken before the new one is put in */
/* place.  <target> may be an expression as long as it evaluates to a */
/* named variable.  LS 11feb97 */
{
  int32_t	iq;
  char	*name, *name2, *p;
  
  if (ps[0] >= TEMPS_START)
    return luxerror("Intended pointer is not a named variable", ps[0]);
  if (symbol_class(ps[0]) != LUX_POINTER)
  { undefine(ps[0]);
    symbol_class(ps[0]) = LUX_POINTER;
    transfer_is_parameter(ps[0]) = (Symboltype) 0;
    transfer_temp_param(ps[0]) = 0; }
  iq = eval(ps[1]);
  if (internalMode)		/* target must be a string variable */
  { if (symbol_class(iq) != LUX_STRING)
      return cerror(NEED_STR, ps[1]);
    name = string_arg(iq);
    p = name2 = strsave(name);
    while (*p)
    { *p = toupper(*p);		/* uppercase variable name */
      p++; }
  }
  switch (internalMode & 11)
  { case 8:
      iq = findName(name2, varHashTable,
		    (internalMode & 8)? 0: curContext);
      Free(name2);
      if (iq < 0)
	return LUX_ERROR;
    case 0:			/* variable */
      if (iq >= TEMPS_START)
	return luxerror("Expression does not reduce to a named variable", ps[1]);
      transfer_target(ps[0]) = iq;
      break;
    case 1:			/* function */
    case 2:			/* subroutine */
      if (internalMode & 4)	/* /INTERNAL */
      { iq = findInternalName(name2, (internalMode & 1)? 0: 1);
	Free(name2);
	if (iq < 0)
	  return LUX_ERROR;
	symbol_class(ps[0]) = LUX_FUNC_PTR;
	func_ptr_routine_num(ps[0]) = -iq;
	func_ptr_type(ps[0]) = (Symboltype) ((internalMode & 1)? LUX_FUNCTION:
                                             LUX_SUBROUTINE);
      }
      else
      { iq = findName(name2, internalMode == 1? funcHashTable: subrHashTable,
		      0);
	Free(name2);
	if (iq < 0)		/* some error occurred */
	  return iq;
	symbol_class(ps[0]) = LUX_FUNC_PTR;
	symbol_class(iq) =
	  (internalMode == 1)? LUX_DEFERRED_FUNC: LUX_DEFERRED_SUBR;
	deferred_routine_filename(iq) = strsave(name);
	if (iq >= TEMPS_START)
	  return luxerror("Pointer target is not a named variable", ps[1]);
	transfer_target(ps[0]) = iq; }
      break;
    default:
      return cerror(INCMP_KEYWORD, 0); }
  return 1;
}
/*-------------------------------------------------------------------------*/
int32_t lux_symbol(int32_t narg, int32_t ps[])
/* SYMBOL('name') returns the variable with the <name> in the current */
/* context, or if not found and if at the main level, then creates */
/* such a variable and returns it.  SYMBOL('name',/MAIN) checks */
/* the main execution level instead.  LS 11feb97 */
{
  return namevar(ps[0], (internalMode & 1) == 1? 3: 1);
}
/*-------------------------------------------------------------------------*/
int32_t stringpointer(char *name, int32_t type)
/* seeks <name> as user-defined function (SP_USER_FUNC), user-defined */
/* subroutine (SP_USER_SUBR), variable (SP_VAR), internal function */
/* (SP_INT_FUNC), or internal subroutine (SP_INT_SUBR).  Also: */
/* any function (SP_FUNC = SP_USER_FUNC then SP_INT_FUNC), any subroutine */
/* (SP_SUBR = SP_USER_SUBR then SP_INT_SUBR), and any at all (SP_ANY). */
/* LS 1apr97 */
{
  int32_t	n;
  char	*p;

  allocate(p, strlen(name) + 1, char);
  strcpy(p, name);
  name = p;
  for (; *p; p++)
    *p = toupper(*p);		/* make sure it is all upper case */
  if (type & SP_VAR)
  { n = lookForName(name, varHashTable, curContext);
    if (n >= 0)
    { type = SP_VAR;
      Free(name);
      return n; }
  }
  if (type & SP_USER_FUNC)
  { n = lookForName(name, funcHashTable, 0);
    if (n >= 0)
    { type = SP_USER_FUNC;
      Free(name);
      return n; }
  }
  if (type & SP_INT_FUNC)
  { n = findInternalName(name, 0);
    if (n >= 0)
    { type = SP_INT_FUNC;
      Free(name);
      return n; }
  }
  if (type & SP_USER_SUBR)
  { n = lookForName(name, subrHashTable, 0);
    if (n >= 0)
    { type = SP_USER_SUBR;
      Free(name);
      return n; }
  }
  if (type & SP_INT_SUBR)
  { n = findInternalName(name, 1);
    if (n >= 0)
    { type = SP_INT_SUBR;
      Free(name);
      return n; }
  }
  Free(name);
  return -1;			/* not found */
}
/*-------------------------------------------------------------------------*/
int32_t lux_show_temps(int32_t narg, int32_t ps[])
/* a routine for debugging LUX.  It shows the temporary variables that */
/* are currently defined.  LS 2mar97 */
{
  int32_t	i;

  setPager(0);
  for (i = TEMPS_START; i < TEMPS_END; i++) {
    if (symbol_class(i) != LUX_UNUSED) {	/* defined */
      printf("%4d [%5d] ", i, symbol_context(i));
      symdumpswitch(i, I_TRUNCATE | I_LENGTH);
    }
  }
  resetPager();
  return 1;
}
/*-------------------------------------------------------------------------*/
int32_t routineContext(int32_t nsym)
/* returns the number of the user-defined subroutine or function that
   symbol <nsym> is in, or 0 if at the main level.  LS 18apr97 */
{
  while (symbol_context(nsym))
  { nsym = symbol_context(nsym);
    if (symbol_class(nsym) == LUX_SUBROUTINE
	|| symbol_class(nsym) == LUX_FUNCTION)
      return nsym; }
  return 0;
}
/*-------------------------------------------------------------------------*/
/* NUMBERS

   LUX interprets strings as explicit numbers as indicated below.
   Square brackets [] indicate optional parts; curly braces {}
   indicate a set of choices from which one must be chosen; a *
   indicates that zero or more (not necessarily equal) copies of the
   preceding item must be chosen; ooo stands for a set of octal
   digits, ddd for a set of decimal digits, hhh for a set of
   hexadecimal digits; literal characters are written in uppercase but
   are case-insensitive on input.

   oooO[{BWLI}]                       octal integer
   ddd[{BWLI}]                        decimal integer
   0Xhhh[{BWLI}]                      hexadecimal integer
   ddd{SH}[ddd:]*[ddd][.ddd][{DE}][I] sexagesimal number
   ddd.[ddd][I]                       floating-point number
   .ddd[{DE}[[{+-}]ddd]][I]           floating-point number
   ddd[.ddd]{DE}[[{+-}]ddd][I]        floating-point number

   B -> BYTE
   W -> WORD
   L -> LONG
   E -> FLOAT
   D -> DOUBLE
   S -> sexagesimal
   H -> sexagesimal with transformation from hours to degrees
   I -> imaginary

   In addition, any number may be preceded by an arbitrary amount of
   whitespace and a plus or minus sign.

   NOTE: we used to allow hexadecimal input hhhX, but this requires
   more than a single character of look-ahead, and we cannot deal with
   that when reading from a file.  We used to define sexagesimal
   numbers by adding an S or T at the very end, but this leads to similar
   look-ahead problems; we now demand S or H (for "hours") after the first
   digits.
 */
/*-------------------------------------------------------------------------*/
#define isodigit(x) (isdigit(x) && x < '8')

void read_a_number(char **buf, Scalar *value, Symboltype *type)
/* reads the number at <*buf>, puts it in <*value> and its data type
   in <*type> (if <type> is not NULL), and modifies <*buf> to point
   just after the detected value.  NOTE: if the type is integer (BYTE,
   WORD, LONG, INT64), then the value is returned in value->q; if the
   type is floating-point (FLOAT, DOUBLE), then the value is returned
   in value->d.  if an I follows the number, then the type is complex
   (CFLOAT, CDOUBLE) and the value (transformed to real) is returned
   in value.d.  So, the union member that the value is in does not
   necessarily correspond exactly to the <*type>: e.g., a BYTE number
   gets its value returned in value->q and *type set to LUX_INT8.  LS
   17sep98 */
{
  int32_t	base = 10, kind, sign;
  char	*p, *numstart, c, ce, *p2;
  Symboltype thetype;

  p = *buf;
  thetype = LUX_INT32;		/* default */
  /* skip whitespace */
  while (!isdigit((int32_t) *p) && !strchr("+-.", (int32_t) *p))
    p++;

  /* skip sign, if any */
  if (strchr("+-", *p)) {
    sign = (*p == '-')? -1: 1;
    p++;
  } else sign = 1;
  numstart = p;

  /* |oooO[{BWLQI}] */
  /* |ddd[{BWLQI}] */
  /* |0X[hhh][{WLQI}] */
  /* |ddd{SH}[ddd:]*[ddd][.ddd][{DE}][I] */
  /* |ddd.[ddd][I] */
  /* |.ddd[{DE}[[{+-}]ddd]][I] */
  /* |ddd[.ddd][{DE}[[{+-}]ddd]][I] */
  /* ^ we are here */
  /* NOTE: vertical bars | are indicated just before the last read */
  /* character; <c> contains the last read character, which must match */
  /* one of the characters just after a vertical bar. */

  while (isodigit((int32_t) *p))
    p++;

  /* ooo|O[{BWLQI}] */
  /*  dd|d[{BWLQI}] */
  /* ddd|{BWLQI} */
  /* ddd| */
  /*   0|X[hhh][{BWLQI}] */
  /*  dd|d{ST}[ddd:]*[ddd][.ddd][{DE}][I] */
  /* ddd|{ST}[ddd:]*[ddd][.ddd][{DE}][I] */
  /*    |.ddd[{DE}[[{+-}]ddd]][I] */
  /*  dd|d[.ddd][{DE}[[{+-}]ddd]][I] */
  /* ddd|{DE}[[{+-}]ddd][I] */
  /* ddd|.[ddd][{DE}[[{+-}]ddd]][I] */
  /*    ^ we are here */

  if (*p == 'O') {		/* octal integer */
    base = 8;
    p++;
    /* oooO|[{BWLQI}]
	   ^ we are here */
  } else
    /* skip remaining digits, if any */
    while (isdigit((int32_t) *p))
      p++;

  /* oooO|{BWLQI} */
  /* oooO| */
  /*  ddd|{BWLQI} */
  /*  ddd| */
  /*    0|X[hhh][{BWLQI}] */
  /*  ddd|{ST}[ddd:]*[ddd][.ddd][{DE}][I] */
  /*  ddd|.[ddd][I] */
  /*     |.ddd[{DE}[[{+-}]ddd]][I] */
  /*  ddd|{DE}[[{+-}]ddd][I] */
  /*     ^ we are here */

  switch (toupper(*p)) {
  case 'W': case 'L': case 'B': case 'Q': case 'I': /* done with the digits */
      /* ddd|{BWLQI} */
      /*    ^ we are here */
      break;
    case 'X':			/* hex number? */
      if (p == numstart + 1 && numstart[0] == '0') { /* yes, a hex number */
	/* 0|X[hhh][{WLQI}] */
	/*  ^ we are here */
	p++;
	while (isxdigit((int32_t) *p))
	  p++;
	base = 16;
	/*  0X[hhh]|[{WLQI}] */
	/*         ^ we are here */
      } /* else we're already at the end of the non-hex number */
      break;
    case 'S': case 'H':	/* sexagesimal numbers */
      /* ddd|{SH}[ddd:]*[ddd][.ddd][{DE}][I] */
      /*    ^ we are here */
      kind = *p;		/* S or H */
      *p = '\0';		/* temporarily terminate */
      value->d = atol(numstart); /* read the first number */
      *p = kind;		/* restore */
      base = 0;			/* count the number of elements */
      p++;			/* skip the S or H */
      numstart = p;
      /* ddd{SH}|[ddd:]*[ddd][.ddd][{DE}][I] */
      /*        ^ we are here */
      while (isdigit((int32_t) *p)) {
	while (isdigit((int32_t) *p))
	  p++;
	/* ddd{SH}ddd|:[ddd:]*[ddd][.ddd][{DE}][I] */
	/* ddd{SH}ddd|.[ddd][{DE}][I] */
	/* ddd{SH}ddd|{DE}[I] */
	/* ddd{SH}ddd|I */
	/*           ^ we are here */
	if (*p == '.') {	/* a float number: the last entry */
	  p++;
	  while (isdigit((int32_t) *p)) /* find the rest */
	    p++;
	  /* ddd{SH}[ddd:]*[ddd][.ddd]|[{DE}][I] */
	  /*                          ^ we are here */
	  c = *p;		/* save */
	  *p = '\0';		/* temporary termination */
	  value->d *= 60;
	  value->d += atof(numstart);
	  *p++ = c;		/* restore */
	  base++;
	  break;		/* we're done */
	} else {		/* we found another integer element */
	  c = *p;		/* save */
	  *p = '\0';		/* temporary termination */
	  value->d *= 60;
	  value->d += atol(numstart);
	  *p = c;		/* restore */
	  numstart = ++p;
	  base++;
	}
      }
      while (base--)		/* back to units */
	value->d /= 60;
      if (kind == 'H')
	value->d *= 15;	/* from hours to degrees */
      kind = toupper(*p);
      if (kind == 'D' || kind == 'E') {
	c = *++p;		/* next one could be an I */
	if (toupper(c) == 'I') { /* imaginary */
	  thetype = (kind == 'D')? LUX_CDOUBLE: LUX_CFLOAT;
	  p++;			/* skip */
	} else
	  thetype = (kind == 'D')? LUX_DOUBLE: LUX_FLOAT;
      } else if (kind == 'I') {
	thetype = LUX_CFLOAT;
	p++;
      } else
	thetype = LUX_FLOAT;
      value->d *= sign;	/* put the sign back on it */
      if (type)
        *type = thetype;
      *buf = p;
      return;
    case '.': case 'D': case 'E':
      /*    |.ddd[{DE}[[{+-}]ddd]][I] */
      /* ddd|.[ddd][{DE}[[{+-}]ddd]][I] */
      /* ddd|{DE}[[{+-}]ddd][I] */
      /*    ^ we are here */
      if (*p == '.') {
	p++;			/* skip . */
	while (isdigit((int32_t) *p))
	  p++;
      }
      /* ddd.[ddd]|[I] */
      /* ddd[.ddd]|{DE}[[{+-}]ddd][I] */
      /*      .ddd|[I] */
      /*      .ddd|{DE}[[{+-}]ddd][I] */
      /*          ^ we are here */
      kind = toupper(*p);
      p2 = NULL;
      thetype = LUX_FLOAT;	/* default */
      if (kind == 'D' || kind == 'E') {
	if (kind == 'D') {
	  ce = *p;
	  *p = 'E';		/* temporarily, so atof can read it */
	  p2 = p;
	  thetype = LUX_DOUBLE;
	}
	p++;
	/* ddd.[ddd]{DE}|[[{+-}ddd][I] */
	/*      .ddd{DE}|[[{+-}ddd][I] */
	/*              ^ we are here */
	if (*p == '+' || *p == '-')
	  p++;
	/* ddd.[ddd]{DE}[[{+-}|ddd][I] */
	/*      .ddd{DE}[[{+-}|ddd][I] */
	/*                    ^ we are here */
	while (isdigit((int32_t) *p))
	  p++;
	kind = toupper(*p);
      }
      /* ddd.[ddd]{DE}[[{+-}ddd]|[I] */
      /*      .ddd{DE}[[{+-}ddd]|[I] */
      /*                        ^ we are here */
      c = *p;
      *p = '\0';		/* temporary end */
      value->d = atof(numstart)*sign;
      *p = c;			/* restore */
      if (p2)
	*p2 = ce;
      if (kind == 'I') {
	thetype = (thetype == LUX_DOUBLE)? LUX_CDOUBLE: LUX_CFLOAT;
	p++;			/* skip the I */
      }
      *buf = p;
      if (type)
        *type = thetype;
      return;
  }

  /*    oooO|[{BWLQI}] */
  /*     ddd|[{BWLQI}] */
  /* 0X[hhh]|[{BWLQI}] */
  /*        ^ we are here */
  kind = toupper(*p);
  c = *p;
  *p = '\0';			/* temporary end */
  /* NOTE: have to use strtoul() instead of strtol() because strtol() */
  /* -- at least on SGI Irix6.3 -- does not accept numbers with their most */
  /* significant Byte set (such as 0xc9460fc0), even though printf() */
  /* has no trouble generating such numbers.  I.e., t,'%x',-0x35b9f040 */
  /* yielded c9460fd0 but t,'%x',0xc9460fd0 when using strtol() yields */
  /* 0xffffffff.  LS 11nov99 */
  value->q = strtoull(numstart, NULL, base)*sign;
  *p = c;
  switch (kind) {
  case 'B':
    thetype = LUX_INT8;
    p++;
    break;
  case 'W':
    thetype = LUX_INT16;
    p++;
    break;
  case 'L':
    thetype = LUX_INT32;
    p++;
    break;
  case 'Q':
    thetype = LUX_INT64;
    p++;
    break;
  case 'I':
    value->d = value->q;
    thetype = LUX_CFLOAT;
    p++;
    break;
  default:
    thetype = LUX_INT32;		/* default */
    break;
  }
  if (type)
    *type = thetype;
  *buf = p;
}
/*-------------------------------------------------------------------------*/
void read_a_number_fp(FILE *fp, Scalar *value, Symboltype *type)
/* reads the number at <*fp>, puts it in <*value> and its data type in
   <*type> (if <type> is not NULL).  Other than the source of the
   data, exactly the same as read_a_number().  NOTE: if the type is
   integer (BYTE, WORD, LONG, QUAD), then the value is returned in
   value->q; if the type is floating-point (FLOAT, DOUBLE), then the
   value is returned in value->d.  if an I follows the number, then
   the type is complex (CFLOAT, CDOUBLE) and the value (transformed to
   real) is returned in value.d.  So, the union member that the value
   is in does not necessarily correspond exactly to the <*type>: e.g.,
   a BYTE number gets its value returned in value->l and *type set to
   LUX_INT8.  LS 17sep98 */
/* Fixed reading of numbers with exponents.  LS 11jul2000 */
{
  int32_t	base = 10, kind, sign, ch;
  char	*p, *numstart;
  Symboltype thetype;

  thetype = LUX_INT32;		/* default */
  /* skip non-digits, non-signs */
  while ((ch = nextchar(fp)) != EOF && !isdigit(ch) && !strchr("+-", ch));
  if (ch == EOF) {		/* end of file; return LONG 0 */
    value->q = 0;
    if (type)
      *type = thetype;
    return;
  }

  /* skip sign, if any */
  if (strchr("+-", ch)) {
    sign = (ch == '-')? -1: 1;
    ch = nextchar(fp);
    if (ch == EOF) {
      value->q = 0;
      if (type)
        *type = thetype;
      return;
    }
  } else sign = 1;

  p = numstart = curScrat;
  *p++ = ch;
  do
    *p++ = nextchar(fp);
  while (isodigit((int32_t) p[-1]));
  ch = p[-1];

  if (ch == EOF) {
    value->q = 0;
    if (type)
      *type = thetype;
    return;
  }
  if (ch == 'O') {		/* octal integer */
    base = 8;
    ch = nextchar(fp);
    if (ch == EOF) {
      value->q = 0;
      if (type)
        *type = thetype;
      return;
    }
  } else if (isdigit(ch)) {	/* not an octal integer */
    /* read remaining digits, if any */
    do
      *p++ = nextchar(fp);
    while (isdigit((int32_t) p[-1]));
    ch = p[-1];
  }

  switch (toupper(ch)) {
  case 'W': case 'L': case 'B': case 'Q': case 'I': /* done with the digits */
    break;
  case 'X':			/* hex number? */
    if (p == numstart + 2 && numstart[0] == '0') { /* yes, a hex number */
      do
        *p++ = nextchar(fp);
      while (isxdigit((int32_t) p[-1]));
      ch = p[-1];
      base = 16;
    } /* else we're already at the end of the non-hex number */
    break;
  case 'S': case 'H':	/* sexagesimal numbers */
    /* ddd|{SH}[ddd:]*[ddd][.ddd][{DE}][I] */
    /*    ^ we are here */
    kind = ch;		/* S or H */
    *p = '\0';		/* temporarily terminate */
    value->d = atol(numstart); /* read the first number */
    p = numstart;
    base = 0;			/* count the number of elements */
    ch = nextchar(fp);
    if (ch == EOF) {
      value->q = (intmax_t) value->d;
      if (type)
        *type = thetype;
      return;
    }
    while (isdigit(ch)) {
      while (isdigit(ch)) {
        *p++ = ch;
        ch = nextchar(fp);
        if (ch == EOF) {
          value->q = (intmax_t) value->d;
          if (type)
            *type = thetype;
          return;
        }
      }
      if (ch == '.') {	/* a float number: the last entry */
        do
          *p++ = nextchar(fp);
        while (isdigit((int32_t) p[-1])); /* find the rest */
        *p = '\0';		/* temporary termination */
        value->d *= 60;
        value->d += atof(numstart);
        base++;
        break;		/* we're done */
      } else {		/* we found another integer element */
        *p = '\0';		/* temporary termination */
        value->d *= 60;
        value->d += atol(numstart);
        base++;
        ch = nextchar(fp);
      }
    }
    while (base--)		/* back to units */
      value->d /= 60;
    if (kind == 'H')
      value->d *= 15;	/* from hours to degrees */
    kind = toupper(*p);
    if (kind == 'D' || kind == 'E') {
      if (toupper(ch) == 'I') { /* imaginary */
        thetype = (kind == 'D')? LUX_CDOUBLE: LUX_CFLOAT;
      } else
        thetype = (kind == 'D')? LUX_DOUBLE: LUX_FLOAT;
    } else if (kind == 'I') {
      thetype = LUX_CFLOAT;
    } else
      thetype = LUX_FLOAT;
    value->d *= sign;	/* put the sign back on it */
    if (type)
      *type = thetype;
    return;
  case '.': case 'D': case 'E':
    if (ch == '.') {
      do
        *p++ = nextchar(fp);
      while (isdigit((int32_t) p[-1]));
    }
    kind = toupper(p[-1]);
    thetype = LUX_FLOAT;	/* default */
    if (kind == 'D' || kind == 'E') {
      if (kind == 'D') {
        p[-1] = 'E';		/* so atof can read it */
        thetype = LUX_DOUBLE;
      }
      ch = nextchar(fp);
      if (ch == '+' || ch == '-') /* a signed exponent */
        *p++ = ch;
      else
        unnextchar(ch, fp);
      do
        *p++ = nextchar(fp);
      while (isdigit((int32_t) p[-1]));
      kind = toupper(p[-1]); /* must look for I */
    }
    *p = '\0';		/* temporary end */
    value->d = atof(numstart)*sign;
    if (kind == 'I') {
      thetype = (thetype == LUX_DOUBLE)? LUX_CDOUBLE: LUX_CFLOAT;
    }
    /* the last character we read was not part of the number. */
    /* if it is not a newline, then we put it back in the stream. */
    if (kind != '\n' && unnextchar(kind, fp) != kind)
      puts("unnextchar() unsuccessful");
    if (type)
      *type = thetype;
    return;
  default:
    /* the last character we read was not part of the number. */
    /* if it is not a newline, then we put it back in the stream. */
    if (ch != '\n' && unnextchar(ch, fp) != ch)
      puts("unnextchar() unsuccessful");
    break;
  }

  kind = toupper(p[-1]);
  p[-1] = '\0';			/* temporary end */
  /* NOTE: have to use strtoul() instead of strtol() because strtol() */
  /* -- at least on SGI Irix6.3 -- does not accept numbers with their most */
  /* significant Byte set (such as 0xc9460fc0), even though printf() */
  /* has no trouble generating such numbers.  I.e., t,'%x',-0x35b9f040 */
  /* yielded c9460fd0 but t,'%x',0xc9460fd0 when using strtol() yields */
  /* 0xffffffff.  LS 11nov99 */
  value->q = strtoull(numstart, NULL, base)*sign;
  switch (kind) {
  case 'B':
    thetype = LUX_INT8;
    break;
  case 'W':
    thetype = LUX_INT16;
    break;
  case 'L':
    thetype = LUX_INT32;
    break;
  case 'Q':
    thetype = LUX_INT64;
    break;
  case 'I':
    value->d = value->q;
    thetype = LUX_CFLOAT;
    break;
  default:
    thetype = LUX_INT32;		/* default */
    break;
  }
  if (type)
    *type = thetype;
}
/*-------------------------------------------------------------------------*/
#ifdef FACTS
void *seekFacts(int32_t symbol, int32_t type, int32_t flag)
/* seeks facts of the indicated <type> and <flag> associated with the */
/* <symbol>. */
/* Returns a pointer to the facts, if found, or else returns NULL. */
/* LS 6apr99 */
{
  uint8_t	n;
  arrayFacts	*facts;

  if (!symbolIsNumericalArray(symbol) /* not a numerical symbol */
      || !array_facts(symbol))	/* no associated facts */
    return NULL;
  
  n = array_num_facts(symbol);
  facts = array_facts(symbol);
  while (n--) {
    if (facts->type == type)
      return (facts->flags & flag)? facts: NULL;
    facts++;
  }
  return NULL;
}
/*-------------------------------------------------------------------------*/
void *setFacts(int32_t symbol, int32_t type, int32_t flag)
/* seeks facts of the indicated <type> associated with the <symbol>. */
/* Returns a pointer to the facts, if found.  Otherwise, allocates a */
/* new slot for the facts and returns a pointer to it. */
/* sets the appropriate <flag>. */
/* Returns NULL if unsucessful.  LS 6apr99 */
{
  arrayFacts	*facts;
  int32_t	n;

  if (!symbolIsNumericalArray(symbol)) /* not a numerical symbol */
    return NULL;
  
  if (array_facts(symbol)) {
    facts = seekFacts(symbol, type, LUX_ANY_FACT);
    if (facts) {		/* it exists already */
      facts->type |= flag;
      return facts;
    }
    /* it doesn't exist yet, but we have some other facts already */
  }

  n = array_num_facts(symbol);
  if (n == 255)			/* no room for more */
    return NULL;
  array_num_facts(symbol) = ++n;

  facts = realloc(array_facts(symbol), n*sizeof(arrayFacts));
  if (!facts)
    return NULL;
  array_facts(symbol) = facts;
  facts += n - 1;		/* the new one */
  facts->type = type;		/* set defaults */
  facts->flags = flag;
  facts->fact.any = NULL;	/* no data yet */
  return facts;			/* pointer to the last one */
}
/*-------------------------------------------------------------------------*/
void deleteFacts(int32_t symbol, int32_t type)
{
  int32_t	n, nf;
  arrayFacts	*facts;

  if (!symbolIsNumericalArray(symbol))
    return;
  
  if (array_facts(symbol)) {
    facts = seekFacts(symbol, type, LUX_ANY_FACT);
    if (facts) {
      nf = array_num_facts(symbol);
      n = nf + (array_facts(symbol) - facts) - 1;
      if (n)
	memcpy(facts, facts + 1, n*sizeof(arrayFacts));
      array_num_facts(symbol) = --nf;
      if (nf)
	facts = realloc(facts, nf*sizeof(arrayFacts));
      else {
	free(facts);
	facts = NULL;
      }
      array_facts(symbol) = facts;
    }
  }
}
#endif
/*-------------------------------------------------------------------------*/
#undef malloc
char *strsave_system(char *str)
/* saves string <str> and returns address; uses system malloc rather
 than debug malloc. */
{
 char	*p;

 if (!(p = (char *) malloc(strlen(str) + 1))) {
   printf("strsave_system: ");
   cerror(ALLOC_ERR, 0);
   return NULL;
 }
 strcpy(p, str);
 return p;
}
/*-----------------------------------------------------*/
/* We implement a simple integer stack.  It uses curScrat() for scratch */
/* space, so it mustn't get too big.  Call newStack(x) to initialize */
/* the stack with user-selected sentinel value <x>.  Call push(x) to */
/* push value <x> unto the stack, and call pop() to pop the top element */
/* from the stack.  When pop() returns the sentinel value, then the */
/* stack is empty.  Call deleteStack() to properly clean up. */
static int32_t *stack, stack_sentinel;
void newStack(int32_t sentinel)
{
  stack = (int32_t *) curScrat;
  *stack++ = stack_sentinel = sentinel;
}
/*-----------------------------------------------------*/
void push(int32_t value)
{
  *stack++ = value;
}
/*-----------------------------------------------------*/
int32_t pop(void)
{
  return *--stack;
}
/*-----------------------------------------------------*/
void deleteStack(void)
{
  while (*stack != stack_sentinel)
    --stack;
}
/*-----------------------------------------------------*/
static struct {
  char *buffer;
  char *ptr;
} keyboard = { NULL, NULL };

int32_t nextchar(FILE *fp) {
  /* returns the next char from the indicated file pointer */
  /* if fp == stdin (i.e., reading from the keyboard), then a special */
  /* data input buffer is used, with all command line editing except */
  /* history buffer stuff enabled.  This can be used as an alternative */
  /* for fgetc(). LS 29mar2001 */
  int32_t getNewLine(char *, size_t, char const *, char);

  if (fp == stdin) {
    if (!keyboard.ptr) {
      keyboard.buffer = (char*) malloc(BUFSIZE);
      keyboard.ptr = keyboard.buffer;
      *keyboard.ptr = '\0';
    }
    if (!*keyboard.ptr) {	/* no more input */
      if (keyboard.ptr == keyboard.buffer) /* we must ask for more */
	while (!*keyboard.ptr) {
	  FILE *is;
	  is = inputStream;
	  inputStream = stdin;
	  getNewLine(keyboard.buffer, BUFSIZE, "dat>", 0);
	  inputStream = is;
	  keyboard.ptr = keyboard.buffer;
	}
      else {			/* we've just arrived at the end of the */
				/* current line */
	keyboard.ptr = keyboard.buffer;
	*keyboard.ptr = '\0';
	return '\n';
      }
    }
    return *keyboard.ptr++;
  } else
    return fgetc(fp);
}
/*------------------------------------------------------------------------- */
int32_t unnextchar(int32_t c, FILE *fp) {
  /* push-back analogon to nextchar().  Can be used instead of ungetc(). */
  /* LS 29mar2001 */
  if (fp == stdin) {
    if (keyboard.ptr > keyboard.buffer)
      --keyboard.ptr;
    *keyboard.ptr = c;
    return c;
  } else
    return ungetc(c, fp);
}
/*------------------------------------------------------------------------- */
char *nextline(char *buf, size_t maxsize, FILE *fp) {
  /* reads an \n-terminated line from the indicated file pointer. */
  /* if fp == stdin (i.e., reading from the keyboard), then a special */
  /* data input buffer is used, with all command line editing except */
  /* history buffer stuff enabled.  This can be used as an alternative */
  /* for fgets(). LS 29mar2001 */
  char *buf0 = buf;

  if (fp == stdin) {
    while (maxsize--) {
      *buf++ = nextchar(fp);
      if (buf[-1] == '\n')
	break;
    }
  } else
    return fgets(buf, maxsize, fp);
  return buf0;
}
/*------------------------------------------------------------------------- */
