/* This is file execute.cc.

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
// File execute.c
// LUX statement executor and auxilliary routines.
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include "editor.hh"
#include "install.hh"
#include "action.hh"

extern int32_t  nFixed, traceMode;

int32_t         nArg, currentEVB = 0, suppressMsg = 0;
uint32_t internalMode;
int32_t         nExecuted = 0, executeLevel = 0, fileLevel = 0,
        returnSym, noTrace = 0, curRoutineNum = 0;
char    preserveKey;

char const* currentRoutineName = NULL;

int32_t         lux_convert(int32_t, int32_t *, Symboltype, int32_t),
  convertScalar(Scalar *, int32_t, Symboltype),
        dereferenceScalPointer(int32_t), eval(int32_t),
        nextCompileLevel(FILE *, char const *);
void    zap(int32_t symbol), updateIndices(void);
void pushExecutionLevel(int32_t line, int32_t target);
void popExecutionLevel(void);
void showExecutionLevel(int32_t symbol);

//------------------------------------------------------------------
void fixContext(int32_t symbol, int32_t context)
// change the context of embedded symbols (in LUX_RANGE, LUX_LIST,
// LUX_CLIST, LUX_KEYWORD) to reflect embedding in <symbol>.
{
  int32_t       i, nElem;
  int16_t       *ptr;
  listElem      *p;

  switch (symbol_class(symbol)) {
    case LUX_RANGE:
      i = range_start(symbol);
      if (i < 0)
        i = -i;
      if (symbol_context(i) == symbol)
        symbol_context(i) = context;
      i = range_end(symbol);
      if (i < 0)
        i = -i;
      if (symbol_context(i) == symbol)
        symbol_context(i) = context;
      i = range_redirect(symbol);
      if (i > 0 && symbol_context(i) == symbol)
        symbol_context(i) = context;
      break;
    case LUX_CLIST:
      ptr = clist_symbols(symbol);
      nElem = clist_num_symbols(symbol);
      while (nElem--)
        if (symbol_context(i = *ptr++) == symbol)
          symbol_context(i) = context;
      break;
    case LUX_LIST:
      p = list_symbols(symbol);
      nElem = list_num_symbols(symbol);
      while (nElem--) {
        if (symbol_context(i = p->value) == symbol)
          symbol_context(i) = context;
        p++;
      }
      break;
    case LUX_KEYWORD:
      if (symbol_context(i = keyword_name_symbol(symbol)) == symbol)
        symbol_context(i) = context;
      if (symbol_context(i = keyword_value(symbol)) == symbol)
        symbol_context(i) = context;
      break;
  }
}
//------------------------------------------------------------------
int32_t copyToSym(int32_t target, int32_t source)
// puts a copy of <source> (including any embedded symbols) in <target>
{
  int32_t       size, i;
  Pointer       optr, ptr;
  char  zapIt = 0;
  listElem      *eptr, *oeptr;
  int32_t       copySym(int32_t);
  extractSec    *etrgt, *esrc;
  void  embed(int32_t target, int32_t context), zap(int32_t);

  if (source < 0)               // some error
    return source;
  if (symbol_class(source) == LUX_SCAL_PTR) {
    source = dereferenceScalPointer(source);
    zapIt = 1;
  }
  symbol_class(target) = symbol_class(source);
  symbol_type(target) = symbol_type(source);
  // target symbol keeps its old context and line number
  switch (symbol_class(source)) {
    case LUX_SCAL_PTR:
      if (scal_ptr_type(source) == LUX_TEMP_STRING) {
        target = string_scratch(strlen(scal_ptr_pointer(source).s));
        strcpy(string_value(target), scal_ptr_pointer(source).s);
      } else {                  // numerical
        memcpy(&scalar_value(target), scal_ptr_pointer(source).b,
               sizeof(Scalar));
        symbol_class(target) = LUX_SCALAR;
      }
      break;
    case LUX_SCALAR:
      memcpy(&scalar_value(target), &scalar_value(source),
             sizeof(scalar_value(source)));
      break;
    case LUX_STRING: case LUX_FILEMAP: case LUX_ASSOC:
      size = symbol_memory(source);
      ALLOCATE(symbol_data(target), size, char);
      memcpy(symbol_data(target), symbol_data(source), size);
      symbol_memory(target) = size;
      if (symbol_class(source) == LUX_STRING)
        string_type(target) = LUX_TEMP_STRING;
      break;
    case LUX_RANGE:
      range_start(target) = (range_start(source) >= 0)?
        copySym(range_start(source)): -copySym(-range_start(source));
      range_end(target) = (range_end(source) >= 0)?
        copySym(range_end(source)): -copySym(-range_end(source));
      range_sum(target) = range_sum(source);
      range_redirect(target) = (range_redirect(source) >= 0)?
        copySym(range_redirect(source)): -1;
      embed(range_start(target), target);
      embed(range_end(target), target);
      embed(range_redirect(target), target);
      break;
    case LUX_CLIST:
      size = clist_num_symbols(source);
      optr.w = clist_symbols(source);
      ALLOCATE(clist_symbols(target), size, int16_t);
      symbol_memory(target) = symbol_memory(source);
      ptr.w = clist_symbols(target);
      while (size--) {
        *ptr.w = (int16_t) copySym(*optr.w++);
        embed(*ptr.w, target);
        ptr.w++;
      }
      break;
    case LUX_CPLIST:
      size = clist_num_symbols(source);
      optr.w = clist_symbols(source);
      ALLOCATE(clist_symbols(target), size, int16_t);
      symbol_memory(target) = symbol_memory(source);
      ptr.w = clist_symbols(target);
      while (size--) {
        *ptr.w = *optr.w++;
        ptr.w++;
      }
      break;
    case LUX_LIST:
      size = list_num_symbols(source);
      oeptr = list_symbols(source);
      ALLOCATE(list_symbols(target), size, listElem);
      symbol_memory(target) = symbol_memory(source);
      eptr = list_symbols(target);
      while (size--) {
        eptr->value = (int16_t) copySym(oeptr->value);
        embed(eptr->value, target);
        eptr->key = oeptr->key? strsave(oeptr->key): NULL;
        eptr++;
        oeptr++;
      }
      break;
    case LUX_STRUCT:
      size = symbol_memory(source);
      symbol_data(target) = malloc(size);
      if (!symbol_data(target))
        return cerror(ALLOC_ERR, 0);
      memcpy(symbol_data(target), symbol_data(source), size);
      break;
    case LUX_ARRAY:
      size = symbol_memory(source);
      if (!(array_header(target) = (array *) malloc(size)))
        return cerror(ALLOC_ERR, target);
      symbol_memory(target) = size;
      memcpy(array_header(target), array_header(source), sizeof(array));
      size = array_size(source);
      if (array_type(source) == LUX_STRING_ARRAY) {
        ptr.sp = (char **) array_data(target);
        optr.sp = (char **) array_data(source);
        while (size--) {
          if (*optr.sp)
            *ptr.sp = strsave(*optr.sp);
          else
            *ptr.sp = NULL;
          ptr.sp++;
          optr.sp++;
        }
      } else {                  // numerical array
        memcpy(array_data(target), array_data(source),
               size*lux_type_size[array_type(source)]);
      }
      break;
    case LUX_CSCALAR: case LUX_CARRAY: // complex scalar or array
      size = symbol_memory(source);
      complex_scalar_data(target).f = (float*) malloc(size);
      if (!complex_scalar_data(target).f)
        return cerror(ALLOC_ERR, target);
      symbol_memory(target) = size;
      memcpy(complex_scalar_data(target).f, complex_scalar_data(source).f,
             size);
      break;
    case LUX_EXTRACT:
      extract_target(target) = extract_target(source);
      symbol_memory(target) = symbol_memory(source);
      etrgt = (extractSec*) malloc(symbol_memory(target));
      if (!etrgt)
        return cerror(ALLOC_ERR, target);
      extract_ptr(target) = etrgt;
      size = extract_num_sec(source);
      esrc = extract_ptr(source);
      while (size--) {
        etrgt->type = esrc->type;
        etrgt->number = esrc->number;
        switch (esrc->type) {
          case LUX_RANGE:
            etrgt->ptr.w = (int16_t*) malloc(esrc->number*sizeof(int16_t));
            i = esrc->number;
            while (i--) {
              *etrgt->ptr.w = copySym(*esrc->ptr.w);
              embed(*etrgt->ptr.w, target);
              etrgt->ptr.w++;
              esrc->ptr.w++;
            }
            break;
          case LUX_LIST:
            etrgt->ptr.sp = (char**) malloc(esrc->number*sizeof(char *));
            i = esrc->number;
            while (i--) {
              *etrgt->ptr.sp = strsave(*esrc->ptr.sp);
              etrgt->ptr.sp++;
              esrc->ptr.sp++;
            }
            break;
        }
        esrc++;
        etrgt++;
      }
      break;
    case LUX_TRANSFER:
      transfer_target(target) = transfer_target(source);
      transfer_is_parameter(target) = (Symboltype) 0;
      transfer_temp_param(target) = 0;
      break;
    case LUX_FUNC_PTR:
      func_ptr_routine_num(target) = func_ptr_routine_num(source);
      func_ptr_type(target) = func_ptr_type(source);
      break;
    case LUX_UNDEFINED:
      break;
    default:
      printf("copyToSym - Sorry, class %d (%s) not yet implemented\n",
             symbol_class(source), className(symbol_class(source)));
      printf("symbol #%1d (%s)\n", source, varName(source));
      target = -1;
      break;
    }
  if (zapIt)
    zap(source);
  return target;
}
//------------------------------------------------------------------
int32_t copySym(int32_t symbol)
// creates an exact copy of <symbol> in a new temporary variable
{
  int32_t       result;

  result = nextFreeTempVariable();
  return copyToSym(result, symbol);
}
//------------------------------------------------------------------
#if REALLY
int32_t extractReplace(int32_t symbol)
{
  int32_t       target, lhs, rhs, result, n;
  int16_t       *ptr;
  char  findTarget = '\0', *name;
  int32_t       lux_replace(int32_t, int32_t);

  lhs = replace_lhs(symbol);
  if (symbol_class(lhs) == LUX_FUNC_PTR)
    return luxerror("Assignments to function pointers are illegal", lhs);
  rhs = replace_rhs(symbol);
  //  ptr = extract_ptr(lhs);
  n = extract_num_arg(lhs);
  target = *ptr;
  if (target < 0) {
    findTarget = 1;
    target = -target;
  }

  if (findTarget) {
    name = string_value(target);
    if ((result = lookForName(name, varHashTable, curContext)) < 0)
      // not found
      return cerror(ILL_SUBSC_LHS, lhs);
    zap(target);                // not needed anymore
    target = result;
  } else if (symbol_class(target) == LUX_EXTRACT)
    return luxerror("Only one subscript/tag allowed on left-hand side of assignment.", symbol);

  /* we assume that the syntax specification assures that target is a
     named variable */

  switch (extract_type(lhs)) {
    case LUX_RANGE:
      /* we have:  symbol: EVB_REPLACE lhs rhs
         we want to get:  symbol: EVB_INSERT (target, subscripts, rhs) */
      // we replace the extraction by a regular insertion
      symbol_class(symbol) = LUX_EVB;
      evb_type(symbol) = EVB_INT_SUB;
      int_sub_routine_num(symbol) = LUX_INSERT_SUB;
      // required argument list:  subscripts, source, target
      // current argument list: target subscripts
      int_sub_arguments(symbol) = ptr = realloc(ptr, (n + 1)*sizeof(int16_t));
      symbol_memory(symbol) = (n + 1)*sizeof(int16_t);
      if (!ptr)
        return LUX_ERROR;       // some reallocation error
      memmove(ptr, ptr + 1, n*sizeof(int16_t)); // now: subscripts ... ...
      ptr[n - 1] = rhs;                 // subscripts rhs ...
      ptr[n] = target;          // subscripts rhs target
      extract_ptr(lhs) = NULL;  // or else it will get zapped
      symbol_memory(lhs) = 0;
      zap(lhs);                         // not needed anymore
      suppressMsg++;
      return execute(symbol);
    case LUX_LIST:
      // have:  symbol: EVB_REPLACE lhs(LUX_EXTRACT) rhs
      // want:  symbol: EVB_REPLACE lhs(LUX_LIST_PTR) rhs
      symbol_class(lhs) = LUX_LIST_PTR;
      result = ptr[1];          // tag symbol
      if (symbol_class(ptr[1]) == LUX_SCALAR) {
        list_ptr_target(lhs) = -target;
        list_ptr_tag_number(lhs) = int_arg(result);
        zap(result);            // don't need this one anymore
        free(ptr);
      } else {                  // assume string
        list_ptr_target(lhs) = target;
        free(ptr);
        list_ptr_tag_string(lhs) = string_value(result);
        symbol_memory(lhs) = symbol_memory(result);
        symbol_memory(result) = 0;
        string_value(result) = NULL;
        zap(result);
      }
      suppressMsg++;
      return lux_replace(lhs, rhs);
    default:
      return cerror(ILL_SUBSC_LHS, lhs);
  }
}
#endif
//------------------------------------------------------------------
int32_t lux_replace(int32_t lhs, int32_t rhs)
     // replaces <lhs> with <rhs>
{
  int32_t       lhsSize, rhsSize, namevar(int32_t, int32_t), result, i, n;
  char  takeOver = 0;
  char const* name;
  extern int32_t        trace, step, nBreakpoint;
  extern breakpointInfo         breakpoint[];
  branchInfo    checkTree(int32_t, int32_t);
  int32_t       oldPipeExec, oldPipeSym;
  extern int32_t        pipeExec, pipeSym, tempVariableIndex, nTempVariable,
    fformat, iformat, sformat, cformat;
  extern char   *fmt_float, *fmt_integer, *fmt_complex, *fmt_string;
  branchInfo    tree;
  int32_t       evalLhs(int32_t), einsert(int32_t, int32_t);
  void  updateIndices(void);

  if (lhs == LUX_ERROR)                 // e.g., when !XX = 3 is tried
    return luxerror("Illegal variable", 0);
  if (lhs < nFixed)
    return luxerror("Cannot modify %s", 0, varName(lhs));
  lhsSize = lhs;
  lhs = evalLhs(lhs);
  if (lhs == LUX_ERROR)
    return LUX_ERROR;

  if (nBreakpoint) {            // have breakpoints: check for /VAR ones
    i = 0;
    for (n = nBreakpoint; n; ) {
      if (breakpoint[i].status & BP_DEFINED) {
        n--;
        if ((breakpoint[i].status & (BP_ENABLED | BP_VARIABLE)) ==
            (BP_ENABLED | BP_VARIABLE)
            && !strcmp(symbolProperName(lhs), breakpoint[i].name)) {
          step = 999;
          break;
        }
        i++;
      }
    }
  }

  if (symbol_class(lhs) == LUX_EXTRACT
      || symbol_class(lhs) == LUX_PRE_EXTRACT) {
    rhs = eval(rhs);
    if (rhs == LUX_ERROR)
      return LUX_ERROR;
    result = einsert(lhs, rhs);
    zapTemp(rhs);
    return result;
  }

  oldPipeExec = pipeExec;       // save because there may be nested
                                // invocations of lux_replace
  oldPipeSym = pipeSym;
  tree = checkTree(lhs, rhs);   // check potential replacement status
  if ((symbol_class(lhs) != LUX_ARRAY
       && symbol_class(lhs) != LUX_CARRAY)
      || symbol_memory(lhs) != tree.size) {
    if (!tree.containLHS &&
        (symbol_class(lhs) == LUX_ARRAY
         || symbol_class(lhs) == LUX_CARRAY))
      undefine(lhs);
    pipeExec = pipeSym = 0;
  } else {                      // piping OK
    pipeExec = tree.symbol;
    pipeSym = lhs;
  }
  rhs = evals(rhs);
  updateIndices();
  pipeExec = oldPipeExec;       // restore
  pipeSym = oldPipeSym;
  if (rhs < 0)                  // some error
    return LUX_ERROR;
  if ((trace > executeLevel && !noTrace && (traceMode & 127))
      || step > executeLevel) {
                                // print replacement info
    name = symbolProperName(lhs);
    if (!name && lhsSize != lhs) // unnamed and pointer target
      name = symbolProperName(lhsSize);
    if (name) {                         // if still unnamed, then it
                                // is likely an unconnected temp
      printf("      ( %s = ", name);
      if (symbolProperName(rhs))
        printf("%s = ", symbolProperName(rhs));
      printf("%s )\n", symbolIdent(rhs, I_VALUE | I_LENGTH | I_TRUNCATE));
    }
  }
  if (lhs == rhs)               // done already
    return 1;
                                // Treat LHS SCAL_PTRs separately
  if (symbol_class(lhs) == LUX_SCAL_PTR) {
    switch (symbol_class(rhs)) {
      default:
        return luxerror("Cannot assign non-scalar non-string value to %s\n",
                     0, varName(lhs));
      case LUX_STRING:
        // if the lhs is a string pointer, then remove the string
        if (scal_ptr_type(lhs) == LUX_TEMP_STRING
            && symbol_memory(lhs))
          free(scal_ptr_pointer(lhs).s);
        scal_ptr_type(lhs) = LUX_TEMP_STRING;
        if (isFreeTemp(rhs)) {  // just take over
          scal_ptr_pointer(lhs).s = string_value(rhs);
          symbol_memory(rhs) = 0; // or may get zapped
        } else {
          scal_ptr_pointer(lhs).s = strsave(string_value(rhs));
          symbol_memory(lhs) = symbol_memory(rhs);
        }
        if (lhs >= fformat && lhs <= cformat) {
          if (lhs == fformat)
            fmt_float = scal_ptr_pointer(lhs).s;
          else if (lhs == iformat)
            fmt_integer = scal_ptr_pointer(lhs).s;
          else if (lhs == sformat)
            fmt_string = scal_ptr_pointer(lhs).s;
          else
            fmt_complex = scal_ptr_pointer(lhs).s;
        }
        break;
      case LUX_SCALAR:
        rhs = lux_convert(1, &rhs, scal_ptr_type(lhs), 1);
        switch (scal_ptr_type(lhs)) {
          case LUX_INT8:
            *scal_ptr_pointer(lhs).b = scalar_value(rhs).b;
            break;
          case LUX_INT16:
            *scal_ptr_pointer(lhs).w = scalar_value(rhs).w;
            break;
          case LUX_INT32:
            *scal_ptr_pointer(lhs).l = scalar_value(rhs).l;
            break;
          case LUX_INT64:
            *scal_ptr_pointer(lhs).q = scalar_value(rhs).q;
            break;
          case LUX_FLOAT:
            *scal_ptr_pointer(lhs).f = scalar_value(rhs).f;
            break;
          case LUX_DOUBLE:
            *scal_ptr_pointer(lhs).d = scalar_value(rhs).d;
            break;
        }
      }
    zapTemp(rhs);
    return 1;
  }

  // Determine memory allocations of rhs
  switch (symbol_class(rhs)) {
    case LUX_SCALAR: case LUX_SCAL_PTR: case LUX_UNDEFINED:
    case LUX_UNUSED: case LUX_TRANSFER:
      rhsSize = 0;              // no extra memory allocated for these
      break;
    case LUX_RANGE:
      rhsSize = 1;              // this one has extra memory allocated
      break;
    default:
      rhsSize = symbol_memory(rhs);
  }
  takeOver = isFreeTemp(rhs) || !rhsSize;
  if (takeOver)         {               // just take over RHS symbol
    undefine(lhs);
    fixContext(rhs, lhs);
    memcpy(&symbol_extra(lhs), &symbol_extra(rhs), sizeof(symbol_extra(rhs)));
    if (symbol_class(rhs) == LUX_TRANSFER) {
      symbol_class(lhs) = LUX_TRANSFER;
      transfer_is_parameter(lhs) = (Symboltype) 0;
      transfer_temp_param(lhs) = 0;
    } else
      symbol_class(lhs) = symbol_class(rhs);
    symbol_type(lhs) =
      (symbol_class(rhs) == LUX_STRING)? LUX_TEMP_STRING: symbol_type(rhs);
    if (isFreeTemp(rhs)) {
      symbol_class(rhs) = LUX_UNUSED; // or linked variables get zapped
      symbol_memory(rhs) = 0;   // or memory gets deallocated
      if (rhs < tempVariableIndex)
        tempVariableIndex = rhs;
      nTempVariable--;
    }
  // in structures, lists and ranges, the context of the elements must be
  // modified to reflect the new situation
    updateIndices();
  } else {                      // we need a copy of the RHS
    undefine(lhs);
    copyToSym(lhs, rhs);
    fixContext(lhs, lhs);
  }
  return 1;
}
//------------------------------------------------------------------
#define NOKEY           -1000
#define ORKEY           -1001
#define MODEKEY                 -1002
#define ZEROKEY                 -1003
int32_t matchKey(int16_t index, char **keys, int32_t *var)
/* matches symbol[index] to the keyword list and returns index
  of matched key (or NOKEY) */
