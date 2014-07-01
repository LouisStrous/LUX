/* This is file install.c.

Copyright 2013 Louis Strous, Richard Shine

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
/* File install.c */
#if HAVE_CONFIG_H
#include "config.h"
#endif
#include "install.h"
#include <ctype.h> /* for toupper(11) isdigit(3) */
#include <errno.h> /* for errno(2) */
#include <error.h> /* for luxerror(58) */
#include <float.h> /* for FLT_MAX(2) DBL_MAX(2) DBL_MIN(1) FLT_MIN(1) */
#include <limits.h> /* for UINT8_MAX(3) INT32_MAX(1) INT16_MAX(1) INT16_MIN(1) INT32_MIN(1) */
#include <malloc.h> /* for malloc(25) free(18) realloc(4) */
#include <math.h> /* for j1(12) */
#include <setjmp.h> /* for longjmp(1) jmp_buf(1) setjmp(1) */
#include <signal.h> /* for signal(7) SIG_ERR(6) SIGCONT(3) SIGINT(2) SIGSEGV(2) ... */
#include <stdarg.h> /* for va_arg(39) va_end(14) va_start(2) va_list(2) */
#include <stddef.h> /* for NULL(20) */
#include <stdio.h> /* for printf(44) puts(16) FILE(6) fclose(4) sprintf(3) ... */
#include <stdlib.h> /* for strtol(4) atol(1) */
#include <string.h> /* for index(41) strlen(17) memcpy(12) strcpy(4) strcmp(4) ... */
#include <time.h> /* for CLK_TCK(1) clock(1) time(1) */
#include <unistd.h> /* for pipe(2) execl(1) sbrk(1) */
#include <obstack.h>
/* clock() on mips-sgi-irix64-6.2 is in unistd.h rather than in ANSI time.h */

#include "editor.h"
#include "editorcharclass.h"
#include "action.h"

extern char		*symbolStack[];
extern symTableEntry	sym[];
extern hashTableEntry	*varHashTable[], *subrHashTable[], *funcHashTable[],
			*blockHashTable[];
extern int16_t		listStack[];
extern int32_t		keepEVB;
extern char		*currentChar, line[];
extern FILE		*inputStream, *outputStream;
extern int32_t		nExecuted;
/* extern internalRoutine	subroutine[], function[]; */
internalRoutine *subroutine, *function;

int32_t	luxerror(char *, int32_t, ...), lookForName(char *, hashTableEntry *[], int32_t),
	newSymbol(int32_t, ...), lux_setup();
void	installKeys(void *keys), zerobytes(void *, int32_t);
char	*strsave(char *), *symName(int32_t, hashTableEntry *[]), *className(int32_t),
	*typeName(int32_t);
FILE	*openPathFile(char *, int32_t);

int32_t	nFixed = 0, noioctl = 0, trace = 0, curTEIndex;

char	batch = 0, *currentInputFile = NULL, ignoreSymbols = 0, restart = 0;

int32_t	traceMode = T_FILE | T_LOOP | T_BLOCK | T_ROUTINE;

int16_t	*listStackItem = listStack;

int32_t	symbolStackIndex = 0, tempVariableIndex = TEMPS_START,
	nTempVariable = 0, namedVariableIndex = NAMED_START,
	nNamedVariable = 0, nSymbolStack = 0, executableIndex = EXE_START,
	nExecutable = 0, tempExecutableIndex = TEMP_EXE_START,
	nTempExecutable, zapContext = 0, installString(char *),
	lux_verify(int32_t, int32_t []), eval_func, insert_subr;

int32_t	markStack[MSSIZE], markIndex = 0;

executionLevelInfo	*exInfo = NULL;
int32_t	nexInfo = 0;

extern int32_t	compileLevel, curLineNumber;
static char	installing = 1;

/*----------------------------------------------------------------*/ 
extern int32_t lux_area(), lux_area2(), lux_array_statistics(),
  lux_atomize(), lux_batch(), lux_callig(), lux_close(),
  lux_contour(), lux_coordmap(), lux_crunch(),
  lux_cubic_spline_extreme(), lux_debug(), lux_decomp(),
  lux_decrunch(), lux_default(), lux_delete(), lux_distr(),
  lux_dsolve(), lux_dump(), lux_dump_stack(), lux_echo(),
  lux_noecho(), lux_endian(), lux_erase(), lux_execute(),
  lux_fcwrite(), lux_fileptr(), lux_fileread(), lux_filewrite(),
  lux_format_set(), lux_fprint(), lux_fprintf(), lux_fzread(),
  lux_fzwrite(), lux_fzhead(), lux_getmin9(), lux_help(), lux_hex(),
  lux_inserter(), lux_limits(), lux_noop(), lux_openr(), lux_openu(),
  lux_openw(), lux_oplot(), lux_pdev(), lux_pen(), lux_plot(),
  lux_pointer(), lux_printf(), lux_pop(), lux_postimage(),
  lux_postraw(), postrelease(), lux_push(), lux_quit(), lux_read(),
  lux_readarr(), lux_readf(), lux_readu(), lux_record(), lux_redim(),
  lux_redirect_diagnostic(), lux_arestore(), lux_rewindf(), lux_sc(),
  lux_scb(), lux_setenv(), lux_show(), lux_show_func(),
  lux_show_subr(), lux_spawn(), lux_step(), lux_swab(), lux_switch(),
  lux_type(), lux_trace(), lux_ulib(), lux_wait(), lux_zap(),
  lux_zero(), showstats(), lux_writeu(), lux_system(), lux_freadf(),
  lux_orientation(), lux_error(), site(), lux_set(), lux_tolookup(),
  lux_coordtrf(), lux_fread(), lux_dump_lun(), lux_cluster(),
  lux_astore(), lux_fzinspect(), lux_multisieve(), lux_crunchrun(),
  lux_swaphalf(), lux_chdir(), lux_replace_values(),
  lux_freads(), lux_one(), lux_disableNewline(), lux_enableNewline(),
  lux_shift(), lux_file_to_fz(), lux_zapnan(),
  lux_pencolor(), lux_idlrestore(), lux_list(),
  lux_extract_bits(), lux_fftshift(), lux_manualterm(), lux_watch(),
  lux_fcrunwrite(), lux_fits_read(), lux_fits_write(), lux_subshift(),
  lux_subshiftc(), lux_byte_inplace(), lux_word_inplace(),
  lux_long_inplace(), lux_float_inplace(), lux_double_inplace(),
  lux_cfloat_inplace(), lux_cdouble_inplace(), lux_string_inplace(),
  lux_fade(), lux_fade_init();

int32_t	lux_name();

#if DEVELOP
extern int32_t lux_fitUnitCube(), lux_projection(), lux_plot3d(),
  lux_trajectory(), lux_getmin2(), lux_projectmap();
#endif

#if DEBUG
extern int32_t	checkList(), lux_whereisAddress(), lux_show_temps(),
		lux_newallocs(), show_files();
#endif

#if HAVE_LIBJPEG
extern int32_t	lux_read_jpeg6b(), lux_write_jpeg6b();
#endif

#if HAVE_SYS_MTIO_H
extern int32_t	lux_tape_status(), lux_rewind(), lux_weof(), lux_unload(),
  lux_skipr(), lux_skipf(), lux_taprd(), lux_tapwrt(), lux_tapebufin(),
  lux_tapebufout(), lux_wait_for_tape();
#endif

extern int32_t	lux_gifread(), lux_gifwrite();

#if HAVE_LIBX11
extern int32_t lux_menu(), lux_menu_hide(), lux_menu_item(),
  lux_menu_kill(), lux_menu_pop(), lux_menu_read(),
  lux_register_event(), lux_window(), lux_xcopy(),
  lux_xdelete(), lux_xdrawline(), lux_xevent(), lux_xflush(),
  lux_xfont(),
  lux_xlabel(), lux_xloop(), lux_xopen(), lux_xplace(),
  lux_xport(), lux_xpurge(), lux_xquery(), lux_xsetaction(),
  lux_xsetbackground(), lux_xsetforeground(), lux_xtv(),
  lux_xtvlct(), lux_xtvmap(), lux_xtvraw(), lux_xtvread(),
  lux_xymov(), lux_wait_for_menu(), lux_xclose(), lux_xraise(),
  lux_xcursor(), lux_xanimate(), lux_xzoom(), lux_show_visuals(),
  lux_zoom(), lux_xtvplane(), lux_threecolors(), lux_tv3(),
  lux_xinvertline(), lux_xinvertarc(), lux_xdrawarc(), lux_colorComponents(),
  lux_colorstogrey(), lux_pixelsto8bit();
#endif

#if MOTIF
extern int32_t lux_xmalignment(), lux_xmarmcolor(), lux_xmattach(),
  lux_xmattach_relative(), lux_xmbackgroundcolor(),
  lux_xmbordercolor(), lux_xmborderwidth(), lux_xmbottomshadowcolor(),
  lux_xmdestroy(), lux_xmdrawinglink(), lux_xmfont(),
  lux_xmforegroundcolor(), lux_xmgetpixmap(), lux_xmgetwidgetsize(),
  lux_xmlistadditem(), lux_xmlistdeleteall(), lux_xmlistdeleteitem(),
  lux_xmlistfunc(), lux_xmlistselect(), lux_xmlistsubr(),
  lux_xmmessage(), lux_xmposition(), lux_xmprompt(),
  lux_xmscaleresetlimits(), lux_xmscalesetvalue(),
  lux_xmselectcolor(), lux_xmsensitive(), lux_xmsetcolors(),
  lux_xmsetlabel(), lux_xmsetmargins(), lux_xmsetmnemonic(),
  lux_xmsetmodal(), lux_xmsetoptionselection(), lux_xmsetpixmap(),
  lux_xmtextappend(), lux_xmtexterase(), lux_xmtextfieldseteditable(),
  lux_xmtextfieldsetstring(), lux_xmtextseteditable(),
  lux_xmtextsetposition(), lux_xmtextsetrowcolumnsize(),
  lux_xmtextsetstring(), lux_xmtogglesetstate(),
  lux_xmtopshadowcolor(), lux_xtloop(), lux_xtmanage(),
  lux_xtunmanage(), lux_xmquery(), lux_xmscrollbarsetvalues(),
  lux_xmsetdirectory(), lux_xmsettitle(), lux_xmset_text_output(),
  lux_xmsize(), lux_xmtextfieldsetmaxlength(), lux_xtpopup(),
  lux_xtpopdown(), lux_xmraise(), lux_xmresizepolicy(), lux_xmtextreplace(),
  lux_xmgetwidgetposition(), lux_xminfo();
#endif

extern int32_t lux_readorbits(), lux_showorbits();

extern int32_t	peek();
extern int32_t	lux_breakpoint();
extern int32_t	insert();
int32_t	lux_restart(int32_t, int32_t []);

#if MOTIF
int32_t	lux_zeroifnotdefined(), lux_compile_file();	/* browser */
#endif

#define MAX_ARG	100

/* Each routine's entry in subroutine[] and function[] has the following 
  elements:  name, min_arg, max_arg, function_pointer, keywords 
   min_arg = the lowest number of non-mode arguments that is allowed 
             with the routine 
   max_arg = the highest number of non-mode arguments that is allowed 
  A non-mode argument is one whose entry in the keyword list does not 
  start with a number.  
   function_pointer = pointer to the appropriate C-routine
   keywords = a string that contains the keywords that go with the routine 
  The keyword string syntax is:  
   [*][+][-][|defaultMode|][%offset%][key1][:[key2]:[key3]...] 

  The stuff before the keys must be in the relative order indicated above:
  for instance, -+ is an invalid specification.

  Initial * indicates that the function generally yields a result that
  has the same number of elements as the (first) argument and that
  each element of the result depends only on the corresponding element
  of the argument.  Inclusion of a * may improve memory efficiency.
  However, * items are only indications and even if they are erroneous
  or missing they cannot make performance worse.

  By default, arguments are evaluated before they are passed to the
  selected routine, but initial + suppresses that evaluation (e.g. to
  catch TRANSFERs) and leaves that to the selected routine.

  By default, arguments of class 0 (UNUSED, e.g., a parameter of a function
  for which the user has not specified a value) are transformed in
  internal_routine() to class UNDEFINED.  Initial - indicates that such
  arguments should be removed so that it will be as if the user did
  not specify those arguments.
  
  The number between vertical bars (|) is an integer which indicates
  the default value of global C variable <internalMode> before the mode
  keywords are treated.

  The offset between %% indicates the argument offset of the first
  non-keyword argument (defaults to zero).  This is convenient when a
  routine has a variable number of ordinary arguments plus a fixed
  number of optional arguments.  With %% the optional arguments can be
  put in fixed positions before the variable number of ordinary
  arguments, which simplifies the argument treatment.

  Individual keys have the following syntax:
   [number[$]][#]name 
  Keywords without a number in front of them indicate keys by
  position; the corresponding arguments are entered into the final
  argument list at the position indicated by the position of the key
  in the defining key list.  Keys with a number in front of them are
  mode keys and the appropriate bits in global C variable
  <internalMode> get set, or get reset (if the key is preceded by NO).
  The key is also entered into the argument list by position if the
  number is followed by a dollar sign; otherwise, the key only affects
  <internalMode> and does not count in the number of arguments to the
  routine.  The number sign # indicates that the value of the key must
  be preserved, i.e. not evaluated before the routine is actually
  executed.  The key name MODE is reserved and may not be used in the
  below key lists. 

  EXAMPLES (with a fictional routine FOO)

  "FIRST:SECOND" -> FOO,SECOND=3,FIRST=2 yields argument list 2,3
  ":SECOND" -> FOO,SECOND=3,2 equals FOO,2,SECOND=3 yields 2,3
  ":1FIE:NUMBER" -> FOO,NUMBER=8,/FIE,12 yields 12,8
                         + internalMode set to 1
  "|4|:2FIE:NUMBER" -> FOO,NUMBER=7,/FIE equals yields (none),7
                         + internalMode set to 6 (= 4 OR 2)
  "|6|2FIE" -> FOO,5,/NOFIE yields 5
                         + internalMode set to 4 (= 6 AND NOT 2)
  "%1%SPECIAL" -> FOO,3,2 yields (none),3,2
                  FOO,3,SPECIAL=1 yields 1,3 */

internalRoutine	subroutine_table[] = {
  { "%INSERT",	3, MAX_ARG, insert, /* execute.c */
    "1INNER:2OUTER:4ONEDIM:8SKIPSPACE:16ZERO:32ALL:64SEPARATE" },
  { "AREA",	1, 4, lux_area, ":SEED:NUMBERS:DIAGONAL" }, /* topology.c */
  { "AREA2",	2, 6, lux_area2, /* toplogy.c */
    "::SEED:NUMBERS:DIAGONAL:SIGN" },
  { "ARESTORE",	1, MAX_ARG, lux_arestore, 0 }, /* files.c */
  { "ARRAY_STATISTICS", 4, 7, lux_array_statistics, 0, }, /* fun1.c */
  { "ASTORE",	2, MAX_ARG, lux_astore, 0 }, /* files.c */
  { "ATOMIZE", 	1, 1, lux_atomize, "1TREE:2LINE" }, /* strous.c */
  { "BATCH",	0, 1, lux_batch, "1QUIT" }, /* symbols.c */
  { "BREAKPOINT", 0, 1, lux_breakpoint, /* install.c */
    "0SET:1ENABLE:2DISABLE:3DELETE:4LIST:8VARIABLE" },
  { "BYTE",	1, MAX_ARG, lux_byte_inplace, 0, }, /* symbols.c */
  { "C",	1, 7, lux_callig, /* hersh.c */
    "0DEP:1DVI:2DEV:3IMG:4PLT:5RIM:6RPL" },
#if CALCULATOR
  { "CALCULATOR",	0, 0, lux_calculator, 0 }, /* calculator.c */
#endif
  { "CALLIG",	1, 7, lux_callig, /* hersh.c */
    "0DEP:1DVI:2DEV:3IMG:4PLT:5RIM:6RPL" },
  { "CDOUBLE",	1, MAX_ARG, lux_cdouble_inplace, 0 }, /* symbols.c */
  { "CFLOAT",	1, MAX_ARG, lux_cfloat_inplace, 0 }, /* symbols.c */
  { "CHDIR",	0, 1, lux_chdir, "1SHOW" }, /* files.c */
#if DEBUG
  { "CHECKLIST", 0, 1, checkList, 0 }, /* debug.c */
#endif
  { "CLOSE",	1, 1, lux_close, 0 }, /* files.c */
  { "CLUSTER",	2, 8, lux_cluster, /* cluster.c */
    "|32|:CENTERS:INDEX:SIZE:SAMPLE:EMPTY:MAXIT:RMS:1UPDATE:2ITERATE:4VOCAL:8QUICK:16RECORD:32ORDERED" },
#if HAVE_LIBX11
  { "COLORCOMPONENTS", 4, 4, lux_colorComponents, 0 }, /* color.c */
  { "COLORSTOGREY", 1, 1, lux_colorstogrey, 0 }, /* color.c */
#endif
#if MOTIF
  { "COMPILE_FILE", 1, 1, lux_compile_file, 0 }, /* motifextra.c */
#endif
  { "CONTOUR",	1, 6, lux_contour, /* contour.c */
    "IMAGE:LEVELS:XMIN:XMAX:YMIN:YMAX:STYLE:DASHSIZE:1AUTOCONTOUR:2USERCONTOUR" },
  { "COORDTRF",	2, 4, lux_coordtrf, /* coord.c */
   "0DEP:1DVI:2DEV:3IMG:4PLT:5RIM:6RPL:7X11:8TODVI:16TODEV:24TOIMG:32TOPLT:40TORIM:48TORPL:56TOX11" },
  { "CRUNCH",	3, 3, lux_crunch, 0 }, /* crunch.c */
  { "CRUNCHRUN",	3, 3, lux_crunchrun, 0 }, /* crunch.c */
  { "CSPLINE_EXTR", 5, 8, lux_cubic_spline_extreme, "1KEEPDIMS:2PERIODIC:4AKIMA::::POS:MINPOS:MINVAL:MAXPOS:MAXVAL" }, /* fun3.c */
  { "D",	0, MAX_ARG, lux_dump, /* fun1.c */
    "+|36|1FIXED:2SYSTEM:4ZERO:8LOCAL:24CONTEXT:32FOLLOW:64FULL" },
  { "DECOMP",	1, 1, lux_decomp, 0 }, /* fun2.c */
  { "DECRUNCH",	2, 2, lux_decrunch, 0 }, /* crunch.c */
  { "DEFAULT",	2, MAX_ARG, lux_default, "+" }, /* strous.c */
  { "DELETE", 	1, MAX_ARG, lux_delete, "+1POINTER" }, /* fun1.c */
  { "DIAGNOSTIC", 0, 1, lux_redirect_diagnostic, 0 }, /* strous.c */
  { "DISTR",	3, 3, lux_distr, 0 }, /* strous.c */
  { "DOUB",	1, MAX_ARG, lux_double_inplace, 0 }, /* symbols.c */
  { "DOUBLE",	1, MAX_ARG, lux_double_inplace, 0 }, /* symbols.c */
  { "DSOLVE",	2, 2, lux_dsolve, 0 }, /* fun2.c */
  { "DUMP", 	0, MAX_ARG, lux_dump, /* fun1.c */
    "+|36|1FIXED:2SYSTEM:4ZERO:8LOCAL:24CONTEXT:32FOLLOW:64FULL" },
  { "DUMP_LUN",	0, 0, lux_dump_lun, 0 }, /* files.c */
  { "DUMP_STACK", 0, 0, lux_dump_stack, 0 }, /* strous.c */
  { "ECHO",	0, 1, lux_echo, 0 }, /* symbols.c */
  { "ENDIAN",	1, 1, lux_endian, 0 }, /* strous.c */
  { "ERASE",	0, 5, lux_erase, /* plots.c */
    "0DEP:1DVI:2DEV:3IMG:4PLT:5RIM:6RPL:7X11:64SCREEN:128POSTSCRIPT:192PDEV" },
  { "ERROR",	0, 2, lux_error, "1STORE:2RESTORE" }, /* error.c */
  { "EXECUTE",	1, 1, lux_execute, "1MAIN" }, /* execute.c */
  { "EXIT",	0, 1, lux_quit, 0 }, /* fun1.c */
  { "EXTRACT_BITS", 4, 4, lux_extract_bits, 0 }, /* fun3.c */
  { "F0H",	1, 2, lux_fzhead, 0 }, /* files.c */
  { "F0HEAD",	1, 2, lux_fzhead, 0 }, /* files.c */
  { "F0R",	2, 3, lux_fzread, "|1|1PRINTHEADER" }, /* files.c */
  { "F0READ",	2, 3, lux_fzread, "|1|1PRINTHEADER" }, /* files.c */
  { "F0W",	2, 3, lux_fzwrite, 0 }, /* files.c */
  { "F0WRITE",	2, 3, lux_fzwrite, 0 }, /* files.c */
  { "FADE",	2, 2, lux_fade, 0 }, /* fun3.c */
  { "FADE_INIT", 2, 2, lux_fade_init, 0 }, /* fun3.c */
  { "FCRUNWRITE", 2, 3, lux_fcrunwrite, 0 }, /* files.c */
  { "FCRW",	2, 3, lux_fcrunwrite, 0 }, /* files.c */
  { "FCW",	2, 3, lux_fcwrite, "1RUNLENGTH" }, /* files.c */
  { "FCWRITE",	2, 3, lux_fcwrite, "1RUNLENGTH" }, /* files.c */
  { "FFTSHIFT",	2, 2, lux_fftshift, 0 }, /* fun3.c */
  { "FILEPTR",	1, 2, lux_fileptr, "1START:2EOF:4ADVANCE" }, /* files.c */
  { "FILEREAD", 5, 5, lux_fileread, 0 }, /* files.c */
  { "FILETOFZ",	3, 3, lux_file_to_fz, 0 }, /* files.c */
  { "FILEWRITE", 2, 3, lux_filewrite, 0 }, /* files.c */
#if DEVELOP
  { "FIT3DCUBE", 0, 0, lux_fitUnitCube, 0 }, /* projection.c */
#endif
  { "FITS_READ", 2, 7, lux_fits_read, "|1|1TRANSLATE:2RAWVALUES::::::BLANK" }, /* files.c */
  { "FITS_WRITE", 2, 4, lux_fits_write, "1VOCAL" }, /* files.c */
  { "FIX",	1, MAX_ARG, lux_long_inplace, 0 }, /* symbols.c */
  { "FLOAT",	1, MAX_ARG, lux_float_inplace, 0 }, /* symbols.c */
  { "FORMAT_SET", 0, 1, lux_format_set, 0 }, /* files.c */
  { "FPRINT",	1, MAX_ARG, lux_fprint, "1ELEMENT" }, /* files.c */
  { "FPRINTF",	2, MAX_ARG, lux_fprintf, "1ELEMENT" }, /* files.c */
  { "FREAD",	2, MAX_ARG, lux_fread, "1COUNTSPACES" }, /* files.c */
  { "FREADF",	3, MAX_ARG, lux_freadf, "1COUNTSPACES" }, /* files.c */
  { "FREADS",	2, MAX_ARG, lux_freads, "1COUNTSPACES" }, /* files.c */
  { "FZH",	1, 2, lux_fzhead, 0 }, /* files.c */
  { "FZHEAD",	1, 2, lux_fzhead, 0 }, /* files.c */
  { "FZINSPECT", 2, 3, lux_fzinspect, 0 }, /* files.c */
  { "FZR",	2, 3, lux_fzread, "|1|1PRINTHEADER" }, /* files.c */
  { "FZREAD",	2, 3, lux_fzread, "|1|1PRINTHEADER" }, /* files.c */
  { "FZW",	2, 3, lux_fzwrite, "1SAFE" }, /* files.c */
  { "FZWRITE",	2, 3, lux_fzwrite, "1SAFE" }, /* files.c */
  { "GETMIN9",	3, 3, lux_getmin9, 0 }, /* fun4.c */
  { "GIFREAD",	2, 3, lux_gifread, 0 }, /* gifread_ana.c */
  { "GIFWRITE",	2, 3, lux_gifwrite, 0 }, /* gifwrite_ana.c */
#if HAVE_LIBX11
  { "HAIRS",	0, 0, lux_xplace, 0 }, /* xport.c */
#endif
  { "HELP",	0, 1, lux_help,	/* strous.c */
    "|30|1EXACT:2ROUTINE:4TREE:8SUBROUTINE:16FUNCTION:32LIST::PAGE" },
  { "HEX",	1, MAX_ARG, lux_hex, 0 }, /* files.c */
  { "IDLRESTORE", 1, 1, lux_idlrestore, 0 }, /* idl.c */
  { "INFO",	0, 0, site, /* site.c */
    "1TABLE:2TIME:4PLATFORM:8PACKAGES:16WARRANTY:32COPY:64BUGS:128KEYS:255ALL" },
  { "INSERT", 	2, 4, lux_inserter, 0 }, /* subsc.c */
  { "INT",	1, MAX_ARG, lux_word_inplace, 0 }, /* symbols.c */
#if HAVE_LIBJPEG
  { "JPEGREAD", 2, 4, lux_read_jpeg6b, ":::SHRINK:1GREYSCALE" }, /* jpeg.c */
  { "JPEGWRITE", 2, 4, lux_write_jpeg6b, 0 },/* jpeg.c */
#endif
  { "LIMITS",	0, 4, lux_limits, 0 }, /* plots.c */
  { "LIST",	1, 1, lux_list, 0 }, /* ident.c */
  { "LONG",	1, MAX_ARG, lux_long_inplace, 0 }, /* symbols.c */
#if HAVE_LIBX11
  { "MENU",	1, MAX_ARG, lux_menu, 0 }, /* menu.c */
  { "MENUHIDE",	1, 1, lux_menu_hide, 0 }, /* menu.c */
  { "MENUITEM",	3, 3, lux_menu_item, 0 }, /* menu.c */
  { "MENUPOP",	1, MAX_ARG, lux_menu_pop, 0 }, /* menu.c */
  { "MENUREAD",	4, 4, lux_menu_read, 0 }, /* menu.c */
  { "MENUZAP",	1, 1, lux_menu_kill, 0 }, /* menu.c */
#endif
  { "MULTISIEVE", 4, 4, lux_multisieve, 0 }, /* strous2.c */
#if DEBUG
  { "NEWALLOCS", 0, 1, lux_newallocs, "1RESET" }, /* debug.c */
#endif
  { "NOECHO",	0, 0, lux_noecho, 0 }, /* symbols.c */
  { "NOOP",	0, 0, lux_noop, 0 }, /* strous2.c */
  { "ONE",	1, 1, lux_one, 0 }, /* fun1.c */
  { "OPENR",	2, 2, lux_openr, "1GET_LUN" }, /* files.c */
  { "OPENU",	2, 2, lux_openu, "1GET_LUN" }, /* files.c */
  { "OPENW",	2, 2, lux_openw, "1GET_LUN" }, /* files.c */
  { "OPLOT",	1, 13, lux_oplot, /* plots.c */
    "1DVI:2DEV:3IMG:4PLT:5RIM:6RPL:128WHOLE:256CLIPBARS:XDATA:YDATA:SYMBOL:LINE:XTITLE:YTITLE:TITLE:DASHSIZE:XERRORS:YERRORS:BREAKS:XBARSIZE:YBARSIZE" },
  { "ORIENTATION", 3, 8, lux_orientation, /* orientation.c */
  "1VOCAL:2GETJ:0PARALLEL:4PERPENDICULAR:::ORIENTATION:VALUES:WAVENUMBER:GRID:ASPECT:ORDER" },
  { "PDEV",	0, 1, lux_pdev, 0 }, /* plots.c */
  { "PEEK",	1, 2, peek, 0 }, /* strous.c */
  { "PEN",	0, 2, lux_pen, "WIDTH:COLOR:1STANDARDGRAY" }, /* plots.c */
  { "PENCOLOR", 0, 1, lux_pencolor, 0 }, /* plots.c */
#if HAVE_LIBX11
  { "PIXELSTO8BIT", 3, 3, lux_pixelsto8bit, 0 }, /* color.c */
#endif
  { "PLOT",	1, 15, lux_plot, /* plots.c */
    "1DVI:2DEV:3IMG:4PLT:5RIM:6RPL:64KEEP:128WHOLE:256CLIPBARS:1024LII:1280LIO:1536LOI:1792LOO:XDATA:YDATA:SYMBOL:LINE:XTITLE:YTITLE:TITLE:DASHSIZE:XERRORS:YERRORS:BREAKS:XBARSIZE:YBARSIZE:XFMT:YFMT" },
#if DEVELOP
  { "PLOT3D",	1, 1, lux_plot3d, "1HIDE:2CUBE" }, /* projection.c */
#endif
  { "POINTER",	2, 2, lux_pointer, /* symbols.c */
    "+:1FUNCTION:2SUBROUTINE:4INTERNAL:8MAIN" },
  { "POP",	1, MAX_ARG, lux_pop, "%1%NUM" }, /* strous.c */
  { "POSTIMAGE", 1, 5, lux_postimage, 0 }, /* plots.c */
  { "POSTRAW",	1, 1, lux_postraw, 0 }, /* plots.c */
  { "POSTREL",	0, 4, postrelease, 0 }, /* plots.c */
  { "POSTRELEASE", 0, 4, postrelease, 0 }, /* plots.c */
  { "PRINT", 	1, MAX_ARG, lux_type, "1JOIN:2RAW:4SEPARATE" }, /* files.c */
  { "PRINTF",	1, MAX_ARG, lux_printf, "1JOIN:2RAW:4SEPARATE" }, /* files.c */
#if DEVELOP
  { "PROJECTION", 0, MAX_ARG, lux_projection, /* projection.c */
	"1RESET:2ORIGINAL::TRANSLATE:ROTATE:SCALE:PERSPECTIVE:OBLIQUE" },
#endif
  { "PUSH",	1, MAX_ARG, lux_push, 0 }, /* strous.c */
  { "QUIT", 	0, 1, lux_quit, 0 }, /* fun1.c */
  { "READ",	1, MAX_ARG, lux_read, "1ASKMORE:2WORD:4FLUSH" }, /* files.c */
  { "READARR",	1, 1, lux_readarr, 0 }, /* strous.c */
  { "READF",	2, MAX_ARG, lux_readf, "1ASKMORE:2WORD" }, /* files.c */
  { "READORBITS", 0, 1, lux_readorbits, "1LIST:2REPLACE" }, /* astron.c */
  { "READU",	2, MAX_ARG, lux_readu, 0 }, /* files.c */
#if HAVE_LIBJPEG
  { "READ_JPEG", 2, 4, lux_read_jpeg6b, ":::SHRINK:1GREYSCALE" }, /* jpeg.c */
#endif
  { "RECORD",	0, 1, lux_record, "1INPUT:2OUTPUT:4RESET" }, /* symbols.c */
  { "REDIM", 	2, 9, lux_redim, 0 }, /* subsc.c */
  { "REPLACE",	3, 3, lux_replace_values, 0 }, /* strous2.c */
  { "RESTART",	0, 0, lux_restart, 0 }, /* install.c */
  { "RESTORE",	2, 3, lux_fzread, "1PRINTHEADER" }, /* files.c */
#if HAVE_SYS_MTIO_H
  { "REWIND",	1, 1, lux_rewind, 0 }, /* tape.c */
#endif
  { "REWINDF",	1, 1, lux_rewindf, 0 }, /* files.c */
  { "S",	0, 1, lux_show, 0 }, /* fun1.c */
  { "SC",	3, 3, lux_sc, 0 }, /* fun3.c */
  { "SCANF",	2, MAX_ARG, lux_freadf, "+1COUNTSPACES" }, /* files.c */
  { "SCB",	3, 3, lux_scb, "1EVEN:2ODD" }, /* fun3.c */
  { "SET",	0, 1, lux_set,	/* symbols.c */
    "VISUAL:1SET:2RESET:4SHOWALLOC:8WHITEBACKGROUND:16ULIMCOORDS:32YREVERSEIMG:64OLDVERSION:128ZOOM:1024ALLOWPROMPTS:2048XSYNCHRONIZE:4096PARSESILENT" },
#if HAVE_LIBX11
  { "SETBACKGROUND", 1, 2, lux_xsetbackground, 0 }, /* xport.c */
  { "SETBG",	1, 2, lux_xsetbackground, 0 }, /* xport.c */
#endif
  { "SETENV",	1, 1, lux_setenv, 0 }, /* files.c */
#if HAVE_LIBX11
  { "SETFG",	1, 2, lux_xsetforeground, 0 }, /* xport.c */
  { "SETFOREGROUND", 1, 2, lux_xsetforeground, 0 }, /* xport.c */
#endif
  { "SHIFT",	1, 4, lux_shift, ":::BLANK:1TRANSLATE" }, /* strous2.c */
  { "SHOW", 	0, 1, lux_show, 0 }, /* fun1.c */
  { "SHOWORBITS", 0, 0, lux_showorbits, 0 }, /* astron.c */
  { "SHOWSTATS", 0, 0, showstats, 0 }, /* strous2.c */
#if DEBUG
  { "SHOW_FILES", 0, 0, show_files, 0 }, /* debug.c */
#endif
  { "SHOW_FUNC", 0, 1, lux_show_func, "1PARAMETERS" }, /* symbols.c */
  { "SHOW_SUBR", 0, 1, lux_show_subr, "1PARAMETERS" }, /* symbols.c */
#if DEBUG
  { "SHOW_TEMPS", 0, 0, lux_show_temps, 0 }, /* symbols.c */
#endif
#if HAVE_LIBX11
  { "SHOW_VISUALS", 0, 0, lux_show_visuals, 0 }, /* xport.c */
#endif
#if HAVE_SYS_MTIO_H
  { "SKIPF",	1, 2, lux_skipf, 0 }, /* tape.c */
  { "SKIPR",	1, 2, lux_skipr, 0 }, /* tape.c */
#endif
  { "SPAWN", 	1, 1, lux_spawn, "1SILENT" }, /* files.c */
  { "SSCANF",	2, MAX_ARG, lux_freads, "1COUNTSPACES" }, /* files.c */
  { "STEP",	0, 1, lux_step, 0 }, /* symbols.c */
  { "STORE",	2, 3, lux_fzwrite, "1SAFE" }, /* files.c */
  { "STRING",	1, MAX_ARG, lux_string_inplace, 0 }, /* symbols.c */
  { "SUBSHIFT",	4, 4, lux_subshift, 0 }, /* fun5.c */
  { "SUBSHIFTC", 4, 5, lux_subshiftc, 0 }, /* fun5.c */
  { "SWAB",	1, MAX_ARG, lux_swab, 0 }, /* fun2.c */
  { "SWAPB",	1, MAX_ARG, lux_swab, 0 }, /* fun2.c */
  { "SWAPHALF",	1, 1, lux_swaphalf, 0 }, /* strous2.c */
  { "SWITCH",	2, 2, lux_switch, 0 }, /* symbols.c */
  { "T", 	1, MAX_ARG, lux_type, "1JOIN:2RAW:4SEPARATE" }, /* files.c */
#if HAVE_SYS_MTIO_H
  { "TAPEBUFIN", 2, 5, lux_tapebufin, 0 }, /* tape.c */
  { "TAPEBUFOUT", 2, 5, lux_tapebufout, 0 }, /* tape.c */
  { "TAPE_STATUS", 0, 0, lux_tape_status, 0 }, /* tape.c */
  { "TAPRD",	2, 2, lux_taprd, 0 }, /* tape.c */
  { "TAPWRT",	2, 2, lux_tapwrt, 0 }, /* tape.c */
#endif
#if HAVE_LIBX11
  { "THREECOLORS", 0, 1, lux_threecolors, 0 }, /* xport.c */
#endif
  { "TOLOOKUP",	3, 3, lux_tolookup, "1ONE" }, /* strous2.c */
  { "TRACE",	0, 1, lux_trace, /* install.c */
    "1FILE:2LOOP:4BRACES:8ROUTINE:143ALL:16SHOWSTATS:32CPUTIME:64SHOWEXEC:128ENTER" },
#if DEVELOP
  { "TRAJECTORY", 3, 7, lux_trajectory, 0 }, /* strous3.c */
#endif
#if HAVE_LIBX11
  { "TV",	1, 5, lux_xtv,	/* xport.c */
    ":X:Y:WINDOW:SCALE:0DEP:1DVI:2DEV:3IMG:4PLT:5RIM:6RPL:7X11:64SCREEN:128POSTSCRIPT:192PDEV:256PLOTWINDOW:512ZOOM:1024CENTER:16384BIT24" },
  { "TV3",	1, 7, lux_tv3, /* xport.c */
    ":TWO:THREE:X:Y:WINDOW:SCALE:0DEP:1DVI:2DEV:3IMG:4PLT:5RIM:6RPL:7X11:64SCREEN:128POSTSCRIPT:192PDEV:256PLOTWINDOW:512ZOOM:1024CENTER" },
  { "TVLAB",	3, 4, lux_xlabel, 0 }, /* xport.c */
  { "TVLCT",	3, 3, lux_xtvlct, "1FIXEDSIZE" }, /* xport.c */
  { "TVMAP",	1, 5, lux_xtvmap, /* xport.c */
    ":::WINDOW:SCALE:0DEP:1DVI:2DEV:3IMG:4PLT:5RIM:6RPL:7X11:64SCREEN:128POSTSCRIPT:192PDEV:256PLOTWINDOW:512ZOOM" },
  { "TVPLANE",	1, 5, lux_xtvplane, 0 }, /* xport.c */
  { "TVRAW",	1, 5, lux_xtvraw, /* xport.c */
    ":::WINDOW:SCALE:0DEP:1DVI:2DEV:3IMG:4PLT:5RIM:6RPL:7X11:64SCREEN:128POSTSCRIPT:192PDEV:256PLOTWINDOW:512ZOOM" },
  { "TVREAD",	0, 5, lux_xtvread, "1GREYSCALE" }, /* xport.c */
#endif
  { "TY",	1, MAX_ARG, lux_type, "1JOIN:2RAW:4SEPARATE" }, /* files.c */
  { "TYPE", 	1, MAX_ARG, lux_type, "1JOIN:2RAW:4SEPARATE" }, /* files.c */
  { "ULIB", 	0, 1, lux_ulib, 0 }, /* files.c */
#if HAVE_SYS_MTIO_H
  { "UNLOAD",	1, 1, lux_unload, 0 }, /* tape.c */
#endif
  { "VERIFY",	0, 1, lux_verify, 0 }, /* install.c */
  { "WAIT",	1, 1, lux_wait, 0 }, /* fun2.c */
#if HAVE_LIBX11
  { "WAIT_FOR_MENU", 0, 1, lux_wait_for_menu, 0 }, /* menu.c */
#endif
#if HAVE_SYS_MTIO_H
  { "WAIT_FOR_TAPE", 1, 1, lux_wait_for_tape, 0 }, /* tape.c */
#endif
  { "WATCH",	1, 1, lux_watch, "1DELETE:2LIST" }, /* install.c */
#if HAVE_SYS_MTIO_H
  { "WEOF",	1, 1, lux_weof, 0 }, /* tape.c */
#endif
#if DEBUG
  { "WHERE",	1, 1, lux_whereisAddress, "1CUT" }, /* debug.c */
#endif
#if HAVE_LIBX11
  { "WINDOW",	0, 6, lux_window, 0 }, /* plots.c */
#endif
  { "WORD",	1, 1, lux_word_inplace, 0 }, /* symbols.c */
  { "WRITEU",	2, MAX_ARG, lux_writeu, 0 }, /* files.c */
#if HAVE_LIBJPEG
  { "WRITE_JPEG", 2, 4, lux_write_jpeg6b, 0 }, /* jpeg.c */
#endif
#if HAVE_LIBX11
  { "XANIMATE",	1, 6, lux_xanimate, ":::FR1:FR2:FRS:1TIME" }, /* xport.c */
  { "XCLOSE",	0, 0, lux_xclose, 0 }, /* xport.c */
  { "XCOPY",	2, 8, lux_xcopy, 0 }, /* xport.c */
  { "XCURSOR",	2, 4, lux_xcursor, 0 }, /* xport.c */
  { "XDELETE",	1, 1, lux_xdelete, 0 }, /* xport.c */
  { "XDRAWARC",	4, 7, lux_xdrawarc, 0 }, /* xport.c */
  { "XDRAWLINE", 4, 5, lux_xdrawline, 0 }, /* xport.c */
  { "XEVENT",	0, 0, lux_xevent, 0 }, /* xport.c */
  { "XFLUSH",	0, 0, lux_xflush, 0 }, /* xport.c */
  { "XFONT",	1, 2, lux_xfont, 0 }, /* xport.c */
  { "XINVERTARC", 4, 7, lux_xinvertarc, 0 }, /* xport.c */
  { "XINVERTLINE", 4, 5, lux_xinvertline, 0 }, /* xport.c */
  { "XLABEL",	3, 4, lux_xlabel, 0 }, /* xport.c */
  { "XLOOP",	0, 1, lux_xloop, 0 }, /* menu.c */
#if MOTIF
  { "XMALIGNMENT", 2, 2, lux_xmalignment, 0 }, /* motif.c */
  { "XMARMCOLOR", 2, 2, lux_xmarmcolor, 0 }, /* motif.c */
  { "XMATTACH",	6, 6, lux_xmattach, 0 }, /* motif.c */
  { "XMATTACH_RELATIVE", 5, 5, lux_xmattach_relative, 0 }, /* motif.c */
  { "XMBACKGROUNDCOLOR", 2, 2, lux_xmbackgroundcolor, 0 }, /* motif.c */
  { "XMBORDERCOLOR", 2, 2, lux_xmbordercolor, 0 }, /* motif.c */
  { "XMBORDERWIDTH", 2, 2, lux_xmborderwidth, 0 }, /* motif.c */
  { "XMBOTTOMSHADOWCOLOR", 2, 2, lux_xmbottomshadowcolor, 0 }, /* motif.c */
  { "XMDESTROY", 1, 1, lux_xmdestroy, 0 }, /* motif.c */
  { "XMDRAWINGLINK", 2, 2, lux_xmdrawinglink, 0 }, /* motif.c */
  { "XMFONT",	2, 2, lux_xmfont, 0 }, /* motif.c */
  { "XMFOREGROUNDCOLOR", 2, 2, lux_xmforegroundcolor, 0 }, /* motif.c */
  { "XMGETPIXMAP", 2, 2, lux_xmgetpixmap, 0 }, /* motif.c */
  { "XMGETWIDGETPOSITION", 3, 3, lux_xmgetwidgetposition, 0 }, /* motif.c */
  { "XMGETWIDGETSIZE", 3, 3, lux_xmgetwidgetsize, 0 }, /* motif.c */
  { "XMINFO",	1, 1, lux_xminfo, 0 }, /* motif.c */
  { "XMLISTADDITEM", 2, 3, lux_xmlistadditem, 0 }, /* motif.c */
  { "XMLISTDELETEALL", 1, 1, lux_xmlistdeleteall, 0 }, /* motif.c */
  { "XMLISTDELETEITEM", 2, 2, lux_xmlistdeleteitem, 0 }, /* motif.c */
  { "XMLISTFUNC", 1, 1, lux_xmlistfunc, 0 }, /* motif.c */
  { "XMLISTSELECT", 2, 3, lux_xmlistselect, 0 }, /* motif.c */
  { "XMLISTSUBR", 1, 1, lux_xmlistsubr, 0 }, /* motif.c */
  { "XMMESSAGE", 1, 5, lux_xmmessage, 0 }, /* motif.c */
  { "XMPOSITION", 3, 5, lux_xmposition, 0 }, /* motif.c */
  { "XMPROMPT",	3, 8, lux_xmprompt, 0 }, /* motif.c */
  { "XMQUERY",	1, 1, lux_xmquery, 0 }, /* motif.c */
  { "XMRAISE",	1, 1, lux_xmraise, 0 },  /* motif.c */
  { "XMRESIZEPOLICY",	2, 2, lux_xmresizepolicy, 0 }, /* motif.c */
  { "XMSCALERESETLIMITS", 3, 4, lux_xmscaleresetlimits, 0 }, /* motif.c */
  { "XMSCALESETVALUE", 2, 2, lux_xmscalesetvalue, 0 }, /* motif.c */
  { "XMSCROLLBARSETVALUES", 1, 5, lux_xmscrollbarsetvalues, 0 }, /* motif.c */
  { "XMSELECTCOLOR", 2, 2, lux_xmselectcolor, 0 }, /* motif.c */
  { "XMSENSITIVE", 2, 2, lux_xmsensitive, 0 }, /* motif.c */
  { "XMSETCOLORS", 2, 2, lux_xmsetcolors, 0 }, /* motif.c */
  { "XMSETDIRECTORY", 2, 2, lux_xmsetdirectory, 0 }, /* motif.c */
  { "XMSETLABEL", 2, 2, lux_xmsetlabel, 0 }, /* motif.c */
  { "XMSETMARGINS", 3, 3, lux_xmsetmargins, 0 }, /* motif.c */
  { "XMSETMNEMONIC", 2, 2, lux_xmsetmnemonic, 0 }, /* motif.c */
  { "XMSETMODAL", 2, 2, lux_xmsetmodal, 0 }, /* motif.c */
  { "XMSETOPTIONSELECTION", 2, 2, lux_xmsetoptionselection, 0 }, /* motif.c */
  { "XMSETPIXMAP", 2, 2, lux_xmsetpixmap, 0 }, /* motif.c */
  { "XMSETTITLE", 2, 2, lux_xmsettitle, 0 }, /* motif.c */
  { "XMSET_TEXT_OUTPUT", 1, 1, lux_xmset_text_output, 0 }, /* motif.c */
  { "XMSIZE",	3, 3, lux_xmsize, 0 }, /* motif.c */
  { "XMTEXTAPPEND", 2, 3, lux_xmtextappend, 0 }, /* motif.c */
  { "XMTEXTERASE", 1, 1, lux_xmtexterase, 0 }, /* motif.c */
  { "XMTEXTFIELDSETEDITABLE", 2, 2, lux_xmtextfieldseteditable, 0 }, /* motif.c */
  { "XMTEXTFIELDSETMAXLENGTH", 2, 2, lux_xmtextfieldsetmaxlength, 0 }, /* motif.c */
  { "XMTEXTFIELDSETSTRING", 2, 3, lux_xmtextfieldsetstring, 0 }, /* motif.c */
  { "XMTEXTREPLACE",	3, 4, lux_xmtextreplace, 0 }, /* motif.c */
  { "XMTEXTSETEDITABLE", 2, 2, lux_xmtextseteditable, 0 }, /* motif.c */
  { "XMTEXTSETPOSITION", 2, 2, lux_xmtextsetposition, 0 }, /* motif.c */
  { "XMTEXTSETROWCOLUMNSIZE", 3, 3, lux_xmtextsetrowcolumnsize, 0 }, /* motif.c */
  { "XMTEXTSETSTRING", 2, 2, lux_xmtextsetstring, 0 }, /* motif.c */
  { "XMTOGGLESETSTATE", 2, 3, lux_xmtogglesetstate, 0 }, /* motif.c */
  { "XMTOPSHADOWCOLOR", 2, 2, lux_xmtopshadowcolor, 0 }, /* motif.c */
#endif
  { "XOPEN",	0, 1, lux_xopen, /* xport.c */
    "1PRIVATE_COLORMAP:2DEFAULT_COLORMAP:4SELECTVISUAL" },
  { "XPLACE",	0, 2, lux_xplace, /* xport.c */
    "1DVI:2DEV:3IMG:4PLT:5RIM:6RPL:7X11" },
  { "XPORT",	0, 7, lux_xport, 0 }, /* xport.c */
  { "XPURGE",	0, 0, lux_xpurge, 0 }, /* xport.c */
  { "XQUERY",	0, 1, lux_xquery, 0 }, /* xport.c */
  { "XRAISE",	1, 1, lux_xraise, 0 }, /* xport.c */
  { "XREGISTER", 0, 3, lux_register_event , /* menu.c */
    "1KEYPRESS:4BUTTONPRESS:8BUTTONRELEASE:16POINTERMOTION:32ENTERWINDOW:64LEAVEWINDOW:127ALLEVENTS:128ALLWINDOWS:256ALLMENUS:512DESELECT::WINDOW:MENU" },
  { "XSETACTION", 0, 2, lux_xsetaction, 0 }, /* xport.c */
#if MOTIF
  { "XTLOOP",	0, 1, lux_xtloop, 0 }, /* motif.c */
  { "XTMANAGE",	1, 1, lux_xtmanage, 0 }, /* motif.c */
  { "XTPOPDOWN", 1, 1, lux_xtpopdown, 0 }, /* motif.c */
  { "XTPOPUP",	1, 1, lux_xtpopup, 0 },	/* motif.c */
  { "XTUNMANAGE", 1, 1, lux_xtunmanage, 0 }, /* motif.c */
#endif
  { "XTV",	1, 4, lux_xtv,  /* xport.c */
    ":::WINDOW:SCALE:0DEP:1DVI:2DEV:3IMG:4PLT:5RIM:6RPL:7X11:64SCREEN:128POSTSCRIPT:192PDEV:256PLOTWINDOW:512ZOOM" },
  { "XTVREAD",	0, 5, lux_xtvread, "1GREYSCALE" }, /* xport.c */
  { "XWIN",	0, 7, lux_xport, 0 }, /* xport.c */
  { "XWINDOW",	0, 7, lux_xport, 0 }, /* xport.c */
  { "XYMOV",	2, 4, lux_xymov, /* plots.c */
    "|192|:::BREAKS:0DEP:1DVI:2DEV:3IMG:4PLT:5RIM:6RPL:7X11:64BOUNDINGBOX:128MOVEFIRST:256ALTDASH" },
#endif
  { "ZAP",	1, MAX_ARG, lux_zap, "+1POINTER" }, /* strous2.c */
  { "ZERO", 	1, MAX_ARG, lux_zero, 0 }, /* fun1.c */
#if MOTIF
  { "ZEROIFNOTDEFINED", 1, MAX_ARG, lux_zeroifnotdefined, 0}, /* motifextra.c */
#endif
  { "ZERONANS",	1, MAX_ARG, lux_zapnan, "*%1%VALUE" }, /* fun1.c */
#if HAVE_LIBX11
  { "ZOOM",	1, 3, lux_zoom, "1OLDCONTRAST" }, /* zoom.c */
#endif
};
int32_t nSubroutine = sizeof(subroutine_table)/sizeof(internalRoutine);

