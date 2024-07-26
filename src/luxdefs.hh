/* This is file luxdefs.hh.

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
#ifndef INCLUDED_LUXDEFS_H
#define INCLUDED_LUXDEFS_H

/// \file
/// Definitions of enumerations and fixed compiler constants.

#ifndef MAX_DIMS
#include "install.hh"
#endif

#include <gsl/gsl_spline.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "types.hh"

#define malloctype(T) (T*) malloc(sizeof(T))
#define calloctype(count, T) (T*) calloc(count, sizeof(T))
#define realloctype(ptr, T) (T*) realloc(ptr, sizeof(T))

// tLine code
#define TRANS_FIXED_STRING      1

// symbol classes

/// LUX symbol classes.
enum Symbolclass
  {
    LUX_UNUSED = 0,      //!< (0) symbol with no assigned class
    LUX_SCALAR,          //!< (1) a real scalar, e.g. 17
    LUX_STRING,          //!< (2) a text string, e.g. 'x'
    LUX_RANGE,           //!< (3) a range, e.g. `(1:2)`
    LUX_ARRAY,           //!< (4) a numerical real or string array, e.g. `[1,2]`

    /// (5) a transfer symbol, a symbol that points at another symbol
    LUX_TRANSFER,
    LUX_ASSOC,                  //!< (6) an associated variable (`ASSOC`)
    LUX_FUNC_PTR,               //!< (7) a function pointer, e.g.`&SIN`
    LUX_SCAL_PTR,               //!< (8) a scalar pointer, e.g. `!MXB`
    LUX_SUBSC_PTR,              //!< (9) a subscript pointer, e.g. `(1:2)`
    LUX_FILEMAP,                //!< (10) a file map (e.g., `FLTFARR`)
    LUX_CLIST,                  //!< (11) a compact list (e.g., `{}`)
    LUX_LIST,                   //!< (12) a list (e.g., `{}`)
    LUX_STRUCT,                 //!< (13) a structure (e.g., `{}`)
    LUX_KEYWORD,                //!< (14) a keyword (e.g., `/FOO`)
    LUX_LIST_PTR,               //!< (15) a list member pointer (e.g., `.TAG`)
    LUX_PRE_RANGE,              //!< (16) a pre-range (e.g.,`(1:3)`)
    LUX_PRE_CLIST,              //!< (17) a pre-compact-list (`{}`)
    LUX_PRE_LIST,               //!< (18) a pre-list (`{}`)
    LUX_ENUM,                   //!< (19) an enumeration
    LUX_META,                   //!< (20) a meta (e.g., `SYMBOL('string')`)
    LUX_CSCALAR,                //!< (21) a complex scalar
    LUX_CARRAY,                 //!< (22) a complex array
    LUX_CPLIST,                 //!< (23) a compact pointer list
    LUX_POINTER,                //!< (24) a pointer (e.g., `&X`)
    LUX_STRUCT_PTR,             //!< (25) a pointer to member(s) of a structure
    LUX_SUBROUTINE = 32,        //!< (32) a subroutine definition
    LUX_FUNCTION,               //!< (33) a function definition
    LUX_BLOCKROUTINE,           //!< (34) a block routine definition
    LUX_DEFERRED_SUBR,          //!< (35) a deferred subroutine definition
    LUX_DEFERRED_FUNC,          //!< (36) a deferred function definition
    LUX_DEFERRED_BLOCK,         //!< (37) a deferred block routine
    LUX_BIN_OP = 192,           //!< (192) a binary operation
    LUX_INT_FUNC,               //!< (193) an internal function call
    LUX_USR_FUNC,               //!< (194) a user-defined function call
    LUX_IF_OP,                  //!< (195) a binary if-operation
    LUX_EXTRACT,                //!< (196) an extraction operation
    LUX_PRE_EXTRACT,            //!< (197) a pre-extraction operation
    LUX_EVB = 200,              //!< (200) an executable
    LUX_FIXED_NUMBER,           //!< (201) a fixed number
    LUX_FIXED_STRING,           //!< (202) a fixed string
    LUX_UNDEFINED = 255         //!< (255) an undefined symbol
  };

// note: do not change the relative order of these classes!  newSymbol
// might not work properly if you do.

/// LUX symbol types
enum Symboltype
  {
    LUX_INT8,                   //!< (0) 1-byte integers
    LUX_INT16,                  //!< (1) 2-byte integers
    LUX_INT32,                  //!< (2) 4-byte integers
    LUX_INT64,                  //!< (3) 8-byte integers
    LUX_FLOAT,                  //!< (4) 4-byte floats
    LUX_DOUBLE,                 //!< (5) 8-byte floats
    LUX_TEMP_STRING,            //!< (6) temporary strings
    LUX_LSTRING,                //!< (7) literal strings
    LUX_STRING_ARRAY,           //!< (8) string arrays
    LUX_CFLOAT,                 //!< (9) 8-byte complex floats
    LUX_CDOUBLE,                //!< (10) 16-byte complex floats
    LUX_NO_SYMBOLTYPE,          //!< sentinel; not for use
  };

template<class T>
constexpr Symboltype
lux_symboltype_for_type = LUX_NO_SYMBOLTYPE;

template<>
constexpr Symboltype
lux_symboltype_for_type<uint8_t> = LUX_INT8;

template<>
constexpr Symboltype
lux_symboltype_for_type<int16_t> = LUX_INT16;

template<>
constexpr Symboltype
lux_symboltype_for_type<int32_t> = LUX_INT32;

template<>
constexpr Symboltype
lux_symboltype_for_type<int64_t> = LUX_INT64;

template<>
constexpr Symboltype
lux_symboltype_for_type<float> = LUX_FLOAT;

template<>
constexpr Symboltype
lux_symboltype_for_type<double> = LUX_DOUBLE;

/// LUX EVB classes
enum EVBclass
  {
    EVB_BLOCK = 1,              //!< (1) statement block { }
    EVB_REPLACE,                //!< (2) assignment statement =
    EVB_INT_SUB,                //!< (3) call to internal subroutine
    EVB_FOR,                    //!< (4) FOR statement
    EVB_INSERT,                 //!< (5) insert statement
    EVB_IF,                     //!< (6) IF statement
    EVB_USR_SUB,                //!< (7) call to user-defined subroutine
    EVB_WHILE_DO,               //!< (8) WHILE-DO statement
    EVB_DO_WHILE,               //!< (9) DO-WHILE statement
    EVB_RETURN,                 //!< (10) RETURN statement
    EVB_CASE,                   //!< (11) CASE statement
    EVB_NCASE,                  //!< (12) NCASE statement
    EVB_REPEAT,                 //!< (13) REPEAT statement
    EVB_USR_CODE,               //!< (14) RUN statement
    EVB_FILE                    //!< (15) file inclusion @, @@
  };

/// LUX File inclusion types
enum includeType
  {
  FILE_INCLUDE,                 //!< (0) @file
  FILE_REPORT                   //!< (1) @@file
  };

// special execution codes
#define LUX_ERROR       -1      //!< error state
#define LUX_OK          1       //!< OK state
#define LOOP_BREAK      -2      //!< BREAK state
#define LOOP_CONTINUE   -3      //!< CONTINUE state
#define LOOP_RETALL     -4      //!< RETALL state
#define LOOP_RETURN     -5      //!< RETURN state

// breakpoints
#define BR_ENABLE       2       //!< enable breakpoint
#define BR_EXIST        1       //!< create breakpoint
#define BR_UNSET        0
#define BR_ENABLED      (BR_EXIST | BR_ENABLE)

// bigendian/littleendian
#if WORDS_BIGENDIAN
#define BIGENDIAN       1
#else
#define LITTLEENDIAN    1
#endif

/// LUX format types
enum fmtType
  {
    FMT_ERROR,                  //!< (0) illegal format
    FMT_PLAIN,                  //!< (1) plain text format
    FMT_INTEGER,                //!< (2) integer format
    FMT_FLOAT,                  //!< (3) float format
    FMT_TIME,                   //!< (4) time format
    FMT_DATE,                   //!< (5) date format
    FMT_STRING,                 //!< (6) string format
    FMT_COMPLEX,                //!< (7) complex float format
    FMT_EMPTY                   //!< (8) empty format
  };

// binary operation classes
/* don't change the order of these constants!  this order must
   correspond with the order in binOpName, binOpSign, binFunc* in
   eval.c, and the positions of LUX_EQ, LUX_OR, and LUX_POW relative
   to the (other) relational and logical operators must remain the
   same.  All between LUX_EQ and LUX_OR accept LUX_FLOAT operands, all
   between LUX_OR and LUX_POW do not. */

