/// \file

// standard includes

#include <algorithm>
#include <cassert>
#include <numeric>              // for std::iota
#include <vector>

// own includes

#include "permutations.hh"

/// Ensure that the last rank is the greatest, by circular rotation of rank \a
/// values.  All rank values get adjusted via \f$ (r + ∆r) \bmod n \f$ where \f$
/// r \f$ is the original rank value, \f$ ∆r \f$ is the appropriate shift, and
/// \f$ n \f$ is the number of elements.
///
/// \param ranks contains the 0-based ranks.  If there are \f$ n \f$ elements,
/// then all ranks from 0 through \f$ n - 1 \f$ must be present.
void
rotate_ranks_greatest_at_end(std::vector<size_t>& ranks) {
  auto n = ranks.size();
  // We want the last element to have the greatest rank, which is n - 1.
  auto d = n - 1 - ranks.back();
  if (d) {
    // The last element doesn't have the greatest rank; shift ranks so it does.
    for (int i = 0; i < n; ++i) {
      // effectively, we calculate (ranks[i] + d) mod n
      ranks[i] += d;
      if (ranks[i] >= n)
        ranks[i] -= n;
    }
  }
}

/// Return a rank-based linear permutation corresponding to a permutation
/// number.  See permutations.hh for a discussion of permutations.
///
/// \param[in] pn is the permutation number.  It must be less than \f$n!\f$.
///
/// \param[in] n is the count of permutation elements.
///
/// \returns a vector of \f$n\f$ nonnegative integers that correspond to the
/// given permutation number, or an empty vector if the permutation number is
/// out of range.
std::vector<size_t>
permutation_linear_rank(size_t pn, size_t n)
{
  // 1. convert permutation number into inversion counts
  std::vector<size_t> inversions(n);
  int factor;
  for (factor = 1; factor < n; ++factor) {
    inversions[n - factor] = pn % factor;
    pn /= factor;
  }
  if (pn >= factor) {
    // the permutation number was too large for the specified count; return an
    // empty permutation
    return std::vector<size_t>(0);
  }
  inversions[0] = pn;

  // 2. convert inversion counts into permutation elements
  std::vector<size_t> p(n);
  std::iota(p.begin(), p.end(), 0);
  auto it = p.begin();
  for (int i = 0; i < n - 1; ++i) {
    std::rotate(it, it + inversions[i], it + inversions[i] + 1);
    ++it;
  }
  return p;
}

/// Return an index-based linear permutation corresponding to a permutation
/// number.  See permutations.hh for a discussion of permutations.
///
/// \param[in] pn is the permutation number.  It must be less than \f$n!\f$.
///
/// \param[in] n is the count of permutation elements.
///
/// \returns a vector of \f$n\f$ nonnegative integers that correspond to the
/// given permutation number, or an empty vector if the permutation number is
/// out of range.
std::vector<size_t>
permutation_linear_index(size_t pn, size_t n)
{
  // get the permutation based on ranks
  auto p = permutation_linear_rank(pn, n);

  // then convert ranks to indexes
  p = sort_index(p);

  return p;
}

/// Return a rank-based circular permutation corresponding to a permutation
/// number.  See permutations.hh for a discussion of permutations.
///
/// \param[in] pn is the permutation number.
///
/// \param[in] n is the count of permutation elements.
///
/// \returns a vector of \f$n\f$ nonnegative integers that correspond to the
/// given permutation number.
std::vector<size_t>
permutation_circular_rank(size_t pn, size_t n)
{
  if (n) {
    // the standard circular permutation of <n> elements for permutation number
    // <p> is equal to the standard linear permutation of <n> - 1 elements, with
    // the next higher number (which is <n> - 1) appended.
    auto p = permutation_linear_rank(pn, n - 1);
    if (p.empty()) {
      // the permutation number was too great for the element count
      return p;
    }
    p.push_back(n - 1);
    return p;
  } else {
    // no elements; return empty vector
    return std::vector<size_t>(0);
  }
}

/// Return an index-based circular permutation corresponding to a permutation
/// number.  See permutations.hh for a discussion of permutations.
///
/// \param[in] pn is the permutation number.
///
/// \param[in] n is the count of permutation elements.
///
/// \returns a vector of \f$n\f$ nonnegative integers that correspond to the
/// given permutation number.
std::vector<size_t>
permutation_circular_index(size_t pn, size_t n)
{
  // get the rank-based permutation
  std::vector<size_t> p = permutation_circular_rank(pn, n);

  // then convert ranks to indexes
  p = sort_index(p);

  return p;
}
