/* This is file error.c.

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
/* File error.c */
/* LUX routines for generating error messages. */
/* error.c - error messages */

#if HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "action.hh"

char const* errorMessages[] = {
/* COND_NO_SCAL    0*/
 "Condition must be scalar",
/* ILL_COMB  1*/
#ifdef __STDC__
 "Illegal argument combination (%s %s %s %s %s)",
#else
 "Illegal argument combination",
#endif
/* INCMP_DIMS      2*/
 "Incompatible dimensions",
/* ALLOC_ERR       3*/
 "Memory allocation error",
/* NO_FLT_COND     4*/
#ifdef __STDC__
 "%s operator expects integer operands, not %s",
#else
 "operator expects integer operands",
#endif
/* ILL_CLASS       5*/
 "Argument is of an illegal class",
/* ILL_TYPE        6*/
 "Expression type %s is not allowed here",
/* ILL_SYM         7*/
#ifdef __STDC__
 "Illegal symbol number: %d (%s)",
#else
 "Illegal symbol number",
#endif
/* ILL_DIM         8*/
 "Illegal dimension",
/* MYST_CLASS      9*/
 "Mysterious class in binary operation or function",
/* NO_SCAL		10*/
 "Argument must be a scalar",
/* ILL_ARG		11*/
 "Illegal argument",
/* CST_LHS		12*/
 "Unmodifiable variable",
/* ONLY_A_S	13*/
 "Only numerical types allowed",
/* ILL_W_STR	14*/
 "Illegal operation with string",
/* INCMP_ARR	15*/
 "Array is incompatible with previous one",
/* ILL_ARG_LIST	16*/
 "Illegal argument list",
/* SUBSC_RANGE	17*/
 "Subscript/index/coordinate out of range",
/* INCMP_INNER_BC	18*/
 "Inner dimension Byte count is not a multiple of element length",
/* LUX_SUB_ARG	19*/
 "(X) Impossible error in lux_sub_arg",
/* NEED_ARR	20*/
 "Need numerical array",
/* N_DIMS_OVR	21*/
 "Too many dimensions",
/* DUPL_INDX	22*/
 "Duplicate index",
/* IMPOSSIBLE	23*/
 "An impossible error",
/* N_STR_DIMS_OVR	24*/
 "Too many subscripts for a string",
/* NEED_DIMS	25*/
 "Need dimensions",
/* ARR_SMALL	26*/
 "Array too small",
/* WRNG_N_ARG	27*/
 "Wrong number of arguments",
/* ILL_CMB_S_NON_S	28*/
 "Illegal combination of string and non-string",
/* SUBSC_NO_INDX	29*/
 "Variable cannot be subscripted",
/* ILL_N_SUBSC	30*/
 "Illegal number of subscripts",
/* ILL_REARRANGE	31*/
 "Illegal dimension rearrangement attempted",
/* ILL_SUBSC_TYPE	32*/
 "Illegal subscript type",
/* DIM_SMALL	33*/
 "Negative dimension",
/* ONLY_1_IF_ARR	34*/
 "Only one argument allowed if an array is used",
/* NEED_STR	35*/
 "Need a string argument",
/* ILL_LUN		36*/
 "Illegal logical unit",
/* LUN_CLOSED	37*/
 "No file is open on the logical unit",
/* READ_ONLY	38*/
 "File is open for reading only",
/* ILL_FORMAT_STR	39*/
 "Illegal format string",
/* UNDEF_ARG	40*/
 "Undefined argument",
/* USED_LUN	41*/
 "Logical unit is already in use",
/* READ_EOF	42*/
 "Reached the end of the file",
/* POS_ERR	43*/
 "Disk file positioning error",
/* READ_ERR	44*/
 "Disk file read error",
/* WRITE_ERR	45*/
 "Disk file write error",
/* ERR_OPEN	46*/
 "Could not open the file",
/* INCMP_ARG	47*/
 "Argument is incompatible with its predecessors",
/* NEED_POS_ARG	48*/
 "Need positive argument",
/* NEED_2D_ARR	49*/
 "Need two-dimensional real array",
/* EMPTY_STACK	50*/
 "Stack is empty",
/* NEED_INT_ARG	51*/
 "Need integer argument",
/* INDX_RANGE	52*/
 "Index out of range",
/* COORD_RANGE	53*/
 "Coordinate(s) out of range",
/* NEED_1D_ARR	54*/
 "Need one-dimensional array",
/* RET_ARG_NO_ATOM	55*/
 "Illegal return argument",
/* NEED_2D_SQ_ARR	56*/
 "Need square two-dimensional array",
/* INCMP_LU_RHS	57*/
 "Incompatible LU decomposition and RHS in DSOLVE",
/* ILL_POWER	58*/
 "Illegal power requested",
/* NEED_1D_2D_ARR	59*/
 "Need one-dimensional or two-dimensional array",
/* BAD_CONTOURS	60*/
 "Bad $CONTOURS array",
/* WIN_NOT_EXIST	61*/
 "Window does not exist",
/* BAD_GRID	62*/
 "Displacement grid not in correct format",
/* NEED_3x3_ARR	63*/
 "Need 3-by-3 array",
/* NEED_NTRV_2D_ARR	64*/
 "Need non-trivial two-dimensional array",
/* N_ARG_OVR	65*/
 "Too many arguments",
/* ILL_SUSBC_LHS	66*/
 "Illegal subscripts on left-hand side variable",
/* WR_N_SUBSC	67*/
 "Incompatible number of subscripts",
 /* BAD_STRUCT_KEY	68 */
 "Bad key to element of structure",
/* ILL_TYPE_IN    69*/
#ifdef __STDC__
 "Illegal type (%d, %s) in %s",
#else
 "Illegal type",
#endif
/* NEED_4x4_ARR	70 */
 "Need 4-by-4 array",
/* NEED_3_ARR 71 */
 "Need 3-element array",
/* ILL_PROJ_MAT 72 */
 "Illegal projection matrix",
/* NEED_2_ARR 73 */
 "Need 2-element array",
/* ILL_COORD_SYS 74 */
#ifdef __STDC__
 "Illegal coordinate system, number %1d",
#else
 "Illegal coordinate system",
#endif
/* WRITE_ONLY 75 */
 "File is open for writing only",
/* RANGE_START 76 */
#ifdef __STDC__
 "Illegal range start %1d (object size %1d)",
#else
 "Illegal range start",
#endif
/* RANGE_END 77 */
#ifdef __STDC__
 "Illegal range end %1d (object size %1d)",
#else
 "Illegal range end",
#endif
/* ILL_SUBSC 78 */
#ifdef __STDC__
 "Subscript out of range (value: %1d, size: %1d)",
#else
 "Subscript out of range",
#endif
/* INCMP_KEYWORD 79 */
 "Incompatible keywords",
 /* NEED_SCAL 80 */
 "Need a scalar here",
 /* ILL_AXIS 81 */
 "Illegal axis number: %1d",
 /* NEED_NUM_ARR 82 */
 "Need numerical array",
 /* NEED_NAMED 83 */
 "Need a named variable",
 /* NO_COMPLEX 84 */
 "Complex numbers are not supported by this routine",
 /* ILL_NUM_DIM 85 */
 "Illegal number of dimensions for current argument",
 /* NEED_REAL_SCAL 86 */
 "Need a real scalar here",
 /* NEED_REAL_ARR 87 */
 "Need a real array here",
 /* NEED_REAL_ARG 88 */
 "Need a real scalar or array here",
#ifndef X11
/* NO_X11 100 */
 "Sorry, this version of LUX was compiled without X Window support",
#endif
 "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""
 /* NEED_BYTE 118 */
 "Need BYTE argument"
};