/// LUX binary operations
enum binaryOp
  {
    LUX_ADD,                    //!< (0) +
    LUX_SUB,                    //!< (1) -
    LUX_MUL,                    //!< (2) *
    LUX_DIV,                    //!< (3) /
    LUX_IDIV,                   //!< (4) \ .
    LUX_MOD,                    //!< (5) %, MOD
    LUX_SMOD,                   //!< (6) SMOD
    LUX_MAX,                    //!< (7) >
    LUX_MIN,                    //!< (8) <
    LUX_EQ,                     //!< (9) EQ
    LUX_GT,                     //!< (10) GT
    LUX_GE,                     //!< (11) GE
    LUX_LT,                     //!< (12) LT
    LUX_LE,                     //!< (13) LE
    LUX_NE,                     //!< (14) NE
    LUX_OR,                     //!< (15) OR
    LUX_AND,                    //!< (16) AND
    LUX_XOR,                    //!< (17) XOR
    LUX_POW,                    //!< (18) ^
    LUX_ANDIF,                  //!< (19) ANDIF
    LUX_ORIF                    //!< (20) ORIF
  };

#define NUM_BIN_OP      19      // note that this number does not
                                // count LUX_ANDIF and LUX_ORIF

 // boundingBoxType (PLOT,BOUNDINGBOX=) stuff
