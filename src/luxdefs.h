/* This is file luxdefs.h.

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
#ifndef INCLUDED_LUXDEFS_H
#define INCLUDED_LUXDEFS_H

/** \file luxdefs.h Definitions of enumerations and fixed compiler
    constants. */

#ifndef MAX_DIMS
#include "install.h"
#endif

#include "types.h"

#include <stdio.h>
#include <stdarg.h>
#include <gsl/gsl_spline.h>

	/* tLine code */
#define TRANS_FIXED_STRING	1

	/* symbol classes */

/** \enum Symbolclass LUX symbol classes. */
enum Symbolclass { 
  LUX_UNUSED = 0,		/*!< (0) symbol with no assigned class */
  LUX_SCALAR,			/*!< (1) a real scalar, e.g. \c 17 */
  LUX_STRING,			/*!< (2) a text string, e.g. \c 'x' */
  LUX_RANGE,			/*!< (3) a range, e.g. \c (1:2) */
  LUX_ARRAY,			/*!< (4) a numerical real or string array, e.g. \c [1,2] */
  LUX_TRANSFER,			/*!< (5) a transfer symbol, a symbol that points at another symbol */
  LUX_ASSOC,			/*!< (6) an associated variable (\c ASSOC) */
  LUX_FUNC_PTR,			/*!< (7) a function pointer, e.g. \c &SIN */
  LUX_SCAL_PTR,			/*!< (8) a scalar pointer, e.g. \c !MXB */
  LUX_SUBSC_PTR,		/*!< (9) a subscript pointer, e.g. \c (1:2) */
  LUX_FILEMAP,			/*!< (10) a file map (e.g., \c FLTFARR) */
  LUX_CLIST,			/*!< (11) a compact list (e.g., \c {}) */
  LUX_LIST,			/*!< (12) a list (e.g., \c {}) */
  LUX_STRUCT,			/*!< (13) a structure (e.g., \c {}) */
  LUX_KEYWORD,			/*!< (14) a keyword (e.g., \c /FOO) */
  LUX_LIST_PTR,			/*!< (15) a list member pointer (e.g., \c .TAG) */
  LUX_PRE_RANGE,		/*!< (16) a pre-range (e.g., \c (1:3)) */
  LUX_PRE_CLIST,		/*!< (17) a pre-compact-list (\c {}) */
  LUX_PRE_LIST,			/*!< (18) a pre-list (\c {}) */
  LUX_ENUM,			/*!< (19) an enumeration */
  LUX_META,			/*!< (20) a meta (e.g., \c SYMBOL('string')) */
  LUX_CSCALAR,			/*!< (21) a complex scalar */
  LUX_CARRAY,			/*!< (22) a complex array */
  LUX_CPLIST,			/*!< (23) a compact pointer list */
  LUX_POINTER,			/*!< (24) a pointer (e.g., \c &X) */
  LUX_STRUCT_PTR,		/*!< (25) a pointer to member(s) of a structure */
  LUX_SUBROUTINE = 32,		/*!< (32) a subroutine definition */
  LUX_FUNCTION,			/*!< (33) a function definition */
  LUX_BLOCKROUTINE,		/*!< (34) a block routine definition */
  LUX_DEFERRED_SUBR,		/*!< (35) a deferred subroutine definition */
  LUX_DEFERRED_FUNC,		/*!< (36) a deferred function definition */
  LUX_DEFERRED_BLOCK,		/*!< (37) a deferred block routine */
  LUX_BIN_OP = 192,		/*!< (192) a binary operation */
  LUX_INT_FUNC,			/*!< (193) an internal function call */
  LUX_USR_FUNC,			/*!< (194) a user-defined function call */
  LUX_IF_OP,			/*!< (195) a binary if-operation */
  LUX_EXTRACT,			/*!< (196) an extraction operation */
  LUX_PRE_EXTRACT,		/*!< (197) a pre-extraction operation */
  LUX_EVB = 200,		/*!< (200) an executable */
  LUX_FIXED_NUMBER,		/*!< (201) a fixed number */
  LUX_FIXED_STRING,		/*!< (202) a fixed string */
  LUX_UNDEFINED = 255		/*!< (255) an undefined symbol */
};

