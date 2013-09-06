/* stuff to make Shine's file compile in Strous ANA without modifications */
/*#define printf printf*/		/* so stdarg does not get included
				   by anaparser.h */
#include "action.h"

Int	execute_error(Int), file_open_error(void);
#define ana_subr	subroutine
#define ana_func	function
#define ana_subr_struct	internalRoutineStruct
#define STROUS	1

struct sdesc { Int n; Byte *p; };

/* declarations for use in motif.c */
Int	ck_motif(void), set_textfontlist(Int), setup_colors(Int),
  set_fontlist(Int), compile(char *), get_widget_id(Int, Int *),
  rows_or_columns(Int, Int [], Int), ana_int_xmscrollbar(Int, Int [], Int),
  ck_window(Int), ana_xdelete(Int, Int []),
  ana_xmradiobox(Int, Int []),
  set_labelfontlist(Int), ana_int_xmscale(Int, Int [], Int),
  colorset(Int, Int [], Int), ck_events(void), redef_string(Int, Int),
  ana_xtloop(Int, Int []), compileString(char *);
void	color_not_available(void), font_not_available(void), ana_xminit(void),
  ana_callback_execute(Int);
#define ana_execute_symbol(symbol,u)		compile(symbol)
/*#define bcopy(src, dst, len)	memcpy(dst, src, len)*/
#define strarr_scratch(ndim,dims)	array_scratch(ANA_STRING_ARRAY,ndim,dims)
#define redef_strarr(sym, ndim, dim) redef_array(sym, ANA_STRING_ARRAY, ndim, dim)
#define find_sym(string)	findVarName(string, 0)
#define execute_string(string)	compileString(string)
#define num_ana_subr		nSubroutine
#define num_ana_func		nFunction