#define BB_NONE         0       // no bounding box
#define BB_ALL          1       // get everything
#define BB_PLOT         2       // plot window only

 // cleanUp() stuff
#define CLEANUP_VARS    1
#define CLEANUP_EDBS    2
#define CLEANUP_COMP    4
#define CLEANUP_BOTH    3
#define CLEANUP_ALL     7
#define CLEANUP_ERROR   8

 // special parser symbol numbers
#define LUX_NEW_LIST    -32766  // must be negative
#define LUX_EXTEND      -32755
#define LUX_LIST_DELIM  -32754
#define LUX_ZERO                4
#define LUX_ONE         1
#define LUX_MINUS_ONE   2

#define LUX_UNSPECIFIED -9999

 // some special functions and routines
#define LUX_NEG_FUN     0
#define LUX_SUBSC_FUN   1
#define LUX_CONCAT_FUN  4
#define LUX_POW_FUN     3

#define LUX_INSERT_SUB  0

 // openPathFile options
#define FIND_SUBR       0
#define FIND_FUNC       1
#define FIND_EITHER     2
#define FIND_LOWER      4

// data file types

#define FILE_TYPE_ANA_FZ        1 // lux fz file
#define FILE_TYPE_IDL_SAVE      2 // IDL Save file
#define FILE_TYPE_GIF           3 // GIF file
#define FILE_TYPE_ANA_ASTORE    4 // LUX ASTORE file
#define FILE_TYPE_JPEG          5 // JPEG file
#define FILE_TYPE_TIFF          6 // TIFF file
#define FILE_TYPE_FITS          7 // FITS file
#define FILE_TYPE_PPM_RAW       8
#define FILE_TYPE_PPM_ASCII     9
#define FILE_TYPE_XPM           10
#define FILE_TYPE_XBM           11 // X11 bitmap
#define FILE_TYPE_BMP           12
#define FILE_TYPE_SUN_RAS       13 // Sun raster file
#define FILE_TYPE_IRIS_RGB      14
#define FILE_TYPE_TARGA_24      15
#define FILE_TYPE_PM            16