/* note: do not change the relative order of these classes!  newSymbol */
/* might not work properly if you do. */

	/* symbol types */
enum Symboltype {
  LUX_BYTE,			/* 0: 1-Byte integers */
  LUX_WORD,			/* 1: 2-Byte integers */
  LUX_LONG,			/* 2: 4-Byte integers */
  LUX_FLOAT,			/* 3: 4-Byte floats */
  LUX_DOUBLE,			/* 4: 8-Byte floats */
  LUX_TEMP_STRING,		/* 5: temporary strings */
  LUX_LSTRING,			/* 6: literal strings */
  LUX_STRING_ARRAY,		/* 7: string arrays */
  LUX_CFLOAT,			/* 8: 8-Byte complex floats */
  LUX_CDOUBLE,			/* 9: 16-Byte complex floats */
  LUX_NO_SYMBOLTYPE,
};

	/* LUX_EVB classes */

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
#define LUX_ERROR	-1	/* error state */
#define LUX_OK		1	/* OK state */
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
  FMT_FLOAT,			/* 3: Float format */
  FMT_TIME,			/* 4: time format */
  FMT_DATE,			/* 5: date format */
  FMT_STRING,			/* 6: string format */
  FMT_COMPLEX,			/* 7: complex Float format */
  FMT_EMPTY			/* 8: empty format */
};

/* binary operation classes */
/* don't change the order of these constants!  this order must
   correspond with the order in binOpName, binOpSign, binFunc* in
   eval.c, and the positions of LUX_EQ, LUX_OR, and LUX_POW relative
   to the (other) relational and logical operators must remain the
   same.  All between LUX_EQ and LUX_OR accept LUX_FLOAT operands, all
   between LUX_OR and LUX_POW do not. */

enum binaryOps {
  LUX_ADD,			/* 0: + */
  LUX_SUB,			/* 1: - */
  LUX_MUL,			/* 2: * */
  LUX_DIV,			/* 3: / */
  LUX_IDIV,			/* 4: \ */
  LUX_MOD,			/* 5: %, MOD */
  LUX_SMOD,			/* 6: SMOD */
  LUX_MAX,			/* 7: > */
  LUX_MIN,			/* 8: < */
  LUX_EQ,			/* 9: EQ */
  LUX_GT,			/* 10: GT */
  LUX_GE,			/* 11: GE */
  LUX_LT,			/* 12: LT */
  LUX_LE,			/* 13: LE */
  LUX_NE,			/* 14: NE */
  LUX_OR,			/* 15: OR */
  LUX_AND,			/* 16: AND */
  LUX_XOR,			/* 17: XOR */
  LUX_POW,			/* 18: ^ */
  LUX_ANDIF,			/* 19: ANDIF */
  LUX_ORIF			/* 20: ORIF */
};
#define NUM_BIN_OP	19	/* note that this number does not */
				/* count LUX_ANDIF and LUX_ORIF */

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
#define LUX_NEW_LIST	-32766	/* must be negative */
#define LUX_EXTEND	-32755
#define LUX_LIST_DELIM	-32754
#define LUX_ZERO		4
#define LUX_ONE		1
#define LUX_MINUS_ONE	2

#define LUX_UNSPECIFIED	-9999

 /* some special functions and routines */
#define LUX_NEG_FUN	0
#define LUX_SUBSC_FUN	1
#define LUX_CONCAT_FUN	4
#define LUX_POW_FUN	3

#define LUX_INSERT_SUB	0

 /* openPathFile options */
#define FIND_SUBR	0
#define FIND_FUNC	1
#define FIND_EITHER	2
#define FIND_LOWER	4

/* data file types */

