#ifndef INCLUDED_PERMUTATIONS_HH
# define INCLUDED_PERMUTATIONS_HH

// This is file permutations.hh.
//
// Copyright 2020-2021 Louis Strous
//
// This file is part of LUX.
//
// LUX is free software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// LUX is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// LUX.  If not, see <http://www.gnu.org/licenses/>.

/// \file
///
/// This file declares functions for converting between permutations and
/// permutation numbers, and for calculating distances between permutations.
///
/// \details
///
/// # Permutations
///
/// A _permutation_ is a specific order in which elements of a collection are
/// arranged.  A _linear_ permutation is one with a definite beginning and end.
/// A _circular_ permutation is one with no definite beginning and end, for
/// example where the elements are arranged in a circle.  Permutations can be
/// specified by _rank_ or by (sorting) _index_.
///
/// The rank says what the relative position of the element's value is in the
/// list of sorted element values.  The element with the least value has rank 0
/// (i.e., we use 0-based ranks).  The rank increases by 1 for each next greater
/// value.  For example, if four elements {A B C D} have associated values {200
/// 34 105 330} then the ranks of the elements are {2 0 1 3}, because element B
/// has the least value (which implies rank 0), element C has the next greater
/// value (rank 1), then comes element A (rank 2), and finally element D (rank
/// 3).
///
/// Any transformation of the values that leaves their ranks intact (the
/// greatest element remains the greatest, and likewise for the 2nd greatest,
/// and so on) also leaves the permutation unchanged.
///
/// If the permutation is defined by indexes then the (sorting) indexes say in
/// what sequence the elements must be arranged to get their associated values
/// in ascending order.  Like for ranks, we use 0-based indexes, so the indexes
/// that arrange the four elements in ascending order of their associated values
/// are {1, 2, 0, 3}, because the element with index 1 (i.e., B) has the least
/// value, the element with index 2 (i.e., C) has the next greater value, then
/// comes the element with index 0 (i.e., A), and the element with index 3
/// (i.e., D) has the greatest value.  If there are \f$n\f$ elements and the
/// value of the element with index \f$i\f$ is \f$v[i]\f$, and if the indexes
/// that define the permutation are \f$s[j]\f$, then \f$v[s[j]]\f$ for \f$j\f$
/// from 0 through \f$n - 1\f$ are in ascending order.
///
/// So the rank-based permutation defined by {2 0 1 3} is the same as the
/// index-based permutation defined by {1 2 0 3}.  Ranks \f$r[i]\f$ and indexes
/// \f$s[i]\f$ are each other's inverse: \f$r[s[i]] = s[r[i]] = i\f$.  The
/// indexes \f$s\f$ can be found from the values \f$v\f$ by sorting via
/// sort_index(): `s = sort_index(v)`.  Then the ranks can be found from the
/// indexes by sorting: `r = sort_index(s)`.  And the indexes can be found from
/// the ranks by sorting: `s = sort_index(r)`.
///
/// To list the elements of a permutation you have to start somewhere.  A linear
/// permutation has a definite start and that is where you start the listing of
/// elements.  A circular permutation has no definite start, so where you start
/// the listing of elements is arbitrary.  Every element of a circular
/// permutation has two neighbors.  The apparently initial and final elements
/// are also next to each other.
///
/// A rank-based circular permutation is one in which the greatest element (the
/// element with the greatest rank) is adjacent not just to the second greatest
/// element but also to the least element (with the least rank).
///
/// Regard the values associated above with the four elements as the angles in
/// degrees of the direction in which each element is visible.  If we start at
/// the direction indicated by 0 degrees and then rotate in the direction of
/// gradually increasing angle, then we encounter the elements in the order B,
/// C, A, D, and then B again.  So element B (rank 0) follows element D (rank
/// 3), and element D precedes element B.
///
/// If we move the zero point of angle forward by 40 degrees without moving the
/// objects at all, then the objects end up at new angle \f$v' = (v − 40) \mod
/// 360\f$.  A, B, C, and D then end up at new angles of 160, 354, 65, 290
/// degrees, with new ranks 1, 3, 0, 2.  In general, the new ranks are \f$r' =
/// (r − ∆r) \mod n\f$ where \f$∆r\f$ is the number of objects that move across
/// the boundary at 0 ≡ 360 degrees.
///
/// If the original ranks of the elements are {2 3 1 0}, then subsequent rank
/// shifts of 1 yield ranks {1 2 0 3}, {0 1 3 2}, {3 0 2 1}, and then again {2 3
/// 1 0}.  All of these sequences define the same rank-based circular
/// permutation.
///
/// For index-based circular permutations the story is basically the same.  Here
/// the arbitrariness is in which element gets index 0.  We started with {A B C
/// D} with element A at index 0.  Perhaps the four elements were affixed to the
/// edge of a rotating disk and when we started looking which elements passed
/// the highest point it was element A that happened to show up first.  But what
/// if we had started looking a little bit later, and it was element B that
/// showed up first?  Then we would have said that the elements are {B C D A},
/// with element B getting index 0.  In general, the new indexes are \f$s' = (s
/// − ∆s) \mod n\f$ where \f$∆s\f$ is the number of objects that rotate from the
/// apparent beginning to the apparent end of the sequence.
///
/// The index sequences {1 2 0 3}, {2 0 3 1}, {0 3 1 2}, and {3 1 2 0} all
/// indicate the same circular permutation, the same one that was specified by
/// rank earlier.
///
/// # Permutation Numbers
///
/// A unique nonnegative number can be associated with each unique linear or
/// circular permutation.  Functions permutation_linear_rank(),
/// permutation_linear_index(), permutation_circular_index() and
/// permutation_circular_rank() calculate a rank- or index-based permutation
/// that is associated with a particular permutation number.  Functions
/// permutation_number_linear_rank(), permutation_number_linear_index(),
/// permutation_number_circular_rank(), and permutation_number_circular_index()
/// calculate the permutation number for the specified permutation.
///
/// The number of unique linear permutations of \f$n\f$ elements is \f$n!\f$,
/// the factorial of \f$n\f$.  The number of unique circular permutations of
/// \f$n\f$ elements is \f$(n - 1)!\f$.
///
/// Here is a table of all 4-element permutations and their linear and circular
/// permutation numbers:
///
/// | Linear | Ranks   | Indexes | Circular |
/// |--------|---------|---------|----------|
/// |      0 | 0 1 2 3 | 0 1 2 3 |        0 |
/// |      1 | 0 1 3 2 | 0 1 3 2 |        3 |
/// |      2 | 0 2 1 3 | 0 2 1 3 |        1 |
/// |      3 | 0 2 3 1 | 0 3 1 2 |        4 |
/// |      4 | 0 3 1 2 | 0 2 3 1 |        2 |
/// |      5 | 0 3 2 1 | 0 3 2 1 |        5 |
/// |      6 | 1 0 2 3 | 1 0 2 3 |        2 |
/// |      7 | 1 0 3 2 | 1 0 3 2 |        5 |
/// |      8 | 1 2 0 3 | 2 0 1 3 |        3 |
/// |      9 | 1 2 3 0 | 3 0 1 2 |        0 |
/// |     10 | 1 3 0 2 | 2 0 3 1 |        4 |
/// |     11 | 1 3 2 0 | 3 0 2 1 |        1 |
/// |     12 | 2 0 1 3 | 1 2 0 3 |        4 |
/// |     13 | 2 0 3 1 | 1 3 0 2 |        1 |
/// |     14 | 2 1 0 3 | 2 1 0 3 |        5 |
/// |     15 | 2 1 3 0 | 3 1 0 2 |        2 |
/// |     16 | 2 3 0 1 | 2 3 0 1 |        0 |
/// |     17 | 2 3 1 0 | 3 2 0 1 |        3 |
/// |     18 | 3 0 1 2 | 1 2 3 0 |        0 |
/// |     19 | 3 0 2 1 | 1 3 2 0 |        3 |
/// |     20 | 3 1 0 2 | 2 1 3 0 |        1 |
/// |     21 | 3 1 2 0 | 3 1 2 0 |        4 |
/// |     22 | 3 2 0 1 | 2 3 1 0 |        2 |
/// |     23 | 3 2 1 0 | 3 2 1 0 |        5 |
///
/// The rank-based circular permutation number is calculated as follows:
///
/// 1. Shift the ranks (as described above) until the last element in the
/// sequence has the greatest rank.
///
/// 2. Calculate the rank-based linear permutation number of the elements
/// excluding the last element.
///
/// For example, the rank-based circular permutation number of {2 3 1 0} is
/// equal to the rank-based circular permutation number of {1 2 0 3}, which by
/// our definition is equal to the rank-based linear permutation number of {1 2
/// 0}.
///
/// The index-based circular permutation number is calculated by converting the
/// indexes to ranks and then using the above algorithm.
///
/// # Permutation Distances
///
/// The _Kendall tau distance_ between two permutations is the least number of
/// swaps of neighbors that are needed to convert the one permutation into the
/// other.  Such distances are calculated by permutation_distance_linear_rank(),
/// permutation_distance_linear_index(), permutation_distance_circular_rank(),
/// and permutation_distance_circular_index().

