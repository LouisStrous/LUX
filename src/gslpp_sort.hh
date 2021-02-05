#ifndef INCLUDED_GSLPP_SORT_HH
# define INCLUDED_GSLPP_SORT_HH

# include "config.h"
# if HAVE_LIBGSL

#  include <cstddef>            // for size_t
#  include <vector>
#  include <gsl/gsl_sort.h>

/// \file
/// C++ GSL bindings for gsl_sort.h.
///
/// Many [GNU Scientific Library](https://www.gnu.org/software/gsl/) (GSL)
/// functions are available for different data types (`unsigned char`, `short`,
/// `int`, `long`, `float`, `double`, sometimes `long double`).  Because GSL is
/// implemented in C, each of those related functions has a different name which
/// includes an indication of the data type, except that that indication is
/// omitted if the data type is `double`.  In C++ it is possible to give those
/// related functions the same name.  This file provides such definitions, so
/// that you can use the GSL function name without a type indication also for
/// data types other than `double`.
///
/// All `void gsl_sort_T_index(size_t* p, const T* data, size_t stride, size_t
/// n)` GSL functions are made available here under the name `gsl_sort_index`,
/// which is the original GSL function name when `T` = `double`.  Additionally,
/// `std::vector<size_t> sort_index(const T* data, size_t stride, size_t n)` and
/// `std::vector<size_t> sort_index(const std::vector<T>& data)` are made
/// available, which do the same as `gsl_sort_index()` but based on
/// `std::vector`.

/// C++ convenience version of gsl_sort_char_index().  Calculates the zero-based
/// indexes into a sequence that produces the elements in ascending order.
/// After `gsl_sort_index(p, data, stride, n)`, `data[p[i]*stride]` for `i` from
/// 0 through `n` - 1 are sorted in ascending order.
///
/// \param[out] p points at the array where the indexes get stored.
///
/// \param[in] data points at the first data element to sort.
///
/// \param[in] stride is the number of elements of \a data to advance to get the
/// next element to include in the sort.
///
/// \param[in] n is the number of data elements to sort.
void
gsl_sort_index(size_t* p, const char* data, size_t stride, size_t n);

/// C++ convenience version of gsl_sort_uchar_index().  Calculates the
/// zero-based indexes into a sequence that produces the elements in ascending
/// order.  After `gsl_sort_index(p, data, stride, n)`, `data[p[i]*stride]` for
/// `i` from 0 through `n` - 1 are sorted in ascending order.
///
/// \param[out] p points at the array where the indexes get stored.
///
/// \param[in] data points at the first data element to sort.
///
/// \param[in] stride is the number of elements of \a data to advance to get the
/// next element to include in the sort.
///
/// \param[in] n is the number of data elements to sort.
void
gsl_sort_index(size_t* p, const unsigned char* data, size_t stride, size_t n);

/// C++ convenience version of gsl_sort_short_index().  Calculates the
/// zero-based indexes into a sequence that produces the elements in ascending
/// order.  After `gsl_sort_index(p, data, stride, n)`, `data[p[i]*stride]` for
/// `i` from 0 through `n` - 1 are sorted in ascending order.
///
/// \param[out] p points at the array where the indexes get stored.
///
/// \param[in] data points at the data elements to sort.
///
/// \param[in] stride is the number of elements of \a data to advance to get the
/// next element to include in the sort.
///
/// \param[in] n is the number of data elements to sort.
void
gsl_sort_index(size_t* p, const short* data, size_t stride, size_t n);

/// C++ convenience version of gsl_sort_ushort_index().  Calculates the
/// zero-based indexes into a sequence that produces the elements in ascending
/// order.  After `gsl_sort_index(p, data, stride, n)`, `data[p[i]*stride]` for
/// `i` from 0 through `n` - 1 are sorted in ascending order.
///
/// \param[out] p points at the array where the indexes get stored.
///
/// \param[in] data points at the data elements to sort.
///
/// \param[in] stride is the number of elements of \a data to advance to get the
/// next element to include in the sort.
///
/// \param[in] n is the number of data elements to sort.
void
gsl_sort_index(size_t* p, const unsigned short* data, size_t stride, size_t n);