// symbol identification options

#define I_FILELEVEL     ((1<<0) | (1<<1))
#define I_LINE          (1<<1)
#define I_VALUE         (1<<2)
#define I_TRUNCATE      (1<<3)
#define I_PARENT        (1<<4)
#define I_NL            (1<<5)
#define I_ROUTINE       (1<<6)
#define I_LENGTH        (1<<7)
#define I_ALTERNATIVE   (1<<8)
#define I_SINGLEMODE    (~(I_PARENT | I_LINE | I_FILELEVEL))
#define I_SINGLEEXEC    (~(I_PARENT | I_LINE | I_FILELEVEL | I_VALUE))

/// Trace Mode bit flags
enum TraceMode
  {
    T_FILE = (1 << 0),          //!< Trace @files.
    T_LOOP = (1 << 1),          //!< Trace any kind of loop.
    T_BLOCK = (1 << 2),         //!< Trace {} blocks.

    /// Trace user-defined subroutines, functions, and block routines.
    T_ROUTINE = (1 << 3),

    T_SHOWSTATS = (1 << 4),       //!< Show execution statistics.
    T_CPUTIME = (1 << 5),         //!< Show CPU time.
    T_SHOWEXEC = (1 << 6),
    T_ROUTINEIO = (1 << 7)
  };

// sprintf modes
#define FMT_LEFT_JUSTIFY        (1 << 0) // - modifier
#define FMT_ALWAYS_SIGN         (1 << 1) // + modifier
#define FMT_ZERO_PAD            (1 << 2) // 0 modifier
#define FMT_ALTERNATIVE         (1 << 3) // # modifier
#define FMT_POSITIVE_BLANK      (1 << 4) // ' ' modifier
#define FMT_SUPPRESS            (1 << 5) // * modifier
#define FMT_BIG                 (1 << 6) // l modifier
#define FMT_SMALL               (1 << 7) // h modifier
#define FMT_BIGINT              (1 << 8) // j modifier
#define FMT_MIX                 (1 << 9) // _ modifier
#define FMT_MIX2                (1 << 10) // = modifier

const double PI = 3.141592653589793238463;
const double TWOPI = 2*PI;
const double DEG = PI/180;
const double RAD = 180/PI;

// debug breakpoint modes
#define DEBUG_STEP      1

#define PROTECT_CONTEXT INT16_MAX
#define PROTECTED       (INT16_MAX - 2000)
#define MAINTAIN        128     // to mark symbols that may not be
                                // overwritten

// getNumerical() stuff
#define GN_UPDATE       1       // update input symbol
#define GN_UPGRADE      2       // output at least <minType>
#define GN_EXACT        4       // output exactly <minType>

// startStandardLoop() stuff
#define SL_SAMEDIMS     0 //!< 0x0 output needs same dimensions as input */
#define SL_COMPRESS     1 //!< 0x1 output lacks first dimension from axis */
#define SL_COMPRESSALL  2 //!< 0x2 output lacks all dimensions from axis */

#define SL_EXACT        0*(1<<2) //!< 0x0 output type is exactly as specified */
#define SL_UPGRADE      1*(1<<2) //!< 0x4 output type is upgraded if necessary */
#define SL_KEEPTYPE     2*(1<<2) //!< 0x8 output type equal to input type */

#define SL_EACHCOORD    0*(1<<4) //!< 0x0 need all coordinates */
#define SL_AXISCOORD    (1<<4) //!< 0x10 need only coordinate along specified axis */
#define SL_EACHROW      (1<<5) //!< 0x20 treat row by row */
#define SL_UNIQUEAXES   (1<<6) //!< 0x40 all axes must be unique */
#define SL_ONEAXIS      (1<<7) //!< 0x80 at most one axis */
#define SL_AXESBLOCK    ((1<<8) | SL_UNIQUEAXES) //!< 0x140 treat selected axes as a block, includes SL_UNIQUEAXES */
#define SL_ONEDIMS      (1<<9) //!< 0x200 "compressed" dimensions replaced by 1s */
#define SL_SRCUPGRADE   (1<<10) //!< 0x400 upgrade source data type if necessary */
#define SL_NEGONED      (1<<11) //!< 0x800 negative axis argument -> treat as 1D */
#define SL_EACHBLOCK    ((1<<12) | SL_AXESBLOCK) //!< 0x1140 treat all axes at once */
#define SL_ALLAXES      (1<<13) //!< 0x2000 select all axes for treatment */
#define SL_TAKEONED     (1<<14) //!< 0x4000 treat ax 1D array */

