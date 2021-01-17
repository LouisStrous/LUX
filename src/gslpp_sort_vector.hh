#ifndef INCLUDED_GSLPP_SORT_VECTOR_HH
#define INCLUDED_GSLPP_SORT_VECTOR_HH
#include "config.h"
#if HAVE_LIBGSL
# include <gsl/gsl_sort_vector.h>

/// \file
/// C++ GSL bindings for gsl_sort_vector.h.
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
/// All `void gsl_sort_vector2_T(gsl_vector_T* v1, gsl_vector_T* v2)` GSL
/// functions are available under the name `gsl_sort_vector2`, which is the GSL
/// function name when `T` = `double`.

/// C++ convenience version of gsl_sort_vector2_uchar().  Sorts vectors \a v1
/// and \a v2 in parallel, into ascending order of the elements of \a v1.
///
/// \param[in,out] v1 points at the first vector.
///
/// \param[in,out] v2 points at the second vector.
void
gsl_sort_vector2(gsl_vector_uchar* v1, gsl_vector_uchar* v2)
{
  return gsl_sort_vector2_uchar(v1, v2);
}

/// C++ convenience version of gsl_sort_vector2_short().  Sorts vectors \a v1
/// and \a v2 in parallel, into ascending order of the elements of \a v1.
///
/// \param[in,out] v1 points at the first vector.
///
/// \param[in,out] v2 points at the second vector.
void
gsl_sort_vector2(gsl_vector_short* v1, gsl_vector_short* v2)
{
  return gsl_sort_vector2_short(v1, v2);
}

/// C++ convenience version of gsl_sort_vector2_int().  Sorts vectors \a v1 and
/// \a v2 in parallel, into ascending order of the elements of \a v1.
///
/// \param[in,out] v1 points at the first vector.
///
/// \param[in,out] v2 points at the second vector.
void
gsl_sort_vector2(gsl_vector_int* v1, gsl_vector_int* v2)
{
  return gsl_sort_vector2_int(v1, v2);
}

/// C++ convenience version of gsl_sort_vector2_float().  Sorts vectors \a v1
/// and \a v2 in parallel, into ascending order of the elements of \a v1.
///
/// \param[in,out] v1 points at the first vector.
///
/// \param[in,out] v2 points at the second vector.
void
gsl_sort_vector2(gsl_vector_float* v1, gsl_vector_float* v2)
{
  return gsl_sort_vector2_float(v1, v2);
}

/// C++ convenience version of gsl_sort_vector2_long_double().  Sorts vectors \a
/// v1 and \a v2 in parallel, into ascending order of the elements of \a v1.
///
/// \param[in,out] v1 points at the first vector.
///
/// \param[in,out] v2 points at the second vector.
void
gsl_sort_vector2(gsl_vector_long_double* v1, gsl_vector_long_double* v2)
{
  return gsl_sort_vector2_long_double(v1, v2);
}

#endif
#endif
