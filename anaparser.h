#ifndef anaparser_h
#define anaparser_h

#ifndef MAX_DIMS
#include "install.h"
#endif

#ifndef transfer_target
#include "symbols.h"
#endif

#include <stdio.h>
#include <stdarg.h>
#include <gsl/gsl_spline.h>
#include "output.h"

	/* tLine code */
#define TRANS_FIXED_STRING	1

	/* symbol classes */

enum Symbolclass { 
  ANA_UNUSED = 0,		/* 0: not used */
  ANA_SCALAR,			/* 1: real scalar 17 */
  ANA_STRING,			/* 2: string 'xx' */
  ANA_RANGE,			/* 3: range  (1:2) */
  ANA_ARRAY,			/* 4: numerical real or string array [1,2] */
  ANA_TRANSFER,			/* 5: transfer symbol */
  ANA_ASSOC,			/* 6: associated variable ASSOC */
  ANA_FUNC_PTR,			/* 7: function pointer &SIN */
  ANA_SCAL_PTR,			/* 8: scalar pointer !MXB */
  ANA_SUBSC_PTR,		/* 9: subscript pointer (1:2) */
  ANA_FILEMAP,			/* 10: file map FLTFARR */
  ANA_CLIST,			/* 11: compact list { } */
  ANA_LIST,			/* 12: list { } */
  ANA_STRUCT,			/* 13: structure { } */
  ANA_KEYWORD,			/* 14: keyword /FOO */
  ANA_LIST_PTR,			/* 15: list member pointer .TAG */
  ANA_PRE_RANGE,		/* 16: pre-range (1:3) */
  ANA_PRE_CLIST,		/* 17: pre-compact-list { } */
  ANA_PRE_LIST,			/* 18: pre-list { } */
  ANA_ENUM,			/* 19: enumeration */
  ANA_META,			/* 20: meta SYMBOL('string') */
  ANA_CSCALAR,			/* 21: complex scalar */
  ANA_CARRAY,			/* 22: complex array */
  ANA_CPLIST,			/* 23: compact pointer list */
  ANA_POINTER,			/* 24: pointer &X */
  ANA_STRUCT_PTR,		/* 25: pointer to member(s) of a structure */
  ANA_SUBROUTINE = 32,		/* 32: subroutine definition */
  ANA_FUNCTION,			/* 33: function definition */
  ANA_BLOCKROUTINE,		/* 34: block routine definition */
  ANA_DEFERRED_SUBR,		/* 35: deferred subroutine definition */
  ANA_DEFERRED_FUNC,		/* 36: deferred function definition */
  ANA_DEFERRED_BLOCK,		/* 37: deferred block routine */
  ANA_BIN_OP = 192,		/* 192: binary operation */
  ANA_INT_FUNC,			/* 193: internal function call */
  ANA_USR_FUNC,			/* 194: user-defined function call */
  ANA_IF_OP,			/* 195: binary if-operation */
  ANA_EXTRACT,			/* 196: extraction */
  ANA_PRE_EXTRACT,		/* 197: pre-extraction */
  ANA_EVB = 200,		/* 200: executable */
  ANA_FIXED_NUMBER,		/* 201: fixed number */
  ANA_FIXED_STRING,		/* 202: fixed string */
  ANA_UNDEFINED = 255		/* 255: undefined */
};

/* note: do not change the relative order of these classes!  newSymbol */
/* might not work properly if you do. */

	/* symbol types */
enum Symboltype {
  ANA_BYTE,			/* 0: 1-byte integers */
  ANA_WORD,			/* 1: 2-byte integers */
  ANA_LONG,			/* 2: 4-byte integers */
  ANA_FLOAT,			/* 3: 4-byte floats */
  ANA_DOUBLE,			/* 4: 8-byte floats */
  ANA_TEMP_STRING,		/* 5: temporary strings */
  ANA_LSTRING,			/* 6: literal strings */
  ANA_STRING_ARRAY,		/* 7: string arrays */
  ANA_CFLOAT,			/* 8: 8-byte complex floats */
  ANA_CDOUBLE,			/* 9: 16-byte complex floats */
  ANA_NO_SYMBOLTYPE,
};

	/* ANA_EVB classes */