// configuration include

# include "config.h"

// standard includes

# include <algorithm>            // for std::max_element
# include <limits>               // for std::numeric_limits

// library includes

# include <gslpp_sort.hh>        // for sort_rank

// own includes

# include "action.hh"

// non-template declarations

std::vector<size_t> permutation_circular_index(size_t pn, size_t n);
std::vector<size_t> permutation_circular_rank(size_t pn, size_t n);
std::vector<size_t> permutation_linear_rank(size_t pn, size_t n);
std::vector<size_t> permutation_linear_index(size_t pn, size_t n);
void rotate_ranks_greatest_at_end(std::vector<size_t>& ranks);

// template definitions

/// \brief Returns a number that uniquely identifies a rank-based linear
/// permutation.  See permutations.hh for a discussion of permutations.
///
/// \tparam T is the type of the data values that describe the permutation.  It
/// must support operator<().
///
/// \param[in] values points at the beginning of the array of data values.
/// There are assumed to be no duplicate data values.
///
/// \param[in] n counts the number of data values.
///
/// \returns the permutation number, which is a number between 0 and \f$n! -
/// 1\f$ inclusive.  If a `size_t` is too small to hold \f$n! - 1\f$ then the
/// permutation number wraps around and is no longer unique.
template<typename T>
size_t
permutation_number_linear_rank(const T* values, size_t n)
{
  size_t result = 0;
  for (size_t i = 0; i < n - 1; ++i) {
    for (size_t j = i + 1; j < n; ++j) {
      result += !(values[i] < values[j]);
    }
    result *= (n - i - 1);
  }
  return result;
}

