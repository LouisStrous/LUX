/* This is file defs.h.

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
/* defining a bunch of symbols for numerical constants denoting classes,
   types, etc., to enhance readability of the code */

/* symbol classes */
#define SCALAR		1
#define STRING		2
#define COMPILE_STR	3
#define ARRAY		4
#define TRANSFER	5
#define ASSOC		6
#define FUNC_PTR	7
#define SCAL_PTR	8
#define SYM_PTR		9
#define SUBSC_DESC	10
#define STRING_PTR	11
#define BIN_OP		192
#define NON_BIN_OP	193
#define EVB		200
#define UNDEFD		255

/* variable types */
#define BYTE		0
#define WORD		1
#define LONG		2
#define FLOAT		3
#define DOUBLE		4
#define TEMP_STRING	14
#define FIXED_STRING	15


/* setup options */
#define INSERT_TOGGLE	1	/* toggle between insert and overstrike modes */
#define OCTAL_ZERO	2	/* initial zero in numbers indicates octal */
#define SHOW_COMPILE	4	/* display compilation messages */
#define TRACE		8	/* display execution trace messages */
#define REDIRECT	16	/* redirect diagnostic output & store all */
#define STEP		32	/* execute one statement at a time */
#define BLACK_BACKGR	64	/* plot white on black instead of black on white */
#define MY_DEBUG	0x100000 /* display extra debugging info, if any */

/* ptab classes */
#define WHITESPACE      0
#define LETTER          1
#define DIGIT           2
#define EQUALS          4
#define LEFTPAREN       8
#define COMMA           16
#define COLON           32
#define OPERATOR        64
#define RIGHTPAREN      128
#define NONBLANK        255

/* stab classes */
#define STAB_IGNORE	0
#define STAB_NUMBER	1
#define STAB_XLETTER	2
#define STAB_OLETTER	4
#define STAB_HIPOUND	8
#define STAB_SEPARATOR	16
#define STAB_SUBSCR	32
#define STAB_SWITCH	64

/* statement codes */
#define BEGIN   1
#define IF      2
#define WHILE   3
#define REPEAT  4
#define FOR     5
#define DO      6
#define CALL    7
#define RETALL  10
#define SUBR    11
#define FUNC    12
#define PROG    13
#define RUN     14
#define EXITL   15
#define CASE    16
#define NCASE   17

/* EVB classes */
#define EVB_BLOCK	1
#define EVB_REPLACE	2
#define EVB_INT_SUB	3
#define EVB_FOR		4
#define EVB_INSERT	5
#define EVB_IF		6
#define EVB_USR_SUB	7
#define EVB_WHILE_DO	8
#define EVB_DO_WHILE	9
#define EVB_RETURN	10
#define EVB_CASE	11
#define EVB_NCASE	12
#define EVB_REPEAT	13
#define EVB_USR_CODE	14

/* standard streams */
#define STDOUT  0
#define STDIN   1
#define STDERR  2

/* line edit codes */
#define NL      10      /* new line */
#define CR      13      /* carriage return */
#define DEL_LN  21      /* delete line           (ctrl-U) */
#define NEXTW   23      /* next Word             (ctrl-W) */
#define DELW    4       /* delete Word           (ctrl-D) */
#define DELEOL  11      /* delete to end of line (ctrl-K) */
#define CONTRL  27      /* control sequence indicator */

#define INSERT  1       /* insert/overwrite      (ctrl-A) */
#define BS      8       /* backspace             (ctrl-H) */
#define DELETE  127     /* delete                         */
#define LINEBEG 2       /* beginning of line     (ctrl-B) */
#define LINEEND 5       /* end of line           (ctrl-E) */
#define FWD     18      /* next char             (ctrl-R) */
#define BKW     12      /* previous char         (ctrl-L) */
#define PRV     16      /* previous line         (ctrl-P) */
#define NXT     14      /* next line             (ctrl-N) */
#define QUIT    24      /* quit                  (ctrl-X) */
#define FIND    6       /* find                  (ctrl-F) */
#define SUBST   20      /* find & replace        (ctrl-T) */