enum EVBclasses {
  EVB_BLOCK = 1,		/* 1: statement block { } */
  EVB_REPLACE,			/* 2: assignment statement = */
  EVB_INT_SUB,			/* 3: call to internal subroutine */
  EVB_FOR,			/* 4: FOR statement */
  EVB_INSERT,			/* 5: insert statement */
  EVB_IF,			/* 6: IF statement */
  EVB_USR_SUB,			/* 7: call to user-defined subroutine */
  EVB_WHILE_DO,			/* 8: WHILE-DO statement */
  EVB_DO_WHILE,			/* 9: DO-WHILE statement */
  EVB_RETURN,			/* 10: RETURN statement */
  EVB_CASE,			/* 11: CASE statement */
  EVB_NCASE,			/* 12: NCASE statement */
  EVB_REPEAT,			/* 13: REPEAT statement */
  EVB_USR_CODE,			/* 14: RUN statement */
  EVB_FILE			/* 15: file inclusion @, @@ */
};

enum includeTypes {
  FILE_INCLUDE,			/* 0: @file */
  FILE_REPORT			/* 1: @@file */
};

/* special execution codes */
#define ANA_ERROR	-1	/* error state */
#define ANA_OK		1	/* OK state */
#define LOOP_BREAK	-2	/* BREAK state */
#define LOOP_CONTINUE	-3	/* CONTINUE state */
#define LOOP_RETALL	-4	/* RETALL state */
#define LOOP_RETURN	-5	/* RETURN state */

/* breakpoints */
#define BR_ENABLE	2	/* enable breakpoint */
#define BR_EXIST	1	/* create breakpoint */
#define BR_UNSET	0
#define BR_ENABLED	(BR_EXIST | BR_ENABLE)

/* bigendian/littleendian */
#if WORDS_BIGENDIAN
#define BIGENDIAN	1
#else
#define LITTLEENDIAN	1
#endif

/* format types */
enum fmtTypes {
  FMT_ERROR,			/* 0: illegal format */
  FMT_PLAIN,			/* 1: plain text format */
  FMT_INTEGER,			/* 2: integer format */
  FMT_FLOAT,			/* 3: float format */
  FMT_TIME,			/* 4: time format */
  FMT_DATE,			/* 5: date format */
  FMT_STRING,			/* 6: string format */
  FMT_COMPLEX,			/* 7: complex float format */
  FMT_EMPTY			/* 8: empty format */
};

/* binary operation classes */
/* don't change the order of these constants!  this order must
   correspond with the order in binOpName, binOpSign, binFunc* in
   eval.c, and the positions of ANA_EQ, ANA_OR, and ANA_POW relative
   to the (other) relational and logical operators must remain the
   same.  All between ANA_EQ and ANA_OR accept ANA_FLOAT operands, all
   between ANA_OR and ANA_POW do not. */

enum binaryOps {
  ANA_ADD,			/* 0: + */
  ANA_SUB,			/* 1: - */
  ANA_MUL,			/* 2: * */
  ANA_DIV,			/* 3: / */
  ANA_IDIV,			/* 4: \ */
  ANA_MOD,			/* 5: %, MOD */
  ANA_SMOD,			/* 6: SMOD */
  ANA_MAX,			/* 7: > */
  ANA_MIN,			/* 8: < */
  ANA_EQ,			/* 9: EQ */
  ANA_GT,			/* 10: GT */
  ANA_GE,			/* 11: GE */
  ANA_LT,			/* 12: LT */
  ANA_LE,			/* 13: LE */
  ANA_NE,			/* 14: NE */
  ANA_OR,			/* 15: OR */
  ANA_AND,			/* 16: AND */
  ANA_XOR,			/* 17: XOR */
  ANA_POW,			/* 18: ^ */
  ANA_ANDIF,			/* 19: ANDIF */
  ANA_ORIF			/* 20: ORIF */
};
#define NUM_BIN_OP	19	/* note that this number does not */
				/* count ANA_ANDIF and ANA_ORIF */

 /* boundingBoxType (PLOT,BOUNDINGBOX=) stuff */
