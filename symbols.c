/* File symbols.c */
/* ANA routines dealing with the creation of ANA symbols. */
/* $Id: symbols.c,v 4.0 2001/02/07 20:37:05 strous Exp $ */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include "install.h"
#include "action.h"
#include "editor.h"		/* for BUFSIZE */
#ifdef X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif
static char rcsid[] __attribute__ ((unused)) =
 "$Id: symbols.c,v 4.0 2001/02/07 20:37:05 strous Exp $";

void	zerobytes(void *, int), updateIndices(void), symdumpswitch(int, int);
#ifdef X11
void	xsynchronize(int);
#endif
int	installString(char *), fixContext(int, int), ana_replace(int, int);
/*-----------------------------------------------------*/
void embed(int target, int context)
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
int structPtrTarget(int symbol)
/* returns number of symbol that LIST_PTR <symbol> points at */
{
  int	base, index = -1, i, n;
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
    case ANA_RANGE:
      /* if the indicated range start or end contains a (*-...) entry,
       then the negative of its value is returned */
      if (index < 0 || index > 1) /* bad label */
	return cerror(BAD_STRUCT_KEY, symbol);
      n = index? range_end(base): range_start(base);
      if (n < 0)
	n = -n;
      return n;
    case ANA_CLIST:
      if (index < 0 || index >= clist_num_symbols(base))
	return cerror(BAD_STRUCT_KEY, symbol);
      return clist_symbols(base)[index];
    case ANA_LIST:
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
    case ANA_SUBROUTINE:
    case ANA_FUNCTION:
      if (index < 0) {		/* need to match the key */
	i = installString(key);
	n = findVar(i, base);
	freeString(i);
      }
      if (index >= 0 || n < 0)
	return cerror(BAD_STRUCT_KEY, symbol);
      return transfer(n);
    default:
      return anaerror("Pointer to non-embedding variable.", symbol);
  }
}
/*-----------------------------------------------------*/
int transfer(int symbol)
/* if symbol is a TRANSFER or POINTER, return symbol number of */
/* ultimate target */
{
  while (symbol_class(symbol) == ANA_POINTER
	 || symbol_class(symbol) == ANA_TRANSFER) {
    if (transfer_target(symbol) == symbol) {
      symbol_class(symbol) = ANA_UNDEFINED;
      printf("WARNING - pointer %s points at itself: make undefined\n",
	     symbolIdent(symbol, I_PARENT));
      return symbol;
    }
    symbol = transfer_target(symbol);
    if (symbol_class(symbol) == ANA_UNUSED)
      symbol = 0;
  }
  return symbol;
}
/*-----------------------------------------------------*/
int ana_convert(int, int [], int, int);
int ana_convertsym(int narg, int ps[])
     /* Y = CONVERT(X, TYPE) returns a copy of X converted to data type */
     /* TYPE (according to #TYPE).  LS 1aug97 */
{
  int	iq, type;

  iq = ps[0];
  switch (symbol_class(ps[1])) {
    case ANA_SCAL_PTR:
      iq = dereferenceScalPointer(iq);
    case ANA_SCALAR:
      type = int_arg(iq);
      break;
    default:
      return cerror(ILL_CLASS, ps[0]);
  }
  if (type < ANA_BYTE || type > ANA_DOUBLE)
    return cerror(ILL_TYPE, ps[1]);
  return ana_convert(1, ps, type, 1);
}
/*-----------------------------------------------------*/
int scalar_scratch(int type)
/* returns a temporary scalar of the indicated type */
{
 int	n;

 getFreeTempVariable(n);
 symbol_type(n) = type;
 symbol_line(n) = curLineNumber;
 if (type >= ANA_CFLOAT) {
   complex_scalar_memory(n) = ana_type_size[type];
   complex_scalar_data(n).f = malloc(complex_scalar_memory(n));
   if (!complex_scalar_data(n).f)
     return cerror(ALLOC_ERR, n);
   symbol_class(n) = ANA_CSCALAR;
 } else
   symbol_class(n) = ANA_SCALAR;
 return n;
}
/*-----------------------------------------------------*/
int scalar_scratch_copy(int nsym)
/* creates a temporary scalar which is a copy of <nsym> */
{
 int	n;

 getFreeTempVariable(n);
 sym[n].class = ANA_SCALAR;
 sym[n].type = sym[nsym].type;
 sym[n].line = curLineNumber;
 memcpy(&sym[n].spec.scalar.b, &sym[nsym].spec.scalar.b, sizeof(double));
 return n;
}
/*-----------------------------------------------------*/
int string_scratch(int size)
/* returns a new temporary string with the indicated size
   (terminating null not counted) */
{
 int	n;

 getFreeTempVariable(n);
 sym[n].class = ANA_STRING;
 sym[n].type = ANA_TEMP_STRING;
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
int to_scratch_array(int n, int type, int ndim, int dims[])
/* modifies symbol <n> to an array of the specified type and dimensions */
/* if the type is ANA_TEMP_STRING or ANA_LSTRING, then the new */
/* array gets type ANA_STRING_ARRAY.  LS 28mar98 */
{
 size_t	size, i;
 float	fsize;
 array	*h;
 pointer	ptr;
 
 if (isStringType(type))
   type = ANA_STRING_ARRAY;
 if (!isLegalType(type))
   return cerror(ILL_TYPE, n, typeName(type));
 size = ana_type_size[type];
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
     return anaerror("Illegal dimension size, %1d", 0, dims[i]);
   fsize *= dims[i];
   size *= dims[i];
 }
 size += sizeof(array);
 fsize += sizeof(array);
 if (fabs(((float) size)/fsize - 1) > 1e-3 || size > INT_MAX)
   return anaerror("The number of bytes requested for the array\n(about %g) is too great", 0, fsize);
 undefine(n);
 symbol_class(n) = isComplexType(type)? ANA_CARRAY: ANA_ARRAY;
 array_type(n) = type;
 symbol_line(n) = curLineNumber;
 if (!(ptr.v = malloc(size))) {
   printf("requested %1d bytes in array_scratch\n", size);
   return cerror(ALLOC_ERR, 0);
 }
 array_header(n) = h = (array *) ptr.v;
 symbol_memory(n) = size;
 array_num_dims(n) = ndim;
 memcpy(array_dims(n), dims, ndim*sizeof(int));
 h->c1 = h->c2 = h->nfacts = 0;
 h->facts = NULL;
 zerobytes(array_data(n), array_size(n)*ana_type_size[type]);
 return n;
} 
/*-----------------------------------------------------*/
int to_scalar(int nsym, int type)
/* turns symbol <nsym> into a scalar or cscalar of the given <type> */
{
  undefine(nsym);
  symbol_class(nsym) = isRealType(type)? ANA_SCALAR: ANA_CSCALAR;
  symbol_type(nsym) = type;
  if (isComplexType(type)) {
    complex_scalar_data(nsym).cf = malloc(ana_type_size[type]);
    if (!complex_scalar_data(nsym).cf)
      return cerror(ALLOC_ERR, 0);
  }
  return ANA_OK;
}
/*-----------------------------------------------------*/
int array_scratch(int type, int ndim, int dims[])
{
  int	n;

  if (!isLegalType(type))
    return cerror(ILL_TYPE, 0, typeName(type));
  getFreeTempVariable(n);
  return to_scratch_array(n, type, ndim, dims);
}
/*-----------------------------------------------------*/
int array_clone(int symbol, int type)
/* returns a new temporary array of the indicated type,
  with the same structure as <symbol> */
{
 int	n, size;
 float	fsize;
 array	*h, *hOld;
 void	*ptr;
 extern int	pipeSym, pipeExec;

 size = ((symbol_memory(symbol) - sizeof(array))
	 / ana_type_size[array_type(symbol)]) * ana_type_size[type]
   + sizeof(array);
 if (ana_type_size[type] > ana_type_size[array_type(symbol)]) {
   fsize = ((float) (symbol_memory(symbol) - sizeof(array))
	    / ana_type_size[array_type(symbol)]) * ana_type_size[type]
     + sizeof(array);
   if (fsize != (float) size)
     return anaerror("The number of bytes requested for the array\n(about %g) is too great", 0, fsize);
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
		      ANA_ARRAY: ANA_CARRAY);
   symbol_line(n) = curLineNumber;
   if (!(ptr = malloc(size))) {
     printf("requested %1d bytes in array_clone\n", size);
     return cerror(ALLOC_ERR, 0);
   } 
   array_header(n) = ptr;
   symbol_memory(n) = size;
   h = (array *) ptr;
   hOld = HEAD(symbol);
   h->ndim = hOld->ndim;
   memcpy(h->dims, hOld->dims, h->ndim*sizeof(int));
   h->c1 = h->c2 = h->nfacts = 0;
   h->facts = NULL;
 }
 array_type(n) = type;
 return n; 
}
/*-----------------------------------------------------*/
int numerical_clone(int iq, enum Symboltype type) {
  switch (symbol_class(iq)) {
  case ANA_ARRAY:
    return array_clone(iq, type);
  case ANA_SCALAR:
    return scalar_scratch(type);
  default:
    return anaerror("Need numerical argument", iq);
  }
}
/*-----------------------------------------------------*/
int dereferenceScalPointer(int nsym)
/* returns an ordinary ANA_SCALAR for a ANA_SCAL_PTR.  NOTE: assumes that
 <nsym> is a SCAL_PTR!  LS 31jul98 */
{
 int	n, type;
 pointer	ptr;

 type = scal_ptr_type(nsym);
 if (type == ANA_TEMP_STRING)
   n = string_scratch(string_size(nsym));
 else if (isFreeTemp(nsym))
   n = nsym;
 else
   n = nextFreeTempVariable();
 switch (type) {
   case ANA_TEMP_STRING:
     break;
   case ANA_CFLOAT: case ANA_CDOUBLE:
     symbol_class(n) = ANA_CSCALAR;
     complex_scalar_type(n) = type;
     complex_scalar_memory(n) = ana_type_size[type];
     complex_scalar_data(n).f = malloc(complex_scalar_memory(n));
     if (!complex_scalar_data(n).f)
       return cerror(ALLOC_ERR, n);
     break;
   default:
     symbol_class(n) = ANA_SCALAR;
     scalar_type(n) = type;
     break;
 }
 ptr = scal_ptr_pointer(nsym);
 switch (scal_ptr_type(nsym)) {
   case ANA_BYTE:
     scalar_value(n).b = *ptr.b;
     break;
   case ANA_WORD:
     scalar_value(n).w = *ptr.w;
     break;
   case ANA_LONG:
     scalar_value(n).l = *ptr.l;
     break;
   case ANA_FLOAT:
     scalar_value(n).f = *ptr.f;
     break;
   case ANA_DOUBLE:
     scalar_value(n).d = *ptr.d;
     break;
   case ANA_CFLOAT:
     complex_scalar_data(n).cf->real = ptr.cf->real;
     complex_scalar_data(n).cf->imaginary = ptr.cf->imaginary;
     break;
   case ANA_CDOUBLE:
     complex_scalar_data(n).cd->real = ptr.cd->real;
     complex_scalar_data(n).cd->imaginary = ptr.cd->imaginary;
     break;
   case ANA_TEMP_STRING:
     string_value(n) = strsave(ptr.s);
     break;
   default:
     return cerror(ILL_CLASS, nsym);
 }
 return n;
}
/*-----------------------------------------------------*/
char *strsave(char *str)
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
int int_arg(int nsym)
/* returns the integer value of a scalar symbol */
{
 if (nsym < 0 || nsym >= NSYM) {
   cerror(ILL_SYM, 0, nsym, "int_arg");
   return 0;
 }
 if (symbol_class(nsym) == ANA_SCAL_PTR)
   nsym = dereferenceScalPointer(nsym);
 if (symbol_class(nsym) != ANA_SCALAR) {
   cerror(NO_SCAL, nsym);
   return 0;
 }
 switch (scalar_type(nsym)) {
   case ANA_BYTE:
     return (int) scalar_value(nsym).b;
   case ANA_WORD:
     return (int) scalar_value(nsym).w;
   case ANA_LONG:
     return (int) scalar_value(nsym).l;
   case ANA_FLOAT:
     return (int) scalar_value(nsym).f;
   case ANA_DOUBLE:
     return (int) scalar_value(nsym).d;
   default:
     return cerror(ILL_TYPE, nsym);
 }
}
/*-----------------------------------------------------*/
int int_arg_stat(int nsym, int *value)
/* returns integer value of symbol <nsym>, if any, or an error */
{
 if (nsym < 0 || nsym >= NSYM) 
   return cerror(ILL_SYM, 0, nsym, "int_arg_stat");
 if (symbol_class(nsym) == ANA_SCAL_PTR)
   nsym = dereferenceScalPointer(nsym);
 if (symbol_class(nsym) != ANA_SCALAR)
   return cerror(NO_SCAL, nsym);
 switch (scalar_type(nsym))
 { case ANA_BYTE:
     *value = (int) scalar_value(nsym).b;
     break;
   case ANA_WORD:
     *value = (int) scalar_value(nsym).w;
     break;
   case ANA_LONG:
     *value = (int) scalar_value(nsym).l;
     break;
   case ANA_FLOAT:
     *value = (int) scalar_value(nsym).f;
     break;
   case ANA_DOUBLE:
     *value = (int) scalar_value(nsym).d;
     break; }
 return 1;			/* everything OK */
}  
/*-----------------------------------------------------*/
float float_arg(int nsym)
/* returns the float value of a scalar symbol */
{
 if (nsym < 0 || nsym >= NSYM) 
 { cerror(ILL_SYM, 0, nsym, "float_arg");
   return 0; }
 if (sym[nsym].class == ANA_SCAL_PTR) nsym = dereferenceScalPointer(nsym);
 if (sym[nsym].class != ANA_SCALAR) { cerror(NO_SCAL, nsym);  return 0.0; }
 switch (scalar_type(nsym))
 { case ANA_BYTE:
     return (float) scalar_value(nsym).b;
   case ANA_WORD:
     return (float) scalar_value(nsym).w;
   case ANA_LONG:
     return (float) scalar_value(nsym).l;
   case ANA_FLOAT:
     return (float) scalar_value(nsym).f;
   case ANA_DOUBLE:
     return (float) scalar_value(nsym).d;
   default:
     return cerror(ILL_TYPE, nsym); }
}
/*-----------------------------------------------------*/
int float_arg_stat(int nsym, float *value)
/* returns float value of symbol <nsym>, if any, or an error */
{
 if (nsym < 0 || nsym >= NSYM) 
   return cerror(ILL_SYM, 0, nsym, "int_arg_stat");
 if (symbol_class(nsym) == ANA_SCAL_PTR)
   nsym = dereferenceScalPointer(nsym);
 if (symbol_class(nsym) != ANA_SCALAR)
   return cerror(NO_SCAL, nsym);
 switch (scalar_type(nsym)) {
   case ANA_BYTE:
     *value = (float) scalar_value(nsym).b;
     break;
   case ANA_WORD:
     *value = (float) scalar_value(nsym).w;
     break;
   case ANA_LONG:
     *value = (float) scalar_value(nsym).l;
     break;
   case ANA_FLOAT:
     *value = (float) scalar_value(nsym).f;
     break;
   case ANA_DOUBLE:
     *value = (float) scalar_value(nsym).d;
     break;
 }
 return ANA_OK;			/* everything OK */
}  
/*-----------------------------------------------------*/
double double_arg(int nsym)
/* returns the double value of a ANA_DOUBLE scalar symbol */
{
 if (nsym < 0 || nsym >= NSYM) 
 { cerror(ILL_SYM, 0, nsym, "double_arg");
   return 0; }
 if (sym[nsym].class == ANA_SCAL_PTR) nsym = dereferenceScalPointer(nsym);
 if (sym[nsym].class != ANA_SCALAR) { cerror(NO_SCAL, nsym);  return 0.0; }
 switch (scalar_type(nsym))
 { case ANA_BYTE:
     return (double) scalar_value(nsym).b;
   case ANA_WORD:
     return (double) scalar_value(nsym).w;
   case ANA_LONG:
     return (double) scalar_value(nsym).l;
   case ANA_FLOAT:
     return (double) scalar_value(nsym).f;
   case ANA_DOUBLE:
     return (double) scalar_value(nsym).d;
   default:
     return cerror(ILL_TYPE, nsym); }
}
/*-----------------------------------------------------*/
int double_arg_stat(int nsym, double *value)
/* returns double value of symbol <nsym>, if any, or an error */
{
 if (nsym < 0 || nsym >= NSYM) 
   return cerror(ILL_SYM, 0, nsym, "int_arg_stat");
 if (symbol_class(nsym) == ANA_SCAL_PTR)
   nsym = dereferenceScalPointer(nsym);
 if (symbol_class(nsym) != ANA_SCALAR)
   return cerror(NO_SCAL, nsym);
 switch (scalar_type(nsym)) {
   case ANA_BYTE:
     *value = (double) scalar_value(nsym).b;
     break;
   case ANA_WORD:
     *value = (double) scalar_value(nsym).w;
     break;
   case ANA_LONG:
     *value = (double) scalar_value(nsym).l;
     break;
   case ANA_FLOAT:
     *value = (double) scalar_value(nsym).f;
     break;
   case ANA_DOUBLE:
     *value = (double) scalar_value(nsym).d;
     break;
 }
 return ANA_OK;			/* everything OK */
}  
/*-----------------------------------------------------*/
char *string_arg(int nsym)
/* returns the string value of a string symbol, or NULL */
{
 if (nsym < 0 || nsym >= NSYM) 
 { cerror(ILL_SYM, 0, nsym, "string_arg");
   return 0; }
 if (sym[nsym].class == ANA_SCAL_PTR) nsym = dereferenceScalPointer(nsym);
 if (sym[nsym].class != ANA_STRING) { cerror(NEED_STR, nsym);  return NULL; }
 return string_value(nsym);
}
/*-----------------------------------------------------*/
int ana_byte(int narg, int ps[])
/* returns a ANA_BYTE version of the argument */
{
  return ana_convert(narg, ps, ANA_BYTE, 1);
}
/*-----------------------------------------------------*/
int ana_word(int narg, int ps[])
/* returns a ANA_WORD version of the argument */
{
  return ana_convert(narg, ps, ANA_WORD, 1);
}
/*-----------------------------------------------------*/
int ana_long(int narg, int ps[])
/* returns a ANA_LONG version of the argument */
{
  return ana_convert(narg, ps, ANA_LONG, 1);
}
/*-----------------------------------------------------*/
int ana_floor(int narg, int ps[])
/* returns a ANA_LONG version of the argument */
/* each float number is transformed into the next lower integer */
/* compare ana_long(), where each float number is transformed into the */
/* next integer closer to zero, and ana_rfix(), where each float is */
/* transformed into the closest integer.  LS 24may96 */
{
 int	iq, result, n, temp, size, type;
 int	value;
 pointer	src, trgt;

 iq = *ps;
 if (!symbolIsNumerical(iq)	/* not numerical */
     && !symbolIsStringScalar(iq))	/* and not a string either */
   return cerror(ILL_CLASS, iq); /* reject */
 if (symbol_type(iq) == ANA_LONG) /* if it's already BYTE then we're done */
   return iq;
 temp = (isFreeTemp(iq))? 1: 0;
 type = symbol_type(iq);	/* gotta store now because "result" may */
				/* be same symbol as "iq" */
 switch (symbol_class(iq)) {
   case ANA_SCAL_PTR:		/* transform to scalar */
     iq = dereferenceScalPointer(iq);
     temp = (isFreeTemp(iq))? 1: 0;
   case ANA_SCALAR:
     if (temp)			/* can use iq to store the result */
       result = iq;
     else			/* need new scalar */
       result = scalar_scratch(ANA_LONG);
     trgt.b = &scalar_value(result).b;
     src.b = &scalar_value(iq).b;
     n = 1;
     break;
   case ANA_STRING:
     if (temp)
       result = iq; 
     else
       result = scalar_scratch(ANA_LONG);
     value = (int) floor(atof((char *) string_value(iq))); /* convert */
     if (temp) {
       free(string_value(iq));	/* change string to scalar: free up memory */
       symbol_class(result) = ANA_SCALAR;
       scalar_type(result) = ANA_LONG;
     }
     scalar_value(result).l = value;
     return result;
   case ANA_CSCALAR:
     result = scalar_scratch(ANA_LONG);
     temp = 0;
     n = 1;
     trgt.b = &scalar_value(result).b;
     src.cf = complex_scalar_data(iq).cf;
     break;
   case ANA_ARRAY:
     /* we can use the input symbol if it is free and if
	it has at least as much memory as we need */
     if (temp
	 && (int) array_type(iq) >= ANA_LONG)
       result = iq;
     else {
       result = array_clone(iq, ANA_LONG);
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
   case ANA_BYTE:
     while (n--)
       *trgt.l++ = (int) *src.b++;
     break;
   case ANA_WORD:
     while (n--)
       *trgt.l++ = (int) *src.w++;
     break;
   case ANA_LONG:
     while (n--)
       *trgt.l++ = (int) *src.l++;
     break;
   case ANA_FLOAT:
     while (n--)
       *trgt.l++ = (int) floor(*src.f++);
     break;
   case ANA_DOUBLE:
     while (n--)
       *trgt.l++ = (int) floor(*src.d++);
     break;
   case ANA_CFLOAT:
     while (n--)
       *trgt.l++ = (int) floor(src.cf++->real);
     break;
   case ANA_CDOUBLE:
     while (n--)
       *trgt.l++ = (int) floor(src.cd++->real);
     break;
 }
 if (temp			/* we used input symbol to store results */
     && symbol_class(iq) == ANA_ARRAY /* it's an array */
     && array_type(iq) > ANA_LONG) { /* and bigger than we needed */
   symbol_memory(iq) = sizeof(array) + size*sizeof(int);
   symbol_data(iq) = (array *) realloc(symbol_data(iq), symbol_memory(iq));
   if (!symbol_data(iq))	/* reallocation failed */
     return anaerror("Realloc() failed in ana_floor", 0);
 }
 symbol_type(result) = ANA_LONG;
 return result;
}
/*-----------------------------------------------------*/
int ana_ceil(int narg, int ps[])
/* returns a ANA_LONG version of the argument */
/* each float number is transformed into the next higher integer */
/* compare ana_long(), where each float number is transformed into the */
/* next integer closer to zero, and ana_rfix(), where each float is */
/* transformed into the closest integer.  LS 24may96 */
{
 int	iq, result, n, temp, size, type;
 int	value;
 pointer	src, trgt;

 iq = *ps;
 if (!symbolIsNumerical(iq)	/* not numerical */
     && !symbolIsStringScalar(iq))	/* and not a string either */
   return cerror(ILL_CLASS, iq); /* reject */
 if (symbol_type(iq) == ANA_LONG) /* if it's already BYTE then we're done */
   return iq;
 temp = (isFreeTemp(iq))? 1: 0;
 type = symbol_type(iq);	/* gotta store now because "result" may */
				/* be same symbol as "iq" */
 switch (symbol_class(iq)) {
   case ANA_SCAL_PTR:		/* transform to scalar */
     iq = dereferenceScalPointer(iq);
     temp = (isFreeTemp(iq))? 1: 0;
   case ANA_SCALAR:
     if (temp)			/* can use iq to store the result */
       result = iq;
     else			/* need new scalar */
       result = scalar_scratch(ANA_LONG);
     trgt.b = &scalar_value(result).b;
     src.b = &scalar_value(iq).b;
     n = 1;
     break;
   case ANA_STRING:
     if (temp)
       result = iq; 
     else
       result = scalar_scratch(ANA_LONG);
     value = (int) ceil(atof((char *) string_value(iq))); /* convert */
     if (temp) {
       free(string_value(iq));	/* change string to scalar: free up memory */
       symbol_class(result) = ANA_SCALAR;
       scalar_type(result) = ANA_LONG;
     }
     scalar_value(result).l = value;
     return result;
   case ANA_CSCALAR:
     result = scalar_scratch(ANA_LONG);
     temp = 0;
     n = 1;
     trgt.b = &scalar_value(result).b;
     src.cf = complex_scalar_data(iq).cf;
     break;
   case ANA_ARRAY:
     /* we can use the input symbol if it is free and if
	it has at least as much memory as we need */
     if (temp
	 && (int) array_type(iq) >= ANA_LONG)
       result = iq;
     else {
       result = array_clone(iq, ANA_LONG);
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
   case ANA_BYTE:
     while (n--)
       *trgt.l++ = (int) *src.b++;
     break;
   case ANA_WORD:
     while (n--)
       *trgt.l++ = (int) *src.w++;
     break;
   case ANA_LONG:
     while (n--)
       *trgt.l++ = (int) *src.l++;
     break;
   case ANA_FLOAT:
     while (n--)
       *trgt.l++ = (int) ceil(*src.f++);
     break;
   case ANA_DOUBLE:
     while (n--)
       *trgt.l++ = (int) ceil(*src.d++);
     break;
   case ANA_CFLOAT:
     while (n--)
       *trgt.l++ = (int) ceil(src.cf++->real);
     break;
   case ANA_CDOUBLE:
     while (n--)
       *trgt.l++ = (int) ceil(src.cd++->real);
     break;
 }
 if (temp			/* we used input symbol to store results */
     && symbol_class(iq) == ANA_ARRAY /* it's an array */
     && array_type(iq) > ANA_LONG) { /* and bigger than we needed */
   symbol_memory(iq) = sizeof(array) + size*sizeof(int);
   symbol_data(iq) = (array *) realloc(symbol_data(iq), symbol_memory(iq));
   if (!symbol_data(iq))	/* reallocation failed */
     return anaerror("Realloc() failed in ana_floor", 0);
 }
 symbol_type(result) = ANA_LONG;
 return result;
}
/*-----------------------------------------------------*/
int ana_float(int narg, int ps[])
/* returns a ANA_FLOAT version of the argument */
{
  return ana_convert(narg, ps, ANA_FLOAT, 1);
}
/*-----------------------------------------------------*/
int ana_double(int narg, int ps[])
/* returns a ANA_DOUBLE version of the argument */
{
  return ana_convert(narg, ps, ANA_DOUBLE, 1);
}
/*-----------------------------------------------------*/
int ana_cfloat(int narg, int ps[])
/* returns an ANA_CFLOAT version of the argument */
{
  return ana_convert(narg, ps, ANA_CFLOAT, 1);
}
/*-----------------------------------------------------*/
int ana_cdouble(int narg, int ps[])
/* returns an ANA_CDOUBLE version of the argument */
{
  return ana_convert(narg, ps, ANA_CDOUBLE, 1);
}
/*-----------------------------------------------------*/
extern int	nFixed;
int ana_convert(int narg, int ps[], int totype, int isFunc)
/* converts ps[0] to data type <totype>. */
/* we use this function in one of two modes: function mode (isFunc != 0)
   or subroutine mode (isFunc = 0).  In function mode, there can be
   only a single argument, so then <narg> == 1.  In that case,
   we generate a copy of ps[0] with data type <totype>, which may
   be one of ANA_BYTE ... ANA_DOUBLE ANA_CFLOAT ANA_CDOUBLE
   ANA_TEMP_STRING.  If ps[0] is a temporary variable, then we use it
   to store the results in, too.  In subroutine mode, each of the
   arguments must be a named, writeable symbol, and we convert each
   of them in-place.  LS 16dec99 */
{
  int	iq, n, size, type, srcstep, trgtstep, temp, result;
  char	do_realloc = 0;
  pointer	src, trgt;
  scalar	value;
  extern char	*fmt_integer, *fmt_float, *fmt_complex;
  void	read_a_number(char **, scalar *, int *);

  while (narg--) {
    iq = *ps++;
    if (!symbolIsNumerical(iq)	/* not numerical */
	&& !symbolIsStringScalar(iq)	/* and not a string */
	&& !symbolIsStringArray(iq))	/* and not a string array either */
      return cerror(ILL_CLASS, iq); /* reject */
    if (!isFunc && (!symbolIsNamed(iq) || iq <= nFixed))
      return anaerror("Cannot modify symbol", iq);
    if (symbol_type(iq) == totype) { /* if it's already of the desired type */
				     /* then we're done */
      if (isFunc)
	return iq;
      continue;
    }
    type = symbol_type(iq);
    srcstep = ana_type_size[type];
    trgtstep = ana_type_size[totype];
    if (!isFunc || isFreeTemp(iq))
      result = iq;
    else {
      getFreeTempVariable(result);
    }
    switch (symbol_class(iq)) {	/* source */
      case ANA_SCALAR:
	if (isComplexType(totype)) {
	  value = scalar_value(iq);
	  src.b = &value.b;
	  complex_scalar_data(result).cf = malloc(trgtstep);
	  symbol_memory(result) = trgtstep;
	  if (!complex_scalar_data(result).cf)
	    return cerror(ALLOC_ERR, 0);
	  trgt.cf = complex_scalar_data(result).cf;
	  symbol_class(result) = ANA_CSCALAR;
	} else if (isStringType(totype)) {
	  src.b = &scalar_value(iq).b;
	  switch (type) {
	    case ANA_BYTE:
	      fmttok(fmt_integer);
	      Sprintf_tok(curScrat, (int) *src.b);
	      break;
	    case ANA_WORD:
	      fmttok(fmt_integer);
	      Sprintf_tok(curScrat, (int) *src.w);
	      break;
	    case ANA_LONG:
	      fmttok(fmt_integer);
	      Sprintf_tok(curScrat, (int) *src.l);
	      break;
	    case ANA_FLOAT:
	      fmttok(fmt_float);
	      Sprintf_tok(curScrat, (double) *src.f);
	      break;
	    case ANA_DOUBLE:
	      fmttok(fmt_float);
	      Sprintf_tok(curScrat, (double) *src.d);
	      break;
	  }
	  size = strlen(curScrat) + 1;
	  symbol_memory(result) = size;
	  symbol_class(result) = ANA_STRING;
	  symbol_type(result) = ANA_TEMP_STRING;
	  string_value(result) = malloc(size);
	  if (!string_value(result))
	    return cerror(ALLOC_ERR, 0);
	  memcpy(string_value(result), curScrat, size);
	  if (isFunc)
	    return result;
	  continue;
	} else {
	  symbol_class(result) = ANA_SCALAR;
	  src.b = &scalar_value(iq).b;
	  trgt.b = &scalar_value(result).b;
	}
	n = 1;
	break;
      case ANA_STRING:
	if (isIntegerType(totype))
	  value.l = atol(string_value(iq));
	else if (isRealType(totype))
	  value.d = atof(string_value(iq));
	else {			/* complex type */
	  src.s = string_value(iq);
	  read_a_number(&src.s, &value, &size);
	}
	if (result == iq)
	  free(string_value(iq)); /* change string to scalar: free up memory */
	if (isRealType(totype)) {
	  symbol_class(result) = ANA_SCALAR;
	  trgt.b = &scalar_value(result).b;
	} else {			/* complex output */
	  symbol_class(result) = ANA_CSCALAR;
	  complex_scalar_data(result).cf = malloc(trgtstep);
	  if (!complex_scalar_data(result).cf)
	    return cerror(ALLOC_ERR, 0);
	  symbol_memory(result) = trgtstep;
	  if (!complex_scalar_data(result).cf)
	    return cerror(ALLOC_ERR, 0);
	  trgt.cf = complex_scalar_data(result).cf;
	}
	switch (totype) {
	  case ANA_BYTE:
	    *trgt.b = value.l;
	    break;
	  case ANA_WORD:
	    *trgt.w = value.l;
	    break;
	  case ANA_LONG:
	    *trgt.l = value.l;
	    break;
	  case ANA_FLOAT:
	    *trgt.f = value.d;
	    break;
	  case ANA_DOUBLE:
	    *trgt.d = value.d;
	    break;
	  case ANA_CFLOAT:
	    trgt.cf->real = value.d;
	    trgt.cf->imaginary = 0.0;
	    break;
	  case ANA_CDOUBLE:
	    trgt.cd->real = value.d;
	    trgt.cd->imaginary = 0.0;
	    break;
	}
	symbol_type(result) = totype;
	if (isFunc)
	  return result;
	continue;
      case ANA_CSCALAR:
	if (isRealType(totype)) {	/* to a real type */
	  switch (type) {
	    case ANA_CFLOAT:
	      value.d = complex_scalar_data(iq).cf->real;
	      break;
	    case ANA_CDOUBLE:
	      value.d = complex_scalar_data(iq).cd->real;
	      break;
	  }
	  if (result == iq)
	    free(complex_scalar_data(iq).cf);
	  symbol_class(result) = ANA_SCALAR;
	  switch (totype) {
	    case ANA_BYTE:
	      scalar_value(result).b = (byte) value.d;
	      break;
	    case ANA_WORD:
	      scalar_value(result).w = (word) value.d;
	      break;
	    case ANA_LONG:
	      scalar_value(result).l = (int) value.d;
	      break;
	    case ANA_FLOAT:
	      scalar_value(result).f = (float) value.d;
	      break;
	    case ANA_DOUBLE:
	      scalar_value(result).d = (double) value.d;
	      break;
	  }
	  scalar_type(result) = totype;
	  if (isFunc)
	    return result;
	  continue;
	} else if (isStringType(totype)) { /* to a string */
	  switch (type) {
	    case ANA_BYTE:
	      fmttok(fmt_integer);
	      Sprintf_tok(curScrat, (int) *src.b);
	      break;
	    case ANA_WORD:
	      fmttok(fmt_integer);
	      Sprintf_tok(curScrat, (int) *src.w);
	      break;
	    case ANA_LONG:
	      fmttok(fmt_integer);
	      Sprintf_tok(curScrat, (int) *src.l);
	      break;
	    case ANA_FLOAT:
	      fmttok(fmt_float);
	      Sprintf_tok(curScrat, (double) *src.f);
	      break;
	    case ANA_DOUBLE:
	      fmttok(fmt_float);
	      Sprintf_tok(curScrat, (double) *src.d);
	      break;
	  }
	  if (result == iq)
	    free(complex_scalar_data(result).cf);
	  size = strlen(curScrat) + 1;
	  symbol_memory(result) = size;
	  symbol_class(result) = ANA_STRING;
	  symbol_type(result) = ANA_TEMP_STRING;
	  string_value(result) = malloc(size);
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
	      complex_scalar_data(result).cf =
		realloc(complex_scalar_data(result).cf, trgtstep);
	    }
	  } else
	    complex_scalar_data(result).cf = malloc(trgtstep);
	  if (!do_realloc && !complex_scalar_data(result).cf)
	    return cerror(ALLOC_ERR, 0);
	  symbol_memory(result) = trgtstep;
	  src.cf = complex_scalar_data(iq).cf;
	  trgt.cf = complex_scalar_data(result).cf;
	  symbol_class(result) = ANA_CSCALAR;
	  n = 1;
	}
	break;
      case ANA_ARRAY: case ANA_CARRAY:
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
	    array_header(result) = realloc(array_header(result), size);
	} else
	  array_header(result) = malloc(size);
	if (!do_realloc && !array_header(result))
	  return cerror(ALLOC_ERR, 0);
	if (iq != result)	/* copy dimensions &c */
	  memcpy(array_header(result), array_header(iq), sizeof(array));
	symbol_memory(result) = size;
	src.v = array_data(iq);
	trgt.v = array_data(result);
	if (isComplexType(totype))
	  symbol_class(result) = ANA_CARRAY;
	else
	  symbol_class(result) = ANA_ARRAY;
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
      case ANA_BYTE:
	switch (totype) {		/* target type */
	  case ANA_WORD:
	    while (n--) {
	      *trgt.w = (word) *src.b;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_LONG:
	    while (n--) {
	      *trgt.l = (int) *src.b;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_FLOAT:
	    while (n--) {
	      *trgt.f = (float) *src.b;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_DOUBLE:
	    while (n--) {
	      *trgt.d = (double) *src.b;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_CFLOAT:
	    while (n--) {
	      trgt.cf->real = (float) *src.b;
	      trgt.cf->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_CDOUBLE:
	    while (n--) {
	      trgt.cd->real = (double) *src.b;
	      trgt.cd->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_STRING_ARRAY:
	    fmttok(fmt_integer);
	    while (n--) {
	      Sprintf_tok(curScrat, (int) *src.b);
	      temp = strlen(curScrat) + 1;
	      *trgt.sp = malloc(temp);
	      if (!*trgt.sp)
		return cerror(ALLOC_ERR, 0);
	      memcpy(*trgt.sp, curScrat, temp);
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	}
	break;
      case ANA_WORD:
	switch (totype) {		/* target type */
	  case ANA_BYTE:
	    while (n--) {
	      *trgt.b = (word) *src.w;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_LONG:
	    while (n--) {
	      *trgt.l = (word) *src.w;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_FLOAT:
	    while (n--) {
	      *trgt.f = (word) *src.w;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_DOUBLE:
	    while (n--) {
	      *trgt.d = (word) *src.w;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_CFLOAT:
	    while (n--) {
	      trgt.cf->real = (float) *src.w;
	      trgt.cf->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_CDOUBLE:
	    while (n--) {
	      trgt.cd->real = (double) *src.w;
	      trgt.cd->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_STRING_ARRAY:
	    fmttok(fmt_integer);
	    while (n--) {
	      Sprintf_tok(curScrat, (int) *src.w);
	      temp = strlen(curScrat) + 1;
	      *trgt.sp = malloc(temp);
	      if (!*trgt.sp)
		return cerror(ALLOC_ERR, 0);
	      memcpy(*trgt.sp, curScrat, temp);
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	}
	break;
      case ANA_LONG:
	switch (totype) {		/* target type */
	  case ANA_BYTE:
	    while (n--) {
	      *trgt.b = (int) *src.l;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_WORD:
	    while (n--) {
	      *trgt.w = (int) *src.l;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_FLOAT:
	    while (n--) {
	      *trgt.f = (int) *src.l;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_DOUBLE:
	    while (n--) {
	      *trgt.d = (int) *src.l;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_CFLOAT:
	    while (n--) {
	      trgt.cf->real = (float) *src.l;
	      trgt.cf->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_CDOUBLE:
	    while (n--) {
	      trgt.cd->real = (double) *src.l;
	      trgt.cd->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_STRING_ARRAY:
	    fmttok(fmt_integer);
	    while (n--) {
	      Sprintf_tok(curScrat, (int) *src.l);
	      temp = strlen(curScrat) + 1;
	      *trgt.sp = malloc(temp);
	      if (!*trgt.sp)
		return cerror(ALLOC_ERR, 0);
	      memcpy(*trgt.sp, curScrat, temp);
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	}
	break;
      case ANA_FLOAT:
	switch (totype) {		/* target type */
	  case ANA_BYTE:
	    while (n--) {
	      *trgt.b = (float) *src.f;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_WORD:
	    while (n--) {
	      *trgt.w = (float) *src.f;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_LONG:
	    while (n--) {
	      *trgt.l = (float) *src.f;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_DOUBLE:
	    while (n--) {
	      *trgt.d = (float) *src.f;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_CFLOAT:
	    while (n--) {
	      trgt.cf->real = (float) *src.f;
	      trgt.cf->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_CDOUBLE:
	    while (n--) {
	      trgt.cd->real = (double) *src.f;
	      trgt.cd->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_STRING_ARRAY:
	    fmttok(fmt_float);
	    while (n--) {
	      Sprintf_tok(curScrat, (double) *src.f);
	      temp = strlen(curScrat) + 1;
	      *trgt.sp = malloc(temp);
	      if (!*trgt.sp)
		return cerror(ALLOC_ERR, 0);
	      memcpy(*trgt.sp, curScrat, temp);
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	}
	break;
      case ANA_DOUBLE:
	switch (totype) {		/* target type */
	  case ANA_BYTE:
	    while (n--) {
	      *trgt.b = (double) *src.d;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_WORD:
	    while (n--) {
	      *trgt.w = (double) *src.d;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_LONG:
	    while (n--) {
	      *trgt.l = (double) *src.d;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_FLOAT:
	    while (n--) {
	      *trgt.f = (double) *src.d;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_CFLOAT:
	    while (n--) {
	      trgt.cf->real = (float) *src.d;
	      trgt.cf->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_CDOUBLE:
	    while (n--) {
	      trgt.cd->real = (double) *src.d;
	      trgt.cd->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_STRING_ARRAY:
	    fmttok(fmt_float);
	    while (n--) {
	      Sprintf_tok(curScrat, (double) *src.d);
	      temp = strlen(curScrat) + 1;
	      *trgt.sp = malloc(temp);
	      if (!*trgt.sp)
		return cerror(ALLOC_ERR, 0);
	      memcpy(*trgt.sp, curScrat, temp);
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	}
	break;
      case ANA_CFLOAT:
	switch (totype) {
	  case ANA_BYTE:
	    while (n--) {
	      *trgt.b = (byte) src.cf->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_WORD:
	    while (n--) {
	      *trgt.w = (word) src.cf->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_LONG:
	    while (n--) {
	      *trgt.l = (int) src.cf->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_FLOAT:
	    while (n--) {
	      *trgt.f = (float) src.cf->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_DOUBLE:
	    while (n--) {
	      *trgt.d = (double) src.cf->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_CDOUBLE:
	    while (n--) {
	      trgt.cd->imaginary = src.cf->imaginary;
	      trgt.cd->real = src.cf->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_STRING_ARRAY:
	    while (n--) {
	      Sprintf(curScrat, fmt_complex, (double) src.cf->real,
		      (double) src.cf->imaginary);
	      temp = strlen(curScrat) + 1;
	      *trgt.sp = malloc(temp);
	      if (!*trgt.sp)
		return cerror(ALLOC_ERR, 0);
	      memcpy(*trgt.sp, curScrat, temp);
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	}
	break;
      case ANA_CDOUBLE:
	switch (totype) {
	  case ANA_BYTE:
	    while (n--) {
	      *trgt.b = (byte) src.cd->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_WORD:
	    while (n--) {
	      *trgt.w = (word) src.cd->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_LONG:
	    while (n--) {
	      *trgt.l = (int) src.cd->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_FLOAT:
	    while (n--) {
	      *trgt.f = (float) src.cd->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_DOUBLE:
	    while (n--) {
	      *trgt.d = (double) src.cd->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_CFLOAT:
	    while (n--) {
	      trgt.cf->imaginary = src.cd->imaginary;
	      trgt.cf->real = src.cd->real;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_STRING_ARRAY:
	    while (n--) {
	      Sprintf(curScrat, fmt_complex, (double) src.cd->real,
		      (double) src.cd->imaginary);
	      temp = strlen(curScrat) + 1;
	      *trgt.sp = malloc(temp);
	      if (!*trgt.sp)
		return cerror(ALLOC_ERR, 0);
	      memcpy(*trgt.sp, curScrat, temp);
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	}
	break;
      case ANA_STRING_ARRAY:	/* from string array */
	switch (totype) {
	  case ANA_BYTE:
	    while (n--) {
	      value.b = atol(*src.sp);
	      if (iq == result && *src.sp)
		free(*src.sp);
	      *trgt.b = value.b;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_WORD:
	    while (n--) {
	      value.w = atol(*src.sp);
	      if (iq == result && *src.sp)
		free(*src.sp);
	      *trgt.w = value.w;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_LONG:
	    while (n--) {
	      value.l = atol(*src.sp);
	      if (iq == result && *src.sp)
		free(*src.sp);
	      *trgt.l = value.l;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_FLOAT:
	    while (n--) {
	      value.f = atof(*src.sp);
	      if (iq == result && *src.sp)
		free(*src.sp);
	      *trgt.f = value.f;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_DOUBLE:
	    while (n--) {
	      value.d = atof(*src.sp);
	      if (iq == result && *src.sp)
		free(*src.sp);
	      *trgt.d = value.d;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_CFLOAT:
	    while (n--) {
	      read_a_number(src.sp, &value, &temp);
	      if (iq == result && *src.sp)
		free(*src.sp);
	      switch (temp) {
		case ANA_LONG:
		  trgt.cf->real = value.l;
		  break;
		case ANA_DOUBLE:
		  trgt.cf->real = value.d;
		  break;
	      }
	      trgt.cf->imaginary = 0.0;
	      trgt.b += trgtstep;
	      src.b += srcstep;
	    }
	    break;
	  case ANA_CDOUBLE:
	    while (n--) {
	      read_a_number(src.sp, &value, &temp);
	      if (iq == result && *src.sp)
		free(*src.sp);
	      switch (temp) {
		case ANA_LONG:
		  trgt.cd->real = value.l;
		  break;
		case ANA_DOUBLE:
		  trgt.cd->real = value.d;
		  break;
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
	array_header(result) = realloc(array_header(result), size);
	if (!array_header(result))
	  return cerror(ALLOC_ERR, 0);
	symbol_memory(result) = size;
	break;
      case 2:
	complex_scalar_data(result).cf =
	  realloc(complex_scalar_data(result).cf, size);
	if (!complex_scalar_data(result).cf)
	  return cerror(ALLOC_ERR, 0);
	symbol_memory(result) = size;
	break;
    }
  }
  return isFunc? result: ANA_OK;
}
/*-----------------------------------------------------*/
int ana_byte_inplace(int narg, int ps[])
/* BYTE,x  converts <x> to ANA_BYTE. */
{
  return ana_convert(narg, ps, ANA_BYTE, 0);
}
/*-----------------------------------------------------*/
int ana_word_inplace(int narg, int ps[])
/* WORD,x  converts <x> to ANA_WORD. */
{
  return ana_convert(narg, ps, ANA_WORD, 0);
}
/*-----------------------------------------------------*/
int ana_long_inplace(int narg, int ps[])
/* LONG,x  converts <x> to ANA_LONG. */
{
  return ana_convert(narg, ps, ANA_LONG, 0);
}
/*-----------------------------------------------------*/
int ana_float_inplace(int narg, int ps[])
/* FLOAT,x  converts <x> to ANA_FLOAT. */
{
  return ana_convert(narg, ps, ANA_FLOAT, 0);
}
/*-----------------------------------------------------*/
int ana_double_inplace(int narg, int ps[])
/* DOUBLE,x  converts <x> to ANA_DOUBLE. */
{
  return ana_convert(narg, ps, ANA_DOUBLE, 0);
}
/*-----------------------------------------------------*/
int ana_cfloat_inplace(int narg, int ps[])
/* CFLOAT,x  converts <x> to ANA_CFLOAT. */
{
  return ana_convert(narg, ps, ANA_CFLOAT, 0);
}
/*-----------------------------------------------------*/
int ana_cdouble_inplace(int narg, int ps[])
/* CDOUBLE,x  converts <x> to ANA_CDOUBLE. */
{
  return ana_convert(narg, ps, ANA_CDOUBLE, 0);
}
/*-----------------------------------------------------*/
int ana_string_inplace(int narg, int ps[])
/* STRING,x  converts <x> into a string form. */
{
  return ana_convert(narg, ps, ANA_STRING_ARRAY, 0);
}
/*-----------------------------------------------------*/
int get_dims(int *num, int *arg, int *dims)
/* reads *num positive integers from subsequent arg[]s and puts them */
/* in dims[]; returns the actual number of dimensions that were read
 in *num. */
{
 int	n, iq, size, *ptr, i;

 n = *num;
 while (n--) {
   iq = *arg++;
   if (symbol_class(iq) == ANA_ARRAY) {
     if (*num != 1)
       return cerror(ONLY_1_IF_ARR, iq);
     iq = ana_long(1, &iq);	/* ensure LONG */
     size = array_size(iq);
     if (size > MAX_DIMS)
       return cerror(N_DIMS_OVR, arg[-1]);
     ptr = array_data(iq);
     *num = size;
     memcpy(dims, ptr, size*sizeof(int));
     for (i = 0; i < size; i++)
       if (dims[i] <= 0)
	 return cerror(ILL_DIM, iq);
   } else if ((*dims++ = int_arg(iq)) <= 0)
     return cerror(ILL_DIM, iq);
 }
 return 1;
}
/*-----------------------------------------------------*/
int create_sub_ptr(int nsym, char *p, int index)
/* creates a ANA_SCAL_PTR symbol pointing at the element with
  index <index> in the array <p> with the data type
  of symbol <nsym> */
{
 int	iq;

 getFreeTempVariable(iq);
 sym[iq].class = ANA_SCAL_PTR;
 sym[iq].type = sym[nsym].type;
 sym[iq].line = curLineNumber;
 sym[iq].spec.general.ptr = p + index*ana_type_size[sym[nsym].type];
 return iq;
}
/*-----------------------------------------------------*/
int ana_array_convert(pointer *q1, pointer *q2, int type1, int type2, int n)
 /* more general conversion, converts data starting at q1 of type1 to type2
         data starting at q2, n count */
 /* note that indices are bumped even when count is one */
 {
 switch (type2) {
   case 0: switch (type1) {
         case 0:        while (n) { *q2->b++ =  (*q1->b++);n--;} break;
         case 1: while (n) { *q2->b++ = (byte) (*q1->w++);n--;} break;
         case 2: while (n) { *q2->b++ = (byte) (*q1->l++);n--;} break;
         case 3: while (n) { *q2->b++ = (byte) (*q1->f++);n--;} break;
         case 4: while (n) { *q2->b++ = (byte) (*q1->d++);n--;} break;
         }
   case 1: switch (type1) {
         case 1: while (n) { *q2->w++ =  (*q1->w++);n--;} break;
         case 0: while (n) { *q2->w++ = (short) (*q1->b++);n--;} break;
         case 2: while (n) { *q2->w++ = (short) (*q1->l++);n--;} break;
         case 3: while (n) { *q2->w++ = (short) (*q1->f++);n--;} break;
         case 4: while (n) { *q2->w++ = (short) (*q1->d++);n--;} break;
         }      break;
   case 2: switch (type1) {
         case 2: while (n) { *q2->l++ =  (*q1->l++);n--;} break;
         case 1: while (n) { *q2->l++ = (int) (*q1->w++);n--;} break;
         case 0: while (n) { *q2->l++ = (int) (*q1->b++);n--;} break;
         case 3: while (n) { *q2->l++ = (int) (*q1->f++);n--;} break;
         case 4: while (n) { *q2->l++ = (int) (*q1->d++);n--;} break;
         }      break;
   case 3: switch (type1) {
         case 3: while (n) { *q2->f++ =  (*q1->f++);n--;} break;
         case 2: while (n) { *q2->f++ = (float) (*q1->l++);n--;} break;
         case 1: while (n) { *q2->f++ = (float) (*q1->w++);n--;} break;
         case 0: while (n) { *q2->f++ = (float) (*q1->b++);n--;} break;
         case 4: while (n) { *q2->f++ = (float) (*q1->d++);n--;} break;
         }      break;
   case 4: switch (type1) {
         case 4: while (n) { *q2->d++ =  (*q1->d++);n--;} break;
         case 3: while (n) { *q2->d++ = (double) (*q1->f++);n--;} break;
         case 2: while (n) { *q2->d++ = (double) (*q1->l++);n--;} break;
         case 1: while (n) { *q2->d++ = (double) (*q1->w++);n--;} break;
         case 0: while (n) { *q2->d++ = (double) (*q1->b++);n--;} break;
         }      break;
  }
 return 1;
 }
/*-----------------------------------------------------*/
int redef_scalar(int nsym, int ntype, void *val)
/* redefine symbol nsym to be a scalar of type ntype with value *val */
{
  wideScalar	*value;

  value = (wideScalar *) val;

  undefine(nsym);
  symbol_class(nsym) = ANA_SCALAR;
  symbol_type(nsym) = ntype;
  switch (ntype) {
    case ANA_BYTE:
      scalar_value(nsym).b = value->b;
      break;
    case ANA_WORD:
      scalar_value(nsym).w = value->w;
      break;
    case ANA_LONG:
      scalar_value(nsym).l = value->l;
      break;
    case ANA_FLOAT:
      scalar_value(nsym).f = value->f;
      break;
    case ANA_DOUBLE:
      scalar_value(nsym).d = value->d;
      break;
    case ANA_CFLOAT:
      complex_scalar_memory(nsym) = ana_type_size[ANA_CFLOAT];
      complex_scalar_data(nsym).cf = malloc(complex_scalar_memory(nsym));
      if (!complex_scalar_data(nsym).cf)
	return cerror(ALLOC_ERR, nsym);
      symbol_class(nsym) = ANA_CSCALAR;
      complex_scalar_data(nsym).cf->real = value->cf.real;
      complex_scalar_data(nsym).cf->imaginary = value->cf.imaginary;
      break;
    case ANA_CDOUBLE:
      complex_scalar_memory(nsym) = ana_type_size[ANA_CDOUBLE];
      complex_scalar_data(nsym).cd = malloc(complex_scalar_memory(nsym));
      if (!complex_scalar_data(nsym).cd)
	return cerror(ALLOC_ERR, nsym);
      symbol_class(nsym) = ANA_CSCALAR;
      complex_scalar_data(nsym).cd->real = value->cd.real;
      complex_scalar_data(nsym).cd->imaginary = value->cd.imaginary;
      break;
  }
  return ANA_OK;
}
/*-----------------------------------------------------*/
int redef_string(int nsym, int len)
/* redefine symbol nsym to be a string with length len (excluding \0) */
{
 undefine(nsym);
 sym[nsym].class = ANA_STRING;
 sym[nsym].type = ANA_TEMP_STRING;
 allocate(string_value(nsym), len + 1, char);
 sym[nsym].spec.array.bstore = len + 1;
 return 1;
}
/*-----------------------------------------------------*/
int redef_array(int nsym, int ntype, int ndim, int *dims)
/* redefines symbol nsym to be an array of the given type, number of
  dimensions, and dimensions; or a scalar if <ndim> == 0 */
{                                /*redefine a symbol i to an array */
  int   mq, j;
  array	*h;
  pointer	p;

  if (ndim > MAX_DIMS)
    return cerror(N_DIMS_OVR, 0);
  if (ndim < 0)
    return cerror(ILL_DIM, 0);
  if (!ndim) {
    undefine(nsym);
    symbol_class(nsym) = ANA_SCALAR;
    scalar_type(nsym) = ntype;
    return 1;
  }
  mq = ana_type_size[ntype];
  for (j = 0; j < ndim; j++)
    mq *= dims[j];
  mq += sizeof(array); /*total memory required including header */
  /* before deleting, check the current size and use it if it matches,
  this avoids a lot of mallocing in loops */
  if (symbol_class(nsym) != ANA_ARRAY
      || mq != symbol_memory(nsym)) {
    undefine(nsym);
    symbol_memory(nsym) = mq;
    allocate(symbol_data(nsym), mq, char);
  }
  symbol_class(nsym) = ANA_ARRAY;
  array_type(nsym) = ntype;
  h = array_header(nsym);
  h->ndim = ndim;
  h->c1 = 0; h->c2 = 0; h->nfacts = 0;
  memcpy(h->dims, dims, ndim*sizeof(int));
  h->facts = NULL;                      /* no known facts */
  if (ntype == ANA_STRING_ARRAY) {
    /* a string array: set all elements to NULL */
    mq = array_size(nsym);
    p.sp = array_data(nsym);
    while (mq--)
      *p.sp++ = NULL;
  }
  return 1;
}
/*-----------------------------------------------------*/
int redef_array_extra_dims(int tgt, int src, enum Symboltype type, int ndim, int *dims)
{
  int *srcdims, srcndim;
  int tgtdims[MAX_DIMS];

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
int bytarr(int narg, int ps[])
/* create an array of I*1 elements */
{
 int	dims[MAX_DIMS];

 if (get_dims(&narg, ps, dims) != 1)
   return ANA_ERROR;
 return array_scratch(ANA_BYTE, narg, dims);
}
/*-----------------------------------------------------*/
int intarr(int narg, int ps[])
/* create an array of I*2 elements */
{
 int	dims[MAX_DIMS];

 if (get_dims(&narg, ps, dims) != 1)
   return ANA_ERROR;
 return array_scratch(ANA_WORD, narg, dims);
}
/*-----------------------------------------------------*/
int lonarr(int narg, int ps[])
/* create an array of I*4 elements */
{
 int	dims[MAX_DIMS];

 if (get_dims(&narg, ps, dims) != 1)
   return ANA_ERROR;
 return array_scratch(ANA_LONG, narg, dims);
}
/*-----------------------------------------------------*/
int fltarr(int narg, int ps[])
/* create an array of F*4 elements */
{
 int	dims[MAX_DIMS];

 if (get_dims(&narg, ps, dims) != 1)
   return ANA_ERROR;
 return array_scratch(ANA_FLOAT, narg, dims);
}
/*-----------------------------------------------------*/
int dblarr(int narg, int ps[])
/* create an array of I*1 elements */
{
 int	dims[MAX_DIMS];

 if (get_dims(&narg, ps, dims) != 1)
   return ANA_ERROR;
 return array_scratch(ANA_DOUBLE, narg, dims);
}
/*-----------------------------------------------------*/
int cfltarr(int narg, int ps[])
/* create a CFLOAT array */
{
  int	dims[MAX_DIMS];

  if (get_dims(&narg, ps, dims) != 1)
    return ANA_ERROR;
  return array_scratch(ANA_CFLOAT, narg, dims);
}
/*-----------------------------------------------------*/
int cdblarr(int narg, int ps[])
/* create a CDOUBLE array */
{
  int	dims[MAX_DIMS];

  if (get_dims(&narg, ps, dims) != 1)
    return ANA_ERROR;
  return array_scratch(ANA_CDOUBLE, narg, dims);
}
/*-----------------------------------------------------*/
int strarr(int narg, int ps[])
/* create an array of NULL string pointers */
{
 int	dims[MAX_DIMS], iq, size, n;
 char	**ptr;

 size = ps[0]? int_arg(ps[0]): 0;
 if (size < 0)
   return anaerror("Illegal negative string size", ps[0]);
 ps++;
 narg--;
 if (get_dims(&narg, ps, dims) != 1)
   return ANA_ERROR;
 iq = array_scratch(ANA_STRING_ARRAY, narg, dims);
 if (size) {			/* fill with specified size of whitespace */
   ptr = array_data(iq);
   n = array_size(iq);
   while (n--) {
     *ptr = malloc(size + 1);
     sprintf(*ptr, "%*s", size, " ");
     ptr++;
   }
 }
 return iq;
}
/*-----------------------------------------------------*/
int show_routine(internalRoutine *table, int tableLength, int narg, int ps[])
/* shows all routine names that contain a specified substring,
 or all of them */
{
 extern int	uTermCol;
 int	i, nOut = 0, nLine = uTermCol/16;
 char	*chars, *p;
 char	*name, **ptr;
 keyList	*keys;

 if (internalMode & 1)		/* /PARAMETERS */
 { if (symbol_class(*ps) != ANA_STRING)
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
   keys = table[i].keys;
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
int ana_show_subr(int narg, int ps[])
{
 return show_routine(subroutine, nSubroutine, narg, ps);
}
/*-----------------------------------------------------*/
int ana_show_func(int narg, int ps[])
{
 return show_routine(function, nFunction, narg, ps);
}
/*-----------------------------------------------------*/
int ana_switch(int narg, int ps[])
/* switches identity of two symbols.  We cannot just swap the names
   because then the connection between a particular name and a
   particular symbol number is broken.  We must swap the values instead.
   LS 9nov98 9oct2010*/
{
 int	sym1, sym2;
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
int ana_array(int narg, int ps[])
/* create an array of the specified type and dimensions */
{
 int	dims[MAX_DIMS], type;

 narg--;
 type = int_arg(*ps);
 if (!isLegalType(type))
   return cerror(ILL_TYPE, 0, type);
 if (isStringType(type))
   type = ANA_STRING_ARRAY;
 if (get_dims(&narg, ps + 1, dims) != 1)
   return ANA_ERROR;
 return array_scratch(type, narg, dims);
}
/*-----------------------------------------------------*/
int ana_assoc(int narg, int ps[])
/* returns an associated variable.
  syntax: assoc(lun, array [, offset]) */
{
 int	lun, result, iq, size;
 array	*h;

 lun = int_arg(*ps);
 iq = ps[1];
 CK_ARR(iq, 2);
 getFreeTempVariable(result);
 sym[result].class = ANA_ASSOC;
 sym[result].type = sym[iq].type;
 sym[result].line = curLineNumber;
 h = HEAD(iq);
 size = sizeof(array);
 if (narg >= 3) size += sizeof(int);
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
int ana_rfix(int narg, int ps[])
/* returns an I*4 version of the argument, rounded to the nearest
 integer if necessary */
{
 int	nsym, type, size, *trgt, i, result;
 pointer	src;
 double	temp;
 array	*h;

 nsym = *ps;
 if ((type = sym[nsym].type) == ANA_LONG) return nsym; /* already of proper type */
 switch (sym[nsym].class)
 { case ANA_SCALAR:
     src.l = &sym[nsym].spec.scalar.l;  size = 1;
     result = scalar_scratch(ANA_LONG);  trgt = &sym[result].spec.scalar.l;  break;
   case ANA_STRING:
     result = scalar_scratch(ANA_LONG);  type = ANA_DOUBLE;
     temp = atof((char *) sym[nsym].spec.array.ptr);  src.d = &temp;
     size = 1;  trgt = &sym[result].spec.scalar.l;  break;
   case ANA_ARRAY:
     result = array_clone(nsym, ANA_LONG);  h = HEAD(nsym);
     GET_SIZE(size, h->dims, h->ndim);  src.l = LPTR(h);
     trgt = LPTR(HEAD(result));  break;
   default: return cerror(ILL_CLASS, nsym); }
 		/* now convert */
 switch (type)
 { case ANA_BYTE:
     while (size--) *trgt++ = (int) *src.b++;  break;
   case ANA_WORD:
     while (size--) *trgt++ = (int) *src.w++;  break;
   case ANA_FLOAT:
     while (size--)
     { *trgt++ = (int) (*src.f + ((*src.f >= 0)? 0.5: -0.5));
       src.f++; }
     break;
   case ANA_DOUBLE:
     while (size--)
     {*trgt++ = (int) (*src.d + ((*src.d >= 0)? 0.5: -0.5));
      src.d++; }
     break; }
 return result;
}
/*-----------------------------------------------------*/
int ana_echo(int narg, int ps[])
/* turn on echoing of input lines from non-keyboard sources */
{
 extern int	echo;
 
 if (narg >= 1) echo = int_arg(*ps); else echo = 1;
 return 1;
}
/*-----------------------------------------------------*/
int ana_noecho(int narg, int ps[])
/* turn on echoing of input lines from non-keyboard sources */
{
 extern int	echo;

 echo = 0;
 return 1;
}
/*-----------------------------------------------------*/
int ana_batch(int narg, int ps[])
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
int ana_record(int narg, int ps[])
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
      recordFile = fopen(expand_name(file, ".ana"), "a");  /* open new file */
    } else {
      if (!recordFile)
	recordFile = fopen("record.ana", "a");
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
int step = 0;
int ana_step(int narg, int ps[])
{
  int	i = 1;

  if (narg) i = int_arg(*ps);
  if (i) { step = i;  printf("Commence stepping at level %d\n", step); }
  else { step = 0;  puts("No stepping"); }
  return 1;
}
/*-------------------------------------------------------------------------*/
int ana_varname(int narg, int ps[])
/* returns the name of the variable */
{
  char	*name;
  int	iq;

  name = varName(*ps);
  iq = string_scratch(strlen(name));
  strcpy((char *) sym[iq].spec.array.ptr, name);
  return iq;
}
/*-------------------------------------------------------------------------*/
int namevar(int symbol, int safe)
/* if safe == 0: returns the variable that goes with the name string */
/* in the current context, or an error if not found. */
/* if safe == 1: as for safe == 0, but if not found and if at main level */
/* then such a variable is created at the main level. */
/* if safe == 2: as for safe == 0, but search main level only. */
/* if safe == 3: as for safe == 1, but search and create at main level. */
{
  char	*name;
  int	iq, context;

  if (symbol_class(symbol) != ANA_STRING)
    return cerror(NEED_STR, symbol);
  name = string_value(symbol);
  strcpy(line, name);
  if (!isalpha((byte) *name) && *name != '$' && *name != '!')
    return anaerror("Illegal symbol name: %s", symbol, name);
  for (name = line; *name; name++)
  { if (!isalnum((byte) *name))
    return anaerror("Illegal symbol name: %s", symbol, name);
    *name = toupper(*name); }
  safe = safe & 3;
  context = (safe >= 2)? 0: curContext;
  iq = lookForVarName(line, context); /* seek symbol */
  if (iq < 0)
  { if ((safe & 1) == 0 || ((safe & 1) == 1 && context))
      return anaerror("Could not find variable %s %s %s\n", 0, line,
                   context? "in": "at",
		   context? varName(context): "main level");
    if ((safe & 1) == 1)	/* create at main level */
    { iq = installString(line);
      iq = findVar(iq, 0); }
  }
  return iq;
}
/*-------------------------------------------------------------------------*/
char *keyName(internalRoutine *routine, int number, int index)
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
  { if (!isdigit((int) **keys)) index--;
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
int ana_set(int narg, int ps[])
/* SET[,/SHOWALLOC,/WHITEBACKGROUND,/INVIMCOORDS,/SET,/RESET,/ZOOM]
 governs aspects of the behaviour of various routines.  LS 11mar98 */
{
  extern int	setup;
#ifdef X11
  char	*string;
  char	*visualNames[] = { "StaticGray", "StaticColor", "TrueColor",
			   "GrayScale", "PseudoColor", "DirectColor",
			   "SG", "SC", "TC", "GS", "PC", "DC",
			   "GSL", "CSL", "CSI", "GDL", "CDL", "CDI" };
  int	visualClassCode[] = { StaticGray, StaticColor, TrueColor,
			      GrayScale, PseudoColor, DirectColor };
  int	setup_x_visual(int), i;
  extern int	connect_flag;
  extern Visual	*visual;
#endif

  if (narg) {
    if (*ps) {			/* VISUAL */
#ifdef X11
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
	return anaerror("Invalid visual", *ps);
      }
      i = visualClassCode[i % 6];
      if (connect_flag) {
	if (i == visual->class)
	  return ANA_OK;	/* already use the selected visual class */
	else
	  return anaerror("Already using a %s visual.", 0,
		       visualNames[visual->class]);
      }
      if (setup_x_visual(i) == ANA_ERROR)
	return ANA_ERROR;
#else
      return anaerror("Need X11 package to set the visual", 0);
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
#ifdef X11
      if (setup & 32)
	puts("/ZOOM: Automatic image zoom.");
#endif
      if (setup & 256)
	puts("/ALLOWPROMPTS: Allow ANA prompts at the beginning of input lines.");
      if (setup & 512)
	puts("/XSYNCHRONIZE: Graphics output occurs immediately.");
      if (setup & 1024)
	puts("/PARSEWARN: Parser warnings.");
    } else if (!narg)
      puts("No SETUP selections.");
  }
  allowPromptInInput = (setup & 256)? 1: 0;
#ifdef X11
  if (internalMode & 2048)
    xsynchronize(setup & 512? 1: 0);
#endif
  return ANA_ONE;
}
/*-------------------------------------------------------------------------*/
void zapTemp(int symbol)
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
int copyEvalSym(int source)
  /* evaluates <source> and returns it in a temp */
{
  int	result, target;

  result = eval(source);
  if (result < 0) return -1;	/* some error */
  if (result != source) zapTemp(source);
  if (isFreeTemp(result)) return result;
  target = nextFreeTempVariable();
  symbol_class(target) = ANA_UNDEFINED; /* else may get trouble when */
				  /* updateIndices() is called in */
				  /* ana_replace() */
  if (target < 0) return -1;	/* some error */
  if (ana_replace(target, result) < 0)
    return -1;			/* some error */
  return target;
}
/*-------------------------------------------------------------------------*/
int (*ana_converts[10])(int, int []) = {
  ana_byte, ana_word, ana_long, ana_float, ana_double, NULL,
  NULL, NULL, ana_cfloat, ana_cdouble
};
int getNumerical(int iq, int minType, int *n, pointer *src, char mode,
		 int *result, pointer *trgt)
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
  int	type;

  if (!symbolIsNumerical(iq))
    return cerror(ILL_CLASS, iq);

  if (symbol_class(iq) == ANA_SCAL_PTR)
    iq = dereferenceScalPointer(iq);

  type = symbol_type(iq);
  if (((mode & GN_UPGRADE) && type < minType)
      || ((mode & GN_EXACT) && type != minType)) {
    if (mode & GN_UPDATE)
      iq = ana_converts[minType](1, &iq);
    type = minType;
  }

  switch (symbol_class(iq)) {
    case ANA_SCALAR:
      (*src).b = &scalar_value(iq).b;
      *n = 1;
      if (trgt) {
	*result = scalar_scratch(type);
	(*trgt).b = &scalar_value(*result).b;
      } else if (result)
	*result = iq;
      break;
    case ANA_ARRAY: case ANA_CARRAY:
      (*src).b = (byte *) array_data(iq);
      *n = array_size(iq);
      if (trgt) {
	*result = array_clone(iq, type);
	(*trgt).b = (byte *) array_data(*result);
      } else if (result)
	*result = iq;
      break;
    case ANA_CSCALAR:
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
int getSimpleNumerical(int iq, pointer *data, int *nelem)
/* returns pointer in <*data> to data in symbol <iq>, and in <*nelem> the
   number of data elements, and 1 as function return value, if <iq>
   is of numerical type.  Otherwise, returns -1 as function value, 0 in
   <*nelem>, and nothing in <*data>.  LS 14apr97 */
{
  switch (symbol_class(iq)) {
    case ANA_SCAL_PTR:
      iq = dereferenceScalPointer(iq);
    case ANA_SCALAR:
      (*data).b = &scalar_value(iq).b;
      *nelem = 1;
      break;
    case ANA_ARRAY:
      (*data).l = array_data(iq);
      *nelem = array_size(iq);
      break;
    default:
      *nelem = 0;
      return cerror(ILL_CLASS, iq);
  }
  return 1;
}
/*-------------------------------------------------------------------------*/
int file_map_size(int symbol)
{
 int	i, n, *p, size;

  p = file_map_dims(symbol);
  n = file_map_num_dims(symbol);
  size = 1;
  for (i = 0; i < n; i++)
    size *= *p++;
  return size;
}
/*-------------------------------------------------------------------------*/
int ana_pointer(int narg, int ps[])
/* POINTER,pointer,target [,/FUNCTION, /SUBROUTINE, /INTERNAL, /MAIN] */
/* makes named variable */
/* <pointer> point at named variable <target>.  If <pointer> is a pointer */
/* already, then its old link is broken before the new one is put in */
/* place.  <target> may be an expression as long as it evaluates to a */
/* named variable.  LS 11feb97 */
{
  int	iq;
  char	*name, *name2, *p;
  
  if (ps[0] >= TEMPS_START)
    return anaerror("Intended pointer is not a named variable", ps[0]);
  if (symbol_class(ps[0]) != ANA_POINTER)
  { undefine(ps[0]);
    symbol_class(ps[0]) = ANA_POINTER;
    transfer_is_parameter(ps[0]) = 0;
    transfer_temp_param(ps[0]) = 0; }
  iq = eval(ps[1]);
  if (internalMode)		/* target must be a string variable */
  { if (symbol_class(iq) != ANA_STRING)
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
	return ANA_ERROR;
    case 0:			/* variable */
      if (iq >= TEMPS_START)
	return anaerror("Expression does not reduce to a named variable", ps[1]);
      transfer_target(ps[0]) = iq;
      break;
    case 1:			/* function */
    case 2:			/* subroutine */
      if (internalMode & 4)	/* /INTERNAL */
      { iq = findInternalName(name2, (internalMode & 1)? 0: 1);
	Free(name2);
	if (iq < 0)
	  return ANA_ERROR;
	symbol_class(ps[0]) = ANA_FUNC_PTR;
	func_ptr_routine_num(ps[0]) = -iq;
	func_ptr_type(ps[0]) = (internalMode & 1)? ANA_FUNCTION:
	ANA_SUBROUTINE; }
      else
      { iq = findName(name2, internalMode == 1? funcHashTable: subrHashTable,
		      0);
	Free(name2);
	if (iq < 0)		/* some error occurred */
	  return iq;
	symbol_class(ps[0]) = ANA_FUNC_PTR;
	symbol_class(iq) =
	  (internalMode == 1)? ANA_DEFERRED_FUNC: ANA_DEFERRED_SUBR;
	deferred_routine_filename(iq) = strsave(name);
	if (iq >= TEMPS_START)
	  return anaerror("Pointer target is not a named variable", ps[1]);
	transfer_target(ps[0]) = iq; }
      break;
    default:
      return cerror(INCMP_KEYWORD, 0); }
  return 1;
}
/*-------------------------------------------------------------------------*/
int ana_symbol(int narg, int ps[])
/* SYMBOL('name') returns the variable with the <name> in the current */
/* context, or if not found and if at the main level, then creates */
/* such a variable and returns it.  SYMBOL('name',/MAIN) checks */
/* the main execution level instead.  LS 11feb97 */
{
  return namevar(ps[0], (internalMode & 1) == 1? 3: 1);
}
/*-------------------------------------------------------------------------*/
int stringpointer(char *name, int type)
/* seeks <name> as user-defined function (SP_USER_FUNC), user-defined */
/* subroutine (SP_USER_SUBR), variable (SP_VAR), internal function */
/* (SP_INT_FUNC), or internal subroutine (SP_INT_SUBR).  Also: */
/* any function (SP_FUNC = SP_USER_FUNC then SP_INT_FUNC), any subroutine */
/* (SP_SUBR = SP_USER_SUBR then SP_INT_SUBR), and any at all (SP_ANY). */
/* LS 1apr97 */
{
  int	n;
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
int ana_show_temps(int narg, int ps[])
/* a routine for debugging ANA.  It shows the temporary variables that */
/* are currently defined.  LS 2mar97 */
{
  int	i;

  setPager(0);
  for (i = TEMPS_START; i < TEMPS_END; i++) {
    if (symbol_class(i) != ANA_UNUSED) {	/* defined */
      printf("%4d [%5d] ", i, symbol_context(i));
      symdumpswitch(i, I_TRUNCATE | I_LENGTH);
    }
  }
  resetPager();
  return 1;
}
/*-------------------------------------------------------------------------*/
int routineContext(int nsym)
/* returns the number of the user-defined subroutine or function that
   symbol <nsym> is in, or 0 if at the main level.  LS 18apr97 */
{
  while (symbol_context(nsym))
  { nsym = symbol_context(nsym);
    if (symbol_class(nsym) == ANA_SUBROUTINE
	|| symbol_class(nsym) == ANA_FUNCTION)
      return nsym; }
  return 0;
}
/*-------------------------------------------------------------------------*/
/* NUMBERS

   ANA interprets strings as explicit numbers as indicated below.
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
 
void read_a_number(char **buf, scalar *value, int *type)
/* reads the number at <*buf>, puts it in <*value> and its data type
   in <*type>, and modifies <*buf> to point just after the detected
   value.  NOTE: if the type is integer (BYTE, WORD, LONG), then the
   value is returned in value->l; if the type is floating-point
   (FLOAT, DOUBLE), then the value is returned in value->d.  if an I
   follows the number, then the type is complex (CFLOAT, CDOUBLE) and
   the value (transformed to real) is returned in value.d.
   So, the union member that the value is in does not necessarily correspond
   exactly to the <*type>: e.g., a BYTE number gets its value returned
   in value->l and *type set to ANA_BYTE.  LS 17sep98 */
{
  int	base = 10, kind, sign;
  char	*p, *numstart, c, ce, *p2;

  p = *buf;
  *type = ANA_LONG;		/* default */
  /* skip whitespace */
  while (!isdigit((int) *p) && !strchr("+-.", (int) *p))
    p++;

  /* skip sign, if any */
  if (strchr("+-", *p)) {
    sign = (*p == '-')? -1: 1;
    p++;
  } else sign = 1;
  numstart = p;
  
  /* |oooO[{BWLI}] */
  /* |ddd[{BWLI}] */
  /* |0X[hhh][{WLI}] */
  /* |ddd{SH}[ddd:]*[ddd][.ddd][{DE}][I] */
  /* |ddd.[ddd][I] */
  /* |.ddd[{DE}[[{+-}]ddd]][I] */
  /* |ddd[.ddd][{DE}[[{+-}]ddd]][I] */
  /* ^ we are here */
  /* NOTE: vertical bars | are indicated just before the last read */
  /* character; <c> contains the last read character, which must match */
  /* one of the characters just after a vertical bar. */

  while (isodigit((int) *p))
    p++;
  
  /* ooo|O[{BWLI}] */
  /*  dd|d[{BWLI}] */
  /* ddd|{BWLI} */
  /* ddd| */
  /*   0|X[hhh][{BWLI}] */
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
    /* oooO|[{BWLI}]
	   ^ we are here */
  } else
    /* skip remaining digits, if any */
    while (isdigit((int) *p))
      p++;

  /* oooO|{BWLI} */
  /* oooO| */
  /*  ddd|{BWLI} */
  /*  ddd| */
  /*    0|X[hhh][{BWLI}] */
  /*  ddd|{ST}[ddd:]*[ddd][.ddd][{DE}][I] */
  /*  ddd|.[ddd][I] */
  /*     |.ddd[{DE}[[{+-}]ddd]][I] */
  /*  ddd|{DE}[[{+-}]ddd][I] */
  /*     ^ we are here */

  switch (toupper(*p)) {
    case 'W': case 'L': case 'B': case 'I': /* done with the digits */
      /* ddd|{BWLI} */
      /*    ^ we are here */
      break;
    case 'X':			/* hex number? */
      if (p == numstart + 1 && numstart[0] == '0') { /* yes, a hex number */
	/* 0|X[hhh][{WLI}] */
	/*  ^ we are here */
	p++;
	while (isxdigit((int) *p))
	  p++;
	base = 16;
	/*  0X[hhh]|[{WLI}] */
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
      while (isdigit((int) *p)) {
	while (isdigit((int) *p))
	  p++;
	/* ddd{SH}ddd|:[ddd:]*[ddd][.ddd][{DE}][I] */
	/* ddd{SH}ddd|.[ddd][{DE}][I] */
	/* ddd{SH}ddd|{DE}[I] */
	/* ddd{SH}ddd|I */
	/*           ^ we are here */
	if (*p == '.') {	/* a float number: the last entry */
	  p++;
	  while (isdigit((int) *p)) /* find the rest */
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
	  *type = (kind == 'D')? ANA_CDOUBLE: ANA_CFLOAT;
	  p++;			/* skip */
	} else 
	  *type = (kind == 'D')? ANA_DOUBLE: ANA_FLOAT;
      } else if (kind == 'I') {
	*type = ANA_CFLOAT;
	p++;
      } else
	*type = ANA_FLOAT;
      value->d *= sign;	/* put the sign back on it */
      *buf = p;
      return;
    case '.': case 'D': case 'E':
      /*    |.ddd[{DE}[[{+-}]ddd]][I] */
      /* ddd|.[ddd][{DE}[[{+-}]ddd]][I] */
      /* ddd|{DE}[[{+-}]ddd][I] */
      /*    ^ we are here */
      if (*p == '.') {
	p++;			/* skip . */
	while (isdigit((int) *p))
	  p++;
      }
      /* ddd.[ddd]|[I] */
      /* ddd[.ddd]|{DE}[[{+-}]ddd][I] */
      /*      .ddd|[I] */
      /*      .ddd|{DE}[[{+-}]ddd][I] */
      /*          ^ we are here */
      kind = toupper(*p);
      p2 = NULL;
      *type = ANA_FLOAT;	/* default */
      if (kind == 'D' || kind == 'E') {
	if (kind == 'D') {
	  ce = *p;
	  *p = 'E';		/* temporarily, so atof can read it */
	  p2 = p;
	  *type = ANA_DOUBLE;
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
	while (isdigit((int) *p))
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
	*type = (*type == ANA_DOUBLE)? ANA_CDOUBLE: ANA_CFLOAT;
	p++;			/* skip the I */
      }
      *buf = p;
      return;
  }

  /*    oooO|[{BWLI}] */
  /*     ddd|[{BWLI}] */
  /* 0X[hhh]|[{BWLI}] */
  /*        ^ we are here */
  kind = toupper(*p);
  c = *p;
  *p = '\0';			/* temporary end */
  /* NOTE: have to use strtoul() instead of strtol() because strtol() */
  /* -- at least on SGI Irix6.3 -- does not accept numbers with their most */
  /* significant byte set (such as 0xc9460fc0), even though printf() */
  /* has no trouble generating such numbers.  I.e., t,'%x',-0x35b9f040 */
  /* yielded c9460fd0 but t,'%x',0xc9460fd0 when using strtol() yields */
  /* 0xffffffff.  LS 11nov99 */
  value->l = strtoul(numstart, NULL, base)*sign;
  *p = c;
  switch (kind) {
    case 'B':
      *type = ANA_BYTE;
      p++;
      break;
    case 'W':
      *type = ANA_WORD;
      p++;
      break;
    case 'L':
      *type = ANA_LONG;
      p++;
      break;
    case 'I':
      value->d = value->l;
      *type = ANA_CFLOAT;
      p++;
      break;
    default:
      *type = ANA_LONG;		/* default */
      break;
  }
  *buf = p;
}
/*-------------------------------------------------------------------------*/
void read_a_number_fp(FILE *fp, scalar *value, int *type)
/* reads the number at <*fp>, puts it in <*value> and its data type
   in <*type>.  Other than the source of the data, exactly the same as
   read_a_number().
   NOTE: if the type is integer (BYTE, WORD, LONG), then the
   value is returned in value->l; if the type is floating-point
   (FLOAT, DOUBLE), then the value is returned in value->d.  if an I
   follows the number, then the type is complex (CFLOAT, CDOUBLE) and
   the value (transformed to real) is returned in value.d.
   So, the union member that the value is in does not necessarily correspond
   exactly to the <*type>: e.g., a BYTE number gets its value returned
   in value->l and *type set to ANA_BYTE.  LS 17sep98 */
/* Fixed reading of numbers with exponents.  LS 11jul2000 */
{
  int	base = 10, kind, sign, ch;
  char	*p, *numstart, c, ce, *p2;

  *type = ANA_LONG;		/* default */
  /* skip non-digits, non-signs */
  while ((ch = nextchar(fp)) != EOF && !isdigit(ch) && !strchr("+-", ch));
  if (ch == EOF) {		/* end of file; return LONG 0 */
    value->l = 0;
    return;
  }

  /* skip sign, if any */
  if (strchr("+-", ch)) {
    sign = (ch == '-')? -1: 1;
    ch = nextchar(fp);
    if (ch == EOF) {
      value->l = 0;
      return;
    }
  } else sign = 1;

  p = numstart = curScrat;
  *p++ = ch;
  do
    *p++ = nextchar(fp);
  while (isodigit((int) p[-1]));
  ch = p[-1];

  if (ch == EOF) {
    value->l = 0;
    return;
  }
  if (ch == 'O') {		/* octal integer */
    base = 8;
    ch = nextchar(fp);
    if (ch == EOF) {
      value->l = 0;
      return;
    }
  } else if (isdigit(ch)) {	/* not an octal integer */
    /* read remaining digits, if any */
    do
      *p++ = nextchar(fp);
    while (isdigit((int) p[-1]));
    ch = p[-1];
  }
		   
  switch (toupper(ch)) {
    case 'W': case 'L': case 'B': case 'I': /* done with the digits */
      break;
    case 'X':			/* hex number? */
      if (p == numstart + 2 && numstart[0] == '0') { /* yes, a hex number */
	do
	  *p++ = nextchar(fp);
	while (isxdigit((int) p[-1]));
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
	value->l = (int) value->d;
	return;
      }
      while (isdigit(ch)) {
	while (isdigit(ch)) {
	  *p++ = ch;
	  ch = nextchar(fp);
	  if (ch == EOF) {
	    value->l = (int) value->d;
	    return;
	  }
	}
	if (ch == '.') {	/* a float number: the last entry */
	  do
	    *p++ = nextchar(fp);
	  while (isdigit((int) p[-1])); /* find the rest */
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
	  *type = (kind == 'D')? ANA_CDOUBLE: ANA_CFLOAT;
	} else 
	  *type = (kind == 'D')? ANA_DOUBLE: ANA_FLOAT;
      } else if (kind == 'I') {
	*type = ANA_CFLOAT;
      } else
	*type = ANA_FLOAT;
      value->d *= sign;	/* put the sign back on it */
      return;
    case '.': case 'D': case 'E':
      if (ch == '.') {
	do
	  *p++ = nextchar(fp);
	while (isdigit((int) p[-1]));
      }
      kind = toupper(p[-1]);
      p2 = NULL;
      *type = ANA_FLOAT;	/* default */
      if (kind == 'D' || kind == 'E') {
	if (kind == 'D') {
	  p[-1] = 'E';		/* so atof can read it */
	  *type = ANA_DOUBLE;
	}
	ch = nextchar(fp);
	if (ch == '+' || ch == '-') /* a signed exponent */
	  *p++ = ch;
	else
	  unnextchar(ch, fp);
	do
	  *p++ = nextchar(fp);
	while (isdigit((int) p[-1]));
	kind = toupper(p[-1]); /* must look for I */
      }
      *p = '\0';		/* temporary end */
      value->d = atof(numstart)*sign;
      if (kind == 'I') {
	*type = (*type == ANA_DOUBLE)? ANA_CDOUBLE: ANA_CFLOAT;
      }
      /* the last character we read was not part of the number. */
      /* if it is not a newline, then we put it back in the stream. */
      if (kind != '\n' && unnextchar(kind, fp) != kind)
	puts("unnextchar() unsuccessful");
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
  /* significant byte set (such as 0xc9460fc0), even though printf() */
  /* has no trouble generating such numbers.  I.e., t,'%x',-0x35b9f040 */
  /* yielded c9460fd0 but t,'%x',0xc9460fd0 when using strtol() yields */
  /* 0xffffffff.  LS 11nov99 */
  value->l = strtoul(numstart, NULL, base)*sign;
  switch (kind) {
    case 'B':
      *type = ANA_BYTE;
      break;
    case 'W':
      *type = ANA_WORD;
      break;
    case 'L':
      *type = ANA_LONG;
      break;
    case 'I':
      value->d = value->l;
      *type = ANA_CFLOAT;
      break;
    default:
      *type = ANA_LONG;		/* default */
      break;
  }
}
/*-------------------------------------------------------------------------*/
#ifdef FACTS
void *seekFacts(int symbol, int type, int flag)
/* seeks facts of the indicated <type> and <flag> associated with the */
/* <symbol>. */
/* Returns a pointer to the facts, if found, or else returns NULL. */
/* LS 6apr99 */
{
  byte	n;
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
void *setFacts(int symbol, int type, int flag)
/* seeks facts of the indicated <type> associated with the <symbol>. */
/* Returns a pointer to the facts, if found.  Otherwise, allocates a */
/* new slot for the facts and returns a pointer to it. */
/* sets the appropriate <flag>. */
/* Returns NULL if unsucessful.  LS 6apr99 */
{
  arrayFacts	*facts;
  int	n;

  if (!symbolIsNumericalArray(symbol)) /* not a numerical symbol */
    return NULL;
  
  if (array_facts(symbol)) {
    facts = seekFacts(symbol, type, ANA_ANY_FACT);
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
void deleteFacts(int symbol, int type)
{
  int	n, nf;
  arrayFacts	*facts;

  if (!symbolIsNumericalArray(symbol))
    return;
  
  if (array_facts(symbol)) {
    facts = seekFacts(symbol, type, ANA_ANY_FACT);
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
static int *stack, stack_sentinel;
void newStack(int sentinel)
{
  stack = (int *) curScrat;
  *stack++ = stack_sentinel = sentinel;
}
/*-----------------------------------------------------*/
void push(int value)
{
  *stack++ = value;
}
/*-----------------------------------------------------*/
int pop(void)
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

int nextchar(FILE *fp) {
  /* returns the next char from the indicated file pointer */
  /* if fp == stdin (i.e., reading from the keyboard), then a special */
  /* data input buffer is used, with all command line editing except */
  /* history buffer stuff enabled.  This can be used as an alternative */
  /* for fgetc(). LS 29mar2001 */

  if (fp == stdin) {
    if (!keyboard.ptr) {
      keyboard.buffer = malloc(BUFSIZE);
      keyboard.ptr = keyboard.buffer;
      *keyboard.ptr = '\0';
    }
    if (!*keyboard.ptr) {	/* no more input */
      if (keyboard.ptr == keyboard.buffer) /* we must ask for more */
	while (!*keyboard.ptr) {
	  FILE *is;
	  is = inputStream;
	  inputStream = stdin;
	  getNewLine(keyboard.buffer, "dat>", 0);
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
int unnextchar(int c, FILE *fp) {
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