int32_t     nErrorMessages = sizeof(errorMessages)/sizeof(char **);
#if __STDC__
char	*what(int32_t, char);
#else
char	*what();
#endif
extern char	*currentInputFile;
extern int32_t	fileLevel;
int32_t	errorSym = 0;
char	*errorPtr = NULL;
static char	storedErrorMessage[256];

/*-------------------------------------------------------------------*/
char *verrorMessage(char const* message, int32_t symbol, va_list ap)
/* returns error messages */
{
  char	*ptr;

  ptr = curScrat;
  if (currentInputFile)
    sprintf(curScrat, "%s", currentInputFile);
  else if (fileLevel)
    sprintf(curScrat, "%1d", fileLevel);
  else
    sprintf(curScrat, "(main)");
  curScrat += strlen(curScrat);	/* NOTE: sprintf does not return the number */
				/* of printed characters on all platforms. */
  sprintf(curScrat, ": ");
  curScrat += strlen(curScrat);
  if (symbol)
    symbolIdent(symbol, I_LINE | I_TRUNCATE | I_LENGTH);
  else
    sprintf(curScrat, "%d", curLineNumber);

  curScrat += strlen(curScrat);

  sprintf(curScrat, ": ");
  curScrat += 2;
  vsprintf(curScrat, message, ap);
  curScrat = ptr;
  return curScrat;
}
/*-------------------------------------------------------------------*/
char *errorMessage(char const* message, int32_t symbol, ...)
{
  char	*result;
  va_list	ap;

  va_start(ap, symbol);
  result = verrorMessage(message, symbol, ap);
  va_end(ap);
  return result;
}
/*-------------------------------------------------------------------*/
int32_t luxerror(char const* message, int32_t symbol, ...)
/* displays error messages */
{
  va_list	ap;

  va_start(ap, symbol);
  puts(verrorMessage(message, symbol, ap));
  va_end(ap);
  errorSym = symbol;
  if (errorPtr) {
    while (errorPtr-- > curScrat)
      putchar('-');
    putchar('^');
    putchar('\n');
    errorPtr = NULL;
  }
  return LUX_ERROR;
}
/*-------------------------------------------------------------------*/
int32_t cerror(int32_t message, int32_t symbol, ...)
/* displays error messages according to message numbers */
{
  va_list	ap;

  if (message == -1) {
    if (symbol_class(symbol) == LUX_UNUSED) { /* got zapped in the meantime */
      errorPtr = NULL;
      errorSym = 0;
      return LUX_ERROR;
    }
    puts(symbolIdent(symbol, I_FILELEVEL | I_LINE | I_TRUNCATE | I_LENGTH));
    if (errorPtr) {
      while (errorPtr-- > curScrat)
	putchar('-');
      putchar('^');
      putchar('\n');
      errorPtr = NULL;
    }
    if (errorSym != symbol)
      errorSym = symbol;
  } else {
    va_start(ap, symbol);
    if (message < 0 || message >= nErrorMessages)
      return luxerror("Illegal Error Number (%d)", 0, message);
    puts(verrorMessage(errorMessages[message], symbol, ap));
    va_end(ap);
    errorSym = symbol;
    if (errorPtr) {
      while (errorPtr-- > curScrat)
	putchar('-');
      putchar('^');
      putchar('\n');
      errorPtr = NULL;
    }
  }
  return LUX_ERROR;
}
/*-------------------------------------------------------------------*/
int32_t lux_error(int32_t narg, int32_t ps[])
/* allows the user to generate an error message */
/* syntax:  error [,format,symbol] [, /store, /restore] */
{
  char const* format;
  int32_t    symbol;

  if (narg) {
    format = string_arg(*ps++);
    symbol = (narg > 1)? *ps++: 0;
  } else {
    format = "(generic error)";
    symbol = 0;
  }
  switch (internalMode & 3) {
    case 0:
      return luxerror(format, symbol);
  case 1:			/* /STORE */
    errorMessage(format, symbol);
    strcpy(storedErrorMessage, curScrat);
    return 1;
  case 3:			/* /STORE,/RESTORE */
    errorMessage(format, symbol);
    strcpy(storedErrorMessage, curScrat);
    /* fall-thru */
  case 2:			/* /RESTORE */
    puts(storedErrorMessage);
    return LUX_ERROR;
  }
  return 1;			/* or some compilers complain */
}
/*---------------------------------------------------------*/