/// Returns a number that uniquely identifies a rank-based linear permutation.
///
/// Is like permutation_number_linear_rank(const T*, size_t) but with the
/// values passed in via a `std::vector`.
///
/// \tparam T is the type of the data values that describe the permutation.  It
/// must support `operator<()`.
///
/// \param[in] values is the vector of data values.  There are assumed to be no
/// duplicate data values.
///
/// \param[in] n counts the number of data values.
///
/// \returns the permutation number, which is a number between 0 and \f$n! -
/// 1\f$ inclusive, if \f$n\f$ is the number of data values.  If a `size_t` is
/// too small to hold \f$n! - 1\f$ then the permutation number wraps around and
/// is no longer unique.
template<typename T>
size_t
permutation_number_linear_rank(const std::vector<T>& values)
{
  return permutation_number_linear_rank(values.data(), values.size());
}

/// Returns a number that uniquely identifies an index-based linear permutation.
/// See permutations.hh for a discussion of permutations.
///
/// \tparam T is the type of the indexes that describe the permutation.  It must
/// support operator<().
///
/// \param[in] i points at the beginning of the array of index values that
/// define the permutation.  There are assumed to be no duplicate index values.
///
/// \param[in] n counts the number of index values.
///
/// \returns the permutation number, which is a number between 0 and \f$n! -
/// 1\f$ inclusive.  If a `size_t` is too small to hold \f$n! - 1\f$ then the
/// permutation number wraps around and is no longer unique.
template<typename T>
size_t
permutation_number_linear_index(const T* i, size_t n)
{
  // convert the indexes to ranks
  std::vector<size_t> r = sort_index(i, n);

  // then return the permutation number based on ranks
  return permutation_number_linear_rank(r);
}