// stringpointer() stuff
#define SP_VAR          1
#define SP_USER_FUNC    2
#define SP_USER_SUBR    4
#define SP_INT_FUNC     8
#define SP_INT_SUBR     16
#define SP_FUNC         (SP_USER_FUNC + SP_INT_FUNC)
#define SP_SUBR         (SP_USER_SUBR + SP_INT_SUBR)
#define SP_ANY          (SP_VAR + SP_FUNC + SP_SUBR)

#define scratSize()     (NSCRAT*sizeof(int32_t) + (curScrat - (char *) scrat))

/// a single-precision complex number */
struct FloatComplex
{
  float real;
  float imaginary;
};

/// a double-precision complex number */
struct DoubleComplex
{
  double real;
  double imaginary;
};

/// a union of scalar values */
union Scalar
{
  uint8_t ui8;
  int16_t i16;
  int32_t i32;
  int64_t i64;
  float f;
  double d;
  char *s;
  char **sp;
  FloatComplex cf;
  DoubleComplex cd;
};

/// A union of pointers to all data types that LUX arrays can hold.
union Pointer
{
  uint8_t* ui8;             //!< Pointer to 8-bit unsigned array
  int16_t *i16;             //!< Pointer to 16-bit signed array
  int32_t *i32;             //!< Pointer to 32-bit signed array
  int64_t *i64;             //!< Pointer to 64-bit signed array
  float *f;                 //!< Pointer to single precision float array
  double *d;                //!< Pointer to double precision float array
  char *s;                  //!< Pointer to text string
  char **sp;                //!< Pointer to array of text strings
  void *v;                  //!< Generic pointer
  FloatComplex *cf;         //!< Pointer to single precision float complex array
  DoubleComplex *cd;        //!< Pointer to double precision float complex array
};

struct ListElem
{
  char const* key;
 int16_t value;
};

struct EnumElem
{
  char const* key; int32_t value;
};

struct KeyList
{
  char suppressEval; char pipe; char suppressUnused;
  int32_t defaultMode; uint8_t offset; char **keys;
};

/// A struct representing the dimensions of a LUX array.
struct Array
{
  uint8_t ndim, c1, c2; int32_t dims[MAX_DIMS];
};

/// A struct that represents the least and greatest values that the LUX
/// numerical data types can hold.
struct BoundsStruct {
  /// A struct with a member for each of the LUX numerical data types.
  struct T {
    uint8_t ui8;
    int16_t i16;
    int32_t i32;
    int64_t i64;
    float f;
    double d;
  };
  T min;     //!< the least values that the LUX numerical data types can hold
  T max;     //!< the greatest values that the LUX numerical data types can hold
};

struct StructElem
{
  union
  {
    struct
    {
      int32_t nelem;
      int32_t size;
      int32_t* dims;
      uint8_t ndim;
    } first;
    struct
    {
      char *tag;
      off_t offset;
      uint8_t type;
      union
      {
        struct
        {
          int32_t* dims;
          uint8_t ndim;
        } singular;
        int32_t member;
      } spec;
    } regular;
  } u;
};

struct ExtractSec {
  uint8_t type;
  uint16_t number;
  union
  {
    int16_t* i16;
    char** sp;
  } ptr;
};