// if a key is preceded by a number, then the number is OR-ed into
// internalMode if the corresponding key is selected.  if NO + key
// is selected, then the number is not-OR-ed.  if # precedes then
// the flag preserveKey is set and the key is not evaluated, i.e.
// is passed on to the internal routine as is.  If the number is
// followed by $, then internalMode is updated AND the index of the
// key is returned.
// the routine returns one of the values NOKEY, ORKEY, MODEKEY, ZEROKEY,
// or the index to the parameter list associated with the key (which is
// always nonnegative).  *var returns the same, except in the ZEROKEY case,
// in which *var receives the index to the parameter list.
{
  char  *key, *theKey, modeKey, negate;
  uint32_t      n, l, indx, theMode;

 if (symbol_class(index) != LUX_STRING)
   return luxerror("Non-string keyword??", index);
 key = string_value(index);
 l = strlen(key);
 if (keys) {                    // there is a key list
   for (n = indx = 0; *keys; n++) {
     preserveKey = 0;
     modeKey = 0;
     negate = 0;
     theKey = *keys++;
     if (*theKey == '~') {      // unset bits corresponding to mode
       negate = 1;
       theKey++;
     }
     if (isdigit((uint8_t) *theKey)) {
       modeKey = 1;
       theMode = strtol(theKey, &theKey, 10);
     }
     if (*theKey == '#') {
       preserveKey = 1;
       theKey++;
     }
     if (!strncmp(key, theKey, l)) {
       if (modeKey) {
         if (negate)
           internalMode &= ~theMode;
         else
           internalMode |= theMode;
         *var = ORKEY;
         return *var;
       }
       *var = indx;
       return *var;
     }
     else if (l > 2 && !strncmp(key + 2, theKey, l - 2)
              && !strncmp(key, "no", 2)) {
       if (modeKey) {
         if (negate)
           internalMode |= theMode;
         else
           internalMode &= ~theMode;
         *var = ORKEY;
         return *var;
       }
       *var = indx;
       return ZEROKEY;
     }
     if (!modeKey)
       indx++;
   }
 }
 if (!strcmp(key, "MODE"))
   return *var = MODEKEY;
 return *var = NOKEY;
}
//------------------------------------------------------------------
int32_t matchUserKey(char *name, int32_t routineNum)
// matches name to the names of the arguments of user routine #routineNum
// and returns the argument index of the matched key (or -1)
{
  char  **keys;
  int32_t       n, l, i;

  if (symbol_class(routineNum) != LUX_SUBROUTINE &&
      symbol_class(routineNum) != LUX_FUNCTION)
    return luxerror("Matching keyword to non-routine??", routineNum);
  l = strlen(name);
  keys = routine_parameter_names(routineNum);
  n = routine_num_parameters(routineNum);
  for (i = 0; i < n; i++) {
    if (!strncmp(name, *keys, l))
      return i;
    else if (l > 2 && !strncmp(name, "NO", 2) &&
             !strncmp(name + 2, *keys, l - 2)) return -i - 1;
    keys++;
  }
  return NOKEY;
}
//------------------------------------------------------------------
#define PRESERVE_KEY    1024
int32_t internal_routine(int32_t symbol, internalRoutine *routine)
// execute an internal routine (subroutine or function)
// keywords:  each internal function and routine has associated with it
// a (possibly empty) list of keywords, associated with particular
// arguments to the routine, or associated with an external variable
// called internalRoutine.  The keyword lists can be found in file
// install.c.  plain keywords (i.e. without a prefixed number)
// connect a user-specified value with a particular argument to the
// routine.  numbered keywords logically OR internalRoutine with the
// number, if the keyword is recognized, or AND with the binary complement
// of the number if the user-typed keyword is preceded by NO.  the special
// keyword MODE is recognized by all routines and any associated value is
// assigned to internalMode.
{
 /* sym[symbol].xx -> routine number
    sym[symbol].spec.array.ptr -> arguments
    sym[symbol].spec.array.bstore/sizeof(int16_t) -> # arguments  */

  // evalArgs must be int32_t* because it is passed on to individual
  // routines (int32_t ps[])
 int32_t        nArg, nKeys = 0, i, maxArg, *evalArgs,
        routineNum, n, thisInternalMode = 0, ordinary = 0;
 uint8_t        isSubroutine;
 keyList        *theKeyList;
 int16_t        *arg;
 char   *name, suppressEval = 0, suppressUnused = 0;
 extern char    evalScalPtr;
 extern int32_t         pipeExec;
 int32_t        treatListArguments(int32_t *, int32_t *[], int32_t);

 nArg = int_sub_num_arguments(symbol); // number of arguments
 routineNum = int_sub_routine_num(symbol); // routine number
 isSubroutine = (routine == subroutine); // 1->sub, 0->func
 arg = int_sub_arguments(symbol); // pointer to start of argument list

 theKeyList = (keyList *) routine[routineNum].keys; // keys of routine
 maxArg = routine[routineNum].maxArg; // max allowed number of arguments
 if (theKeyList) {
   suppressEval = theKeyList->suppressEval; // suppress evaluation of args
   thisInternalMode = internalMode = theKeyList->defaultMode; // defaults
   ordinary = theKeyList->offset; // first non-keyword argument goes in
                                  // ps[ordinary]
   suppressUnused = theKeyList->suppressUnused;
 }

 // how many keyword arguments?
 if (theKeyList) {              // routine has keys
   for (i = 0; i < nArg; i++)
     if (symbol_class(*arg++) == LUX_KEYWORD)
       nKeys++;
 } else {                       // routine has no keys
   for (i = 0; i < nArg; i++)
     if (symbol_class(*arg++) == LUX_KEYWORD)
       return luxerror("No keywords allowed with routine %s", symbol,
                    routine[routineNum].name);
 }

                // legal number of arguments?
 if (nArg - nKeys > routine[routineNum].maxArg)
   return luxerror("Too many arguments to %s %s: found %d,"
                   " cannot accept more than %d",
                   symbol, isSubroutine? "subroutine": "function",
                   routine[routineNum].name, nArg - nKeys,
                   routine[routineNum].maxArg);

 // treat the arguments
 if (nKeys? maxArg: (nArg + ordinary)) {// arguments possible
   i = nKeys? maxArg: (nArg + ordinary);
   evalArgs = (int32_t*) malloc(i*sizeof(int32_t));
   if (!evalArgs)
     return cerror(ALLOC_ERR, 0);
   if (!nKeys) {                // default: no keys
     evalArgs += nArg + ordinary; // start at the end
     for (i = nArg; i; i--) {
       --arg;
       n = (suppressEval? *arg: eval(*arg)); // next earlier argument
       if (!n) {        // 0 -> probably an LUX_TRANSFER to a non-existent
                        // symbol;  make LUX_UNDEFINED instead
         if (symbol_class(*arg) == LUX_TRANSFER // a TRANSFER
             && symbol_context(*arg)) {         // we're inside a routine
           symbol_class(*arg) = LUX_UNDEFINED; // make UNDEFINED instead
           n = *arg;            // and use
         } else {               // cannot have an LUX_TRANSFER to symbol 0
                                // at the main execution level
           free(evalArgs - i - ordinary);
           return cerror(ILL_CLASS, *arg);
         }
       }
       if (n < 0) {             // some error
         free(evalArgs - i - ordinary);
         return LUX_ERROR;
       }
       *--evalArgs = n;                 // store resulting symbol
     }
     // the argument slots before <ordinary> default to zero
     for (i = 0; i < ordinary; i++)
       *--evalArgs = 0;
     maxArg = nArg + ordinary;  // index of last specified argument
   } else {                     // we've got some keys
     int32_t    *keys;

     for (i = 0; i < maxArg; i++) // default max number of allowed arguments
                                  // for this routine to zero
       *evalArgs++ = 0;
     evalArgs -= maxArg;        // back to beginning of list
     arg -= nArg;               // back to beginning of list

     // match the keywords with the routine's keyword list
     ALLOCATE(keys, nArg, int32_t);     // storage
     for (i = maxArg = 0; i < nArg; i++, arg++, keys++)
       if (symbol_class(*arg) == LUX_KEYWORD) {         // a keyword
         n = matchKey(keyword_name_symbol(*arg),
                      theKeyList? theKeyList->keys: NULL, keys); // match
         // if a key is found, we want to store the nonnegative index
         // to the corresponding parameter in *key, or a negative number
         // which indicates that there is no parameter associated with
         // the keyword. e
         if (n == NOKEY) {      // no keyword matched
           free(keys - i);
           free(evalArgs);
           name = string_value(keyword_name_symbol(*arg));
           return luxerror("Nonexistent keyword: %s", 0, name);
         } else if (n == MODEKEY) { // MODE keyword
           int32_t result = eval(keyword_value(*arg));
           internalMode = int_arg(result);
           zapTemp(result);
         } else if (n != ORKEY) {       // not a mode (number) keyword
           if (*keys >= maxArg)         // proper index is beyond current max index
             maxArg = *keys + 1; // update max index
           if (preserveKey)
             *keys |= PRESERVE_KEY;
         }
         if (n == ZEROKEY)      // substitute zero value
           keyword_value(*arg) = LUX_ZERO;
       } else                   // no keyword, but we do have an argument
         *keys = 0;
     if (maxArg < nArg - nKeys + ordinary) // ??
       maxArg = nArg - nKeys + ordinary;
     keys -= nArg;              // point at first one again
     arg -= nArg;
     // evaluations may change internalMode, so save it for later
     thisInternalMode = internalMode;
     // put all arguments in proper position
     for (i = 0; i < nArg; i++, arg++) {
       if (*keys < 0) {                 // a mode keyword
         keys++;
         continue;
       }
       if (symbol_class(*arg) == LUX_KEYWORD) {         // a keyword associated with
                                                // a variable
         if (*keys & PRESERVE_KEY) {            // preserve keyword value
           *keys &= ~PRESERVE_KEY; // remove flag
           preserveKey = 1;     // but preserve keyword value
         } else                         // no preservation
           preserveKey = 0;
         if (evalArgs[*keys]) {         // already a value at indicated position
           free(keys - i + ordinary);
           free(evalArgs);
           return luxerror("Parameter #%d (%s) doubly defined", 0, *keys,
                        keyName(subroutine, routineNum, *keys));
         }
         n = keyword_value(*arg); // keyword value
         if (!suppressEval && !preserveKey)
           n = eval(n);                 // do evaluate
         if (!n) {              // probably a LUX_TRANSFER to a non-existent
                                // symbol;  make LUX_UNDEFINED instead
           if (symbol_class(*arg) == LUX_TRANSFER && symbol_context(*arg)) {
             symbol_class(*arg) = LUX_UNDEFINED;
             n = *arg;          // use it
           } else {             // should never get here
             puts("Strange argument in internal function??");
             free(keys - i + ordinary);
             free(evalArgs);
             return cerror(ILL_ARG, *arg);
           }
         }
         evalArgs[*keys] = n;   // store result at appropriate position
       }
       else if (evalArgs[ordinary]) { // no keyword here, but already a
                                      // value at indicated position
         free(keys - i + ordinary);
         free(evalArgs);
         return luxerror("Argument #%d doubly defined", 0, ordinary);
       } else                   // no keyword: just evaluate if allowed
         evalArgs[ordinary++] = suppressEval? *arg: eval(*arg);
       keys++;                  // next
     }
     free(keys - nArg);
   }
 } else {                       // no regular arguments allowed
   if (nKeys) {                         // nKeys non-zero and maxArg zero:
                                // only mode keywords are allowed
     arg -= nArg;
     for (i = 0; i < nArg; i++, arg++)
       if (symbol_class(*arg) == LUX_KEYWORD) {
         switch (matchKey(keyword_name_symbol(*arg),
                          theKeyList? theKeyList->keys: NULL, &n)) {
           case NOKEY:
             name = string_value(keyword_name_symbol(*arg));
             return luxerror("Nonexistent keyword: %s", 0, name);
           case MODEKEY:                // replace internalMode value
             internalMode = int_arg(eval(keyword_value(*arg)));
             break;
           case ORKEY:          // dealt with inside matchKey()
             break;
           default:
             return luxerror("This %s only accepts mode keywords", *arg,
                          isSubroutine? "subroutine": "function");
         }
       } else                   // no keyword -> illegal
         return luxerror("This %s only accepts mode keywords", *arg,
                      isSubroutine? "subroutine": "function");
   }
   thisInternalMode = internalMode; // internalMode may be modified during
                                    // execution of the statement: save
   evalArgs = NULL;
   maxArg = 0;                  // no regular arguments
 }

 // did any errors occur during evaluation of the arguments?
 for (i = 0; i < maxArg; i++)
   if (evalArgs[i] < 0) {       // yes: some error
     free(evalArgs);
     return LUX_ERROR;
   }

 switch (routineNum) {
   case LUX_SUBSC_FUN:
     n = -1;
     break;
   case LUX_INSERT_SUB:
     n = -2;
     break;
   default:
     n = 0;
     break;
 }
 if (!suppressEval              // we're not suppressing evaluation
     && evalArgs                // and we do have some arguments
                                // then expand.
     && treatListArguments(&maxArg, &evalArgs, n) == LUX_ERROR) {
   free(evalArgs);
   return LUX_ERROR;
 }

                // legal number of arguments?
 if (maxArg < routine[routineNum].minArg
     || maxArg > routine[routineNum].maxArg) {
   free(evalArgs);
   return luxerror("Illegal number of arguments to %s %s:"
                   "found %d, accept between %d and %d",
                   symbol, isSubroutine? "subroutine": "function",
                   routine[routineNum].name, maxArg,
                   routine[routineNum].minArg, routine[routineNum].maxArg);
 }

 // suppress unused (class 0) arguments if requested.  Remove any such
 // arguments from the end of the argument list and modify maxArg
 // accordingly.
 if (suppressUnused)
   for (i = maxArg - 1; i >= 0; i--) {
     if (evalArgs[i] && symbol_class(evalArgs[i]) == LUX_UNDEFINED
         && undefined_par(evalArgs[i]))
       evalArgs[i] = 0;
     if (i == maxArg - 1)       // last one
       maxArg--;
   }

 // scalar pointers shall be evaluated during execution of the routine
 evalScalPtr = 1;
                // execute
 curRoutineNum = routineNum;    // make routine number available globally
                                // so that we have access to the routine's
                                // keyword list inside the routine
 internalMode = thisInternalMode;
 if (symbol == pipeExec)
   pipeExec = 0;                // allow piping (for functions)
 i = (*routine[routineNum].ptr)(maxArg, evalArgs); // execute
 // now get rid of temporary variables created in the routine -- except
 // for the return value, of course
 for (ordinary = 0; ordinary < maxArg; ordinary++)
   if (isFreeTemp(evalArgs[ordinary]) && evalArgs[ordinary] != i)
     zap(evalArgs[ordinary]);
 updateIndices();
 free(evalArgs);
 return i;
}
//------------------------------------------------------------------
int32_t getBody(int32_t routine)
// finds and compiles body of uncompiled routine
{
 char   *name, isFunction;
 int32_t        result;
 extern int32_t         findBody, ignoreInput;
 FILE   *fp;

 name = deferred_routine_filename(routine);
 isFunction = (symbol_class(routine) == LUX_DEFERRED_FUNC);
 if (!(fp = openPathFile(name, (isFunction? FIND_FUNC: FIND_SUBR) | FIND_LOWER)))
   if (isFunction)
     fp = openPathFile(name, FIND_SUBR | FIND_LOWER);
 if (!fp)
   return luxerror("Could not open file %s.", 0,
                deferred_routine_filename(routine));
 findBody = routine;
 ignoreInput++;
 result = nextCompileLevel(fp, (char *) scrat);
 findBody = 0;
 ignoreInput--;
 fclose(fp);
 if (!routine_num_statements(routine))
   return luxerror("Read file %s but %s %s still not compiled.\n", 0,
         deferred_routine_filename(routine),
                isFunction? "function": "subroutine",
                symbolProperName(routine));
 return result;
}
//------------------------------------------------------------------
int32_t usr_routine(int32_t symbol)
// executes a user-defined subroutine, function or block routine
{
  // evalArg must be int32_t* because unProtect expects that, because
  // it does so in internal_routine because there it gets passed on
  // to individual routines (int32_t ps[])
 int32_t        nPar, nStmnt, i, oldContext = curContext, n, routineNum, nKeys = 0,
        ordinary = 0, thisNArg, *evalArg, oldNArg, listSym = 0;
 int16_t        *arg, *par, *list = NULL;
 char   type, *name, msg, isError;
 char const* routineTypeNames[] = { "func", "subr", "block" };
 symTableEntry  *oldpars;
 extern int32_t         returnSym, defined(int32_t, int32_t);
 extern char    evalScalPtr;
 void pushExecutionLevel(int32_t, int32_t), popExecutionLevel(void);

 executeLevel++;
 fileLevel++;
 if ((traceMode & T_ROUTINE) == 0)
   noTrace++;
 routineNum = usr_routine_num(symbol); // routine number
 if (symbol_class(routineNum) == LUX_FUNC_PTR) {
   // must point at user-defined
   if (func_ptr_type(routineNum) <= 0) { // internal routine??
     executeLevel--;            // restore
     fileLevel--;               // restore
     if ((traceMode & T_ROUTINE) == 0)
       noTrace--;               // restore
     return luxerror("Trying to execute a non-user-routine!", symbol);
   }
 }
 switch (symbol_class(routineNum)) {
   case LUX_FUNCTION:
     type = 0;                  // a function
     break;
   case LUX_SUBROUTINE:
     type = 1;                  // a subroutine
     break;
   case LUX_BLOCKROUTINE:       // a block routine
     type = 2;
     break;
   case LUX_DEFERRED_SUBR:
     if (getBody(routineNum) < 0) { // compilation failed
       if ((traceMode & T_ROUTINE) == 0)
         noTrace--;             // restore
       return LUX_ERROR;
     }
     type = 1;
     break;
   case LUX_DEFERRED_FUNC:
     if (getBody(routineNum) < 0) { // compilation failed
       if ((traceMode & T_ROUTINE) == 0)
         noTrace--;             // restore
       return LUX_ERROR;
     }
     type = 0;
     break;
   case LUX_DEFERRED_BLOCK:
     if (getBody(routineNum) < 0) { // compilation failed
       if ((traceMode & T_ROUTINE) == 0)
         noTrace--;             // restore
       return LUX_ERROR;
     }
     type = 2;
     break;
   default:
     executeLevel--;            // restore
     fileLevel--;               // restore
     if ((traceMode & T_ROUTINE) == 0)
       noTrace--;               // restore
     return luxerror("Trying to execute a non-routine!", symbol);
 }
 nStmnt = routine_num_statements(routineNum);
 par = routine_parameters(routineNum);
 nPar = routine_num_parameters(routineNum); // number of parameters
                // (formal arguments) in the routine's definition
 msg = (traceMode & T_ROUTINEIO);
 if (msg) {
   currentRoutineName = symbolProperName(routineNum);
   printf("Entering %s %s", routineTypeNames[(uint8_t) type],
          currentRoutineName);
   if (!type)
     putchar('(');
   if (nPar) {
     if (type)
       putchar(',');
     for (i = 0; i < nPar; i++) {
       printf("%s", symbolProperName(par[i]));
       if (i != nPar - 1 && !routine_has_extended_param(routineNum))
         putchar(',');
     }
   }
   if (routine_has_extended_param(routineNum))
     printf(",...");
   if (!type)
     putchar(')');
   putchar('\n');
 }
 pushExecutionLevel(symbol_line(symbol), routineNum);

        // "arguments" are actual arguments; "parameters" formal arguments
 evalArg = NULL;
 thisNArg = 0;
 if (symbol_class(routineNum) != LUX_BLOCKROUTINE) { // subroutine/ function
   thisNArg = usr_routine_num_arguments(symbol); // number of arguments in
                                                 // the call
   // cannot use real nArg yet because it may be changed during evaluation
   // of the arguments
   if (thisNArg > nPar
       && !routine_has_extended_param(routineNum)) { // # args > # params
     luxerror("Too many arguments specified in %s %s\n", 0,
           type? "subroutine": "function",
           type? subrName(routineNum): funcName(routineNum));
     goto usr_routine_1;
   }
   arg = usr_routine_arguments(symbol);         // start of argument list
   // evaluate actual arguments.  By first evaluating all arguments
   // and only then linking them to the routine's formal parameters,
   // we can allow recursion during calculation of the parameters
   for (i = 0; i < thisNArg; i++) // count the number of keyword assignments
     if (symbol_class(*arg++) == LUX_KEYWORD)
       nKeys++;
   arg -= thisNArg;             // back to start of list
   evalArg = (int32_t*) malloc(nPar*sizeof(int32_t)); // room for the evaluated parameters
   if (nPar && !evalArg) {
     cerror(ALLOC_ERR, 0);
     goto usr_routine_1;
   }
   for (i = 0; i < nPar; i++)
     *evalArg++ = 0;            // initialize to 0 ( = unspecified )
   evalArg -= nPar;             // back to start of list
   ordinary = 0;                // counts the number of ordinary
                                // ( = positional ) arguments
   if (thisNArg) {              // arguments were specified
     for (i = 0; i < thisNArg; i++) {
       if (symbol_class(*arg) == LUX_KEYWORD) { // a keyword assignment
         n = keyword_name_symbol(*arg);         // the keyword string symbol
         name = string_value(n); // keyword name
         n = matchUserKey(name, routineNum); // does the keyword name
         // match the name of one of the routine's parameters?
         if (n == NOKEY) {      // no match: illegal keyword
           luxerror("Nonexistent keyword: %s\n", *arg, name);
           goto usr_routine_2;
         } // end if (n == NOKEY)
         if (evalArg[n < 0? -n - 1: n]) { // keyword param already defined
           luxerror("Argument #%d doubly defined", 0, n < 0? -n - 1: n);
           goto usr_routine_2;
         } // end if (evalArg[n < 0? -n -1: n])
         if (n >= 0) {          // keyword is recognized as is
           evalArg[n] = eval(keyword_value(*arg++)); // assign value
           if (evalArg[n] == LUX_ERROR)         // some error
             goto usr_routine_2;
           if (isFreeTemp(evalArg[n])) // if the argument is a temporary
             // symbol, then give it the routine's context so it isn't
             // deleted too early
             symbol_context(evalArg[n]) = routineNum;
         } else {               // keyword was recognized after stripping
                                // initial NO
           if (keyword_value(*arg++) != 1)
             puts("Ignored value on NO.. keyword");
           evalArg[-n - 1] = LUX_ZERO;
         } // end if (n >= 0) else
       } else {                         // a positional assignment (ie, no keyword)
         if (ordinary == nPar - 1 && thisNArg > nPar) {
           /* at end of ordinary argument list, and still have arguments:
            must be an extended argument */
           listSym = nextFreeTempVariable();
           if (listSym == LUX_ERROR)
             goto usr_routine_2;
           symbol_class(listSym) = LUX_CPLIST;
           symbol_memory(listSym) = (thisNArg - nPar + 1)*sizeof(int16_t);
           list = (int16_t*) malloc(symbol_memory(listSym));
           if (!list) {
             cerror(ALLOC_ERR, listSym);
             goto usr_routine_3;
           }
           clist_symbols(listSym) = list;
           evalArg[ordinary] = listSym;
           symbol_context(listSym) = routineNum;
         }
         if (ordinary < nPar - 1 || !list) {
           if (evalArg[ordinary]) { // the current positional parameter
             // already has a (keyword-specified) value assigned to it
             luxerror("Argument #%d doubly defined", 0, ordinary);
             goto usr_routine_3;
           } // end if (evalArg[ordinary])
           evalArg[ordinary] = eval((int32_t) *arg++); // assign value
           if (evalArg[ordinary] == LUX_ERROR) // some error
             goto usr_routine_3;
           if (isFreeTemp(evalArg[ordinary]))
             symbol_context(evalArg[ordinary]) = routineNum;
         } else {
           *list = eval(*arg++); // assign value
           if (isFreeTemp(*list))
             symbol_context(*list) = listSym;
           list++;
         }
         ordinary++;
       } // end if (symbol_class(*arg) == LUX_KEYWORD)
     } // end for (i = 0; i < thisNArg; i++)
   } // end if (thisNArg)
 } // end if (symbol_class(routineNum != LUX_BLOCKROUTINE)

 // did an error occur during evaluation of the arguments?
 isError = 0;
 for (i = 0; i < nPar; i++)
   if (evalArg[i] < 0) {
     evalArg[i] = 0;            /* so we can safely investigate them
                                   when we're cleaning up */
     isError = 1;
   }
 if (!isError) {                // no error occurred
   // scalar pointers shall be evaluated during execution of the routine
   evalScalPtr = 1;
   // link actual arguments to formal arguments
   // the formal arguments are all TRANSFERs, except when one of them was
   // not specified by the user yet used in the routine

   // We must make copies of the current values of the parameters, just
   // in case we're recursively entering the routine.  The trick of
   // evaluating all arguments first and only linking them to parameters
   // later is insufficient in the following example:
   // subr foo,x d,x if x eq 0 then return foo,x-1 d,x endsubr foo,5
   /* The response without parameter copies is as follows:
      201 FOO.X     pointer, points at <25000>
      *25000 (unnamed)  scalar, LONG, value = 5
      201 FOO.X     pointer, points at <5000>
      *5000 (unnamed)  scalar, LONG, value = 4
      201 FOO.X     pointer, points at <5001>
      *5001 (unnamed)  scalar, LONG, value = 3
      201 FOO.X     pointer, points at <5002>
      *5002 (unnamed)  scalar, LONG, value = 2
      201 FOO.X     pointer, points at <5003>
      *5003 (unnamed)  scalar, LONG, value = 1
      201 FOO.X     pointer, points at <5004>
      *5004 (unnamed)  scalar, LONG, value = 0
      201 FOO.X     pointer, points at <0>
      201 FOO.X     pointer, points at <0>
      201 FOO.X     pointer, points at <0>
      201 FOO.X     pointer, points at <0>
      201 FOO.X     pointer, points at <0> */
   if (nPar) {
     oldpars = (symTableEntry *) malloc(nPar*sizeof(symTableEntry));
     if (!oldpars) {
       cerror(ALLOC_ERR, 0);
       goto usr_routine_3;
     }
   } else
     oldpars = NULL;
   for (i = 0; i < nPar; i++)
     memcpy(oldpars + i, &sym[par[i]], sizeof(symTableEntry));

   for (i = 0; i < nPar; i++) {
     if (evalArg[i]) {          // have an argument
       if (symbol_class(*par) != LUX_TRANSFER) {
         undefine(*par);
         symbol_class(*par) = LUX_TRANSFER;
         transfer_is_parameter(*par) = (Symboltype) 1;
         transfer_temp_param(*par) = 0;
       }
       if (evalArg[i] < nFixed) { // fixed number -> assign value to par
         // if we use a pointer to the fixed number, then we cannot
         // modify the parameter inside the routine.
         undefine(*par);
         lux_replace(*par++, evalArg[i]);
       } else
         transfer_target(*par++) = evalArg[i];
     } else {
       undefine(*par);
       undefined_par(*par) = (Symboltype) 1; // this parameter has not been assigned
       // a value by the user
       par++;
     }
   }
   if (symbol_class(routineNum) != LUX_BLOCKROUTINE)
     curContext = routineNum;
 } else {
   evalArg = 0;
   thisNArg = 0;
 }
 // now set !NARG (nArg) to proper value
 // save old value for later restoration - LS 2jun97
 oldNArg = nArg;
 nArg = 0;
 if (thisNArg > nPar)
   thisNArg = nPar;
 arg = routine_parameters(routineNum);
 for (i = 0; i < nPar; i++)
   if (defined(arg[i], 1))
     nArg++;
 // now execute the statements
 while (nStmnt--) {
   i = execute(*par++);
   if (i != 1)
     break;
 }
 // restore old value of !NARG
 nArg = oldNArg;

 if (i == LUX_ERROR)
   cerror(-1, symbol);

 for (n = 0; n < thisNArg; n++) // zap temp arguments
   if (symbol_context(evalArg[n]) == routineNum)
     zap(evalArg[n]);
 par = routine_parameters(routineNum);
 for (n = 0; n < nPar; n++) {   // make parameters point at #0 again
   if (symbol_class(*par) == LUX_TRANSFER
       && transfer_temp_param(*par))
     zap(transfer_target(*par)); // remove temp that has been
     // created to accomodate values for dangling or unspecified parameters
   par++;
 }
 // restore old values
 par = routine_parameters(routineNum);
 for (n = 0; n < nPar; n++)
   memcpy(&sym[par[n]], oldpars + n, sizeof(symTableEntry));
 free(oldpars);

 if (evalArg)
   free(evalArg);
 popExecutionLevel();
 curContext = oldContext;       // restore
 executeLevel--;
 fileLevel--;
 if ((traceMode & T_ROUTINE) == 0)
   noTrace--;
 if (i != LUX_ERROR && i != LOOP_RETALL)
   i = 1;
 if (msg) {
   printf("Leaving %s %s", routineTypeNames[(uint8_t) type],
          currentRoutineName);
   putchar('\n');
 }
 if (type || i != 1)
   return i;
 i = returnSym;
 returnSym = 0;
 if (!i)
   return luxerror("No value returned from user function %s", symbol,
                currentRoutineName);
 return i;

  usr_routine_3:
 zapTemp(listSym);
  usr_routine_2:
 free(evalArg);
  usr_routine_1:
 executeLevel--;
 fileLevel--;
 if ((traceMode & T_ROUTINE) == 0)
   noTrace--;
 popExecutionLevel();
 return LUX_ERROR;
}
//------------------------------------------------------------------
int32_t lux_for(int32_t nsym)
// executes LUX FOR-statement
{
 Pointer        counter;
 Scalar                 start, inc, end;
 int32_t        n, temp, action;
 Symboltype hiType, st;
 int16_t                startSym, endSym, stepSym, counterSym;
 char           forward;
 extern int32_t         trace, step;

                // find highest type of loop start, increment, and end
 startSym = eval(for_start(nsym));
 switch (symbol_class(startSym)) {
   case LUX_SCAL_PTR: case LUX_SCALAR:
     break;
   default:
     zapTemp(startSym);
     return luxerror("For-loop initializer must be scalar!", nsym);
 }
 endSym = eval(for_end(nsym));
 switch (symbol_class(endSym)) {
   case LUX_SCAL_PTR: case LUX_SCALAR:
     break;
   default:
     zapTemp(startSym);
     zapTemp(endSym);
     return luxerror("For-loop end marker must be scalar!", nsym);
 }
 stepSym = eval(for_step(nsym));
 switch (symbol_class(stepSym)) {
   case LUX_SCAL_PTR: case LUX_SCALAR:
     break;
   default:
     zapTemp(startSym);
     zapTemp(endSym);
     zapTemp(stepSym);
     return luxerror("For-loop step must be scalar!", nsym);
 }
 hiType = combinedType(symbol_type(startSym), symbol_type(endSym));
 if (for_step(nsym) != LUX_ONE
     && (st = symbol_type(stepSym)) > hiType)
   hiType = st;
 counterSym = for_loop_symbol(nsym); // loop counter
 undefine(counterSym);          // loop counter
 symbol_type(counterSym) = hiType;      // give loop counter proper type
 symbol_class(counterSym) = LUX_SCALAR;
 counter.b = &scalar_value(counterSym).b;
 convertScalar(&start, startSym, hiType); // and start, increment, end
 convertScalar(&end, endSym, hiType);
 convertScalar(&inc, stepSym, hiType);
 zapTemp(startSym);
 zapTemp(endSym);
 zapTemp(stepSym);
 switch (hiType) {
   case LUX_INT8:
     forward = 1;
     break;
   case LUX_INT16:
     forward = (inc.w >= 0)? 1: 0;
     break;
   case LUX_INT32:
     forward = (inc.l >= 0)? 1: 0;
     break;
   case LUX_INT64:
     forward = (inc.q >= 0)? 1: 0;
     break;
   case LUX_FLOAT:
     forward = (inc.f >= 0)? 1: 0;
     break;
   case LUX_DOUBLE:
     forward = (inc.d >= 0)? 1: 0;
     break;
 }
 temp = for_body(nsym);                 // body
 // initialize counter with start value
 switch (hiType) {
   case LUX_INT8:
     *counter.b = start.b;
     break;
   case LUX_INT16:
     *counter.w = start.w;
     break;
   case LUX_INT32:
     *counter.l = start.l;
     break;
   case LUX_INT64:
     *counter.q = start.q;
     break;
   case LUX_FLOAT:
     *counter.f = start.f;
     break;
   case LUX_DOUBLE:
     *counter.d = start.d;
     break;
 }
 // check if the end criterion is already met (n = 0) or not (n = 1)
 if (forward)
   switch (hiType) {
     case LUX_INT8:
     n = *counter.b > end.b? 0: 1;
     break;
   case LUX_INT16:
     n = *counter.w > end.w? 0: 1;
     break;
   case LUX_INT32:
     n = *counter.l > end.l? 0: 1;
     break;
   case LUX_INT64:
     n = *counter.q > end.q? 0: 1;
     break;
   case LUX_FLOAT:
     n = *counter.f > end.f? 0: 1;
     break;
   case LUX_DOUBLE:
     n = *counter.d > end.d? 0: 1;
     break;
   }
 else switch (hiType) {
   case LUX_INT8:
     n = *counter.b < end.b? 0: 1;
     break;
   case LUX_INT16:
     n = *counter.w < end.w? 0: 1;
     break;
   case LUX_INT32:
     n = *counter.l < end.l? 0: 1;
     break;
   case LUX_INT64:
     n = *counter.q < end.q? 0: 1;
     break;
   case LUX_FLOAT:
     n = *counter.f < end.f? 0: 1;
     break;
   case LUX_DOUBLE:
     n = *counter.d < end.d? 0: 1;
     break;
 }

 // if we're tracing or stepping then we must display extra info
 action = ((trace > executeLevel && !noTrace)
           || step > executeLevel);

 switch (hiType) {
   case LUX_INT8:
     if (action)
       while (n) {
         printf("FOR-loop: ");  // show for-loop status
         printf("%1d,%1d,%1d; counter %s = %1d\n", start.b, end.b, inc.b,
                symbolIdent(counterSym, 0), *counter.b);
         n = execute(temp);
         switch (n) {
           case LOOP_BREAK:
             n = 0;
             continue;
           case LUX_ERROR:
             printf("(counter %s = %1d)", symbolIdent(counterSym, 0),
                    *counter.b);
             return LUX_ERROR;
           case LOOP_RETALL: case LOOP_RETURN:
             return n;
         }
         *counter.b += inc.b;
         n = forward? (*counter.b > end.b? 0: 1): (*counter.b < end.b? 0: 1);
       }
     else
       while (n) {
         n = execute(temp);
         switch (n) {
           case LOOP_BREAK:
             n = 0;
             continue;
           case LUX_ERROR:
             printf("(counter %s = %1d)", symbolIdent(counterSym, 0),
                    *counter.b);
             return LUX_ERROR;
           case LOOP_RETALL: case LOOP_RETURN:
             return n;
         }
         *counter.b += inc.b;
         n = forward? (*counter.b > end.b? 0: 1): (*counter.b < end.b? 0: 1);
       }
     break;
   case LUX_INT16:
     if (action)
       while (n) {
         printf("FOR-loop: ");  // show for-loop status
         printf("%1d,%1d,%1d; counter %s = %1d\n", start.w, end.w, inc.w,
                symbolIdent(counterSym, 0), *counter.w);
         n = execute(temp);
         switch (n) {
           case LOOP_BREAK:
             n = 0;
             continue;
           case LUX_ERROR:
             printf("(counter %s = %1d)", symbolIdent(counterSym, 0),
                    *counter.w);
             return LUX_ERROR;
           case LOOP_RETALL: case LOOP_RETURN:
             return n;
         }
         *counter.w += inc.w;
         n = forward? (*counter.w > end.w? 0: 1): (*counter.w < end.w? 0: 1);
       }
     else
       while (n) {
         n = execute(temp);
         switch (n) {
           case LOOP_BREAK:
             n = 0;
             continue;
           case LUX_ERROR:
             printf("(counter %s = %1d)", symbolIdent(counterSym, 0),
                    *counter.w);
             return LUX_ERROR;
           case LOOP_RETALL: case LOOP_RETURN:
             return n;
         }
         *counter.w += inc.w;
         n = forward? (*counter.w > end.w? 0: 1): (*counter.w < end.w? 0: 1);
       }
     break;
   case LUX_INT32:
     if (action)
       while (n) {
         printf("FOR-loop: ");  // show for-loop status
         printf("%1d,%1d,%1d; counter %s = %1d\n", start.l, end.l, inc.l,
                symbolIdent(counterSym, 0), *counter.l);
         n = execute(temp);
         switch (n) {
           case LOOP_BREAK:
             n = 0;
             continue;
           case LUX_ERROR:
             printf("(counter %s = %1d)", symbolIdent(counterSym, 0),
                    *counter.l);
             return LUX_ERROR;
           case LOOP_RETALL: case LOOP_RETURN:
             return n;
         }
         *counter.l += inc.l;
         n = forward? (*counter.l > end.l? 0: 1): (*counter.l < end.l? 0: 1);
       }
     else
       while (n) {
         n = execute(temp);
         switch (n) {
           case LOOP_BREAK:
             n = 0;
             continue;
           case LUX_ERROR:
             printf("(counter %s = %1d)", symbolIdent(counterSym, 0),
                    *counter.l);
             return LUX_ERROR;
           case LOOP_RETALL: case LOOP_RETURN:
             return n;
         }
         *counter.l += inc.l;
         n = forward? (*counter.l > end.l? 0: 1): (*counter.l < end.l? 0: 1);
       }
     break;
   case LUX_INT64:
     if (action)
       while (n) {
         printf("FOR-loop: ");  // show for-loop status
         printf("%1d,%1d,%1d; counter %s = %1d\n", start.q, end.q, inc.q,
                symbolIdent(counterSym, 0), *counter.q);
         n = execute(temp);
         switch (n) {
           case LOOP_BREAK:
             n = 0;
             continue;
           case LUX_ERROR:
             printf("(counter %s = %1d)", symbolIdent(counterSym, 0),
                    *counter.l);
             return LUX_ERROR;
           case LOOP_RETALL: case LOOP_RETURN:
             return n;
         }
         *counter.q += inc.q;
         n = forward? (*counter.q > end.q? 0: 1): (*counter.q < end.q? 0: 1);
       }
     else
       while (n) {
         n = execute(temp);
         switch (n) {
           case LOOP_BREAK:
             n = 0;
             continue;
           case LUX_ERROR:
             printf("(counter %s = %1d)", symbolIdent(counterSym, 0),
                    *counter.l);
             return LUX_ERROR;
           case LOOP_RETALL: case LOOP_RETURN:
             return n;
         }
         *counter.q += inc.q;
         n = forward? (*counter.q > end.q? 0: 1): (*counter.q < end.q? 0: 1);
       }
     break;
   case LUX_FLOAT:
     if (action)
       while (n) {
         printf("FOR-loop: ");  // show for-loop status
         printf("%1g,%1g,%1g; counter %s = %1g\n", start.f, end.f, inc.f,
                symbolIdent(counterSym, 0), *counter.f);
         n = execute(temp);
         switch (n) {
           case LOOP_BREAK:
             n = 0;
             continue;
           case LUX_ERROR:
             printf("(counter %s = %1g)", symbolIdent(counterSym, 0),
                    *counter.f);
             return LUX_ERROR;
           case LOOP_RETALL: case LOOP_RETURN:
             return n;
         }
         *counter.f += inc.f;
         n = forward? (*counter.f > end.f? 0: 1): (*counter.f < end.f? 0: 1);
       }
     else
       while (n) {
         n = execute(temp);
         switch (n) {
           case LOOP_BREAK:
             n = 0;
             continue;
           case LUX_ERROR:
             printf("(counter %s = %1g)", symbolIdent(counterSym, 0),
                    *counter.f);
             return LUX_ERROR;
           case LOOP_RETALL: case LOOP_RETURN:
             return n;
         }
         *counter.f += inc.f;
         n = forward? (*counter.f > end.f? 0: 1): (*counter.f < end.f? 0: 1);
       }
     break;
   case LUX_DOUBLE:
     if (action)
       while (n) {
         printf("FOR-loop: ");  // show for-loop status
         printf("%1g,%1g,%1g; counter %s = %1g\n", start.d, end.d, inc.d,
                symbolIdent(counterSym, 0), *counter.d);
         n = execute(temp);
         switch (n) {
           case LOOP_BREAK:
             n = 0;
             continue;
           case LUX_ERROR:
             printf("(counter %s = %1g)", symbolIdent(counterSym, 0),
                    *counter.d);
             return LUX_ERROR;
           case LOOP_RETALL: case LOOP_RETURN:
             return n;
         }
         *counter.d += inc.d;
         n = forward? (*counter.d > end.d? 0: 1): (*counter.d < end.d? 0: 1);
       }
     else
       while (n) {
         n = execute(temp);
         switch (n) {
           case LOOP_BREAK:
             n = 0;
             continue;
           case LUX_ERROR:
             printf("(counter %s = %1g)", symbolIdent(counterSym, 0),
                    *counter.d);
             return LUX_ERROR;
           case LOOP_RETALL: case LOOP_RETURN:
             return n;
         }
         *counter.d += inc.d;
         n = forward? (*counter.d > end.d? 0: 1): (*counter.d < end.d? 0: 1);
       }
     break;
 }
 return 1;
}
//------------------------------------------------------------------
#define ACTION_BREAKPOINT       1
#define ACTION_TRACE    2
#define ACTION_STEP     4
float   CPUtime = 0.0;
int32_t execute(int32_t symbol)
     // executes LUX_EVB <symbol>.  Returns -1 on error, 1 on success,
     // various negative numbers on breaks, returns, etc.
{
  int16_t       *ptr;
  int32_t       temp, temp2, temp3, n, c = 0, oldStep, go = 0, oldEVB, oldBreakpoint,
    action;
  static int32_t        atBreakpoint = 0;
  extern int32_t        trace, step, traceMode, findBody, nWatchVars,
                tempVariableIndex, nBreakpoint;
  extern char   reportBody, ignoreSymbols, debugLine, evalScalPtr;
  extern char   *currentInputFile;
  extern breakpointInfo         breakpoint[];
  extern uint8_t        disableNewline;
  extern int16_t        watchVars[];
  uint8_t       oldNL;
  char const* name;
  char const* p;
  char const* oldRoutineName;
  FILE  *fp;
#if DEBUG
  void checkTemps(void);
#endif
  float         newCPUtime;
  int32_t       showstats(int32_t, int32_t []), getNewLine(char *, size_t, char const *, char),
    lux_restart(int32_t, int32_t []), showError(int32_t), insert(int32_t, int32_t []),
    nextFreeStackEntry(void);
  void showExecutionLevel(int32_t);

  curSymbol = symbol;           // put in global so we know where an
                                // exception (interrupt) occurred, if any
  if (symbol <= 0) {            // error or break condition
    if (symbol == LOOP_RETALL) {
      if (!curContext           // we're at the main execution level
          && !compileLevel      // and not compiling a file
          && !executeLevel) {   // and not inside a statement
        puts("RETALL - at top level already"); // at top level already
        return LUX_OK;
      }
    }
    return symbol;
  }
  if (symbol >= NSYM)           // beyond possible
    return cerror(ILL_SYM, 0, symbol, "execute");

#if DEBUG
  checkTemps();
#endif

  action = 0;

  if (suppressMsg)
    /* this replacement is performed as a side effect of something the
       user wants, but was not explicitly requested by the user, so we
       suppress the regular assignment messages that are usually
       shown when TRACEing. */
    suppressMsg--;
  else {
    oldBreakpoint = atBreakpoint;
    atBreakpoint = 0;
    if (nBreakpoint) {
      go = nBreakpoint;
      /* we cannot assume that all currently set breakpoints form a
         contiguous set, even if they were set that way, because the user
         may have deleted one or more.  We must check all breakpoint
         slots until we have found all the ones that are currently being
         used (whether active or inactive).  <go> keeps track of how
         many used breakpoints we still need to find. */
      for (n = 0; go; n++) {
        // check breakpoints at internal routines as well as in user-defined
        // routines
        if (breakpoint[n].status == BR_ENABLED // breakpoint enabled
            && (symbol_line(symbol) == breakpoint[n].line // correct line
                || !breakpoint[n].line)) { // or first line requested
          temp = routineContext(symbol); /* the number of the routine
                                            that the statement is in */
          if (temp && !strccmp(symbolProperName(temp), breakpoint[n].name)) {
            // correct file
            atBreakpoint = n + 1;

            if (!breakpoint[n].line) // no line set yet
              breakpoint[n].line = symbol_line(symbol); // set first line
          } else if (currentInputFile != NULL
                     && !strcmp(currentInputFile, breakpoint[n].name))
            // breakpoint in LUX code outside of routine or function
            atBreakpoint = n + 1;
        }
        if (breakpoint[n].status & BR_EXIST)
          go--;                         // one fewer still to account for
      }
      if (atBreakpoint && atBreakpoint != oldBreakpoint) {
        action |= ACTION_BREAKPOINT;
        traceMode |= 143;       // (/ENTER, /ROUTINE, /BRACES, /LOOP, /FILE)
      }
    } // end if (nBreakpoint)

    if (trace > executeLevel && !noTrace)
      action |= ACTION_TRACE;
    if (step > executeLevel)
      action |= ACTION_STEP;

    if (action) {
      if (!executeLevel)                // not doing anything
        noTrace = 0;            // stop tracing when at top level

      if (traceMode & T_SHOWEXEC)// /SHOWEXEC
        printf("%3d:", nExecuted);
      if (traceMode & 127)
        puts(symbolIdent(symbol, I_TRUNCATE | I_FILELEVEL));
      if (traceMode & T_CPUTIME) { // CPUtime
        newCPUtime = (float) clock()/CLOCKS_PER_SEC;
        printf("CPUtime: %10.2f\n", newCPUtime - CPUtime);
        CPUtime = newCPUtime;
      }
      if (traceMode & T_SHOWSTATS)
        showstats(0, NULL);
      if (action & ACTION_BREAKPOINT) {
        temp = atBreakpoint - 1;
        printf("At breakpoint %1d: file %s, line %1d\n", temp,
               breakpoint[temp].name, breakpoint[temp].line);
      }
      if (action & (ACTION_BREAKPOINT | ACTION_STEP)) {
        c = '?';
        while (c == '?') {
          printf(">%1d?", executeLevel);
          c = getSingleStdinChar();     // get user command
          printf("%c\r", c);    // echo user command
          fflush(stdout);       // ensure that it is displayed
          switch (c) {
            case 'n':           // next at this level
              step = executeLevel + 1;
              break;
            case 's':           // step to next at any level
              step = 99999;
              break;
            case 'e':           // step to next at lower level (i.e. to end
              // at this level)
              step = executeLevel;
              break;
            case 'g':           // go on until on/after a specific line#
              getNewLine((char *) scrat, NSCRAT, "> go to line?", 0);
              go = atoi((char *) scrat);
              break;
            case 'm':           // toggle showstats()
              traceMode ^= T_SHOWSTATS;
              c = '?';
              break;
            case 'w':           // show where we are
              showExecutionLevel(symbol);
              c = '?';
              break;
            case 'z':           // execute one inserted command
              oldStep = step;
              step = 0;
              debugLine = 1;
              executeLevel++;
              oldNL = disableNewline;
              disableNewline = -1;
              nextCompileLevel(stdin, "standard input");
              disableNewline = oldNL;
              debugLine = 0;
              step = oldStep;
              executeLevel--;
              c = '?';
              break;
            case '?':           // display options
              printw("step commands:");
              printw(" s: one step, n: next at this level, e: next at higher");
              printw(" level, g: go until specific line number, x: exit");
              printw(" stepping, z: enter and execute a command,");
              printw(" q: abort execution, m: toggle status display,");
              printw(" r: restart LUX, w: where?, ?: this text\n");
              break;
            case 'r':
              lux_restart(0, NULL);
              break;
            case 'x':           // stop stepping
              step = 0;
              go = 0;
              break;
            case 'q':           // stop this calculation
              puts("Aborting this calculation");
              step = 0;
              go = 0;
              return LUX_ERROR;
          } // end of switch (c)
        } // end of while (c == '?')
      }         // end of if (action & ...)
    } // end of if (action)
  } // end of if (suppressMsg) else

  oldEVB = currentEVB;          // so it can be restored later
  currentEVB = symbol;          // so we can make temps have this
                                // context (see nextFree...)
  ptr = evb_args(symbol);       // point at parameters
  returnSym = 0;
  pegMark();
  switch (evb_type(symbol)) {
    default:
      n = luxerror("Sorry, LUX_EVB type %d is not recognized\n",
                symbol, evb_type(symbol));
      break;
    case EVB_REPLACE:           // assignment
      nExecuted++;
      n = lux_replace(replace_lhs(symbol), replace_rhs(symbol));
      break;
    case EVB_INT_SUB:           // internal subroutine
      nExecuted++;
      evalScalPtr = 0;          // don't evaluate scalar pointer arguments,
                                // because we may want to assign stuff to
                                // them
      n = internal_routine(symbol, subroutine);
      evalScalPtr = 1;          // OK again to evaluate scalar pointers
      break;
    case EVB_USR_SUB:           // subroutine call
      temp = usr_sub_routine_num(symbol);
      if (symbol_class(temp) == LUX_STRING) {
        // routine name was not yet evaluated
        name = string_value(temp);
        if ((n = lookForName(name, subrHashTable, 0)) < 0) {
          // not compiled in the meantime;  try it now
          if ((fp = openPathFile(name, FIND_SUBR | FIND_LOWER))) {
            if (nextCompileLevel(fp, name) < 0) {
              n = -1;
              break;
            }
            fclose(fp);
            n = lookForName(name, subrHashTable, 0);
            if (n < 0) {
              n = luxerror("File %s found but subroutine still is not compiled",
                        0, name);
              break;
            }
          } else {
            n = luxerror("Subroutine %s not found", symbol, name);
            break;
          }
          zap(temp);            // don't need it anymore
        }

        /* call to a user-defined subroutine that has not yet been
         compiled */
        // availability of this type means that user-defined
        // subroutines only get compiled when they are actually called
        // for the first time.  This saves a lot of work and (symbol)
        // memory when large multi-branch programs are run.
        usr_sub_routine_num(symbol) = n;
      }
      // fall-thru to EVB_USR_CODE case
    case EVB_USR_CODE:
      temp = usr_code_routine_num(symbol);
      if (symbol_class(temp) == LUX_STRING) {
        // routine name was not yet evaluated
        name = string_value(temp);
        if ((n = lookForName(name, blockHashTable, curContext)) < 0) {
          // not compiled
          p = curContext? symbolProperName(curContext): "(main)";
          n = luxerror("Block routine %s not found in %s\n", symbol, name,
                    p? p: "(unnamed)");
          zap(temp);
          break;
        }
        usr_code_routine_num(symbol) = n;
        zap(temp);              // don't need string anymore
      }
      nExecuted++;
      evalScalPtr = 0;
      oldRoutineName = currentRoutineName;
      n = usr_routine(symbol);
      currentRoutineName = oldRoutineName;
      if (traceMode & T_ROUTINEIO) {
        if (currentRoutineName)
          printf("Back in routine %s\n", currentRoutineName);
        else
          puts("Back at top level");
      }
      evalScalPtr = 1;
      break;
    case EVB_CASE:
      temp = case_num_statements(symbol);
      ptr = case_statements(symbol);
      while (--temp) {
        n = int_arg(temp2 = eval(*ptr++));
        zapTemp(temp2);
        if (n) {
          n = execute(*ptr++);
          break;
        } else
          ptr++;
        temp--;
      }
      if (!temp) {
        if (*ptr)
          n = execute(*ptr++);  // default
        else
          n = 1;
      }
      break;
    case EVB_NCASE:
      n = ncase_num_statements(symbol);
      ptr = ncase_statements(symbol);
      temp = int_arg(temp2 = eval(ncase_switch_value(symbol)));
      zapTemp(temp2);
      if (temp < 0 || temp > n)
        temp = n;
      n = execute(ptr[temp]);
      if (!n)
        n = 1;
      break;
    case EVB_FILE:      // an include file
      name = file_name(symbol);
      p = strchr(name, ':');
      char const* q;
      q = p? p: strchr(name, '\0');
      {
        char *part = (char*) malloc(q - name + 1);
        memcpy(part, name, q - name);
        part[q - name] = '\0';
        fp = openPathFile(part, FIND_EITHER);
      }
      if (!fp)
        fp = openPathFile(name, FIND_EITHER);
      if (!fp) {
        n = luxerror("Cannot open file %s", symbol, name);
        perror("System message");
        break;
      }
      if (p) {  // seek specific routine
        if ((findBody = nextFreeStackEntry()) < 0) {    // error
          n = LUX_ERROR;
          break;
        }
        symbolStack[findBody] = p;
      }
      if (file_include_type(symbol) == FILE_REPORT)
        reportBody++;
      fileLevel++;
      if ((traceMode & T_FILE) == 0)
        noTrace++;
      n = nextCompileLevel(fp, string_value(symbol));
      fileLevel--;
      if (file_include_type(symbol) == FILE_REPORT)
        reportBody--;
      if (p) {
        unlinkString(findBody);
        findBody = 0;
      }
      if (!reportBody)
        ignoreSymbols = 0;
      if ((traceMode & T_FILE) == 0)
        noTrace--;
      fclose(fp);
      break;
    case EVB_BLOCK:
      temp = block_num_statements(symbol);
                                // number of statements
      ptr = block_statements(symbol);
      if ((traceMode & T_BLOCK) == 0)
        noTrace++;
      executeLevel++;
      n = 1;
      while (temp--) {
        n = execute(*ptr++);
        if (n < 0)
          break;
      }
      executeLevel--;
      if ((traceMode & T_BLOCK) == 0)
        noTrace--;
      break;
    case EVB_FOR:
      executeLevel++;
      if ((traceMode & T_LOOP) == 0)
        noTrace++;
      n = lux_for(symbol);
      executeLevel--;
      if ((traceMode & T_LOOP) == 0)
        noTrace--;
      break;
    case EVB_IF:
      temp = evals((int32_t) *ptr);                     // evaluate condition
      if (symbol_class(temp) != LUX_SCALAR) {
        n = cerror(COND_NO_SCAL, symbol, "in IF-statement");
        zapTemp(temp);
        break;
      }
      switch (scalar_type(temp)) {
      case LUX_INT8:
        n = scalar_value(temp).b? 1: 0;
        break;
      case LUX_INT16:
        n = scalar_value(temp).w? 1: 0;
        break;
      case LUX_INT32:
        n = scalar_value(temp).l? 1: 0;
        break;
      case LUX_INT64:
        n = scalar_value(temp).q? 1: 0;
        break;
      case LUX_FLOAT:
        n = scalar_value(temp).f? 1: 0;
        break;
      case LUX_DOUBLE:
        n = scalar_value(temp).d? 1: 0;
        break;
      }
      zapTemp(temp);
      if (n)
        n = execute(ptr[1]);
      else if (ptr[2])
        n = execute(ptr[2]);
      else
        n = 1;
      break;
    case EVB_REPEAT:
      temp = *ptr;                      // body
      temp2 = ptr[1];                   // condition
      executeLevel++;
      if ((traceMode & T_LOOP) == 0)
        noTrace++;
      do {
        n = execute(temp);                      // execute body
        if (n == LOOP_CONTINUE) {
          n = 0;
          continue;
        } else if (n <= 0)
          break;
        temp3 = eval((int32_t) temp2);          // condition
        n = int_arg(temp3);
        if (symbol_class(temp3) != LUX_SCALAR)
          n = LUX_ERROR;
        zapTemp(temp3);
        if (n < 0) break;
      }
      while (!n);
      executeLevel--;
      if ((traceMode & T_LOOP) == 0)
        noTrace--;
      if (!n || n == LOOP_BREAK)
        n = 1;
      break;
    case EVB_DO_WHILE:
      temp = *ptr;                      // body
      temp2 = ptr[1];                   // condition
      executeLevel++;
      if ((traceMode & T_LOOP) == 0)
        noTrace++;
      do {
        n = execute(temp);                      // execute body
        if (n == LOOP_CONTINUE)
          continue;
        else if (n <= 0)
          break;
        temp3 = eval((int32_t) temp2);          // condition
        n = int_arg(temp3);
        if (symbol_class(temp3) != LUX_SCALAR)
          n = LUX_ERROR;
        zapTemp(temp3);
        if (n < 0)
          break;
      }
      while (n);
      executeLevel--;
      if ((traceMode & T_LOOP) == 0)
        noTrace--;
      if (!n || n == LOOP_BREAK)
        n = 1;
      break;
    case EVB_WHILE_DO:
      temp2 = *ptr;                     // condition
      temp = ptr[1];                    // body
      executeLevel++;
      if ((traceMode & T_LOOP) == 0)
        noTrace++;
      while (1) {
        temp3 = eval((int32_t) temp2);          // condition
        if (symbol_class(temp3) != LUX_SCALAR)
          n = 0;
        else n = int_arg(temp3);
        zapTemp(temp3);
        if (!n)
          break;
        n = execute(temp);
        if (n == LOOP_CONTINUE) continue;
        if (n <= 0)
          break;
      }
      executeLevel--;
      if ((traceMode & T_LOOP) == 0)
        noTrace--;
      if (!n || n == LOOP_BREAK)
        n = 1;
      break;
    case EVB_RETURN:
      n = *ptr? eval(*ptr): 0;
      nExecuted++;
      switch (symbol_class(curContext)) {
        case LUX_SUBROUTINE:
          if (n)
            n = luxerror("No value may be RETURNed in subroutine %s", 0,
                      subrName(curContext));
          else
            n = LOOP_RETURN;
          break;
        case LUX_FUNCTION:
          returnSym = n;
          if (!n)
            n = luxerror("No value RETURNed in function %s", 0,
                      funcName(curContext));
          else {
            // if return symbol is not a temporary, then must return an
            // exact copy, because return symbols with names may be changed
            // in the evaluation of subsequent arguments to a function or
            // subroutine if the same user function is called in their
            // evaluation.  (may be wasting memory!)
            if (!isFreeTemp(returnSym)
                && symbol_context(returnSym) == curContext)
              returnSym = copySym(returnSym);
            n = LOOP_RETURN;
            if (trace > executeLevel && (traceMode & 127)) {
              name = symbolProperName(returnSym);
              printf("      ( RETURN,");
              if (name)
                printf("%s  = ", name);
              printf("%s )\n", symbolIdent(returnSym, I_VALUE | I_TRUNCATE));
            }
          }
          break;
        case LUX_BLOCKROUTINE:
          n = luxerror("No RETURN allowed in block %s", 0,
                    blockName(curContext));
          break;
        default:
          n = -7;
          break; }
      if (n == -7)
        n = luxerror("No RETURN possible from main execution level", 0);
      else if (n > 0)
        zapTemp(n);
  }
  if (n == LUX_ERROR)           // some error
    cerror(-1, symbol);
  if (symbol_context(symbol) == -compileLevel)
    zap(symbol);
  if (returnSym > 0)
    unMark(returnSym);
  zapMarked();
  // if the return symbol is a temporary that isn't used elsewhere, then
  // we move it back in the list of temporaries as far as it will go,
  // so the used part of that list is compact.
  if (returnSym && isFreeTemp(returnSym)) {
    temp3 = returnSym;
    while (temp3 > TEMPS_START && symbol_class(temp3 - 1) == LUX_UNUSED)
      temp3--;
    if (temp3 < returnSym) {
      sym[temp3] = sym[returnSym];
      fixContext(temp3, temp3);
      symbol_class(returnSym) = LUX_UNUSED;
      tempVariableIndex = temp3 + 1;
      returnSym = temp3;
    }
  }
  updateIndices();
  currentEVB = oldEVB;
  if ((action & (ACTION_TRACE | ACTION_STEP) && nWatchVars))
    for (n = 0; n < NWATCHVARS; n++) {
      if (watchVars[n]) {
        printf("w: %s =", symbolIdent(watchVars[n], I_PARENT));
        printf(" %s\n",
               symbolIdent(watchVars[n], I_VALUE | I_TRUNCATE | I_LENGTH));
      }
    }
  fflush(stdout);
  return n;
}
//------------------------------------------------------------------
int32_t lux_execute(int32_t narg, int32_t ps[])
// execute a string as if it were typed at the keyboard
// or execute the symbol indicated by number
// keyword /MAIN (1) has the execution take place at the main level
{
  int32_t       n, oldContext;
  int32_t       compileString(char *);

  if (internalMode & 1)
  { oldContext = curContext;
    curContext = 0; }
  switch (symbol_class(*ps))
  { case LUX_STRING:
      n = compileString(string_arg(*ps));
      break;
    case LUX_SCAL_PTR: case LUX_SCALAR:
      n = execute(int_arg(*ps));
      break;
    default:
      n = cerror(ILL_CLASS, ps[0]); }
  if (internalMode & 1)
    curContext = oldContext;
  return n;
}
//------------------------------------------------------------------
int32_t compileString(char *string)
// compiles string <string>
{
  char  *oldInputString;
  extern char   *inputString;
  extern int32_t        executeLevel;
  int32_t       n;

  oldInputString = inputString;
  executeLevel++;
  inputString = string;
  n = nextCompileLevel(NULL, NULL);
  executeLevel--;
  inputString = oldInputString;
  return n;
}
//------------------------------------------------------------------
#define UNDEFINED       0
#define INNER   1
#define OUTER   2