#define BB_NONE		0	/* no bounding box */
#define BB_ALL		1	/* get everything */
#define BB_PLOT		2	/* plot window only */

 /* cleanUp() stuff */
#define CLEANUP_VARS	1
#define CLEANUP_EDBS	2
#define CLEANUP_COMP	4
#define CLEANUP_BOTH	3
#define CLEANUP_ALL	7
#define CLEANUP_ERROR	8

 /* special parser symbol numbers */
#define ANA_NEW_LIST	-32766	/* must be negative */
#define ANA_EXTEND	-32755
#define ANA_LIST_DELIM	-32754
#define ANA_ZERO		4
#define ANA_ONE		1
#define ANA_MINUS_ONE	2

#define ANA_UNSPECIFIED	-9999

 /* some special functions and routines */
#define ANA_NEG_FUN	0
#define ANA_SUBSC_FUN	1
#define ANA_CONCAT_FUN	4
#define ANA_POW_FUN	3

#define ANA_INSERT_SUB	0

 /* openPathFile options */
#define FIND_SUBR	0
#define FIND_FUNC	1
#define FIND_EITHER	2
#define FIND_LOWER	4

/* data file types */

#define FILE_TYPE_ANA_FZ	1 /* ana fz file */
#define FILE_TYPE_IDL_SAVE	2 /* IDL Save file */
#define FILE_TYPE_GIF		3 /* GIF file */
#define FILE_TYPE_ANA_ASTORE	4 /* ANA ASTORE file */
#define FILE_TYPE_JPEG		5 /* JPEG file */
#define FILE_TYPE_TIFF		6 /* TIFF file */
#define FILE_TYPE_FITS		7 /* FITS file */
#define FILE_TYPE_PPM_RAW	8
#define FILE_TYPE_PPM_ASCII	9
#define FILE_TYPE_XPM		10
#define FILE_TYPE_XBM		11 /* X11 bitmap */
#define FILE_TYPE_BMP		12
#define FILE_TYPE_SUN_RAS	13 /* Sun raster file */
#define FILE_TYPE_IRIS_RGB	14
#define FILE_TYPE_TARGA_24	15
#define FILE_TYPE_PM		16

/* symbol identification options */

#define I_FILELEVEL	((1<<0) | (1<<1))
#define I_LINE		(1<<1)
#define I_VALUE		(1<<2)
#define I_TRUNCATE	(1<<3)
#define I_PARENT	(1<<4)
#define I_NL		(1<<5)
#define I_ROUTINE	(1<<6)
#define I_LENGTH	(1<<7)
#define I_ALTERNATIVE	(1<<8)
#define I_SINGLEMODE	(~(I_PARENT | I_LINE | I_FILELEVEL))
#define I_SINGLEEXEC	(~(I_PARENT | I_LINE | I_FILELEVEL | I_VALUE))

/* traceMode definitions */
#define T_FILE		(1 << 0)
#define T_LOOP		(1 << 1)
#define T_BLOCK		(1 << 2)
#define T_ROUTINE	(1 << 3)
#define T_SHOWSTATS	(1 << 4)
#define T_CPUTIME	(1 << 5)
#define T_SHOWEXEC	(1 << 6)
#define T_ROUTINEIO	(1 << 7)

/* sprintf modes */
#define FMT_LEFT_JUSTIFY	(1 << 0) /* - modifier */
#define FMT_ALWAYS_SIGN		(1 << 1) /* + modifier */
#define FMT_ZERO_PAD		(1 << 2) /* 0 modifier */
#define FMT_ALTERNATIVE		(1 << 3) /* # modifier */
#define FMT_POSITIVE_BLANK	(1 << 4) /* ' ' modifier */
#define FMT_SUPPRESS		(1 << 5) /* * modifier */
#define FMT_BIG			(1 << 6) /* l modifier */
#define FMT_SMALL		(1 << 7) /* h modifier */
#define FMT_MIX			(1 << 8) /* _ modifier */
#define FMT_MIX2		(1 << 9) /* = modifier */