extern int32_t lux_abs(), lux_acos(), lux_arestore_f(), lux_arg(),
  lux_array(), lux_asin(), lux_assoc(), lux_astore_f(), lux_atan(),
  lux_atan2(), lux_basin(), lux_basin2(), lux_beta(), lux_bisect(),
  lux_bmap(), bytarr(), lux_byte(), bytfarr(), lux_cbrt(), cdblarr(),
  cdblfarr(), lux_cdmap(), lux_cdouble(), lux_ceil(), lux_cfloat(),
  lux_cfmap(), cfltarr(), cfltfarr(), lux_chi_square(),
  lux_classname(), lux_complex(), lux_complexsquare(), lux_compress(),
  lux_concat(), lux_conjugate(), lux_convertsym(), lux_cos(),
  lux_cosh(), lux_cputime(), lux_crosscorr(), lux_crunch_f(),
  lux_ctime(), lux_cubic_spline(), lux_date(), lux_date_from_tai(),
  dblarr(), dblfarr(), lux_defined(), lux_delete(), lux_despike(),
  lux_detrend(), lux_differ(), lux_dilate(), lux_dilate_dir(),
  lux_dimen(), lux_dir_smooth(), lux_dir_smooth2(), lux_distarr(),
  lux_distr_f(), lux_dmap(), lux_double(), lux_equivalence(),
  lux_erf(), lux_erfc(), lux_erode(), lux_erode_dir(), lux_esmooth(),
  lux_eval(), lux_exp(), lux_expand(), lux_expm1(),
  lux_extract_bits_f(), lux_extreme_general(), lux_f_ratio(),
  lux_fcwrite_f(), lux_fftshift_f(), lux_fileptr_f(),
  lux_filesize(), lux_filetype_name(), lux_find(), lux_find2(), lux_findfile(),
  lux_find_max(), lux_find_maxloc(), lux_find_min(),
  lux_find_minloc(), lux_fitskey(), lux_fits_header_f(),
  lux_fits_read_f(), lux_fits_xread_f(), lux_float(), lux_floor(),
  fltarr(), fltfarr(), lux_fmap(), lux_freadf_f(), lux_freads_f(),
  lux_fstring(), lux_fzarr(), lux_fzhead_f(), lux_fzread_f(),
  lux_fzwrite_f(), lux_gamma(), lux_generalfit(), lux_get_lun(),
  lux_getenv(), lux_gridmatch(), lux_gsmooth(), lux_hamming(), lux_hilbert(),
  lux_hist(), lux_histr(), lux_identify_file(), lux_idlread_f(),
  lux_imaginary(), lux_incomplete_beta(), lux_incomplete_gamma(),
  lux_index(), lux_indgen(), lux_inpolygon(), intarr(), intfarr(),
  lux_isarray(), lux_isnan(), lux_isscalar(), lux_isstring(),
  lux_istring(), lux_j0(), lux_j1(), lux_jd(), lux_jn(), lux_cjd(),
  lux_ksmooth(), lux_laplace2d(), lux_lmap(), lux_local_maxf(),
  lux_local_maxloc(),
  lux_local_minf(), lux_local_minloc(), lux_log(), lux_log10(),
  lux_log1p(), lonarr(), lonfarr(), lux_long(), lux_lower(),
  lux_lsq(), lux_lsq2(), lux_match(), lux_max_dir(),
  lux_maxf(), lux_maxfilter(), lux_maxloc(), lux_mean(),
  lux_medfilter(), lux_median(), lux_memory(), lux_minf(),
  lux_minfilter(), lux_minloc(), lux_neg_func(),
  lux_noncentral_chi_square(), lux_not(), lux_num_dimen(),
  lux_num_elem(), lux_onef(), lux_openr_f(), lux_openu_f(),
  lux_openw_f(), lux_orderfilter(), lux_pit(), lux_poly(), lux_pow(),
  lux_power(), lux_printf_f(), lux_psum(), lux_quantile(), lux_quit(),
  lux_random(), lux_randomb(), lux_randomd(), lux_randomn(),
  lux_randomu(), lux_randoml(), lux_readf_f(), lux_readkey(),
  lux_readkeyne(), lux_readu_f(), lux_real(), lux_redim_f(),
  lux_regrid(), lux_regrid3(), lux_regrid3ns(), lux_reorder(),
  lux_reverse(), lux_rfix(), lux_root3(), lux_runcum(), lux_runprod(),
  lux_runsum(), lux_scale(), lux_scalerange(),
  lux_sdev(), lux_segment(), lux_segment_dir(), lux_sgn(),
  lux_shift_f(), lux_sieve(), lux_sin(), lux_sinh(), lux_skipc(),
  lux_smap(), lux_smooth(), lux_solar_b(), lux_solar_l(),
  lux_solar_p(), lux_solar_r(), lux_sort(), lux_spawn_f(), lux_sqrt(),
  strarr(), lux_strcount(), lux_stretch(), lux_string(), lux_strlen(),
  lux_strloc(), lux_strpos(), lux_strreplace(), lux_strskp(),
  lux_strsub(), lux_strtok(), lux_strtol(), lux_strtrim(),
  lux_struct(), lux_student(), lux_subsc_func(), lux_sun_b(),
  lux_sun_d(), lux_sun_p(), lux_sun_r(), lux_symbol(),
  lux_symbol_memory(), lux_symbol_number(), lux_symclass(),
  lux_symdtype(), lux_systime(), lux_table(), lux_tai_from_date(),
  lux_tan(), lux_tanh(), lux_temp(), lux_tense(), lux_tense_curve(),
  lux_tense_loop(), lux_time(), lux_total(), lux_trace_decoder(),
  lux_trend(), lux_tri_name_from_tai(), lux_typeName(), lux_upper(),
  lux_variance(), lux_varname(), lux_voigt(), lux_wait_for_menu(),
  lux_wmap(), lux_word(), lux_y0(), lux_y1(), lux_yn(),
  lux_zapnan_f(), lux_zerof(), lux_zinv(), lux_fcrunwrite_f(),
  lux_strpbrk(), lux_shift3(), lux_area_connect(), lux_legendre(),
  lux_cartesian_to_polar(), lux_polar_to_cartesian(), lux_roll(),
  lux_siderealtime(), lux_asinh(),
  lux_acosh(), lux_atanh(), lux_astrf(), lux_antilaplace2d(),
  lux_cspline_find(), lux_covariance();

#if HAVE_REGEX_H
extern int32_t lux_getdirectories(), lux_getfiles(), lux_getfiles_r(),
  lux_getmatchedfiles(), lux_getmatchedfiles_r(), lux_regex();
#endif

#if DEVELOP
extern int32_t	lux_project(), lux_bsmooth(), lux_compile(),
  lux_bessel_i0(), lux_bessel_i1(), lux_bessel_k0(), lux_bessel_k1(),
  lux_bessel_kn(), lux_regridls(), lux_bigger235(),
  lux_geneticfit();
#endif

extern int32_t	lux_gifread_f(), lux_gifwrite_f();

#if HAVE_LIBX11
extern int32_t	lux_check_menu(), lux_check_window(), lux_colorpixel(),
  lux_event_name(), lux_xlabelwidth(), lux_xquery_f(), lux_xexist();
#endif

extern int32_t lux_calendar(), lux_EasterDate(), /* lux_orbitalElement(), */
  lux_astropos(), lux_precess(), lux_constellation(),
  lux_constellationname(), lux_enhanceimage();

#if MOTIF

extern int32_t lux_xmarrow(), lux_xmboard(), lux_xmbutton(),
  lux_xmcheckbox(), lux_xmcolumns(), lux_xmcommand(),
  lux_xmdialog_board(), lux_xmdrawingarea(), lux_xmfileselect(),
  lux_xmform(), lux_xmframe(), lux_xmgetoptionselection(),
  lux_xmgetwidgetaddress(), lux_xmhscale(), lux_xmlabel(),
  lux_xmlist(), lux_xmlistcount(), lux_xmlistfromfile(),
  lux_xmmenubar(), lux_xmoptionmenu(), lux_xmpixmapbutton(),
  lux_xmpulldownmenu(), lux_xmradiobox(), lux_xmrows(),
  lux_xmscalegetvalue(), lux_xmscrolledwindow(), lux_xmseparator(),
  lux_xmtext(), lux_xmtextbox(), lux_xmtextfield(),
  lux_xmtextfieldarray(), lux_xmtextfieldgetstring(),
  lux_xmtextfromfile(), lux_xmtextgetstring(), lux_xmtogglegetstate(),
  lux_xmvscale(), lux_xtparent(), lux_xtwindow(), lux_xmdialog_form(),
  lux_xmaddfiletolist(), lux_xmtoplevel_form(),
  lux_xmtoplevel_board(), lux_xmpixmapoptionmenu(), lux_xmscrolledwindowapp(),
  lux_xmfilegetlist(), lux_xmhscrollbar(), lux_xmtextgetinsertposition(),
  lux_xmtextgetlastposition(), lux_xmtextgetselection(), lux_xmvscrollbar();

#endif

#if HAVE_LIBJPEG
extern int32_t lux_read_jpeg6b_f(), lux_write_jpeg6b_f();
#endif

extern int32_t	vargsmooth(), lux_test();

