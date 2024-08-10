/// \file
///
/// This file defines various functions related to matrices.

/* This is file matrix.cc.

Copyright 2013-2024 Louis Strous

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
// HEADERS
#include <malloc.h> // for malloc free
#include <string.h> // for memcpy
// END HEADERS
#include "config.h"
#include "action.hh"
#include <algorithm>
#include <errno.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_linalg.h>

/// Implements LUX function `mproduct` that calculates the matrix product of two
/// matrices.
Symbol
lux_matrix_product(ArgumentCount narg, Symbol ps[])
{
  Pointer* ptrs;
  LoopInfo* infos;

  StandardArguments sa(narg, ps, "i>D*;i>D*;rD1", &ptrs, &infos);
  int32_t iq = sa.result();
  if (iq < 0)
    return LUX_ERROR;

  int32_t* dims1;
  int32_t* dims2;
  if (infos[0].ndim >= 2)
    dims1 = infos[0].dims;
  else
    return luxerror("Need at least 2 dimensions", ps[0]);
  if (infos[1].ndim >= 2)
    dims2 = infos[1].dims;
  else
    return luxerror("Need at least 2 dimensions", ps[1]);

  // the elements are stored in column-major order
  /* dimsX[0] = number of rows
     dimsX[1] = number of columns
     the number of columns of argument 1 must equal the number of rows
     of argument 2 */
  if (dims1[1] != dims2[0])
    return luxerror("The number of columns (now %d) of the 1st argument must "
                    "equal the number of rows (now %d) of the 2nd argument",
                    ps[1], dims1[1], dims2[0]);

  std::vector<int32_t> tdims { dims1[0], dims2[1] };
  if (internalMode & 1)         // /OUTER
  {
    // result gets dimensions:
    // (1) #rows of argument 1:  dims1[0]
    // (2) #cols of argument 2:  dims2[1]
    // (3) dims of argument 1 beyond the first 2
    // (4) dims of argument 2 beyond the first 2
    auto tndim = infos[0].ndim + infos[1].ndim - 2;
    if (tndim > MAX_DIMS)
      return luxerror("Result would have %d dimensions, but at most %d"
                      " are allowed", ps[1], tndim, MAX_DIMS);
    tdims.insert(tdims.end(), infos[0].dims + 2,
                 infos[0].dims + infos[0].ndim);
    tdims.insert(tdims.end(), infos[1].dims + 2,
                 infos[1].dims + infos[1].ndim);
  } else                        // /INNER
  {
    // result gets dimensions:
    // (1) #rows of argument 1:  dims1[0]
    // (2) #cols of argument 2:  dims2[1]
    // (3) dims of argument 1 beyond the first 2,
    //     which must equal the dims of argument 2 beyond the first 2
    if (infos[1].ndim != infos[0].ndim)
      return luxerror("Needs the same number of dimensions as "
                      "the previous argument", ps[1]);
    for (int i = 2; i < infos[0].ndim; i++)
      if (dims1[i] != dims2[i])
        return luxerror("The dimensions beyond the first two must be the "
                        "same as in the previous argument", ps[1]);
    tdims.insert(tdims.end(), infos[0].dims + 2,
                 infos[0].dims + infos[0].ndim);
  }

  standard_redef_array(iq, LUX_DOUBLE, tdims.size(), tdims.data(), 0, NULL,
                       infos[2].mode, &ptrs[2], &infos[2]);
  {
    auto i2 = make_iota<int32_t>(2);
    infos[0].setAxes(i2, SL_EACHBLOCK);
    infos[1].setAxes(i2, SL_EACHBLOCK);
    infos[2].setAxes(i2, SL_EACHBLOCK);
  }
  if (internalMode & 1) {       // /OUTER
    do
    {
      do
      {
        for (int r1 = 0; r1 < dims1[0]; r1++) // rows of #1
          for (int c2 = 0; c2 < dims2[1]; c2++) // columns of #2
          {
            double* p = &ptrs[2].d[r1 + c2*tdims[0]];
            *p = 0.0;
            for (int k = 0; k < dims1[1]; k++) // columns of #1 = rows of #2
              *p += ptrs[0].d[r1 + k*dims1[0]]*ptrs[1].d[k + c2*dims2[0]];
          }
        ptrs[0].d += infos[0].singlestep[2];
        ptrs[2].d += infos[2].singlestep[2];
      } while (infos[2].advanceLoop(&ptrs[2].ui8),
               infos[0].advanceLoop(&ptrs[0].ui8) < infos[0].ndim);
      ptrs[0].d -= infos[0].ndim > 3? infos[0].singlestep[3]: infos[0].nelem;
      ptrs[1].d += infos[1].singlestep[2];
    } while (infos[1].advanceLoop(&ptrs[1].ui8) < infos[1].ndim);
  } else                        // /INNER
  {
    do
    {
      for (int r1 = 0; r1 < dims1[0]; r1++) // rows of #1
        for (int c2 = 0; c2 < dims2[1]; c2++) // columns of #2
        {
          double* p = &ptrs[2].d[r1 + c2*tdims[0]];
          *p = 0.0;
          for (int k = 0; k < dims1[1]; k++) // columns of #1 = rows of #2
            *p += ptrs[0].d[r1 + k*dims1[0]]*ptrs[1].d[k + c2*dims2[0]];
        }
      ptrs[0].d += infos[0].singlestep[2];
      ptrs[1].d += infos[1].singlestep[2];
      ptrs[2].d += infos[2].singlestep[2];
    } while (infos[0].advanceLoop(&ptrs[0].ui8),
             infos[1].advanceLoop(&ptrs[1].ui8),
             infos[2].advanceLoop(&ptrs[2].ui8) < infos[2].ndim);
  }
  return iq;
}
REGISTER(matrix_product, f, mproduct, 2, 2, "0inner:1outer");
//-------------------------------------------------------------------------
#if HAVE_LIBGSL
/// Fill a row-major GSL matrix from a column-major LUX array.
///
/// \param tgt is the GSL matrix to fill.
///
/// \param src points to the beginning of the LUX array.
///
/// \param rows is the count of rows of data.
///
/// \param columns is the count of columns of data.
///
/// \warning Requires \ref ext-lib-gsl.
void
set_gsl_matrix(gsl_matrix* tgt, const double* src, const size_t rows,
               const size_t columns)
{
  size_t ir, ic;
  for (ic = 0; ic < columns; ++ic)
    for (ir = 0; ir < rows; ++ir)
      gsl_matrix_set(tgt, ir, ic, *src++);
}