/* tv, tvraw modes */
#define TV_SCREEN	(1 << 6) /* 64 */
#define TV_POSTSCRIPT	(1 << 7) /* 128 */
#define TV_PDEV		(TV_SCREEN | TV_POSTSCRIPT) /* 192 */
#define TV_PLOTWINDOW	(1 << 8) /* 256 */
#define TV_ZOOM		(1 << 9) /* 512 */
#define TV_CENTER	(1 << 10) /* 1024 */
#define TV_SCALE	(1 << 11) /* 2048 */
#define TV_MAP		(1 << 12) /* 4096 */
#define TV_RAW		(1 << 13) /* 8192 */
#define TV_24		(1 << 14) /* 16384 */

/* for inspecting X11 visual classes (e.g., in color.c) */
#define visualIsRW(code)			(code % 2 == 1)
#define visualIsRO(code)			(code % 2 == 0)
#define visualIsGray(code)			(code < 3)
#define visualIsColor(code)			(code >= 3)
#define visualPrimariesAreLinked(code)		(code < 4)
#define visualPrimariesAreSeparate(code)	(code >= 4)
#define visualIsLegal(code)			(code >= 0 && code < 6)

#define PLOT_INFTY	(FLT_MAX/2)
/* PLOT_INFTY cannot be as large as FLT_MAX or internal errors in SYMPLOT */
/* occur  - LS 21mar94 */

#ifndef TWOPI
#define TWOPI	(2*M_PI)
#endif
#define DEG	(M_PI/180)
#define RAD	(180*M_1_PI)

/* coordinate systems */
#define ANA_DEP	0		/* ANA_DVI or ANA_DEV, depending on */
				/* coordinate magnitudes */
#define ANA_DVI	1		/* device-independent */
#define ANA_DEV	2		/* device-dependent (xport) */
#define ANA_IMG	3		/* image coordinates (tv) */
#define ANA_PLT	4		/* plot coordinates */
#define ANA_RIM	5		/* relative image coordinates */
#define ANA_RPL	6		/* relative plot coordinates */
#define ANA_X11	7		/* X coordinate system */
#define ANA_XLOG	64		/* logarithmic X */
#define ANA_YLOG	128		/* logarithmic Y */

/* debug breakpoint modes */
#define DEBUG_STEP	1

#define PROTECT_CONTEXT	SHRT_MAX
#define PROTECTED	(SHRT_MAX - 2000)
#define MAINTAIN	128	/* to mark symbols that may not be */
				/* overwritten */

/* getNumerical() stuff */
#define GN_UPDATE	1	/* update input symbol */
#define GN_UPGRADE	2	/* output at least <minType> */
#define GN_EXACT	4	/* output exactly <minType> */

/* startStandardLoop() stuff */
#define SL_SAMEDIMS	0	/* 0x0 output needs same dimensions as input */
#define SL_COMPRESS	1	/* 0x1 output lacks first dimension from axis */
#define SL_COMPRESSALL	2	/* 0x2 output lacks all dimensions from axis */

#define SL_EXACT	0*(1<<2) /* 0x0 output type is exactly as specified */
#define SL_UPGRADE	1*(1<<2) /* 0x4 output type is upgraded if necessary */
#define SL_KEEPTYPE	2*(1<<2) /* 0x8 output type equal to input type */

#define SL_EACHCOORD	0*(1<<4) /* 0x0 need all coordinates */
#define SL_AXISCOORD	(1<<4)	/* 0x10 need only coordinate along specified axis */
#define SL_EACHROW	(1<<5)	/* 0x20 treat row by row */
#define SL_UNIQUEAXES	(1<<6)	/* 0x40 all axes must be unique */
#define SL_ONEAXIS	(1<<7)	/* 0x80 at most one axis */
#define SL_AXESBLOCK	((1<<8) | SL_UNIQUEAXES) /* 0x140 treat selected axes
				   as a block, includes SL_UNIQUEAXES */
#define SL_ONEDIMS	(1<<9)	/* 0x200 "compressed" dimensions replaced by 1s */
#define SL_SRCUPGRADE	(1<<10)	/* 0x400 upgrade source data type if necessary */
#define SL_NEGONED	(1<<11)	/* 0x800 negative axis argument -> treat as 1D */
#define SL_EACHBLOCK	((1<<12) | SL_AXESBLOCK) /* 0x1140 treat all axes at once */
#define SL_ALLAXES	(1<<13)	/* 0x2000 select all axes for treatment */
#define SL_TAKEONED	(1<<14)	/* 0x4000 treat ax 1D array */

