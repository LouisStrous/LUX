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
  COND_NO_SCAL,
  ILL_COMB,
  INCMP_DIMS,
  ALLOC_ERR,
  NO_FLT_COND,
  ILL_CLASS,
  ILL_TYPE,
  ILL_SYM,
  ILL_DIM,
  MYST_CLASS,
  NO_SCAL,
  ILL_ARG,
  CST_LHS,
  ONLY_A_S,
  ILL_W_STR,
  INCMP_ARR,
  ILL_ARG_LIST,
  SUBSC_RANGE,
  INCMP_INNER_BC,
  LUX_SUB_ARG,
  NEED_ARR,
  N_DIMS_OVR,
  DUPL_INDX,
  IMPOSSIBLE,
  N_STR_DIMS_OVR,
  NEED_DIMS,
  ARR_SMALL,
  WRNG_N_ARG,
  ILL_CMB_S_NON_S,
  SUBSC_NO_INDX,
  ILL_N_SUBSC,
  ILL_REARRANGE,
  ILL_SUBSC_TYPE,
  DIM_SMALL,
  ONLY_1_IF_ARR,
  NEED_STR,
  ILL_LUN,
  LUN_CLOSED,
  READ_ONLY,
  ILL_FORMAT_STR,
  UNDEF_ARG,
  USED_LUN,
  READ_EOF,
  POS_ERR,
  READ_ERR,
  WRITE_ERR,
  ERR_OPEN,
  INCMP_ARG,
  NEED_POS_ARG,
  NEED_2D_ARR,
  EMPTY_STACK,
  NEED_INT_ARG,
  INDX_RANGE,
  COORD_RANGE,
  NEED_1D_ARR,
  RET_ARG_NO_ATOM,
  NEED_2D_SQ_ARR,
  INCMP_LU_RHS,
  ILL_POWER,
  NEED_1D_2D_ARR,
  BAD_CONTOURS,
  WIN_NOT_EXIST,
  BAD_GRID,
  NEED_3x3_ARR,
  NEED_NTRV_2D_ARR,
  N_ARG_OVR,
  ILL_SUBSC_LHS,
  WR_N_SUBSC,
  BAD_STRUCT_KEY,
  ILL_TYPE_IN,
  NEED_4x4_ARR,
  NEED_3_ARR,
  ILL_PROJ_MAT,
  NEED_2_ARR,
  ILL_COORD_SYS,
  WRITE_ONLY,
  RANGE_START,
  RANGE_END,
  ILL_SUBSC,
  INCMP_KEYWORD,
  NEED_SCAL,
  ILL_AXIS,
  NEED_NUM_ARR,
  NEED_NAMED,
  NO_COMPLEX,
  ILL_NUM_DIM,
  NEED_REAL_SCAL,
  NEED_REAL_ARR,
  NEED_REAL_ARG,
  NO_X11,
  NEED_BYTE,
  NOSUPPORT
};
#endif