/// Fill a row-major GSL matrix from the transposed version of a column-major
/// LUX array.
///
/// \param tgt points to the GSL matrix to fill.
///
/// \param src points to the beginning of the LUX array.
///
/// \param rows is the count of rows of data.
///
/// \param columns is the count of columns of data.
///
/// \warning Requires \ref ext-lib-gsl.
void
set_gsl_matrix_transposed(gsl_matrix* tgt, const double* src, const size_t rows,
                          const size_t columns)
{
  size_t ir, ic;
  for (ic = 0; ic < columns; ++ic)
    for (ir = 0; ir < rows; ++ir)
      gsl_matrix_set(tgt, ic, ir, *src++);
}

/// Fill a column-major LUX array from a row-major GSL matrix.
///
/// \param tgt points to the beginning of the LUX array to fill.
///
/// \param src points to the GSL matrix to copy from.
///
/// \param rows is the row count.
///
/// \param columns is the column count.
///
/// \warning Requires \ref ext-lib-gsl.
void
get_gsl_matrix(double* tgt, const gsl_matrix* src, const size_t rows,
               const size_t columns)
{
  size_t ir, ic;
  for (ic = 0; ic < columns; ++ic)
    for (ir = 0; ir < rows; ++ir)
      *tgt++ = gsl_matrix_get(src, ir, ic);
}

