/* This is file error.cc.

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
// File error.c
// LUX routines for generating error messages.
// error.c - error messages

#include "config.hh"
#include <map>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "action.hh"

std::map<int, char const*> errorMessages = {
  {ALLOC_ERR, "Memory allocation error"},
  {BAD_CONTOURS, "Bad $CONTOURS array"},
  {BAD_GRID, "Displacement grid not in correct format"},
  {BAD_STRUCT_KEY, "Bad key to element of structure"},
  {COND_NO_SCAL, "Condition must be scalar"},
  {CST_LHS, "Unmodifiable variable"},
  {DIM_SMALL, "Negative dimension"},
  {ERR_OPEN, "Could not open the file"},
  {ILL_ARG, "Illegal argument"},
  {ILL_ARG_LIST, "Illegal argument list"},
  {ILL_AXIS, "Illegal axis number: %1d"},
  {ILL_CLASS, "Argument is of an illegal class"},
  {ILL_CMB_S_NON_S, "Illegal combination of string and non-string"},
  {ILL_COMB, "Illegal argument combination (%s %s %s %s %s)"},
  {ILL_COORD_SYS, "Illegal coordinate system, number %1d"},
  {ILL_DIM, "Illegal dimension"},
  {ILL_LUN, "Illegal logical unit"},
  {ILL_NUM_DIM, "Illegal number of dimensions for current argument"},
  {ILL_N_SUBSC, "Illegal number of subscripts"},
  {ILL_POWER, "Illegal power requested"},
  {ILL_REARRANGE, "Illegal dimension rearrangement attempted"},
  {ILL_SUBSC, "Subscript out of range (value: %1d, size: %1d)"},
  {ILL_SUBSC_LHS, "Illegal subscripts on left-hand side variable"},
  {ILL_SUBSC_TYPE, "Illegal subscript type"},
  {ILL_SYM, "Illegal symbol number: %d (%s)"},
  {ILL_TYPE, "Expression type %s is not allowed here"},
  {ILL_W_STR, "Illegal operation with string"},
  {IMPOSSIBLE, "An impossible error"},
  {INCMP_ARG, "Argument is incompatible with its predecessors"},
  {INCMP_ARR, "Array is incompatible with previous one"},
  {INCMP_DIMS, "Incompatible dimensions"},
  {INCMP_KEYWORD, "Incompatible keywords"},
  {INCMP_LU_RHS, "Incompatible LU decomposition and RHS in DSOLVE"},
  {LUN_CLOSED, "No file is open on the logical unit"},
  {NEED_1D_2D_ARR, "Need one-dimensional or two-dimensional array"},
  {NEED_1D_ARR, "Need one-dimensional array"},
  {NEED_2D_ARR, "Need two-dimensional real array"},
  {NEED_2D_SQ_ARR, "Need square two-dimensional array"},
  {NEED_2_ARR, "Need 2-element array"},
  {NEED_3_ARR, "Need 3-element array"},
  {NEED_3x3_ARR, "Need 3-by-3 array"},
  {NEED_4x4_ARR, "Need 4-by-4 array"},
  {NEED_ARR, "Need numerical array"},
  {NEED_BYTE, "Need BYTE argument"},
  {NEED_INT_ARG, "Need integer argument"},
  {NEED_NAMED, "Need a named variable"},
  {NEED_NTRV_2D_ARR, "Need non-trivial two-dimensional array"},
  {NEED_NUM_ARR, "Need numerical array"},
  {NEED_POS_ARG, "Need positive argument"},
  {NEED_REAL_ARR, "Need a real array here"},
  {NEED_REAL_SCAL, "Need a real scalar here"},
  {NEED_SCAL, "Need a scalar here"},
  {NEED_STR, "Need a string argument"},
  {NOSUPPORT, "%s is not supported because this application was compiled without %s"},
  {NO_COMPLEX, "Complex numbers are not supported by this routine"},
  {NO_FLT_COND, "%s operator expects integer operands, not %s"},
  {NO_SCAL, "Argument must be a scalar"},
  {N_ARG_OVR, "Too many arguments"},
  {N_DIMS_OVR, "Too many dimensions"},
  {N_STR_DIMS_OVR, "Too many subscripts for a string"},
  {ONLY_1_IF_ARR, "Only one argument allowed if an array is used"},
  {ONLY_A_S, "Only numerical types allowed"},
  {POS_ERR, "Disk file positioning error"},
  {RANGE_END, "Illegal range end %1d (object size %1d)"},
  {RANGE_START, "Illegal range start %1d (object size %1d)"},
  {READ_EOF, "Reached the end of the file"},
  {READ_ERR, "Disk file read error"},
  {READ_ONLY, "File is open for reading only"},
  {RET_ARG_NO_ATOM, "Illegal return argument"},
  {SUBSC_NO_INDX, "Variable cannot be subscripted"},
  {SUBSC_RANGE, "Subscript/index/coordinate out of range"},
  {USED_LUN, "Logical unit is already in use"},
  {WRITE_ERR, "Disk file write error"},
  {WRITE_ONLY, "File is open for writing only"},
  {WRNG_N_ARG, "Wrong number of arguments"},
};

#if __STDC__
char        *what(int32_t, char);
#else
char        *what();
#endif
extern char        *currentInputFile;
extern int32_t        fileLevel;
int32_t        errorSym = 0;
char        *errorPtr = NULL;
static char        storedErrorMessage[256];

//-------------------------------------------------------------------
char const*
verrorMessage(char const* message, Symbol symbol, va_list ap)
// returns error messages
{
  char *ptr;

  ptr = curScrat;
  if (currentInputFile)
    sprintf(curScrat, "%s", currentInputFile);
  else if (fileLevel)
    sprintf(curScrat, "%1d", fileLevel);
  else
    sprintf(curScrat, "(main)");
  curScrat += strlen(curScrat); // NOTE: sprintf does not return the number
                                // of printed characters on all platforms.
  sprintf(curScrat, ": ");
  curScrat += strlen(curScrat);
  if (symbol) {
    symbolIdent(symbol, I_LINE | I_TRUNCATE | I_LENGTH);
    curScrat += strlen(curScrat);
    strcpy(curScrat, " (");
    curScrat += 2;
    symbolIdent(symbol, I_VALUE | I_TRUNCATE | I_LENGTH);
    curScrat += strlen(curScrat);
    strcpy(curScrat, ")");
    ++curScrat;
  } else {
    sprintf(curScrat, "%d", curLineNumber);
    curScrat += strlen(curScrat);
  }

  sprintf(curScrat, ": ");
  curScrat += 2;
  vsprintf(curScrat, message, ap);
  curScrat = ptr;
  return curScrat;
}
//-------------------------------------------------------------------
char const*
errorMessage(char const* message, int32_t symbol, ...)
{
  va_list ap;

  va_start(ap, symbol);
  auto result = verrorMessage(message, symbol, ap);
  va_end(ap);
  return result;
}
//-------------------------------------------------------------------
Symbol
luxerror(char const* message, Symbol symbol, ...)
// displays error messages
{
  va_list        ap;

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
//-------------------------------------------------------------------
Symbol
cerror(int32_t message, Symbol symbol, ...)
// displays error messages according to message numbers
{
  va_list        ap;

  if (message == -1) {
    if (symbol_class(symbol) == LUX_UNUSED) { // got zapped in the meantime
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
    auto it = errorMessages.find(message);
    if (it == errorMessages.cend())
      return luxerror("Illegal Error Number (%d)", 0, message);
    puts(verrorMessage(it->second, symbol, ap));
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
//-------------------------------------------------------------------
Symbol
lux_error(ArgumentCount narg, Symbol ps[])
// allows the user to generate an error message
// syntax:  error [,format,symbol] [, /store, /restore]
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
  case 1:                        // /STORE
    errorMessage(format, symbol);
    strcpy(storedErrorMessage, curScrat);
    return 1;
  case 3:                        // /STORE,/RESTORE
    errorMessage(format, symbol);
    strcpy(storedErrorMessage, curScrat);
    // fall-thru
  case 2:                        // /RESTORE
    puts(storedErrorMessage);
    return LUX_ERROR;
  }
  return 1;                        // or some compilers complain
}
REGISTER(error, s, error, 0, 2, "1store:2restore" );
//---------------------------------------------------------