struct StructPtrMember
{
  uint8_t type;                 //!< subscript type: scalar, range, index
  union
  {
    struct
    {
      int32_t value;            //!< the single integer subscript
    } scalar;
    struct
    {
      int32_t start;            //!< the integer range start
      int32_t end;              //!< the integer range end
    } range;
    struct
    {
      int32_t n_elem;           //!< the number of index array elements
      int32_t* ptr;             //!< pointer to the index array elements
    } array;
  } data;
};

struct StructPtr
{
  int32_t desc;                 //!< index of structure descriptor
  int32_t n_subsc;              //!< number of subscripts
  StructPtrMember* member;
};

struct PreExtract {
  char* name;
  ExtractSec* extract;
};

struct SymbolImpl
{
  Symbolclass sclass;
  Symboltype type;
  int16_t xx;
  int32_t line;
  int16_t context;
  int32_t exec;
  union specUnion
  {
    Scalar scalar;
    struct
    {
      Array* ptr;
      int32_t bstore;
    } array;
    struct
    {
      int16_t* ptr;
      int32_t bstore;
    } wlist;
    struct
    {
      uint16_t* ptr;
      int32_t bstore;
    } uwlist;
    struct
    {
      EnumElem* ptr;
      int32_t bstore;
    } enumElem;
    struct
    {
      char* ptr;
      int32_t bstore;
    } name;
    struct
    {
      ListElem* ptr;
      int32_t bstore;
    } listElem;
    struct
    {
      int32_t* ptr;
      int32_t bstore;
    } intList;
    struct
    {
      ExtractSec* ptr;
      int32_t bstore;
    } extract;
    struct
    {
      PreExtract* ptr;
      int32_t bstore;
    } preExtract;
    struct
    {
      void* ptr;
      int32_t bstore;
    } general;
    struct
    {
      StructPtr* ptr;
      int32_t bstore;
    } structPtr;
    Pointer dpointer;
    struct
    {
      int16_t args[4];
    } evb;
    struct
    {
      uint16_t args[4];
    } uevb;
    struct
    {
      uint8_t narg;
      char** keys;
      uint8_t extend;
      uint16_t nstmnt;
      int16_t* ptr;
    } routine;
  } spec;
};

struct HashTableEntry
{
  char const* name;
  int32_t symNum;
  HashTableEntry *next;
};

struct InternalRoutine
{
  char const* name;
  int16_t minArg;
  int16_t maxArg;
  int32_t (*ptr)(int32_t, int32_t []);
  char const* keys;
};

struct FzHead
{
  int32_t synch_pattern;
  uint8_t subf;
  uint8_t source;
  uint8_t nhb;
  uint8_t datyp;
  uint8_t ndim;
  uint8_t free1;
  uint8_t cbytes[4];
  uint8_t free[178];
  int32_t dim[16];
  char txt[256];
};

/// A struct that captures at what depth in an assignment the left hand side can
/// be used for storing an intermediate result.
struct BranchInfo
{
  int32_t depth;
  int32_t symbol;
  int32_t size;
  char containLHS;
};

/// A struct that holds data for cubic spline calculations.
struct CsplineInfo
{
  gsl_spline* spline;
  gsl_interp_accel* acc;
  double* x;
  double* y;
};


/// for nextCompileLevel
struct CompileInfo
{
  char* line;
  int32_t (*charfunc)(void);
  char* name;
  FILE* stream;
  int32_t line_number;
  CompileInfo* next;
  CompileInfo* prev;
};

/// for execution nesting info:
struct ExecutionLevelInfo
{
  int32_t target;
  int32_t line;
};

struct FormatInfo
{
  char* format;               //!< whole format string
  char* current;              //!< the currently selected format entry
  char* start;                //!< initial % of current format entry
  char* spec_char;            //!< current specification character
  char* repeat;               //!< start of repeat count
  char* plain;                //!< start of plain text?
  char* end;                  //!< end of current format entry
  char* next;                 //!< start of next format entry
  fmtType type;               //!< format type
  int32_t width;              //!< format width
  int32_t precision;          //!< format precision
  int32_t flags;              //!< format modification flags
  int32_t count;              //!< format repetition count
  int32_t active_group;       //!< number of currently active groups
  int32_t group_count[MAXFMT];  //!< current group counts
  char* group_start[MAXFMT];    //!< current group starts
  char save1;                   //!< for temporary storage
  char save2;                   //!< for temporary storage
  char only_whitespace;         //!< whitespace only?
};