/// Fill a column-major LUX array from the transposed version of a row-major GSL
/// matrix.
///
/// \param tgt points to the beginning of the LUX array to fill.
///
/// \param src points to the GSL matrix to copy from.
///
/// \param rows is the row count.
///
/// \param columns is the column count.
///
/// \warning Requires \ref ext-lib-gsl.
void
get_gsl_matrix_transposed(double* tgt, const gsl_matrix* src, const size_t rows,
                          const size_t columns)
{
  size_t ir, ic;
  for (ic = 0; ic < columns; ++ic)
    for (ir = 0; ir < rows; ++ir)
      *tgt++ = gsl_matrix_get(src, ic, ir);
}

/// Gets the thin Singular Value Decomposition \f$X = U S V^t\f$.  If \f$X\f$
/// has \f$r\f$ rows and \f$c\f$ columns, then \f$S\f$ is a square diagonal
/// matrix with side \f$n = \min(r,c)\f$, \f$U\f$ has \f$r\f$ rows and \f$n\f$
/// columns, and \f$V\f$ has \f$c\f$ rows and \f$n\f$ columns.
///
/// \param X points at the first of the input values.
///
/// \param r is the row count.
///
/// \param c is the column count.
///
/// \param U points to the start of the memory where \f$U\f$ is returned.
///
/// \param s points to the start of the memory where only the diagonal elements
/// of \f$S\f$ (i.e., the singular values) are returned.  The non-diagnonal
/// elements are always 0.
///
/// \param V points to the start of the memory where \f$V\f$ is returned.
///
/// \returns 0 if all is well, and non-0 if there is a problem (such as any of
/// the pointers being null).
///
/// \warning Requires \ref ext-lib-gsl.
int32_t
singular_value_decomposition(const double* X, const size_t r, const size_t c,
                             double* U, double* s, double* V)
{
  if (!X || !U || !s || !V || !r || !c)
  {
    errno = EDOM;
    return 1;
  }

  int32_t result;

  // The GSL routine that we use, gsl_linalg_SV_decomp, expects r ≥ c.

  if (r >= c)                   // we can use gsl_linalg_SV_decomp directly
  {
    gsl_matrix* XU_gsl = gsl_matrix_alloc(r, c); // used for X and U
    gsl_vector* s_gsl = gsl_vector_alloc(c);
    gsl_matrix* V_gsl = gsl_matrix_alloc(c, c);
    gsl_vector* work = gsl_vector_alloc(c);

    // GSL matrices are stored in row-major order, but LUX arrays are stored in
    // column-major order.
    set_gsl_matrix(XU_gsl, X, r, c);

    result = gsl_linalg_SV_decomp(XU_gsl, V_gsl, s_gsl, work);

    if (!result)
    {
      get_gsl_matrix(U, XU_gsl, r, c);
      memcpy(s, s_gsl->data, c*sizeof(double));
      get_gsl_matrix(V, V_gsl, c, c);
    }
    gsl_matrix_free(XU_gsl);
    gsl_vector_free(s_gsl);
    gsl_matrix_free(V_gsl);
    gsl_vector_free(work);
  } else                        // r < c
  {
    // We cannot use gsl_linalg_SV_decomp directly because it expects r ≥ c.  We
    // use X transposed.
    gsl_matrix* XV_gsl = gsl_matrix_alloc(c, r); // used for X and transposed V
    gsl_vector* s_gsl = gsl_vector_alloc(r);
    gsl_matrix* U_gsl = gsl_matrix_alloc(r, r);
    gsl_vector* work = gsl_vector_alloc(r);

    set_gsl_matrix_transposed(XV_gsl, X, r, c);

    result = gsl_linalg_SV_decomp(XV_gsl, U_gsl, s_gsl, work);

    if (!result)
    {
      get_gsl_matrix(V, XV_gsl, c, r);
      memcpy(s, s_gsl->data, r*sizeof(double));
      get_gsl_matrix(U, U_gsl, r, r);
    }
    gsl_matrix_free(XV_gsl);
    gsl_vector_free(s_gsl);
    gsl_matrix_free(U_gsl);
    gsl_vector_free(work);
  }
  return result;
}
#endif

