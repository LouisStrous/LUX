/* This is file error.h.

Copyright 2013 Louis Strous

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
#include "types.h"
Int cerror(Int, Int, ...);

#define COND_NO_SCAL	0
#define ILL_COMB	1
#define INCMP_DIMS	2
#define ALLOC_ERR	3
#define NO_FLT_COND	4
#define ILL_CLASS	5
#define ILL_TYPE	6
#define ILL_SYM		7
#define ILL_DIM		8
#define MYST_CLASS	9
#define NO_SCAL		10
#define ILL_ARG		11
#define CST_LHS		12
#define ONLY_A_S	13
#define ILL_W_STR	14
#define INCMP_ARR	15
#define ILL_ARG_LIST	16
#define SUBSC_RANGE	17
#define INCMP_INNER_BC	18
#define ANA_SUB_ARG	19
#define NEED_ARR	20
#define N_DIMS_OVR	21
#define DUPL_INDX	22
#define IMPOSSIBLE	23
#define N_STR_DIMS_OVR	24
#define NEED_DIMS	25
#define ARR_SMALL	26
#define WRNG_N_ARG	27
#define ILL_CMB_S_NON_S	28
#define SUBSC_NO_INDX	29
#define ILL_N_SUBSC	30
#define ILL_REARRANGE	31
#define ILL_SUBSC_TYPE	32
#define DIM_SMALL	33
#define ONLY_1_IF_ARR	34
#define NEED_STR	35
#define ILL_LUN		36
#define LUN_CLOSED	37
#define READ_ONLY	38
#define ILL_FORMAT_STR	39
#define UNDEF_ARG	40
#define USED_LUN	41
#define READ_EOF	42
#define POS_ERR		43
#define READ_ERR	44
#define WRITE_ERR	45
#define ERR_OPEN	46
#define INCMP_ARG	47
#define NEED_POS_ARG	48
#define NEED_2D_ARR	49
#define EMPTY_STACK	50
#define NEED_INT_ARG	51
#define INDX_RANGE	52
#define COORD_RANGE	53
#define NEED_1D_ARR	54
#define RET_ARG_NO_ATOM	55
#define NEED_2D_SQ_ARR	56
#define INCMP_LU_RHS	57
#define ILL_POWER	58
#define NEED_1D_2D_ARR	59
#define BAD_CONTOURS	60
#define WIN_NOT_EXIST	61
#define BAD_GRID	62
#define NEED_3x3_ARR	63
#define NEED_NTRV_2D_ARR	64
#define N_ARG_OVR	65
#define ILL_SUBSC_LHS	66
#define WR_N_SUBSC	67
#define BAD_STRUCT_KEY	68
#define ILL_TYPE_IN	69
#define NEED_4x4_ARR	70
#define NEED_3_ARR	71
#define ILL_PROJ_MAT	72
#define NEED_2_ARR	73
#define ILL_COORD_SYS	74
#define WRITE_ONLY	75
#define RANGE_START	76
#define RANGE_END	77
#define ILL_SUBSC	78
#define INCMP_KEYWORD	79
#define NEED_SCAL	80
#define ILL_AXIS	81
#define NEED_NUM_ARR	82
#define NEED_NAMED	83
#define NO_COMPLEX	84
#define ILL_NUM_DIM	85
#define NEED_REAL_SCAL	86
#define NEED_REAL_ARR   87
#define NEED_REAL_ARG   88
#define NO_X11		100
#define NEED_BYTE	118
