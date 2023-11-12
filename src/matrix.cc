/* This is file matrix.cc.

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
// HEADERS
#include <malloc.h> // for malloc free
#include <string.h> // for memcpy
// END HEADERS
#ifndef HAVE_CONFIG_H
#include "config.h"
#endif
#include "action.hh"
#include <errno.h>
#include <gsl/gsl_linalg.h>

int32_t lux_matrix_product(int32_t narg, int32_t ps[])
{
  Pointer *ptrs;
  LoopInfo *infos;

  StandardArguments sa(narg, ps, "i>D*;i>D*;rD1", &ptrs, &infos);
  int32_t iq = sa.result();
  if (iq < 0)
    return LUX_ERROR;

  int32_t *dims1, *dims2;
  if (infos[0].ndim >= 2)
    dims1 = infos[0].dims;
  else {
    iq = luxerror("Need at least 2 dimensions", ps[0]);
    goto error_2;
  }
  if (infos[1].ndim >= 2)
    dims2 = infos[1].dims;
  else {
    iq = luxerror("Need at least 2 dimensions", ps[1]);
    goto error_2;
  }

  // the elements are stored in column-major order
  /* dimsX[0] = number of columns
     dimsX[1] = number of rows
     the number of columns of argument 1 must equal the number of rows
     of argument 2 */
  if (dims1[0] != dims2[1]) {
    iq = luxerror("The number of columns (now %d) of the 1st argument "
                  "must equal the number of rows (now %d) of the 2nd argument",
                  ps[1], dims1[0], dims2[1]);
    goto error_2;
  }

  int32_t i, *tdims, tndim;
  if (internalMode & 1) {       // /OUTER
    tndim = infos[0].ndim + infos[1].ndim - 2;
    if (tndim > MAX_DIMS) {
      iq = luxerror("Result would have %d dimensions, but at most %d"
                    " are allowed", ps[1], tndim, MAX_DIMS);
      goto error_2;
    }
    tdims = (int32_t*) malloc(tndim*sizeof(int32_t));
    memcpy(tdims + 2, infos[0].dims + 2, (infos[0].ndim - 2)*sizeof(int32_t));
    memcpy(tdims + 2 + infos[0].ndim - 2, infos[1].dims + 2,
           (infos[1].ndim - 2)*sizeof(int32_t));
  } else {                      // /INNER
    tndim = infos[0].ndim;
    tdims = (int32_t*) malloc(tndim*sizeof(int32_t));
    if (infos[1].ndim != infos[0].ndim) {
      iq = luxerror("Needs the same number of dimensions as the previous argument",
                    ps[1]);
      goto error_1;
    }
    for (i = 2; i < infos[0].ndim; i++)
      if (dims1[i] != dims2[i]) {
        iq = luxerror("The dimensions beyond the first two must be the same as "
                      "in the previous argument", ps[1]);
        goto error_1;
      }
    memcpy(tdims + 2, infos[0].dims + 2, (infos[0].ndim - 2)*sizeof(int32_t));
  }
  tdims[0] = dims2[0];
  tdims[1] = dims1[1];

  standard_redef_array(iq, LUX_DOUBLE, tndim, tdims, 0, NULL,
                       infos[2].mode, &ptrs[2], &infos[2]);
  free(tdims);
  {
    auto i2 = make_iota<int32_t>(2);
    infos[0].setAxes(i2, SL_EACHBLOCK);
    infos[1].setAxes(i2, SL_EACHBLOCK);
    infos[2].setAxes(i2, SL_EACHBLOCK);
  }
  int32_t j, k;
  if (internalMode & 1) {       // /OUTER
    do {
      do {
        for (i = 0; i < dims1[1]; i++) // rows of #1
          for (j = 0; j < dims2[0]; j++) { // columns of #2
            double *p = &ptrs[2].d[j + i*dims2[0]];
            *p = 0.0;
            for (k = 0; k < dims1[0]; k++) // columns of #1 = rows of #2
              *p += ptrs[0].d[k + i*dims1[0]]*ptrs[1].d[j + k*dims2[0]];
          }
        ptrs[0].d += infos[0].singlestep[2];
        ptrs[2].d += infos[2].singlestep[2];
      } while (infos[0].advanceLoop(&ptrs[0].ui8),
               infos[2].advanceLoop(&ptrs[2].ui8) < 2);
      ptrs[0].d -= infos[0].ndim > 3? infos[0].singlestep[3]: infos[0].nelem;
      ptrs[1].d += infos[1].singlestep[2];
    } while (infos[1].advanceLoop(&ptrs[1].ui8) < infos[1].ndim);
  } else {                      // /INNER
    do {
      for (i = 0; i < dims1[1]; i++) // rows of #1
        for (j = 0; j < dims2[0]; j++) { // columns of #2
          double *p = &ptrs[2].d[j + i*dims2[0]];
          *p = 0.0;
          for (k = 0; k < dims1[0]; k++) // columns of #1 = rows of #2
            *p += ptrs[0].d[k + i*dims1[0]]*ptrs[1].d[j + k*dims2[0]];
        }
      ptrs[0].d += infos[0].singlestep[2];
      ptrs[1].d += infos[1].singlestep[2];
      ptrs[2].d += infos[2].singlestep[2];
    } while (infos[0].advanceLoop(&ptrs[0].ui8),
             infos[1].advanceLoop(&ptrs[1].ui8),
             infos[2].advanceLoop(&ptrs[2].ui8) < infos[2].ndim);
  }
  return iq;

 error_1:
  free(tdims);
 error_2:
  return iq;
}
REGISTER(matrix_product, f, mproduct, 2, 2, "0inner:1outer");
//-------------------------------------------------------------------------
#if HAVE_LIBGSL
int32_t singular_value_decomposition(double *a_in, size_t ncol, size_t nrow,
                                 double *u_out, double *s_out,
                                 double *v_out)
{
  if (!a_in || !u_out || !s_out || !v_out || !nrow || !ncol) {
    errno = EDOM;
    return 1;
  }

  gsl_matrix *a;
  gsl_matrix *v;
  gsl_vector *s;
  gsl_vector *w;
  int32_t result;
  int32_t nmin, nmax;

  if (ncol <= nrow) {
    nmin = ncol;
    nmax = nrow;

    a = gsl_matrix_alloc(nmax, nmin);
    memcpy(a->data, a_in, nmin*nmax*sizeof(*a_in));

    v = gsl_matrix_alloc(nmin, nmin);
    s = gsl_vector_alloc(nmin);
    w = gsl_vector_alloc(nmin);

    result = gsl_linalg_SV_decomp(a, v, s, w);

    if (!result) {
      memcpy(u_out, a->data, nmin*nmax*sizeof(*u_out));
      memcpy(s_out, s->data, nmin*sizeof(*s_out));
      memcpy(v_out, v->data, nmin*nmin*sizeof(*v_out));
    }
  } else {                      // ncol > nrow
    nmin = nrow;
    nmax = ncol;
    a = gsl_matrix_alloc(nmax, nmin);

    int32_t i, j;

    for (i = 0; i < nmax; i++)
      for (j = 0; j < nmin; j++)
        a->data[i*a->tda + j] = a_in[i + j*nmax];

    v = gsl_matrix_alloc(nmin, nmin);
    s = gsl_vector_alloc(nmin);
    w = gsl_vector_alloc(nmin);

    result = gsl_linalg_SV_decomp(a, v, s, w);

    if (!result) {
      for (i = 0; i < nmax; i++)
        for (j = 0; j < nmin; j++)
          v_out[i + j*nmax] = a->data[i*a->tda + j];
      memcpy(s_out, s->data, nmin*sizeof(*s_out));
      for (i = 0; i < nmin; i++)
        for (j = 0; j < nmin; j++)
          u_out[i + j*nmin] = v->data[i*v->tda + j];
    }
  }
  gsl_matrix_free(a);
  gsl_matrix_free(v);
  gsl_vector_free(s);
  gsl_vector_free(w);
  return result;
}
#endif
//--------------------------------------------------------------------
/*
   SVD,A,U2,S2,V2

   Calculates the 'thin' singular value decomposition of matrix A,
   which is easier to calculate and takes less storage than the 'full'
   singular value decomposition.  If A is an m-by-n matrix (has m rows
   and n columns), then A = U # S # V is the 'full' singular value
   decomposition of A, where U is an m-by-m orthogonal matrix, S is an
   m-by-n diagonal matrix (all elements not on the main diagnonal are
   zero), and V is an n-by-n orthogonal matrix.

   In S2 is returned a vector containing the min(m,n) values from the
   diagnonal of S, in non-increasing order.  All other values of S are
   zero.

   If m = n, then returned U2 and V2 have the same dimensions as A,
   and U2 is U, and V2 is the transpose of V.

   If m > n, then U2 is U except that the rightmost columns of U
   beyond the first min(m,n) columns are omitted.  These correspond to
   the bottommost all-zero rows of S.  U2 has the same dimensions as
   A.  V2 is the transpose of V.

   If m < n, then V2 is V except that the bottommost rows of V beyond
   the first min(m,n) rows are omitted.  These correspond to the
   rightmost all-zero columns of S.  V2 has the same dimensions as
   A.  U2 is the transpose of U.

   If Sd is the square matrix with the values of S2 on its diagnonal,
   then A = U2 # Sd # transpose(V2).
*/
int32_t lux_svd(int32_t narg, int32_t ps[])
{
#if HAVE_LIBGSL
  Pointer *ptrs;
  LoopInfo *infos;
  int32_t iq;

  StandardArguments sa;
  if ((iq = sa.set(narg, ps, "i>D*;oD&;oD1;oD1", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  if (infos[0].ndim < 2) {
    return luxerror("Need at least two dimensions", ps[0]);
  }
  int32_t dims[MAX_DIMS];
  memcpy(dims, infos[0].dims, infos[0].ndim*sizeof(int32_t));
  if (infos[0].dims[0] <= infos[0].dims[1]) {
    dims[1] = dims[0];          // # columns
    // S (vector)
    standard_redef_array(ps[2], LUX_DOUBLE, infos[0].ndim - 1, dims + 1,
                         0, NULL, infos[2].mode, &ptrs[2], &infos[2]);
    standard_redef_array(ps[3], LUX_DOUBLE, infos[0].ndim, dims, 0, NULL,
                         infos[3].mode, &ptrs[3], &infos[3]); // Vt
  } else {
    standard_redef_array(ps[3], LUX_DOUBLE, infos[0].ndim, infos[0].dims,
                         0, NULL, infos[3].mode, &ptrs[3], &infos[3]); // Vt
    dims[0] = dims[1];                                  // # rows
    // S (vector)
    standard_redef_array(ps[2], LUX_DOUBLE, infos[0].ndim - 1, dims + 1,
                         0, NULL, infos[2].mode, &ptrs[2], &infos[2]);
    standard_redef_array(ps[1], LUX_DOUBLE, infos[0].ndim, dims, 0, NULL,
                         infos[1].mode, &ptrs[1], &infos[1]);
  }
  infos[0].setAxes(2, NULL, SL_EACHBLOCK);
  infos[1].setAxes(1, NULL, SL_EACHBLOCK);
  infos[2].setAxes(2, NULL, SL_EACHBLOCK);
  infos[3].setAxes(2, NULL, SL_EACHBLOCK);
  do {
    if (singular_value_decomposition(ptrs[0].d, infos[0].dims[0], // # cols
                                     infos[0].dims[1],          // # rows
                                     ptrs[1].d, ptrs[2].d, ptrs[3].d)) {
      return luxerror("SVD decomposition failed", ps[0]);
    }
    ptrs[0].d += infos[0].singlestep[2];
    ptrs[1].d += infos[1].singlestep[2];
    ptrs[2].d += infos[2].singlestep[1];
    ptrs[3].d += infos[3].singlestep[2];
  } while (infos[0].advanceLoop(&ptrs[0].ui8),
           infos[1].advanceLoop(&ptrs[1].ui8),
           infos[2].advanceLoop(&ptrs[2].ui8),
           infos[3].advanceLoop(&ptrs[3].ui8) < infos[3].ndim);
  iq = LUX_OK;
  return iq;
#else
  return cerror(NOSUPPORT, 0, "SVD", "libgsl");
#endif
}
REGISTER(svd, s, svd, 4, 4, NULL);
//--------------------------------------------------------------------
int32_t matrix_transpose(double *in, double *out, size_t in_ncol, size_t in_nrow)
{
  int32_t i, j;

  if (!in || !out || in_ncol < 1 || in_nrow < 1) {
    errno = EDOM;
    return 1;
  }
  for (i = 0; i < in_ncol; i++)
    for (j = 0; j < in_nrow; j++)
      out[j + i*in_nrow] = in[i + j*in_ncol];
  return 0;
}
//--------------------------------------------------------------------
int32_t lux_transpose_matrix(int32_t narg, int32_t ps[])
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
//--------------------------------------------------------------------
int32_t lux_diagonal_matrix(int32_t narg, int32_t ps[])
{
  Pointer *ptrs;
  LoopInfo *infos;
  int32_t iq;

  StandardArguments sa;
  if ((iq = sa.set(narg, ps, "i>D*;rD1", &ptrs, &infos)) < 0)
    return LUX_ERROR;
  int32_t dims[2];
  dims[0] = dims[1] = infos[0].nelem;
  standard_redef_array(iq, LUX_DOUBLE, 2, dims, 0, NULL, infos[1].mode,
                       &ptrs[1], &infos[1]);
  lux_zero(1, &iq);
  int32_t i;
  for (i = 0; i < infos[0].nelem; i++)
    ptrs[1].d[i + i*infos[0].nelem] = ptrs[0].d[i];
  return iq;
}
REGISTER(diagonal_matrix, f, mdiagonal, 1, 1, NULL);
//--------------------------------------------------------------------