/// Implements LUX subroutine `svd` that calculates the thin Singular Value
/// Decomposition.  `svd,X,U,S,V` decomposes `X` into `U # S # transpose(V)`
/// where `S` is a diagonal matrix with the singular values on the diagonal, and
/// `U` and `V` are semi-orthogonal matrices.  If `X` has `r` rows by `c`
/// columns and optionally additional dimensions `e`, i.e., has dimensions
/// `[r,c,e]`, and if `n = min(r,c)` then `U` gets dimensions `[r,n,e]`, `S`
/// gets dimensions `[n,n,e]`, and `V` gets dimensions `[c,n,e]`.  Option
/// /svarray causes S to contain only the singular values, i.e., the diagonal of
/// the matrix.
///
/// \param narg is the number of arguments (LUX symbols).
///
/// \param ps points at the array of arguments.
///
/// \returns #LUX_OK for success, otherwise #LUX_ERROR.
///
/// \warning Requires \ref ext-lib-gsl.
Symbol
lux_svd(ArgumentCount narg, Symbol ps[])
{
#if HAVE_LIBGSL
  Pointer *ptrs;
  LoopInfo *infos;
  int32_t iq;
  const bool return_svarray = (internalMode & 1);

  StandardArguments sa;
  if ((iq = sa.set(narg, ps, "i>D:,:,*;oD&;oD1;oD1", &ptrs, &infos)) < 0)
    return LUX_ERROR;

  // We prepare the output symbols.  The dimensions of X are r,c,e where e
  // stands for "the extra dimensions" that iterate over the different matrices
  // to process.

  // Copy the dimensions from X so we can adjust them as needed to act as
  // dimensions for the output symbols
  std::vector<int32_t> dims(&infos[0].dims[0],
                            &infos[0].dims[0] + infos[0].ndim);

  auto r = dims[0];
  auto c = dims[1];
  auto n = std::min(r, c);

  // create U, r by n by e
  // dims now contains r, c, e
  dims[1] = n;               // r, n, e
  standard_redef_array(ps[1], LUX_DOUBLE, dims.size(), &dims[0],
                       0, NULL, infos[1].mode, &ptrs[1], &infos[1]);

  // create S
  if (return_svarray)         // /svarray; n by e
  {
    standard_redef_array(ps[2], LUX_DOUBLE, dims.size() - 1, &dims[1],
                         0, NULL, infos[2].mode, &ptrs[2], &infos[2]);
  } else                        // n by n by e
  {
    dims[0] = n;                  // n, n, e
    standard_redef_array(ps[2], LUX_DOUBLE, dims.size(), &dims[0],
                         0, NULL, infos[2].mode, &ptrs[2], &infos[2],
                         true);
  }

  // create V, c by n by e
  dims[0] = c;                // c, n, e
  standard_redef_array(ps[3], LUX_DOUBLE, dims.size(), &dims[0],
                       0, NULL, infos[3].mode, &ptrs[3], &infos[3]);

  infos[0].setAxes(2, NULL, SL_EACHBLOCK); // X
  infos[1].setAxes(2, NULL, SL_EACHBLOCK); // U
  if (return_svarray)                    // /svarray
    infos[2].setAxes(1, NULL, SL_EACHBLOCK); // S
  else
    infos[2].setAxes(2, NULL, SL_EACHBLOCK); // S
  infos[3].setAxes(2, NULL, SL_EACHBLOCK); // V

  std::vector<double> s(n);
  do {
    if (singular_value_decomposition(ptrs[0].d, r, c, ptrs[1].d, s.data(),
                                     ptrs[3].d))
      return luxerror("SVD decomposition failed", ps[0]);
    ptrs[0].d += infos[0].singlestep[2];
    ptrs[1].d += infos[1].singlestep[2]; // U
    // fill S
    if (return_svarray)
    {
      memcpy(ptrs[2].d, s.data(), n*sizeof(double));
      ptrs[2].d += infos[2].singlestep[1]; // S
    } else
    {
      double* sp = ptrs[2].d;
      for (int i = 0; i < n; ++i)
      {
        *sp = s[i];
        sp += n + 1;
      }
      ptrs[2].d += infos[2].singlestep[2]; // S
    }
    ptrs[3].d += infos[3].singlestep[2]; // V
  } while (infos[0].advanceLoop(&ptrs[0].ui8) < infos[0].ndim);
  iq = LUX_OK;
  return iq;
#else
  return cerror(NOSUPPORT, 0, "svd", "libgsl");
#endif
}
REGISTER(svd, s, svd, 4, 4, "1svarray");

