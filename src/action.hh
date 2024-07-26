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
/// \file

#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#ifndef INCLUDED_ACTION_HH
#define INCLUDED_ACTION_HH

#include <numeric>              // for iota
#include <vector>

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
extern HashTableEntry   *varHashTable[], *subrHashTable[], *funcHashTable[],
                        *blockHashTable[];
extern SymbolImpl    sym[];
extern InternalRoutine  *subroutine, *function;
extern int32_t          nSubroutine, nFunction, curLineNumber, compileLevel,
                        ignoreInput, curSymbol, axisTally[];
extern unsigned int     internalMode;
extern BoundsStruct      bounds;
extern int32_t  (*lux_converts[])(int32_t, int32_t []);

FILE* openPathFile(char const*, int32_t);

Symboltype combinedType(Symboltype, Symboltype);

char* expand_name(char const*, char const*);
char* nextline(char*, size_t, FILE*);
char* string_arg(int32_t);
char* strsave(char const*);
char* strsave_system(char*);
char* symbolIdent(int32_t, int32_t);

char const* className(int32_t);
char const* keyName(InternalRoutine* routine, int32_t number, int32_t index);
char const* symName(int32_t, HashTableEntry*[]);
char const* symbolProperName(int32_t);
char const* typeName(int32_t);

CsplineInfo const empty_cubic_spline(void);

double cspline_derivative(double, CsplineInfo*);
double cspline_second_derivative(double, CsplineInfo*);
double cspline_value(double, CsplineInfo*);

double double_arg(int32_t);
double famod(double,double);
double fasmod(double,double);
double find_cspline_value(double, double, double, CsplineInfo*);
double hypota(int32_t, double*);
double vhypot(int32_t, double, double, ...);

float float_arg(int32_t);

int setupDimensionLoop(LoopInfo*, int32_t, int32_t const*, Symboltype, int32_t,
                       int32_t const*, Pointer*, int32_t);

int32_t Sprintf(char*, char*, ...);
int32_t approximately_equal(double, double, double);
int32_t approximately_equal_f(float, float, float);
int32_t approximately_equal_z(DoubleComplex, DoubleComplex, double);
int32_t approximately_equal_z_f(FloatComplex, FloatComplex, float);
int32_t array_clone(int32_t, Symboltype);
int32_t array_clone_zero(int32_t, Symboltype);
int32_t array_scratch(Symboltype, int32_t, int32_t []);
int32_t cerror(int32_t, int32_t, ...);
int32_t convertRange(int32_t);
int32_t copyEvalSym(int32_t);
int32_t copySym(int32_t);
int32_t cubic_spline_tables(void*, int32_t, int32_t, void*, int32_t, int32_t,
                            int32_t, uint8_t, uint8_t, CsplineInfo*);
int32_t dereferenceScalPointer(int32_t);
int32_t double_arg_stat(int32_t, double*);
int32_t essentially_equal(double, double, double);
int32_t essentially_equal_f(float, float, float);
int32_t essentially_equal_z(DoubleComplex, DoubleComplex, double);
int32_t essentially_equal_z_f(FloatComplex, FloatComplex, float);
int32_t eval(int32_t);
int32_t evals(int32_t);
int32_t findInternalName(char const*, int32_t);
int32_t findName(char const*, HashTableEntry**, int32_t);
int32_t findSym(int32_t, HashTableEntry*[], int32_t);
int32_t float_arg_stat(int32_t, float*);
int32_t getNumerical(int32_t, Symboltype, int32_t*, Pointer*, char, int32_t*,
                     Pointer*);
int32_t getSimpleNumerical(int32_t, Pointer*, int32_t*);
int32_t get_dims(int32_t*, int32_t*, int32_t*);
int32_t iamod(int32_t,int32_t);
int32_t iasmod(int32_t,int32_t);
int32_t int_arg(int32_t);
int32_t int_arg_stat(int32_t, int32_t*);
int32_t listNumElements(int32_t);
int32_t lux_byte(int32_t, int32_t []);
int32_t lux_cdouble(int32_t, int32_t []);
int32_t lux_cfloat(int32_t, int32_t []);
int32_t lux_double(int32_t, int32_t []);
int32_t lux_float(int32_t, int32_t []);
int32_t lux_int64(int32_t, int32_t []);
int32_t lux_long(int32_t, int32_t []);
int32_t lux_string(int32_t, int32_t []);
int32_t lux_word(int32_t, int32_t []);
int32_t lux_zero(int32_t, int32_t []);
int32_t lux_zerof(int32_t, int32_t []);
int32_t luxerror(char const*, int32_t, ...);
int32_t my_fprintf(FILE*, char*, ...);
int32_t nextLoops(LoopInfo*, LoopInfo*);
int32_t nextchar(FILE*);
int32_t numerical(int32_t, int32_t**, int32_t*, int32_t*, Pointer*);
int32_t numerical_clone(int32_t, Symboltype);
int32_t numerical_or_string(int32_t, int32_t**, int32_t*, int32_t*, Pointer*);
int32_t pop(void);
int32_t popTempVariable(int32_t);
int32_t prepareDiagonals(int32_t, LoopInfo*, int32_t, int32_t**, int32_t**,
                         int32_t**, int32_t**);
