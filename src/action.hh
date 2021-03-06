/* This is file action.hh.

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
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "luxdefs.hh"
#include "error.hh"
#include "dmalloc.hh"
#include "bindings.hh"
#include "axis.hh"
#include "StandardArguments.hh"

extern char             expname[], line[], *curScrat;
extern char const* currentRoutineName;
extern int16_t          listStack[],  curContext;
extern int32_t          scrat[], lux_file_open[], errorSym,
                        MSBfirst, suppressMsg;
extern int32_t  lux_type_size[];

extern FILE             *inputStream, *lux_file[];
extern hashTableEntry   *varHashTable[], *subrHashTable[], *funcHashTable[],
                        *blockHashTable[];
extern symTableEntry    sym[];
extern internalRoutine  *subroutine, *function;
extern int32_t          nSubroutine, nFunction, curLineNumber, compileLevel,
                        ignoreInput, curSymbol, axisTally[];
extern unsigned int     internalMode;
extern struct boundsStruct      bounds;
extern int32_t  (*lux_converts[])(int32_t, int32_t []);

char    *strsave(char const *), *symbolIdent(int32_t, int32_t),
        *string_arg(int32_t), *expand_name(char const *, char const *),
        *strsave_system(char *), *nextline(char *, size_t, FILE *);
char const* symbolProperName(int32_t);
char const* symName(int32_t, hashTableEntry *[]);
char const* className(int32_t);
char const* typeName(int32_t);
char const* keyName(internalRoutine *routine, int32_t number, int32_t index);

int32_t         dereferenceScalPointer(int32_t), scalar_scratch(Symboltype),
  array_scratch(Symboltype, int32_t, int32_t []), int_arg(int32_t), array_clone(int32_t, Symboltype),
  convertRange(int32_t), popTempVariable(int32_t), int_arg_stat(int32_t, int32_t *),
  findSym(int32_t, hashTableEntry *[], int32_t), findInternalName(char const *, int32_t),
  lux_float(int32_t, int32_t []), lux_long(int32_t, int32_t []), lux_byte(int32_t , int32_t []), lux_int64(int32_t, int32_t []),
  lux_word(int32_t, int32_t []), lux_double(int32_t, int32_t []), lux_cfloat(int32_t, int32_t []),
  luxerror(char const *, int32_t, ...), cerror(int32_t, int32_t, ...), copyEvalSym(int32_t),
  getNumerical(int32_t, Symboltype, int32_t *, Pointer *, char, int32_t *, Pointer *),
  strccmp(char const *, char const *), eval(int32_t), evals(int32_t), lux_cdouble(int32_t, int32_t []),
  strncasecmp_p(char *, char *, int32_t), strcasecmp_p(char *, char *),
  stringpointer(char *, int32_t), routineContext(int32_t), lux_string(int32_t, int32_t []),
  getSimpleNumerical(int32_t, Pointer *, int32_t *), double_arg_stat(int32_t, double *),
  numerical(int32_t, int32_t **, int32_t *, int32_t *, Pointer *),
  numerical_or_string(int32_t, int32_t **, int32_t *, int32_t *, Pointer *),
  findName(char const *, hashTableEntry **, int32_t), to_scalar(int32_t, Symboltype),
  to_scratch_array(int32_t, Symboltype, int32_t, int32_t *), get_dims(int32_t *, int32_t *, int32_t *),
  listNumElements(int32_t), lux_zero(int32_t, int32_t []),
  cubic_spline_tables(void *, int32_t, int32_t, void *, int32_t, int32_t, int32_t,
                      uint8_t, uint8_t, csplineInfo *),
  numerical_clone(int32_t, Symboltype),
  redef_array(int32_t, Symboltype, int32_t, int32_t *), string_scratch(int32_t),
  transferAll(int32_t symbol), transfer(int32_t), copySym(int32_t),
  scalar_scratch_copy(int32_t), redef_scalar(int32_t, Symboltype, void *),
  redef_string(int32_t, int32_t), float_arg_stat(int32_t, float *),
  Sprintf(char *, char *, ...), my_fprintf(FILE *, char *, ...),
  translateEscapes(char *),
  nextchar(FILE *), unnextchar(int32_t, FILE *);

int32_t approximately_equal(double, double, double),
  approximately_equal_f(float, float, float),
  essentially_equal(double, double, double),
  essentially_equal_f(float, float, float);
int32_t approximately_equal_z(doubleComplex, doubleComplex, double),
  approximately_equal_z_f(floatComplex, floatComplex, float),
  essentially_equal_z(doubleComplex, doubleComplex, double),
  essentially_equal_z_f(floatComplex, floatComplex, float);

int32_t         standardLoop(int32_t, int32_t, int32_t, Symboltype, LoopInfo *,
                     Pointer *, int32_t *, LoopInfo *, Pointer *),
  standardLoopX(int32_t, int32_t, int32_t, LoopInfo *, Pointer *, int32_t,
                int32_t const*, int32_t, int32_t const*, Symboltype, int32_t,
                int32_t*, LoopInfo*, Pointer*),
  standardLoop0(int32_t, int32_t, int32_t *, int32_t, Symboltype,
                LoopInfo *, Pointer *, int32_t *, LoopInfo *, Pointer *),
  nextLoops(LoopInfo *, LoopInfo *),
  prepareDiagonals(int32_t, LoopInfo *, int32_t, int32_t **, int32_t **, int32_t **, int32_t **),
  standardLoopX(int32_t, int32_t, int32_t, LoopInfo *, Pointer *, int32_t, int32_t const *,
                int32_t, int32_t const *, Symboltype, int32_t, int32_t *, LoopInfo *,
                Pointer *);

void    addVerify(char *, char),
  *seekFacts(int32_t symbol, int32_t type, int32_t flag),
  *setFacts(int32_t symbol, int32_t type, int32_t flag),
  deleteFacts(int32_t symbol, int32_t type), returnLoop(LoopInfo *, Pointer *, int32_t),
  standard_redef_array(int32_t, Symboltype, int32_t, int32_t *, int32_t,
                       int32_t *, int32_t, Pointer *, LoopInfo *);
void convertWidePointer(wideScalar *, int32_t, int32_t);

void    newStack(int32_t), push(int32_t), deleteStack(void);
int32_t         pop(void);

#if HAVE_LIBX11
int32_t         setup_x(void);
#endif

void    clearToPopTempVariable(int32_t), pushTempVariable(int32_t), printw(char const *),
  protect(int32_t *, int32_t), protectOne(int16_t), unProtect(int32_t *, int32_t),
  pushTempVariableIndex(void), checkErrno(void), updatIndices(void),
  zapTemp(int32_t), freeString(int32_t), unlinkString(int32_t), mark(int32_t),
  zapMarked(void), pegMark(void), unMark(int32_t),
  dupList(void), zerobytes(void *, int32_t), printwf(char const *, ...),
  cleanup_cubic_spline_tables(csplineInfo *),
  cspline_value_and_derivative(double, double *, double *, csplineInfo *),
  find_cspline_extremes(double, double, double *, double *,
                        double *, double *, csplineInfo *),
  undefine(int32_t),
  setPager(int32_t), resetPager(void), embed(int32_t, int32_t),
  convertPointer(Scalar *, Symboltype, Symboltype), zap(int32_t);

const csplineInfo empty_cubic_spline(void);

int setupDimensionLoop(LoopInfo *info, int32_t ndim, int32_t const *dims,
                        Symboltype type, int32_t naxes, int32_t const *axes,
                       Pointer *data, int32_t mode);

void endian(void *, int32_t, int32_t),
  rearrangeDimensionLoop(LoopInfo*),
  rearrangeEdgeLoop(LoopInfo *, LoopInfo *, int32_t);

float   float_arg(int32_t);
double  double_arg(int32_t), cspline_value(double, csplineInfo *),
        find_cspline_value(double, double, double, csplineInfo *),
        cspline_derivative(double, csplineInfo *),
  cspline_second_derivative(double, csplineInfo *), famod(double,double),
  fasmod(double,double), vhypot(int32_t, double, double, ...), hypota(int32_t, double *);
int32_t iamod(int32_t,int32_t), iasmod(int32_t,int32_t);
FILE    *openPathFile(char const *, int32_t);

Symboltype combinedType(Symboltype, Symboltype);

#define axisAxes(i)     (axisAxis? axisAxis[i]: (i))

#define debugout(msg)   printf("DEBUG - %s [%s, line %d]\n", (msg), __FILE__, __LINE__)
#define debugout1(fmt,arg)      printf("DEBUG - "); printf((fmt), (arg)); printf(" [%s, line %d]\n", __FILE__, __LINE__)