internalRoutine function_table[] = {
  { "%A_UNARY_NEGATIVE", 1, 1, lux_neg_func, "*" },	/* fun1.c */
  { "%B_SUBSCRIPT", 1, MAX_ARG, lux_subsc_func, /* subsc.c */ "1INNER:2OUTER:4ZERO:8SUBGRID:16KEEPDIMS:32ALL:64SEPARATE" },
  { "%C_CPUTIME", 0, 0, lux_cputime, 0 }, /* fun1.c */
  { "%D_POWER",	2, 2, lux_pow, "*" }, /* fun1.c */
  { "%E_CONCAT", 1, MAX_ARG, lux_concat, "1SLOPPY" }, /* subsc.c */
  { "%F_CTIME", 0, 0, lux_ctime, 0 }, /* fun1.c */
  { "%G_TIME", 	0, 0, lux_time, 0 }, /* fun1.c */
  { "%H_DATE", 	0, 0, lux_date, 0 }, /* fun1.c */
  { "%I_READKEY", 0, 0, lux_readkey, 0 }, /* strous.c */
  { "%J_READKEYNE", 0, 0, lux_readkeyne, 0 }, /* strous.c */
  { "%K_SYSTIME", 0, 0, lux_systime, 0 }, /* fun1.c */
  { "%L_JD",	0, 0, lux_jd, 0 }, /* fun1.c */
  { "%M_CJD",   0, 0, lux_cjd, 0 },                /* fun1.c */
  { "ABS",	1, 1, lux_abs, "*" }, /* fun1.c */
  { "ACOS",	1, 1, lux_acos, "*" }, /* fun1.c */
  { "ACOSH",	1, 1, lux_acosh, "*" }, /* fun1.c */
  { "ALOG",	1, 1, lux_log, "*" }, /* fun1.c */
  { "ALOG10",	1, 1, lux_log10, "*" }, /* fun1.c */
  { "ANTILAPLACE2D", 2, 2, lux_antilaplace2d, 0 }, /* poisson.c */
  { "AREACONNECT", 2, 3, lux_area_connect, "::COMPACT:1RAW" }, /* topology.c */
  { "ARESTORE",	1, MAX_ARG, lux_arestore_f, 0 }, /* files.c */
  { "ARG",	1, 1, lux_arg, 0 }, /* fun3.c */
  { "ARRAY",	1, MAX_DIMS + 1, lux_array, 0 }, /* symbols.c */
  { "ASIN",	1, 1, lux_asin, "*" }, /* fun1.c */
  { "ASINH",	1, 1, lux_asinh, "*" }, /* fun1.c */
  { "ASSOC",	2, 3, lux_assoc, "::OFFSET" }, /* symbols.c */
  { "ASTORE",	2, MAX_ARG, lux_astore_f, 0 }, /* files.c */
  { "ASTRF",	1, 2, lux_astrf, "1FROMEQUATORIAL:2FROMECLIPTICAL:4FROMGALACTIC:8TOEQUATORIAL:16TOECLIPTICAL:32TOGALACTIC:64JULIAN:128BESSELIAN" }, /* astron.c */
  { "ASTRON",	2, 7, lux_astropos, /* astron.c */ ":::OBSERVER:EQUINOX:ELEMENTS:TOLERANCE:1ECLIPTICAL:2EQUATORIAL:3HORIZONTAL:4ELONGATION:8XYZ:16LIGHTTIME:32DATE:64TDT:256ABERRATION:512NUTATION:2832APPARENT:1024QELEMENTS:2048FK5:8192CONJSPREAD:16384PLANETOCENTRIC:32768KEEPDIMENSIONS:65536VOCAL:~131072VSOP87A:131072VSOP87C:262144BARE" },
  { "ATAN",	1, 1, lux_atan, "*" }, /* fun1.c */
  { "ATAN2",	2, 2, lux_atan2, "*" }, /* fun1.c */
  { "ATANH",	1, 1, lux_atanh, "*" }, /* fun1.c */
  { "ATOL",	1, 2, lux_strtol, 0 }, /* fun3.c */
  { "BASIN",	1, 2, lux_basin2, /* strous.c */ "*1NUMBER:2SINK:4DIFFERENCE" },
#if DEVELOP
  { "BESSEL_I0",  1, 1, lux_bessel_i0, "*1DEFLATE" }, /* fun1.c */
  { "BESSEL_I1",  1, 1, lux_bessel_i1, "*" }, /* fun1.c */
#endif
  { "BESSEL_J0", 1, 1, lux_j0, "*" }, /* fun1.c */
  { "BESSEL_J1", 1, 1, lux_j1, "*" }, /* fun1.c */
  { "BESSEL_JN", 2, 2, lux_jn, "*" }, /* fun1.c */
#if DEVELOP
  { "BESSEL_K0", 1, 1, lux_bessel_k0, "*" }, /* fun1.c */
  { "BESSEL_K1", 1, 1, lux_bessel_k1, "*" }, /* fun1.c */
  { "BESSEL_KN", 2, 2, lux_bessel_kn, "*" }, /* fun1.c */
#endif
  { "BESSEL_Y0", 1, 1, lux_y0, "*" }, /* fun1.c */
  { "BESSEL_Y1", 1, 1, lux_y1, "*" }, /* fun1.c */
  { "BESSEL_YN", 2, 2, lux_yn, "*" }, /* fun1.c */
  { "BETA",	2, 2, lux_beta, "*1LOG" }, /* fun1.c */
#if DEVELOP
  { "BI0",	1, 1, lux_bessel_i0, "*1DEFLATE" }, /* fun1.c */
  { "BI1",	1, 1, lux_bessel_i1, "*" }, /* fun1.c */
  { "BIGGER235", 1, 1, lux_bigger235, "*" }, /* fun4.c */
#endif
  { "BISECT",	2, 6, lux_bisect, ":::AXIS:POS:WIDTH" }, /* strous3.c */
  { "BJ0",	1, 1, lux_j0, "*" }, /* fun1.c */
  { "BJ1",	1, 1, lux_j1, "*" }, /* fun1.c */
  { "BJN",	2, 2, lux_jn, "*" }, /* fun1.c */
#if DEVELOP
  { "BK0",	1, 1, lux_bessel_k0, "*" }, /* fun1.c */
  { "BK1",	1, 1, lux_bessel_k1, "*" }, /* fun1.c */
  { "BKN",	2, 2, lux_bessel_kn, "*" }, /* fun1.c */
#endif
  { "BMAP",	1, 1, lux_bmap, "*" }, /* subsc.c */
#if DEVELOP
  { "BSMOOTH",	1, 3, lux_bsmooth, 0 }, /* strous.c */
#endif
  { "BY0",	1, 1, lux_y0, "*" }, /* fun1.c */
  { "BY1",	1, 1, lux_y1, "*" }, /* fun1.c */
  { "BYN",	2, 2, lux_yn, "*" }, /* fun1.c */
  { "BYTARR",	1, MAX_DIMS, bytarr, 0 }, /* symbols.c */
  { "BYTE",	1, 1, lux_byte, "*" }, /* symbols.c */
  { "BYTFARR",	3, MAX_DIMS + 1, bytfarr, "%1%OFFSET:1READONLY:2SWAP" }, /* filemap.c */
  { "CALENDAR",	1, 1, lux_calendar, /* astron.c */ "1FROMCOMMON:2FROMGREGORIAN:3FROMISLAMIC:4FROMJULIAN:5FROMHEBREW:6FROMEGYPTIAN:7FROMJD:8FROMCJD:9FROMLUNAR:10FROMMAYAN:11FROMLONGCOUNT:12FROMLATIN:16TOCOMMON:32TOGREGORIAN:48TOISLAMIC:64TOJULIAN:80TOHEBREW:96TOEGYPTIAN:112TOJD:128TOCJD:144TOLUNAR:160TOMAYAN:176TOLONGCOUNT:192TOLATIN:0TONUMERIC:256TOLONG:512TODOUBLE:768TOTEXT:0FROMUTC:1024FROMTAI:2048FROMTT:3072FROMLT:0TOUTC:4096TOTAI:8192TOTT:12288TOLT:0FROMYMD:16384FROMDMY:0TOYMD:32768TODMY" },
  { "CBRT",	1, 1, lux_cbrt, "*" }, /* fun1.c */
  { "CDBLARR",	1, MAX_ARG, cdblarr, 0 }, /* symbols.c */
  { "CDBLFARR", 3, MAX_DIMS + 1, cdblfarr, "%1%OFFSET:1READONLY:2SWAP" }, /* filemap.c */
  { "CDMAP",	1, 1, lux_cdmap, 0 }, /* subsc.c */
  { "CDOUBLE",	1, 1, lux_cdouble, "*" }, /* fun1.c */
  { "CEIL",	1, 1, lux_ceil, "*" }, /* symbols.c */
  { "CFLOAT",	1, 1, lux_cfloat, "*" }, /* fun1.c */
  { "CFLTARR",	1, MAX_ARG, cfltarr, 0 }, /* symbols.c */
  { "CFLTFARR", 3, MAX_DIMS + 1, cfltfarr, "%1%OFFSET:1READONLY:2SWAP" }, /* filemap.c */
  { "CFMAP",	1, 1, lux_cfmap, 0 }, /* subsc.c */
#if HAVE_LIBX11
  { "CHECKMENU", 0, 1, lux_check_menu, 0 }, /* menu.c */
  { "CHECKWINDOW", 0, 1, lux_check_window, 0 }, /* xport.c */
#endif
  { "CHI2",	2, 2, lux_chi_square, "*1COMPLEMENT:2LOG" }, /* fun1.c */
  { "CLASSNAME", 1, 1, lux_classname, 0 }, /* install.c */
#if HAVE_LIBX11
  { "COLORPIXEL", 1, 1, lux_colorpixel, "*" }, /* xport.c */
#endif
#if DEVELOP
  { "COMPILE",	1, 1, lux_compile, 0 }, /* install.c */
#endif
  { "COMPLEX",	1, 1, lux_complex, 0 },	/* fun3.c */
  { "COMPLEXSQUARE", 1, 1, lux_complexsquare, 0 }, /* fun1.c */
  { "COMPRESS",	2, 3, lux_compress, 0 }, /* fun4.c */
  { "CONCAT",	1, MAX_ARG, lux_concat, "1SLOPPY" }, /* subsc.c */
  { "CONJUGATE", 1, 1, lux_conjugate, "*" }, /* fun1.c */
  { "CONSTELLATION", 1, 2, lux_constellation, "1JULIAN:2BESSELIAN:4VOCAL" }, /* astron.c */
  { "CONSTELLATIONNAME", 1, 1, lux_constellationname, 0 }, /* astron.c */
  { "CONVERT",	2, 2, lux_convertsym, "*" }, /* symbols.c */
  { "COS",	1, 1, lux_cos, "*" }, /* fun1.c */
  { "COSH",	1, 1, lux_cosh, "*" }, /* fun1.c */
  { "COVARIANCE", 2, 4, lux_covariance,
    ":::WEIGHTS:*0SAMPLE:1POPULATION:2KEEPDIMS:4DOUBLE" }, /* fun2.c */
  { "CROSSCORR", 2, 3, lux_crosscorr, 0 }, /* fun2.c */
  { "CRUNCH",	3, 3, lux_crunch_f, 0 }, /* crunch.c */
  { "CSPLINE",	0, 5, lux_cubic_spline, /* fun3.c */ "1KEEP:2PERIODIC:4AKIMA:8GETDERIVATIVE" },
  { "CSPLINE_FIND", 2, 4, lux_cspline_find, ":::AXIS:INDEX" }, /* strous3.c */
  { "CTOP",	1, 3, lux_cartesian_to_polar, 0 }, /* fun4.c */
  { "DATE_FROM_TAI", 1, 2, lux_date_from_tai, 0 }, /* ephem.c */
  { "DBLARR",	1, MAX_DIMS, dblarr, 0 }, /* symbols.c */
  { "DBLFARR",	3, MAX_DIMS + 1, dblfarr, /* filemap.c */ "%1%OFFSET:1READONLY:2SWAP" },
  { "DEFINED",	1, 1, lux_defined, "+1TARGET" }, /* fun1.c */
  { "DESPIKE",  1, 6, lux_despike, ":FRAC:LEVEL:NITER:SPIKES:RMS" }, /* fun6.c */
  { "DETREND",	1, 2, lux_detrend, "*" }, /* fun2.c */
  { "DIFFER",	1, 3, lux_differ, "*1CENTRAL:2CIRCULAR" }, /* strous.c */
  { "DILATE",	1, 1, lux_dilate, 0 }, /* fun5.c */
  { "DIMEN",	1, 2, lux_dimen, 0 }, /* subsc.c */
  { "DISTARR",	1, 3, lux_distarr, 0 }, /* strous2.c */
  { "DISTR",	2, 2, lux_distr_f, /* strous.c */ "2IGNORELIMIT:4INCREASELIMIT" },
  { "DMAP",	1, 1, lux_dmap, 0 }, /* subsc.c */
  { "DOUB",	1, 1, lux_double, "*" }, /* symbols.c */
  { "DOUBLE",	1, 1, lux_double, "*" }, /* symbols.c */
  { "DSMOOTH",	3, 3, lux_dir_smooth, "0TWOSIDED:0BOXCAR:1ONESIDED:2GAUSSIAN:4TOTAL:8STRAIGHT" }, /* strous3.c */
  { "DSUM",	1, 4, lux_total, "|1|::POWER:WEIGHTS:2KEEPDIMS" }, /* fun1.c */
  { "EASTERDATE", 1, 1, lux_EasterDate, 0 }, /* astron.c */
  { "ENHANCEIMAGE", 1, 3, lux_enhanceimage, ":PART:TARGET:1SYMMETRIC" }, /* strous3.c */
  { "EQUIVALENCE", 2, 2, lux_equivalence, 0 }, /* strous2.c */
  { "ERF",	1, 1, lux_erf, "*" }, /* fun1.c */
  { "ERFC",	1, 1, lux_erfc, "*" }, /* fun1.c */
  { "ERODE",	1, 1, lux_erode, "1ZEROEDGE" }, /* fun5.c */
  { "ESEGMENT",	1, 4, lux_extreme_general, /* topology.c */ ":SIGN:DIAGONAL:THRESHOLD" },
  { "ESMOOTH",	1, 3, lux_esmooth, 0 }, /* fun2.c */
  { "EVAL",	1, 2, lux_eval, "1ALLNUMBER" }, /* fun3.c */
#if HAVE_LIBX11
  { "EVENTNAME", 0, 1, lux_event_name, 0 }, /* menu.c */
#endif
  { "EXP",	1, 1, lux_exp, "*" }, /* fun1.c */
  { "EXPAND",	2, 4, lux_expand, "1SMOOTH:2NEAREST" }, /* fun4.c */
  { "EXPM1",	1, 1, lux_expm1, "*" }, /* fun1.c */
  { "EXTRACT_BITS", 3, 3, lux_extract_bits_f, 0 }, /* fun3.c */
  { "FCRUNWRITE", 2, 3, lux_fcrunwrite_f, 0 }, /* files.c */
  { "FCRW",	2, 3, lux_fcrunwrite_f, 0 }, /* files.c */
  { "FCW",	2, 3, lux_fcwrite_f, "1RUNLENGTH" }, /* files.c */
  { "FCWRITE",	2, 3, lux_fcwrite_f, "1RUNLENGTH" }, /* files.c */
  { "FFTSHIFT",	2, 2, lux_fftshift_f, 0 }, /* fun3.c */
  { "FILEPTR",	1, 2, lux_fileptr_f, "1START:2EOF:4ADVANCE" }, /* files.c */
  { "FILESIZE",	1, 1, lux_filesize, 0 }, /* files.c */
  { "FILETYPE",	1, 1, lux_identify_file, 0 }, /* files.c */
  { "FILETYPENAME", 1, 1, lux_filetype_name, 0 }, /* install.c */
  { "FIND",	2, 4, lux_find,	/* strous.c */ "0EXACT:1INDEX_GE:2VALUE_GE:4FIRST" },
  { "FIND2",    2, 2, lux_find2, "0EXACT" },    /* strous.c */
  { "FINDFILE",	2, 2, lux_findfile, 0 }, /* files.c */
  { "FIND_MAX",	1, 3, lux_find_max, /* strous2.c */ "::DIAGONAL:1DEGREE:2SUBGRID" },
  { "FIND_MAXLOC", 1, 3, lux_find_maxloc, /* strous2.c */ "::DIAGONAL:1DEGREE:2SUBGRID:4COORDS:8OLD" },
  { "FIND_MIN",	1, 3, lux_find_min, /* strous2.c */ "::DIAGONAL:1DEGREE:2SUBGRID" },
  { "FIND_MINLOC", 1, 3, lux_find_minloc, /* strous2.c */ "::DIAGONAL:1DEGREE:2SUBGRID:4COORDS" },
  { "FIT",	3, 17, lux_generalfit, /* fit.c */ "|4|::START:STEP:LOWBOUND:HIGHBOUND:WEIGHTS:QTHRESH:PTHRESH:ITHRESH:DTHRESH:FAC:NITER:NSAME:ERR:FIT:TTHRESH:1VOCAL:4DOWN:8PCHI:16GAUSSIANS:32POWERFUNC:64ONEBYONE:129VERR" },
#if DEVELOP
  { "FIT2",	4, 11, lux_geneticfit, "X:Y:NPAR:FIT:WEIGHTS:MU:GENERATIONS:POPULATION:PCROSS:PMUTATE:VOCAL:1ELITE:2BYTE:4WORD:6LONG:8FLOAT:10DOUBLE" }, /* fit.c */
#endif
  { "FITS_HEADER", 1, 4, lux_fits_header_f, 0 }, /* files.c */
  { "FITS_KEY",	2, 2, lux_fitskey, "1COMMENT" }, /* strous3.c */
  { "FITS_READ", 2, 7, lux_fits_read_f, "|1|1TRANSLATE:2RAWVALUES::::::BLANK" }, /* files.c */
  { "FITS_XREAD", 2, 6, lux_fits_xread_f, 0 }, /* files.c */
  { "FIX",	1, 1, lux_long, "*" }, /* symbols.c */
  { "FLOAT",	1, 1, lux_float, "*" }, /* symbols.c */
  { "FLOOR",	1, 1, lux_floor, "*" }, /* symbols.c */
  { "FLTARR",	1, MAX_DIMS, fltarr, 0 }, /* symbols.c */
  { "FLTFARR",	3, MAX_DIMS + 1, fltfarr, /* filemap.c */ "%1%OFFSET:1READONLY:2SWAP" },
  { "FMAP",	1, 1, lux_fmap, 0 }, /* subsc.c */
  { "FRATIO",	3, 3, lux_f_ratio, "*1COMPLEMENT:2LOG" }, /* fun1.c */
  { "FREADF",	2, MAX_ARG, lux_freadf_f, "|1|1EOF" }, /* files.c */
  { "FREADS",	3, MAX_ARG, lux_freads_f, "1COUNTSPACES" }, /* files.c */
  { "FSTRING",	1, MAX_ARG, lux_fstring, "1SKIP_UNDEFINED" }, /* fun2.c */
  { "FSUM",	1, 4, lux_total, "::POWER:WEIGHTS:1DOUBLE:2KEEPDIMS" }, /* fun1.c */
  { "FZARR",	1, 1, lux_fzarr, "1READONLY" }, /* filemap.c */
  { "FZH",	1, 2, lux_fzhead_f, 0 }, /* files.c */
  { "FZHEAD",	1, 2, lux_fzhead_f, 0 }, /* files.c */
  { "FZR",	2, 3, lux_fzread_f, "|1|1PRINTHEADER" }, /* files.c */
  { "FZREAD",	2, 3, lux_fzread_f, "|1|1PRINTHEADER" }, /* files.c */
  { "FZW",	2, 3, lux_fzwrite_f, 0 }, /* files.c */
  { "FZWRITE",	2, 3, lux_fzwrite_f, 0 }, /* files.c */
  { "GAMMA",	1, 1, lux_gamma, "*1LOG" }, /* fun1.c */
#if HAVE_REGEX_H
  { "GETDIRECTORIES", 1, 2, lux_getdirectories, 0 }, /* files.c */
  { "GETDIRECTS", 1, 2, lux_getdirectories, 0 }, /* files.c */
#endif
  { "GETENV",	1, 1, lux_getenv, 0 }, /* files.c */
#if HAVE_REGEX_H
  { "GETFILES", 1, 2, lux_getfiles, 0 }, /* files.c */
  { "GETFILES_R", 1, 2, lux_getfiles_r, 0 }, /* files.c */
  { "GETMATCHEDFILES", 2, 3, lux_getmatchedfiles, 0 }, /* files.c */
  { "GETMATCHEDFILES_R", 2, 3, lux_getmatchedfiles_r, 0 }, /* files.c */
#endif
  { "GET_LUN",	0, 0, lux_get_lun, 0 }, /* files.c */
  { "GIFREAD",	2, 3, lux_gifread_f, 0 }, /* gifread_ana.c */
  { "GIFWRITE",	2, 3, lux_gifwrite_f, 0 }, /* gifwrite_ana.c */
  { "GRIDMATCH", 7, 8, lux_gridmatch, "1VOCAL" }, /* fun4.c */
  { "GSMOOTH",	1, 4, lux_gsmooth, /* fun2.c */
    ":::KERNEL:1NORMALIZE:2FULLNORM:4BALANCED:8ALL" },
  { "HAMMING",  1, 2, lux_hamming, 0 }, /* strous3.c */
  { "HIST",	1, 2, lux_hist, /* fun3.c */
    "1FIRST:2IGNORELIMIT:4INCREASELIMIT:8SILENT" },
  { "HISTR",	1, 1, lux_histr, /* fun3.c */
    "1FIRST:2IGNORELIMIT:4INCREASELIMIT:8SILENT" },
  { "IBETA",	3, 3, lux_incomplete_beta, "*1COMPLEMENT:2LOG" }, /* fun1.c */
  { "IDLREAD",	2, 2, lux_idlread_f, 0 }, /* strous3.c */
  { "IGAMMA",	2, 2, lux_incomplete_gamma, "*1COMPLEMENT:2LOG" }, /* fun1.c */
  { "IMAGINARY", 1, 1, lux_imaginary, 0 }, /* fun3.c */
  { "INDEX",	1, 1, lux_index, "*1AXIS:2RANK" }, /* fun4.c */
  { "INDGEN",	1, 2, lux_indgen, "*" }, /* fun1.c */
  { "INPOLYGON", 4, 4, lux_inpolygon, 0 }, /* topology.c */
  { "INT",	1, 1, lux_word, "*" }, /* symbols.c */
  { "INTARR",	1, MAX_DIMS, intarr, 0 }, /* symbols.c */
  { "INTFARR",	3, MAX_DIMS + 1, intfarr, /* filemap.c */
    "%1%OFFSET:1READONLY:2SWAP" },
  { "ISARRAY",	1, 1, lux_isarray, 0 }, /* subsc.c */
  { "ISNAN",	1, 1, lux_isnan, 0 }, /* fun1.c; needs IEEE isnan! */
  { "ISSCALAR",	1, 1, lux_isscalar, 0 }, /* subsc.c */
  { "ISSTRING",	1, 1, lux_isstring, 0 }, /* subsc.c */
  { "IST",	1, 3, lux_istring, 0 }, /* fun2.c */
  { "ISTRING",	1, 3, lux_istring, 0 }, /* fun2.c */
#if HAVE_LIBJPEG
  { "JPEGREAD",	2, 4, lux_read_jpeg6b_f, ":::SHRINK:1GREYSCALE" }, /* jpeg.c */
  { "JPEGWRITE", 2, 4, lux_write_jpeg6b_f, 0 }, /* jpeg.c */
#endif
  { "KSMOOTH",	2, 3, lux_ksmooth, "1BALANCED" }, /* fun2.c */
  { "LAPLACE2D",	1, 1, lux_laplace2d, 0 }, /* poisson.c */
  { "LEGENDRE", 2, 2, lux_legendre, "1NORMALIZE" }, /* strous3.c */
  { "LLSQ",	2, 9, lux_lsq2,	/* strous2.c */
    ":::FWHM:WEIGHTS:COV:ERR:CHISQ:1FORMAL:2REDUCE" },
  { "LMAP",	1, 1, lux_lmap, 0 }, /* subsc.c */
  { "LOCAL_MAX", 2, 2, lux_local_maxf, /* strous.c */
    "3SUBGRID:2BOUND" },
  { "LOCAL_MAXLOC", 2, 2, lux_local_maxloc, /* strous.c */
    "3SUBGRID:2BOUND:4RELATIVE" },
  { "LOCAL_MIN", 2, 2, lux_local_minf, /* strous.c */
    "3SUBGRID:2BOUND" },
  { "LOCAL_MINLOC", 2, 2, lux_local_minloc, /* strous.c */
    "3SUBGRID:2BOUND:4RELATIVE" },
  { "LOG",	1, 1, lux_log, "*" }, /* fun1.c */
  { "LOG10",	1, 1, lux_log10, "*" }, /* fun1.c */
  { "LOG1P",	1, 1, lux_log1p, "*" }, /* fun1.c */
  { "LONARR",	1, MAX_DIMS, lonarr, 0 }, /* symbols.c */
  { "LONFARR",	3, MAX_DIMS+1, lonfarr, /* filemap.c */
    "%1%OFFSET:1READONLY:2SWAP" },
  { "LONG",	1, 1, lux_long, "*" }, /* symbols.c */
  { "LOWCASE",	1, 1, lux_lower, 0 }, /* fun2.c */
  { "LOWER",	1, 1, lux_lower, 0 }, /* fun2.c */
  { "LSMOOTH",	3, 3, lux_dir_smooth2, /* strous3.c */
    "0TWOSIDED:0BOXCAR:1ONESIDED:2GAUSSIAN:4NORMALIZE:8STRAIGHT" },
  { "LSQ",	2, 6, lux_lsq,	/* strous2.c */
    "::WEIGHTS:COV:ERR:CHISQ:1FORMAL:2REDUCE" },
  { "MATCH",	2, 2, lux_match, 0 }, /* strous.c */
  { "MAX",	1, 2, lux_maxf, "1KEEPDIMS" }, /* fun3.c */
  { "MAXDIR",	2, 3, lux_max_dir, 0 },	/* topology.c */
  { "MAXFILTER", 1, 3, lux_maxfilter, 0 }, /* strous2.c */
  { "MAXLOC",	1, 2, lux_maxloc, "1KEEPDIMS" }, /* fun3.c */
  { "MEAN", 	1, 4, lux_mean, "::POWER:WEIGHTS:1DOUBLE:2KEEPDIMS:4FLOAT" }, /* fun1.c */
  { "MEDFILTER", 1, 4, lux_medfilter, "%1%" }, /* strous2.c */
  { "MEDIAN",	1, 3, lux_median, "%1%" }, /* strous2.c */
  { "MEMORY",	0, 0, lux_memory, 0 }, /* memck.c */
  { "MIN",	1, 2, lux_minf, "1KEEPDIMS" }, /* fun3.c */
  { "MINFILTER", 1, 3, lux_minfilter, 0 }, /* strous2.c */
  { "MINLOC",	1, 2, lux_minloc, "1KEEPDIMS" }, /* fun3.c */
  { "NCCHI2",	3, 3, lux_noncentral_chi_square, 0 }, /* fun1.c */
  { "NOT",	1, 1, lux_not, "*" }, /* strous.c */
  { "NUM_DIM",	1, 1, lux_num_dimen, 0 }, /* subsc.c */
  { "NUM_ELEM", 1, 2, lux_num_elem, 0 }, /* subsc.c */
  { "ONE",	1, 1, lux_onef, 0 }, /* fun1.c */
  { "OPENR",	2, 2, lux_openr_f, "1GET_LUN" }, /* files.c */
  { "OPENU",	2, 2, lux_openu_f, "1GET_LUN" }, /* files.c */
  { "OPENW",	2, 2, lux_openw_f, "1GET_LUN" }, /* files.c */
  /* { "ORBITELEM",	3, 3, lux_orbitalElement, 0, */
  { "ORDFILTER", 1, 4, lux_orderfilter, /* strous2.c */
    "%1%ORDER:1MEDIAN:2MINIMUM:3MAXIMUM" },
  { "PIT",	1, 3, lux_pit, 0 }, /* fun2.c */
  { "POLATE",	3, 3, lux_table, 0 }, /* strous.c */
  { "POLY",	2, 2, lux_poly, "*" }, /* fun2.c */
  { "POWER",	1, 2, lux_power, "1POWER:2SHAPE:4ONEDIM" }, /* fun3.c */
  { "PRECESS",	3, 3, lux_precess, "1JULIAN:2BESSELIAN" }, /* astron.c */
  { "PRINTF",	1, MAX_ARG, lux_printf_f, 0 }, /* files.c */
#if DEVELOP
  { "PROJECT",	1, 1, lux_project, 0 }, /* projection.c */
  { "PROJECTMAP", 2, 8, lux_projectmap, "::HDIST:ANGLE:MAG:XMAP:YMAP:SIZE" }, /* projection.c */
#endif
  { "PSUM",	2, 4, lux_psum, /* strous2.c */
    "1ONEDIM:2VNORMALIZE:4CNORMALIZE:8SINGLE" },
  { "PTOC",	1, 5, lux_polar_to_cartesian, 0 }, /* fun4.c */
  { "QUANTILE",	2, 3, lux_quantile, 0 }, /* strous2.c */
  { "RANDOM",	1, MAX_DIMS, lux_random, /* random.c */
    "%2%SEED:PERIOD:1UNIFORM:2NORMAL:3SAMPLE:4SHUFFLE:5BITS" },
  { "RANDOMB",	3, MAX_DIMS, lux_randomb, "%2%SEED:1LONG" }, /* random.c */
  { "RANDOMD",  3, MAX_DIMS, lux_randomd, "%1%SEED" }, /* random.c */
  { "RANDOML",	3, MAX_DIMS, lux_randoml, "%2%SEED:1DOUBLE" }, /* random.c */
  { "RANDOMN",	3, MAX_DIMS, lux_randomn, "%2%SEED" }, /* random.c */
  { "RANDOMU",	3, MAX_DIMS, lux_randomu, "%2%SEED:PERIOD" }, /* random.c */
  { "READF",	2, MAX_ARG, lux_readf_f, "1ASKMORE:2WORD" }, /* files.c */
  { "READU",	2, MAX_ARG, lux_readu_f, 0 }, /* files.c */
#if HAVE_LIBJPEG
  { "READ_JPEG", 2, 4, lux_read_jpeg6b_f, ":::SHRINK:1GREYSCALE" }, /* jpeg.c */
#endif
  { "REAL",	1, 1, lux_real, 0 }, /* fun3.c */
  { "REDIM",	2, 9, lux_redim_f, 0 }, /* subsc.c */
#if HAVE_REGEX_H
  { "REGEX",	1, 2, lux_regex, "1CASE" }, /* regex.c */
#endif
  { "REGRID",	5, 5, lux_regrid, 0 }, /* fun4.c */
  { "REGRID3",	5, 5, lux_regrid3, 0 }, /* fun4.c */
  { "REGRID3NS", 5, 5, lux_regrid3ns, 0 }, /* fun4.c */
#if DEVELOP
  { "REGRIDLS", 5, 5, lux_regridls, 0 }, /* fun4.c */
#endif
  { "REORDER",	2, 2, lux_reorder, 0 },	/* fun6.c */
  { "RESTORE",	2, 3, lux_fzread_f, "1PRINTHEADER" }, /* files.c */
  { "REVERSE",	1, MAX_ARG, lux_reverse, "1ZERO" }, /* subsc.c */
  { "RFIX",	1, 1, lux_rfix, "*" }, /* symbols.c */
  { "ROLL",	2, 2, lux_roll, 0 }, /* subsc.c */
  { "ROOT3",	3, 3, lux_root3, 0 }, /* orientation.c */
  { "RUNCUM",	1, 3, lux_runcum, "*1PARTIAL_WIDTH:2VARSUM" }, /* strous.c */
  { "RUNPROD",	1, 2, lux_runprod, "*" }, /* strous2.c */
  { "RUNSUM",	1, 3, lux_runsum, "*" }, /* fun2.c */
  { "SCALE",	1, 3, lux_scale, "*1FULLRANGE:2ZOOM" }, /* fun3.c */
  { "SCALERANGE", 3, 5, lux_scalerange, "*1FULLRANGE:2ZOOM" }, /* fun3.c */
  { "SCANF",	2, MAX_ARG, lux_freadf_f, "|1|1EOF" }, /* files.c */
  { "SDEV",	1, 3, lux_sdev, /* fun2.c */
    "::WEIGHTS:*0SAMPLE:1POPULATION:2KEEPDIMS:4DOUBLE" },
  { "SEGMENT",	1, 3, lux_segment, ":SIGN:DIAGONAL:1DEGREE" }, /* topology.c */
  { "SEGMENTDIR", 2, 3, lux_segment_dir, "::SIGN" }, /* topology.c */
  { "SGN",	1, 1, lux_sgn, "*" }, /* fun1.c */
  { "SHIFT",	1, 4, lux_shift_f, ":::BLANK:1TRANSLATE" }, /* strous2.c */
  { "SHIFT3",	2, 3, lux_shift3, 0 }, /* fun4.c */
  { "SIDEREALTIME", 1, 1, lux_siderealtime, "1ATZEROTIME" }, /* astron.c */
  { "SIEVE",	1, 2, lux_sieve, 0 }, /* fun3.c */
  { "SIN", 	1, 1, lux_sin, "*" }, /* fun1.c */
  { "SINH",	1, 1, lux_sinh, "*" }, /* fun1.c */
  { "SKIPC",	2, 2, lux_skipc, 0 }, /* fun2.c */
  { "SMAP",	1, 1, lux_smap, "1TRUNCATE:2ARRAY" }, /* subsc.c */
  { "SMOOTH",	1, 3, lux_smooth, "1PARTIAL_WIDTH:4ALL" }, /* strous.c */
  { "SOLAR_B",	1, 1, lux_solar_b, "*2MODERN" }, /* ephem2.c */
  { "SOLAR_L",	1, 1, lux_solar_l, "*1FULL:2MODERN" }, /* ephem2.c */
  { "SOLAR_P",	1, 1, lux_solar_p, "*2MODERN" }, /* ephem2.c */
  { "SOLAR_R",	1, 1, lux_solar_r, "*" }, /* ephem2.c */
  { "SORT",	1, 1, lux_sort, "*1HEAP:2SHELL:4AXIS" }, /* fun4.c */
  { "SPAWN",	1, 1, lux_spawn_f, 0 }, /* files.c */
  { "SPRINTF",	1, MAX_ARG, lux_fstring, "1SKIP_UNDEFINED" }, /* fun2.c */
  { "SQRT",	1, 1, lux_sqrt, "*" }, /* fun1.c */
  { "SSCANF",	3, MAX_ARG, lux_freads_f, "1COUNTSPACES" }, /* files.c */
  { "STORE",	2, 3, lux_fzwrite_f, "1SAFE" }, /* files.c */
  { "STR",	1, MAX_ARG, lux_string, 0 }, /* fun2.c */
  { "STRARR",	2, 1 + MAX_DIMS, strarr, "%1%SIZE" }, /* symbols.c */
  { "STRCOUNT",	2, 2, lux_strcount, 0 }, /* fun2.c */
  { "STRETCH",	2, 2, lux_stretch, 0 }, /* fun4.c */
  { "STRING",	1, MAX_ARG, lux_string, 0 }, /* fun2.c */
  { "STRLEN",	1, 1, lux_strlen, 0 }, /* fun2.c */
  { "STRLOC",	2, 2, lux_strloc, 0 }, /* fun2.c */
  { "STRPBRK",	2, 2, lux_strpbrk, 0 },	/* fun2.c */
  { "STRPOS",	2, 3, lux_strpos, 0 }, /* fun2.c */
  { "STRR",	3, 4, lux_strreplace, 0 }, /* fun2.c */
  { "STRREPLACE", 3, 4, lux_strreplace, 0 }, /* fun2.c */
  { "STRSKP",	2, 2, lux_strskp, 0 }, /* fun2.c */
  { "STRSUB",	3, 3, lux_strsub, 0 }, /* fun2.c */
  { "STRTOK",	1, 2, lux_strtok, "1SKIP_FINAL_DELIM" }, /* fun2.c */
  { "STRTOL",	1, 2, lux_strtol, 0 }, /* fun3.c */
  { "STRTRIM",	1, 1, lux_strtrim, 0 }, /* fun2.c */
#if DEVELOP
  { "STRUCT",	2, MAX_ARG, lux_struct, 0 }, /* install.c */
#endif
  { "STUDENT",	2, 2, lux_student, "*1COMPLEMENT:2LOG" }, /* fun1.c */
  { "SUN_B",	2, 2, lux_sun_b, 0 }, /* ephem.c */
  { "SUN_D",	2, 2, lux_sun_d, 0 }, /* ephem.c */
  { "SUN_P",	2, 2, lux_sun_p, 0 }, /* ephem.c */
  { "SUN_R",	2, 2, lux_sun_r, 0 }, /* ephem.c */
  { "SYMBOL",	1, 1, lux_symbol, "1MAIN" }, /* symbols.c */
  { "SYMCLASS",	1, 1, lux_symclass, "+|1|1FOLLOW:2NUMBER" }, /* subsc.c */
  { "SYMDTYPE",	1, 1, lux_symdtype, 0 }, /* subsc.c */
  { "SYMMEM",	0, 0, lux_symbol_memory, 0 }, /* install.c */
  { "SYMNUM",	1, 1, lux_symbol_number, 0 }, /* install.c */
  { "TABLE",	3, 4, lux_table, "|2|1ALL:2MIDDLE" }, /* strous.c */
  { "TAI_FROM_DATE", 5, 5, lux_tai_from_date, 0 }, /* ephem.c */
  { "TAN", 	1, 1, lux_tan, "*" }, /* fun1.c */
  { "TANH", 	1, 1, lux_tanh, "*" }, /* fun1.c */
  { "TEMPORARY", 1, 1, lux_temp, 0 }, /* strous2.c */
  { "TENSE",	3, 6, lux_tense, 0 }, /* fun3.c */
  { "TENSE_CURVE", 3, 6, lux_tense_curve, 0 }, /* fun3.c */
  { "TENSE_LOOP", 3, 4, lux_tense_loop, 0 }, /* fun3.c */
#if DEVELOP
  { "TEST",	2, 3, lux_test, 0 }, /* execute.c */
#endif
  { "TOTAL", 	1, 4, lux_total, "::POWER:WEIGHTS:1DOUBLE:2KEEPDIMS:4FLOAT" }, /* fun1.c */
  { "TRACE_DECODER", 3, 3, lux_trace_decoder, 0 }, /* trace_decoder_ana.c */
  { "TREND",	1, 2, lux_trend, "*" }, /* fun2.c */
  { "TRI_NAME_FROM_TAI", 1, 1, lux_tri_name_from_tai, 0 }, /* ephem.c */
#if HAVE_LIBX11
  { "TVREAD",	1, 5, lux_xtvread, "1GREYSCALE" }, /* xport.c */
#endif
  { "TYPENAME",	1, 1, lux_typeName, 0 }, /* install.c */
  { "UPCASE",	1, 1, lux_upper, 0 }, /* fun2.c */
  { "UPPER",	1, 1, lux_upper, 0 }, /* fun2.c */
  { "VARIANCE", 1, 3, lux_variance, /* fun2.c */
    "::WEIGHTS:*0SAMPLE:1POPULATION:2KEEPDIMS:4DOUBLE" },
  { "VARNAME",	1, 1, lux_varname, 0 }, /* symbols.c */
  { "VOIGT",	2, 2, lux_voigt, "*" }, /* fun1.c */
  { "WMAP",	1, 1, lux_wmap, 0 }, /* subsc.c */
  { "WORD",	1, 1, lux_word, "*" }, /* symbols.c */
  { "WRITEU",	2, MAX_ARG, lux_writeu, 0 }, /* files.c */
#if HAVE_LIBJPEG
  { "WRITE_JPEG", 2, 4, lux_write_jpeg6b_f, 0 }, /* jpeg.c */
#endif
#if HAVE_LIBX11
  { "XEXIST",	1, 1, lux_xexist, 0 }, /* xport.c */
  { "XLABELWIDTH", 1, 1, lux_xlabelwidth, 0 }, /* xport.c */
#if MOTIF
  { "XMADDFILETOLIST", 2, 2, lux_xmaddfiletolist, 0 }, /* motif.c */
  { "XMARROW",	3, 4, lux_xmarrow, 0 }, /* motif.c */
  { "XMBOARD",	1, 5, lux_xmboard, 0 }, /* motif.c */
  { "XMBUTTON",	3, 5, lux_xmbutton, 0 }, /* motif.c */
  { "XMCHECKBOX", 5, 32, lux_xmcheckbox, 0 }, /* motif.c */
  { "XMCOLUMNS", 3, 5, lux_xmcolumns, 0 }, /* motif.c */
  { "XMCOMMAND", 3, 7, lux_xmcommand, 0 }, /* motif.c */
  { "XMDIALOG_BOARD", 4, 9, lux_xmdialog_board, 0 }, /* motif.c */
  { "XMDIALOG_FORM", 4, 8, lux_xmdialog_form, 0 }, /* motif.c */
  { "XMDRAWINGAREA", 2, 7, lux_xmdrawingarea, 0 }, /* motif.c */
  { "XMFILEGETLIST",	1, 1, lux_xmfilegetlist, 0 }, /* motif.c */
  { "XMFILESELECT", 3, 12, lux_xmfileselect, 0 }, /* motif.c */
  { "XMFORM",	1, 7, lux_xmform, 0 }, /* motif.c */
  { "XMFRAME",	1, 5, lux_xmframe, 0 }, /* motif.c */
  { "XMGETOPTIONSELECTION", 1, 1, lux_xmgetoptionselection, 0 }, /* motif.c */
  { "XMGETWIDGETADDRESS", 1, 1, lux_xmgetwidgetaddress, 0 }, /* motif.c */
  { "XMHSCALE",	5, 8, lux_xmhscale, 0 }, /* motif.c */
  { "XMHSCROLLBAR",	5, 6, lux_xmhscrollbar, 0 }, /* motif.c */
  { "XMLABEL",	2, 5, lux_xmlabel, 0 }, /* motif.c */
  { "XMLIST",	3, 6, lux_xmlist, 0 }, /* motif.c */
  { "XMLISTCOUNT", 1, 1, lux_xmlistcount, 0 }, /* motif.c */
  { "XMLISTFROMFILE", 4, 6, lux_xmlistfromfile, 0 }, /* motif.c */
  { "XMMENUBAR", 4, 32, lux_xmmenubar, 0 }, /* motif.c */
  { "XMOPTIONMENU", 6, 50, lux_xmoptionmenu, 0 }, /* motif.c */
  { "XMPIXMAPBUTTON", 3, 3, lux_xmpixmapbutton, 0 }, /* motif.c */
  { "XMPIXMAPOPTIONMENU", 6, 32, lux_xmpixmapoptionmenu, 0 }, /* motif.c */
  { "XMPULLDOWNMENU", 6, 32, lux_xmpulldownmenu, 0 }, /* motif.c */
  { "XMRADIOBOX", 5, 32, lux_xmradiobox, 0 }, /* motif.c */
  { "XMROWS",	3, 5, lux_xmrows, 0 }, /* motif.c */
  { "XMSCALEGETVALUE", 1, 1, lux_xmscalegetvalue, 0 }, /* motif.c */
  { "XMSCROLLEDWINDOW", 3, 3, lux_xmscrolledwindow, 0 }, /* motif.c */
  { "XMSCROLLEDWINDOWAPP", 1, 4, lux_xmscrolledwindowapp, 0 }, /* motif.c */
  { "XMSEPARATOR", 1, 4, lux_xmseparator, 0 }, /* motif.c */
  { "XMTEXT",	1, 5, lux_xmtext, 0 }, /* motif.c */
  { "XMTEXTBOX", 1, 5, lux_xmtextbox, 0 }, /* motif.c */
  { "XMTEXTFIELD", 4, 8, lux_xmtextfield, 0 }, /* motif.c */
  { "XMTEXTFIELDARRAY", 9, 40, lux_xmtextfieldarray, 0 }, /* motif.c */
  { "XMTEXTFIELDGETSTRING", 1, 1, lux_xmtextfieldgetstring, 0 }, /* motif.c */
  { "XMTEXTFROMFILE", 2, 6, lux_xmtextfromfile, 0 }, /* motif.c */
  { "XMTEXTGETINSERTPOSITION", 1, 1, lux_xmtextgetinsertposition, 0 }, /* motif.c */
  { "XMTEXTGETLASTPOSITION",	1, 1, lux_xmtextgetlastposition, 0 }, /* motif.c */
  { "XMTEXTGETSELECTION",	1, 3, lux_xmtextgetselection, 0 }, /* motif.c */
  { "XMTEXTGETSTRING", 1, 1, lux_xmtextgetstring, 0 }, /* motif.c */
  { "XMTOGGLEGETSTATE", 1, 1, lux_xmtogglegetstate, 0 }, /* motif.c */
  { "XMTOPLEVEL_BOARD", 3, 5, lux_xmtoplevel_board, 0 }, /* motif.c */
  { "XMTOPLEVEL_FORM", 3, 7, lux_xmtoplevel_form, 0 }, /* motif.c */
  { "XMVSCALE",	5, 8, lux_xmvscale, 0 }, /* motif.c */
  { "XMVSCROLLBAR",	5, 6, lux_xmvscrollbar, 0 }, /* motif.c */
#endif
  { "XQUERY",	0, 1, lux_xquery_f, 0 }, /* xport.c */
#if MOTIF
  { "XTPARENT",	1, 1, lux_xtparent, 0 }, /* motif.c */
#endif
  { "XTVREAD",	1, 5, lux_xtvread, "1GREYSCALE" }, /* xport.c */
#if MOTIF
  { "XTWINDOW",	1, 1, lux_xtwindow, 0 }, /* motif.c */
#endif
#endif
  { "ZERO", 	1, 1, lux_zerof, "*" }, /* fun1.c */
  { "ZERONANS",	1, 2, lux_zapnan_f, "*%1%VALUE" }, /* fun1.c */
  { "ZINV",	1, 1, lux_zinv, "*" }, /* strous.c */
};
int32_t nFunction = sizeof(function_table)/sizeof(internalRoutine);
/*----------------------------------------------------------------*/
void undefine(int32_t symbol)
/* free up memory allocated for <symbol> and make it undefined */
{
  void	zap(int32_t), updateIndices(void);
  char	hasMem = 0;
  int32_t	n, k, oldZapContext, i;
  int16_t	*ptr;
  pointer	p2;
  listElem	*p;
  extractSec	*eptr, *eptr0;
  extern int32_t	tempSym;
  extern char	restart;

  if (symbol < 0 || symbol > NSYM)
    cerror(ILL_SYM, 0, symbol, "undefine");
  if (symbol < nFixed && !restart) {
    luxerror("Constant of nature cannot be deleted!\n", symbol);
    return;
  }
  switch (symbol_class(symbol)) {
    case LUX_SCALAR: case LUX_POINTER: case LUX_TRANSFER:
    case LUX_UNDEFINED: case LUX_UNUSED: case LUX_FUNC_PTR:
    case LUX_DEFERRED_SUBR: case LUX_DEFERRED_FUNC: case LUX_DEFERRED_BLOCK:
      break;
    case LUX_SCAL_PTR:
      if (symbol_type(symbol) == LUX_TEMP_STRING) {
	free(scal_ptr_pointer(symbol).s);
      }
      break;
    case LUX_ARRAY:
      if (isStringType(array_type(symbol))) { /* a string array */
	/* must free the components' memory */
	p2.sp = array_data(symbol);
	n = array_size(symbol);
	while (n--)
	  free(*p2.sp++);
      }
      /* fall through to generic case to take care of remaining */
      /* array memory. */
    case LUX_STRING: case LUX_SUBSC_PTR: case LUX_FILEMAP:
    case LUX_ASSOC: case LUX_CSCALAR: case LUX_CARRAY: case LUX_STRUCT:
      hasMem = 1;
      break;
    case LUX_META:
      if (symbol_context(k = meta_target(symbol)) == symbol
	  || (zapContext > 0 && symbol_context(k) == zapContext))
	zap(k);
      break;
    case LUX_RANGE: case LUX_PRE_RANGE:
      k = range_start(symbol);
      if (k < 0)
	k = -k;
      if (symbol_context(k) == symbol
	  || (zapContext > 0 && symbol_context(k) == zapContext))
	zap(k);
      k = range_end(symbol);
      if (k < 0)
	k = -k;
      if (symbol_context(k) == symbol
	  || (zapContext > 0 && symbol_context(k) == zapContext))
	zap(k);
      k = range_redirect(symbol);
      if (k > 0 && (symbol_context(k) == symbol
		    || (zapContext > 0 && symbol_context(k) == zapContext)))
	zap(k);
      break;
    case LUX_LIST_PTR:
      n = list_ptr_target(symbol);
      if (n > 0) 		/* string key */
	free(list_ptr_tag_string(symbol));
      hasMem = 0;
      break;
    case LUX_LIST: case LUX_PRE_LIST:
      p = list_symbols(symbol);
      n = list_num_symbols(symbol);
      while (n--) {
	if ((symbol_context(k = p->value) == symbol)
	    || (zapContext > 0 && symbol_context(k) == zapContext))
	  zap(k);
	if (p->key)
	  free(p->key);
	p++;
      }
      hasMem = 1;
      break;
    case LUX_CLIST: case LUX_PRE_CLIST: case LUX_CPLIST:
      ptr = clist_symbols(symbol);
      n = clist_num_symbols(symbol);
      while (n--)
	if (symbol_context(k = *ptr++) == symbol
	    || (zapContext > 0 && symbol_context(k) == zapContext))
	  zap(k);
      hasMem = 1;
      break;
    case LUX_KEYWORD:
      if (symbol_context(k = keyword_name_symbol(symbol)) == symbol
	  || (zapContext > 0 && symbol_context(k) == zapContext))
	zap(k);
      if ((symbol_context(k = keyword_value(symbol)) == symbol
	   || (zapContext > 0 && symbol_context(k) == zapContext))
	  && k > tempSym)
	zap(k);
      break;
    case LUX_SUBROUTINE: case LUX_FUNCTION: case LUX_BLOCKROUTINE:
      oldZapContext = zapContext;
      zapContext = symbol;
      ptr = routine_parameters(symbol);
      n = routine_num_parameters(symbol);
      while (n--)
	zap(*ptr++);
      n = routine_num_statements(symbol);
      while (n--) {
	if (*ptr > 0)		/* BREAK, CONTINUE, RETALL, RETURN are < 0 */
	  zap(*ptr);
	ptr++;
      }
      n = routine_num_parameters(symbol);
      if (n)
	free(routine_parameter_names(symbol));
      n += routine_num_statements(symbol);
      if (n)
	free(routine_parameters(symbol));
      zapContext = oldZapContext;
      break;
    case LUX_EXTRACT: case LUX_PRE_EXTRACT:
      if (symbol_class(symbol) == LUX_EXTRACT) {
	n = extract_target(symbol);
	if (n > 0		/* an expression */
	    && symbol_context(n) == symbol)
	  zap(n);
	eptr = eptr0 = extract_ptr(symbol);
	n = extract_num_sec(symbol);
      } else {			/* PRE_EXTRACT */
	free(pre_extract_name(symbol));
	eptr = eptr0 = pre_extract_ptr(symbol);
	n = pre_extract_num_sec(symbol);
      }
      if (n) {
	while (n--) {
	  i = eptr->number;
	  switch (eptr->type) {
	    case LUX_RANGE:
	      p2.w = eptr->ptr.w;
	      while (i--) {
		if (symbol_context(*p2.w) == symbol
		    || (zapContext > 0 && symbol_context(*p2.w) == zapContext))
		  zap(*p2.w);
		p2.w++;
	      }
	      free(eptr->ptr.w);
	      break;
	    case LUX_LIST:
	      p2.sp = eptr->ptr.sp;
	      while (i--)
		free(*p2.sp++);
	      free(eptr->ptr.sp);
	      break;
	  } /* end of switch (eptr->type) */
	  eptr++;
	} /* end of while (n--) */
	free(eptr0);
      }	/* end of if (n) */
      hasMem = 0;
      break;
    case LUX_EVB: case LUX_INT_FUNC: case LUX_USR_FUNC:
      n = 0;
      k = symbol_class(symbol);
      if (k == LUX_EVB)
	k = evb_type(symbol);
      switch (k) {
	case EVB_RETURN:
	  n = 1;
	  break;
	case EVB_REPLACE:  case EVB_REPEAT:  case EVB_WHILE_DO:
	case EVB_DO_WHILE:
	  n = 2;
	  break;
	case EVB_IF:
	  n = 3;
	  break;
	case EVB_FOR:
	  n = 4;
	  break;
	case EVB_USR_CODE:
	  break;
	case EVB_FILE:
	  hasMem = 1;
	  break;
	case EVB_USR_SUB:
	  if (symbol_class(usr_sub_routine_num(symbol)) == LUX_STRING)
	    /* the subroutine has not yet been sought; it's name is
	       stored in the usr_sub_routine_num symbol. */
	    zap(usr_sub_routine_num(symbol));
	  /* fall through to the below case */
	case EVB_INT_SUB: case EVB_INSERT: case LUX_INT_FUNC: case
	LUX_USR_FUNC: case EVB_CASE: case EVB_NCASE: case EVB_BLOCK:
	  n = symbol_memory(symbol)/sizeof(int16_t);
	  ptr = symbol_data(symbol);
	  while (n--)
	    if (symbol_context(k = *ptr++) == symbol
		|| (zapContext > 0 && symbol_context(k) == zapContext))
	      zap(k);
	  hasMem = 1;
	  n = 0;
	  break;
      }
      if (n > 0) {
	ptr = sym[symbol].spec.evb.args;
	while (n--)
	  if (symbol_context(k = *ptr++) == symbol
	      || (zapContext > 0 && symbol_context(k) == zapContext))
	    zap(k);
	if (evb_type(symbol) == EVB_FOR)
	  if (symbol_context(k = for_body(symbol)) == symbol
	      || (zapContext > 0 && symbol_context(k) == zapContext))
	    zap(k);
      }
      break;
    case LUX_BIN_OP: case LUX_IF_OP:
      if (symbol_context(k = bin_op_lhs(symbol)) == symbol
	  || (zapContext > 0 && symbol_context(k) == zapContext))
	zap(k);
      if (symbol_context(k = bin_op_rhs(symbol)) == symbol
	  || (zapContext > 0 && symbol_context(k) == zapContext))
	zap(k);
      break;
    default:
      printf("Sorry, can't delete class %d (%s) yet.\n",
	     symbol_class(symbol), className(symbol_class(symbol)));
      break;
  }
  if (hasMem && symbol_memory(symbol)) {
    free(symbol_data(symbol));
    symbol_memory(symbol) = 0;
  }
  symbol_class(symbol) = LUX_UNDEFINED;
  undefined_par(symbol) = 0;	/* used in usr_routine() (with value 1) */
				/* to indicate unspecified parameters */
				/* of user-defined routines. */
/* context must remain the same, or local variables become global */
/* when they are undefined (e.g. as lhs in a replacement) */
}
/*----------------------------------------------------------------*/
void zap(int32_t nsym)
/* undefine & remove name (if any) */
{
 char	*name, *noName = "[]";
 int32_t	context, hashValue;
 hashTableEntry	*hp, *oldHp, **hashTable;
#if DEBUG
 void	checkTemps(void);
#endif

 if (nsym < 0 || nsym > NSYM)
   cerror(ILL_SYM, 0, nsym, "zap");
 if (!nsym)
   return;
 if (symbol_class(nsym) == LUX_UNUSED)
   return;
 if (nsym < NAMED_START || nsym >= NAMED_END)
   name = noName;
 else
   name = symbolProperName(nsym);
 if (!name)
   name = noName;
 context = sym[nsym].context;
 hashValue = sym[nsym].xx - 1;
 if (*name != '[' && hashValue >= 0) { /* has name */
   switch (symbol_class(nsym)) {
     case LUX_SUBROUTINE: case LUX_DEFERRED_SUBR:
       hashTable = subrHashTable;
       break;
     case LUX_FUNCTION: case LUX_DEFERRED_FUNC:
       hashTable = funcHashTable;
       break;
     case LUX_BLOCKROUTINE: case LUX_DEFERRED_BLOCK:
       hashTable = blockHashTable;
       break;
     default:
       hashTable = varHashTable;
       break;
   }
   hp = oldHp = hashTable[hashValue];
   while (hp) {
     if (!strcmp(hp->name, name) && sym[hp->symNum].context == context) {
       free(hp->name);		/* found name; remove */
       hp->name = 0;
       if (hp != oldHp)
	 oldHp->next = hp->next;
       else
	 hashTable[hashValue] = 0;
       free(hp);
       oldHp = 0;		/* signal that name was found */
       break;
     }
     oldHp = hp;
     hp = hp->next;
   }
   if (oldHp)
     luxerror("Symbol name not found in tables??", nsym);
 }
 undefine(nsym);
 symbol_class(nsym) = LUX_UNUSED;
 symbol_context(nsym) = 0;
 if (nsym >= EXE_START && nsym < EXE_END) {
   nExecutable--;
   if (nsym < executableIndex)
     executableIndex = nsym;
   while (executableIndex > EXE_START
	  && symbol_class(executableIndex - 1) == LUX_UNUSED)
     executableIndex--;
 } else if (nsym >= NAMED_START && nsym < NAMED_END) {
   nNamedVariable--;
   if (nsym < nNamedVariable)
     namedVariableIndex = nsym;
   while (namedVariableIndex > NAMED_START
	  && symbol_class(namedVariableIndex - 1) == LUX_UNUSED)
     namedVariableIndex--;
 } else if (nsym >= TEMPS_START && nsym < TEMPS_END) {
   if (nsym < tempVariableIndex)
     tempVariableIndex = nsym;
   nTempVariable--;
 } else if (nsym >= TEMP_EXE_START && nsym < TEMP_EXE_END) {
   if (nsym < tempExecutableIndex)
     tempExecutableIndex = nsym;
   nTempExecutable--;
 }
#if DEBUG
 checkTemps();
#endif
}
/*----------------------------------------------------------------*/
void cleanUp(int32_t context, int32_t which)
/* names in symbolStack are supposed to be removed after use by the */
/* routines that use them.  only resetting of the index is done here. */
/* all temporary variables that have the specified context are removed */ 
/* <which> can be:  CLEANUP_VARS  CLEANUP_EDBS  CLEANUP_ALL */
/* CLEANUP_COMP CLEANUP_ERROR */
{
  char	comp;
  int32_t	i;
  void	zapParseTemps(void);

  comp = which & CLEANUP_COMP;
/*  while (symbolStackIndex > 0 && !symbolStack[symbolStackIndex])
    symbolStackIndex--; */
  if (context > 0)
  { if (which & CLEANUP_VARS)
    { for (i = TEMPS_START; i < tempVariableIndex; i++)
	if (symbol_class(i) != LUX_UNUSED
	    && comp? symbol_context(i) <= 0: symbol_context(i) == context)
	  zap(i);
      while (tempVariableIndex > TEMPS_START
	     && symbol_class(tempVariableIndex - 1) == LUX_UNUSED)
	tempVariableIndex--; }
    if (which & CLEANUP_EDBS)
    { for (i = curTEIndex; i < tempExecutableIndex; i++)
	if (symbol_class(i) != LUX_UNUSED
	    && comp? symbol_context(i) <= 0: symbol_context(i) == context)
	  zap(i);
      while (tempExecutableIndex > TEMP_EXE_START
	     && symbol_class(tempExecutableIndex - 1) == LUX_UNUSED)
	tempExecutableIndex--; }
  }
  if (which & CLEANUP_ERROR)
    while (markIndex > 0)
    { i = markStack[--markIndex];
      if (i > 0)
	zapTemp(i); }
  else
    zapParseTemps();
}
/*----------------------------------------------------------------*/
void cleanUpRoutine(int32_t context, char keepBase)
/* completely removes all traces of the routine with the given context */
/* keeps the LUX_SUBROUTINE, LUX_FUNCTION, or LUX_BLOCKROUTINE symbol */
/* itself if keepBase is unequal to zero.  If keepBase is unequal to
 zero, then an LUX_DEFERRED_SUBR is transformed into an (empty)
 LUX_SUBR, LUX_DEFERRED_FUNC into an LUX_FUNC, and LUX_DEFERRED_BLOCK
 into an LUX_BLOCK.  LS 19feb97 21may99 */
{
  char	mem;
  int32_t	n;
  int16_t	*ptr;

  if (context < nFixed || context >= NAMED_END) {
    luxerror("Illegal routine or function specified", context);
    return;
  }
  if (keepBase) {
    switch (symbol_class(context)) {
      case LUX_DEFERRED_SUBR:
	symbol_class(context) = LUX_SUBROUTINE;
	routine_num_parameters(context) = 0;
	routine_num_statements(context) = 0;
	break;
      case LUX_DEFERRED_FUNC:
	symbol_class(context) = LUX_FUNCTION;
	routine_num_parameters(context) = 0;
	routine_num_statements(context) = 0;
	break;
      case LUX_DEFERRED_BLOCK:
	symbol_class(context) = LUX_BLOCKROUTINE;
	routine_num_parameters(context) = 0;
	routine_num_statements(context) = 0;
	break;	
      case LUX_SUBROUTINE: case LUX_FUNCTION: case LUX_BLOCKROUTINE:
	n = routine_num_parameters(context);
	mem = n? 1: 0;
	ptr = routine_parameters(context);
	while (n--)		/* get rid of the parameters */
	  zap(*ptr++);
	routine_num_parameters(context) = 0;
	n = routine_num_statements(context);
	if (n)
	  mem = 1;
	ptr = routine_statements(context);
	while (n--) {		/* get rid of the statements */
	  if (*ptr > 0)		/* RETALL, RETURN, BREAK, CONTINUE < 0 */
	    zap(*ptr);
	  ptr++;
	}
	routine_num_statements(context) = 0;
	if (mem)			/* if the routine had any statements */
	  /* or parameters, then we need to free the memory that was used to */
	  /* store them in */
	  free(routine_parameters(context));
	break; 
    }
  } else
    zap(context);
}
/*----------------------------------------------------------------*/
void updateIndices(void)
/* moves tempVariableIndex and tempExecutableIndex as far back as */
/* possible */
{
  while (tempVariableIndex > TEMPS_START
	 && symbol_class(tempVariableIndex - 1) == LUX_UNUSED)
    tempVariableIndex--;
  while (tempExecutableIndex > TEMP_EXE_START
	 && symbol_class(tempExecutableIndex - 1) == LUX_UNUSED)
    tempExecutableIndex--;
}
/*----------------------------------------------------------------*/
int32_t nextFreeStackEntry(void)
     /* returns index to next free item in symbolStack.  May cycle. */
{
  int32_t	oldIndex = symbolStackIndex;

  while (symbolStack[symbolStackIndex]) {
    if (++symbolStackIndex == SYMBOLSTACKSIZE)
      symbolStackIndex = 0;
    if (symbolStackIndex == oldIndex)
      return luxerror("Symbol stack full", 0);
  }
  nSymbolStack++;
  return symbolStackIndex;
}
/*----------------------------------------------------------------*/
int32_t nextFreeNamedVariable(void)
     /* returns index to next free named variable in symbol table.
	some may have been zapped in the meantime, so cycle if at end of
	table */
{
  int32_t	oldIndex = namedVariableIndex;
  extern int32_t	compileLevel;

  while (sym[namedVariableIndex].class) {
    if (++namedVariableIndex == NAMED_END) namedVariableIndex = NAMED_START;
    if (namedVariableIndex == oldIndex)    /* nothing free */
      return luxerror("Too many named variables - symbol table full", 0);
  }
  sym[namedVariableIndex].exec = nExecuted;
  sym[namedVariableIndex].context = -compileLevel;
  sym[namedVariableIndex].line = curLineNumber;
  nNamedVariable++;
  return namedVariableIndex++;
}
/*----------------------------------------------------------------*/
int32_t nextFreeTempVariable(void)
/* returns index to next free temporary variable in symbol table */
{
 extern int32_t	compileLevel;
 int32_t oldIndex = tempVariableIndex;
 static long int count = 0;

 ++count;
 while (sym[tempVariableIndex].class) {
   if (++tempVariableIndex == TEMPS_END)
     tempVariableIndex = TEMPS_START;
   if (tempVariableIndex == oldIndex)
     return luxerror("Too many temp variables - symbol table full", 0);
 }
 sym[tempVariableIndex].exec = nExecuted;
 sym[tempVariableIndex].context = -compileLevel;
 sym[tempVariableIndex].line = curLineNumber;
 mark(tempVariableIndex);
 nTempVariable++;
 return tempVariableIndex++;
}
/*----------------------------------------------------------------*/
int32_t nextFreeTempExecutable(void)
/* returns index to next free temporary executable in symbol table */
{
  int32_t oldIndex = tempExecutableIndex;
  extern int32_t	compileLevel;

  while (sym[tempExecutableIndex].class) {
    if (++tempExecutableIndex == EXE_END) 
      tempExecutableIndex = EXE_START;
    if (tempExecutableIndex == oldIndex) /* nothing free */
      return luxerror("Too many temporary executables - symbol table full", 0);
  }
  sym[tempExecutableIndex].exec = nExecuted;
  sym[tempExecutableIndex].context = -compileLevel;
  sym[tempExecutableIndex].line = curLineNumber;
  nTempExecutable++;
  return tempExecutableIndex++;
}
/*----------------------------------------------------------------*/
int32_t nextFreeExecutable(void)
/* returns index to next free executable in symbol table
   some may have been zapped in the meantime, so cycle if at end
   of table */
{
  int32_t    oldIndex = executableIndex;
  extern int32_t	compileLevel;

  while (sym[executableIndex].class) {
    if (++executableIndex == EXE_END)
      executableIndex = EXE_START;
    if (executableIndex == oldIndex)    /* nothing free */
      return luxerror("Too many permanent executables - symbol table full", 0);
  }
  sym[executableIndex].exec = nExecuted;
  sym[executableIndex].context = -compileLevel;
  sym[executableIndex].line = curLineNumber;
  nExecutable++;
  return executableIndex++;
}
/*----------------------------------------------------------------*/
int32_t nextFreeUndefined(void)
/* returns a free undefined temporary variable */
{
  extern int32_t	compileLevel;
  int32_t	n;

  n = nextFreeTempVariable();
  if (n < 0) return n;		/* some error */
  sym[n].class = LUX_UNDEFINED;
  sym[n].context = -compileLevel;
  return n;
}
/*----------------------------------------------------------------*/
void pushList(int16_t symNum)
/* pushes a symbol number unto the list stack */
{
 if (listStackItem - listStack < NLIST) {
   *listStackItem++ = symNum;
   return;
 }
 luxerror("Too many elements (%d) in list; list stack full\n", 0,
       listStackItem - listStack);
}
/*----------------------------------------------------------------*/
int16_t popList(void)
/* pops a symbol number from the list stack */
{
 if (listStackItem > listStack)
   return *--listStackItem;
 return luxerror("Attempt to read from empty list stack\n", 0);
}
/*----------------------------------------------------------------*/
int32_t moveList(int32_t n)
/* moves the topmost <n> entries on the list stack over by one */
{
  if (listStackItem - listStack < NLIST)
  { memcpy(listStack + 1, listStack, n*sizeof(int16_t));
    listStackItem++;
    return 1; }
  return luxerror("Too many elements (%d) in list; list stack full\n", 0,
	       listStackItem - listStack);
}
/*----------------------------------------------------------------*/
void swapList(int32_t n1, int32_t n2)
/* swaps the elements <n1> and <n2> positions down the list stack */
{
 int32_t	temp;

 temp = listStackItem[-n2];
 listStackItem[-n2] = listStackItem[-n1];
 listStackItem[-n1] = temp;
}
/*----------------------------------------------------------------*/
int32_t stackListLength(void)
/* returns the number of elements in the topmost list in the stack */
/* assumes that all lists are delimited by LUX_NEW_LIST */
{
 int16_t	*i = listStackItem - 1;
 int32_t	n = 0;

 if (i < listStack)
   return -1;		/* no list in stack */
 while (i >= listStack && *i != LUX_NEW_LIST) {
   i--;
   n++;
 }
 return n;
}
/*----------------------------------------------------------------*/
void dupList(void)
/* duplicates the topmost list */
{
  int32_t	n, n2;

  n = n2 = stackListLength();
  pushList(LUX_NEW_LIST);
  while (n2--)
    pushList(listStackItem[-n-1]);
}
/*----------------------------------------------------------------*/
void unlinkString(int32_t index)
/* zero symbolStack[index] and update symbolStackIndex, */
/* but do not free the memory */
{
  symbolStack[index] = 0;
  if (index < symbolStackIndex)
    symbolStackIndex = index;
  while (symbolStackIndex && !symbolStack[symbolStackIndex - 1])
    symbolStackIndex--;
  nSymbolStack--;
}
/*----------------------------------------------------------------*/
void freeString(int32_t index)
     /* removes the string (if any) from index <index> in symbolStack[] */
     /* and updates symbolStackIndex  */
{
  free(symbolStack[index]);
  unlinkString(index);
}
/*----------------------------------------------------------------*/
int32_t installStruct(int32_t base, int32_t key)
/* returns a struct pointer, supports variables, user-functions, */
/* and user-routines as structures */
{
  int32_t	n;

  n = lookForVar(base, curContext); /* seek user-defined item */
  if (n < 0)			/* no such item found */
    n = lookForSubr(base);	/* seek internal subroutine */
  if (n < 0)			/* no such internal subroutine */
    n = lookForFunc(base);	/* seek internal function */
  if (n < 0)			/* no such internal function */
    n = findVar(base, curContext); /* force variable */
  else				/* found something already, remove name */
    freeString(base);
  return newSymbol(LUX_LIST_PTR, n, key);
}
/*----------------------------------------------------------------*/
int32_t copyElement(int32_t symbol, char *key)
/* return a complete copy of variable <symbol>, with name (key) <key>, for */
/* inclusion in a list, structure, or range.  such a variable must */
/* not be a temporary, to prevent its premature deletion. */
{
  int32_t	n;
  int32_t	lux_replace(int32_t, int32_t);

  if ((n = installString(key)) < 0) return n;
  if ((n = findVar(n, 0)) < 0) return n;
  if (lux_replace(n, symbol) < 0) return -1;
  return n;
}
/*----------------------------------------------------------------*/
int32_t findTarget(char *name, int32_t *type, int32_t allowSubr)
/* seeks identification of the name <*name> as a
   variable or a user-defined routine or an internal function.
   User-defined subroutines are only checked if <allowSubr> is non-zero.
   (they must be included when looking for a local variable in a subroutine
   through a structure tag -- e.g., MYFUNC.VAR -- but not when looking for
   a possible target of a subscript)
   Returns in <*type> an identifier of the type of target that was found: the
   variable's class for a named variable, LUX_FUNCTION for a
   user-defined function, LUX_SUBROUTINE for a user-defined subroutine,
   LUX_INT_FUNC for a built-in function, or
   LUX_ERROR if no target was found.  The function's return value
   identifies the particular found target: the symbol number if it's a
   named variable or user-defined function, the appropriate index to
   the function[] array if it's a built-in function, or LUX_ERROR if
   no target was found.  The symbolStack[] entry is removed.
   LS 26dec98 */
{
  int32_t	result;
  FILE	*fp;
  int32_t	nextCompileLevel(FILE *, char *);

  /* seek a named variable */
  result = lookForName(name, varHashTable, curContext);
  if (result >= 0) {
    *type = symbol_class(result);
    return result;
  }
  if (*name == '$' || *name == '!' || *name == '#') {
    /* it's not an existing named variable, yet starts with $ ! or #,
       so it cannot be a function either. */
    *type = LUX_ERROR;
    return luxerror("Subscripted variable %s is undefined", 0, name);
  }
  /* seek a user-defined function */
  result = lookForName(name, funcHashTable, 0);
  if (result >= 0) {
    *type = LUX_FUNCTION;	/* a user-defined function */
    return result;
  }
  if (allowSubr) {
    /* seek a user-defined subroutine */
    result = lookForName(name, subrHashTable, 0);
    if (result >= 0) {
      *type = LUX_SUBROUTINE;
      return result;
    }
  }
  /* seek a built-in function */
  result = findInternalName(name, 0);
  if (result >= 0) {
    *type = LUX_INT_FUNC;	/* an internal function */
    return result;
  }
  /* not a built-in function either.  Try to compile a
     user-defined function */
  fp = openPathFile(name, FIND_FUNC | FIND_LOWER);
  if (fp) {
    /* found it */
    nextCompileLevel(fp, name); /* compile it */
    fclose(fp);
    result = lookForName(name, funcHashTable, 0); /* seek again */
    if (result < 0) {
      *type = LUX_ERROR;
      return luxerror("Compiled file %s but function %s still is not compiled",
		   0, expname, name);
    } else {
      *type = LUX_FUNCTION;
      return result;
    }
  }
  /* we did not find the target anywhere */
  *type = LUX_ERROR;
  return LUX_ERROR;
}
/*----------------------------------------------------------------*/
int32_t newSymbol(int32_t kind, ...)
/* returns index to symbol table for a new symbol.
   arguments depend on kind:
     (LUX_SCALAR, type)
     (LUX_FIXED_NUMBER, type)
     (LUX_FIXED_STRING, symbolStackIndex)
     (LUX_RANGE, range_start, range_end)
     (LUX_EVB, EVB_REPLACE, lhs, rhs)
     (LUX_EVB, EVB_FOR, loopVar, start, end, step, statement)
     (LUX_EVB, EVB_WHILE_DO, condition, statement)
     (LUX_EVB, EVB_DO_WHILE, statement, condition)
     (LUX_EVB, EVB_IF, condition, true_statement, false_statement)
     (LUX_EVB, EVB_CASE)
     (LUX_EVB, EVB_NCASE)
     (LUX_EVB, EVB_INT_SUB)
     (LUX_EVB, EVB_USR_SUB)
     (LUX_EVB, EVB_BLOCK)
     (LUX_EVB, EVB_INSERT, target)
     (LUX_EVB, EVB_RETURN, expr)
     (LUX_EVB, EVB_FILE)
     (LUX_INT_FUNC, funcNr)
     (LUX_BIN_OP, opNr, arg2, arg1)
     (LUX_IF_OP, opNr, arg2, arg1)
     (LUX_KEYWORD, param, expr)
     (LUX_CLIST)
     (LUX_LIST)
     (LUX_LIST_PTR, struct, key)
     (LUX_POINTER, target)
     (LUX_SUBROUTINE, name)
     (LUX_FUNCTION, name)
     (LUX_BLOCKROUTINE, name)
     (LUX_SUBSC_PTR)
     (LUX_META, expr)
     (LUX_EXTRACT)
     (LUX_STRUCT_PTR)
*/
{
  int32_t		n, i, narg, isStruct, isScalarRange, j, target, depth;
  extern char	reportBody, ignoreSymbols, compileOnly;
  extractSec	*eptr;
  int16_t	*ptr;
  pointer	p;
#if YYDEBUG
  extern int32_t	yydebug;
#endif
  /* static char	inDefinition = 0; */
  int16_t		*arg;
  va_list	ap;
  int32_t	int_arg(int32_t);
  void	fixContext(int32_t, int32_t);

 /* don't generate symbols in bodies of routines when using @@file */
 /* (reportBody) and when defining a routine (inDefinition).  */
/* ignoreSymbols = (reportBody && inDefinition); */
  va_start(ap, kind);
  n = -1;
  if (!ignoreSymbols) {		/* really need a new symbol.  what kind? */
    if (kind >= LUX_BIN_OP) {
				/* executable */
      if (keepEVB)
	n = nextFreeExecutable();
      else
	n = nextFreeTempExecutable();
      if (n < 0) {
	va_end(ap);
	return LUX_ERROR;	/* didn't work */
      }
      symbol_class(n) = kind;
      if (keepEVB)
	symbol_context(n) = curContext;
    } else if (kind < LUX_SUBROUTINE) {	/* named variable */
      if (keepEVB)
	n = nextFreeNamedVariable();
      else
	n = nextFreeTempVariable();
      if (n < 0) {
	va_end(ap);
	return LUX_ERROR;
      }
      symbol_class(n) = kind;
      if (keepEVB)
	symbol_context(n) = curContext;
    }
    switch (kind) {
      case LUX_SCALAR:
	scalar_type(n) = va_arg(ap, int32_t);
	break;
      case LUX_FIXED_NUMBER:
	/* same as an ordinary scalar, but in EDB symbol space, so that it */
	/* doesn't get overwritten as "just another temp" (i.e. isFreeTemp() */
	/* returns 0).  otherwise, get problems e.g. in a for-loop, where */
	/* temp numbers are used more than once.  */
	symbol_type(n) = va_arg(ap, int32_t);
	if (symbol_type(n) >= LUX_CFLOAT) { /* a complex scalar */
	  symbol_class(n) = LUX_CSCALAR;
	  complex_scalar_memory(n) = lux_type_size[symbol_type(n)];
	  complex_scalar_data(n).f = malloc(complex_scalar_memory(n));
	  if (!complex_scalar_data(n).f)
	    return cerror(ALLOC_ERR, n);
	} else
	  symbol_class(n) = LUX_SCALAR;
	break;
      case LUX_FIXED_STRING:
	/* a literal string */
	symbol_class(n) = LUX_STRING;
	string_type(n) = LUX_LSTRING;
	string_value(n) = symbolStack[i = va_arg(ap, int32_t)];
	symbol_memory(n) = strlen(symbolStack[i]) + 1; /* count \0 */
	unlinkString(i);		/* free position in stack */
	break;
      case LUX_STRUCT_PTR:
	symbol_class(n) = LUX_STRUCT_PTR;
	symbol_memory(n) = sizeof(structPtr);
	struct_ptr_elements(n) = malloc(symbol_memory(n));
	break;
      case LUX_EXTRACT:
	target = popList();
	if (target > 0) {	/* regular symbol */
	  extract_target(n) = target;
	  extract_num_sec(n) = depth = popList();
	  symbol_memory(n) = depth*sizeof(extractSec);
	  extract_ptr(n) = eptr = malloc(symbol_memory(n));
	  embed(target, n);
	} else {
	  i = -target;
	  symbol_class(n) = LUX_PRE_EXTRACT;
	  symbol_memory(n) = sizeof(preExtract);
	  symbol_data(n) = malloc(sizeof(preExtract));
	  pre_extract_name(n) = symbolStack[i];
	  unlinkString(i);	/* so it does not get zapped */
	  pre_extract_num_sec(n) = depth = popList();
	  pre_extract_ptr(n) = eptr = depth? malloc(depth*sizeof(extractSec)):
	    NULL;
	}
	if (!eptr && depth)
	  return cerror(ALLOC_ERR, 0);
	ptr = listStackItem;
	eptr += depth;		/* start at end of list */
	if (!depth)
	  popList();
	while (depth--) {
	  ptr--;		/* skip the (potential) LUX_NEW_LIST */
	  eptr--;		/* go to previous entry */
	  eptr->type = popList(); /* the type of the current list;
				    either LUX_RANGE or LUX_LIST */
	  eptr->number = stackListLength(); /* the number of entries in this
					      one */
	  i = eptr->number;
	  switch (eptr->type) {
	    case LUX_RANGE:
	      eptr->ptr.w = malloc(i*sizeof(int16_t));
	      p.w = eptr->ptr.w + i; /* start at the end */
	      while (i--) {
		*--p.w = popList();
		embed(*p.w, n);
	      }
	      break;
	    case LUX_LIST:
	      eptr->ptr.sp = malloc(i*sizeof(char *));
	      p.sp = eptr->ptr.sp + i; /* start at the end */
	      while (i--) {
		j = popList();
		*--p.sp = strsave(string_value(j));
		if (isFreeTemp(j))
		  zap(j);
	      }
	      break;
	  }
	  popList();		/* remove the LUX_NEW_LIST */
	}
	break;
      case LUX_META:		/* a meta symbol, i.e. a string expression */
				/* which points at a symbol */
	meta_target(n) = va_arg(ap, int32_t);
	embed(meta_target(n), n);
	break;
      case LUX_RANGE:  /* a range */
	isScalarRange = 1;
	/* range start: */
	i = va_arg(ap, int32_t);
	range_start(n) = i;
	if (i < 0)
	  i = -i;
	embed(i, n);
	if (symbol_class(i) != LUX_SCALAR)
	  isScalarRange = 0;
	/* range end: */
	i = va_arg(ap, int32_t);
	range_end(n) = i;
	if (i < 0)
	  i = -i;
	embed(i, n);
	if (symbol_class(i) != LUX_SCALAR)
	  isScalarRange = 0;
	range_sum(n) = 0; /* default summation flag */
	range_redirect(n) = -1; /* redirection flag */
	range_scalar(n) = isScalarRange;
	break;
      case LUX_PRE_RANGE:
	isScalarRange = 1;
	/* pre_range start: */
	i = va_arg(ap, int32_t);
	pre_range_start(n) = i;
	if (i < 0)
	  i = -i;
	embed(i, n);
	if (symbol_class(i) != LUX_SCALAR)
	  isScalarRange = 0;
	/* pre_range end: */
	i = va_arg(ap, int32_t);
	pre_range_end(n) = i;
	if (i < 0)
	  i = -i;
	embed(i, n);
	if (symbol_class(i) != LUX_SCALAR)
	  isScalarRange = 0;
	pre_range_sum(n) = 0; /* default summation flag */
	pre_range_redirect(n) = -1; /* redirection flag */
	pre_range_scalar(n) = isScalarRange;
	break;
      case LUX_LIST_PTR:		/* pointer to a struct element */
	list_ptr_target(n) = va_arg(ap, int32_t); /* the struct */
	if ((i = va_arg(ap, int32_t)) >= 0) { /* non-numerical key */
	  list_ptr_tag_string(n) = symbolStack[i]; /* key */
	 list_ptr_tag_size(n) = strlen(symbolStack[i]) + 1;
	 unlinkString(i);	/* unlink keyword from stack */
       } else {			/* numerical key;  must be integer */
	 j = int_arg(-i);
	 if (symbol_context(-i) == curContext
	     && ((-i >= TEMP_EXE_START && -i < TEMP_EXE_END)
		 || (-i >= EXE_START && -i < EXE_END)))
	   /* a literal number */
	   zap(-i);
	 list_ptr_target(n) = -list_ptr_target(n);
				/* negative indicates numerical */
	 list_ptr_tag_number(n) = j;
       }
	break;
      case LUX_LIST: case LUX_PRE_LIST: /* includes LISTs */
	narg = stackListLength()/2; /* # of name-value pairs */
	isStruct = 0;
	arg = listStackItem - 2;
	for (i = 0; i < narg; i++) {
	  if (*arg >= 0) {
	    isStruct = 1;
	    break;
	  }
	  arg -= 2;
	}
	if (isStruct) {
	  listElem	*p;
	 
	  i = narg*(sizeof(listElem));
	  if (!(list_symbols(n) = (listElem *) malloc(i))) {
	    va_end(ap); 
	    return luxerror("Could not allocate memory for a struct", 0);
	  }
	  symbol_memory(n) = i;
	  p = list_symbols(n);
	  p += narg - 1;
	  while (narg--) {
	    p->value = popList();
	    embed(p->value, n);
	    i = popList();
	    p->key = (i >= 0)? symbolStack[i]: NULL;
	    if (i >= 0)
	      unlinkString(i);	/* unlink from symbolStack */
	    p--;
	  }
	} else {		/* must be a list */
	  symbol_class(n) = LUX_PRE_CLIST;
	  if (narg) {
	    if (!(arg = (int16_t *) malloc(narg*sizeof(int16_t)))) {
	      va_end(ap); 
	      return luxerror("Could not allocate memory for a list", 0);
	    }
	  } else
	    arg = NULL;
	  symbol_memory(n) = narg*sizeof(int16_t);
	  pre_clist_symbols(n) = arg;
	  arg += narg;
	  while (narg--) {
	    *--arg = popList();
	    embed(*arg, n);
	    popList();
	  }
	}
	popList();				/* pop LUX_NEW_LIST */
	break;
      case LUX_SUBSC_PTR:	/* subscript pointer */
	if (!(symbol_data(n) = (int32_t *) malloc(4*sizeof(int32_t)))) {
	  va_end(ap);
	  printf("newSymbol: ");
	  return cerror(ALLOC_ERR, 0);
	}
	symbol_memory(n) = 4*sizeof(int32_t);
	break;
      case LUX_POINTER:
	transfer_is_parameter(n) = 0;	/* not a formal argument in a */
					/* user-defined function or routine */
	narg = va_arg(ap, int32_t);
	i = lookForSubr(narg);
	if (i < 0)
	  i = lookForFunc(narg);
	if (i < 0)
	  i = lookForBlock(narg);
	if (i >= 0) {		/* found a routine */
	  symbol_class(n) = LUX_FUNC_PTR;
	  func_ptr_routine_num(n) = i;
	  unlinkString(narg);
	  return n;
	}
	i = findInternalSubr(narg);
	kind = LUX_SUBROUTINE;
	if (i < 0) {
	  i = findInternalFunc(narg);
	  kind = LUX_FUNCTION;
	}
	if (i >= 0) {		/* found an internal routine */
	  symbol_class(n) = LUX_FUNC_PTR;
	  func_ptr_routine_num(n) = -i;
	  func_ptr_type(n) = kind;
	  unlinkString(narg);
	  return n;
	}
	i = findVar(narg, curContext);
	if (i < 0)
	  return i;		/* some error */
	transfer_target(n) = i;
	break;
      case LUX_KEYWORD:
	keyword_name_symbol(n) = newSymbol(LUX_FIXED_STRING, va_arg(ap, int32_t));
	embed(keyword_name_symbol(n), n);
	keyword_value(n) = va_arg(ap, int32_t);
	embed(keyword_value(n), n);
	break;
      case LUX_SUBROUTINE: case LUX_FUNCTION: case LUX_BLOCKROUTINE:
       /* define these routines in two passes: first, define all
	  parameters, and then parse the statements when the
	  parameters are already known */
       /* if this routine is encountered during a @-compilation, then
	  delete any old definition and compile the new one.
	  if encountered during a @@-compilation, then don't do anything
	  if the routine is already compiled;  merely note the file if
	  the routine is not yet defined */
      { int16_t	nArg, nStatement;
	int32_t	oldContext;
	char	**key;
	 
	n = i = va_arg(ap, int32_t);
	if (n >= 0) {		/* first pass: n = index of routine's name */
				/* in symbolStack[] */
	  switch (kind)	{	/* first see if the routine is already */
				/* defined */
	    case LUX_SUBROUTINE:
	      n = lookForSubr(n);
	      break;
	    case LUX_FUNCTION:
	      n = lookForFunc(n);
	      break;
	    case LUX_BLOCKROUTINE:
	      n = lookForBlock(n);
	      break;
	  }
	  if (n == -1)		/* the routine is not yet defined */
	    switch (kind) {	/* so allocate a symbol for it */
	      case LUX_SUBROUTINE:
		n = findSubr(i);
		break;
	      case LUX_FUNCTION:
		n = findFunc(i);
		break;
	      case LUX_BLOCKROUTINE:
		n = findBlock(i);
		break;
	    }
	  else {		/* the routine is already known */
	    if (!reportBody)	/* need to compile body */
	      cleanUpRoutine(n, 1); /* get rid of old definition, */
				     /* but keep routine symbol */
	    freeString(i); 	/* deallocate symbolStack[i] */
	  }
	  if (n == -1) {	/* couldn't get new symbol */
	    va_end(ap);
	    reportBody = 0;
	    return LUX_ERROR; 	/* pass on the error */
	  }
	  symbol_class(n) = kind;

	  if (kind != LUX_BLOCKROUTINE) { /* has parameters */
	    nArg = stackListLength();	/* # parameters */
	    if (!reportBody) {	/* we're compiling the body */
	      if (listStackItem[-1] == LUX_EXTEND) { /* extended parameter */
		routine_has_extended_param(n) = 1;
		nArg--;
	      } else
		routine_has_extended_param(n) = 0;
	      if (nArg > UINT8_MAX) { /* too many parameters */
		va_end(ap);
		reportBody = 0;
		return luxerror("More than %1d parameters specified\n", n,
			     UINT8_MAX);
	      }
	      routine_num_parameters(n) = nArg;
	      if (nArg
		  && !(routine_parameters(n)
		       = (int16_t *) malloc(nArg*sizeof(int16_t)))) {
				/* could not allocate room for parameters */
		va_end(ap);
		reportBody = 0;
		return luxerror("Routine-definition memory-allocation error", 0);
	      }
	    } else		/* deferred compilation */
	      symbol_class(n) = (kind == LUX_SUBROUTINE)?
		LUX_DEFERRED_SUBR: LUX_DEFERRED_FUNC;
	  } else {		/* a block routine, which has no parametrs */
	    if (reportBody)
	      symbol_class(n) = LUX_DEFERRED_BLOCK;
	    routine_num_parameters(n) = nArg = 0;
	    routine_has_extended_param(n) = 0;
	  }
	  if (!reportBody) {	/* compiling the body */
	    if (nArg >= 1
		&& routine_has_extended_param(n)) { /* extended arg */
	      popList();
	    }
	    arg = routine_parameters(n) + nArg;
	    /* now save parameters (start at back) */
	    if (kind != LUX_BLOCKROUTINE) {
	      if (nArg &&
		  !eallocate(routine_parameter_names(n), nArg, char *)) {
		/* could not allocate memory to store the parameter names */
		va_end(ap); 
		return luxerror("Memory allocation error", 0);
	      }
	      key = routine_parameter_names(n) + nArg;
	      while (nArg--) {
		*--arg = findVar(popList(), n); /* parameter's symbol # */
		*--key = varName(*arg); /* parameter's name */
		symbol_class(*arg) = LUX_POINTER;
		transfer_target(*arg) = 0;
		transfer_temp_param(*arg) = 0;
		transfer_is_parameter(*arg) = 1;
	      }
	      popList(); 		/* remove list start marker */
	    }
	    routine_num_statements(n) = curContext; /* temporarily store */
						     /* current context */
	     /* variables in SUBRs and FUNCs are local to that particular */
	     /* SUBR or FUNC, whereas variables in BLOCKs are local to the */
	     /* embedding SUBR or FUNC or main level. */
	    if (kind != LUX_BLOCKROUTINE)
	      curContext = n;	/* make current function or subroutine
				   the current context */
	  } else {		/* not compiling the body */
	    if (kind != LUX_BLOCKROUTINE) {
	      while (nArg--) {	/* discard the parameters */
		i = popList();
		freeString(i);
	      }
	      popList();
	    }
	    /* save the name of the file in which the definition of */
	    /* this routine can be found if necessary */
	    deferred_routine_filename(n) = strsave(currentInputFile);
	  }
	  keepEVB++;		/* executables must be preserved */
	  if (reportBody)
	    ignoreSymbols = 1;	/* ignore symbols during compilation of */
				/* the routine */
	  return n;
	} else {		/* second pass */
	  n = i = -n - 1;	/* routine # */
	  oldContext = routine_num_statements(n); /* save for later */
	  nStatement = stackListLength();
	  if (nStatement < 0)
	    nStatement = 0;
	  routine_num_statements(n) = nStatement;
	  if (!nStatement
	      && (!compileOnly || symbol_class(n) != LUX_BLOCKROUTINE)) {
	    /* can't have empty routines or functions
	       except for an empty block routine in VERIFY if the verified
	       file contains only a definition for a subroutine or
	       function. */
	    va_end(ap);
	    curContext = oldContext;
	    ignoreSymbols = 0;
	    return luxerror("No statements in user routine or function", 0);
	  }
	  if (kind == LUX_BLOCKROUTINE) { /* allocate space for the */
	    /* statements (since there are no parameters, this is at */
	    /* the beginning of the combined parameters+statements list) */
	    if (nStatement &&
		!(routine_parameters(n) =
		  (int16_t *) malloc(nStatement*sizeof(int16_t)))) {
	      va_end(ap); 
	      curContext = oldContext;	/* restore context */
	      ignoreSymbols = 0;
	      return
		luxerror("Allocation error in 2nd pass of routine definition",
		      0);
	    }
	    nArg = 0;		/* no parameters to a block routine */
	  } else {			/* subroutine or function */
	    nArg = routine_num_parameters(n);
	    if (nArg)		/* reallocate memory for combined */
				/* parameters+statements list */
	      routine_parameters(n) =
		(int16_t *) realloc(routine_parameters(n),
				 (nArg + nStatement)*sizeof(int16_t));
	    else		/* no parameters, just allocate space for */
				/* statements */
	      routine_parameters(n) =
		(int16_t *) malloc(nStatement*sizeof(int16_t));
	    if (!routine_parameters(n)) { /* allocation failed */
	      va_end(ap);
	      curContext = oldContext;	/* restore context */
	      ignoreSymbols = 0;
	      return
		luxerror("Allocation error in 2nd pass of routine definition",
		      0);
	    }
	  }
	  arg = routine_parameters(n) + nStatement + nArg; /* end of list */
	  if (nStatement) {
	    while (nStatement--) { /* store statements, starting at end */
	      *--arg = popList();
	      embed(*arg, n);	/* give the statement and all enclosed */
	    }
	    /* symbols the context of the routine: this makes the */
	    /* statement local to the routine */
	    popList();		/* remove beginning-of-list marker */
	  }
	  curContext = oldContext; /* restore context now definition is */
				    /* done */
	  ignoreSymbols = 0;	/* no longer ignore symbols - if we were */
				/* doing that */
	  keepEVB--;		/* executables are again more temporary */
	  symbol_context(n) = 0; /* routines always have context 0 */
	  usr_routine_recursion(n) = 0; /* no recursion yet */
	  return 0;
	}
      }
      case LUX_EVB: case LUX_INT_FUNC: case LUX_USR_FUNC:
	if (kind == LUX_EVB) {
	  kind = va_arg(ap, int32_t);
	  symbol_type(n) = kind;
	}
	i = 0;			/* default: no more items to retrieve */
	switch (kind) {
	  case EVB_RETURN:
	    i = 1; 
	    break;
	  case EVB_REPLACE: case EVB_REPEAT: case EVB_WHILE_DO:
	  case EVB_DO_WHILE:
	    i = 2;
	    break;
	  case EVB_IF:
	    i = 3;
	    break;
	  case EVB_FOR:
	    i = 4; 
	    break;
	  case EVB_USR_CODE:
	    usr_code_routine_num(n) = va_arg(ap, int32_t); /* routine number */
	    break;
	  case EVB_FILE:
	    i = va_arg(ap, int32_t);	/* index to string */
	    file_name(n) = symbolStack[i];
	    symbol_memory(n) = strlen(symbolStack[i]) + 1;
	    unlinkString(i);
	    i = va_arg(ap, int32_t);	/* include type: INCLUDE -> always,
					   REPORT -> only if necessary */
	    file_include_type(n) = i;
	    i = 0;		/* no more items to retrieve */
	    break;
	  case EVB_INT_SUB: case EVB_USR_SUB: case EVB_INSERT:
	  case LUX_INT_FUNC: case LUX_USR_FUNC:
	    sym[n].xx = va_arg(ap, int32_t); /* routine number (SUB) or target */
	  case EVB_CASE: case EVB_NCASE: case EVB_BLOCK: 
	    i = stackListLength();		/* # of expr and statements */
	    if (i) {			/* only if there are any elements */
	      if (!(arg = (int16_t *) malloc(i*sizeof(int16_t)))) {
		va_end(ap);
		return luxerror("Could not allocate memory for stacked elements",
			     0);
	      }
	      symbol_data(n) = arg; /* the elements */
	      symbol_memory(n) = i*sizeof(int16_t);	/* the memory size */
	      arg += i;	/* start with the last element (which is */
	      /* on top of the stack) */
	      while (i--) {	/* all elements */
		*--arg = popList(); /* pop element from stack */
		embed(*arg, n);  /* context is enclosing statement */
	      }
	    } else
	      symbol_memory(n) = 0; /* no args */
	    popList();		/* pop LUX_NEW_LIST marker */
	    i = 0;		/* no more items to retrieve */
	    break;
	}
	if (i > 0) {
	  arg = sym[n].spec.evb.args;
	  while (i--) {
	    *arg = va_arg(ap, int32_t);
	    embed(*arg, n);
	    arg++;
	  }
	  if (kind == EVB_FOR) {
	    for_body(n) = va_arg(ap, int32_t);
	    embed(for_body(n), n);
	  }
	}
	break;
      case LUX_BIN_OP: case LUX_IF_OP:
	bin_op_type(n) = va_arg(ap, int32_t);
	bin_op_lhs(n) = va_arg(ap, int32_t);
	embed(bin_op_lhs(n), n);
	bin_op_rhs(n) = va_arg(ap, int32_t);
	embed(bin_op_rhs(n), n);
	break;
    }
  } else {			/* reportBody & in definition */
    switch (kind) {
      case LUX_FIXED_STRING:
	i = va_arg(ap, int32_t);	/* index to symbolStack */
	freeString(i);
	break;
      case LUX_LIST_PTR:
	i = va_arg(ap, int32_t);	/* struct number */
	i = va_arg(ap, int32_t);	/* key */
	if (i >= 0)		/* non-numerical key */
	  freeString(i); 
	break;
      case LUX_LIST:  case LUX_PRE_LIST:
	narg = stackListLength()/2;
	while (narg--) {
	  popList();		/* value */
	  i = popList();	/* key */
	  if (i >= 0)		/* string key */
	    freeString(i);
	}
	break;
      case LUX_KEYWORD:
	i = va_arg(ap, int32_t);
	freeString(i);
	break;
      case LUX_SUBROUTINE: case LUX_FUNCTION: case LUX_BLOCKROUTINE:
	/* when we get here, a symbol has already been reserved for the */
	/* routine, and the parameters have been ignored.  we only need to */
	/* get rid of the routine body. */
	n = va_arg(ap, int32_t);
	n = -n - 1;		/* routine # */
	routine_num_statements(n) = 0; /* no statements */
	i = stackListLength();
	while (i--)
	  popList();
	popList();
	/*       inDefinition = 0; */
	keepEVB--;
	ignoreSymbols = 0;	/* no longer ignore symbols, if we were */
				/* doing that. */
	return 0;
      case LUX_EXTRACT:
	target = popList();
	if (target > 0) {	/* regular symbol */
	  depth = popList();
	} else {
	  /* i = -target;
	     freeString(i); */
	  depth = popList();
	}
	ptr = listStackItem;
	if (!depth)
	  popList();
	while (depth--) {
	  ptr--;
	  popList();
	  i = stackListLength(); /* the number of entries in this
					      one */
	  while (i--)
	    popList();
	  popList();		/* remove the LUX_NEW_LIST */
	}
	break;
      case LUX_EVB: case LUX_INT_FUNC: case LUX_USR_FUNC:
	if (kind == LUX_EVB)
	  kind = va_arg(ap, int32_t);
	switch (kind) {
	  case EVB_FILE:
	    i = va_arg(ap, int32_t);
	    freeString(i);
	    break;
	  case EVB_INT_SUB: case EVB_USR_SUB: case EVB_INSERT:
	  case LUX_INT_FUNC: case LUX_USR_FUNC:
	  case EVB_NCASE: case EVB_CASE: case EVB_BLOCK:
	    i = stackListLength();
	    while (i--)
	      popList();
	    popList();
	    break;
	}
	break;
    }
    n = 0;
  }
  va_end(ap);
#if YYDEBUG
  if (yydebug)
    printf("--- new symbol %1d, class %1d (%s): %s\n", n, symbol_class(n),
	   className(symbol_class(n)), symbolIdent(n, 0));
#endif
  return n;
}
/*----------------------------------------------------------------*/
int32_t hash(char *string)
{
 int32_t	i;

 for (i = 0; *string; ) i += *string++;
 return i % HASHSIZE;
}
/*----------------------------------------------------------------*/
int32_t ircmp(const void *a, const void *b)
{
  internalRoutine *ra, *rb;
  
  ra = (internalRoutine *) a;
  rb = (internalRoutine *) b;
  return strcmp(ra->name, rb->name);
}
/*----------------------------------------------------------------*/
int32_t findInternalName(char *name, int32_t isSubroutine)
/* searches for name in the appropriate subroutine
  or function table.  if found, returns
  index, else returns -1 */
{
  internalRoutine	*table, *found, key;
  size_t n;

  if (isSubroutine) {
    table = subroutine;
    n = nSubroutine;
  } else {
    table = function;
    n = nFunction;
  }
  key.name = name;
  found = bsearch(&key, table, n, sizeof(*table), ircmp);
  return found? found - table: -1;
}
/*----------------------------------------------------------------*/
static compileInfo	*c_info = NULL;
int32_t	cur_c_info = 0, n_c_info = 0;
compileInfo	*curCompileInfo;