/* stringpointer() stuff */
#define SP_VAR		1
#define SP_USER_FUNC	2
#define SP_USER_SUBR	4
#define SP_INT_FUNC	8
#define SP_INT_SUBR	16
#define SP_FUNC		(SP_USER_FUNC + SP_INT_FUNC)
#define SP_SUBR		(SP_USER_SUBR + SP_INT_SUBR)
#define SP_ANY		(SP_VAR + SP_FUNC + SP_SUBR)

#define scratSize()	(NSCRAT*sizeof(int) + (curScrat - (char *) scrat))

	/* C auxilliary variable types */
typedef unsigned char		byte;
typedef unsigned short int	uword;
typedef signed short int	word;

typedef struct {
  float real; float imaginary;
} floatComplex;

typedef struct {
  double real; double imaginary;
} doubleComplex;

typedef union {
  floatComplex *f; doubleComplex *d;
} complexPointer;
  
typedef union {
  byte b; word w; int l; long int ll; float f; double d; char *s; char **sp;
} scalar;

/* wideScalar is equal to scalar plus the complex data types; we have */
/* separate scalar and wideScalars because wideScalar is wider, which is */
/* not always desirable. */
typedef union {
  byte b; word w; int l; long int ll; float f; double d; floatComplex cf;
  doubleComplex cd; char *s; char **sp;
} wideScalar;

typedef union pointerUnion {
  byte *b; word *w; int *l; long int *ll; float *f; double *d; char *s;
  char **sp; void *v; floatComplex *cf; doubleComplex *cd;
} pointer;

typedef struct {
  char *key; word value;
} listElem;

typedef struct {
  char *key; int value;
} enumElem;

typedef struct {
  char suppressEval; char pipe; char suppressUnused;
  int defaultMode; byte offset; char **keys;
} keyList;

/* kinds of facts */
enum {
  ANA_NO_FACT, ANA_STAT_FACTS
};

#define ANA_ANY_FACT		(0xffffffff)

#define ANA_STAT_FACTS_MINMAX	(1<<0)
#define ANA_STAT_FACTS_TOTAL	(1<<1)
#define ANA_STAT_FACTS_SDEV	(1<<2)

typedef struct {
  int min; int max; int minloc; int maxloc; double total; double sdev;
} statFacts_b;

typedef struct {
  word min; word max; int minloc; int maxloc; double total; double sdev;
} statFacts_w;

typedef struct {
  int min; int max; int minloc; int maxloc; double total; double sdev;
} statFacts_l;

typedef struct {
  float min; float max; int minloc; int maxloc; double total; double sdev;
} statFacts_f;

typedef struct {
  double min; double max; int minloc; int maxloc; double total; double sdev;
} statFacts_d;

typedef struct {
  floatComplex min; floatComplex max; int minloc; int maxloc;
  doubleComplex total; double sdev;
} statFacts_cf;

typedef struct {
  doubleComplex min; doubleComplex max; int minloc; int maxloc;
  doubleComplex total; double sdev;
} statFacts_cd;

typedef union {
  statFacts_b	*b;
  statFacts_w	*w;
  statFacts_l	*l;
  statFacts_f	*f;
  statFacts_d	*d;
  statFacts_cf	*cf;
  statFacts_cd	*cd;
  void	*any;
} allFacts;

typedef struct {
  byte	type;
  byte	flags;
  byte	pad[2];
  allFacts	fact;
} arrayFacts;

typedef struct arrayStruct {
  byte ndim, c1, c2, nfacts; int dims[MAX_DIMS]; arrayFacts *facts;
} array;

struct boundsStruct {
  struct { byte b; word w; int l; float f; double d; } min;
  struct { byte b; word w; int l; float f; double d; } max;
};

typedef struct structElemStruct {
  union {
    struct {
      int nelem; int size; int *dims; byte ndim;
    } first;
    struct {
      char *tag; unsigned int offset; byte type;
      union {
	struct {
	  int *dims; byte ndim;
	} singular;
	int member;
      } spec;
    } regular;
  } u;
} structElem;

