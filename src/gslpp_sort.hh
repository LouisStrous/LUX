#include "config.h"
#if HAVE_LIBGSL
# include <gsl/gsl_sort.h>

/// \file
/// C++ GSL bindings for gsl_sort.h.
///
/// Many GSL functions are available for different data types (unsigned char,
/// short, int, long, float, double, sometimes long double).  Because GSL is
/// implemented in C, each of those related functions has a different name which
/// includes an indication of the data type, except that that indication is
/// omitted if the data type is `double`.  In C++ it is possible to give those
/// related functions the same name.  This file provides such definitions, so
/// that you can use the GSL function name without a type indication also for
/// data types other than `double`.

/// All `T gsl_sort_T_index(size_t* p, const T* data, size_t stride, size_t n)`
/// GSL functions are available under the name `gsl_sort_index`, which is the
/// GSL function name when `T` = `double`.

/// C++ convenience version of gsl_sort_uchar_index().  Returns the indexes into
/// an array that produce the elements in ascending order.
///
/// \param[in,out] p points at the array where the indexes get stored.
///
/// \param[in] data points at the data elements to sort.
///
/// \param[in] stride is the number of data elements to advance to get the next
/// element to include in the sort.
///
/// \param[in] n is the number of data elements to sort.
void
gsl_sort_index(size_t* p, const unsigned char* data, size_t stride, size_t n)
{
  return gsl_sort_uchar_index(p, data, stride, n);
}

/// C++ convenience version of gsl_sort_short_index().  Returns the indexes into
/// an array that produce the elements in ascending order.
///
/// \param[in,out] p points at the array where the indexes get stored.
///
/// \param[in] data points at the data elements to sort.
///
/// \param[in] stride is the number of data elements to advance to get the next
/// element to include in the sort.
///
/// \param[in] n is the number of data elements to sort.
void
gsl_sort_index(size_t* p, const short* data, size_t stride, size_t n)
{
  return gsl_sort_short_index(p, data, stride, n);
}

/// C++ convenience version of gsl_sort_int_index().  Returns the indexes into
/// an array that produce the elements in ascending order.
///
/// \param[in,out] p points at the array where the indexes get stored.
///
/// \param[in] data points at the data elements to sort.
///
/// \param[in] stride is the number of data elements to advance to get the next
/// element to include in the sort.
///
/// \param[in] n is the number of data elements to sort.
void
gsl_sort_index(size_t* p, const int* data, size_t stride, size_t n)
{
  return gsl_sort_int_index(p, data, stride, n);
}

/// C++ convenience version of gsl_sort_long_index().  Returns the indexes into
/// an array that produce the elements in ascending order.
///
/// \param[in,out] p points at the array where the indexes get stored.
///
/// \param[in] data points at the data elements to sort.
///
/// \param[in] stride is the number of data elements to advance to get the next
/// element to include in the sort.
///
/// \param[in] n is the number of data elements to sort.
void
gsl_sort_index(size_t* p, const long* data, size_t stride, size_t n)
{
  return gsl_sort_long_index(p, data, stride, n);
}

/// C++ convenience version of gsl_sort_float_index().  Returns the indexes into
/// an array that produce the elements in ascending order.
///
/// \param[in,out] p points at the array where the indexes get stored.
///
/// \param[in] data points at the data elements to sort.
///
/// \param[in] stride is the number of data elements to advance to get the next
/// element to include in the sort.
///
/// \param[in] n is the number of data elements to sort.
void
gsl_sort_index(size_t* p, const float* data, size_t stride, size_t n)
{
  return gsl_sort_float_index(p, data, stride, n);
}

/// C++ convenience version of gsl_sort_long_double_index().  Returns the
/// indexes into an array that produce the elements in ascending order.
///
/// \param[in,out] p points at the array where the indexes get stored.
///
/// \param[in] data points at the data elements to sort.
///
/// \param[in] stride is the number of data elements to advance to get the next
/// element to include in the sort.
///
/// \param[in] n is the number of data elements to sort.
void
gsl_sort_index(size_t* p, const long double* data, size_t stride, size_t n)
{
  return gsl_sort_long_double_index(p, data, stride, n);
}

/// Returns the zero-based ranks of array elements.  Similar to
/// gsl_sort_index(), but returns ranks instead of indexes.  GSL does not
/// provide functions like these, hence no `gsl_` prefix.
///
/// \param[in,out] p at the array where the ranks get stored.
///
/// \param[in] data points at the data elements to sort.
///
/// \param[in] stride is the number of data elements to advance to get the next
/// element to include in the sort.
///
/// \param[in] n is the number of data elements to sort.
template<typename T>
void
sort_rank(size_t* p, const T* data, size_t stride, size_t n)
{
  // 1. Determine the indexes
  std::vector<size_t> index(n);
  gsl_sort_index(index.data(), data, stride, n);

  // 2. Calculate the ranks
  for (size_t i = 0; i < n; ++i) {
    p[index[i]] = i;
  }
}

#endif
