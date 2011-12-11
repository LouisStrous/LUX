#include "error.h"
#include "anaparser.h"
/*#include "anaparser.c.tab.h"*/
#include "dmalloc.h"
#include "bindings.h"

extern char		expname[], line[], *curScrat, *currentRoutineName;
extern word		listStack[],  curContext;
extern int		scrat[], ana_file_open[], errorSym,
			internalMode, MSBfirst, suppressMsg;
extern int	ana_type_size[];

extern FILE		*inputStream, *ana_file[];
extern hashTableEntry	*varHashTable[], *subrHashTable[], *funcHashTable[],
			*blockHashTable[];
extern symTableEntry	sym[];
extern internalRoutine	*subroutine, *function;
extern int		nSubroutine, nFunction, curLineNumber, compileLevel,
			internalMode, ignoreInput, curSymbol, axisTally[];
extern struct boundsStruct	bounds;
extern int	(*ana_converts[10])(int, int []);

char 	*strsave(char *), *symbolIdent(int, int),
	*symName(int, hashTableEntry *[]), *string_arg(int),
	*expand_name(char *, char *), *className(int), *typeName(int),
	*symbolProperName(int), *strcasestr(char *, char *),
	*keyName(internalRoutine *routine, int number, int index),
	*strsave_system(char *), *nextline(char *, size_t, FILE *);

int 	dereferenceScalPointer(int), scalar_scratch(int),
  array_scratch(int, int, int []), int_arg(int), array_clone(int, int),
  convertRange(int), popTempVariable(int), int_arg_stat(int, int *),
  findSym(int, hashTableEntry *[], int), findInternalName(char *, int),
  ana_float(int, int []), ana_long(int, int []), ana_byte(int , int []),
  ana_word(int, int []), ana_double(int, int []), ana_cfloat(int, int []),
  anaerror(char *, int, ...), cerror(int, int, ...), copyEvalSym(int),
  getNumerical(int, int, int *, pointer *, char, int *, pointer *),
  strccmp(char *, char *), eval(int), evals(int), ana_cdouble(int, int []),
  strncasecmp_p(char *, char *, int), strcasecmp_p(char *, char *),
  stringpointer(char *, int), routineContext(int),
  getSimpleNumerical(int, pointer *, int *), double_arg_stat(int, double *),
  numerical(int, int **, int *, int *, pointer *),
  numerical_or_string(int, int **, int *, int *, pointer *),
  findName(char *, hashTableEntry **, int), to_scalar(int, int),
  to_scratch_array(int, int, int, int *), get_dims(int *, int *, int *),
  listNumElements(int), ana_zero(int, int []),
  cubic_spline_tables(void *, int, int, void *, int, int, int,
		      byte, csplineInfo *), 
  numerical_clone(int, enum Symboltype),
  redef_array(int, int, int, int *), string_scratch(int),
  transferAll(int symbol), transfer(int), copySym(int),
  scalar_scratch_copy(int), redef_scalar(int, int, void *),
  redef_string(int, int), float_arg_stat(int, float *),
  Sprintf(char *, char *, ...), translateEscapes(char *),
  nextchar(FILE *), unnextchar(int, FILE *);

int	standardLoop(int, int, int, int, loopInfo *, pointer *, int *,
		     loopInfo *, pointer *),
  standardLoop0(int, int, int *, int, int, loopInfo *, pointer *, int *,
	       loopInfo *, pointer *), advanceLoop(loopInfo *),
  advanceLoops(loopInfo *, loopInfo *), nextLoop(loopInfo *),
  nextLoops(loopInfo *, loopInfo *),
  prepareDiagonals(int, loopInfo *, int, int **, int **, int **, int **),
  moveLoop(loopInfo *, int, int),
  standardLoopX(int, int, int, loopInfo *, pointer *, int, int const *,
                int, int const *, enum Symboltype, int, int *, loopInfo *,
                pointer *),
  loopIsAtStart(loopInfo const *), 
  standard_args(int, int [], char const *, pointer **, loopInfo **);

void	subdataLoop(int *, loopInfo *), addVerify(char *, char),
  *seekFacts(int symbol, int type, int flag),
  *setFacts(int symbol, int type, int flag),
  deleteFacts(int symbol, int type), returnLoop(loopInfo *, int),
  setAxisMode(loopInfo *, int mode);

void	newStack(int), push(int), deleteStack(void);
int	pop(void);
        
#ifdef X11
int	setup_x(void);
#endif

void	clearToPopTempVariable(int), pushTempVariable(int), printw(char *),
  protect(int *, int), protectOne(word), unProtect(int *, int),
  pushTempVariableIndex(void), checkErrno(void), updatIndices(void),
  zapTemp(int), freeString(int), unlinkString(int), mark(int),
  zapMarked(void), pegMark(void), unMark(int),
  dupList(void), zerobytes(void *, int), printwf(char *, ...),
  cleanup_cubic_spline_tables(csplineInfo *),
  cspline_value_and_derivative(double, double *, double *, csplineInfo *),
  find_cspline_extremes(double, double, double *, double *,
			double *, double *, csplineInfo *),
  undefine(int),
  setPager(int), resetPager(void), embed(int, int),
  convertPointer(scalar *, int, int), zap(int);

const csplineInfo empty_cubic_spline(void);

void setupDimensionLoop(loopInfo *info, int ndim, int const *dims, 
                        enum Symboltype type, int naxes, int const *axes,
                        pointer *data, int mode),
  rearrangeDimensionLoop(loopInfo *), endian(void *, int, int),
  rearrangeEdgeLoop(loopInfo *, loopInfo *, int);

float	float_arg(int);
double	double_arg(int), cspline_value(double, csplineInfo *),
	find_cspline_value(double, double, double, csplineInfo *),
	cspline_derivative(double, csplineInfo *),
  cspline_second_derivative(double, csplineInfo *), famod(double,double),
  fasmod(double,double);
int iamod(int,int), iasmod(int,int);
FILE	*openPathFile(char *, int);

#define axisAxes(i)	(axisAxis? axisAxis[i]: i)

#define round(x)	((int) (floor(x + 0.5)))

#define debugout(msg)	printf("DEBUG - %s [%s, line %d]\n", msg, __FILE__, __LINE__)
#define debugout1(fmt,arg)	printf("DEBUG - "); printf(fmt, arg); printf(" [%s, line %d]\n", __FILE__, __LINE__)

#define ALLOCATE(tgt, num) tgt = calloc(num, sizeof(*tgt))
