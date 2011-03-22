/* File install.c */
#if HAVE_CONFIG_H
#include "config.h"
#endif
#include "install.h"
#include <ctype.h> /* for toupper(11) isdigit(3) */
#include <errno.h> /* for errno(2) */
#include <error.h> /* for anaerror(58) */
#include <float.h> /* for FLT_MAX(2) DBL_MAX(2) DBL_MIN(1) FLT_MIN(1) */
#include <limits.h> /* for UCHAR_MAX(3) INT_MAX(1) SHRT_MAX(1) SHRT_MIN(1) INT_MIN(1) */
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
/* clock() on mips-sgi-irix64-6.2 is in unistd.h rather than in ANSI time.h */

#include "editorcharclass.h"
#include "action.h"
static char rcsid[] __attribute__ ((unused)) =
"$Id: install.c,v 4.3 2001/02/09 23:11:56 strous Exp $";

extern char		*symbolStack[];
extern symTableEntry	sym[];
extern hashTableEntry	*varHashTable[], *subrHashTable[], *funcHashTable[],
			*blockHashTable[];
extern word		listStack[];
extern internalRoutine	subroutine[], function[];
extern int		keepEVB;
extern char		*currentChar, line[];
extern FILE		*inputStream, *outputStream;
extern int		nExecuted;

int	anaerror(char *, int, ...), lookForName(char *, hashTableEntry *[], int),
	newSymbol(int, ...), ana_setup(), rawIo(void), cookedIo(void);
void	installKeys(void *keys), zerobytes(void *, int);
char	*strsave(char *), *symName(int, hashTableEntry *[]), *className(int),
	*typeName(int);
FILE	*openPathFile(char *, int);

int	nFixed = 0, noioctl = 0, trace = 0, curTEIndex;

char	batch = 0, *currentInputFile = NULL, ignoreSymbols = 0, restart = 0;

int	traceMode = T_FILE | T_LOOP | T_BLOCK | T_ROUTINE;

word	*listStackItem = listStack;

int	symbolStackIndex = 0, tempVariableIndex = TEMPS_START,
	nTempVariable = 0, namedVariableIndex = NAMED_START,
	nNamedVariable = 0, nSymbolStack = 0, executableIndex = EXE_START,
	nExecutable = 0, tempExecutableIndex = TEMP_EXE_START,
	nTempExecutable, zapContext = 0, installString(char *),
	ana_verify(int, int []), eval_func, insert_subr;

int	markStack[MSSIZE], markIndex = 0;

executionLevelInfo	*exInfo = NULL;
int	nexInfo = 0;

extern int	compileLevel, curLineNumber;
static char	installing = 1;

/*----------------------------------------------------------------*/ 
extern int ana_area(), ana_area2(), ana_array_statistics(),
  ana_atomize(), ana_batch(), ana_callig(), ana_close(),
  ana_contour(), ana_coordmap(), ana_crunch(),
  ana_cubic_spline_extreme(), ana_debug(), ana_decomp(),
  ana_decrunch(), ana_default(), ana_delete(), ana_distr(),
  ana_dsolve(), ana_dump(), ana_dump_stack(), ana_echo(),
  ana_noecho(), ana_endian(), ana_erase(), ana_execute(),
  ana_fcwrite(), ana_fileptr(), ana_fileread(), ana_filewrite(),
  ana_format_set(), ana_fprint(), ana_fprintf(), ana_fzread(),
  ana_fzwrite(), ana_fzhead(), ana_getmin9(), ana_help(), ana_hex(),
  ana_inserter(), ana_limits(), ana_noop(), ana_openr(), ana_openu(),
  ana_openw(), ana_oplot(), ana_pdev(), ana_pen(), ana_plot(),
  ana_pointer(), ana_printf(), ana_pop(), ana_postimage(),
  ana_postraw(), postrelease(), ana_push(), ana_quit(), ana_read(),
  ana_readarr(), ana_readf(), ana_readu(), ana_record(), ana_redim(),
  ana_redirect_diagnostic(), ana_arestore(), ana_rewindf(), ana_sc(),
  ana_scb(), ana_setenv(), ana_show(), ana_show_func(),
  ana_show_subr(), ana_spawn(), ana_step(), ana_swab(), ana_switch(),
  ana_type(), ana_trace(), ana_ulib(), ana_wait(), ana_zap(),
  ana_zero(), showstats(), ana_writeu(), ana_system(), ana_freadf(),
  ana_orientation(), ana_error(), site(), ana_set(), ana_tolookup(),
  ana_coordtrf(), ana_fread(), ana_dump_lun(), ana_cluster(),
  ana_astore(), ana_fzinspect(), ana_multisieve(), ana_crunchrun(),
  ana_swaphalf(), ana_chdir(), ana_replace_values(),
  ana_freads(), ana_one(), ana_disableNewline(), ana_enableNewline(),
  ana_shift(), ana_file_to_fz(), ana_zapnan(), ana_fft(),
  ana_buffering(), ana_pencolor(), ana_idlrestore(), ana_list(),
  ana_extract_bits(), ana_fftshift(), ana_manualterm(), ana_watch(),
  ana_fcrunwrite(), ana_fits_read(), ana_fits_write(), ana_subshift(),
  ana_subshiftc(), ana_byte_inplace(), ana_word_inplace(),
  ana_long_inplace(), ana_float_inplace(), ana_double_inplace(),
  ana_cfloat_inplace(), ana_cdouble_inplace(), ana_string_inplace(),
  ana_fade(), ana_fade_init();

int	ana_name();

#if DEVELOP
extern int ana_fitUnitCube(), ana_projection(), ana_plot3d(),
  ana_trajectory(), ana_getmin2(), ana_projectmap();
#endif

#if DEBUG
extern int	checkList(), ana_whereisAddress(), ana_show_temps(),
		ana_newallocs(), show_files();
#endif

#if HAVE_LIBJPEG
extern int	ana_read_jpeg6b(), ana_write_jpeg6b();
#endif

#if HAVE_SYS_MTIO_H
extern int	ana_tape_status(), ana_rewind(), ana_weof(), ana_unload(),
  ana_skipr(), ana_skipf(), ana_taprd(), ana_tapwrt(), ana_tapebufin(),
  ana_tapebufout(), ana_wait_for_tape();
#endif

extern int	ana_gifread(), ana_gifwrite();

#if X11
extern int ana_menu(), ana_menu_hide(), ana_menu_item(),
  ana_menu_kill(), ana_menu_pop(), ana_menu_read(),
  ana_register_event(), ana_window(), ana_xcopy(),
  ana_xdelete(), ana_xdrawline(), ana_xevent(), ana_xflush(),
  ana_xfont(),
  ana_xlabel(), ana_xloop(), ana_xopen(), ana_xplace(),
  ana_xport(), ana_xpurge(), ana_xquery(), ana_xsetaction(),
  ana_xsetbackground(), ana_xsetforeground(), ana_xtv(),
  ana_xtvlct(), ana_xtvmap(), ana_xtvraw(), ana_xtvread(),
  ana_xymov(), ana_wait_for_menu(), ana_xclose(), ana_xraise(),
  ana_xcursor(), ana_xanimate(), ana_xzoom(), ana_show_visuals(),
  ana_zoom(), ana_xtvplane(), ana_threecolors(), ana_tv3(),
  ana_xinvertline(), ana_xinvertarc(), ana_xdrawarc(), ana_colorComponents(),
  ana_colorstogrey(), ana_pixelsto8bit();
#endif

#if MOTIF
extern int ana_xmalignment(), ana_xmarmcolor(), ana_xmattach(),
  ana_xmattach_relative(), ana_xmbackgroundcolor(),
  ana_xmbordercolor(), ana_xmborderwidth(), ana_xmbottomshadowcolor(),
  ana_xmdestroy(), ana_xmdrawinglink(), ana_xmfont(),
  ana_xmforegroundcolor(), ana_xmgetpixmap(), ana_xmgetwidgetsize(),
  ana_xmlistadditem(), ana_xmlistdeleteall(), ana_xmlistdeleteitem(),
  ana_xmlistfunc(), ana_xmlistselect(), ana_xmlistsubr(),
  ana_xmmessage(), ana_xmposition(), ana_xmprompt(),
  ana_xmscaleresetlimits(), ana_xmscalesetvalue(),
  ana_xmselectcolor(), ana_xmsensitive(), ana_xmsetcolors(),
  ana_xmsetlabel(), ana_xmsetmargins(), ana_xmsetmnemonic(),
  ana_xmsetmodal(), ana_xmsetoptionselection(), ana_xmsetpixmap(),
  ana_xmtextappend(), ana_xmtexterase(), ana_xmtextfieldseteditable(),
  ana_xmtextfieldsetstring(), ana_xmtextseteditable(),
  ana_xmtextsetposition(), ana_xmtextsetrowcolumnsize(),
  ana_xmtextsetstring(), ana_xmtogglesetstate(),
  ana_xmtopshadowcolor(), ana_xtloop(), ana_xtmanage(),
  ana_xtunmanage(), ana_xmquery(), ana_xmscrollbarsetvalues(),
  ana_xmsetdirectory(), ana_xmsettitle(), ana_xmset_text_output(),
  ana_xmsize(), ana_xmtextfieldsetmaxlength(), ana_xtpopup(),
  ana_xtpopdown(), ana_xmraise(), ana_xmresizepolicy(), ana_xmtextreplace(),
  ana_xmgetwidgetposition(), ana_xminfo();
#endif

extern int ana_readorbits(), ana_showorbits();

extern int	peek();
extern int	ana_breakpoint();
extern int	insert();
int	ana_restart(int, int []);

#if MOTIF
int	ana_zeroifnotdefined(), ana_compile_file();	/* browser */
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

