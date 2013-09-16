/* This is file lux_structures.h.

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
/* stuff to make Shine's file compile in Strous LUX without modifications */
#include "action.h"

Int	execute_error(Int), file_open_error(void);
#define lux_subr	subroutine
#define lux_func	function
#define lux_subr_struct	internalRoutineStruct
#define STROUS	1

struct sdesc { Int n; Byte *p; };

/* declarations for use in motif.c */
Int	ck_motif(void), set_textfontlist(Int), setup_colors(Int),
  set_fontlist(Int), compile(char *), get_widget_id(Int, Int *),
  rows_or_columns(Int, Int [], Int), lux_int_xmscrollbar(Int, Int [], Int),
  ck_window(Int), lux_xdelete(Int, Int []),
  lux_xmradiobox(Int, Int []),
  set_labelfontlist(Int), lux_int_xmscale(Int, Int [], Int),
  colorset(Int, Int [], Int), ck_events(void), redef_string(Int, Int),
  lux_xtloop(Int, Int []), compileString(char *);
void	color_not_available(void), font_not_available(void), lux_xminit(void),
  lux_callback_execute(Int);
#define lux_execute_symbol(symbol,u)		compile(symbol)
/*#define bcopy(src, dst, len)	memcpy(dst, src, len)*/
#define strarr_scratch(ndim,dims)	array_scratch(LUX_STRING_ARRAY,ndim,dims)
#define redef_strarr(sym, ndim, dim) redef_array(sym, LUX_STRING_ARRAY, ndim, dim)
#define find_sym(string)	findVarName(string, 0)
#define execute_string(string)	compileString(string)
#define num_lux_subr		nSubroutine
#define num_lux_func		nFunction