#define FILE_TYPE_ANA_FZ	1 /* lux fz file */
#define FILE_TYPE_IDL_SAVE	2 /* IDL Save file */
#define FILE_TYPE_GIF		3 /* GIF file */
#define FILE_TYPE_ANA_ASTORE	4 /* LUX ASTORE file */
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
#define LUX_DEP	0		/* LUX_DVI or LUX_DEV, depending on */
				/* coordinate magnitudes */
#define LUX_DVI	1		/* device-independent */
#define LUX_DEV	2		/* device-dependent (xport) */
#define LUX_IMG	3		/* image coordinates (tv) */
#define LUX_PLT	4		/* plot coordinates */
#define LUX_RIM	5		/* relative image coordinates */
#define LUX_RPL	6		/* relative plot coordinates */
#define LUX_X11	7		/* X coordinate system */
#define LUX_XLOG	64		/* logarithmic X */
#define LUX_YLOG	128		/* logarithmic Y */

/* debug breakpoint modes */
#define DEBUG_STEP	1

#define PROTECT_CONTEXT	INT16_MAX
#define PROTECTED	(INT16_MAX - 2000)
#define MAINTAIN	128	/* to mark symbols that may not be */
				/* overwritten */

/* getNumerical() stuff */
#define GN_UPDATE	1	/* update input symbol */
#define GN_UPGRADE	2	/* output at least <minType> */
#define GN_EXACT	4	/* output exactly <minType> */

/* startStandardLoop() stuff */
#define SL_SAMEDIMS	0	/*!< 0x0 output needs same dimensions as input */
#define SL_COMPRESS	1	/*!< 0x1 output lacks first dimension from axis */
#define SL_COMPRESSALL	2	/*!< 0x2 output lacks all dimensions from axis */

#define SL_EXACT	0*(1<<2) /*!< 0x0 output type is exactly as specified */
#define SL_UPGRADE	1*(1<<2) /*!< 0x4 output type is upgraded if necessary */
#define SL_KEEPTYPE	2*(1<<2) /*!< 0x8 output type equal to input type */

#define SL_EACHCOORD	0*(1<<4) /*!< 0x0 need all coordinates */
#define SL_AXISCOORD	(1<<4)	/*!< 0x10 need only coordinate along specified axis */
#define SL_EACHROW	(1<<5)	/*!< 0x20 treat row by row */
#define SL_UNIQUEAXES	(1<<6)	/*!< 0x40 all axes must be unique */
#define SL_ONEAXIS	(1<<7)	/*!< 0x80 at most one axis */
#define SL_AXESBLOCK	((1<<8) | SL_UNIQUEAXES) /*!< 0x140 treat selected axes
				   as a block, includes SL_UNIQUEAXES */
#define SL_ONEDIMS	(1<<9)	/*!< 0x200 "compressed" dimensions replaced by 1s */
#define SL_SRCUPGRADE	(1<<10)	/*!< 0x400 upgrade source data type if necessary */
#define SL_NEGONED	(1<<11)	/*!< 0x800 negative axis argument -> treat as 1D */
#define SL_EACHBLOCK	((1<<12) | SL_AXESBLOCK) /*!< 0x1140 treat all axes at once */
#define SL_ALLAXES	(1<<13)	/*!< 0x2000 select all axes for treatment */
#define SL_TAKEONED	(1<<14)	/*!< 0x4000 treat ax 1D array */

/* stringpointer() stuff */
#define SP_VAR		1
#define SP_USER_FUNC	2
#define SP_USER_SUBR	4
#define SP_INT_FUNC	8
#define SP_INT_SUBR	16
#define SP_FUNC		(SP_USER_FUNC + SP_INT_FUNC)
#define SP_SUBR		(SP_USER_SUBR + SP_INT_SUBR)
#define SP_ANY		(SP_VAR + SP_FUNC + SP_SUBR)

#define scratSize()	(NSCRAT*sizeof(Int) + (curScrat - (char *) scrat))

typedef struct {
  Float real; Float imaginary;
} floatComplex;

typedef struct {
  Double real; Double imaginary;
} doubleComplex;

typedef union {
  floatComplex *f; doubleComplex *d;
} complexPointer;
  
