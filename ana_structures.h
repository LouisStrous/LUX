/* stuff to make Shine's file compile in Strous ANA without modifications */
/*#define printf printf*/		/* so stdarg does not get included
				   by anaparser.h */
#include "action.h"

int	execute_error(int), file_open_error(void);
#define ana_subr	subroutine
#define ana_func	function
#define ana_subr_struct	internalRoutineStruct
#define STROUS	1

struct sdesc { int n; byte *p; };

/* declarations for use in motif.c */
int	ck_motif(void), set_textfontlist(int), setup_colors(int),
  set_fontlist(int), compile(char *), get_widget_id(int, int *),
  rows_or_columns(int, int [], int), ana_int_xmscrollbar(int, int [], int),
  ck_window(int), ana_xdelete(int, int []),
  ana_xmradiobox(int, int []),
  set_labelfontlist(int), ana_int_xmscale(int, int [], int),
  colorset(int, int [], int), ck_events(void), redef_string(int, int),
  ana_xtloop(int, int []), compileString(char *);
void	color_not_available(void), font_not_available(void), ana_xminit(void),
  ana_callback_execute(int);
#define ana_execute_symbol(symbol,u)		compile(symbol)
/*#define bcopy(src, dst, len)	memcpy(dst, src, len)*/
#define strarr_scratch(ndim,dims)	array_scratch(ANA_STRING_ARRAY,ndim,dims)
#define redef_strarr(sym, ndim, dim) redef_array(sym, ANA_STRING_ARRAY, ndim, dim)
#define find_sym(string)	findVarName(string, 0)
#define execute_string(string)	compileString(string)
#define num_ana_subr		nSubroutine
#define num_ana_func		nFunction
