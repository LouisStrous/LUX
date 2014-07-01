/* This is file ident.c.

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
/* ident.c: a number of routines for identifying pieces of LUX code */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include "action.h"

extern char	*curScrat, *binOpSign[];
char	*fmt_integer, *fmt_float, *fmt_string, *fmt_complex, *fmt_time;
FILE	*outputStream;

/*-------------------------------------------------------------------*/
char *symbolProperName(Int symbol)
/* returns the proper name of the symbol, if any, or NULL */
{
  hashTableEntry	**hashTable, *hp;
  Int	hashValue;

  if (symbol < 0 || symbol >= NAMED_END) /* out of range */
    return NULL;
  switch (symbol_class(symbol)) {
    case LUX_SUBROUTINE: case LUX_DEFERRED_SUBR:
      hashTable = subrHashTable;
      break;
    case LUX_FUNCTION: case LUX_DEFERRED_FUNC:
      hashTable = funcHashTable;
      break;
    case LUX_BLOCKROUTINE: case LUX_DEFERRED_BLOCK:
      hashTable = blockHashTable;
      break;
    default:			/* must be a regular variable */
      hashTable = varHashTable;
      break;
  }

  hashValue = sym[symbol].xx - 1;
  if (hashValue < 0 || hashValue > HASHSIZE) /* illegal hash value */
    return NULL;
  hp = hashTable[hashValue];	/* start of appropriate name list */
  while (hp) {			/* not at end of list */
    if (hp->symNum == symbol)	/* found symbol */
      return hp->name;
    hp = hp->next;		/* go to next */
  }
  return NULL;			/* did not find it */
}
/*---------------------------------------------------------------------*/
Int fmt_entry(formatInfo *fmi)
/* finds the next format entry in format string <fmi->current> and returns
   information in other members of <fmi>.  These members are:

   fmi->width: the field width extracted from the format entry, if any, or
           else -1 
   fmi->precision: the precision, if any, or else -1
   fmi->count: the repeat count, if any, or else -1
   fmi->flags: the modifier flags; the logical sum ("or") of zero or more
           of the following: FMT_LEFT_JUSTIFY (-), FMT_ALWAYS_SIGN
           (+), FMT_ZERO_PAD (0), FMT_ALTERNATIVE (#),
           FMT_POSITIVE_BLANK ( ), FMT_SUPPRESS (*), FMT_SMALL (h),
	   FMT_BIG (l), FMT_MIX (_), FMT_MIX2 (=), FMT_SEPARATE (:).
   fmi->start: points at the % that introduces the format.
   fmi->spec_char: a pointer to the format specifier character.
   fmi->repeat: points at the start of the repeat count, if any, or
           else equal to fmi->plain
   fmi->plain: a pointer to the start of the plain text, if any
   fmi->end:   a pointer to one beyond the last character in the plain text,
           if any (if the _ or = modifiers are specified).
   fmi->next: a pointer to the beginning of the next format entry, if any,
           or else to one beyond the end of the current entry (the \0).
   fmi->only_whitespace: non-zero if the format consists of whitespace only.
   LS 16nov98 23jan99
   Return values: one of
   FMT_ERROR	illegal format
   FMT_PLAIN	plain text only; no % format entry
   FMT_INTEGER	integer
   FMT_FLOAT	floating point
   FMT_TIME	sexagesimal
   FMT_DATE     date
   FMT_STRING	string
   FMT_COMPLEX	complex number

  */
{
  Int	type, k;
  char	*p, *p2;

  if (!fmi->current || !*fmi->current) { /* no format */
    fmi->start = fmi->next = NULL;
    fmi->type = FMT_ERROR;
    puts("Illegal format string");
    return FMT_ERROR;
  }

  if (fmi->current == fmi->format)  /* at beginning of whole format string */
    /* initialize grouping info */
    fmi->active_group = -1;

  p = fmi->start = fmi->current;
  /* find the next % format entry or plain text, treating any format
     group beginnings along the way. */
  do {
    if (*p == '%' && p[1] != '%') { /* format specification or %( %) */
      if (p[1] == '(') {		/* group beginning */
	p += 2;
	if (++(fmi->active_group) == MAXFMT) {
	  puts("Format groups nested too deeply");
	  return FMT_ERROR;	/* too may active groups */
	}
	fmi->group_start[fmi->active_group] = p;
	fmi->group_count[fmi->active_group] = -1; /* repeat count yet unknown */
	fmi->start = p;
	continue;
      }
    }
  } while (0);

  /* now we're pointing either at a regular format specification,
     or at a format group end or at the beginning of some plain text */
  type = FMT_PLAIN;		/* default */
  fmi->flags = 0;
  if (*p == '%' && p[1] != '%' && p[1] != ')') { /* a format specification */
    uint8_t done = 0;
    p++;			/* skip the initial % */
    
    /* service any modifier */
    do {
      switch (*p++) {
	case '*':
	  fmi->flags |= FMT_SUPPRESS;
	  break;
	case '-':
	  fmi->flags |= FMT_LEFT_JUSTIFY;
	  break;
	case '+':
	  fmi->flags |= FMT_ALWAYS_SIGN;
	  break;
	case '0':
	  fmi->flags |= FMT_ZERO_PAD;
	  break;
	case '#':
	  fmi->flags |= FMT_ALTERNATIVE;
	  break;
	case ' ':
	  fmi->flags |= FMT_POSITIVE_BLANK;
	  break;
	case '_':
	  fmi->flags |= FMT_MIX;
	  break;
	case '=':
	  fmi->flags |= FMT_MIX2;
	  break;
	default:
	  p--;			/* we went one too far */
	  done = 1;
	  break;
      }
    } while (!done);

    if (isdigit((uint8_t) *p))
      fmi->width = strtol(p, &p, 10); /* find & skip the number */
    else
      fmi->width = -1;		/* no explicit width */

    /* look for the precision */
    if (*p == '.') 		/* the precision */
      fmi->precision = strtol(p + 1, &p, 10); /* find and skip */
    else
      fmi->precision = -1;	/* no explicit precision */

    /* more modifiers? */
    do {
      switch (*p++) {
	case 'h':
	  fmi->flags |= FMT_SMALL;
	  continue;
	case 'l':
	  fmi->flags |= FMT_BIG;
	  continue;
	default:
	  p--;			/* we went one too far */
	  break;
      }
    } while (0);

    /* now look for the specification and deduce the type */
    switch (*p) {
    case 'd': case 'i': case 'o': case 'x': case 'X':
      type = FMT_INTEGER;
      break;
    case 'e': case 'E': case 'f': case 'g': case 'G':
      type = FMT_FLOAT;
      break;
    case 'J':
      type = FMT_DATE;
      break;
    case 'T':
      type = FMT_TIME;
      break;
    case 's': case 'S':
      type = FMT_STRING;
      break;
    case 'z':
      type = FMT_COMPLEX;
      break;
    case '[':
      type = FMT_STRING;
      break;
    default:
      printf("Illegal format entry, %s\n", fmi->current);
      fmi->type = FMT_ERROR;
      return FMT_ERROR;	/* illegal type */
    }
    fmi->spec_char = p++;		/* record this position */
    if (p[-1] == '[') {
      /* we make p point at the closing ] */
      /* in '[]...]' and '[^]....]', the first ] is a regular character */
      /* and doesn't indicate the end of the scan set */
      if (p[1] == ']')
	p += 2;
      else if (p[1] == '^' && p[2] == ']')
	p += 3;
      while (*p && *p != ']')
	p++;
      p++;			/* point one beyond the closing ] */
    }
  
    fmi->repeat = p;		/* record this position */

    /* look for a repeat count */
    fmi->count = -1;		/* default */
    if (isdigit((uint8_t) *p)) {	/* maybe a repeat count */
      p2 = p + 1;
      while (isdigit((uint8_t) *p2))
	p2++;
      if (*p2 == '#') {		/* yes, a repeat count */
	fmi->count = strtol(p, &p, 10);
	p++;			/* skip the # */
      }
    }
  } /* end of if (*p == '%' && p[1] != '%' && p[1] != ')') */

  fmi->plain = p;
  /* now we are looking at plain text or a group end */
  if (*p != '%' || p[1] != ')')	{ /* we have plain text */
    if (type == FMT_PLAIN || 
	(fmi->flags & (FMT_MIX | FMT_MIX2))) { /* plain text is allowed */
      do {
	p2 = strchr(p, '%');
	if (p2 && p2[1] == '%')	/* a %% escape sequence */
	  p2 += 2;		/* skip */
      } while (p2 && *p2 && *p2 != '%');
      p = (p2 && *p2)? p2: p + strlen(p);
    }
  }
  fmi->end = p;
 
  if (*p == '%' && p[1] == ')') { /* a group end */
    if (fmi->active_group < 0) { /* group ending but no corresponding
				    group beginning */
      puts("Unbalanced format group end");
      fmi->type = FMT_ERROR;
      return FMT_ERROR;
    }
    fmi->end = p;
    p += 2;
    if (fmi->group_count[fmi->active_group] == -1) {
      /* we don't have a repeat count for this grouping yet */
      fmi->group_count[fmi->active_group] = 1;/* default */
      if (isdigit((uint8_t) *p)) { /* may be a repeat count */
	p2 = p;
	k = strtol(p2, &p2, 10);
	if (*p2 == '#')  	/* yes, it's a repeat count */
	  fmi->group_count[fmi->active_group] = k;
      }
    } /* else we already had a repeat count */
    fmi->group_count[fmi->active_group]--; /* one less repetition to go */
    if (fmi->group_count[fmi->active_group]) /* more repeats to do? */
      fmi->next = fmi->group_start[fmi->active_group]; /* then go back */
    else {
      fmi->active_group--;	/* done with this one */
      p2 = strchr(p, '#');
      fmi->next = p2? p2 + 1: p;
    }
  } else
    fmi->next = fmi->end;

  fmi->type = type;
  fmi->only_whitespace = 
    (fmi->type == FMT_PLAIN
     && fmi->start == fmi->plain
     && strspn(fmi->plain, " ") == fmi->next - fmi->plain);
  return type;
}
/*---------------------------------------------------------------------*/
formatInfo	theFormat;
char *fmttok(char *format)
/* returns the next token from the installed format.  If <format> is equal
   to NULL, then the next token from the last installed format is returned.
   If <format> is unequal to NULL, then the string that it points at is
   installed as the new format from which tokens will be returned, and
   the first token from that new format is returned in the same call. */