internalRoutine	subroutine[] = {
  { "%INSERT",	3, MAX_ARG, insert, /* execute.c */
    "1INNER:2OUTER:4ONEDIM:8SKIPSPACE:16ZERO:32ALL:64SEPARATE" },
  { "AREA",	1, 4, ana_area, ":SEED:NUMBERS:DIAGONAL" }, /* topology.c */
  { "AREA2",	2, 6, ana_area2, /* toplogy.c */
    "::SEED:NUMBERS:DIAGONAL:SIGN" },
  { "ARESTORE",	1, MAX_ARG, ana_arestore, 0 }, /* files.c */
  { "ARRAY_STATISTICS", 4, 7, ana_array_statistics, 0, }, /* fun1.c */
  { "ASTORE",	2, MAX_ARG, ana_astore, 0 }, /* files.c */
  { "ATOMIZE", 	1, 1, ana_atomize, "1TREE:2LINE" }, /* strous.c */
  { "BATCH",	0, 1, ana_batch, "1QUIT" }, /* symbols.c */
  { "BREAKPOINT", 0, 1, ana_breakpoint, /* install.c */
    "0SET:1ENABLE:2DISABLE:3DELETE:4LIST:8VARIABLE" },
  { "BUFFERING", 0, 1, ana_buffering, "0CHAR:1LINE:2PIPE" }, /* install.c */
  { "BYTE",	1, MAX_ARG, ana_byte_inplace, 0, }, /* symbols.c */
  { "C",	1, 7, ana_callig, /* hersh.c */
    "0DEP:1DVI:2DEV:3IMG:4PLT:5RIM:6RPL" },
#if CALCULATOR
  { "CALCULATOR",	0, 0, ana_calculator, 0 }, /* calculator.c */
#endif
  { "CALLIG",	1, 7, ana_callig, /* hersh.c */
    "0DEP:1DVI:2DEV:3IMG:4PLT:5RIM:6RPL" },
  { "CDOUBLE",	1, MAX_ARG, ana_cdouble_inplace, 0 }, /* symbols.c */
  { "CFLOAT",	1, MAX_ARG, ana_cfloat_inplace, 0 }, /* symbols.c */
  { "CHDIR",	0, 1, ana_chdir, "1SHOW" }, /* files.c */
#if DEBUG
  { "CHECKLIST", 0, 1, checkList, 0 }, /* debug.c */
#endif
  { "CLOSE",	1, 1, ana_close, 0 }, /* files.c */
  { "CLUSTER",	2, 8, ana_cluster, /* cluster.c */
    "|32|:CENTERS:INDEX:SIZE:SAMPLE:EMPTY:MAXIT:RMS:1UPDATE:2ITERATE:4VOCAL:8QUICK:16RECORD:32ORDERED" },
#if X11
  { "COLORCOMPONENTS", 4, 4, ana_colorComponents, 0 }, /* color.c */
  { "COLORSTOGREY", 1, 1, ana_colorstogrey, 0 }, /* color.c */
#endif
#if MOTIF
  { "COMPILE_FILE", 1, 1, ana_compile_file, 0 }, /* motifextra.c */
#endif
  { "CONTOUR",	1, 6, ana_contour, /* contour.c */
    "IMAGE:LEVELS:XMIN:XMAX:YMIN:YMAX:STYLE:DASHSIZE:1AUTOCONTOUR:2USERCONTOUR" },
  { "COORDTRF",	2, 4, ana_coordtrf, /* coord.c */
   "0DEP:1DVI:2DEV:3IMG:4PLT:5RIM:6RPL:7X11:8TODVI:16TODEV:24TOIMG:32TOPLT:40TORIM:48TORPL:56TOX11" },
  { "CRUNCH",	3, 3, ana_crunch, 0 }, /* crunch.c */
  { "CRUNCHRUN",	3, 3, ana_crunchrun, 0 }, /* crunch.c */
  { "CSPLINE_EXTR", 5, 8, ana_cubic_spline_extreme, "1KEEPDIMS:2PERIODIC::::POS:MINPOS:MINVAL:MAXPOS:MAXVAL" }, /* fun3.c */
  { "D",	0, MAX_ARG, ana_dump, /* fun1.c */
    "+|36|1FIXED:2SYSTEM:4ZERO:8LOCAL:24CONTEXT:32FOLLOW:64FULL" },
  { "DECOMP",	1, 1, ana_decomp, 0 }, /* fun2.c */
  { "DECRUNCH",	2, 2, ana_decrunch, 0 }, /* crunch.c */
  { "DEFAULT",	2, MAX_ARG, ana_default, "+" }, /* strous.c */
  { "DELETE", 	1, MAX_ARG, ana_delete, "+1POINTER" }, /* fun1.c */
  { "DIAGNOSTIC", 0, 1, ana_redirect_diagnostic, 0 }, /* strous.c */
  { "DISTR",	3, 3, ana_distr, 0 }, /* strous.c */
  { "DOUB",	1, MAX_ARG, ana_double_inplace, 0 }, /* symbols.c */
  { "DOUBLE",	1, MAX_ARG, ana_double_inplace, 0 }, /* symbols.c */
  { "DSOLVE",	2, 2, ana_dsolve, 0 }, /* fun2.c */
  { "DUMP", 	0, MAX_ARG, ana_dump, /* fun1.c */
    "+|36|1FIXED:2SYSTEM:4ZERO:8LOCAL:24CONTEXT:32FOLLOW:64FULL" },
  { "DUMP_LUN",	0, 0, ana_dump_lun, 0 }, /* files.c */
  { "DUMP_STACK", 0, 0, ana_dump_stack, 0 }, /* strous.c */
  { "ECHO",	0, 1, ana_echo, 0 }, /* symbols.c */
  { "ENDIAN",	1, 1, ana_endian, 0 }, /* strous.c */
  { "ERASE",	0, 5, ana_erase, /* plots.c */
    "0DEP:1DVI:2DEV:3IMG:4PLT:5RIM:6RPL:7X11:64SCREEN:128POSTSCRIPT:192PDEV" },
  { "ERROR",	0, 2, ana_error, "1STORE:2RESTORE" }, /* error.c */
  { "EXECUTE",	1, 1, ana_execute, "1MAIN" }, /* execute.c */
  { "EXIT",	0, 1, ana_quit, 0 }, /* fun1.c */
  { "EXTRACT_BITS", 4, 4, ana_extract_bits, 0 }, /* fun3.c */
  { "F0H",	1, 2, ana_fzhead, 0 }, /* files.c */
  { "F0HEAD",	1, 2, ana_fzhead, 0 }, /* files.c */
  { "F0R",	2, 3, ana_fzread, "|1|1PRINTHEADER" }, /* files.c */
  { "F0READ",	2, 3, ana_fzread, "|1|1PRINTHEADER" }, /* files.c */
  { "F0W",	2, 3, ana_fzwrite, 0 }, /* files.c */
  { "F0WRITE",	2, 3, ana_fzwrite, 0 }, /* files.c */
  { "FADE",	2, 2, ana_fade, 0 }, /* fun3.c */
  { "FADE_INIT", 2, 2, ana_fade_init, 0 }, /* fun3.c */
  { "FCRUNWRITE", 2, 3, ana_fcrunwrite, 0 }, /* files.c */
  { "FCRW",	2, 3, ana_fcrunwrite, 0 }, /* files.c */
  { "FCW",	2, 3, ana_fcwrite, "1RUNLENGTH" }, /* files.c */
  { "FCWRITE",	2, 3, ana_fcwrite, "1RUNLENGTH" }, /* files.c */
  { "FFT",	1, 3, ana_fft,	/* fun3.c */
    "::DIRECTION:1BACK:2ALL:4COMPLEX" }, 
  { "FFTSHIFT",	2, 2, ana_fftshift, 0 }, /* fun3.c */
  { "FILEPTR",	1, 2, ana_fileptr, "1START:2EOF:4ADVANCE" }, /* files.c */
  { "FILEREAD", 5, 5, ana_fileread, 0 }, /* files.c */
  { "FILETOFZ",	3, 3, ana_file_to_fz, 0 }, /* files.c */
  { "FILEWRITE", 2, 3, ana_filewrite, 0 }, /* files.c */
#if DEVELOP
  { "FIT3DCUBE", 0, 0, ana_fitUnitCube, 0 }, /* projection.c */
#endif
  { "FITS_READ", 2, 7, ana_fits_read, "|1|1TRANSLATE:2RAWVALUES::::::BLANK" }, /* files.c */
  { "FITS_WRITE", 2, 4, ana_fits_write, "1VOCAL" }, /* files.c */
  { "FIX",	1, MAX_ARG, ana_long_inplace, 0 }, /* symbols.c */
  { "FLOAT",	1, MAX_ARG, ana_float_inplace, 0 }, /* symbols.c */
  { "FORMAT_SET", 0, 1, ana_format_set, 0 }, /* files.c */
  { "FPRINT",	1, MAX_ARG, ana_fprint, "1ELEMENT" }, /* files.c */
  { "FPRINTF",	2, MAX_ARG, ana_fprintf, "1ELEMENT" }, /* files.c */
  { "FREAD",	2, MAX_ARG, ana_fread, "1COUNTSPACES" }, /* files.c */
  { "FREADF",	3, MAX_ARG, ana_freadf, "1COUNTSPACES" }, /* files.c */
  { "FREADS",	2, MAX_ARG, ana_freads, "1COUNTSPACES" }, /* files.c */
  { "FZH",	1, 2, ana_fzhead, 0 }, /* files.c */
  { "FZHEAD",	1, 2, ana_fzhead, 0 }, /* files.c */
  { "FZINSPECT", 2, 3, ana_fzinspect, 0 }, /* files.c */
  { "FZR",	2, 3, ana_fzread, "|1|1PRINTHEADER" }, /* files.c */
  { "FZREAD",	2, 3, ana_fzread, "|1|1PRINTHEADER" }, /* files.c */
  { "FZW",	2, 3, ana_fzwrite, "1SAFE" }, /* files.c */
  { "FZWRITE",	2, 3, ana_fzwrite, "1SAFE" }, /* files.c */
  { "GETMIN9",	3, 3, ana_getmin9, 0 }, /* fun4.c */
  { "GIFREAD",	2, 3, ana_gifread, 0 }, /* gifread_ana.c */
  { "GIFWRITE",	2, 3, ana_gifwrite, 0 }, /* gifwrite_ana.c */
#if X11
  { "HAIRS",	0, 0, ana_xplace, 0 }, /* xport.c */
#endif
  { "HELP",	0, 1, ana_help,	/* strous.c */
    "|30|1EXACT:2ROUTINE:4TREE:8SUBROUTINE:16FUNCTION:32LIST::PAGE" },
  { "HEX",	1, MAX_ARG, ana_hex, 0 }, /* files.c */
  { "IDLRESTORE", 1, 1, ana_idlrestore, 0 }, /* idl.c */
  { "INFO",	0, 0, site, /* site.c */
    "1TABLE:2TIME:4PLATFORM:8PACKAGES:16WARRANTY:32COPY:64BUGS:128KEYS:255ALL" },
  { "INSERT", 	2, 4, ana_inserter, 0 }, /* subsc.c */
  { "INT",	1, MAX_ARG, ana_word_inplace, 0 }, /* symbols.c */
#if HAVE_LIBJPEG
  { "JPEGREAD", 2, 4, ana_read_jpeg6b, ":::SHRINK:1GREYSCALE" }, /* jpeg.c */
  { "JPEGWRITE", 2, 4, ana_write_jpeg6b, 0 },/* jpeg.c */
#endif
  { "LIMITS",	0, 4, ana_limits, 0 }, /* plots.c */
  { "LIST",	1, 1, ana_list, 0 }, /* ident.c */
  { "LONG",	1, MAX_ARG, ana_long_inplace, 0 }, /* symbols.c */
#if X11
  { "MENU",	1, MAX_ARG, ana_menu, 0 }, /* menu.c */
  { "MENUHIDE",	1, 1, ana_menu_hide, 0 }, /* menu.c */
  { "MENUITEM",	3, 3, ana_menu_item, 0 }, /* menu.c */
  { "MENUPOP",	1, MAX_ARG, ana_menu_pop, 0 }, /* menu.c */
  { "MENUREAD",	4, 4, ana_menu_read, 0 }, /* menu.c */
  { "MENUZAP",	1, 1, ana_menu_kill, 0 }, /* menu.c */
#endif
  { "MULTISIEVE", 4, 4, ana_multisieve, 0 }, /* strous2.c */
#if DEBUG
  { "NEWALLOCS", 0, 1, ana_newallocs, "1RESET" }, /* debug.c */
#endif
  { "NOECHO",	0, 0, ana_noecho, 0 }, /* symbols.c */
  { "NOOP",	0, 0, ana_noop, 0 }, /* strous2.c */
  { "ONE",	1, 1, ana_one, 0 }, /* fun1.c */
  { "OPENR",	2, 2, ana_openr, "1GET_LUN" }, /* files.c */
  { "OPENU",	2, 2, ana_openu, "1GET_LUN" }, /* files.c */
  { "OPENW",	2, 2, ana_openw, "1GET_LUN" }, /* files.c */
  { "OPLOT",	1, 13, ana_oplot, /* plots.c */
    "1DVI:2DEV:3IMG:4PLT:5RIM:6RPL:128WHOLE:256CLIPBARS:XDATA:YDATA:SYMBOL:LINE:XTITLE:YTITLE:TITLE:DASHSIZE:XERRORS:YERRORS:BREAKS:XBARSIZE:YBARSIZE" },
  { "ORIENTATION", 3, 8, ana_orientation, /* orientation.c */
  "1VOCAL:2GETJ:0PARALLEL:4PERPENDICULAR:::ORIENTATION:VALUES:WAVENUMBER:GRID:ASPECT:ORDER" },
  { "PDEV",	0, 1, ana_pdev, 0 }, /* plots.c */
  { "PEEK",	1, 2, peek, 0 }, /* strous.c */
  { "PEN",	0, 2, ana_pen, "WIDTH:COLOR:1STANDARDGRAY" }, /* plots.c */
  { "PENCOLOR", 0, 1, ana_pencolor, 0 }, /* plots.c */
#if X11
  { "PIXELSTO8BIT", 3, 3, ana_pixelsto8bit, 0 }, /* color.c */
#endif
  { "PLOT",	1, 15, ana_plot, /* plots.c */
    "1DVI:2DEV:3IMG:4PLT:5RIM:6RPL:64KEEP:128WHOLE:256CLIPBARS:1024LII:1280LIO:1536LOI:1792LOO:XDATA:YDATA:SYMBOL:LINE:XTITLE:YTITLE:TITLE:DASHSIZE:XERRORS:YERRORS:BREAKS:XBARSIZE:YBARSIZE:XFMT:YFMT" },
#if DEVELOP
  { "PLOT3D",	1, 1, ana_plot3d, "1HIDE:2CUBE" }, /* projection.c */
#endif
  { "POINTER",	2, 2, ana_pointer, /* symbols.c */
    "+:1FUNCTION:2SUBROUTINE:4INTERNAL:8MAIN" },
  { "POP",	1, MAX_ARG, ana_pop, "%1%NUM" }, /* strous.c */
  { "POSTIMAGE", 1, 5, ana_postimage, 0 }, /* plots.c */
  { "POSTRAW",	1, 1, ana_postraw, 0 }, /* plots.c */
  { "POSTREL",	0, 4, postrelease, 0 }, /* plots.c */
  { "POSTRELEASE", 0, 4, postrelease, 0 }, /* plots.c */
  { "PRINT", 	1, MAX_ARG, ana_type, "1JOIN:2RAW:4SEPARATE" }, /* files.c */
  { "PRINTF",	1, MAX_ARG, ana_printf, "1JOIN:2RAW:4SEPARATE" }, /* files.c */
#if DEVELOP
  { "PROJECTION", 0, MAX_ARG, ana_projection, /* projection.c */
	"1RESET:2ORIGINAL::TRANSLATE:ROTATE:SCALE:PERSPECTIVE:OBLIQUE" },
#endif
  { "PUSH",	1, MAX_ARG, ana_push, 0 }, /* strous.c */
  { "QUIT", 	0, 1, ana_quit, 0 }, /* fun1.c */
  { "READ",	1, MAX_ARG, ana_read, "1ASKMORE:2WORD:4FLUSH" }, /* files.c */
  { "READARR",	1, 1, ana_readarr, 0 }, /* strous.c */
  { "READF",	2, MAX_ARG, ana_readf, "1ASKMORE:2WORD" }, /* files.c */
  { "READORBITS", 0, 1, ana_readorbits, "1LIST:2REPLACE" }, /* astron.c */
  { "READU",	2, MAX_ARG, ana_readu, 0 }, /* files.c */
#if HAVE_LIBJPEG
  { "READ_JPEG", 2, 4, ana_read_jpeg6b, ":::SHRINK:1GREYSCALE" }, /* jpeg.c */
#endif
  { "RECORD",	0, 1, ana_record, "1INPUT:2OUTPUT:4RESET" }, /* symbols.c */
  { "REDIM", 	2, 9, ana_redim, 0 }, /* subsc.c */
  { "REPLACE",	3, 3, ana_replace_values, 0 }, /* strous2.c */
  { "RESTART",	0, 0, ana_restart, 0 }, /* install.c */
  { "RESTORE",	2, 3, ana_fzread, "1PRINTHEADER" }, /* files.c */
#if HAVE_SYS_MTIO_H
  { "REWIND",	1, 1, ana_rewind, 0 }, /* tape.c */
#endif
  { "REWINDF",	1, 1, ana_rewindf, 0 }, /* files.c */
  { "S",	0, 1, ana_show, 0 }, /* fun1.c */
  { "SC",	3, 3, ana_sc, 0 }, /* fun3.c */
  { "SCANF",	2, MAX_ARG, ana_freadf, "+1COUNTSPACES" }, /* files.c */
  { "SCB",	3, 3, ana_scb, "1EVEN:2ODD" }, /* fun3.c */
  { "SET",	0, 1, ana_set,	/* symbols.c */
    "VISUAL:1SET:2RESET:4SHOWALLOC:8WHITEBACKGROUND:16ULIMCOORDS:32YREVERSEIMG:64OLDVERSION:128ZOOM:1024ALLOWPROMPTS:2048XSYNCHRONIZE:4096PARSESILENT" },
#if X11
  { "SETBACKGROUND", 1, 2, ana_xsetbackground, 0 }, /* xport.c */
  { "SETBG",	1, 2, ana_xsetbackground, 0 }, /* xport.c */
#endif
  { "SETENV",	1, 1, ana_setenv, 0 }, /* files.c */
#if X11
  { "SETFG",	1, 2, ana_xsetforeground, 0 }, /* xport.c */
  { "SETFOREGROUND", 1, 2, ana_xsetforeground, 0 }, /* xport.c */
#endif
  { "SHIFT",	1, 4, ana_shift, ":::BLANK:1TRANSLATE" }, /* strous2.c */
  { "SHOW", 	0, 1, ana_show, 0 }, /* fun1.c */
  { "SHOWORBITS", 0, 0, ana_showorbits, 0 }, /* astron.c */
  { "SHOWSTATS", 0, 0, showstats, 0 }, /* strous2.c */
#if DEBUG
  { "SHOW_FILES", 0, 0, show_files, 0 }, /* debug.c */
#endif
  { "SHOW_FUNC", 0, 1, ana_show_func, "1PARAMETERS" }, /* symbols.c */
  { "SHOW_SUBR", 0, 1, ana_show_subr, "1PARAMETERS" }, /* symbols.c */
#if DEBUG
  { "SHOW_TEMPS", 0, 0, ana_show_temps, 0 }, /* symbols.c */
#endif
#if X11
  { "SHOW_VISUALS", 0, 0, ana_show_visuals, 0 }, /* xport.c */
#endif
#if HAVE_SYS_MTIO_H
  { "SKIPF",	1, 2, ana_skipf, 0 }, /* tape.c */
  { "SKIPR",	1, 2, ana_skipr, 0 }, /* tape.c */
#endif
  { "SPAWN", 	1, 1, ana_spawn, "1SILENT" }, /* files.c */
  { "SSCANF",	2, MAX_ARG, ana_freads, "1COUNTSPACES" }, /* files.c */
  { "STEP",	0, 1, ana_step, 0 }, /* symbols.c */
  { "STORE",	2, 3, ana_fzwrite, "1SAFE" }, /* files.c */
  { "STRING",	1, MAX_ARG, ana_string_inplace, 0 }, /* symbols.c */
  { "SUBSHIFT",	4, 4, ana_subshift, 0 }, /* fun5.c */
  { "SUBSHIFTC", 4, 5, ana_subshiftc, 0 }, /* fun5.c */
  { "SWAB",	1, MAX_ARG, ana_swab, 0 }, /* fun2.c */
  { "SWAPB",	1, MAX_ARG, ana_swab, 0 }, /* fun2.c */
  { "SWAPHALF",	1, 1, ana_swaphalf, 0 }, /* strous2.c */
  { "SWITCH",	2, 2, ana_switch, 0 }, /* symbols.c */
  { "T", 	1, MAX_ARG, ana_type, "1JOIN:2RAW:4SEPARATE" }, /* files.c */
#if HAVE_SYS_MTIO_H
  { "TAPEBUFIN", 2, 5, ana_tapebufin, 0 }, /* tape.c */
  { "TAPEBUFOUT", 2, 5, ana_tapebufout, 0 }, /* tape.c */
  { "TAPE_STATUS", 0, 0, ana_tape_status, 0 }, /* tape.c */
  { "TAPRD",	2, 2, ana_taprd, 0 }, /* tape.c */
  { "TAPWRT",	2, 2, ana_tapwrt, 0 }, /* tape.c */
#endif
  { "TERMINAL", 1, 1, ana_manualterm, 0}, /* dummyterm.c */
#if X11
  { "THREECOLORS", 0, 1, ana_threecolors, 0 }, /* xport.c */
#endif
  { "TOLOOKUP",	3, 3, ana_tolookup, "1ONE" }, /* strous2.c */
  { "TRACE",	0, 1, ana_trace, /* install.c */
    "1FILE:2LOOP:4BRACES:8ROUTINE:143ALL:16SHOWSTATS:32CPUTIME:64SHOWEXEC:128ENTER" },
#if DEVELOP
  { "TRAJECTORY", 3, 7, ana_trajectory, 0 }, /* strous3.c */
#endif
#if X11
  { "TV",	1, 5, ana_xtv,	/* xport.c */
    ":X:Y:WINDOW:SCALE:0DEP:1DVI:2DEV:3IMG:4PLT:5RIM:6RPL:7X11:64SCREEN:128POSTSCRIPT:192PDEV:256PLOTWINDOW:512ZOOM:1024CENTER:16384BIT24" },
  { "TV3",	1, 7, ana_tv3, /* xport.c */
    ":TWO:THREE:X:Y:WINDOW:SCALE:0DEP:1DVI:2DEV:3IMG:4PLT:5RIM:6RPL:7X11:64SCREEN:128POSTSCRIPT:192PDEV:256PLOTWINDOW:512ZOOM:1024CENTER" },
  { "TVLAB",	3, 4, ana_xlabel, 0 }, /* xport.c */
  { "TVLCT",	3, 3, ana_xtvlct, "1FIXEDSIZE" }, /* xport.c */
  { "TVMAP",	1, 5, ana_xtvmap, /* xport.c */
    ":::WINDOW:SCALE:0DEP:1DVI:2DEV:3IMG:4PLT:5RIM:6RPL:7X11:64SCREEN:128POSTSCRIPT:192PDEV:256PLOTWINDOW:512ZOOM" },
  { "TVPLANE",	1, 5, ana_xtvplane, 0 }, /* xport.c */
  { "TVRAW",	1, 5, ana_xtvraw, /* xport.c */
    ":::WINDOW:SCALE:0DEP:1DVI:2DEV:3IMG:4PLT:5RIM:6RPL:7X11:64SCREEN:128POSTSCRIPT:192PDEV:256PLOTWINDOW:512ZOOM" },
  { "TVREAD",	0, 5, ana_xtvread, "1GREYSCALE" }, /* xport.c */
#endif
  { "TY",	1, MAX_ARG, ana_type, "1JOIN:2RAW:4SEPARATE" }, /* files.c */
  { "TYPE", 	1, MAX_ARG, ana_type, "1JOIN:2RAW:4SEPARATE" }, /* files.c */
  { "ULIB", 	0, 1, ana_ulib, 0 }, /* files.c */
#if HAVE_SYS_MTIO_H
  { "UNLOAD",	1, 1, ana_unload, 0 }, /* tape.c */
#endif
  { "VERIFY",	0, 1, ana_verify, 0 }, /* install.c */
  { "WAIT",	1, 1, ana_wait, 0 }, /* fun2.c */
#if X11
  { "WAIT_FOR_MENU", 0, 1, ana_wait_for_menu, 0 }, /* menu.c */
#endif
#if HAVE_SYS_MTIO_H
  { "WAIT_FOR_TAPE", 1, 1, ana_wait_for_tape, 0 }, /* tape.c */
#endif
  { "WATCH",	1, 1, ana_watch, "1DELETE:2LIST" }, /* install.c */
#if HAVE_SYS_MTIO_H
  { "WEOF",	1, 1, ana_weof, 0 }, /* tape.c */
#endif
#if DEBUG
  { "WHERE",	1, 1, ana_whereisAddress, "1CUT" }, /* debug.c */
#endif
#if X11
  { "WINDOW",	0, 6, ana_window, 0 }, /* plots.c */
#endif
  { "WORD",	1, 1, ana_word_inplace, 0 }, /* symbols.c */
  { "WRITEU",	2, MAX_ARG, ana_writeu, 0 }, /* files.c */
#if HAVE_LIBJPEG
  { "WRITE_JPEG", 2, 4, ana_write_jpeg6b, 0 }, /* jpeg.c */
#endif
#if X11
  { "XANIMATE",	1, 6, ana_xanimate, ":::FR1:FR2:FRS:1TIME" }, /* xport.c */
  { "XCLOSE",	0, 0, ana_xclose, 0 }, /* xport.c */
  { "XCOPY",	2, 8, ana_xcopy, 0 }, /* xport.c */
  { "XCURSOR",	2, 4, ana_xcursor, 0 }, /* xport.c */
  { "XDELETE",	1, 1, ana_xdelete, 0 }, /* xport.c */
  { "XDRAWARC",	4, 7, ana_xdrawarc, 0 }, /* xport.c */
  { "XDRAWLINE", 4, 5, ana_xdrawline, 0 }, /* xport.c */
  { "XEVENT",	0, 0, ana_xevent, 0 }, /* xport.c */
  { "XFLUSH",	0, 0, ana_xflush, 0 }, /* xport.c */
  { "XFONT",	1, 2, ana_xfont, 0 }, /* xport.c */
  { "XINVERTARC", 4, 7, ana_xinvertarc, 0 }, /* xport.c */
  { "XINVERTLINE", 4, 5, ana_xinvertline, 0 }, /* xport.c */
  { "XLABEL",	3, 4, ana_xlabel, 0 }, /* xport.c */
  { "XLOOP",	0, 1, ana_xloop, 0 }, /* menu.c */
#if MOTIF
  { "XMALIGNMENT", 2, 2, ana_xmalignment, 0 }, /* motif.c */
  { "XMARMCOLOR", 2, 2, ana_xmarmcolor, 0 }, /* motif.c */
  { "XMATTACH",	6, 6, ana_xmattach, 0 }, /* motif.c */
  { "XMATTACH_RELATIVE", 5, 5, ana_xmattach_relative, 0 }, /* motif.c */
  { "XMBACKGROUNDCOLOR", 2, 2, ana_xmbackgroundcolor, 0 }, /* motif.c */
  { "XMBORDERCOLOR", 2, 2, ana_xmbordercolor, 0 }, /* motif.c */
  { "XMBORDERWIDTH", 2, 2, ana_xmborderwidth, 0 }, /* motif.c */
  { "XMBOTTOMSHADOWCOLOR", 2, 2, ana_xmbottomshadowcolor, 0 }, /* motif.c */
  { "XMDESTROY", 1, 1, ana_xmdestroy, 0 }, /* motif.c */
  { "XMDRAWINGLINK", 2, 2, ana_xmdrawinglink, 0 }, /* motif.c */
  { "XMFONT",	2, 2, ana_xmfont, 0 }, /* motif.c */
  { "XMFOREGROUNDCOLOR", 2, 2, ana_xmforegroundcolor, 0 }, /* motif.c */
  { "XMGETPIXMAP", 2, 2, ana_xmgetpixmap, 0 }, /* motif.c */
  { "XMGETWIDGETPOSITION", 3, 3, ana_xmgetwidgetposition, 0 }, /* motif.c */
  { "XMGETWIDGETSIZE", 3, 3, ana_xmgetwidgetsize, 0 }, /* motif.c */
  { "XMINFO",	1, 1, ana_xminfo, 0 }, /* motif.c */
  { "XMLISTADDITEM", 2, 3, ana_xmlistadditem, 0 }, /* motif.c */
  { "XMLISTDELETEALL", 1, 1, ana_xmlistdeleteall, 0 }, /* motif.c */
  { "XMLISTDELETEITEM", 2, 2, ana_xmlistdeleteitem, 0 }, /* motif.c */
  { "XMLISTFUNC", 1, 1, ana_xmlistfunc, 0 }, /* motif.c */
  { "XMLISTSELECT", 2, 3, ana_xmlistselect, 0 }, /* motif.c */
  { "XMLISTSUBR", 1, 1, ana_xmlistsubr, 0 }, /* motif.c */
  { "XMMESSAGE", 1, 5, ana_xmmessage, 0 }, /* motif.c */
  { "XMPOSITION", 3, 5, ana_xmposition, 0 }, /* motif.c */
  { "XMPROMPT",	3, 8, ana_xmprompt, 0 }, /* motif.c */
  { "XMQUERY",	1, 1, ana_xmquery, 0 }, /* motif.c */
  { "XMRAISE",	1, 1, ana_xmraise, 0 },  /* motif.c */
  { "XMRESIZEPOLICY",	2, 2, ana_xmresizepolicy, 0 }, /* motif.c */
  { "XMSCALERESETLIMITS", 3, 4, ana_xmscaleresetlimits, 0 }, /* motif.c */
  { "XMSCALESETVALUE", 2, 2, ana_xmscalesetvalue, 0 }, /* motif.c */
  { "XMSCROLLBARSETVALUES", 1, 5, ana_xmscrollbarsetvalues, 0 }, /* motif.c */
  { "XMSELECTCOLOR", 2, 2, ana_xmselectcolor, 0 }, /* motif.c */
  { "XMSENSITIVE", 2, 2, ana_xmsensitive, 0 }, /* motif.c */
  { "XMSETCOLORS", 2, 2, ana_xmsetcolors, 0 }, /* motif.c */
  { "XMSETDIRECTORY", 2, 2, ana_xmsetdirectory, 0 }, /* motif.c */
  { "XMSETLABEL", 2, 2, ana_xmsetlabel, 0 }, /* motif.c */
  { "XMSETMARGINS", 3, 3, ana_xmsetmargins, 0 }, /* motif.c */
  { "XMSETMNEMONIC", 2, 2, ana_xmsetmnemonic, 0 }, /* motif.c */
  { "XMSETMODAL", 2, 2, ana_xmsetmodal, 0 }, /* motif.c */
  { "XMSETOPTIONSELECTION", 2, 2, ana_xmsetoptionselection, 0 }, /* motif.c */
  { "XMSETPIXMAP", 2, 2, ana_xmsetpixmap, 0 }, /* motif.c */
  { "XMSETTITLE", 2, 2, ana_xmsettitle, 0 }, /* motif.c */
  { "XMSET_TEXT_OUTPUT", 1, 1, ana_xmset_text_output, 0 }, /* motif.c */
  { "XMSIZE",	3, 3, ana_xmsize, 0 }, /* motif.c */
  { "XMTEXTAPPEND", 2, 3, ana_xmtextappend, 0 }, /* motif.c */
  { "XMTEXTERASE", 1, 1, ana_xmtexterase, 0 }, /* motif.c */
  { "XMTEXTFIELDSETEDITABLE", 2, 2, ana_xmtextfieldseteditable, 0 }, /* motif.c */
  { "XMTEXTFIELDSETMAXLENGTH", 2, 2, ana_xmtextfieldsetmaxlength, 0 }, /* motif.c */
  { "XMTEXTFIELDSETSTRING", 2, 3, ana_xmtextfieldsetstring, 0 }, /* motif.c */
  { "XMTEXTREPLACE",	3, 4, ana_xmtextreplace, 0 }, /* motif.c */
  { "XMTEXTSETEDITABLE", 2, 2, ana_xmtextseteditable, 0 }, /* motif.c */
  { "XMTEXTSETPOSITION", 2, 2, ana_xmtextsetposition, 0 }, /* motif.c */
  { "XMTEXTSETROWCOLUMNSIZE", 3, 3, ana_xmtextsetrowcolumnsize, 0 }, /* motif.c */
  { "XMTEXTSETSTRING", 2, 2, ana_xmtextsetstring, 0 }, /* motif.c */
  { "XMTOGGLESETSTATE", 2, 3, ana_xmtogglesetstate, 0 }, /* motif.c */
  { "XMTOPSHADOWCOLOR", 2, 2, ana_xmtopshadowcolor, 0 }, /* motif.c */
#endif
  { "XOPEN",	0, 1, ana_xopen, /* xport.c */
    "1PRIVATE_COLORMAP:2DEFAULT_COLORMAP:4SELECTVISUAL" },
  { "XPLACE",	0, 2, ana_xplace, /* xport.c */
    "1DVI:2DEV:3IMG:4PLT:5RIM:6RPL:7X11" },
  { "XPORT",	0, 7, ana_xport, 0 }, /* xport.c */
  { "XPURGE",	0, 0, ana_xpurge, 0 }, /* xport.c */
  { "XQUERY",	0, 1, ana_xquery, 0 }, /* xport.c */
  { "XRAISE",	1, 1, ana_xraise, 0 }, /* xport.c */
  { "XREGISTER", 0, 3, ana_register_event , /* menu.c */
    "1KEYPRESS:4BUTTONPRESS:8BUTTONRELEASE:16POINTERMOTION:32ENTERWINDOW:64LEAVEWINDOW:127ALLEVENTS:128ALLWINDOWS:256ALLMENUS:512DESELECT::WINDOW:MENU" },
  { "XSETACTION", 0, 2, ana_xsetaction, 0 }, /* xport.c */
#if MOTIF
  { "XTLOOP",	0, 1, ana_xtloop, 0 }, /* motif.c */
  { "XTMANAGE",	1, 1, ana_xtmanage, 0 }, /* motif.c */
  { "XTPOPDOWN", 1, 1, ana_xtpopdown, 0 }, /* motif.c */
  { "XTPOPUP",	1, 1, ana_xtpopup, 0 },	/* motif.c */
  { "XTUNMANAGE", 1, 1, ana_xtunmanage, 0 }, /* motif.c */
#endif
  { "XTV",	1, 4, ana_xtv,  /* xport.c */
    ":::WINDOW:SCALE:0DEP:1DVI:2DEV:3IMG:4PLT:5RIM:6RPL:7X11:64SCREEN:128POSTSCRIPT:192PDEV:256PLOTWINDOW:512ZOOM" },
  { "XTVREAD",	0, 5, ana_xtvread, "1GREYSCALE" }, /* xport.c */
  { "XWIN",	0, 7, ana_xport, 0 }, /* xport.c */
  { "XWINDOW",	0, 7, ana_xport, 0 }, /* xport.c */
  { "XYMOV",	2, 4, ana_xymov, /* plots.c */
    "|192|:::BREAKS:0DEP:1DVI:2DEV:3IMG:4PLT:5RIM:6RPL:7X11:64BOUNDINGBOX:128MOVEFIRST:256ALTDASH" },
#endif
  { "ZAP",	1, MAX_ARG, ana_zap, "+1POINTER" }, /* strous2.c */
  { "ZERO", 	1, MAX_ARG, ana_zero, 0 }, /* fun1.c */
#if MOTIF
  { "ZEROIFNOTDEFINED", 1, MAX_ARG, ana_zeroifnotdefined, 0}, /* motifextra.c */
#endif
  { "ZERONANS",	1, MAX_ARG, ana_zapnan, "*%1%VALUE" }, /* fun1.c */
#if X11
  { "ZOOM",	1, 3, ana_zoom, "1OLDCONTRAST" }, /* zoom.c */
#endif
};
int	nSubroutine = sizeof(subroutine)/sizeof(internalRoutine);

extern int ana_abs(), ana_acos(), ana_arestore_f(), ana_arg(),
  ana_array(), ana_asin(), ana_assoc(), ana_astore_f(), ana_atan(),
  ana_atan2(), ana_basin(), ana_basin2(), ana_beta(), ana_bisect(),
  ana_bmap(), bytarr(), ana_byte(), bytfarr(), ana_cbrt(), cdblarr(),
  cdblfarr(), ana_cdmap(), ana_cdouble(), ana_ceil(), ana_cfloat(),
  ana_cfmap(), cfltarr(), cfltfarr(), ana_chi_square(),
  ana_classname(), ana_complex(), ana_complexsquare(), ana_compress(),
  ana_concat(), ana_conjugate(), ana_convertsym(), ana_cos(),
  ana_cosh(), ana_cputime(), ana_crosscorr(), ana_crunch_f(),
  ana_ctime(), ana_cubic_spline(), ana_date(), ana_date_from_tai(),
  dblarr(), dblfarr(), ana_defined(), ana_delete(), ana_despike(),
  ana_detrend(), ana_differ(), ana_dilate(), ana_dilate_dir(),
  ana_dimen(), ana_dir_smooth(), ana_dir_smooth2(), ana_distarr(),
  ana_distr_f(), ana_dmap(), ana_double(), ana_equivalence(),
  ana_erf(), ana_erfc(), ana_erode(), ana_erode_dir(), ana_esmooth(),
  ana_eval(), ana_exp(), ana_expand(), ana_expm1(),
  ana_extract_bits_f(), ana_extreme_general(), ana_f_ratio(),
  ana_fcwrite_f(), ana_fft_f(), ana_fftshift_f(), ana_fileptr_f(),
  ana_filesize(), ana_filetype_name(), ana_find(), ana_findfile(),
  ana_find_max(), ana_find_maxloc(), ana_find_min(),
  ana_find_minloc(), ana_fitskey(), ana_fits_header_f(),
  ana_fits_read_f(), ana_fits_xread_f(), ana_float(), ana_floor(),
  fltarr(), fltfarr(), ana_fmap(), ana_freadf_f(), ana_freads_f(),
  ana_fstring(), ana_fzarr(), ana_fzhead_f(), ana_fzread_f(),
  ana_fzwrite_f(), ana_gamma(), ana_generalfit(), ana_get_lun(),
  ana_getenv(), ana_gridmatch(), ana_gsmooth(), ana_hamming(), ana_hilbert(),
  ana_hist(), ana_histr(), ana_identify_file(), ana_idlread_f(),
  ana_imaginary(), ana_incomplete_beta(), ana_incomplete_gamma(),
  ana_index(), ana_indgen(), ana_inpolygon(), intarr(), intfarr(),
  ana_isarray(), ana_isnan(), ana_isscalar(), ana_isstring(),
  ana_istring(), ana_j0(), ana_j1(), ana_jd(), ana_jn(),
  ana_ksmooth(), ana_laplace2d(), ana_lmap(), ana_local_maxf(),
  ana_local_maxloc(),
  ana_local_minf(), ana_local_minloc(), ana_log(), ana_log10(),
  ana_log1p(), lonarr(), lonfarr(), ana_long(), ana_lower(),
  ana_lsq(), ana_lsq2(), ana_match(), ana_matmul(), ana_max_dir(),
  ana_maxf(), ana_maxfilter(), ana_maxloc(), ana_mean(),
  ana_medfilter(), ana_median(), ana_memory(), ana_minf(),
  ana_minfilter(), ana_minloc(), ana_neg_func(),
  ana_noncentral_chi_square(), ana_not(), ana_num_dimen(),
  ana_num_elem(), ana_onef(), ana_openr_f(), ana_openu_f(),
  ana_openw_f(), ana_orderfilter(), ana_pit(), ana_poly(), ana_pow(),
  ana_power(), ana_printf_f(), ana_psum(), ana_quantile(), ana_quit(),
  ana_random(), ana_randomb(), ana_randomd(), ana_randomn(),
  ana_randomu(), ana_randome(), ana_readf_f(), ana_readkey(),
  ana_readkeyne(), ana_readu_f(), ana_real(), ana_redim_f(),
  ana_regrid(), ana_regrid3(), ana_regrid3ns(), ana_reorder(),
  ana_reverse(), ana_rfix(), ana_root3(), ana_runcum(), ana_runprod(),
  ana_runsum(), ana_scale(), ana_scalerange(), ana_sccomplex(),
  ana_sdev(), ana_segment(), ana_segment_dir(), ana_sgn(),
  ana_shift_f(), ana_sieve(), ana_sin(), ana_sinh(), ana_skipc(),
  ana_smap(), ana_smooth(), ana_solar_b(), ana_solar_l(),
  ana_solar_p(), ana_solar_r(), ana_sort(), ana_spawn_f(), ana_sqrt(),
  strarr(), ana_strcount(), ana_stretch(), ana_string(), ana_strlen(),
  ana_strloc(), ana_strpos(), ana_strreplace(), ana_strskp(),
  ana_strsub(), ana_strtok(), ana_strtol(), ana_strtrim(),
  ana_struct(), ana_student(), ana_subsc_func(), ana_sun_b(),
  ana_sun_d(), ana_sun_p(), ana_sun_r(), ana_symbol(),
  ana_symbol_memory(), ana_symbol_number(), ana_symclass(),
  ana_symdtype(), ana_systime(), ana_table(), ana_tai_from_date(),
  ana_tan(), ana_tanh(), ana_temp(), ana_tense(), ana_tense_curve(),
  ana_tense_loop(), ana_time(), ana_total(), ana_trace_decoder(),
  ana_trend(), ana_tri_name_from_tai(), ana_typeName(), ana_upper(),
  ana_variance(), ana_varname(), ana_voigt(), ana_wait_for_menu(),
  ana_wmap(), ana_word(), ana_y0(), ana_y1(), ana_yn(),
  ana_zapnan_f(), ana_zerof(), ana_zinv(), ana_fcrunwrite_f(),
  ana_strpbrk(), ana_shift3(), ana_area_connect(), ana_legendre(),
  ana_cartesian_to_polar(), ana_polar_to_cartesian(), ana_roll(),
  ana_siderealtime(), ana_asinh(),
  ana_acosh(), ana_atanh(), ana_astrf(), ana_antilaplace2d(),
  ana_cspline_find();