typedef union {
  Byte b; Word w; Int l; Float f; Double d; char *s; char **sp;
} scalar;

/* wideScalar is equal to scalar plus the complex data types; we have */
/* separate scalar and wideScalars because wideScalar is wider, which is */
/* not always desirable. */
typedef union {
  Byte b; Word w; Int l; Float f; Double d; floatComplex cf;
  doubleComplex cd; char *s; char **sp;
} wideScalar;

typedef union pointerUnion {
  Byte *b; Word *w; Int *l; Float *f; Double *d; char *s;
  char **sp; void *v; floatComplex *cf; doubleComplex *cd;
} pointer;

typedef struct {
  char *key; Word value;
} listElem;

typedef struct {
  char *key; Int value;
} enumElem;

typedef struct {
  char suppressEval; char pipe; char suppressUnused;
  Int defaultMode; Byte offset; char **keys;
} keyList;

/* kinds of facts */
enum {
  LUX_NO_FACT, LUX_STAT_FACTS
};

#define LUX_ANY_FACT		(0xffffffff)

#define LUX_STAT_FACTS_MINMAX	(1<<0)
#define LUX_STAT_FACTS_TOTAL	(1<<1)
#define LUX_STAT_FACTS_SDEV	(1<<2)

typedef struct {
  Int min; Int max; Int minloc; Int maxloc; Double total; Double sdev;
} statFacts_b;

typedef struct {
  Word min; Word max; Int minloc; Int maxloc; Double total; Double sdev;
} statFacts_w;

typedef struct {
  Int min; Int max; Int minloc; Int maxloc; Double total; Double sdev;
} statFacts_l;

typedef struct {
  Float min; Float max; Int minloc; Int maxloc; Double total; Double sdev;
} statFacts_f;

typedef struct {
  Double min; Double max; Int minloc; Int maxloc; Double total; Double sdev;
} statFacts_d;

typedef struct {
  floatComplex min; floatComplex max; Int minloc; Int maxloc;
  doubleComplex total; Double sdev;
} statFacts_cf;

typedef struct {
  doubleComplex min; doubleComplex max; Int minloc; Int maxloc;
  doubleComplex total; Double sdev;
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
  Byte	type;
  Byte	flags;
  Byte	pad[2];
  allFacts	fact;
} arrayFacts;

typedef struct arrayStruct {
  Byte ndim, c1, c2, nfacts; Int dims[MAX_DIMS]; arrayFacts *facts;
} array;

struct boundsStruct {
  struct { Byte b; Word w; Int l; Float f; Double d; } min;
  struct { Byte b; Word w; Int l; Float f; Double d; } max;
};

typedef struct structElemStruct {
  union {
    struct {
      Int nelem; Int size; Int *dims; Byte ndim;
    } first;
    struct {
      char *tag; off_t offset; Byte type;
      union {
	struct {
	  Int *dims; Byte ndim;
	} singular;
	Int member;
      } spec;
    } regular;
  } u;
} structElem;

typedef struct {
  Byte type;
  uWord number;
  union { Word *w; char **sp; } ptr;
} extractSec;

typedef struct {
  Byte	type;			/* subscript type: scalar, range, index */
  union {
    struct {
      Int	value;		/* the single integer subscript */
    } scalar;
    struct {
      Int	start;		/* the integer range start */
      Int	end;		/* the integer range end */
    } range;
    struct {
      Int	n_elem;		/* the number of index array elements */
      Int	*ptr;		/* pointer to the index array elements */
    } array;
  } data;
} structPtrMember;

typedef struct structPtrStruct {
  Int	desc;			/* index of structure descriptor */
  Int	n_subsc;		/* number of subscripts */
  structPtrMember	*member;
} structPtr;
	
typedef struct {
  char	*name;
  extractSec	*extract;
} preExtract;