/// A LUX breakpoint
struct Breakpoint
{
  int32_t line;
  char* name;
  char status;
};
#define BP_DEFINED      1
#define BP_ENABLED      2
#define BP_VARIABLE     4


// for Dick's stuff
#define types_ptr       Pointer
#define ahead           Array
#define sym_desc        SymbolImpl
#define class8_to_1(x)  dereferenceScalPointer(x)

#define symbol_ident_single(x,y)        what(x,y)
int32_t     nextFreeTempVariable(void), nextFreeNamedVariable(void),
        nextFreeExecutable(void), nextFreeTempExecutable(void),
        dereferenceScalPointer(int32_t), findSym(int32_t, HashTableEntry *[], int32_t),
        findInternalName(char const *, int32_t), luxerror(char const *, int32_t, ...),
        lookForName(char const *, HashTableEntry *[], int32_t), execute(int32_t);
char const* symName(int32_t, HashTableEntry *[]);

#define getFreeTempVariable(a)\
        { if ((a = nextFreeTempVariable()) < 0) return a; }
#define getFreeNamedVariable(a)\
        { if ((a = nextFreeNamedVariable()) < 0) return a; }
#define getFreeExecutable(a)\
        { if ((a = nextFreeExecutable()) < 0) return a; }
#define getFreeTempExecutable(a)\
        { if ((a = nextFreeTempExecutable()) < 0) return a; }
#define isTemp(a)       (a >= TEMPS_START && a < TEMPS_END)
#define isFreeTemp(a)   (isTemp(a) && symbol_context(a) == -compileLevel)
#define scalPointer(a)  { if (symbol_class(a) == LUX_SCAL_PTR)\
                          a = dereferenceScalPointer(a); }
#define findVar(index, context) findSym(index, varHashTable, context)
#define findSubr(index)         findSym(index, subrHashTable, 0)
#define findFunc(index)         findSym(index, funcHashTable, 0)
#define findBlock(index)        findSym(index, blockHashTable, 0)
extern char const* symbolStack[];
#define findInternalSym(index, a) findInternalName(symbolStack[index], a)
#define findInternalSubr(index) findInternalSym(index, 1)
#define findInternalFunc(index) findInternalSym(index, 0)
#define lookForSym(indx, table, context) lookForName(symbolStack[indx], table, context)
#define lookForVar(index, context) lookForSym(index, varHashTable, context)
#define lookForVarName(name, context) lookForName(name, varHashTable, context)
#define findVarName(name, context) findName(name, varHashTable, context)
#define lookForSubr(index)      lookForSym(index, subrHashTable, 0)
#define lookForFunc(index)      lookForSym(index, funcHashTable, 0)
#define lookForBlock(index)     lookForSym(index, blockHashTable, 0)
#define lookForSubrName(name)   lookForName(name, subrHashTable, 0)
#define lookForFuncName(name)   lookForName(name, funcHashTable, 0)
#define lookForBlockName(name)  lookForName(name, blockHashTable, 0)
#define varName(index)          symName(index, varHashTable)
#define subrName(index)         symName(index, subrHashTable)
#define funcName(index)         symName(index, funcHashTable)
#define blockName(index)        symName(index, blockHashTable)
#define doit(symbol)            if ((n = execute(symbol)) < 0) return n
#define restore                 curContext = oldContext
#define readGhost(fp, value, index, size)\
        { if (fseek(fp, index*(size), SEEK_SET))\
          return cerror(POS_ERR, 0); \
        if (fread(&(value), size, 1, fp) != 1)\
          return cerror(READ_ERR, 0); }