/// Returns a number that uniquely identifies an index-based linear permutation.
///
/// Is like permutation_number_linear_index(const T*, size_t) but with the
/// indexes passed in via a `std::vector`.
///
/// \tparam T is the type of the index values that describe the permutation.  It
/// must support `operator<()`.
///
/// \param[in] i is the vector of index values that define the permutation.
/// There are assumed to be no duplicate data values.
///
/// \returns the permutation number, which is a number between 0 and \f$n! -
/// 1\f$ inclusive, if \f$n\f$ is the number of elements.  If a `size_t` is too
/// small to hold \f$n! - 1\f$ then the permutation number wraps around and is
/// no longer unique.
template<typename T>
size_t
permutation_number_linear_index(const std::vector<T>& i)
{
  // convert the indexes to ranks
  std::vector<size_t> r = sort_index(i);

  // then return the permutation number based on ranks
  return permutation_number_linear_rank(r);
}

/// Returns a number that uniquely identifies a rank-based circular permutation.
/// See permutations.hh for a discussion of permutations.
///
/// \tparam T is the type of the data values that describe the permutation.
///
/// \param[in] v points at the beginning of the array of data values.  The range
/// (difference between greatest and least) of the values is assumed less than
/// the period of the circle.  There are assumed to be no duplicate data values.
///
/// \param[in] n counts the number of data values.
///
/// \returns the rank-based circular permutation number, which is a number
/// between 0 and \f$(n - 1)! - 1\f$ inclusive.
template<typename T>
size_t
permutation_number_circular_rank(const T* v, size_t n)
{
  // get the ranks
  std::vector<size_t> r = sort_rank(v, 1, n);

  // rotate until the last element has the greatest rank
  rotate_ranks_greatest_at_end(r);

  // omit the last element
  r.pop_back();

  // return rank-based linear permutation number of the remaining elements
  return permutation_number_linear_rank(r);
}

/// Returns a number that uniquely identifies a rank-based circular permutation.
///
/// Is like permutation_number_circular_rank(const T*, size_t) but with the
/// values passed in via a `std::vector`.
///
/// \tparam T is the type of the data values that describe the permutation.
///
/// \param[in] v is the vector of data values.  The range (difference between
/// greatest and least) of the values is assumed less than the period of the
/// circle.  There are assumed to be no duplicate data values.
///
/// \returns the rank-based circular permutation number, which is a number
/// between 0 and \f$(n - 1)! - 1\f$ inclusive.
template<typename T>
size_t
permutation_number_circular_rank(const std::vector<T>& v)
{
  return permutation_number_circular_rank(v.data(), v.size());
}

/// Returns a number that uniquely identifies an index-based circular
/// permutation.  See permutations.hh for a discussion of permutations.
///
/// \tparam T is the type of the data values that describe the permutation.
///
/// \param[in] i points at the beginning of the array of indexes that define the
/// permutation.  There are assumed to be no duplicate indexes.
///
/// \param[in] n counts the number of indexes.
///
/// \returns the index-based circular permutation number, which is a number
/// between 0 and \f$(n - 1)! - 1\f$ inclusive.
template<typename T>
size_t
permutation_number_circular_index(const T* i, size_t n)
{
  // get the ranks; sort_index() when applied to indexes inverts them, yielding
  // the ranks
  std::vector<size_t> r = sort_index(i, n);

    // rotate until the last element has the greatest rank
  rotate_ranks_greatest_at_end(r);

  // omit the last element
  r.pop_back();

  // return rank-based linear permutation number of the remaining elements
  return permutation_number_linear_rank(r);
}

