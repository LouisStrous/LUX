#ifndef INCLUDED_GSLPP_VECTOR_HH
#define INCLUDED_GSLPP_VECTOR_HH
#include "config.hh"
#if GSL_INCLUDE
# include <gsl/gsl_vector.h>

/// \file
/// C++ GSL bindings for gsl_vector.h.
///
/// Many GSL functions are available for different data types (unsigned char,
/// short, int, long, float, double, sometimes long double).  Because GSL is
/// implemented in C, each of those related functions has a different name which
/// includes an indication of the data type, except that that indication is
/// omitted if the data type is `double`.  In C++ it is possible to give those
/// related functions the same name.  This file provides such definitions, so
/// that you can use the GSL function name without a type indication also for
/// data types other than `double`.
///
/// All `T gsl_vector_T_get(const gsl_vector_T* v, const size_t i)` GSL
/// functions are available under the name `gsl_vector_get`, which is the GSL
/// function name when `T` = `double`.
///
/// All `gsl_vector_T_view gsl_vector_T_view_array(T* base, size_t n)` GSL
/// functions are available under the name `gsl_vector_view_array`, which is the
/// GSL function name when `T` = `double`.
///
/// All `gsl_vector_T_const_view gsl_vector_T_const_view_array(const T* base,
/// size_t n)` GSL functions are available under the name
/// `gsl_vector_const_view_array`, which is the GSL function name when `T` =
/// `double`.

/// C++ convenience version of gsl_vector_uchar_get().  Returns the value of a
/// vector element by index.
///
/// \param[in] v points at the vector.
///
/// \param[in] i is the index of the desired element.
///
/// \returns the value of the desired element.
unsigned char
gsl_vector_get(const gsl_vector_uchar* v, const size_t i)
{
  return gsl_vector_uchar_get(v, i);
}

/// C++ convenience version of gsl_vector_short_get().  Returns the value of a
/// vector element by index.
///
/// \param[in] v points at the vector.
///
/// \param[in] i is the index of the desired element.
///
/// \returns the value of the desired element.
short
gsl_vector_get(const gsl_vector_short* v, const size_t i)
{
  return gsl_vector_short_get(v, i);
}

/// C++ convenience version of gsl_vector_int_get().  Returns the value of a
/// vector element by index.
///
/// \param[in] v points at the vector.
///
/// \param[in] i is the index of the desired element.
///
/// \returns the value of the desired element.
int
gsl_vector_get(const gsl_vector_int* v, const size_t i)
{
  return gsl_vector_int_get(v, i);
}

/// C++ convenience version of gsl_vector_float_get().  Returns the value of a
/// vector element by index.
///
/// \param[in] v points at the vector.
///
/// \param[in] i is the index of the desired element.
///
/// \returns the value of the desired element.
float
gsl_vector_get(const gsl_vector_float* v, const size_t i)
{
  return gsl_vector_float_get(v, i);
}

/// C++ convenience version of gsl_vector_long_double_get().  Returns the value
/// of a vector element by index.
///
/// \param[in] v points at the vector.
///
/// \param[in] i is the index of the desired element.
///
/// \returns the value of the desired element.
long double
gsl_vector_get(const gsl_vector_long_double* v, const size_t i)
{
  return gsl_vector_long_double_get(v, i);
}

/// C++ convenience version of gsl_vector_uchar_view_array().  Returns a vector
/// view of an array.
///
/// \param[in] base points at the array.
///
/// \param[in] n is count of elements in the array.
///
/// \returns the vector view.
gsl_vector_uchar_view
gsl_vector_view_array(unsigned char* base, size_t n)
{
  return gsl_vector_uchar_view_array(base, n);
}

/// C++ convenience version of gsl_vector_short_view_array().  Returns a vector
/// view of an array.
///
/// \param[in] base points at the array.
///
/// \param[in] n is count of elements in the array.
///
/// \returns the vector view.
gsl_vector_short_view
gsl_vector_view_array(short* base, size_t n)
{
  return gsl_vector_short_view_array(base, n);
}