#if HAVE_LIBGSL
enum Eigenvalues_sort
{
  descending_absolute,
  descending,
  ascending_absolute,
  ascending,
};

/// Calculates the eigenvalues of a symmetric real matrix.
///
/// \param X points at the input values, which form a symmetric matrix.  \a X
/// must not be null.  No check is done if the input really is symmetric.  If it
/// is not, then you may get useless results.
///
/// \param n is the size of the side of the matrix.  The matrix has \a n rows
/// and \a n columns.  \a n must be non-zero.
///
/// \param s points at memory where the \a n found eigenvalues get stored, in
/// descending order of absolute magnitude.  \a s must not be null.
///
/// \param esort says how to sort the found eigenvalues.  The default is
/// Eigenvalues_sort::descending_absolute.
///
/// \returns 0 if all is well, and non-0 if there is a problem (such as any of
/// the pointers being null).
///
/// \warning Requires \ref ext-lib-gsl.
int
eigenvalues_symmetric(const double* X, const size_t n, double* s,
                      Eigenvalues_sort esort = descending_absolute)
{
  if (!X || !s || !n)
  {
    errno = EDOM;
    return 1;
  }

  gsl_matrix* X_gsl = gsl_matrix_alloc(n, n);
  gsl_vector* s_gsl = gsl_vector_alloc(n);
  auto work = gsl_eigen_symm_alloc(n);

  // GSL matrices are stored in row-major order, but LUX arrays are stored in
  // column-major order.
  set_gsl_matrix(X_gsl, X, n, n);

  int result = gsl_eigen_symm(X_gsl, s_gsl, work);

  if (!result)
  {
    // sort the eigenvalues
    switch (esort)
    {
      case ascending:
        std::sort(&s_gsl->data[0], &s_gsl->data[n],
                  [](double lhs, double rhs)
                  { return rhs > lhs; });
        break;
      case ascending_absolute:
        std::sort(&s_gsl->data[0], &s_gsl->data[n],
                  [](double lhs, double rhs)
                  { return std::abs(rhs) > std::abs(lhs); });
        break;
      case descending:
        std::sort(&s_gsl->data[0], &s_gsl->data[n],
                  [](double lhs, double rhs)
                  { return rhs < lhs; });
        break;
      case descending_absolute:
        std::sort(&s_gsl->data[0], &s_gsl->data[n],
                  [](double lhs, double rhs)
                  { return std::abs(rhs) < std::abs(lhs); });
        break;
    }
    memcpy(s, s_gsl->data, n*sizeof(double));
  }

  gsl_matrix_free(X_gsl);
  gsl_vector_free(s_gsl);
  gsl_eigen_symm_free(work);

  return result;
}