int32_t insert(int32_t narg, int32_t ps[])
// an insertion statement with subscripted target
/* ps[0]..ps[narg-3]: subscripts; ps[narg - 2]: source;
   ps[narg - 1]: target */
{
  int32_t       target, source, i, iq, start[MAX_DIMS], width, *dims,
        ndim, nelem, type, size[MAX_DIMS], subsc_type[MAX_DIMS],
        *index[MAX_DIMS], *iptr, j, n, class_id, srcNdim, *srcDims, srcNelem,
        srcType, stride[MAX_DIMS], tally[MAX_DIMS], offset0, nmult,
        tstep[MAX_DIMS], offset, onestep, unit, combineType;
  Pointer       src, trgt;
  wideScalar    value;
  char  *name;
  FILE  *fp;
  extern int32_t        trace, step;
  int32_t       lux_assoc_output(int32_t, int32_t, int32_t, int32_t);

  target = ps[--narg];
  source = ps[--narg];
  if (narg > MAX_DIMS)
    return cerror(N_ARG_OVR, 0);
  // check if legal to change target
  if (target <= nFixed)
    return cerror(CST_LHS, target);
  // check if target is transfer symbol, and resolve if so
  target = transfer(target);
  class_id = symbol_class(target);

  offset0 = 0;                  // so it can be updated for LUX_FILEMAPs

  switch (internalMode & 3) {
    case 0:
      combineType = UNDEFINED;
      break;
    case 1:
      combineType = INNER;
      break;
    case 2:
      combineType = OUTER;
      break;
    case 3:
      return luxerror("Specified incompatible /INNER and /OUTER", 0);
  }

  switch (class_id) {
    default:
      return cerror(SUBSC_NO_INDX, target);
    case LUX_STRING:
      ndim = 1;
      nelem = string_size(target);
      dims = &nelem;
      trgt.s = string_value(target);
      type = string_type(target);
      break;
    case LUX_ASSOC:
      ndim = assoc_num_dims(target);
      dims = assoc_dims(target);
      nelem = 1;
      for (i = 0; i < ndim; i++)
        nelem *= dims[i];
      break;
    case LUX_FILEMAP:
      if (file_map_readonly(target))
        return luxerror("File array is read-only", target);
      ndim = file_map_num_dims(target);
      dims = file_map_dims(target);
      nelem = file_map_size(target);
      type = file_map_type(target);
      name = file_map_file_name(target);
      // We want to open the file for updating at any position in that file.
      // If it does not yet exist, then it should be created.  DEC Ultrix
      // does all this with the a+ file access mode, but SGI Irix does not
      // allow overwriting the existing contents of a file with a+ mode,
      // and complains if a file does not yet exist when opened with r+
      // mode.  Solution: catch r+ complaint and do a w+ if necessary.
      if ((fp = Fopen(name, "r+")) == NULL
          && (fp = Fopen(name, "w+")) == NULL) {
        printf("File %s: ", name);
        return cerror(ERR_OPEN, target);
      }
      offset0 = file_map_has_offset(target)? file_map_offset(target): 0;
      break;
    case LUX_ARRAY: case LUX_CARRAY:
      ndim = array_num_dims(target);
      dims = array_dims(target);
      nelem = array_size(target);
      type = array_type(target);
      trgt.l = (int32_t *) array_data(target);
      break;
  }

  unit = lux_type_size[type];

  if (narg > ndim)
    return cerror(N_DIMS_OVR, target);

  switch (symbol_class(source)) {
    default:
      return cerror(ILL_CLASS, source);
    case LUX_SCAL_PTR:
      source = dereferenceScalPointer(source);
      // fall-thru
    case LUX_SCALAR:
      srcNdim = 1;
      srcNelem = 1;
      srcDims = &srcNelem;
      srcType = scalar_type(source);
      src.l = &scalar_value(source).l;
      break;
    case LUX_CSCALAR:
      srcNdim = 1;
      srcNelem = 1;
      srcDims = &srcNelem;
      srcType = complex_scalar_type(source);
      src.cf = complex_scalar_data(source).cf;
      break;
    case LUX_STRING:
      srcNdim = 1;
      if (class_id == LUX_ARRAY)        // insert string in string array
        srcNelem = 1;
      else                      // insert string in string
        srcNelem = string_size(source);
      srcDims = &srcNelem;
      srcType = LUX_TEMP_STRING;
      src.s = string_value(source);
      break;
    case LUX_ARRAY: case LUX_CARRAY:
      srcNdim = array_num_dims(source);
      srcDims = array_dims(source);
      srcNelem = array_size(source);
      srcType = array_type(source);
      src.l = (int32_t *) array_data(source);
      break;
  }

  // get info about all subscripts
  // start[i] = start index (for LUX_RANGE subscripts)
  // size[i] = number of indices
  // index[i] = pointer to list of indices (for LUX_ARRAY subscripts)
  // subsc_type[i] = type of subscript: LUX_RANGE (scalar or range) or
  //           LUX_ARRAY (array of indices)

  if (internalMode & 64) {      // /SEPARATE
    // treat the elements of the single subscript (a numerical array)
    // as if that many separate scalars were specified.  LS 23jul98
    if (narg != 1               // but not exactly one subscript
        || !symbolIsNumericalArray(ps[0]) // or it's not a numerical array
        || array_size(ps[0]) > ndim) // or it has too many elements
      return luxerror("Wrong subscript for /SEPARATE", ps[0]);
    iq = lux_long(1, ps);       // ensure LONG
    narg = array_size(iq);
    iptr = (int32_t*) array_data(iq);
    for (i = 0; i < narg; i++) {
      start[i] = *iptr++;
      if (start[i] < 0 || start[i] >= dims[i]) // illegal subscript value
        return cerror(ILL_SUBSC, ps[0], start[i], dims[i]);
      size[i] = 1;
      subsc_type[i] = LUX_RANGE;
    }
    nmult = 0;                  // no multiple-element subscripts
  } else {                      // no /SEPARATE
    nmult = 0;
    for (i = 0; i < narg; i++) { // all subscripts
      iq = ps[i];               // next subscript
      switch (symbol_class(iq)) { // subscript class
        case LUX_SCALAR:
          start[i] = int_arg(iq); // start index (scalar value)
          if (narg == 1                 // only a single subscript
              && (internalMode & 48) == 0) // no /ALL or /ZERO keywords
                                // -> treat the array as if it were 1D
            width = nelem;
          else
            width = dims[i];
          if (start[i] < 0 || start[i] >= width) // check if within range
            return cerror(ILL_SUBSC, iq, start[i], width);
          size[i] = 1;          // size (1 element)
          subsc_type[i] = LUX_RANGE; // subscript type
          break;
        case LUX_ARRAY:
          iq = lux_long(1, &iq); // get LONG indices
          n = size[i] = array_size(iq); // number of array elements
          if (n == 1) {                 // only one element: mimic scalar
            start[i] = *(int32_t *) array_data(iq);
            if (narg == 1)      // single subscript only
              width = nelem;
            else
              width = dims[i];
            if (start[i] < 0 || start[i] >= width)
              return cerror(ILL_SUBSC, iq, start[i], width);
            subsc_type[i] = LUX_RANGE;
          } else {
            iptr = index[i] = (int32_t *) array_data(iq);
            if (narg == 1)      // only a single subscript; addresses
                                // whole source array as if it were 1D
              width = nelem;
            else
              width = dims[i];
            while (n--) {       // check if all subscripts are within range
              if (*iptr < 0 || *iptr >= width)
                return cerror(ILL_SUBSC, iq, *iptr, width);
              iptr++;
            }
            nmult++;
            if (combineType == UNDEFINED && array_num_dims(iq) > 1)
              combineType = INNER;
            subsc_type[i] = LUX_ARRAY;
          }
          break;
        case LUX_RANGE:
          if (!range_scalar(iq))
            return cerror(ILL_CLASS, iq);
          // get here if scalar range
          iq = convertRange(iq);// convert to subscript pointer
          if (iq < 0)           // some error occurred
            return iq;
          // fall-thru
        case LUX_SUBSC_PTR:
          start[i] = subsc_ptr_start(iq);
          if (start[i] < 0)     // count from end of range
            start[i] += dims[i];
          if (start[i] < 0 || start[i] >= dims[i])
            return cerror(RANGE_START, iq, start[i], dims[i]);
          size[i] = subsc_ptr_end(iq); // endpoint
          if (size[i] < 0)      // count from end of range
            size[i] += dims[i];
          if (size[i] < start[i] || size[i] >= dims[i])
            return cerror(RANGE_END, iq, size[i], dims[i]);
          size[i] += 1 - start[i]; // now we have the size
          if (subsc_ptr_redirect(iq) >= 0)
            return luxerror("No redirection allowed on insert target", target);
          if (subsc_ptr_sum(iq))
            return luxerror("No summation allowed on insert target", target);
          subsc_type[i] = LUX_RANGE;
          if (size[i] > 1)
            nmult++;
          break;
        default:
          return cerror(ILL_SUBSC_TYPE, iq);
      }
    }
  }

  switch (class_id) {
    case LUX_ASSOC:
      // only a list of scalars is allowed here
      if (nmult)
        return cerror(ILL_SUBSC_LHS, target);
      if (narg > 1) {           // more than one subscript
        i = array_scratch(LUX_INT32, 1, &narg);
        memcpy(array_data(i), start, narg);
        offset = -1;
      } else {
        i = start[0];
        offset = -3; }
      n = lux_assoc_output(target, source, i, offset);
      return target;
  }

  /* get here if source is a (numerical, string, or file) array or a
     string */

  // NOTE that (file) array treatment is not streamlined (yet) for
  // insertion of contiguous blocks, (except for the simplest cases)
  // so it may be unnecessarily slow.  LS 18jun97

  // if either source or target contains strings, then both must
  if (isStringType(type) ^ isStringType(srcType))
    return cerror(INCMP_ARG, source);

  if (class_id == LUX_ARRAY && isStringType(type)) // string array
    type = LUX_STRING_ARRAY;    // distinguish string array from string
  if (symbol_class(source) == LUX_ARRAY && isStringType(srcType))
    srcType = LUX_STRING_ARRAY;

  onestep = lux_type_size[srcType];

  switch (internalMode & 48) {
    case 0:                     // none
      if (narg > 1 && narg != ndim)
        return cerror(ILL_N_SUBSC, target);
      break;
    case 16:                    // /ZERO
      for (i = narg; i < ndim; i++) {
        start[i] = 0;
        size[i] = 1;
        subsc_type[i] = LUX_RANGE;
      }
      narg = ndim;
      break;
    case 32:                    // /ALL
      for (i = narg; i < ndim; i++) {
        start[i] = 0;
        size[i] = dims[i];
        if (size[i] > 1)
          nmult++;
        subsc_type[i] = LUX_RANGE;
      }
      narg = ndim;
      break;
    default:
      return luxerror("Specified incompatible /ZERO and /ALL", 0);
  }

  if (trace > executeLevel || step > executeLevel) { // output some info
    printf("      ( %s(", symbolIdent(target, I_TRUNCATE));
    for (j = 0; j < narg; j++) {
      if (j) putchar(',');
      printf("%s", symbolIdent(ps[j], I_VALUE | I_TRUNCATE));
    }
    printf(") = %s )\n", symbolIdent(source, I_VALUE | I_TRUNCATE));
  }

  if (combineType == UNDEFINED)
    combineType = OUTER;

  if (combineType == INNER)
                                // /INNER - inner-style subscript
                                // combinations
  // each element in the first dimension is combined with only
  // the corresponding elements in the other dimensions
  {
    // the number of elements in all dimensions must be the same
    for (i = 1; i < narg; i++)
      if (size[i] != size[0])
        return cerror(INCMP_DIMS, target);

    // the number of elements in the dimensions must equal the
    // number of elements in the source -- unless the source has
    // only one element.
    if (srcNelem == 1)          // only one element: insert everywhere
      onestep = 0;
    else if (size[0] != srcNelem)
      return cerror(INCMP_ARG, source);

    n = (class_id == LUX_FILEMAP)? unit: 1;
    for (i = 0; i < narg; i++) {
      stride[i] = n;
      n *= dims[i];
    }

    switch (class_id) {
      case LUX_ARRAY: case LUX_CARRAY:
        for (j = 0; j < size[0]; j++) {
          offset = 0;
          for (i = 0; i < narg; i++)
            offset += ((subsc_type[i] == LUX_ARRAY)? index[i][j]: start[i]++)
              * stride[i];
          switch (type) {
            case LUX_INT8: case LUX_TEMP_STRING:
              switch (srcType) {
                case LUX_INT8: case LUX_TEMP_STRING:
                  trgt.b[offset] = *src.b;
                  break;
                case LUX_INT16:
                  trgt.b[offset] = (uint8_t) *src.w;
                  break;
                case LUX_INT32:
                  trgt.b[offset] = (uint8_t) *src.l;
                  break;
                case LUX_INT64:
                  trgt.b[offset] = *src.q;
                  break;
                case LUX_FLOAT:
                  trgt.b[offset] = (uint8_t) *src.f;
                  break;
                case LUX_DOUBLE:
                  trgt.b[offset] = (uint8_t) *src.d;
                  break;
                case LUX_CFLOAT:
                  trgt.b[offset] = hypot(src.cf->real, src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  trgt.b[offset] = hypot(src.cd->real, src.cd->imaginary);
                  break;
              }
              break;
            case LUX_INT16:
              switch (srcType) {
                case LUX_INT8:
                  trgt.w[offset] = (int16_t) *src.b;
                  break;
                case LUX_INT16:
                  trgt.w[offset] = *src.w;
                  break;
                case LUX_INT32:
                  trgt.w[offset] = (int16_t) *src.l;
                  break;
                case LUX_INT64:
                  trgt.w[offset] = *src.q;
                  break;
                case LUX_FLOAT:
                  trgt.w[offset] = (int16_t) *src.f;
                  break;
                case LUX_DOUBLE:
                  trgt.w[offset] = (int16_t) *src.d;
                  break;
                case LUX_CFLOAT:
                  trgt.w[offset] = hypot(src.cf->real, src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  trgt.w[offset] = hypot(src.cd->real, src.cd->imaginary);
                  break;
              }
              break;
            case LUX_INT32:
              switch (srcType) {
                case LUX_INT8:
                  trgt.l[offset] = (int32_t) *src.b;
                  break;
                case LUX_INT16:
                  trgt.l[offset] = (int32_t) *src.w;
                  break;
                case LUX_INT32:
                  trgt.l[offset] = *src.l;
                  break;
                case LUX_INT64:
                  trgt.l[offset] = *src.q;
                  break;
                case LUX_FLOAT:
                  trgt.l[offset] = (int32_t) *src.f;
                  break;
                case LUX_DOUBLE:
                  trgt.l[offset] = (int32_t) *src.d;
                  break;
                case LUX_CFLOAT:
                  trgt.l[offset] = hypot(src.cf->real, src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  trgt.l[offset] = hypot(src.cd->real, src.cd->imaginary);
                  break;
              }
              break;
            case LUX_INT64:
              switch (srcType) {
                case LUX_INT8:
                  trgt.q[offset] = *src.b;
                  break;
                case LUX_INT16:
                  trgt.q[offset] = *src.w;
                  break;
                case LUX_INT32:
                  trgt.q[offset] = *src.l;
                  break;
                case LUX_INT64:
                  trgt.q[offset] = *src.q;
                  break;
                case LUX_FLOAT:
                  trgt.q[offset] = *src.f;
                  break;
                case LUX_DOUBLE:
                  trgt.q[offset] = *src.d;
                  break;
                case LUX_CFLOAT:
                  trgt.q[offset] = hypot(src.cf->real, src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  trgt.q[offset] = hypot(src.cd->real, src.cd->imaginary);
                  break;
              }
              break;
            case LUX_FLOAT:
              switch (srcType) {
                case LUX_INT8:
                  trgt.f[offset] = (float) *src.b;
                  break;
                case LUX_INT16:
                  trgt.f[offset] = (float) *src.w;
                  break;
                case LUX_INT32:
                  trgt.f[offset] = (float) *src.l;
                  break;
                case LUX_INT64:
                  trgt.f[offset] = *src.q;
                  break;
                case LUX_FLOAT:
                  trgt.f[offset] = *src.f;
                  break;
                case LUX_DOUBLE:
                  trgt.f[offset] = (float) *src.d;
                  break;
                case LUX_CFLOAT:
                  trgt.f[offset] = hypot(src.cf->real, src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  trgt.f[offset] = hypot(src.cd->real, src.cd->imaginary);
                  break;
              }
              break;
            case LUX_DOUBLE:
              switch (srcType) {
                case LUX_INT8:
                  trgt.d[offset] = (double) *src.b;
                  break;
                case LUX_INT16:
                  trgt.d[offset] = (double) *src.w;
                  break;
                case LUX_INT32:
                  trgt.d[offset] = (double) *src.l;
                  break;
                case LUX_INT64:
                  trgt.d[offset] = *src.q;
                  break;
                case LUX_FLOAT:
                  trgt.d[offset] = (double) *src.f;
                  break;
                case LUX_DOUBLE:
                  trgt.d[offset] = *src.d;
                  break;
                case LUX_CFLOAT:
                  trgt.d[offset] = hypot(src.cf->real, src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  trgt.d[offset] = hypot(src.cd->real, src.cd->imaginary);
                  break;
              }
              break;
            case LUX_CFLOAT:
              switch (srcType) {
                case LUX_INT8:
                  trgt.cf[offset].real = (float) *src.b;
                  trgt.cf[offset].imaginary = 0;
                  break;
                case LUX_INT16:
                  trgt.cf[offset].real = (float) *src.w;
                  trgt.cf[offset].imaginary = 0;
                  break;
                case LUX_INT32:
                  trgt.cf[offset].real = (float) *src.l;
                  trgt.cf[offset].imaginary = 0;
                  break;
                case LUX_INT64:
                  trgt.cf[offset].real = *src.q;
                  trgt.cf[offset].imaginary = 0;
                  break;
                case LUX_FLOAT:
                  trgt.cf[offset].real = *src.f;
                  trgt.cf[offset].imaginary = 0;
                  break;
                case LUX_DOUBLE:
                  trgt.cf[offset].real = (float) *src.d;
                  trgt.cf[offset].imaginary = 0;
                  break;
                case LUX_CFLOAT:
                  trgt.cf[offset].real = src.cf->real;
                  trgt.cf[offset].imaginary = src.cf->imaginary;
                  break;
                case LUX_CDOUBLE:
                  trgt.cf[offset].real = (float) src.cd->real;
                  trgt.cf[offset].imaginary = (float) src.cd->imaginary;
                  break;
              }
              break;
            case LUX_CDOUBLE:
              switch (srcType) {
                case LUX_INT8:
                  trgt.cd[offset].real = (double) *src.b;
                  trgt.cd[offset].imaginary = 0;
                  break;
                case LUX_INT16:
                  trgt.cd[offset].real = (double) *src.w;
                  trgt.cd[offset].imaginary = 0;
                  break;
                case LUX_INT32:
                  trgt.cd[offset].real = (double) *src.l;
                  trgt.cd[offset].imaginary = 0;
                  break;
                case LUX_INT64:
                  trgt.cd[offset].real = *src.q;
                  trgt.cd[offset].imaginary = 0;
                  break;
                case LUX_FLOAT:
                  trgt.cd[offset].real = (double) *src.f;
                  trgt.cd[offset].imaginary = 0;
                  break;
                case LUX_DOUBLE:
                  trgt.cd[offset].real = (double) *src.d;
                  trgt.cd[offset].imaginary = 0;
                  break;
                case LUX_CFLOAT:
                  trgt.cd[offset].real = (double) src.cf->real;
                  trgt.cd[offset].imaginary = (double) src.cf->imaginary;
                  break;
                case LUX_CDOUBLE:
                  trgt.cd[offset].real = (double) src.cd->real;
                  trgt.cd[offset].imaginary = (double) src.cd->imaginary;
                  break;
              }
              break;
            case 999:           // string array
              if (trgt.sp[offset])      // already a string there
                free(trgt.sp[offset]);
              switch (srcType) {
                case 999:               // string array
                  trgt.sp[offset] = strsave(*src.sp);
                  break;
                case LUX_TEMP_STRING: // string
                  trgt.sp[offset] = strsave(src.s);
                  break;
              }
              break;
          }
          src.b += onestep;
        }
        break;
      case LUX_FILEMAP:
        for (j = 0; j < size[0]; j++) {
          offset = offset0;
          for (i = 0; i < narg; i++)
            offset += ((subsc_type[i] == LUX_ARRAY)? index[i][j]: start[i]++)
              * stride[i];
          if (fseek(fp, offset, SEEK_SET)) {
            fclose(fp);
            return cerror(POS_ERR, target);
          }
          switch (type) {
            case LUX_INT8: case LUX_TEMP_STRING:
              switch (srcType) {
                case LUX_INT8: case LUX_TEMP_STRING:
                  value.b = *src.b;
                  break;
                case LUX_INT16:
                  value.b = (uint8_t) *src.w;
                  break;
                case LUX_INT32:
                  value.b = (uint8_t) *src.l;
                  break;
                case LUX_FLOAT:
                  value.b = (uint8_t) *src.f;
                  break;
                case LUX_DOUBLE:
                  value.b = (uint8_t) *src.d;
                  break;
                case LUX_CFLOAT:
                  value.b = sqrt(src.cf->real*src.cf->real
                                 + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  value.b = sqrt(src.cd->real*src.cd->real
                                 + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_INT16:
              switch (srcType) {
                case LUX_INT8:
                  value.w = (int16_t) *src.b;
                  break;
                case LUX_INT16:
                  value.w = *src.w;
                  break;
                case LUX_INT32:
                  value.w = (int16_t) *src.l;
                  break;
                case LUX_FLOAT:
                  value.w = (int16_t) *src.f;
                  break;
                case LUX_DOUBLE:
                  value.w = (int16_t) *src.d;
                  break;
                case LUX_CFLOAT:
                  value.w = sqrt(src.cf->real*src.cf->real
                                 + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  value.w = sqrt(src.cd->real*src.cd->real
                                 + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_INT32:
              switch (srcType) {
                case LUX_INT8:
                  value.l = (int32_t) *src.b;
                  break;
                case LUX_INT16:
                  value.l = (int32_t) *src.w;
                  break;
                case LUX_INT32:
                  value.l = *src.l;
                  break;
                case LUX_FLOAT:
                  value.l = (int32_t) *src.f;
                  break;
                case LUX_DOUBLE:
                  value.l = (int32_t) *src.d;
                  break;
                case LUX_CFLOAT:
                  value.l = sqrt(src.cf->real*src.cf->real
                                 + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  value.l = sqrt(src.cd->real*src.cd->real
                                 + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_FLOAT:
              switch (srcType) {
                case LUX_INT8:
                  value.f = (float) *src.b;
                  break;
                case LUX_INT16:
                  value.f = (float) *src.w;
                  break;
                case LUX_INT32:
                  value.f = (float) *src.l;
                  break;
                case LUX_FLOAT:
                  value.f = *src.f;
                  break;
                case LUX_DOUBLE:
                  value.f = (float) *src.d;
                  break;
                case LUX_CFLOAT:
                  value.f = sqrt(src.cf->real*src.cf->real
                                 + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  value.f = sqrt(src.cd->real*src.cd->real
                                 + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_DOUBLE:
              switch (srcType) {
                case LUX_INT8:
                  value.d = (double) *src.b;
                  break;
                case LUX_INT16:
                  value.d = (double) *src.w;
                  break;
                case LUX_INT32:
                  value.d = (double) *src.l;
                  break;
                case LUX_FLOAT:
                  value.d = (double) *src.f;
                  break;
                case LUX_DOUBLE:
                  value.d = *src.d;
                  break;
                case LUX_CFLOAT:
                  value.f = sqrt(src.cf->real*src.cf->real
                                 + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  value.d = sqrt(src.cd->real*src.cd->real
                                 + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_CFLOAT:
              switch (srcType) {
                case LUX_INT8:
                  value.cf.real = *src.b;
                  value.cf.imaginary = 0;
                  break;
                case LUX_INT16:
                  value.cf.real = *src.w;
                  value.cf.imaginary = 0;
                  break;
                case LUX_INT32:
                  value.cf.real = *src.l;
                  value.cf.imaginary = 0;
                  break;
                case LUX_FLOAT:
                  value.cf.real = *src.f;
                  value.cf.imaginary = 0;
                  break;
                case LUX_DOUBLE:
                  value.cf.real = *src.d;
                  value.cf.imaginary = 0;
                  break;
                case LUX_CFLOAT:
                  value.cf.real = src.cf->real;
                  value.cf.imaginary = src.cf->imaginary;
                  break;
                case LUX_CDOUBLE:
                  value.cf.real = src.cd->real;
                  value.cf.imaginary = src.cd->imaginary;
                  break;
              }
              break;
          }
          fwrite(&value.b, unit, 1, fp);
          if (ferror(fp)) {
            fclose(fp);
            return cerror(WRITE_ERR, target);
          }
          src.b += onestep;
        }
        fclose(fp);
        break;
    }
  } else {                      // /OUTER - outer-style subscript
                                // combinations
  // each value of each coordinate is
  // combined with each value of each other coordinate

    // the number of elements implied by the subscripts must match that in
    // the source for each dimension
    // the number of dimensions in the source must be matched either by
    // the number of subscripts with size greater than 1, or by the total
    // number of subscripts

    if (srcNelem != 1 && srcNdim != nmult && srcNdim != narg && nmult)
      return cerror(ILL_SUBSC, target);

    j = 0;                      /* counts the number of subscripts with
                                   size greater than 1 */
    if (srcNelem > 1) {
      if (srcNdim == nmult) {   // disregard scalar subscripts
        for (i = 0; i < narg; i++) { // all subscripts
          if (size[i] > 1) {
            if (j >= srcNdim)
              return luxerror("Too many non-scalar subscripts", target);
            if (size[i] != srcDims[j])
              return luxerror("Subscript size incompatible with source dimension",
                           target);
            j++;
          } // end of if (size[i]...)
        }       // end of for (i = 0; ...)
      } else {                  // scalar subscripts matter
        for (i = 0; i < narg; i++) {
          if (size[i] > 1 && size[i] != srcDims[i])
            return luxerror("Subscript size incompatible with source dimension",
                         target);
          if (size[i] > 1)
            j++;                // all OK
        }
      }
    } else                      // one source element: insert everywhere
      for (i = 0; i < narg; i++)
        if (size[i] > 1)
          j++;

    if (!j)                     // all subscripts are scalars:
      // they indicate the start address.  If there is only one subscript,
      // or if the number of subscripts is greater than the number of
      // dimensions in the source, or if the /ONEDIM keyword is specified,
      // then 1D insertion is performed.

      if (narg > 1 && srcNdim == narg && (internalMode & 4) == 0) {
        // multi-dimensional insertion
        for (i = 0; i < narg; i++)
          size[i] = srcDims[i];
        j = srcNdim;            // to indicate multi-dimensional insertion
      }
    if (!j) {                   // 1D insert
      if (srcNelem == 1)        // enter scalar in all elements
        srcDims = &nelem;
      else
        srcDims = &srcNelem;
      srcNdim = 1;
      offset = 0;
      n = 1;
      for (i = 0; i < narg; i++) { // calculate data offset
        offset += n*start[i];
        n *= dims[i];
      }
      if (offset + srcNelem > nelem)
        return luxerror("Source extends beyond target boundaries", source);
      start[0] = offset;
      dims = &nelem;
      ndim = 1;
      narg = 1;
      size[0] = srcNelem;
    } else if (j != srcNdim && srcNelem > 1)
      return luxerror("Too few non-scalar subscripts", target);

    if (srcNelem == 1)
      onestep = 0;

    // now do the insertion
    // for LUX_ARRAY subscripts the offset is calculated for each index
    // for LUX_RANGE subscripts the offset is updated for each element

    n = (class_id == LUX_FILEMAP)? unit: 1;

    if (!j && ndim == 1 && srcNdim == 1 && type == srcType &&
        (srcNelem > 1 || size[0] == 1)) {
                                // in this case we do a fast insert
      offset0 += start[0]*unit;
      switch (class_id) {
        case LUX_ARRAY: case LUX_CARRAY:
          memcpy(trgt.b + offset0, src.b, srcNelem*unit);
          break;
        case LUX_FILEMAP:
          if (fseek(fp, offset0, SEEK_SET)) {
            fclose(fp);
            if (ferror(fp))
              perror("System message");
            return cerror(POS_ERR, target);
          }
          fwrite(src.b, unit, srcNelem, fp);
          if (ferror(fp)) {
            fclose(fp);
            return cerror(WRITE_ERR, target);
          }
          fclose(fp);
          break;
      }
      return LUX_OK;
    }

    for (i = 0; i < narg; i++) {
      stride[i] = n;
      tally[i] = 0;
      if (subsc_type[i] == LUX_RANGE)
        offset0 += start[i]*n;
      n *= dims[i];
    }

    if (subsc_type[0] == LUX_RANGE) // ??
      tstep[0] = stride[0];
    else
      tstep[0] = 0;

    for (i = 1; i < narg; i++)
      tstep[i] = (subsc_type[i] == LUX_RANGE? stride[i]: 0)
        - (subsc_type[i - 1] == LUX_RANGE? size[i - 1]*stride[i - 1]: 0);

    switch (class_id) {
      case LUX_ARRAY: case LUX_CARRAY:
        do {
          offset = offset0;
          for (i = 0; i < narg; i++)
            if (subsc_type[i] == LUX_ARRAY)
              offset += index[i][tally[i]]*stride[i];
          switch (type) {
            case LUX_INT8: case LUX_TEMP_STRING:
              switch (srcType) {
                case LUX_INT8: case LUX_TEMP_STRING:
                  trgt.b[offset] = *src.b;
                  break;
                case LUX_INT16:
                  trgt.b[offset] = (uint8_t) *src.w;
                  break;
                case LUX_INT32:
                  trgt.b[offset] = (uint8_t) *src.l;
                  break;
                case LUX_FLOAT:
                  trgt.b[offset] = (uint8_t) *src.f;
                  break;
                case LUX_DOUBLE:
                  trgt.b[offset] = (uint8_t) *src.d;
                  break;
                case LUX_CFLOAT:
                  trgt.b[offset] = sqrt(src.cf->real*src.cf->real
                                        + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  trgt.b[offset] = sqrt(src.cd->real*src.cd->real
                                        + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_INT16:
              switch (srcType) {
                case LUX_INT8:
                  trgt.w[offset] = (int16_t) *src.b;
                  break;
                case LUX_INT16:
                  trgt.w[offset] = *src.w;
                  break;
                case LUX_INT32:
                  trgt.w[offset] = (int16_t) *src.l;
                  break;
                case LUX_FLOAT:
                  trgt.w[offset] = (int16_t) *src.f;
                  break;
                case LUX_DOUBLE:
                  trgt.w[offset] = (int16_t) *src.d;
                  break;
                case LUX_CFLOAT:
                  trgt.w[offset] = sqrt(src.cf->real*src.cf->real
                                        + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  trgt.w[offset] = sqrt(src.cd->real*src.cd->real
                                        + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_INT32:
              switch (srcType) {
                case LUX_INT8:
                  trgt.l[offset] = (int32_t) *src.b;
                  break;
                case LUX_INT16:
                  trgt.l[offset] = (int32_t) *src.w;
                  break;
                case LUX_INT32:
                  trgt.l[offset] = *src.l;
                  break;
                case LUX_FLOAT:
                  trgt.l[offset] = (int32_t) *src.f;
                  break;
                case LUX_DOUBLE:
                  trgt.l[offset] = (int32_t) *src.d;
                  break;
                case LUX_CFLOAT:
                  trgt.l[offset] = sqrt(src.cf->real*src.cf->real
                                        + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  trgt.l[offset] = sqrt(src.cd->real*src.cd->real
                                        + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_FLOAT:
              switch (srcType) {
                case LUX_INT8:
                  trgt.f[offset] = (float) *src.b;
                  break;
                case LUX_INT16:
                  trgt.f[offset] = (float) *src.w;
                  break;
                case LUX_INT32:
                  trgt.f[offset] = (float) *src.l;
                  break;
                case LUX_FLOAT:
                  trgt.f[offset] = *src.f;
                  break;
                case LUX_DOUBLE:
                  trgt.f[offset] = (float) *src.d;
                  break;
                case LUX_CFLOAT:
                  trgt.f[offset] = sqrt(src.cf->real*src.cf->real
                                        + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  trgt.f[offset] = sqrt(src.cd->real*src.cd->real
                                        + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_DOUBLE:
              switch (srcType) {
                case LUX_INT8:
                  trgt.d[offset] = (double) *src.b;
                  break;
                case LUX_INT16:
                  trgt.d[offset] = (double) *src.w;
                  break;
                case LUX_INT32:
                  trgt.d[offset] = (double) *src.l;
                  break;
                case LUX_FLOAT:
                  trgt.d[offset] = (double) *src.f;
                  break;
                case LUX_DOUBLE:
                  trgt.d[offset] = *src.d;
                  break;
                case LUX_CFLOAT:
                  trgt.d[offset] = sqrt(src.cf->real*src.cf->real
                                        + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  trgt.d[offset] = sqrt(src.cd->real*src.cd->real
                                        + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_CFLOAT:
              switch (srcType) {
                case LUX_INT8:
                  trgt.cf[offset].real = *src.b;
                  trgt.cf[offset].imaginary = 0;
                  break;
                case LUX_INT16:
                  trgt.cf[offset].real = *src.w;
                  trgt.cf[offset].imaginary = 0;
                  break;
                case LUX_INT32:
                  trgt.cf[offset].real = *src.l;
                  trgt.cf[offset].imaginary = 0;
                  break;
                case LUX_FLOAT:
                  trgt.cf[offset].real = *src.f;
                  trgt.cf[offset].imaginary = 0;
                  break;
                case LUX_DOUBLE:
                  trgt.cf[offset].real = *src.d;
                  trgt.cf[offset].imaginary = 0;
                  break;
                case LUX_CFLOAT:
                  trgt.cf[offset].real = src.cf->real;
                  trgt.cf[offset].imaginary = src.cf->imaginary;
                  break;
                case LUX_CDOUBLE:
                  trgt.cf[offset].real = src.cd->real;
                  trgt.cf[offset].imaginary = src.cd->imaginary;
                  break;
              }
              break;
            case LUX_CDOUBLE:
              switch (srcType) {
                case LUX_INT8:
                  trgt.cd[offset].real = *src.b;
                  trgt.cd[offset].imaginary = 0;
                  break;
                case LUX_INT16:
                  trgt.cd[offset].real = *src.w;
                  trgt.cd[offset].imaginary = 0;
                  break;
                case LUX_INT32:
                  trgt.cd[offset].real = *src.l;
                  trgt.cd[offset].imaginary = 0;
                  break;
                case LUX_FLOAT:
                  trgt.cd[offset].real = *src.f;
                  trgt.cd[offset].imaginary = 0;
                  break;
                case LUX_DOUBLE:
                  trgt.cd[offset].real = *src.d;
                  trgt.cd[offset].imaginary = 0;
                  break;
                case LUX_CFLOAT:
                  trgt.cd[offset].real = src.cf->real;
                  trgt.cd[offset].imaginary = src.cf->imaginary;
                  break;
                case LUX_CDOUBLE:
                  trgt.cd[offset].real = src.cd->real;
                  trgt.cd[offset].imaginary = src.cd->imaginary;
                  break;
              }
              break;
            case LUX_STRING_ARRAY: // string array
              if (trgt.sp[offset])      // already a string there
                free(trgt.sp[offset]);
              switch (srcType) {
                case LUX_STRING_ARRAY: // string array
                  trgt.sp[offset] = strsave(*src.sp);
                  break;
                case LUX_TEMP_STRING: // string
                  trgt.sp[offset] = strsave(src.s);
                  break;
              }
              break;
          }
          src.b += onestep;
          for (i = 0; i < narg; i++) {
            if (i)
              tally[i - 1] = 0;
            offset0 += tstep[i]; // update for LUX_RANGE subscripts
            tally[i]++;
            if (tally[i] != size[i])
              break;
          }
        } while (i != narg);
        break;
      case LUX_FILEMAP:
        do {
          offset = offset0;
          for (i = 0; i < narg; i++)
            if (subsc_type[i] == LUX_ARRAY)
              offset += index[i][tally[i]]*stride[i];
          if (fseek(fp, offset, SEEK_SET)) {
            fclose(fp);
            return cerror(POS_ERR, target);
          }
          switch (type) {
            case LUX_INT8: case LUX_TEMP_STRING:
              switch (srcType) {
                case LUX_INT8: case LUX_TEMP_STRING:
                  value.b = *src.b;
                  break;
                case LUX_INT16:
                  value.b = (uint8_t) *src.w;
                  break;
                case LUX_INT32:
                  value.b = (uint8_t) *src.l;
                  break;
                case LUX_FLOAT:
                  value.b = (uint8_t) *src.f;
                  break;
                case LUX_DOUBLE:
                  value.b = (uint8_t) *src.d;
                  break;
                case LUX_CFLOAT:
                  value.b = sqrt(src.cf->real*src.cf->real
                                 + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  value.b = sqrt(src.cd->real*src.cd->real
                                 + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_INT16:
              switch (srcType) {
                case LUX_INT8:
                  value.w = (int16_t) *src.b;
                  break;
                case LUX_INT16:
                  value.w = *src.w;
                  break;
                case LUX_INT32:
                  value.w = (int16_t) *src.l;
                  break;
                case LUX_FLOAT:
                  value.w = (int16_t) *src.f;
                  break;
                case LUX_DOUBLE:
                  value.w = (int16_t) *src.d;
                  break;
                case LUX_CFLOAT:
                  value.w = sqrt(src.cf->real*src.cf->real
                                 + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  value.w = sqrt(src.cd->real*src.cd->real
                                 + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_INT32:
              switch (srcType) {
                case LUX_INT8:
                  value.l = (int32_t) *src.b;
                  break;
                case LUX_INT16:
                  value.l = (int32_t) *src.w;
                  break;
                case LUX_INT32:
                  value.l = *src.l;
                  break;
                case LUX_FLOAT:
                  value.l = (int32_t) *src.f;
                  break;
                case LUX_DOUBLE:
                  value.l = (int32_t) *src.d;
                  break;
                case LUX_CFLOAT:
                  value.l = sqrt(src.cf->real*src.cf->real
                                 + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  value.l = sqrt(src.cd->real*src.cd->real
                                 + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_FLOAT:
              switch (srcType) {
                case LUX_INT8:
                  value.f = (float) *src.b;
                  break;
                case LUX_INT16:
                  value.f = (float) *src.w;
                  break;
                case LUX_INT32:
                  value.f = (float) *src.l;
                  break;
                case LUX_FLOAT:
                  value.f = *src.f;
                  break;
                case LUX_DOUBLE:
                  value.f = (float) *src.d;
                  break;
                case LUX_CFLOAT:
                  value.f = sqrt(src.cf->real*src.cf->real
                                 + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  value.f = sqrt(src.cd->real*src.cd->real
                                 + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_DOUBLE:
              switch (srcType) {
                case LUX_INT8:
                  value.d = (double) *src.b;
                  break;
                case LUX_INT16:
                  value.d = (double) *src.w;
                  break;
                case LUX_INT32:
                  value.d = (double) *src.l;
                  break;
                case LUX_FLOAT:
                  value.d = (double) *src.f;
                  break;
                case LUX_DOUBLE:
                  value.d = *src.d;
                  break;
                case LUX_CFLOAT:
                  value.d = sqrt(src.cf->real*src.cf->real
                                 + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  value.d = sqrt(src.cd->real*src.cd->real
                                 + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_CFLOAT:
              switch (srcType) {
                case LUX_INT8:
                  value.cf.real = *src.b;
                  value.cf.imaginary = 0;
                  break;
                case LUX_INT16:
                  value.cf.real = *src.w;
                  value.cf.imaginary = 0;
                  break;
                case LUX_INT32:
                  value.cf.real = *src.l;
                  value.cf.imaginary = 0;
                  break;
                case LUX_FLOAT:
                  value.cf.real = *src.f;
                  value.cf.imaginary = 0;
                  break;
                case LUX_DOUBLE:
                  value.cf.real = *src.d;
                  value.cf.imaginary = 0;
                  break;
                case LUX_CFLOAT:
                  value.cf.real = src.cf->real;
                  value.cf.imaginary = src.cf->imaginary;
                  break;
                case LUX_CDOUBLE:
                  value.cf.real = src.cd->real;
                  value.cf.imaginary = src.cd->imaginary;
                  break;
              }
              break;
            case LUX_CDOUBLE:
              switch (srcType) {
                case LUX_INT8:
                  value.cd.real = *src.b;
                  value.cd.imaginary = 0;
                  break;
                case LUX_INT16:
                  value.cd.real = *src.w;
                  value.cd.imaginary = 0;
                  break;
                case LUX_INT32:
                  value.cd.real = *src.l;
                  value.cd.imaginary = 0;
                  break;
                case LUX_FLOAT:
                  value.cd.real = *src.f;
                  value.cd.imaginary = 0;
                  break;
                case LUX_DOUBLE:
                  value.cd.real = *src.d;
                  value.cd.imaginary = 0;
                  break;
                case LUX_CFLOAT:
                  value.cd.real = src.cf->real;
                  value.cd.imaginary = src.cf->imaginary;
                  break;
                case LUX_CDOUBLE:
                  value.cd.real = src.cd->real;
                  value.cd.imaginary = src.cd->imaginary;
                  break;
              }
              break;
          }
          fwrite(&value.b, unit, 1, fp);
          if (ferror(fp)) {
            fclose(fp);
            return cerror(WRITE_ERR, target);
          }
          src.b += onestep;
          for (i = 0; i < narg; i++) {
            if (i)
              tally[i - 1] = 0;
            offset0 += tstep[i]; // update for LUX_RANGE subscripts
            tally[i]++;
            if (tally[i] != size[i])
              break;
          }
        } while (i != narg);
        fclose(fp);
        break;
      }
  }
  return 1;
}
//------------------------------------------------------------------
int32_t einsert(int32_t lhs, int32_t rhs)
// an insertion statement with subscripted target
// lhs = an LUX_EXTRACT symbol; rhs is already evaluated
{
  int32_t       target, source, i, iq, start[MAX_DIMS], width, *dims,
    ndim, nelem, type, size[MAX_DIMS], subsc_type[MAX_DIMS],
    *index[MAX_DIMS], *iptr, j, n, class_id, srcNdim, *srcDims, srcNelem,
    srcType, stride[MAX_DIMS], tally[MAX_DIMS], offset0, nmult,
    tstep[MAX_DIMS], offset, onestep, unit, combineType, narg,
    oldInternalMode, *ps2, srcMult;
  int16_t       *ps;
  Pointer       src, trgt;
  wideScalar    value;
  char  *name, keepps2;
  FILE  *fp;
  extern int32_t        trace, step, insert_subr;
  int32_t       lux_assoc_output(int32_t, int32_t, int32_t, int32_t),
    treatListArguments(int32_t *, int32_t **, int32_t);

  target = extract_target(lhs);
  source = rhs;
  /* we assume there is exactly one set of subscripts and that it is of
     LUX_RANGE type */
  ps = extract_ptr(lhs)->ptr.w;
  narg = extract_ptr(lhs)->number;
  if (narg > MAX_DIMS)
    return cerror(N_ARG_OVR, 0);
  // check if legal to change target
  if (target <= nFixed)
    return cerror(CST_LHS, target);
  // check if target is transfer symbol, and resolve if so
  class_id = symbol_class(target);

  offset0 = 0;                  // so it can be updated for LUX_FILEMAPs

  // first we look for any keywords
  nelem = 0;
  for (i = 0; i < narg; i++) {
    if (symbol_class(ps[i]) == LUX_KEYWORD)
      nelem++;
  }
  oldInternalMode = internalMode;
  internalMode = 0;
  ps2 = (int32_t*) malloc((narg - nelem)*sizeof(int32_t));
  j = 0;
  if (!ps2)
    return cerror(ALLOC_ERR, 0);
  keepps2 = 0;
  for (i = 0; i < narg; i++) {
    if (symbol_class(ps[i]) == LUX_KEYWORD) {
      n = matchKey(keyword_name_symbol(ps[i]),
                   ((keyList *) subroutine[insert_subr].keys)->keys, &iq);
      if (n != ORKEY) {
        internalMode = oldInternalMode;
        return luxerror("Illegal keyword %s", 0, keyword_name(ps[i]));
      }
    } else
      ps2[j++] = eval(ps[i]);
  }
  narg -= nelem;                // number of non-keyword arguments
  if (treatListArguments(&narg, &ps2, 0) == LUX_ERROR) {
    free(ps2);
    return LUX_ERROR;
  }

  switch (internalMode & 3) {
    case 0:
      combineType = UNDEFINED;
      break;
    case 1:
      combineType = INNER;
      break;
    case 2:
      combineType = OUTER;
      break;
    case 3:
      free(ps2);
      return luxerror("Specified incompatible /INNER and /OUTER", 0);
  }

  switch (class_id) {
    default:
      free(ps2);
      return cerror(SUBSC_NO_INDX, target);
    case LUX_STRING:
      ndim = 1;
      nelem = string_size(target);
      dims = &nelem;
      trgt.s = string_value(target);
      type = string_type(target);
      break;
    case LUX_ASSOC:
      ndim = assoc_num_dims(target);
      dims = assoc_dims(target);
      nelem = 1;
      for (i = 0; i < ndim; i++)
        nelem *= dims[i];
      break;
    case LUX_FILEMAP:
      if (file_map_readonly(target))
        return luxerror("File array is read-only", target);
      ndim = file_map_num_dims(target);
      dims = file_map_dims(target);
      nelem = file_map_size(target);
      type = file_map_type(target);
      name = file_map_file_name(target);
      // We want to open the file for updating at any position in that file.
      // If it does not yet exist, then it should be created.  DEC Ultrix
      // does all this with the a+ file access mode, but SGI Irix does not
      // allow overwriting the existing contents of a file with a+ mode,
      // and complains if a file does not yet exist when opened with r+
      // mode.  Solution: catch r+ complaint and do a w+ if necessary.
      if ((fp = fopen(name, "r+")) == NULL
          && (fp = fopen(name, "w+")) == NULL) {
        printf("File %s: ", name);
        free(ps2);
        return cerror(ERR_OPEN, target);
      }
      offset0 = file_map_has_offset(target)? file_map_offset(target): 0;
      break;
    case LUX_ARRAY: case LUX_CARRAY:
      ndim = array_num_dims(target);
      dims = array_dims(target);
      nelem = array_size(target);
      type = array_type(target);
      trgt.l = (int32_t *) array_data(target);
      break;
  }

  unit = lux_type_size[type];

  if (narg > ndim) {
    free(ps2);
    return cerror(N_DIMS_OVR, target);
  }

  switch (symbol_class(source)) {
    default:
      free(ps2);
      return cerror(ILL_CLASS, source);
    case LUX_SCAL_PTR:
      source = dereferenceScalPointer(source);
      // fall-thru
    case LUX_SCALAR:
      srcNdim = 1;
      srcNelem = 1;
      srcDims = &srcNelem;
      srcMult = 0;
      srcType = scalar_type(source);
      src.l = &scalar_value(source).l;
      break;
    case LUX_CSCALAR:
      srcNdim = 1;
      srcNelem = 1;
      srcDims = &srcNelem;
      srcMult = 0;
      srcType = complex_scalar_type(source);
      src.cf = complex_scalar_data(source).cf;
      break;
    case LUX_STRING:
      srcNdim = 1;
      if (class_id == LUX_ARRAY)        // insert string in string array
        srcNelem = 1;
      else                      // insert string in string
        srcNelem = string_size(source);
      srcDims = &srcNelem;
      srcMult = (srcNelem > 1);
      srcType = LUX_TEMP_STRING;
      src.s = string_value(source);
      break;
    case LUX_ARRAY: case LUX_CARRAY:
      srcNdim = array_num_dims(source);
      srcDims = array_dims(source);
      srcMult = 0;
      for (i = 0; i < srcNdim; i++)
        srcMult += (srcDims[i] > 1);
      srcNelem = array_size(source);
      srcType = array_type(source);
      src.l = (int32_t *) array_data(source);
      break;
  }

  // get info about all subscripts
  // start[i] = start index (for LUX_RANGE subscripts)
  // size[i] = number of indices
  // index[i] = pointer to list of indices (for LUX_ARRAY subscripts)
  // subsc_type[i] = type of subscript: LUX_RANGE (scalar or range) or
  //           LUX_ARRAY (array of indices)

  pegMark();                    /* because we use eval() below, and eval()
                                   does not mark temporaries in which final
                                   results are returned.  LS 19jan99 */
  iq = 0;

  if (internalMode & 64) {      // /SEPARATE
    // treat the elements of the single subscript (a numerical array)
    // as if that many separate scalars were specified.  LS 23jul98
    if (narg != 1               // but not exactly one subscript
        || !symbolIsNumericalArray(ps2[0]) // or it's not a numerical array
        || array_size(ps2[0]) > ndim) // or it has too many elements
      iq = luxerror("Wrong subscript for /SEPARATE", ps2[0]);
    if (iq != LUX_ERROR) {
      iq = ps2[0];
      iq = lux_long(1, &iq);    // ensure LONG
      narg = array_size(iq);
      iptr = (int32_t*) array_data(iq);
      for (i = 0; i < narg; i++) {
        start[i] = *iptr++;
        if (start[i] < 0 || start[i] >= dims[i]) { // illegal subscript value
          iq = cerror(ILL_SUBSC, ps2[0], start[i], dims[i]);
          break;
        }
        size[i] = 1;
        subsc_type[i] = LUX_RANGE;
      }
      nmult = 0;                // no multiple-element subscripts
    }
  } else {                      // no /SEPARATE
    nmult = 0;
    for (i = 0; i < narg; i++) { // all subscripts
      iq = ps2[i];              // next subscript
      mark(iq);
      switch (symbol_class(iq)) { // subscript class
        case LUX_SCALAR:
          start[i] = int_arg(iq); // start index (scalar value)
          if (narg == 1                 // only a single subscript
              && (internalMode & 48) == 0) // no /ALL or /ZERO keywords
                                // -> treat the array as if it were 1D
            width = nelem;
          else
            width = dims[i];
          if (start[i] < 0 || start[i] >= width) { // check if within range
            iq = cerror(ILL_SUBSC, iq, start[i], width);
            break;
          }
          size[i] = 1;          // size (1 element)
          subsc_type[i] = LUX_RANGE; // subscript type
          break;
        case LUX_ARRAY:
          iq = lux_long(1, &iq); // get LONG indices
          n = size[i] = array_size(iq); // number of array elements
          if (n == 1) {                 // only one element: mimic scalar
            start[i] = *(int32_t *) array_data(iq);
            if (narg == 1)      // single subscript only
              width = nelem;
            else
              width = dims[i];
            if (start[i] < 0 || start[i] >= width) {
              iq = cerror(ILL_SUBSC, iq, start[i], width);
              break;
            }
            subsc_type[i] = LUX_RANGE;
          } else {
            iptr = index[i] = (int32_t *) array_data(iq);
            keepps2 = 1;        /* we should not delete until the end of
                                 the routine, or else our index data
                                 may be clobbered too soon */
            if (narg == 1)      // only a single subscript; addresses
                                // whole source array as if it were 1D
              width = nelem;
            else
              width = dims[i];
            while (n--) {       // check if all subscripts are within range
              if (*iptr < 0 || *iptr >= width) {
                iq = cerror(ILL_SUBSC, iq, *iptr, width);
                break;
              }
              iptr++;
            }
            if (iq == LUX_ERROR)
              break;
            nmult++;
            if (combineType == UNDEFINED && array_num_dims(iq) > 1)
              combineType = INNER;
            subsc_type[i] = LUX_ARRAY;
          }
          break;
        case LUX_RANGE:
          if (!range_scalar(iq)) {
            iq = cerror(ILL_CLASS, iq);
            break;
          }
          // get here if scalar range
          iq = convertRange(iq);// convert to subscript pointer
          if (iq < 0)           // some error occurred
            break;
          // fall-thru
        case LUX_SUBSC_PTR:
          start[i] = subsc_ptr_start(iq);
          if (start[i] < 0)     // count from end of range
            start[i] += dims[i];
          if (start[i] < 0 || start[i] >= dims[i]) {
            iq = cerror(RANGE_START, iq, start[i], dims[i]);
            break;
          }
          size[i] = subsc_ptr_end(iq); // endpoint
          if (size[i] < 0)      // count from end of range
            size[i] += dims[i];
          if (size[i] < start[i] || size[i] >= dims[i]) {
            iq = cerror(RANGE_END, iq, size[i], dims[i]);
            break;
          }
          size[i] += 1 - start[i]; // now we have the size
          if (subsc_ptr_redirect(iq) >= 0) {
            iq = luxerror("No redirection allowed on insert target", target);
            break;
          }
          if (subsc_ptr_sum(iq)) {
            iq = luxerror("No summation allowed on insert target", target);
            break;
          }
          subsc_type[i] = LUX_RANGE;
          if (size[i] > 1)
            nmult++;
          break;
        default:
          iq = cerror(ILL_SUBSC_TYPE, iq);
          break;
      }
    }
  }

  if (!keepps2 || iq == LUX_ERROR) {
    free(ps2);
    zapMarked();
  }
  if (iq == LUX_ERROR)          // some error
    return LUX_ERROR;

  switch (class_id) {
    case LUX_ASSOC:
      // only a list of scalars is allowed here
      if (nmult) {
        iq = cerror(ILL_SUBSC_LHS, target);
        goto einsert_1;
      }
      if (narg > 1) {           // more than one subscript
        i = array_scratch(LUX_INT32, 1, &narg);
        memcpy(array_data(i), start, narg);
        offset = -1;
      } else {
        i = start[0];
        offset = -3;
      }
      n = lux_assoc_output(target, source, i, offset);
      iq = target;
      goto einsert_1;
  }

  /* get here if source is a (numerical, string, or file) array or a
     string */

  // NOTE that (file) array treatment is not streamlined (yet) for
  // insertion of contiguous blocks, (except for the simplest cases)
  // so it may be unnecessarily slow.  LS 18jun97

  // if either source or target contains strings, then both must
  if (isStringType(type) ^ isStringType(srcType)) {
    iq = cerror(INCMP_ARG, source);
    goto einsert_1;
  }

  if (class_id == LUX_ARRAY && isStringType(type)) // string array
    type = LUX_STRING_ARRAY;    // distinguish string array from string
  if (symbol_class(source) == LUX_ARRAY && isStringType(srcType))
    srcType = LUX_STRING_ARRAY;

  onestep = lux_type_size[srcType];

  switch (internalMode & 48) {
    case 0:                     // none
      if (narg > 1 && narg != ndim) {
        iq = cerror(ILL_N_SUBSC, target);
        goto einsert_1;
      }
      break;
    case 16:                    // /ZERO
      for (i = narg; i < ndim; i++) {
        start[i] = 0;
        size[i] = 1;
        subsc_type[i] = LUX_RANGE;
      }
      narg = ndim;
      break;
    case 32:                    // /ALL
      for (i = narg; i < ndim; i++) {
        start[i] = 0;
        size[i] = dims[i];
        if (size[i] > 1)
          nmult++;
        subsc_type[i] = LUX_RANGE;
      }
      narg = ndim;
      break;
    default:
      iq = luxerror("Specified incompatible /ZERO and /ALL", 0);
      goto einsert_1;
  }

  if ((trace > executeLevel && (traceMode & 127))
       || step > executeLevel) { // output some info
    printf("      ( %s(", symbolIdent(target, I_TRUNCATE));
    for (j = 0; j < extract_ptr(lhs)->number; j++) {
      if (j)
        putchar(',');
      printf("%s", symbolIdent(ps[j], I_VALUE | I_TRUNCATE));
    }
    printf(") = %s )\n", symbolIdent(source, I_VALUE | I_TRUNCATE));
  }

  if (combineType == UNDEFINED)
    combineType = OUTER;

  if (combineType == INNER)
                                // /INNER - inner-style subscript
                                // combinations
  // each element in the first dimension is combined with only
  // the corresponding elements in the other dimensions
  {
    // the number of elements in all dimensions must be the same
    for (i = 1; i < narg; i++)
      if (size[i] != size[0]) {
        iq = cerror(INCMP_DIMS, target);
        goto einsert_1;
      }

    // the number of elements in the dimensions must equal the
    // number of elements in the source -- unless the source has
    // only one element.
    if (srcNelem == 1)          // only one element: insert everywhere
      onestep = 0;
    else if (size[0] != srcNelem) {
      iq = cerror(INCMP_ARG, source);
      goto einsert_1;
    }

    n = (class_id == LUX_FILEMAP)? unit: 1;
    for (i = 0; i < narg; i++) {
      stride[i] = n;
      n *= dims[i];
    }

    switch (class_id) {
      case LUX_ARRAY: case LUX_CARRAY:
        for (j = 0; j < size[0]; j++) {
          offset = 0;
          for (i = 0; i < narg; i++)
            offset += ((subsc_type[i] == LUX_ARRAY)? index[i][j]: start[i]++)
              * stride[i];
          switch (type) {
            case LUX_INT8: case LUX_TEMP_STRING:
              switch (srcType) {
                case LUX_INT8: case LUX_TEMP_STRING:
                  trgt.b[offset] = *src.b;
                  break;
                case LUX_INT16:
                  trgt.b[offset] = (uint8_t) *src.w;
                  break;
                case LUX_INT32:
                  trgt.b[offset] = (uint8_t) *src.l;
                  break;
                case LUX_FLOAT:
                  trgt.b[offset] = (uint8_t) *src.f;
                  break;
                case LUX_DOUBLE:
                  trgt.b[offset] = (uint8_t) *src.d;
                  break;
                case LUX_CFLOAT:
                  trgt.b[offset] = sqrt(src.cf->real*src.cf->real
                                        + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  trgt.b[offset] = sqrt(src.cd->real*src.cd->real
                                        + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_INT16:
              switch (srcType) {
                case LUX_INT8:
                  trgt.w[offset] = (int16_t) *src.b;
                  break;
                case LUX_INT16:
                  trgt.w[offset] = *src.w;
                  break;
                case LUX_INT32:
                  trgt.w[offset] = (int16_t) *src.l;
                  break;
                case LUX_FLOAT:
                  trgt.w[offset] = (int16_t) *src.f;
                  break;
                case LUX_DOUBLE:
                  trgt.w[offset] = (int16_t) *src.d;
                  break;
                case LUX_CFLOAT:
                  trgt.w[offset] = sqrt(src.cf->real*src.cf->real
                                        + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  trgt.w[offset] = sqrt(src.cd->real*src.cd->real
                                        + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_INT32:
              switch (srcType) {
                case LUX_INT8:
                  trgt.l[offset] = (int32_t) *src.b;
                  break;
                case LUX_INT16:
                  trgt.l[offset] = (int32_t) *src.w;
                  break;
                case LUX_INT32:
                  trgt.l[offset] = *src.l;
                  break;
                case LUX_FLOAT:
                  trgt.l[offset] = (int32_t) *src.f;
                  break;
                case LUX_DOUBLE:
                  trgt.l[offset] = (int32_t) *src.d;
                  break;
                case LUX_CFLOAT:
                  trgt.l[offset] = sqrt(src.cf->real*src.cf->real
                                        + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  trgt.l[offset] = sqrt(src.cd->real*src.cd->real
                                        + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_FLOAT:
              switch (srcType) {
                case LUX_INT8:
                  trgt.f[offset] = (float) *src.b;
                  break;
                case LUX_INT16:
                  trgt.f[offset] = (float) *src.w;
                  break;
                case LUX_INT32:
                  trgt.f[offset] = (float) *src.l;
                  break;
                case LUX_FLOAT:
                  trgt.f[offset] = *src.f;
                  break;
                case LUX_DOUBLE:
                  trgt.f[offset] = (float) *src.d;
                  break;
                case LUX_CFLOAT:
                  trgt.f[offset] = sqrt(src.cf->real*src.cf->real
                                        + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  trgt.f[offset] = sqrt(src.cd->real*src.cd->real
                                        + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_DOUBLE:
              switch (srcType) {
                case LUX_INT8:
                  trgt.d[offset] = (double) *src.b;
                  break;
                case LUX_INT16:
                  trgt.d[offset] = (double) *src.w;
                  break;
                case LUX_INT32:
                  trgt.d[offset] = (double) *src.l;
                  break;
                case LUX_FLOAT:
                  trgt.d[offset] = (double) *src.f;
                  break;
                case LUX_DOUBLE:
                  trgt.d[offset] = *src.d;
                  break;
                case LUX_CFLOAT:
                  trgt.d[offset] = sqrt(src.cf->real*src.cf->real
                                        + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  trgt.d[offset] = sqrt(src.cd->real*src.cd->real
                                        + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_CFLOAT:
              switch (srcType) {
                case LUX_INT8:
                  trgt.cf[offset].real = (float) *src.b;
                  trgt.cf[offset].imaginary = 0;
                  break;
                case LUX_INT16:
                  trgt.cf[offset].real = (float) *src.w;
                  trgt.cf[offset].imaginary = 0;
                  break;
                case LUX_INT32:
                  trgt.cf[offset].real = (float) *src.l;
                  trgt.cf[offset].imaginary = 0;
                  break;
                case LUX_FLOAT:
                  trgt.cf[offset].real = *src.f;
                  trgt.cf[offset].imaginary = 0;
                  break;
                case LUX_DOUBLE:
                  trgt.cf[offset].real = (float) *src.d;
                  trgt.cf[offset].imaginary = 0;
                  break;
                case LUX_CFLOAT:
                  trgt.cf[offset].real = src.cf->real;
                  trgt.cf[offset].imaginary = src.cf->imaginary;
                  break;
                case LUX_CDOUBLE:
                  trgt.cf[offset].real = (float) src.cd->real;
                  trgt.cf[offset].imaginary = (float) src.cd->imaginary;
                  break;
              }
              break;
            case LUX_CDOUBLE:
              switch (srcType) {
                case LUX_INT8:
                  trgt.cd[offset].real = (double) *src.b;
                  trgt.cd[offset].imaginary = 0;
                  break;
                case LUX_INT16:
                  trgt.cd[offset].real = (double) *src.w;
                  trgt.cd[offset].imaginary = 0;
                  break;
                case LUX_INT32:
                  trgt.cd[offset].real = (double) *src.l;
                  trgt.cd[offset].imaginary = 0;
                  break;
                case LUX_FLOAT:
                  trgt.cd[offset].real = (double) *src.f;
                  trgt.cd[offset].imaginary = 0;
                  break;
                case LUX_DOUBLE:
                  trgt.cd[offset].real = (double) *src.d;
                  trgt.cd[offset].imaginary = 0;
                  break;
                case LUX_CFLOAT:
                  trgt.cd[offset].real = (double) src.cf->real;
                  trgt.cd[offset].imaginary = (double) src.cf->imaginary;
                  break;
                case LUX_CDOUBLE:
                  trgt.cd[offset].real = (double) src.cd->real;
                  trgt.cd[offset].imaginary = (double) src.cd->imaginary;
                  break;
              }
              break;
            case 999:           // string array
              if (trgt.sp[offset])      // already a string there
                free(trgt.sp[offset]);
              switch (srcType) {
                case 999:               // string array
                  trgt.sp[offset] = strsave(*src.sp);
                  break;
                case LUX_TEMP_STRING: // string
                  trgt.sp[offset] = strsave(src.s);
                  break;
              }
              break;
          }
          src.b += onestep;
        }
        break;
      case LUX_FILEMAP:
        for (j = 0; j < size[0]; j++) {
          offset = offset0;
          for (i = 0; i < narg; i++)
            offset += ((subsc_type[i] == LUX_ARRAY)? index[i][j]: start[i]++)
              * stride[i];
          if (fseek(fp, offset, SEEK_SET)) {
            fclose(fp);
            iq = cerror(POS_ERR, target);
            goto einsert_1;
          }
          switch (type) {
            case LUX_INT8: case LUX_TEMP_STRING:
              switch (srcType) {
                case LUX_INT8: case LUX_TEMP_STRING:
                  value.b = *src.b;
                  break;
                case LUX_INT16:
                  value.b = (uint8_t) *src.w;
                  break;
                case LUX_INT32:
                  value.b = (uint8_t) *src.l;
                  break;
                case LUX_FLOAT:
                  value.b = (uint8_t) *src.f;
                  break;
                case LUX_DOUBLE:
                  value.b = (uint8_t) *src.d;
                  break;
                case LUX_CFLOAT:
                  value.b = sqrt(src.cf->real*src.cf->real
                                 + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  value.b = sqrt(src.cd->real*src.cd->real
                                 + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_INT16:
              switch (srcType) {
                case LUX_INT8:
                  value.w = (int16_t) *src.b;
                  break;
                case LUX_INT16:
                  value.w = *src.w;
                  break;
                case LUX_INT32:
                  value.w = (int16_t) *src.l;
                  break;
                case LUX_FLOAT:
                  value.w = (int16_t) *src.f;
                  break;
                case LUX_DOUBLE:
                  value.w = (int16_t) *src.d;
                  break;
                case LUX_CFLOAT:
                  value.w = sqrt(src.cf->real*src.cf->real
                                 + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  value.w = sqrt(src.cd->real*src.cd->real
                                 + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_INT32:
              switch (srcType) {
                case LUX_INT8:
                  value.l = (int32_t) *src.b;
                  break;
                case LUX_INT16:
                  value.l = (int32_t) *src.w;
                  break;
                case LUX_INT32:
                  value.l = *src.l;
                  break;
                case LUX_FLOAT:
                  value.l = (int32_t) *src.f;
                  break;
                case LUX_DOUBLE:
                  value.l = (int32_t) *src.d;
                  break;
                case LUX_CFLOAT:
                  value.l = sqrt(src.cf->real*src.cf->real
                                 + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  value.l = sqrt(src.cd->real*src.cd->real
                                 + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_FLOAT:
              switch (srcType) {
                case LUX_INT8:
                  value.f = (float) *src.b;
                  break;
                case LUX_INT16:
                  value.f = (float) *src.w;
                  break;
                case LUX_INT32:
                  value.f = (float) *src.l;
                  break;
                case LUX_FLOAT:
                  value.f = *src.f;
                  break;
                case LUX_DOUBLE:
                  value.f = (float) *src.d;
                  break;
                case LUX_CFLOAT:
                  value.f = sqrt(src.cf->real*src.cf->real
                                 + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  value.f = sqrt(src.cd->real*src.cd->real
                                 + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_DOUBLE:
              switch (srcType) {
                case LUX_INT8:
                  value.d = (double) *src.b;
                  break;
                case LUX_INT16:
                  value.d = (double) *src.w;
                  break;
                case LUX_INT32:
                  value.d = (double) *src.l;
                  break;
                case LUX_FLOAT:
                  value.d = (double) *src.f;
                  break;
                case LUX_DOUBLE:
                  value.d = *src.d;
                  break;
                case LUX_CFLOAT:
                  value.f = sqrt(src.cf->real*src.cf->real
                                 + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  value.d = sqrt(src.cd->real*src.cd->real
                                 + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_CFLOAT:
              switch (srcType) {
                case LUX_INT8:
                  value.cf.real = *src.b;
                  value.cf.imaginary = 0;
                  break;
                case LUX_INT16:
                  value.cf.real = *src.w;
                  value.cf.imaginary = 0;
                  break;
                case LUX_INT32:
                  value.cf.real = *src.l;
                  value.cf.imaginary = 0;
                  break;
                case LUX_FLOAT:
                  value.cf.real = *src.f;
                  value.cf.imaginary = 0;
                  break;
                case LUX_DOUBLE:
                  value.cf.real = *src.d;
                  value.cf.imaginary = 0;
                  break;
                case LUX_CFLOAT:
                  value.cf.real = src.cf->real;
                  value.cf.imaginary = src.cf->imaginary;
                  break;
                case LUX_CDOUBLE:
                  value.cf.real = src.cd->real;
                  value.cf.imaginary = src.cd->imaginary;
                  break;
              }
              break;
          }
          fwrite(&value.b, unit, 1, fp);
          if (ferror(fp)) {
            fclose(fp);
            iq = cerror(WRITE_ERR, target);
            goto einsert_1;
          }
          src.b += onestep;
        }
        fclose(fp);
        break;
    }
  } else {                      // /OUTER - outer-style subscript
                                // combinations
  // each value of each coordinate is
  // combined with each value of each other coordinate

    // the number of elements implied by the subscripts must match that in
    // the source for each dimension
    // the number of dimensions in the source must be matched by
    /* the total number of subscripts, and the number of multiple-element
     subscripts in the source must match that in the target */

    if (srcNelem != 1 && srcMult != nmult && srcNdim != narg && nmult) {
      iq = cerror(ILL_SUBSC, target);
      goto einsert_1;
    }

    j = 0;                      /* counts the number of subscripts with
                                   size greater than 1 */
    if (srcNelem > 1) {
      if (srcNdim == nmult) {   // disregard scalar subscripts
        for (i = 0; i < narg; i++) { // all subscripts
          if (size[i] > 1) {
            if (j >= srcNdim) {
              iq = luxerror("Too many multi-element subscripts", target);
              goto einsert_1;
            }
            if (size[i] != srcDims[j]) {
              iq = luxerror("Subscript size incompatible with source dimension",
                         target);
              goto einsert_1;
            }
            j++;
          } // end of if (size[i]...)
        }       // end of for (i = 0; ...)
      } else {                  // scalar subscripts matter
        for (i = 0; i < narg; i++) {
          if (size[i] > 1 && size[i] != srcDims[i]) {
            iq = luxerror("Subscript size incompatible with source dimension",
                       target);
            goto einsert_1;
          }
          if (size[i] > 1)
            j++;                // all OK
        }
      }
    } else                      // one source element: insert everywhere
      for (i = 0; i < narg; i++)
        if (size[i] > 1)
          j++;

    if (!j)                     // all subscripts are scalars:
      // they indicate the start address.  If there is only one subscript,
      // or if the number of subscripts is greater than the number of
      // dimensions in the source, or if the /ONEDIM keyword is specified,
      // then 1D insertion is performed.

      if (narg > 1 && srcNdim == narg && (internalMode & 4) == 0) {
        // multi-dimensional insertion
        for (i = 0; i < narg; i++)
          size[i] = srcDims[i];
        j = srcNdim;            // to indicate multi-dimensional insertion
      }
    if (!j) {                   // 1D insert
      if (srcNelem == 1)        // enter scalar in all elements
        srcDims = &nelem;
      else
        srcDims = &srcNelem;
      srcNdim = 1;
      offset = 0;
      n = 1;
      for (i = 0; i < narg; i++) { // calculate data offset
        offset += n*start[i];
        n *= dims[i];
      }
      if (offset + srcNelem > nelem) {
        iq = luxerror("Source extends beyond target boundaries", source);
        goto einsert_1;
      }
      start[0] = offset;
      dims = &nelem;
      ndim = 1;
      narg = 1;
      size[0] = srcNelem;
    } else if (j != srcMult && srcNelem > 1) {
      iq = luxerror("Too few multi-element subscripts", target);
      goto einsert_1;
    }

    if (srcNelem == 1)
      onestep = 0;

    // now do the insertion
    // for LUX_ARRAY subscripts the offset is calculated for each index
    // for LUX_RANGE subscripts the offset is updated for each element

    n = (class_id == LUX_FILEMAP)? unit: 1;

    if (!j && ndim == 1 && srcNdim == 1 && type == srcType &&
        !isStringType(type) && (srcNelem > 1 || size[0] == 1)) {
                                // in this case we do a fast insert
      offset0 += start[0]*unit;
      switch (class_id) {
        case LUX_ARRAY: case LUX_CARRAY:
          memcpy(trgt.b + offset0, src.b, srcNelem*unit);
          break;
        case LUX_FILEMAP:
          if (fseek(fp, offset0, SEEK_SET)) {
            fclose(fp);
            if (ferror(fp))
              perror("System message");
            iq = cerror(POS_ERR, target);
            goto einsert_1;
          }
          fwrite(src.b, unit, srcNelem, fp);
          if (ferror(fp)) {
            fclose(fp);
            iq = cerror(WRITE_ERR, target);
            goto einsert_1;
          }
          fclose(fp);
          break;
      }
      iq = LUX_OK;
      goto einsert_1;
    }

    for (i = 0; i < narg; i++) {
      stride[i] = n;
      tally[i] = 0;
      if (subsc_type[i] == LUX_RANGE)
        offset0 += start[i]*n;
      n *= dims[i];
    }

    if (subsc_type[0] == LUX_RANGE) // ??
      tstep[0] = stride[0];
    else
      tstep[0] = 0;

    for (i = 1; i < narg; i++)
      tstep[i] = (subsc_type[i] == LUX_RANGE? stride[i]: 0)
        - (subsc_type[i - 1] == LUX_RANGE? size[i - 1]*stride[i - 1]: 0);

    switch (class_id) {
      case LUX_ARRAY: case LUX_CARRAY:
        do {
          offset = offset0;
          for (i = 0; i < narg; i++)
            if (subsc_type[i] == LUX_ARRAY)
              offset += index[i][tally[i]]*stride[i];
          switch (type) {
            case LUX_INT8: case LUX_TEMP_STRING:
              switch (srcType) {
                case LUX_INT8: case LUX_TEMP_STRING:
                  trgt.b[offset] = *src.b;
                  break;
                case LUX_INT16:
                  trgt.b[offset] = (uint8_t) *src.w;
                  break;
                case LUX_INT32:
                  trgt.b[offset] = (uint8_t) *src.l;
                  break;
                case LUX_FLOAT:
                  trgt.b[offset] = (uint8_t) *src.f;
                  break;
                case LUX_DOUBLE:
                  trgt.b[offset] = (uint8_t) *src.d;
                  break;
                case LUX_CFLOAT:
                  trgt.b[offset] = sqrt(src.cf->real*src.cf->real
                                        + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  trgt.b[offset] = sqrt(src.cd->real*src.cd->real
                                        + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_INT16:
              switch (srcType) {
                case LUX_INT8:
                  trgt.w[offset] = (int16_t) *src.b;
                  break;
                case LUX_INT16:
                  trgt.w[offset] = *src.w;
                  break;
                case LUX_INT32:
                  trgt.w[offset] = (int16_t) *src.l;
                  break;
                case LUX_FLOAT:
                  trgt.w[offset] = (int16_t) *src.f;
                  break;
                case LUX_DOUBLE:
                  trgt.w[offset] = (int16_t) *src.d;
                  break;
                case LUX_CFLOAT:
                  trgt.w[offset] = sqrt(src.cf->real*src.cf->real
                                        + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  trgt.w[offset] = sqrt(src.cd->real*src.cd->real
                                        + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_INT32:
              switch (srcType) {
                case LUX_INT8:
                  trgt.l[offset] = (int32_t) *src.b;
                  break;
                case LUX_INT16:
                  trgt.l[offset] = (int32_t) *src.w;
                  break;
                case LUX_INT32:
                  trgt.l[offset] = *src.l;
                  break;
                case LUX_FLOAT:
                  trgt.l[offset] = (int32_t) *src.f;
                  break;
                case LUX_DOUBLE:
                  trgt.l[offset] = (int32_t) *src.d;
                  break;
                case LUX_CFLOAT:
                  trgt.l[offset] = sqrt(src.cf->real*src.cf->real
                                        + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  trgt.l[offset] = sqrt(src.cd->real*src.cd->real
                                        + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_FLOAT:
              switch (srcType) {
                case LUX_INT8:
                  trgt.f[offset] = (float) *src.b;
                  break;
                case LUX_INT16:
                  trgt.f[offset] = (float) *src.w;
                  break;
                case LUX_INT32:
                  trgt.f[offset] = (float) *src.l;
                  break;
                case LUX_FLOAT:
                  trgt.f[offset] = *src.f;
                  break;
                case LUX_DOUBLE:
                  trgt.f[offset] = (float) *src.d;
                  break;
                case LUX_CFLOAT:
                  trgt.f[offset] = sqrt(src.cf->real*src.cf->real
                                        + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  trgt.f[offset] = sqrt(src.cd->real*src.cd->real
                                        + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_DOUBLE:
              switch (srcType) {
                case LUX_INT8:
                  trgt.d[offset] = (double) *src.b;
                  break;
                case LUX_INT16:
                  trgt.d[offset] = (double) *src.w;
                  break;
                case LUX_INT32:
                  trgt.d[offset] = (double) *src.l;
                  break;
                case LUX_FLOAT:
                  trgt.d[offset] = (double) *src.f;
                  break;
                case LUX_DOUBLE:
                  trgt.d[offset] = *src.d;
                  break;
                case LUX_CFLOAT:
                  trgt.d[offset] = sqrt(src.cf->real*src.cf->real
                                        + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  trgt.d[offset] = sqrt(src.cd->real*src.cd->real
                                        + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_CFLOAT:
              switch (srcType) {
                case LUX_INT8:
                  trgt.cf[offset].real = *src.b;
                  trgt.cf[offset].imaginary = 0;
                  break;
                case LUX_INT16:
                  trgt.cf[offset].real = *src.w;
                  trgt.cf[offset].imaginary = 0;
                  break;
                case LUX_INT32:
                  trgt.cf[offset].real = *src.l;
                  trgt.cf[offset].imaginary = 0;
                  break;
                case LUX_FLOAT:
                  trgt.cf[offset].real = *src.f;
                  trgt.cf[offset].imaginary = 0;
                  break;
                case LUX_DOUBLE:
                  trgt.cf[offset].real = *src.d;
                  trgt.cf[offset].imaginary = 0;
                  break;
                case LUX_CFLOAT:
                  trgt.cf[offset].real = src.cf->real;
                  trgt.cf[offset].imaginary = src.cf->imaginary;
                  break;
                case LUX_CDOUBLE:
                  trgt.cf[offset].real = src.cd->real;
                  trgt.cf[offset].imaginary = src.cd->imaginary;
                  break;
              }
              break;
            case LUX_CDOUBLE:
              switch (srcType) {
                case LUX_INT8:
                  trgt.cd[offset].real = *src.b;
                  trgt.cd[offset].imaginary = 0;
                  break;
                case LUX_INT16:
                  trgt.cd[offset].real = *src.w;
                  trgt.cd[offset].imaginary = 0;
                  break;
                case LUX_INT32:
                  trgt.cd[offset].real = *src.l;
                  trgt.cd[offset].imaginary = 0;
                  break;
                case LUX_FLOAT:
                  trgt.cd[offset].real = *src.f;
                  trgt.cd[offset].imaginary = 0;
                  break;
                case LUX_DOUBLE:
                  trgt.cd[offset].real = *src.d;
                  trgt.cd[offset].imaginary = 0;
                  break;
                case LUX_CFLOAT:
                  trgt.cd[offset].real = src.cf->real;
                  trgt.cd[offset].imaginary = src.cf->imaginary;
                  break;
                case LUX_CDOUBLE:
                  trgt.cd[offset].real = src.cd->real;
                  trgt.cd[offset].imaginary = src.cd->imaginary;
                  break;
              }
              break;
            case LUX_STRING_ARRAY: // string array
              if (trgt.sp[offset])      // already a string there
                free(trgt.sp[offset]);
              switch (srcType) {
                case LUX_STRING_ARRAY: // string array
                  trgt.sp[offset] = strsave(*src.sp);
                  break;
                case LUX_TEMP_STRING: // string
                  trgt.sp[offset] = strsave(src.s);
                  break;
              }
              break;
          }
          src.b += onestep;
          for (i = 0; i < narg; i++) {
            if (i)
              tally[i - 1] = 0;
            offset0 += tstep[i]; // update for LUX_RANGE subscripts
            tally[i]++;
            if (tally[i] != size[i])
              break;
          }
        } while (i != narg);
        break;
      case LUX_FILEMAP:
        do {
          offset = offset0;
          for (i = 0; i < narg; i++)
            if (subsc_type[i] == LUX_ARRAY)
              offset += index[i][tally[i]]*stride[i];
          if (fseek(fp, offset, SEEK_SET)) {
            fclose(fp);
            iq = cerror(POS_ERR, target);
            goto einsert_1;
          }
          switch (type) {
            case LUX_INT8: case LUX_TEMP_STRING:
              switch (srcType) {
                case LUX_INT8: case LUX_TEMP_STRING:
                  value.b = *src.b;
                  break;
                case LUX_INT16:
                  value.b = (uint8_t) *src.w;
                  break;
                case LUX_INT32:
                  value.b = (uint8_t) *src.l;
                  break;
                case LUX_FLOAT:
                  value.b = (uint8_t) *src.f;
                  break;
                case LUX_DOUBLE:
                  value.b = (uint8_t) *src.d;
                  break;
                case LUX_CFLOAT:
                  value.b = sqrt(src.cf->real*src.cf->real
                                 + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  value.b = sqrt(src.cd->real*src.cd->real
                                 + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_INT16:
              switch (srcType) {
                case LUX_INT8:
                  value.w = (int16_t) *src.b;
                  break;
                case LUX_INT16:
                  value.w = *src.w;
                  break;
                case LUX_INT32:
                  value.w = (int16_t) *src.l;
                  break;
                case LUX_FLOAT:
                  value.w = (int16_t) *src.f;
                  break;
                case LUX_DOUBLE:
                  value.w = (int16_t) *src.d;
                  break;
                case LUX_CFLOAT:
                  value.w = sqrt(src.cf->real*src.cf->real
                                 + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  value.w = sqrt(src.cd->real*src.cd->real
                                 + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_INT32:
              switch (srcType) {
                case LUX_INT8:
                  value.l = (int32_t) *src.b;
                  break;
                case LUX_INT16:
                  value.l = (int32_t) *src.w;
                  break;
                case LUX_INT32:
                  value.l = *src.l;
                  break;
                case LUX_FLOAT:
                  value.l = (int32_t) *src.f;
                  break;
                case LUX_DOUBLE:
                  value.l = (int32_t) *src.d;
                  break;
                case LUX_CFLOAT:
                  value.l = sqrt(src.cf->real*src.cf->real
                                 + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  value.l = sqrt(src.cd->real*src.cd->real
                                 + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_FLOAT:
              switch (srcType) {
                case LUX_INT8:
                  value.f = (float) *src.b;
                  break;
                case LUX_INT16:
                  value.f = (float) *src.w;
                  break;
                case LUX_INT32:
                  value.f = (float) *src.l;
                  break;
                case LUX_FLOAT:
                  value.f = *src.f;
                  break;
                case LUX_DOUBLE:
                  value.f = (float) *src.d;
                  break;
                case LUX_CFLOAT:
                  value.f = sqrt(src.cf->real*src.cf->real
                                 + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  value.f = sqrt(src.cd->real*src.cd->real
                                 + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_DOUBLE:
              switch (srcType) {
                case LUX_INT8:
                  value.d = (double) *src.b;
                  break;
                case LUX_INT16:
                  value.d = (double) *src.w;
                  break;
                case LUX_INT32:
                  value.d = (double) *src.l;
                  break;
                case LUX_FLOAT:
                  value.d = (double) *src.f;
                  break;
                case LUX_DOUBLE:
                  value.d = *src.d;
                  break;
                case LUX_CFLOAT:
                  value.d = sqrt(src.cf->real*src.cf->real
                                 + src.cf->imaginary*src.cf->imaginary);
                  break;
                case LUX_CDOUBLE:
                  value.d = sqrt(src.cd->real*src.cd->real
                                 + src.cd->imaginary*src.cd->imaginary);
                  break;
              }
              break;
            case LUX_CFLOAT:
              switch (srcType) {
                case LUX_INT8:
                  value.cf.real = *src.b;
                  value.cf.imaginary = 0;
                  break;
                case LUX_INT16:
                  value.cf.real = *src.w;
                  value.cf.imaginary = 0;
                  break;
                case LUX_INT32:
                  value.cf.real = *src.l;
                  value.cf.imaginary = 0;
                  break;
                case LUX_FLOAT:
                  value.cf.real = *src.f;
                  value.cf.imaginary = 0;
                  break;
                case LUX_DOUBLE:
                  value.cf.real = *src.d;
                  value.cf.imaginary = 0;
                  break;
                case LUX_CFLOAT:
                  value.cf.real = src.cf->real;
                  value.cf.imaginary = src.cf->imaginary;
                  break;
                case LUX_CDOUBLE:
                  value.cf.real = src.cd->real;
                  value.cf.imaginary = src.cd->imaginary;
                  break;
              }
              break;
            case LUX_CDOUBLE:
              switch (srcType) {
                case LUX_INT8:
                  value.cd.real = *src.b;
                  value.cd.imaginary = 0;
                  break;
                case LUX_INT16:
                  value.cd.real = *src.w;
                  value.cd.imaginary = 0;
                  break;
                case LUX_INT32:
                  value.cd.real = *src.l;
                  value.cd.imaginary = 0;
                  break;
                case LUX_FLOAT:
                  value.cd.real = *src.f;
                  value.cd.imaginary = 0;
                  break;
                case LUX_DOUBLE:
                  value.cd.real = *src.d;
                  value.cd.imaginary = 0;
                  break;
                case LUX_CFLOAT:
                  value.cd.real = src.cf->real;
                  value.cd.imaginary = src.cf->imaginary;
                  break;
                case LUX_CDOUBLE:
                  value.cd.real = src.cd->real;
                  value.cd.imaginary = src.cd->imaginary;
                  break;
              }
              break;
          }
          fwrite(&value.b, unit, 1, fp);
          if (ferror(fp)) {
            fclose(fp);
            iq = cerror(WRITE_ERR, target);
            goto einsert_1;
          }
          src.b += onestep;
          for (i = 0; i < narg; i++) {
            if (i)
              tally[i - 1] = 0;
            offset0 += tstep[i]; // update for LUX_RANGE subscripts
            tally[i]++;
            if (tally[i] != size[i])
              break;
          }
        } while (i != narg);
        fclose(fp);
        break;
      }
  }
  iq = LUX_OK;
  // fall through

  einsert_1:
  if (keepps2) {
    free(ps2);
    zapMarked();
  }
  return iq;
}
//------------------------------------------------------------------
int32_t lux_test(int32_t narg, int32_t ps[])
// a test function
{
  int32_t       n, value, *edge, i, *offset;
  LoopInfo      info;
  Pointer       src;

  if (symbol_type(ps[0]) != LUX_INT32)
    return luxerror("Accepts only LONG arrays", ps[0]);
  if (standardLoop(ps[0], 0, SL_ALLAXES | SL_EACHCOORD, LUX_INT8, &info,
                   &src, NULL, NULL, NULL) < 0)
    return LUX_ERROR;
  value = int_arg(ps[1]);

  n = prepareDiagonals(narg > 2? ps[2]: 0, &info, 2, &offset, &edge,
                       NULL, NULL);
  if (n == LUX_ERROR)
    return LUX_ERROR;

  // set the edges to zero
  for (i = 0; i < 2*info.ndim; i++) {
    if (edge[i]) {
      info.rearrangeEdgeLoop(NULL, i);
      do
        *src.l = value;
      while (info.advanceLoop(&src) < info.ndim - 1);
    }
  }

  free(edge);
  return LUX_OK;
}