int32_t nextCompileLevel(FILE *fp, char *fileName)
/* saves the rest of the current input line and starts reading
 input from file fp.  When the file is processed, compilation
 at the current level is resumed. */
{
 int32_t	n, oldZapContext;
 char	*name;
 extern int32_t	echo; 
 extern char	inHistoryBuffer, tLine[], *inputString;
 extern uint8_t	disableNewline;
 int32_t	yyparse(void), getStreamChar(void), getStringChar(void);
 extern int32_t	(*getChar)(void);
 compileInfo	*nextFreeCompileInfo(void);
 void	pegParse(void), removeParseMarker(void), releaseCompileInfo(void);

 curCompileInfo = nextFreeCompileInfo();
 curCompileInfo->line = strsave(currentChar); /* save rest of line */
 curCompileInfo->charfunc = getChar;
 curCompileInfo->name = currentInputFile? strsave(currentInputFile): NULL;

 if (fp) {
   name = fileName;		/* was strsave(fileName) */
   getChar = getStreamChar;
 } else {
   name = "(string)";
   getChar = getStringChar;
 }
 currentChar = line;
 compileLevel++;
 oldZapContext = zapContext;
 zapContext = -compileLevel;
 *currentChar = '\0';		/* abort compilation of this line */
 curCompileInfo->stream = inputStream; /* save stream */
 inputStream = fp;		/* new input stream */
 inHistoryBuffer = 0;		/* don't save new stuff in history buffer */
 currentInputFile = fileName;
 curCompileInfo->line_number = curLineNumber;
 curLineNumber = 0;
 pegParse();
 if (echo || (traceMode & T_ROUTINEIO)) {
   if (fp)
     printf("Compiling file %s\n", name);
   else
     printf("Compiling string \"%s\"\n", inputString);
 }
 disableNewline++;		/* ignore newlines during parsing */
 n = yyparse();			/* parse ; return 0 -> OK, 1 -> error */
	/* now restore state before this interruption */
 disableNewline--;
 if (echo || traceMode & T_ROUTINEIO) {
   if (fp)
     printf("Done compiling file %s\n", name);     
   else
     printf("Done compiling string\n", inputString);
   printf("back in %s\n", (cur_c_info > 1)? curCompileInfo[-1].name: "main");
 }
 curLineNumber = curCompileInfo->line_number;
 cleanUp(-compileLevel, CLEANUP_BOTH);
 removeParseMarker();
 compileLevel--;
 zapContext = oldZapContext;
 inputStream = curCompileInfo->stream;

 currentInputFile = curCompileInfo->name;
 getChar = curCompileInfo->charfunc;
 strcpy(tLine, curCompileInfo->line);
 free(curCompileInfo->line);
 releaseCompileInfo();
 currentChar = tLine;
 *line = '\n';			/* must not be \0 or user input is asked for */
 if (!compileLevel)
   inHistoryBuffer = 1;
 n = n? -1: 1;
 if (n < 0) {
   if (fp)
     luxerror("*** error in file %s", 0, name);
   else
     luxerror("*** error in execution string \"%s\"", 0, inputString);
 }
 /* if (fp) 
   free(name); */
 return n;
} 
/*----------------------------------------------------------------*/
static int32_t	compileCount = 0;
int32_t compile(char *string)
/* compiles string <string> and store in BLOCKROUTINE.  Returns */
/* but does not execute the block routine.  LS 5feb96 */
{
  int32_t	oldContext, n, getStringChar(void), getStreamChar(void), nsym, result;
  extern char	*inputString, compileOnly;
  extern int32_t	(*getChar)(void), executeLevel;
  char	compileName[12], oldInstalling;
  int32_t	newBlockSymbol(int32_t);

  inputString = string;
  compileOnly++;
  executeLevel++;
  getChar = getStringChar;
  oldContext = curContext;
  sprintf(compileName, "#COMPILE%1d", compileCount);
  n = installString(compileName);
  oldInstalling = installing;
  installing = 1;
  nsym = newSymbol(LUX_BLOCKROUTINE, n);
  installing = oldInstalling;
  curContext = nsym;
  n = nextCompileLevel(NULL, NULL);
  if (n < 0
      || newSymbol(LUX_BLOCKROUTINE, -nsym - 1) < 0) { /* some error */
    zap(nsym);
    return n;
  } else {
    n = installString(compileName);
    result = newBlockSymbol(n);
    symbol_context(result) = nsym;
  }
  curContext = oldContext;
  compileCount++;
  compileOnly--;
  executeLevel--;
  getChar = getStreamChar;
  return result;
}
/*----------------------------------------------------------------*/
#if DEVELOP
int32_t lux_compile(int32_t narg, int32_t ps[])
{
  char	*string;
  int32_t	result, value;

  string = string_arg(*ps);
  result = scalar_scratch(LUX_LONG);
  if (string)
  { value = compile(string);
    scalar_value(result).l = value; }
  else
    scalar_value(result).l = -1;
  return result;
}
#endif
/*----------------------------------------------------------------*/
int32_t newBlockSymbol(int32_t index)
/* searches for user block symbolStack[index] in list of user-defined
  block routines.  treats function pointers */
{
  int32_t	n, result;
  extern char	reportBody;
  
  if (reportBody) {		/* remove name from stack */
    freeString(index);
    return 0;
  }
  if ((n = lookForVar(index, curContext)) >= 0) { /* blockroutine pointer? */
    if (sym[n].class == LUX_FUNC_PTR) {
      if (func_ptr_routine_num(n) > 0) { /* user-defined */
	if (sym[n = func_ptr_routine_num(n)].class == LUX_BLOCKROUTINE) {
	  freeString(index);
	  return newSymbol(LUX_EVB, EVB_USR_CODE, n);
	}
      } else
	return luxerror("Func/subr pointer does not point at executable block routine!", n);
    }
  }
  n = lookForBlock(index);
  if (n < 0) {			/* block not yet defined */
    n = nextFreeTempVariable();
    if (n < 0)
      return LUX_ERROR;
    symbol_class(n) = LUX_STRING;
    string_value(n) = symbolStack[index];
    unlinkString(index);
    symbol_memory(n) = strlen(string_value(n)) + 1;
    result = newSymbol(LUX_EVB, EVB_USR_CODE, n);
    symbol_context(usr_code_routine_num(result)) = result;
  } else {
    n = findBlock(index);
    result = newSymbol(LUX_EVB, EVB_USR_CODE, n);
  }
  return result;
}
/*----------------------------------------------------------------*/
int32_t newSubrSymbol(int32_t index)
/* searches for subroutine symbolStack[index] in lists of internal
  and user-defined subroutines.  if not found, then searches for an
  appropriate file to find a definition.  if such a file is found,
  then installs name as new subroutine in user-defined subroutine list,
  and a new symbol with appropriate class (LUX_EVB) and type (EVB_INT_SUB 
  or EVB_USR_SUB) is returned.  if such a file is not found, then
  an error is generated. */
{
 int32_t	n, i;
 extern char	reportBody;
 extern int32_t	findBody;

 /* In order, look for: */
 /* 1. function pointer to some user-defined or internal subroutine */
 /* 2. user-defined subroutine */
 /* 3. internal subroutine */
 /* 4. user-defined function */

 if (ignoreInput && findBody > 0) { /* not compiling this */
   freeString(index);		/* remove name from stack */
   /* take care of deleting arguments: */
   return newSymbol(LUX_EVB, EVB_INT_SUB, 0);
 }
 if (findBody < 0)		/* we're at the end of the definition
				   of a deferred routine */
   findBody = -findBody;
 n = lookForVar(index, curContext); /* look for variable */
 if (n >= 0 && symbol_class(n) == LUX_FUNC_PTR) { /* maybe subr pointer */
   freeString(index);		/* remove name from stacke */
   if (func_ptr_routine_num(n) < 0) {	/* internal routine/function */
     if (func_ptr_type(n) == LUX_SUBROUTINE)
       return newSymbol(LUX_EVB, EVB_INT_SUB, -sym[n].spec.evb.args[0]);
   } else {
     n = func_ptr_routine_num(n); /* user-defined routine */
     return newSymbol(LUX_EVB, EVB_USR_SUB, n);
   }
 }
 /* no subroutine pointer */
 n = lookForSubr(index);	/* already defined user-defined routine? */
 if (n < 0) {			/* none found */
   if ((n = findInternalSym(index, 1)) >= 0) { /* internal routine */
     freeString(index);
     return newSymbol(LUX_EVB, EVB_INT_SUB, n);
   } else {			/* no internal: assume user-defined */
     n = newSymbol(LUX_FIXED_STRING, index);
     i = newSymbol(LUX_EVB, EVB_USR_SUB, n);
     symbol_context(n) = i;
     return i;
   }
 }
				/* user-defined routine */
 freeString(index);
 return newSymbol(LUX_EVB, EVB_USR_SUB, n);
}
/*----------------------------------------------------------------*/
int32_t lookForName(char *name, hashTableEntry *hashTable[], int32_t context)
     /* searches name in hashTable[] for context.  if found,
	returns symbol number, otherwise returns -1 */
{
  int32_t		hashValue, n;
  hashTableEntry	*hp;
  
  hashValue = hash(name);
  if (*name == '$' || *name == '#' || *name == '!') context = 0;
  hp = hashTable[hashValue];
  while (hp)
  { if (!strcmp(hp->name, name) && sym[hp->symNum].context == context)
      return hp->symNum;
    hp = hp->next; }
  /* to distinguish between functions and subroutines with the same */
  /* name (e.g. when creating a pointer to a function or subroutine), */
  /* function names may be specified with an _F extension.  check if */
  /* we're dealing with such a case here */
  if (hashTable == funcHashTable)
  { n = strlen(name);
    if (n > 2 && name[n - 2] == '_' && name[n - 1] == 'F')
    { name[n - 2] = '\0';
      return lookForName(name, hashTable, context); }
  }
  return -1;
}
/*----------------------------------------------------------------*/
int32_t findSym(int32_t index, hashTableEntry *hashTable[], int32_t context)
/* searches symbolStack[index] in hashTable[] for context.  if found,
   returns symbol number, otherwise installs the name in hashTable[]
   and sym[].  always removes the entry from the symbolStack. */
{
 char	*name;
 int32_t	n;
 extern char	ignoreSymbols;

 if (ignoreSymbols) {
   freeString(index);
   return 0;
 }
 name = symbolStack[index];
 n = findName(name, hashTable, context);
 freeString(index);
 return n;
}
/*----------------------------------------------------------------*/
char *symName(int32_t symNum, hashTableEntry *hashTable[])
/* returns the name of the symbol, if any, or "[symNum]" */
{
 static char	name[7];
 int32_t		hashValue;
 hashTableEntry	*hp;

 if (symNum < 0 || symNum >= NSYM)
 { printf("Illegal symbol number (symName): %d\n", symNum);
   return "[error]"; }
 hashValue = sym[symNum].xx - 1;
 if (hashValue < 0 || hashValue > HASHSIZE)
 { return "[unnamed]"; }
 hp = hashTable[hashValue];
 while (hp)
 { if (hp->symNum == symNum)
     return hp->name;
   hp = hp->next; }
 sprintf(name, "[%1d]", symNum);
 return name;
}
/*----------------------------------------------------------------*/
char *symbolName(int32_t symbol)
/* returns the name of the symbol. */
{
  hashTableEntry	**hashTable;

  if (symbol < 0 || symbol >= NSYM) {
    cerror(ILL_SYM, 0, symbol, "symbolName");
    return "(error)";
  }
  switch (symbol_class(symbol)) {
    case LUX_SUBROUTINE: case LUX_DEFERRED_SUBR:
      hashTable = subrHashTable;
      break;
    case LUX_FUNCTION: case LUX_DEFERRED_FUNC:
      hashTable = funcHashTable;
      break;
    case LUX_BLOCKROUTINE: case LUX_DEFERRED_BLOCK:
      hashTable = blockHashTable;
      break;
    default:
      hashTable = varHashTable;
      break;
  }
  return symName(symbol, hashTable);
}
/*----------------------------------------------------------------*/
int32_t suppressEvalRoutine(int32_t index)
/* returns evaluation suppression associated with internal routine */
/* symbolStack[index] */
{
  int32_t	n;
  keyList	*keys;

  n = findInternalSym(index, 1); /* >= 0 -> internal subroutine */
  if (n < 0) return 0;
  keys = (keyList *) subroutine[n].keys;
  return keys->suppressEval;
}
/*----------------------------------------------------------------*/
#define	IGNORE_SIG	1
#define ASK_SIG		2
#define SIG_BREAK	3
void exception(int32_t sig)
/* exception handler */
{
 int32_t	c, saveHistory(void);
 extern int32_t	curSymbol, executeLevel, step, statementDepth;
 extern jmp_buf	jmpenv;
 void	cleanUp(int32_t, int32_t), Quit(int32_t);

 if (sig != SIGCONT && curSymbol)
   puts(symbolIdent(curSymbol, 1));

 /* we flush the input buffer; otherwise the user cannot indicate what
  is to be done about the interrupt. */
 fflush(stdin);

 switch (sig) {
   case SIGFPE:
     puts("*** Floating-point exception - aborting calculation");
     checkErrno();
     c = SIG_BREAK;
     break;
   case SIGINT:
     puts("Interrupt - quit current calculation? (type ? for help)");
     do {
       c = getSingleStdinChar();
       switch (c) {
	 case 'y': case 'Y':
	   c = SIG_BREAK;
	   break;
	 case 't': case 'T':
	   trace = 999;
	   traceMode = 255;
	   puts("Tracing everything at level 999");
	   c = IGNORE_SIG;
	   break;
	 case 's': case 'S':
	   step = executeLevel;
	   printf("Commence stepping at level %1d\n", step);
	   c = IGNORE_SIG;
	   break;
	 case 'q': case 'Q':
	   trace = step = 0;
	   puts("Continue quietly, without stepping or tracing");
	   c = IGNORE_SIG;
	   break;
	 case 'a': case 'A':
	   Quit(1);		/* exit LUX completely */
	 case 'r': case 'R':
	   saveHistory();
	   lux_restart(0, NULL);
	 case '?':
	   printw("Options:  y - yes, quit;  t - start tracing;  ");
	   printw("s - start stepping;  q - run quietly (no tracing or ");
	   printw("stepping);  a - abort LUX;  r - restart LUX; ");
	   printw("? - show options.");
	   break; }
     } while (c == '?');
     break;
   case SIGTRAP:
     puts("*** integer division by zero - quit this calculation");
     c = SIG_BREAK;
     break;
   case SIGSEGV:
     printf("Segmentation fault: You wrote in an off-limits memory location\n"
	    "- quit?\n");
     c = getSingleStdinChar();
     if (c == 'y' || c == 'Y')
       Quit(2);
     c = SIG_BREAK;
     break;
   case SIGCONT:
     puts("Continuing LUX...");
     c = SIG_BREAK;
     break;
   default:
     printf("Exception %d - Quitting.\n", sig);
     Quit(3);
 }
 checkErrno();
 if (signal(sig, exception) == SIG_ERR)
   luxerror("Could not reinstall exception handler", 0);
 if (c == SIG_BREAK) {
   curContext = executeLevel = statementDepth = 0;
   cleanUp(-compileLevel, CLEANUP_ALL);
   longjmp(jmpenv, 0);
 }
 return;
}
/*----------------------------------------------------------------*/
char *typeName(int32_t type)
/* returns the name that goes with the data type */
{
  static char *typeNames[] = {
    "BYTE", "WORD", "LONG", "QUAD", "FLOAT", "DOUBLE",
    "STRING", "STRING", "STRING", "CFLOAT", "CDOUBLE",
    "undefined", "unknown"
  };
  int32_t	index;

  if (type == LUX_UNDEFINED)
    index = 10;			/* undefined */
  else if (type < 0 || type > LUX_CDOUBLE)
    index = sizeof(typeNames)/sizeof(*typeNames) - 1; /* unknown */
  else
    index = type;		/* OK */
  return typeNames[index];
}
/*----------------------------------------------------------------*/
char *className(int32_t class)
/* returns the name of the class */
{
  static struct classInfo {
    uint8_t number; char *name;
  } classes[] = {
    { LUX_UNUSED, "not used" },
    { LUX_SCALAR, "scalar" },
    { LUX_STRING, "string" },
    { LUX_RANGE, "range" },
    { LUX_ARRAY, "array" },
    { LUX_POINTER, "pointer" },
    { LUX_ASSOC, "associated variable" },
    { LUX_FUNC_PTR, "function pointer" },
    { LUX_SCAL_PTR, "scalar pointer" },
    { LUX_SUBSC_PTR, "subscript pointer" },
    { LUX_FILEMAP, "file array" },
    { LUX_CLIST, "compact list" },
    { LUX_LIST, "list" },
    { LUX_STRUCT, "structure" },
    { LUX_KEYWORD, "keyword" },
    { LUX_LIST_PTR, "list pointer" },
    { LUX_PRE_RANGE, "pre-range" },
    { LUX_PRE_CLIST, "pre-compact-list" },
    { LUX_PRE_LIST, "pre-list" },
    { LUX_ENUM, "enumeration constant" },
    { LUX_META, "SYMBOL call" },
    { LUX_CSCALAR, "complex scalar" },
    { LUX_CARRAY, "complex array" },
    { LUX_CPLIST, "compact pointer list" },
    { LUX_TRANSFER, "transfer symbol" },
    { LUX_STRUCT_PTR, "struct pointer" },
    { LUX_SUBROUTINE, "subroutine" },
    { LUX_FUNCTION, "function" },
    { LUX_BLOCKROUTINE, "block routine" },
    { LUX_DEFERRED_SUBR, "deferred subroutine" },
    { LUX_DEFERRED_FUNC, "deferred function" }, 
    { LUX_DEFERRED_BLOCK, "deferred block routine" },
    { LUX_BIN_OP, "binary operation" },
    { LUX_INT_FUNC, "internal function call" },
    { LUX_USR_FUNC, "user function call" },
    { LUX_IF_OP, "if-operation" },
    { LUX_EXTRACT, "extraction" },
    { LUX_PRE_EXTRACT, "pre-extraction" },
    { LUX_EVB, "executable" },
    { LUX_FIXED_NUMBER, "fixed number" },
    { LUX_FIXED_STRING, "fixed string" },
    { LUX_UNDEFINED, "undefined" },
    { 0, "unknown" }
  };

 static char	classHashTable[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
  20, 21, 22, 23, 24, 25, 42, 41, 42, 42, 42, 42, 26, 27, 28, 29, 30, 31,
  42, 42, 32, 33, 34, 35, 36, 37, 42, 42, 38, 39, 40
 };

 int32_t	hash;

 if (class < 0)
   hash = 26;
 else {
   hash = class % 76;
   if (hash > 51)
     hash = 25;
   hash = classHashTable[hash];
   if (class != (int32_t) classes[hash].number)
     hash = 42;
 }
 return classes[hash].name;
}
/*----------------------------------------------------------------*/
int32_t lux_classname(int32_t narg, int32_t ps[])
     /* returns name associated with class number */
{
  int32_t	class, result;
  char	*name;

  class = int_arg(*ps);
  getFreeTempVariable(result);
  sym[result].class = LUX_STRING;
  string_type(result) = LUX_TEMP_STRING;
  name = string_value(result) = strsave(className(class));
  symbol_memory(result) = strlen(name) + 1;
  return result;
}
/*----------------------------------------------------------------*/
int32_t lux_typeName(int32_t narg, int32_t ps[])
     /* returns name associated with type number */
{
  int32_t	type, result;
  char	*name;

  if ((type = int_arg(*ps)) < 0) return -1;
  getFreeTempVariable(result);
  sym[result].class = LUX_STRING;
  string_type(result) = LUX_TEMP_STRING;
  name = string_value(result) = strsave(typeName(type));
  symbol_memory(result) = strlen(name) + 1;
  return result;
}
/*----------------------------------------------------------------*/
char *evbName(int32_t evbType)
     /* returns the name of an EVB type */
{
  static char *evbTypeNames[] = {
    "statement group", "replacement", "internal suboutine call",
    "for statement", "insertion", "if statement",
    "user-defined subroutine call", "while-do statement",
    "do-while statement", "return statement", "case statement",
    "ncase statement", "repeat statement", "block routine call",
    "file inclusion", "unknown"
  };

  if (evbType < EVB_BLOCK || evbType > EVB_FILE)
    evbType = EVB_FILE + 1;
  
  return evbTypeNames[evbType - 1];
}
/*----------------------------------------------------------------*/
char *filetypeName(int32_t filetype)
/* returns the name associated with a file type */
{
  static char *filetypeNames[] = {
    "Unknown", "LUX fz", "IDL Save", "GIF", "LUX Astore", "JPEG", "TIFF",
    "FITS", "PPM (raw)", "PPM (ascii)", "XPM", "X11 bitmap", "BMP",
    "Sun raster", "Iris RGB", "Targa (24 bit)", "PM"
  };

  if (filetype < 0 || filetype >= sizeof(filetypeNames)/sizeof(char *))
    filetype = 0;
  return filetypeNames[filetype];
}
/*----------------------------------------------------------------*/
int32_t lux_filetype_name(int32_t narg, int32_t ps[])
{
  char	*name;
  int32_t	result;

  name = filetypeName(int_arg(ps[0]));
  result = string_scratch(strlen(name));
  strcpy(string_value(result), name);
  return result;
}
/*----------------------------------------------------------------*/
void fixedValue(char *name, Symboltype type, ...)
/* install a numerical constant */
{
 int32_t	n, iq;
 pointer	p;
 va_list	ap;

 va_start(ap, type);
 iq = installString(name);
 n = findVar(iq, 0);
 switch (type) {
 case LUX_LSTRING:
   symbol_class(n) = LUX_STRING;
   string_type(n) = LUX_LSTRING;
   string_value(n) = va_arg(ap, char *);
   symbol_memory(n) = strlen(string_value(n)) + 1;
   break;
 case LUX_CFLOAT: case LUX_CDOUBLE:
   complex_scalar_data(n).cf = malloc(lux_type_size[type]);
   if (!complex_scalar_data(n).cf)
     puts("WARNING - memory allocation error in symbol initialization");
   complex_scalar_memory(n) = lux_type_size[type];
   symbol_class(n) = LUX_CSCALAR;
   complex_scalar_type(n) = type;
   p.cf = complex_scalar_data(n).cf;
   if (type == LUX_CFLOAT) {
     p.cf->real = (float) va_arg(ap, double);
     p.cf->imaginary = (float) va_arg(ap, double);
     p.cf++;
   } else {
     p.cd->real = va_arg(ap, double);
     p.cd->imaginary = va_arg(ap, double);
     p.cd++;
   }
   break;
 default:
   symbol_class(n) = LUX_SCALAR;
   scalar_type(n) = type;
   switch (type) {
   case LUX_LONG:
     scalar_value(n).l = va_arg(ap, int32_t);
     break;
   case LUX_QUAD:
     scalar_value(n).q = va_arg(ap, int64_t);
     break;
   case LUX_FLOAT:
     scalar_value(n).f = (float) va_arg(ap, double);
     break;
   case LUX_DOUBLE:
     scalar_value(n).d = va_arg(ap, double);
     break;
   default:
     cerror(ILL_TYPE, n);
   }
   break;
 }
}
/*----------------------------------------------------------------*/
int32_t installSysFunc(char *name, int32_t number)
/* install a system function.  These are implemented as FUNC_PTRs to */
/* the appropriate function */
{
 int32_t	n, iq;

 iq = installString(name);
 n = findVar(iq,0);
 sym[n].class = LUX_FUNC_PTR;
 func_ptr_routine_num(n) = -number;
 func_ptr_type(n) = LUX_FUNCTION;
 return n;
}
/*----------------------------------------------------------------*/
int32_t installPointer(char *name, int32_t type, void *ptr)
/* install a LUX_SCAL_PTR system variable */
{ 
 int32_t	n, iq;
 
 iq = installString(name);
 n = findVar(iq, 0);
 sym[n].class = LUX_SCAL_PTR;
 scal_ptr_type(n) = type;
 scal_ptr_pointer(n).l = (int32_t *) ptr;
 if (type == LUX_TEMP_STRING)
   symbol_memory(n) = strlen(ptr) + 1;
 return n;
}
/*----------------------------------------------------------------*/
int32_t convertRange(int32_t range)
/* convert a LUX_RANGE symbol to a LUX_SUBSC_PTR symbol. */
/* elements:  #1:  range start */
/*            #2:  range end */
/*            #3:  summation flag (0 or 1) */
/*            #4:  redirection flag (symbol, MINUSONE if none) */
/*            for #1 and #2, if the symbol is > 0, then count from the */
/*            start of the list/array; otherwise from one beyond the last */
/*            element of the list/array.  if #2 == LUX_ZERO, then only one */
/*            element is requested. */
{
  int32_t	subsc, eval(int32_t), j1, j2;
  
  if ((subsc = newSymbol(LUX_SUBSC_PTR)) < 0) return -1;
  j1 = range_start(range);
  if (j1 == -LUX_ONE)		/* (*) */
  { subsc_ptr_start(subsc) = 0;
    subsc_ptr_end(subsc) = -1; }
  else 
  { if (j1 >= 0)		/* (X:...) */
    { j2 = int_arg(eval(j1));
      if (j2 < 0)
	return luxerror("Illegal range start", range_start(range)); }
    else			/* (*-X:...) */
    { j2 = -int_arg(eval(-j1));
      if (-j2 <= 0)
	return luxerror("Illegal range start", range_start(range)); }
    subsc_ptr_start(subsc) = j2;

    j1 = range_end(range);
    if (j1 == LUX_ZERO)		/* (X) */
      subsc_ptr_end(subsc) = subsc_ptr_start(subsc);
    else if (j1 == -LUX_ONE)	/* (...:*) */
      subsc_ptr_end(subsc) = -1;
    else
    { if (j1 >= 0)		/* (...:Y) */
      { j2 = int_arg(eval(j1));
	if (j2 < 0)
	  return luxerror("Illegal range end", range_end(range)); }
      else			/* (...:*-Y) */
      { j2 = -int_arg(eval(-j1));
	if (-j2 <= 0)
	  return luxerror("Illegal range end", range_end(range)); }
      subsc_ptr_end(subsc) = j2; }
  }

  if (subsc_ptr_start(subsc) > subsc_ptr_end(subsc) &&
      ((subsc_ptr_start(subsc) >= 0 && subsc_ptr_end(subsc) >= 0) ||
       (subsc_ptr_start(subsc) < 0 && subsc_ptr_end(subsc) < 0)))
  { return luxerror("Range end < range start", range_end(range)); }

  if ((subsc_ptr_sum(subsc) = range_sum(range)) < 0 ||
      subsc_ptr_sum(subsc) > 1)
  { return luxerror("Illegal range summation flag??", range_sum(range)); }
  if (range_redirect(range) == -1) subsc_ptr_redirect(subsc) = -1;
  else if ((subsc_ptr_redirect(subsc) =
	    int_arg(eval(range_redirect(range)))) < 0 ||
	   subsc_ptr_redirect(subsc) >= MAX_DIMS)
  { return luxerror("Illegal range redirection", range_redirect(range)); }
  return subsc;
}
/*----------------------------------------------------------------*/
void convertPointer(scalar *target, int32_t inType, int32_t outType)
/* converts value in target from inType to outType */
{
  switch (outType) {
  case LUX_BYTE:
    switch (inType) {
    case LUX_WORD:
      (*target).b = (uint8_t) (*target).w;
      break;
    case LUX_LONG:
      (*target).b = (uint8_t) (*target).l;
      break;
    case LUX_FLOAT:
      (*target).b = (uint8_t) (*target).f;
      break;
    case LUX_DOUBLE:
      (*target).b = (uint8_t) (*target).d;
      break;
    }
    break;
  case LUX_WORD:
    switch (inType) {
    case LUX_BYTE:
      (*target).w = (int16_t) (*target).b;
      break;
    case LUX_LONG:
      (*target).w = (int16_t) (*target).l;
      break;
    case LUX_FLOAT:
      (*target).w = (int16_t) (*target).f;
      break;
    case LUX_DOUBLE:
      (*target).w = (int16_t) (*target).d;
      break;
    }
    break;
  case LUX_LONG:
    switch (inType) {
    case LUX_BYTE:
      (*target).l = (int32_t) (*target).b;
      break;
    case LUX_WORD:
      (*target).l = (int32_t) (*target).w;
      break;
    case LUX_FLOAT:
      (*target).l = (int32_t) (*target).f;
      break;
    case LUX_DOUBLE:
      (*target).l = (int32_t) (*target).d;
      break;
    }
    break;
  case LUX_FLOAT:
    switch (inType) {
    case LUX_BYTE:
      (*target).f = (float) (*target).b;
      break;
    case LUX_WORD:
      (*target).f = (float) (*target).w;
      break;
    case LUX_LONG:
      (*target).f = (float) (*target).l;
      break;
    case LUX_DOUBLE:
      (*target).f = (float) (*target).d;
      break;
    }
    break;
  case LUX_DOUBLE:
    switch (inType) {
    case LUX_BYTE:
      (*target).d = (double) (*target).b;
      break;
    case LUX_WORD:
      (*target).d = (double) (*target).w;
      break;
    case LUX_LONG:
      (*target).d = (double) (*target).l;
      break;
    case LUX_FLOAT:
      (*target).d = (double) (*target).f;
      break;
    }
    break;
  }
}
/*----------------------------------------------------------------*/
void convertWidePointer(wideScalar *target, int32_t inType, int32_t outType)
/* converts value in <target> from <inType> to <outType> */
{
  switch (inType) {
    case LUX_BYTE:
      switch (outType) {
	case LUX_BYTE:
	  break;
	case LUX_WORD:
	  target->w = (int16_t) target->b;
	  break;
	case LUX_LONG:
	  target->l = (int32_t) target->b;
	  break;
	case LUX_FLOAT:
	  target->f = (float) target->b;
	  break;
	case LUX_DOUBLE:
	  target->d = (double) target->b;
	  break;
	case LUX_CFLOAT:
	  target->cf.real = (float) target->b;
	  target->cf.imaginary = 0.0;
	  break;
	case LUX_CDOUBLE:
	  target->cd.real = (double) target->b;
	  target->cd.imaginary = 0.0;
	  break;
      }
      break;
    case LUX_WORD:
      switch (outType) {
	case LUX_BYTE:
	  target->b = (uint8_t) target->w;
	  break;
	case LUX_WORD:
	  break;
	case LUX_LONG:
	  target->l = (int32_t) target->w;
	  break;
	case LUX_FLOAT:
	  target->f = (float) target->w;
	  break;
	case LUX_DOUBLE:
	  target->d = (double) target->w;
	  break;
	case LUX_CFLOAT:
	  target->cf.real = (float) target->w;
	  target->cf.imaginary = 0.0;
	  break;
	case LUX_CDOUBLE:
	  target->cd.real = (double) target->w;
	  target->cd.imaginary = 0.0;
	  break;
      }
      break;
    case LUX_LONG:
      switch (outType) {
	case LUX_BYTE:
	  target->b = (uint8_t) target->l;
	  break;
	  break;
	case LUX_WORD:
	  target->w = (int16_t) target->l;
	  break;
	case LUX_LONG:
	  break;
	case LUX_FLOAT:
	  target->f = (float) target->l;
	  break;
	case LUX_DOUBLE:
	  target->d = (double) target->l;
	  break;
	case LUX_CFLOAT:
	  target->cf.real = (float) target->l;
	  target->cf.imaginary = 0.0;
	  break;
	case LUX_CDOUBLE:
	  target->cd.real = (double) target->l;
	  target->cd.imaginary = 0.0;
	  break;
      }
      break;
    case LUX_FLOAT:
      switch (outType) {
	case LUX_BYTE:
	  target->b = (uint8_t) target->f;
	  break;
	case LUX_WORD:
	  target->w = (int16_t) target->f;
	  break;
	case LUX_LONG:
	  target->l = (int32_t) target->f;
	  break;
	case LUX_FLOAT:
	  break;
	case LUX_DOUBLE:
	  target->d = (double) target->f;
	  break;
	case LUX_CFLOAT:
	  target->cf.real = (float) target->f;
	  target->cf.imaginary = 0.0;
	  break;
	case LUX_CDOUBLE:
	  target->cd.real = (double) target->f;
	  target->cd.imaginary = 0.0;
	  break;
      }
      break;
    case LUX_DOUBLE:
      switch (outType) {
	case LUX_BYTE:
	  target->b = (uint8_t) target->d;
	  break;
	case LUX_WORD:
	  target->w = (int16_t) target->d;
	  break;
	case LUX_LONG:
	  target->l = (int32_t) target->d;
	  break;
	case LUX_FLOAT:
	  target->f = (float) target->d;
	  break;
	case LUX_DOUBLE:
	  break;
	case LUX_CFLOAT:
	  target->cf.real = (float) target->d;
	  target->cf.imaginary = 0.0;
	  break;
	case LUX_CDOUBLE:
	  target->cd.real = (double) target->d;
	  target->cd.imaginary = 0.0;
	  break;
      }
      break;
    case LUX_CFLOAT:
      switch (outType) {
	case LUX_BYTE:
	  target->b = (uint8_t) target->cf.real;
	  break;
	case LUX_WORD:
	  target->w = (int16_t) target->cf.real;
	  break;
	case LUX_LONG:
	  target->l = (int32_t) target->cf.real;
	  break;
	case LUX_FLOAT:
	  target->f = (float) target->cf.real;
	  break;
	case LUX_DOUBLE:
	  target->d = (double) target->cf.real;
	  break;
	case LUX_CFLOAT:
	  break;
	case LUX_CDOUBLE:
	  target->cd.real = (double) target->cf.real;
	  target->cd.imaginary = (double) target->cf.imaginary;
	  break;
      }
      break;
    case LUX_CDOUBLE:
      switch (outType) {
	case LUX_BYTE:
	  target->b = (uint8_t) target->cd.real;
	  break;
	case LUX_WORD:
	  target->w = (int16_t) target->cd.real;
	  break;
	case LUX_LONG:
	  target->l = (int32_t) target->cd.real;
	  break;
	case LUX_FLOAT:
	  target->f = (float) target->cd.real;
	  break;
	case LUX_DOUBLE:
	  target->d = (double) target->cd.real;
	  break;
	case LUX_CFLOAT:
	  target->cf.real = (float) target->cd.real;
	  target->cf.imaginary = (float) target->cd.imaginary;
	  break;
	case LUX_CDOUBLE:
	  break;
      }
      break;
  }
}
/*----------------------------------------------------------------*/
void convertScalar(scalar *target, int32_t nsym, int32_t type)
/* returns scalar value of nsym, converted to proper type, in target */
{
 int32_t		n;
 pointer	ptr;

 n = scalar_type(nsym);
 ptr.b = &scalar_value(nsym).b;
 switch (type) {
 case LUX_BYTE:
   switch (n) {
   case LUX_BYTE:
     (*target).b = (uint8_t) *ptr.b;
     break;
   case LUX_WORD:
     (*target).b = (uint8_t) *ptr.w;
     break;
   case LUX_LONG:
     (*target).b = (uint8_t) *ptr.l;
     break;
   case LUX_FLOAT:
     (*target).b = (uint8_t) *ptr.f;
     break;
   case LUX_DOUBLE:
     (*target).b = (uint8_t) *ptr.d;
     break;
   }
   break;
 case LUX_WORD:
   switch (n) {
   case LUX_BYTE:
     (*target).w = (int16_t) *ptr.b;
     break;
   case LUX_WORD:
     (*target).w = (int16_t) *ptr.w;
     break;
   case LUX_LONG:
     (*target).w = (int16_t) *ptr.l;
     break;
   case LUX_FLOAT:
     (*target).w = (int16_t) *ptr.f;
     break;
   case LUX_DOUBLE:
     (*target).w = (int16_t) *ptr.d;
     break;
   }
   break;
 case LUX_LONG:
   switch (n) {
   case LUX_BYTE:
     (*target).l = (int32_t) *ptr.b;
     break;
   case LUX_WORD:
     (*target).l = (int32_t) *ptr.w;
     break;
   case LUX_LONG:
     (*target).l = (int32_t) *ptr.l;
     break;
   case LUX_FLOAT:
     (*target).l = (int32_t) *ptr.f;
     break;
   case LUX_DOUBLE:
     (*target).l = (int32_t) *ptr.d;
     break;
   }
   break;
 case LUX_FLOAT:
   switch (n) {
   case LUX_BYTE:
     (*target).f = (float) *ptr.b;
     break;
   case LUX_WORD:
     (*target).f = (float) *ptr.w;
     break;
   case LUX_LONG:
     (*target).f = (float) *ptr.l;
     break;
   case LUX_FLOAT:
     (*target).f = (float) *ptr.f;
     break;
   case LUX_DOUBLE:
     (*target).f = (float) *ptr.d;
     break;
   }
   break;
 case LUX_DOUBLE:
   switch (n) {
   case LUX_BYTE:
     (*target).d = (double) *ptr.b;
     break;
   case LUX_WORD:
     (*target).d = (double) *ptr.w;
     break;
   case LUX_LONG:
     (*target).d = (double) *ptr.l;
     break;
   case LUX_FLOAT:
     (*target).d = (double) *ptr.f;
     break;
   case LUX_DOUBLE:
     (*target).d = (double) *ptr.d;
     break;
   }
   break;
 }
}
/*----------------------------------------------------------------*/
int32_t lux_symbol_memory()
/* returns the total of the memory allocated for each LUX symbol */
/* - which is NOT the same as the total allocated memory. */
/* Note:  some small stuff is not included. */
{
 int32_t	i, mem = 0;

 for (i = 0; i < NSYM; i++)
 { switch (sym[i].class)
   { case LUX_EVB:
       switch (sym[i].type)
       { default:
	   break;
	 case EVB_CASE: case EVB_NCASE: case EVB_BLOCK: case EVB_INT_SUB:
	 case EVB_USR_SUB: case EVB_INSERT:
	   mem += symbol_memory(i);  break; }
       break;
     case LUX_STRING: case LUX_LIST: case LUX_SUBSC_PTR: case LUX_INT_FUNC:
     case LUX_USR_FUNC: case LUX_ARRAY:
       mem += symbol_memory(i);  break;
     case LUX_LIST_PTR:
       if (list_ptr_target(i) > 0)
	 mem += strlen(list_ptr_tag_string(i)) + 1;
       break;
     case LUX_SUBROUTINE: case LUX_FUNCTION: case LUX_BLOCKROUTINE:
       mem += routine_num_parameters(i)*(sizeof(char *) + sizeof(int16_t))
	 + routine_num_statements(i)*sizeof(int16_t);
       break; }
 }
 i = scalar_scratch(LUX_LONG);
 scalar_value(i).l = mem;
 return i;
}
/*----------------------------------------------------------------*/
int32_t lux_trace(int32_t narg, int32_t ps[])
/* activates/deactivates trace facility */
{
  extern float	CPUtime;

  if (narg > 0)
    trace = int_arg(*ps);
  else
    trace = 999;
  traceMode = internalMode;
  if (!traceMode)
    traceMode = trace? 143: 0;	/* 128 + 8 + 4 + 2 + 1 */
				/* /ENTER, /ROUTINE, /BRACES, /LOOP, /FILE */
  if (trace < 0) {
    trace = 0;
    return luxerror("Negative Trace Level", ps[0]);
  }
  if (trace) {
    printf("Tracing activated at level %1d\n", trace);
    if (traceMode & 127)
      printf("Tracing: ordinary");
    if (traceMode & T_FILE)
      printw(",  @files");
    if (traceMode & T_LOOP)
      printw(",  loops");
    if (traceMode & T_BLOCK)
      printw(",  {blocks}");
    if (traceMode & T_ROUTINE)
      printw(",  routines");
    if (traceMode & T_ROUTINEIO)
      printw(",  routine entry/exit");
    if (traceMode & T_SHOWSTATS)
      printw(".\nRegular SHOWSTATS output");
    if (traceMode & T_CPUTIME) {
      printw(".  CPUtime output");
      CPUtime = (float) clock()/CLOCKS_PER_SEC;
    }
    if (traceMode & T_SHOWEXEC)
      printw(".  Statement execution index");
    printw(".\n");
  } else
    puts("No tracing");
  return 1;
}
/*----------------------------------------------------------------*/
#define b_fix(name, value) { fixedValue(name, LUX_BYTE, value);  nFixed++; }
#define l_fix(name, value) { fixedValue(name, LUX_LONG, value);  nFixed++; }
#define q_fix(name, value) { fixedValue(name, LUX_QUAD, value);  nFixed++; }
#define f_fix(name, value) { fixedValue(name, LUX_FLOAT, value);  nFixed++; }
#define d_fix(name, value) { fixedValue(name, LUX_DOUBLE, value);  nFixed++; }
#define s_fix(name, value) { fixedValue(name, LUX_LSTRING, value);  nFixed++; }
#define cf_fix(name, re, im) { fixedValue(name, LUX_CFLOAT, re, im); nFixed++; }
#define cd_fix(name, re, im) { fixedValue(name, LUX_DFLOAT, re, im); nFixed++; }
#define l_ptr(name, value) installPointer(name, LUX_LONG, value)
#define f_ptr(name, value) installPointer(name, LUX_FLOAT, value)
#define d_ptr(name, value) installPointer(name, LUX_DOUBLE, value)
#define s_ptr(name, value) installPointer(name, LUX_TEMP_STRING, value)
#define fnc_p(name, value) installSysFunc(name, value)

