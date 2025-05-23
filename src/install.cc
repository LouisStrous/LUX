/* This is file install.cc.

Copyright 2013 Louis Strous, Richard Shine
Copyright 2014 Louis Strous

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
// File install.c
#include "config.h"
#include "install.hh"
#include <ctype.h> // for toupper(11) isdigit(3)
#include <errno.h> // for errno(2)
#include <error.h> // for luxerror(58)
#include <float.h> // for FLT_MAX(2) DBL_MAX(2) DBL_MIN(1) FLT_MIN(1)
#include <limits.h> // for UINT8_MAX(3) INT32_MAX(1) INT16_MAX(1) INT16_MIN(1) INT32_MIN(1)
#include <malloc.h> // for malloc(25) free(18) realloc(4)
#include <math.h> // for j1(12)
#include <setjmp.h> // for longjmp(1) jmp_buf(1) setjmp(1)
#include <signal.h> // for signal(7) SIG_ERR(6) SIGCONT(3) SIGINT(2) SIGSEGV(2) ...
#include <stdarg.h> // for va_arg(39) va_end(14) va_start(2) va_list(2)
#include <stddef.h> // for NULL(20)
#include <stdio.h> // for printf(44) puts(16) FILE(6) fclose(4) sprintf(3) ...
#include <stdlib.h> // for strtol(4) atol(1)
#include <string.h> // for index(41) strlen(17) memcpy(12) strcpy(4) strcmp(4) ...
#include <time.h> // for CLK_TCK(1) clock(1) time(1)
#include <unistd.h> // for pipe(2) execl(1) sbrk(1)
#include <obstack.h>
// clock() on mips-sgi-irix64-6.2 is in unistd.h rather than in ANSI time.h

#include "editor.hh"
#include "editorcharclass.hh"
#include "action.hh"

extern char const* symbolStack[];
extern SymbolImpl    sym[];
extern HashTableEntry   *varHashTable[], *subrHashTable[], *funcHashTable[],
                        *blockHashTable[];
extern int16_t          listStack[];
extern int32_t          keepEVB;
extern char             *currentChar, line[];
extern FILE             *inputStream, *outputStream;
extern int32_t          nExecuted;
// extern InternalRoutine       subroutine[], function[];
InternalRoutine *subroutine, *function;

int32_t         luxerror(char const*, int32_t, ...),
  lookForName(char const*, HashTableEntry *[], int32_t),
        newSymbol(Symbolclass, ...), lux_setup();
void    installKeys(void *keys), zerobytes(void *, int32_t);
char    *strsave(char const *);
char const* symName(int32_t, HashTableEntry *[]), *className(int32_t),
        *typeName(int32_t);

int32_t         nFixed = 0, noioctl = 0, trace = 0, curTEIndex;

char    batch = 0, ignoreSymbols = 0, restart = 0;
char const* currentInputFile = NULL;

int32_t         traceMode = T_FILE | T_LOOP | T_BLOCK | T_ROUTINE;

int16_t         *listStackItem = listStack;

int32_t         symbolStackIndex = 0, tempVariableIndex = TEMPS_START,
        nTempVariable = 0, namedVariableIndex = NAMED_START,
        nNamedVariable = 0, nSymbolStack = 0, executableIndex = EXE_START,
        nExecutable = 0, tempExecutableIndex = TEMP_EXE_START,
        nTempExecutable, zapContext = 0, installString(char const *),
        lux_verify(int32_t, int32_t []), eval_func, insert_subr;

int32_t         markStack[MSSIZE], markIndex = 0;

ExecutionLevelInfo      *exInfo = NULL;
int32_t         nexInfo = 0;

extern int32_t  compileLevel, curLineNumber;
static char     installing = 1;

//----------------------------------------------------------------
typedef int32_t LuxRoutine(int32_t, int32_t*);

extern LuxRoutine lux_area, lux_area2, lux_arestore, lux_astore,
  lux_atomize, lux_batch, lux_byte_inplace,
  lux_cdouble_inplace, lux_cfloat_inplace, lux_chdir, lux_close,
  lux_cluster, lux_coordmap, lux_crunch,
  lux_crunchrun, lux_cubic_spline_extreme, lux_debug, lux_decomp,
  lux_decrunch, lux_default, lux_delete, lux_disableNewline,
  lux_distr, lux_double_inplace, lux_dsolve, lux_dump, lux_dump_lun,
  lux_dump_stack, lux_echo, lux_enableNewline, lux_endian,
  lux_error, lux_execute, lux_extract_bits, lux_fade, lux_fade_init,
  lux_fcrunwrite, lux_fcwrite, lux_fftshift, lux_file_to_fz,
  lux_fileptr, lux_fileread, lux_filewrite, lux_fits_read,
  lux_fits_write, lux_float_inplace, lux_format_set, lux_fprint,
  lux_fprintf, lux_fread, lux_freadf, lux_freads, lux_fzhead,
  lux_fzinspect, lux_fzread, lux_fzwrite, lux_getmin9, lux_help,
  lux_hex, lux_inserter, lux_int64_inplace,
  lux_limits, lux_list, lux_long_inplace, lux_manualterm,
  lux_multisieve, lux_noecho, lux_noop, lux_one, lux_openr, lux_openu,
  lux_openw, lux_orientation,
  lux_pointer, lux_pop,
  lux_printf, lux_push, lux_quit, lux_read, lux_readarr,
  lux_readf, lux_readu, lux_record, lux_redim,
  lux_redirect_diagnostic, lux_replace_values, lux_rewindf, lux_sc,
  lux_scb, lux_set, lux_setenv, lux_shift, lux_show, lux_show_func,
  lux_show_subr, lux_spawn, lux_step, lux_string_inplace,
  lux_subshift, lux_subshiftc, lux_swab, lux_swaphalf, lux_switch,
  lux_system, lux_trace, lux_type, lux_ulib, lux_wait,
  lux_watch, lux_word_inplace, lux_writeu, lux_zap, lux_zapnan,
  lux_zero, showstats, site;

int32_t         lux_name();

#if DEVELOP
extern LuxRoutine lux_trajectory, lux_getmin2, lux_projectmap;
#endif

#if DEBUG
extern LuxRoutine checkList, lux_whereisAddress, lux_show_temps,
                lux_newallocs, show_files;
#endif

extern LuxRoutine lux_gifread, lux_gifwrite;

extern LuxRoutine lux_menu, lux_menu_hide, lux_menu_item, lux_menu_kill,
  lux_menu_pop, lux_menu_read, lux_register_event, lux_xcopy, lux_xdelete,
  lux_xdrawline, lux_xevent, lux_xflush, lux_xfont, lux_xlabel, lux_xloop,
  lux_xopen, lux_xplace, lux_xport, lux_xpurge, lux_xquery, lux_xsetaction,
  lux_xsetbackground, lux_xsetforeground, lux_xtv, lux_xtvlct, lux_xtvmap,
  lux_xtvraw, lux_xtvread, lux_xymov, lux_wait_for_menu, lux_xclose, lux_xraise,
  lux_xcursor, lux_xanimate, lux_xzoom, lux_show_visuals, lux_zoom,
  lux_xtvplane, lux_threecolors, lux_tv3, lux_xinvertline, lux_xinvertarc,
  lux_xdrawarc, lux_colorComponents, lux_colorstogrey, lux_pixelsto8bit;

extern LuxRoutine lux_readorbits, lux_showorbits;

extern LuxRoutine peek;
extern LuxRoutine lux_breakpoint;
extern LuxRoutine insert;
LuxRoutine lux_restart;

#define MAX_ARG         100

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

InternalRoutine         subroutine_table[] = {
  { "%insert",  3, MAX_ARG, insert, // execute.c
    "1inner:2outer:4onedim:8skipspace:16zero:32all:64separate" },
  { "area",     1, 4, lux_area, ":seed:numbers:diagonal" }, // topology.c
  { "area2",    2, 6, lux_area2,                            // toplogy.c
    "::seed:numbers:diagonal:sign" },
  { "arestore", 1, MAX_ARG, lux_arestore, 0 },      // files.c
  { "astore",   2, MAX_ARG, lux_astore, 0 },        // files.c
  { "atomize",  1, 1, lux_atomize, "1tree:2line" }, // strous.c
  { "batch",    0, 1, lux_batch, "1quit" },         // symbols.c
  { "breakpoint", 0, 1, lux_breakpoint,             // install.c
    "0set:1enable:2disable:3delete:4list:8variable" },
  { "byte",     1, MAX_ARG, lux_byte_inplace, 0, }, // symbols.c
#if CALCULATOR
  { "calculator", 0, 0, lux_calculator, 0 }, // calculator.c
#endif
  { "cdouble",  1, MAX_ARG, lux_cdouble_inplace, 0 }, // symbols.c
  { "cfloat",   1, MAX_ARG, lux_cfloat_inplace, 0 },  // symbols.c
  { "chdir",    0, 1, lux_chdir, "1show" },           // files.c
#if DEBUG
  { "checklist", 0, 1, checkList, 0 }, // debug.c
#endif
  { "close",    1, 1, lux_close, 0 }, // files.c
  { "cluster",  2, 8, lux_cluster, // cluster.c
    "|32|:centers:index:size:sample:empty:maxit:rms:1update:2iterate"
    ":4vocal:8quick:16record:32ordered" },
  { "crunch",   3, 3, lux_crunch, 0 }, // crunch.c
  { "crunchrun",        3, 3, lux_crunchrun, 0 }, // crunch.c
  { "cspline_extr", 5, 8, lux_cubic_spline_extreme, "1keepdims:2periodic:4akima::::pos:minpos:minval:maxpos:maxval" }, // fun3.c
  { "d",        0, MAX_ARG, lux_dump, // fun1.c
    "+|36|1fixed:2system:4zero:8local:24context:32follow:64full" },
  { "decomp",   1, 1, lux_decomp, 0 },                 // fun2.c
  { "decrunch", 2, 2, lux_decrunch, 0 },               // crunch.c
  { "default",  2, MAX_ARG, lux_default, "+" },        // strous.c
  { "delete",   1, MAX_ARG, lux_delete, "+1pointer" }, // fun1.c
  { "diagnostic", 0, 1, lux_redirect_diagnostic, 0 },  // strous.c
  { "distr",    3, 3, lux_distr, 0 },                  // strous.c
  { "doub",     1, MAX_ARG, lux_double_inplace, 0 },   // symbols.c
  { "double",   1, MAX_ARG, lux_double_inplace, 0 },   // symbols.c
  { "dsolve",   2, 2, lux_dsolve, 0 },                 // fun2.c
  { "dump",     0, MAX_ARG, lux_dump,                  // fun1.c
    "+|36|1fixed:2system:4zero:8local:24context:32follow:64full" },
  { "dump_lun", 0, 0, lux_dump_lun, 0 },     // files.c
  { "dump_stack", 0, 0, lux_dump_stack, 0 }, // strous.c
  { "echo",     0, 1, lux_echo, 0 },         // symbols.c
  { "endian",   1, 1, lux_endian, 0 },       // strous.c
  { "execute",  1, 1, lux_execute, "1main" },         // execute.c
  { "exit",     0, 1, lux_quit, 0 },                  // fun1.c
  { "extract_bits", 4, 4, lux_extract_bits, 0 },      // fun3.c
  { "f0h",      1, 2, lux_fzhead, 0 },                // files.c
  { "f0head",   1, 2, lux_fzhead, 0 },                // files.c
  { "f0r",      2, 3, lux_fzread, "|1|1printheader" },       // files.c
  { "f0read",   2, 3, lux_fzread, "|1|1printheader" },       // files.c
  { "f0w",      2, 3, lux_fzwrite, 0 },                      // files.c
  { "f0write",  2, 3, lux_fzwrite, 0 },                      // files.c
  { "fade",     2, 2, lux_fade, 0 },                         // fun3.c
  { "fade_init", 2, 2, lux_fade_init, 0 },                   // fun3.c
  { "fcrunwrite", 2, 3, lux_fcrunwrite, 0 },                 // files.c
  { "fcrw",     2, 3, lux_fcrunwrite, 0 },                   // files.c
  { "fcw",      2, 3, lux_fcwrite, "1runlength" },           // files.c
  { "fcwrite",  2, 3, lux_fcwrite, "1runlength" },           // files.c
  { "fftshift", 2, 2, lux_fftshift, 0 },                     // fun3.c
  { "fileptr",  1, 2, lux_fileptr, "1start:2eof:4advance" }, // files.c
  { "fileread", 5, 5, lux_fileread, 0 },                     // files.c
  { "filetofz", 3, 3, lux_file_to_fz, 0 },                   // files.c
  { "filewrite", 2, 3, lux_filewrite, 0 },                   // files.c
  { "fits_read", 2, 7, lux_fits_read, // files.c
    "|1|1translate:2rawvalues::::::blank" },
  { "fits_write", 2, 4, lux_fits_write, "1vocal" },       // files.c
  { "fix",      1, MAX_ARG, lux_long_inplace, 0 },        // symbols.c
  { "float",    1, MAX_ARG, lux_float_inplace, 0 },       // symbols.c
  { "format_set", 0, 1, lux_format_set, 0 },              // files.c
  { "fprint",   1, MAX_ARG, lux_fprint, "1element" },     // files.c
  { "fprintf",  2, MAX_ARG, lux_fprintf, "1element" },    // files.c
  { "fread",    2, MAX_ARG, lux_fread, "1countspaces" },  // files.c
  { "freadf",   3, MAX_ARG, lux_freadf, "1countspaces" }, // files.c
  { "freads",   2, MAX_ARG, lux_freads, "1countspaces" }, // files.c
  { "fzh",      1, 2, lux_fzhead, 0 },                    // files.c
  { "fzhead",   1, 2, lux_fzhead, 0 },                    // files.c
  { "fzinspect", 2, 3, lux_fzinspect, 0 },                // files.c
  { "fzr",      2, 3, lux_fzread, "|1|1printheader" },    // files.c
  { "fzread",   2, 3, lux_fzread, "|1|1printheader" },    // files.c
  { "fzw",      2, 3, lux_fzwrite, "1safe" },             // files.c
  { "fzwrite",  2, 3, lux_fzwrite, "1safe" },             // files.c
  { "getmin9",  3, 3, lux_getmin9, 0 },                   // fun4.c
  { "gifread",  2, 3, lux_gifread, 0 },                   // gifread_lux.c
  { "gifwrite", 2, 3, lux_gifwrite, 0 },                  // gifwrite_lux.c
  { "help",     0, 1, lux_help, "1manual" },              // strous.c
  { "hex",      1, MAX_ARG, lux_hex, 0 },                 // files.c
  { "info",     0, 0, site,                               // site.c
    "1table:2time:4platform:8packages:16warranty:32copy"
    ":64bugs:128keys:255all" },
  { "insert",   2, 4, lux_inserter, 0 },                         // subsc.c
  { "int",      1, MAX_ARG, lux_word_inplace, 0 },               // symbols.c
  { "int64",    1, MAX_ARG, lux_int64_inplace, 0 },              // symbols.c
  { "limits",   0, 6, lux_limits, 0 },                           // plots.c
  { "list",     1, 1, lux_list, 0 },                             // ident.c
  { "long",     1, MAX_ARG, lux_long_inplace, 0 },               // symbols.c
  { "multisieve", 4, 4, lux_multisieve, 0 },                     // strous2.c
#if DEBUG
  { "newallocs", 0, 1, lux_newallocs, "1reset" }, // debug.c
#endif
  { "noecho",   0, 0, lux_noecho, 0 },         // symbols.c
  { "one",      1, 1, lux_one, 0 },            // fun1.c
  { "openr",    2, 2, lux_openr, "1get_lun" }, // files.c
  { "openu",    2, 2, lux_openu, "1get_lun" }, // files.c
  { "openw",    2, 2, lux_openw, "1get_lun" }, // files.c
  { "orientation", 3, 8, lux_orientation, // orientation.c
    "1vocal:2getj:0parallel:4perpendicular:::orientation:values"
    ":wavenumber:grid:aspect:order" },
  { "peek",     1, 2, peek, 0 },                              // strous.c
  { "pointer",  2, 2, lux_pointer, // symbols.c
    "+:1function:2subroutine:4internal:8main" },
  { "pop",      1, MAX_ARG, lux_pop, "%1%num" },                  // strous.c
  { "print",    1, MAX_ARG, lux_type, "1join:2raw:4separate" },   // files.c
  { "printf",   1, MAX_ARG, lux_printf, "1join:2raw:4separate" }, // files.c
  { "push",     1, MAX_ARG, lux_push, 0 },                        // strous.c
  { "quit",     0, 1, lux_quit, 0 },                              // fun1.c
  { "read",     1, MAX_ARG, lux_read, "1askmore:2word:4flush" },  // files.c
  { "readarr",  1, 1, lux_readarr, 0 },                           // strous.c
  { "readf",    2, MAX_ARG, lux_readf, "1askmore:2word" },        // files.c
  { "readorbits", 0, 1, lux_readorbits, "1list:2replace" },       // astron.c
  { "readu",    2, MAX_ARG, lux_readu, 0 },                       // files.c
  { "record",   0, 1, lux_record, "1input:2output:4reset" },      // symbols.c
  { "redim",    2, 9, lux_redim, 0 },                             // subsc.c
  { "replace",  3, 3, lux_replace_values, 0 },                    // strous2.c
  { "restart",  0, 0, lux_restart, 0 },                           // install.c
  { "restore",  2, 3, lux_fzread, "1printheader" },               // files.c
  { "rewindf",  1, 1, lux_rewindf, 0 },                    // files.c
  { "s",        0, 1, lux_show, 0 },                       // fun1.c
  { "sc",       3, 3, lux_sc, 0 },                         // fun3.c
  { "scanf",    2, MAX_ARG, lux_freadf, "+1countspaces" }, // files.c
  { "scb",      3, 3, lux_scb, "1even:2odd" },             // fun3.c
  { "set",      0, 1, lux_set,                             // symbols.c
    ":1set:2reset:4showalloc:64oldversion:1024allowprompts"
    ":4096parsesilent" },
  { "setenv",   1, 1, lux_setenv, 0 },                    // files.c
  { "shift",    1, 4, lux_shift, ":::blank:1translate" }, // strous2.c
  { "show",     0, 1, lux_show, 0 },                      // fun1.c
  { "showorbits", 0, 0, lux_showorbits, 0 },              // astron.c
  { "showstats", 0, 0, showstats, 0 },                    // strous2.c
#if DEBUG
  { "show_files", 0, 0, show_files, 0 }, // debug.c
#endif
  { "show_func", 0, 1, lux_show_func, "1parameters" }, // symbols.c
  { "show_subr", 0, 1, lux_show_subr, "1parameters" }, // symbols.c
#if DEBUG
  { "show_temps", 0, 0, lux_show_temps, 0 }, // symbols.c
#endif
  { "spawn",    1, 1, lux_spawn, "1silent" },                   // files.c
  { "sscanf",   2, MAX_ARG, lux_freads, "1countspaces" },       // files.c
  { "step",     0, 1, lux_step, 0 },                            // symbols.c
  { "store",    2, 3, lux_fzwrite, "1safe" },                   // files.c
  { "string",   1, MAX_ARG, lux_string_inplace, 0 },            // symbols.c
  { "subshift", 4, 4, lux_subshift, 0 },                        // fun5.c
  { "subshiftc", 4, 5, lux_subshiftc, 0 },                      // fun5.c
  { "swab",     1, MAX_ARG, lux_swab, 0 },                      // fun2.c
  { "swapb",    1, MAX_ARG, lux_swab, 0 },                      // fun2.c
  { "swaphalf", 1, 1, lux_swaphalf, 0 },                        // strous2.c
  { "switch",   2, 2, lux_switch, 0 },                          // symbols.c
  { "t",        1, MAX_ARG, lux_type, "1join:2raw:4separate" }, // files.c
  { "trace",    0, 1, lux_trace,               // install.c
    "1file:2loop:4braces:8routine:143all:16showstats:32cputime"
    ":64showexec:128enter" },
#if DEVELOP
  { "trajectory", 3, 7, lux_trajectory, 0 }, // strous3.c
#endif
  { "ty",       1, MAX_ARG, lux_type, "1join:2raw:4separate" }, // files.c
  { "type",     1, MAX_ARG, lux_type, "1join:2raw:4separate" }, // files.c
  { "ulib",     0, 1, lux_ulib, 0 },                            // files.c
  { "verify",   0, 1, lux_verify, 0 },             // install.c
  { "wait",     1, 1, lux_wait, 0 },               // fun2.c
  { "watch",    1, 1, lux_watch, "1delete:2list" }, // install.c
#if DEBUG
  { "where",    1, 1, lux_whereisAddress, "1cut" }, // debug.c
#endif
  { "word",     1, 1, lux_word_inplace, 0 },   // symbols.c
  { "writeu",   2, MAX_ARG, lux_writeu, 0 },   // files.c
  { "zap",      1, MAX_ARG, lux_zap, "+1pointer" }, // strous2.c
  { "zero",     1, MAX_ARG, lux_zero, 0 },          // fun1.c
  { "zeronans", 1, MAX_ARG, lux_zapnan, "*%1%value" }, // fun1.c
};
int32_t nSubroutine = sizeof(subroutine_table)/sizeof(InternalRoutine);

extern LuxRoutine lux_abs, lux_acos, lux_arestore_f, lux_arg,
  lux_array, lux_asin, lux_astore_f, lux_atan,
  lux_atan2, lux_basin, lux_basin2, lux_beta, lux_bisect,
  lux_bmap, bytarr, lux_byte, bytfarr, lux_cbrt, cdblarr,
  cdblfarr, lux_cdmap, lux_cdouble, lux_ceil, lux_cfloat,
  lux_cfmap, cfltarr, cfltfarr, lux_chi_square,
  lux_classname, lux_complex, lux_complexsquare, lux_compress,
  lux_concat, lux_conjugate, lux_convertsym, lux_cos,
  lux_cosh, lux_cputime, lux_crosscorr, lux_crunch_f,
  lux_ctime, lux_cubic_spline, lux_date, lux_date_from_tai,
  dblarr, dblfarr, lux_defined, lux_delete, lux_despike,
  lux_detrend, lux_differ, lux_dilate, lux_dilate_dir,
  lux_dimen, lux_dir_smooth, lux_dir_smooth2, lux_distarr,
  lux_distr_f, lux_dmap, lux_double, lux_equivalence,
  lux_erf, lux_erfc, lux_erode, lux_erode_dir, lux_esmooth,
  lux_eval, lux_exp, lux_expand, lux_expm1,
  lux_extract_bits_f, lux_extreme_general, lux_f_ratio,
  lux_fcwrite_f, lux_fftshift_f, lux_fileptr_f,
  lux_filesize, lux_filetype_name, lux_findfile,
  lux_find_max, lux_find_maxloc, lux_find_min,
  lux_find_minloc, lux_fitskey, lux_fits_header_f,
  lux_fits_read_f, lux_fits_xread_f, lux_float, lux_floor,
  fltarr, fltfarr, lux_fmap, lux_freadf_f, lux_freads_f,
  lux_fstring, lux_fzarr, lux_fzhead_f, lux_fzread_f,
  lux_fzwrite_f, lux_gamma, lux_generalfit, lux_get_lun,
  lux_getenv, lux_gridmatch, lux_gsmooth, lux_hamming, lux_hilbert,
  lux_histr, lux_identify_file,
  lux_imaginary, lux_incomplete_beta, lux_incomplete_gamma,
  lux_index, lux_indgen, lux_inpolygon, intarr, int64arr,
  int64farr, intfarr, lux_int64,
  lux_isarray, lux_isnan, lux_isscalar, lux_isstring,
  lux_istring, lux_j0, lux_j1, lux_jd, lux_jn, lux_cjd,
  lux_ksmooth, lux_laplace2d, lux_lmap, lux_local_maxf,
  lux_local_maxloc, lux_int64map,
  lux_local_minf, lux_local_minloc, lux_log, lux_log10,
  lux_log1p, lonfarr, lux_long, lux_lower,
  lux_lsq, lux_lsq2, lux_match, lux_max_dir,
  lux_maxf, lux_maxfilter, lux_maxloc, lux_mean,
  lux_medfilter, lux_median, lux_memory, lux_minf,
  lux_minfilter, lux_minloc, lux_neg_func,
  lux_noncentral_chi_square, lux_not, lux_num_dimen,
  lux_num_elem, lux_onef, lux_openr_f, lux_openu_f,
  lux_openw_f, lux_orderfilter, lux_pit, lux_poly, lux_pow,
  lux_power, lux_printf_f, lux_psum, lux_quantile, lux_quit,
  lux_random, lux_randomb, lux_randomd, lux_randomn,
  lux_randomu, lux_randoml, lux_readf_f, lux_readkey,
  lux_readkeyne, lux_readu_f, lux_real, lux_redim_f,
  lux_regrid, lux_regrid3, lux_regrid3ns, lux_reorder,
  lux_reverse, lux_rfix, lux_root3, lux_runcum, lux_runprod,
  lux_scale, lux_scalerange,
  lux_sdev, lux_segment, lux_segment_dir, lux_sgn,
  lux_shift_f, lux_sieve, lux_sin, lux_sinh, lux_skipc,
  lux_smap, lux_smooth, lux_solar_b, lux_solar_l,
  lux_solar_p, lux_solar_r, lux_sort, lux_spawn_f, lux_sqrt,
  strarr, lux_strcount, lux_stretch, lux_string, lux_strlen,
  lux_strloc, lux_strpos, lux_strreplace, lux_strskp,
  lux_strsub, lux_strtok, lux_strtol, lux_strtrim,
  lux_struct, lux_student, lux_subsc_func, lux_sun_b,
  lux_sun_d, lux_sun_p, lux_sun_r, lux_symbol,
  lux_symbol_memory, lux_symbol_number, lux_symclass,
  lux_symdtype, lux_systime, lux_table, lux_tai_from_date,
  lux_tan, lux_tanh, lux_temp, lux_tense, lux_tense_curve,
  lux_tense_loop, lux_time, lux_total,
  lux_trend, lux_tri_name_from_tai, lux_typeName, lux_upper,
  lux_variance, lux_varname, lux_voigt, lux_wait_for_menu,
  lux_wmap, lux_word, lux_y0, lux_y1, lux_yn,
  lux_zapnan_f, lux_zerof, lux_zinv, lux_fcrunwrite_f,
  lux_strpbrk, lux_shift3, lux_area_connect, lux_legendre,
  lux_cartesian_to_polar, lux_polar_to_cartesian, lux_roll,
  lux_siderealtime, lux_asinh,
  lux_acosh, lux_atanh, lux_astrf, lux_antilaplace2d,
  lux_cspline_find, lux_covariance;

#if HAVE_REGEX_H
extern LuxRoutine lux_getdirectories, lux_getfiles, lux_getfiles_r,
  lux_getmatchedfiles, lux_getmatchedfiles_r, lux_regex;
#endif

#if DEVELOP
extern LuxRoutine lux_project, lux_bsmooth, lux_compile,
  lux_bessel_i0, lux_bessel_i1, lux_bessel_k0, lux_bessel_k1,
  lux_bessel_kn, lux_regridls, lux_bigger235,
  lux_geneticfit;
#endif

extern LuxRoutine lux_gifread_f, lux_gifwrite_f;

extern LuxRoutine lux_calendar, lux_EasterDate, // lux_orbitalElement,
  lux_astropos, lux_precess, lux_constellation,
  lux_constellationname, lux_enhanceimage;

extern int32_t  vargsmooth;
extern LuxRoutine lux_test;

InternalRoutine function_table[] = {
  { "%a_unary_negative", 1, 1, lux_neg_func, "*" }, // fun1.cc
  { "%b_subscript", 1, MAX_ARG, lux_subsc_func,     // subsc.cc
    "1inner:2outer:4zero:8subgrid:16keepdims:32all:64separate" },
  { "%c_cputime", 0, 0, lux_cputime, 0 },                      // fun1.cc
  { "%d_power", 2, 2, lux_pow, "*" },                          // fun1.cc
  { "%e_concat", 1, MAX_ARG, lux_concat, "1sloppy" },          // subsc.cc
  { "%f_ctime", 0, 0, lux_ctime, 0 },                          // fun1.cc
  { "%g_time",  0, 0, lux_time, 0 },                           // fun1.cc
  { "%h_date",  0, 0, lux_date, 0 },                           // fun1.cc
  { "%i_readkey", 0, 0, lux_readkey, 0 },                      // strous.cc
  { "%j_readkeyne", 0, 0, lux_readkeyne, 0 },                  // strous.cc
  { "%k_systime", 0, 0, lux_systime, 0 },                      // fun1.cc
  { "%l_jd",    0, 0, lux_jd, 0 },                             // fun1.cc
  { "%m_cjd",   0, 0, lux_cjd, 0 },                            // fun1.cc
  { "abs",      1, 1, lux_abs, "*" },                          // fun1.cc
  { "acos",     1, 1, lux_acos, "*" },                         // fun1.cc
  { "acosh",    1, 1, lux_acosh, "*" },                        // fun1.cc
  { "alog",     1, 1, lux_log, "*" },                          // fun1.cc
  { "alog10",   1, 1, lux_log10, "*" },                        // fun1.cc
  { "antilaplace2d", 2, 2, lux_antilaplace2d, 0 },             // poisson.cc
  { "areaconnect", 2, 3, lux_area_connect, "::compact:1raw" }, // topology.cc
  { "arestore", 1, MAX_ARG, lux_arestore_f, 0 },               // files.cc
  { "arg",      1, 1, lux_arg, 0 },                            // fun3.cc
  { "array",    1, MAX_DIMS + 1, lux_array, 0 },               // symbols.cc
  { "asin",     1, 1, lux_asin, "*" },                         // fun1.cc
  { "asinh",    1, 1, lux_asinh, "*" },                        // fun1.cc
  { "astore",   2, MAX_ARG, lux_astore_f, 0 },                 // files.cc
  { "astrf",    1, 2, lux_astrf,                               // astron.cc
    "1fromequatorial:2fromecliptical:4fromgalactic:8toequatorial"
    ":16toecliptical:32togalactic:64julian:128besselian" },
  { "astron",   2, 7, lux_astropos, // astron.cc
    ":::observer:equinox:elements:tolerance:1ecliptical:2equatorial"
    ":3horizontal:4elongation:8xyz:16lighttime:32date:64tdt"
    ":256aberration:512nutation:2832apparent:1024qelements:2048fk5"
    ":8192conjspread:16384planetocentric:32768keepdimensions"
    ":65536vocal:~131072vsop87a:131072vsop87c:262144bare" },
  { "atan",     1, 1, lux_atan, "*" },  // fun1.cc
  { "atan2",    2, 2, lux_atan2, "*" }, // fun1.cc
  { "atanh",    1, 1, lux_atanh, "*" }, // fun1.cc
  { "atol",     1, 2, lux_strtol, 0 },  // fun3.cc
  { "basin",    1, 2, lux_basin2, "*1number:2sink:4difference" }, // strous.cc
#if DEVELOP
  { "bessel_i0", 1, 1, lux_bessel_i0, "*1deflate" }, // fun1.cc
  { "bessel_i1", 1, 1, lux_bessel_i1, "*" },         // fun1.cc
#endif
  { "bessel_j0", 1, 1, lux_j0, "*" }, // fun1.cc
  { "bessel_j1", 1, 1, lux_j1, "*" }, // fun1.cc
  { "bessel_jn", 2, 2, lux_jn, "*" }, // fun1.cc
#if DEVELOP
  { "bessel_k0", 1, 1, lux_bessel_k0, "*" }, // fun1.cc
  { "bessel_k1", 1, 1, lux_bessel_k1, "*" }, // fun1.cc
  { "bessel_kn", 2, 2, lux_bessel_kn, "*" }, // fun1.cc
#endif
  { "bessel_y0", 1, 1, lux_y0, "*" }, // fun1.cc
  { "bessel_y1", 1, 1, lux_y1, "*" }, // fun1.cc
  { "bessel_yn", 2, 2, lux_yn, "*" }, // fun1.cc
  { "beta",     2, 2, lux_beta, "*1log" }, // fun1.cc
#if DEVELOP
  { "bi0",      1, 1, lux_bessel_i0, "*1deflate" }, // fun1.cc
  { "bi1",      1, 1, lux_bessel_i1, "*" },         // fun1.cc
  { "bigger235", 1, 1, lux_bigger235, "*" },        // fun4.cc
#endif
  { "bisect",   2, 6, lux_bisect, ":::axis:pos:width" }, // strous3.cc
  { "bj0",      1, 1, lux_j0, "*" },                     // fun1.cc
  { "bj1",      1, 1, lux_j1, "*" },                     // fun1.cc
  { "bjn",      2, 2, lux_jn, "*" },                     // fun1.cc
#if DEVELOP
  { "bk0",      1, 1, lux_bessel_k0, "*" }, // fun1.cc
  { "bk1",      1, 1, lux_bessel_k1, "*" }, // fun1.cc
  { "bkn",      2, 2, lux_bessel_kn, "*" }, // fun1.cc
#endif
  { "bmap",     1, 1, lux_bmap, "*" }, // subsc.cc
#if DEVELOP
  { "bsmooth",  1, 3, lux_bsmooth, 0 }, // strous.cc
#endif
  { "by0",      1, 1, lux_y0, "*" },      // fun1.cc
  { "by1",      1, 1, lux_y1, "*" },      // fun1.cc
  { "byn",      2, 2, lux_yn, "*" },      // fun1.cc
  { "calendar", 1, 3, lux_calendar, // astron.cc
    "1fromcommon:2fromgregorian:3fromislamic:4fromjulian:5fromhebrew"
    ":6fromegyptian:7fromjd:8fromcjd:9fromlunar:10frommayan"
    ":11fromlongcount:12fromlatin:16tocommon:32togregorian"
    ":48toislamic:64tojulian:80tohebrew:96toegyptian:112tojd:128tocjd"
    ":144tolunar:160tomayan:176tolongcount:192tolatin:0tonumeric"
    ":256toint:512todouble:768totext:0fromutc:1024fromtai:2048fromtt"
    ":3072fromlt:0toutc:4096totai:8192tott:12288tolt:0fromymd"
    ":16384fromdmy:0toymd:32768todmy" },
  { "cbrt",     1, 1, lux_cbrt, "*" },    // fun1.cc
  { "cdblarr",  1, MAX_ARG, cdblarr, 0 }, // symbols.cc
  { "cdblfarr", 3, MAX_DIMS + 1, cdblfarr, // filemap.cc
    "%1%offset:1readonly:2swap" },
  { "cdmap",    1, 1, lux_cdmap, 0 },     // subsc.cc
  { "cdouble",  1, 1, lux_cdouble, "*" }, // fun1.cc
  { "cfloat",   1, 1, lux_cfloat, "*" },  // fun1.cc
  { "cfltarr",  1, MAX_ARG, cfltarr, 0 }, // symbols.cc
  { "cfltfarr", 3, MAX_DIMS + 1, cfltfarr, // filemap.cc
    "%1%offset:1readonly:2swap" },
  { "cfmap",    1, 1, lux_cfmap, 0 },                        // subsc.cc
  { "chi2",     2, 2, lux_chi_square, "*1complement:2log" }, // fun1.cc
  { "classname", 1, 1, lux_classname, 0 },                   // install.cc
#if DEVELOP
  { "compile",  1, 1, lux_compile, 0 }, // install.cc
#endif
  { "complex",  1, 1, lux_complex, 0 },              // fun3.cc
  { "complexsquare", 1, 1, lux_complexsquare, 0 },   // fun1.cc
  { "compress", 2, 3, lux_compress, 0 },             // fun4.cc
  { "concat",   1, MAX_ARG, lux_concat, "1sloppy" }, // subsc.cc
  { "conjugate", 1, 1, lux_conjugate, "*" },         // fun1.cc
  { "constellation", 1, 2, lux_constellation,        // astron.cc
    "1julian:2besselian:4vocal" },
  { "constellationname", 1, 1, lux_constellationname, 0 }, // astron.cc
  { "convert",  2, 2, lux_convertsym, "*" },               // symbols.cc
  { "cos",      1, 1, lux_cos, "*" },                      // fun1.cc
  { "cosh",     1, 1, lux_cosh, "*" },                     // fun1.cc
  { "covariance", 2, 4, lux_covariance,                    // fun2.cc
    ":::weights:*0sample:1population:2keepdims:4double:8omitnans" },
  { "crosscorr", 2, 3, lux_crosscorr, 0 }, // fun2.cc
  { "crunch",   3, 3, lux_crunch_f, 0 },   // crunch.cc
  { "cspline",  0, 6, lux_cubic_spline,    // fun3.cc
    "1keep:2periodic:4akima:8getderivative:16getintegral" },
  { "cspline_find", 2, 4, lux_cspline_find, ":::axis:index" }, // strous3.cc
  { "ctop",     1, 3, lux_cartesian_to_polar, 0 },             // fun4.cc
  { "date_from_tai", 1, 2, lux_date_from_tai, 0 },             // ephem.cc
  { "dblarr",   1, MAX_DIMS, dblarr, 0 },                      // symbols.cc
  { "dblfarr",  3, MAX_DIMS + 1, dblfarr,                      // filemap.cc
    "%1%offset:1readonly:2swap" },
  { "defined",  1, 1, lux_defined, "+1target" },                     // fun1.cc
  { "despike",  1, 6, lux_despike, ":frac:level:niter:spikes:rms" }, // fun6.cc
  { "detrend",  1, 2, lux_detrend, "*" },                            // fun2.cc
  { "differ",   1, 3, lux_differ, "*1central:2circular" }, // strous.cc
  { "dilate",   1, 1, lux_dilate, 0 },                     // fun5.cc
  { "dimen",    1, 2, lux_dimen, 0 },                      // subsc.cc
  { "distarr",  1, 3, lux_distarr, 0 },                    // strous2.cc
  { "dmap",     1, 1, lux_dmap, 0 },     // subsc.cc
  { "doub",     1, 1, lux_double, "*" }, // symbols.cc
  { "double",   1, 1, lux_double, "*" }, // symbols.cc
  { "dsmooth",  3, 3, lux_dir_smooth,    // strous3.cc
    "0twosided:0boxcar:1onesided:2gaussian:4total:8straight" },
  { "dsum",     1, 4, lux_total, "|1|::power:weights:2keepdims" }, // fun1.cc
  { "easterdate", 1, 1, lux_EasterDate, 0 },                       // astron.cc
  { "enhanceimage", 1, 3, lux_enhanceimage,                        // strous3.cc
    ":part:target:1symmetric" },
  { "equivalence", 2, 2, lux_equivalence, 0 },  // strous2.cc
  { "erf",      1, 1, lux_erf, "*" },           // fun1.cc
  { "erfc",     1, 1, lux_erfc, "*" },          // fun1.cc
  { "erode",    1, 1, lux_erode, "1zeroedge" }, // fun5.cc
  { "esegment", 1, 4, lux_extreme_general,      // topology.cc
    ":sign:diagonal:threshold" },
  { "esmooth",  1, 3, lux_esmooth, 0 },                        // fun2.cc
  { "eval",     1, 2, lux_eval, "1allnumber" },                // fun3.cc
  { "exp",      1, 1, lux_exp, "*" },                          // fun1.cc
  { "expand",   2, 4, lux_expand, "1smooth:2nearest" },        // fun4.cc
  { "expm1",    1, 1, lux_expm1, "*" },                        // fun1.cc
  { "extract_bits", 3, 3, lux_extract_bits_f, 0 },             // fun3.cc
  { "fcrunwrite", 2, 3, lux_fcrunwrite_f, 0 },                 // files.cc
  { "fcrw",     2, 3, lux_fcrunwrite_f, 0 },                   // files.cc
  { "fcw",      2, 3, lux_fcwrite_f, "1runlength" },           // files.cc
  { "fcwrite",  2, 3, lux_fcwrite_f, "1runlength" },           // files.cc
  { "fftshift", 2, 2, lux_fftshift_f, 0 },                     // fun3.cc
  { "fileptr",  1, 2, lux_fileptr_f, "1start:2eof:4advance" }, // files.cc
  { "filesize", 1, 1, lux_filesize, 0 },                       // files.cc
  { "filetype", 1, 1, lux_identify_file, 0 },                  // files.cc
  { "filetypename", 1, 1, lux_filetype_name, 0 },              // install.cc
  { "findfile", 2, 2, lux_findfile, 0 },         // files.cc
  { "fit",      3, 17, lux_generalfit, // fit.cc
    "|4|::start:step:lowbound:highbound:weights:qthresh:pthresh:ithresh"
    ":dthresh:fac:niter:nsame:err:fit:tthresh:1vocal:4down:8pchi"
    ":16gaussians:32powerfunc:64onebyone:129verr" },
#if DEVELOP
  { "fit2",     4, 11, lux_geneticfit, // fit.cc
    "x:y:npar:fit:weights:mu:generations:population:pcross:pmutate:vocal"
    ":1elite:2byte:4word:6long:8float:10double" },
#endif
  { "fits_header", 1, 4, lux_fits_header_f, 0 }, // files.cc
  { "fits_key", 2, 2, lux_fitskey, "1comment" }, // strous3.cc
  { "fits_read", 2, 7, lux_fits_read_f,          // files.cc
    "|1|1translate:2rawvalues::::::blank" },
  { "fits_xread", 2, 6, lux_fits_xread_f, 0 }, // files.cc
  { "fix",      1, 1, lux_long, "*" },         // symbols.cc
  { "float",    1, 1, lux_float, "*" },        // symbols.cc
  { "fltarr",   1, MAX_DIMS, fltarr, 0 },      // symbols.cc
  { "fltfarr",  3, MAX_DIMS + 1, fltfarr,      // filemap.cc
    "%1%offset:1readonly:2swap" },
  { "fmap",     1, 1, lux_fmap, 0 },                          // subsc.cc
  { "fratio",   3, 3, lux_f_ratio, "*1complement:2log" },     // fun1.cc
  { "freadf",   2, MAX_ARG, lux_freadf_f, "|1|1eof" },        // files.cc
  { "freads",   3, MAX_ARG, lux_freads_f, "1countspaces" },   // files.cc
  { "fstring",  1, MAX_ARG, lux_fstring, "1skip_undefined" }, // fun2.cc
  { "fsum",     1, 4, lux_total,                              // fun1.cc
    "::power:weights:1double:2keepdims" },
  { "fzarr",    1, 1, lux_fzarr, "1readonly" },          // filemap.cc
  { "fzh",      1, 2, lux_fzhead_f, 0 },                 // files.cc
  { "fzhead",   1, 2, lux_fzhead_f, 0 },                 // files.cc
  { "fzr",      2, 3, lux_fzread_f, "|1|1printheader" }, // files.cc
  { "fzread",   2, 3, lux_fzread_f, "|1|1printheader" }, // files.cc
  { "fzw",      2, 3, lux_fzwrite_f, 0 },                // files.cc
  { "fzwrite",  2, 3, lux_fzwrite_f, 0 },                // files.cc
  { "gamma",    1, 1, lux_gamma, "*1log" },              // fun1.cc
#if HAVE_REGEX_H
  { "getdirectories", 1, 2, lux_getdirectories, 0 }, // files.cc
  { "getdirects", 1, 2, lux_getdirectories, 0 },     // files.cc
#endif
  { "getenv",   1, 1, lux_getenv, 0 }, // files.cc
#if HAVE_REGEX_H
  { "getfiles", 1, 2, lux_getfiles, 0 },                   // files.cc
  { "getfiles_r", 1, 2, lux_getfiles_r, 0 },               // files.cc
  { "getmatchedfiles", 2, 3, lux_getmatchedfiles, 0 },     // files.cc
  { "getmatchedfiles_r", 2, 3, lux_getmatchedfiles_r, 0 }, // files.cc
#endif
  { "get_lun",  0, 0, lux_get_lun, 0 },           // files.cc
  { "gifread",  2, 3, lux_gifread_f, 0 },         // gifread_lux.cc
  { "gifwrite", 2, 3, lux_gifwrite_f, 0 },        // gifwrite_lux.cc
  { "gridmatch", 7, 8, lux_gridmatch, "1vocal" }, // fun4.cc
  { "gsmooth",  1, 4, lux_gsmooth,                // fun2.cc
    ":::kernel:1normalize:2fullnorm:4balanced:8all" },
  { "hamming",  1, 2, lux_hamming, 0 }, // strous3.cc
  { "histr",    1, 1, lux_histr, // fun3.cc
    "1first:2ignorelimit:4increaselimit:8silent" },
  { "ibeta",    3, 3, lux_incomplete_beta, "*1complement:2log" },  // fun1.cc
  { "igamma",   2, 2, lux_incomplete_gamma, "*1complement:2log" }, // fun1.cc
  { "imaginary", 1, 1, lux_imaginary, 0 },                         // fun3.cc
  { "index",    1, 1, lux_index, "*1axis:2rank" },                 // fun4.cc
  { "indgen",   1, 2, lux_indgen, "*" },                           // fun1.cc
  { "inpolygon", 4, 4, lux_inpolygon, 0 },  // topology.cc
  { "int",      1, 1, lux_word, "*" },      // symbols.cc
  { "int64map", 1, 1, lux_int64map, 0 },  // subsc.cc
  { "isarray",  1, 1, lux_isarray, 0 },  // subsc.cc
  { "isnan",    1, 1, lux_isnan, 0 },    // fun1.cc; needs IEEE isnan!
  { "isscalar", 1, 1, lux_isscalar, 0 }, // subsc.cc
  { "isstring", 1, 1, lux_isstring, 0 }, // subsc.cc
  { "ist",      1, 3, lux_istring, 0 },  // fun2.cc
  { "istring",  1, 3, lux_istring, 0 },  // fun2.cc
  { "ksmooth",  2, 3, lux_ksmooth, "1balanced" },                  // fun2.cc
  { "laplace2d", 1, 1, lux_laplace2d, 0 },                         // poisson.cc
  { "legendre", 2, 2, lux_legendre, "1normalize" },                // strous3.cc
  { "llsq",     2, 9, lux_lsq2,                                    // strous2.cc
    ":::fwhm:weights:cov:err:chisq:1formal:2reduce" },
  { "lmap",     1, 1, lux_lmap, 0 },   // subsc.cc
  { "local_max", 2, 2, lux_local_maxf, // strous.cc
    "3subgrid:2bound" },
  { "local_maxloc", 2, 2, lux_local_maxloc, // strous.cc
    "3subgrid:2bound:4relative" },
  { "local_min", 2, 2, lux_local_minf, // strous.cc
    "3subgrid:2bound" },
  { "local_minloc", 2, 2, lux_local_minloc, // strous.cc
    "3subgrid:2bound:4relative" },
  { "log",      1, 1, lux_log, "*" },     // fun1.cc
  { "log10",    1, 1, lux_log10, "*" },   // fun1.cc
  { "log1p",    1, 1, lux_log1p, "*" },   // fun1.cc
  { "lowcase",  1, 1, lux_lower, 0 },  // fun2.cc
  { "lower",    1, 1, lux_lower, 0 },  // fun2.cc
  { "lsmooth",  3, 3, lux_dir_smooth2, // strous3.cc
    "0twosided:0boxcar:1onesided:2gaussian:4normalize:8straight" },
  { "lsq",      2, 6, lux_lsq,  // strous2.cc
    "::weights:cov:err:chisq:1formal:2reduce" },
  { "match",    2, 2, lux_match, 0 },            // strous.cc
  { "max",      1, 2, lux_maxf, "1keepdims" },   // fun3.cc
  { "maxdir",   2, 3, lux_max_dir, 0 },          // topology.cc
  { "maxfilter", 1, 3, lux_maxfilter, 0 },       // strous2.cc
  { "maxloc",   1, 2, lux_maxloc, "1keepdims" }, // fun3.cc
  { "mean",     1, 4, lux_mean,                  // fun1.cc
    "::power:weights:1double:2keepdims:4float:8omitnans" },
  { "medfilter", 1, 4, lux_medfilter, "%1%" },        // strous2.cc
  { "median",   1, 3, lux_median, "%1%" },            // strous2.cc
  { "memory",   0, 0, lux_memory, 0 },                // memck.cc
  { "min",      1, 2, lux_minf, "1keepdims" },        // fun3.cc
  { "minfilter", 1, 3, lux_minfilter, 0 },            // strous2.cc
  { "minloc",   1, 2, lux_minloc, "1keepdims" },      // fun3.cc
  { "ncchi2",   3, 3, lux_noncentral_chi_square, 0 }, // fun1.cc
  { "not",      1, 1, lux_not, "*" },                 // strous.cc
  { "num_dim",  1, 1, lux_num_dimen, 0 },             // subsc.cc
  { "num_elem", 1, 2, lux_num_elem, 0 },              // subsc.cc
  { "one",      1, 1, lux_onef, 0 },                  // fun1.cc
  { "openr",    2, 2, lux_openr_f, "1get_lun" },      // files.cc
  { "openu",    2, 2, lux_openu_f, "1get_lun" },      // files.cc
  { "openw",    2, 2, lux_openw_f, "1get_lun" },      // files.cc
  // { "orbitelem", 3, 3, lux_orbitalElement, 0,
  { "ordfilter", 1, 4, lux_orderfilter, // strous2.cc
    "%1%order:1median:2minimum:3maximum" },
  { "pit",      1, 3, lux_pit, 0 },                         // fun2.cc
  { "polate",   3, 3, lux_table, 0 },                       // strous.cc
  { "poly",     2, 2, lux_poly, "*" },                      // fun2.cc
  { "power",    1, 2, lux_power, "1power:2shape:4onedim" }, // fun3.cc
  { "precess",  3, 3, lux_precess, "1julian:2besselian" },  // astron.cc
  { "printf",   1, MAX_ARG, lux_printf_f, 0 },              // files.cc
#if DEVELOP
  { "project",  1, 1, lux_project, 0 }, // projection.cc
  { "projectmap", 2, 8, lux_projectmap, // projection.cc
    "::hdist:angle:mag:xmap:ymap:size" },
#endif
  { "psum",     2, 4, lux_psum, // strous2.cc
    "1onedim:2vnormalize:4cnormalize:8single" },
  { "ptoc",     1, 5, lux_polar_to_cartesian, 0 }, // fun4.cc
  { "quantile", 2, 3, lux_quantile, 0 },           // strous2.cc
  { "random",   1, MAX_DIMS, lux_random,           // random.cc
    "%2%seed:period:1uniform:2normal:3sample:4shuffle:5bits" },
  { "randomb",  3, MAX_DIMS, lux_randomb, "%2%seed:1long" },        // random.cc
  { "randomd",  3, MAX_DIMS, lux_randomd, "%1%seed" },              // random.cc
  { "randoml",  3, MAX_DIMS, lux_randoml, "%2%seed:1double" },      // random.cc
  { "randomn",  3, MAX_DIMS, lux_randomn, "%2%seed" },              // random.cc
  { "randomu",  3, MAX_DIMS, lux_randomu, "%2%seed:period" },       // random.cc
  { "readf",    2, MAX_ARG, lux_readf_f, "1askmore:2word" },        // files.cc
  { "readu",    2, MAX_ARG, lux_readu_f, 0 },                       // files.cc
  { "real",     1, 1, lux_real, 0 },                                // fun3.cc
  { "redim",    1, 9, lux_redim_f, 0 },                             // subsc.cc
#if HAVE_REGEX_H
  { "regex",    1, 2, lux_regex, "1case" }, // regex.cc
#endif
  { "regrid",   5, 5, lux_regrid, 0 },     // fun4.cc
  { "regrid3",  5, 5, lux_regrid3, 0 },    // fun4.cc
  { "regrid3ns", 5, 5, lux_regrid3ns, 0 }, // fun4.cc
#if DEVELOP
  { "regridls", 5, 5, lux_regridls, 0 }, // fun4.cc
#endif
  { "reorder",  2, 2, lux_reorder, 0 },                        // fun6.cc
  { "restore",  2, 3, lux_fzread_f, "1printheader" },          // files.cc
  { "reverse",  1, MAX_ARG, lux_reverse, "1zero" },            // subsc.cc
  { "rfix",     1, 1, lux_rfix, "*" },                         // symbols.cc
  { "roll",     2, 2, lux_roll, 0 },                           // subsc.cc
  { "root3",    3, 3, lux_root3, 0 },                          // orientation.cc
  { "runcum",   1, 3, lux_runcum, "*1partial_width:2varsum" }, // strous.cc
  { "runprod",  1, 2, lux_runprod, "*" },                      // strous2.cc
  { "scale",    1, 3, lux_scale, "*1fullrange:2zoom" },        // fun3.cc
  { "scalerange", 3, 5, lux_scalerange, "*1fullrange:2zoom" }, // fun3.cc
  { "scanf",    2, MAX_ARG, lux_freadf_f, "|1|1eof" },         // files.cc
  { "sdev",     1, 3, lux_sdev,                                // fun2.cc
    "::weights:*0sample:1population:2keepdims:4double:8omitnans" },
  { "segment",  1, 3, lux_segment, ":sign:diagonal:1degree" }, // topology.cc
  { "segmentdir", 2, 3, lux_segment_dir, "::sign" },           // topology.cc
  { "sgn",      1, 1, lux_sgn, "*" },                          // fun1.cc
  { "shift",    1, 4, lux_shift_f, ":::blank:1translate" },    // strous2.cc
  { "shift3",   2, 3, lux_shift3, 0 },                         // fun4.cc
  { "siderealtime", 1, 1, lux_siderealtime, "1atzerotime" },   // astron.cc
  { "sieve",    1, 2, lux_sieve, 0 },                          // fun3.cc
  { "sin",      1, 1, lux_sin, "*" },                          // fun1.cc
  { "sinh",     1, 1, lux_sinh, "*" },                         // fun1.cc
  { "skipc",    2, 2, lux_skipc, 0 },                          // fun2.cc
  { "smap",     1, 1, lux_smap, "1truncate:2array" },          // subsc.cc
  { "smooth",   1, 3, lux_smooth, "1partial_width:4all" },     // strous.cc
  { "solar_b",  1, 1, lux_solar_b, "*2modern" },               // ephem2.cc
  { "solar_l",  1, 1, lux_solar_l, "*1full:2modern" },         // ephem2.cc
  { "solar_p",  1, 1, lux_solar_p, "*2modern" },               // ephem2.cc
  { "solar_r",  1, 1, lux_solar_r, "*" },                      // ephem2.cc
  { "sort",     1, 1, lux_sort, "*1heap:2shell:4axis" },       // fun4.cc
  { "spawn",    1, 1, lux_spawn_f, 0 },                        // files.cc
  { "sprintf",  1, MAX_ARG, lux_fstring, "1skip_undefined" },  // fun2.cc
  { "sqrt",     1, 1, lux_sqrt, "*" },                         // fun1.cc
  { "sscanf",   3, MAX_ARG, lux_freads_f, "1countspaces" },    // files.cc
  { "store",    2, 3, lux_fzwrite_f, "1safe" },                // files.cc
  { "str",      1, MAX_ARG, lux_string, 0 },                   // fun2.cc
  { "strarr",   2, 1 + MAX_DIMS, strarr, "%1%size" },          // symbols.cc
  { "strcount", 2, 2, lux_strcount, 0 },                       // fun2.cc
  { "stretch",  2, 2, lux_stretch, 0 },                        // fun4.cc
  { "string",   1, MAX_ARG, lux_string, 0 },                   // fun2.cc
  { "strlen",   1, 1, lux_strlen, 0 },                         // fun2.cc
  { "strloc",   2, 2, lux_strloc, 0 },                         // fun2.cc
  { "strpbrk",  2, 2, lux_strpbrk, 0 },                        // fun2.cc
  { "strpos",   2, 3, lux_strpos, 0 },                         // fun2.cc
  { "strr",     3, 4, lux_strreplace, 0 },                     // fun2.cc
  { "strreplace", 3, 4, lux_strreplace, 0 },                   // fun2.cc
  { "strskp",   2, 2, lux_strskp, 0 },                         // fun2.cc
  { "strsub",   3, 3, lux_strsub, 0 },                         // fun2.cc
  { "strtok",   1, 2, lux_strtok, "1skip_final_delim" },       // fun2.cc
  { "strtol",   1, 2, lux_strtol, 0 },                         // fun3.cc
  { "strtrim",  1, 1, lux_strtrim, 0 },                        // fun2.cc
#if DEVELOP
  { "struct",   2, MAX_ARG, lux_struct, 0 }, // install.cc
#endif
  { "student",  2, 2, lux_student, "*1complement:2log" }, // fun1.cc
  { "sun_b",    2, 2, lux_sun_b, 0 },                     // ephem.cc
  { "sun_d",    2, 2, lux_sun_d, 0 },                     // ephem.cc
  { "sun_p",    2, 2, lux_sun_p, 0 },                     // ephem.cc
  { "sun_r",    2, 2, lux_sun_r, 0 },                     // ephem.cc
  { "symbol",   1, 1, lux_symbol, "1main" },              // symbols.cc
  { "symclass", 1, 1, lux_symclass, "+|1|1follow:2number" }, // subsc.cc
  { "symdtype", 1, 1, lux_symdtype, 0 },                     // subsc.cc
  { "symmem",   0, 0, lux_symbol_memory, 0 },                // install.cc
  { "symnum",   1, 1, lux_symbol_number, 0 },                // install.cc
  { "table",    3, 4, lux_table, "|2|1all:2middle" },        // strous.cc
  { "tai_from_date", 5, 5, lux_tai_from_date, 0 },           // ephem.cc
  { "tan",      1, 1, lux_tan, "*" },                        // fun1.cc
  { "tanh",     1, 1, lux_tanh, "*" },                       // fun1.cc
  { "temporary", 1, 1, lux_temp, 0 },                        // strous2.cc
  { "tense",    3, 6, lux_tense, 0 },                        // fun3.cc
  { "tense_curve", 3, 6, lux_tense_curve, 0 },               // fun3.cc
  { "tense_loop", 3, 4, lux_tense_loop, 0 },                 // fun3.cc
#if DEVELOP
  { "test",     2, 3, lux_test, 0 }, // execute.cc
#endif
  { "total",    1, 4, lux_total, // fun1.cc
    "::power:weights:1double:2keepdims:4float:8omitnans" },
  { "trend",    1, 2, lux_trend, "*" },            // fun2.cc
  { "tri_name_from_tai", 1, 1, lux_tri_name_from_tai, 0 }, // ephem.cc
  { "typename", 1, 1, lux_typeName, 0 },                   // install.cc
  { "upcase",   1, 1, lux_upper, 0 },                      // fun2.cc
  { "upper",    1, 1, lux_upper, 0 },                      // fun2.cc
  { "variance", 1, 3, lux_variance,                        // fun2.cc
    "::weights:*0sample:1population:2keepdims:4double" },
  { "varname",  1, 1, lux_varname, 0 },          // symbols.cc
  { "voigt",    2, 2, lux_voigt, "*" },          // fun1.cc
  { "wmap",     1, 1, lux_wmap, 0 },             // subsc.cc
  { "writeu",   2, MAX_ARG, lux_writeu, 0 },     // files.cc
  { "zero",     1, 2, lux_zerof, "*" },            // fun1.cc
  { "zeronans", 1, 2, lux_zapnan_f, "*%1%value" }, // fun1.cc
  { "zinv",     1, 1, lux_zinv, "*" },             // strous.cc
};
int32_t nFunction = sizeof(function_table)/sizeof(InternalRoutine);
//----------------------------------------------------------------
void undefine(int32_t symbol)
// free up memory allocated for <symbol> and make it undefined
{
  void  zap(int32_t), updateIndices(void);
  char  hasMem = 0;
  int32_t       n, k, oldZapContext, i;
  int16_t       *ptr;
  Pointer       p2;
  ListElem* p;
  ExtractSec* eptr;
  ExtractSec* eptr0;
  extern int32_t        tempSym;
  extern char   restart;

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
      if (isStringType(array_type(symbol))) { // a string array
        // must free the components' memory
        p2.sp = (char**) array_data(symbol);
        n = array_size(symbol);
        while (n--)
          free(*p2.sp++);
      }
      // fall through to generic case to take care of remaining
      // array memory.
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
      if (n > 0)                // string key
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
          free((void*) p->key);
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
        if (*ptr > 0)           // BREAK, CONTINUE, RETALL, RETURN are < 0
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
        if (n > 0               // an expression
            && symbol_context(n) == symbol)
          zap(n);
        eptr = eptr0 = extract_ptr(symbol);
        n = extract_num_sec(symbol);
      } else {                  // PRE_EXTRACT
        free(pre_extract_name(symbol));
        eptr = eptr0 = pre_extract_ptr(symbol);
        n = pre_extract_num_sec(symbol);
      }
      if (n) {
        while (n--) {
          i = eptr->number;
          switch (eptr->type) {
            case LUX_RANGE:
              p2.i16 = eptr->ptr.i16;
              while (i--) {
                if (symbol_context(*p2.i16) == symbol
                    || (zapContext > 0 && symbol_context(*p2.i16) == zapContext))
                  zap(*p2.i16);
                p2.i16++;
              }
              free(eptr->ptr.i16);
              break;
            case LUX_LIST:
              p2.sp = eptr->ptr.sp;
              while (i--)
                free(*p2.sp++);
              free(eptr->ptr.sp);
              break;
          } // end of switch (eptr->type)
          eptr++;
        } // end of while (n--)
        free(eptr0);
      }         // end of if (n)
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
          // fall through to the below case
        case EVB_INT_SUB: case EVB_INSERT: case LUX_INT_FUNC: case
        LUX_USR_FUNC: case EVB_CASE: case EVB_NCASE: case EVB_BLOCK:
          n = symbol_memory(symbol)/sizeof(int16_t);
          ptr = (int16_t*) symbol_data(symbol);
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
  undefined_par(symbol) = (Symboltype) 0; // used in usr_routine() (with value 1)
                                // to indicate unspecified parameters
                                // of user-defined routines.
// context must remain the same, or local variables become global
// when they are undefined (e.g. as lhs in a replacement)
}
//----------------------------------------------------------------
void zap(int32_t nsym)
// undefine & remove name (if any)
{
 char const *name, *noName = "[]";
 int32_t        context, hashValue;
 HashTableEntry         *hp, *oldHp, **hashTable;
#if DEBUG
 void   checkTemps(void);
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
 if (*name != '[' && hashValue >= 0) { // has name
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
       free((void*) hp->name);          // found name; remove
       hp->name = 0;
       if (hp != oldHp)
         oldHp->next = hp->next;
       else
         hashTable[hashValue] = 0;
       free(hp);
       oldHp = 0;               // signal that name was found
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
//----------------------------------------------------------------
void cleanUp(int32_t context, int32_t which)
// names in symbolStack are supposed to be removed after use by the
// routines that use them.  only resetting of the index is done here.
// all temporary variables that have the specified context are removed
// <which> can be:  CLEANUP_VARS  CLEANUP_EDBS  CLEANUP_ALL
// CLEANUP_COMP CLEANUP_ERROR
{
  char  comp;
  int32_t       i;
  void  zapParseTemps(void);

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
//----------------------------------------------------------------
void cleanUpRoutine(int32_t context, char keepBase)
// completely removes all traces of the routine with the given context
// keeps the LUX_SUBROUTINE, LUX_FUNCTION, or LUX_BLOCKROUTINE symbol
/* itself if keepBase is unequal to zero.  If keepBase is unequal to
 zero, then an LUX_DEFERRED_SUBR is transformed into an (empty)
 LUX_SUBR, LUX_DEFERRED_FUNC into an LUX_FUNC, and LUX_DEFERRED_BLOCK
 into an LUX_BLOCK.  LS 19feb97 21may99 */
{
  char  mem;
  int32_t       n;
  int16_t       *ptr;

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
        while (n--)             // get rid of the parameters
          zap(*ptr++);
        routine_num_parameters(context) = 0;
        n = routine_num_statements(context);
        if (n)
          mem = 1;
        ptr = routine_statements(context);
        while (n--) {           // get rid of the statements
          if (*ptr > 0)                 // RETALL, RETURN, BREAK, CONTINUE < 0
            zap(*ptr);
          ptr++;
        }
        routine_num_statements(context) = 0;
        if (mem)                        // if the routine had any statements
          // or parameters, then we need to free the memory that was used to
          // store them in
          free(routine_parameters(context));
        break;
    }
  } else
    zap(context);
}
//----------------------------------------------------------------
void updateIndices(void)
// moves tempVariableIndex and tempExecutableIndex as far back as
// possible
{
  while (tempVariableIndex > TEMPS_START
         && symbol_class(tempVariableIndex - 1) == LUX_UNUSED)
    tempVariableIndex--;
  while (tempExecutableIndex > TEMP_EXE_START
         && symbol_class(tempExecutableIndex - 1) == LUX_UNUSED)
    tempExecutableIndex--;
}
//----------------------------------------------------------------
int32_t nextFreeStackEntry(void)
     // returns index to next free item in symbolStack.  May cycle.
{
  int32_t       oldIndex = symbolStackIndex;

  while (symbolStack[symbolStackIndex]) {
    if (++symbolStackIndex == SYMBOLSTACKSIZE)
      symbolStackIndex = 0;
    if (symbolStackIndex == oldIndex)
      return luxerror("Symbol stack full", 0);
  }
  nSymbolStack++;
  return symbolStackIndex;
}
//----------------------------------------------------------------
int32_t nextFreeNamedVariable(void)
     /* returns index to next free named variable in symbol table.
        some may have been zapped in the meantime, so cycle if at end of
        table */
{
  int32_t       oldIndex = namedVariableIndex;
  extern int32_t        compileLevel;

  while (symbol_class(namedVariableIndex)) {
    if (++namedVariableIndex == NAMED_END) namedVariableIndex = NAMED_START;
    if (namedVariableIndex == oldIndex)    // nothing free
      return luxerror("Too many named variables - symbol table full", 0);
  }
  sym[namedVariableIndex].exec = nExecuted;
  sym[namedVariableIndex].context = -compileLevel;
  sym[namedVariableIndex].line = curLineNumber;
  nNamedVariable++;
  return namedVariableIndex++;
}
//----------------------------------------------------------------
int32_t nextFreeTempVariable(void)
// returns index to next free temporary variable in symbol table
{
 extern int32_t         compileLevel;
 int32_t oldIndex = tempVariableIndex;
 static long int count = 0;

 ++count;
 while (symbol_class(tempVariableIndex)) {
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
//----------------------------------------------------------------
int32_t nextFreeTempExecutable(void)
// returns index to next free temporary executable in symbol table
{
  int32_t oldIndex = tempExecutableIndex;
  extern int32_t        compileLevel;

  while (symbol_class(tempExecutableIndex)) {
    if (++tempExecutableIndex == EXE_END)
      tempExecutableIndex = EXE_START;
    if (tempExecutableIndex == oldIndex) // nothing free
      return luxerror("Too many temporary executables - symbol table full", 0);
  }
  sym[tempExecutableIndex].exec = nExecuted;
  sym[tempExecutableIndex].context = -compileLevel;
  sym[tempExecutableIndex].line = curLineNumber;
  nTempExecutable++;
  return tempExecutableIndex++;
}
//----------------------------------------------------------------
int32_t nextFreeExecutable(void)
/* returns index to next free executable in symbol table
   some may have been zapped in the meantime, so cycle if at end
   of table */
{
  int32_t    oldIndex = executableIndex;
  extern int32_t        compileLevel;

  while (symbol_class(executableIndex)) {
    if (++executableIndex == EXE_END)
      executableIndex = EXE_START;
    if (executableIndex == oldIndex)    // nothing free
      return luxerror("Too many permanent executables - symbol table full", 0);
  }
  sym[executableIndex].exec = nExecuted;
  sym[executableIndex].context = -compileLevel;
  sym[executableIndex].line = curLineNumber;
  nExecutable++;
  return executableIndex++;
}
//----------------------------------------------------------------
int32_t nextFreeUndefined(void)
// returns a free undefined temporary variable
{
  extern int32_t        compileLevel;
  int32_t       n;

  n = nextFreeTempVariable();
  if (n < 0) return n;          // some error
  symbol_class(n) = LUX_UNDEFINED;
  sym[n].context = -compileLevel;
  return n;
}
//----------------------------------------------------------------
void pushList(int16_t symNum)
// pushes a symbol number unto the list stack
{
 if (listStackItem - listStack < NLIST) {
   *listStackItem++ = symNum;
   return;
 }
 luxerror("Too many elements (%d) in list; list stack full\n", 0,
       listStackItem - listStack);
}
//----------------------------------------------------------------
int16_t popList(void)
// pops a symbol number from the list stack
{
 if (listStackItem > listStack)
   return *--listStackItem;
 return luxerror("Attempt to read from empty list stack\n", 0);
}
//----------------------------------------------------------------
int32_t moveList(int32_t n)
// moves the topmost <n> entries on the list stack over by one
{
  if (listStackItem - listStack < NLIST)
  { memcpy(listStack + 1, listStack, n*sizeof(int16_t));
    listStackItem++;
    return 1; }
  return luxerror("Too many elements (%d) in list; list stack full\n", 0,
               listStackItem - listStack);
}
//----------------------------------------------------------------
void swapList(int32_t n1, int32_t n2)
// swaps the elements <n1> and <n2> positions down the list stack
{
 int32_t        temp;

 temp = listStackItem[-n2];
 listStackItem[-n2] = listStackItem[-n1];
 listStackItem[-n1] = temp;
}
//----------------------------------------------------------------
int32_t stackListLength(void)
// returns the number of elements in the topmost list in the stack
// assumes that all lists are delimited by LUX_NEW_LIST
{
 int16_t        *i = listStackItem - 1;
 int32_t        n = 0;

 if (i < listStack)
   return -1;           // no list in stack
 while (i >= listStack && *i != LUX_NEW_LIST) {
   i--;
   n++;
 }
 return n;
}
//----------------------------------------------------------------
void dupList(void)
// duplicates the topmost list
{
  int32_t       n, n2;

  n = n2 = stackListLength();
  pushList(LUX_NEW_LIST);
  while (n2--)
    pushList(listStackItem[-n-1]);
}
//----------------------------------------------------------------
void unlinkString(int32_t index)
// zero symbolStack[index] and update symbolStackIndex,
// but do not free the memory
{
  symbolStack[index] = 0;
  if (index < symbolStackIndex)
    symbolStackIndex = index;
  while (symbolStackIndex && !symbolStack[symbolStackIndex - 1])
    symbolStackIndex--;
  nSymbolStack--;
}
//----------------------------------------------------------------
void freeString(int32_t index)
     // removes the string (if any) from index <index> in symbolStack[]
     // and updates symbolStackIndex
{
  free((void*) symbolStack[index]);
  unlinkString(index);
}
//----------------------------------------------------------------
int32_t installStruct(int32_t base, int32_t key)
// returns a struct pointer, supports variables, user-functions,
// and user-routines as structures
{
  int32_t       n;

  n = lookForVar(base, curContext); // seek user-defined item
  if (n < 0)                    // no such item found
    n = lookForSubr(base);      // seek internal subroutine
  if (n < 0)                    // no such internal subroutine
    n = lookForFunc(base);      // seek internal function
  if (n < 0)                    // no such internal function
    n = findVar(base, curContext); // force variable
  else                          // found something already, remove name
    freeString(base);
  return newSymbol(LUX_LIST_PTR, n, key);
}
//----------------------------------------------------------------
int32_t copyElement(int32_t symbol, char *key)
// return a complete copy of variable <symbol>, with name (key) <key>, for
// inclusion in a list, structure, or range.  such a variable must
// not be a temporary, to prevent its premature deletion.
{
  int32_t       n;
  int32_t       lux_replace(int32_t, int32_t);

  if ((n = installString(key)) < 0) return n;
  if ((n = findVar(n, 0)) < 0) return n;
  if (lux_replace(n, symbol) < 0) return -1;
  return n;
}
//----------------------------------------------------------------
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
  int32_t       result;
  FILE  *fp;
  int32_t       nextCompileLevel(FILE *, char const *);

  // seek a named variable
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
  // seek a user-defined function
  result = lookForName(name, funcHashTable, 0);
  if (result >= 0) {
    *type = LUX_FUNCTION;       // a user-defined function
    return result;
  }
  if (allowSubr) {
    // seek a user-defined subroutine
    result = lookForName(name, subrHashTable, 0);
    if (result >= 0) {
      *type = LUX_SUBROUTINE;
      return result;
    }
  }
  // seek a built-in function
  result = findInternalName(name, 0);
  if (result >= 0) {
    *type = LUX_INT_FUNC;       // an internal function
    return result;
  }
  /* not a built-in function either.  Try to compile a
     user-defined function */
  fp = openPathFile(name, FIND_FUNC | FIND_LOWER);
  if (fp) {
    // found it
    nextCompileLevel(fp, name); // compile it
    fclose(fp);
    result = lookForName(name, funcHashTable, 0); // seek again
    if (result < 0) {
      *type = LUX_ERROR;
      return luxerror("Compiled file %s but function %s still is not compiled",
                   0, expname, name);
    } else {
      *type = LUX_FUNCTION;
      return result;
    }
  }
  // we did not find the target anywhere
  *type = LUX_ERROR;
  return LUX_ERROR;
}
//----------------------------------------------------------------
int32_t newSymbol(Symbolclass kind, ...)
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
  int32_t               n, i, narg, isStruct, isScalarRange, j, target, depth;
  extern char   reportBody, ignoreSymbols, compileOnly;
  ExtractSec* eptr;
  int16_t       *ptr;
  Pointer       p;
#if YYDEBUG
  extern int32_t        yydebug;
#endif
  // static char        inDefinition = 0;
  int16_t               *arg;
  va_list       ap;
  int32_t       int_arg(int32_t);
  void  fixContext(int32_t, int32_t);

 // don't generate symbols in bodies of routines when using @@file
 // (reportBody) and when defining a routine (inDefinition).
// ignoreSymbols = (reportBody && inDefinition);
  va_start(ap, kind);
  n = -1;
  if (!ignoreSymbols) {                 // really need a new symbol.  what kind?
    if (kind >= LUX_BIN_OP) {
                                // executable
      if (keepEVB)
        n = nextFreeExecutable();
      else
        n = nextFreeTempExecutable();
      if (n < 0) {
        va_end(ap);
        return LUX_ERROR;       // didn't work
      }
      symbol_class(n) = kind;
      if (keepEVB)
        symbol_context(n) = curContext;
    } else if (kind < LUX_SUBROUTINE) {         // named variable
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
        scalar_type(n) = (Symboltype) va_arg(ap, int);
        break;
      case LUX_FIXED_NUMBER:
        // same as an ordinary scalar, but in EDB symbol space, so that it
        // doesn't get overwritten as "just another temp" (i.e. isFreeTemp()
        // returns 0).  otherwise, get problems e.g. in a for-loop, where
        // temp numbers are used more than once.
        symbol_type(n) = (Symboltype) va_arg(ap, int);
        if (symbol_type(n) >= LUX_CFLOAT) { // a complex scalar
          symbol_class(n) = LUX_CSCALAR;
          complex_scalar_memory(n) = lux_type_size[symbol_type(n)];
          complex_scalar_data(n).f = (float*) malloc(complex_scalar_memory(n));
          if (!complex_scalar_data(n).f)
            return cerror(ALLOC_ERR, n);
        } else
          symbol_class(n) = LUX_SCALAR;
        break;
      case LUX_FIXED_STRING:
        // a literal string
        symbol_class(n) = LUX_STRING;
        sym_string_type(n) = LUX_LSTRING;
        string_value(n) = (char*) symbolStack[i = va_arg(ap, int32_t)];
        symbol_memory(n) = strlen(symbolStack[i]) + 1; // count \0
        unlinkString(i);                // free position in stack
        break;
      case LUX_STRUCT_PTR:
        symbol_class(n) = LUX_STRUCT_PTR;
        symbol_memory(n) = sizeof(StructPtr);
        struct_ptr_elements(n) = (StructPtr*) malloc(symbol_memory(n));
        break;
      case LUX_EXTRACT:
        target = popList();
        if (target > 0) {       // regular symbol
          extract_target(n) = target;
          depth = popList();
          extract_num_sec(n) = (Symboltype) depth;
          symbol_memory(n) = depth*sizeof(ExtractSec);
          extract_ptr(n) = eptr = (ExtractSec*) malloc(symbol_memory(n));
          embed(target, n);
        } else {
          i = -target;
          symbol_class(n) = LUX_PRE_EXTRACT;
          symbol_memory(n) = sizeof(PreExtract);
          symbol_data(n) = malloc(sizeof(PreExtract));
          pre_extract_name(n) = (char*) symbolStack[i];
          unlinkString(i);      // so it does not get zapped
          depth = popList();
          pre_extract_num_sec(n) = (Symboltype) depth;
          pre_extract_ptr(n) = eptr = (ExtractSec*) (depth? malloc(depth*sizeof(ExtractSec)): NULL);
        }
        if (!eptr && depth)
          return cerror(ALLOC_ERR, 0);
        ptr = listStackItem;
        eptr += depth;          // start at end of list
        if (!depth)
          popList();
        while (depth--) {
          ptr--;                // skip the (potential) LUX_NEW_LIST
          eptr--;               // go to previous entry
          eptr->type = popList(); /* the type of the current list;
                                    either LUX_RANGE or LUX_LIST */
          eptr->number = stackListLength(); /* the number of entries in this
                                              one */
          i = eptr->number;
          switch (eptr->type) {
            case LUX_RANGE:
              eptr->ptr.i16 = (int16_t*) malloc(i*sizeof(int16_t));
              p.i16 = eptr->ptr.i16 + i; // start at the end
              while (i--) {
                *--p.i16 = popList();
                embed(*p.i16, n);
              }
              break;
            case LUX_LIST:
              eptr->ptr.sp = (char**) malloc(i*sizeof(char *));
              p.sp = eptr->ptr.sp + i; // start at the end
              while (i--) {
                j = popList();
                *--p.sp = strsave(string_value(j));
                if (isFreeTemp(j))
                  zap(j);
              }
              break;
          }
          popList();            // remove the LUX_NEW_LIST
        }
        break;
      case LUX_META:            // a meta symbol, i.e. a string expression
                                // which points at a symbol
        meta_target(n) = va_arg(ap, int32_t);
        embed(meta_target(n), n);
        break;
      case LUX_RANGE:  // a range
        isScalarRange = 1;
        // range start:
        i = va_arg(ap, int32_t);
        range_start(n) = i;
        if (i < 0)
          i = -i;
        embed(i, n);
        if (symbol_class(i) != LUX_SCALAR)
          isScalarRange = 0;
        // range end:
        i = va_arg(ap, int32_t);
        range_end(n) = i;
        if (i < 0)
          i = -i;
        embed(i, n);
        if (symbol_class(i) != LUX_SCALAR)
          isScalarRange = 0;
        range_sum(n) = 0; // default summation flag
        range_redirect(n) = -1; // redirection flag
        range_scalar(n) = (Symboltype) isScalarRange;
        break;
      case LUX_PRE_RANGE:
        isScalarRange = 1;
        // pre_range start:
        i = va_arg(ap, int32_t);
        pre_range_start(n) = i;
        if (i < 0)
          i = -i;
        embed(i, n);
        if (symbol_class(i) != LUX_SCALAR)
          isScalarRange = 0;
        // pre_range end:
        i = va_arg(ap, int32_t);
        pre_range_end(n) = i;
        if (i < 0)
          i = -i;
        embed(i, n);
        if (symbol_class(i) != LUX_SCALAR)
          isScalarRange = 0;
        pre_range_sum(n) = 0; // default summation flag
        pre_range_redirect(n) = -1; // redirection flag
        pre_range_scalar(n) = (Symboltype) isScalarRange;
        break;
      case LUX_LIST_PTR:                // pointer to a struct element
        list_ptr_target(n) = va_arg(ap, int32_t); // the struct
        if ((i = va_arg(ap, int32_t)) >= 0) { // non-numerical key
          list_ptr_tag_string(n) = (char*) symbolStack[i]; // key
         list_ptr_tag_size(n) = strlen(symbolStack[i]) + 1;
         unlinkString(i);       // unlink keyword from stack
       } else {                         // numerical key;  must be integer
         j = int_arg(-i);
         if (symbol_context(-i) == curContext
             && ((-i >= TEMP_EXE_START && -i < TEMP_EXE_END)
                 || (-i >= EXE_START && -i < EXE_END)))
           // a literal number
           zap(-i);
         list_ptr_target(n) = -list_ptr_target(n);
                                // negative indicates numerical
         list_ptr_tag_number(n) = j;
       }
        break;
      case LUX_LIST: case LUX_PRE_LIST: // includes LISTs
        narg = stackListLength()/2; // # of name-value pairs
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
          ListElem* p;
          i = narg*(sizeof(ListElem));
          if (!(list_symbols(n) = (ListElem *) malloc(i))) {
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
            p->key = (char*) ((i >= 0)? symbolStack[i]: NULL);
            if (i >= 0)
              unlinkString(i);  // unlink from symbolStack
            p--;
          }
        } else {                // must be a list
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
        popList();                              // pop LUX_NEW_LIST
        break;
      case LUX_SUBSC_PTR:       // subscript pointer
        if (!(symbol_data(n) = (int32_t *) malloc(4*sizeof(int32_t)))) {
          va_end(ap);
          printf("newSymbol: ");
          return cerror(ALLOC_ERR, 0);
        }
        symbol_memory(n) = 4*sizeof(int32_t);
        break;
      case LUX_POINTER:
        transfer_is_parameter(n) = (Symboltype) 0; // not a formal argument in a
                                        // user-defined function or routine
        narg = va_arg(ap, int32_t);
        i = lookForSubr(narg);
        if (i < 0)
          i = lookForFunc(narg);
        if (i < 0)
          i = lookForBlock(narg);
        if (i >= 0) {           // found a routine
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
        if (i >= 0) {           // found an internal routine
          symbol_class(n) = LUX_FUNC_PTR;
          func_ptr_routine_num(n) = -i;
          func_ptr_type(n) = (Symboltype) kind;
          unlinkString(narg);
          return n;
        }
        i = findVar(narg, curContext);
        if (i < 0)
          return i;             // some error
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
      { int16_t         nArg, nStatement;
        int32_t         oldContext;
        char ** key;

        n = i = va_arg(ap, int32_t);
        if (n >= 0) {           // first pass: n = index of routine's name
                                // in symbolStack[]
          switch (kind)         {       // first see if the routine is already
                                // defined
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
          if (n == -1)          // the routine is not yet defined
            switch (kind) {     // so allocate a symbol for it
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
          else {                // the routine is already known
            if (!reportBody)    // need to compile body
              cleanUpRoutine(n, 1); // get rid of old definition,
                                     // but keep routine symbol
            freeString(i);      // deallocate symbolStack[i]
          }
          if (n == -1) {        // couldn't get new symbol
            va_end(ap);
            reportBody = 0;
            return LUX_ERROR;   // pass on the error
          }
          symbol_class(n) = kind;

          if (kind != LUX_BLOCKROUTINE) { // has parameters
            nArg = stackListLength();   // # parameters
            if (!reportBody) {  // we're compiling the body
              if (listStackItem[-1] == LUX_EXTEND) { // extended parameter
                routine_has_extended_param(n) = 1;
                nArg--;
              } else
                routine_has_extended_param(n) = 0;
              if (nArg > UINT8_MAX) { // too many parameters
                va_end(ap);
                reportBody = 0;
                return luxerror("More than %1d parameters specified\n", n,
                             UINT8_MAX);
              }
              routine_num_parameters(n) = nArg;
              if (nArg)
              {
                // For unknown reasons, g++ -Werror version 14.0.1 on Fedora 40
                // produces an "alloc-size-larger-than" compiler error if
                // variable s has type size_t in the next statement.  nArg < 256
                // and sizeof(int16_t) == 2 so s < 512, which fits in a
                // uint16_t.
                uint16_t s = nArg*sizeof(int16_t);
                if (!(routine_parameters(n)
                      = (int16_t *) malloc(s)))
                {
                  // could not allocate room for parameters
                  va_end(ap);
                  reportBody = 0;
                  return luxerror("Routine-definition memory-allocation error", 0);
                }
              }
            } else              // deferred compilation
              symbol_class(n) = (kind == LUX_SUBROUTINE)?
                LUX_DEFERRED_SUBR: LUX_DEFERRED_FUNC;
          } else {              // a block routine, which has no parametrs
            if (reportBody)
              symbol_class(n) = LUX_DEFERRED_BLOCK;
            routine_num_parameters(n) = nArg = 0;
            routine_has_extended_param(n) = 0;
          }
          if (!reportBody) {    // compiling the body
            if (nArg >= 1
                && routine_has_extended_param(n)) { // extended arg
              popList();
            }
            arg = routine_parameters(n) + nArg;
            // now save parameters (start at back)
            if (kind != LUX_BLOCKROUTINE) {
              if (nArg)
              {
                auto p = static_cast<char**>(malloc(nArg*sizeof(char*)));
                if (p)
                {
                  routine_parameter_names(n) = p;
                }
                else
                {
                  // could not allocate memory to store the parameter names
                  va_end(ap);
                  return luxerror("Memory allocation error", 0);
                }
              }
              key = routine_parameter_names(n) + nArg;
              while (nArg--) {
                *--arg = findVar(popList(), n); // parameter's symbol #
                *--key = (char*) varName(*arg); // parameter's name
                symbol_class(*arg) = LUX_POINTER;
                transfer_target(*arg) = 0;
                transfer_temp_param(*arg) = 0;
                transfer_is_parameter(*arg) = (Symboltype) 1;
              }
              popList();                // remove list start marker
            }
            routine_num_statements(n) = curContext; // temporarily store
                                                     // current context
             // variables in SUBRs and FUNCs are local to that particular
             // SUBR or FUNC, whereas variables in BLOCKs are local to the
             // embedding SUBR or FUNC or main level.
            if (kind != LUX_BLOCKROUTINE)
              curContext = n;   /* make current function or subroutine
                                   the current context */
          } else {              // not compiling the body
            if (kind != LUX_BLOCKROUTINE) {
              while (nArg--) {  // discard the parameters
                i = popList();
                freeString(i);
              }
              popList();
            }
            // save the name of the file in which the definition of
            // this routine can be found if necessary
            deferred_routine_filename(n) = strsave(currentInputFile);
          }
          keepEVB++;            // executables must be preserved
          if (reportBody)
            ignoreSymbols = 1;  // ignore symbols during compilation of
                                // the routine
          return n;
        } else {                // second pass
          n = i = -n - 1;       // routine #
          oldContext = routine_num_statements(n); // save for later
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
          if (kind == LUX_BLOCKROUTINE) { // allocate space for the
            // statements (since there are no parameters, this is at
            // the beginning of the combined parameters+statements list)
            if (nStatement &&
                !(routine_parameters(n) =
                  (int16_t *) malloc(nStatement*sizeof(int16_t)))) {
              va_end(ap);
              curContext = oldContext;  // restore context
              ignoreSymbols = 0;
              return
                luxerror("Allocation error in 2nd pass of routine definition",
                      0);
            }
            nArg = 0;           // no parameters to a block routine
          } else {                      // subroutine or function
            nArg = routine_num_parameters(n);
            if (nArg)           // reallocate memory for combined
                                // parameters+statements list
              routine_parameters(n) =
                (int16_t *) realloc(routine_parameters(n),
                                 (nArg + nStatement)*sizeof(int16_t));
            else                // no parameters, just allocate space for
                                // statements
              routine_parameters(n) =
                (int16_t *) malloc(nStatement*sizeof(int16_t));
            if (!routine_parameters(n)) { // allocation failed
              va_end(ap);
              curContext = oldContext;  // restore context
              ignoreSymbols = 0;
              return
                luxerror("Allocation error in 2nd pass of routine definition",
                      0);
            }
          }
          arg = routine_parameters(n) + nStatement + nArg; // end of list
          if (nStatement) {
            while (nStatement--) { // store statements, starting at end
              *--arg = popList();
              embed(*arg, n);   // give the statement and all enclosed
            }
            // symbols the context of the routine: this makes the
            // statement local to the routine
            popList();          // remove beginning-of-list marker
          }
          curContext = oldContext; // restore context now definition is
                                    // done
          ignoreSymbols = 0;    // no longer ignore symbols - if we were
                                // doing that
          keepEVB--;            // executables are again more temporary
          symbol_context(n) = 0; // routines always have context 0
          usr_routine_recursion(n) = (Symboltype) 0; // no recursion yet
          return 0;
        }
      }
      case LUX_EVB: case LUX_INT_FUNC: case LUX_USR_FUNC:
        if (kind == LUX_EVB) {
          kind = (Symbolclass) va_arg(ap, int);
          symbol_type(n) = (Symboltype) kind;
        }
        i = 0;                  // default: no more items to retrieve
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
            usr_code_routine_num(n) = va_arg(ap, int32_t); // routine number
            break;
          case EVB_FILE:
            i = va_arg(ap, int32_t);    // index to string
            file_name(n) = (char*) symbolStack[i];
            symbol_memory(n) = strlen(symbolStack[i]) + 1;
            unlinkString(i);
            i = va_arg(ap, int32_t);    /* include type: INCLUDE -> always,
                                           REPORT -> only if necessary */
            file_include_type(n) = i;
            i = 0;              // no more items to retrieve
            break;
          case EVB_INT_SUB: case EVB_USR_SUB: case EVB_INSERT:
          case LUX_INT_FUNC: case LUX_USR_FUNC:
            sym[n].xx = va_arg(ap, int32_t); // routine number (SUB) or target
          case EVB_CASE: case EVB_NCASE: case EVB_BLOCK:
            i = stackListLength();              // # of expr and statements
            if (i) {                    // only if there are any elements
              if (!(arg = (int16_t *) malloc(i*sizeof(int16_t)))) {
                va_end(ap);
                return luxerror("Could not allocate memory for stacked elements",
                             0);
              }
              symbol_data(n) = arg; // the elements
              symbol_memory(n) = i*sizeof(int16_t);     // the memory size
              arg += i;         // start with the last element (which is
              // on top of the stack)
              while (i--) {     // all elements
                *--arg = popList(); // pop element from stack
                embed(*arg, n);  // context is enclosing statement
              }
            } else
              symbol_memory(n) = 0; // no args
            popList();          // pop LUX_NEW_LIST marker
            i = 0;              // no more items to retrieve
            break;
        }
        if (i > 0) {
          arg = sym[n].spec.evb.args;
          while (i--) {
            *arg = va_arg(ap, int32_t);
            embed(*arg, n);
            arg++;
          }
          if ((EVBclass) kind == EVB_FOR) {
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
  } else {                      // reportBody & in definition
    switch (kind) {
      case LUX_FIXED_STRING:
        i = va_arg(ap, int32_t);        // index to symbolStack
        freeString(i);
        break;
      case LUX_LIST_PTR:
        i = va_arg(ap, int32_t);        // struct number
        i = va_arg(ap, int32_t);        // key
        if (i >= 0)             // non-numerical key
          freeString(i);
        break;
      case LUX_LIST:  case LUX_PRE_LIST:
        narg = stackListLength()/2;
        while (narg--) {
          popList();            // value
          i = popList();        // key
          if (i >= 0)           // string key
            freeString(i);
        }
        break;
      case LUX_KEYWORD:
        i = va_arg(ap, int32_t);
        freeString(i);
        break;
      case LUX_SUBROUTINE: case LUX_FUNCTION: case LUX_BLOCKROUTINE:
        // when we get here, a symbol has already been reserved for the
        // routine, and the parameters have been ignored.  we only need to
        // get rid of the routine body.
        n = va_arg(ap, int32_t);
        n = -n - 1;             // routine #
        routine_num_statements(n) = 0; // no statements
        i = stackListLength();
        while (i--)
          popList();
        popList();
        //       inDefinition = 0;
        keepEVB--;
        ignoreSymbols = 0;      // no longer ignore symbols, if we were
                                // doing that.
        return 0;
      case LUX_EXTRACT:
        target = popList();
        if (target > 0) {       // regular symbol
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
          popList();            // remove the LUX_NEW_LIST
        }
        break;
      case LUX_EVB: case LUX_INT_FUNC: case LUX_USR_FUNC:
        if (kind == LUX_EVB)
          kind = (Symbolclass) va_arg(ap, int);
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
//----------------------------------------------------------------
int32_t hash(char const* string)
{
 int32_t        i;

 for (i = 0; *string; ) i += *string++;
 return i % HASHSIZE;
}
//----------------------------------------------------------------
int32_t ircmp(const void *a, const void *b)
{
  InternalRoutine *ra, *rb;

  ra = (InternalRoutine *) a;
  rb = (InternalRoutine *) b;
  return strcmp(ra->name, rb->name);
}
//----------------------------------------------------------------
int32_t findInternalName(char const* name, int32_t isSubroutine)
/* searches for name in the appropriate subroutine
  or function table.  if found, returns
  index, else returns -1 */
{
  InternalRoutine       *table, *found, key;
  size_t n;

  if (isSubroutine) {
    table = subroutine;
    n = nSubroutine;
  } else {
    table = function;
    n = nFunction;
  }
  key.name = name;
  found = (InternalRoutine*) bsearch(&key, table, n, sizeof(*table), ircmp);
  return found? found - table: -1;
}
//----------------------------------------------------------------
static CompileInfo      *c_info = NULL;
int32_t         cur_c_info = 0, n_c_info = 0;
CompileInfo     *curCompileInfo;

int32_t nextCompileLevel(FILE *fp, char const* fileName)
/* saves the rest of the current input line and starts reading
 input from file fp.  When the file is processed, compilation
 at the current level is resumed. */
{
 int32_t        n, oldZapContext;
 char const* name;
 extern int32_t         echo;
 extern char    inHistoryBuffer, tLine[], *inputString;
 extern uint8_t         disableNewline;
 int32_t        yyparse(void), getStreamChar(void), getStringChar(void);
 extern int32_t         (*getChar)(void);
 CompileInfo    *nextFreeCompileInfo(void);
 void   pegParse(void), removeParseMarker(void), releaseCompileInfo(void);

 curCompileInfo = nextFreeCompileInfo();
 curCompileInfo->line = strsave(currentChar); // save rest of line
 curCompileInfo->charfunc = getChar;
 curCompileInfo->name = currentInputFile? strsave(currentInputFile): NULL;

 if (fp) {
   name = fileName;             // was strsave(fileName)
   getChar = getStreamChar;
 } else {
   name = "(string)";
   getChar = getStringChar;
 }
 currentChar = line;
 compileLevel++;
 oldZapContext = zapContext;
 zapContext = -compileLevel;
 *currentChar = '\0';           // abort compilation of this line
 curCompileInfo->stream = inputStream; // save stream
 inputStream = fp;              // new input stream
 inHistoryBuffer = 0;           // don't save new stuff in history buffer
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
 disableNewline++;              // ignore newlines during parsing
 n = yyparse();                         // parse ; return 0 -> OK, 1 -> error
        // now restore state before this interruption
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
 *line = '\n';                  // must not be \0 or user input is asked for
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
//----------------------------------------------------------------
static int32_t  compileCount = 0;
int32_t compile(char *string)
// compiles string <string> and store in BLOCKROUTINE.  Returns
// but does not execute the block routine.  LS 5feb96
{
  int32_t       oldContext, n, getStringChar(void), getStreamChar(void), nsym, result;
  extern char   *inputString, compileOnly;
  extern int32_t        (*getChar)(void), executeLevel;
  char  compileName[12], oldInstalling;
  int32_t       newBlockSymbol(int32_t);

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
      || newSymbol(LUX_BLOCKROUTINE, -nsym - 1) < 0) { // some error
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
//----------------------------------------------------------------
#if DEVELOP
int32_t lux_compile(ArgumentCount narg, Symbol ps[])
{
  char  *string;
  int32_t       result, value;

  string = string_arg(*ps);
  result = scalar_scratch(LUX_INT32);
  if (string)
  { value = compile(string);
    scalar_value(result).i32 = value; }
  else
    scalar_value(result).i32 = -1;
  return result;
}
#endif
//----------------------------------------------------------------
int32_t newBlockSymbol(int32_t index)
/* searches for user block symbolStack[index] in list of user-defined
  block routines.  treats function pointers */
{
  int32_t       n, result;
  extern char   reportBody;

  if (reportBody) {             // remove name from stack
    freeString(index);
    return 0;
  }
  if ((n = lookForVar(index, curContext)) >= 0) { // blockroutine pointer?
    if (symbol_class(n) == LUX_FUNC_PTR) {
      if (func_ptr_routine_num(n) > 0) { // user-defined
        if (symbol_class(n = func_ptr_routine_num(n)) == LUX_BLOCKROUTINE) {
          freeString(index);
          return newSymbol(LUX_EVB, EVB_USR_CODE, n);
        }
      } else
        return luxerror("Func/subr pointer does not point at executable block routine!", n);
    }
  }
  n = lookForBlock(index);
  if (n < 0) {                  // block not yet defined
    n = nextFreeTempVariable();
    if (n < 0)
      return LUX_ERROR;
    symbol_class(n) = LUX_STRING;
    string_value(n) = (char*) symbolStack[index];
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
//----------------------------------------------------------------
int32_t newSubrSymbol(int32_t index)
/* searches for subroutine symbolStack[index] in lists of internal
  and user-defined subroutines.  if not found, then searches for an
  appropriate file to find a definition.  if such a file is found,
  then installs name as new subroutine in user-defined subroutine list,
  and a new symbol with appropriate class (LUX_EVB) and type (EVB_INT_SUB
  or EVB_USR_SUB) is returned.  if such a file is not found, then
  an error is generated. */
{
 int32_t        n, i;
 extern int32_t         findBody;

 // In order, look for:
 // 1. function pointer to some user-defined or internal subroutine
 // 2. user-defined subroutine
 // 3. internal subroutine
 // 4. user-defined function

 if (ignoreInput && findBody > 0) { // not compiling this
   freeString(index);           // remove name from stack
   // take care of deleting arguments:
   return newSymbol(LUX_EVB, EVB_INT_SUB, 0);
 }
 if (findBody < 0)              /* we're at the end of the definition
                                   of a deferred routine */
   findBody = -findBody;
 n = lookForVar(index, curContext); // look for variable
 if (n >= 0 && symbol_class(n) == LUX_FUNC_PTR) { // maybe subr pointer
   freeString(index);           // remove name from stacke
   if (func_ptr_routine_num(n) < 0) {   // internal routine/function
     if ((Symbolclass) func_ptr_type(n) == LUX_SUBROUTINE)
       return newSymbol(LUX_EVB, EVB_INT_SUB, -sym[n].spec.evb.args[0]);
   } else {
     n = func_ptr_routine_num(n); // user-defined routine
     return newSymbol(LUX_EVB, EVB_USR_SUB, n);
   }
 }
 // no subroutine pointer
 n = lookForSubr(index);        // already defined user-defined routine?
 if (n < 0) {                   // none found
   if ((n = findInternalSym(index, 1)) >= 0) { // internal routine
     freeString(index);
     return newSymbol(LUX_EVB, EVB_INT_SUB, n);
   } else {                     // no internal: assume user-defined
     n = newSymbol(LUX_FIXED_STRING, index);
     i = newSymbol(LUX_EVB, EVB_USR_SUB, n);
     symbol_context(n) = i;
     return i;
   }
 }
                                // user-defined routine
 freeString(index);
 return newSymbol(LUX_EVB, EVB_USR_SUB, n);
}
//----------------------------------------------------------------
int32_t lookForName(char const* name, HashTableEntry *hashTable[], int32_t context)
     /* searches name in hashTable[] for context.  if found,
        returns symbol number, otherwise returns -1 */
{
  int32_t               hashValue, n;
  HashTableEntry        *hp;

  hashValue = hash(name);
  if (*name == '$' || *name == '#' || *name == '!') context = 0;
  hp = hashTable[hashValue];
  while (hp)
  { if (!strcmp(hp->name, name) && sym[hp->symNum].context == context)
      return hp->symNum;
    hp = hp->next; }
  // to distinguish between functions and subroutines with the same
  // name (e.g. when creating a pointer to a function or subroutine),
  // function names may be specified with an _F extension.  check if
  // we're dealing with such a case here
  if (hashTable == funcHashTable)
  { n = strlen(name);
    if (n > 2 && name[n - 2] == '_' && name[n - 1] == 'F')
    {
      char *t = strdup(name);
      t[n - 2] = '\0';
      int32_t result = lookForName(t, hashTable, context);
      free(t);
      return result;
    }
  }
  return -1;
}
//----------------------------------------------------------------
int32_t findSym(int32_t index, HashTableEntry *hashTable[], int32_t context)
/* searches symbolStack[index] in hashTable[] for context.  if found,
   returns symbol number, otherwise installs the name in hashTable[]
   and sym[].  always removes the entry from the symbolStack. */
{
 char   *name;
 int32_t        n;
 extern char    ignoreSymbols;

 if (ignoreSymbols) {
   freeString(index);
   return 0;
 }
 name = (char*) symbolStack[index];
 n = findName(name, hashTable, context);
 freeString(index);
 return n;
}
//----------------------------------------------------------------
char const* symName(int32_t symNum, HashTableEntry *hashTable[])
// returns the name of the symbol, if any, or "[symNum]"
{
 static char    name[7];
 int32_t                hashValue;
 HashTableEntry         *hp;

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
//----------------------------------------------------------------
char const* symbolName(int32_t symbol)
// returns the name of the symbol.
{
  HashTableEntry        **hashTable;

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
//----------------------------------------------------------------
int32_t suppressEvalRoutine(int32_t index)
// returns evaluation suppression associated with internal routine
// symbolStack[index]
{
  int32_t       n;
  KeyList       *keys;

  n = findInternalSym(index, 1); // >= 0 -> internal subroutine
  if (n < 0) return 0;
  keys = (KeyList *) subroutine[n].keys;
  return keys->suppressEval;
}
//----------------------------------------------------------------
#define         IGNORE_SIG      1
#define ASK_SIG                 2
#define SIG_BREAK       3
void exception(int32_t sig)
// exception handler
{
 int32_t        c, saveHistory(void);
 extern int32_t         curSymbol, executeLevel, step, statementDepth;
 extern jmp_buf         jmpenv;
 void   cleanUp(int32_t, int32_t), Quit(int32_t);

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
           Quit(1);             // exit LUX completely
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
//----------------------------------------------------------------
char const* typeName(int32_t type)
// returns the name that goes with the data type
{
  static char const* typeNames[] = {
    "BYTE", "WORD", "LONG", "INT64", "FLOAT", "DOUBLE",
    "STRING", "STRING", "STRING", "CFLOAT", "CDOUBLE",
    "undefined", "unknown"
  };
  int32_t       index;

  if (type == LUX_UNDEFINED)
    index = 10;                         // undefined
  else if (type < 0 || type > LUX_CDOUBLE)
    index = sizeof(typeNames)/sizeof(*typeNames) - 1; // unknown
  else
    index = type;               // OK
  return typeNames[index];
}
//----------------------------------------------------------------
char const* className(int32_t class_id)
// returns the name of the class
{
  static struct classInfo {
    uint8_t number; char const* name;
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

 static char    classHashTable[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
  20, 21, 22, 23, 24, 25, 42, 41, 42, 42, 42, 42, 26, 27, 28, 29, 30, 31,
  42, 42, 32, 33, 34, 35, 36, 37, 42, 42, 38, 39, 40
 };

 int32_t        hash;

 if (class_id < 0)
   hash = 26;
 else {
   hash = class_id % 76;
   if (hash > 51)
     hash = 25;
   hash = classHashTable[hash];
   if (class_id != (int32_t) classes[hash].number)
     hash = 42;
 }
 return classes[hash].name;
}
//----------------------------------------------------------------
int32_t lux_classname(ArgumentCount narg, Symbol ps[])
     // returns name associated with class number
{
  int32_t       class_id, result;
  char  *name;

  class_id = int_arg(*ps);
  getFreeTempVariable(result);
  symbol_class(result) = LUX_STRING;
  sym_string_type(result) = LUX_TEMP_STRING;
  name = string_value(result) = strsave(className(class_id));
  symbol_memory(result) = strlen(name) + 1;
  return result;
}
//----------------------------------------------------------------
int32_t lux_typeName(ArgumentCount narg, Symbol ps[])
     // returns name associated with type number
{
  int32_t       type, result;
  char  *name;

  if ((type = int_arg(*ps)) < 0) return -1;
  getFreeTempVariable(result);
  symbol_class(result) = LUX_STRING;
  sym_string_type(result) = LUX_TEMP_STRING;
  name = string_value(result) = strsave(typeName(type));
  symbol_memory(result) = strlen(name) + 1;
  return result;
}
//----------------------------------------------------------------
char const* evbName(int32_t evbType)
     // returns the name of an EVB type
{
  static char const* evbTypeNames[] = {
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
//----------------------------------------------------------------
char const* filetypeName(int32_t filetype)
// returns the name associated with a file type
{
  static char const* filetypeNames[] = {
    "Unknown", "LUX fz", "IDL Save", "GIF", "LUX Astore", "JPEG", "TIFF",
    "FITS", "PPM (raw)", "PPM (ascii)", "XPM", "X11 bitmap", "BMP",
    "Sun raster", "Iris RGB", "Targa (24 bit)", "PM"
  };

  if (filetype < 0 || filetype >= sizeof(filetypeNames)/sizeof(char *))
    filetype = 0;
  return filetypeNames[filetype];
}
//----------------------------------------------------------------
int32_t lux_filetype_name(ArgumentCount narg, Symbol ps[])
{
  char const* name;
  int32_t       result;

  name = filetypeName(int_arg(ps[0]));
  result = string_scratch(strlen(name));
  strcpy(string_value(result), name);
  return result;
}
//----------------------------------------------------------------
void fixedValue(char const* name, Symboltype type, ...)
// install a numerical constant
{
 int32_t        n, iq;
 Pointer        p;
 va_list        ap;

 va_start(ap, type);
 iq = installString(name);
 n = findVar(iq, 0);
 switch (type) {
 case LUX_LSTRING:
   symbol_class(n) = LUX_STRING;
   sym_string_type(n) = LUX_LSTRING;
   string_value(n) = va_arg(ap, char *);
   symbol_memory(n) = strlen(string_value(n)) + 1;
   break;
 case LUX_CFLOAT: case LUX_CDOUBLE:
   complex_scalar_data(n).cf = (FloatComplex*) malloc(lux_type_size[type]);
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
   case LUX_INT32:
     scalar_value(n).i32 = va_arg(ap, int32_t);
     break;
   case LUX_INT64:
     scalar_value(n).i64 = va_arg(ap, int64_t);
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
//----------------------------------------------------------------
int32_t installSysFunc(char const* name, int32_t number)
// install a system function.  These are implemented as FUNC_PTRs to
// the appropriate function
{
 int32_t        n, iq;

 iq = installString(name);
 n = findVar(iq,0);
 symbol_class(n) = LUX_FUNC_PTR;
 func_ptr_routine_num(n) = -number;
 func_ptr_type(n) = (Symboltype) LUX_FUNCTION;
 return n;
}
//----------------------------------------------------------------
int32_t installPointer(char const* name, Symboltype type, void *ptr)
// install a LUX_SCAL_PTR system variable
{
 int32_t        n, iq;

 iq = installString(name);
 n = findVar(iq, 0);
 symbol_class(n) = LUX_SCAL_PTR;
 scal_ptr_type(n) = type;
 scal_ptr_pointer(n).i32 = (int32_t *) ptr;
 if (type == LUX_TEMP_STRING)
   symbol_memory(n) = strlen((char*) ptr) + 1;
 return n;
}
//----------------------------------------------------------------
int32_t convertRange(int32_t range)
// convert a LUX_RANGE symbol to a LUX_SUBSC_PTR symbol.
// elements:  #1:  range start
//            #2:  range end
//            #3:  summation flag (0 or 1)
//            #4:  redirection flag (symbol, MINUSONE if none)
//            for #1 and #2, if the symbol is > 0, then count from the
//            start of the list/array; otherwise from one beyond the last
//            element of the list/array.  if #2 == LUX_ZERO, then only one
//            element is requested.
{
  int32_t       subsc, eval(int32_t), j1, j2;

  if ((subsc = newSymbol(LUX_SUBSC_PTR)) < 0) return -1;
  j1 = range_start(range);
  if (j1 == -LUX_ONE)           // (*)
  { subsc_ptr_start(subsc) = 0;
    subsc_ptr_end(subsc) = -1; }
  else
  { if (j1 >= 0)                // (X:...)
    { j2 = int_arg(eval(j1));
      if (j2 < 0)
        return luxerror("Illegal range start", range_start(range)); }
    else                        // (*-X:...)
    { j2 = -int_arg(eval(-j1));
      if (-j2 <= 0)
        return luxerror("Illegal range start", range_start(range)); }
    subsc_ptr_start(subsc) = j2;

    j1 = range_end(range);
    if (j1 == LUX_ZERO)                 // (X)
      subsc_ptr_end(subsc) = subsc_ptr_start(subsc);
    else if (j1 == -LUX_ONE)    // (...:*)
      subsc_ptr_end(subsc) = -1;
    else
    { if (j1 >= 0)              // (...:Y)
      { j2 = int_arg(eval(j1));
        if (j2 < 0)
          return luxerror("Illegal range end", range_end(range)); }
      else                      // (...:*-Y)
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
//----------------------------------------------------------------
void convertPointer(Scalar *target, Symboltype inType, Symboltype outType)
// converts value in target from inType to outType
{
  switch (outType)
  {
  case LUX_INT8:
    switch (inType)
    {
    case LUX_INT8:
      break;
    case LUX_INT16:
      (*target).ui8 = (uint8_t) (*target).i16;
      break;
    case LUX_INT32:
      (*target).ui8 = (uint8_t) (*target).i32;
      break;
    case LUX_INT64:
      (*target).ui8 = (uint8_t) (*target).i64;
      break;
    case LUX_FLOAT:
      (*target).ui8 = (uint8_t) (*target).f;
      break;
    case LUX_DOUBLE:
      (*target).ui8 = (uint8_t) (*target).d;
      break;
#if 0
    case LUX_CFLOAT:
      (*target).ui8 = (uint8_t) (*target).cf.real;
      break;
    case LUX_CDOUBLE:
      (*target).ui8 = (uint8_t) (*target).cd.real;
      break;
#endif
    }
  case LUX_INT16:
    switch (inType)
    {
    case LUX_INT8:
      (*target).i16 = (int16_t) (*target).ui8;
      break;
    case LUX_INT16:
      break;
    case LUX_INT32:
      (*target).i16 = (int16_t) (*target).i32;
      break;
    case LUX_INT64:
      (*target).i16 = (int16_t) (*target).i64;
      break;
    case LUX_FLOAT:
      (*target).i16 = (int16_t) (*target).f;
      break;
    case LUX_DOUBLE:
      (*target).i16 = (int16_t) (*target).d;
      break;
#if 0
    case LUX_CFLOAT:
      (*target).i16 = (int16_t) (*target).cf.real;
      break;
    case LUX_CDOUBLE:
      (*target).i16 = (int16_t) (*target).cd.real;
      break;
#endif
    }
    break;
  case LUX_INT32:
    switch (inType)
    {
    case LUX_INT8:
      (*target).i32 = (int32_t) (*target).ui8;
      break;
    case LUX_INT16:
      (*target).i32 = (int32_t) (*target).i16;
      break;
    case LUX_INT32:
      break;
    case LUX_INT64:
      (*target).i32 = (int32_t) (*target).i64;
      break;
    case LUX_FLOAT:
      (*target).i32 = (int32_t) (*target).f;
      break;
    case LUX_DOUBLE:
      (*target).i32 = (int32_t) (*target).d;
      break;
#if 0
    case LUX_CFLOAT:
      (*target).i32 = (int32_t) (*target).cf.real;
      break;
    case LUX_CDOUBLE:
      (*target).i32 = (int32_t) (*target).cd.real;
      break;
#endif
    }
    break;
  case LUX_INT64:
    switch (inType)
    {
    case LUX_INT8:
      (*target).i64 = (int64_t) (*target).ui8;
      break;
    case LUX_INT16:
      (*target).i64 = (int64_t) (*target).i16;
      break;
    case LUX_INT32:
      (*target).i64 = (int64_t) (*target).i32;
      break;
    case LUX_INT64:
      break;
    case LUX_FLOAT:
      (*target).i64 = (int64_t) (*target).f;
      break;
    case LUX_DOUBLE:
      (*target).i64 = (int64_t) (*target).d;
      break;
#if 0
    case LUX_CFLOAT:
      (*target).i64 = (int64_t) (*target).cf.real;
      break;
    case LUX_CDOUBLE:
      (*target).i64 = (int64_t) (*target).cd.real;
      break;
#endif
    }
    break;
  case LUX_FLOAT:
    switch (inType)
    {
    case LUX_INT8:
      (*target).f = (float) (*target).ui8;
      break;
    case LUX_INT16:
      (*target).f = (float) (*target).i16;
      break;
    case LUX_INT32:
      (*target).f = (float) (*target).i32;
      break;
    case LUX_INT64:
      (*target).f = (float) (*target).i64;
      break;
    case LUX_FLOAT:
      break;
    case LUX_DOUBLE:
      (*target).f = (float) (*target).d;
      break;
#if 0
    case LUX_CFLOAT:
      (*target).f = (float) (*target).cf.real;
      break;
    case LUX_CDOUBLE:
      (*target).f = (float) (*target).cd.real;
      break;
#endif
    }
    break;
  case LUX_DOUBLE:
    switch (inType)
    {
    case LUX_INT8:
      (*target).d = (double) (*target).ui8;
      break;
    case LUX_INT16:
      (*target).d = (double) (*target).i16;
      break;
    case LUX_INT32:
      (*target).d = (double) (*target).i32;
      break;
    case LUX_INT64:
      (*target).d = (double) (*target).i64;
      break;
    case LUX_FLOAT:
      (*target).d = (double) (*target).f;
      break;
    case LUX_DOUBLE:
      break;
#if 0
    case LUX_CFLOAT:
      (*target).d = (double) (*target).cf.real;
      break;
    case LUX_CDOUBLE:
      (*target).d = (*target).cd.real;
      break;
#endif
    }
    break;
  }
#if 0
  case LUX_CFLOAT:
    switch (inType)
    {
    case LUX_INT8:
      (*target).cf.real = (float) (*target).ui8;
      (*target).cf.imaginary = 0;
      break;
    case LUX_INT16:
      (*target).cf.real = (float) (*target).i16;
      (*target).cf.imaginary = 0;
      break;
    case LUX_INT32:
      (*target).cf.real = (float) (*target).i32;
      (*target).cf.imaginary = 0;
      break;
    case LUX_INT64:
      (*target).cf.real = (float) (*target).i64;
      (*target).cf.imaginary = 0;
      break;
    case LUX_DOUBLE:
      (*target).cf.real = (float) (*target).d;
      (*target).cf.imaginary = 0;
      break;
    case LUX_CFLOAT:
      break;
    case LUX_CDOUBLE:
      (*target).cf.real = (float) (*target).cd.real;
      (*target).cf.imaginary = 0;
      break;
    }
    break;
  case LUX_CDOUBLE:
    switch (inType)
    {
    case LUX_INT8:
      (*target).cd.real = (double) (*target).ui8;
      (*target).cd.imaginary = 0;
      break;
    case LUX_INT16:
      (*target).cd.real = (double) (*target).i16;
      (*target).cd.imaginary = 0;
      break;
    case LUX_INT32:
      (*target).cd.real = (double) (*target).i32;
      (*target).cd.imaginary = 0;
      break;
    case LUX_INT64:
      (*target).cd.real = (double) (*target).i64;
      (*target).cd.imaginary = 0;
      break;
    case LUX_DOUBLE:
      (*target).cd.real = (double) (*target).d;
      (*target).cd.imaginary = 0;
      break;
    case LUX_CFLOAT:
      (*target).cd.real = (double) (*target).cf.real;
      (*target).cd.imaginary = (double) (*target).cf.imaginary;
      break;
    case LUX_CDOUBLE:
      break;
    }
    break;
#endif
}
//----------------------------------------------------------------
void convertScalar(Scalar *target, int32_t nsym, Symboltype type)
// returns scalar value of nsym, converted to proper type, in target
{
 int32_t                n;
 Pointer        ptr;

 n = scalar_type(nsym);
 ptr.ui8 = &scalar_value(nsym).ui8;
 switch (type) {
 case LUX_INT8:
   switch (n) {
   case LUX_INT8:
     (*target).ui8 = (uint8_t) *ptr.ui8;
     break;
   case LUX_INT16:
     (*target).ui8 = (uint8_t) *ptr.i16;
     break;
   case LUX_INT32:
     (*target).ui8 = (uint8_t) *ptr.i32;
     break;
   case LUX_INT64:
     (*target).ui8 = (uint8_t) *ptr.i64;
     break;
   case LUX_FLOAT:
     (*target).ui8 = (uint8_t) *ptr.f;
     break;
   case LUX_DOUBLE:
     (*target).ui8 = (uint8_t) *ptr.d;
     break;
   }
   break;
 case LUX_INT16:
   switch (n) {
   case LUX_INT8:
     (*target).i16 = (int16_t) *ptr.ui8;
     break;
   case LUX_INT16:
     (*target).i16 = (int16_t) *ptr.i16;
     break;
   case LUX_INT32:
     (*target).i16 = (int16_t) *ptr.i32;
     break;
   case LUX_INT64:
     (*target).i16 = (int16_t) *ptr.i64;
     break;
   case LUX_FLOAT:
     (*target).i16 = (int16_t) *ptr.f;
     break;
   case LUX_DOUBLE:
     (*target).i16 = (int16_t) *ptr.d;
     break;
   }
   break;
 case LUX_INT32:
   switch (n) {
   case LUX_INT8:
     (*target).i32 = (int32_t) *ptr.ui8;
     break;
   case LUX_INT16:
     (*target).i32 = (int32_t) *ptr.i16;
     break;
   case LUX_INT32:
     (*target).i32 = (int32_t) *ptr.i32;
     break;
   case LUX_INT64:
     (*target).i32 = (int32_t) *ptr.i64;
     break;
   case LUX_FLOAT:
     (*target).i32 = (int32_t) *ptr.f;
     break;
   case LUX_DOUBLE:
     (*target).i32 = (int32_t) *ptr.d;
     break;
   }
   break;
 case LUX_INT64:
   switch (n) {
   case LUX_INT8:
     (*target).i64 = (int64_t) *ptr.ui8;
     break;
   case LUX_INT16:
     (*target).i64 = (int64_t) *ptr.i16;
     break;
   case LUX_INT32:
     (*target).i64 = (int64_t) *ptr.i32;
     break;
   case LUX_INT64:
     (*target).i64 = (int64_t) *ptr.i64;
     break;
   case LUX_FLOAT:
     (*target).i64 = (int64_t) *ptr.f;
     break;
   case LUX_DOUBLE:
     (*target).i64 = (int64_t) *ptr.d;
     break;
   }
   break;
 case LUX_FLOAT:
   switch (n) {
   case LUX_INT8:
     (*target).f = (float) *ptr.ui8;
     break;
   case LUX_INT16:
     (*target).f = (float) *ptr.i16;
     break;
   case LUX_INT32:
     (*target).f = (float) *ptr.i32;
     break;
   case LUX_INT64:
     (*target).f = (float) *ptr.i64;
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
   case LUX_INT8:
     (*target).d = (double) *ptr.ui8;
     break;
   case LUX_INT16:
     (*target).d = (double) *ptr.i16;
     break;
   case LUX_INT32:
     (*target).d = (double) *ptr.i32;
     break;
   case LUX_INT64:
     (*target).d = (double) *ptr.i64;
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
//----------------------------------------------------------------
int32_t lux_symbol_memory(ArgumentCount narg, Symbol ps[])
// returns the total of the memory allocated for each LUX symbol
// - which is NOT the same as the total allocated memory.
// Note:  some small stuff is not included.
{
 int32_t        i, mem = 0;

 for (i = 0; i < NSYM; i++)
 { switch (symbol_class(i))
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
 i = scalar_scratch(LUX_INT32);
 scalar_value(i).i32 = mem;
 return i;
}
//----------------------------------------------------------------
int32_t lux_trace(ArgumentCount narg, Symbol ps[])
// activates/deactivates trace facility
{
  extern float  CPUtime;

  if (narg > 0)
    trace = int_arg(*ps);
  else
    trace = 999;
  traceMode = internalMode;
  if (!traceMode)
    traceMode = trace? 143: 0;  // 128 + 8 + 4 + 2 + 1
                                // /ENTER, /ROUTINE, /BRACES, /LOOP, /FILE
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
//----------------------------------------------------------------
#define b_fix(name, value) { fixedValue(name, LUX_INT8, value);  nFixed++; }
#define l_fix(name, value) { fixedValue(name, LUX_INT32, value);  nFixed++; }
#define q_fix(name, value) { fixedValue(name, LUX_INT64, value);  nFixed++; }
#define f_fix(name, value) { fixedValue(name, LUX_FLOAT, value);  nFixed++; }
#define d_fix(name, value) { fixedValue(name, LUX_DOUBLE, value);  nFixed++; }
#define s_fix(name, value) { fixedValue(name, LUX_LSTRING, value);  nFixed++; }
#define cf_fix(name, re, im) { fixedValue(name, LUX_CFLOAT, re, im); nFixed++; }
#define cd_fix(name, re, im) { fixedValue(name, LUX_DFLOAT, re, im); nFixed++; }
#define l_ptr(name, value) installPointer(name, LUX_INT32, value)
#define q_ptr(name, value) installPointer(name, LUX_INT64, value)
#define f_ptr(name, value) installPointer(name, LUX_FLOAT, value)
#define d_ptr(name, value) installPointer(name, LUX_DOUBLE, value)
#define s_ptr(name, value) installPointer(name, LUX_TEMP_STRING, value)
#define fnc_p(name, value) installSysFunc(name, value)

#if HAVE_LIBGSL
/// Should error messages from the Gnu Scientific Library be displayed?  This
/// global variable is accessable within Lux through `!gsl_show_errors`.
int32_t gsl_show_errors = 0;
#endif


char const* defaultRedirect = "diagnostic.lux";

int32_t range_warn_flag = 0, redim_warn_flag = 0, error_extra = 0, histmin,
  histmax, lastmaxloc, lastminloc, scalemin = 0, scalemax = 255, fftdp = 0,
  lastmax_sym, lastmin_sym, setup = 0, tempSym, iformat, fformat, sformat,
  cformat, tformat, projectSym, projectTk = 0, stackSym, MSBfirst, area_diag =
  1, lastmean_sym, lastsdev_sym, r_d_sym, d_r_sym;

Scalar  lastmin, lastmax, lastmean, lastsdev;
extern int32_t ndx, ndxs, nd, ndys, maxregridsize, nExecuted, kb, nArg, tvsmt,
  badmatch, sort_flag, crunch_bits, crunch_slice, byte_count,
  index_cnt, uTermCol, page;
extern double   meritc;
extern float plims[], stepx, stepy, slabx, slaby, crunch_bpp;
extern int16_t  *stackPointer;

#if DEVELOP
extern int32_t  irzf, ifzz, ndz, ndzs, resample_type, fstepz;
extern float    wzb, wzt, ticz, ticzr, zmin, zmax, defaultProjection[], dvz;
#endif

char    *firstbreak;            // for memck.c

EnumElem classesStruct[]
= {
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

EnumElem typesStruct[]
= {
  { "byte", LUX_INT8 },
  { "word", LUX_INT16 },
  { "long", LUX_INT32 },
  { "int64", LUX_INT64 },
  { "float", LUX_FLOAT },
  { "double", LUX_DOUBLE },
  { "string", LUX_TEMP_STRING },
  { "cfloat", LUX_CFLOAT },
  { "cdouble", LUX_CDOUBLE },
  { "undefined", LUX_UNDEFINED }
};

EnumElem filetypeStruct[]
= {
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

struct BoundsStruct bounds = {
  { 0, INT16_MIN, INT32_MIN, INT64_MIN, -FLT_MAX, -DBL_MAX},
  { UINT8_MAX, INT16_MAX, INT32_MAX, INT64_MAX, FLT_MAX, DBL_MAX }
};

int32_t         LUX_MATMUL_FUN;

#define         FORMATSIZE      1024
void symbolInitialization(void)
{
 int32_t        i, iq;
#if YYDEBUG
 extern int32_t         yydebug;
#endif
 extern int32_t         termRow, termCol, despike_count;
#if DEVELOP
 char   *p;
#endif
 extern char    *fmt_integer, *fmt_float, *fmt_string, *fmt_complex,
  *curScrat, *printString;
 union { uint8_t b[2]; int16_t w; } whichendian;

 // determine if the machine is little-endian or bigendian
 whichendian.w = 1;
 MSBfirst = (int32_t) whichendian.b[1]; // most significant Byte first?

 firstbreak = (char*) sbrk(0);          // for memck.c
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
 subroutine = (InternalRoutine*) malloc(registered_subroutines_size
                     + nSubroutine*sizeof(InternalRoutine));
 if (registered_subroutines)
   memcpy(subroutine,
          (InternalRoutine*) obstack_finish(registered_subroutines),
          registered_subroutines_size);
 memcpy((char *) subroutine + registered_subroutines_size,
        subroutine_table, nSubroutine*sizeof(InternalRoutine));
 nSubroutine += registered_subroutines_size/sizeof(InternalRoutine);
 if (registered_subroutines)
   obstack_free(registered_subroutines, NULL);

 extern struct obstack *registered_functions;
 int32_t registered_functions_size
   = registered_functions? obstack_object_size(registered_functions): 0;
 function = (InternalRoutine*) malloc(registered_functions_size
                     + nFunction*sizeof(InternalRoutine));
 if (registered_functions)
   memcpy(function,
          (InternalRoutine*) obstack_finish(registered_functions),
          registered_functions_size);
 memcpy((char *) function + registered_functions_size,
        function_table, nFunction*sizeof(InternalRoutine));
 nFunction += registered_functions_size/sizeof(InternalRoutine);
 if (registered_functions)
   obstack_free(registered_functions, NULL);

 qsort(subroutine, nSubroutine, sizeof(InternalRoutine), ircmp);
 qsort(function, nFunction, sizeof(InternalRoutine), ircmp);
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
 LUX_MATMUL_FUN = findInternalName("mproduct", 0);
 inputStream = stdin;
 outputStream = stdout;
 l_fix("#zero",         0);
 l_fix("#1",            1);
 l_fix("#minus1",       -1);
 l_fix("#42",           42);
 l_fix("#0",            0);
 f_fix("#infty",        INFTY);
 f_fix("#nan",          acos(2));
 cf_fix("#i",           0.0, 1.0); // imaginary unit
 l_fix("#max_args",     MAX_ARG);
 l_fix("#max_byte",     bounds.max.ui8);
 l_fix("#max_word",     bounds.max.i16);
 l_fix("#max_long",     bounds.max.i32);
 q_fix("#max_int64",    bounds.max.i64);
 f_fix("#max_float",    bounds.max.f);
 d_fix("#max_double",   bounds.max.d);
 l_fix("#min_word",     bounds.min.i16);
 l_fix("#min_long",     bounds.min.i32);
 q_fix("#min_int64",    bounds.min.i64);
 f_fix("#min_float",    FLT_MIN);
 d_fix("#min_double",   DBL_MIN);
 l_fix("#max_dims",     MAX_DIMS);
 f_fix("#eps_float",    FLT_EPSILON);
 d_fix("#eps_double",   DBL_EPSILON);
 d_fix("#pi",           M_PI);
 d_fix("#2pi",          2*M_PI);
 d_fix("#e",            M_E);
 d_fix("#c",            299792458e0);
 d_fix("#G",            6.668E-8);
 d_fix("#h",            6.6252E-27);
 d_fix("#hb",           1.0544E-27);
 d_fix("#ec",           6.6252E-27);
 d_fix("#m",            9.1084E-28);
 d_fix("#k",            1.308046E-16);
 d_fix("#R",            8.317E7);
 d_fix("#rad",          RAD);
 r_d_sym = nFixed;
 d_fix("#r.d",          RAD);
 d_fix("#deg",          DEG);
 d_r_sym = nFixed;
 d_fix("#d.r",          DEG);

 iq = installString("#class");
 stackSym = findVar(iq, 0);     // stackSym is a dummy variable
 symbol_class(stackSym) = LUX_ENUM;
 enum_type(stackSym) = LUX_INT32;
 enum_list(stackSym) = classesStruct;
 symbol_memory(stackSym) = sizeof(classesStruct);
 nFixed++;

 iq = installString("#filetype");
 stackSym = findVar(iq, 0);
 symbol_class(stackSym) = LUX_ENUM;
 enum_type(stackSym) = LUX_INT32;
 enum_list(stackSym) = filetypeStruct;
 symbol_memory(stackSym) = sizeof(filetypeStruct);
 nFixed++;

 iq = installString("#type");
 stackSym = findVar(iq, 0);
 symbol_class(stackSym) = LUX_ENUM;
 enum_type(stackSym) = LUX_INT32;
 enum_list(stackSym) = typesStruct;
 symbol_memory(stackSym) = sizeof(typesStruct);
 nFixed++;

 iq = installString("#stack");
 stackSym = findVar(iq, 0);
 symbol_class(stackSym) = LUX_CLIST;
 clist_symbols(stackSym) = stackPointer;
 symbol_memory(stackSym) = 0;   // or it will get deallocated sometime
 nFixed++;

 iq = findVarName("#typesize", 0);
 i = LUX_NO_SYMBOLTYPE; // lux_type_size[] # elements!
 to_scratch_array(iq, LUX_INT32, 1, &i);
 memcpy((int32_t*) array_data(iq), lux_type_size, i*sizeof(int32_t));

 // s_fix("#nl",                "\n");
 l_ptr("#col",          &termCol);
 l_ptr("#row",          &termRow);

 l_ptr("#msbfirst",     &MSBfirst);
 s_ptr("#version",      (char*) VERSION);

 l_ptr("!area_diag",    &area_diag);
 l_ptr("!badmatch",     &badmatch);
 l_ptr("!bc",           &byte_count);
 fnc_p("!cjd",          12);
 l_ptr("!col",          &uTermCol);
 fnc_p("!cputime",      2);
 l_ptr("!crunch_bits",  &crunch_bits);
 f_ptr("!crunch_bpp",   &crunch_bpp);
 l_ptr("!crunch_slice", &crunch_slice);
 fnc_p("!ctime",        5);
 fnc_p("!date",                 7);
 l_ptr("!despike_count", &despike_count);
 l_ptr("!error_extra",  &error_extra);
 l_ptr("!errno",        &errno);
 l_ptr("!fftdp",        &fftdp);
 fformat = s_ptr("!format_f", strsave("%14.7g"));
 iformat = s_ptr("!format_i", strsave("%10jd"));
 sformat = s_ptr("!format_s", strsave("%s"));
 cformat = s_ptr("!format_c", strsave("%28.7z"));
 fmt_integer = string_value(iformat);
 fmt_float = string_value(fformat);
 fmt_string = string_value(sformat);
 fmt_complex = string_value(cformat);
#if HAVE_LIBGSL
 l_ptr("!gsl_show_errors", &gsl_show_errors);
#endif
 l_ptr("!histmin",      &histmin);
 l_ptr("!histmax",      &histmax);
 fnc_p("!jd",           11);
 l_ptr("!lastmax",      &lastmax);
 l_ptr("!lastmaxloc",   &lastmaxloc);
 l_ptr("!lastmean",     &lastmean);
 l_ptr("!lastmin",      &lastmin);
 l_ptr("!lastminloc",   &lastminloc);
 l_ptr("!lastsdev",     &lastsdev);
 l_ptr("!maxregridsize", &maxregridsize);
 d_ptr("!meritc",       &meritc);
 l_ptr("!narg",                 &nArg);
 l_ptr("!nexecuted",    &nExecuted);
 l_ptr("!range_warn_flag",      &range_warn_flag);
 l_ptr("!read_count",   &index_cnt);
 fnc_p("!readkey",      8);
 fnc_p("!readkeyne",    9);
 l_ptr("!redim_warn_flag",      &redim_warn_flag);
#if DEVELOP
 l_ptr("!regrid_type",  &resample_type);
#endif
 l_ptr("!row",          &page);
 l_ptr("!scalemax",     &scalemax);
 l_ptr("!scalemin",     &scalemin);
 l_ptr("!sort_flag",    &sort_flag);
 fnc_p("!systime",      10);
 fnc_p("!time",                 6);
 l_ptr("!tvsmt",        &tvsmt);
 l_ptr("!project",      &projectTk);
 f_ptr("!xb",           &plims[0]);
 f_ptr("!xt",           &plims[1]);
 f_ptr("!yb",           &plims[2]);
 f_ptr("!yt",           &plims[3]);
#if YYDEBUG
 l_ptr("!yydebug",      &yydebug);
#endif
 f_ptr("!zb",           &plims[4]);
 f_ptr("!zt",           &plims[5]);

        /* create a "universal" variable to assign to
           in  EVAL(string) */
 iq = installString("!temp");
 tempSym = findVar(iq, 0);
 symbol_class(tempSym) = LUX_SCALAR;
 scalar_type(tempSym) = LUX_INT32;
 scalar_value(tempSym).i32 = 0;
 lastmax_sym = lookForVarName("!lastmax", 0);
 lastmin_sym = lookForVarName("!lastmin", 0);
 lastsdev_sym = lookForVarName("!lastsdev", 0);
 lastmean_sym = lookForVarName("!lastmean", 0);
 eval_func = findInternalName("eval", 0);
 insert_subr = findInternalName("%insert", 1);
 printString = curScrat + FORMATSIZE;
 installing = 0;
}
//----------------------------------------------------------------
int32_t matchInternalName(char *name, InternalRoutine *table, int32_t size, int32_t hi)
/* matches name against the initial parts of all names in the table.
   returns index of first match (i.e., closest to the start of the table),
   or -1 if none were found.  LS97 */
{
 int32_t        lo = 0, mid, s;

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
//----------------------------------------------------------------
void zerobytes(void *sp, int32_t len)
// zeros <len> bytes starting at <sp>
{
  char  *p;

  p = (char *) sp;
  while (len--) *p++ = '\0';
}
//----------------------------------------------------------------
int32_t strncasecmp_p(char *s1, char *s2, int32_t n)
// compares the first <n> bytes of strings <s1> and <s2> and returns 0
// if they are equal in both strings, a number > 0 if <s2> is later
// that <s1> in the internal character set, or < 0 otherwise.
// LS 17feb97
{
  char  c1, c2;
  int32_t       i = 0;

  do
  { c1 = toupper(*s1++);
    c2 = toupper(*s2++);
    i++; }
  while (c1 == c2 && c1 != '\0' && c2 != '\0' && i < n);
  return c2 - c1;
}
//----------------------------------------------------------------
int32_t strcasecmp_p(char *s1, char *s2)
// compares strings <s1> and <s2> without regard to case and returns 0
// if they are equal, a number > 0 if <s2> is later
// that <s1> in the internal character set, or < 0 otherwise.
// LS 21feb97
{
  int32_t       c1, c2;

  do
  { c1 = toupper(*s1++);
    c2 = toupper(*s2++); }
  while (c1 == c2 && c1 != '\0' && c2 != '\0');
  return c2 - c1;
}
//----------------------------------------------------------------
int32_t         nBreakpoint = 0;
Breakpoint  breakpoint[NBREAKPOINTS];
int32_t lux_breakpoint(ArgumentCount narg, Symbol ps[])
// BREAKPOINT,string[,/SET,/VARIABLE]
// BREAKPOINT,n[,/DISABLE,/ENABLE,/DELETE]
// BREAKPOINT,/LIST
// /LIST can be specified together with one of the other switches
{
  static int32_t        curBreakpoint = 0;
  char  *s, *p;
  int32_t       n;

  if (narg)
    switch (internalMode & 3) {
      case 0:
        switch (symbol_class(ps[0])) {
          case LUX_STRING:      // /SET
            s = string_arg(ps[0]);
            if (!s)             // empty string
              return -1;
            if (nBreakpoint == NBREAKPOINTS) {
              printf("Maximum number of breakpoints (%1d) has been reached\n",
                     NBREAKPOINTS);
              return luxerror("New breakpoint has been rejected.", ps[0]);
            }
            // seek an empty breakpoint slot
            while (breakpoint[curBreakpoint].status & BP_DEFINED)
              if (++curBreakpoint == NBREAKPOINTS)
                curBreakpoint = 0;
            p = strtok(s, ":");
            breakpoint[curBreakpoint].name = strsave(p);
            p = strtok(NULL, ":");
            if (!p)                     // no number
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
            }
            nBreakpoint++;
            break;
          default:
            return cerror(ILL_CLASS, ps[0]);
          case LUX_SCALAR:              // /ENABLE
            n = int_arg(ps[0]);

            if (n < 0 || n >= NBREAKPOINTS)
              return luxerror("Illegal breakpoint number", ps[0]);
            if (!(breakpoint[n].status & 1))
              return luxerror("Non-existent breakpoint", ps[0]);
            breakpoint[n].status |= BP_ENABLED; // enable
            break;
        }
        break;
      case 1:                   // /ENABLE
        n = int_arg(ps[0]);
        if (n < 0 || n >= NBREAKPOINTS)
          return luxerror("Illegal breakpoint number", ps[0]);
        if (!(breakpoint[n].status & BP_DEFINED))
          return luxerror("Non-existent breakpoint", ps[0]);
        breakpoint[n].status |= BP_ENABLED; // enable
        break;
      case 2:                   // /DISABLE
        n = int_arg(ps[0]);
        if (n < 0 || n >= NBREAKPOINTS)
          return luxerror("Illegal breakpoint number", ps[0]);
        if (!(breakpoint[n].status & BP_DEFINED))
          return luxerror("Non-existent breakpoint", ps[0]);
        breakpoint[n].status &= ~BP_ENABLED; // disable
        break;
      case 3:                   // /DELETE
        n = int_arg(ps[0]);
        if (n < 0 || n >= NBREAKPOINTS)
          return luxerror("Illegal breakpoint number", ps[0]);
        if (!(breakpoint[n].status & BP_DEFINED))
          return luxerror("Non-existent breakpoint", ps[0]);
        free(breakpoint[n].name);
        breakpoint[n].status = 0; // delete
        nBreakpoint--;
        break;
    }
  if (!narg || internalMode & 4) { // /LIST
    if (nBreakpoint) {
      printf("Breakpoints:\n%2s: %4s %20s %4s %s\n", "nr", "type", "name",
             "line", "status");
      for (n = 0; n < NBREAKPOINTS; n++)
        if (breakpoint[n].status & BP_DEFINED) { // exists
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
//----------------------------------------------------------------
int16_t         watchVars[NWATCHVARS];
int32_t         nWatchVars = 0;
int32_t lux_watch(ArgumentCount narg, Symbol ps[])
// WATCH,<variable>[,/DELETE,/LIST]
{
  static int32_t        curWatchVar = 0;
  int32_t       i;

  if (narg) {
    if (!symbolIsNamed(ps[0]))
      return luxerror("Need a named variable", ps[0]);
    if (internalMode & 1) {     // /DELETE
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
    } else {                    // install
      if (nWatchVars == NWATCHVARS - 1)
        return luxerror("Maximum number of watched variables is already reached",
                     ps[0]);
      while (watchVars[curWatchVar])
        curWatchVar++;
      watchVars[curWatchVar] = ps[0];
      nWatchVars++;
    }
  }
  if (internalMode & 2) {       // /LIST
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
//----------------------------------------------------------------
int32_t lux_symbol_number(ArgumentCount narg, Symbol ps[])
     // returns the symbol number of the argument
{
  int32_t       result;

  result = scalar_scratch(LUX_INT32);
  sym[result].spec.scalar.i32 = *ps;
  return result;
}
//----------------------------------------------------------------
void mark(int32_t symbol)
{
  if (markIndex == MSSIZE - 1)
  { luxerror("mark: WARNING - Too many temps marked", symbol);
    return; }
  markStack[markIndex++] = symbol;
}
//----------------------------------------------------------------
void pegMark(void)
{
  if (markIndex == MSSIZE - 1)
  { luxerror("pegMark: WARNING - Too many temps marked", -1);
    return; }
  markStack[markIndex++] = -1;
}
//----------------------------------------------------------------
void pegParse(void)
{
  if (markIndex == MSSIZE - 1)
  { luxerror("pegParse: WARNING - Too many temps marked", -1);
    return; }
  markStack[markIndex++] = -2;
}
//----------------------------------------------------------------
void zapParseTemps(void)
{
  int32_t       iq;

  while (markIndex > 0 && (iq = markStack[--markIndex]) >= 0)
    zapTemp(iq);
  markIndex++;                  // retain -2 on mark stack
  if (iq != -2)
  { luxerror("zapParseTemps: WARNING - Not at parse level", -1);
    return; }
}
//----------------------------------------------------------------
void removeParseMarker(void)
{
  if (markIndex < 1 || markStack[markIndex - 1] != -2)
  { luxerror("removeParseMarker: WARNING - Not at parse level", -1);
    return; }
  markIndex--;
}
//----------------------------------------------------------------
void unMark(int32_t symbol)
{
  int32_t       i;

  i = markIndex;
  while (i--)
  { if (markStack[i] == symbol)
    { markStack[i] = 0;
      return; }
  }
}
//----------------------------------------------------------------
void zapMarked(void)
{
  int32_t       iq;

  while (markIndex > 0 && (iq = markStack[--markIndex]) >= 0)
    zapTemp(iq);
}
//----------------------------------------------------------------
void checkTemps(void)
// for debugging: checks that the number of temporary (unnamed)
// variables is equal to what is expected.
{
  int32_t       i, n;
  extern int32_t        nTempVariable;

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
//----------------------------------------------------------------
#include <unistd.h>
int32_t lux_restart(ArgumentCount narg, Symbol ps[])
{
  extern char   *programName;
  int32_t       saveHistory(void);

  printf("\nRestarting LUX...\n\n");
  saveHistory();
  if (execl(programName, programName, NULL)) {
    return luxerror("Restarting LUX (%s) failed; reason: %s\n", 0,
                    programName, strerror(errno));
  }
  return 1;
}
//----------------------------------------------------------------
int32_t strccmp(char const* s1, char const* s2)
// checks strings s1 and s2 for equality disregarding upper/lower case
// distinctions
{
  while (*s1 && toupper(*s1++) == toupper(*s2++)) ;
  return *s1 - *s2;
}
//----------------------------------------------------------------
int32_t structSize(int32_t symbol, int32_t *nstruct, int32_t *nbyte)
/* returns in <*nstruct> the number of structure descriptors that are
   required to describe <symbol>, and in <*nbyte> the total number of bytes
   covered by the data in <symbol>. */
{
  int32_t       n, ns, nb;
  Pointer       p;
  ListElem* l;

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
      p.i16 = clist_symbols(symbol);
      n = clist_num_symbols(symbol);
      *nbyte = 0;
      *nstruct = 1;             // one extra for the struct info
      while (n--) {
        if (structSize(*p.i16++, &ns, &nb) == LUX_ERROR)
          return LUX_ERROR;
        *nbyte += nb;
        *nstruct += ns;
      }
      return 1;
    case LUX_LIST:
      l = list_symbols(symbol);
      n = list_num_symbols(symbol);
      *nbyte = 0;
      *nstruct = 1;             // one extra for the struct info
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
//----------------------------------------------------------------
int32_t makeStruct(int32_t symbol, char const* tag, StructElem** se,
                   char *data, int32_t *offset, int32_t descend)
{
  int32_t       size, offset0, ndim, n;
  StructElem* se0;
  int16_t       *arg;
  ListElem* le;

  if (descend) {
    return LUX_OK;
  } else {
    (*se)->u.regular.tag = tag? strsave(tag): NULL;
    (*se)->u.regular.offset = *offset; // Byte offset from start
    switch (symbol_class(symbol)) {
      case LUX_SCALAR: case LUX_CSCALAR:
        (*se)->u.regular.type = scalar_type(symbol); // data type
        (*se)->u.regular.spec.singular.ndim = 0; // 0 -> scalar
        // copy the value into the structure
        size = lux_type_size[scalar_type(symbol)];// bytes per value
        memcpy(data + *offset, &scalar_value(symbol).ui8, size);
        break;
      case LUX_STRING:
        (*se)->u.regular.type = LUX_TEMP_STRING; // data type
        (*se)->u.regular.spec.singular.ndim = 1; // strings always have 1
        if (!((*se)->u.regular.spec.singular.dims = (int32_t*) malloc(sizeof(int32_t))))
          return cerror(ALLOC_ERR, 0);
        size = string_size(symbol); // bytes per value
        (*se)->u.regular.spec.singular.dims[0] = size; // first dimension
        memcpy(data + *offset, string_value(symbol), size); // copy value
        break;
      case LUX_ARRAY: case LUX_CARRAY:
        (*se)->u.regular.type = array_type(symbol);
        ndim = array_num_dims(symbol);
        if (array_type(symbol) == LUX_STRING_ARRAY)
          ndim++;               // add one for string arrays to hold the
                                // length of the strings
        (*se)->u.regular.spec.singular.ndim = ndim;
        if (!((*se)->u.regular.spec.singular.dims = (int32_t*) malloc(ndim*sizeof(int32_t))))
          return cerror(ALLOC_ERR, 0);
        if (array_type(symbol) == LUX_STRING_ARRAY) {
          (*se)->u.regular.spec.singular.dims[0] =
            strlen(*(char **) array_data(symbol)); // take length of first
                                                   // one for all
          memcpy((*se)->u.regular.spec.singular.dims + 1,
                 array_dims(symbol), array_num_dims(symbol)*sizeof(int32_t));
        } else
          memcpy((*se)->u.regular.spec.singular.dims,
                 array_dims(symbol), array_num_dims(symbol)*sizeof(int32_t));
        size = lux_type_size[array_type(symbol)]*array_size(symbol);
        memcpy(data + *offset, (char*) array_data(symbol), size); // copy values
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
        // CONTINUE HERE
        break;
    }
    *offset += size;
    (*se)++;
  }
  return LUX_OK;
}
//----------------------------------------------------------------
int32_t lux_struct(ArgumentCount narg, Symbol ps[])
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
  int32_t       result, size, nstruct, dims[MAX_DIMS], ndim, n, i, offset;
  Pointer       data;
  StructElem* se;

  if (structSize(ps[0], &nstruct, &size) == LUX_ERROR) /* check
                                                          specification */
    return LUX_ERROR;
  nstruct++;                    // one extra for the top-level description
  ndim = narg - 1;
  if (get_dims(&ndim, ps + 1, dims) == LUX_ERROR) // read dimensions
    return LUX_ERROR;
  // calculate the number of repetitions of the outer structure
  n = 1;
  for (i = 0; i < ndim; i++)
    n *= dims[i];

  result = nextFreeTempVariable();
  if (result == LUX_ERROR)
    return LUX_ERROR;
  symbol_class(result) = LUX_STRUCT;
  symbol_memory(result) = sizeof(int32_t) // to store the number of elements
    + nstruct*sizeof(StructElem) // to store the structure information
    + size*n;                   // to store the data values
  data.v = malloc(symbol_memory(result));
  if (!data.v)
    return cerror(ALLOC_ERR, 0);
  *data.i32 = nstruct;
  symbol_data(result) = data.v;
  se = struct_elements(result);
  data.v = struct_data(result);

  // we must fill in the first descriptor
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
  if (!(se->u.first.dims = (int32_t*) malloc(ndim*sizeof(int32_t))))
    return cerror(ALLOC_ERR, 0);
  memcpy(se->u.first.dims, dims, ndim*sizeof(int32_t));
  se++;                                 // point at the next one
  offset = 0;

  // now recursively fill in the deeper ones
  if (makeStruct(ps[0], NULL, &se, (char*) data.v, &offset, 0) == LUX_ERROR)
    return LUX_ERROR;
  return result;
}
//----------------------------------------------------------------
int32_t translateEscapes(char *p)
// replace explicit escape sequences \x by internal ones; returns
// the final length of the string
{
  char  escapechars[] = "ntvbrfa\\?'\"", escapes[] = "\n\t\v\b\r\f\a\\?'\"",
    *p2, *p0;
  int32_t       i, c;

  p0 = p;
  while (*p) {
    if (*p == '\\') {           // an escape sequence
      p2 = strchr(escapechars, p[1]);
      if (p2) {
        *p = escapes[p2 - escapechars];
        memcpy(p + 1, p + 2, strlen(p) - 1);
      } else if (p[1] == 'x') {         // a hex number
        i = strtol(p + 2, &p2, 10);
        *p = i;
        memcpy(p + 1, p2, strlen(p2) + 1);
      } else if (isdigit((uint8_t) p[1]) && p[1] < '8') { // an octal number
        /* octal-number escape sequences have at most 3 octal digits.
           we cannot rely on strtol because it may find more than 3
           (e.g., when the user specifies '\000123' the \000 is an octal
           specification and the 123 is regular text; strtol would
           return 000123 as the number.  we must find the end of the
           octal number manually. */
        p2 = p + 2;             // just beyond the first octal digit
        for (i = 2; i < 4; i++)
          if (isdigit((uint8_t) *p2) && *p2 < '8')      // an octal digit
            p2++;
        c = *p2;                // temporary storage
        *p2 = '\0';             /* temporary end to force strtol not to
                                   read beyond the first three octal digits */
        i = strtol(p + 1, 8);
        *p2 = c;
        *p = i;
        memmove(p + 1, p2, strlen(p2) + 1);
      }
    }
    p++;
  }
  return p - p0;
}
//----------------------------------------------------------------
int32_t installString(char const* string)
// installs string in symbol stack; returns index to stack
{
 int32_t        index, n;
 char   *p, *p0;
#if YYDEBUG
 extern int32_t         yydebug;
#endif

 if ((index = nextFreeStackEntry()) == LUX_ERROR)
   return LUX_ERROR;            // error
 n = strlen(string) + 1;
 p = (char*) malloc(n);
 if (!p)
   return luxerror("Could not allocate %d bytes for string %s", 0, n, string);
 strcpy(p, string);
 p0 = p;

 if (translateEscapes(p) != n - 1) // we shortened the string
   p0 = (char*) realloc(p0, n + 1);

 symbolStack[index] = p0;
#if YYDEBUG
 if (yydebug)
   printf("installing %s as item %d\n", string, index);
#endif
 return index;
}
//----------------------------------------------------------------
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
    +                   (optional)  suppress evaluation of arguments
    -                   (optional)  suppress unused arguments
    |number|            (optional)  default value of internalMode
    %number%            (optional)  default value of plain offset
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
 char   *p, **result, *copy;
 KeyList        *theKeyList;
 int32_t        n = 1, i;

 if (!*(char **) keys)          // empty key
   return;
 // ANSI C does not allow string constants to be modified, so we
 // make a copy of the string and modify that
 if (!(copy = (char *) malloc(strlen(*(char **) keys) + 1))) {
   luxerror("Memory allocation error in installKeys [%s]", 0, (char *) keys);
   return;
 }
 strcpy(copy, *(char **) keys);
        // count and null-terminate individual keywords
 for (p = copy; *p; p++)
   if (*p == ':') {
     *p = '\0';
     n++;
   }
 if (!(result = (char **) malloc((n + 1)*sizeof(char *)))
     || !(theKeyList = (KeyList *) malloc(sizeof(KeyList)))) {
   luxerror("Memory allocation error in installKeys [%s].", 0, (char *) keys);
   return;
 }
 theKeyList->keys = result;
 p = copy;
 if (*p == '*') {               // suitable for piping
   theKeyList->pipe = 1;
   p++;
 } else
   theKeyList->pipe = 0;
 if (*p == '+') {               // evaluation suppression
   theKeyList->suppressEval = 1;
   p++;
 } else
   theKeyList->suppressEval = 0;
 if (*p == '-') {               // suppress unused arguments
                                // (see internal_routine())
   theKeyList->suppressUnused = 1;
   p++;
 } else
   theKeyList->suppressUnused = 0;
 if (*p == '|')         {               // default internalMode
   theKeyList->defaultMode = strtol(p + 1, &p, 10);
   p++;                         // skip final |
 } else
   theKeyList->defaultMode = 0;
 if (*p == '%')         {               // default offset for plain arguments
   theKeyList->offset = strtol(p + 1, &p, 10);
   p++;                         // skip final %
 } else
   theKeyList->offset = 0;
        // enter all keyword addresses in result list
 *result++ = p;
 for (i = 1; i < n; i++) {
   while (*p++);
   *result++ = p;
 }
 *result = 0;
 *(KeyList **) keys = theKeyList;
}
//----------------------------------------------------------------
int32_t findName(char const* name, HashTableEntry *hashTable[], int32_t context)
// searches for <name> in <hashTable[]> (with <context>).  if found,
// returns symbol number, otherwise installs a copy of the name in
// <hashTable[]> and sym[].  Returns -1 if an error occurs.  LS 6feb96
{
 int32_t                hashValue, i;
 HashTableEntry         *hp, *oldHp;
#if YYDEBUG
 extern int32_t         yydebug;
#endif
 extern char    ignoreSymbols;

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
     return hp->symNum;                 // found name: variable already defined
   oldHp = hp;
   hp = hp->next;
 }
                // wasn't defined yet;  install if not !xxx
 if (*name == '!' && !installing)
   return luxerror("Non-existent system variable %s", 0, name);
 hp = (HashTableEntry *) malloc(sizeof(HashTableEntry));
 if (!hp)
   return cerror(ALLOC_ERR, 0);
 if (oldHp)                     // current hash chain wasn't empty
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
//----------------------------------------------------------------
int32_t lux_verify(ArgumentCount narg, Symbol ps[])
/* verifies that all referenced subroutines, functions, and files
   actually exist */
{
  char  *name, *p, compileName[12], oldInstalling;
  FILE  *fp;
  int32_t       i, n, oldContext, nsym, result;
  extern char   compileOnly;
  extern int32_t        executeLevel;

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
        || newSymbol(LUX_BLOCKROUTINE, -nsym - 1) < 0) { // some error
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
            if ((p = strchr(name, ':'))) // seek specific routine
              *p++ = '\0';              // temporary string end
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
              // routine name was not yet evaluated
              name = string_value(n);
              if ((n = lookForName(name, subrHashTable, 0)) < 0) {
                // not compiled in the meantime
                fp = openPathFile(name, FIND_SUBR | FIND_LOWER);
                if (fp)                 // file exists
                  fclose(fp);
                else {
                  char const* cp = symbolProperName(symbol_context(i));
                  if (cp)
                    printf("%s: ", cp);
                  puts(symbolIdent(i, I_LINE));
                  printf("Cannot find subroutine %s\n", name);
                }
              }
            }
            break;
          case EVB_USR_CODE:
            n = usr_code_routine_num(i);
            if (symbol_class(n) == LUX_STRING) {
              // routine name was not yet evaluated
              name = string_value(n);
              if ((n = lookForName(name, blockHashTable, curContext)) < 0) {
                char const* cp = symbolProperName(symbol_context(i));
                if (cp)
                  printf("%s: ", cp);
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
//----------------------------------------------------------------
CompileInfo *nextFreeCompileInfo(void)
{
  if (cur_c_info >= n_c_info) { // need more room
    n_c_info += 16;
    c_info = (CompileInfo*) realloc(c_info, n_c_info*sizeof(CompileInfo));
    if (!c_info) {
      puts("pushCompileLevel:");
        cerror(ALLOC_ERR, 0);
      return NULL;
    }
  }
  return &c_info[cur_c_info++];
}
//----------------------------------------------------------------
void releaseCompileInfo(void)
{
  if (--cur_c_info)
    curCompileInfo--;
  else
    curCompileInfo = NULL;
}
//----------------------------------------------------------------
static ExecutionLevelInfo       *e_info;
static int32_t  n_e_info = 0, cur_e_info = 0;
void pushExecutionLevel(int32_t line, int32_t target)
{
  if (cur_e_info + 1 >= n_e_info) { // need more room
    n_e_info += 16;
    e_info = (ExecutionLevelInfo*) realloc(e_info, n_e_info*sizeof(ExecutionLevelInfo));
    if (!e_info) {
      puts("pushExecutionLevel:");
      cerror(ALLOC_ERR, 0);
      return;
    }
    e_info[0].target = 0;
  }
  e_info[cur_e_info].line = line; // remember the line number in the current
                                  // context
  e_info[++cur_e_info].target = target;// remember the next target
}
//----------------------------------------------------------------
void popExecutionLevel(void)
{
  cur_e_info--;
}
//----------------------------------------------------------------
void showExecutionLevel(int32_t symbol)
{
  int32_t       i, target;

  if (cur_e_info)
    for (i = cur_e_info; i >= 0; i--) {
      printf("At line ");
      if (i == cur_e_info)
        printf("%4d", symbol_line(symbol));
      else
        printf("%4d", e_info[i].line);
      printf(" in ");
      target = e_info[i].target;
      if (target > 0) {         // something user-defined
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
      } else {                  // a compiling file
        if (c_info[-target].name)
          printf("file \"%s\"\n", c_info[-target].name);
        else
          printf("main\n");
      }
    } else
      printf("At line %4d in main\n", symbol_line(symbol));
}
//----------------------------------------------------------------