int32_t redef_array(int32_t, Symboltype, int32_t, int32_t*);
int32_t redef_scalar(int32_t, Symboltype, void*);
int32_t redef_string(int32_t, int32_t);
int32_t routineContext(int32_t);
int32_t scalar_scratch(Symboltype);
int32_t scalar_scratch_copy(int32_t);
int32_t standardLoop(int32_t, int32_t, int32_t, Symboltype, LoopInfo*,
                     Pointer*,int32_t*, LoopInfo*, Pointer*);
int32_t standardLoop0(int32_t, int32_t, int32_t*, int32_t, Symboltype,
                      LoopInfo*, Pointer*, int32_t*, LoopInfo*, Pointer*);
int32_t standardLoopX(int32_t, int32_t, int32_t, LoopInfo*, Pointer*, int32_t,
                      int32_t const*, int32_t, int32_t const*, Symboltype,
                      int32_t, int32_t*, LoopInfo*, Pointer*);
int32_t standardLoopX(int32_t, int32_t, int32_t, LoopInfo*, Pointer*, int32_t,
                      int32_t const*, int32_t, int32_t const*, Symboltype,
                      int32_t, int32_t*, LoopInfo*, Pointer*);
int32_t strcasecmp_p(char*, char*);
int32_t strccmp(char const*, char const*);
int32_t string_scratch(int32_t);
int32_t stringpointer(char*, int32_t);
int32_t strncasecmp_p(char*, char*, int32_t);
int32_t to_scalar(int32_t, Symboltype);
int32_t to_scratch_array(int32_t, Symboltype, int32_t, int32_t*);
int32_t transfer(int32_t);
int32_t transferAll(int32_t symbol);
int32_t translateEscapes(char*);
int32_t unnextchar(int32_t, FILE*);

void addVerify(char*, char);
void checkErrno(void);
void cleanup_cubic_spline_tables(CsplineInfo*);
void clearToPopTempVariable(int32_t);
void convertPointer(Scalar*, Symboltype, Symboltype);
void cspline_value_and_derivative(double, double*, double*, CsplineInfo*);
void deleteFacts(int32_t symbol, int32_t type);
void deleteStack(void);
void dupList(void);
void embed(int32_t, int32_t);
void endian(void*, int32_t, int32_t);
void find_cspline_extremes(double, double, double*, double*, double*, double*, CsplineInfo*);
void freeString(int32_t);
void mark(int32_t);
void newStack(int32_t);
void pegMark(void);
void printw(char const*);
void printwf(char const*, ...);
void protect(int32_t*, int32_t);
void protectOne(int16_t);
void push(int32_t);
void pushTempVariable(int32_t);
void pushTempVariableIndex(void);
void rearrangeDimensionLoop(LoopInfo*);
void rearrangeEdgeLoop(LoopInfo*, LoopInfo*, int32_t);
void resetPager(void);
void returnLoop(LoopInfo*, Pointer*, int32_t);
void setPager(int32_t);
void standard_redef_array(int32_t, Symboltype, int32_t, int32_t*, int32_t,
                          int32_t*, int32_t, Pointer*, LoopInfo*);
void unMark(int32_t);
void unProtect(int32_t*, int32_t);
void undefine(int32_t);
void unlinkString(int32_t);
void updatIndices(void);
void zap(int32_t);
void zapMarked(void);
void zapTemp(int32_t);
void zerobytes(void*, int32_t);

// It is tricky to avoid round-off error when calculating running sums.
// Even using Kahan summation it is possible to get small residual
// round-off errors, and to get negative running sums for all-zero data
// in the running-sum window at the end of a sequence containing
// non-zero values.

// ACP 4.2.2 says:

// Let u′ = (u f+ v) f- w
//     v′ = (u f+ v) f- u
//     u″ = (u f+ v) f- v′
//     v″ = (u f+ v) f- u′