/// C++ convenience version of gsl_sort_int_index().  Calculates the zero-based
/// indexes into a sequence that produces the elements in ascending order.
/// After `gsl_sort_index(p, data, stride, n)`, `data[p[i]*stride]` for `i` from
/// 0 through `n` - 1 are sorted in ascending order.
///
/// \param[out] p points at the array where the indexes get stored.
///
/// \param[in] data points at the data elements to sort.
///
/// \param[in] stride is the number of elements of \a data to advance to get the
/// next element to include in the sort.
///
/// \param[in] n is the number of data elements to sort.
void
gsl_sort_index(size_t* p, const int* data, size_t stride, size_t n);

/// C++ convenience version of gsl_sort_uint_index().  Calculates the zero-based
/// indexes into a sequence that produces the elements in ascending order.
/// After `gsl_sort_index(p, data, stride, n)`, `data[p[i]*stride]` for `i` from
/// 0 through `n` - 1 are sorted in ascending order.
///
/// \param[out] p points at the array where the indexes get stored.
///
/// \param[in] data points at the data elements to sort.
///
/// \param[in] stride is the number of elements of \a data to advance to get the
/// next element to include in the sort.
///
/// \param[in] n is the number of data elements to sort.
void
gsl_sort_index(size_t* p, const unsigned int* data, size_t stride, size_t n);

/// C++ convenience version of gsl_sort_long_index().  Calculates the zero-based
/// indexes into a sequence that produces the elements in ascending order.
/// After `gsl_sort_index(p, data, stride, n)`, `data[p[i]*stride]` for `i` from
/// 0 through `n` - 1 are sorted in ascending order.
///
/// \param[out] p points at the array where the indexes get stored.
///
/// \param[in] data points at the data elements to sort.
///
/// \param[in] stride is the number of elements of \a data to advance to get the
/// next element to include in the sort.
///
/// \param[in] n is the number of data elements to sort.
void
gsl_sort_index(size_t* p, const long* data, size_t stride, size_t n);

/// C++ convenience version of gsl_sort_ulong_index().  Calculates the
/// zero-based indexes into a sequence that produces the elements in ascending
/// order.  After `gsl_sort_index(p, data, stride, n)`, `data[p[i]*stride]` for
/// `i` from 0 through `n` - 1 are sorted in ascending order.
///
/// \param[out] p points at the array where the indexes get stored.
///
/// \param[in] data points at the data elements to sort.
///
/// \param[in] stride is the number of elements of \a data to advance to get the
/// next element to include in the sort.
///
/// \param[in] n is the number of data elements to sort.
void
gsl_sort_index(size_t* p, const unsigned long* data, size_t stride, size_t n);

/// C++ convenience version of gsl_sort_float_index().  Calculates the
/// zero-based indexes into a sequence that produces the elements in ascending
/// order.  After `gsl_sort_index(p, data, stride, n)`, `data[p[i]*stride]` for
/// `i` from 0 through `n` - 1 are sorted in ascending order.
///
/// \param[out] p points at the array where the indexes get stored.
///
/// \param[in] data points at the data elements to sort.
///
/// \param[in] stride is the number of elements of \a data to advance to get the
/// next element to include in the sort.
///
/// \param[in] n is the number of data elements to sort.
void
gsl_sort_index(size_t* p, const float* data, size_t stride, size_t n);

/// C++ convenience version of gsl_sort_long_double_index().  Returns the
/// indexes into a sequence that produce the elements in ascending order.
///
/// \param[out] p points at the array where the indexes get stored.
///
/// \param[in] data points at the data elements to sort.
///
/// \param[in] stride is the number of elements of \a data to advance to get the
/// next element to include in the sort.
///
/// \param[in] n is the number of data elements to sort.
void
gsl_sort_index(size_t* p, const long double* data, size_t stride, size_t n);

