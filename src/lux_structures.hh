/* This is file lux_structures.hh.

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
// stuff to make Shine's file compile in Strous LUX without modifications
#include "action.hh"

int32_t	execute_error(int32_t), file_open_error(void);
#define lux_subr	subroutine
#define lux_func	function
#define lux_subr_struct	internalRoutineStruct
#define STROUS	1

struct sdesc { int32_t n; uint8_t *p; };

// declarations for use in motif.c
int32_t	ck_motif(void), set_textfontlist(int32_t), setup_colors(int32_t),
  set_fontlist(int32_t), compile(char *), get_widget_id(int32_t, int32_t *),
  rows_or_columns(int32_t, int32_t [], int32_t), lux_int_xmscrollbar(int32_t, int32_t [], int32_t),
  ck_window(int32_t), lux_xdelete(int32_t, int32_t []),
  lux_xmradiobox(int32_t, int32_t []),
  set_labelfontlist(int32_t), lux_int_xmscale(int32_t, int32_t [], int32_t),
  colorset(int32_t, int32_t [], int32_t), ck_events(void), redef_string(int32_t, int32_t),
  lux_xtloop(int32_t, int32_t []), compileString(char *);
void	color_not_available(void), font_not_available(void), lux_xminit(void),
  lux_callback_execute(int32_t);
#define lux_execute_symbol(symbol,u)		compile(symbol)
//#define bcopy(src, dst, len)	memcpy(dst, src, len)
#define strarr_scratch(ndim,dims)	array_scratch(LUX_STRING_ARRAY,ndim,dims)
#define redef_strarr(sym, ndim, dim) redef_array(sym, LUX_STRING_ARRAY, ndim, dim)
#define find_sym(string)	findVarName(string, 0)
#define execute_string(string)	compileString(string)
#define num_lux_subr		nSubroutine
#define num_lux_func		nFunction