char	*defaultRedirect = "diagnostic.lux";

int32_t range_warn_flag = 0, redim_warn_flag = 0, error_extra = 0,
  maxhistsize = 20000, histmin, histmax, lastmaxloc, lastminloc,
  scalemin = 0, scalemax = 255, fftdp = 0, lastmax_sym, lastmin_sym,
  autocon = 1, contour_mode, contour_box, contour_nlev = 5,
  contour_border = 3, contour_ticks = 3, contour_tick_pen = 3,
  contour_style = 1, setup = 0, tempSym, iformat, fformat, sformat,
  cformat, tformat, projectSym, projectTk = 0, stackSym, psfile,
  MSBfirst, area_diag = 1, lastmean_sym, lastsdev_sym, r_d_sym,
  d_r_sym;

float	contour_dash_lev, contour_tick_fac = 0.5, *p3d;
scalar	lastmin, lastmax, lastmean, lastsdev;
extern int32_t	lunplt, landscape, iorder, ilabx, ilaby, irxf, iryf, ndx,
        ndxs, nd, ndys, ifz, ifzx, ier, ifont, ndlabx, ndlaby, 
        ndot, ipltyp, iblank, maxregridsize, nExecuted,
        kb, nArg, ixhigh, iyhigh,
        tvsmt, badmatch, fstepx, fstepy,
        sort_flag, crunch_bits, crunch_slice, byte_count,
	current_pen, updateBoundingBox, index_cnt, uTermCol, page;