// where f+ is properly rounded floating-point addition and f- is
// properly rounded floating-point subtraction.

// Then u + v = (u f+ v) + ((u f- u′) f+ (v f- v″))

/// A class to do summation in the Kahan way to reduce accumulated round-off
/// error.  See https://en.wikipedia.org/wiki/Kahan_summation_algorithm for the
/// algorithm.
///
/// \tparam T is the data type of the values being summed.
template<class T>
class Kahan_sum
{
public:
  // constructor

  /// Constructor.  The sum is initialized to 0.
  Kahan_sum()
    : m_sum(), m_compensation()
  { }

  // const methods

  /// Get the current sum.
  ///
  /// \returns the current sum.
  T
  operator()() const
  {
    return m_sum;
  }

  // non-const methods

  /// Assign a value, losing the previous state.
  ///
  /// \param x is the value to assign.
  ///
  /// \returns a reference to the instance.
  Kahan_sum&
  operator=(const T x)
  {
    m_sum = x;
    m_compensation = 0;
    return *this;
  }

  /// Add a value.
  ///
  /// \param x is the value to add to the sum.
  ///
  /// \returns a reference to the instance.
  Kahan_sum&
  operator+=(const T x)
  {
    auto y = x - m_compensation;
    auto t = m_sum + y;
    m_compensation = t - m_sum;
    m_compensation -= y;
    m_sum = t;
    return *this;
  }

  /// Subtract a value.
  ///
  /// \param x is the value to subtract from the sum.
  ///
  /// \returns a reference to the instance.
  Kahan_sum&
  operator-=(const T x)
  {
    return operator+(-x);
  }

private:
  T m_sum;                      //!< the sum
  T m_compensation;             //!< the compensation value
};

/// A variant of strtol() that accepts a const 2nd argument.  Read a number from
/// text.
///
/// \param[in] str is the text to parse.
///
/// \param[in,out] endptr points to the address where, if the address is not
/// null, the pointer to just after the parsed text gets stored.
///
/// \param base is the number base to assume for the parsing.
inline int
strtol(const char* str, const char** endptr, int base) {
  return std::strtol(str, const_cast<char**>(endptr), base);
}

/// A variant of strtol() that doesn't return a pointer to just after the parsed
/// text.  Read a number from text.
///
/// \param[in] str is the text to parse.
///
/// \param base is the number base to assume for the parsing.
inline int
strtol(const char* str, int base) {
  return std::strtol(str, nullptr, base);
}

#define axisAxes(i)     (axisAxis? axisAxis[i]: (i))

#define debugout(msg)   printf("DEBUG - %s [%s, line %d]\n", (msg), __FILE__, __LINE__)
#define debugout1(fmt,arg)      printf("DEBUG - "); printf((fmt), (arg)); printf(" [%s, line %d]\n", __FILE__, __LINE__)

/// Get the value from a numerical scalar LUX symbol and convert it to the
/// desired type.
///
/// \tparam is the desired data type
///
/// \par iq identifies the LUX symbol.
///
/// \returns the converted value, or reports an error and returns 0 if the
/// symbol wasn't a scalar.
template<typename T>
T
get_scalar_value(Symbol iq)
{
 if (symbol_class(iq) == LUX_SCAL_PTR)
    iq = dereferenceScalPointer(iq);
  if (symbol_class(iq) != LUX_SCALAR) {
    cerror(NO_SCAL, iq);
    return 0;
  }
  switch (scalar_type(iq)) {
  case LUX_INT8:
    return (T) scalar_value(iq).ui8;
  case LUX_INT16:
    return (T) scalar_value(iq).i16;
  case LUX_INT32:
    return (T) scalar_value(iq).i32;
  case LUX_INT64:
    return (T) scalar_value(iq).i64;
  case LUX_FLOAT:
    return (T) scalar_value(iq).f;
  case LUX_DOUBLE:
    return (T) scalar_value(iq).d;
  default:
    cerror(ILL_TYPE, iq);
    return 0;
  }
}

/// A template function that produces a vector with elements incrementing
/// from zero.
///
/// \tparam T is the data type of the elements.
///
/// \param n is the number of elements to put in the vector.
///
/// \param start is the value to store in the first element.
///
/// \returns the vector.
template<typename T>
std::vector<T>
make_iota(size_t n, T start = T{})
{
  std::vector<T> result(n);
  std::iota(result.begin(), result.end(), start);
  return result;
}

#endif