#define CSI     "\033["

#define LTARROW 'D'
#define RTARROW 'C'
#define DNARROW 'B'
#define UPARROW 'A'

/* macros */

/* Byte string macros */
#define bstrcpy(a, b) strcpy((char *) a, (char *) b)
#define bstrlen(a)    strlen((char *) a)

/* check whether symbol is an array */
#define CK_ARR(SYM, ARGN) \
  if (sym[SYM].class != ARRAY) return exec_n_error(66, ARGN, SYM)
#define CK_GHARR(SYM, ARGN)\
  if (sym[SYM].class != ARRAY && sym[SYM].class != GHOST_ARRAY) \
    return exec_n_error(66, ARGN, SYM)

/* memory allocation */
#define allocate(ptr, num, type) \
  if (!(ptr = (type *) malloc(num*sizeof(type)))) return execute_error(ALLOC_ERR);

/* ptr to array's ahead */
#define HEAD(SYM)		(struct ahead *) sym[SYM].spec.array.ptr

/* LONG ptr to array data */
#define LPTR(HEAD)		(Int *)((char *) HEAD + sizeof(struct ahead))

/* check that all array elements are nonnegative */
#define CK_SGN(ARR, N, ARGN, SYM) \
  for (i = 0; i < N; i++) \
    if (ARR[i] < 0) return exec_n_error(65, ARGN, SYM)

/* check that all array element are smaller than MAG */
#define CK_MAG(MAG, ARR, N, ARGN, SYM) \
  for (i = 0; i < N; i++) \
    if (ARR[i] >= MAG) return exec_n_error(81, ARGN, SYM)

/* check that all array elements are smaller than the corresponding RNG elms */
#define CK_RNG(RNG, ARR, N, ARGN, SYM) \
  for (i = 0; i < N; i++) \
    if (ARR[i] >= RNG[i]) return exec_n_error(98, ARGN, SYM)

/* multiply all array elements (dimensions) to get size */
#define GET_SIZE(SIZ, ARR, N) SIZ = 1; for (i = 0; i < N; i++) SIZ *= ARR[i]

/* get pointer to numerical values in PTR.l, # of elements in SIZE
   (works for scalar, scalar pointer, and arrays) */
#define GET_NUMERICAL(PTR, SIZE, ARGNUM) \
  switch (sym[iq].class) \
  { case SCAL_PTR:  iq = class8_to_1(iq);  \
    case SCALAR:    PTR.l = &sym[iq].spec.scalar.l;  SIZE = 1;  break;  \
    case ARRAY:     h = HEAD(iq);  PTR.l = LPTR(h);  GET_SIZE(SIZE, h->dims, h->ndim);  \
      break; \
    default:        return exec_n_error(ILL_CLASS, ARGNUM, iq); }    

/* array display macro */
#define SHOW_ARR(STR, ARR, SIZE) printf(STR); \
  for (i = 0; i < SIZE; i++) printf("%d ", ARR[i]); putchar('\n')

/* multi-type macros: */
#define multiSwitch2(type, first, second) \
 switch (type) \
 { case BYTE:   first .b second; break; \
   case WORD:   first .w second; break; \
   case LONG:   first .l second; break; \
   case FLOAT:  first .f second; break; \
   case DOUBLE: first .d second; break; }

#define multiSwitch3(type, first, second, third) \
 switch (type) \
 { case BYTE:   first .b second .b third; break; \
   case WORD:   first .w second .w third; break; \
   case LONG:   first .l second .l third; break; \
   case FLOAT:  first .f second .f third; break; \
   case DOUBLE: first .d second .d third; break; }

#define multiSwitch4(type, first, second, third, fourth) \
 switch (type) \
 { case BYTE:   first .b second .b third .b fourth; break; \
   case WORD:   first .w second .w third .w fourth; break; \
   case LONG:   first .l second .l third .l fourth; break; \
   case FLOAT:  first .f second .f third .f fourth; break; \
   case DOUBLE: first .d second .d third .d fourth; break; }