extern double	meritc;
#if MOTIF
extern int32_t	radio_state, radio_button;
#endif
extern float	xfac, yfac, xmin, xmax, ymin, ymax,
	wxb, wxt, wyb, wyt, ticx, ticxr, ticy, ticyr, plims[],
	fsized,	symsize, symratio, startx, starty, stepx, stepy,
	callig_xb, callig_yb, callig_ratio, slabx, slaby,
        dashsize, crunch_bpp, postXBot, postXTop,
	postYBot, postYTop, xerrsize, yerrsize;
extern int16_t	*stackPointer;

#if DEVELOP
extern int32_t	irzf, ifzz, ndz, ndzs, resample_type, fstepz;
extern float	wzb, wzt, ticz, ticzr, zmin, zmax, defaultProjection[], dvz;
#endif

#if HAVE_LIBX11
extern int32_t	text_menus, tvplanezoom;
#endif

#if HAVE_LIBX11
extern int32_t lux_button, eventSource, xcoord, ycoord, lux_keycode, lux_keysym,
  last_menu, menu_item, lux_event, preventEventFlush, root_x, root_y,
  xerrors, last_wid, display_width, display_height, private_colormap,
  zoom_frame, foreground_pixel, nColors, colormin, colormax, lux_keystate;

extern float	tviy, tviyb, tvix, tvixb, xhair, yhair, menu_x, menu_y,
		tvscale, zoom_xc, zoom_yc, zoom_mag, lumpx;