/* if there is nothing left, then theFormat.type is set to FMT_EMPTY */
/* and NULL is returned.  If an error occurs, then theFormat.type is */
/* set to FMT_ERROR and NULL is returned.  LS 12nov99 */
{
  formatInfo	*fmi = &theFormat;

  if (format) {			/* we install the new format */
    fmi->format = realloc(fmi->format, strlen(format) + 1);
    if (!fmi->format) {
      puts("fmttok:");
      cerror(ALLOC_ERR, 0);
      return NULL;
    }
    strcpy(fmi->format, format);
    fmi->current = fmi->format;
  } else if (fmi->next) {
    *fmi->end = fmi->save1;	/* restore */
    if (fmi->type != FMT_PLAIN
	&& fmi->plain > fmi->spec_char + 1)
      *fmi->repeat = fmi->save2;
    fmi->current = fmi->next;
    if (!fmi->current || !*fmi->current) {
      fmi->start = NULL;
      fmi->type = FMT_EMPTY;
      return NULL;
    }
  } else {
    fmi->start = NULL;
    fmi->type = FMT_EMPTY;
    return NULL;
  }

  if (fmt_entry(fmi) == FMT_ERROR) /* something illegal */
    return NULL;

  if (fmi->type != FMT_PLAIN
      && fmi->plain > fmi->spec_char + 1) {
    /* the plain text does not follow the format specification directly */
    fmi->save2 = *fmi->repeat;
    *fmi->repeat = '\0';	/* \0 over the start of the repeat count */
  }
  /* we must terminate the current format */
  fmi->save1 = *fmi->end;
  *fmi->end = '\0';
  return fmi->start;
}
/*---------------------------------------------------------------------*/
Int Sprintf_general(char *str, char *format, va_list ap)
/* prints the first argument into a string at <str> under guidance of
   the format specification in <format>.
   Only one argument is serviced, and only the first % entry is
   recognized: everything else is printed as plain text.  LS 16nov98 */
/* Sprintf does not attempt to write into <format>. */
{
  static formatInfo	ownfmi, *fmi;
  Int	width, n;
  double	d, d2;
  char	*p;
  static char	tmp[20];

  if (format) {
    ownfmi.format = ownfmi.current = format;
    fmi = &ownfmi;
    fmt_entry(fmi);
  } else
    fmi = &theFormat;

  width = fmi->width;

  if (fmi->type == FMT_ERROR)	/* some error */
    return LUX_ERROR;
  
  /* first we initialize */
  switch (*fmi->spec_char) {
    case 'z':
      /* construct a format for sprintf() in tmp[]*/
      p = tmp;
      strcpy(p++, "%");
      if (fmi->precision >= 0) {
	sprintf(p, ".%1d", fmi->precision);
	p += strlen(p);
      }
      if (fmi->flags & FMT_ALTERNATIVE)
	strcpy(p++, "f");
      else
	strcpy(p++, "g");
      strcpy(p + 1, tmp);
      *p++ = '%';
      *p++ = '+';
      strcat(p, "i%n");
      break;
    default:
      if (fmi->flags & (FMT_MIX | FMT_MIX2)) {
	p = strpbrk(fmi->start, "_=");
	/* if we're printing a multi-element array, then when we get here */
	/* for the second and later elements, fmi->start is already fixed, */
	/* so there's no longer a _ or = in it, but fmi->flags is still */
	/* set to lead us in here.  Then, we can just skip the rest. */
	/* LS 22jan2001 */
	if (p) {
	  memcpy(tmp, fmi->start, p - fmi->start);
	  memcpy(tmp + (p - fmi->start), p + 1, fmi->end - p);
	  tmp[fmi->end - p] = '\0';
	  fmi->start = tmp;
	}
      }
      break;
  }
  
  switch (*fmi->spec_char) {
  default:			/* let vsprintf handle it */
    n = vsprintf(str, fmi->start, ap);
    str += strlen(str);
    break;
  case 'z':			/* complex number */
    /* for complex numbers, the real and imaginary parts must be specified
       as separate (double) arguments */
    d = va_arg(ap, double);	/* the real part */
    d2 = va_arg(ap, double); /* the imaginary part */
    sprintf(str, tmp, d, d2, &n);
    if (n < width) {
      memmove(str + (width - n), str, n + 1);
      width -= n;
      while (width--)
	*str++ = ' ';
    }
    str += strlen(str);
    break;
  }
  va_end(ap);
  /* print any trailing plain text */
  if ((fmi->plain - fmi->spec_char) > 1 && *fmi->plain) {/* have some more text */
    strcpy(str, fmi->plain);
    n += strlen(fmi->plain);
  }
  return n;
}
/*---------------------------------------------------------------------*/
Int Sprintf_tok(char *str, ...)
/* prints the first argument into a string at <str> under guidance of
   the format specification currently installed through fmttok().
   Only one argument is serviced, and only the first % entry is
   recognized: everything else is printed as plain text.  LS 16nove98 */
{
  va_list	ap;
  Int	n;

  va_start(ap, str);
  n = Sprintf_general(str, NULL, ap);
  va_end(ap);
  return n;
}
/*---------------------------------------------------------------------*/
Int Sprintf(char *str, char *format, ...)
/* prints the first argument into a string at <str> under guidance of
   the format specification in <format>.
   Only one argument is serviced, and only the first % entry is
   recognized: everything else is printed as plain text.  LS 16nov98 */