typedef struct symTableEntryStruct {
 Byte class; Byte type; Word xx; Int line; Word context; Int exec;
  union specUnion
  { scalar scalar;
    struct { array      *ptr; Int bstore; } array;
    struct { Word       *ptr; Int bstore; } wlist;
    struct { uWord	*ptr; Int bstore; } uwlist;
    struct { enumElem   *ptr; Int bstore; } enumElem;
    struct { char       *ptr; Int bstore; } name;
    struct { listElem   *ptr; Int bstore; } listElem;
    struct { Int	*ptr; Int bstore; } intList;
    struct { extractSec	*ptr; Int bstore; } extract;
    struct { preExtract	*ptr; Int bstore; } preExtract;
    struct { void	*ptr; Int bstore; } general;
    struct { structPtr	*ptr; Int bstore; } structPtr;
    pointer	pointer;  
    struct { Word args[4]; } evb;
    struct { uWord args[4]; } uevb;
    struct { Byte narg; char **keys; Byte extend; uWord nstmnt;
      Word *ptr; } routine;
  } spec;
} symTableEntry;

typedef struct hashTableEntryStruct {
  char *name; Int symNum; struct hashTableEntryStruct *next;
} hashTableEntry;

typedef struct internalRoutineStruct {
  char *name; Word minArg; Word maxArg; Int (*ptr)(Int, Int []); void *keys;
} internalRoutine;

typedef struct {
 Int	synch_pattern;
 Byte	subf, source, nhb, datyp, ndim, free1, cbytes[4], free[178];
 Int	dim[16];
 char	txt[256];
} fzHead;

typedef struct {
  Int symbol;  char kind; char mode;
} debugItem;

typedef struct {
  Int depth, symbol, size;  char containLHS;
} branchInfo;

typedef struct {
  gsl_spline *spline; gsl_interp_accel *acc; Double *x; Double *y;
} csplineInfo;

typedef struct {
  pointer *data;		/* data pointer pointer */
  void *data0;			/* pointer to start of data */
  Int coords[MAX_DIMS];		/* current (rearranged) coordinates */
  Int singlestep[MAX_DIMS];	/* step size per coordinate */
  Int step[MAX_DIMS];		/* combined step size for loop transfer */
  Int dims[MAX_DIMS];		/* original dimensions */
  Int nelem;			/* number of elements */
  Int ndim;			/* number of original dimensions */
  Int axes[MAX_DIMS];		/* selected axes */
  Int naxes;			/* selected number of axes */
  Int rdims[MAX_DIMS];		/* compressed rearranged dimensions */
  Int rndim;			/* number of compressed rearranged dims */
  Int rsinglestep[MAX_DIMS];	/* step size per rearranged coordinate */
  Int axisindex;		/* index to current axis (in axes[]) */
  Int mode;			/* desired treatment modes */
  Int stride;			/* bytes per data element */
  enum Symboltype type;         /* data type */
  Int advanceaxis;		/* how many axes not to advance (from start) */
  Int raxes[MAX_DIMS];		/* from rearranged to old axes */
  Int iraxes[MAX_DIMS];		/* from old to rearranged axes */
} loopInfo;

/* for nextCompileLevel: */
typedef struct compileInfoStruct {
  char	*line;
  Int (*charfunc)(void);
  char	*name;
  FILE	*stream;
  Int	line_number;
  struct compileInfoStruct	*next;
  struct compileInfoStruct	*prev;
} compileInfo;

/* for execution nesting info: */
typedef struct executionLevelInfoStruct {
  Int	target;
  Int	line;
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
  Int	type;			/* format type */
  Int	width;			/* format width */
  Int	precision;		/* format precision */
  Int	flags;			/* format modification flags */
  Int	count;			/* format repetition count */
  Int	active_group;		/* number of currently active groups */
  Int	group_count[MAXFMT];	/* current group counts */
  char	*group_start[MAXFMT];	/* current group starts */
  char	save1;			/* for temporary storage  */
  char	save2;			/* for temporary storage */
  char  only_whitespace;	/* whitespace only? */
} formatInfo;