/// Calculates the eigenvalues and eigenvectors of a symmetric real matrix.  For
/// a symmetric real matrix \f$X\f$, the eigenvalue decomposition is \f$X = V S
/// V^t\f$ where \f$S\f$ is a diagonal matrix with the eigenvalues on the
/// diagonal, and \f$V\f$ is an orthonormal matrix holding the eigenvectors.
///
/// \param X points at the input values.  \a X must not be null.  Only the lower
/// left triangular part and diagonal of \a X are used, on the assumption that
/// \a X is symmetric so the upper right triangular part is equal to the
/// transposition of the lower left triangular part.
///
/// \param n is the size of the side of the matrix.  The matrix has \a n rows
/// and \a n columns.  \a n must be non-zero.
///
/// \param s points at memory where the \a n found eigenvalues get stored, in
/// descending order of absolute magnitude.  \a s must not be null.
///
/// \param V points at memory where the eigenvectors get stored in a matrix of
/// \a n by \a elements.
///
/// \returns 0 if all is well, and non-0 if there is a problem (such as any of
/// the pointers being null).
///
/// \warning Requires \ref ext-lib-gsl.
int
eigensystem_symmetric(const double* X, const size_t n, double* s, double* V)
{
  if (!X || !s || !V || !n)
  {
    errno = EDOM;
    return 1;
  }

  gsl_matrix* X_gsl = gsl_matrix_alloc(n, n);
  gsl_vector* s_gsl = gsl_vector_alloc(n);
  gsl_matrix* V_gsl = gsl_matrix_alloc(n, n);
  auto work = gsl_eigen_symmv_alloc(n);

  // GSL matrices are stored in row-major order, but LUX arrays are stored in
  // column-major order.
  set_gsl_matrix(X_gsl, X, n, n);

  int result = gsl_eigen_symmv(X_gsl, s_gsl, V_gsl, work);

  if (!result)
  {
    gsl_eigen_symmv_sort(s_gsl, V_gsl, GSL_EIGEN_SORT_ABS_DESC);
    get_gsl_matrix(V, V_gsl, n, n);
    memcpy(s, s_gsl->data, n*sizeof(double));
  }

  gsl_matrix_free(X_gsl);
  gsl_vector_free(s_gsl);
  gsl_matrix_free(V_gsl);
  gsl_eigen_symmv_free(work);

  return result;
}
#endif