/// Returns a number that uniquely identifies an index-based circular
/// permutation.
///
/// Is like permutation_number_circular_index(const T*, size_t) but with the
/// values passed in via a `std::vector`.
///
/// \tparam T is the type of the data values that describe the permutation.
///
/// \param[in] i is a vector of indexes that define the permutation.  There are
/// assumed to be no duplicate indexes.
///
/// \returns the index-based circular permutation number, which is a number
/// between 0 and \f$(n - 1)! - 1\f$ inclusive, if \f$n\f$ is the number of
/// indexes..
template<typename T>
size_t
permutation_number_circular_index(const std::vector<T>& i)
{
  return permutation_number_circular_index(i.data(), i.size());
}

/// Returns the Kendall tau distance between two rank-based linear permutations.
/// See permutations.hh for a discussion of permutations.
///
/// \param[in] n is the number of elements in each permutation.
///
/// \param[in] v1 points at the beginning of the values that define the first
/// permutation.
///
/// \param[in] v2 points at the beginning of the values that define the second
/// permutation.
///
/// \returns the nonnegative distance, which is less than \f$n(n - 1)/2\f$ for
/// permutations of \f$n\f$ elements.
template<typename T>
size_t
permutation_distance_linear_rank(size_t n, const T* v1, const T* v2)
{
  // Calculate the number of inversions between v1 and v2
  size_t d = 0;
  for (int i1 = 0; i1 < n - 1; ++i1) {
    for (int i2 = i1 + 1; i2 < n; ++i2) {
      d += (   ((v1[i1] > v1[i2]) && (v2[i1] < v2[i2]))
               || ((v1[i1] < v1[i2]) && (v2[i1] > v2[i2])));
    }
  }
  return d;
}

/// Returns the Kendall tau distance between two rank-based linear permutations.
///
/// Is like permutation_distance_linear_rank(const T*, const T*, size_t) but
/// with the values passed in via a `std::vector`.
///
/// \param[in] v1 contains the values that define the first permutation.
///
/// \param[in] v2 contains the values that define the second permutation.
///
/// \returns the nonnegative distance, which is less than \f$n(n - 1)/2\f$ for
/// permutations of \f$n\f$ elements.
template<typename T>
size_t
permutation_distance_linear_rank(const std::vector<T>& v1,
                                 const std::vector<T>& v2)
{
  if (v1.size() != v2.size())
    return 0;

  return permutation_distance_linear_rank(v1.size(), v1.data(), v2.data());
}

/// Returns the Kendall tau distance between two index-based linear
/// permutations.  See permutations.hh for a discussion of permutations.
///
/// \param[in] n is the number of elements in each permutation.
///
/// \param[in] i1 points at the beginning of the indexes that define the first
/// permutation.
///
/// \param[in] i2 points at the beginning of the indexes that define the second
/// permutation.
///
/// \returns the nonnegative distance, which is less than \f$n(n - 1)/2\f$ for
/// permutations of \f$n\f$ elements.
template<typename T>
size_t
permutation_distance_linear_index(size_t n, const T* i1, const T* i2)
{
  std::vector<size_t> r1 = sort_index(i1, 1, n);
  std::vector<size_t> v2 = sort_index(i2, 1, n);

  return permutation_distance_linear_rank<size_t>(r1, v2);
}

/// Returns the Kendall tau distance between two index-based linear
/// permutations.
///
/// Is like permutation_distance_linear_index(const T*, const T*, size_t) but
/// with the values passed in via a `std::vector`.
///
/// \param[in] i1 contains the indexes that define the first permutation.
///
/// \param[in] i2 contains the indexes that define the second permutation.
///
/// \returns the nonnegative distance, which is less than \f$n(n - 1)/2\f$ for
/// permutations of \f$n\f$ elements.
template<typename T>
size_t
permutation_distance_linear_index(const std::vector<T>& i1,
                                  const std::vector<T>& i2)
{
  if (i1.size() != i2.size())
    return 0;

  return permutation_distance_linear_index(i1.size(), i1.data(), i2.data());
}