/// Calculates the zero-based indexes into a sequence that produces the elements
/// in ascending order.  After `p = gsl_sort_index(data, stride, n)`,
/// `data[p[i]*stride]` for `i` from 0 through `n` - 1 are sorted in ascending
/// order.
///
/// \param[in] data points at the first data element to sort.
///
/// \param[in] stride is the number of elements of \a data to advance to get the
/// next element to include in the sort.
///
/// \param[in] n is the number of data elements to sort.
///
/// \returns a vector of zero-based indexes.
template<typename T>
std::vector<size_t>
sort_index(const T* data, size_t stride, size_t n)
{
  std::vector<size_t> p(n);
  gsl_sort_index(p.data(), data, stride, n);
  return p;
}

/// Calculates the zero-based indexes into a sequence that produces the elements
/// in ascending order.  After `p = gsl_sort_index(data)`, `data[p[i]]` for `i`
/// from 0 through `n` - 1 are sorted in ascending order.
///
/// \param[in] data is the vector of data values to sort.
///
/// \returns a vector of zero-based indexes.
template<typename T>
std::vector<size_t>
sort_index(const std::vector<T>& data)
{
  return sort_index(data.data(), 1, data.size());
}

/// Calculates the zero-based ranks of the elements.  Similar to
/// gsl_sort_index(), but calculates ranks instead of indexes.  After
/// `sort_rank(p, data, stride, n)`, `p[i]` for `i` from 0 through `n` - 1 is
/// the zero-based rank of `data[i*stride]`.  The least element gets rank 0, the
/// next greater element gets rank 1, and so on.
///
/// \param[in,out] p points at the memory where the ranks get stored.
///
/// \param[in] data points at the first data element to sort.
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
  std::vector<size_t> indexes = sort_index(data, stride, n);

  // 2. Calculate the ranks
  for (size_t i = 0; i < n; ++i) {
    p[indexes[i]] = i;
  }
}

/// Calculates the zero-based ranks of the elements.  Similar to sort_index(),
/// but calculates ranks instead of indexes.  After `p = sort_rank(data, stride,
/// n)`, `p[i]` for `i` from 0 through `n` - 1 is the zero-based rank of
/// `data[i*stride]`.  The least element gets rank 0, the next greater element
/// gets rank 1, and so on.
///
/// \param[in] data points at the first data element to sort.
///
/// \param[in] stride is the number of data elements to advance to get the next
/// element to include in the sort.
///
/// \param[in] n is the number of data elements to sort.
///
/// \returns a vector of zero-based ranks.
template<typename T>
std::vector<size_t>
sort_rank(const T* data, size_t stride, size_t n)
{
  // 1. Determine the indexes
  std::vector<size_t> indexes = sort_index(data, stride, n);

  std::vector<size_t> ranks(n);
  // 2. Calculate the ranks
  for (size_t i = 0; i < indexes.size(); ++i) {
    ranks[indexes[i]] = i;
  }

  return ranks;
}

/// Calculates the zero-based ranks of the elements.  Similar to sort_index(),
/// but calculates ranks instead of indexes.  After `p = sort_rank(data)`,
/// `p[i]` for `i` from 0 through `n` - 1 is the zero-based rank of `data[i]`.
/// The least element gets rank 0, the next greater element gets rank 1, and so
/// on.
///
/// \param[in] data contains the data elements to sort.
///
/// \returns a vector of zero-based ranks.
template<typename T>
std::vector<size_t>
sort_rank(const std::vector<T>& data)
{
  return sort_rank(data.data(), 1, data.size());
}

# endif  // HAVE_LIBGSL
#endif  // INCLUDED_GSLPP_SORT_HH