#if HAVE_REGEX_H
extern int ana_getdirectories(), ana_getfiles(), ana_getfiles_r(),
  ana_getmatchedfiles(), ana_getmatchedfiles_r(), ana_regex();
#endif

#if DEVELOP
extern int	ana_project(), ana_bsmooth(), ana_compile(),
  ana_bessel_i0(), ana_bessel_i1(), ana_bessel_k0(), ana_bessel_k1(),
  ana_bessel_kn(), ana_regridls(), ana_bigger235(),
  ana_geneticfit();
#endif

extern int	ana_gifread_f(), ana_gifwrite_f();

#if X11
extern int	ana_check_menu(), ana_check_window(), ana_colorpixel(),
  ana_event_name(), ana_xlabelwidth(), ana_xquery_f(), ana_xexist();
#endif

extern int ana_calendar(), ana_EasterDate(), /* ana_orbitalElement(), */
  ana_astropos(), ana_precess(), ana_constellation(),
  ana_constellationname(), ana_enhanceimage();

#if MOTIF

extern int ana_xmarrow(), ana_xmboard(), ana_xmbutton(),
  ana_xmcheckbox(), ana_xmcolumns(), ana_xmcommand(),
  ana_xmdialog_board(), ana_xmdrawingarea(), ana_xmfileselect(),
  ana_xmform(), ana_xmframe(), ana_xmgetoptionselection(),
  ana_xmgetwidgetaddress(), ana_xmhscale(), ana_xmlabel(),
  ana_xmlist(), ana_xmlistcount(), ana_xmlistfromfile(),
  ana_xmmenubar(), ana_xmoptionmenu(), ana_xmpixmapbutton(),
  ana_xmpulldownmenu(), ana_xmradiobox(), ana_xmrows(),
  ana_xmscalegetvalue(), ana_xmscrolledwindow(), ana_xmseparator(),
  ana_xmtext(), ana_xmtextbox(), ana_xmtextfield(),
  ana_xmtextfieldarray(), ana_xmtextfieldgetstring(),
  ana_xmtextfromfile(), ana_xmtextgetstring(), ana_xmtogglegetstate(),
  ana_xmvscale(), ana_xtparent(), ana_xtwindow(), ana_xmdialog_form(),
  ana_xmaddfiletolist(), ana_xmtoplevel_form(),
  ana_xmtoplevel_board(), ana_xmpixmapoptionmenu(), ana_xmscrolledwindowapp(),
  ana_xmfilegetlist(), ana_xmhscrollbar(), ana_xmtextgetinsertposition(),
  ana_xmtextgetlastposition(), ana_xmtextgetselection(), ana_xmvscrollbar();

#endif

#if HAVE_LIBJPEG
extern int ana_read_jpeg6b_f(), ana_write_jpeg6b_f();
#endif

extern int	vargsmooth(), ana_test();