/* Sprintf does not attempt to write into <format>. */
{
  va_list	ap;
  Int	n;

  va_start(ap, format);
  n = Sprintf_general(str, format, ap);
  va_end(ap);
  return n;
}
/*---------------------------------------------------------------------*/
char *symbolIdent(Int symbol, Int mode)
/* assembles a string identifying symbol <iq>, depending on <mode>, at
   curScrat, and returns curScrat. 
   modes:
     I_VALUE		show symbol values rather than symbol names; except
			for routines and functions (see I_ROUTINE)
     I_PARENT		show symbol as member of parent structure
                        (e.g., ROUTINE.SYMBOLNAME)
     I_TRUNCATE		truncate strings at 20 characters; arrays, 
			structures, and lists at 3 elements; statement
			groups at 1 member statement
     I_LINE		show the line number associated with the symbol
     I_FILELEVEL	show the compilation depth associated with symbol;
			includes I_LINE
     I_ROUTINE		show the statements of a routine or function; not
			just the name
     I_LENGTH		show the number of elements in truncated symbols
  LS 23jul98
*/
{
  char	*save, *p, *scalarIndicator = "bw\0\0d", *name;
  scalar	number;
  Int	i, j, n, m;
  pointer	ptr;
  listElem	*sptr;
  enumElem	*eptr;
  extractSec	*sec;
  structElem	*se;
  structPtr	*spe;
  structPtrMember	*spm;
  int16_t	*arg;
  extern Int	fileLevel, errorSym;
  extern char	*errorPtr;
  static Int	indent = 0;
  Int	identStruct(structElem *);
  
  save = curScrat;

  if (symbol == LUX_ERROR) {	/* the error symbol: should not occur here */
    strcpy(curScrat, "(error)");
    return curScrat;
  }

  if (symbol < 0) {	/* some break condition */
    switch (symbol) {
      case LOOP_BREAK:
	strcpy(curScrat, "BREAK ");
	return curScrat;
      case LOOP_CONTINUE:
	strcpy(curScrat, "CONTINUE ");
	return curScrat;
      case LOOP_RETALL:
	strcpy(curScrat, "RETALL ");
	return curScrat;
      default:
	sprintf(curScrat, "[symbol %1d] ", symbol);
	return curScrat;
    }
  }
  
  if ((mode & I_FILELEVEL) /* want file level */
      && fileLevel) {		/* have file level */
    sprintf(curScrat, "%1d, ", fileLevel);
    curScrat += strlen(curScrat); /* update */
  }

  if ((mode & I_LINE) /* want line number */
      && symbol_line(symbol)) {	/* have line number */
    sprintf(curScrat, "%3d| ", symbol_line(symbol));
    curScrat += strlen(curScrat); /* update */
  }

  if ((mode & I_PARENT)) { /* want parent's name, too */
    if (symbol_context(symbol)) {
      p = symbolProperName(symbol_context(symbol));
      if (p) {
	strcpy(curScrat, p);
	curScrat += strlen(curScrat);
	strcpy(curScrat++, ".");
      }
    }
  }

  if ((mode & I_NL) == 0)
    mode &= ~I_FILELEVEL;
  
  switch (symbol_class(symbol)) {
    case LUX_SUBROUTINE: case LUX_FUNCTION: case LUX_BLOCKROUTINE:
    case LUX_DEFERRED_SUBR: case LUX_DEFERRED_FUNC: case LUX_DEFERRED_BLOCK:
      i = (mode & I_ROUTINE);
      break;
    default:
      i = (mode & I_VALUE);
      break;
  }

  if (symbol == errorSym)
    errorPtr = curScrat;
  
  if (i == 0) {		/* want a name, if available */
    p = symbolProperName(symbol); /* get symbol's name at curScrat */
    if (p) { 			/* have a name */
      strcpy(curScrat, p);
      if (symbol_class(symbol) == LUX_UNDEFINED) /* undefined symbol */
	strcat(curScrat, "?"); /* add ? to indicate no value */
      curScrat = save;
      return curScrat;
    }
  }

  /* if we got here, then either we want the value of the symbol,
     or we wanted the name but none was available and we're getting
     the value after all */
  i = symbol;
  j = 0;
  while (symbol_class(i) == LUX_POINTER || symbol_class(i) == LUX_TRANSFER) {
    j = i;
    i = transfer_target(i);
  }
  if (i == 0)			/* pointer to nothing */
    symbol = j;
  else
    symbol = i;
  
  switch (symbol_class(symbol)) {
    case LUX_SCALAR: case LUX_FIXED_NUMBER:
      switch (scalar_type(symbol)) {
	case LUX_BYTE:
	  number.l = (Int) scalar_value(symbol).b;
	  break;
	case LUX_WORD:
	  number.l = (Int) scalar_value(symbol).w;
	  break;
	case LUX_LONG:
	  number.l = (Int) scalar_value(symbol).l;
	  break;
	case LUX_FLOAT:
	  number.d = scalar_value(symbol).f;
	  break;
	case LUX_DOUBLE:
	  number.d = scalar_value(symbol).d;
	  break;
      }
      if (scalar_type(symbol) < LUX_FLOAT) /* integer type */
	sprintf(curScrat, "%d%c", number.l,
		scalarIndicator[scalar_type(symbol)]);
      else {
	sprintf(curScrat, "%g", number.d);
	if (scalar_type(symbol) == LUX_FLOAT) {
	  p = strchr(curScrat, '.');
	  if (!p) {		/* none yet: add one */
	    p = curScrat + strlen(curScrat);
	    strcpy(p, ".");
	  }
	} else {		/* DOUBLE */
	  p = strchr(curScrat, 'e');
	  if (p)
	    *p = 'd';		/* replace "e" with "d" to indicate DOUBLE*/
	  else {
	    p = curScrat + strlen(curScrat);
	    strcpy(p, "d");
	  }
	}
      }
      break;
    case LUX_CSCALAR:		/* complex number */
      ptr.cf = complex_scalar_data(symbol).cf;
      if (complex_scalar_type(symbol) == LUX_CFLOAT) {
	if (ptr.cf->real == 0.0)
	  sprintf(curScrat, "(%g", (double) ptr.cf->imaginary);
	else
	  sprintf(curScrat, "(%g+%g", (double) ptr.cf->real,
		  (double) ptr.cf->imaginary);
      } else {
	if (ptr.cd->real == 0.0)
	  sprintf(curScrat, "(%g", ptr.cd->imaginary);
	else
	  sprintf(curScrat, "(%g+%g", ptr.cd->real, ptr.cd->imaginary);
      }
      /* we must make sure that there is an "e" or "d" type indicator
	 in the string; sprintf may provide one or two "e"s if the numbers
	 are big enough; for CDOUBLE these "e"s must be replaced by "d"s;
	 if there is no "e" or "d" already in the string, then we add one
	 at the end.  we also add an "i" at the end to indicate the
	 complex nature of the number.  LS 16nov98 */
      p = strchr(curScrat, 'e');
      if (p) {
	if (complex_scalar_type(symbol) == LUX_CDOUBLE) {
	  *p = 'd';
	  p = strchr(p + 1, 'e');
	  if (p)
	    *p = 'd';
	}
      } else {
	/* add one at the end */
	if (complex_scalar_type(symbol) == LUX_CFLOAT)
	  strcat(curScrat, "e");
	else
	  strcat(curScrat, "d");
      }
      strcat(curScrat, "i)");	/* indicate complex number */
      break;
    case LUX_STRING:
      if ((mode & I_TRUNCATE) 
	  && string_size(symbol) > 20) { /* truncate */
	sprintf(curScrat, "'%.17s...'", string_value(symbol));
      } else			/* full value */
	sprintf(curScrat, "'%s'", string_value(symbol));
      if ((mode & I_LENGTH)) {	/* need length indication */
	curScrat += strlen(curScrat);
	sprintf(curScrat, " (%1d#)", string_size(symbol));
      }
      break;
    case LUX_RANGE: case LUX_PRE_RANGE:
      strcpy(curScrat++, "(");
      i = range_start(symbol);
      if (i == -1)		/* (*) */
	strcpy(curScrat++, "*");
      else {
	if (i < 0) {		/* (* - expr ...) */
	  i = -i;
	  strcpy(curScrat, "*-");
	  curScrat += 2;
	}
	symbolIdent(i, mode);
	curScrat += strlen(curScrat);
	i = range_end(symbol);
	if (i != LUX_ZERO) {	/* really have a range end */
	  strcpy(curScrat++, ":");
	  if (i < 0) {		/* * ... */
	    if (i == -1) {	/* just * */
	      strcpy(curScrat++, "*");
	      i = 0;
	    } else {		/* * - expr */
	      strcpy(curScrat, "*-");
	      curScrat += 2;
	      i = -i;
	    }
	  }
	  if (i)
	    symbolIdent(i, mode);
	  curScrat += strlen(curScrat);
	}
	if (range_sum(symbol)) {
	  strcpy(curScrat, ":+");
	  curScrat += 2;
	}
	if (range_redirect(symbol) >= 0) {
	  strcpy(curScrat, ":>");
	  curScrat += 2;
	  symbolIdent(range_redirect(symbol), mode);
	  curScrat += strlen(curScrat);
	}
      }
      strcpy(curScrat, ")");
      break;
    case LUX_ARRAY:
      strcpy(curScrat++, "[");
      n = array_size(symbol);
      if ((mode & I_TRUNCATE)
	  && n > 3) {
	j = 3;			/* number to print */
	i = 1;			/* did truncate */
      } else {
	j = n;			/* print all */
	i = 0;			/* no truncation */
      }
      ptr.b = array_data(symbol);
      switch (array_type(symbol)) {
	case LUX_BYTE:
	  while (j--) {
	    sprintf(curScrat, "%d", (Int) *ptr.b++);
	    curScrat += strlen(curScrat);
	    if (j || i)
	      strcpy(curScrat++, ",");
	  }
	  break;
	case LUX_WORD:
	  while (j--) {
	    sprintf(curScrat, "%d", (Int) *ptr.w++);
	    curScrat += strlen(curScrat);
	    if (j || i)
	      strcpy(curScrat++, ",");
	  }
	  break;
	case LUX_LONG:
	  while (j--) {
	    sprintf(curScrat, "%d", *ptr.l++);
	    curScrat += strlen(curScrat);
	    if (j || i)
	      strcpy(curScrat++, ",");
	  }
	  break;
	case LUX_FLOAT:
	  while (j--) {
	    sprintf(curScrat, "%g", (double) *ptr.f++);
	    curScrat += strlen(curScrat);
	    if (j || i)
	      strcpy(curScrat++, ",");
	  }
	  break;
	case LUX_DOUBLE:
	  while (j--) {
	    sprintf(curScrat, "%g", *ptr.d++);
	    curScrat += strlen(curScrat);
	    if (j || i)
	      strcpy(curScrat++, ",");
	  }
	  break;
	case LUX_STRING_ARRAY:
	  if ((mode & I_TRUNCATE)) {
	    while (j--) {
	      if (*ptr.sp) {
		if (strlen(*ptr.sp) > 20)  /* need to truncate */
		  sprintf(curScrat, "'%.17s...'", *ptr.sp);
		else
		  sprintf(curScrat, "'%s'", *ptr.sp);
	      } else
		strcpy(curScrat, "''");
	      ptr.sp++;
	      curScrat += strlen(curScrat);
	      if (j || i)
		strcpy(curScrat++, ",");
	    }
	  } else {		/* no truncation */
	    while (j--) {
	      if (*ptr.sp)
		sprintf(curScrat, "'%s'", *ptr.sp);
	      else
		strcpy(curScrat, "''");
	      ptr.sp++;
	      curScrat += strlen(curScrat);
	      if (j)
		strcpy(curScrat++, ",");
	    }
	  }
	  break;
      }
      if (i) {		/* we truncated */
	strcpy(curScrat, "...");
	curScrat += 3;
      }
      strcpy(curScrat, "]");
      if ((mode & I_LENGTH)) { /* need length indication */
	curScrat += strlen(curScrat);
	sprintf(curScrat, " (");
	curScrat += 2;
	ptr.l = array_dims(symbol);
	n = array_num_dims(symbol);
	while (n--) {
	  sprintf(curScrat, "%1d", *ptr.l++);
	  curScrat += strlen(curScrat);
	  if (n)
	    strcpy(curScrat++, ",");
	}
	strcpy(curScrat++, "#)");
      }
      break;
    case LUX_POINTER: case LUX_TRANSFER:
      /* if we get here then this is a pointer to nothing */
      p = symbolProperName(symbol);
      strcpy(curScrat, p? p: "(unnamed)");
      break;
    case LUX_ASSOC:
      sprintf(curScrat, "ASSOC(%1d,", assoc_lun(symbol));
      curScrat += strlen(curScrat);
      switch (assoc_type(symbol)) {
	case LUX_BYTE:
	  strcpy(curScrat, "BYTARR(");
	  break;
	case LUX_WORD:
	  strcpy(curScrat, "INTARR(");
	  break;
	case LUX_LONG:
	  strcpy(curScrat, "LONARR(");
	  break;
	case LUX_FLOAT:
	  strcpy(curScrat, "FLTARR(");
	  break;
	case LUX_DOUBLE:
	  strcpy(curScrat, "DBLARR(");
	  break;
      }
      curScrat += strlen(curScrat);
      n = assoc_num_dims(symbol);
      ptr.l = assoc_dims(symbol);
      while (n--) {
	sprintf(curScrat, "%1d", *ptr.l++);
	curScrat += strlen(curScrat);
	if (n)
	  strcpy(curScrat++, ",");
      }
      strcpy(curScrat++, ")");
      if (assoc_has_offset(symbol)) {
	sprintf(curScrat, ",OFFSET=%1d", assoc_offset(symbol));
	curScrat += strlen(curScrat);
      }
      strcpy(curScrat, ")");
      break;
    case LUX_FUNC_PTR:
      strcpy(curScrat++, "&");
      n = func_ptr_routine_num(symbol);
      if (n < 0) {		/* internal function/routine */
	n = -n;
	switch (func_ptr_type(symbol)) {
	  case LUX_SUBROUTINE:
	    strcpy(curScrat, subroutine[n].name);
	    break;
	  case LUX_FUNCTION:
	    strcpy(curScrat, function[n].name);
	    break;
	}
      } else			/* user-defined function/routine */
	symbolIdent(n, mode);	/* is put at curScratch */
      break;
    case LUX_SCAL_PTR:
      switch (scal_ptr_type(symbol)) {
	case LUX_BYTE:
	  number.l = (Int) *scal_ptr_pointer(symbol).b;
	  break;
	case LUX_WORD:
	  number.l = (Int) *scal_ptr_pointer(symbol).w;
	  break;
	case LUX_LONG:
	  number.l = *scal_ptr_pointer(symbol).l;
	  break;
	case LUX_FLOAT:
	  number.f = *scal_ptr_pointer(symbol).f;
	  break;
	case LUX_DOUBLE:
	  number.f = (float) *scal_ptr_pointer(symbol).d;
	  break;
	case LUX_TEMP_STRING:
	  strcpy(curScrat, scal_ptr_pointer(symbol).s);
	  curScrat += strlen(curScrat);
	  break;
      }
      if (scal_ptr_type(symbol) < LUX_FLOAT) /* integer type */
	sprintf(curScrat, "%d", number.l);
      else if (scal_ptr_type(symbol) <= LUX_DOUBLE) /* float type */
	sprintf(curScrat, "%g", number.f);
      break;
    case LUX_SUBSC_PTR:
      if (subsc_ptr_start(symbol) == -1) /* (*) */
	strcpy(curScrat++, "*");
      else {
	if (subsc_ptr_start(symbol) < 0) /* (*-...) */
	  sprintf(curScrat, "*%+1d", subsc_ptr_start(symbol));
	else
	  sprintf(curScrat, "%1d", subsc_ptr_start(symbol));
	curScrat += strlen(curScrat);
	strcpy(curScrat++, ":");
	if (subsc_ptr_end(symbol) < 0) { /* (...:*...) */
	  strcpy(curScrat++, "*");
	  if (subsc_ptr_end(symbol) != -1) { /* not (...:*-...) */
	    sprintf(curScrat, "%+1d", subsc_ptr_end(symbol));
	    curScrat += strlen(curScrat);
	  }
	} else {		/* (...:...) */
	  sprintf(curScrat, "%1d", subsc_ptr_end(symbol));
	  curScrat += strlen(curScrat);
	}
	if (subsc_ptr_sum(symbol)) {
	  strcpy(curScrat, ":+");
	  curScrat += 2;
	}
	if (subsc_ptr_redirect(symbol) >= 0) {
	  sprintf(curScrat, ":>%1d", subsc_ptr_redirect(symbol));
	  curScrat += strlen(curScrat);
	}
      }
      break;
    case LUX_FILEMAP:
      switch (file_map_type(symbol)) {
	case LUX_BYTE:
	  strcpy(curScrat, "BYTFARR(");
	  break;
	case LUX_WORD:
	  strcpy(curScrat, "INTFARR(");
	  break;
	case LUX_LONG:
	  strcpy(curScrat, "LONFARR(");
	  break;
	case LUX_FLOAT:
	  strcpy(curScrat, "FLTFARR(");
	  break;
	case LUX_DOUBLE:
	  strcpy(curScrat, "DBLFARR(");
	  break;
      }
      curScrat += strlen(curScrat);
      sprintf(curScrat, "'%s',", file_map_file_name(symbol));
      curScrat += strlen(curScrat);
      n = file_map_num_dims(symbol);
      ptr.l = file_map_dims(symbol);
      while (n--) {
	sprintf(curScrat, "%1d", *ptr.l++);
	curScrat += strlen(curScrat);
	if (n)
	  strcpy(curScrat++, ",");
      }
      if (file_map_has_offset(symbol)) {
	sprintf(curScrat, ",OFFSET=%1d", file_map_offset(symbol));
	curScrat += strlen(curScrat);
      }
      if (file_map_readonly(symbol)) {
	strcpy(curScrat, ",/READONLY");
	curScrat += strlen(curScrat);
      }
      strcpy(curScrat, ")");
      break;
    case LUX_CLIST: case LUX_PRE_CLIST: case LUX_CPLIST:
      strcpy(curScrat++, "{");
      n = clist_num_symbols(symbol);
      ptr.w = clist_symbols(symbol);
      if ((mode & I_TRUNCATE)) {
	if (n > 3) {
	  j = 3;		/* number of elements to display */
	  i = 1;		/* flag truncation */
	} else {
	  j = n;
	  i = 0;		/* no trunctation was necessary */
	}
      } else {
	j = n;
	i = 0;
      }
      m = mode & I_SINGLEMODE;
      if (symbol_class(symbol) == LUX_CPLIST)
	/* for CPLIST, show names if possible */
	m &= ~I_VALUE;
      while (j--) {
	symbolIdent(*ptr.w++, m);
	curScrat += strlen(curScrat);
	if (j || i)
	  strcpy(curScrat++, ",");
      }
      if (i) {			/* we did truncate */
	strcpy(curScrat, "...");
	curScrat += strlen(curScrat);
      }
      strcpy(curScrat++, "}");
      if (i && (mode & I_LENGTH))
	sprintf(curScrat, " (%1d#)", n);
      break;
    case LUX_LIST: case LUX_PRE_LIST:
      strcpy(curScrat++, "{");
      n = list_num_symbols(symbol);
      sptr = list_symbols(symbol);
      if ((mode & I_TRUNCATE)) {
	if (n > 3) {
	  j = 3;
	  i = 1;
	} else {
	  j = n;
	  i = 0;
	}
      } else {
	j = n;
	i = 0;
      }
      while (j--) {
	if (sptr->key) {
	  sprintf(curScrat, "%s:", sptr->key);
	  curScrat += strlen(curScrat);
	}
	symbolIdent(sptr->value, mode & I_SINGLEMODE);
	curScrat += strlen(curScrat);
	if (j || i)
	  strcpy(curScrat++, ",");
	sptr++;
      }
      if (i) {
	strcpy(curScrat, "...");
	curScrat += strlen(curScrat);
      }
      strcpy(curScrat++, "}");
      if (i && (mode & I_LENGTH))
	sprintf(curScrat, " (%1d#)", n);
      break;
    case LUX_KEYWORD:
      if (keyword_value(symbol) == LUX_ONE)
	sprintf(curScrat, "/%s", keyword_name(symbol));
      else {
	sprintf(curScrat, "%s=", keyword_name(symbol));
	curScrat += strlen(curScrat);
	symbolIdent(keyword_value(symbol), mode & I_SINGLEMODE);
      }
      break;
    case LUX_LIST_PTR:
      symbolIdent(list_ptr_target(symbol), mode & I_SINGLEMODE);
      curScrat += strlen(curScrat);
      if (list_ptr_target(symbol) < 0) /* numerical tag */
	sprintf(curScrat, ".%1d", list_ptr_tag_number(symbol));
      else
	sprintf(curScrat, ".%s", list_ptr_tag_string(symbol));
      break;
    case LUX_ENUM:
      n = enum_num_elements(symbol);
      strcpy(curScrat++, "{");
      eptr = enum_list(symbol);
      if ((mode & I_TRUNCATE) && n > 3) {
	j = 3;
	i = 1;
      } else {
	j = n;
	i = 0;
      }
      while (j--) {
	sprintf(curScrat, "%s:%1d", eptr->key, eptr->value);
	eptr++;
	curScrat += strlen(curScrat);
	if (i || j)
	  strcpy(curScrat++, ",");
      }
      if (i) {
	strcpy(curScrat, "...");
	curScrat += strlen(curScrat);
      }
      if (mode & I_LENGTH) {
	sprintf(curScrat, " (%1d#)", n);
	curScrat += strlen(curScrat);
      }
      strcpy(curScrat, "}");
      break;
    case LUX_META:
      sprintf(curScrat, "SYMBOL('%s')", string_value(meta_target(symbol)));
      break;
    case LUX_CARRAY:
      ptr.cf = array_data(symbol);
      n = array_size(symbol);
      strcpy(curScrat++, "[");
      if ((mode & I_TRUNCATE)
	  && n > 3) {
	j = 3;
	i = 1;
      } else {
	j = n;
	i = 0;
      }
      switch (array_type(symbol)) {
	case LUX_CFLOAT:
	  while (j--) {
	    sprintf(curScrat, "(%g%+-1gi)", ptr.cf->real, ptr.cf->imaginary);
	    curScrat += strlen(curScrat);
	    if (i || j)
	      strcpy(curScrat++, ",");
	    ptr.cf++;
	  }
	  break;
	case LUX_CDOUBLE:
	  while (j--) {
	    sprintf(curScrat, "(%g%+-1gi)", ptr.cd->real, ptr.cd->imaginary);
	    curScrat += strlen(curScrat);
	    if (i || j)
		strcpy(curScrat++, ",");
	    ptr.cd++;
	    }
	  break;
      }
      if (i) {
	sprintf(curScrat, "...");
	curScrat += strlen(curScrat);
      }
      if (mode & I_LENGTH) {
	sprintf(curScrat, " (%1d#)", n);
	curScrat += strlen(curScrat);
      }
      strcpy(curScrat, "]");
      break;
    case LUX_SUBROUTINE: case LUX_FUNCTION: case LUX_BLOCKROUTINE:
    case LUX_DEFERRED_SUBR: case LUX_DEFERRED_FUNC: case LUX_DEFERRED_BLOCK:
      switch (symbol_class(symbol)) {
	case LUX_SUBROUTINE: case LUX_DEFERRED_SUBR:
	  name = "SUBR";
	  break;
	case LUX_FUNCTION: case LUX_DEFERRED_FUNC:
	  name = "FUNC";
	  break;
	case LUX_BLOCKROUTINE: case LUX_DEFERRED_BLOCK:
	  name = "BLOCK";
	  break;
      }
      sprintf(curScrat, " %s ", name);
      curScrat += strlen(curScrat);
      p = symbolProperName(symbol);
      strcpy(curScrat, p? p: "(unnamed)");
      curScrat += strlen(curScrat);
      if (symbol_class(symbol) == LUX_DEFERRED_SUBR
	  || symbol_class(symbol) == LUX_DEFERRED_FUNC
	  || symbol_class(symbol) == LUX_DEFERRED_BLOCK) {
	sprintf(curScrat, " (deferred, file \"%s\") END%s",
		deferred_routine_filename(symbol), name);
	curScrat += strlen(curScrat);
	if (mode & I_NL) {
	  sprintf(curScrat, "\n%*s", indent, "");
	  curScrat += strlen(curScrat);
	} else
	  strcpy(curScrat++, " ");
	break;
      }
      n = routine_num_parameters(symbol);
      if (symbol_class(symbol) == LUX_FUNCTION)
	strcpy(curScrat++, "(");
      if (n) {			/* have parameters */
	ptr.sp = routine_parameter_names(symbol);
	if (symbol_class(symbol) == LUX_SUBROUTINE)
	  strcpy(curScrat++, ",");
	while (n--) {
	  sprintf(curScrat, "%s", *ptr.sp++);
	  curScrat += strlen(curScrat);
	  if (n)
	    strcpy(curScrat++, ",");
	}
      }
      if (symbol_class(symbol) == LUX_FUNCTION)
	strcpy(curScrat++, ")");
      if (mode & I_NL) {
	indent += 2;
	sprintf(curScrat, "\n%*s", indent, "");
	curScrat += strlen(curScrat);
      } else
	strcpy(curScrat++, " ");
      n = routine_num_statements(symbol);
      ptr.w = routine_statements(symbol);
      if ((mode & I_TRUNCATE) && n > 1) {
	j = 1;
	i = 1;
      } else {
	j = n;
	i = 0;
      }
      if (j) {			/* have statements */
	while (j--) {
	  symbolIdent(*ptr.w++, mode & ~I_PARENT);
	  curScrat += strlen(curScrat);
	  if (mode & I_NL) {
	    if (!j && !i)
	      indent -= 2;
	    sprintf(curScrat, "\n%*s", indent, "");
	    curScrat += strlen(curScrat);
	  } else
	    strcpy(curScrat++, " ");
	}
      }
      if (i) {			/* we truncated */
	strcpy(curScrat, "...");
	curScrat += strlen(curScrat);
	if ((mode & I_LENGTH)) {
	  sprintf(curScrat, " (%1d#)", n);
	  curScrat += strlen(curScrat);
	}
	if (mode & I_NL) {
	  indent -= 2;
	  sprintf(curScrat, "\n%*s", indent, "");
	  curScrat += strlen(curScrat);
	} else
	  strcpy(curScrat++, " ");
      }
      sprintf(curScrat, "END%s", name);
      curScrat += strlen(curScrat);
      if (mode & I_NL) {
	sprintf(curScrat, "\n%*s", indent, "");
	curScrat += strlen(curScrat);
      }	else
	strcpy(curScrat++, " ");
      break;
    case LUX_BIN_OP: case LUX_IF_OP:
      strcpy(curScrat++, "(");
      symbolIdent(bin_op_lhs(symbol), mode & I_SINGLEMODE);
      curScrat += strlen(curScrat);
      switch (bin_op_type(symbol)) {
      case LUX_ADD: case LUX_SUB: case LUX_MUL: case LUX_DIV:
      case LUX_POW: case LUX_MOD: case LUX_MAX: case LUX_MIN:
      case LUX_IDIV:
	strcpy(curScrat, binOpSign[bin_op_type(symbol)]);
	break;
      case LUX_EQ: case LUX_LE: case LUX_LT: case LUX_GT:
      case LUX_NE: case LUX_AND: case LUX_OR: case LUX_XOR:
      case LUX_ANDIF: case LUX_ORIF: case LUX_GE: case LUX_SMOD:
	sprintf(curScrat, " %s ", binOpSign[bin_op_type(symbol)]);
	break;
      }
      curScrat += strlen(curScrat);
      symbolIdent(bin_op_rhs(symbol), mode & I_SINGLEMODE);
      curScrat += strlen(curScrat);
      strcpy(curScrat, ")");
      break;
    case LUX_INT_FUNC:
      n = 0;
      switch (int_func_number(symbol)) {
	case 0:			/* unary_negative */
	  strcpy(curScrat++, "-");
	  if (symbolIsUnitaryExpression(symbol)) {
	    symbolIdent(*int_func_arguments(symbol), mode & I_SINGLEMODE);
	    n = 0;
	  } else {
	    strcpy(curScrat++, "(");
	    n = 1;
	  }
	  break;
	case 1:			/* subscript */
	  n = int_func_num_arguments(symbol);
	  arg = int_func_arguments(symbol);
	  symbolIdent(arg[n - 1], mode & I_SINGLEMODE);
	  curScrat += strlen(curScrat);
	  strcpy(curScrat++, "(");
	  n--;
	  while (n--) {
	    symbolIdent(*arg++, mode & I_SINGLEMODE);
	    curScrat += strlen(curScrat);
	    if (n)
	      strcpy(curScrat++, ",");
	  }
	  strcpy(curScrat, ")");
	  n = 0;
	  break;
	case 2:			/* cputime */
	  strcpy(curScrat, "!CPUTIME");
	  curScrat += strlen(curScrat);
	  n = 0;
	  break;
	case 3:			/* power */
	  n = 1;
	  break;
	case 4:			/* concat */
	  strcpy(curScrat++, "[");
	  n = int_func_num_arguments(symbol);
	  if ((mode & I_TRUNCATE)
	      && n > 3) {
	    n = 3;
	    i = 1;
	  } else
	    i = 0;
	  arg = int_func_arguments(symbol);
	  while (n--) {
	    symbolIdent(*arg++, mode & I_SINGLEMODE);
	    curScrat += strlen(curScrat);
	    if (n)
	      strcpy(curScrat++, ",");
	  }
	  if (i) {
	    strcpy(curScrat, ",...");
	    curScrat += strlen(curScrat);
	  }
	  strcpy(curScrat, "]");
	  n = 0;
	  break;
	case 5:			/* ctime */
	  strcpy(curScrat, "!CTIME");
	  curScrat += strlen(curScrat);
	  n = 0;
	  break;
	case 6:			/* time */
	  strcpy(curScrat, "!TIME");
	  curScrat += strlen(curScrat);
	  n = 0;
	  break;
	case 7:			/* date */
	  strcpy(curScrat, "!DATE");
	  curScrat += strlen(curScrat);
	  n = 0;
	  break;
	case 8:			/* readkey */
	  strcpy(curScrat, "!READKEY");
	  curScrat += strlen(curScrat);
	  n = 0;
	  break;
	case 9:			/* readkeyne */
	  strcpy(curScrat, "!READKEYNE");
	  curScrat += strlen(curScrat);
	  n = 0;
	  break;
	case 10:		/* systime */
	  strcpy(curScrat, "!SYSTIME");
	  curScrat += strlen(curScrat);
	  n = 0;
	  break;
	case 11:		/* jd */
	  strcpy(curScrat, "!JD");
	  curScrat += strlen(curScrat);
	  n = 0;
	  break;
	default:
	  sprintf(curScrat, "%s(", function[int_func_number(symbol)].name);
	  curScrat += strlen(curScrat);
	  n = 1;
	  break;
      }
      if (n) {
	n = int_func_num_arguments(symbol);
	ptr.w = int_func_arguments(symbol);
	while (n--) {
	  symbolIdent(*ptr.w++, mode & I_SINGLEMODE);
	  curScrat += strlen(curScrat);
	  if (n)
	    strcpy(curScrat++, ",");
	}
	strcpy(curScrat++, ")");
      }
      break;
    case LUX_USR_FUNC:
      p = symbolProperName(usr_func_number(symbol));
      sprintf(curScrat, "%s(", p? p: "(unnamed)");
      curScrat += strlen(curScrat);
      n = usr_func_num_arguments(symbol);
      ptr.w = usr_func_arguments(symbol);
      while (n--) {
	symbolIdent(*ptr.w++, mode & I_SINGLEMODE);
	curScrat += strlen(curScrat);
	if (n)
	  strcpy(curScrat++, ",");
      }
      strcpy(curScrat++, ")");
      break;
    case LUX_EXTRACT: case LUX_PRE_EXTRACT:
      if (symbol_class(symbol) == LUX_EXTRACT) {
	if (extract_target(symbol) > 0) { /* target is regular symbol */
	  switch (symbol_class(extract_target(symbol))) {
	    case LUX_FUNCTION:
	      strcpy(curScrat, symbolProperName(extract_target(symbol)));
	      break;
	    default:
	      symbolIdent(extract_target(symbol), mode & I_SINGLEMODE);
	      break;
	  }
	} else
	  strcpy(curScrat, function[-extract_target(symbol)].name);
	curScrat += strlen(curScrat);
	sec = extract_ptr(symbol);
	n = extract_num_sec(symbol);
      } else {			/* an LUX_PRE_EXTRACT symbol */
	strcpy(curScrat, pre_extract_name(symbol));
	sec = pre_extract_ptr(symbol);
	n = pre_extract_num_sec(symbol);
      }
      curScrat += strlen(curScrat);
      if (!n) {
	strcpy(curScrat, "()");
	break;
      }
      while (n--) {
	i = sec->number;
	switch (sec->type) {
	  case LUX_RANGE:
	    strcpy(curScrat++, "(");
	    ptr.w = sec->ptr.w;
	    while (i--) {
	      symbolIdent(*ptr.w++, mode & I_SINGLEMODE);
	      curScrat += strlen(curScrat);
	      if (i)
		strcpy(curScrat++, ",");
	    }
	    strcpy(curScrat++, ")");
	    break;
	  case LUX_LIST:
	    ptr.sp = sec->ptr.sp;
	    while (i--) {
	      strcpy(curScrat++, ".");
	      strcpy(curScrat, *ptr.sp++);
	      curScrat += strlen(curScrat);
	    }
	    break;
	}
	sec++;
      }
      break;
    case LUX_EVB:
      n = evb_num_elements(symbol);
      ptr.w = evb_args(symbol);
      switch (evb_type(symbol)) {
	case EVB_RETURN:
	  strcpy(curScrat, "RETURN");
	  curScrat += strlen(curScrat);
	  if (return_value(symbol)) {
	    strcpy(curScrat++, ",");
	    symbolIdent(return_value(symbol), mode & I_SINGLEMODE);
	  }
	  break;
	case EVB_REPLACE:
	  symbolIdent(replace_lhs(symbol), mode & I_SINGLEMODE & ~I_VALUE);
	  curScrat += strlen(curScrat);
	  strcpy(curScrat++, "=");
	  symbolIdent(replace_rhs(symbol), mode & I_SINGLEMODE);
	  break;
	case EVB_REPEAT:
	  strcpy(curScrat, "REPEAT ");
	  curScrat += strlen(curScrat);
	  if (mode & I_NL) {
	    indent += 2;
	    sprintf(curScrat, "\n%*s", indent, "");
	    curScrat += strlen(curScrat);
	  } else
	    strcpy(curScrat++, " ");
	  symbolIdent(repeat_body(symbol), mode);
	  curScrat += strlen(curScrat);
	  if (mode & I_NL) {
	    indent -= 2;
	    sprintf(curScrat, "\n%*s", indent, "");
	    curScrat += strlen(curScrat);
	  } else
	    strcpy(curScrat++, " ");
	  strcpy(curScrat, "UNTIL ");
	  curScrat += strlen(curScrat);
	  symbolIdent(repeat_condition(symbol), mode & I_SINGLEMODE);
	  break;
	case EVB_WHILE_DO:
	  strcpy(curScrat, "WHILE ");
	  curScrat += strlen(curScrat);
	  symbolIdent(while_do_condition(symbol), mode & I_SINGLEMODE);
	  curScrat += strlen(curScrat);
	  strcpy(curScrat, " DO ");
	  curScrat += strlen(curScrat);
	  if (mode & I_NL) {
	    indent += 2;
	    sprintf(curScrat, "\n%*s", indent, "");
	    curScrat += strlen(curScrat);
	  } else
	    strcpy(curScrat++, " ");
	  symbolIdent(while_do_body(symbol), mode);
	  curScrat += strlen(curScrat);
	  if (mode & I_NL)
	    indent -= 2;
	  break;
	case EVB_DO_WHILE:
	  strcpy(curScrat, "DO ");
	  curScrat += strlen(curScrat);
	  if (mode & I_NL) {
	    indent += 2;
	    sprintf(curScrat, "\n%*s", indent, "");
	    curScrat += strlen(curScrat);
	  } else
	    strcpy(curScrat++, " ");
	  symbolIdent(do_while_body(symbol), mode);
	  curScrat += strlen(curScrat);
	  if (mode & I_NL) {
	    indent -= 2;
	    sprintf(curScrat, "\n%*s", indent, "");
	    curScrat += strlen(curScrat);
	  } else
	    strcpy(curScrat++, " ");
	  strcpy(curScrat, "WHILE ");
	  curScrat += strlen(curScrat);
	  symbolIdent(do_while_condition(symbol), mode & I_SINGLEMODE);
	  break;
	case EVB_IF:
	  strcpy(curScrat, "IF ");
	  curScrat += strlen(curScrat);
	  symbolIdent(if_condition(symbol), mode & I_SINGLEMODE);
	  curScrat += strlen(curScrat);
	  strcpy(curScrat, " THEN");
	  curScrat += strlen(curScrat);
	  if (mode & I_NL) {
	    indent += 2;
	    sprintf(curScrat, "\n%*s", indent, "");
	    curScrat += strlen(curScrat);
	  } else
	    strcpy(curScrat++, " ");
	  symbolIdent(if_true_body(symbol), mode);
	  curScrat += strlen(curScrat);
	  if (mode & I_NL) {
	    indent -= 2;
	    sprintf(curScrat, "\n%*s", indent, "");
	    curScrat += strlen(curScrat);
	  } else
	    strcpy(curScrat++, " ");
	  if (if_false_body(symbol)) {
	    strcpy(curScrat, "ELSE");
	    curScrat += strlen(curScrat);
	    if (mode & I_NL) {
	      indent += 2;
	      sprintf(curScrat, "\n%*s", indent, "");
	      curScrat += strlen(curScrat);
	    } else
	      strcpy(curScrat++, " ");
	    symbolIdent(if_false_body(symbol), mode);
	    curScrat += strlen(curScrat);
	    if (mode & I_NL) {
	      indent -= 2;
	      sprintf(curScrat, "\n%*s", indent, "");
	      curScrat += strlen(curScrat);
	    } else
	      strcpy(curScrat++, " ");
	  }
	  break;
	case EVB_FOR:
	  strcpy(curScrat, "FOR ");
	  curScrat += strlen(curScrat);
	  p = symbolProperName(for_loop_symbol(symbol));
	  sprintf(curScrat, "%s=", p? p: "(unnamed)");
	  curScrat += strlen(curScrat);
	  symbolIdent(for_start(symbol), mode & I_SINGLEMODE);
	  curScrat += strlen(curScrat);
	  strcpy(curScrat++, ",");
	  symbolIdent(for_end(symbol), mode & I_SINGLEMODE);
	  curScrat += strlen(curScrat);
	  if (for_step(symbol) != LUX_ONE) {
	    strcpy(curScrat++, ",");
	    symbolIdent(for_step(symbol), mode & I_SINGLEMODE);
	    curScrat += strlen(curScrat);
	  }
	  if (mode & I_NL) {
	    indent += 2;
	    sprintf(curScrat, "\n%*s", indent, "");
	    curScrat += strlen(curScrat);
	  } else
	    strcpy(curScrat++, " ");
	  symbolIdent(for_body(symbol), mode);
	  curScrat += strlen(curScrat);
	  if (mode & I_NL)
	    indent -= 2;
	  break;
	case EVB_USR_CODE:
	  strcpy(curScrat, "RUN,");
	  curScrat += strlen(curScrat);
	  n = usr_code_routine_num(symbol);
	  if (symbol_class(n) == LUX_STRING)  /* unevaluated name */
	    strcpy(curScrat, string_value(n));
	  else {
	    p = symbolProperName(usr_code_routine_num(symbol));
	    strcpy(curScrat, p? p: "(unnamed)");
	  }
	  break;
	case EVB_FILE:
	  switch (file_include_type(symbol)) {
	    case FILE_REPORT:
	      strcpy(curScrat, "@@");
	      curScrat += 2;
	      break;
	    case FILE_INCLUDE:
	      strcpy(curScrat++, "@");
	      break;
	  }
	  strcpy(curScrat, file_name(symbol));
	  break;
	case EVB_INT_SUB:
	  switch (int_sub_routine_num(symbol)) {
	    case LUX_INSERT_SUB: /* INSERT */
	      n = int_sub_num_arguments(symbol) - 2;
	      ptr.w = int_sub_arguments(symbol);
	      symbolIdent(ptr.w[n + 1], mode & I_SINGLEMODE);
	      curScrat += strlen(curScrat);
	      strcpy(curScrat++, "(");
	      while (n--) {
		symbolIdent(*ptr.w++, mode & I_SINGLEMODE);
		curScrat += strlen(curScrat);
		if (n)
		  strcpy(curScrat++, ",");
	      }
	      strcpy(curScrat, ")=");
	      curScrat += strlen(curScrat);
	      symbolIdent(*ptr.w, mode & I_SINGLEMODE);
	      break;
	    default:
	      strcpy(curScrat, subroutine[int_sub_routine_num(symbol)].name);
	      curScrat += strlen(curScrat);
	      n = int_sub_num_arguments(symbol);
	      ptr.w = int_sub_arguments(symbol);
	      while (n--) {
		strcpy(curScrat++, ",");
		symbolIdent(*ptr.w++, mode & I_SINGLEMODE);
		curScrat += strlen(curScrat);
	      }
	      break;
	  }
	  break;
	case EVB_USR_SUB:
	  if (usr_sub_is_deferred(symbol))  /* call to an LUX_DEFERRED_SUBR */
	    strcpy(curScrat, string_value(usr_sub_routine_num(symbol)));
	  else {
	    p = symbolProperName(usr_sub_routine_num(symbol));
	    sprintf(curScrat, "%s", p? p: "(unnamed)");
	  }
	  curScrat += strlen(curScrat);
	  n = usr_sub_num_arguments(symbol);
	  ptr.w = usr_sub_arguments(symbol);
	  while (n--) {
	    strcpy(curScrat++, ",");
	    symbolIdent(*ptr.w++, mode & I_SINGLEMODE);
	    curScrat += strlen(curScrat);
	  }
	  break;
	case EVB_INSERT:
	  p = symbolProperName(insert_target(symbol));
	  sprintf(curScrat, "%s(", p? p: "(unnamed)");
	  curScrat += strlen(curScrat);
	  n = insert_num_target_indices(symbol);
	  ptr.w = insert_target_indices(symbol);
	  while (n--) {
	    symbolIdent(*ptr.w++, mode & I_SINGLEMODE);
	    curScrat += strlen(curScrat);
	    if (n)
	      strcpy(curScrat++, ",");
	  }
	  strcpy(curScrat++, ")=");
	  symbolIdent(insert_source(symbol), mode & I_SINGLEMODE);
	  break;
	case EVB_CASE:
	  strcpy(curScrat, "CASE ");
	  curScrat += strlen(curScrat);
	  if (mode & I_NL) {
	    indent += 2;
	    sprintf(curScrat, "\n%*s", indent, "");
	    curScrat += strlen(curScrat);
	  } else
	    strcpy(curScrat++, " ");
	  n = case_num_statements(symbol);
	  n = (n - 1)/2;	/* number of statements */
	  if ((mode & I_TRUNCATE) && n > 1) {
	    j = 1;
	    i = 1;
	  } else {
	    j = n;
	    i = 0;
	  }
	  ptr.w = case_statements(symbol);
	  while (j--) {
	    symbolIdent(*ptr.w++, mode & I_SINGLEMODE);	/* condition */
	    curScrat += strlen(curScrat);
	    strcpy(curScrat, " : ");
	    curScrat += 3;
	    if (mode & I_NL) {
	      indent += 2;
	      sprintf(curScrat, "\n%*s", indent, "");
	      curScrat += strlen(curScrat);
	    } else
	      strcpy(curScrat++, " ");
	    curScrat += strlen(curScrat);
	    symbolIdent(*ptr.w++, mode & I_SINGLEMODE);	/* action */
	    curScrat += strlen(curScrat);
	    if (mode & I_NL) {
	      indent -= 2;
	      if (!j && !*ptr.w) /* last one and no ELSE */
		indent -= 2;
	      sprintf(curScrat, "\n%*s", indent, "");
	      curScrat += strlen(curScrat);
	    } else
	      strcpy(curScrat++, " ");
	  }
	  if (i) {		/* we truncated */
	    strcpy(curScrat, "...");
	    curScrat += 3;
	    if ((mode & I_LENGTH)) {
	      sprintf(curScrat, " (%1d#)", n);
	      curScrat += strlen(curScrat);
	    }
	    ptr.w += n - 1;
	  }
	  if (*ptr.w) {		/* have an ELSE clause */
	    strcpy(curScrat, "ELSE ");
	    curScrat += strlen(curScrat);
	    if (mode & I_NL) {
	      indent += 2;
	      sprintf(curScrat, "\n%*s", indent, "");
	      curScrat += strlen(curScrat);
	    } else
	      strcpy(curScrat++, " ");
	    symbolIdent(*ptr.w, mode & I_SINGLEMODE);
	    curScrat += strlen(curScrat);
	    if (mode & I_NL) {
	      indent -= 4;
	      sprintf(curScrat, "\n%*s", indent, "");
	      curScrat += strlen(curScrat);
	    } else
	      strcpy(curScrat++, " ");
	  }
	  strcpy(curScrat, "ENDCASE");
	  break;
	case EVB_NCASE:
	  strcpy(curScrat, "NCASE ");
	  curScrat += strlen(curScrat);
	  symbolIdent(ncase_switch_value(symbol), mode & I_SINGLEMODE);
	  curScrat += strlen(curScrat);
	  if (mode & I_NL) {
	    indent += 2;
	    sprintf(curScrat, "\n%*s", indent, "");
	    curScrat += strlen(curScrat);
	  } else
	    strcpy(curScrat++, " ");
	  n = ncase_num_statements(symbol);
	  if ((mode & I_TRUNCATE) && n > 1) {
	    j = 1;
	    i = 1;
	  } else {
	    j = n;
	    i = 0;
	  }
	  ptr.w = ncase_statements(symbol);
	  while (j--) {
	    symbolIdent(*ptr.w++, mode & I_SINGLEMODE);
	    curScrat += strlen(curScrat);
	    if (mode & I_NL) {
	      if (!j && !ncase_else(symbol))
		indent -= 2;
	      sprintf(curScrat, "\n%*s", indent, "");
	      curScrat += strlen(curScrat);
	    } else
	      strcpy(curScrat++, " ");
	  }
	  if (i) {		/* we were truncating */
	    strcpy(curScrat, "...");
	    curScrat += strlen(curScrat);
	    if ((mode & I_LENGTH)) {
	      sprintf(curScrat, " (%1d#)", n);
	      curScrat += strlen(curScrat);
	    }
	    ptr.w += n - 1;
	  }
	  if (ncase_else(symbol)) {
	    strcpy(curScrat, "ELSE ");
	    curScrat += strlen(curScrat);
	    if (mode & I_NL) {
	      indent += 2;
	      sprintf(curScrat, "\n%*s", indent, "");
	      curScrat += strlen(curScrat);
	    } else
	      strcpy(curScrat++, " ");
	    symbolIdent(ncase_else(symbol), mode & I_SINGLEMODE);
	    curScrat += strlen(curScrat);
	    if (mode & I_NL) {
	      indent -= 4;
	      sprintf(curScrat, "\n%*s", indent, "");
	      curScrat += strlen(curScrat);
	    } else
	      strcpy(curScrat++, " ");
	  }
	  strcpy(curScrat, "ENDCASE");
	  break;
	case EVB_BLOCK:
	  strcpy(curScrat, "{ ");
	  if (mode & I_NL)
	    indent += 2;
	  curScrat += strlen(curScrat);
	  n = block_num_statements(symbol);
	  ptr.w = block_statements(symbol);
	  if ((mode & I_TRUNCATE) && n > 1) {
	    j = 1;
	    i = 1;
	  } else {
	    j = n;
	    i = 0;
	  }
	  while (j--) {
	    symbolIdent(*ptr.w++, mode);
	    curScrat += strlen(curScrat);
	    if ((mode & I_NL) && (j || i)) {
	      sprintf(curScrat, "\n%*s", indent, "");
	      curScrat += strlen(curScrat);
	    } else
	      strcpy(curScrat++, " ");
	  }
	  if (i) {		/* we truncated */
	    strcpy(curScrat, "...");
	    curScrat += strlen(curScrat);
	    if ((mode & I_LENGTH)) {
	      sprintf(curScrat, " (%1d#)", n);
	      curScrat += strlen(curScrat);
	    }
	  }
	  strcpy(curScrat++, "}");
	  if (mode & I_NL)
	    indent -= 2;
	  break;
	default:
	  sprintf(curScrat, "[EVB type %1d]", evb_type(symbol));
	  break;
      }
      break;
    case LUX_STRUCT:
      se = struct_elements(symbol);
      identStruct(se);
      break;
    case LUX_STRUCT_PTR:
      spe = struct_ptr_elements(symbol);
      n = struct_ptr_n_elements(symbol);
      symbolIdent(struct_ptr_target(symbol), mode);
      curScrat += strlen(curScrat);
      strcpy(curScrat++, "(");
      se = struct_elements(struct_ptr_target(symbol));
      while (n--) {
	if (!spe->desc) {	/* top element */
	  i = spe->n_subsc;	/* number of subscripts */
	  spm = spe->member;
	  while (i--) {
	    switch (spm->type) {
	      case LUX_SCALAR:
		sprintf(curScrat, "%1d", spm->data.scalar.value);
		break;
	      case LUX_RANGE:
		sprintf(curScrat, "%1d:%1d", spm->data.range.start,
			spm->data.range.end);
		break;
	      case LUX_ARRAY:
		strcpy(curScrat++, "[");
		j = spm->data.array.n_elem;
		if ((mode & I_TRUNCATE) && j > 3)
		  j = 3;
		ptr.l = spm->data.array.ptr;
		while (j--) {
		  sprintf(curScrat, "%1d", *ptr.l++);
		  curScrat += strlen(curScrat);
		  if (j)
		    strcpy(curScrat++, ",");
		}
		strcpy(curScrat++, "]");
		break;
	    }
	    curScrat += strlen(curScrat);
	    spm++;
	  }
	}
	spe++;
      }
      break;
    case LUX_UNDEFINED:
      p = symbolProperName(symbol);
      sprintf(curScrat, "%s?", p? p: "(unnamed)");
      break;
    default:
      sprintf(curScrat, "[class %1d (%s)]",
	      symbol_class(symbol), className(symbol_class(symbol)));
      break;
  }
  
  curScrat = save;		/* restore proper value */
  return curScrat;
}
/*---------------------------------------------------------------------*/
Int identStruct(structElem *se)
{
  Int	n, nelem, ndim, *dims, ndim2, *dims2;
  char	*arrName[] = {
    "BYTARR", "INTARR", "LONARR", "FLTARR", "DBLARR", "STRARR",
    "STRARR", "STRARR", "CFLTARR", "CDBLARR"
  };

  n = nelem = se->u.first.nelem;
  dims = se->u.first.dims;
  ndim = se->u.first.ndim;
  sprintf(curScrat, "STRUCT(");
  curScrat += strlen(curScrat);
  if (nelem > 1)
    *curScrat++ = '{';
  while (n--) {
    se++;
    switch (se->u.regular.type) {
      default:
	if (se->u.regular.tag) {
	  sprintf(curScrat, "%s:", se->u.regular.tag);
	  curScrat += strlen(curScrat);
	}
	ndim2 = se->u.regular.spec.singular.ndim;
	sprintf(curScrat, "%s(", arrName[se->u.regular.type]);
	curScrat += strlen(curScrat);
	if (ndim2) {
	  dims2 = se->u.regular.spec.singular.dims;
	  if (se->u.regular.type == LUX_TEMP_STRING) {
	    sprintf(curScrat, "SIZE=%1d,1", *dims2++);
	    curScrat += strlen(curScrat);
	    ndim2 = 0;
	  } else if (se->u.regular.type == LUX_STRING_ARRAY) {
	    sprintf(curScrat, "SIZE=%1d", *dims2++);
	    curScrat += strlen(curScrat);
	    if (ndim2 > 1)
	      *curScrat++ = ',';
	    ndim2--;
	  }
	  while (ndim2--) {
	    sprintf(curScrat, "%1d", *dims2++);
	    curScrat += strlen(curScrat);
	    if (ndim2)
	      *curScrat++ = ',';
	  }
	  *curScrat++ = ')';
	} else {		/* scalar: show as 1-element array */
	  sprintf(curScrat, "1)");
	  curScrat += strlen(curScrat);
	}
	break;
    }
    if (n)
      *curScrat++ = ',';
  }
  if (nelem > 1)
    *curScrat++ = '}';
  *curScrat++ = ',';
  while (ndim--) {
    sprintf(curScrat, "%1d", *dims++);
    curScrat += strlen(curScrat);
    if (ndim)
      *curScrat++ = ',';
  }
  strcat(curScrat, ")");
  curScrat += strlen(curScrat);
  return 1;
}
/*---------------------------------------------------------------------*/
void dumpTree(Int symbol)
{
  Int	kind, i, n, *l;
  int16_t	*ptr;
  static Int	indent = 0;
  extern char *binOpName[];
  char	*name, noName[] = "-", **sp;
  extractSec	*eptr;

  if (!indent)
    puts(symbolIdent(symbol, 0));
  for (i = 0; i < indent; i++)
    putchar('|');
  name = symbolProperName(symbol);
  if (name == NULL)
    name = noName;
  printf("<%1d> %s (%1d) %s: ", symbol, name, symbol_context(symbol),
	 className(symbol_class(symbol)));
  switch (symbol_class(symbol)) {
  default:
      printf(" type %1d\n", symbol_type(symbol));
      return;
    case LUX_SCALAR: case LUX_SCAL_PTR: case LUX_ARRAY:
    case LUX_STRING: case LUX_SUBSC_PTR:
      puts(symbolIdent(symbol, I_VALUE | I_TRUNCATE));
      return;
    case LUX_POINTER: case LUX_TRANSFER:
      printf("target: %1d\n", transfer_target(symbol));
      indent++;
      dumpTree(transfer_target(symbol));
      indent--;
      return;
    case LUX_PRE_EXTRACT: case LUX_EXTRACT:
      if (symbol_class(symbol) == LUX_PRE_EXTRACT) {
	printf("target: %s\n", pre_extract_name(symbol));
	eptr = pre_extract_ptr(symbol);
	n = pre_extract_num_sec(symbol);
      } else {
	printf("target:\n");
	dumpTree(extract_target(symbol));
	eptr = extract_ptr(symbol);
	n = extract_num_sec(symbol);
      }
      printf("extractions: %1d\n", n);
      if (!n)
	break;
      indent++;
      while (n--) {
	i = eptr->number;
	switch (eptr->type) {
	  case LUX_RANGE:
	    printf("range:\n");
	    ptr = eptr->ptr.w;
	    while (i--)
	      dumpTree(*ptr++);
	    break;
	  case LUX_LIST:
	    printf("tags:\n");
	    sp = eptr->ptr.sp;
	    while (i--)
	      printf(".%s ", *sp++);
	    break;
	}
	eptr++;
      }
      indent--;
      break;
    case LUX_SUBROUTINE: case LUX_FUNCTION: case LUX_BLOCKROUTINE:
      printf(" parameters: ");
      n = routine_num_parameters(symbol);
      if (!n)
	printf(" none");
      else {
	ptr = routine_parameters(symbol);
	for (i = 0; i < n; i++)
	  printf(" %1d", *ptr++);
      }
      n = routine_num_statements(symbol);
      printf("; statements:");
      if (!n)
	printf(" none");
      else {
	ptr = routine_statements(symbol);
	for (i = 0; i < n; i++)
	  printf(" %1d", *ptr++);
      }
      putchar('\n');
      n = routine_num_parameters(symbol);
      indent++;
      if (n) {
	ptr = routine_parameters(symbol);
	for (i = 0; i < n; i++)
	  dumpTree(*ptr++);
      }
      n = routine_num_statements(symbol);
      if (n) {
	ptr = routine_statements(symbol);
	for (i = 0; i < n; i++)
	  dumpTree(*ptr++);
      }
      indent--;
      return;
    case LUX_RANGE:
      printf("start: %1d, end: %1d, sum: %1d, redirect: %1d\n",
	     range_start(symbol), range_end(symbol), range_sum(symbol),
	     range_redirect(symbol));
      indent++;
      n = range_start(symbol);
      if (n < 0)
	n = -n;
      dumpTree(n);
      n = range_end(symbol);
      if (n < 0)
	n = -n;
      dumpTree(n);
      n = range_redirect(symbol);
      if (n >= 0)
	dumpTree(n);
      indent--;
      return;
    case LUX_LIST:
      printf("key - value: ");
      n = list_num_symbols(symbol);
      for (i = 0; i < n; i++)
	printf(" %s: %1d;", list_key(symbol, i), list_symbol(symbol, i));
      putchar('\n');
      indent++;
      for (i = 0; i < n; i++)
	dumpTree(list_symbol(symbol, i));
      indent--;
      return;
    case LUX_CLIST:
      printf("elements: ");
      n = clist_num_symbols(symbol);
      ptr = clist_symbols(symbol);
      for (i = 0; i < n; i++)
	printf(" %1d", *ptr++);
      putchar('\n');
      ptr -= n;
      indent++;
      for (i = 0; i < n; i++)
	dumpTree(*ptr++);
      indent--;
      return;
    case LUX_KEYWORD:
      printf("name: %1d, value: %1d\n", keyword_name_symbol(symbol),
	   keyword_value(symbol));
      indent++;
      dumpTree(keyword_name_symbol(symbol));
      dumpTree(keyword_value(symbol));
      indent--;
      return;
    case LUX_FILEMAP:
      printf("type: %s, file: %s, dimensions:",
	     typeName(file_map_type(symbol)),
	     file_map_file_name(symbol));
      n = file_map_num_dims(symbol);
      l = file_map_dims(symbol);
      for (i = 0; i < n; i++)
	printf(" %1d", *l++);
      putchar('\n');
      return;
    case LUX_ASSOC:
      printf("lun: %1d, dimensions:", assoc_lun(symbol));
      n = assoc_num_dims(symbol);
      l = assoc_dims(symbol);
      for (i = 0; i < n; i++)
	printf(" %1d", *l++);
      putchar('\n');
      return;
    case LUX_ENUM:
      printf("type: %s, key - value: %s\n", typeName(enum_type(symbol)),
	     symbolIdent(symbol, I_VALUE));
      return;
    case LUX_UNDEFINED:
      putchar('\n');
      return;
    case LUX_META:
      printf("number: %1d\n", meta_target(symbol));
      indent++;
      dumpTree(meta_target(symbol));
      indent--;
      return;
    case LUX_FUNC_PTR:
      n = func_ptr_routine_num(symbol);
      if (n < 0) {
	if (func_ptr_type(symbol) == LUX_FUNCTION)
	  printf("internal function: %s\n", function[-n].name);
	else
	  printf("internal subroutine: %s\n", subroutine[-n].name);
      } else {
	printf("routine: %1d\n", n);
	indent++;
	dumpTree(n);
	indent--;
      }
      return;
    case LUX_PRE_LIST:
      printf("key - element: ");
      n = pre_list_num_symbols(symbol);
      for (i = 0; i < n; i++)
	printf(" %s: %1d;", pre_list_key(symbol, i),
	       pre_list_symbol(symbol, i));
      putchar('\n');
      indent++;
      for (i = 0; i < n; i++)
	dumpTree(pre_list_symbol(symbol, i));
      indent--;
      return;
    case LUX_PRE_CLIST:
      printf("elements:");
      n = pre_clist_num_symbols(symbol);
      ptr = pre_clist_symbols(symbol);
      for (i = 0; i < n; i++)
	printf(" %1d", *ptr++);
      putchar('\n');
      ptr--;
      indent++;
      for (i = 0; i < n; i++)
	dumpTree(*ptr++);
      indent--;
      return;
    case LUX_PRE_RANGE:
      printf("start: %1d, end: %1d, sum: %1d, redirect: %1d\n",
	     pre_range_start(symbol), pre_range_end(symbol),
	     pre_range_sum(symbol), pre_range_redirect(symbol));
      indent++;
      n = pre_range_start(symbol);
      if (n < 0)
	n = -n;
      dumpTree(n);
      n = pre_range_end(symbol);
      if (n < 0) n = -n;
      dumpTree(n);
      n = pre_range_redirect(symbol);
      if (n >= 0)
	dumpTree(n);
      indent--;
      return;
    case LUX_LIST_PTR:
      n = list_ptr_target(symbol);
      printf("target: %1d, tag: ", n < 0? -n: n);
      if (n < 0) {
	printf("%1d\n", list_ptr_tag_number(symbol));
	n = -n;
      }
      else
	printf("%s\n", list_ptr_tag_string(symbol));
      indent++;
      dumpTree(n);
      indent--;
      return;
    case LUX_INT_FUNC:
      printf("function: %s, arguments:",
	     function[int_func_number(symbol)].name);
      n = int_func_num_arguments(symbol);
      if (!n) {
	puts(" none");
	return;
      }
      ptr = int_func_arguments(symbol);
      for (i = 0; i < n; i++)
	printf(" %1d", *ptr++);
      putchar('\n');
      ptr -= n;
      indent++;
      for (i = 0; i < n; i++)
	dumpTree(*ptr++);
      indent--;
      return;
    case LUX_USR_FUNC:
      printf("function: %1d (%s), arguments:", usr_func_number(symbol),
	     symbolProperName(usr_func_number(symbol)));
      n = usr_func_num_arguments(symbol);
      if (!n) {
	puts(" none");
	return;
      } else {
	ptr = usr_func_arguments(symbol);
	for (i = 0; i < n; i++)
	  printf(" %1d", *ptr++);
	putchar('\n');
	ptr -= n;
      }
      indent++;
      for (i = 0; i < n; i++)
	dumpTree(*ptr++);
      indent--;
      return;
    case LUX_IF_OP: case LUX_BIN_OP:
      printf("%s, lhs: %1d, rhs: %1d\n",
	     binOpName[bin_op_type(symbol)], bin_op_lhs(symbol),
	     bin_op_rhs(symbol));
      indent++;
      dumpTree(bin_op_lhs(symbol));
      dumpTree(bin_op_rhs(symbol));
      indent--;
      return;
    case LUX_EVB:
      kind = evb_type(symbol);
      switch (kind) {
	case EVB_REPLACE:
	  printf("assignment: lhs: %1d, rhs: %1d\n", replace_lhs(symbol),
		 replace_rhs(symbol));
	  indent++;
	  dumpTree(replace_lhs(symbol));
	  dumpTree(replace_rhs(symbol));
	  indent--;
	  return;
	case EVB_INT_SUB:
	  printf("internal routine: %s, arguments:",
		 subroutine[int_sub_routine_num(symbol)].name);
	  n = int_sub_num_arguments(symbol);
	  if (!n) {
	    puts(" none");
	    return;
	  }
	  ptr = int_sub_arguments(symbol);
	  for (i = 0; i < n; i++)
	    printf(" %1d", *ptr++);
	  putchar('\n');
	  ptr -= n;
	  indent++;
	  for (i = 0; i < n; i++)
	    dumpTree(*ptr++);
	  indent--;
	  return;
	case EVB_USR_SUB:
	  printf("subr call: %1d (%s), arguments:",
		 usr_sub_routine_num(symbol),
		 symbolProperName(usr_sub_routine_num(symbol)));
	  n = usr_sub_num_arguments(symbol);
	  if (!n) {
	    puts(" none");
	    return;
	  }
	  ptr = usr_sub_arguments(symbol);
	  for (i = 0; i < n; i++)
	    printf(" %1d", *ptr++);
	  putchar('\n');
	  ptr -= n;
	  indent++;
	  for (i = 0; i < n; i++)
	    dumpTree(*ptr++);
	  indent--;
	  return;
	case EVB_USR_CODE:
	  printf("block call: %1d (%s)\n", usr_code_routine_num(symbol),
		 symbolProperName(usr_code_routine_num(symbol)));
	  return;
	case EVB_INSERT:
	  printf("insertion: source: %1d, target: %1d, target coordinates:",
		 insert_source(symbol), insert_target(symbol));
	  n = insert_num_target_indices(symbol);
	  ptr = insert_target_indices(symbol);
	  if (!n)
	    puts(" none");
	  indent++;
	  dumpTree(insert_source(symbol));
	  dumpTree(insert_target(symbol));
	  if (!n) {
	    indent--;
	    return;
	  }
	  for (i = 0; i < n; i++)
	    printf(" %1d", *ptr++);
	  putchar('\n');
	  ptr -= n;
	  for (i = 0; i < n; i++)
	    dumpTree(*ptr++);
	  indent--;
	  return;
	case EVB_CASE:
	  printf("case: condition - statements:");
	  n = case_num_statements(symbol)/2;
	  if (!n) {
	    puts(" none");
	    return;
	  }
	  ptr = case_statements(symbol);
	  for (i = 0; i < n; i++) {
	    printf(" %1d", *ptr++);
	    printf(" %1d;", *ptr++);
	  }
	  putchar('\n');
	  ptr -= n*2;
	  indent++;
	  for (i = 0; i < 2*n; i++)
	    dumpTree(*ptr++);
	  indent--;
	  return;
	case EVB_NCASE:
	  printf("ncase: condition: %1d, statements:",
		 ncase_switch_value(symbol));
	  n = ncase_num_statements(symbol);
	  if (!n) {
	    puts(" none");
	    return;
	  }
	  ptr = ncase_statements(symbol);
	  for (i = 0; i < n + 1; i++)
	    printf(" %1d", *ptr++);
	  putchar('\n');
	  ptr -= n;
	  indent++;
	  for (i = 0; i < n + 1; i++)
	    dumpTree(*ptr++);
	  indent--;
	  return;
	case EVB_FILE:
	  printf("file inclusion: name: %s, type: %s\n", file_name(symbol),
		 file_include_type(symbol) == FILE_INCLUDE?
		 "include (@)": "report (@@)");
	  break;
	case EVB_BLOCK:
	  printf("block statement: statements:");
	  n = block_num_statements(symbol);
	  if (!n) {
	    puts(" none");
	    return;
	  }
	  ptr = block_statements(symbol);
	  for (i = 0; i < n; i++)
	    printf(" %1d", *ptr++);
	  putchar('\n');
	  ptr -= n;
	  indent++;
	  for (i = 0; i < n; i++)
	    dumpTree(*ptr++);
	  indent--;
	  return;
	case EVB_FOR:
	  printf("for statement: start: %1d, end: %1d, step: %1d, body: %1d\n",
		 for_start(symbol), for_end(symbol), for_step(symbol),
		 for_body(symbol));
	  indent++;
	  dumpTree(for_start(symbol));
	  dumpTree(for_end(symbol));
	  dumpTree(for_step(symbol));
	  dumpTree(for_body(symbol));
	  indent--;
	  return;
	case EVB_IF:
	  printf("if statement: condition: %1d, true: %1d, false: %1d\n",
		 if_condition(symbol), if_true_body(symbol),
		 if_false_body(symbol));
	  indent++;
	  dumpTree(if_condition(symbol));
	  dumpTree(if_true_body(symbol));
	  dumpTree(if_false_body(symbol));
	  indent--;
	  return;
	case EVB_REPEAT:
	  printf("repeat statement: body: %1d, condition: %1d\n",
		 repeat_body(symbol), repeat_condition(symbol));
	  indent++;
	  dumpTree(repeat_body(symbol));
	  dumpTree(repeat_condition(symbol));
	  indent--;
	  return;
	case EVB_DO_WHILE:
	  printf("do-while statement: body: %1d, condition: %1d\n",
		 do_while_body(symbol), do_while_condition(symbol));
	  indent++;
	  dumpTree(do_while_body(symbol));
	  dumpTree(do_while_condition(symbol));
	  indent--;
	  return;
	case EVB_WHILE_DO:
	  printf("while-do statement: condition: %1d, body: %1d\n",
		 while_do_condition(symbol), while_do_body(symbol));
	  indent++;
	  dumpTree(while_do_condition(symbol));
	  dumpTree(while_do_body(symbol));
	  indent--;
	  return;
	case EVB_RETURN:
	  printf("return: value: %1d\n", return_value(symbol));
	  indent++;
	  dumpTree(return_value(symbol));
	  indent--;
	  return;
	default:
	  return;
	}
    }
  return;
}
/*------------------------------------------------------------------------- */
void dumpLine(Int symbol)
{
  while (symbol_context(symbol) > 0)
    symbol = symbol_context(symbol);
  dumpTree(symbol);
}
/*------------------------------------------------------------------------- */
Int lux_list(Int narg, Int ps[])
/* shows the definition of a user-defined subroutine, function,
   or block routine */
/* LIST,symbol  or  LIST,'name'  lists the definition of the given symbol
 or of the routine with the given name.  */
{
  Int	symbol;
  char	*name, *p;

  switch (symbol_class(ps[0])) {
    case LUX_SCALAR:
      symbol = int_arg(ps[0]);
      break;
    case LUX_STRING:
      name = p = strsave(string_value(ps[0]));
      while (*p) {
	*p = toupper(*p);
	p++;
      }
      symbol = lookForName(name, varHashTable, 0);
      if (symbol < 0)
	symbol = lookForName(name, subrHashTable, 0);
      if (symbol < 0)
	symbol = lookForName(name, funcHashTable, 0);
      if (symbol < 0)
	symbol = lookForName(name, blockHashTable, 0);
      free(name);
      if (symbol < 0)
	return luxerror("Could not find the symbol", ps[0]);
      break;
    default:
      return cerror(ILL_CLASS, ps[0]);
  }
  setPager(0);
  printw(symbolIdent(symbol, I_ROUTINE | I_NL));
  resetPager();
  return LUX_ONE;
}
/*------------------------------------------------------------------------- */