#undef MIN
#define MIN(A,B)                ((A < B)? A: B)
#undef MAX
#define MAX(A,B)                ((A > B)? A: B)
#undef ABS
#define ABS(A)                  (((A) >= 0)? (A): -(A))
#define HEAD(SYM)               ((Array *) sym[SYM].spec.array.ptr)
#define LPTR(HEAD)              ((int32_t *)((char *) HEAD + sizeof(Array)))
#define CK_ARR(SYM, ARGN)       if (symbol_class(SYM) != LUX_ARRAY)\
                                return cerror(NEED_ARR, SYM)
#define CK_SGN(ARR, N, ARGN, SYM)\
        for (i = 0; i < N; i++) \
          if (ARR[i] < 0) return cerror(DIM_SMALL, SYM)
#define CK_MAG(MAG, ARR, N, ARGN, SYM) \
  for (i = 0; i < N; i++) \
    if (ARR[i] >= MAG) return cerror(ILL_DIM, ARGN, SYM)
#define GET_SIZE(SIZ, ARR, N) SIZ = 1; for (i = 0; i < (int32_t) N; i++)\
                         SIZ *= ARR[i]
#define N_ELEM(N) ((sym[N].spec.array.bstore - sizeof(Array))\
                   /anaTypeSize[sym[N].type])

#define ALLOCATE(arg, size, type)\
   if (!(size)) arg = NULL; \
   else { if (!(arg = (type *) malloc((size)*sizeof(type))))\
   return luxerror("Memory allocation error\n", 0); }
// #define eallocate(arg, size, type)\
//  (size? ((arg = (type *) Malloc((size)*sizeof(type)))? 1: 0): (arg = NULL, 0))
#define GET_NUMERICAL(PTR, SIZE) \
  switch (symbol_class(iq)) \
  { case LUX_SCALAR:    PTR.i32 = &sym[iq].spec.scalar.i32;  SIZE = 1;  break;  \
    case LUX_ARRAY:     h = HEAD(iq);  PTR.i32 = LPTR(h);  GET_SIZE(SIZE, h->dims, h->ndim);  \
      break; \
    default:        return cerror(ILL_CLASS, iq); }

        // multi-type macro
#define multiSwitch2(type, first, second) \
 switch (type) \
 { case LUX_INT8:   first .ui8 second ; break; \
   case LUX_INT16:   first .i16 second ; break; \
   case LUX_INT32:   first .i32 second ; break; \
   case LUX_FLOAT:  first .f second ; break; \
   case LUX_DOUBLE: first .d second ; break; }

#define multiSwitch3(type, first, second, third) \
 switch (type) \
 { case LUX_INT8:   first .ui8 second .ui8 third ; break; \
   case LUX_INT16:   first .i16 second .i16 third ; break; \
   case LUX_INT32:   first .i32 second .i32 third ; break; \
   case LUX_FLOAT:  first .f second .f third ; break; \
   case LUX_DOUBLE: first .d second .d third ; break; }

#define multiSwitch4(type, first, second, third, fourth) \
 switch (type) \
 { case LUX_INT8:   first .ui8 second .ui8 third .ui8 fourth ; break; \
   case LUX_INT16:   first .i16 second .i16 third .ui8 fourth ; break; \
   case LUX_INT32:   first .i32 second .i32 third .ui8 fourth ; break; \
   case LUX_FLOAT:  first .f second .f third .ui8 fourth ; break; \
   case LUX_DOUBLE: first .d second .d third .ui8 fourth ; break; }

#define multiSwitch5(type, first, second, third, fourth, fifth) \
 switch (type) \
 { case LUX_INT8:   first .ui8 second .ui8 third .ui8 fourth .ui8 fifth ; break; \
   case LUX_INT16:   first .i16 second .i16 third .ui8 fourth .ui8 fifth ; break; \
   case LUX_INT32:   first .i32 second .i32 third .ui8 fourth .ui8 fifth ; break; \
   case LUX_FLOAT:  first .f second .f third .ui8 fourth .ui8 fifth ; break; \
   case LUX_DOUBLE: first .d second .d third .ui8 fourth .ui8 fifth ; break; }

#include "output.hh"

#ifndef transfer_target
#include "symbols.hh"
#endif

#endif
