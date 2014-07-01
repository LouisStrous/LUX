/* This is file action.h.

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
#define _GNU_SOURCE

#include "luxdefs.h"
#include "error.h"
#include "dmalloc.h"
#include "bindings.h"

extern char		expname[], line[], *curScrat, *currentRoutineName;
extern int16_t		listStack[],  curContext;
extern Int		scrat[], lux_file_open[], errorSym,
			MSBfirst, suppressMsg;
extern Int	lux_type_size[];

extern FILE		*inputStream, *lux_file[];
extern hashTableEntry	*varHashTable[], *subrHashTable[], *funcHashTable[],
			*blockHashTable[];
extern symTableEntry	sym[];
extern internalRoutine	*subroutine, *function;
extern Int		nSubroutine, nFunction, curLineNumber, compileLevel,
			ignoreInput, curSymbol, axisTally[];
extern unsigned int     internalMode;
extern struct boundsStruct	bounds;
extern Int	(*lux_converts[10])(Int, Int []);

char 	*strsave(char *), *symbolIdent(Int, Int),
	*symName(Int, hashTableEntry *[]), *string_arg(Int),
	*expand_name(char *, char *), *className(Int), *typeName(Int),
	*symbolProperName(Int),
	*keyName(internalRoutine *routine, Int number, Int index),
	*strsave_system(char *), *nextline(char *, size_t, FILE *);

Int 	dereferenceScalPointer(Int), scalar_scratch(Int),
  array_scratch(Int, Int, Int []), int_arg(Int), array_clone(Int, Int),
  convertRange(Int), popTempVariable(Int), int_arg_stat(Int, Int *),
  findSym(Int, hashTableEntry *[], Int), findInternalName(char *, Int),
  lux_float(Int, Int []), lux_long(Int, Int []), lux_byte(Int , Int []),
  lux_word(Int, Int []), lux_double(Int, Int []), lux_cfloat(Int, Int []),
  luxerror(char *, Int, ...), cerror(Int, Int, ...), copyEvalSym(Int),
  getNumerical(Int, Int, Int *, pointer *, char, Int *, pointer *),
  strccmp(char *, char *), eval(Int), evals(Int), lux_cdouble(Int, Int []),
  strncasecmp_p(char *, char *, Int), strcasecmp_p(char *, char *),
  stringpointer(char *, Int), routineContext(Int), lux_string(Int, Int []),
  getSimpleNumerical(Int, pointer *, Int *), double_arg_stat(Int, double *),
  numerical(Int, Int **, Int *, Int *, pointer *),
  numerical_or_string(Int, Int **, Int *, Int *, pointer *),
  findName(char *, hashTableEntry **, Int), to_scalar(Int, Int),
  to_scratch_array(Int, Int, Int, Int *), get_dims(Int *, Int *, Int *),
  listNumElements(Int), lux_zero(Int, Int []),
  cubic_spline_tables(void *, Int, Int, void *, Int, Int, Int,
		      uint8_t, uint8_t, csplineInfo *), 
  numerical_clone(Int, Symboltype),
  redef_array(Int, Int, Int, Int *), string_scratch(Int),
  transferAll(Int symbol), transfer(Int), copySym(Int),
  scalar_scratch_copy(Int), redef_scalar(Int, Int, void *),
  redef_string(Int, Int), float_arg_stat(Int, float *),
  Sprintf(char *, char *, ...), translateEscapes(char *),
  nextchar(FILE *), unnextchar(Int, FILE *);

Int	standardLoop(Int, Int, Int, Int, loopInfo *, pointer *, Int *,
		     loopInfo *, pointer *),
  standardLoop0(Int, Int, Int *, Int, Int, loopInfo *, pointer *, Int *,
		loopInfo *, pointer *), advanceLoop(loopInfo *, pointer *),
  nextLoop(loopInfo *),
  nextLoops(loopInfo *, loopInfo *),
  prepareDiagonals(Int, loopInfo *, Int, Int **, Int **, Int **, Int **),
  moveLoop(loopInfo *, Int, Int),
  standardLoopX(Int, Int, Int, loopInfo *, pointer *, Int, Int const *,
                Int, Int const *, Symboltype, Int, Int *, loopInfo *,
                pointer *),
  loopIsAtStart(loopInfo const *), setAxes(loopInfo *, Int, Int *, Int),
  standard_args(Int, Int [], char const *, pointer **, loopInfo **);

void	subdataLoop(Int *, loopInfo *), addVerify(char *, char),
  *seekFacts(Int symbol, Int type, Int flag),
  *setFacts(Int symbol, Int type, Int flag),
  deleteFacts(Int symbol, Int type), returnLoop(loopInfo *, pointer *, Int),
  setAxisMode(loopInfo *, Int mode),
  standard_redef_array(Int, Symboltype, Int, Int *, Int, Int *, pointer *,
		       loopInfo *);
void convertWidePointer(wideScalar *, Int, Int);

void	newStack(Int), push(Int), deleteStack(void);
Int	pop(void);
        
#if HAVE_LIBX11
Int	setup_x(void);
#endif

void	clearToPopTempVariable(Int), pushTempVariable(Int), printw(char *),
  protect(Int *, Int), protectOne(int16_t), unProtect(Int *, Int),
  pushTempVariableIndex(void), checkErrno(void), updatIndices(void),
  zapTemp(Int), freeString(Int), unlinkString(Int), mark(Int),
  zapMarked(void), pegMark(void), unMark(Int),
  dupList(void), zerobytes(void *, Int), printwf(char *, ...),
  cleanup_cubic_spline_tables(csplineInfo *),
  cspline_value_and_derivative(double, double *, double *, csplineInfo *),
  find_cspline_extremes(double, double, double *, double *,
			double *, double *, csplineInfo *),
  undefine(Int),
  setPager(Int), resetPager(void), embed(Int, Int),
  convertPointer(scalar *, Int, Int), zap(Int);

const csplineInfo empty_cubic_spline(void);

void setupDimensionLoop(loopInfo *info, Int ndim, Int const *dims, 
                        Symboltype type, Int naxes, Int const *axes,
                        pointer *data, Int mode),
  rearrangeDimensionLoop(loopInfo *), endian(void *, Int, Int),
  rearrangeEdgeLoop(loopInfo *, loopInfo *, Int);

float	float_arg(Int);
double	double_arg(Int), cspline_value(double, csplineInfo *),
	find_cspline_value(double, double, double, csplineInfo *),
	cspline_derivative(double, csplineInfo *),
  cspline_second_derivative(double, csplineInfo *), famod(double,double),
  fasmod(double,double), vhypot(Int, double, double, ...), hypota(Int, double *);
Int iamod(Int,Int), iasmod(Int,Int);
FILE	*openPathFile(char *, Int);

#define axisAxes(i)	(axisAxis? axisAxis[i]: i)

#define round(x)	((Int) (floor(x + 0.5)))

#define debugout(msg)	printf("DEBUG - %s [%s, line %d]\n", msg, __FILE__, __LINE__)
#define debugout1(fmt,arg)	printf("DEBUG - "); printf(fmt, arg); printf(" [%s, line %d]\n", __FILE__, __LINE__)

#define ALLOCATE(tgt, num) tgt = calloc(num, sizeof(*tgt))