/// Returns the Kendall tau distance between two rank-based circular
/// permutations.  See permutations.hh for a discussion of permutations.
///
/// \param[in] n is the number of elements in each permutation.
///
/// \param[in] v1 points at the beginning of the values that define the first
/// permutation.
///
/// \param[in] v2 points at the beginning of the values that define the second
/// permutation.
///
/// \returns the nonnegative distance, which is less than \f$(n - 1)(n - 2)/2\f$
/// for permutations of \f$n\f$ elements if \f$n\f$ is 2 or greater, and is 0
/// otherwise.
template<typename T>
size_t
permutation_distance_circular_rank(size_t n, const T* v1, const T* v2)
{
  if (n < 2)
    return 0;

  size_t d_min = std::numeric_limits<size_t>::max();
  std::vector<size_t> r1 = sort_rank(v1, 1, n); // get the ranks
  std::vector<size_t> r2 = sort_rank(v2, 1, n);
  for (size_t i1 = 0; i1 < n; ++i1) {
    for (size_t i2 = 0; i2 < n; ++i2) {
      size_t d = permutation_distance_linear_rank(r1, r2);
      if (d < d_min)
        d_min = d;

      // adjust the r2 ranks for the next circular candidate; calculate (r + 1)
      // mod n
      for (int i = 0; i < n; ++i) {
        if (r2[i] == n - 1)
          r2[i] = 0;
        else
          ++r2[i];
      }
    }
    // adjust the r1 ranks for the next circular candidate
    for (int i = 0; i < n; ++i) {
      if (r1[i] == n - 1)
        r1[i] = 0;
      else
        ++r1[i];
    }
  }
  return d_min;
}

/// Returns the Kendall tau distance between two rank-based circular
/// permutations.
///
/// Is like permutation_distance_circular_rank(const T*, const T*, size_t) but
/// with the values passed in via a `std::vector`.
///
/// \param[in] v1 contains the values that define the first permutation.
///
/// \param[in] v2 contains the values that define the second permutation.
///
/// \returns the nonnegative distance, which is less than \f$(n - 1)(n - 2)/2\f$
/// for permutations of \f$n\f$ elements if \f$n\f$ is 2 or greater, and is 0
/// otherwise.
template<typename T>
size_t
permutation_distance_circular_rank(const std::vector<T>& v1,
                                   const std::vector<T>& v2)
{
  if (v1.size() != v2.size())
    return 0;

  return permutation_distance_circular_rank(v1.size(), v1.data(), v2.data());
}

/// Returns the Kendall tau distance between two index-based circular
/// permutations.  See permutations.hh for a discussion of permutations.
///
/// \param[in] n is the number of elements in each permutation.
///
/// \param[in] i1 points at the beginning of the indexes that define the first
/// permutation.
///
/// \param[in] i2 points at the beginning of the indexes that define the second
/// permutation.
///
/// \returns the nonnegative distance, which is less than \f$(n - 1)(n - 2)/2\f$
/// for permutations of \f$n\f$ elements if \f$n\f$ is 2 or greater, and is 0
/// otherwise.
template<typename T>
size_t
permutation_distance_circular_index(size_t n, const T* i1, const T* i2)
{
  std::vector<size_t> r1 = sort_index(i1, 1, n);
  std::vector<size_t> r2 = sort_index(i2, 1, n);

  return permutation_distance_circular_rank<size_t>(r1, r2);
}

/// Returns the Kendall tau distance between two index-based circular
/// permutations.
///
/// Is like permutation_distance_circular_index(const T*, const T*, size_t) but
/// with the values passed in via a `std::vector`.
///
/// \param[in] i1 contains the indexes that define the first permutation.
///
/// \param[in] i2 contains the indexes that define the second permutation.
///
/// \returns the nonnegative distance, which is less than \f$(n - 1)(n - 2)/2\f$
/// for permutations of \f$n\f$ elements if \f$n\f$ is 2 or greater, and is 0
/// otherwise.
template<typename T>
size_t
permutation_distance_circular_index(const std::vector<T>& i1,
                                    const std::vector<T>& i2)
{
  if (i1.size() != i2.size())
    return 0;

  return permutation_distance_circular_index(i1.size(), i1.data(), i2.data());
}

#endif