typedef struct {
  byte type;
  uword number;
  union { word *w; char **sp; } ptr;
} extractSec;

typedef struct {
  byte	type;			/* subscript type: scalar, range, index */
  union {
    struct {
      int	value;		/* the single integer subscript */
    } scalar;
    struct {
      int	start;		/* the integer range start */
      int	end;		/* the integer range end */
    } range;
    struct {
      int	n_elem;		/* the number of index array elements */
      int	*ptr;		/* pointer to the index array elements */
    } array;
  } data;
} structPtrMember;

typedef struct structPtrStruct {
  int	desc;			/* index of structure descriptor */
  int	n_subsc;		/* number of subscripts */
  structPtrMember	*member;
} structPtr;
	
typedef struct {
  char	*name;
  extractSec	*extract;
} preExtract;

typedef struct symTableEntryStruct {
 byte class; byte type; word xx; int line; word context; int exec;
  union specUnion
  { scalar scalar;
    struct { array      *ptr; int bstore; } array;
    struct { word       *ptr; int bstore; } wlist;
    struct { uword	*ptr; int bstore; } uwlist;
    struct { enumElem   *ptr; int bstore; } enumElem;
    struct { char       *ptr; int bstore; } name;
    struct { listElem   *ptr; int bstore; } listElem;
    struct { int	*ptr; int bstore; } intList;
    struct { extractSec	*ptr; int bstore; } extract;
    struct { preExtract	*ptr; int bstore; } preExtract;
    struct { void	*ptr; int bstore; } general;
    struct { structPtr	*ptr; int bstore; } structPtr;
    pointer	pointer;  
    struct { word args[4]; } evb;
    struct { uword args[4]; } uevb;
    struct { byte narg; char **keys; byte extend; uword nstmnt;
      word *ptr; } routine;
  } spec;
} symTableEntry;

typedef struct hashTableEntryStruct {
  char *name; int symNum; struct hashTableEntryStruct *next;
} hashTableEntry;

typedef struct internalRoutineStruct {
  char *name; word minArg; word maxArg; int (*ptr)(int, int []); void *keys;
} internalRoutine;

typedef struct {
 int	synch_pattern;
 byte	subf, source, nhb, datyp, ndim, free1, cbytes[4], free[178];
 int	dim[16];
 char	txt[256];
} fzHead;

typedef struct {
  int symbol;  char kind; char mode;
} debugItem;

typedef struct {
  int depth, symbol, size;  char containLHS;
} branchInfo;

typedef struct {
  gsl_spline *spline; gsl_interp_accel *acc;
} csplineInfo;

typedef struct {
  pointer *data;		/* data pointer pointer */
  void *data0;			/* pointer to start of data */
  int coords[MAX_DIMS];		/* current (rearranged) coordinates */
  int singlestep[MAX_DIMS];	/* step size per coordinate */
  int step[MAX_DIMS];		/* combined step size for loop transfer */
  int dims[MAX_DIMS];		/* original dimensions */
  int nelem;			/* number of elements */
  int ndim;			/* number of original dimensions */
  int axes[MAX_DIMS];		/* selected axes */
  int naxes;			/* selected number of axes */
  int rdims[MAX_DIMS];		/* compressed rearranged dimensions */
  int rndim;			/* number of compressed rearranged dims */
  int rsinglestep[MAX_DIMS];	/* step size per rearranged coordinate */
  int axisindex;		/* index to current axis (in axes[]) */
  int mode;			/* desired treatment modes */
  int stride;			/* bytes per data element */
  enum Symboltype type;         /* data type */
  int advanceaxis;		/* how many axes not to advance (from start) */
  int raxes[MAX_DIMS];		/* from rearranged to old axes */
  int iraxes[MAX_DIMS];		/* from old to rearranged axes */
} loopInfo;

/* for nextCompileLevel: */
typedef struct compileInfoStruct {
  char	*line;
  int (*charfunc)(void);
  char	*name;
  FILE	*stream;
  int	line_number;
  struct compileInfoStruct	*next;
  struct compileInfoStruct	*prev;
} compileInfo;