internalRoutine function[] = {
  { "%A_UNARY_NEGATIVE", 1, 1, ana_neg_func, "*" },	/* fun1.c */
  { "%B_SUBSCRIPT", 1, MAX_ARG, ana_subsc_func, /* subsc.c */
    "1INNER:2OUTER:4ZERO:8SUBGRID:16KEEPDIMS:32ALL:64SEPARATE" },
  { "%C_CPUTIME", 0, 0, ana_cputime, 0 }, /* fun1.c */
  { "%D_POWER",	2, 2, ana_pow, "*" }, /* fun1.c */
  { "%E_CONCAT", 1, MAX_ARG, ana_concat, "1SLOPPY" }, /* subsc.c */
  { "%F_CTIME", 0, 0, ana_ctime, 0 }, /* fun1.c */
  { "%G_TIME", 	0, 0, ana_time, 0 }, /* fun1.c */
  { "%H_DATE", 	0, 0, ana_date, 0 }, /* fun1.c */
  { "%I_READKEY", 0, 0, ana_readkey, 0 }, /* strous.c */
  { "%J_READKEYNE", 0, 0, ana_readkeyne, 0 }, /* strous.c */
  { "%K_SYSTIME", 0, 0, ana_systime, 0 }, /* fun1.c */
  { "%L_JD",	0, 0, ana_jd, 0 }, /* fun1.c */
  { "ABS",	1, 1, ana_abs, "*" }, /* fun1.c */
  { "ACOS",	1, 1, ana_acos, "*" }, /* fun1.c */
  { "ACOSH",	1, 1, ana_acosh, "*" }, /* fun1.c */
  { "ALOG",	1, 1, ana_log, "*" }, /* fun1.c */
  { "ALOG10",	1, 1, ana_log10, "*" }, /* fun1.c */
  { "ANTILAPLACE2D", 2, 2, ana_antilaplace2d, 0 }, /* poisson.c */
  { "AREACONNECT", 2, 3, ana_area_connect, "::COMPACT:1RAW" }, /* topology.c */
  { "ARESTORE",	1, MAX_ARG, ana_arestore_f, 0 }, /* files.c */
  { "ARG",	1, 1, ana_arg, 0 }, /* fun3.c */
  { "ARRAY",	1, MAX_DIMS + 1, ana_array, 0 }, /* symbols.c */
  { "ASIN",	1, 1, ana_asin, "*" }, /* fun1.c */
  { "ASINH",	1, 1, ana_asinh, "*" }, /* fun1.c */
  { "ASSOC",	2, 3, ana_assoc, "::OFFSET" }, /* symbols.c */
  { "ASTORE",	2, MAX_ARG, ana_astore_f, 0 }, /* files.c */
  { "ASTRF",	1, 2, ana_astrf, "1FROMEQUATORIAL:2FROMECLIPTICAL:4FROMGALACTIC:8TOEQUATORIAL:16TOECLIPTICAL:32TOGALACTIC:64JULIAN:128BESSELIAN" }, /* astron.c */
  { "ASTRON",	2, 6, ana_astropos, /* astron.c */
    "|2304|:::OBSERVER:EQUINOX:ELEMENTS:1ECLIPTICAL:2EQUATORIAL:3HORIZONTAL:4ELONGATION:8XYZ:32DATE:64TDT:128ERROR:256ABBERATION:512GEOMETRIC:1024QELEMENTS:2048FK5:4096TRUNCVSOP:8192CONJSPREAD:16384PLANETOCENTRIC:32768KEEPDIMENSIONS:65536VOCAL" },
  { "ATAN",	1, 1, ana_atan, "*" }, /* fun1.c */
  { "ATAN2",	2, 2, ana_atan2, "*" }, /* fun1.c */
  { "ATANH",	1, 1, ana_atanh, "*" }, /* fun1.c */
  { "ATOL",	1, 2, ana_strtol, 0 }, /* fun3.c */
  { "BASIN",	1, 2, ana_basin2, /* strous.c */
    "*1NUMBER:2SINK:4DIFFERENCE" },
#if DEVELOP
  { "BESSEL_I0",  1, 1, ana_bessel_i0, "*1DEFLATE" }, /* fun1.c */
  { "BESSEL_I1",  1, 1, ana_bessel_i1, "*" }, /* fun1.c */
#endif
  { "BESSEL_J0", 1, 1, ana_j0, "*" }, /* fun1.c */
  { "BESSEL_J1", 1, 1, ana_j1, "*" }, /* fun1.c */
  { "BESSEL_JN", 2, 2, ana_jn, "*" }, /* fun1.c */
#if DEVELOP
  { "BESSEL_K0", 1, 1, ana_bessel_k0, "*" }, /* fun1.c */
  { "BESSEL_K1", 1, 1, ana_bessel_k1, "*" }, /* fun1.c */
  { "BESSEL_KN", 2, 2, ana_bessel_kn, "*" }, /* fun1.c */
#endif
  { "BESSEL_Y0", 1, 1, ana_y0, "*" }, /* fun1.c */
  { "BESSEL_Y1", 1, 1, ana_y1, "*" }, /* fun1.c */
  { "BESSEL_YN", 2, 2, ana_yn, "*" }, /* fun1.c */
  { "BETA",	2, 2, ana_beta, "*1LOG" }, /* fun1.c */
#if DEVELOP
  { "BI0",	1, 1, ana_bessel_i0, "*1DEFLATE" }, /* fun1.c */
  { "BI1",	1, 1, ana_bessel_i1, "*" }, /* fun1.c */
  { "BIGGER235", 1, 1, ana_bigger235, "*" }, /* fun4.c */
#endif
  { "BISECT",	2, 6, ana_bisect, ":::AXIS:POS:WIDTH" }, /* strous3.c */
  { "BJ0",	1, 1, ana_j0, "*" }, /* fun1.c */
  { "BJ1",	1, 1, ana_j1, "*" }, /* fun1.c */
  { "BJN",	2, 2, ana_jn, "*" }, /* fun1.c */
#if DEVELOP
  { "BK0",	1, 1, ana_bessel_k0, "*" }, /* fun1.c */
  { "BK1",	1, 1, ana_bessel_k1, "*" }, /* fun1.c */
  { "BKN",	2, 2, ana_bessel_kn, "*" }, /* fun1.c */
#endif
  { "BMAP",	1, 1, ana_bmap, "*" }, /* subsc.c */
#if DEVELOP
  { "BSMOOTH",	1, 3, ana_bsmooth, 0 }, /* strous.c */
#endif
  { "BY0",	1, 1, ana_y0, "*" }, /* fun1.c */
  { "BY1",	1, 1, ana_y1, "*" }, /* fun1.c */
  { "BYN",	2, 2, ana_yn, "*" }, /* fun1.c */
  { "BYTARR",	1, MAX_DIMS, bytarr, 0 }, /* symbols.c */
  { "BYTE",	1, 1, ana_byte, "*" }, /* symbols.c */
  { "BYTFARR",	3, MAX_DIMS + 1, bytfarr, "%1%OFFSET:1READONLY:2SWAP" }, /* filemap.c */
 { "CALENDAR",	1, 1, ana_calendar, /* astron.c */
   "1FROMCOMMON:2FROMGREGORIAN:3FROMISLAMIC:4FROMJULIAN:5FROMJD:6FROMHEBREW:8FROMLONGCOUNT:9FROMEGYPTIAN:10FROMLUNAR:16TOCOMMON:32TOGREGORIAN:48TOISLAMIC:64TOJULIAN:80TOJD:96TOHEBREW:112TOMAYAN:128TOLONGCOUNT:144TOEGYPTIAN:160TOLUNAR:176TOLATIN:256TOTEXT:512TOISOTEXT:0FROMUTC:1024FROMTAI:2048FROMTT:0TOUTC:4096TOTAI:8192TOTT:0FROMYMD:16384FROMDMY:0TOYMD:32768TODMY" },
  { "CBRT",	1, 1, ana_cbrt, "*" }, /* fun1.c */
  { "CDBLARR",	1, MAX_ARG, cdblarr, 0 }, /* symbols.c */
  { "CDBLFARR", 3, MAX_DIMS + 1, cdblfarr, "%1%OFFSET:1READONLY:2SWAP" }, /* filemap.c */
  { "CDMAP",	1, 1, ana_cdmap, 0 }, /* subsc.c */
  { "CDOUBLE",	1, 1, ana_cdouble, "*" }, /* fun1.c */
  { "CEIL",	1, 1, ana_ceil, "*" }, /* symbols.c */
  { "CFLOAT",	1, 1, ana_cfloat, "*" }, /* fun1.c */
  { "CFLTARR",	1, MAX_ARG, cfltarr, 0 }, /* symbols.c */
  { "CFLTFARR", 3, MAX_DIMS + 1, cfltfarr, "%1%OFFSET:1READONLY:2SWAP" }, /* filemap.c */
  { "CFMAP",	1, 1, ana_cfmap, 0 }, /* subsc.c */
#if X11
  { "CHECKMENU", 0, 1, ana_check_menu, 0 }, /* menu.c */
  { "CHECKWINDOW", 0, 1, ana_check_window, 0 }, /* xport.c */
#endif
  { "CHI2",	2, 2, ana_chi_square, "*1COMPLEMENT:2LOG" }, /* fun1.c */
  { "CLASSNAME", 1, 1, ana_classname, 0 }, /* install.c */
#if X11
  { "COLORPIXEL", 1, 1, ana_colorpixel, "*" }, /* xport.c */
#endif
#if DEVELOP
  { "COMPILE",	1, 1, ana_compile, 0 }, /* install.c */
#endif
  { "COMPLEX",	1, 1, ana_complex, 0 },	/* fun3.c */
  { "COMPLEXSQUARE", 1, 1, ana_complexsquare, 0 }, /* fun1.c */
  { "COMPRESS",	2, 3, ana_compress, 0 }, /* fun4.c */
  { "CONCAT",	1, MAX_ARG, ana_concat, "1SLOPPY" }, /* subsc.c */
  { "CONJUGATE", 1, 1, ana_conjugate, "*" }, /* fun1.c */
  { "CONSTELLATION", 1, 2, ana_constellation, "1JULIAN:2BESSELIAN:4VOCAL" }, /* astron.c */
  { "CONSTELLATIONNAME", 1, 1, ana_constellationname, 0 }, /* astron.c */
  { "CONVERT",	2, 2, ana_convertsym, "*" }, /* symbols.c */
  { "COS",	1, 1, ana_cos, "*" }, /* fun1.c */
  { "COSH",	1, 1, ana_cosh, "*" }, /* fun1.c */
  { "CROSSCORR", 2, 3, ana_crosscorr, 0 }, /* fun2.c */
  { "CRUNCH",	3, 3, ana_crunch_f, 0 }, /* crunch.c */
  { "CSPLINE",	0, 5, ana_cubic_spline, /* fun3.c */
    "1KEEP:2PERIODIC:4GETDERIVATIVE" },
  { "CSPLINE_FIND", 1, 4, ana_cspline_find, "::AXIS:INDEX" }, /* strous3.c */
  { "CTOP",	1, 3, ana_cartesian_to_polar, 0 }, /* fun4.c */
  { "DATE_FROM_TAI", 1, 2, ana_date_from_tai, 0 }, /* ephem.c */
  { "DBLARR",	1, MAX_DIMS, dblarr, 0 }, /* symbols.c */
  { "DBLFARR",	3, MAX_DIMS + 1, dblfarr, /* filemap.c */
    "%1%OFFSET:1READONLY:2SWAP" },
  { "DEFINED",	1, 1, ana_defined, "+1TARGET" }, /* fun1.c */
  { "DESPIKE",  1, 6, ana_despike, ":FRAC:LEVEL:NITER:SPIKES:RMS" }, /* fun6.c */
  { "DETREND",	1, 2, ana_detrend, "*" }, /* fun2.c */
  { "DIFFER",	1, 3, ana_differ, "*1CENTRAL:2CIRCULAR" }, /* strous.c */
  { "DILATE",	1, 1, ana_dilate, 0 }, /* fun5.c */
  { "DIMEN",	1, 2, ana_dimen, 0 }, /* subsc.c */
  { "DISTARR",	1, 3, ana_distarr, 0 }, /* strous2.c */
  { "DISTR",	2, 2, ana_distr_f, /* strous.c */
    "2IGNORELIMIT:4INCREASELIMIT" },
  { "DMAP",	1, 1, ana_dmap, 0 }, /* subsc.c */
  { "DOUB",	1, 1, ana_double, "*" }, /* symbols.c */
  { "DOUBLE",	1, 1, ana_double, "*" }, /* symbols.c */
  { "DSMOOTH",	3, 3, ana_dir_smooth,
    "0TWOSIDED:0BOXCAR:1ONESIDED:2GAUSSIAN:4TOTAL:8STRAIGHT" }, /* strous3.c */
  { "DSUM",	1, 4, ana_total, "|1|::POWER:WEIGHTS:2KEEPDIMS" }, /* fun1.c */
  { "EASTERDATE", 1, 1, ana_EasterDate, 0 }, /* astron.c */
  { "ENHANCEIMAGE", 1, 3, ana_enhanceimage, ":PART:TARGET:1SYMMETRIC" }, /* strous3.c */
  { "EQUIVALENCE", 2, 2, ana_equivalence, 0 }, /* strous2.c */
  { "ERF",	1, 1, ana_erf, "*" }, /* fun1.c */
  { "ERFC",	1, 1, ana_erfc, "*" }, /* fun1.c */
  { "ERODE",	1, 1, ana_erode, "1ZEROEDGE" }, /* fun5.c */
  { "ESEGMENT",	1, 4, ana_extreme_general, /* topology.c */
    ":SIGN:DIAGONAL:THRESHOLD" },
  { "ESMOOTH",	1, 3, ana_esmooth, 0 }, /* fun2.c */
  { "EVAL",	1, 2, ana_eval, "1ALLNUMBER" }, /* fun3.c */
#if X11
  { "EVENTNAME", 0, 1, ana_event_name, 0 }, /* menu.c */
#endif
  { "EXP",	1, 1, ana_exp, "*" }, /* fun1.c */
  { "EXPAND",	2, 4, ana_expand, "1SMOOTH:2NEAREST" }, /* fun4.c */
  { "EXPM1",	1, 1, ana_expm1, "*" }, /* fun1.c */
  { "EXTRACT_BITS", 3, 3, ana_extract_bits_f, 0 }, /* fun3.c */
  { "FCRUNWRITE", 2, 3, ana_fcrunwrite_f, 0 }, /* files.c */
  { "FCRW",	2, 3, ana_fcrunwrite_f, 0 }, /* files.c */
  { "FCW",	2, 3, ana_fcwrite_f, "1RUNLENGTH" }, /* files.c */
  { "FCWRITE",	2, 3, ana_fcwrite_f, "1RUNLENGTH" }, /* files.c */
  { "FFT",	1, 3, ana_fft_f, "::DIRECTION:1BACK:2ALL" }, /* fun3.c */
  { "FFTSHIFT",	2, 2, ana_fftshift_f, 0 }, /* fun3.c */
  { "FILEPTR",	1, 2, ana_fileptr_f, "1START:2EOF:4ADVANCE" }, /* files.c */
  { "FILESIZE",	1, 1, ana_filesize, 0 }, /* files.c */
  { "FILETYPE",	1, 1, ana_identify_file, 0 }, /* files.c */
  { "FILETYPENAME", 1, 1, ana_filetype_name, 0 }, /* install.c */
  { "FIND",	2, 4, ana_find,	/* strous.c */
    "0EXACT:1INDEX_GE:2VALUE_GE:4FIRST" },
  { "CSPLINE_FIND", 2, 4, ana_cspline_find, ":::AXIS:INDEX" }, /* strous3.c */
  { "FINDFILE",	2, 2, ana_findfile, 0 }, /* files.c */
  { "FIND_MAX",	1, 3, ana_find_max, /* strous2.c */
    "::DIAGONAL:1DEGREE:2SUBGRID" },
  { "FIND_MAXLOC", 1, 3, ana_find_maxloc, /* strous2.c */
    "::DIAGONAL:1DEGREE:2SUBGRID:4COORDS:8OLD" },
  { "FIND_MIN",	1, 3, ana_find_min, /* strous2.c */
    "::DIAGONAL:1DEGREE:2SUBGRID" },
  { "FIND_MINLOC", 1, 3, ana_find_minloc, /* strous2.c */
    "::DIAGONAL:1DEGREE:2SUBGRID:4COORDS" },
  { "FIT",	3, 17, ana_generalfit, /* fit.c */
  "|4|::START:STEP:LOWBOUND:HIGHBOUND:WEIGHTS:QTHRESH:PTHRESH:ITHRESH:DTHRESH:FAC:NITER:NSAME:ERR:FIT:TTHRESH:1VOCAL:4DOWN:8PCHI:16GAUSSIANS:32POWERFUNC:64ONEBYONE" },
#if DEVELOP
  { "FIT2",	4, 10, ana_geneticfit, "X:Y:NPAR:FIT:MU:GENERATIONS:POPULATION:PCROSS:PMUTATE:VOCAL:1ELITE:2BYTE:4WORD:6LONG:8FLOAT:10DOUBLE" }, /* fit.c */
#endif
  { "FITS_HEADER", 1, 4, ana_fits_header_f, 0 }, /* files.c */
  { "FITS_KEY",	2, 2, ana_fitskey, "1COMMENT" }, /* strous3.c */
  { "FITS_READ", 2, 7, ana_fits_read_f, "|1|1TRANSLATE:2RAWVALUES::::::BLANK" }, /* files.c */
  { "FITS_XREAD", 2, 6, ana_fits_xread_f, 0 }, /* files.c */
  { "FIX",	1, 1, ana_long, "*" }, /* symbols.c */
  { "FLOAT",	1, 1, ana_float, "*" }, /* symbols.c */
  { "FLOOR",	1, 1, ana_floor, "*" }, /* symbols.c */
  { "FLTARR",	1, MAX_DIMS, fltarr, 0 }, /* symbols.c */
  { "FLTFARR",	3, MAX_DIMS + 1, fltfarr, /* filemap.c */
    "%1%OFFSET:1READONLY:2SWAP" },
  { "FMAP",	1, 1, ana_fmap, 0 }, /* subsc.c */
  { "FRATIO",	3, 3, ana_f_ratio, "*1COMPLEMENT:2LOG" }, /* fun1.c */
  { "FREADF",	2, MAX_ARG, ana_freadf_f, "|1|1EOF" }, /* files.c */
  { "FREADS",	3, MAX_ARG, ana_freads_f, "1COUNTSPACES" }, /* files.c */
  { "FSTRING",	1, MAX_ARG, ana_fstring, "1SKIP_UNDEFINED" }, /* fun2.c */
  { "FSUM",	1, 4, ana_total, "::POWER:WEIGHTS:1DOUBLE:2KEEPDIMS" }, /* fun1.c */
  { "FZARR",	1, 1, ana_fzarr, "1READONLY" }, /* filemap.c */
  { "FZH",	1, 2, ana_fzhead_f, 0 }, /* files.c */
  { "FZHEAD",	1, 2, ana_fzhead_f, 0 }, /* files.c */
  { "FZR",	2, 3, ana_fzread_f, "|1|1PRINTHEADER" }, /* files.c */
  { "FZREAD",	2, 3, ana_fzread_f, "|1|1PRINTHEADER" }, /* files.c */
  { "FZW",	2, 3, ana_fzwrite_f, 0 }, /* files.c */
  { "FZWRITE",	2, 3, ana_fzwrite_f, 0 }, /* files.c */
  { "GAMMA",	1, 1, ana_gamma, "*1LOG" }, /* fun1.c */
#if HAVE_REGEX_H
  { "GETDIRECTORIES", 1, 2, ana_getdirectories, 0 }, /* files.c */
  { "GETDIRECTS", 1, 2, ana_getdirectories, 0 }, /* files.c */
#endif
  { "GETENV",	1, 1, ana_getenv, 0 }, /* files.c */
#if HAVE_REGEX_H
  { "GETFILES", 1, 2, ana_getfiles, 0 }, /* files.c */
  { "GETFILES_R", 1, 2, ana_getfiles_r, 0 }, /* files.c */
  { "GETMATCHEDFILES", 2, 3, ana_getmatchedfiles, 0 }, /* files.c */
  { "GETMATCHEDFILES_R", 2, 3, ana_getmatchedfiles_r, 0 }, /* files.c */
#endif
  { "GET_LUN",	0, 0, ana_get_lun, 0 }, /* files.c */
  { "GIFREAD",	2, 3, ana_gifread_f, 0 }, /* gifread_ana.c */
  { "GIFWRITE",	2, 3, ana_gifwrite_f, 0 }, /* gifwrite_ana.c */
  { "GRIDMATCH", 7, 8, ana_gridmatch, "1VOCAL" }, /* fun4.c */
  { "GSMOOTH",	1, 4, ana_gsmooth, /* fun2.c */
    ":::KERNEL:1NORMALIZE:2FULLNORM:4BALANCED:8ALL" },
  { "HAMMING",  1, 2, ana_hamming, 0 }, /* strous3.c */
  { "HILBERT",	1, 3, ana_hilbert, /* fun3.c */
    "::DIRECTION:1BACK:2ALL:4AVGKEEP:8HIGHKEEP" },
  { "HIST",	1, 2, ana_hist, /* fun3.c */
    "1FIRST:2IGNORELIMIT:4INCREASELIMIT:8SILENT" },
  { "HISTR",	1, 1, ana_histr, /* fun3.c */
    "1FIRST:2IGNORELIMIT:4INCREASELIMIT:8SILENT" },
  { "IBETA",	3, 3, ana_incomplete_beta, "*1COMPLEMENT:2LOG" }, /* fun1.c */
  { "IDLREAD",	2, 2, ana_idlread_f, 0 }, /* strous3.c */
  { "IGAMMA",	2, 2, ana_incomplete_gamma, "*1COMPLEMENT:2LOG" }, /* fun1.c */
  { "IMAGINARY", 1, 1, ana_imaginary, 0 }, /* fun3.c */
  { "INDEX",	1, 1, ana_index, "*1AXIS:2RANK" }, /* fun4.c */
  { "INDGEN",	1, 2, ana_indgen, "*" }, /* fun1.c */
  { "INPOLYGON", 4, 4, ana_inpolygon, 0 }, /* topology.c */
  { "INT",	1, 1, ana_word, "*" }, /* symbols.c */
  { "INTARR",	1, MAX_DIMS, intarr, 0 }, /* symbols.c */
  { "INTFARR",	3, MAX_DIMS + 1, intfarr, /* filemap.c */
    "%1%OFFSET:1READONLY:2SWAP" },
  { "ISARRAY",	1, 1, ana_isarray, 0 }, /* subsc.c */
  { "ISNAN",	1, 1, ana_isnan, 0 }, /* fun1.c; needs IEEE isnan! */
  { "ISSCALAR",	1, 1, ana_isscalar, 0 }, /* subsc.c */
  { "ISSTRING",	1, 1, ana_isstring, 0 }, /* subsc.c */
  { "IST",	1, 3, ana_istring, 0 }, /* fun2.c */
  { "ISTRING",	1, 3, ana_istring, 0 }, /* fun2.c */
#if HAVE_LIBJPEG
  { "JPEGREAD",	2, 4, ana_read_jpeg6b_f, ":::SHRINK:1GREYSCALE" }, /* jpeg.c */
  { "JPEGWRITE", 2, 4, ana_write_jpeg6b_f, 0 }, /* jpeg.c */
#endif
  { "KSMOOTH",	2, 3, ana_ksmooth, "1BALANCED" }, /* fun2.c */
  { "LAPLACE2D",	1, 1, ana_laplace2d, 0 }, /* poisson.c */
  { "LEGENDRE", 2, 2, ana_legendre, "1NORMALIZE" }, /* strous3.c */
  { "LLSQ",	2, 9, ana_lsq2,	/* strous2.c */
    ":::FWHM:WEIGHTS:COV:ERR:CHISQ:1FORMAL:2REDUCE" },
  { "LMAP",	1, 1, ana_lmap, 0 }, /* subsc.c */
  { "LOCAL_MAX", 2, 2, ana_local_maxf, /* strous.c */
    "3SUBGRID:2BOUND" },
  { "LOCAL_MAXLOC", 2, 2, ana_local_maxloc, /* strous.c */
    "3SUBGRID:2BOUND:4RELATIVE" },
  { "LOCAL_MIN", 2, 2, ana_local_minf, /* strous.c */
    "3SUBGRID:2BOUND" },
  { "LOCAL_MINLOC", 2, 2, ana_local_minloc, /* strous.c */
    "3SUBGRID:2BOUND:4RELATIVE" },
  { "LOG",	1, 1, ana_log, "*" }, /* fun1.c */
  { "LOG10",	1, 1, ana_log10, "*" }, /* fun1.c */
  { "LOG1P",	1, 1, ana_log1p, "*" }, /* fun1.c */
  { "LONARR",	1, MAX_DIMS, lonarr, 0 }, /* symbols.c */
  { "LONFARR",	3, MAX_DIMS+1, lonfarr, /* filemap.c */
    "%1%OFFSET:1READONLY:2SWAP" },
  { "LONG",	1, 1, ana_long, "*" }, /* symbols.c */
  { "LOWCASE",	1, 1, ana_lower, 0 }, /* fun2.c */
  { "LOWER",	1, 1, ana_lower, 0 }, /* fun2.c */
  { "LSMOOTH",	3, 3, ana_dir_smooth2, /* strous3.c */
    "0TWOSIDED:0BOXCAR:1ONESIDED:2GAUSSIAN:4NORMALIZE:8STRAIGHT" },
  { "LSQ",	2, 6, ana_lsq,	/* strous2.c */
    "::WEIGHTS:COV:ERR:CHISQ:1FORMAL:2REDUCE" },
  { "MATCH",	2, 2, ana_match, 0 }, /* strous.c */
  { "MATMUL",	2, 2, ana_matmul, 0 }, /* fun2.c */
  { "MAX",	1, 2, ana_maxf, "1KEEPDIMS" }, /* fun3.c */
  { "MAXDIR",	2, 3, ana_max_dir, 0 },	/* topology.c */
  { "MAXFILTER", 1, 3, ana_maxfilter, 0 }, /* strous2.c */
  { "MAXLOC",	1, 2, ana_maxloc, "1KEEPDIMS" }, /* fun3.c */
  { "MEAN", 	1, 4, ana_mean, "::POWER:WEIGHTS:1DOUBLE:2KEEPDIMS:4FLOAT" }, /* fun1.c */
  { "MEDFILTER", 1, 4, ana_medfilter, "%1%" }, /* strous2.c */
  { "MEDIAN",	1, 3, ana_median, "%1%" }, /* strous2.c */
  { "MEMORY",	0, 0, ana_memory, 0 }, /* memck.c */
  { "MIN",	1, 2, ana_minf, "1KEEPDIMS" }, /* fun3.c */
  { "MINFILTER", 1, 3, ana_minfilter, 0 }, /* strous2.c */
  { "MINLOC",	1, 2, ana_minloc, "1KEEPDIMS" }, /* fun3.c */
  { "NCCHI2",	3, 3, ana_noncentral_chi_square, 0 }, /* fun1.c */
  { "NOT",	1, 1, ana_not, "*" }, /* strous.c */
  { "NUM_DIM",	1, 1, ana_num_dimen, 0 }, /* subsc.c */
  { "NUM_ELEM", 1, 2, ana_num_elem, 0 }, /* subsc.c */
  { "ONE",	1, 1, ana_onef, 0 }, /* fun1.c */
  { "OPENR",	2, 2, ana_openr_f, "1GET_LUN" }, /* files.c */
  { "OPENU",	2, 2, ana_openu_f, "1GET_LUN" }, /* files.c */
  { "OPENW",	2, 2, ana_openw_f, "1GET_LUN" }, /* files.c */
  /* { "ORBITELEM",	3, 3, ana_orbitalElement, 0, */
  { "ORDFILTER", 1, 4, ana_orderfilter, /* strous2.c */
    "%1%ORDER:1MEDIAN:2MINIMUM:3MAXIMUM" },
  { "PIT",	1, 3, ana_pit, 0 }, /* fun2.c */
  { "POLATE",	3, 3, ana_table, 0 }, /* strous.c */
  { "POLY",	2, 2, ana_poly, "*" }, /* fun2.c */
  { "POWER",	1, 2, ana_power, "1POWER:2SHAPE:4ONEDIM" }, /* fun3.c */
  { "PRECESS",	3, 3, ana_precess, "1JULIAN:2BESSELIAN" }, /* astron.c */
  { "PRINTF",	1, MAX_ARG, ana_printf_f, 0 }, /* files.c */
#if DEVELOP
  { "PROJECT",	1, 1, ana_project, 0 }, /* projection.c */
  { "PROJECTMAP", 2, 8, ana_projectmap, "::HDIST:ANGLE:MAG:XMAP:YMAP:SIZE" }, /* projection.c */
#endif
  { "PSUM",	2, 4, ana_psum, /* strous2.c */
    "1ONEDIM:2VNORMALIZE:4CNORMALIZE:8SINGLE" },
  { "PTOC",	1, 5, ana_polar_to_cartesian, 0 }, /* fun4.c */
  { "QUANTILE",	2, 3, ana_quantile, 0 }, /* strous2.c */
  { "RANDOM",	1, MAX_DIMS, ana_random, /* random.c */
    "%2%SEED:PERIOD:1UNIFORM:2NORMAL:3SAMPLE:4SHUFFLE:5BITS" },
  { "RANDOMB",	3, MAX_DIMS, ana_randomb, "%2%SEED:1LONG" }, /* random.c */
  { "RANDOMD",  3, MAX_DIMS, ana_randomd, "%1%SEED" }, /* random.c */
  { "RANDOME",	3, MAX_DIMS, ana_randome, "%2%SEED:1DOUBLE" }, /* random.c */
  { "RANDOMN",	3, MAX_DIMS, ana_randomn, "%2%SEED" }, /* random.c */
  { "RANDOMU",	3, MAX_DIMS, ana_randomu, "%2%SEED:PERIOD" }, /* random.c */
  { "READF",	2, MAX_ARG, ana_readf_f, "1ASKMORE:2WORD" }, /* files.c */
  { "READU",	2, MAX_ARG, ana_readu_f, 0 }, /* files.c */
#if HAVE_LIBJPEG
  { "READ_JPEG", 2, 4, ana_read_jpeg6b_f, ":::SHRINK:1GREYSCALE" }, /* jpeg.c */
#endif
  { "REAL",	1, 1, ana_real, 0 }, /* fun3.c */
  { "REDIM",	2, 9, ana_redim_f, 0 }, /* subsc.c */
#if HAVE_REGEX_H
  { "REGEX",	1, 2, ana_regex, "1CASE" }, /* regex.c */
#endif
  { "REGRID",	5, 5, ana_regrid, 0 }, /* fun4.c */
  { "REGRID3",	5, 5, ana_regrid3, 0 }, /* fun4.c */
  { "REGRID3NS", 5, 5, ana_regrid3ns, 0 }, /* fun4.c */
#if DEVELOP
  { "REGRIDLS", 5, 5, ana_regridls, 0 }, /* fun4.c */
#endif
  { "REORDER",	2, 2, ana_reorder, 0 },	/* fun6.c */
  { "RESTORE",	2, 3, ana_fzread_f, "1PRINTHEADER" }, /* files.c */
  { "REVERSE",	1, MAX_ARG, ana_reverse, "1ZERO" }, /* subsc.c */
  { "RFIX",	1, 1, ana_rfix, "*" }, /* symbols.c */
  { "ROLL",	2, 2, ana_roll, 0 }, /* subsc.c */
  { "ROOT3",	3, 3, ana_root3, 0 }, /* orientation.c */
  { "RUNCUM",	1, 3, ana_runcum, "*1PARTIAL_WIDTH:2VARSUM" }, /* strous.c */
  { "RUNPROD",	1, 2, ana_runprod, "*" }, /* strous2.c */
  { "RUNSUM",	1, 3, ana_runsum, "*" }, /* fun2.c */
  { "SCALE",	1, 3, ana_scale, "*1FULLRANGE:2ZOOM" }, /* fun3.c */
  { "SCALERANGE", 3, 5, ana_scalerange, "*1FULLRANGE:2ZOOM" }, /* fun3.c */
  { "SCANF",	2, MAX_ARG, ana_freadf_f, "|1|1EOF" }, /* files.c */
  { "SCCOMPLEX", 1, 1, ana_sccomplex, "1TOCOMPLEX:2TOSC" }, /* fun3.c */
  { "SDEV",	1, 3, ana_sdev, /* fun2.c */
    "::WEIGHTS:*0SAMPLE:1POPULATION:2KEEPDIMS:4DOUBLE" },
  { "SEGMENT",	1, 3, ana_segment, ":SIGN:DIAGONAL:1DEGREE" }, /* topology.c */
  { "SEGMENTDIR", 2, 3, ana_segment_dir, "::SIGN" }, /* topology.c */
  { "SGN",	1, 1, ana_sgn, "*" }, /* fun1.c */
  { "SHIFT",	1, 4, ana_shift_f, ":::BLANK:1TRANSLATE" }, /* strous2.c */
  { "SHIFT3",	2, 3, ana_shift3, 0 }, /* fun4.c */
  { "SIDEREALTIME", 1, 1, ana_siderealtime, "1ATZEROTIME" }, /* astron.c */
  { "SIEVE",	1, 2, ana_sieve, 0 }, /* fun3.c */
  { "SIN", 	1, 1, ana_sin, "*" }, /* fun1.c */
  { "SINH",	1, 1, ana_sinh, "*" }, /* fun1.c */
  { "SKIPC",	2, 2, ana_skipc, 0 }, /* fun2.c */
  { "SMAP",	1, 1, ana_smap, "1TRUNCATE:2ARRAY" }, /* subsc.c */
  { "SMOOTH",	1, 3, ana_smooth, "1PARTIAL_WIDTH:2VARSMOOTH:4ALL" }, /* strous.c */
  { "SOLAR_B",	1, 1, ana_solar_b, "*2MODERN" }, /* ephem2.c */
  { "SOLAR_L",	1, 1, ana_solar_l, "*1FULL:2MODERN" }, /* ephem2.c */
  { "SOLAR_P",	1, 1, ana_solar_p, "*2MODERN" }, /* ephem2.c */
  { "SOLAR_R",	1, 1, ana_solar_r, "*" }, /* ephem2.c */
  { "SORT",	1, 1, ana_sort, "*1HEAP:2SHELL:4AXIS" }, /* fun4.c */
  { "SPAWN",	1, 1, ana_spawn_f, 0 }, /* files.c */
  { "SPRINTF",	1, MAX_ARG, ana_fstring, "1SKIP_UNDEFINED" }, /* fun2.c */
  { "SQRT",	1, 1, ana_sqrt, "*" }, /* fun1.c */
  { "SSCANF",	3, MAX_ARG, ana_freads_f, "1COUNTSPACES" }, /* files.c */
  { "STORE",	2, 3, ana_fzwrite_f, "1SAFE" }, /* files.c */
  { "STR",	1, MAX_ARG, ana_string, 0 }, /* fun2.c */
  { "STRARR",	2, 1 + MAX_DIMS, strarr, "%1%SIZE" }, /* symbols.c */
  { "STRCOUNT",	2, 2, ana_strcount, 0 }, /* fun2.c */
  { "STRETCH",	2, 2, ana_stretch, 0 }, /* fun4.c */
  { "STRING",	1, MAX_ARG, ana_string, 0 }, /* fun2.c */
  { "STRLEN",	1, 1, ana_strlen, 0 }, /* fun2.c */
  { "STRLOC",	2, 2, ana_strloc, 0 }, /* fun2.c */
  { "STRPBRK",	2, 2, ana_strpbrk, 0 },	/* fun2.c */
  { "STRPOS",	2, 3, ana_strpos, 0 }, /* fun2.c */
  { "STRR",	3, 4, ana_strreplace, 0 }, /* fun2.c */
  { "STRREPLACE", 3, 4, ana_strreplace, 0 }, /* fun2.c */
  { "STRSKP",	2, 2, ana_strskp, 0 }, /* fun2.c */
  { "STRSUB",	3, 3, ana_strsub, 0 }, /* fun2.c */
  { "STRTOK",	1, 2, ana_strtok, "1SKIP_FINAL_DELIM" }, /* fun2.c */
  { "STRTOL",	1, 2, ana_strtol, 0 }, /* fun3.c */
  { "STRTRIM",	1, 1, ana_strtrim, 0 }, /* fun2.c */
#if DEVELOP
  { "STRUCT",	2, MAX_ARG, ana_struct, 0 }, /* install.c */
#endif
  { "STUDENT",	2, 2, ana_student, "*1COMPLEMENT:2LOG" }, /* fun1.c */
  { "SUN_B",	2, 2, ana_sun_b, 0 }, /* ephem.c */
  { "SUN_D",	2, 2, ana_sun_d, 0 }, /* ephem.c */
  { "SUN_P",	2, 2, ana_sun_p, 0 }, /* ephem.c */
  { "SUN_R",	2, 2, ana_sun_r, 0 }, /* ephem.c */
  { "SYMBOL",	1, 1, ana_symbol, "1MAIN" }, /* symbols.c */
  { "SYMCLASS",	1, 1, ana_symclass, "+|1|1FOLLOW:2NUMBER" }, /* subsc.c */
  { "SYMDTYPE",	1, 1, ana_symdtype, 0 }, /* subsc.c */
  { "SYMMEM",	0, 0, ana_symbol_memory, 0 }, /* install.c */
  { "SYMNUM",	1, 1, ana_symbol_number, 0 }, /* install.c */
  { "TABLE",	3, 4, ana_table, "|2|1ALL:2MIDDLE" }, /* strous.c */
  { "TAI_FROM_DATE", 5, 5, ana_tai_from_date, 0 }, /* ephem.c */
  { "TAN", 	1, 1, ana_tan, "*" }, /* fun1.c */
  { "TANH", 	1, 1, ana_tanh, "*" }, /* fun1.c */
  { "TEMPORARY", 1, 1, ana_temp, 0 }, /* strous2.c */
  { "TENSE",	3, 6, ana_tense, 0 }, /* fun3.c */
  { "TENSE_CURVE", 3, 6, ana_tense_curve, 0 }, /* fun3.c */
  { "TENSE_LOOP", 3, 4, ana_tense_loop, 0 }, /* fun3.c */
#if DEVELOP
  { "TEST",	2, 3, ana_test, 0 }, /* execute.c */
#endif
  { "TOTAL", 	1, 4, ana_total, "::POWER:WEIGHTS:1DOUBLE:2KEEPDIMS:4FLOAT" }, /* fun1.c */
  { "TRACE_DECODER", 3, 3, ana_trace_decoder, 0 }, /* trace_decoder_ana.c */
  { "TREND",	1, 2, ana_trend, "*" }, /* fun2.c */
  { "TRI_NAME_FROM_TAI", 1, 1, ana_tri_name_from_tai, 0 }, /* ephem.c */
#if X11
  { "TVREAD",	1, 5, ana_xtvread, "1GREYSCALE" }, /* xport.c */
#endif
  { "TYPENAME",	1, 1, ana_typeName, 0 }, /* install.c */
  { "UPCASE",	1, 1, ana_upper, 0 }, /* fun2.c */
  { "UPPER",	1, 1, ana_upper, 0 }, /* fun2.c */
  { "VARIANCE", 1, 3, ana_variance, /* fun2.c */
    "::WEIGHTS:*0SAMPLE:1POPULATION:2KEEPDIMS:4DOUBLE" },
  { "VARNAME",	1, 1, ana_varname, 0 }, /* symbols.c */
  { "VOIGT",	2, 2, ana_voigt, "*" }, /* fun1.c */
  { "WMAP",	1, 1, ana_wmap, 0 }, /* subsc.c */
  { "WORD",	1, 1, ana_word, "*" }, /* symbols.c */
  { "WRITEU",	2, MAX_ARG, ana_writeu, 0 }, /* files.c */
#if HAVE_LIBJPEG
  { "WRITE_JPEG", 2, 4, ana_write_jpeg6b_f, 0 }, /* jpeg.c */
#endif
#if X11
  { "XEXIST",	1, 1, ana_xexist, 0 }, /* xport.c */
  { "XLABELWIDTH", 1, 1, ana_xlabelwidth, 0 }, /* xport.c */
#if MOTIF
  { "XMADDFILETOLIST", 2, 2, ana_xmaddfiletolist, 0 }, /* motif.c */
  { "XMARROW",	3, 4, ana_xmarrow, 0 }, /* motif.c */
  { "XMBOARD",	1, 5, ana_xmboard, 0 }, /* motif.c */
  { "XMBUTTON",	3, 5, ana_xmbutton, 0 }, /* motif.c */
  { "XMCHECKBOX", 5, 32, ana_xmcheckbox, 0 }, /* motif.c */
  { "XMCOLUMNS", 3, 5, ana_xmcolumns, 0 }, /* motif.c */
  { "XMCOMMAND", 3, 7, ana_xmcommand, 0 }, /* motif.c */
  { "XMDIALOG_BOARD", 4, 9, ana_xmdialog_board, 0 }, /* motif.c */
  { "XMDIALOG_FORM", 4, 8, ana_xmdialog_form, 0 }, /* motif.c */
  { "XMDRAWINGAREA", 2, 7, ana_xmdrawingarea, 0 }, /* motif.c */
  { "XMFILEGETLIST",	1, 1, ana_xmfilegetlist, 0 }, /* motif.c */
  { "XMFILESELECT", 3, 12, ana_xmfileselect, 0 }, /* motif.c */
  { "XMFORM",	1, 7, ana_xmform, 0 }, /* motif.c */
  { "XMFRAME",	1, 5, ana_xmframe, 0 }, /* motif.c */
  { "XMGETOPTIONSELECTION", 1, 1, ana_xmgetoptionselection, 0 }, /* motif.c */
  { "XMGETWIDGETADDRESS", 1, 1, ana_xmgetwidgetaddress, 0 }, /* motif.c */
  { "XMHSCALE",	5, 8, ana_xmhscale, 0 }, /* motif.c */
  { "XMHSCROLLBAR",	5, 6, ana_xmhscrollbar, 0 }, /* motif.c */
  { "XMLABEL",	2, 5, ana_xmlabel, 0 }, /* motif.c */
  { "XMLIST",	3, 6, ana_xmlist, 0 }, /* motif.c */
  { "XMLISTCOUNT", 1, 1, ana_xmlistcount, 0 }, /* motif.c */
  { "XMLISTFROMFILE", 4, 6, ana_xmlistfromfile, 0 }, /* motif.c */
  { "XMMENUBAR", 4, 32, ana_xmmenubar, 0 }, /* motif.c */
  { "XMOPTIONMENU", 6, 50, ana_xmoptionmenu, 0 }, /* motif.c */
  { "XMPIXMAPBUTTON", 3, 3, ana_xmpixmapbutton, 0 }, /* motif.c */
  { "XMPIXMAPOPTIONMENU", 6, 32, ana_xmpixmapoptionmenu, 0 }, /* motif.c */
  { "XMPULLDOWNMENU", 6, 32, ana_xmpulldownmenu, 0 }, /* motif.c */
  { "XMRADIOBOX", 5, 32, ana_xmradiobox, 0 }, /* motif.c */
  { "XMROWS",	3, 5, ana_xmrows, 0 }, /* motif.c */
  { "XMSCALEGETVALUE", 1, 1, ana_xmscalegetvalue, 0 }, /* motif.c */
  { "XMSCROLLEDWINDOW", 3, 3, ana_xmscrolledwindow, 0 }, /* motif.c */
  { "XMSCROLLEDWINDOWAPP", 1, 4, ana_xmscrolledwindowapp, 0 }, /* motif.c */
  { "XMSEPARATOR", 1, 4, ana_xmseparator, 0 }, /* motif.c */
  { "XMTEXT",	1, 5, ana_xmtext, 0 }, /* motif.c */
  { "XMTEXTBOX", 1, 5, ana_xmtextbox, 0 }, /* motif.c */
  { "XMTEXTFIELD", 4, 8, ana_xmtextfield, 0 }, /* motif.c */
  { "XMTEXTFIELDARRAY", 9, 40, ana_xmtextfieldarray, 0 }, /* motif.c */
  { "XMTEXTFIELDGETSTRING", 1, 1, ana_xmtextfieldgetstring, 0 }, /* motif.c */
  { "XMTEXTFROMFILE", 2, 6, ana_xmtextfromfile, 0 }, /* motif.c */
  { "XMTEXTGETINSERTPOSITION", 1, 1, ana_xmtextgetinsertposition, 0 }, /* motif.c */
  { "XMTEXTGETLASTPOSITION",	1, 1, ana_xmtextgetlastposition, 0 }, /* motif.c */
  { "XMTEXTGETSELECTION",	1, 3, ana_xmtextgetselection, 0 }, /* motif.c */
  { "XMTEXTGETSTRING", 1, 1, ana_xmtextgetstring, 0 }, /* motif.c */
  { "XMTOGGLEGETSTATE", 1, 1, ana_xmtogglegetstate, 0 }, /* motif.c */
  { "XMTOPLEVEL_BOARD", 3, 5, ana_xmtoplevel_board, 0 }, /* motif.c */
  { "XMTOPLEVEL_FORM", 3, 7, ana_xmtoplevel_form, 0 }, /* motif.c */
  { "XMVSCALE",	5, 8, ana_xmvscale, 0 }, /* motif.c */
  { "XMVSCROLLBAR",	5, 6, ana_xmvscrollbar, 0 }, /* motif.c */
#endif
  { "XQUERY",	0, 1, ana_xquery_f, 0 }, /* xport.c */
#if MOTIF
  { "XTPARENT",	1, 1, ana_xtparent, 0 }, /* motif.c */
#endif
  { "XTVREAD",	1, 5, ana_xtvread, "1GREYSCALE" }, /* xport.c */
#if MOTIF
  { "XTWINDOW",	1, 1, ana_xtwindow, 0 }, /* motif.c */
#endif
#endif
  { "ZERO", 	1, 1, ana_zerof, "*" }, /* fun1.c */
  { "ZERONANS",	1, 2, ana_zapnan_f, "*%1%VALUE" }, /* fun1.c */
  { "ZINV",	1, 1, ana_zinv, "*" }, /* strous.c */
};
int	nFunction = sizeof(function)/sizeof(internalRoutine);
/*----------------------------------------------------------------*/
void undefine(int symbol)
/* free up memory allocated for <symbol> and make it undefined */
{
  void	zap(int), updateIndices(void);
  char	hasMem = 0, **key, *temp;
  int	n, k, oldZapContext, i;
  word	*ptr;
  pointer	p2;
  listElem	*p;
  extractSec	*eptr, *eptr0;
  extern int	tempSym;
  extern char	restart;

  if (symbol < 0 || symbol > NSYM)
    cerror(ILL_SYM, 0, symbol, "undefine");
  if (symbol < nFixed && !restart) {
    anaerror("Constant of nature cannot be deleted!\n", symbol);
    return;
  }
  switch (symbol_class(symbol)) {
    case ANA_SCALAR: case ANA_POINTER: case ANA_TRANSFER:
    case ANA_UNDEFINED: case ANA_UNUSED: case ANA_FUNC_PTR:
    case ANA_DEFERRED_SUBR: case ANA_DEFERRED_FUNC: case ANA_DEFERRED_BLOCK:
      break;
    case ANA_SCAL_PTR:
      if (symbol_type(symbol) == ANA_TEMP_STRING) {
	temp = scal_ptr_pointer(symbol).s;
	free(scal_ptr_pointer(symbol).s);
      }
      break;
    case ANA_ARRAY:
      if (isStringType(array_type(symbol))) { /* a string array */
	/* must free the components' memory */
	p2.sp = array_data(symbol);
	n = array_size(symbol);
	while (n--)
	  free(*p2.sp++);
      }
      /* fall through to generic case to take care of remaining */
      /* array memory. */
    case ANA_STRING: case ANA_SUBSC_PTR: case ANA_FILEMAP:
    case ANA_ASSOC: case ANA_CSCALAR: case ANA_CARRAY: case ANA_STRUCT:
      hasMem = 1;
      break;
    case ANA_META:
      if (symbol_context(k = meta_target(symbol)) == symbol
	  || (zapContext > 0 && symbol_context(k) == zapContext))
	zap(k);
      break;
    case ANA_RANGE: case ANA_PRE_RANGE:
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
    case ANA_LIST_PTR:
      n = list_ptr_target(symbol);
      if (n > 0) 		/* string key */
	free(list_ptr_tag_string(symbol));
      hasMem = 0;
      break;
    case ANA_LIST: case ANA_PRE_LIST:
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
    case ANA_CLIST: case ANA_PRE_CLIST: case ANA_CPLIST:
      ptr = clist_symbols(symbol);
      n = clist_num_symbols(symbol);
      while (n--)
	if (symbol_context(k = *ptr++) == symbol
	    || (zapContext > 0 && symbol_context(k) == zapContext))
	  zap(k);
      hasMem = 1;
      break;
    case ANA_KEYWORD:
      if (symbol_context(k = keyword_name_symbol(symbol)) == symbol
	  || (zapContext > 0 && symbol_context(k) == zapContext))
	zap(k);
      if ((symbol_context(k = keyword_value(symbol)) == symbol
	   || (zapContext > 0 && symbol_context(k) == zapContext))
	  && k > tempSym)
	zap(k);
      break;
    case ANA_SUBROUTINE: case ANA_FUNCTION: case ANA_BLOCKROUTINE:
      oldZapContext = zapContext;
      zapContext = symbol;
      ptr = routine_parameters(symbol);
      n = routine_num_parameters(symbol);
      key = routine_parameter_names(symbol);
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
    case ANA_EXTRACT: case ANA_PRE_EXTRACT:
      if (symbol_class(symbol) == ANA_EXTRACT) {
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
	    case ANA_RANGE:
	      p2.w = eptr->ptr.w;
	      while (i--) {
		if (symbol_context(*p2.w) == symbol
		    || (zapContext > 0 && symbol_context(*p2.w) == zapContext))
		  zap(*p2.w);
		p2.w++;
	      }
	      free(eptr->ptr.w);
	      break;
	    case ANA_LIST:
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
    case ANA_EVB: case ANA_INT_FUNC: case ANA_USR_FUNC:
      n = 0;
      k = symbol_class(symbol);
      if (k == ANA_EVB)
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
	  if (symbol_class(usr_sub_routine_num(symbol)) == ANA_STRING)
	    /* the subroutine has not yet been sought; it's name is
	       stored in the usr_sub_routine_num symbol. */
	    zap(usr_sub_routine_num(symbol));
	  /* fall through to the below case */
	case EVB_INT_SUB: case EVB_INSERT: case ANA_INT_FUNC: case
	ANA_USR_FUNC: case EVB_CASE: case EVB_NCASE: case EVB_BLOCK:
	  n = symbol_memory(symbol)/sizeof(word);
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
    case ANA_BIN_OP: case ANA_IF_OP:
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
  symbol_class(symbol) = ANA_UNDEFINED;
  undefined_par(symbol) = 0;	/* used in usr_routine() (with value 1) */
				/* to indicate unspecified parameters */
				/* of user-defined routines. */
/* context must remain the same, or local variables become global */
/* when they are undefined (e.g. as lhs in a replacement) */
}
/*----------------------------------------------------------------*/
void zap(int nsym)
/* undefine & remove name (if any) */
{
 char	*name, *noName = "[]";
 int	context, hashValue;
 hashTableEntry	*hp, *oldHp, **hashTable;
#if DEBUG
 void	checkTemps(void);
#endif

 if (nsym < 0 || nsym > NSYM)
   cerror(ILL_SYM, 0, nsym, "zap");
 if (!nsym)
   return;
 if (symbol_class(nsym) == ANA_UNUSED)
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
     case ANA_SUBROUTINE: case ANA_DEFERRED_SUBR:
       hashTable = subrHashTable;
       break;
     case ANA_FUNCTION: case ANA_DEFERRED_FUNC:
       hashTable = funcHashTable;
       break;
     case ANA_BLOCKROUTINE: case ANA_DEFERRED_BLOCK:
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
     anaerror("Symbol name not found in tables??", nsym);
 }
 undefine(nsym);
 symbol_class(nsym) = ANA_UNUSED;
 symbol_context(nsym) = 0;
 if (nsym >= EXE_START && nsym < EXE_END) {
   nExecutable--;
   if (nsym < executableIndex)
     executableIndex = nsym;
   while (executableIndex > EXE_START
	  && symbol_class(executableIndex - 1) == ANA_UNUSED)
     executableIndex--;
 } else if (nsym >= NAMED_START && nsym < NAMED_END) {
   nNamedVariable--;
   if (nsym < nNamedVariable)
     namedVariableIndex = nsym;
   while (namedVariableIndex > NAMED_START
	  && symbol_class(namedVariableIndex - 1) == ANA_UNUSED)
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
void cleanUp(int context, int which)
/* names in symbolStack are supposed to be removed after use by the */
/* routines that use them.  only resetting of the index is done here. */
/* all temporary variables that have the specified context are removed */ 
/* <which> can be:  CLEANUP_VARS  CLEANUP_EDBS  CLEANUP_ALL */
/* CLEANUP_COMP CLEANUP_ERROR */
{
  char	comp;
  int	i;
  void	zapParseTemps(void);

  comp = which & CLEANUP_COMP;
/*  while (symbolStackIndex > 0 && !symbolStack[symbolStackIndex])
    symbolStackIndex--; */
  if (context > 0)
  { if (which & CLEANUP_VARS)
    { for (i = TEMPS_START; i < tempVariableIndex; i++)
	if (symbol_class(i) != ANA_UNUSED
	    && comp? symbol_context(i) <= 0: symbol_context(i) == context)
	  zap(i);
      while (tempVariableIndex > TEMPS_START
	     && symbol_class(tempVariableIndex - 1) == ANA_UNUSED)
	tempVariableIndex--; }
    if (which & CLEANUP_EDBS)
    { for (i = curTEIndex; i < tempExecutableIndex; i++)
	if (symbol_class(i) != ANA_UNUSED
	    && comp? symbol_context(i) <= 0: symbol_context(i) == context)
	  zap(i);
      while (tempExecutableIndex > TEMP_EXE_START
	     && symbol_class(tempExecutableIndex - 1) == ANA_UNUSED)
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
void cleanUpRoutine(int context, char keepBase)
/* completely removes all traces of the routine with the given context */
/* keeps the ANA_SUBROUTINE, ANA_FUNCTION, or ANA_BLOCKROUTINE symbol */
/* itself if keepBase is unequal to zero.  If keepBase is unequal to
 zero, then an ANA_DEFERRED_SUBR is transformed into an (empty)
 ANA_SUBR, ANA_DEFERRED_FUNC into an ANA_FUNC, and ANA_DEFERRED_BLOCK
 into an ANA_BLOCK.  LS 19feb97 21may99 */
{
  char	mem;
  int	n;
  word	*ptr;

  if (context < nFixed || context >= NAMED_END) {
    anaerror("Illegal routine or function specified", context);
    return;
  }
  if (keepBase) {
    switch (symbol_class(context)) {
      case ANA_DEFERRED_SUBR:
	symbol_class(context) = ANA_SUBROUTINE;
	routine_num_parameters(context) = 0;
	routine_num_statements(context) = 0;
	break;
      case ANA_DEFERRED_FUNC:
	symbol_class(context) = ANA_FUNCTION;
	routine_num_parameters(context) = 0;
	routine_num_statements(context) = 0;
	break;
      case ANA_DEFERRED_BLOCK:
	symbol_class(context) = ANA_BLOCKROUTINE;
	routine_num_parameters(context) = 0;
	routine_num_statements(context) = 0;
	break;	
      case ANA_SUBROUTINE: case ANA_FUNCTION: case ANA_BLOCKROUTINE:
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
	 && symbol_class(tempVariableIndex - 1) == ANA_UNUSED)
    tempVariableIndex--;
  while (tempExecutableIndex > TEMP_EXE_START
	 && symbol_class(tempExecutableIndex - 1) == ANA_UNUSED)
    tempExecutableIndex--;
}
/*----------------------------------------------------------------*/
int nextFreeStackEntry(void)
     /* returns index to next free item in symbolStack.  May cycle. */
{
  int	oldIndex = symbolStackIndex;

  while (symbolStack[symbolStackIndex]) {
    if (++symbolStackIndex == SYMBOLSTACKSIZE)
      symbolStackIndex = 0;
    if (symbolStackIndex == oldIndex)
      return anaerror("Symbol stack full", 0);
  }
  nSymbolStack++;
  return symbolStackIndex;
}
/*----------------------------------------------------------------*/
int nextFreeNamedVariable(void)
     /* returns index to next free named variable in symbol table.
	some may have been zapped in the meantime, so cycle if at end of
	table */
{
  int	oldIndex = namedVariableIndex;
  extern int	compileLevel;

  while (sym[namedVariableIndex].class) {
    if (++namedVariableIndex == NAMED_END) namedVariableIndex = NAMED_START;
    if (namedVariableIndex == oldIndex)    /* nothing free */
      return anaerror("Too many named variables - symbol table full", 0);
  }
  sym[namedVariableIndex].exec = nExecuted;
  sym[namedVariableIndex].context = -compileLevel;
  sym[namedVariableIndex].line = curLineNumber;
  nNamedVariable++;
  return namedVariableIndex++;
}
/*----------------------------------------------------------------*/
int nextFreeTempVariable(void)
/* returns index to next free temporary variable in symbol table */
{
 extern int	compileLevel;

 while (sym[tempVariableIndex].class)
   if (++tempVariableIndex == TEMPS_END) {
     return anaerror("Too many temp variables - symbol table full", 0);
   }
 sym[tempVariableIndex].exec = nExecuted;
 sym[tempVariableIndex].context = -compileLevel;
 sym[tempVariableIndex].line = curLineNumber;
 mark(tempVariableIndex);
 nTempVariable++;
 return tempVariableIndex++;
}
/*----------------------------------------------------------------*/
int nextFreeTempExecutable(void)
/* returns index to next free temporary executable in symbol table */
{
  extern int	compileLevel;

  while (sym[tempExecutableIndex].class) {
    if (++tempExecutableIndex == EXE_END) 
      return anaerror("Too many temporary executables - symbol table full", 0);
  }
  sym[tempExecutableIndex].exec = nExecuted;
  sym[tempExecutableIndex].context = -compileLevel;
  sym[tempExecutableIndex].line = curLineNumber;
  nTempExecutable++;
  return tempExecutableIndex++;
}
/*----------------------------------------------------------------*/
int nextFreeExecutable(void)
/* returns index to next free executable in symbol table
   some may have been zapped in the meantime, so cycle if at end
   of table */
{
  int    oldIndex = executableIndex;
  extern int	compileLevel;

  while (sym[executableIndex].class) {
    if (++executableIndex == EXE_END)
      executableIndex = EXE_START;
    if (executableIndex == oldIndex)    /* nothing free */
      return anaerror("Too many permanent executables - symbol table full", 0);
  }
  sym[executableIndex].exec = nExecuted;
  sym[executableIndex].context = -compileLevel;
  sym[executableIndex].line = curLineNumber;
  nExecutable++;
  return executableIndex++;
}
/*----------------------------------------------------------------*/
int nextFreeUndefined(void)
/* returns a free undefined temporary variable */
{
  extern int	compileLevel;
  int	n;

  n = nextFreeTempVariable();
  if (n < 0) return n;		/* some error */
  sym[n].class = ANA_UNDEFINED;
  sym[n].context = -compileLevel;
  return n;
}
/*----------------------------------------------------------------*/
void pushList(word symNum)
/* pushes a symbol number unto the list stack */
{
 if (listStackItem - listStack < NLIST) {
   *listStackItem++ = symNum;
   return;
 }
 anaerror("Too many elements (%d) in list; list stack full\n", 0,
       listStackItem - listStack);
}
/*----------------------------------------------------------------*/
word popList(void)
/* pops a symbol number from the list stack */
{
 if (listStackItem > listStack)
   return *--listStackItem;
 return anaerror("Attempt to read from empty list stack\n", 0);
}
/*----------------------------------------------------------------*/
int moveList(int n)
/* moves the topmost <n> entries on the list stack over by one */
{
  if (listStackItem - listStack < NLIST)
  { memcpy(listStack + 1, listStack, n*sizeof(word));
    listStackItem++;
    return 1; }
  return anaerror("Too many elements (%d) in list; list stack full\n", 0,
	       listStackItem - listStack);
}
/*----------------------------------------------------------------*/
void swapList(int n1, int n2)
/* swaps the elements <n1> and <n2> positions down the list stack */
{
 int	temp;

 temp = listStackItem[-n2];
 listStackItem[-n2] = listStackItem[-n1];
 listStackItem[-n1] = temp;
}
/*----------------------------------------------------------------*/
int stackListLength(void)
/* returns the number of elements in the topmost list in the stack */
/* assumes that all lists are delimited by ANA_NEW_LIST */
{
 word	*i = listStackItem - 1;
 int	n = 0;

 if (i < listStack)
   return -1;		/* no list in stack */
 while (i >= listStack && *i != ANA_NEW_LIST) {
   i--;
   n++;
 }
 return n;
}
/*----------------------------------------------------------------*/
void dupList(void)
/* duplicates the topmost list */
{
  int	n, n2;

  n = n2 = stackListLength();
  pushList(ANA_NEW_LIST);
  while (n2--)
    pushList(listStackItem[-n-1]);
}
/*----------------------------------------------------------------*/
void unlinkString(int index)
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
void freeString(int index)
     /* removes the string (if any) from index <index> in symbolStack[] */
     /* and updates symbolStackIndex  */
{
  free(symbolStack[index]);
  unlinkString(index);
}
/*----------------------------------------------------------------*/
int installStruct(int base, int key)
/* returns a struct pointer, supports variables, user-functions, */
/* and user-routines as structures */
{
  int	n;

  n = lookForVar(base, curContext); /* seek user-defined item */
  if (n < 0)			/* no such item found */
    n = lookForSubr(base);	/* seek internal subroutine */
  if (n < 0)			/* no such internal subroutine */
    n = lookForFunc(base);	/* seek internal function */
  if (n < 0)			/* no such internal function */
    n = findVar(base, curContext); /* force variable */
  else				/* found something already, remove name */
    freeString(base);
  return newSymbol(ANA_LIST_PTR, n, key);
}
/*----------------------------------------------------------------*/
int copyElement(int symbol, char *key)
/* return a complete copy of variable <symbol>, with name (key) <key>, for */
/* inclusion in a list, structure, or range.  such a variable must */
/* not be a temporary, to prevent its premature deletion. */
{
  int	n;
  int	ana_replace(int, int);

  if ((n = installString(key)) < 0) return n;
  if ((n = findVar(n, 0)) < 0) return n;
  if (ana_replace(n, symbol) < 0) return -1;
  return n;
}
/*----------------------------------------------------------------*/
int findTarget(char *name, int *type, int allowSubr)
/* seeks identification of the name <*name> as a
   variable or a user-defined routine or an internal function.
   User-defined subroutines are only checked if <allowSubr> is non-zero.
   (they must be included when looking for a local variable in a subroutine
   through a structure tag -- e.g., MYFUNC.VAR -- but not when looking for
   a possible target of a subscript)
   Returns in <*type> an identifier of the type of target that was found: the
   variable's class for a named variable, ANA_FUNCTION for a
   user-defined function, ANA_SUBROUTINE for a user-defined subroutine,
   ANA_INT_FUNC for a built-in function, or
   ANA_ERROR if no target was found.  The function's return value
   identifies the particular found target: the symbol number if it's a
   named variable or user-defined function, the appropriate index to
   the function[] array if it's a built-in function, or ANA_ERROR if
   no target was found.  The symbolStack[] entry is removed.
   LS 26dec98 */
{
  int	result;
  FILE	*fp;
  int	nextCompileLevel(FILE *, char *);

  /* seek a named variable */
  result = lookForName(name, varHashTable, curContext);
  if (result >= 0) {
    *type = symbol_class(result);
    return result;
  }
  if (*name == '$' || *name == '!' || *name == '#') {
    /* it's not an existing named variable, yet starts with $ ! or #,
       so it cannot be a function either. */
    *type = ANA_ERROR;
    return anaerror("Subscripted variable %s is undefined", 0, name);
  }
  /* seek a user-defined function */
  result = lookForName(name, funcHashTable, 0);
  if (result >= 0) {
    *type = ANA_FUNCTION;	/* a user-defined function */
    return result;
  }
  if (allowSubr) {
    /* seek a user-defined subroutine */
    result = lookForName(name, subrHashTable, 0);
    if (result >= 0) {
      *type = ANA_SUBROUTINE;
      return result;
    }
  }
  /* seek a built-in function */
  result = findInternalName(name, 0);
  if (result >= 0) {
    *type = ANA_INT_FUNC;	/* an internal function */
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
      *type = ANA_ERROR;
      return anaerror("Compiled file %s but function %s still is not compiled",
		   0, expname, name);
    } else {
      *type = ANA_FUNCTION;
      return result;
    }
  }
  /* we did not find the target anywhere */
  *type = ANA_ERROR;
  return ANA_ERROR;
}
/*----------------------------------------------------------------*/
int newSymbol(int kind, ...)
/* returns index to symbol table for a new symbol.
   arguments depend on kind:
     (ANA_SCALAR, type)
     (ANA_FIXED_NUMBER, type)
     (ANA_FIXED_STRING, symbolStackIndex)
     (ANA_RANGE, range_start, range_end)
     (ANA_EVB, EVB_REPLACE, lhs, rhs)
     (ANA_EVB, EVB_FOR, loopVar, start, end, step, statement)
     (ANA_EVB, EVB_WHILE_DO, condition, statement)
     (ANA_EVB, EVB_DO_WHILE, statement, condition)
     (ANA_EVB, EVB_IF, condition, true_statement, false_statement)
     (ANA_EVB, EVB_CASE)
     (ANA_EVB, EVB_NCASE)
     (ANA_EVB, EVB_INT_SUB)
     (ANA_EVB, EVB_USR_SUB)
     (ANA_EVB, EVB_BLOCK)
     (ANA_EVB, EVB_INSERT, target)
     (ANA_EVB, EVB_RETURN, expr)
     (ANA_EVB, EVB_FILE)
     (ANA_INT_FUNC, funcNr)
     (ANA_BIN_OP, opNr, arg2, arg1)
     (ANA_IF_OP, opNr, arg2, arg1)
     (ANA_KEYWORD, param, expr)
     (ANA_CLIST)
     (ANA_LIST)
     (ANA_LIST_PTR, struct, key)
     (ANA_POINTER, target)
     (ANA_SUBROUTINE, name)
     (ANA_FUNCTION, name)
     (ANA_BLOCKROUTINE, name)
     (ANA_SUBSC_PTR)
     (ANA_META, expr)
     (ANA_EXTRACT)
     (ANA_STRUCT_PTR)
*/
{
  int		n, i, narg, isStruct, isScalarRange, j, target, depth;
  extern char	reportBody, ignoreSymbols, compileOnly;
  extractSec	*eptr;
  word	*ptr;
  pointer	p;
#if YYDEBUG
  extern int	yydebug;
#endif
  /* static char	inDefinition = 0; */
  word		*arg;
  va_list	ap;
  int	int_arg(int);
  void	fixContext(int, int);

 /* don't generate symbols in bodies of routines when using @@file */
 /* (reportBody) and when defining a routine (inDefinition).  */
/* ignoreSymbols = (reportBody && inDefinition); */
  va_start(ap, kind);
  n = -1;
  if (!ignoreSymbols) {		/* really need a new symbol.  what kind? */
    if (kind >= ANA_BIN_OP) {
				/* executable */
      if (keepEVB)
	n = nextFreeExecutable();
      else
	n = nextFreeTempExecutable();
      if (n < 0) {
	va_end(ap);
	return ANA_ERROR;	/* didn't work */
      }
      symbol_class(n) = kind; 
      if (keepEVB)
	symbol_context(n) = curContext;
    } else if (kind < ANA_SUBROUTINE) {	/* named variable */
      if (keepEVB)
	n = nextFreeNamedVariable();
      else
	n = nextFreeTempVariable();
      if (n < 0) {
	va_end(ap);
	return ANA_ERROR;
      }
      symbol_class(n) = kind;
      if (keepEVB)
	symbol_context(n) = curContext;
    }
    switch (kind) {
      case ANA_SCALAR:
	scalar_type(n) = va_arg(ap, int);
	break;
      case ANA_FIXED_NUMBER:
	/* same as an ordinary scalar, but in EDB symbol space, so that it */
	/* doesn't get overwritten as "just another temp" (i.e. isFreeTemp() */
	/* returns 0).  otherwise, get problems e.g. in a for-loop, where */
	/* temp numbers are used more than once.  */
	symbol_type(n) = va_arg(ap, int);
	if (symbol_type(n) >= ANA_CFLOAT) { /* a complex scalar */
	  symbol_class(n) = ANA_CSCALAR;
	  complex_scalar_memory(n) = ana_type_size[symbol_type(n)];
	  complex_scalar_data(n).f = malloc(complex_scalar_memory(n));
	  if (!complex_scalar_data(n).f)
	    return cerror(ALLOC_ERR, n);
	} else
	  symbol_class(n) = ANA_SCALAR;
	break;
      case ANA_FIXED_STRING:
	/* a literal string */
	symbol_class(n) = ANA_STRING;
	string_type(n) = ANA_LSTRING;
	string_value(n) = symbolStack[i = va_arg(ap, int)];
	symbol_memory(n) = strlen(symbolStack[i]) + 1; /* count \0 */
	unlinkString(i);		/* free position in stack */
	break;
      case ANA_STRUCT_PTR:
	symbol_class(n) = ANA_STRUCT_PTR;
	symbol_memory(n) = sizeof(structPtr);
	struct_ptr_elements(n) = malloc(symbol_memory(n));
	break;
      case ANA_EXTRACT:
	target = popList();
	if (target > 0) {	/* regular symbol */
	  extract_target(n) = target;
	  extract_num_sec(n) = depth = popList();
	  symbol_memory(n) = depth*sizeof(extractSec);
	  extract_ptr(n) = eptr = malloc(symbol_memory(n));
	  embed(target, n);
	} else {
	  i = -target;
	  symbol_class(n) = ANA_PRE_EXTRACT;
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
	  ptr--;		/* skip the (potential) ANA_NEW_LIST */
	  eptr--;		/* go to previous entry */
	  eptr->type = popList(); /* the type of the current list;
				    either ANA_RANGE or ANA_LIST */
	  eptr->number = stackListLength(); /* the number of entries in this
					      one */
	  i = eptr->number;
	  switch (eptr->type) {
	    case ANA_RANGE:
	      eptr->ptr.w = malloc(i*sizeof(word));
	      p.w = eptr->ptr.w + i; /* start at the end */
	      while (i--) {
		*--p.w = popList();
		embed(*p.w, n);
	      }
	      break;
	    case ANA_LIST:
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
	  popList();		/* remove the ANA_NEW_LIST */
	}
	break;
      case ANA_META:		/* a meta symbol, i.e. a string expression */
				/* which points at a symbol */
	meta_target(n) = va_arg(ap, int);
	embed(meta_target(n), n);
	break;
      case ANA_RANGE:  /* a range */
	isScalarRange = 1;
	/* range start: */
	i = va_arg(ap, int);
	range_start(n) = i;
	if (i < 0)
	  i = -i;
	embed(i, n);
	if (symbol_class(i) != ANA_SCALAR)
	  isScalarRange = 0;
	/* range end: */
	i = va_arg(ap, int);
	range_end(n) = i;
	if (i < 0)
	  i = -i;
	embed(i, n);
	if (symbol_class(i) != ANA_SCALAR)
	  isScalarRange = 0;
	range_sum(n) = 0; /* default summation flag */
	range_redirect(n) = -1; /* redirection flag */
	range_scalar(n) = isScalarRange;
	break;
      case ANA_PRE_RANGE:
	isScalarRange = 1;
	/* pre_range start: */
	i = va_arg(ap, int);
	pre_range_start(n) = i;
	if (i < 0)
	  i = -i;
	embed(i, n);
	if (symbol_class(i) != ANA_SCALAR)
	  isScalarRange = 0;
	/* pre_range end: */
	i = va_arg(ap, int);
	pre_range_end(n) = i;
	if (i < 0)
	  i = -i;
	embed(i, n);
	if (symbol_class(i) != ANA_SCALAR)
	  isScalarRange = 0;
	pre_range_sum(n) = 0; /* default summation flag */
	pre_range_redirect(n) = -1; /* redirection flag */
	pre_range_scalar(n) = isScalarRange;
	break;
      case ANA_LIST_PTR:		/* pointer to a struct element */
	list_ptr_target(n) = va_arg(ap, int); /* the struct */
	if ((i = va_arg(ap, int)) >= 0) { /* non-numerical key */
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
      case ANA_LIST: case ANA_PRE_LIST: /* includes LISTs */
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
	    return anaerror("Could not allocate memory for a struct", 0);
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
	  symbol_class(n) = ANA_PRE_CLIST;
	  if (narg) {
	    if (!(arg = (word *) malloc(narg*sizeof(word)))) {
	      va_end(ap); 
	      return anaerror("Could not allocate memory for a list", 0);
	    }
	  } else
	    arg = NULL;
	  symbol_memory(n) = narg*sizeof(word);
	  pre_clist_symbols(n) = arg;
	  arg += narg;
	  while (narg--) {
	    *--arg = popList();
	    embed(*arg, n);
	    popList();
	  }
	}
	popList();				/* pop ANA_NEW_LIST */
	break;
      case ANA_SUBSC_PTR:	/* subscript pointer */
	if (!(symbol_data(n) = (int *) malloc(4*sizeof(int)))) {
	  va_end(ap);
	  printf("newSymbol: ");
	  return cerror(ALLOC_ERR, 0);
	}
	symbol_memory(n) = 4*sizeof(int);
	break;
      case ANA_POINTER:
	transfer_is_parameter(n) = 0;	/* not a formal argument in a */
					/* user-defined function or routine */
	narg = va_arg(ap, int);
	i = lookForSubr(narg);
	if (i < 0)
	  i = lookForFunc(narg);
	if (i < 0)
	  i = lookForBlock(narg);
	if (i >= 0) {		/* found a routine */
	  symbol_class(n) = ANA_FUNC_PTR;
	  func_ptr_routine_num(n) = i;
	  unlinkString(narg);
	  return n;
	}
	i = findInternalSubr(narg);
	kind = ANA_SUBROUTINE;
	if (i < 0) {
	  i = findInternalFunc(narg);
	  kind = ANA_FUNCTION;
	}
	if (i >= 0) {		/* found an internal routine */
	  symbol_class(n) = ANA_FUNC_PTR;
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
      case ANA_KEYWORD:
	keyword_name_symbol(n) = newSymbol(ANA_FIXED_STRING, va_arg(ap, int));
	embed(keyword_name_symbol(n), n);
	keyword_value(n) = va_arg(ap, int);
	embed(keyword_value(n), n);
	break;
      case ANA_SUBROUTINE: case ANA_FUNCTION: case ANA_BLOCKROUTINE:
       /* define these routines in two passes: first, define all
	  parameters, and then parse the statements when the
	  parameters are already known */
       /* if this routine is encountered during a @-compilation, then
	  delete any old definition and compile the new one.
	  if encountered during a @@-compilation, then don't do anything
	  if the routine is already compiled;  merely note the file if
	  the routine is not yet defined */
      { word	nArg, nStatement;
	int	oldContext;
	char	**key;
	 
	n = i = va_arg(ap, int);
	if (n >= 0) {		/* first pass: n = index of routine's name */
				/* in symbolStack[] */
	  switch (kind)	{	/* first see if the routine is already */
				/* defined */
	    case ANA_SUBROUTINE:
	      n = lookForSubr(n);
	      break;
	    case ANA_FUNCTION:
	      n = lookForFunc(n);
	      break;
	    case ANA_BLOCKROUTINE:
	      n = lookForBlock(n);
	      break;
	  }
	  if (n == -1)		/* the routine is not yet defined */
	    switch (kind) {	/* so allocate a symbol for it */
	      case ANA_SUBROUTINE:
		n = findSubr(i);
		break;
	      case ANA_FUNCTION:
		n = findFunc(i);
		break;
	      case ANA_BLOCKROUTINE:
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
	    return ANA_ERROR; 	/* pass on the error */
	  }
	  symbol_class(n) = kind;

	  if (kind != ANA_BLOCKROUTINE) { /* has parameters */
	    nArg = stackListLength();	/* # parameters */
	    if (!reportBody) {	/* we're compiling the body */
	      if (listStackItem[-1] == ANA_EXTEND) { /* extended parameter */
		routine_has_extended_param(n) = 1;
		nArg--;
	      } else
		routine_has_extended_param(n) = 0;
	      if (nArg > UCHAR_MAX) { /* too many parameters */
		va_end(ap);
		reportBody = 0;
		return anaerror("More than %1d parameters specified\n", n,
			     UCHAR_MAX);
	      }
	      routine_num_parameters(n) = nArg;
	      if (nArg
		  && !(routine_parameters(n)
		       = (word *) malloc(nArg*sizeof(word)))) {
				/* could not allocate room for parameters */
		va_end(ap);
		reportBody = 0;
		return anaerror("Routine-definition memory-allocation error", 0);
	      }
	    } else		/* deferred compilation */
	      symbol_class(n) = (kind == ANA_SUBROUTINE)?
		ANA_DEFERRED_SUBR: ANA_DEFERRED_FUNC;
	  } else {		/* a block routine, which has no parametrs */
	    if (reportBody)
	      symbol_class(n) = ANA_DEFERRED_BLOCK;
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
	    if (kind != ANA_BLOCKROUTINE) {
	      if (nArg &&
		  !eallocate(routine_parameter_names(n), nArg, char *)) {
		/* could not allocate memory to store the parameter names */
		va_end(ap); 
		return anaerror("Memory allocation error", 0);
	      }
	      key = routine_parameter_names(n) + nArg;
	      while (nArg--) {
		*--arg = findVar(popList(), n); /* parameter's symbol # */
		*--key = varName(*arg); /* parameter's name */
		symbol_class(*arg) = ANA_POINTER;
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
	    if (kind != ANA_BLOCKROUTINE)
	      curContext = n;	/* make current function or subroutine
				   the current context */
	  } else {		/* not compiling the body */
	    if (kind != ANA_BLOCKROUTINE) {
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
	      && (!compileOnly || symbol_class(n) != ANA_BLOCKROUTINE)) {
	    /* can't have empty routines or functions
	       except for an empty block routine in VERIFY if the verified
	       file contains only a definition for a subroutine or
	       function. */
	    va_end(ap);
	    curContext = oldContext;
	    ignoreSymbols = 0;
	    return anaerror("No statements in user routine or function", 0);
	  }
	  if (kind == ANA_BLOCKROUTINE) { /* allocate space for the */
	    /* statements (since there are no parameters, this is at */
	    /* the beginning of the combined parameters+statements list) */
	    if (nStatement &&
		!(routine_parameters(n) =
		  (word *) malloc(nStatement*sizeof(word)))) {
	      va_end(ap); 
	      curContext = oldContext;	/* restore context */
	      ignoreSymbols = 0;
	      return
		anaerror("Allocation error in 2nd pass of routine definition",
		      0);
	    }
	    nArg = 0;		/* no parameters to a block routine */
	  } else {			/* subroutine or function */
	    nArg = routine_num_parameters(n);
	    if (nArg)		/* reallocate memory for combined */
				/* parameters+statements list */
	      routine_parameters(n) =
		(word *) realloc(routine_parameters(n),
				 (nArg + nStatement)*sizeof(word));
	    else		/* no parameters, just allocate space for */
				/* statements */
	      routine_parameters(n) =
		(word *) malloc(nStatement*sizeof(word));
	    if (!routine_parameters(n)) { /* allocation failed */
	      va_end(ap);
	      curContext = oldContext;	/* restore context */
	      ignoreSymbols = 0;
	      return
		anaerror("Allocation error in 2nd pass of routine definition",
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
      case ANA_EVB: case ANA_INT_FUNC: case ANA_USR_FUNC:
	if (kind == ANA_EVB) {
	  kind = va_arg(ap, int);
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
	    usr_code_routine_num(n) = va_arg(ap, int); /* routine number */
	    break;
	  case EVB_FILE:
	    i = va_arg(ap, int);	/* index to string */
	    file_name(n) = symbolStack[i];
	    symbol_memory(n) = strlen(symbolStack[i]) + 1;
	    unlinkString(i);
	    i = va_arg(ap, int);	/* include type: INCLUDE -> always,
					   REPORT -> only if necessary */
	    file_include_type(n) = i;
	    i = 0;		/* no more items to retrieve */
	    break;
	  case EVB_INT_SUB: case EVB_USR_SUB: case EVB_INSERT:
	  case ANA_INT_FUNC: case ANA_USR_FUNC:
	    sym[n].xx = va_arg(ap, int); /* routine number (SUB) or target */
	  case EVB_CASE: case EVB_NCASE: case EVB_BLOCK: 
	    i = stackListLength();		/* # of expr and statements */
	    if (i) {			/* only if there are any elements */
	      if (!(arg = (word *) malloc(i*sizeof(word)))) {
		va_end(ap);
		return anaerror("Could not allocate memory for stacked elements",
			     0);
	      }
	      symbol_data(n) = arg; /* the elements */
	      symbol_memory(n) = i*sizeof(word);	/* the memory size */
	      arg += i;	/* start with the last element (which is */
	      /* on top of the stack) */
	      while (i--) {	/* all elements */
		*--arg = popList(); /* pop element from stack */
		embed(*arg, n);  /* context is enclosing statement */
	      }
	    } else
	      symbol_memory(n) = 0; /* no args */
	    popList();		/* pop ANA_NEW_LIST marker */
	    i = 0;		/* no more items to retrieve */
	    break;
	}
	if (i > 0) {
	  arg = sym[n].spec.evb.args;
	  while (i--) {
	    *arg = va_arg(ap, int);
	    embed(*arg, n);
	    arg++;
	  }
	  if (kind == EVB_FOR) {
	    for_body(n) = va_arg(ap, int);
	    embed(for_body(n), n);
	  }
	}
	break;
      case ANA_BIN_OP: case ANA_IF_OP:
	bin_op_type(n) = va_arg(ap, int);
	bin_op_lhs(n) = va_arg(ap, int);
	embed(bin_op_lhs(n), n);
	bin_op_rhs(n) = va_arg(ap, int);
	embed(bin_op_rhs(n), n);
	break;
    }
  } else {			/* reportBody & in definition */
    switch (kind) {
      case ANA_FIXED_STRING:
	i = va_arg(ap, int);	/* index to symbolStack */
	freeString(i);
	break;
      case ANA_LIST_PTR:
	i = va_arg(ap, int);	/* struct number */
	i = va_arg(ap, int);	/* key */
	if (i >= 0)		/* non-numerical key */
	  freeString(i); 
	break;
      case ANA_LIST:  case ANA_PRE_LIST:
	narg = stackListLength()/2;
	while (narg--) {
	  popList();		/* value */
	  i = popList();	/* key */
	  if (i >= 0)		/* string key */
	    freeString(i);
	}
	break;
      case ANA_KEYWORD:
	i = va_arg(ap, int);
	freeString(i);
	break;
      case ANA_SUBROUTINE: case ANA_FUNCTION: case ANA_BLOCKROUTINE:
	/* when we get here, a symbol has already been reserved for the */
	/* routine, and the parameters have been ignored.  we only need to */
	/* get rid of the routine body. */
	n = va_arg(ap, int);
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
      case ANA_EXTRACT:
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
	  popList();		/* remove the ANA_NEW_LIST */
	}
	break;
      case ANA_EVB: case ANA_INT_FUNC: case ANA_USR_FUNC:
	if (kind == ANA_EVB)
	  kind = va_arg(ap, int);
	switch (kind) {
	  case EVB_FILE:
	    i = va_arg(ap, int);
	    freeString(i);
	    break;
	  case EVB_INT_SUB: case EVB_USR_SUB: case EVB_INSERT:
	  case ANA_INT_FUNC: case ANA_USR_FUNC:
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
int hash(char *string)
{
 int	i;

 for (i = 0; *string; ) i += *string++;
 return i % HASHSIZE;
}
/*----------------------------------------------------------------*/
int findInternalName(char *name, int isSubroutine)
/* searches for name in the appropriate subroutine
  or function table.  if found, returns
  index, else returns -1 */
{
 int	hi, lo = 0, mid, s;
 internalRoutine	*table;

 if (isSubroutine)
 { table = subroutine;  hi = nSubroutine - 1; }
 else
 { table = function;  hi = nFunction - 1; }
 while (lo <= hi)
 { mid = (lo + hi)/2;
   if ((s = strcmp(name, table[mid].name)) < 0)
     hi = mid - 1;
   else if (s > 0)  lo = mid + 1;
   else return mid; }
 return -1;
}
/*----------------------------------------------------------------*/
static compileInfo	*c_info = NULL;
int	cur_c_info = 0, n_c_info = 0;
compileInfo	*curCompileInfo;

int nextCompileLevel(FILE *fp, char *fileName)
/* saves the rest of the current input line and starts reading
 input from file fp.  When the file is processed, compilation
 at the current level is resumed. */
{
 int	n, oldZapContext;
 char	*name;
 extern int	echo; 
 extern char	inHistoryBuffer, tLine[], *inputString;
 extern byte	disableNewline;
 int	yyparse(void), getStreamChar(void), getStringChar(void);
 extern int	(*getChar)(void);
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
     anaerror("*** error in file %s", 0, name);
   else
     anaerror("*** error in execution string \"%s\"", 0, inputString);
 }
 /* if (fp) 
   free(name); */
 return n;
} 
/*----------------------------------------------------------------*/
static int	compileCount = 0;
int compile(char *string)
/* compiles string <string> and store in BLOCKROUTINE.  Returns */
/* but does not execute the block routine.  LS 5feb96 */
{
  int	oldContext, n, getStringChar(void), getStreamChar(void), nsym, result;
  extern char	*inputString, compileOnly;
  extern int	(*getChar)(void), executeLevel;
  char	compileName[12], oldInstalling;
  int	newBlockSymbol(int);

  inputString = string;
  compileOnly++;
  executeLevel++;
  getChar = getStringChar;
  oldContext = curContext;
  sprintf(compileName, "#COMPILE%1d", compileCount);
  n = installString(compileName);
  oldInstalling = installing;
  installing = 1;
  nsym = newSymbol(ANA_BLOCKROUTINE, n);
  installing = oldInstalling;
  curContext = nsym;
  n = nextCompileLevel(NULL, NULL);
  if (n < 0
      || newSymbol(ANA_BLOCKROUTINE, -nsym - 1) < 0) { /* some error */
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
int ana_compile(int narg, int ps[])
{
  char	*string;
  int	result, value;

  string = string_arg(*ps);
  result = scalar_scratch(ANA_LONG);
  if (string)
  { value = compile(string);
    scalar_value(result).l = value; }
  else
    scalar_value(result).l = -1;
  return result;
}
#endif
/*----------------------------------------------------------------*/
int newBlockSymbol(int index)
/* searches for user block symbolStack[index] in list of user-defined
  block routines.  treats function pointers */
{
  int	n, result;
  extern char	reportBody;
  
  if (reportBody) {		/* remove name from stack */
    freeString(index);
    return 0;
  }
  if ((n = lookForVar(index, curContext)) >= 0) { /* blockroutine pointer? */
    if (sym[n].class == ANA_FUNC_PTR) {
      if (func_ptr_routine_num(n) > 0) { /* user-defined */
	if (sym[n = func_ptr_routine_num(n)].class == ANA_BLOCKROUTINE) {
	  freeString(index);
	  return newSymbol(ANA_EVB, EVB_USR_CODE, n);
	}
      } else
	return anaerror("Func/subr pointer does not point at executable block routine!", n);
    }
  }
  n = lookForBlock(index);
  if (n < 0) {			/* block not yet defined */
    n = nextFreeTempVariable();
    if (n < 0)
      return ANA_ERROR;
    symbol_class(n) = ANA_STRING;
    string_value(n) = symbolStack[index];
    unlinkString(index);
    symbol_memory(n) = strlen(string_value(n)) + 1;
    result = newSymbol(ANA_EVB, EVB_USR_CODE, n);
    symbol_context(usr_code_routine_num(result)) = result;
  } else {
    n = findBlock(index);
    result = newSymbol(ANA_EVB, EVB_USR_CODE, n);
  }
  return result;
}
/*----------------------------------------------------------------*/
int newSubrSymbol(int index)
/* searches for subroutine symbolStack[index] in lists of internal
  and user-defined subroutines.  if not found, then searches for an
  appropriate file to find a definition.  if such a file is found,
  then installs name as new subroutine in user-defined subroutine list,
  and a new symbol with appropriate class (ANA_EVB) and type (EVB_INT_SUB 
  or EVB_USR_SUB) is returned.  if such a file is not found, then
  an error is generated. */
{
 int	n, i;
 char	*name;
 extern char	reportBody;
 extern int	findBody;

 /* In order, look for: */
 /* 1. function pointer to some user-defined or internal subroutine */
 /* 2. user-defined subroutine */
 /* 3. internal subroutine */
 /* 4. user-defined function */

 if (ignoreInput && findBody > 0) { /* not compiling this */
   freeString(index);		/* remove name from stack */
   /* take care of deleting arguments: */
   return newSymbol(ANA_EVB, EVB_INT_SUB, 0);
 }
 if (findBody < 0)		/* we're at the end of the definition
				   of a deferred routine */
   findBody = -findBody;
 n = lookForVar(index, curContext); /* look for variable */
 if (n >= 0 && symbol_class(n) == ANA_FUNC_PTR) { /* maybe subr pointer */
   freeString(index);		/* remove name from stacke */
   if (func_ptr_routine_num(n) < 0) {	/* internal routine/function */
     if (func_ptr_type(n) == ANA_SUBROUTINE)
       return newSymbol(ANA_EVB, EVB_INT_SUB, -sym[n].spec.evb.args[0]);
   } else {
     n = func_ptr_routine_num(n); /* user-defined routine */
     return newSymbol(ANA_EVB, EVB_USR_SUB, n);
   }
 }
 /* no subroutine pointer */
 name = symbolStack[index];
 n = lookForSubr(index);	/* already defined user-defined routine? */
 if (n < 0) {			/* none found */
   if ((n = findInternalSym(index, 1)) >= 0) { /* internal routine */
     freeString(index);
     return newSymbol(ANA_EVB, EVB_INT_SUB, n);
   } else {			/* no internal: assume user-defined */
     n = newSymbol(ANA_FIXED_STRING, index);
     i = newSymbol(ANA_EVB, EVB_USR_SUB, n);
     symbol_context(n) = i;
     return i;
   }
 }
				/* user-defined routine */
 freeString(index);
 return newSymbol(ANA_EVB, EVB_USR_SUB, n);
}
/*----------------------------------------------------------------*/
int lookForName(char *name, hashTableEntry *hashTable[], int context)
     /* searches name in hashTable[] for context.  if found,
	returns symbol number, otherwise returns -1 */
{
  int		hashValue, n;
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
int findSym(int index, hashTableEntry *hashTable[], int context)
/* searches symbolStack[index] in hashTable[] for context.  if found,
   returns symbol number, otherwise installs the name in hashTable[]
   and sym[].  always removes the entry from the symbolStack. */
{
 char	*name;
 int	n;
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
char *symName(int symNum, hashTableEntry *hashTable[])
/* returns the name of the symbol, if any, or "[symNum]" */
{
 static char	name[7];
 int		hashValue;
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
char *symbolName(int symbol)
/* returns the name of the symbol. */
{
  hashTableEntry	**hashTable;

  if (symbol < 0 || symbol >= NSYM) {
    cerror(ILL_SYM, 0, symbol, "symbolName");
    return "(error)";
  }
  switch (symbol_class(symbol)) {
    case ANA_SUBROUTINE: case ANA_DEFERRED_SUBR:
      hashTable = subrHashTable;
      break;
    case ANA_FUNCTION: case ANA_DEFERRED_FUNC:
      hashTable = funcHashTable;
      break;
    case ANA_BLOCKROUTINE: case ANA_DEFERRED_BLOCK:
      hashTable = blockHashTable;
      break;
    default:
      hashTable = varHashTable;
      break;
  }
  return symName(symbol, hashTable);
}
/*----------------------------------------------------------------*/
int suppressEvalRoutine(int index)
/* returns evaluation suppression associated with internal routine */
/* symbolStack[index] */
{
  int	n;
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
void exception(int sig)
/* exception handler */
{
 int	c, saveHistory(void);
 extern int	curSymbol, executeLevel, step, statementDepth;
 extern jmp_buf	jmpenv;
 void	cleanUp(int, int), Quit(int);

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
       c = getchar();
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
	   Quit(1);		/* exit ANA completely */
	 case 'r': case 'R':
	   saveHistory();
	   ana_restart(0, NULL);
	 case '?':
	   printw("Options:  y - yes, quit;  t - start tracing;  ");
	   printw("s - start stepping;  q - run quietly (no tracing or ");
	   printw("stepping);  a - abort ANA;  r - restat ANA; ");
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
     c = getchar();
     if (c == 'y' || c == 'Y')
       Quit(2);
     c = SIG_BREAK;
     break;
   case SIGCONT:
     puts("Continuing ANA...");
     rawIo();
     c = SIG_BREAK;
     break;
   default:
     printf("Exception %d - Quitting.\n", sig);
     Quit(3);
 }
 checkErrno();
 if (signal(sig, exception) == SIG_ERR)
   anaerror("Could not reinstall exception handler", 0);
 if (c == SIG_BREAK) {
   curContext = executeLevel = statementDepth = 0;
   cleanUp(-compileLevel, CLEANUP_ALL);
   longjmp(jmpenv, 0);
 }
 return;
}
/*----------------------------------------------------------------*/
char *typeName(int type)
/* returns the name that goes with the data type */
{
  static char *typeNames[] = {
    "BYTE", "WORD", "LONG", "FLOAT", "DOUBLE",
    "STRING", "STRING", "STRING", "CFLOAT", "CDOUBLE",
    "undefined", "unknown"
  };
  int	index;

  if (type == ANA_UNDEFINED)
    index = 10;			/* undefined */
  else if (type < 0 || type > ANA_CDOUBLE)
    index = 11;			/* unknown */
  else
    index = type;		/* OK */
  return typeNames[index];
}
/*----------------------------------------------------------------*/
char *className(int class)
/* returns the name of the class */
{
  static struct classInfo {
    byte number; char *name;
  } classes[] = {
    { ANA_UNUSED, "not used" },
    { ANA_SCALAR, "scalar" },
    { ANA_STRING, "string" },
    { ANA_RANGE, "range" },
    { ANA_ARRAY, "array" },
    { ANA_POINTER, "pointer" },
    { ANA_ASSOC, "associated variable" },
    { ANA_FUNC_PTR, "function pointer" },
    { ANA_SCAL_PTR, "scalar pointer" },
    { ANA_SUBSC_PTR, "subscript pointer" },
    { ANA_FILEMAP, "file array" },
    { ANA_CLIST, "compact list" },
    { ANA_LIST, "list" },
    { ANA_STRUCT, "structure" },
    { ANA_KEYWORD, "keyword" },
    { ANA_LIST_PTR, "list pointer" },
    { ANA_PRE_RANGE, "pre-range" },
    { ANA_PRE_CLIST, "pre-compact-list" },
    { ANA_PRE_LIST, "pre-list" },
    { ANA_ENUM, "enumeration constant" },
    { ANA_META, "SYMBOL call" },
    { ANA_CSCALAR, "complex scalar" },
    { ANA_CARRAY, "complex array" },
    { ANA_CPLIST, "compact pointer list" },
    { ANA_TRANSFER, "transfer symbol" },
    { ANA_STRUCT_PTR, "struct pointer" },
    { ANA_SUBROUTINE, "subroutine" },
    { ANA_FUNCTION, "function" },
    { ANA_BLOCKROUTINE, "block routine" },
    { ANA_DEFERRED_SUBR, "deferred subroutine" },
    { ANA_DEFERRED_FUNC, "deferred function" }, 
    { ANA_DEFERRED_BLOCK, "deferred block routine" },
    { ANA_BIN_OP, "binary operation" },
    { ANA_INT_FUNC, "internal function call" },
    { ANA_USR_FUNC, "user function call" },
    { ANA_IF_OP, "if-operation" },
    { ANA_EXTRACT, "extraction" },
    { ANA_PRE_EXTRACT, "pre-extraction" },
    { ANA_EVB, "executable" },
    { ANA_FIXED_NUMBER, "fixed number" },
    { ANA_FIXED_STRING, "fixed string" },
    { ANA_UNDEFINED, "undefined" },
    { 0, "unknown" }
  };

 static char	classHashTable[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
  20, 21, 22, 23, 24, 25, 42, 41, 42, 42, 42, 42, 26, 27, 28, 29, 30, 31,
  42, 42, 32, 33, 34, 35, 36, 37, 42, 42, 38, 39, 40
 };

 int	hash;

 if (class < 0)
   hash = 26;
 else {
   hash = class % 76;
   if (hash > 51)
     hash = 25;
   hash = classHashTable[hash];
   if (class != (int) classes[hash].number)
     hash = 42;
 }
 return classes[hash].name;
}
/*----------------------------------------------------------------*/
int ana_classname(int narg, int ps[])
     /* returns name associated with class number */
{
  int	class, result;
  char	*name;

  class = int_arg(*ps);
  getFreeTempVariable(result);
  sym[result].class = ANA_STRING;
  string_type(result) = ANA_TEMP_STRING;
  name = string_value(result) = strsave(className(class));
  symbol_memory(result) = strlen(name) + 1;
  return result;
}
/*----------------------------------------------------------------*/
int ana_typeName(int narg, int ps[])
     /* returns name associated with type number */
{
  int	type, result;
  char	*name;

  if ((type = int_arg(*ps)) < 0) return -1;
  getFreeTempVariable(result);
  sym[result].class = ANA_STRING;
  string_type(result) = ANA_TEMP_STRING;
  name = string_value(result) = strsave(typeName(type));
  symbol_memory(result) = strlen(name) + 1;
  return result;
}
/*----------------------------------------------------------------*/
char *evbName(int evbType)
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
char *filetypeName(int filetype)
/* returns the name associated with a file type */
{
  static char *filetypeNames[] = {
    "Unknown", "ANA fz", "IDL Save", "GIF", "ANA Astore", "JPEG", "TIFF",
    "FITS", "PPM (raw)", "PPM (ascii)", "XPM", "X11 bitmap", "BMP",
    "Sun raster", "Iris RGB", "Targa (24 bit)", "PM"
  };

  if (filetype < 0 || filetype >= sizeof(filetypeNames)/sizeof(char *))
    filetype = 0;
  return filetypeNames[filetype];
}
/*----------------------------------------------------------------*/
int ana_filetype_name(int narg, int ps[])
{
  char	*name;
  int	result;

  name = filetypeName(int_arg(ps[0]));
  result = string_scratch(strlen(name));
  strcpy(string_value(result), name);
  return result;
}
/*----------------------------------------------------------------*/
void fixedValue(char *name, int type, ...)
/* install a numerical constant */
{
 int	n, iq;
 pointer	p;
 va_list	ap;

 va_start(ap, type);
 iq = installString(name);
 n = findVar(iq, 0);
 switch (type) {
   case ANA_LSTRING:
     symbol_class(n) = ANA_STRING;
     string_type(n) = ANA_LSTRING;
     string_value(n) = va_arg(ap, char *);
     symbol_memory(n) = strlen(string_value(n)) + 1;
     break;
   case ANA_CFLOAT: case ANA_CDOUBLE:
     complex_scalar_data(n).cf = malloc(ana_type_size[type]);
     if (!complex_scalar_data(n).cf)
       puts("WARNING - memory allocation error in symbol initialization");
     complex_scalar_memory(n) = ana_type_size[type];
     symbol_class(n) = ANA_CSCALAR;
     complex_scalar_type(n) = type;
     p.cf = complex_scalar_data(n).cf;
     if (type == ANA_CFLOAT) {
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
     symbol_class(n) = ANA_SCALAR;
     scalar_type(n) = type;
     switch (type) {
       case ANA_LONG:
	 scalar_value(n).l = va_arg(ap, int);
	 break;
       case ANA_FLOAT:
	 scalar_value(n).f = (float) va_arg(ap, double);
	 break;
       case ANA_DOUBLE:
	 scalar_value(n).d = va_arg(ap, double);
	 break;
     }
     break;
 }
}
/*----------------------------------------------------------------*/
int installSysFunc(char *name, int number)
/* install a system function.  These are implemented as FUNC_PTRs to */
/* the appropriate function */
{
 int	n, iq;

 iq = installString(name);
 n = findVar(iq,0);
 sym[n].class = ANA_FUNC_PTR;
 func_ptr_routine_num(n) = -number;
 func_ptr_type(n) = ANA_FUNCTION;
 return n;
}
/*----------------------------------------------------------------*/
int installPointer(char *name, int type, void *ptr)
/* install a ANA_SCAL_PTR system variable */
{ 
 int	n, iq;
 
 iq = installString(name);
 n = findVar(iq, 0);
 sym[n].class = ANA_SCAL_PTR;
 scal_ptr_type(n) = type;
 scal_ptr_pointer(n).l = (int *) ptr;
 if (type == ANA_TEMP_STRING)
   symbol_memory(n) = strlen(ptr) + 1;
 return n;
}
/*----------------------------------------------------------------*/
int convertRange(int range)
/* convert a ANA_RANGE symbol to a ANA_SUBSC_PTR symbol. */
/* elements:  #1:  range start */
/*            #2:  range end */
/*            #3:  summation flag (0 or 1) */
/*            #4:  redirection flag (symbol, MINUSONE if none) */
/*            for #1 and #2, if the symbol is > 0, then count from the */
/*            start of the list/array; otherwise from one beyond the last */
/*            element of the list/array.  if #2 == ANA_ZERO, then only one */
/*            element is requested. */
{
  int	subsc, eval(int), j1, j2;
  
  if ((subsc = newSymbol(ANA_SUBSC_PTR)) < 0) return -1;
  j1 = range_start(range);
  if (j1 == -ANA_ONE)		/* (*) */
  { subsc_ptr_start(subsc) = 0;
    subsc_ptr_end(subsc) = -1; }
  else 
  { if (j1 >= 0)		/* (X:...) */
    { j2 = int_arg(eval(j1));
      if (j2 < 0)
	return anaerror("Illegal range start", range_start(range)); }
    else			/* (*-X:...) */
    { j2 = -int_arg(eval(-j1));
      if (-j2 <= 0)
	return anaerror("Illegal range start", range_start(range)); }
    subsc_ptr_start(subsc) = j2;

    j1 = range_end(range);
    if (j1 == ANA_ZERO)		/* (X) */
      subsc_ptr_end(subsc) = subsc_ptr_start(subsc);
    else if (j1 == -ANA_ONE)	/* (...:*) */
      subsc_ptr_end(subsc) = -1;
    else
    { if (j1 >= 0)		/* (...:Y) */
      { j2 = int_arg(eval(j1));
	if (j2 < 0)
	  return anaerror("Illegal range end", range_end(range)); }
      else			/* (...:*-Y) */
      { j2 = -int_arg(eval(-j1));
	if (-j2 <= 0)
	  return anaerror("Illegal range end", range_end(range)); }
      subsc_ptr_end(subsc) = j2; }
  }

  if (subsc_ptr_start(subsc) > subsc_ptr_end(subsc) &&
      ((subsc_ptr_start(subsc) >= 0 && subsc_ptr_end(subsc) >= 0) ||
       (subsc_ptr_start(subsc) < 0 && subsc_ptr_end(subsc) < 0)))
  { return anaerror("Range end < range start", range_end(range)); }

  if ((subsc_ptr_sum(subsc) = range_sum(range)) < 0 ||
      subsc_ptr_sum(subsc) > 1)
  { return anaerror("Illegal range summation flag??", range_sum(range)); }
  if (range_redirect(range) == -1) subsc_ptr_redirect(subsc) = -1;
  else if ((subsc_ptr_redirect(subsc) =
	    int_arg(eval(range_redirect(range)))) < 0 ||
	   subsc_ptr_redirect(subsc) >= MAX_DIMS)
  { return anaerror("Illegal range redirection", range_redirect(range)); }
  return subsc;
}
/*----------------------------------------------------------------*/
void convertPointer(scalar *target, int inType, int outType)
/* converts value in target from inType to outType */
{
  switch (outType) {
  case ANA_BYTE:
    switch (inType) {
    case ANA_WORD:
      (*target).b = (byte) (*target).w;
      break;
    case ANA_LONG:
      (*target).b = (byte) (*target).l;
      break;
    case ANA_FLOAT:
      (*target).b = (byte) (*target).f;
      break;
    case ANA_DOUBLE:
      (*target).b = (byte) (*target).d;
      break;
    }
    break;
  case ANA_WORD:
    switch (inType) {
    case ANA_BYTE:
      (*target).w = (word) (*target).b;
      break;
    case ANA_LONG:
      (*target).w = (word) (*target).l;
      break;
    case ANA_FLOAT:
      (*target).w = (word) (*target).f;
      break;
    case ANA_DOUBLE:
      (*target).w = (word) (*target).d;
      break;
    }
    break;
  case ANA_LONG:
    switch (inType) {
    case ANA_BYTE:
      (*target).l = (int) (*target).b;
      break;
    case ANA_WORD:
      (*target).l = (int) (*target).w;
      break;
    case ANA_FLOAT:
      (*target).l = (int) (*target).f;
      break;
    case ANA_DOUBLE:
      (*target).l = (int) (*target).d;
      break;
    }
    break;
  case ANA_FLOAT:
    switch (inType) {
    case ANA_BYTE:
      (*target).f = (float) (*target).b;
      break;
    case ANA_WORD:
      (*target).f = (float) (*target).w;
      break;
    case ANA_LONG:
      (*target).f = (float) (*target).l;
      break;
    case ANA_DOUBLE:
      (*target).f = (float) (*target).d;
      break;
    }
    break;
  case ANA_DOUBLE:
    switch (inType) {
    case ANA_BYTE:
      (*target).d = (double) (*target).b;
      break;
    case ANA_WORD:
      (*target).d = (double) (*target).w;
      break;
    case ANA_LONG:
      (*target).d = (double) (*target).l;
      break;
    case ANA_FLOAT:
      (*target).d = (double) (*target).f;
      break;
    }
    break;
  }
}
/*----------------------------------------------------------------*/
void convertWidePointer(wideScalar *target, int inType, int outType)
/* converts value in <target> from <inType> to <outType> */
{
  switch (inType) {
    case ANA_BYTE:
      switch (outType) {
	case ANA_BYTE:
	  break;
	case ANA_WORD:
	  target->w = (word) target->b;
	  break;
	case ANA_LONG:
	  target->l = (int) target->b;
	  break;
	case ANA_FLOAT:
	  target->f = (float) target->b;
	  break;
	case ANA_DOUBLE:
	  target->d = (double) target->b;
	  break;
	case ANA_CFLOAT:
	  target->cf.real = (float) target->b;
	  target->cf.imaginary = 0.0;
	  break;
	case ANA_CDOUBLE:
	  target->cd.real = (double) target->b;
	  target->cd.imaginary = 0.0;
	  break;
      }
      break;
    case ANA_WORD:
      switch (outType) {
	case ANA_BYTE:
	  target->b = (byte) target->w;
	  break;
	case ANA_WORD:
	  break;
	case ANA_LONG:
	  target->l = (int) target->w;
	  break;
	case ANA_FLOAT:
	  target->f = (float) target->w;
	  break;
	case ANA_DOUBLE:
	  target->d = (double) target->w;
	  break;
	case ANA_CFLOAT:
	  target->cf.real = (float) target->w;
	  target->cf.imaginary = 0.0;
	  break;
	case ANA_CDOUBLE:
	  target->cd.real = (double) target->w;
	  target->cd.imaginary = 0.0;
	  break;
      }
      break;
    case ANA_LONG:
      switch (outType) {
	case ANA_BYTE:
	  target->b = (byte) target->l;
	  break;
	  break;
	case ANA_WORD:
	  target->w = (word) target->l;
	  break;
	case ANA_LONG:
	  break;
	case ANA_FLOAT:
	  target->f = (float) target->l;
	  break;
	case ANA_DOUBLE:
	  target->d = (double) target->l;
	  break;
	case ANA_CFLOAT:
	  target->cf.real = (float) target->l;
	  target->cf.imaginary = 0.0;
	  break;
	case ANA_CDOUBLE:
	  target->cd.real = (double) target->l;
	  target->cd.imaginary = 0.0;
	  break;
      }
      break;
    case ANA_FLOAT:
      switch (outType) {
	case ANA_BYTE:
	  target->b = (byte) target->f;
	  break;
	case ANA_WORD:
	  target->w = (word) target->f;
	  break;
	case ANA_LONG:
	  target->l = (int) target->f;
	  break;
	case ANA_FLOAT:
	  break;
	case ANA_DOUBLE:
	  target->d = (double) target->f;
	  break;
	case ANA_CFLOAT:
	  target->cf.real = (float) target->f;
	  target->cf.imaginary = 0.0;
	  break;
	case ANA_CDOUBLE:
	  target->cd.real = (double) target->f;
	  target->cd.imaginary = 0.0;
	  break;
      }
      break;
    case ANA_DOUBLE:
      switch (outType) {
	case ANA_BYTE:
	  target->b = (byte) target->d;
	  break;
	case ANA_WORD:
	  target->w = (word) target->d;
	  break;
	case ANA_LONG:
	  target->l = (int) target->d;
	  break;
	case ANA_FLOAT:
	  target->f = (float) target->d;
	  break;
	case ANA_DOUBLE:
	  break;
	case ANA_CFLOAT:
	  target->cf.real = (float) target->d;
	  target->cf.imaginary = 0.0;
	  break;
	case ANA_CDOUBLE:
	  target->cd.real = (double) target->d;
	  target->cd.imaginary = 0.0;
	  break;
      }
      break;
    case ANA_CFLOAT:
      switch (outType) {
	case ANA_BYTE:
	  target->b = (byte) target->cf.real;
	  break;
	case ANA_WORD:
	  target->w = (word) target->cf.real;
	  break;
	case ANA_LONG:
	  target->l = (int) target->cf.real;
	  break;
	case ANA_FLOAT:
	  target->f = (float) target->cf.real;
	  break;
	case ANA_DOUBLE:
	  target->d = (double) target->cf.real;
	  break;
	case ANA_CFLOAT:
	  break;
	case ANA_CDOUBLE:
	  target->cd.real = (double) target->cf.real;
	  target->cd.imaginary = (double) target->cf.imaginary;
	  break;
      }
      break;
    case ANA_CDOUBLE:
      switch (outType) {
	case ANA_BYTE:
	  target->b = (byte) target->cd.real;
	  break;
	case ANA_WORD:
	  target->w = (word) target->cd.real;
	  break;
	case ANA_LONG:
	  target->l = (int) target->cd.real;
	  break;
	case ANA_FLOAT:
	  target->f = (float) target->cd.real;
	  break;
	case ANA_DOUBLE:
	  target->d = (double) target->cd.real;
	  break;
	case ANA_CFLOAT:
	  target->cf.real = (float) target->cd.real;
	  target->cf.imaginary = (float) target->cd.imaginary;
	  break;
	case ANA_CDOUBLE:
	  break;
      }
      break;
  }
}
/*----------------------------------------------------------------*/
void convertScalar(scalar *target, int nsym, int type)
/* returns scalar value of nsym, converted to proper type, in target */
{
 int		n;
 pointer	ptr;

 n = scalar_type(nsym);
 ptr.b = &scalar_value(nsym).b;
 switch (type) {
 case ANA_BYTE:
   switch (n) {
   case ANA_BYTE:
     (*target).b = (byte) *ptr.b;
     break;
   case ANA_WORD:
     (*target).b = (byte) *ptr.w;
     break;
   case ANA_LONG:
     (*target).b = (byte) *ptr.l;
     break;
   case ANA_FLOAT:
     (*target).b = (byte) *ptr.f;
     break;
   case ANA_DOUBLE:
     (*target).b = (byte) *ptr.d;
     break;
   }
   break;
 case ANA_WORD:
   switch (n) {
   case ANA_BYTE:
     (*target).w = (word) *ptr.b;
     break;
   case ANA_WORD:
     (*target).w = (word) *ptr.w;
     break;
   case ANA_LONG:
     (*target).w = (word) *ptr.l;
     break;
   case ANA_FLOAT:
     (*target).w = (word) *ptr.f;
     break;
   case ANA_DOUBLE:
     (*target).w = (word) *ptr.d;
     break;
   }
   break;
 case ANA_LONG:
   switch (n) {
   case ANA_BYTE:
     (*target).l = (int) *ptr.b;
     break;
   case ANA_WORD:
     (*target).l = (int) *ptr.w;
     break;
   case ANA_LONG:
     (*target).l = (int) *ptr.l;
     break;
   case ANA_FLOAT:
     (*target).l = (int) *ptr.f;
     break;
   case ANA_DOUBLE:
     (*target).l = (int) *ptr.d;
     break;
   }
   break;
 case ANA_FLOAT:
   switch (n) {
   case ANA_BYTE:
     (*target).f = (float) *ptr.b;
     break;
   case ANA_WORD:
     (*target).f = (float) *ptr.w;
     break;
   case ANA_LONG:
     (*target).f = (float) *ptr.l;
     break;
   case ANA_FLOAT:
     (*target).f = (float) *ptr.f;
     break;
   case ANA_DOUBLE:
     (*target).f = (float) *ptr.d;
     break;
   }
   break;
 case ANA_DOUBLE:
   switch (n) {
   case ANA_BYTE:
     (*target).d = (double) *ptr.b;
     break;
   case ANA_WORD:
     (*target).d = (double) *ptr.w;
     break;
   case ANA_LONG:
     (*target).d = (double) *ptr.l;
     break;
   case ANA_FLOAT:
     (*target).d = (double) *ptr.f;
     break;
   case ANA_DOUBLE:
     (*target).d = (double) *ptr.d;
     break;
   }
   break;
 }
}
/*----------------------------------------------------------------*/
int ana_symbol_memory()
/* returns the total of the memory allocated for each ANA symbol */
/* - which is NOT the same as the total allocated memory. */
/* Note:  some small stuff is not included. */
{
 int	i, mem = 0;

 for (i = 0; i < NSYM; i++)
 { switch (sym[i].class)
   { case ANA_EVB:
       switch (sym[i].type)
       { default:
	   break;
	 case EVB_CASE: case EVB_NCASE: case EVB_BLOCK: case EVB_INT_SUB:
	 case EVB_USR_SUB: case EVB_INSERT:
	   mem += symbol_memory(i);  break; }
       break;
     case ANA_STRING: case ANA_LIST: case ANA_SUBSC_PTR: case ANA_INT_FUNC:
     case ANA_USR_FUNC: case ANA_ARRAY:
       mem += symbol_memory(i);  break;
     case ANA_LIST_PTR:
       if (list_ptr_target(i) > 0)
	 mem += strlen(list_ptr_tag_string(i)) + 1;
       break;
     case ANA_SUBROUTINE: case ANA_FUNCTION: case ANA_BLOCKROUTINE:
       mem += routine_num_parameters(i)*(sizeof(char *) + sizeof(word))
	 + routine_num_statements(i)*sizeof(word);
       break; }
 }
 i = scalar_scratch(ANA_LONG);
 scalar_value(i).l = mem;
 return i;
}
/*----------------------------------------------------------------*/
int ana_trace(int narg, int ps[])
/* activates/deactivates trace facility */
{
  extern int	internalMode;
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
    return anaerror("Negative Trace Level", ps[0]);
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
#define b_fix(name, value) { fixedValue(name, ANA_BYTE, value);  nFixed++; }
#define l_fix(name, value) { fixedValue(name, ANA_LONG, value);  nFixed++; }
#define f_fix(name, value) { fixedValue(name, ANA_FLOAT, value);  nFixed++; }
#define d_fix(name, value) { fixedValue(name, ANA_DOUBLE, value);  nFixed++; }
#define s_fix(name, value) { fixedValue(name, ANA_LSTRING, value);  nFixed++; }
#define cf_fix(name, re, im) { fixedValue(name, ANA_CFLOAT, re, im); nFixed++; }
#define cd_fix(name, re, im) { fixedValue(name, ANA_DFLOAT, re, im); nFixed++; }
#define l_ptr(name, value) installPointer(name, ANA_LONG, value)
#define f_ptr(name, value) installPointer(name, ANA_FLOAT, value)
#define d_ptr(name, value) installPointer(name, ANA_DOUBLE, value)
#define s_ptr(name, value) installPointer(name, ANA_TEMP_STRING, value)
#define fnc_p(name, value) installSysFunc(name, value)

char	*defaultRedirect = "diagnostic.ana";

int range_warn_flag = 0, redim_warn_flag = 0, error_extra = 0,
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
extern int	lunplt, landscape, iorder, ilabx, ilaby, irxf, iryf, ndx,
        ndxs, nd, ndys, ifz, ifzx, ier, ifont, ndlabx, ndlaby, 
        ndot, ipltyp, iblank, maxregridsize, nExecuted,
        kb, nArg, ixhigh, iyhigh,
        tvsmt, badmatch, fstepx, fstepy,
        sort_flag, crunch_bits, crunch_slice, byte_count,
	current_pen, updateBoundingBox, index_cnt, uTermCol, page;
extern double	meritc;
#if MOTIF
extern int	radio_state, radio_button;
#endif
extern float	xfac, yfac, xmin, xmax, ymin, ymax,
	wxb, wxt, wyb, wyt, ticx, ticxr, ticy, ticyr, plims[],
	fsized,	symsize, symratio, startx, starty, stepx, stepy,
	callig_xb, callig_yb, callig_ratio, slabx, slaby,
        dashsize, crunch_bpp, postXBot, postXTop,
	postYBot, postYTop, xerrsize, yerrsize;
extern word	*stackPointer;

#if DEVELOP
extern int	irzf, ifzz, ndz, ndzs, resample_type, fstepz;
extern float	wzb, wzt, ticz, ticzr, zmin, zmax, defaultProjection[], dvz;
#endif

#if X11
extern int	text_menus, tvplanezoom;
#endif

#if X11
extern int ana_button, eventSource, xcoord, ycoord, ana_keycode, ana_keysym,
  last_menu, menu_item, ana_event, preventEventFlush, root_x, root_y,
  xerrors, last_wid, display_width, display_height, private_colormap,
  zoom_frame, foreground_pixel, nColors, colormin, colormax, ana_keystate;

extern float	tviy, tviyb, tvix, tvixb, xhair, yhair, menu_x, menu_y,
		tvscale, zoom_xc, zoom_yc, zoom_mag, lumpx;
extern double	last_time, zoom_clo, zoom_chi;
#endif

#if MOTIF
extern int	motif_flag;
#endif

char	*firstbreak;		/* for memck.c */

enumElem	classesStruct[] = {
  { "SCALAR", ANA_SCALAR },
  { "STRING", ANA_STRING },
  { "RANGE", ANA_RANGE },
  { "ARRAY", ANA_ARRAY },
  { "POINTER", ANA_POINTER },
  { "ASSOC", ANA_ASSOC },
  { "FUNC_PTR", ANA_FUNC_PTR },
  { "SCAL_PTR", ANA_SCAL_PTR },
  { "SUBSC_PTR", ANA_SUBSC_PTR },
  { "FILEMAP", ANA_FILEMAP },
  { "CLIST", ANA_CLIST },
  { "LIST", ANA_LIST },
  { "KEYWORD", ANA_KEYWORD },
  { "LIST_PTR", ANA_LIST_PTR },
  { "CPLIST", ANA_CPLIST },
  { "SUBROUTINE", ANA_SUBROUTINE },
  { "FUNCTION", ANA_FUNCTION },
  { "BLOCKROUTINE", ANA_BLOCKROUTINE },
  { "EVB", ANA_EVB },
  { "UNDEFINED", ANA_UNDEFINED }
};

enumElem	typesStruct[] = {
  { "BYTE", ANA_BYTE },
  { "WORD", ANA_WORD },
  { "LONG", ANA_LONG },
  { "FLOAT", ANA_FLOAT },
  { "DOUBLE", ANA_DOUBLE },
  { "STRING", ANA_TEMP_STRING },
  { "CFLOAT", ANA_CFLOAT },
  { "CDOUBLE", ANA_CDOUBLE },
  { "UNDEFINED", ANA_UNDEFINED }
};

enumElem	eventStruct[] =	{ /* see ana_register_event in menu.c */
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

struct boundsStruct	bounds = {
 { 0, SHRT_MIN, INT_MIN, -FLT_MAX, -DBL_MAX},
 { UCHAR_MAX, SHRT_MAX, INT_MAX, FLT_MAX, DBL_MAX }
};

int	ANA_MATMUL_FUN;

#define	FORMATSIZE	1024
void symbolInitialization(void)
{
 int	i, iq;
#if YYDEBUG
 extern int	yydebug;
#endif
 extern int	termRow, termCol, despike_count;
#if DEVELOP
 char	*p;
#endif
 int	to_scratch_array(int, int, int, int []);
 extern char	*fmt_integer, *fmt_float, *fmt_string, *fmt_complex,
  *curScrat, *printString, ANAversion[];
 union { byte b[2]; word w; } whichendian;

 /* determine if the machine is little-endian or bigendian */
 whichendian.w = 1;
 MSBfirst = (int) whichendian.b[1]; /* most significant byte first? */

 firstbreak = sbrk(0);		/* for memck.c */
 curTEIndex = tempExecutableIndex;
 if (signal(SIGFPE, exception) == SIG_ERR
     || signal(SIGINT, exception) == SIG_ERR
     || signal(SIGSEGV, exception) == SIG_ERR
     || signal(SIGCONT, exception) == SIG_ERR
     || signal(SIGTRAP, exception) == SIG_ERR)
   anaerror("Could not install exception handlers", 0);
 for (i = 0; i < nSubroutine; i++)
   installKeys(&subroutine[i].keys);
 for (i = 0; i < nFunction; i++)
   installKeys(&function[i].keys);
 ANA_MATMUL_FUN = findInternalName("MATMUL", 0);
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
 f_fix("#MAX_FLOAT", 	bounds.max.f);
 d_fix("#MAX_DOUBLE", 	bounds.max.d);
 l_fix("#MIN_WORD",	bounds.min.w);
 l_fix("#MIN_LONG",	bounds.min.l);
 f_fix("#MIN_FLOAT", 	FLT_MIN);
 d_fix("#MIN_DOUBLE", 	DBL_MIN);
 l_fix("#MAX_DIMS",	MAX_DIMS);
#ifdef X11
 l_fix("#NUM_WINDOWS",  MAXWINDOWS - 1);
 l_fix("#NUM_PIXMAPS",  MAXPIXMAPS - 1);
#endif
 d_fix("#PI",		M_PI);
 d_fix("#2PI",		2*M_PI);
 f_fix("#E",		2.718281828);
 f_fix("#C",		2.997929E10);
 f_fix("#G",		6.668E-8);
 f_fix("#H",		6.6252E-27);
 f_fix("#HB",		1.0544E-27);
 f_fix("#EC",		6.6252E-27);
 f_fix("#M",		9.1084E-28);
 f_fix("#K",		1.308046E-16);
 f_fix("#R",		8.317E7); 
 d_fix("#RAD",		RAD);
 r_d_sym = nFixed;
 f_fix("#R.D",		RAD);
 d_fix("#DEG",		DEG);
 d_r_sym = nFixed;
 f_fix("#D.R",		DEG);

 iq = installString("#CLASS");
 stackSym = findVar(iq, 0);	/* stackSym is a dummy variable */
 sym[stackSym].class = ANA_ENUM;
 enum_type(stackSym) = ANA_LONG;
 enum_list(stackSym) = classesStruct;
 symbol_memory(stackSym) = sizeof(classesStruct);
 nFixed++;

 iq = installString("#COORDSYS");
 stackSym = findVar(iq, 0);
 symbol_class(stackSym) = ANA_ENUM;
 enum_type(stackSym) = ANA_LONG;
 enum_list(stackSym) = coordSysStruct;
 symbol_memory(stackSym) = sizeof(coordSysStruct);
 nFixed++;

 iq = installString("#EVENT");
 stackSym = findVar(iq, 0);
 sym[stackSym].class = ANA_ENUM;
 enum_type(stackSym) = ANA_LONG;
 enum_list(stackSym) = eventStruct;
 symbol_memory(stackSym) = sizeof(eventStruct);
 nFixed++;

 iq = installString("#FILETYPE");
 stackSym = findVar(iq, 0);
 sym[stackSym].class = ANA_ENUM;
 enum_type(stackSym) = ANA_LONG;
 enum_list(stackSym) = filetypeStruct;
 symbol_memory(stackSym) = sizeof(filetypeStruct);
 nFixed++;

 iq = installString("#TYPE");
 stackSym = findVar(iq, 0);
 sym[stackSym].class = ANA_ENUM;
 enum_type(stackSym) = ANA_LONG;
 enum_list(stackSym) = typesStruct;
 symbol_memory(stackSym) = sizeof(typesStruct);
 nFixed++;
 
#if DEVELOP
 iq = installString("#P3D");
 projectSym = findVar(iq, 0);
 sym[projectSym].class = ANA_ARRAY;
 array_type(projectSym) = ANA_FLOAT;
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
 sym[stackSym].class = ANA_CLIST;
 clist_symbols(stackSym) = stackPointer;
 symbol_memory(stackSym) = 0;	/* or it will get deallocated sometime */
 nFixed++;

 iq = findVarName("#TYPESIZE", 0);
 i = 10;			/* ana_type_size[] # elements! */
 to_scratch_array(iq, ANA_LONG, 1, &i);
 memcpy(array_data(iq), ana_type_size, i*sizeof(int));

 /* s_fix("#NL",		"\n"); */
 l_ptr("#COL",		&termCol);
 l_ptr("#ROW",		&termRow);

 l_ptr("#MSBFIRST",	&MSBfirst);
 s_ptr("#VERSION",	ANAversion);

 l_ptr("!AREA_DIAG",	&area_diag);
 l_ptr("!AUTOCON", 	&autocon);
 l_ptr("!BADMATCH",	&badmatch);
 l_ptr("!BB_UPDATE",	&updateBoundingBox);
 l_ptr("!BC",		&byte_count);
#if X11
 l_ptr("!BUTTON",	&ana_button);
#endif
 f_ptr("!BXB",		&postXBot);
 f_ptr("!BXT",		&postXTop);
 f_ptr("!BYB",		&postYBot);
 f_ptr("!BYT",		&postYTop);
 l_ptr("!COL",		&uTermCol);
#if X11
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
#if X11
 l_ptr("!EVENTSOURCE",	&eventSource);
 l_ptr("!EVENTTYPE",	&ana_event);
#endif
 l_ptr("!FFTDP", 	&fftdp);
 l_ptr("!FONT",		&ifont);
#if X11
 l_ptr("!FOREGROUND_COLOR",	&foreground_pixel);
#endif
 fformat = s_ptr("!FORMAT_F", strsave("%14.7g"));
 iformat = s_ptr("!FORMAT_I", strsave("%10d"));
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
#if X11
 l_ptr("!IX",		&xcoord);
#endif
 l_ptr("!IXHIGH",	&ixhigh);
#if X11
 l_ptr("!IY",		&ycoord);
#endif
 l_ptr("!IYHIGH",	&iyhigh);
 fnc_p("!JD",		11);
#if X11			/* a non-X11 version of this is needed */
 l_ptr("!KB",		&kb);
 l_ptr("!KEYCODE",	&ana_keycode);
 l_ptr("!KEYSTATE",	&ana_keystate);
 l_ptr("!KEYSYM",	&ana_keysym);
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
#if X11
 l_ptr("!LAST_MENU",	&last_menu);
 f_ptr("!LUMPX",	&lumpx);
#endif
 l_ptr("!MAXHISTSIZE", 	&maxhistsize);
 l_ptr("!MAXREGRIDSIZE", &maxregridsize);
#if X11
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
#if X11
 l_ptr("!NCOLORCELLS",	&nColors);
#endif
 l_ptr("!NEXECUTED",	&nExecuted);
#if X11
 l_ptr("!NOEVENTFLUSH",	&preventEventFlush);
#endif
 l_ptr("!PDEV", 	&lunplt);
 l_ptr("!PEN",		&current_pen);
 l_ptr("!PLTYP",	&ipltyp);
 f_ptr("!PLXERRB",	&xerrsize);
 f_ptr("!PLYERRB",	&yerrsize);
#if X11
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
#if X11
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
#if X11
 l_ptr("!SCREEN_HEIGHT", &display_height);
 l_ptr("!SCREEN_WIDTH",	&display_width);
#endif
#endif
 l_ptr("!SORT_FLAG",	&sort_flag);
 f_ptr("!STARTX",	&startx);
 f_ptr("!STARTY",	&starty);
 f_ptr("!STEPX",	&stepx);
 f_ptr("!STEPY",	&stepy);
 f_ptr("!SYMRATIO",	&symratio);
 f_ptr("!SYMSIZE",	&symsize);
 fnc_p("!SYSTIME",	10);
#if X11
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
#if X11
 f_ptr("!TVIX",		&tvix);
 f_ptr("!TVIXB",	&tvixb);
 f_ptr("!TVIY",		&tviy);
 f_ptr("!TVIYB",	&tviyb);
 l_ptr("!TVPLANEZOOM",	&tvplanezoom); /* browser */
#endif
#if X11
 f_ptr("!TVSCALE",	&tvscale);
#endif
 l_ptr("!TVSMT",	&tvsmt);
 l_ptr("!PROJECT",	&projectTk);
 f_ptr("!WIDTH",	&xfac);
#if X11
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
#if X11
 l_ptr("!XERRORS",	&xerrors);
 f_ptr("!XF",		&xhair);
 f_ptr("!XMENU",	&menu_x);
#endif
 f_ptr("!XT",		&plims[1]);
#if X11
 d_ptr("!XTIME",	&last_time);
#endif
 f_ptr("!YB",		&plims[2]);
 f_ptr("!YC",		&callig_yb);
#if X11
 f_ptr("!YF",		&yhair);
 f_ptr("!YMENU",	&menu_y);
#endif
 f_ptr("!YT",		&plims[3]);
#if YYDEBUG
 l_ptr("!YYDEBUG",	&yydebug);
#endif
 f_ptr("!ZB",		&plims[4]);
#if X11
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
 sym[tempSym].class = ANA_SCALAR;
 scalar_type(tempSym) = ANA_LONG;
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
int matchInternalName(char *name, internalRoutine *table, int size, int hi)
/* matches name against the initial parts of all names in the table.
   returns index of first match (i.e., closest to the start of the table),
   or -1 if none were found.  LS97 */
{
 int	lo = 0, mid, s;

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
void zerobytes(void *sp, int len)
/* zeros <len> bytes starting at <sp> */
{
  char	*p;

  p = (char *) sp;
  while (len--) *p++ = '\0';
}
/*----------------------------------------------------------------*/
char *strcasestr(char *s1, char *s2)
/* locates the first occurrence of string <s2> in string <s1> and returns
   a pointer to the located string, or NULL if string <s1> is not found.
   If <s2> is an empty string, then returns <s1>.  Case is disregarded.
   LS 7jun95 */
{
  char	*p1, *p2;

  if (!s2) return s1;
  do
				/* find first char of <s2> in <s1> */
  { while (*s1 && toupper(*s1) != toupper(*s2)) s1++;
    if (!*s1) return NULL;	/* not found */
    p1 = s1;  p2 = s2;
    while (*p2 && toupper(*p1) == toupper(*p2)) /* how far do they match? */
    { p1++;  p2++; }
    if (!*p2) return s1;	/* all the way */
    s1++;
  } while (1);
}
/*----------------------------------------------------------------*/
int strncasecmp_p(char *s1, char *s2, int n)
/* compares the first <n> bytes of strings <s1> and <s2> and returns 0 */
/* if they are equal in both strings, a number > 0 if <s2> is later */
/* that <s1> in the internal character set, or < 0 otherwise. */
/* LS 17feb97 */
{
  char	c1, c2;
  int	i = 0;

  do
  { c1 = toupper(*s1++);
    c2 = toupper(*s2++);
    i++; }
  while (c1 == c2 && c1 != '\0' && c2 != '\0' && i < n);
  return c2 - c1;
}
/*----------------------------------------------------------------*/
int strcasecmp_p(char *s1, char *s2)
/* compares strings <s1> and <s2> without regard to case and returns 0 */
/* if they are equal, a number > 0 if <s2> is later */
/* that <s1> in the internal character set, or < 0 otherwise. */
/* LS 21feb97 */
{
  int	c1, c2;

  do
  { c1 = toupper(*s1++);
    c2 = toupper(*s2++); }
  while (c1 == c2 && c1 != '\0' && c2 != '\0');
  return c2 - c1;
}
/*----------------------------------------------------------------*/
int	nBreakpoint = 0;
breakpointInfo	breakpoint[NBREAKPOINTS];
int ana_breakpoint(int narg, int ps[])
/* BREAKPOINT,string[,/SET,/VARIABLE] */
/* BREAKPOINT,n[,/DISABLE,/ENABLE,/DELETE] */
/* BREAKPOINT,/LIST */
/* /LIST can be specified together with one of the other switches */
{
  static int	curBreakpoint = 0;
  char	*s, *p;
  int	n;
  
  if (narg) 
    switch (internalMode & 3) {
      case 0:
	switch (symbol_class(ps[0])) {
	  case ANA_STRING:	/* /SET */
	    s = string_arg(ps[0]);
	    if (!s)		/* empty string */
	      return -1;
	    if (nBreakpoint == NBREAKPOINTS) {
	      printf("Maximum number of breakpoints (%1d) has been reached\n",
		     NBREAKPOINTS);
	      return anaerror("New breakpoint has been rejected.", ps[0]);
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
	      if (!isdigit((byte) *p))
		return
		  anaerror("Illegal breakpoint line number specification (%s)",
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
	  case ANA_SCALAR:		/* /ENABLE */
	    n = int_arg(ps[0]);

	    if (n < 0 || n >= NBREAKPOINTS)
	      return anaerror("Illegal breakpoint number", ps[0]);
	    if (!(breakpoint[n].status & 1))
	      return anaerror("Non-existent breakpoint", ps[0]);
	    breakpoint[n].status |= BP_ENABLED; /* enable */
	    break;
	}
	break;
      case 1:			/* /ENABLE */
	n = int_arg(ps[0]);
	if (n < 0 || n >= NBREAKPOINTS)
	  return anaerror("Illegal breakpoint number", ps[0]);
	if (!(breakpoint[n].status & BP_DEFINED))
	  return anaerror("Non-existent breakpoint", ps[0]);
	breakpoint[n].status |= BP_ENABLED; /* enable */
	break;
      case 2:			/* /DISABLE */
	n = int_arg(ps[0]);
	if (n < 0 || n >= NBREAKPOINTS)
	  return anaerror("Illegal breakpoint number", ps[0]);
	if (!(breakpoint[n].status & BP_DEFINED))
	  return anaerror("Non-existent breakpoint", ps[0]);
	breakpoint[n].status &= ~BP_ENABLED; /* disable */
	break;
      case 3:			/* /DELETE */
	n = int_arg(ps[0]);
	if (n < 0 || n >= NBREAKPOINTS)
	  return anaerror("Illegal breakpoint number", ps[0]);
	if (!(breakpoint[n].status & BP_DEFINED))
	  return anaerror("Non-existent breakpoint", ps[0]);
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
word	watchVars[NWATCHVARS];
int	nWatchVars = 0;
int ana_watch(int narg, int ps[])
/* WATCH,<variable>[,/DELETE,/LIST] */
{
  static int	curWatchVar = 0;
  int	i;

  if (narg) {
    if (!symbolIsNamed(ps[0]))
      return anaerror("Need a named variable", ps[0]);
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
	return anaerror("Maximum number of watched variables is already reached",
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
  return ANA_OK;
}
/*----------------------------------------------------------------*/
int ana_symbol_number(int narg, int ps[])
     /* returns the symbol number of the argument */
{
  int	result;
  
  result = scalar_scratch(ANA_LONG);
  sym[result].spec.scalar.l = *ps;
  return result;
}
/*----------------------------------------------------------------*/
void mark(int symbol)
{
  if (markIndex == MSSIZE - 1)
  { anaerror("mark: WARNING - Too many temps marked", symbol);
    return; }
  markStack[markIndex++] = symbol;
}
/*----------------------------------------------------------------*/
void pegMark(void)
{
  if (markIndex == MSSIZE - 1)
  { anaerror("pegMark: WARNING - Too many temps marked", -1);
    return; }
  markStack[markIndex++] = -1;
}
/*----------------------------------------------------------------*/
void pegParse(void)
{
  if (markIndex == MSSIZE - 1)
  { anaerror("pegParse: WARNING - Too many temps marked", -1);
    return; }
  markStack[markIndex++] = -2;
}
/*----------------------------------------------------------------*/
void zapParseTemps(void)
{
  int	iq;
  
  while (markIndex > 0 && (iq = markStack[--markIndex]) >= 0)
    zapTemp(iq);
  markIndex++;			/* retain -2 on mark stack */
  if (iq != -2)
  { anaerror("zapParseTemps: WARNING - Not at parse level", -1);
    return; }
}
/*----------------------------------------------------------------*/
void removeParseMarker(void)
{
  if (markIndex < 1 || markStack[markIndex - 1] != -2)
  { anaerror("removeParseMarker: WARNING - Not at parse level", -1);
    return; }
  markIndex--;
}
/*----------------------------------------------------------------*/
void unMark(int symbol)
{
  int	i;

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
  int	iq;
  
  while (markIndex > 0 && (iq = markStack[--markIndex]) >= 0)
    zapTemp(iq);
}
/*----------------------------------------------------------------*/
void checkTemps(void)
/* for debugging: checks that the number of temporary (unnamed) */
/* variables is equal to what is expected. */
{
  int	i, n;
  extern int	nTempVariable;

  n = 0;
  for (i = TEMPS_START; i < TEMPS_END; i++)
    if (symbol_class(i) != ANA_UNUSED)
      n++;
  if (n != nTempVariable)
    printf("WARNING - %1d temps expected, %1d found\n",
	   nTempVariable, n);
  n = 0;
  for (i = TEMP_EXE_START; i < TEMP_EXE_END; i++)
    if (symbol_class(i) != ANA_UNUSED)
      n++;
  if (n != nTempExecutable)
    printf("WARNING - %1d temp executables expected, %1d found\n",
	   nTempExecutable, n);
}
/*----------------------------------------------------------------*/
#include <unistd.h>
int ana_restart(int narg, int ps[])
{
  extern char	*programName;
  int	saveHistory(void);

  printf("\nRestarting ANA...\n\n");
  saveHistory();
  execl(programName, programName, NULL);
  return 1;
}
/*----------------------------------------------------------------*/
int strccmp(char *s1, char *s2)
/* checks strings s1 and s2 for equality disregarding upper/lower case */
/* distinctions */
{
  while (*s1 && toupper(*s1++) == toupper(*s2++)) ;
  return *s1 - *s2;
}
/*----------------------------------------------------------------*/
int structSize(int symbol, int *nstruct, int *nbyte)
/* returns in <*nstruct> the number of structure descriptors that are
   required to describe <symbol>, and in <*nbyte> the total number of bytes
   covered by the data in <symbol>. */
{
  int	n, ns, nb;
  pointer	p;
  listElem	*l;

  switch (symbol_class(symbol)) {
    case ANA_SCALAR: case ANA_CSCALAR:
      *nbyte = ana_type_size[symbol_type(symbol)];
      *nstruct = 1;
      return 1;
    case ANA_STRING:
      *nbyte = string_size(symbol);
      *nstruct = 1;
      return 1;
    case ANA_ARRAY: case ANA_CARRAY:
      *nbyte = array_size(symbol)*ana_type_size[array_type(symbol)];
      *nstruct = 1;
      return 1;
    case ANA_CLIST:
      p.w = clist_symbols(symbol);
      n = clist_num_symbols(symbol);
      *nbyte = 0;
      *nstruct = 1;		/* one extra for the struct info */
      while (n--) {
	if (structSize(*p.w++, &ns, &nb) == ANA_ERROR)
	  return ANA_ERROR;
	*nbyte += nb;
	*nstruct += ns;
      }
      return 1;
    case ANA_LIST:
      l = list_symbols(symbol);
      n = list_num_symbols(symbol);
      *nbyte = 0;
      *nstruct = 1;		/* one extra for the struct info */
      while (n--) {
	if (structSize(l++->value, &ns, &nb) == ANA_ERROR)
	  return ANA_ERROR;
	*nbyte += nb;
	*nstruct += ns;
      }
      return 1;
    case ANA_STRUCT:
      *nstruct = struct_num_all_elements(symbol);
      *nbyte = struct_total_size(symbol);
      return 1;
    default:
      puts("In structSize():");
      return cerror(ILL_CLASS, symbol);
  }
}
/*----------------------------------------------------------------*/
int makeStruct(int symbol, char *tag, structElem **se, char *data,
	       int *offset, int descend)
{
  int	size, offset0, ndim, n;
  structElem	*se0;
  word	*arg;
  listElem	*le;

  if (descend) {
    return ANA_OK;
  } else {
    (*se)->u.regular.tag = tag? strsave(tag): NULL;
    (*se)->u.regular.offset = *offset; /* byte offset from start */
    switch (symbol_class(symbol)) {
      case ANA_SCALAR: case ANA_CSCALAR:
	(*se)->u.regular.type = scalar_type(symbol); /* data type */
	(*se)->u.regular.spec.singular.ndim = 0; /* 0 -> scalar */
	/* copy the value into the structure */
	size = ana_type_size[scalar_type(symbol)];/* bytes per value */
	memcpy(data + *offset, &scalar_value(symbol).b, size);
	break;
      case ANA_STRING:
	(*se)->u.regular.type = ANA_TEMP_STRING; /* data type */
	(*se)->u.regular.spec.singular.ndim = 1; /* strings always have 1 */
	if (!((*se)->u.regular.spec.singular.dims = malloc(sizeof(int))))
	  return cerror(ALLOC_ERR, 0);
	size = string_size(symbol); /* bytes per value */
	(*se)->u.regular.spec.singular.dims[0] = size; /* first dimension */
	memcpy(data + *offset, string_value(symbol), size); /* copy value */
	break;
      case ANA_ARRAY: case ANA_CARRAY:
	(*se)->u.regular.type = array_type(symbol);
	ndim = array_num_dims(symbol);
	if (array_type(symbol) == ANA_STRING_ARRAY)
	  ndim++;		/* add one for string arrays to hold the */
				/* length of the strings */
	(*se)->u.regular.spec.singular.ndim = ndim;
	if (!((*se)->u.regular.spec.singular.dims = malloc(ndim*sizeof(int))))
	  return cerror(ALLOC_ERR, 0);
	if (array_type(symbol) == ANA_STRING_ARRAY) {
	  (*se)->u.regular.spec.singular.dims[0] =
	    strlen(*(char **) array_data(symbol)); /* take length of first */
						   /* one for all */
	  memcpy((*se)->u.regular.spec.singular.dims + 1, 
		 array_dims(symbol), array_num_dims(symbol)*sizeof(int));
	} else
	  memcpy((*se)->u.regular.spec.singular.dims,
		 array_dims(symbol), array_num_dims(symbol)*sizeof(int));
	size = ana_type_size[array_type(symbol)]*array_size(symbol);
	memcpy(data + *offset, array_data(symbol), size); /* copy values */
	break;
      case ANA_CLIST:
	arg = clist_symbols(symbol);
	n = clist_num_symbols(symbol);
	offset0 = *offset;
	se0 = *se;
	while (n--)
	  if (makeStruct(*arg++, NULL, se, data, offset, 0) == ANA_ERROR)
	    return ANA_ERROR;
	arg = clist_symbols(symbol);
	n = clist_num_symbols(symbol);
	*offset = offset0;
	*se = se0;
	while (n--)
	  makeStruct(*arg++, NULL, se, data, offset, 1);
	break;
      case ANA_LIST:
	le = list_symbols(symbol);
	n = list_num_symbols(symbol);
	offset0 = *offset;
	se0 = *se;
	while (n--) {
	  if (makeStruct(le->value, le->key, se, data, offset, 0) == ANA_ERROR)
	    return ANA_ERROR;
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
      case ANA_STRUCT:
	/* CONTINUE HERE */
	break;
    }
    *offset += size;
    (*se)++;
  }
  return ANA_OK;
}
/*----------------------------------------------------------------*/
int ana_struct(int narg, int ps[])
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
  int	result, size, nstruct, dims[MAX_DIMS], ndim, n, i, offset;
  word	*arg;
  listElem	*le;
  pointer	data;
  structElem	*se, *se0;
  
  if (structSize(ps[0], &nstruct, &size) == ANA_ERROR) /* check
							  specification */
    return ANA_ERROR;
  nstruct++;			/* one extra for the top-level description */
  ndim = narg - 1;
  if (get_dims(&ndim, ps + 1, dims) == ANA_ERROR) /* read dimensions */
    return ANA_ERROR;
  /* calculate the number of repetitions of the outer structure */
  n = 1;
  for (i = 0; i < ndim; i++)
    n *= dims[i];

  result = nextFreeTempVariable();
  if (result == ANA_ERROR)
    return ANA_ERROR;
  symbol_class(result) = ANA_STRUCT;
  symbol_memory(result) = sizeof(int) /* to store the number of elements */
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
    case ANA_SCALAR: case ANA_CSCALAR: case ANA_STRING: case ANA_ARRAY:
    case ANA_CARRAY:
      n = 1;
      break;
    case ANA_CLIST:
      n = clist_num_symbols(ps[0]);
      break;
    case ANA_LIST:
      n = list_num_symbols(ps[0]);
      break;
    case ANA_STRUCT:
      n = struct_num_top_elements(ps[0]);
      break;
  }  
  se->u.first.nelem = n;
  se->u.first.size = size;
  se->u.first.ndim = ndim;
  if (!(se->u.first.dims = malloc(ndim*sizeof(int))))
    return cerror(ALLOC_ERR, 0);
  memcpy(se->u.first.dims, dims, ndim*sizeof(int));
  se++;				/* point at the next one */
  offset = 0;

  /* now recursively fill in the deeper ones */
  if (makeStruct(ps[0], NULL, &se, data.v, &offset, 0) == ANA_ERROR)
    return ANA_ERROR;
  return result;
}
/*----------------------------------------------------------------*/
int ana_buffering(int narg, int ps[])
/* BUFFERING [, <type>, /LINE, /CHAR ]
 shows or sets the kind of input buffering for ANA.
 no arguments -> show current setting
 <type> == 2 or /PIPE -> by line & no prompts
 <type> == 1 or /LINE -> by line
 <type> == 0 or /CHAR -> by char */
{
  extern int	buffering, noPrompt;
  extern char	*programName;
  int	newBuf;

  if (!narg && !internalMode) {
    printf("%s currently reads %sbuffered input.\n", programName,
	   buffering? "": "un");
    return ANA_OK;
  }
  if (narg)
    newBuf = int_arg(ps[0]);
  else
    newBuf = internalMode;
  if (newBuf < 0 || newBuf > 2)
    return anaerror("Illegal buffering type specification", 0);
  noPrompt = (newBuf == 2);
  newBuf &= ~2;
  if (newBuf ^ buffering) {
    if (newBuf)
      cookedIo();
    else
      rawIo();
  }
  printf("%s now reads %sbuffered input.\n", programName,
	 buffering? "": "un");
  return ANA_OK;
}
/*----------------------------------------------------------------*/
int translateEscapes(char *p)
/* replace explicit escape sequences \x by internal ones; returns */
/* the final length of the string */
{
  char	escapechars[] = "ntvbrfa\\?'\"", escapes[] = "\n\t\v\b\r\f\a\\?'\"",
    *p2, *p0;
  int	i, c;

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
      } else if (isdigit((byte) p[1]) && p[1] < '8') { /* an octal number */
	/* octal-number escape sequences have at most 3 octal digits.
	   we cannot rely on strtol because it may find more than 3
	   (e.g., when the user specifies '\000123' the \000 is an octal
	   specification and the 123 is regular text; strtol would
	   return 000123 as the number.  we must find the end of the
	   octal number manually. */
	p2 = p + 2;		/* just beyond the first octal digit */
	for (i = 2; i < 4; i++)
	  if (isdigit((byte) *p2) && *p2 < '8')	/* an octal digit */
	    p2++;
	c = *p2;		/* temporary storage */
	*p2 = '\0';		/* temporary end to force strtol not to
				   read beyond the first three octal digits */
	i = strtol(p + 1, NULL, 8);
	*p2 = c;
	*p = i;
	memcpy(p + 1, p2, strlen(p2) + 1);
      }
    }
    p++;
  }
  return p - p0;
}
/*----------------------------------------------------------------*/
int installString(char *string)
/* installs string in symbol stack; returns index to stack */
{
 int	index, n;
 char	*p, *p0;
#if YYDEBUG
 extern int	yydebug;
#endif

 if ((index = nextFreeStackEntry()) == ANA_ERROR)
   return ANA_ERROR;		/* error */
 n = strlen(string) + 1;
 p = malloc(n);
 if (!p)
   return anaerror("Could not allocate %d bytes for string %s", 0, n, string);
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
 ANA symbol and would otherwise show up as an error in checkList.
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
 int	n = 1, i;

 if (!*(char **) keys)		/* empty key */
   return;
 /* ANSI C does not allow string constants to be modified, so we */
 /* make a copy of the string and modify that */
 if (!(copy = (char *) malloc(strlen(*(char **) keys) + 1))) {
   anaerror("Memory allocation error in installKeys [%s]", 0, (char *) keys);
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
   anaerror("Memory allocation error in installKeys [%s].", 0, (char *) keys);
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
int findName(char *name, hashTableEntry *hashTable[], int context)
/* searches for <name> in <hashTable[]> (with <context>).  if found, */
/* returns symbol number, otherwise installs a copy of the name in */
/* <hashTable[]> and sym[].  Returns -1 if an error occurs.  LS 6feb96 */
{
 int		hashValue, i;
 hashTableEntry	*hp, *oldHp;
#if YYDEBUG
 extern int	yydebug;
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
   return anaerror("Non-existent system variable %s", 0, name);
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
 symbol_class(i) = ANA_UNDEFINED;
 sym[i].context = context;
 sym[i].line = curLineNumber;
 return i;
}
/*----------------------------------------------------------------*/
int ana_verify(int narg, int ps[])
/* verifies that all referenced subroutines, functions, and files
   actually exist */
{
  char	*name, *p, compileName[12], oldInstalling;
  FILE	*fp;
  int	i, n, oldContext, nsym, result;
  extern char	compileOnly;
  extern int	executeLevel;

  result = 0;
  if (narg) {
    name = string_arg(ps[0]);
    fp = openPathFile(name, FIND_EITHER);
    if (!fp) {
      printf("Cannot open file %s\n", name);
      perror("System message");
      return ANA_ERROR;
    }
    compileOnly++;
    executeLevel++;
    oldContext = curContext;
    sprintf(compileName, "#COMPILE%1d", compileCount);
    n = installString(compileName);
    oldInstalling = installing;
    installing = 1;
    nsym = newSymbol(ANA_BLOCKROUTINE, n);
    installing = oldInstalling;
    curContext = nsym;
    n = nextCompileLevel(fp, expname);
    fclose(fp);
    if (n < 0
	|| newSymbol(ANA_BLOCKROUTINE, -nsym - 1) < 0) { /* some error */
      zap(nsym);
      return ANA_OK;
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
      case ANA_EVB:
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
	    if (symbol_class(n) == ANA_STRING) {
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
	    if (symbol_class(n) == ANA_STRING) {
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
  return ANA_OK;
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
      return;
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
static int	n_e_info = 0, cur_e_info = 0;
void pushExecutionLevel(int line, int target)
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
void showExecutionLevel(int symbol)
{
  int	i, target;

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
	  case ANA_SUBROUTINE:
	    printf("subr");
	    break;
	  case ANA_FUNCTION:
	    printf("func");
	    break;
	  case ANA_BLOCKROUTINE:
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
