#ifndef ERROR_HH
#define ERROR_HH
/* This is file error.hh.

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

#include <stdint.h>

int32_t cerror(int32_t, int32_t, ...);

enum ErrorNumbers {
  ALLOC_ERR,
  BAD_CONTOURS,
  BAD_GRID,
  BAD_STRUCT_KEY,
  COND_NO_SCAL,
  CST_LHS,
  DIM_SMALL,
  ERR_OPEN,
  ILL_ARG,
  ILL_ARG_LIST,
  ILL_AXIS,
  ILL_CLASS,
  ILL_CMB_S_NON_S,
  ILL_COMB,
  ILL_COORD_SYS,
  ILL_DIM,
  ILL_LUN,
  ILL_NUM_DIM,
  ILL_N_SUBSC,
  ILL_POWER,
  ILL_REARRANGE,
  ILL_SUBSC,
  ILL_SUBSC_LHS,
  ILL_SUBSC_TYPE,
  ILL_SYM,
  ILL_TYPE,
  ILL_W_STR,
  IMPOSSIBLE,
  INCMP_ARG,
  INCMP_ARR,
  INCMP_DIMS,
  INCMP_KEYWORD,
  INCMP_LU_RHS,
  LUN_CLOSED,
  NEED_1D_2D_ARR,
  NEED_1D_ARR,
  NEED_2D_ARR,
  NEED_2D_SQ_ARR,
  NEED_2_ARR,
  NEED_3_ARR,
  NEED_3x3_ARR,
  NEED_4x4_ARR,
  NEED_ARR,
  NEED_BYTE,
  NEED_INT_ARG,
  NEED_NAMED,
  NEED_NTRV_2D_ARR,
  NEED_NUM_ARR,
  NEED_POS_ARG,
  NEED_REAL_ARR,
  NEED_REAL_SCAL,
  NEED_SCAL,
  NEED_STR,
  NOSUPPORT,
  NO_COMPLEX,
  NO_FLT_COND,
  NO_SCAL,
  NO_X11,
  N_ARG_OVR,
  N_DIMS_OVR,
  N_STR_DIMS_OVR,
  ONLY_1_IF_ARR,
  ONLY_A_S,
  POS_ERR,
  RANGE_END,
  RANGE_START,
  READ_EOF,
  READ_ERR,
  READ_ONLY,
  RET_ARG_NO_ATOM,
  SUBSC_NO_INDX,
  SUBSC_RANGE,
  USED_LUN,
  WRITE_ERR,
  WRITE_ONLY,
  WRNG_N_ARG,
};
#endif