extern double	last_time, zoom_clo, zoom_chi;
#endif

#if MOTIF
extern int32_t	motif_flag;
#endif

char	*firstbreak;		/* for memck.c */

enumElem	classesStruct[] = {
  { "SCALAR", LUX_SCALAR },
  { "STRING", LUX_STRING },
  { "RANGE", LUX_RANGE },
  { "ARRAY", LUX_ARRAY },
  { "POINTER", LUX_POINTER },
  { "ASSOC", LUX_ASSOC },
  { "FUNC_PTR", LUX_FUNC_PTR },
  { "SCAL_PTR", LUX_SCAL_PTR },
  { "SUBSC_PTR", LUX_SUBSC_PTR },
  { "FILEMAP", LUX_FILEMAP },
  { "CLIST", LUX_CLIST },
  { "LIST", LUX_LIST },
  { "KEYWORD", LUX_KEYWORD },
  { "LIST_PTR", LUX_LIST_PTR },
  { "CPLIST", LUX_CPLIST },
  { "SUBROUTINE", LUX_SUBROUTINE },
  { "FUNCTION", LUX_FUNCTION },
  { "BLOCKROUTINE", LUX_BLOCKROUTINE },
  { "EVB", LUX_EVB },
  { "UNDEFINED", LUX_UNDEFINED }
};

enumElem	typesStruct[] = {
  { "BYTE", LUX_BYTE },
  { "WORD", LUX_WORD },
  { "LONG", LUX_LONG },
  { "FLOAT", LUX_FLOAT },
  { "DOUBLE", LUX_DOUBLE },
  { "STRING", LUX_TEMP_STRING },
  { "CFLOAT", LUX_CFLOAT },
  { "CDOUBLE", LUX_CDOUBLE },
  { "UNDEFINED", LUX_UNDEFINED }
};

enumElem	eventStruct[] =	{ /* see lux_register_event in menu.c */
  { "KEYPRESS", 1 },
  { "BUTTONPRESS", 4 },
  { "BUTTONRELEASE", 8 },
  { "POINTERMOTION", 16 },
  { "ENTERWINDOW", 32 },
  { "LEAVEWINDOW", 64 },
  { "WINDOW", 256 },
  { "MENU", 512 }
};

enumElem	coordSysStruct[] = {
  { "DEP", 0 },
  { "DVI", 1 },
  { "DEV", 2 },
  { "IMG", 3 },
  { "PLT", 4 },
  { "RIM", 5 },
  { "RPL", 6 },
  { "X11", 7 },
};

enumElem	filetypeStruct[] = {
  { "FZ", FILE_TYPE_ANA_FZ },
  { "ASTORE", FILE_TYPE_ANA_ASTORE },
  { "IDL", FILE_TYPE_IDL_SAVE },
  { "FITS", FILE_TYPE_FITS },
  { "JPEG", FILE_TYPE_JPEG },
  { "GIF", FILE_TYPE_GIF },
  { "TIFF", FILE_TYPE_TIFF },
  { "PPM", FILE_TYPE_PPM_RAW },
  { "PPMA", FILE_TYPE_PPM_ASCII },
  { "XPM", FILE_TYPE_XPM },
  { "XBM", FILE_TYPE_XBM },
  { "BMP", FILE_TYPE_BMP },
  { "SUN", FILE_TYPE_SUN_RAS },
  { "IRIS", FILE_TYPE_IRIS_RGB },
  { "TARGA", FILE_TYPE_TARGA_24 },
  { "PM", FILE_TYPE_PM }
};

struct boundsStruct bounds = {
  { 0, INT16_MIN, INT32_MIN, INT64_MIN, -FLT_MAX, -DBL_MAX},
  { UINT8_MAX, INT16_MAX, INT32_MAX, INT64_MAX, FLT_MAX, DBL_MAX }
};

int32_t	LUX_MATMUL_FUN;

internalRoutine *subroutine, *function;