/* for execution nesting info: */
typedef struct executionLevelInfoStruct {
  int	target;
  int	line;
} executionLevelInfo;

typedef struct {
  char	*format;		/* whole format string */
  char	*current;		/* the currently selected format entry */
  char	*start;			/* initial % of current format entry */
  char	*spec_char;		/* current specification character */
  char	*repeat;		/* start of repeat count */
  char	*plain;			/* start of plain text? */
  char	*end;			/* end of current format entry */
  char	*next;			/* start of next format entry */
  int	type;			/* format type */
  int	width;			/* format width */
  int	precision;		/* format precision */
  int	flags;			/* format modification flags */
  int	count;			/* format repetition count */
  int	active_group;		/* number of currently active groups */
  int	group_count[MAXFMT];	/* current group counts */
  char	*group_start[MAXFMT];	/* current group starts */
  char	save1;			/* for temporary storage  */
  char	save2;			/* for temporary storage */
  char  only_whitespace;	/* whitespace only? */
} formatInfo;

/* for breakpoints: */
typedef struct {
  int	line;
  char	*name;
  char	status;
} breakpointInfo;
#define BP_DEFINED	1
#define BP_ENABLED	2
#define BP_VARIABLE	4


/* for Dick's stuff */
#define types_ptr	pointerUnion
#define ahead		arrayStruct
#define sym_desc	symTableEntryStruct
#define class8_to_1(x)	dereferenceScalPointer(x)

#define symbol_ident_single(x,y)	what(x,y)
int	nextFreeTempVariable(void), nextFreeNamedVariable(void),
	nextFreeExecutable(void), nextFreeTempExecutable(void),
	dereferenceScalPointer(int), findSym(int, hashTableEntry *[], int),
	findInternalName(char *, int), anaerror(char *, int, ...),
	lookForName(char *, hashTableEntry *[], int), execute(int);
char	*symName(int, hashTableEntry *[]);

#define getFreeTempVariable(a)\
	{ if ((a = nextFreeTempVariable()) < 0) return a; }
#define getFreeNamedVariable(a)\
	{ if ((a = nextFreeNamedVariable()) < 0) return a; }
#define getFreeExecutable(a)\
	{ if ((a = nextFreeExecutable()) < 0) return a; }
#define getFreeTempExecutable(a)\
	{ if ((a = nextFreeTempExecutable()) < 0) return a; }
#define isTemp(a)	(a >= TEMPS_START && a < TEMPS_END)
#define isFreeTemp(a)	(isTemp(a) && symbol_context(a) == -compileLevel)
#define scalPointer(a)	{ if (sym[a].class == ANA_SCAL_PTR)\
			  a = dereferenceScalPointer(a); }
#define findVar(index, context)	findSym(index, varHashTable, context)
#define findSubr(index)		findSym(index, subrHashTable, 0)
#define findFunc(index)		findSym(index, funcHashTable, 0)
#define findBlock(index)	findSym(index, blockHashTable, 0)
extern char *symbolStack[];
#define findInternalSym(index, a) findInternalName(symbolStack[index], a)
#define findInternalSubr(index)	findInternalSym(index, 1)
#define findInternalFunc(index)	findInternalSym(index, 0)
#define lookForSym(indx, table, context) lookForName(symbolStack[indx], table, context)
#define lookForVar(index, context) lookForSym(index, varHashTable, context)
#define lookForVarName(name, context) lookForName(name, varHashTable, context)
#define findVarName(name, context) findName(name, varHashTable, context)
#define lookForSubr(index)	lookForSym(index, subrHashTable, 0)
#define lookForFunc(index)	lookForSym(index, funcHashTable, 0)
#define lookForBlock(index)	lookForSym(index, blockHashTable, 0)
#define lookForSubrName(name)	lookForName(name, subrHashTable, 0)
#define lookForFuncName(name)	lookForName(name, funcHashTable, 0)
#define lookForBlockName(name)	lookForName(name, blockHashTable, 0)
#define varName(index)		symName(index, varHashTable)
#define subrName(index)		symName(index, subrHashTable)
#define funcName(index)		symName(index, funcHashTable)
#define blockName(index)	symName(index, blockHashTable)
#define doit(symbol)		if ((n = execute(symbol)) < 0) return n
#define restore			curContext = oldContext
#define readGhost(fp, value, index, size)\
	{ if (fseek(fp, index*(size), SEEK_SET))\
	  return cerror(POS_ERR, 0); \
        if (fread(&(value), size, 1, fp) != 1)\
          return cerror(READ_ERR, 0); }