/* for breakpoints: */
typedef struct {
  Int	line;
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
Int	nextFreeTempVariable(void), nextFreeNamedVariable(void),
	nextFreeExecutable(void), nextFreeTempExecutable(void),
	dereferenceScalPointer(Int), findSym(Int, hashTableEntry *[], Int),
	findInternalName(char *, Int), luxerror(char *, Int, ...),
	lookForName(char *, hashTableEntry *[], Int), execute(Int);
char	*symName(Int, hashTableEntry *[]);

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
#define scalPointer(a)	{ if (sym[a].class == LUX_SCAL_PTR)\
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
#define LPTR(HEAD)		((Int *)((char *) HEAD + sizeof(array)))
#define CK_ARR(SYM, ARGN)	if (sym[SYM].class != LUX_ARRAY)\
				return cerror(NEED_ARR, SYM)
#define CK_SGN(ARR, N, ARGN, SYM)\
	for (i = 0; i < N; i++) \
	  if (ARR[i] < 0) return cerror(DIM_SMALL, SYM)
#define CK_MAG(MAG, ARR, N, ARGN, SYM) \
  for (i = 0; i < N; i++) \
    if (ARR[i] >= MAG) return cerror(ILL_DIM, ARGN, SYM)
#define GET_SIZE(SIZ, ARR, N) SIZ = 1; for (i = 0; i < (Int) N; i++)\
			 SIZ *= ARR[i]
#define N_ELEM(N) ((sym[N].spec.array.bstore - sizeof(array))\
                   /anaTypeSize[sym[N].type])

#define allocate(arg, size, type)\
   if (!(size)) arg = NULL; \
   else { if (!(arg = (void *) malloc((size)*sizeof(type))))\
   return luxerror("Memory allocation error\n", 0); }
#define eallocate(arg, size, type)\
 (size? ((arg = (type *) Malloc((size)*sizeof(type)))? 1: 0): (arg = NULL, 0))
#define GET_NUMERICAL(PTR, SIZE) \
  switch (sym[iq].class) \
  { case LUX_SCALAR:    PTR.l = &sym[iq].spec.scalar.l;  SIZE = 1;  break;  \
    case LUX_ARRAY:     h = HEAD(iq);  PTR.l = LPTR(h);  GET_SIZE(SIZE, h->dims, h->ndim);  \
      break; \
    default:        return cerror(ILL_CLASS, iq); }

	/* multi-type macro */
#define multiSwitch2(type, first, second) \
 switch (type) \
 { case LUX_BYTE:   first .b second ; break; \
   case LUX_WORD:   first .w second ; break; \
   case LUX_LONG:   first .l second ; break; \
   case LUX_FLOAT:  first .f second ; break; \
   case LUX_DOUBLE: first .d second ; break; }

#define multiSwitch3(type, first, second, third) \
 switch (type) \
 { case LUX_BYTE:   first .b second .b third ; break; \
   case LUX_WORD:   first .w second .w third ; break; \
   case LUX_LONG:   first .l second .l third ; break; \
   case LUX_FLOAT:  first .f second .f third ; break; \
   case LUX_DOUBLE: first .d second .d third ; break; }

#define multiSwitch4(type, first, second, third, fourth) \
 switch (type) \
 { case LUX_BYTE:   first .b second .b third .b fourth ; break; \
   case LUX_WORD:   first .w second .w third .b fourth ; break; \
   case LUX_LONG:   first .l second .l third .b fourth ; break; \
   case LUX_FLOAT:  first .f second .f third .b fourth ; break; \
   case LUX_DOUBLE: first .d second .d third .b fourth ; break; }

#define multiSwitch5(type, first, second, third, fourth, fifth) \
 switch (type) \
 { case LUX_BYTE:   first .b second .b third .b fourth .b fifth ; break; \
   case LUX_WORD:   first .w second .w third .b fourth .b fifth ; break; \
   case LUX_LONG:   first .l second .l third .b fourth .b fifth ; break; \
   case LUX_FLOAT:  first .f second .f third .b fourth .b fifth ; break; \
   case LUX_DOUBLE: first .d second .d third .b fourth .b fifth ; break; }

#include "output.h"

#ifndef transfer_target
#include "symbols.h"
#endif

#endif