#define	FORMATSIZE	1024
void symbolInitialization(void)
{
 int32_t	i, iq;
#if YYDEBUG
 extern int32_t	yydebug;
#endif
 extern int32_t	termRow, termCol, despike_count;
#if DEVELOP
 char	*p;
#endif
 int32_t	to_scratch_array(int32_t, int32_t, int32_t, int32_t []);
 extern char	*fmt_integer, *fmt_float, *fmt_string, *fmt_complex,
  *curScrat, *printString;
 union { uint8_t b[2]; int16_t w; } whichendian;

 /* determine if the machine is little-endian or bigendian */
 whichendian.w = 1;
 MSBfirst = (int32_t) whichendian.b[1]; /* most significant Byte first? */

 firstbreak = sbrk(0);		/* for memck.c */
 curTEIndex = tempExecutableIndex;
 if (signal(SIGFPE, exception) == SIG_ERR
     || signal(SIGINT, exception) == SIG_ERR
     || signal(SIGSEGV, exception) == SIG_ERR
     || signal(SIGCONT, exception) == SIG_ERR
     || signal(SIGTRAP, exception) == SIG_ERR)
   luxerror("Could not install exception handlers", 0);

 extern struct obstack *registered_subroutines;
 int32_t registered_subroutines_size
   = registered_subroutines? obstack_object_size(registered_subroutines): 0;
 subroutine = malloc(registered_subroutines_size
                     + nSubroutine*sizeof(internalRoutine));
 if (registered_subroutines)
   memcpy(subroutine, obstack_finish(registered_subroutines), 
          registered_subroutines_size);
 memcpy((char *) subroutine + registered_subroutines_size,
        subroutine_table, nSubroutine*sizeof(internalRoutine));
 nSubroutine += registered_subroutines_size/sizeof(internalRoutine);
 if (registered_subroutines)
   obstack_free(registered_subroutines, NULL);

 extern struct obstack *registered_functions;
 int32_t registered_functions_size
   = registered_functions? obstack_object_size(registered_functions): 0;
 function = malloc(registered_functions_size
                     + nFunction*sizeof(internalRoutine));
 if (registered_functions)
   memcpy(function, obstack_finish(registered_functions), 
          registered_functions_size);
 memcpy((char *) function + registered_functions_size,
        function_table, nFunction*sizeof(internalRoutine));
 nFunction += registered_functions_size/sizeof(internalRoutine);
 if (registered_functions)
   obstack_free(registered_functions, NULL);

 qsort(subroutine, nSubroutine, sizeof(internalRoutine), ircmp);
 qsort(function, nFunction, sizeof(internalRoutine), ircmp);
 for (i = 0; i < nSubroutine; i++) {
   if (i && !strcmp(subroutine[i].name, subroutine[i - 1].name))
     luxerror("Internal subroutine name %s is used for multiple routines!",
              0, subroutine[i].name);
   installKeys(&subroutine[i].keys);
 }
 for (i = 0; i < nFunction; i++) {
   if (i && !strcmp(function[i].name, function[i - 1].name))
     luxerror("Internal function name %s is used for multiple routines!",
              0, function[i].name);
   installKeys(&function[i].keys);
 }
 LUX_MATMUL_FUN = findInternalName("MPRODUCT", 0);
 inputStream = stdin;
 outputStream = stdout;
 l_fix("#ZERO", 	0);
 l_fix("#1", 		1);
 l_fix("#MINUS1", 	-1);
 l_fix("#42", 		42);
 l_fix("#0", 		0);
 f_fix("#INFTY",	INFTY);
 cf_fix("#I",		0.0, 1.0); /* imaginary unit */
 l_fix("#MAX_ARGS",	MAX_ARG);
 l_fix("#MAX_BYTE",	bounds.max.b);
 l_fix("#MAX_WORD",	bounds.max.w);
 l_fix("#MAX_LONG",	bounds.max.l);
 q_fix("#MAX_QUAD",     bounds.max.q);
 f_fix("#MAX_FLOAT", 	bounds.max.f);
 d_fix("#MAX_DOUBLE", 	bounds.max.d);
 l_fix("#MIN_WORD",	bounds.min.w);
 l_fix("#MIN_LONG",	bounds.min.l);
 q_fix("#MIN_QUAD",     bounds.min.q);
 f_fix("#MIN_FLOAT", 	FLT_MIN);
 d_fix("#MIN_DOUBLE", 	DBL_MIN);
 l_fix("#MAX_DIMS",	MAX_DIMS);
#if HAVE_LIBX11
 l_fix("#NUM_WINDOWS",  MAXWINDOWS - 1);
 l_fix("#NUM_PIXMAPS",  MAXPIXMAPS - 1);
#endif
 d_fix("#PI",		M_PI);
 d_fix("#2PI",		2*M_PI);
 d_fix("#E",		M_E);
 d_fix("#C",		299792458e0);
 d_fix("#G",		6.668E-8);
 d_fix("#H",		6.6252E-27);
 d_fix("#HB",		1.0544E-27);
 d_fix("#EC",		6.6252E-27);
 d_fix("#M",		9.1084E-28);
 d_fix("#K",		1.308046E-16);
 d_fix("#R",		8.317E7);
 d_fix("#RAD",		RAD);
 r_d_sym = nFixed;
 d_fix("#R.D",		RAD);
 d_fix("#DEG",		DEG);
 d_r_sym = nFixed;
 d_fix("#D.R",		DEG);

 iq = installString("#CLASS");
 stackSym = findVar(iq, 0);	/* stackSym is a dummy variable */
 sym[stackSym].class = LUX_ENUM;
 enum_type(stackSym) = LUX_LONG;
 enum_list(stackSym) = classesStruct;
 symbol_memory(stackSym) = sizeof(classesStruct);
 nFixed++;

 iq = installString("#COORDSYS");
 stackSym = findVar(iq, 0);
 symbol_class(stackSym) = LUX_ENUM;
 enum_type(stackSym) = LUX_LONG;
 enum_list(stackSym) = coordSysStruct;
 symbol_memory(stackSym) = sizeof(coordSysStruct);
 nFixed++;

 iq = installString("#EVENT");
 stackSym = findVar(iq, 0);
 sym[stackSym].class = LUX_ENUM;
 enum_type(stackSym) = LUX_LONG;
 enum_list(stackSym) = eventStruct;
 symbol_memory(stackSym) = sizeof(eventStruct);
 nFixed++;

 iq = installString("#FILETYPE");
 stackSym = findVar(iq, 0);
 sym[stackSym].class = LUX_ENUM;
 enum_type(stackSym) = LUX_LONG;
 enum_list(stackSym) = filetypeStruct;
 symbol_memory(stackSym) = sizeof(filetypeStruct);
 nFixed++;

 iq = installString("#TYPE");
 stackSym = findVar(iq, 0);
 sym[stackSym].class = LUX_ENUM;
 enum_type(stackSym) = LUX_LONG;
 enum_list(stackSym) = typesStruct;
 symbol_memory(stackSym) = sizeof(typesStruct);
 nFixed++;
 
#if DEVELOP
 iq = installString("#P3D");
 projectSym = findVar(iq, 0);
 sym[projectSym].class = LUX_ARRAY;
 array_type(projectSym) = LUX_FLOAT;
 symbol_memory(projectSym) =
   sizeof(array) + 16*sizeof(float);
 eallocate(p, i, char);
 array_header(projectSym) = (array *) p;
 array_num_dims(projectSym) = 2;
 array_dims(projectSym)[0] = array_dims(projectSym)[1] = 4;
 memcpy(array_data(projectSym), defaultProjection, 16*sizeof(float));
 p3d = (float *) array_data(projectSym);
 nFixed++;
#endif

 iq = installString("#STACK");
 stackSym = findVar(iq, 0);
 sym[stackSym].class = LUX_CLIST;
 clist_symbols(stackSym) = stackPointer;
 symbol_memory(stackSym) = 0;	/* or it will get deallocated sometime */
 nFixed++;

 iq = findVarName("#TYPESIZE", 0);
 i = 10;			/* lux_type_size[] # elements! */
 to_scratch_array(iq, LUX_LONG, 1, &i);
 memcpy(array_data(iq), lux_type_size, i*sizeof(int32_t));

 /* s_fix("#NL",		"\n"); */
 l_ptr("#COL",		&termCol);
 l_ptr("#ROW",		&termRow);

 l_ptr("#MSBFIRST",	&MSBfirst);
 s_ptr("#VERSION",	PACKAGE_VERSION);

 l_ptr("!AREA_DIAG",	&area_diag);
 l_ptr("!AUTOCON", 	&autocon);
 l_ptr("!BADMATCH",	&badmatch);
 l_ptr("!BB_UPDATE",	&updateBoundingBox);
 l_ptr("!BC",		&byte_count);
#if HAVE_LIBX11
 l_ptr("!BUTTON",	&lux_button);
#endif
 f_ptr("!BXB",		&postXBot);
 f_ptr("!BXT",		&postXTop);
 f_ptr("!BYB",		&postYBot);
 f_ptr("!BYT",		&postYTop);
 fnc_p("!CJD",		12);
 l_ptr("!COL",		&uTermCol);
#if HAVE_LIBX11
 l_ptr("!COLORMAX",	&colormax);
 l_ptr("!COLORMIN",	&colormin);
#endif
 l_ptr("!CONTOUR_BORDER",	&contour_border);
 l_ptr("!CONTOUR_BOX", 	&contour_box);
 f_ptr("!CONTOUR_DASH_LEV",	&contour_dash_lev);
 l_ptr("!CONTOUR_MODE",	&contour_mode);
 l_ptr("!CONTOUR_NLEV", 	&contour_nlev);
 l_ptr("!CONTOUR_STYLE",	&contour_style);
 l_ptr("!CONTOUR_TICKS", 	&contour_ticks);
 l_ptr("!CONTOUR_TICK_PEN", 	&contour_tick_pen);
 f_ptr("!CONTOUR_TICK_FAC", 	&contour_tick_fac);
 fnc_p("!CPUTIME",	2);
 f_ptr("!CRATIO",	&callig_ratio);
 l_ptr("!CRUNCH_BITS",	&crunch_bits);
 f_ptr("!CRUNCH_BPP",	&crunch_bpp);
 l_ptr("!CRUNCH_SLICE", &crunch_slice);
 fnc_p("!CTIME",	5);
 f_ptr("!DASHSIZE",	&dashsize);
 fnc_p("!DATE",		7);
 l_ptr("!DESPIKE_COUNT", &despike_count);
 l_ptr("!DLABX",	&ndlabx);
 l_ptr("!DLABY",	&ndlaby);
 l_ptr("!DOT",		&ndot);
 l_ptr("!DOTS",		&ndot);
 l_ptr("!ERASE",	&ier);
 l_ptr("!ERROR_EXTRA",	&error_extra);
 l_ptr("!ERRNO",	&errno);
#if HAVE_LIBX11
 l_ptr("!EVENTSOURCE",	&eventSource);
 l_ptr("!EVENTTYPE",	&lux_event);
#endif
 l_ptr("!FFTDP", 	&fftdp);
 l_ptr("!FONT",		&ifont);
#if HAVE_LIBX11
 l_ptr("!FOREGROUND_COLOR",	&foreground_pixel);
#endif
 fformat = s_ptr("!FORMAT_F", strsave("%14.7g"));
 iformat = s_ptr("!FORMAT_I", strsave("%10jd"));
 sformat = s_ptr("!FORMAT_S", strsave("%s"));
 cformat = s_ptr("!FORMAT_C", strsave("%28.7z"));
 fmt_integer = string_value(iformat);
 fmt_float = string_value(fformat);
 fmt_string = string_value(sformat);
 fmt_complex = string_value(cformat);
 f_ptr("!FSIZE",	&fsized);
 l_ptr("!FSTEPX",	&fstepx);
 l_ptr("!FSTEPY",	&fstepy);
#if DEVELOP
 l_ptr("!FSTEPZ",	&fstepz);
#endif
 l_ptr("!FZ",		&ifz);
 l_ptr("!FZX",		&ifzx);
#if DEVELOP
 l_ptr("!FZZ",		&ifzz);
#endif
 f_ptr("!HEIGHT",	&yfac);
 l_ptr("!HISTMIN", 	&histmin);
 l_ptr("!HISTMAX", 	&histmax);
 l_ptr("!IBLANK",	&iblank);
 l_ptr("!IORDER", 	&iorder);
#if HAVE_LIBX11
 l_ptr("!IX",		&xcoord);
#endif
 l_ptr("!IXHIGH",	&ixhigh);
#if HAVE_LIBX11
 l_ptr("!IY",		&ycoord);
#endif
 l_ptr("!IYHIGH",	&iyhigh);
 fnc_p("!JD",		11);
#if HAVE_LIBX11			/* a non-X11 version of this is needed */
 l_ptr("!KB",		&kb);
 l_ptr("!KEYCODE",	&lux_keycode);
 l_ptr("!KEYSTATE",	&lux_keystate);
 l_ptr("!KEYSYM",	&lux_keysym);
#endif
 l_ptr("!LABX",		&ilabx);
 l_ptr("!LABY",		&ilaby);
 l_ptr("!LANDSCAPE",	&landscape);
 l_ptr("!LASTMAX", 	&lastmax);
 l_ptr("!LASTMAXLOC", 	&lastmaxloc);
 l_ptr("!LASTMEAN",	&lastmean);
 l_ptr("!LASTMIN", 	&lastmin);
 l_ptr("!LASTMINLOC", 	&lastminloc);
 l_ptr("!LASTSDEV",	&lastsdev);
#if HAVE_LIBX11
 l_ptr("!LAST_MENU",	&last_menu);
 f_ptr("!LUMPX",	&lumpx);
#endif
 l_ptr("!MAXHISTSIZE", 	&maxhistsize);
 l_ptr("!MAXREGRIDSIZE", &maxregridsize);
#if HAVE_LIBX11
 l_ptr("!MENU_ITEM",	&menu_item);
#endif
 d_ptr("!MERITC",	&meritc);
#if MOTIF
 l_ptr("!MOTIF",	&motif_flag);
#endif
 f_ptr("!MXB",		&xmin);
 f_ptr("!MXT",		&xmax);
 f_ptr("!MYB",		&ymin);
 f_ptr("!MYT",		&ymax);
#if DEVELOP
 f_ptr("!MZB",		&zmin);
 f_ptr("!MZT",		&zmax);
#endif
 l_ptr("!NARG",		&nArg);
#if HAVE_LIBX11
 l_ptr("!NCOLORCELLS",	&nColors);
#endif
 l_ptr("!NEXECUTED",	&nExecuted);
#if HAVE_LIBX11
 l_ptr("!NOEVENTFLUSH",	&preventEventFlush);
#endif
 l_ptr("!PDEV", 	&lunplt);
 l_ptr("!PEN",		&current_pen);
 l_ptr("!PLTYP",	&ipltyp);
 f_ptr("!PLXERRB",	&xerrsize);
 f_ptr("!PLYERRB",	&yerrsize);
#if HAVE_LIBX11
 l_ptr("!PRIVATE_COLORMAP", &private_colormap);
#endif
 psfile = s_ptr("!PS_FILE", strsave("junk.eps"));
 l_ptr("!RANGE_WARN_FLAG",	&range_warn_flag);
 l_ptr("!READ_COUNT",	&index_cnt);
 fnc_p("!READKEY",	8);
 fnc_p("!READKEYNE",	9);
 l_ptr("!REDIM_WARN_FLAG",	&redim_warn_flag);
#if DEVELOP
 l_ptr("!REGRID_TYPE",	&resample_type);
#endif
#if HAVE_LIBX11
 l_ptr("!ROOT_X",	&root_x);
 l_ptr("!ROOT_Y",	&root_y);
#endif
 l_ptr("!ROW",		&page);
 l_ptr("!RX",		&irxf);
 l_ptr("!RY",		&iryf);
#if DEVELOP
 l_ptr("!RZ",		&irzf);
#endif
 l_ptr("!SCALEMAX", 	&scalemax);
 l_ptr("!SCALEMIN", 	&scalemin);
#if HAVE_LIBX11
 l_ptr("!SCREEN_HEIGHT", &display_height);
 l_ptr("!SCREEN_WIDTH",	&display_width);
#endif
 l_ptr("!SORT_FLAG",	&sort_flag);
 f_ptr("!STARTX",	&startx);
 f_ptr("!STARTY",	&starty);
 f_ptr("!STEPX",	&stepx);
 f_ptr("!STEPY",	&stepy);
 f_ptr("!SYMRATIO",	&symratio);
 f_ptr("!SYMSIZE",	&symsize);
 fnc_p("!SYSTIME",	10);
#if HAVE_LIBX11
 l_ptr("!TEXTMENUS",	&text_menus); /* development of text menus */
#endif
 f_ptr("!TICKX",	&ticx);
 f_ptr("!TICKXR",	&ticxr);
 f_ptr("!TICKY",	&ticy);
 f_ptr("!TICKYR",	&ticyr);
#if DEVELOP
 f_ptr("!TICKZ",	&ticz);
 f_ptr("!TICKZR",	&ticzr);
#endif
 fnc_p("!TIME",		6);
#if HAVE_LIBX11
 f_ptr("!TVIX",		&tvix);
 f_ptr("!TVIXB",	&tvixb);
 f_ptr("!TVIY",		&tviy);
 f_ptr("!TVIYB",	&tviyb);
 l_ptr("!TVPLANEZOOM",	&tvplanezoom); /* browser */
#endif
#if HAVE_LIBX11
 f_ptr("!TVSCALE",	&tvscale);
#endif
 l_ptr("!TVSMT",	&tvsmt);
 l_ptr("!PROJECT",	&projectTk);
 f_ptr("!WIDTH",	&xfac);
#if HAVE_LIBX11
 l_ptr("!WINDOW",	&last_wid);
#endif
 f_ptr("!WXB", 		&wxb);
 f_ptr("!WXT", 		&wxt);
 f_ptr("!WYB", 		&wyb);
 f_ptr("!WYT", 		&wyt);
#if DEVELOP
 f_ptr("!WZB",		&wzb);
 f_ptr("!WZT",		&wzt);
#endif
 f_ptr("!XB",		&plims[0]);
 f_ptr("!XC",		&callig_xb);
#if HAVE_LIBX11
 l_ptr("!XERRORS",	&xerrors);
 f_ptr("!XF",		&xhair);
 f_ptr("!XMENU",	&menu_x);
#endif
 f_ptr("!XT",		&plims[1]);
#if HAVE_LIBX11
 d_ptr("!XTIME",	&last_time);
#endif
 f_ptr("!YB",		&plims[2]);
 f_ptr("!YC",		&callig_yb);
#if HAVE_LIBX11
 f_ptr("!YF",		&yhair);
 f_ptr("!YMENU",	&menu_y);
#endif
 f_ptr("!YT",		&plims[3]);
#if YYDEBUG
 l_ptr("!YYDEBUG",	&yydebug);
#endif
 f_ptr("!ZB",		&plims[4]);
#if HAVE_LIBX11
 l_ptr("!ZOOMFRAME",	&zoom_frame);
 d_ptr("!ZOOMHIGH",	&zoom_chi);
 d_ptr("!ZOOMLOW",	&zoom_clo);
 f_ptr("!ZOOMMAG",	&zoom_mag);
 f_ptr("!ZOOMX",	&zoom_xc);
 f_ptr("!ZOOMY",	&zoom_yc);
#endif
 f_ptr("!ZT",		&plims[5]);

#if MOTIF
 l_ptr("$RADIO_BUTTON",	&radio_button);
 l_ptr("$RADIO_STATE",	&radio_state); /* browser */
#endif

	/* create a "universal" variable to assign to
	   in  EVAL(string) */
 iq = installString("!TEMP");
 tempSym = findVar(iq, 0);
 sym[tempSym].class = LUX_SCALAR;
 scalar_type(tempSym) = LUX_LONG;
 scalar_value(tempSym).l = 0;
 lastmax_sym = lookForVarName("!LASTMAX", 0);
 lastmin_sym = lookForVarName("!LASTMIN", 0);
 lastsdev_sym = lookForVarName("!LASTSDEV", 0);
 lastmean_sym = lookForVarName("!LASTMEAN", 0);
 eval_func = findInternalName("EVAL", 0);
 insert_subr = findInternalName("%INSERT", 1);
 printString = curScrat + FORMATSIZE;
 installing = 0;
}
/*----------------------------------------------------------------*/
int32_t matchInternalName(char *name, internalRoutine *table, int32_t size, int32_t hi)
/* matches name against the initial parts of all names in the table.
   returns index of first match (i.e., closest to the start of the table),
   or -1 if none were found.  LS97 */
{
 int32_t	lo = 0, mid, s;

 hi--;
 while (lo <= hi) {
   mid = (lo + hi)/2;
   if ((s = strncmp(name, table[mid].name, size)) < 0)
     hi = mid - 1;
   else if (s > 0)
     lo = mid + 1;
   else {
     while (mid > 0 && !strncmp(name, table[mid - 1].name, size))
       mid--;
     return mid;
   }
 }
 return -1;
}
/*----------------------------------------------------------------*/
void zerobytes(void *sp, int32_t len)
/* zeros <len> bytes starting at <sp> */
{
  char	*p;

  p = (char *) sp;
  while (len--) *p++ = '\0';
}
/*----------------------------------------------------------------*/
int32_t strncasecmp_p(char *s1, char *s2, int32_t n)
/* compares the first <n> bytes of strings <s1> and <s2> and returns 0 */
/* if they are equal in both strings, a number > 0 if <s2> is later */
/* that <s1> in the internal character set, or < 0 otherwise. */
/* LS 17feb97 */
{
  char	c1, c2;
  int32_t	i = 0;

  do
  { c1 = toupper(*s1++);
    c2 = toupper(*s2++);
    i++; }
  while (c1 == c2 && c1 != '\0' && c2 != '\0' && i < n);
  return c2 - c1;
}
/*----------------------------------------------------------------*/
int32_t strcasecmp_p(char *s1, char *s2)
/* compares strings <s1> and <s2> without regard to case and returns 0 */
/* if they are equal, a number > 0 if <s2> is later */
/* that <s1> in the internal character set, or < 0 otherwise. */
/* LS 21feb97 */
{
  int32_t	c1, c2;

  do
  { c1 = toupper(*s1++);
    c2 = toupper(*s2++); }
  while (c1 == c2 && c1 != '\0' && c2 != '\0');
  return c2 - c1;
}
/*----------------------------------------------------------------*/
int32_t	nBreakpoint = 0;
breakpointInfo	breakpoint[NBREAKPOINTS];
int32_t lux_breakpoint(int32_t narg, int32_t ps[])
/* BREAKPOINT,string[,/SET,/VARIABLE] */
/* BREAKPOINT,n[,/DISABLE,/ENABLE,/DELETE] */
/* BREAKPOINT,/LIST */
/* /LIST can be specified together with one of the other switches */
{
  static int32_t	curBreakpoint = 0;
  char	*s, *p;
  int32_t	n;
  
  if (narg) 
    switch (internalMode & 3) {
      case 0:
	switch (symbol_class(ps[0])) {
	  case LUX_STRING:	/* /SET */
	    s = string_arg(ps[0]);
	    if (!s)		/* empty string */
	      return -1;
	    if (nBreakpoint == NBREAKPOINTS) {
	      printf("Maximum number of breakpoints (%1d) has been reached\n",
		     NBREAKPOINTS);
	      return luxerror("New breakpoint has been rejected.", ps[0]);
	    }
	    /* seek an empty breakpoint slot */
	    while (breakpoint[curBreakpoint].status & BP_DEFINED)
	      if (++curBreakpoint == NBREAKPOINTS)
		curBreakpoint = 0;
	    p = strtok(s, ":");
	    breakpoint[curBreakpoint].name = strsave(p);
	    p = strtok(NULL, ":");
	    if (!p)			/* no number */
	      breakpoint[curBreakpoint].line = 0;
	    else {
	      if (!isdigit((uint8_t) *p))
		return
		  luxerror("Illegal breakpoint line number specification (%s)",
			ps[0], p);
	      breakpoint[curBreakpoint].line = atol(p);
	    }
	    breakpoint[curBreakpoint].status = BP_DEFINED | BP_ENABLED;
	    if (internalMode & 8) {
	      breakpoint[curBreakpoint].status |= BP_VARIABLE;
	      p = breakpoint[curBreakpoint].name;
	      while (*p) {
		*p = toupper(*p);
		p++;
	      }
	    }
	    nBreakpoint++;
	    break;
	  default:
	    return cerror(ILL_CLASS, ps[0]);
	  case LUX_SCALAR:		/* /ENABLE */
	    n = int_arg(ps[0]);

	    if (n < 0 || n >= NBREAKPOINTS)
	      return luxerror("Illegal breakpoint number", ps[0]);
	    if (!(breakpoint[n].status & 1))
	      return luxerror("Non-existent breakpoint", ps[0]);
	    breakpoint[n].status |= BP_ENABLED; /* enable */
	    break;
	}
	break;
      case 1:			/* /ENABLE */
	n = int_arg(ps[0]);
	if (n < 0 || n >= NBREAKPOINTS)
	  return luxerror("Illegal breakpoint number", ps[0]);
	if (!(breakpoint[n].status & BP_DEFINED))
	  return luxerror("Non-existent breakpoint", ps[0]);
	breakpoint[n].status |= BP_ENABLED; /* enable */
	break;
      case 2:			/* /DISABLE */
	n = int_arg(ps[0]);
	if (n < 0 || n >= NBREAKPOINTS)
	  return luxerror("Illegal breakpoint number", ps[0]);
	if (!(breakpoint[n].status & BP_DEFINED))
	  return luxerror("Non-existent breakpoint", ps[0]);
	breakpoint[n].status &= ~BP_ENABLED; /* disable */
	break;
      case 3:			/* /DELETE */
	n = int_arg(ps[0]);
	if (n < 0 || n >= NBREAKPOINTS)
	  return luxerror("Illegal breakpoint number", ps[0]);
	if (!(breakpoint[n].status & BP_DEFINED))
	  return luxerror("Non-existent breakpoint", ps[0]);
	free(breakpoint[n].name);
	breakpoint[n].status = 0; /* delete */
	nBreakpoint--;
	break;
    }
  if (!narg || internalMode & 4) { /* /LIST */
    if (nBreakpoint) {
      printf("Breakpoints:\n%2s: %4s %20s %4s %s\n", "nr", "type", "name",
	     "line", "status");
      for (n = 0; n < NBREAKPOINTS; n++)
	if (breakpoint[n].status & BP_DEFINED) { /* exists */
	  if (breakpoint[n].line == BP_VARIABLE)
	    printf("%2d: %4s %20s %4s %s\n", n, "var", breakpoint[n].name,
		   "", (breakpoint[n].status & BP_ENABLED)? "enabled":
		   "disabled");
	  else
	    printf("%2d: %4s %20s %4d %s\n", n, "f/r", breakpoint[n].name,
		   breakpoint[n].line,
		   (breakpoint[n].status & BP_ENABLED)? "enabled": "disabled");
	}
    } else
      puts("No breakpoints.");
  }
  return 1;
}
/*----------------------------------------------------------------*/
int16_t	watchVars[NWATCHVARS];
int32_t	nWatchVars = 0;
int32_t lux_watch(int32_t narg, int32_t ps[])
/* WATCH,<variable>[,/DELETE,/LIST] */
{
  static int32_t	curWatchVar = 0;
  int32_t	i;

  if (narg) {
    if (!symbolIsNamed(ps[0]))
      return luxerror("Need a named variable", ps[0]);
    if (internalMode & 1) {	/* /DELETE */
      for (i = 0; i < NWATCHVARS; i++) {
	if (watchVars[i] == ps[0]) {
	  watchVars[i] = 0;
	  nWatchVars--;
	  break;
	}
      }
      if (i == NWATCHVARS)
	printf("Variable %s was not being watched.\n",
	       symbolProperName(ps[0]));
    } else {			/* install */
      if (nWatchVars == NWATCHVARS - 1)
	return luxerror("Maximum number of watched variables is already reached",
		     ps[0]);
      while (watchVars[curWatchVar])
	curWatchVar++;
      watchVars[curWatchVar] = ps[0];
      nWatchVars++;
    }
  }
  if (internalMode & 2) {	/* /LIST */
    printw("Watched variables: ");
    for (i = 0; i < NWATCHVARS; i++)
      if (watchVars[i]) {
	if (symbol_context(watchVars[i]) > 0) {
	  printw(symbolProperName(symbol_context(watchVars[i])));
	  printw(".");
	}
	printw(symbolProperName(watchVars[i]));
      }
  }
  return LUX_OK;
}
/*----------------------------------------------------------------*/
int32_t lux_symbol_number(int32_t narg, int32_t ps[])
     /* returns the symbol number of the argument */
{
  int32_t	result;
  
  result = scalar_scratch(LUX_LONG);
  sym[result].spec.scalar.l = *ps;
  return result;
}
/*----------------------------------------------------------------*/
void mark(int32_t symbol)
{
  if (markIndex == MSSIZE - 1)
  { luxerror("mark: WARNING - Too many temps marked", symbol);
    return; }
  markStack[markIndex++] = symbol;
}
/*----------------------------------------------------------------*/
void pegMark(void)
{
  if (markIndex == MSSIZE - 1)
  { luxerror("pegMark: WARNING - Too many temps marked", -1);
    return; }
  markStack[markIndex++] = -1;
}
/*----------------------------------------------------------------*/
void pegParse(void)
{
  if (markIndex == MSSIZE - 1)
  { luxerror("pegParse: WARNING - Too many temps marked", -1);
    return; }
  markStack[markIndex++] = -2;
}
/*----------------------------------------------------------------*/
void zapParseTemps(void)
{
  int32_t	iq;
  
  while (markIndex > 0 && (iq = markStack[--markIndex]) >= 0)
    zapTemp(iq);
  markIndex++;			/* retain -2 on mark stack */
  if (iq != -2)
  { luxerror("zapParseTemps: WARNING - Not at parse level", -1);
    return; }
}
/*----------------------------------------------------------------*/
void removeParseMarker(void)
{
  if (markIndex < 1 || markStack[markIndex - 1] != -2)
  { luxerror("removeParseMarker: WARNING - Not at parse level", -1);
    return; }
  markIndex--;
}
/*----------------------------------------------------------------*/
void unMark(int32_t symbol)
{
  int32_t	i;

  i = markIndex;
  while (i--)
  { if (markStack[i] == symbol)
    { markStack[i] = 0;
      return; }
  }
}
/*----------------------------------------------------------------*/
void zapMarked(void)
{
  int32_t	iq;
  
  while (markIndex > 0 && (iq = markStack[--markIndex]) >= 0)
    zapTemp(iq);
}
/*----------------------------------------------------------------*/
void checkTemps(void)
/* for debugging: checks that the number of temporary (unnamed) */
/* variables is equal to what is expected. */
{
  int32_t	i, n;
  extern int32_t	nTempVariable;

  n = 0;
  for (i = TEMPS_START; i < TEMPS_END; i++)
    if (symbol_class(i) != LUX_UNUSED)
      n++;
  if (n != nTempVariable)
    printf("WARNING - %1d temps expected, %1d found\n",
	   nTempVariable, n);
  n = 0;
  for (i = TEMP_EXE_START; i < TEMP_EXE_END; i++)
    if (symbol_class(i) != LUX_UNUSED)
      n++;
  if (n != nTempExecutable)
    printf("WARNING - %1d temp executables expected, %1d found\n",
	   nTempExecutable, n);
}
/*----------------------------------------------------------------*/
#include <unistd.h>
int32_t lux_restart(int32_t narg, int32_t ps[])
{
  extern char	*programName;
  int32_t	saveHistory(void);

  printf("\nRestarting LUX...\n\n");
  saveHistory();
  if (execl(programName, programName, NULL)) {
    return luxerror("Restarting LUX (%s) failed; reason: %s\n", 0,
                    programName, strerror(errno));
  }
  return 1;
}
/*----------------------------------------------------------------*/
int32_t strccmp(char *s1, char *s2)
/* checks strings s1 and s2 for equality disregarding upper/lower case */
/* distinctions */
{
  while (*s1 && toupper(*s1++) == toupper(*s2++)) ;
  return *s1 - *s2;
}
/*----------------------------------------------------------------*/
int32_t structSize(int32_t symbol, int32_t *nstruct, int32_t *nbyte)
/* returns in <*nstruct> the number of structure descriptors that are
   required to describe <symbol>, and in <*nbyte> the total number of bytes
   covered by the data in <symbol>. */
{
  int32_t	n, ns, nb;
  pointer	p;
  listElem	*l;

  switch (symbol_class(symbol)) {
    case LUX_SCALAR: case LUX_CSCALAR:
      *nbyte = lux_type_size[symbol_type(symbol)];
      *nstruct = 1;
      return 1;
    case LUX_STRING:
      *nbyte = string_size(symbol);
      *nstruct = 1;
      return 1;
    case LUX_ARRAY: case LUX_CARRAY:
      *nbyte = array_size(symbol)*lux_type_size[array_type(symbol)];
      *nstruct = 1;
      return 1;
    case LUX_CLIST:
      p.w = clist_symbols(symbol);
      n = clist_num_symbols(symbol);
      *nbyte = 0;
      *nstruct = 1;		/* one extra for the struct info */
      while (n--) {
	if (structSize(*p.w++, &ns, &nb) == LUX_ERROR)
	  return LUX_ERROR;
	*nbyte += nb;
	*nstruct += ns;
      }
      return 1;
    case LUX_LIST:
      l = list_symbols(symbol);
      n = list_num_symbols(symbol);
      *nbyte = 0;
      *nstruct = 1;		/* one extra for the struct info */
      while (n--) {
	if (structSize(l++->value, &ns, &nb) == LUX_ERROR)
	  return LUX_ERROR;
	*nbyte += nb;
	*nstruct += ns;
      }
      return 1;
    case LUX_STRUCT:
      *nstruct = struct_num_all_elements(symbol);
      *nbyte = struct_total_size(symbol);
      return 1;
    default:
      puts("In structSize():");
      return cerror(ILL_CLASS, symbol);
  }
}
/*----------------------------------------------------------------*/
int32_t makeStruct(int32_t symbol, char *tag, structElem **se, char *data,
	       int32_t *offset, int32_t descend)
{
  int32_t	size, offset0, ndim, n;
  structElem	*se0;
  int16_t	*arg;
  listElem	*le;

  if (descend) {
    return LUX_OK;
  } else {
    (*se)->u.regular.tag = tag? strsave(tag): NULL;
    (*se)->u.regular.offset = *offset; /* Byte offset from start */
    switch (symbol_class(symbol)) {
      case LUX_SCALAR: case LUX_CSCALAR:
	(*se)->u.regular.type = scalar_type(symbol); /* data type */
	(*se)->u.regular.spec.singular.ndim = 0; /* 0 -> scalar */
	/* copy the value into the structure */
	size = lux_type_size[scalar_type(symbol)];/* bytes per value */
	memcpy(data + *offset, &scalar_value(symbol).b, size);
	break;
      case LUX_STRING:
	(*se)->u.regular.type = LUX_TEMP_STRING; /* data type */
	(*se)->u.regular.spec.singular.ndim = 1; /* strings always have 1 */
	if (!((*se)->u.regular.spec.singular.dims = malloc(sizeof(int32_t))))
	  return cerror(ALLOC_ERR, 0);
	size = string_size(symbol); /* bytes per value */
	(*se)->u.regular.spec.singular.dims[0] = size; /* first dimension */
	memcpy(data + *offset, string_value(symbol), size); /* copy value */
	break;
      case LUX_ARRAY: case LUX_CARRAY:
	(*se)->u.regular.type = array_type(symbol);
	ndim = array_num_dims(symbol);
	if (array_type(symbol) == LUX_STRING_ARRAY)
	  ndim++;		/* add one for string arrays to hold the */
				/* length of the strings */
	(*se)->u.regular.spec.singular.ndim = ndim;
	if (!((*se)->u.regular.spec.singular.dims = malloc(ndim*sizeof(int32_t))))
	  return cerror(ALLOC_ERR, 0);
	if (array_type(symbol) == LUX_STRING_ARRAY) {
	  (*se)->u.regular.spec.singular.dims[0] =
	    strlen(*(char **) array_data(symbol)); /* take length of first */
						   /* one for all */
	  memcpy((*se)->u.regular.spec.singular.dims + 1, 
		 array_dims(symbol), array_num_dims(symbol)*sizeof(int32_t));
	} else
	  memcpy((*se)->u.regular.spec.singular.dims,
		 array_dims(symbol), array_num_dims(symbol)*sizeof(int32_t));
	size = lux_type_size[array_type(symbol)]*array_size(symbol);
	memcpy(data + *offset, array_data(symbol), size); /* copy values */
	break;
      case LUX_CLIST:
	arg = clist_symbols(symbol);
	n = clist_num_symbols(symbol);
	offset0 = *offset;
	se0 = *se;
	while (n--)
	  if (makeStruct(*arg++, NULL, se, data, offset, 0) == LUX_ERROR)
	    return LUX_ERROR;
	arg = clist_symbols(symbol);
	n = clist_num_symbols(symbol);
	*offset = offset0;
	*se = se0;
	while (n--)
	  makeStruct(*arg++, NULL, se, data, offset, 1);
	break;
      case LUX_LIST:
	le = list_symbols(symbol);
	n = list_num_symbols(symbol);
	offset0 = *offset;
	se0 = *se;
	while (n--) {
	  if (makeStruct(le->value, le->key, se, data, offset, 0) == LUX_ERROR)
	    return LUX_ERROR;
	  le++;
	}
	le = list_symbols(symbol);
	n = list_num_symbols(symbol);
	*offset = offset0;
	*se = se0;
	while (n--) {
	  makeStruct(le->value, le->key, se, data, offset, 1);
	  le++;
	}
	break;
      case LUX_STRUCT:
	/* CONTINUE HERE */
	break;
    }
    *offset += size;
    (*se)++;
  }
  return LUX_OK;
}
/*----------------------------------------------------------------*/
int32_t lux_struct(int32_t narg, int32_t ps[])
/* definition of a structure.  Structures can contain values of all
   numerical and string types, with individual dimensional structures
   for each component.  Each element of a structure covers a specific
   amount of memory: strings are included with a predefined size.
   We specify structures as follows:

   STRUCT(list,dimensions)

   where <list> is a LIST or CLIST containing scalars, arrays, and
   lists, compact lists, and structures of such elements.
   LS 8aug98
*/
{
  int32_t	result, size, nstruct, dims[MAX_DIMS], ndim, n, i, offset;
  pointer	data;
  structElem	*se;
  
  if (structSize(ps[0], &nstruct, &size) == LUX_ERROR) /* check
							  specification */
    return LUX_ERROR;
  nstruct++;			/* one extra for the top-level description */
  ndim = narg - 1;
  if (get_dims(&ndim, ps + 1, dims) == LUX_ERROR) /* read dimensions */
    return LUX_ERROR;
  /* calculate the number of repetitions of the outer structure */
  n = 1;
  for (i = 0; i < ndim; i++)
    n *= dims[i];

  result = nextFreeTempVariable();
  if (result == LUX_ERROR)
    return LUX_ERROR;
  symbol_class(result) = LUX_STRUCT;
  symbol_memory(result) = sizeof(int32_t) /* to store the number of elements */
    + nstruct*sizeof(structElem) /* to store the structure information */
    + size*n;			/* to store the data values */
  data.v = malloc(symbol_memory(result));
  if (!data.v)
    return cerror(ALLOC_ERR, 0);
  *data.l = nstruct;
  symbol_data(result) = data.v;
  se = struct_elements(result);
  data.v = struct_data(result);

  /* we must fill in the first descriptor */
  switch (symbol_class(ps[0])) {
    case LUX_SCALAR: case LUX_CSCALAR: case LUX_STRING: case LUX_ARRAY:
    case LUX_CARRAY:
      n = 1;
      break;
    case LUX_CLIST:
      n = clist_num_symbols(ps[0]);
      break;
    case LUX_LIST:
      n = list_num_symbols(ps[0]);
      break;
    case LUX_STRUCT:
      n = struct_num_top_elements(ps[0]);
      break;
  }  
  se->u.first.nelem = n;
  se->u.first.size = size;
  se->u.first.ndim = ndim;
  if (!(se->u.first.dims = malloc(ndim*sizeof(int32_t))))
    return cerror(ALLOC_ERR, 0);
  memcpy(se->u.first.dims, dims, ndim*sizeof(int32_t));
  se++;				/* point at the next one */
  offset = 0;

  /* now recursively fill in the deeper ones */
  if (makeStruct(ps[0], NULL, &se, data.v, &offset, 0) == LUX_ERROR)
    return LUX_ERROR;
  return result;
}
/*----------------------------------------------------------------*/
int32_t translateEscapes(char *p)
/* replace explicit escape sequences \x by internal ones; returns */
/* the final length of the string */
{
  char	escapechars[] = "ntvbrfa\\?'\"", escapes[] = "\n\t\v\b\r\f\a\\?'\"",
    *p2, *p0;
  int32_t	i, c;

  p0 = p;
  while (*p) {
    if (*p == '\\') {		/* an escape sequence */
      p2 = strchr(escapechars, p[1]);
      if (p2) {
	*p = escapes[p2 - escapechars];
	memcpy(p + 1, p + 2, strlen(p) - 1);
      } else if (p[1] == 'x') {	/* a hex number */
	i = strtol(p + 2, &p2, 10);
	*p = i;
	memcpy(p + 1, p2, strlen(p2) + 1);
      } else if (isdigit((uint8_t) p[1]) && p[1] < '8') { /* an octal number */
	/* octal-number escape sequences have at most 3 octal digits.
	   we cannot rely on strtol because it may find more than 3
	   (e.g., when the user specifies '\000123' the \000 is an octal
	   specification and the 123 is regular text; strtol would
	   return 000123 as the number.  we must find the end of the
	   octal number manually. */
	p2 = p + 2;		/* just beyond the first octal digit */
	for (i = 2; i < 4; i++)
	  if (isdigit((uint8_t) *p2) && *p2 < '8')	/* an octal digit */
	    p2++;
	c = *p2;		/* temporary storage */
	*p2 = '\0';		/* temporary end to force strtol not to
				   read beyond the first three octal digits */
	i = strtol(p + 1, NULL, 8);
	*p2 = c;
	*p = i;
	memmove(p + 1, p2, strlen(p2) + 1);
      }
    }
    p++;
  }
  return p - p0;
}
/*----------------------------------------------------------------*/
int32_t installString(char *string)
/* installs string in symbol stack; returns index to stack */
{
 int32_t	index, n;
 char	*p, *p0;
#if YYDEBUG
 extern int32_t	yydebug;
#endif

 if ((index = nextFreeStackEntry()) == LUX_ERROR)
   return LUX_ERROR;		/* error */
 n = strlen(string) + 1;
 p = malloc(n);
 if (!p)
   return luxerror("Could not allocate %d bytes for string %s", 0, n, string);
 strcpy(p, string);
 p0 = p;

 if (translateEscapes(p) != n - 1) /* we shortened the string */
   p0 = realloc(p0, n + 1);
 
 symbolStack[index] = p0;
#if YYDEBUG
 if (yydebug)
   printf("installing %s as item %d\n", string, index);
#endif
 return index;
}
/*----------------------------------------------------------------*/
/* we undefine malloc to ensure we pick up the regular version.
 memory allocated in installKeys() is not associated with any particular
 LUX symbol and would otherwise show up as an error in checkList.
 LS 21sep98 */
#undef malloc
void installKeys(void *keys)
/* transforms a single string of keys to a null-terminated list of
   keyword addresses */
/* syntax:
    *                   (optional)  piping may work for this function
    +			(optional)  suppress evaluation of arguments
    -                   (optional)  suppress unused arguments
    |number|		(optional)  default value of internalMode
    %number%		(optional)  default value of plain offset
    entry:entry:...     (optional)  keyword entry list
   each entry consists of an optional number, immediately followed by
   the keyword.  The number is the value that is logically or-ed into
   internalMode if the corresponding key is selected.
   Example:
     |7|%2%4FIRST::11THIRD:FOURTH
   the default value of internalMode is 7.  if /FIRST or /THIRD is selected,
   then 4 or 11 is or-ed into internalMode.  Any non-keyword arguments start
   at position 2. */
{
 char	*p, **result, *copy;
 keyList	*theKeyList;
 int32_t	n = 1, i;

 if (!*(char **) keys)		/* empty key */
   return;
 /* ANSI C does not allow string constants to be modified, so we */
 /* make a copy of the string and modify that */
 if (!(copy = (char *) malloc(strlen(*(char **) keys) + 1))) {
   luxerror("Memory allocation error in installKeys [%s]", 0, (char *) keys);
   return;
 }
 strcpy(copy, *(char **) keys);
	/* count and null-terminate individual keywords */ 
 for (p = copy; *p; p++)
   if (*p == ':') {
     *p = '\0';
     n++;
   }
 if (!(result = (char **) malloc((n + 1)*sizeof(char *)))
     || !(theKeyList = (keyList *) malloc(sizeof(keyList)))) {
   luxerror("Memory allocation error in installKeys [%s].", 0, (char *) keys);
   return;
 }
 theKeyList->keys = result;
 p = copy;
 if (*p == '*') {		/* suitable for piping */
   theKeyList->pipe = 1;
   p++;
 } else
   theKeyList->pipe = 0;
 if (*p == '+') {		/* evaluation suppression */
   theKeyList->suppressEval = 1;
   p++;
 } else
   theKeyList->suppressEval = 0;
 if (*p == '-') {		/* suppress unused arguments */
				/* (see internal_routine()) */
   theKeyList->suppressUnused = 1;
   p++;
 } else
   theKeyList->suppressUnused = 0;
 if (*p == '|')	{		/* default internalMode */
   theKeyList->defaultMode = strtol(p + 1, &p, 10);
   p++; 			/* skip final | */
 } else
   theKeyList->defaultMode = 0;
 if (*p == '%')	{		/* default offset for plain arguments */
   theKeyList->offset = strtol(p + 1, &p, 10);
   p++; 			/* skip final % */
 } else
   theKeyList->offset = 0;
	/* enter all keyword addresses in result list */
 *result++ = p;
 for (i = 1; i < n; i++) {
   while (*p++);
   *result++ = p;
 }
 *result = 0;
 *(keyList **) keys = theKeyList;
}
/*----------------------------------------------------------------*/
int32_t findName(char *name, hashTableEntry *hashTable[], int32_t context)
/* searches for <name> in <hashTable[]> (with <context>).  if found, */
/* returns symbol number, otherwise installs a copy of the name in */
/* <hashTable[]> and sym[].  Returns -1 if an error occurs.  LS 6feb96 */
{
 int32_t		hashValue, i;
 hashTableEntry	*hp, *oldHp;
#if YYDEBUG
 extern int32_t	yydebug;
#endif
 extern char	ignoreSymbols;

 if (ignoreSymbols)
   return 0;
#if YYDEBUG
 if (yydebug)
   printf("seeking variable name %s\n", name);
#endif
 if (*name == '#' || *name == '!' || *name == '$')
   context = 0;
 hashValue = hash(name);
 hp = oldHp = hashTable[hashValue];
 while (hp) {
   if (!strcmp(hp->name, name)
       && sym[hp->symNum].context == context)
     return hp->symNum;		/* found name: variable already defined */
   oldHp = hp;
   hp = hp->next;
 }
		/* wasn't defined yet;  install if not !xxx */
 if (*name == '!' && !installing)
   return luxerror("Non-existent system variable %s", 0, name);
 hp = (hashTableEntry *) malloc(sizeof(hashTableEntry));
 if (!hp)
   return cerror(ALLOC_ERR, 0);
 if (oldHp)			/* current hash chain wasn't empty */
   oldHp->next = hp;
 else
   hashTable[hashValue] = hp;
 /* we use strsave_system so the name is not included in allocation lists
  by debug malloc.  LS 21sep98 */
 hp->name = strsave(name);
 getFreeNamedVariable(i);
 hp->symNum = i;
 hp->next = NULL;
 sym[i].xx = hashValue + 1;
 symbol_class(i) = LUX_UNDEFINED;
 sym[i].context = context;
 sym[i].line = curLineNumber;
 return i;
}
/*----------------------------------------------------------------*/
int32_t lux_verify(int32_t narg, int32_t ps[])
/* verifies that all referenced subroutines, functions, and files
   actually exist */
{
  char	*name, *p, compileName[12], oldInstalling;
  FILE	*fp;
  int32_t	i, n, oldContext, nsym, result;
  extern char	compileOnly;
  extern int32_t	executeLevel;

  result = 0;
  if (narg) {
    name = string_arg(ps[0]);
    fp = openPathFile(name, FIND_EITHER);
    if (!fp) {
      printf("Cannot open file %s\n", name);
      perror("System message");
      return LUX_ERROR;
    }
    compileOnly++;
    executeLevel++;
    oldContext = curContext;
    sprintf(compileName, "#COMPILE%1d", compileCount);
    n = installString(compileName);
    oldInstalling = installing;
    installing = 1;
    nsym = newSymbol(LUX_BLOCKROUTINE, n);
    installing = oldInstalling;
    curContext = nsym;
    n = nextCompileLevel(fp, expname);
    fclose(fp);
    if (n < 0
	|| newSymbol(LUX_BLOCKROUTINE, -nsym - 1) < 0) { /* some error */
      zap(nsym);
      return LUX_OK;
    } else {
      n = installString(compileName);
      result = newBlockSymbol(n);
      symbol_context(result) = nsym;
    }
    curContext = oldContext;
    executeLevel--;
  }

  for (i = EXE_START; i < TEMP_EXE_END; i++) {
    switch (symbol_class(i)) {
      case LUX_EVB:
	switch (evb_type(i)) {
	  case EVB_FILE:
	    name = file_name(i);
	    if ((p = strchr(name, ':'))) /* seek specific routine */
	      *p++ = '\0';		/* temporary string end */
	    fp = openPathFile(name, FIND_EITHER);
	    if (!fp && p) {
	      *--p = ':';
	      fp = openPathFile(name, FIND_EITHER);
	    }
	    if (fp)
	      fclose(fp);
	    else {
	      puts(symbolIdent(i, I_LINE | I_PARENT));
	      printf("Cannot open file %s\n", name);
	      perror("System message");
	    }
	    break;
	  case EVB_USR_SUB:
	    n = usr_sub_routine_num(i);
	    if (symbol_class(n) == LUX_STRING) {
	      /* routine name was not yet evaluated */
	      name = string_value(n);
	      if ((n = lookForName(name, subrHashTable, 0)) < 0) {
		/* not compiled in the meantime */
		fp = openPathFile(name, FIND_SUBR | FIND_LOWER);
		if (fp)		/* file exists */
		  fclose(fp);
		else {
		  p = symbolProperName(symbol_context(i));
		  if (p)
		    printf("%s: ", p);
		  puts(symbolIdent(i, I_LINE));
		  printf("Cannot find subroutine %s\n", name);
		}
	      }
	    }
	    break;
	  case EVB_USR_CODE:
	    n = usr_code_routine_num(i);
	    if (symbol_class(n) == LUX_STRING) {
	      /* routine name was not yet evaluated */
	      name = string_value(n);
	      if ((n = lookForName(name, blockHashTable, curContext)) < 0) {
		p = symbolProperName(symbol_context(i));
		if (p)
		  printf("%s: ", p);
		puts(symbolIdent(i, I_LINE));
		printf("Cannot find block routine %s\n", name);
	      }
	    }
	    break;
	}
	break;
    }
  }
  if (result) {
    zap(result);
    zap(nsym);
    compileOnly--;
  }
  return LUX_OK;
}
/*----------------------------------------------------------------*/
compileInfo *nextFreeCompileInfo(void)
{
  if (cur_c_info >= n_c_info) { /* need more room */
    n_c_info += 16;
    c_info = realloc(c_info, n_c_info*sizeof(compileInfo));
    if (!c_info) {
      puts("pushCompileLevel:");
	cerror(ALLOC_ERR, 0);
      return NULL;
    }
  }
  return &c_info[cur_c_info++];
}
/*----------------------------------------------------------------*/
void releaseCompileInfo(void)
{
  if (--cur_c_info)
    curCompileInfo--;
  else
    curCompileInfo = NULL;
}
/*----------------------------------------------------------------*/
static executionLevelInfo	*e_info;
static int32_t	n_e_info = 0, cur_e_info = 0;
void pushExecutionLevel(int32_t line, int32_t target)
{
  if (cur_e_info + 1 >= n_e_info) { /* need more room */
    n_e_info += 16;
    e_info = realloc(e_info, n_e_info*sizeof(executionLevelInfo));
    if (!e_info) {
      puts("pushExecutionLevel:");
      cerror(ALLOC_ERR, 0);
      return;
    }
    e_info[0].target = 0;
  }
  e_info[cur_e_info].line = line; /* remember the line number in the current */
				  /* context */
  e_info[++cur_e_info].target = target;/* remember the next target */
}
/*----------------------------------------------------------------*/
void popExecutionLevel(void)
{
  cur_e_info--;
}
/*----------------------------------------------------------------*/
void showExecutionLevel(int32_t symbol)
{
  int32_t	i, target;

  if (cur_e_info)
    for (i = cur_e_info; i >= 0; i--) {
      printf("At line ");
      if (i == cur_e_info)
	printf("%4d", symbol_line(symbol));
      else
	printf("%4d", e_info[i].line);
      printf(" in ");
      target = e_info[i].target;
      if (target > 0) {	/* something user-defined */
	switch (symbol_class(target)) {
	  case LUX_SUBROUTINE:
	    printf("subr");
	    break;
	  case LUX_FUNCTION:
	    printf("func");
	    break;
	  case LUX_BLOCKROUTINE:
	    printf("block");
	    break;
	  default:
	    printf("unknown");
	}
	printf(" %s\n", symbolProperName(target));
      } else {			/* a compiling file */
	if (c_info[-target].name)
	  printf("file \"%s\"\n", c_info[-target].name);
	else
	  printf("main\n");
      }
    } else
      printf("At line %4d in main\n", symbol_line(symbol));
}
/*----------------------------------------------------------------*/