/// Implements LUX subroutine `eigensystem` that returns the eigenvalues and
/// eigenvectors of a real symmetric matrix.  `eigensystem,X,S,V` decomposes `X`
/// into `V # S # V` where `S` contains diagnonal matrices and `V` contains
/// orthonormal matrices.  `X` must have at least two dimensions.  The first two
/// dimensions must be equal and represent a matrix to decompose.  Any remaining
/// dimensions iterate over those matrices.  `S` and `V` get the same dimensions
/// as `V`.  The diagonal of `S` contains the eigenvalues in descending order of
/// absolute magnitude.
///
/// \param narg is the number of arguments.
///
/// \param ps points at the arguments.
///
/// \warning Requires \ref ext-lib-gsl.
Symbol
lux_eigensystem(ArgumentCount narg, Symbol ps[])
{
#if HAVE_LIBGSL
  Pointer *ptrs;
  LoopInfo *infos;
  int32_t iq;

  StandardArguments sa;
  if ((iq = sa.set(narg, ps, "i>D:,:,*;oD&;oD&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  if (infos[0].dims[0] != infos[0].dims[1])
  {
    return luxerror("First two dimensions must be equal", ps[0]);
  }

  // We prepare the output symbols, which have the same dimensions as the input
  // symbol.

  // create S
  standard_redef_array(ps[1], LUX_DOUBLE, infos[0].ndim, &infos[0].dims[0], 0,
                       NULL, infos[1].mode, &ptrs[1], &infos[1]);
  lux_zero(1, &ps[1]);          // we need the off-diagonal elements to be 0

  // create V
  standard_redef_array(ps[2], LUX_DOUBLE, infos[0].ndim, &infos[0].dims[0], 0,
                       NULL, infos[2].mode, &ptrs[2], &infos[2]);

  infos[0].setAxes(2, NULL, SL_EACHBLOCK); // X
  infos[1].setAxes(2, NULL, SL_EACHBLOCK); // S
  infos[2].setAxes(2, NULL, SL_EACHBLOCK); // V

  const size_t n = infos[0].dims[0];
  std::vector<double> s(n);
  do {
    if (eigensystem_symmetric(ptrs[0].d, n, s.data(), ptrs[2].d))
      return luxerror("Eigenvalue/vector determination failed", ps[0]);

    ptrs[0].d += infos[0].singlestep[2];
    // fill S
    double* sp = ptrs[1].d;
    for (int i = 0; i < n; ++i)
    {
      *sp = s[i];
      sp += n + 1;
    }
    ptrs[1].d += infos[1].singlestep[1]; // S
    ptrs[2].d += infos[2].singlestep[2]; // V
  } while (infos[0].advanceLoop(&ptrs[0].ui8) < infos[0].ndim);
  iq = LUX_OK;
  return iq;
#else
  return cerror(NOSUPPORT, 0, "eigenvalues", "libgsl");
#endif
}
REGISTER(eigensystem, s, eigensystem, 2, 4, NULL);

/// Implements LUX function `eigenvalues` that returns the eigenvalues of a real
/// symmetric matrix.  `eigenvalues(X)` returns the eigenvalues in descending
/// order of absolute magnitude.  `X` must have at least two dimensions.  The
/// first two dimensions must be equal and represent a matrix for which to
/// determine the eigenvalues.  Any remaining dimensions iterate over those
/// matrices.  If the dimensions of `X` are `[n,n,e]` then the dimensions of the
/// returned symbol are `[n,e]`.
///
/// \param narg is the number of arguments.
///
/// \param ps points at the arguments.
///
/// \warning Requires \ref ext-lib-gsl.
Symbol
lux_eigenvalues(ArgumentCount narg, Symbol ps[])
{
#if HAVE_LIBGSL
  Pointer *ptrs;
  LoopInfo *infos;
  int32_t iq;

  StandardArguments sa;
  if ((iq = sa.set(narg, ps, "i>D:,:,*;rD-,=,&", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  if (infos[0].dims[0] != infos[0].dims[1])
  {
    return luxerror("First two dimensions must be equal", ps[0]);
  }

  infos[0].setAxes(2, NULL, SL_EACHBLOCK); // X
  infos[1].setAxes(1, NULL, SL_EACHBLOCK); // return symbol

  const size_t n = infos[0].dims[0];
  const auto esort = static_cast<Eigenvalues_sort>(internalMode & 3);
  do {
    if (eigenvalues_symmetric(ptrs[0].d, n, ptrs[1].d, esort))
      return luxerror("Eigenvalue determination failed", ps[0]);

    ptrs[0].d += infos[0].singlestep[2]; // X
    ptrs[1].d += infos[1].singlestep[1]; // return symbol
  } while (infos[0].advanceLoop(&ptrs[0].ui8) < infos[0].ndim);
  return iq;
#else
  return cerror(NOSUPPORT, 0, "eigenvalues", "libgsl");
#endif
}
REGISTER(eigenvalues, f, eigenvalues, 1, 1, "0descending:2ascending:0absolute:1relative:");

/// Produces a transposed matrix.
///
/// \param in points at the input matrix and must not be null.
///
/// \param out points at the output matrix, which must preexist and have the
/// same dimensions as the input matrix.
///
/// \param ncol is the column count.
///
/// \param nrow is the row count.
///
/// \returns 0 for success, 1 for failure.
int32_t
matrix_transpose(double const *in, double *out, size_t ncol, size_t nrow)
{
  int32_t i, j;

  if (!in || !out || ncol < 1 || nrow < 1) {
    errno = EDOM;
    return 1;
  }
  for (i = 0; i < ncol; i++)
    for (j = 0; j < nrow; j++)
      out[j + i*nrow] = in[i + j*ncol];
  return 0;
}

/// Implements the LUX function `transpose` which produces a transposed version
/// of a matrix.
///
/// \param narg is the number of arguments (LUX symbols).
///
/// \param ps points at the array of arguments.
///
/// \returns the Symbol holding the transposed matrix.
Symbol
lux_transpose_matrix(ArgumentCount narg, Symbol ps[])
{
  Pointer *ptrs;
  LoopInfo *infos;
  int32_t iq;
  int32_t n;
  int32_t *dims;

  StandardArguments sa;
  if ((iq = sa.set(narg, ps, "i>D*;rD1", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  if (infos[0].ndim < 2) {
    return luxerror("Need at least 2 dimensions", ps[0]);
  }
  dims = (int32_t*) malloc(infos[0].ndim*sizeof(int32_t));
  dims[0] = infos[0].dims[1];
  dims[1] = infos[0].dims[0];
  memcpy(dims + 2, infos[0].dims + 2, (infos[0].ndim - 2)*sizeof(int32_t));
  standard_redef_array(iq, LUX_DOUBLE, infos[0].ndim, dims, 0, NULL,
                       infos[1].mode, &ptrs[1], &infos[1]);
  {
    auto i2 = make_iota<int32_t>(2);
    infos[0].setAxes(i2, SL_EACHBLOCK);
    infos[1].setAxes(i2, SL_EACHBLOCK);
  }
  n = infos[0].dims[0]*infos[0].dims[1];
  do {
    matrix_transpose(ptrs[0].d, ptrs[1].d, infos[0].dims[0], infos[0].dims[1]);
    ptrs[0].d += n;
    ptrs[1].d += n;
  } while (infos[0].advanceLoop(&ptrs[0].ui8),
           infos[1].advanceLoop(&ptrs[1].ui8) < infos[1].rndim);
  return iq;
}
REGISTER(transpose_matrix, f, transpose, 1, 1, NULL);

// Implements LUX function `mdiagonal` which produces a diagonal square or
// non-square matrix.
//
// `mdiagonal(v,r,c)` produces an array of `r` rows and `c` columns with the
// elements of `v` on its diagonal.  `r` defaults to the count of elements in
// `v`, and `c` defaults to `r`.  It is an error if the count of elements in `v`
// is greater than `r` and than `c`.
///
/// \param narg is the number of arguments (LUX symbols).
///
/// \param ps points at the array of arguments.
///
/// \returns the Symbol holding the diagonal matrix.
Symbol
lux_diagonal_matrix(ArgumentCount narg, Symbol ps[])
{
  Pointer *ptrs;
  LoopInfo *infos;
  int32_t iq;

  StandardArguments sa;
  // mdiagonal(diagonal_values [, rowcount [, colcount]])
  if ((iq = sa.set(narg, ps, "i>D*;iL?;iL?;rD1", &ptrs, &infos)) < 0)
    return LUX_ERROR;

  int32_t dims[2];
  if (narg > 1)
  {                             // have rowcount
    dims[0] = int_arg(ps[1]);
    if (dims[0] < infos[0].nelem)
      return luxerror("Row count cannot be less than diagonal values' count",
                      ps[1]);
    if (narg > 2)
    {                           // have colcount
      dims[1] = int_arg(ps[2]);
      if (dims[1] < infos[0].nelem)
        return luxerror("Column count cannot be less than "
                        "diagonal values' count", ps[2]);
    } else
      dims[1] = dims[0];
  } else
    dims[0] = dims[1] = infos[0].nelem;
  standard_redef_array(iq, LUX_DOUBLE, 2, dims, 0, NULL, infos[1].mode,
                       &ptrs[1], &infos[1]);
  lux_zero(1, &iq);
  for (int i = 0; i < infos[0].nelem; i++)
    ptrs[1].d[i + i*dims[0]] = ptrs[0].d[i];
  return iq;
}
REGISTER(diagonal_matrix, f, mdiagonal, 1, 3, NULL);