#undef MIN
#define MIN(A,B)		((A < B)? A: B)
#undef MAX
#define MAX(A,B)		((A > B)? A: B)
#undef ABS
#define ABS(A)			(((A) >= 0)? (A): -(A))
#define HEAD(SYM)		((array *) sym[SYM].spec.array.ptr)
#define LPTR(HEAD)		((int *)((char *) HEAD + sizeof(array)))
#define CK_ARR(SYM, ARGN)	if (sym[SYM].class != ANA_ARRAY)\
				return cerror(NEED_ARR, SYM)
#define CK_SGN(ARR, N, ARGN, SYM)\
	for (i = 0; i < N; i++) \
	  if (ARR[i] < 0) return cerror(DIM_SMALL, SYM)
#define CK_MAG(MAG, ARR, N, ARGN, SYM) \
  for (i = 0; i < N; i++) \
    if (ARR[i] >= MAG) return cerror(ILL_DIM, ARGN, SYM)
#define GET_SIZE(SIZ, ARR, N) SIZ = 1; for (i = 0; i < (int) N; i++)\
			 SIZ *= ARR[i]
#define N_ELEM(N) ((sym[N].spec.array.bstore - sizeof(array))\
                   /anaTypeSize[sym[N].type])

#define allocate(arg, size, type)\
   if (!(size)) arg = NULL; \
   else { if (!(arg = (void *) malloc((size)*sizeof(type))))\
   return anaerror("Memory allocation error\n", 0); }
#define eallocate(arg, size, type)\
 (size? ((arg = (type *) Malloc((size)*sizeof(type)))? 1: 0): (arg = NULL, 0))
#define GET_NUMERICAL(PTR, SIZE) \
  switch (sym[iq].class) \
  { case ANA_SCALAR:    PTR.l = &sym[iq].spec.scalar.l;  SIZE = 1;  break;  \
    case ANA_ARRAY:     h = HEAD(iq);  PTR.l = LPTR(h);  GET_SIZE(SIZE, h->dims, h->ndim);  \
      break; \
    default:        return cerror(ILL_CLASS, iq); }

	/* multi-type macro */
#define multiSwitch2(type, first, second) \
 switch (type) \
 { case ANA_BYTE:   first .b second ; break; \
   case ANA_WORD:   first .w second ; break; \
   case ANA_LONG:   first .l second ; break; \
   case ANA_FLOAT:  first .f second ; break; \
   case ANA_DOUBLE: first .d second ; break; }

#define multiSwitch3(type, first, second, third) \
 switch (type) \
 { case ANA_BYTE:   first .b second .b third ; break; \
   case ANA_WORD:   first .w second .w third ; break; \
   case ANA_LONG:   first .l second .l third ; break; \
   case ANA_FLOAT:  first .f second .f third ; break; \
   case ANA_DOUBLE: first .d second .d third ; break; }

#define multiSwitch4(type, first, second, third, fourth) \
 switch (type) \
 { case ANA_BYTE:   first .b second .b third .b fourth ; break; \
   case ANA_WORD:   first .w second .w third .b fourth ; break; \
   case ANA_LONG:   first .l second .l third .b fourth ; break; \
   case ANA_FLOAT:  first .f second .f third .b fourth ; break; \
   case ANA_DOUBLE: first .d second .d third .b fourth ; break; }

#define multiSwitch5(type, first, second, third, fourth, fifth) \
 switch (type) \
 { case ANA_BYTE:   first .b second .b third .b fourth .b fifth ; break; \
   case ANA_WORD:   first .w second .w third .b fourth .b fifth ; break; \
   case ANA_LONG:   first .l second .l third .b fourth .b fifth ; break; \
   case ANA_FLOAT:  first .f second .f third .b fourth .b fifth ; break; \
   case ANA_DOUBLE: first .d second .d third .b fourth .b fifth ; break; }

#endif