/// C++ convenience version of gsl_vector_int_view_array().  Returns a vector
/// view of an array.
///
/// \param[in] base points at the array.
///
/// \param[in] n is count of elements in the array.
///
/// \returns the vector view.
gsl_vector_int_view
gsl_vector_view_array(int* base, size_t n)
{
  return gsl_vector_int_view_array(base, n);
}

/// C++ convenience version of gsl_vector_long_view_array().  Returns a vector
/// view of an array.
///
/// \param[in] base points at the array.
///
/// \param[in] n is count of elements in the array.
///
/// \returns the vector view.
gsl_vector_long_view
gsl_vector_view_array(long* base, size_t n)
{
  return gsl_vector_long_view_array(base, n);
}

/// C++ convenience version of gsl_vector_float_view_array().  Returns a vector
/// view of an array.
///
/// \param[in] base points at the array.
///
/// \param[in] n is count of elements in the array.
///
/// \returns the vector view.
gsl_vector_float_view
gsl_vector_view_array(float* base, size_t n)
{
  return gsl_vector_float_view_array(base, n);
}

/// C++ convenience version of gsl_vector_long_double_view_array().  Returns a
/// vector view of an array.
///
/// \param[in] base points at the array.
///
/// \param[in] n is count of elements in the array.
///
/// \returns the vector view.
gsl_vector_long_double_view
gsl_vector_view_array(long double* base, size_t n)
{
  return gsl_vector_long_double_view_array(base, n);
}

/// C++ convenience version of gsl_vector_uchar_const_view_array().  Returns a
/// vector view of an array.
///
/// \param[in] base points at the array.
///
/// \param[in] n is count of elements in the array.
///
/// \returns the vector view.
gsl_vector_uchar_const_view
gsl_vector_const_view_array(const unsigned char* base, size_t n)
{
  return gsl_vector_uchar_const_view_array(base, n);
}

/// C++ convenience version of gsl_vector_short_const_view_array().  Returns a
/// vector view of an array.
///
/// \param[in] base points at the array.
///
/// \param[in] n is count of elements in the array.
///
/// \returns the vector view.
gsl_vector_short_const_view
gsl_vector_const_view_array(const short* base, size_t n)
{
  return gsl_vector_short_const_view_array(base, n);
}

/// C++ convenience version of gsl_vector_int_const_view_array().  Returns a
/// vector view of an array.
///
/// \param[in] base points at the array.
///
/// \param[in] n is count of elements in the array.
///
/// \returns the vector view.
gsl_vector_int_const_view
gsl_vector_const_view_array(const int* base, size_t n)
{
  return gsl_vector_int_const_view_array(base, n);
}

/// C++ convenience version of gsl_vector_long_const_view_array().  Returns a
/// vector view of an array.
///
/// \param[in] base points at the array.
///
/// \param[in] n is count of elements in the array.
///
/// \returns the vector view.
gsl_vector_long_const_view
gsl_vector_const_view_array(const long* base, size_t n)
{
  return gsl_vector_long_const_view_array(base, n);
}

/// C++ convenience version of gsl_vector_float_const_view_array().  Returns a
/// vector view of an array.
///
/// \param[in] base points at the array.
///
/// \param[in] n is count of elements in the array.
///
/// \returns the vector view.
gsl_vector_float_const_view
gsl_vector_const_view_array(const float* base, size_t n)
{
  return gsl_vector_float_const_view_array(base, n);
}

/// C++ convenience version of gsl_vector_long_double_const_view_array().
/// Returns a vector view of an array.
///
/// \param[in] base points at the array.
///
/// \param[in] n is count of elements in the array.
///
/// \returns the vector view.
gsl_vector_long_double_const_view
gsl_vector_const_view_array(const long double* base, size_t n)
{
  return gsl_vector_long_double_const_view_array(base, n);
}

#endif
#endif
