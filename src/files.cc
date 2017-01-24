/* This is file files.cc.

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
/* File files.c */
/* LUX routines dealing with file and terminal I/O. */

/* LUX FZ file format:
  Byte     description
   0-3     .synch_pattern: magic number that identifies LUX FZ files
           The value 0x5555aaaa is always written as an int32_t, but it depends
	   on the Byte order of the machine whether Byte 0 becomes 0x55
	   (MSB) or 0xaa (LSB).  This can therefore be used to check on
	   the Byte order of the source machine.
   4       .subf: 0th bit: 1 if Rice compressed, 0 if plain
           7th bit: 1 if MSB first, 0 if LSB first.
   5       .source: an indication for the source of the data.  currently
           always equal to 0.
   6       .nhb: number of header blocks of 512 bytes each
           (including the current block).  If the text header has less than
           256 characters, then it is stored in .txt; otherwise (nhb-1)
	   additional 512-Byte blocks follow the current data header.
   7       .datyp: the type of the data; 0 -> BYTE, 1 -> WORD, 2 -> LONG,
           3 -> FLOAT, 4 -> DOUBLE.
   8       .ndim: the number of dimensions in the data.
   9       .free1: unused.
  10-13    .cbytes[4]: used if Rice compression is applied.
  14-191   .free[178]: unused.
  192-255  .dim[16]: the dimensions of the data in the Byte order of the
           source machine.
  256-511  .txt[256]: text header.

*/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>		/* for close() */

#include <strings.h>		/* for bzero(), bcopy() */

#include "lux_structures.hh"	/* for RAS FITS stuff */
#include <stdio.h>		/* for printf(), puts(), FILE, fopen(), */
				/* sscanf(), vsprintf(), sprintf(), fputs(), */
				/* fputc(), stdout, fflush(), fprintf(), */
				/* fclose(), fseek(), getc(), feof(), */
				/* fgetc(), fscanf(), stdin, fread(), */
				/* fwrite(), perror(), putchar(), putc(), */
				/* ferror(), remove(), rename(), NULL, */
				/* ftell(), fgetpos(), fsetpos(), getchar(), */
				/* vfprintf() */
#include <string.h>		/* for strcspn(), strlen(), strcpy(), */
				/* strncpy(), strrchr(), strcmp(), strcat(), */
				/* strtok(), strchr(), strspn(), strpbrk(), */
				/* memcpy(), memmove(), strstr(), strdup() */
#include <stdlib.h>		/* for getenv(), putenv(), system(), free(), */
				/* malloc(), strtol(), realloc(), atol(), */
				/* atof() */
#include <ctype.h>		/* for tolower(), isspace(), isdigit() */
#include <limits.h>		/* for INT32_MAX */
#include <float.h>		/* for FLT_MAX, DBL_MAX */
#include <stdarg.h>		/* for va_list, va_start(), va_end() */
#include <sys/types.h>		/* for stat(), opendir(), readdir(), */
				/* closedir(), regcomp(), regfree(), */
				/* regexec() */
#include <sys/stat.h>		/* for stat(), struct stat */
#include <unistd.h>		/* for chdir(), getcwd() */
#include <dirent.h>		/* for DIR, struct dirent, opendir(), */
				/* readdir(), closedir() */
#if HAVE_REGEX_H
#include <regex.h>		/* for regcomp(), regfree(), regexec() */
#endif
#include "action.hh"
#include "install.hh"
#include "editor.hh"		/* for BUFSIZE */
#include "format.hh"
#include <errno.h>

#define FMT_INSTALL	1
#define FMT_CLEANUP	2

extern	int32_t	noioctl, iformat, fformat, sformat, cformat, tformat,
  noTrace;
int32_t	lux_swab(int32_t, int32_t *);
int32_t	swapl(void *, int32_t);
extern uint8_t	line2[];
extern char	expname[], batch;
extern int32_t	nest;	/* batch flag */
extern FILE	*inputStream;
extern int32_t	 error_extra;
int32_t	type_ascii(int32_t, int32_t [], FILE *),
  read_ascii(int32_t, int32_t [], FILE *, int32_t),
  getNewLine(char *, size_t, char const *, char),
  redef_string(int32_t, int32_t);
void	zerobytes(void *, int32_t), endian(void *, int32_t, int32_t),
  fprintw(FILE *fp, char const* string);
extern	int32_t scrat[];				/* scratch storage, also used 
						 in anatest.c */
/* some tables for lun type files */
FILE	*lux_file[MAXFILES];
int32_t	lux_file_open[MAXFILES];	/* our own flag for each file */
char	*lux_file_name[MAXFILES]; /* pointers to open file names */
int32_t	lux_rec_size[MAXFILES];		/*for associated variable files */
/* items used by ASCII read routines */
int32_t	maxline = BUFSIZE;
char	expname[BUFSIZE];
char	*str, *str2, *ulib_path; /* ulib_path global for lux_help
				    LS 15sep92*/
int32_t	index_cnt = 0, line_cnt, left_cnt;
extern int32_t	crunch_slice;
int32_t	column = 0;

static char	fits_head_malloc_flag = 0, *fitshead, *preamble;
static int32_t	runlengthflag = 0;

char	*fmttok(char *);

int32_t	anacrunch(uint8_t *, int16_t [], int32_t, int32_t, int32_t, int32_t),
	anacrunch8(uint8_t *, uint8_t [], int32_t, int32_t, int32_t, int32_t);
int32_t	anacrunchrun(uint8_t *, int16_t [], int32_t, int32_t, int32_t, int32_t),
	anacrunchrun8(uint8_t *, uint8_t [], int32_t, int32_t, int32_t, int32_t);
#if SIZEOF_LONG_LONG_INT == 8	/* 64-bit integers */
int32_t	anacrunch32(uint8_t *, int32_t [], int32_t, int32_t, int32_t, int32_t);
#endif

int32_t	byte_count;		/* also used by tape.c, which is only */
				/* included if HAVE_SYS_MTIO_H is defined */

void read_a_number_fp(FILE *, Scalar *, Symboltype *);

#define ASK_MORE	0
#define ERROR_EOF	1
#define CONT_EOF	2
#define TRUNCATE_EOF	3
/*------------------------------------------------------------------------- */
char *expand_name(char const* name, char const* extension)
 /* expands environment variables (preceded by $ or ~) at the start of a file
    name, i.e. filenames like  $HOME/test.dat, $FILE, and ~/Scratch.
    The environment name may have multiple / at the end; the expanded name
    will only have one at the corresponding position.  Final \n is stripped.
    If <extension> is unequal to NULL, it is appended to the file name if it
    is not yet present.
    (Possibly) expanded name is put in expname, which is returned.
    LS 7jul92 */
/* Headers:
   <string.h>: strcspn(), strlen(), strcpy(), strrchr(), strcmp(), strcat()
   <stdlib.h>: getenv()
   <stdio.h>: printf()
 */

{
 int32_t    n, n2, error = 0;
 char   *envname, *p;

	/* first expand possible environment variable */
 *expname = '\0';
 if (*name == '$' || *name == '~')
 { n = strcspn(name, "/");
   if (n == 0)  n = strlen(name) + 1;
   if (*name == '~') strcpy(expname, "HOME");
   else
   { strncpy(expname, name + 1, n - 1);                  /* skip $ */
     *(expname + n - 1) = '\0';	}			/* terminate */
   envname = getenv(expname);
   if (envname != NULL)				/* translation found */
   { n2 = strlen(envname);
     if (n2 >= BUFSIZE) {error = 1; *expname = '\0';}
     else
     { strcpy(expname, envname);
       for (p = expname + n2 - 1; *p == '/'; p--);	/* remove spurious / */
       *++p = '\0';
       n2 = strlen(expname) + strlen(name) - n;
       if (n2 >= BUFSIZE) {error = 1; *expname = '\0';}
       else strcpy(expname + strlen(expname), name + n); }
   } else {error = 2; *expname = '\0';}
 }
 if (*expname == '\0') strcpy(expname, name);
 for (p = expname; *p && *p != '\n'; p++);	/* remove \n */
 *p = '\0';
 if (extension)				/* examine extension */
 { if ((p = strrchr(expname, '/')) == NULL) p = expname;	/* last / */
   if ((p = strrchr(p, '.')) == NULL || strcmp(p, extension))
     /* no extension yet, or extension is unequal to <extension> */
   { if (strlen(expname) + strlen(extension) >= BUFSIZE) error = 1;
     else strcat(expname, extension); }
 }
 return expname;
}
/*------------------------------------------------------------------------- */
int32_t lux_setenv(int32_t narg, int32_t ps[])
/* specify or display an environment variable; these are useful in file names.
   Uses POSIX routine putenv(char *string)
   LS 1apr94, 20jul2000 */
/* Headers:
   <string.h>: strchr()
   <stdio.h>: puts()
   <stdlib.h>: getenv(), putenv()
*/
{
 char	*p, *p2;

 if (symbol_class(*ps) != LUX_STRING)
   return cerror(NEED_STR, *ps);
 p = string_value(*ps);
 if ((p2 = strchr(p, '=')) == NULL) { /* no "=" -> display translation */
   puts(getenv(p));
   return LUX_OK;
 }
 if (putenv(strsave(p)))
   /* we must use strsave because only the pointer argument to putenv() is
      really remembered. LS 2dec98 */
   return luxerror("Could not add %s to environment", *ps, p);
 return LUX_OK;
}
/*------------------------------------------------------------------------- */
int32_t lux_getenv(int32_t narg, int32_t ps[])
/* get an environment variable; these are useful in file names. */
/* Headers:
   <stdlib.h>: getenv()
   <string.h>: strlen(), strcpy()
*/
{
 char	*p;
 int32_t	n, result;

 if (symbol_class(*ps) != LUX_STRING)
   return cerror(NEED_STR, *ps);
 p = getenv(string_value(*ps));
 n = p? strlen(p): 0;
 result = string_scratch(n);
 if (p)
   strcpy(string_value(result), p);
 else
   *string_value(result) = '\0';
 return result;
}
/*---------------------------------------------------------------------------*/
int32_t lux_spawn(int32_t narg, int32_t ps[])		/* execute a shell command */
 /* uses the system call to execute a shell command, but may not be the
 	shell you want! depends on local setup */
/* Headers:
   <stdlib.h>: system()
 */
{
  char *p;
  int32_t	result;

  if (!symbolIsStringScalar(ps[0]))
    return cerror(NEED_STR, *ps);
  p = string_value(ps[0]);
  result = system(p);
  if (internalMode & 1)		/* just return */
    return 1;
  else return result == 0? 1: -1;
 }
/*---------------------------------------------------------------------------*/
int32_t lux_spawn_f(int32_t narg, int32_t ps[])		/* execute a shell command */
 /* uses the system call to execute a shell command, but may not be the
 	shell you want! depends on local setup */
/* Headers:
   <stdlib.h>: system()
 */
{
  char *p;
  int32_t	result;

  if (!symbolIsStringScalar(ps[0]))
    return cerror(NEED_STR, *ps);
  p = string_value(ps[0]);
  result = scalar_scratch(LUX_INT32);
  scalar_value(result).l = system(p);
  return result;
 }
/*------------------------------------------------------------------------- */
FILE *openPathFile(char const* name, int32_t mode)
/* If name starts with $, then expands environment variable and tries
  to open file with resulting name.  If name starts with /, then searches
  in named directory (full path name).  Else, searches for the named file in
  the directories listed in LUX_PATH (if set), and in ulib (if set), and
  in the current directory (if LUX_PATH nor ulib are set).
  Returns file pointer to found and opened file, or zero.
  mode = FIND_SUBR: find routine (.lux); FIND_FUNC: function (_f.lux);
  FIND_EITHER: either; else don't add extension.  If FIND_LOWER is also
  set, then the name is transformed to lower case before the file is
  sought.
  Extension is added only if no extension (.something) is present yet.
  LS 10/20/92 */
/* Headers:
   <stdio.h>: FILE, fopen(), printf()
   <ctype.h>: tolower(), isspace()
   <stdlib.h>: getenv(), free()
   <string.h>: strcpy(), strcat(), strtok(), strchr(), strrchr()
 */
{
  char	*copy, *p, *plist;
  FILE	*fin;
  extern int32_t	echo, trace, step, executeLevel, traceMode;

  copy = strsave((char*) name);
  if (mode & FIND_LOWER) {
    p = copy;
    while (*p) {
      *p = tolower(*p);
      p++;
    }
  }
  mode &= ~FIND_LOWER;
  p = copy;
  while (isspace((uint8_t) *p++));			/* skip spaces */
  switch (*--p) {
    case '$': case '~': case '/':
      plist = NULL;
      break;
    default:
      *curScrat = '\0';
      p = getenv("LUX_PATH");
      if (p) {
	strcpy(curScrat, p);
      }
      if (ulib_path) {		/* the ulib path goes last */
	if (*curScrat)
	  strcat(curScrat, ":");
	strcat(curScrat, ulib_path);
      }
      if (!*curScrat)		/* none yet */
	strcpy(curScrat, ".");
      plist = strsave(curScrat);
  }
  p = 0;
  fin = 0;
  do {
    if (plist) {
      if (p)
	p = strtok(NULL, ":");
      else
	p = strtok(plist, ":");
      if (p) {
	strcpy(curScrat, expand_name(p, NULL));
        strcat(curScrat, "/");
        strcat(curScrat, (char *) copy);
      }
    } else
      strcpy(curScrat, expand_name(copy, NULL));
    if (p || !plist) {					/* still a path */
      strcpy(expname, curScrat);
      if (mode == 0 || mode == 2) {		/* routine */
	if (!strchr(strrchr(curScrat, '/'), '.'))
	  strcat(curScrat, ".lux");
	fin = fopen(curScrat, "r");
      }
      if (fin == NULL && mode >= 1) {		/* function */
	if (mode == 2)
	  strcpy(curScrat, expname);	/* delete extension */
	if (!strchr(strrchr(curScrat, '/'), '.'))
	  strcat(curScrat, "_f.lux");
                                                /* add new one */
        fin = fopen(curScrat, "r");
      }
    }
  } while (p && fin == NULL);
  free(copy);
  if (plist)
    free(plist);
  if (fin && (echo || trace > executeLevel || step > executeLevel
	      || (traceMode & T_ROUTINEIO)))
    printf("Reading from file %s\n", curScrat);
  return fin;
}
/*------------------------------------------------------------------------- */
int32_t lux_ulib(int32_t narg, int32_t ps[])
 /*set ulib path */
/* Headers:
   <stdio.h>: printf()
   <stdlib.h>: free()
 */
{
 if (narg == 0) {
   printf("current ulib path: %s\n", ulib_path);
   return LUX_OK;
 }

 /* printf("ulib_path = %d\n", ulib_path);*/
 if (ulib_path != NULL)
   free(ulib_path);
 if (symbol_class(ps[0]) != LUX_STRING )
   return cerror(NEED_STR, *ps);
 ulib_path = strsave(string_value(ps[0]));
 return LUX_OK;
}
/*------------------------------------------------------------------------- */
int32_t lux_type(int32_t narg, int32_t ps[])
 /* (stdin) printing */
{
  int32_t	type_ascii(int32_t, int32_t [], FILE *);

  return type_ascii(narg, ps, stdout);
}
/*------------------------------------------------------------------------- */
int32_t lux_printf(int32_t narg, int32_t ps[])
 /* file (stdin) reading and formatting */
/* Headers:
   <stdio.h>: FILE
 */
{
  int32_t	lun, col, result;
  FILE	*fp;

  lun = int_arg(ps[0]);
  if (lun < 0 || lun >= MAXFILES)
    return cerror(ILL_LUN, *ps);
  if (lux_file_open[lun] == 0)
    return cerror(LUN_CLOSED, *ps);
  if (lux_file_open[lun] != 2)
    return cerror(READ_ONLY, *ps);
  fp = lux_file[lun];
  /* column is changed by type_ascii. store it (screen column) and the
     screen width, and put in our own values. */
  col = column;  column = 0;
  result = type_ascii(narg - 1, &ps[1], fp);
  /* restore column */
  column = col;
  return result;
 }
/*------------------------------------------------------------------------- */
int32_t lux_printf_f(int32_t narg, int32_t ps[])
 /* a function version that returns 1 if read OK */
{
  return lux_printf(narg, ps) == LUX_OK? LUX_ONE: LUX_ZERO;
}
/*------------------------------------------------------------------------- */
int32_t format_width = 0, format_precision = 0;
int32_t format_check(char *format, char **next, int32_t store)
/* checks whether a string is a valid output format string.
  Stores string in apropriate global variable (fformat, iformat, sformat)
  if legal.
  Returns pointer to just after the checked format in next, if non-NULL.
  Returns code of field type (0..12 for diouxXeEfgGts), or an error code:
  -1 -> no %, -2 -> too long (if stored), -3 -> no legal field type.
   Allowed formats:
     [anything] % [any of -+# or space] [digits] [.] [digits]
     [any of hlL] {one of diouxXeEfgGst} [ignored anything]
   LS 24apr93 */
/* stores format's width and precision in format_width and format_precision, */
/* or stores -1 if not specified.  LS 8mar97 */
/* Headers:
   <string.h>: strchr(), strspn(), strncpy()
   <ctype.h>: isdigit()
   <stdio.h>: sscanf()
   <stdlib.h>: malloc()
 */
{
 char	*p, *p2, *p3;
 static char	format_kinds[] = "diouxXeEfgGtTjs";
 int32_t	n = 0, k;
 int32_t	lux_delete(int32_t, int32_t []);

 if (next)
   *next = format;	/* defaults */
 do {
   p = format;
   while (1) {
     p = strchr(p, '%');	/* find next % */
     if (!p)
       return -1;		/* no % */
     if (p[1] == '%')
       p = p + 2;
     else
       break;
   }
   n += p - format;
   p2 = p++;			/* add prefix to size */
   p += strspn(p, "-+ 0#");	/* skip -+0# and space */
   if (isdigit((uint8_t) *p))		/* width */
     sscanf(p, "%d", &format_width);
   else
     format_width = -1;		/* not specified */
   while (isdigit((uint8_t) *p))
     p++;			/* skip width */
   if (*p == '.') {
     p++;
     if (isdigit((uint8_t) *p))		/* precision */
       sscanf(p, "%d", &format_precision);
     else
       format_precision = -1;	/* not specified */
     while (isdigit((uint8_t) *p))
       p++;			/* skip precision */
   } else
     format_precision = -1;	/* not specified */
   p += strspn(p, "hlL");	/* skip hlL */
   p3 = strchr(format_kinds, *p); /* get conversion type */
   if (!p3 && *p != '%')
     return -3;			/* illegal type */
   p++;
 }
 while (p[-1] == '%') ;		/* %% is escape */
 k = p3 - format_kinds;
 if (next)
   *next = p;
 if (store) {
   n = n + p - p2;
   if (isIntegerFormat(k)) {	/* integer type */
     lux_delete(1, &iformat);
     symbol_class(iformat) = LUX_SCAL_PTR;
     scal_ptr_type(iformat) = LUX_TEMP_STRING;
     scal_ptr_pointer(iformat).b = (uint8_t*) malloc(n + 1);
     symbol_memory(iformat) = n + 1;
     strncpy(p = scal_ptr_pointer(iformat).s, format, n);
     p[n] = '\0';
   } else if (isFloatFormat(k)) { /* floating point kind */
     lux_delete(1, &fformat);
     symbol_class(fformat) = LUX_SCAL_PTR;
     scal_ptr_type(fformat) = LUX_TEMP_STRING;
     scal_ptr_pointer(fformat).b = (uint8_t*) Malloc(n + 1);
     symbol_memory(fformat) = n + 1;
     strncpy(p = scal_ptr_pointer(fformat).s, format, n);
     p[n] = '\0';
   } else {			/* string kind */
     lux_delete(1, &sformat);
     lux_delete(1, &sformat);
     symbol_class(sformat) = LUX_SCAL_PTR;
     scal_ptr_type(sformat) = LUX_TEMP_STRING;
     scal_ptr_pointer(sformat).b = (uint8_t*) malloc(n + 1);
     symbol_memory(sformat) = n + 1;
     strncpy(p = scal_ptr_pointer(sformat).s, format, n);
     p[n] = '\0';
   }
 }
 return k;
}
/*------------------------------------------------------------------------- */
int32_t input_format_check(char *format, char **next, char **widths, int32_t *datatype,
		       char *formatchar, char *suppress, int32_t *number)
/* checks whether a string is a valid input format string.  Returns pointer
 to just after the checked format in <*next>, if <next> is non-NULL.
 Returns character code of data type in <*formatchar>, or 0 if no data type
 is present.  Returns suppression flag (1 for suppression, 0 for no
 suppression) in <*suppress>.  Returns requested data value type
 (LUX_INT32 etc) in <*datatype>, and the number of repeats in <*number>.
 Returns a positive number if all is OK (1 plus the number of chars of
 explicit text before the actual format entry), -1 if an illegal format
 is found, and 0 if explicit text but no data type specification is found. 
 Allowed formats:
   [anything] % [*] [digits] [-digits...] [.digits] [any of hlL]
   {one of dioxefgcsS} [digits #]
 Returns pointer to start of field width(s) in <*widths>.
 LS 17aug97 */
/* Headers:
   <ctype.h>: isspace(), isdigit()
   <stdlib.h>: strtol()
 */
{
  int32_t	big, is_explicit;
  char	*p, *p2;

  /* skip initial whitespace: not all compilers handle it the same way */
  p = format;
  while (*format && isspace((uint8_t) *format))
    format++;
  p2 = format;			/* possible start of explicit text */
  /* seek format entry */
  while (1) {
    while (*format && *format != '%')
      format++;
    if (*format != '%') {	/* no format entry */
      if (next)
	*next = format;
      return 0;
    }
    if (*++format != '%')	/* not a literal % */
      break;
    format++;
  }
  is_explicit = (format - p2);
  if (is_explicit > 1)
    is_explicit = (format - p);
  if (*format == '*') {
    *suppress = 1;
    format++;
  } else
    *suppress = 0;
  *number = 0;			/* count elements per format */
  *widths = format;
  while (1) {
    strtol(format, &format, 10); /* format width - if any */
    (*number)++;
    if (*format == '-')		/* format set */
      format++;
    else
      break;
  }
  if (*format == '.')		/* precision - only useful for %t and %T */
    while (isdigit((uint8_t) *++format));
  big = 0;			/* modifier */
  switch (*format) {
  case 'h':
    big = 1;
    format++;
    break;
  case 'l':
    big = 2;
    format++;
    break;
  case 'L':
    big = 3;
    format++;
    break;
  case 'j':
    big = 4;
    ++format;
    break;
  }
  *formatchar = *format;
  switch (*format) {		/* data type */
    case 'd': case 'i': case 'o': case 'x': /* integer */
      switch (big) {
      case 1:
        *datatype = LUX_INT16;
        break;
      case 4:
        *datatype = LUX_INT64;
        break;
      default:
        *datatype = LUX_INT32;
        break;
      }
      break;
    case 'e': case 'f': case 'g': case 't': case 'T': /* floating point */
      *datatype = (big == 3)? LUX_DOUBLE: LUX_FLOAT;
      break;
    case 'c':
      *datatype = LUX_INT8;
      break;
    case 's': case 'S':
      *datatype = LUX_TEMP_STRING;
      break;
    case '[':
      *datatype = LUX_TEMP_STRING;
      if (*++format == ']')
	format++;
      while (*format != ']')
	format++;
      break;
    default:			/* illegal type */
      *datatype = LUX_ERROR;
      return -1;
  }
  p = ++format;			/* just after data type specification */
  /* look for repeat count */
  while (isdigit((uint8_t) *p))
    p++;
  if (p != format && *p == '#')	 /* have a repeat count */
    *number *= strtol(format, &format, 10);
  if (next)
    *next = format;
  return is_explicit;
}
/*------------------------------------------------------------------------- */
int32_t lux_format_set(int32_t narg, int32_t ps[])
 /* check & set (multiple) print formats.  Allowed formats:
     [anything] % [digits] [.] [digits] {one of diouxXeEfgGsz}
    d,i,o,u,x,X-types are put in !format_i; e,E,f,g,G-types in !format_f,
    s-type in !format_s, and z-type in !format_c
    If no arguments to LUX call, then set to default values
    LS 13jul92  16nov98 */
/* Headers:
   <string.h>: strlen(), strcat(), strcpy()
   <stdlib.h>: realloc()
 */
{
  char	*string, *fmt;
  int32_t	n, iq = 0;
  extern formatInfo	theFormat;
  Pointer	p;

  if (narg) {
    if (symbol_class(ps[0]) != LUX_STRING)
      return cerror(NEED_STR, ps[0]);
    string = string_value(ps[0]);
    fmt = fmttok(string);
    if (!fmt)
      return luxerror("Illegal format string", ps[0]);
    while (fmt) {
      if (theFormat.count >= 0)
	return luxerror("Repeat counts not allowed in FORMAT_SET", ps[0]);
      switch (theFormat.type) {
      case FMT_INTEGER:
	iq = iformat;
	break;
      case FMT_FLOAT: case FMT_TIME: case FMT_DATE:
	iq = fformat;
	break;
      case FMT_STRING:
	iq = sformat;
	break;
      case FMT_COMPLEX:
	iq = cformat;
	break;
      case FMT_PLAIN:
	if (!iq)
	  return luxerror("Illegal format string", ps[0]);
	/* else we add it to the last one */
	p = scal_ptr_pointer(iq);
	n = strlen(fmt) + 1;
	p.s = (char*) realloc(p.s, strlen(p.s) + n);
	strcat(p.s, fmt);
	symbol_memory(iq) += n;
	fmt = fmttok(NULL);
	continue;
      }
      p = scal_ptr_pointer(iq);
      n = strlen(fmt) + 1;
      p.s = (char*) realloc(p.s, n);
      strcpy(p.s, fmt);
      symbol_memory(iq) = n;
      fmt = fmttok(NULL);
    }
  } else {			/* no arguments: restore defaults */
    p = scal_ptr_pointer(fformat);
    p.s = (char*) realloc(p.s, 7);
    strcpy(p.s, "%14.7g");
    symbol_memory(fformat) = 7;
    p = scal_ptr_pointer(iformat);
    p.s = (char*) realloc(p.s, 5);
    strcpy(p.s, "%10jd");
    symbol_memory(iformat) = 5;
    p = scal_ptr_pointer(sformat);
    p.s = (char*) realloc(p.s, 3);
    strcpy(p.s, "%s");
    symbol_memory(sformat) = 3;
    p = scal_ptr_pointer(cformat);
    p.s = (char*) realloc(p.s, 7);
    strcpy(p.s, "%14.7z");
    symbol_memory(cformat) = 7;
  }
  return LUX_OK;
}
/*------------------------------------------------------------------------- */
static int32_t	pager = -1;
extern int32_t	page;		/* number of lines per page */
static int32_t	printwLines = 0; /* number of lines output by printw so far */
void setPager(int32_t offset)
     /* sets pager for printw and printwf */
{
  pager = (printwLines + page - 2 + offset) % (page - 2);
}
/*------------------------------------------------------------------------- */
void resetPager(void)
     /* turns off paging */
{
  pager = -1;
}
/*------------------------------------------------------------------------- */
void printwf(char const* fmt, ...)
/* Like printw, but with a format string and multiple argument strings. */
/* LS 12jul95 */
/* Headers:
   <stdarg.h>: va_list, va_start(), va_end(), vsprintf()
   <stdio.h>: vsprintf()
 */
{
  va_list	ap;

  va_start(ap, fmt);
  vsprintf(curScrat, fmt, ap);
  va_end(ap);
  printw(curScrat);
}
/*------------------------------------------------------------------------- */
void type_ascii_one(int32_t symbol, FILE *fp)
/* prints one symbol's contents to a stream. */
/* Headers:
   <stdio.h>: sprintf(), FILE
   <string.h>: strcpy()
*/
{
  uintmax_t intval;
  double fltval;
  extern char	*fmt_integer, *fmt_float, *fmt_complex, *fmt_string;
  extern formatInfo	theFormat;
  int32_t	i, n, j;
  int32_t	Sprintf_tok(char *, ...);
  char	*nextformat(char *, int32_t);
  Pointer	ptr;
  listElem	*sptr;

  symbol = transfer(symbol);

  switch (symbol_class(symbol)) {
  case LUX_SCALAR: case LUX_FIXED_NUMBER:
    switch (scalar_type(symbol)) {
    case LUX_INT8:
      intval = (intmax_t) scalar_value(symbol).b;
      break;
    case LUX_INT16:
      intval = (intmax_t) scalar_value(symbol).w;
      break;
    case LUX_INT32:
      intval = (intmax_t) scalar_value(symbol).l;
      break;
    case LUX_INT64:
      intval = (intmax_t) scalar_value(symbol).q;
      break;
    case LUX_FLOAT:
      fltval = scalar_value(symbol).f;
      break;
    case LUX_DOUBLE:
      fltval = scalar_value(symbol).d;
      break;
    default:
      cerror(ILL_TYPE, symbol);
      break;
    }
    if (symbolIsInteger(symbol)) { /* integer type */
      sprintf(curScrat, fmt_integer, intval);
      fprintw(fp, curScrat);
    } else { 			/* float type */
      /* it may be a %j, %z, %t, or %T so we must use our own printer */
      Sprintf(curScrat, fmt_float, fltval);
      fprintw(fp, curScrat);
    }
    break;
  case LUX_CSCALAR:
    ptr.cf = complex_scalar_data(symbol).cf;
    if (complex_scalar_type(symbol) == LUX_CFLOAT)
      Sprintf(curScrat, fmt_complex, (double) ptr.cf->real,
              (double) ptr.cf->imaginary);
    else
      Sprintf(curScrat, fmt_complex, ptr.cd->real, ptr.cd->imaginary);
    fprintw(fp, curScrat);
    break;
  case LUX_STRING:
    sprintf(curScrat, fmt_string, string_value(symbol));
    fprintw(fp, curScrat);
    break;
  case LUX_RANGE:
    fprintw(fp, "(");
    i = range_start(symbol);
    if (i == -1)		/* (*) */
      strcpy(curScrat++, "*");
    else {
      if (i < 0) {		/* (* - expr ...) */
        i = -i;
        fprintw(fp, "*-");
      }
      type_ascii_one(i, fp);
      i = range_end(symbol);
      if (i != LUX_ZERO) {	/* really have a range end */
        fprintw(fp, ":");
        if (i < 0) {		/* * ... */
          if (i == -1) {	/* just * */
            fprintw(fp, "*");
            i = 0;
          } else {		/* * - expr */
            fprintw(fp, "*-");
            i = -i;
          }
        }
        if (i)
          type_ascii_one(i, fp);
      }
      if (range_sum(symbol))
        fprintw(fp, ":+");
      if (range_redirect(symbol) >= 0) {
        fprintw(fp, ":>");
        type_ascii_one(range_redirect(symbol), fp);
      }
    }
    fprintw(fp, ")");
    break;
  case LUX_ARRAY:
    j = array_size(symbol);
    ptr.b = (uint8_t*) array_data(symbol);
    switch (array_type(symbol)) {
    case LUX_INT8:
      fmttok(fmt_integer);
      while (j--) {
        if (!j && (theFormat.flags & FMT_MIX2))
          theFormat.spec_char[1] = '\0';
        Sprintf_tok(curScrat, (intmax_t) *ptr.b++);
        fprintw(fp, curScrat);
        if (j && (internalMode & 4))
          nextformat(NULL, 1);
      }
      break;
    case LUX_INT16:
      fmttok(fmt_integer);
      while (j--) {
        if (!j && (theFormat.flags & FMT_MIX2))
          theFormat.spec_char[1] = '\0';
        Sprintf_tok(curScrat, (intmax_t) *ptr.w++);
        fprintw(fp, curScrat);
        if (j && (internalMode & 4))
          nextformat(NULL, 1);
      }
      break;
    case LUX_INT32:
      fmttok(fmt_integer);
      while (j--) {
        if (!j && (theFormat.flags & FMT_MIX2))
          theFormat.spec_char[1] = '\0';
        Sprintf_tok(curScrat, (intmax_t) *ptr.l++);
        fprintw(fp, curScrat);
        if (j && (internalMode & 4))
          nextformat(NULL, 1);
      }
      break;
    case LUX_INT64:
      fmttok(fmt_integer);
      while (j--) {
        if (!j && (theFormat.flags & FMT_MIX2))
          theFormat.spec_char[1] = '\0';
        Sprintf_tok(curScrat, (intmax_t) *ptr.q++);
        fprintw(fp, curScrat);
        if (j && (internalMode & 4))
          nextformat(NULL, 1);
      }
      break;
    case LUX_FLOAT:
      fmttok(fmt_float);
      while (j--) {
        if (!j && (theFormat.flags & FMT_MIX2))
          theFormat.spec_char[1] = '\0';
        Sprintf_tok(curScrat, (double) *ptr.f++);
        fprintw(fp, curScrat);
        if (j && (internalMode & 4))
          nextformat(NULL, 1);
      }
      break;
    case LUX_DOUBLE:
      fmttok(fmt_float);
      while (j--) {
        if (!j && (theFormat.flags & FMT_MIX2))
          theFormat.spec_char[1] = '\0';
        Sprintf_tok(curScrat, (double) *ptr.d++);
        fprintw(fp, curScrat);
        if (j && (internalMode & 4))
          nextformat(NULL, 1);
      }
      break;
    case LUX_STRING_ARRAY:
      fmttok(fmt_string);
      while (j--) {
        if (!j && (theFormat.flags & FMT_MIX2))
          theFormat.spec_char[1] = '\0';
        if (*ptr.sp)
          Sprintf_tok(curScrat, *ptr.sp);
        else
          *curScrat = '\0';
        ptr.sp++;
        fprintw(fp, curScrat);
        if (j && (internalMode & 4))
          nextformat(NULL, 1);
      }
      break;
    default:
      cerror(ILL_TYPE, symbol);
    }
    break;
  case LUX_SCAL_PTR:
    switch (scal_ptr_type(symbol)) {
    case LUX_INT8:
      intval = (intmax_t) *scal_ptr_pointer(symbol).b;
      break;
    case LUX_INT16:
      intval = (intmax_t) *scal_ptr_pointer(symbol).w;
      break;
    case LUX_INT32:
      intval = (intmax_t) *scal_ptr_pointer(symbol).l;
      break;
    case LUX_INT64:
      intval = (intmax_t) *scal_ptr_pointer(symbol).q;
      break;
    case LUX_FLOAT:
      fltval = (double) *scal_ptr_pointer(symbol).f;
      break;
    case LUX_DOUBLE:
      fltval = *scal_ptr_pointer(symbol).d;
      break;
    default:
      cerror(ILL_TYPE, symbol);
    }
    if (isIntegerType(scal_ptr_type(symbol))) { /* integer type */
      sprintf(curScrat, fmt_integer, intval);
      fprintw(fp, curScrat);
    } else if (scal_ptr_type(symbol) <= LUX_DOUBLE) { /* float type */
      Sprintf(curScrat, fmt_float, fltval);
      fprintw(fp, curScrat);
    }
    break;
  case LUX_CLIST: case LUX_PRE_CLIST: case LUX_CPLIST:
    n = clist_num_symbols(symbol);
    ptr.w = clist_symbols(symbol);
    while (n--) {
      type_ascii_one(*ptr.w++, fp);
      if (n && (internalMode & 4))
        nextformat(NULL, 1);
    }
    break;
  case LUX_LIST: case LUX_PRE_LIST:
    n = list_num_symbols(symbol);
    sptr = list_symbols(symbol);
    while (n--) {
      type_ascii_one(sptr->value, fp);
      sptr++;
      if (n && (internalMode & 4))
        nextformat(NULL, 1);
    }
    break;
  case LUX_CARRAY:
    ptr.cf = (floatComplex*) array_data(symbol);
    n = array_size(symbol);
    switch (array_type(symbol)) {
    case LUX_CFLOAT:
      while (n--) {
        Sprintf(curScrat, fmt_complex, (double) ptr.cf->real,
                (double) ptr.cf->imaginary);
        fprintw(fp, curScrat);
        ptr.cf++;
        if (n && (internalMode & 4))
          nextformat(NULL, 1);
      }
      break;
    case LUX_CDOUBLE:
      while (n--) {
        Sprintf(curScrat, fmt_complex, ptr.cd->real, ptr.cd->imaginary);
        fprintw(fp, curScrat);
        ptr.cd++;
        if (n && (internalMode & 4))
          nextformat(NULL, 1);
      }
      break;
    }
    break;
  default:
    symbolIdent(symbol, I_VALUE);
    fprintw(fp, curScrat);
  }
}
/*------------------------------------------------------------------------- */
char *nextformat(char *fmt, int32_t mode)
/* <fmt> non-NULL: install <fmt> as new format string. */
/* always: selects next format entry from the currently installed format */
/* string.  If <mode> is equal to FMT_INSTALL, then also install the */
/* current format entry in the appropriate !FMT_... variable. */
/* If <mode> is equal to FMT_CLEANUP, then restore the original contents */
/* of the modified !FMT_... variables. */
{
  static char	*saveI = NULL, *saveF = NULL, *saveC = NULL, *saveS = NULL;
  extern formatInfo	theFormat;
  extern char	*fmt_integer, *fmt_float, *fmt_complex, *fmt_string;
  char	install = 1;

  if (fmt) 			/* install new format */
    fmttok(fmt);
  else if (theFormat.count < 0)	/* get the next one */
    fmttok(NULL);
  else				/* just return the old one */
    install = 0;

  if (theFormat.count > 0)
    theFormat.count--;

  if (install && mode == FMT_INSTALL) {
    switch (theFormat.type) {
    case FMT_INTEGER:
      if (!saveI)
	saveI = fmt_integer;
      fmt_integer = theFormat.start;
      break;
    case FMT_FLOAT: case FMT_TIME: case FMT_DATE:
      if (!saveF)
	saveF = fmt_float;
      fmt_float = theFormat.start;
      break;
    case FMT_COMPLEX:
      if (!saveC)
	saveC = fmt_complex;
      fmt_complex = theFormat.start;
      break;
    case FMT_STRING:
      if (!saveS)
	saveS = fmt_string;
      fmt_string = theFormat.start;
      break;
    }
  } else if (mode == FMT_CLEANUP) {
    if (saveI) {
      fmt_integer = saveI;
      saveI = NULL;
    }
    if (saveF) {
      fmt_float = saveF;
      saveF = NULL;
    }
    if (saveS) {
      fmt_string = saveS;
      saveS = NULL;
    }
    if (saveC) {
      fmt_complex = saveC;
      saveC = NULL;
    }
  }
  return theFormat.start;
}
/*------------------------------------------------------------------------- */
int32_t type_ascii(int32_t narg, int32_t ps[], FILE *fp)
/* print the argument(s) in ASCII format to the file at <fp>.  If any
   one but the last argument starts with a C-style format (beginning
   with %) then such an argument is a format specification and the
   format(s) contained in it are installed one at a time to service
   the following arguments.

   if no explicit formats are included, then the standard formats from
   !FORMAT_I, !FORMAT_F, and !FORMAT_S are used.  The string contents
   of these are pointed at by <fmt_integer>, <fmt_float>, and
   <fmt_string>, respectively, and those pointers are used by
   symbolIdent() (in ident.c) to do the actual printing.  To install a
   format of our own, we must remember the original contents of the
   appropriate <fmt_...> and then have <fmt_...> temporarily point at
   the new format.  LS 11oct98
   */
/* Headers:
   <stdio.h>: fputs(), fputc(), stdout, fflush(), FILE
 */
{
  int32_t	i, iq, fmtsym;
  char	*p = NULL;
  char	*fmttok(char *);
  extern formatInfo	theFormat;

  theFormat.type = (fmtType) 0;
  theFormat.start = theFormat.next = NULL;
  for (i = 0; i < narg; i++) {
    iq = ps[i];
    if (symbol_class(iq) == LUX_SCAL_PTR)
      iq = dereferenceScalPointer(iq);
    if (iq < 0)
      return cerror(ILL_CLASS, 0);
    if (symbol_class(iq) == LUX_STRING /* it's a string */
	&& i < narg - 1) {	/* and not the last argument */
      fmtsym = iq;
      p = string_value(iq);
      if (*p == '%' && p[1] != '%' && p[1] != '\0') { /* a format string */
	nextformat(p, FMT_INSTALL);
	continue;
      }
    }

    if (p)
      p = NULL;
    else {
      nextformat(NULL, 1);
      if (theFormat.type == FMT_PLAIN) /* plain text from old format */
	i--;			/* don't advance argument pointer: we're */
				/* just going to print the plain text and */
				/* won't service the next argument yet. */
				/* LS 25may99 */
    }

    if (theFormat.type == FMT_PLAIN) {
      fputs(theFormat.start, fp);
      continue;
    }
    type_ascii_one(iq, fp);
  }

  /* check out any remaining format entries */
  while (nextformat(NULL, 0)) {
    switch (theFormat.type) {
      case FMT_PLAIN:
	if (fp == stdout)
	  printw(theFormat.start);
	else
	  fputs(theFormat.start, fp);
	break;
      case FMT_ERROR:
	return luxerror("Illegal format", fmtsym);
      default:
	return luxerror("Format expects more arguments", fmtsym);
    }
  }
  if ((internalMode & 1) == 0) { /* no /JOIN */
    fputc('\n', fp);
    column = 0;
  }
  if (fp == stdout)
    fflush(fp);

  nextformat(NULL, FMT_CLEANUP);
  return LUX_OK;
}
/*------------------------------------------------------------------------- */
int32_t type_formatted_ascii(int32_t narg, int32_t ps[], FILE *fp)
/* print using a user-supplied format string.  Arguments are cast to
   the expected type.  If <fp> is equal to NULL, then prints into a
   string at <curScrat>. 13oct98 */
/* Headers:
   <stdio.h>: FILE, fputs(), fprintf(), sprintf(), fflush()
   <string.h>: strcpy(), strlen(), strpbrk(), memcpy()
   <stdlib.h>: free()
 */
{
  char	*fmt, *thefmt, *save, haveTrailer, dofreefmt, *newfmt, *ptr;
  int32_t	iq, n, nn, iq0;
  Pointer	p;
  extern formatInfo	theFormat;
  int32_t	Sprintf(char *, char *, ...);

  iq = iq0 = ps[0];		/* the format symbol */
  if (symbol_class(iq0) == LUX_SCAL_PTR)
    iq = dereferenceScalPointer(iq0);
  if (symbol_class(iq) != LUX_STRING)
    return cerror(NEED_STR, iq0);
  if (narg == 1) {		/* only one symbol: just print it */
    if (fp)
      fputs(string_value(iq), fp);
    else			/* into curScrat */
      strcpy(curScrat, string_value(iq));
    return LUX_OK;
  }

  thefmt = fmt = fmttok(string_value(iq)); /* install format */
  if (!fmt)
    return luxerror("Illegal format string", iq0);
  dofreefmt = 0;

  if (!fp)
    save = curScrat;

  narg--;			/* number of arguments after format */
  while (narg || theFormat.type == FMT_PLAIN) {
    if (theFormat.type == FMT_PLAIN) { /* literal string */
      if (fp) {
	fputs(thefmt, fp);
      } else {
	strcpy(curScrat, thefmt);
	curScrat += strlen(curScrat);
      }
      if (fmt) {
	fmt = fmttok(NULL);	/* next format */
	dofreefmt = 0;
	if (fmt)
	  thefmt = fmt;
	else if (theFormat.type == FMT_ERROR) {
	  luxerror("Illegal format string", iq0);
	  goto lux_type_ascii2;
	} else if (!thefmt)	/* no more format strings but still more */
				/* arguments: re-use the last one, if */
				/* present */
	  goto lux_type_ascii1;
      }
    } else {
      iq = *++ps;
      narg--;
      switch (symbol_class(iq)) {
	case LUX_SCAL_PTR:
	  iq = dereferenceScalPointer(iq);
	case LUX_SCALAR:
	  n = 1;
	  p.l = &scalar_value(iq).l;
	  break;
	case LUX_ARRAY:
	  n = array_size(iq);
	  p.l = (int32_t*) array_data(iq);
	  break;
	case LUX_CSCALAR:
	  n = 1;
	  p.cf = complex_scalar_data(iq).cf;
	  break;
	case LUX_STRING:
	  n = 1;
	  p.s = string_value(iq);
	  break;
	default:
	  cerror(ILL_CLASS, *ps);
	  goto lux_type_ascii2;
      }

      if (internalMode & 1) {
	nn = n;
	n = 1;
      } else
	nn = 1;
      /* if the format has a _ or = in it, then we must use a copy without
	 the _ or =, because C output routines cannot deal with them. */
      if (theFormat.flags & (FMT_MIX | FMT_MIX2)) { /* have a _ */
	newfmt = strsave(thefmt);
	ptr = strpbrk(newfmt, "_="); /* find the _ or = */
	memcpy(ptr, ptr + 1, strlen(ptr)); /* move the rest back by one */
	if (theFormat.flags & FMT_MIX2) {
	  /* we have a =, so we must print the plain text for all elements */
	  /* except the last one */
	  newfmt[(theFormat.plain-thefmt) - 1] = '\0';/* cut off plain text */
	}
	thefmt = newfmt;
	dofreefmt = 1;
      }
      /* if theFormat.plain == theFormat.spec_char + 1, then the trailer
	 directly follows the format % specification, i.e., there is no repeat
	 count in the format entry.  In that case, there is no terminating
	 \0 plugged in just after the % specification, and no need for us
	 to print the trailer separately.  LS 19jan99 */
      haveTrailer = ((theFormat.plain > theFormat.spec_char + 1)
		     || (theFormat.flags & FMT_MIX2));
      while (nn--) {
	switch (theFormat.type) {
	  case FMT_INTEGER:
	    switch (symbol_type(iq)) {
	      case LUX_INT8:
		if (fp)
		  while (n--) {
		    fprintf(fp, thefmt, (int32_t) *p.b++);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2)))
		      fputs(theFormat.plain, fp);
		  }
		else
		  while (n--) {
		    sprintf(curScrat, thefmt, (int32_t) *p.b++);
		    curScrat += strlen(curScrat);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		break;
	      case LUX_INT16:
		if (fp)
		  while (n--) {
		    fprintf(fp, thefmt, (int32_t) *p.w++);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2)))
		      fputs(theFormat.plain, fp);
		  }
		else
		  while (n--) {
		    sprintf(curScrat, thefmt, (int32_t) *p.w++);
		    curScrat += strlen(curScrat);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		break;
	      case LUX_INT32:
		if (fp)
		  while (n--) {
		    fprintf(fp, thefmt, (int32_t) *p.l++);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2)))
		      fputs(theFormat.plain, fp);
		  }
		else
		  while (n--) {
		    sprintf(curScrat, thefmt, (int32_t) *p.l++);
		    curScrat += strlen(curScrat);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		break;
	      case LUX_INT64:
		if (fp)
		  while (n--) {
		    fprintf(fp, thefmt, *p.q++);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2)))
		      fputs(theFormat.plain, fp);
		  }
		else
		  while (n--) {
		    sprintf(curScrat, thefmt, *p.q++);
		    curScrat += strlen(curScrat);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		break;
	      case LUX_FLOAT:
		if (fp)
		  while (n--) {
		    fprintf(fp, thefmt, (int32_t) *p.f++);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2)))
		      fputs(theFormat.plain, fp);
		  }
		else
		  while (n--) {
		    sprintf(curScrat, thefmt, (int32_t) *p.f++);
		    curScrat += strlen(curScrat);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		break;
	      case LUX_DOUBLE:
		if (fp)
		  while (n--) {
		    fprintf(fp, thefmt, (int32_t) *p.d++);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2)))
		      fputs(theFormat.plain, fp);
		  }
		else
		  while (n--) {
		    sprintf(curScrat, thefmt, (int32_t) *p.d++);
		    curScrat += strlen(curScrat);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		break;
	      case LUX_CFLOAT:
		if (fp)
		  while (n--) {
		    fprintf(fp, thefmt, (int32_t) p.cf->real);
		    p.cf++;
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2)))
		      fputs(theFormat.plain, fp);
		  }
		else
		  while (n--) {
		    sprintf(curScrat, thefmt, (int32_t) p.cf->real);
		    p.cf++;
		    curScrat += strlen(curScrat);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		break;
	      case LUX_CDOUBLE:
		if (fp)
		  while (n--) {
		    fprintf(fp, thefmt, (int32_t) p.cd->real);
		    p.cd++;
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2)))
		      fputs(theFormat.plain, fp);
		  }
		else
		  while (n--) {
		    sprintf(curScrat, thefmt, p.cd->real);
		    p.cd++;
		    curScrat += strlen(curScrat);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		break;
	      case LUX_STRING_ARRAY: case LUX_TEMP_STRING:
	      case LUX_LSTRING:
		/* string arguments with numerical formats ->
		   don't print anything */
		break;
	    }
	    break;
	  case FMT_FLOAT:
	    switch (symbol_type(iq)) {
	      case LUX_INT8:
		if (fp)
		  while (n--) {
		    fprintf(fp, thefmt, (double) *p.b++);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2)))
		      fputs(theFormat.plain, fp);
		  }
		else
		  while (n--) {
		    sprintf(curScrat, thefmt, (double) *p.b++);
		    curScrat += strlen(curScrat);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		break;
	      case LUX_INT16:
		if (fp)
		  while (n--) {
		    fprintf(fp, thefmt, (double) *p.w++);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2)))
		      fputs(theFormat.plain, fp);
		  }
		else
		  while (n--) {
		    sprintf(curScrat, thefmt, (double) *p.w++);
		    curScrat += strlen(curScrat);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		break;
	      case LUX_INT32:
		if (fp)
		  while (n--) {
		    fprintf(fp, thefmt, (double) *p.l++);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2)))
		      fputs(theFormat.plain, fp);
		  }
		else
		  while (n--) {
		    sprintf(curScrat, thefmt, (double) *p.l++);
		    curScrat += strlen(curScrat);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		break;
	      case LUX_INT64:
		if (fp)
		  while (n--) {
		    fprintf(fp, thefmt, (double) *p.q++);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2)))
		      fputs(theFormat.plain, fp);
		  }
		else
		  while (n--) {
		    sprintf(curScrat, thefmt, (double) *p.q++);
		    curScrat += strlen(curScrat);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		break;
	      case LUX_FLOAT:
		if (fp)
		  while (n--) {
		    fprintf(fp, thefmt, (double) *p.f++);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2)))
		      fputs(theFormat.plain, fp);
		  }
		else
		  while (n--) {
		    sprintf(curScrat, thefmt, (double) *p.f++);
		    curScrat += strlen(curScrat);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		break;
	      case LUX_DOUBLE:
		if (fp)
		  while (n--) {
		    fprintf(fp, thefmt, (double) *p.d++);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2)))
		      fputs(theFormat.plain, fp);
		  }
		else
		  while (n--) {
		    sprintf(curScrat, thefmt, (double) *p.d++);
		    curScrat += strlen(curScrat);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		break;
	      case LUX_CFLOAT:
		if (fp)
		  while (n--) {
		    fprintf(fp, thefmt, (double) p.cf->real);
		    p.cf++;
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2)))
		      fputs(theFormat.plain, fp);
		  }
		else
		  while (n--) {
		    sprintf(curScrat, thefmt, (double) p.cf->real);
		    p.cf++;
		    curScrat += strlen(curScrat);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		break;
	      case LUX_CDOUBLE:
		if (fp)
		  while (n--) {
		    fprintf(fp, thefmt, (double) p.cd->real);
		    p.cd++;
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2)))
		      fputs(theFormat.plain, fp);
		  }
		else
		  while (n--) {
		    sprintf(curScrat, thefmt, p.cd->real);
		    p.cd++;
		    curScrat += strlen(curScrat);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		break;
	      case LUX_STRING_ARRAY: case LUX_TEMP_STRING:
	      case LUX_LSTRING:
		/* don't print strings with numerical formats */
		break;
	    }
	    break;
	  case FMT_TIME: case FMT_DATE:
	    switch (symbol_type(iq)) {
	      case LUX_INT8:
		while (n--) {
		  Sprintf(curScrat, thefmt, (double) *p.b++);
		  if (fp)
		    fputs(curScrat, fp);
		  else
		    curScrat += strlen(curScrat);
		  if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		    if (fp)
		      fputs(theFormat.plain, fp);
		    else {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		}
		break;
	      case LUX_INT16:
		while (n--) {
		  Sprintf(curScrat, thefmt, (double) *p.w++);
		  if (fp)
		    fputs(curScrat, fp);
		  else
		    curScrat += strlen(curScrat);
		  if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		    if (fp)
		      fputs(theFormat.plain, fp);
		    else {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		}
		break;
	      case LUX_INT32:
		while (n--) {
		  Sprintf(curScrat, thefmt, (double) *p.l++);
		  if (fp)
		    fputs(curScrat, fp);
		  else
		    curScrat += strlen(curScrat);
		  if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		    if (fp)
		      fputs(theFormat.plain, fp);
		    else {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		}
		break;
	      case LUX_INT64:
		while (n--) {
		  Sprintf(curScrat, thefmt, (double) *p.q++);
		  if (fp)
		    fputs(curScrat, fp);
		  else
		    curScrat += strlen(curScrat);
		  if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		    if (fp)
		      fputs(theFormat.plain, fp);
		    else {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		}
		break;
	      case LUX_FLOAT:
		while (n--) {
		  Sprintf(curScrat, thefmt, (double) *p.f++);
		  if (fp)
		    fputs(curScrat, fp);
		  else
		    curScrat += strlen(curScrat);
		  if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		    if (fp)
		      fputs(theFormat.plain, fp);
		    else {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		}
		break;
	      case LUX_DOUBLE:
		while (n--) {
		  Sprintf(curScrat, thefmt, (double) *p.d++);
		  if (fp)
		    fputs(curScrat, fp);
		  else
		    curScrat += strlen(curScrat);
		  if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		    if (fp)
		      fputs(theFormat.plain, fp);
		    else {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		}
		break;
	    }
	    break;
	  case FMT_COMPLEX:
	    switch (symbol_type(iq)) {
	      case LUX_INT8:
		while (n--) {
		  Sprintf(curScrat, thefmt, (double) *p.b++, (double) 0.0);
		  if (fp)
		    fputs(curScrat, fp);
		  else
		    curScrat += strlen(curScrat);
		  if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		    if (fp)
		      fputs(theFormat.plain, fp);
		    else {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		}
		break;
	      case LUX_INT16:
		while (n--) {
		  Sprintf(curScrat, thefmt, (double) *p.w++, (double) 0.0);
		  if (fp)
		    fputs(curScrat, fp);
		  else
		    curScrat += strlen(curScrat);
		  if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		    if (fp)
		      fputs(theFormat.plain, fp);
		    else {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		}
		break;
	      case LUX_INT32:
		while (n--) {
		  Sprintf(curScrat, thefmt, (double) *p.l++, (double) 0.0);
		  if (fp)
		    fputs(curScrat, fp);
		  else
		    curScrat += strlen(curScrat);
		  if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		    if (fp)
		      fputs(theFormat.plain, fp);
		    else {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		}
		break;
	      case LUX_INT64:
		while (n--) {
		  Sprintf(curScrat, thefmt, (double) *p.q++, (double) 0.0);
		  if (fp)
		    fputs(curScrat, fp);
		  else
		    curScrat += strlen(curScrat);
		  if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		    if (fp)
		      fputs(theFormat.plain, fp);
		    else {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		}
		break;
	      case LUX_FLOAT:
		while (n--) {
		  Sprintf(curScrat, thefmt, (double) *p.f++, (double) 0.0);
		  if (fp)
		    fputs(curScrat, fp);
		  else
		    curScrat += strlen(curScrat);
		  if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		    if (fp)
		      fputs(theFormat.plain, fp);
		    else {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		}
		break;
	      case LUX_DOUBLE:
		while (n--) {
		  Sprintf(curScrat, thefmt, (double) *p.d++, (double) 0.0);
		  if (fp)
		    fputs(curScrat, fp);
		  else
		    curScrat += strlen(curScrat);
		  if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		    if (fp)
		      fputs(theFormat.plain, fp);
		    else {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		}
		break;
	      case LUX_CFLOAT:
		while (n--) {
		  Sprintf(curScrat, thefmt, (double) p.cf->real,
			  (double) p.cf->imaginary);
		  p.cf++;
		  if (fp)
		    fputs(curScrat, fp);
		  else
		    curScrat += strlen(curScrat);
		  if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		    if (fp)
		      fputs(theFormat.plain, fp);
		    else {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		}
		break;
	      case LUX_CDOUBLE:
		while (n--) {
		  Sprintf(curScrat, thefmt, p.cd->real, p.cd->imaginary);
		  p.cd++;
		  if (fp)
		    fputs(curScrat, fp);
		  else
		    curScrat += strlen(curScrat);
		  if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		    if (fp)
		      fputs(theFormat.plain, fp);
		    else {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		}
		break;
	      case LUX_STRING_ARRAY: case LUX_TEMP_STRING:
	      case LUX_LSTRING:
		/* don't print strings with numerical arguments */
		break;
	    }
	    break;
	  case FMT_STRING:
	    switch (symbol_type(iq)) {
	      case LUX_STRING_ARRAY:
		if (fp)
		  while (n--) {
		    fprintf(fp, thefmt, *p.sp++);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2)))
		      fputs(theFormat.plain, fp);
		  }
		else
		  while (n--) {
		    sprintf(curScrat, thefmt, *p.sp++);
		    curScrat += strlen(curScrat);
		    if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		      strcpy(curScrat, theFormat.plain);
		      curScrat += strlen(curScrat);
		    }
		  }
		break;
	      case LUX_TEMP_STRING: case LUX_LSTRING:
		if (fp) {
		  fprintf(fp, thefmt, p.s);
		  if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2)))
		    fputs(theFormat.plain, fp);
		} else {
		  sprintf(curScrat, thefmt, p.s);
		  curScrat += strlen(curScrat);
		  if (haveTrailer && (n || !(theFormat.flags & FMT_MIX2))) {
		    strcpy(curScrat, theFormat.plain);
		    curScrat += strlen(curScrat);
		  }
		}
		break;
	    }
	    break;
	} /* end of switch (theFormat.type) */
	theFormat.count--;
	if (theFormat.count <= 0) {
	  if (dofreefmt)
	    free(thefmt);
	  if (theFormat.save1) { /* have more */
	    fmt = fmttok(NULL);	/* next format */
	    if (fmt)
	      thefmt = fmt;
	  } /* else we re-use the last one */
	  dofreefmt = 0;
	}
	n = 1;
      }	/* end of while (nn--) */
    } /* end of if (theFormat.type == LUX_UNUSED) */
  } /* end of if (narg || theFormat.type == LUX_UNUSED) */

  lux_type_ascii1:
  if (fp == stdout)
    fflush(stdout);
  else if (!fp)
    curScrat = save;
  return LUX_OK;
  lux_type_ascii2:
  if (!fp)
    curScrat = save;
  return LUX_ERROR;
}
/*------------------------------------------------------------------------- */
int32_t lux_fprint(int32_t narg, int32_t ps[])
 /* (stdout) printing */
/* Headers:
   <stdio.h>: FILE, stdout
 */
{
 FILE	*fp;

 fp = stdout;
 return	type_formatted_ascii(narg, ps, fp);
}
/*------------------------------------------------------------------------- */
int32_t lux_fprintf(int32_t narg, int32_t ps[])
 /* file (stdout) formatting and printing */
/* Headers:
   <stdio.h>: FILE
 */
{
 int32_t	lun;
 FILE	*fp;

 lun = int_arg( ps[0] );
 if (lun < 0 || lun >= MAXFILES)
   return cerror(ILL_LUN, *ps);
 if (lux_file_open[lun] == 0)
   return cerror(LUN_CLOSED, *ps);
 if (lux_file_open[lun] != 2)
   return cerror(READ_ONLY, *ps);
 fp = lux_file[lun];
 return	type_formatted_ascii(narg - 1, &ps[1], fp);
}
/*------------------------------------------------------------------------- */
int32_t lux_close(int32_t narg, int32_t ps[]) /* close subroutine */		
/* Headers:
   <stdio.h>: fclose()
   <stdlib.h>: free()
 */
{
  int32_t	lun;

  lun = int_arg(ps[0]);
  if (lun < 0 || lun >= MAXFILES)
    return cerror(ILL_LUN, *ps);
  if (lux_file_open[lun]) {
    fclose(lux_file[lun]);
    lux_file[lun] = NULL;
    lux_file_open[lun] = 0;
    free(lux_file_name[lun]);
  }
  return 1;
}
/*------------------------------------------------------------------------- */
int32_t open_file(int32_t narg, int32_t ps[], char const* access, char function)
 /* generic file opening routine, called by OPENR, OPENW, OPENU routines
    and functions   LS 8jul92
    openu must open a file for reading and writing anywhere in the file.
    add switch /get_lun to find and use the next available lun  LS 1feb95 */
/* Headers:
   <stdio.h>: FILE, fopen(), fseek(), printf()
   <string.h>: strcmp()
 */
{
 FILE	*fp;
 int32_t	lun, result_sym;
 char	*name;

 if (function) {		/* function call */
   result_sym = scalar_scratch(LUX_INT32);
   scalar_value(result_sym).l = 0; /* default: error */
 }
 if (internalMode & 1) {	/* /GET_LUN */
   for (lun = 0; lun < MAXFILES; lun++)
     if (!lux_file_open[lun])
       break;
   if (lun == MAXFILES)
     return luxerror("No logical unit free", 0);
   undefine(*ps);		/* clear old value of lun and put in new */
   symbol_class(*ps) = LUX_SCALAR;
   scalar_type(*ps) = LUX_INT32;
   scalar_value(*ps).l = lun;
 } else {
   lun = int_arg(ps[0]);
   if (lun < 0 || lun >= MAXFILES)
     return (function)? result_sym: cerror(ILL_LUN, *ps);
   if (lux_file_open[lun])
     return (function? result_sym: cerror(USED_LUN, *ps));
 }
 /* file name is second, must be a string */
 if (symbol_class(ps[1]) != LUX_STRING)
   return (function)? result_sym: cerror(NEED_STR, ps[1]);
 name = string_value(ps[1]);
 if (!strcmp(access, "u")) {	/* open the file for update */
   /* fopen mode r+, which allows reading and writing anywhere in the file, */
   /* only works if the file already exists.  fopen mode a+ creates the */
   /* file if it doesn't exist, but forces writing at the end of the file */
   /* in SGI Irix (in contrast to DEC Ultrix, where writing is allowed */
   /* anywhere in that mode).  We want the file to be created if it doesn't */
   /* yet exist, and to allow reading and writing anywhere in the file. */
   /* The file pointer should be positioned at the end of the file.  This */
   /* mimics ordinary update (appending at the end of the file) and also */
   /* allows associated variables to write anywhere in the file. */
   fp = fopen(expand_name(name, NULL), "r+");
   if (!fp)
     fp = fopen(expname, "w+");
   if (!fp			/* could not open */
       || fseek(fp, 0, SEEK_END)) { /* or could not go to end of file */
     printf("%s: ", expname);
     return cerror(ERR_OPEN, 0);
   }
 } else
   fp = fopen(expand_name(name, NULL), access);
 if (!fp) {			/* could not open the file */
   if (function)
     return result_sym;
   else {
     printf("%s: ", expname);
     return cerror(ERR_OPEN, 0);
   }
 }
 lux_file[lun] = fp;
 lux_file_open[lun] = (*access == 'r')? 1: 2; /* read-only or read-write */
 lux_file_name[lun] = strsave(expname);
 if (function)
   scalar_value(result_sym).l = 1; /* success */
 return (function)? result_sym: 1;
}
/*------------------------------------------------------------------------- */
int32_t lux_openr_f(int32_t narg, int32_t ps[])/* openr subroutine */
 /* intended mainly for reading ASCII files in old LUX, but may be useful
 for streams in general for UNIX version */
 /* associates a file (or a stream) with a lun */
 /* function version supporting environment variables LS 7jul92 */
{
  return open_file(narg, ps, "r", 1);
}
/*------------------------------------------------------------------------- */
int32_t lux_openw_f(int32_t narg, int32_t ps[])/* openw subroutine */
 /* like openr but for (over)writing */
{
  return open_file(narg, ps, "w", 1);
}
/*------------------------------------------------------------------------- */
int32_t lux_openu_f(int32_t narg, int32_t ps[])/* openu subroutine */
/* like openr but for updating */
{
  return open_file(narg, ps, "u", 1);
}
/*------------------------------------------------------------------------- */
int32_t lux_openr(int32_t narg, int32_t ps[])/* openr subroutine */
/* intended mainly for reading ASCII files in old LUX, but may be useful
   for streams in general for UNIX version */
/* associates a file (or a stream) with a lun */
{
  return open_file(narg, ps, "r", 0);
}
/*------------------------------------------------------------------------- */
int32_t lux_openw(int32_t narg, int32_t ps[])/* openw subroutine */
/* like openr but for output */
{
  return open_file(narg, ps, "w", 0);
}
/*------------------------------------------------------------------------- */
int32_t lux_openu(int32_t narg, int32_t ps[])/* openu subroutine */
/* like openr but for updating */
{
  return open_file(narg, ps, "u", 0);
}
/*------------------------------------------------------------------------- */
int32_t lux_rewindf(int32_t narg, int32_t ps[]) /*rewind file subroutine */
/* Headers:
   <stdio.h>: fseek()
 */
{
  int32_t	lun;

  /* first arg. is the lun */
  lun = int_arg(ps[0]);
  if (lun < 0 || lun >= MAXFILES)
    return cerror(ILL_LUN, *ps);
  if (!lux_file_open[lun])
    return cerror(LUN_CLOSED, *ps);
  fseek(lux_file[lun], 0, SEEK_SET);	/* rewind it */
  return 1;
}
/*------------------------------------------------------------------------- */
int32_t lux_read(int32_t narg, int32_t ps[])
 /* keyboard (stdin) or file (batch) reading and formatting */
/* Headers:
   <stdio.h>: stdin
 */
{
 int32_t	read_ascii(int32_t, int32_t [], FILE *, int32_t);

 return read_ascii(narg, ps, batch? inputStream: stdin,
		   (internalMode & 1)? ASK_MORE: TRUNCATE_EOF);
}
/*------------------------------------------------------------------------- */
int32_t readf(int32_t narg, int32_t ps[], int32_t flag)
 /* file (stdin) reading and formatting */
/* Headers:
   <stdio.h>: FILE
 */
{
  int32_t	lun;
  FILE	*fp;

  lun = int_arg(ps[0]);
  if (lun < 0 || lun >= MAXFILES)
    return flag? cerror(ILL_LUN, *ps): 0;
  if (lux_file_open[lun] == 0)
    return flag? cerror(LUN_CLOSED, *ps): 0;
  fp = lux_file[lun];
  return read_ascii(narg - 1, &ps[1], fp, flag);
}
/*------------------------------------------------------------------------- */
int32_t lux_readf(int32_t narg, int32_t ps[])
/* file (stdin) reading and formatting */
{
  return readf(narg, ps, 1);
}
/*------------------------------------------------------------------------- */
int32_t lux_readf_f(int32_t narg, int32_t ps[])
/* a function version that returns 1 if read OK */
{
  return readf(narg, ps, 0)? LUX_ONE: LUX_ZERO;
}
/*------------------------------------------------------------------------- */
char *moreInput(FILE *fp, char *separators)
/* reads input from <fp>.  If the first read character is in <separators>, */
/* then characters are read and discarded until one is encountered that is */
/* not in <separators>; then characters are read into storage (at */
/* <curScrat>) until a character from <separators> is found; then */
/* the string at <curScrat> is terminated and <curScrat> is returned. */
/* LS 30mar99 */
/* An initial \n always terminates the input.  LS 12jan00 */
/* Headers:
   <stdio.h>: getc(), feof()
   <string.h>: strchr()
 */
{
  char	*p;
  int32_t	c;

  c = nextchar(fp); /* first one */
  if (feof(fp))
    return NULL;
  p = curScrat;
  if (c == '\n') {		/* begin with \n; empty line; terminate */
    *p = '\0';
    return curScrat;
  }
  if (strchr(separators, c))	/* it's a separator character */
    do
      c = nextchar(fp);		/* read more */
    while (strchr(separators, c)); /* until a non-separator is found */
  if (c == EOF) {		/* end of file */
    *p = '\0';			/* terminate */
    return curScrat;
  }

  do {
    *p++ = c;			/* store OK char */
    c = nextchar(fp);		/* get next one */
  } while (!strchr(separators, c) /* while not a separator */
	   && c != EOF);	/* and not at end of file */
  *p = '\0';			/* terminate */
  return curScrat;
}
/*------------------------------------------------------------------------- */
int32_t read_ascii(int32_t narg, int32_t ps[], FILE *fp, int32_t flag)
/* read ascii stream, format and load into passed arguments
   called by lux_read and lux_readf
   flag: 0 -> no error messages (function)
   internalMode & 2 -> /WORD read strings by words (whitespace-separated)
                       else read by line
   skip comments in files, i.e. if fp != stdin and a number is
   asked for but something else is found, then skip to the next
   item and try again  LS 26feb93 */
/* redesigned LS 20mar00 */
/* Headers:
   <stdio.h>: FILE, stdin, puts(), fgetc(), EOF, fgets(), feof()
   <ctype.h>: isspace()
   <string.h>: strncpy(), strlen()
   <stdlib.h>: realloc()
 */
{
  int32_t	i, iq, n, c, nelem;
  Symboltype type;
  char	*p;
  Scalar	value;
  Pointer	pp;

  index_cnt = 0;		/* keep track of the number of read elements */
  for (i = 0; i < narg; i++) {	/* loop over all arguments */
    iq = ps[i];			/* next argument */
    if (!symbolIsNamed(iq)) {	/* an expression of some sort; must be
				   a string, which we display if we're
				   reading from standard input */
      if (symbol_class(iq) == LUX_STRING) {
	if (fp == stdin)
	  puts(string_value(iq));
      } else
	return flag?
	  luxerror("Cannot read a value into an unnamed symbol", iq): 0;
    } else 			/* a named variable: we must read something */
      switch (symbol_class(iq)) {
      case LUX_STRING:	/* looking for a string */
	if (internalMode & 2)	{ /* /WORD: reading by Word */
	  n = 0;
	  p = curScrat;
	  n = scratSize();	/* how much room we have at curScrat */
	  /* first we skip any whitespace */
	  while ((c = nextchar(fp)) != EOF && isspace(c));
	  if (c == EOF) 	/* reached the end of the file */
	    return flag? luxerror("Encountered EOF", 0): 0;
	  /* the last one wasn't whitespace, so we push that one */
	  /* back */
	  unnextchar(c, fp);
	  /* then we read non-whitespace characters until the next
	     whitespace */
	  while (n && (c = nextchar(fp)) != EOF && !isspace(c)) {
	    *p++ = c;
	    n--;
	  }
	  if (!n)		/* we ran out of room */
	    puts("WARNING -- an input string was too large to handle.");
	  n = p - curScrat;
	  redef_string(iq, n);
	  strncpy(string_value(iq), curScrat, n);
	  string_value(iq)[n] = '\0';	/* terminate */
	} else {		/* reading by line */
	  if (!nextline(curScrat, scratSize(), fp)) /* some error or EOF */
	    return flag? luxerror("Encountered EOF", 0): 0;
	  /* if the line ends with a newline, then this is included */
	  /* in the read string; however, there need not be a newline */
	  /* in the input */
	  n = strlen(curScrat);
	  if (curScrat[n - 1] == '\n')
	    curScrat[--n] = '\0'; /* suppress final \n, if any */
	  redef_string(iq, n);
	  strncpy(string_value(iq), curScrat, n + 1);
	}
	index_cnt++;
	break;
      case LUX_SCALAR: case LUX_UNDEFINED: /* read scalar */
	if (fp != stdin && feof(fp))
	  return flag? luxerror("Encounterd EOF", 0): 0;
	read_a_number_fp(fp, &value, &type);
	index_cnt++;
	redef_scalar(iq, type, &value);
	switch (type) {
	  /* <iq> is returned as LUX_INT32, LUX_INT64, or LUX_DOUBLE
             by redef_scalar() */
	case LUX_INT8:
	  scalar_value(iq).b = (uint8_t) value.l;
	  break;
	case LUX_INT16:
	  scalar_value(iq).w = (int16_t) value.l;
	  break;
	case LUX_FLOAT:
	  scalar_value(iq).f = (float) value.d;
	  break;
	} /* end switch (type) */
	break;
      case LUX_SCAL_PTR:
	if (feof(fp))
	  return flag? luxerror("Encounterd EOF", 0): 0;
	read_a_number_fp(fp, &value, &type);
	index_cnt++;
	switch (type) {
	case LUX_INT8: case LUX_INT16: case LUX_INT32:
	  *scal_ptr_pointer(iq).l = value.l;
	  scal_ptr_type(iq) = LUX_INT32;
	  break;
	case LUX_INT64:
	  *scal_ptr_pointer(iq).q = value.q;
	  scal_ptr_type(iq) = LUX_INT64;
	  break;
	case LUX_FLOAT:
	  *scal_ptr_pointer(iq).f = (float) value.d;
	  scal_ptr_type(iq) = LUX_FLOAT;
	  break;
	case LUX_DOUBLE:
	  *scal_ptr_pointer(iq).d = value.d;
	  scal_ptr_type(iq) = LUX_DOUBLE;
	  break;
	} /* end switch (type) */
	break;
      case LUX_ARRAY:
	nelem = array_size(iq);
	pp.b = (uint8_t*) array_data(iq);
	if (isNumericalType(array_type(iq))) {
	  while (nelem--) {
	    /* for some strange reason feof(stdin) can return non-zero */
	    /* under ordinary circumstances, so we exclude it here */
	    if (fp != stdin && feof(fp))
	      return flag? luxerror("Encounterd EOF", 0): 0;
	    read_a_number_fp(fp, &value, &type);
	    index_cnt++;
	    switch (array_type(iq)) {
	    case LUX_INT8:
	      switch (type) {
	      case LUX_INT8: case LUX_INT16: case LUX_INT32:
		*pp.b++ = (uint8_t) value.l;
		break;
	      case LUX_INT64:
		*pp.b++ = value.q;
		break;
	      case LUX_FLOAT: case LUX_DOUBLE:
		*pp.b++ = (uint8_t) value.d;
		break;
	      }
	      break;
	    case LUX_INT16:
	      switch (type) {
	      case LUX_INT8: case LUX_INT16: case LUX_INT32:
		*pp.w++ = (int16_t) value.l;
		break;
	      case LUX_INT64:
		*pp.w++ = value.q;
		break;
	      case LUX_FLOAT: case LUX_DOUBLE:
		*pp.w++ = (int16_t) value.d;
		break;
	      }
	      break;
	    case LUX_INT32:
	      switch (type) {
	      case LUX_INT8: case LUX_INT16: case LUX_INT32:
		*pp.l++ = (int32_t) value.l;
		break;
	      case LUX_INT64:
		*pp.l++ = value.q;
		break;
	      case LUX_FLOAT: case LUX_DOUBLE:
		*pp.l++ = (int32_t) value.d;
		break;
	      }
	      break;
	    case LUX_INT64:
	      switch (type) {
	      case LUX_INT8: case LUX_INT16: case LUX_INT32:
		*pp.q++ = value.l;
		break;
	      case LUX_INT64:
		*pp.q++ = value.q;
		break;
	      case LUX_FLOAT: case LUX_DOUBLE:
		*pp.q++ = value.d;
		break;
	      }
	      break;
	    case LUX_FLOAT:
	      switch (type) {
	      case LUX_INT8: case LUX_INT16: case LUX_INT32:
		*pp.f++ = (float) value.l;
		break;
	      case LUX_INT64:
		*pp.f++ = value.q;
		break;
	      case LUX_FLOAT: case LUX_DOUBLE:
		*pp.f++ = (float) value.d;
		break;
	      }
	      break;
	    case LUX_DOUBLE:
	      switch (type) {
	      case LUX_INT8: case LUX_INT16: case LUX_INT32:
		*pp.d++ = (double) value.l;
		break;
	      case LUX_INT64:
		*pp.d++ = value.q;
		break;
	      case LUX_FLOAT: case LUX_DOUBLE:
		*pp.d++ = (double) value.d;
		break;
	      }
	      break;
	    }	/* end else switch (array_type(iq)) */
	  } /* end while (n--) */
	} else {		/* a string array */
	  if (internalMode & 2) { /* /WORD: reading by Word */
	    n = 0;
	    p = curScrat;
	    n = scratSize();	/* how much room we have at curScrat */
	    /* first we skip any whitespace */
	    while ((c = nextchar(fp)) != EOF && isspace(c));
	    if (c == EOF) 	/* reached the end of the file */
	      return flag? luxerror("Encountered EOF", 0): 0;
	    /* then we read non-whitespace characters until the next
	       whitespace */
	    while (n && (c = nextchar(fp)) != EOF && !isspace(c)) {
	      *p++ = c;
	      n--;
	    }
	    if (!n)		/* we ran out of room */
	      puts("WARNING -- an input string was too large to handle.");
	    n = p - curScrat;
	    *pp.sp = (char*) realloc(*pp.sp, n + 1);
	    strncpy(*pp.sp, curScrat, n);
	    (*pp.sp)[n] = '\0'; /* terminate */
	    pp.sp++;
	  } else {		/* reading by line */
	    if (!nextline(curScrat, scratSize(), fp))
	      return flag? luxerror("Encounterd EOF", 0): 0;
	    /* if the buffer was big enough then we read a newline and
	       must remove it.  If the buffer was too small, then there
	       won't be a newline */
	    n = strlen(curScrat);
	    if (curScrat[n - 1] == '\n')
	      curScrat[--n] = '\0'; /* suppress final \n, if any */
	    *pp.sp = (char*) realloc(*pp.sp, n + 1);
	    strncpy(*pp.sp, curScrat, n + 1);
	      pp.sp++;
	  }
	}
	break;
      default:
	return flag? cerror(ILL_CLASS, 0): 0;
      }
  }
  return LUX_OK;
}
/*------------------------------------------------------------------------- */
int32_t gscanf(void **source, char const* format, void *arg, int32_t isString)
/* Headers:
   <string.h>: strlen(), strcpy(), strcat()
   <stdlib.h>: realloc()
   <stdio.h>: sscanf(), fscanf()
 */
{
  static char	*aformat = NULL;
  static int32_t	nformat = 0;
  int32_t	i;

  i = strlen(format) + 2;
  if (i > nformat) {
    aformat = (char*) realloc(aformat, i);
    nformat = i;
    if (!aformat) {
      cerror(ALLOC_ERR, 0);
      return 0;
    }
  }
  strcpy(aformat, format);
  strcat(aformat, "%n");	/* we want to know how many chars we read;
				   not all versions of sscanf return this
				   number */

  i = 0;
  if (isString) {		/* reading from a string */
    if (!*((char **) source) || !**((char **) source)) /* empty string */
      return 0;
    if (arg) 			/* want to keep the argument */
      sscanf(*((char **) source), aformat, arg, &i);
    else
      sscanf(*((char **) source), aformat, &i);
    *(char **) source += i;	/* advance string pointer */
  } else {			/* reading from a file */
    if (feof(*((FILE **) source)))
      return 0;
    if (arg) 
      fscanf(*((FILE **) source), aformat, arg, &i);
    else
      fscanf(*((FILE **) source), aformat, &i);
  }
  return i;
}
/*------------------------------------------------------------------------- */
int32_t read_formatted_ascii(int32_t narg, int32_t ps[], void *ptr, int32_t showerrors,
			 int32_t isString)
/* read from ascii source <ptr> according to a specified format string,
   which is taken from ps[0].  If <isString> is equal to 0, then <ptr> is
   interpreted as a FILE pointer to a stream from which input is to be
   read.  If <isString> is unequal to 0, then <ptr> is interpreted as a
   char pointer to the start of a string from which input is to be
   read.  If an error occurs, then LUX_ERROR is returned.  Only if
   <showerrors> is unequal to zero are error messages displayed as well. */
/* the format string is as in C, but with the following exceptions: */
/* 1. C-formats %p and %n are not supported.
   2. LUX format %20S (not available in C) reads the next 20 characters
      up to a newline, whereas LUX and C format %s reads the next
      20 characters up to a whitespace.
   3. LUX formats %t and %T read numbers in sexagesimal (base-60) notation,
      for example hours-minutes-seconds.
   4. LUX format %z reads a complex number
   5. LUX postfix 6# indicates that the preceding format must be used
      6 times and the result stored in an array (if not suppressed). */
/* Headers:
   <stdio.h>: FILE, stdin, sprintf()
   <stdlib.h>: malloc(), realloc()
   <string.h>: strcpy(), strlen()
 */
{
  char	*fmt, *scr, *p, c, *ptr0;
  int32_t	string2, pr, i, len, nout = 0;
  Symboltype type;
  double	d, k, f, d2;
  Pointer	trgt;
  void	**ptr2;
  static char	*keyboard = NULL;
  extern formatInfo	theFormat;
  int32_t	lux_replace(int32_t, int32_t);

  if (!isString) {
    if ((FILE *) ptr == stdin) { /* reading from the keyboard */
      if (!keyboard) {
	keyboard = (char*) malloc(BUFSIZE);
	if (!keyboard)
	  return showerrors? cerror(ALLOC_ERR, 0): 0;
      }
      getNewLine(keyboard, BUFSIZE, "dat>", 0); /* read a line from the keyboard */
      ptr = keyboard;
      isString = 1;
    }
  }

  ptr0 = (char *) ptr;

  fmt = string_arg(*ps++);	/* format string */
  if (!fmt)
    return LUX_ERROR;		/* no format string */
  narg--;			/* number of target arguments */
  scr = curScrat;

  fmt = fmttok(fmt);		/* install format string and determine
				   its first token */
  if (!fmt)
    return luxerror("Illegal format string", ps[-1]);

  while (fmt) {
    /* The ANSI C manual (K&R 2nd edition) specifies that whitespace
     in scanf format strings is ignored, but at least the GNU C
     compiler gcc takes whitespace in format strings to match any
     amount of whitespace in the input.  This means that a format
     string ' %10c' would return the next 10 characters if ANSI C is
     followed, but the next 10 characters after initial whitespace if
     gcc is used.  We cannot use such confusion, so I skip initial
     whitespace in format entries.  There's a new non-C format %S that
     accepts whitespace characters (%s only accepts non-whitespace).  LS
     5may97 */
    if (narg < 1		/* no more arguments */
	&& ((theFormat.flags & FMT_SUPPRESS) == 0) /* and not suppressing */
	&& theFormat.type != FMT_PLAIN) { /* and not plain text */
      if (showerrors)
	return luxerror("Too few arguments to service all format string entries",
		     0);
      else
	return nout;
    }

    if (theFormat.type != FMT_PLAIN) {
      /* have some data to read */
      switch (theFormat.type) {
	case FMT_STRING:
	  type = LUX_TEMP_STRING;
	  break;
	default:
	  if (showerrors)
	    return luxerror("Illegal format %s", 0, fmt);
	  else
	    return nout;
	case FMT_INTEGER:
          if (theFormat.flags & FMT_SMALL)
            type = LUX_INT16;
          else if (theFormat.flags & FMT_BIGINT)
            type = LUX_INT64;
          else
            type = LUX_INT32;
	  break;
	case FMT_FLOAT: case FMT_TIME:
	  type = (theFormat.flags & FMT_BIG)? LUX_DOUBLE: LUX_FLOAT;
	  break;
	case FMT_COMPLEX:
	  type = (theFormat.flags & FMT_BIG)? LUX_CDOUBLE: LUX_CFLOAT;
	  break;
      }

      /* modify output symbol, if necessary */
      if (!(theFormat.flags & FMT_SUPPRESS)) {
	/* have some data to read */
	if (theFormat.count > 1) {		/* we need an array */
	  if (type == LUX_TEMP_STRING) {
	    if (showerrors)
	      return
		luxerror("Formatted read of string arrays not yet implemented!",
		      0);
	    else
	      return nout;
	  }
	  to_scratch_array(*ps, type, 1, &theFormat.count);
	  trgt.l = (int32_t*) array_data(*ps);
	} else {		/* only one item */
	  if (type == LUX_TEMP_STRING) {
	    undefine(*ps);
	    symbol_class(*ps) = LUX_STRING;
	    string_type(*ps) = LUX_TEMP_STRING;
	    symbol_memory(*ps) = 1;
	    string_value(*ps) = (char*) malloc(1);
	    trgt.s = string_value(*ps);
	  } else if (isComplexType(type)) {
	    undefine(*ps);
	    symbol_class(*ps) = LUX_CSCALAR;
	    complex_scalar_type(*ps) = type;
	    complex_scalar_memory(*ps) = lux_type_size[type];
	    complex_scalar_data(*ps).cf = (floatComplex*) malloc(complex_scalar_memory(*ps));
	    trgt.cf = complex_scalar_data(*ps).cf;
	  } else {
	    undefine(*ps);
	    symbol_class(*ps) = LUX_SCALAR;
	    scalar_type(*ps) = type;
	    trgt.l = &scalar_value(*ps).l;
	  }
	}
      }
    }

    if (theFormat.type != FMT_PLAIN)
      switch (type) {
	case LUX_TEMP_STRING:	/* a string */
	  if (theFormat.count > 1)
	    return showerrors? 
	      luxerror("Reading of string arrays is not yet implemented", 0):
		nout;
	  if (theFormat.flags & FMT_SUPPRESS) {
	    /* %*s or %*S or %*[...]: skip next whitespace-delimited Word */
	    /* (%*s) or to line end (%*S) or next scan set (%*[...]) */
	    if (*theFormat.spec_char == '[') {
	      if (theFormat.width > 0)
		sprintf(scr, "%%*%1d%s", theFormat.width, theFormat.start);
	      else
		strcpy(scr, theFormat.start);
	    } else {
	      if (theFormat.width > 0)
		sprintf(scr, (*theFormat.spec_char == 's')? "%%*%1d[^ \n]":
			"%%*%1d[^\n]", theFormat.width);
	      else
		strcpy(scr, (*theFormat.spec_char == 's')? "%*[^ \n]":
		       "%*[^\n]");
	    }
	    p = scr + strlen(scr) + 1; /* beyond format string */
	    if (!(len = gscanf(&ptr, scr, NULL, isString)))
	      return showerrors? cerror(READ_ERR, 0): nout;
	    if (*theFormat.spec_char == 'S' 
		&& len < theFormat.width /* not yet at field width */
		&& *(char *) ptr == '\n') /* and we have a \n */
	      ptr = (void *) ((char *) ptr + 1); /* skip the \n */
	  } else {		/* %s or %S or %[...]: read */
	    if (*theFormat.spec_char == '[') {
	      if (theFormat.width > 0)
		sprintf(scr, "%%%1d%s", theFormat.width, theFormat.start);
	      else
		strcpy(scr, theFormat.start);
	    } else {            /* %s or %S */
              strcpy(scr, theFormat.start);
	    }
	    p = scr + strlen(scr) + 1; /* beyond specification */
	    if (!(len = gscanf(&ptr, scr, p, isString))) /* read */
	      return showerrors? cerror(READ_ERR, 0): nout;
	    if (*theFormat.spec_char == 'S'
		&& len < theFormat.width /* not yet at field width */
		&& *(char *) ptr == '\n') /* and we have a \n */
	      ptr = (void *) ((char *) ptr + 1); /* skip the \n */

	    symbol_memory(*ps) = strlen(p) + 1;
	    trgt.s = (char*) realloc(trgt.s, symbol_memory(*ps));
	    if (!trgt.s)
	      return showerrors? cerror(ALLOC_ERR, 0): nout;
	    string_value(*ps) = trgt.s;
	    strcpy(trgt.s, p);
	  }
	  break;
	default:		/* something other than a string */
	  if (theFormat.count < 1) /* no explicit count */
	    theFormat.count = 1; /* so read 1 */
	  while (theFormat.count--) {
	    if (theFormat.width > 0 && (internalMode & 1)) {
	      /* in gcc at least, whitespace ordinarily does not count against
		 the field width for numerical arguments; the %5d format, for
		 example, reads up to 5 digits after first skipping any initial
		 whitespace.  This is not suitable for reading tables when some
		 table entries may be missing, so we offer /COUNTSPACES which,
		 when set, makes whitespace count against field width.
		 LS 19jan99 */
	      /* have an explicit width and must count whitespace */
	      /* first we read the number of chars indicated by the width */
	      sprintf(scr, "%%%1dc", theFormat.width);
	      p = scr + strlen(scr) + 1;
	      gscanf(&ptr, scr, p, isString);
	      p[theFormat.width] = '\0'; /* terminate */
	      /* and then we read values from the read string */
	      ptr2 = (void **) &p;
	      string2 = 1;
	    } else {		/* we convert directly from the source */
	      ptr2 = &ptr;
	      string2 = isString;
	    }
	    switch (theFormat.type) {
	      case FMT_TIME:	/* a time specification xx:yy:zz.zzz */
		pr = theFormat.precision;
		if (pr < 2)
		  pr = 2;
		d = 0.0;	/* will contain the read value */
		k = 1.0;	/* multiplication factor */
		while (pr--) {
		  if (pr) {	/* not the last one: read an integer */
		    if (!gscanf(ptr2, "%d", &i, string2))
		      return
			showerrors? luxerror("Expected a digit while looking for a time at character index %d in %s", 0, (char *) *ptr2 - ptr0, ptr0): nout;
		    d += i*k;
		    if (i < 0)
		      k = -k;
		    k /= 60.0;
		    /* skip the separator: anything except digits */
		    if (!gscanf(ptr2, "%*[^0-9]", NULL, string2))
		      return
			showerrors? luxerror("Expected a non-digit while looking for a time at character index %d in %s", 0, (char *) *ptr2 - ptr0, ptr0): nout;
		  } else {	/* the last one: read a float */
		    if (!gscanf(ptr2, "%lf", &f, string2))
		      return
			showerrors? luxerror("Expected a (floating-point) number while looking for a time at character index %d in %s", 0, (char *) *ptr2 - ptr0, ptr0): nout;
		    d += f*k;
		  }
		}
		
		if (theFormat.flags & FMT_ALTERNATIVE)
		  d *= 15;	/* from hours to degrees */
		
		if (!(theFormat.flags & FMT_SUPPRESS)) {
		  if (type == LUX_FLOAT)
		    *trgt.f = (float) d;
		  else
		    *trgt.d = d;
		}
		break;
	      case FMT_COMPLEX:
		/* we expect numbers in the form  aaa.bbb+ccc.dddi */
		/* NOTE: sscanf (and gscanf) do not like whitespace
		   between a sign and the rest of the number! */
		gscanf(ptr2, "%lf", &d, string2);
		gscanf(ptr2, "%lf", &d2, string2);
		if (!gscanf(ptr2, "%*1[iI]", NULL, string2)) {
		  if (internalMode & 1)	/* /COUNTSPACES: the number is
					 absent so we substitute 0 */
		    d = d2 = 0.0;
		  else
		    return showerrors? luxerror("Unexpected input", 0): nout;
		}
		if (!(theFormat.flags & FMT_SUPPRESS)) {
		  if (type == LUX_CFLOAT) {
		    trgt.cf->real = d;
		    trgt.cf->imaginary = d2;
		  } else {
		    trgt.cd->real = d;
		    trgt.cd->imaginary = d2;
		  }
		}
		break;
	      default:		/* regular numbers */
		if (!gscanf(ptr2, theFormat.current,
			    (theFormat.flags & FMT_SUPPRESS)? NULL: trgt.b,
			    string2))
		  return showerrors? luxerror("Expected a number at character index %d in %s", 0, (char *) *ptr2 - ptr0, ptr0): nout;
		break;
	    } /* end of switch (type) */
	    trgt.b += lux_type_size[type];
	  } /* end of while (theFormat.count--) */
	  break;
      } /* end of switch (type) */
    
    if (theFormat.type == FMT_PLAIN || theFormat.end > theFormat.plain) {
      c = *theFormat.end;
      *theFormat.end = '\0';	/* temporary end */
      if (!gscanf(&ptr, theFormat.plain, NULL, isString)
	  && !theFormat.only_whitespace) {
	/* text did not match */
	if (showerrors)
	  return luxerror("Input did not match explicit text '%s'", 0,
		       theFormat.plain);
	else
	  return nout;
      }
      *theFormat.end = c;		/* restore */
    }

    if (theFormat.type != FMT_PLAIN && !(theFormat.flags & FMT_SUPPRESS)) {
      ps++;
      narg--;
      nout++;
    }
    fmt = fmttok(NULL);
  }
  return nout;
}
/*------------------------------------------------------------------------- */
int32_t lux_freadf(int32_t narg, int32_t ps[])
/* FREADF,lun,format,var,... */
/* Headers:
   <stdio.h>: FILE
*/
{
  int32_t	lun;
  FILE	*fp;

  lun = int_arg(*ps);
  if (lun < 0 || lun >= MAXFILES)
    return cerror(ILL_LUN, *ps);
  if (lux_file_open[lun] == 0)
    return cerror(LUN_CLOSED, *ps);
  if (lux_file_open[lun] != 1)
    return cerror(WRITE_ONLY, *ps);
  fp = lux_file[lun];
  if (read_formatted_ascii(narg - 1, &ps[1], (void *) fp, 1, 0) == LUX_ERROR)
    return luxerror("Error reading file %s", 0, lux_file_name[lun]);
  else 
    return LUX_OK;
}
/*------------------------------------------------------------------------- */
int32_t lux_fread(int32_t narg, int32_t ps[])
/* FREAD,format,var,... */
/* Headers:
   <stdio.h>: stdin
 */
{
  int32_t	result, iq;

  result = read_formatted_ascii(narg, ps, (void *) stdin, 1, 0);
  switch (result) {
    case LUX_ERROR: case 0:
      luxerror("Error reading from standard input", 0);
      return LUX_ZERO;
    case 1:
      return LUX_ONE;
    default:
      iq = scalar_scratch(LUX_INT32);
      scalar_value(iq).l = result;
      break;
  }
  return iq;
}
/*------------------------------------------------------------------------- */
int32_t lux_freadf_f(int32_t narg, int32_t ps[])
/* FREADF(lun,format,var,...) */
/* Headers:
   <stdio.h>: FILE
 */
{
  int32_t	lun, result, iq;
  FILE	*fp;

  lun = int_arg(*ps);
  if (lun < 0 || lun >= MAXFILES)
    return cerror(ILL_LUN, *ps);
  if (lux_file_open[lun] == 0)
    return cerror(LUN_CLOSED, *ps);
  if (lux_file_open[lun] != 1)
    return cerror(WRITE_ONLY, *ps);
  fp = lux_file[lun];
  result = read_formatted_ascii(narg - 1, &ps[1], (void *) fp, 0, 0);
  switch (result) {
    case LUX_ERROR: case 0:
      luxerror("Error reading from file %s", 0, lux_file_name[lun]);
      return LUX_ZERO;
    case 1:
      return LUX_ONE;
    default:
      iq = scalar_scratch(LUX_INT32);
      scalar_value(iq).l = result;
      break;
  }
  return iq;
}
/*------------------------------------------------------------------------- */
int32_t lux_assoc_input(int32_t narg, int32_t ps[])
     /* use an associated variable as a guide to reading a file 
	syntax:  y = x(record_offset)
	y = x(subsc)
	x is an associated variable.
	<record_offset> = # records from file start
	<subsc> = subcripts as for arrays  (as yet, + entries are disregarded)
	You may have one more subscripts than the number of dimensions
	in the assoc definition.
	Subscript case added by LS 30nov92 */
/* the associated variable is assumed to be in ps[narg]; narg is the number
 of subscripts.  LS 19aug98 */
/* Headers:
   <stdio.h>: FILE, puts(), printf(), fseek(), fread()
 */
{
  int32_t	iq, dfile, *file, dout = 0, lun, i, offset;
  int32_t	tally[8], ystep[8], rstep[8], *step, n, done, doff;
  int32_t	off[8], outdims[8], axes[8], *out;
  int32_t	len, nsym, baseOffset;
  Symboltype type;
  Pointer	q;
  array	*h;
  FILE	*fp;
  char	warn = 0;
  extern int32_t	range_warn_flag;

  step = rstep;
  out = outdims;
  offset = 0;
  nsym = iq = ps[narg];		/* assoc var */
  if (symbol_class(iq) != LUX_ASSOC)
    return cerror(ILL_CLASS, iq);
  lun = assoc_lun(iq);
  if (lun < 0 || lun >= MAXFILES)
    return cerror(ILL_LUN, iq);
  if (lux_file_open[lun] == 0)
    return cerror(LUN_CLOSED, iq);
  fp = lux_file[lun];
  type = assoc_type(iq);
  dfile = assoc_num_dims(iq);	/* #dims in file */
  file = assoc_dims(iq);	/* file dims */
  baseOffset = assoc_offset(iq); /* offset */
  doff = dfile;
  if (narg > dfile + 2)		/* too many subscripts */
    return cerror(ILL_N_SUBSC, 0);
  else if (narg == 1) {		/* one subscript */
    for (i = 0; i < dfile; i++) {
      off[i] = 0;		/* zero offsets */
      outdims[i] = file[i];	/* whole coord range */
      axes[i] = i; 		/* no redirection */
    }
    dout = dfile;
    off[i] = int_arg(ps[0]);
    axes[i] = i;
    doff++;
  } else {
    for (i = 0; i < dfile; i++) { /* other args */
      iq = ps[i];		/* next argument */
      switch (symbol_class(iq)) {
	case LUX_RANGE:		/* a range subscript */
	  iq = convertRange(iq);
	case LUX_SUBSC_PTR:	/* a complex subscript */
	  off[i] = subsc_ptr_start(iq);
	  if (off[i] >= file[i]) {
	    off[i] = file[i] - 1;
	    warn = 1;
	  } else if (off[i] < 0) {
	    off[i] = 0;
	    warn = 1;
	  }
	  len = subsc_ptr_end(iq); /* end */
	  if (len == INT32_MAX)
	    len = file[i] - 1;	/* * -> until end */
	  else if (len >= file[i]) {
	    len = file[i] - 1;
	    warn = 1;
	  } else if (len < off[i]) {
	    len = off[i];
	    warn = 1;
	  }
	  len = len - off[i] + 1;
	  outdims[dout] = len;	/* length */
	  if (subsc_ptr_sum(iq)) {
	    puts("+ subscript operator not implemented for associated variables");
	    return cerror(ILL_SUBSC_TYPE, iq);
	  }
	  axes[dout] = subsc_ptr_redirect(iq);
	  if (axes[dout] == -1)
	    axes[dout] = i;	/* acessed axes */
	  dout++;
	  break;
	case LUX_SCALAR: case LUX_SCAL_PTR:
	  off[i] = int_arg(iq);
	  if (off[i] >= file[i] || off[i] < 0) /* bad index */
	    return cerror(SUBSC_RANGE, iq);
	  break;
	default:
	  return cerror(ILL_CLASS, iq);
	} /* end of switch */
    } /* end of for */
    if (warn && range_warn_flag) {
      printf("WARNING - clipped invalid subscript range(s) in %s()\n",
	     varName(nsym));
    }
    if (narg == dfile + 1) {	/* one more subscr */
      off[i] = int_arg(ps[i]);
      if (off[i] < 0)
	return cerror(SUBSC_RANGE, iq);
      doff++;
    }
  } /* end of else */
  if (!dout) {			/* only one element */
    dout = 1;
    *out = 1;
    *axes = 0;
  }

				/* set up step sizes etc. */
  for (i = 0; i < dout; i++) tally[i] = 1;
  n = *ystep = 1; for (i = 1; i < doff; i++) ystep[i] = (n *= file[i - 1]);
  n = lux_type_size[type];
  for (i = 0; i < doff; i++) offset += off[i]*ystep[i];  offset *= n;
  for (i = 0; i < dout; i++) step[i] = ystep[axes[i]]*n;
  for (i = dout - 1; i; i--) step[i] -= step[i - 1]*out[i - 1];
  *step -= n;			/* because reading advances file pointer */
  iq = array_scratch(type, dout, out); /* result array */
  h = HEAD(iq);
  q.l = LPTR(h);
  if (fseek(fp, offset + baseOffset, SEEK_SET))
    return cerror(POS_ERR, iq);
  while (!*step && dout)	/* can read in batches */
  { n *= *out;			/* read rows at a time */
    step++;			/* one level higher */
    out++;
    dout--; }
  if (!dout) done = 1;		/* immediate exit of do-while */
  do
  { if (fread(q.b, n, 1, fp) != 1) return cerror(READ_ERR, iq);
    q.b += n;
    for (i = 0; i < dout; i++)
    { if (tally[i]++ != out[i]) { done = 0; break; }
      tally[i] = 1; done = 1;
      fseek(fp, step[i + 1], SEEK_CUR); }
    if (!done && fseek(fp, *step, SEEK_CUR)) return cerror(POS_ERR, iq);
  } while (!done);
  return iq;
}
/*------------------------------------------------------------------------- */
int32_t lux_assoc_output(int32_t iq, int32_t jq, int32_t offsym, int32_t axsym)
 /* use an associated variable as a guide to writing into a file */
 /* iq is sym # of assoc var, jq is rhs sym with data */
/* Headers:
   <stdio.h>: FILE, printf(), puts(), fwrite(), fseek(), perror()
 */
{
 int32_t	ddat, *dat, dfile, *file, lun, daxes, *axes, doff, *off, offset;
 int32_t	i, dattype, assoctype, ystep[8], rstep[8], tally[8], done;
 int32_t	efile, n, *step, baseOffset;
 Pointer	q;
 array	*h;
 FILE	*fp;

 step = rstep;
 assoctype = sym[iq].type;
 dattype = sym[jq].type;
 if (assoctype != dattype)			/* make proper type */
 { switch (assoctype)
   { case LUX_INT8:  jq = lux_byte(1, &jq);  break;
     case LUX_INT16:  jq = lux_word(1, &jq);  break;
     case LUX_INT32:  jq = lux_long(1, &jq);  break;
     case LUX_INT64:  jq = lux_int64(1, &jq);  break;
     case LUX_FLOAT: jq = lux_float(1, &jq); break;
     case LUX_DOUBLE: jq = lux_double(1, &jq); break; }
 }
 switch (symbol_class(jq))
 { case LUX_SCAL_PTR: case LUX_SCALAR:
     done = ddat = 1;
     dat = &done;
     q.l = &sym[jq].spec.scalar.l;
     break;
   case LUX_ARRAY:
     h = HEAD(jq);					/* data */
     ddat = h->ndim;
     dat = h->dims;
     q.l = LPTR(h);
     break;
   default:  return cerror(ILL_CLASS, jq); }
 h = HEAD(iq);					/* associated variable */
 dfile = h->ndim;
 file = h->dims;
 lun = h->c1;
 if (lun < 0 || lun > MAXFILES) return cerror(ILL_LUN, 0);
 if (lux_file_open[lun] == 0) return cerror(LUN_CLOSED, 0);
 if (lux_file_open[lun] != 2) return cerror(READ_ONLY, 0);
 fp = lux_file[lun];
 baseOffset = assoc_has_offset(iq)? assoc_offset(iq): 0;
					  /* LUX_ASSOC call, e.g. header */
 if (ddat > dfile + 1)
 { printf("Source has too many dimensions for associated file structure");
   return cerror(INCMP_DIMS, jq); }
 if (!offsym && axsym >= 0) { offset = 0;  doff = 0; }
 else
 { if (axsym == -1)
   { if (symbol_class(offsym) == LUX_ARRAY)		/* coordinate offset */
     { h = HEAD(offsym);
       GET_SIZE(doff, h->dims, h->ndim);		/* #offset elements */
       if (doff < dfile || doff > dfile + 1)
       { puts("Offset has wrong # dimensions");
	 return cerror(INCMP_DIMS, offsym); }
       off = LPTR(h);
       CK_SGN(off, doff, 1, iq); }
     else 
     { offset = int_arg(offsym);
       doff = 0; }
   } else
   { offset = offsym;  doff = 0; }
 }
 if (axsym < 0)			/* assoc(off) = .. quick insert */
 { GET_SIZE(done, dat, ddat);
   ddat = 1;					/* fake 1D insert */
   daxes = 0;
   dat = &done; }
 else if (axsym)				/* redirection axes */
 { h = HEAD(axsym);
   GET_SIZE(daxes, h->dims, h->ndim);
   if (daxes != ddat)
   { puts("Wrong number of axes");
     return cerror(INCMP_DIMS, axsym); }
   axes = LPTR(h); 
   CK_SGN(axes, daxes, 2, iq);			/* nonnegative? */
   CK_MAG(dfile, axes, daxes, 2, iq); }		/* within rearrange bounds? */
 else daxes = 0;
 for (i = 0; i < ddat; i++) tally[i] = 1; 
 n = *ystep = 1; 
 for (i = 1; i < dfile; i++) ystep[i] = (n *= file[i - 1]);
 n = lux_type_size[assoctype];
 if (doff)
 { offset = 0;
   for (i = 0; i < doff; i++) offset += off[i]*ystep[i]; }
 else
 { GET_SIZE(efile, dat, ddat);
   offset *= efile; }
 offset *= n;
 if (axsym >= 0) for (i = 0; i < ddat; i++)
   if (off[(daxes)? axes[i]: i] + dat[i] > file[(daxes)? axes[i]: i])
     { printf("%d + %d > %d\n", off[(daxes)? axes[i]: i], dat[i],
         file[(daxes)? axes[i]: i]);
       puts("Specified coordinates reach outside file");
       return cerror(INCMP_DIMS, iq); }
 for (i = 0; i < ddat; i++) step[i] = ystep[(daxes)? axes[i]: i]*n;
 for (i = ddat - 1; i; i--) step[i] -= step[i - 1]*dat[i - 1];
 *step -= n;			/* because writing advances file pointer */
 if (fseek(fp, offset + baseOffset, SEEK_SET))
 { perror("System message");  return cerror(POS_ERR, iq); }
 while (!*step && ddat)				/* can write in batches */
 { n *= *dat;					/* write a row at a time */
   step++;					/* one level higher */
   dat++;
   ddat--; }
 if (!ddat) done = 1;
 do
 { if (fwrite(q.b, n, 1, fp) != 1) return cerror(WRITE_ERR, iq);
   q.b += n;
   for (i = 0; i < ddat; i++)
   { if (tally[i]++ != dat[i]) { done = 0; break; }
     tally[i] = 1; done = 1;
     fseek(fp, step[i + 1], SEEK_CUR); }
  if (!done && fseek(fp, *step, SEEK_CUR)) return cerror(POS_ERR, iq);
 } while (!done);
 return 1;
}
/*------------------------------------------------------------------------- */
FILE *fopenr_sym(int32_t nsym)	/* internal utility */
 /* decode an lux string symbol and use as a file name to open a file for
 read, 1/17/99 */
/* Headers:
   <stdio.h>: FILE, NULL
 */
{
  FILE *fin;

  if (!symbolIsStringScalar(nsym)) {
    cerror(NEED_STR, nsym);
    return NULL;
  }
						 /* try to open the file */
  if ((fin = fopen(expand_name(string_value(nsym), NULL), "r")) == NULL) {
    cerror(ERR_OPEN, 0);
    return NULL;
  }
  return fin;
}
/*------------------------------------------------------------------------- */
#if WORDS_BIGENDIAN
#define SYNCH_OK	0xaaaa5555
#define SYNCH_REVERSE	0x5555aaaa
#else
#define SYNCH_OK	0x5555aaaa
#define SYNCH_REVERSE	0xaaaa5555
#endif
int32_t ck_synch_hd(FILE *fin, fzHead *fh, int32_t *wwflag)
/* internal utility */
/* reads the start of an fz file, checks for synch and a reasonable header,
   returns -1 if something amiss, used by several fz readers. */
/* Headers:
   <stdio.h>: fread(), perror(), printf(), fclose()
 */
{
  if (fread(fh, 1, 256, fin) != 256)
    perror("fzread in header");
  if (fh->synch_pattern != SYNCH_OK) {
    if (fh->synch_pattern == SYNCH_REVERSE) {
      printf("Reversed F0 magic number -- keep your fingers crossed.\n");
      *wwflag = 1;
    } else {
      printf("File does not have F0 magic number, but %x\n",
	     fh->synch_pattern);
      fclose(fin);
      return LUX_ERROR;
    }
  } else
    *wwflag = 0;
  if (MSBfirst ^ *wwflag)
    endian(fh->dim, fh->ndim*sizeof(int32_t), LUX_INT32);
  return 1; 
}
/*------------------------------------------------------------------------- */
void fz_get_header(FILE *fin, int32_t nsym, int32_t n) /* internal utility */
/* reads <n> blocks from stream <fin> into symbol <nsym>.  The first */
/* block is 256 bytes, all successive blocks are 512 bytes.  If a \0 is found */
/* in the read text, then the resulting string is terminated at that point. */
/* Otherwise, a \0 is added.  LS 4feb00 */
/* Headers:
   <stdio.h>: fread()
   <stdlib.h>: realloc()
 */
{
  char	*q;

  if (n)
    n = (n - 1)*512 + 256;
  redef_string(nsym, n);
  q = string_value(nsym);
  if (n)
    fread(q, 1, n, fin);
  while (*q && n--)
    q++;
  symbol_memory(nsym) = (q - string_value(nsym)) + 1;
  string_value(nsym) = (char*) realloc(string_value(nsym), symbol_memory(nsym));
  if (*q)
    *q = '\0';			/* proper termination */
}
 /*------------------------------------------------------------------------- */
void fz_print_header(FILE *fin, int32_t n) /* internal utility */
/* reads <n> blocks from stream <fin> and prints them to the standard */
/* output.  The first block is 256 bytes, all successive blocks are 512 */
/* bytes.  If a \0 is found in the read text, then the printed text is */
/* terminated at that point.  LS 4feb00 */
/* Headers:
   <stdio.h>: getc(), FILE, putchar(), fseek()
*/
{
  int32_t	c;

  if (n)
    n = (n - 1)*512 + 256;
  while (n--) {
    c = getc(fin);
    if (c)
      putchar(c);
    else {
      putchar('\n');
      break;
    }
  }
  if (n > 0)
    fseek(fin, n, SEEK_CUR);
}
/*------------------------------------------------------------------------- */
int32_t lux_fzinspect(int32_t narg, int32_t ps[])		/* fzinspect subroutine */
 /* return some info about an fz file */
 /* subr version, fzinspect(name, param, [header])
 where name is the file name, param is an I*4 array containing the following:
 filesize (-1 if no file), variable type, ndim, dim(1 to 8)
 the header is optionally returned
 fzinspect usually returns with a "success" (unless file name is not a
 string) and user must check param(0) to see if file was actually readable */
/* Headers:
   <stdio.h>: FILE, fopen(), NULL, perror(), fclose()
   <sys/types.h>: stat()
   <sys/stat.h>: stat()
 */
{
  int32_t	wwflag=0, *q1, i;
  char	*name;
  fzHead	*fh;
  FILE	*fin;
  struct stat statbuf;

  /* define the return variable param, a long vector with 11 elements */
  /* pre-load with error condition in case we have a problem reading the
     file */
  i = 11;
  if (redef_array(ps[1], LUX_INT32, 1, &i) != 1)
    return -1;
  q1 = (int32_t*) array_data(ps[1]);
  q1[0] = -1;
  for (i = 1; i < 11; i++)
    q1[i] = 0;
  if (symbol_class(ps[0]) != LUX_STRING)
    return cerror(NEED_STR, ps[0]);
  name = string_value(ps[0]);
						 /* try to open the file */
  if ((fin = fopen(expand_name(name, NULL), "r")) == NULL)
    return cerror(ERR_OPEN, ps[0]);

  /* a few chores, check the synch and get a pointer to header */
  /* use scrat to read in header block */
  fh = (fzHead *) scrat;
  if (ck_synch_hd(fin, fh, &wwflag) < 0 ) return 1;

			 /* if a header requested, create and load */
  if (narg > 2)
    fz_get_header(fin, ps[2], fh->nhb);
  else
    fz_print_header(fin, fh->nhb);
 
  if (stat(expname, &statbuf) != 0) {
    perror("stat");
    return 1;
  }
  q1[0] = statbuf.st_size;
  q1[1]= fh->datyp;
  q1[2] = fh->ndim;
  for (i = 0; i < fh->ndim; i++)
    q1[i+3]= fh->dim[i];
  fclose(fin);
  return 1;
}
/*------------------------------------------------------------------------- */
int32_t fzhead(int32_t narg, int32_t ps[], int32_t flag) /* fzhead subroutine */
 /* read header in fz files */
 /* PROCEDURE -- Called by: FZHEAD,'FILENAME' [,TEXT HEADER] */
 /* LUX_FUNCTION  -- Called by: FZHEAD('FILENAME' [,TEXT HEADER] */
 /* flag = 0: procedure   flag = 1: function */
/* Headers:
   <stdio.h>: FILE, fopen(), perror(), printf(), fclose()
 */
{
 int32_t	wwflag;
 char	*name;
 fzHead	*fh;
 FILE	*fin;

 flag = flag & !error_extra;
 if ( symbol_class( ps[0] ) != 2 ) return (flag)? -1: cerror(NEED_STR, *ps);
 name = (char *) sym[ps[0] ].spec.array.ptr;
						 /* try to open the file */
 if ((fin = fopen(expand_name(name, NULL),"r")) == NULL) {
   if (flag)
     return LUX_ERROR;
   else {
     perror("System Message");
     printf("%s: ", expname);
     return cerror(ERR_OPEN, *ps);
   }
 }
				 /* use scrat to read in header block */
 fh = (fzHead *) scrat;
 if (ck_synch_hd(fin, fh, &wwflag) < 0)
   return LUX_ERROR;
 
 if (narg > 1)
   fz_get_header(fin, ps[1], fh->nhb);
 else
   fz_print_header(fin, fh->nhb);
 fclose(fin);
 return 1;
}
/*------------------------------------------------------------------------- */
/* NOTE on files:
   Ultrix machines (DECstations) have reversed Byte order relative to
   Irix machines (SGI).  Appropriate Byte swapping is done when writing
   to disk and tape, so that the files are Byte for Byte equal, whether
   written from a DEC or an SGI machine.   LS 10/8/92 */
/*------------------------------------------------------------------------- */
int32_t fzread(int32_t narg, int32_t ps[], int32_t flag) /* fzread subroutine */
 /* read standard f0 files, compressed or not */
 /* PROCEDURE -- Called by: FZREAD,VAR,'FILENAME' [,TEXT HEADER]*/
/* Headers:
   <stdio.h>: FILE, fopen(), NULL, perror(), printf(), fseek(), fclose(),
              fread()
   <stdlib.h>: malloc()
 */
{
  int32_t	iq, n, nb, i, mq, nq, sbit, nelem, wwflag=0;
  Symboltype type;
  char	*p, *name;
  fzHead	*fh;
  Pointer q1;
  FILE	*fin;
  int32_t	anadecrunch(uint8_t *, int16_t [], int32_t, int32_t, int32_t),
	anadecrunch8(uint8_t *, uint8_t [], int32_t, int32_t, int32_t),
	anadecrunchrun(uint8_t *, int16_t [], int32_t, int32_t, int32_t),
	anadecrunchrun8(uint8_t *, uint8_t [], int32_t, int32_t, int32_t);
#if SIZEOF_LONG_LONG_INT == 8	/* 64-bit integers */
  int32_t	anadecrunch32(uint8_t *, int32_t [], int32_t, int32_t, int32_t);
#endif

  struct compresshead {
    int32_t     tsize, nblocks, bsize;
    uint8_t    slice_size, type; } ch;

#if WORDS_BIGENDIAN
  union { int32_t i;  uint8_t b[4];} lmap;
#endif

	 /* first arg is the variable to load, second is name of file */
  if (symbol_class(ps[1]) != LUX_STRING)
    return (flag)? LUX_ERROR: cerror(NEED_STR, ps[1]);
  name = string_value(ps[1]);
						 /* try to open the file */
  if ((fin = fopen(expand_name(name, NULL), "r")) == NULL) {
    if (flag)
      return LUX_ERROR;
    else {
      perror("System Message");
      printf("%s: ", expname);
      return cerror(ERR_OPEN, ps[1]);
    }
  }

				 /* use scrat to read in header block */
  fh = (fzHead *) scrat;

  if (ck_synch_hd(fin, fh, &wwflag) < 0)
    return LUX_ERROR;

  type = (Symboltype) fh->datyp;		/* data type */

  /* compute size of array */
  nelem = 1;
  for (i = 0; i < fh->ndim; i++)
    nelem *= fh->dim[i];
  nb = nelem * lux_type_size[type]; /* data size */
			 /* if a header requested, create and load */
  if (narg > 2)
    fz_get_header(fin, ps[2], fh->nhb);
  else if (internalMode & 1)	/* print header */
    fz_print_header(fin, fh->nhb);
  else {			/* skip header */
    n = fh->nhb? (fh->nhb - 1)*512 + 256: 0;
    if (n)
      fseek(fin, n, SEEK_CUR);
  }
						 /* create the output array */
  iq = ps[0];
  if (redef_array(iq, type, fh->ndim, fh->dim) != 1) {
    fclose(fin);
    return LUX_ERROR;
  }

  q1.l = (int32_t*) array_data(iq);
				 /* big branch on compression flag */
  if ((fh->subf & 1) == 1 ) {	/* compression case */
    /* read in the compression header */
    nq = fread(&ch, 1, 14, fin);
    if (nq != 14 && !flag)
      perror("error reading in compression header");
#if WORDS_BIGENDIAN
    endian(&ch.tsize, sizeof(int32_t), LUX_INT32);
    endian(&ch.nblocks, sizeof(int32_t), LUX_INT32);
    endian(&ch.bsize, sizeof(int32_t), LUX_INT32);
    for (i = 0; i < 4; i++)
      lmap.b[i] = fh->cbytes[i];
    endian(&lmap.i, sizeof(int32_t), LUX_INT32);
#endif
    mq = ch.tsize - 14;
    if (mq <= NSCRAT)
      p = (char *) scrat;
    else {
      iq = string_scratch(mq);
      p = string_value(iq);
    }
    nq = fread(p, 1, mq, fin);

    if (nq != mq) {
      perror("error reading in compressed data");
      printf("expected %1d bytes; found %1d\n", mq, nq);
    }
    sbit = ch.slice_size;
    /* fix a problem with ch.nblocks */
    if (ch.bsize * ch.nblocks > nelem) {
      if (!flag)
	printf("warning, bad ch.nblocks = %d\n", ch.nblocks);
      ch.nblocks = nelem / ch.bsize;
      if (!flag)
	printf("correcting to %d, hope this is right!\n", ch.nblocks); }
    /* some consistency checks */
    if (ch.type % 2 == type)
      return flag? LUX_ERROR: luxerror("inconsisent compression type", 0);
    switch (ch.type) {
      case 0:
	iq = anadecrunch((uint8_t *) p, q1.w, sbit, ch.bsize, ch.nblocks);
	break;
      case 1:
	iq = anadecrunch8((uint8_t *) p, q1.b, sbit, ch.bsize, ch.nblocks);
	break;
      case 2:
	iq = anadecrunchrun((uint8_t *) p, q1.w, sbit, ch.bsize, ch.nblocks);
	break;
      case 3:
	iq = anadecrunchrun8((uint8_t *) p, q1.b, sbit, ch.bsize, ch.nblocks);
	break;
      case 4:
#if SIZEOF_LONG_LONG_INT == 8	/* 64-bit integers */
	iq = anadecrunch32((uint8_t *) p, q1.l, sbit, ch.bsize, ch.nblocks);
#else
	puts("32-bit decompression was not compiled into your version of LUX");
	iq = LUX_ERROR;
#endif
	break;
      default:
	if (!flag)
	  printf("error in data type for compressed data, type = %d\n", type);
	iq = -1;
	break;
    }
    if (iq != 1) {
      fclose(fin);
      return LUX_ERROR;
    }
  } else {			/* non-compression case */
    nq = (((fh->subf >> 7) & 1) ^ MSBfirst);
    if (type == LUX_STRING_ARRAY) {
      /* a string array */
      while (nelem--) {
	fread(&mq, sizeof(int32_t), 1, fin);
	if (nq)
	  endian(&mq, sizeof(int32_t), LUX_INT32);
	if (mq) {
	  *q1.sp = (char*) malloc(mq + 1);
	  if (!*q1.sp) {
	    fclose(fin);
	    return flag? LUX_ERROR: cerror(ALLOC_ERR, 0);
	  }
	  fread(*q1.sp, 1, mq, fin);
	  (*q1.sp)[mq] = '\0';	/* terminate */
	} else
	  *q1.sp = NULL;
	q1.sp++;
      }
    } else {
      /* just read into the array */
      if (fread(q1.l, 1, nb, fin) != nb) {
	fclose(fin);
	return flag? LUX_ERROR: cerror(READ_EOF, 0);
      }
      if (nq)
	endian(q1.b, nb, type);	/* swap bytes */
    }
  }
  /* common section again */
  fclose(fin);
  return 1;
}
/*------------------------------------------------------------------------- */
int32_t fzwrite(int32_t narg, int32_t ps[], int32_t flag) /* fzwrite subroutine */      
 /* write standard f0 files, uncompressed
    flag = 0: procedure   flag = 1: function */
 /* If you are storing an updated version of a big file and if something */
 /* goes wrong between the reopening of the file (i.e. throwing away of */
 /* the old data) and the end of the routine, then you end up with a */
 /* truncated or empty file.  To ensure that the old data isn't thrown away */
 /* before the new file is ready, use the /SAFE switch.  This opens a */
 /* temporary file for writing and renames it to the target file name */
 /* only after the file has been successfully written and closed. */
 /* LS 5dec95 */
/* Headers:
   <stdio.h>: FILE, NULL, fopen(), perror(), printf(), fwrite(), putc(),
              fclose(), ferror(), remove(), rename()
   <string.h>: strrchr(), strcpy(), strcat(), strlen(), memcpy(), strlen()
   <stdlib.h>: malloc()
 */
{
  int32_t	iq, n, nd, j, type, mq, i, sz, safe;
  char const	*p, *name;
  char* safename;
  fzHead	*fh;
  Pointer q1, q2;
  FILE	*fout;
  static char const* safeFile = "Safe_file";
					 /* first arg. must be an array */
  iq = ps[0];
  if (symbol_class(iq) != LUX_ARRAY)
    return (flag)? LUX_ERROR: cerror(NEED_ARR, iq);
  type = symbol_type(iq);
  q1.l = (int32_t *) array_data(iq);
  nd = array_num_dims(iq);
			 /* second argument must be a string, file name */
  if (symbol_class(ps[1]) != LUX_STRING)
    return flag? LUX_ERROR: cerror(NEED_STR, ps[1]);

  name = expand_name(string_value(ps[1]), NULL); /* target file */
  safe = internalMode & 1;
  if (safe) {
    p = strrchr(name, '/');
    if (p) {
      char *q;
      q = safename = (char *) malloc(strlen(name) + 11);
      memcpy(q, name, p - name);
      q += p - name;
      *q = '\0';
      strcat(q, "/");
      strcat(q, safeFile);
    } else
      safename = NULL;
    name = safename? safename: safeFile;
  }
						 /* try to open the file */
  if ((fout = fopen(name, "w")) == NULL) {
    if (flag)
      return LUX_ERROR;
    else {
      perror("System Message");
      printf("%s: ", name);
      return cerror(ERR_OPEN, ps[1]);
    }
  }
					 /* use scrat to setup header */
  fh = (fzHead *) scrat;
  zerobytes(fh, 512);		/*zero it first */
#if WORDS_BIGENDIAN
  fh->synch_pattern = 0xaaaa5555;
#else
  fh->synch_pattern = 0x5555aaaa;
#endif
  fh->subf = MSBfirst << 7;	/* no compression; indicate endian */
  fh->source = 0;
  fh->nhb = 1;			/*may be changed later */
  fh->datyp = type;
  fh->ndim = nd;
  n = array_size(iq)*lux_type_size[type];
  memcpy(fh->dim, array_dims(iq), nd*sizeof(int32_t));
#if WORDS_BIGENDIAN
  endian(fh->dim, fh->ndim*sizeof(int32_t), LUX_INT32);
#endif

  if (narg > 2) {			 /* have a header */
    /* NOTE: the first 256 bytes of the first 512-Byte block are used */
    /* for other stuff, so only 256 bytes of header text can be */
    /* stored in the first block, and 512 bytes in all subsequent blocks */
    if (symbolIsStringScalar(ps[2]))  /* the header is a string */
      mq = symbol_memory(ps[2]) - 1; /* don't count final \0 */
    else {
      q2.sp = (char**) array_data(ps[2]);
      i = array_size(ps[2]);
      mq = 0;			/* determine the total length */
      while (i--) {
	if (*q2.sp)
	  mq += strlen(*q2.sp);
	q2.sp++;
      }
    }
    fh->nhb = (mq + 767)/512;	/* number of blocks required */
    j = fwrite(fh, 1, 256, fout); /* write the first block until the start */
    if (j != 256)
      goto fzwrite_1;
    if (symbolIsStringScalar(ps[2])) { /* the header is a string */
      p = string_value(ps[2]);
      if (fwrite(p, 1, mq, fout) != mq) /* write the header */
	goto fzwrite_1;
    } else if (symbolIsStringArray(ps[2])) { /* string array */
      q2.sp = (char**) array_data(ps[2]);
      i = array_size(ps[2]);
      while (i--) {
	sz = *q2.sp? strlen(*q2.sp): 0;
	if (sz && sz != fwrite(*q2.sp, 1, strlen(*q2.sp), fout))
	  goto fzwrite_1;
	q2.sp++;
      }
    }
    mq = (mq + 256) % 512;	/* how many in the last 512-uint8_t block */
    if (mq) {			/* we must pad until we get another full */
				/* 512-uint8_t block */
      mq = 512 - mq;
      while (mq--)		/* pad */
	putc('\0', fout);
    } else {			/* no string: illegal */
      fclose(fout);
      return flag? LUX_ERROR: cerror(NEED_STR, ps[2]);
    }
  } else {			/* no header */
    fh->nhb = 1;
    zerobytes(fh->txt, 256);
    if (fwrite(fh, 1, 512, fout) != 512)
      goto fzwrite_1;
  }
  
  if (type == LUX_STRING_ARRAY) {
    /* a string array is special: we write for each string in succession
       the string size and then the string itself.  LS 22jan99 */
    n = array_size(iq);
    while (n--) {
      mq = *q1.sp? strlen(*q1.sp): 0;
      fwrite(&mq, sizeof(int32_t), 1, fout);
      if (mq)
	fwrite(*q1.sp, 1, strlen(*q1.sp), fout);
      q1.sp++;
    }
    if (ferror(fout)) {
      if (!flag)
	cerror(WRITE_ERR, 0);
      fclose(fout);
      if (safe)
	remove(name);
      return LUX_ERROR;
    }
  } else {
    j = fwrite(q1.l, 1, n, fout);	/* write data */
  
    if (j != n) {			/* not all data was written */
      if (!flag)
	cerror(WRITE_ERR, 0);
      fclose(fout);
      if (safe)
	remove(name);
      return LUX_ERROR;
    }
  }
  fclose(fout);
  if (safe) {
    if (rename(name, expand_name(string_value(ps[1]), NULL)) < 0)
      perror("System message:");
  }
  return LUX_OK;

  fzwrite_1:
  if (!flag)
    cerror(WRITE_ERR, 0);
  fclose(fout);
  if (safe)
    remove(name);
  return LUX_ERROR;
}
/*------------------------------------------------------------------------- */
int32_t fcwrite(int32_t, int32_t [], int32_t);
int32_t fcrunwrite(int32_t narg, int32_t ps[], int32_t flag)/* fcrunwrite subroutine */
/* write standard f0 files, run-length compressed format */
/* flag: 0 -> procedure, 1 -> function.  LS 10nov99 */
{
  int32_t	iq;

  runlengthflag = 1;		/* flag run-length compression */
  iq = fcwrite(narg, ps, flag);
  /* runlengthflag is reset to 0 by fcwrite(() */
  return iq;
}
/*------------------------------------------------------------------------- */
int32_t fcwrite(int32_t narg, int32_t ps[], int32_t flag)/* fcwrite subroutine */	
 /* write standard f0 files, compressed format */
 /* not done yet 12/11/91 */
 /* flag = 0: procedure  flag = 1: function   LS 1mar93 */
/* Headers:
   <stdio.h>: FILE, puts(), fopen(), NULL, perror(), printf(), fwrite(),
              putc(), fclose()
   <stdlib.h>: malloc(), free()
   <string.h>: strlen()
 */
{
 int32_t	iq, n, nd, j, type, i, mq, nx, ny, limit, sz;
 char	*name, *p;
 fzHead	*fh;
 Pointer q1, q2;
 union { int32_t i;  uint8_t b[4];} lmap;
 FILE	*fout;
					 /* first arg. must be an array */
 iq = ps[0];
 if (symbol_class(iq) != LUX_ARRAY)
   return (flag)? LUX_ERROR: cerror(NEED_ARR, *ps);
 type = array_type(iq);
 if (type != LUX_INT8 && type != LUX_INT16) {
   if (!flag) {
     puts("FCWRITE - Need I*1 or I*2 argument");
     return cerror(ILL_TYPE, iq, typeName(symbol_type(iq)));
   } else
     return LUX_ERROR;
 }
 q1.l = (int32_t*) array_data(iq);
 nd = array_num_dims(iq);

 if (internalMode & 1)
   runlengthflag = 1;

			 /* second argument must be a string, file name */
 if (symbol_class(ps[1]) != LUX_STRING) {
   runlengthflag = 0;
   return flag? LUX_ERROR: cerror(NEED_STR, ps[1]);
 }
 name = string_value(ps[1]);
						 /* try to open the file */
 if ((fout = fopen(expand_name(name, NULL), "w")) == NULL) {
   runlengthflag = 0;
   if (flag)
     return LUX_ERROR;
   else {
     perror("System Message");
     printf("%s: ", expname);
     return cerror(ERR_OPEN, ps[1]);
   }
 }
					 /* use scrat to setup header */
 fh = (fzHead *) scrat;
 zerobytes(fh, 512);		/*zero it first */
			 /* these have to be readable by the Vax's */
#if WORDS_BIGENDIAN
 fh->synch_pattern = 0xaaaa5555;
#else
 fh->synch_pattern = 0x5555aaaa;
#endif
 fh->subf = (MSBfirst << 7) | 1;
 fh->source = 0;
 fh->nhb = 1;					/*may be changed later */
 fh->datyp = type;
 fh->ndim = nd;
 n = array_size(iq);
 memcpy(fh->dim, array_dims(iq), nd*sizeof(int32_t));
 nx = fh->dim[0];
#if WORDS_BIGENDIAN
 endian(fh->dim, fh->ndim*sizeof(int32_t), LUX_INT32);
#endif
 ny = n/nx;
 n = n * lux_type_size[type];

 /* now compress the array, must be a uint8_t or short */
 limit = 2*n;
 q2.l = (int32_t *) malloc(limit);	/* some room for mistakes */
 switch (type) {
   case LUX_INT8:
     if (runlengthflag)
       iq = anacrunchrun8(q2.b, q1.b, crunch_slice, nx, ny, limit);
     else
       iq = anacrunch8(q2.b, q1.b, crunch_slice, nx, ny, limit);
     break;
   case LUX_INT16:
     if (runlengthflag)
       iq = anacrunchrun(q2.b, q1.w, crunch_slice, nx, ny, limit);
     else
       iq = anacrunch(q2.b, q1.w, crunch_slice, nx, ny, limit);
     break;
#if SIZEOF_LONG_LONG_INT == 8
   case LUX_INT32:
     if (!runlengthflag)
       iq = anacrunch32(q2.b, q1.l, crunch_slice, nx, ny, limit);
     /* else fall-through to default: we don't have a 32-bit crunchrun */
#endif
   default:
     runlengthflag = 0;
     return flag? LUX_ERROR: cerror(IMPOSSIBLE, 0);
 }
 q1.l = q2.l;			/* compressed data */
 if (iq < 0) {
   if (!flag)
     printf("not enough space allocated (%d bytes) for compressed array\n",
	    limit);
   goto fcwrite_1;
 }
 lmap.i = iq;			/* need to use this roundabout method because
				 .cbytes is not aligned on an int32_t boundary */
#if WORDS_BIGENDIAN
 endian(&lmap.i, sizeof(int32_t), LUX_INT32);
#endif
 for (i = 0; i < 4; i++)
   fh->cbytes[i] = lmap.b[i];

 if (narg > 2) {			 /* have a header */
   /* NOTE: the first 256 bytes of the first 512-Byte block are used */
   /* for other stuff, so only 256 bytes of header text can be */
   /* stored in the first block, and 512 bytes in all subsequent blocks */
   if (symbolIsStringScalar(ps[2]))  /* the header is a string */
     mq = symbol_memory(ps[2]) - 1; /* don't count final \0 */
   else {
     q2.sp = (char**) array_data(ps[2]);
     i = array_size(ps[2]);
     mq = 0;			/* determine the total length */
     while (i--) {
       if (*q2.sp)
	 mq += strlen(*q2.sp);
       q2.sp++;
     }
   }
   fh->nhb = (mq + 767)/512;	/* number of blocks required */
   j = fwrite(fh, 1, 256, fout); /* write the first block until the start */
   if (j != 256)
     goto fcwrite_1;
   if (symbolIsStringScalar(ps[2])) { /* the header is a string */
     p = string_value(ps[2]);
     j = fwrite(p, 1, mq, fout); /* write the header */
     if (j != mq)
       goto fcwrite_1;
   } else if (symbolIsStringArray(ps[2])) { /* string array */
     q2.sp = (char**) array_data(ps[2]);
     i = array_size(ps[2]);
     while (i--) {
       sz = *q2.sp? strlen(*q2.sp): 0;
       if (sz && sz != fwrite(*q2.sp, 1, strlen(*q2.sp), fout))
	 goto fcwrite_1;
       q2.sp++;
     }
   }
   mq = (mq + 256) % 512;	/* how many in the last 512-Byte block */
   if (mq) {			/* we must pad until we get another full */
				/* 512-Byte block */
     mq = 512 - mq;
     while (mq--)		/* pad */
       putc('\0', fout);
   } else {			/* no string: illegal */
     fclose(fout);
     runlengthflag = 0;
     return flag? LUX_ERROR: cerror(NEED_STR, ps[2]);
   }
 } else {			/* no header */
   fh->nhb = 1;
   zerobytes(fh->txt, 256);
   if (fwrite(fh, 1, 512, fout) != 512)
     goto fcwrite_1;
 }

 if (fwrite(q1.l, 1, iq, fout) != iq)
   goto fcwrite_1;
 free(q1.l);
 fclose(fout);
 runlengthflag = 0;
 return	LUX_OK;

  fcwrite_1:
 free(q1.l);
 runlengthflag = 0;
 if (!flag)
   cerror(WRITE_ERR, 0);
 fclose(fout);
 return LUX_ERROR;
}
/*------------------------------------------------------------------------- */
int32_t lux_fzread(int32_t narg, int32_t ps[])
 /* routine version */
{ return fzread(narg, ps, 0); }
/*------------------------------------------------------------------------- */
int32_t lux_fzwrite(int32_t narg, int32_t ps[])
 /* routine version */
{ return fzwrite(narg, ps, 0); }
/*------------------------------------------------------------------------- */
int32_t lux_fzhead(int32_t narg, int32_t ps[])
 /* routine version */
{ return fzhead(narg, ps, 0); }
/*------------------------------------------------------------------------- */
int32_t lux_fcwrite(int32_t narg, int32_t ps[])
 /* routine version */
{ return fcwrite(narg, ps, 0); }
/*------------------------------------------------------------------------- */
int32_t lux_fcrunwrite(int32_t narg, int32_t ps[])
{ return fcrunwrite(narg, ps, 0); }
/*------------------------------------------------------------------------- */
int32_t lux_fzread_f(int32_t narg, int32_t ps[])
 /* a function version that returns 1 if read OK */
 { return (fzread(narg, ps, 1) == 1)? 1: 4; }
/*------------------------------------------------------------------------- */
int32_t lux_fzhead_f(int32_t narg, int32_t ps[])
 /* a function version that returns 1 if read OK */
 { return (fzhead(narg, ps, 1) == 1)? 1: 4; }
/*------------------------------------------------------------------------- */
int32_t lux_fzwrite_f(int32_t narg, int32_t ps[])
 /* function version */
{ return (fzwrite(narg, ps, 1) == 1)? 1: 4; }
/*------------------------------------------------------------------------- */
int32_t lux_fcwrite_f(int32_t narg, int32_t ps[])
 /* function version */
{ return (fcwrite(narg, ps, 1) == 1)? 1: 4; }
/*------------------------------------------------------------------------- */
int32_t lux_fcrunwrite_f(int32_t narg, int32_t ps[])
{ return (fcrunwrite(narg, ps, 1) == 1)? LUX_OK: LUX_ERROR; }
/*------------------------------------------------------------------------- */
int32_t readu(int32_t narg, int32_t ps[], int32_t flag)
/* read unformatted data from a file into a variable, which must be predefined.
   syntax: readu,lun,x [,y,...]
   LS 12may93 */
/* Headers:
   <stdio.h>: FILE, fread(), feof()
   <stdlib.h>: free()
 */
{
 int32_t	lun, n, iq, i;
 FILE	*fp;
 Pointer	p;
 array	*h;
 char	str, *q;

 lun = int_arg(*ps++);
 if (lun < 0 || lun >= MAXFILES)
 { if (flag) return 0; else return cerror(ILL_LUN, ps[-1]); }
 if (!lux_file_open[lun])
 { if (flag) return 0; else return cerror(LUN_CLOSED, ps[-1]); }
 fp = lux_file[lun];
 narg--;
 while (narg--)
 { iq = *ps++;				/* next variable */
   str = 0;
   switch (symbol_class(iq))
   { case LUX_SCALAR:
       n = lux_type_size[sym[iq].type];
       p.b = &sym[iq].spec.scalar.b;  break;
     case LUX_STRING:
       n = sym[iq].spec.array.bstore - 1;
       p.s = sym[iq].spec.name.ptr;  break;
     case LUX_ARRAY:
       h = HEAD(iq);
       GET_SIZE(n, h->dims, h->ndim);
       if ((int32_t) symbol_type(iq) < LUX_TEMP_STRING)	/* numerical array */
	 n *= lux_type_size[sym[iq].type];
       else str = 1;			/* string array */
       p.l = LPTR(h);  break;
     default:
       if (flag) return 0; else return cerror(ILL_CLASS, iq);
    }
    if (str)						/* string array */
      while (n--)
      { q = *p.sp;  if (q) free(q);
        fread(scrat, strlen(q), 1, fp);
	if (feof(fp)) return (flag? 0: cerror(READ_EOF,0));
        q = strsave((char *) scrat);
        p.sp++; }
    else
    { fread(p.b, n, 1, fp);				/* numerical arg */
      if (feof(fp)) return (flag? 0: cerror(READ_EOF,0)); }
  }
  return 1;
}
/*------------------------------------------------------------------------- */
int32_t writeu(int32_t narg, int32_t ps[], int32_t flag)
/* write unformatted data from a variable into a file. 
   syntax: writeu,lun,x [,y,...]
   LS 12may93 */
/* Headers:
   <stdio.h>: FILE, fwrite()
   <string.h>: strlen()
 */
{
 int32_t	lun, n, iq, i;
 FILE	*fp;
 Pointer	p;
 array	*h;
 char	str, *q;

 lun = int_arg(*ps++);
 if (lun < 0 || lun >= MAXFILES)
 { if (!flag) return cerror(ILL_LUN, ps[-1]); else return 0; }
 if (!lux_file_open[lun])
 { if (!flag) return cerror(LUN_CLOSED, ps[-1]); else return 0; }
 fp = lux_file[lun];
 narg--;
 while (narg--)
 { iq = *ps++;				/* next variable */
   str = 0;
   switch (symbol_class(iq))
   { case LUX_SCALAR:
       n = lux_type_size[sym[iq].type];
       p.b = &sym[iq].spec.scalar.b;  break;
     case LUX_STRING:
       n = sym[iq].spec.array.bstore - 1;
       p.s = sym[iq].spec.name.ptr;  break;
     case LUX_ARRAY:
       h = HEAD(iq);
       GET_SIZE(n, h->dims, h->ndim);
       if ((int32_t) symbol_type(iq) < LUX_TEMP_STRING)	/* numerical array */
	 n *= lux_type_size[sym[iq].type];
       else str = 1;			/* string array */
       p.l = LPTR(h);  break;
     default:
       if (flag) return 0; else return cerror(ILL_CLASS, iq);
    }
    if (str)						/* string array */
      while (n--)
      { q = *p.sp;
        fwrite(q, strlen(q), 1, fp);
        p.sp++; }
    else fwrite(p.b, n, 1, fp);				/* numerical arg */
  }
  return 1;
}
/*------------------------------------------------------------------------- */
int32_t lux_readu(int32_t narg, int32_t ps[])
 /* subroutine version */
{ return readu(narg, ps, 0); }
/*------------------------------------------------------------------------- */
int32_t lux_readu_f(int32_t narg, int32_t ps[])
 /* function version */
{ return (readu(narg, ps, 1) == 1)? 1: 4; }
/*------------------------------------------------------------------------- */
int32_t lux_writeu(int32_t narg, int32_t ps[])
 /* subroutine version */
{ return writeu(narg, ps, 0); }
/*------------------------------------------------------------------------- */
int32_t lux_writeu_f(int32_t narg, int32_t ps[])
 /* function version */
{ return (writeu(narg, ps, 1) == 1)? 1: 4; }
/*------------------------------------------------------------------------- */
int32_t ok_for_astore(int32_t iq)
{
  switch (symbol_class(iq)) {
  case LUX_STRING: case LUX_SUBSC_PTR:
  case LUX_FILEMAP: case LUX_ASSOC:
  case LUX_SCALAR: case LUX_UNDEFINED:
  case LUX_ARRAY: case LUX_CLIST:
    return 1;
  default:
    return 0;
  }
}
/*------------------------------------------------------------------------- */
void astore_one(FILE *fp, int32_t iq)
{
  int32_t n, sz, i;
  Pointer p;

  fwrite(&sym[iq], sizeof(symTableEntry), 1, fp); /* the symbol */
  switch (symbol_class(iq)) {
  case LUX_ARRAY:
    n = symbol_memory(iq);
    p.l = (int32_t *) array_header(iq);
    fwrite(p.b, 1, n, fp);
    if (isStringType(array_type(iq))) { /* string array */
      sz = array_size(iq);	/* number of elements */
      p.sp = (char**) array_data(iq); /* pointer to list of strings */
      while (sz--) {
	n = *p.sp? strlen(*p.sp): 0; /* the length of the string */
	fwrite(&n, sizeof(int32_t), 1, fp); /* write it */
	if (n)
	  fwrite(*p.sp, 1, n, fp); /* write the string */
	p.sp++;
      }
    }
    break;
  case LUX_STRING:
    n = symbol_memory(iq);
    p.s = string_value(iq);
    fwrite(p.b, 1, n, fp);
    break;
  case LUX_FILEMAP:
    n = symbol_memory(iq);
    p.s = (char *) file_map_header(iq);
    fwrite(p.b, 1, n, fp);
    break;
  case LUX_CLIST:
    n = clist_num_symbols(iq);
    p.w = clist_symbols(iq);
    fwrite(&n, 1, sizeof(n), fp);
    for (i = 0; i < n; i++)
      astore_one(fp, p.w[i]);
    break;
  }
}
/*------------------------------------------------------------------------- */
int32_t astore(int32_t narg, int32_t ps[], int32_t flag)
/* STORE,x1 [, x2, x3, ...], file
  stores arbitrary data <x1>, <x2>, etcetera in file <file>.  The names
  (if any) as well as the data are stored.  LS 11mar97 */
/* currently BIGENDIAN/LITTLEENDIAN is disregarded. */
/* <flag> is 1 for a subroutine, 0 for a function */
/* Headers:
   <stdio.h>: FILE, fopen(), perror(), printf(), fwrite(), fclose()
   <string.h>: strlen()
 */
{
  int32_t	iq, i, intro[2], n;
  char const* file, *name;
  FILE	*fp;

  iq = ps[narg - 1];		/* file name */
  if (symbol_class(iq) != LUX_STRING)
    return flag? cerror(NEED_STR, iq): LUX_ZERO;
  file = string_arg(iq);
  --narg;
  for (i = 0; i < narg; i++) { /* all data arguments */
    iq = ps[i];
    if (!ok_for_astore(iq))
      return flag? cerror(ILL_CLASS, iq): LUX_ZERO;
  }
  /* all arguments are OK for storing */
  fp = fopen(expand_name(file, NULL), "w");
  if (!fp) {
    if (flag) {
      perror("System message:");
      printf("(%s)", expname);
      return cerror(ERR_OPEN, iq);
    } else
      return LUX_ZERO;
  }
  intro[0] = 0x6666aaaa;	/* identification */
  intro[1] = narg;
  fwrite(intro, sizeof(int32_t), 2, fp);
  for (i = 0; i < narg; i++) {
    iq = ps[i];
    if (iq < NAMED_END) {	/* a named variable */
      name = symbolProperName(iq);
      n = strlen(name) + 1;
      fwrite(&n, sizeof(int32_t), 1, fp); /* the name length */
      fwrite(name, sizeof(char), n, fp); /* write the name */
    } else {			/* unnamed */
      n = 0;
      fwrite(&n, sizeof(int32_t), 1, fp);
    }
    iq = transfer(iq);
    astore_one(fp, iq);
  }
  fclose(fp);
  return flag? LUX_ONE: 1;
}
/*------------------------------------------------------------------------- */
int32_t lux_astore(int32_t narg, int32_t ps[])
{
  return astore(narg, ps, 1);
}
/*------------------------------------------------------------------------- */
int32_t lux_astore_f(int32_t narg, int32_t ps[])
{
  return astore(narg, ps, 0);
}
/*------------------------------------------------------------------------- */
int32_t arestore_one(FILE* fp, int32_t iq, int32_t reverseOrder)
{
  int16_t hash, context;
  int32_t line, exec, n, j;
  Pointer p;

  hash = sym[iq].xx;		/* save because they'll be overwritten */
  line = sym[iq].line;
  context = symbol_context(iq);
  exec = sym[iq].exec;
  if (!fread(&sym[iq], sizeof(symTableEntry), 1, fp))
    return 1;			/* some problem: check errno for kind */
  sym[iq].xx = hash;		/* restore */
  symbol_line(iq) = line;
  symbol_context(iq) = context;
  sym[iq].exec = exec;
  switch (symbol_class(iq)) {
  case LUX_ARRAY:
    if (reverseOrder)
      endian(&symbol_memory(iq), sizeof(int32_t), LUX_INT32);
    n = symbol_memory(iq);
    ALLOCATE(p.v, n, char);
    array_header(iq) = (array *) p.v;
    if (!fread(p.b, n, 1, fp))
      return 1;
    n = array_num_dims(iq);
    if (reverseOrder) {
      endian(array_dims(iq), n*sizeof(int32_t), LUX_INT32);
      endian(array_data(iq), array_size(iq)*lux_type_size[array_type(iq)],
	     array_type(iq));
    }
    if (isStringType(symbol_type(iq))) { /* a string array */
      n = array_size(iq);	/* the number of strings */
      p.sp = (char**) array_data(iq);
      while (n--) {
	/* read the size of the next string */
	if (!fread(&j, sizeof(int32_t), 1, fp))
	  return 1;
	if (reverseOrder)
	  endian(&j, sizeof(int32_t), LUX_INT32);
	if (j) {
	  *p.sp = (char*) malloc(j + 1); /* reserve space for the string */
	  if (!*p.sp)
	    return 1;
	  if (!fread(*p.sp, j, 1, fp))
	    return 1;
	  (*p.sp)[j] = '\0'; /* terminate properly */
	} else
	  *p.sp = NULL;
	p.sp++;
      }
    }
    break;
  case LUX_STRING:
    n = symbol_memory(iq);
    if (reverseOrder)
      endian(&n, sizeof(int32_t), LUX_INT32);
    ALLOCATE(p.s, n, char);
    string_value(iq) = p.s;
    if (!fread(p.b, n, 1, fp))
      return 1;
    break;
  case LUX_FILEMAP:
    n = symbol_memory(iq);
    if (reverseOrder)
      endian(&n, sizeof(int32_t), LUX_INT32);
    ALLOCATE(p.s, n, char);
    file_map_header(iq) = (array *) p.s;
    if (!fread(p.b, n, 1, fp))
      return 1;
    n = file_map_num_dims(iq);
    endian(file_map_dims(iq), n*sizeof(int32_t), LUX_INT32);
    endian(&file_map_offset(iq), sizeof(int32_t), LUX_INT32);
    break;
  case LUX_CLIST:
    if (!fread(&n, sizeof(int32_t), 1, fp))
      return 1;
    if (reverseOrder)
      endian(&n, sizeof(int32_t), LUX_INT32);
    ALLOCATE(p.w, n*sizeof(int16_t), int16_t);
    clist_symbols(iq) = p.w;
    for (j = 0; j < n; j++) {
      int32_t iq2 = nextFreeTempVariable();
      arestore_one(fp, iq2, reverseOrder);
      symbol_context(iq2) = iq;
      p.w[j] = iq2;
    }
    break;
  }	  
  return 0;
}
/*------------------------------------------------------------------------- */
int32_t arestore(int32_t narg, int32_t ps[], int32_t flag)
/* ARESTORE [, x1, x2, x3, ...], file
  restores data <x1>, <x2>, etcetera from file <file>.
  if no data arguments are specified, then the whole contents of
  the file is restored to the variables with the names that are
  also stored in the file.  If data arguments are specified, then
  only the first that many variables are restored and the orgininal
  names associated with the stored variables are ignored.  LS 11mar97 */
/* the one-argument case is currently disallowed due to context */
/* problems.  LS 13mar97 */
/* flag = 1 -> subroutine */
/* Headers:
   <stdio.h>: FILE, perror(), fclose(), fseek(), NULL
   <stdlib.h>: malloc(), free()
 */
{
  int32_t	iq, i, intro[2], n, nvalue;
  char	*file, *name, reverseOrder;
  FILE	*fp;
  
  iq = ps[narg - 1];		/* file name */
  if (symbol_class(iq) != LUX_STRING) /* filename is not a string */
    return flag? cerror(NEED_STR, iq): LUX_ZERO;
  file = string_arg(iq);
  --narg;
  fp = openPathFile(file, 0);
  if (!fp) {			/* could not open file for reading */
    if (flag) {
      perror("System message:");
      return cerror(ERR_OPEN, iq);
    } else
      return LUX_ZERO;
  }
  if (!fread(intro, 2*sizeof(int32_t), 1, fp)) /* some error */
    return flag? cerror(READ_ERR, 0): LUX_ZERO;
  if (intro[0] == 0x6666aaaa)	/* native Byte order */
    reverseOrder = 0;
  else if (intro[0] == 0xaaaa6666) /* reversed Byte order */
    reverseOrder = 1;
  else {			/* wrong magic number */
    fclose(fp);
    return flag? luxerror("Not ASTORE file format", iq): LUX_ZERO;
  }
  nvalue = intro[1];		/* number of variables in the file */
  if (reverseOrder)
    endian((uint8_t *) &nvalue, sizeof(int32_t), LUX_INT32);
  if (narg && narg < nvalue)
    nvalue = narg;		/* the number of variables to restore */
  if (narg)			/* all arguments must be named variables */
    for (i = 0; i < nvalue; i++)
      if (ps[i] >= NAMED_END) {	/* not a named variable */
	fclose(fp);
	return flag? luxerror("Need a named variable", ps[i]): LUX_ZERO;
      }
  for (i = 0; i < nvalue; i++) {
    if (!fread(&n, sizeof(int32_t), 1, fp))  /* size of name */
      return flag? cerror(READ_ERR, 0): LUX_ZERO;
    if (reverseOrder)
      endian(&n, sizeof(int32_t), LUX_INT32);
    if (!narg) {		/* reading all of them: restore original */
				/* names */
      name = (char *) malloc(n);
      if (!fread(name, n, 1, fp)) { /* reading the name failed */
	free(name);
	return flag? cerror(READ_ERR, 0): LUX_ZERO;
      }
      if ((iq = findVarName(name, curContext)) < 0) { /* could not get
							 variable */
	fclose(fp);
	free(name);
	return flag? LUX_ERROR: LUX_ZERO;
      }
      free(name);
    } else {			/* assign value to the next argument */
      /* skip the name */
      if (fseek(fp, n, SEEK_CUR)) { /* some file positioning error */
	fclose(fp);
	return flag? LUX_ERROR: LUX_ZERO;
      }
      iq = ps[i];
    }
    undefine(iq); 		/* get rid of previous contents, if any */
    if (arestore_one(fp, iq, curContext)) {
      switch (errno) {
      case ENOMEM:
	return flag? cerror(ALLOC_ERR, 0): LUX_ZERO;
      default:
	return flag? cerror(READ_ERR, 0): LUX_ZERO;
      }
    }
  }
  fclose(fp);
  return LUX_ONE;
}
/*------------------------------------------------------------------------- */
int32_t lux_arestore(int32_t narg, int32_t ps[])
{
  return arestore(narg, ps, 1);
}
/*------------------------------------------------------------------------- */
int32_t lux_arestore_f(int32_t narg, int32_t ps[])
     /* function form of lux_restore */
{
  return arestore(narg, ps, 0);
}
/*------------------------------------------------------------------------- */
int32_t fileptr(int32_t narg, int32_t ps[], char function)
/* shows or sets a file pointer. */
/* FILEPTR,lun
     shows the position of the file pointer of the file opened on
     logical unit <lun> in bytes from the start.
   FILEPTR,lun,offset
     sets the file pointer <offset> bytes from the start (if
     <offset> >= 0), or to <offset> bytes from the end (if
     <offset> < 0).  FILEPTR,lun,-1 sets the pointer to the last Byte in the
     file.
   FILEPTR,lun [,offset] ,/START
     sets the file pointer to <offset> (default zero) bytes from the start.
   FILEPTR,lun [,offset] ,/EOF
     sets the file pointer to <offset> (default zero) bytes from the end-of-file
     marker.  The last Byte in the file is the target of FILEPTR,lun,1,/EOF.
   FILEPTR,lun [,offset] ,/ADVANCE
     advances the file pointer by <offset> (default zero) bytes.
 LS 11feb97 */
/* Headers:
   <stdio.h>: FILE, ftell(), printf(), fgetpos(), fseek(), fsetpos(), perror()
 */
{
  int32_t	lun, n, now, iq;
  off_t	offset;
  FILE	*fp;
  fpos_t	save;

  lun = int_arg(*ps++);		/* logical unit */
  if (lun < 0 || lun >= MAXFILES) /* illegal unit number */
    return function? LUX_ZERO: cerror(ILL_LUN, ps[-1]);
  if (!lux_file_open[lun])	/* no file open on unit */
    return function? LUX_ZERO: cerror(LUN_CLOSED, ps[-1]);
  fp = lux_file[lun];
  if (narg == 1 && internalMode == 0) { /* show file pointer */
    if (function) {
      iq = scalar_scratch(LUX_INT32);
      scalar_value(iq).l = ftell(fp);
      return iq;
    }
    printf("lun %1d: file pointer %ld\n", lun, ftell(fp));
    return 1;
  }
  /* if we get here then the file pointer must be changed */
  offset = narg > 1 ?int_arg(*ps): 0;
  fgetpos(fp, &save);		/* save current pointer position */
  now = ftell(fp);		/* current offset from start */
  fseek(fp, 0, SEEK_END);	/* determine length of file */
  n = ftell(fp);		/* file length */
  fsetpos(fp, &save);		/* to original pointer position */
  switch (internalMode & 7) {
    case 0:			/* none */
      if (offset < -n)
	return function? LUX_ZERO: luxerror("Illegal file offset value", *ps);
      if (offset >= 0)
	offset = offset - now;
      else if (offset < 0)
	offset = n + offset - now;
      break;
    case 1:			/* /START */
      if (offset < 0)
       return function? LUX_ZERO: luxerror("Illegal file offset value", *ps);
      offset = offset - now;
      break;
    case 2:			/* /EOF */
      if (offset < -n)
	return function? LUX_ZERO:  luxerror("Illegal file offset value", *ps);
      offset = n + offset - now;
      break;
    case 4:			/* /ADVANCE */
      if (offset + now < 0)
	return function? LUX_ZERO: luxerror("Illegal file offset value", *ps);
      break;
    default:
      return function? LUX_ZERO: cerror(INCMP_KEYWORD, -1);
  }
  n = fseek(fp, offset, SEEK_CUR);
  if (n) {
    if (function)
      return LUX_ZERO;
    perror("System message");
    return LUX_ERROR;
  }
  return function? LUX_ONE: 1;
}
/*------------------------------------------------------------------------- */
int32_t lux_fileptr(int32_t narg, int32_t ps[])
{
  return fileptr(narg, ps, (char) 0);
}
/*------------------------------------------------------------------------- */
int32_t lux_fileptr_f(int32_t narg, int32_t ps[])
{
  return fileptr(narg, ps, (char) 1);
}
/*------------------------------------------------------------------------- */
int32_t lux_dump_lun(int32_t narg, int32_t ps[])
/* lists the files associated with all used logical units */
/* LS 2mar97 */
/* Headers:
   <stdio.h>: printf()
 */
{
  int32_t	lun, n = 0;

  for (lun = 0; lun < MAXFILES; lun++)
  { if (!lux_file_open[lun])
      continue;
    if (!n)
      printf("lun type file\n");
    n++;
    printf("%3d %4s %s\n", lun, lux_file_open[lun] == 1? "r": "w/u",
	   lux_file_name[lun]); }
  if (!n)
    printf("No files open.\n");
  return 1;
}
/*------------------------------------------------------------------------- */
int32_t lux_get_lun(int32_t narg, int32_t ps[])
/* returns a free logical unit number */
{
  int32_t	i, n;

  for (i = 0; i < MAXFILES; i++)
    if (!lux_file_open[i]) {
      n = scalar_scratch(LUX_INT32);
      scalar_value(n).l = i;
      return n; 
    }
  /* if we get here then there were no free luns */
  return luxerror("No free LUNs available", 0);
}
/*------------------------------------------------------------------------- */
int32_t lux_chdir(int32_t narg, int32_t ps[])
/* CHDIR,name changes directory to <name>.  CHDIR shows the current
   directory.  LS 12aug97 */
/* Headers:
   <unistd.h>: chdir(), getcwd(), printf()
   <stdio.h>: perror()
 */
{
  char	*name;

  if (narg) {
    if (symbol_class(ps[0]) != LUX_STRING)
      return cerror(NEED_STR, ps[0]);
    name = expand_name(string_value(ps[0]), NULL);
    if (chdir(name)) {		/* didn't work */
      perror("CHDIR");
      return LUX_ERROR;
    }
  }
  if (!narg || internalMode & 1) { /* show current */
    name = getcwd((char *) scrat, NSCRAT);
    if (name == NULL) { /* something went wrong */
      perror("CHDIR");
      return LUX_ERROR;
    }
    printf("Current working directory: %s\n", name);
  }
  return 1;
}
/*------------------------------------------------------------------------- */
int32_t lux_filesize(int32_t narg, int32_t ps[])
/* FILESIZE(file) returns the size (in bytes) of the file, or -1 if the */
/* file cannot be opened for reading.  LS 10nov97 */
/* Headers:
   <stdio.h>: FILE, fopen(), fseek(), fclose()
 */
{
  FILE	*fp;
  int32_t	result;

  if (symbol_class(ps[0]) != LUX_STRING)
    return cerror(ILL_CLASS, ps[0]);
  fp = fopen(expand_name(string_arg(ps[0]), NULL), "r");
  result = scalar_scratch(LUX_INT32);
  if (fp) {
    fseek(fp, 0, SEEK_END);
    scalar_value(result).l = ftell(fp);
    fclose(fp);
  } else
    scalar_value(result).l = -1;
  return result;
}
/*------------------------------------------------------------------------- */
int32_t lux_freads(int32_t narg, int32_t ps[])
     /* FREADS,<string>,<format>,<arg1>,... reads arguments from the */
     /* string under guidance of the <format>. */
{
  if (symbol_class(ps[0]) != LUX_STRING)
    return cerror(ILL_CLASS, ps[0]);
  return read_formatted_ascii(narg - 1, &ps[1], (void *) string_value(ps[0]),
			      1, LUX_STRING) == LUX_ERROR? LUX_ERROR: LUX_OK;
}
/*------------------------------------------------------------------------- */
int32_t lux_freads_f(int32_t narg, int32_t ps[])
     /* FREADS(<string>,<format>,<arg1>,...) reads arguments from the */
     /* string under guidance of the <format>.  Returns 0 if an error occurred,
	1 otherwise.  LS 25jan99 */
{
  int32_t	result, iq;

  if (symbol_class(ps[0]) != LUX_STRING)
    return LUX_ZERO;
  result = read_formatted_ascii(narg - 1, &ps[1], (void *) string_value(ps[0]),
				0, LUX_STRING);
  switch (result) {
    case LUX_ERROR: case 0:
      return LUX_ZERO;
    case 1:
      return LUX_ONE;
    default:
      iq = scalar_scratch(LUX_INT32);
      scalar_value(iq).l = result;
      break;
  }
  return iq;
}
/*------------------------------------------------------------------------- */
int32_t lux_file_to_fz(int32_t narg, int32_t ps[])
/* FILETOFZ,file,type,dims  transforms a regular file with unformatted */
/* data to an uncompressed FZ file with the indicated type and dimensions, */
/* but only if the size of the file is approriate for the specified */
/* type and dimensions.  This routine assumes that you have reserved */
/* 512 bytes at the beginning of the file for the header information and */
/* that the data starts just after that.  LS 28apr98 */
/* Headers:
   <stdio.h>: FILE, fopen(), fseek(), ftell(), fwrite(), fclose()
   <string.h>: memcpy()
 */
{
  int32_t	i, nd, *dims, n, m, type, j;
  FILE	*fp;
  fzHead	*fh;

  if (symbol_class(ps[0]) != LUX_STRING)
    return cerror(NEED_STR, ps[0]);
  type = int_arg(ps[1]);
  if (type < LUX_INT8 || type > LUX_DOUBLE)
    return cerror(ILL_TYPE, ps[1]);
  switch (symbol_class(ps[2])) {
    case LUX_ARRAY:
      n = lux_long(1, &ps[2]);
      nd = array_size(n);
      dims = (int32_t*) array_data(n);
      break;
    case LUX_SCALAR:
      n = lux_long(1, &ps[2]);
      nd = 1;
      dims = &scalar_value(n).l;
      break;
    default:
      return cerror(ILL_CLASS, ps[2]);
  }

  fp = fopen(expand_name(string_value(*ps), ""), "r+");
  if (!fp)
    fp = fopen(expname, "w+");
  if (!fp)
    return cerror(ERR_OPEN, ps[0]);
  fseek(fp, 0, SEEK_END);
  n = ftell(fp);		/* length of the file, in bytes */
  fseek(fp, 0, SEEK_SET);
  n -= 512;			/* minus header length */
  
  m = lux_type_size[type];	/* get desired size */
  for (i = 0; i < nd; i++)
    m *= dims[i];
  if (n < m)
    return luxerror("File too small for desired type and dimensions", 0);

  fh = (fzHead *) scrat;
  zerobytes(fh, 512);		/*zero it first */
  fh->synch_pattern = SYNCH_OK;
  fh->subf = MSBfirst << 7;	/* no compression; indicate endian */
  fh->source = 0;
  fh->nhb = 1;			/*may be changed later */
  fh->datyp = type;
  fh->ndim = nd;
  memcpy(fh->dim, dims, nd*sizeof(int32_t));
  if (MSBfirst)
    endian(fh->dim, fh->ndim*sizeof(int32_t), LUX_INT32);
  j = fwrite(fh, 1, 512 * fh->nhb, fp); /*write header */
  fclose(fp);
  if (j != 512 * fh->nhb) {
    return cerror(WRITE_ERR, 0);
  }
  return 1;
}
/*------------------------------------------------------------------------- */
char* file_find(char const* sdir, char const* fname)
/* find a file given a starting directory, checks all sub-directories */
/* Headers:
   <regex.h>: regex_t
   <sys/types.h>: stat(), opendir(), readdir(), closedir()
   <dirent.h>: struct dirent, DIR, opendir(), readdir(), closedir()
   <sys/stat.h>: stat()
   <string.h>: strlen(), strcmp(), strlen(), strcpy(), strcat()
   <stdlib.h>: malloc(), free()
   <unistd.h>: getcwd()
   <stdio.h>: NULL
 */
{
  DIR	*dirp;
  struct dirent *dp;
  struct stat	statbuf;
  int32_t	n, mq;
  char	*sq, *s2, *s3;

  n = strlen(fname);
  dirp = opendir(sdir);		/* open directory */
  if (!dirp)
    return NULL;		/* can't read directory or something,
				   not found */
  while ((dp = readdir(dirp)) != NULL) {
    if (strcmp(dp->d_name, ".") == 0)
      continue;
    if (strcmp(dp->d_name, "..") == 0)
      continue;

    if (!strcmp(dp->d_name, fname)) {
      closedir(dirp);  		/* got it !, make the whole name */
      mq = n + strlen(sdir) + 2;
      sq = (char *) malloc(mq);
      strcpy(sq, sdir);
      strcat(sq, "/");
      strcat(sq, fname);
      /* a few things that we could make optional, first replace a
	 leading . with the full default path */
      if (sq[0] == '.') {
	s2 = (char *) malloc(PATH_MAX);
	getcwd(s2, PATH_MAX);
	n = strlen(s2);
	mq = n + strlen(sq) + 1;
	s3 = (char *) malloc(mq);
	strcpy(s3, s2);
	strcat(s3, sq + 1);
	free(s2);
	free(sq);
	sq = s3;
      }
      return sq;
    }

    /* append the file */
    mq = strlen(dp->d_name) + strlen(sdir) + 3;
    sq = (char *) malloc(mq);
    /* put in a /, extras are OK if there was one already */
    strcpy(sq, sdir);
    strcat(sq, "/");
    strcat(sq, dp->d_name);

    /* stat the combined file name and check if a directory */

    /* printf("checking if %s is a directory\n", sq); */
    if(stat(sq, &statbuf) != 0) {
      printf("stat error for file: %s\n", sq);
    } else
      if ((statbuf.st_mode & S_IFMT) == S_IFDIR) { /* got a directory */
	/* call ourselves for the next level */
	char* q = file_find(sq, fname);
	/* did we find it down there ? */
	if (q) {
	  free(sq);
	  closedir(dirp);
	  return q;
	}
      }
    /* no joy, free sq */
    free(sq);
  }

  closedir(dirp);
  return NULL;
}
/*------------------------------------------------------------------------- */
int32_t lux_findfile(int32_t narg, int32_t ps[])
/* find a file given a starting path */
/* Headers:
   <string.h>: strlen(), strchr(), printf(), NULL, strcpy()
   <stdlib.h>: free()
 */
{
  char const* fname;
  char const* current_dir = ".";
  char const* startpath;
  char* pq;
  char const* cc;
  int32_t	ns, result_sym;

  /* first argment is a string with the start path */
  if (!symbolIsStringScalar(ps[0]))
    return cerror(NEED_STR, ps[0]);
  startpath = string_value(ps[0]);

  if (strlen(startpath) <= 0)
    startpath = current_dir;

  /* second is the file name */
  if (!symbolIsStringScalar(ps[1]))
    return cerror(NEED_STR, ps[1]);
  fname = string_value(ps[1]);

  /* check if file name has any /'s in it, we want it to be simple */
  if ((cc = strchr(fname, '/')) != 0) {
    printf("WARNING - path stripped from file name input to findfile\n");
    printf("input name: %s becomes %s\n", fname, cc);
    fname = cc;
  }

  pq = file_find(startpath, fname);
  /* pq will have the entire path+name if it was found */
  if (pq == NULL)
    return LUX_ZERO;		/* return symbol for 0 */
  /* NOTE: LHS changed "return 0" to "return LUX_ZERO" above because 0 should
     not be used as a return value in LHS LUX. */
  ns = strlen(pq);
  result_sym = string_scratch(ns);
  strcpy(string_value(result_sym), pq);
  free(pq);
  return result_sym;
}
/*------------------------------------------------------------------------- */
/* NOTE: REG_BASIC is defined in BSD (e.g., SGI irix) regexp.h but not
   in POSIX (e.g., linux).  Under POSIX, basic behavior results if
   REG_EXTENDED is *not* selected, so we can define REG_BASIC to be
   equal to zero if it is not already defined.  LS 21sep98 */
static int32_t	nfiles = 0, max;
static char	**p;
#if HAVE_REGEX_H
static regex_t	re;
#ifndef REG_BASIC
#define REG_BASIC	0
#endif
int32_t lux_getmatchedfiles(int32_t narg, int32_t ps[])/* get all file names that match */
 /* call is:  strings = getmatchedfiles( expr, path, [maxfiles] ) */
 /* uses regular expression matches, not the more familiar shell wildcarding,
 this is more general but not as intuitive, it uses the Unix routines
 regcomp and regexec
 used to be re_comp and re_exec */
/* Headers:
   <regex.h>: regcomp()
   <sys/types.h>: regcomp(), regfree()
   <stdio.h>: printf()
 */
{
  int32_t	result_sym;
  char	*s;
  int32_t	lux_getfiles_m(int32_t, int32_t [], int32_t);

  /* the first arg is the regular expression string */
  if (!symbolIsString(ps[0]))
    return cerror(NEED_STR, ps[0]);
  s = string_value(ps[0]);
  /* printf("expression: %s\n", s); */
  if (regcomp(&re, s, REG_BASIC | REG_NOSUB)) {
    printf("GETMATCHEDFILES - error in regular expression: %s\n", s);
    regfree(&re);
    return LUX_ERROR;
  }
  result_sym = lux_getfiles_m(narg - 1, &ps[1], 1);
  regfree(&re);
  return result_sym;
}
#endif
 /*------------------------------------------------------------------------- */
#if HAVE_REGEX_H
int32_t lux_getmatchedfiles_r(int32_t narg, int32_t ps[])	/* also search subdir */
 /* call is:  strings = getmatchedfiles_r( expr, path, [maxfiles] ) */
{
  int32_t	result_sym;

  recursive_flag = 1;
  result_sym = lux_getmatchedfiles(narg, ps);
  recursive_flag = 0;
  return result_sym;
}
#endif
 /*------------------------------------------------------------------------- */
#if HAVE_REGEX_H
int32_t file_get(char *startpath, int match_flag)
/* Headers:
   <sys/types.h>: stat(), opendir(), readdir(). regexec(), closedir()
   <sys/stat.h>: stat(), struct stat
   <dirent.h>: opendir(), DIR, struct dirent, readdir(), closedir()
   <stdio.h>: printf()
   <string.h>: strcmp(), strlen(), strcpy(), strcat()
   <regex.h>: regexec()
   <stdlib.h>: malloc(), free()
 */
{
  int32_t	mq, matches;
  char	*sq;
  struct stat	statbuf;
  DIR	*dirp;
  struct dirent *dp;

  dirp = opendir(startpath);	/* open directory */
  if (!dirp) {			/* can't read directory or something,
				   not found */
    printf("GETFILES - can't read directory: %s\n", startpath);
    /* closedir(dirp); */
    return 1;			/* go on, this happens a lot when searching */
  }

  while ((dp = readdir(dirp)) != NULL) {
    /* printf("dp->d_name: %s\n", dp->d_name); */
    if (strcmp(dp->d_name, ".") == 0)
      continue;
    if (strcmp(dp->d_name, "..") == 0)
      continue;

    /* we don't stat the file unless we have to, we have to if it is
       a potential match or if we are checking for sub-directories */
    
    /* are we matching to a regular expression ? */
    matches = 1;		/* everybody a match for get all case */
    if (match_flag) {
      /* apparently, check if we match */
      if( regexec(&re, dp->d_name, 0, NULL, 0) == 0)
	matches = 1;
      else
	matches = 0;
      if (!matches && !recursive_flag)
	continue;
    }
  
    /* make a full path to file so we can stat it */
    mq = strlen(dp->d_name) + strlen(startpath) + 2;
    sq = (char *) malloc(mq);
    /* put in a /, extras are OK if there was one already */
    strcpy(sq, startpath);
    strcat(sq, "/");
    strcat(sq, dp->d_name);
    /* printf("checking if %s is a regular file\n", sq); */
    if (stat(sq, &statbuf) != 0)
      printf("GETFILES - stat error for file: %s\n", sq);
    else
      /* check if a directory */
      if (recursive_flag && (statbuf.st_mode & S_IFMT) == S_IFDIR) {
	/* and call ourselves for the next level */
	if (file_get(sq, match_flag) != 1) {
	  closedir(dirp);
	  return LUX_ERROR;
	}
      } else if (matches && (statbuf.st_mode & S_IFMT) == S_IFREG) {
	nfiles++;
	/* printf("nfiles = %d\n", nfiles); */
	if (nfiles >= max) {
	  nfiles--;
	  printf("GETFILES - found too many files, max = %d\n", max);
	  free(sq);
	  break;
	}
	*p++ = sq;
      } else
	free(sq);
  }
  closedir(dirp);
  return 1;
}
#endif
 /*------------------------------------------------------------------------- */
int32_t directs_get(char *startpath)
/* Headers:
   <dirent.h>: DIR, struct dirent, opendir(), readdir(), closedir()
   <sys/types.h>: opendir(), readdir(), stat(), closedir()
   <stdio.h>: printf()
   <string.h>: strcmp(), strlen(), strcpy(), strcat()
   <stdlib.h>: malloc(), free()
   <sys/stat.h>: stat()
 */
{
  int32_t	mq;
  char	*sq;
  struct stat statbuf;
  DIR	*dirp;
  struct dirent *dp;

  dirp = opendir(startpath);	  /* open directory */
  if (!dirp) {  /* can't read directory or something, not found */
    printf("GETFILES - can't read directory: %s\n", startpath);
    /* closedir(dirp); */
    return 1;			/* go on, this happens a lot when searching */
  }

  while ((dp = readdir(dirp)) != NULL) {
    /* printf("dp->d_name: %s\n", dp->d_name); */
    if (strcmp(dp->d_name, ".") == 0) continue;
    if (strcmp(dp->d_name, "..") == 0) continue;

    /* we have to stat the file since we are checking for sub-directories */
  
    /* make a full path to file so we can stat it */
    mq = strlen(dp->d_name) + strlen(startpath) + 2;
    sq = (char *) malloc(mq);
    /* put in a /, extras are OK if there was one already */
    strcpy(sq, startpath);
    strcat(sq, "/");
    strcat(sq, dp->d_name);
    if (stat(sq, &statbuf) != 0)
      printf("GETFILES - stat error for file: %s\n", sq);
    else
      /* check if a directory */
      if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
	nfiles++;
	if (nfiles >= max) {
	  nfiles--;
	  printf("GETFILES - found too many directories, max = %d\n", max);
	  free(sq);
	  break;
	}
	/* for directories, we don't really want to pass the whole path to the
	   usr (he/she already has the starting path anyhow) but we want to
	   attach a / to the end */
	free(sq);
	mq = strlen(dp->d_name) + 2;
	sq = (char *) malloc(mq);
	strcpy(sq, dp->d_name);
	strcat(sq, "/");
	*p++ = sq;
      } else
	free(sq);
  }
  closedir(dirp);
  return 1;
}
 /*------------------------------------------------------------------------- */
#if HAVE_REGEX_H
/* lux_getfiles_r() calls lux_getfiles() which calls file_get() which uses */
/* <regex.h> */
int32_t lux_getfiles_r(int32_t narg, int32_t ps[])	/* also search subdir */
 /* call is:  strings = getfiles_r( path, [maxfiles] ) */
{
  int32_t	result_sym;
  int32_t	lux_getfiles(int32_t, int32_t []);

  recursive_flag = 1;
  result_sym = lux_getfiles(narg, ps);
  recursive_flag = 0;
  return result_sym;
}
#endif
/*------------------------------------------------------------------------- */
#if HAVE_REGEX_H
/* lux_getfiles() calls file_get() which uses <regex.h> */
int32_t lux_getfiles_m(int32_t narg, int32_t ps[], int32_t match_flag)
/* get all file names in directory */
/* call is:  strings = getfiles( path, [maxfiles] ) */
/* Headers:
   <stdio.h>: printf()
   <stdlib.h>: malloc(), free()
 */
{
  char	*startpath;
  int32_t	result_sym, dim[1], mq, malloc_flag = 0, status;
  char	**names;

  /* first argment is a string with the start path */
  if (!symbolIsString(ps[0]))
    return cerror(NEED_STR, 70);
  startpath = expand_name(string_value(ps[0]), NULL);
  nfiles = 0;
  /* the second optional arg. is the max # of files to find */
  max = 100;
  if (narg > 1 && int_arg_stat(ps[1], &max) != 1)
    return LUX_ERROR;
  /* get space to store pointers to copies of names, use SCRAT if big enough */
  mq = max * sizeof (char *);
 /* printf("mq = %d, NSCRAT * sizeof(int32_t) = %d\n", mq, NSCRAT * sizeof(int32_t));*/
  if (mq <= NSCRAT * sizeof(int32_t))
    names = (char **) scrat;
  else {
    malloc_flag = 1;
    printf("too big for SCRAT\n");
    if ((names = (char **) malloc(mq)) == NULL)
      return luxerror("GETFILES - malloc failure for names array pointer, max = %d\n", max);
  }

  p = names;

  if (get_type_flag == 0)
    status = file_get(startpath, match_flag);
  else
    status = directs_get(startpath);
  /* reset the flag to default here so we needn't do it in getdirectories */
  get_type_flag = 0;
  if (status == 1) {
    /* make a strarr to match # of found files */
    /* printf("nfiles = %d\n", nfiles); */
    if (nfiles <= 0) {
      printf("GETFILES - no files found\n");
      return LUX_ZERO;		/* changed 0 to LUX_ZERO, LS 21jan99 */
    }
    *dim = nfiles;
    result_sym = array_scratch(LUX_STRING_ARRAY, 1, dim);
    p = array_data(result_sym);
    while (nfiles--)
      *p++ = *names++;
  } else
    result_sym = 0;		/* error case */

  if (malloc_flag)
    free(names);
  return result_sym;
}
 /*------------------------------------------------------------------------- */
int32_t lux_getfiles(int32_t narg, int32_t ps[], int32_t match_flag)
{
  return lux_getfiles_m(narg, ps, 0:
}
#endif
 /*------------------------------------------------------------------------- */
#if HAVE_REGEX_H
/* lux_getdirectories() calls lux_getfiles() which calls file_get() which */
/* uses <regex.h> */
int32_t lux_getdirectories(int32_t narg, int32_t ps[], int match_flag)
/* get all subdirectories in directory */
/* call is:  strings = getdirectories( path, [maxfiles] ) */
{
  /* very similar to getfiles, just set a flag here */
  get_type_flag = 1;
  return lux_getfiles(narg, ps);
}
#endif
/*------------------------------------------------------------------------- */
int32_t lux_identify_file(int32_t narg, int32_t ps[])
/* tries to identify the type of a file based on the values of its
 first four bytes (its "magic number") LS 21sep98 */
/* Headers:
   <stdio.h>: FILE, fopen(), fread(), ferror(), feof(), fclose(), printf()
 */
{
  char	*name;
  uint8_t	buf[4];
  FILE	*fp;
  int32_t	type, result;

  if (!symbolIsStringScalar(ps[0]))
    return cerror(NEED_STR, ps[0]);
  name = string_value(ps[0]);
  fp = fopen(expand_name(name, NULL), "r");
  if (!fp)
    return cerror(ERR_OPEN, ps[0]);
  fread(buf, 1, 4, fp);
  if (ferror(fp) || feof(fp))
    return luxerror("Could not read 4 bytes from file %s\n", ps[0], expname);

  type = 0;
  switch (buf[0]) {
    case 0:			/* Targa 24-bit? */
      if (buf[1] == 0 && buf[2] == 2 && buf[3] == 0)
	type = FILE_TYPE_TARGA_24;
      break;
    case 1:			/* IRIS RGB? */
      if (buf[1] == 218 && buf[2] == 1 && buf[3] == 1)
	type = FILE_TYPE_IRIS_RGB;
      break;
    case 35:			/* X11 bitmap? */
      if (buf[1] == 100 && buf[2] == 101 && buf[3] == 102)
	type = FILE_TYPE_XBM;
      break;
    case 47:
      if (buf[1] == 42 && buf[2] == 32 && buf[3] == 88)
	type = FILE_TYPE_XPM;
      break;
    case 66:
      if (buf[1] == 77 && buf[2] == 206 && buf[3] == 1)
	type = FILE_TYPE_BMP;
      break;
    case 71:			/* GIF file? */
      if (buf[1] == 73 && buf[2] == 70 && buf[3] == 56)
	type = FILE_TYPE_GIF;
      break;
    case 77:			/* TIFF file? */
      if (buf[1] == 77 && buf[2] == 0 && buf[3] == 42)
	type = FILE_TYPE_TIFF;
      break;
    case 80:			/* PBM/PGM/PPM file? */
      switch (buf[1]) {
	case 54:
	  if (buf[2] == 10 && buf[3] == 35)
	    type = FILE_TYPE_PPM_RAW;
	  break;
	case 51:
	  if (buf[2] == 10 && buf[3] == 35)
	    type = FILE_TYPE_PPM_ASCII;
	  break;
      }
      break;
    case 83:
      switch (buf[1]) {
	case 73:		/* FITS */
	  if (buf[2] == 77 && buf[3] == 80)
	    type = FILE_TYPE_FITS;
	  break;
	case 82:		/* IDL Save */
	  if (buf[2] == 0 && buf[3] == 4)
	    type = FILE_TYPE_IDL_SAVE;
	  break;
      }
      break;
    case 86:			/* PM? */
      if (buf[1] == 73 && buf[2] == 69 && buf[3] == 87)
	type = FILE_TYPE_PM;
      break;
    case 89:			/* Sun raster? */
      if (buf[1] == 166 && buf[2] == 106 && buf[3] == 149)
	type = FILE_TYPE_SUN_RAS;
      break;
    case 102:			/* LUX astore file? */
      if (buf[1] == 102 && buf[2] == 170 && buf[3] == 170)
	type = FILE_TYPE_ANA_ASTORE;
      break;
    case 170:			/* LUX fz? */
      if (buf[1] == 170 && buf[2] == 85 && buf[3] == 85)
	type = FILE_TYPE_ANA_FZ;
      break;
    case 255:			/* JPEG? */
      if (buf[1] == 216 && buf[2] == 255 && buf[3] == 224)
	type = FILE_TYPE_JPEG;
      break;
  }

  fclose(fp);

  if (type) {
    result = scalar_scratch(LUX_INT32);
    scalar_value(result).l = type;
    return result;
  }

  printf("File %s has an unknown file type\n", expname);
  printf("Magic numbers: %d-%d-%d-%d\n", buf[0], buf[1], buf[2], buf[3]);
  return LUX_ZERO;
}
/*------------------------------------------------------------------------- */
void printw(char const* string)
/* prints the string to the screen, but tries to break the line outside
   of alphanumerical words.  Added printwLines which counts the number
   of lines (newlines) printed so far by printw.  LS 12jul95 15oct98 */
{
  char const* p0;
  char const* p;
  char const* pn;
  char const* p2;
  int32_t	n, a, keepws, toolong;
  extern char	*cl_eos;
  extern int32_t	uTermCol;

  n = strlen(string);
  p = string;
  /* if the first character is whitespace (but no newline), then we
   keep whitespace at the beginning of screen lines; otherwise we
   discard it. */
  keepws = (isspace((uint8_t) *p) && *p != '\n');
  pn = strpbrk(p, "\n\r");	/* find next newline or return */
  while (n) {			/* while we have more characters to print */
    toolong = (uTermCol > 0 && n + column >= uTermCol);
    if (toolong)		/* current line is too long for screen */
      p2 = p + (uTermCol - column - 1); /* proposed break point */
    else
      p2 = p + n;
    p0 = p2;
    if (pn && pn < p2) {	/* have an \r or \n before proposed break */
      /* we must break there */
      p2 = pn + 1;
      pn = strpbrk(p2, "\n\r");	/* next newline or return */
    } else if (isspace((uint8_t) p2[-1]) && keepws) {
      /* if <keepws> is set, then we only want breakpoints such that there
	 is no whitespace directly before the breakpoint.  This is suitable
	 for printing lists of numbers in formats with a fixed field width;
	 they'll then line up nicely in columns, even if the screen width
	 is not some simple multiple of the field width. */
      p2--;
      while (p2 > p && isspace((uint8_t) p2[-1]))
	p2--;
      if (p2 == p)		/* the whole line is whitespace: use
				   the original breakpoint. */
	p2 = p0;
    }
    p0 = p2;			/* save */
    /* we now have a tentative break point.  We allow breakpoints between
       non-whitespace characters and whitespace characters only.  If the
       tentative breakpoint does not comply, then we back up until we find
       a place that does.  If there isn't any, then we accept the tentative
       one. */
    if (p2 > p			/* not at start of string */
	&& *p2			/* not at end of string */
	&& p2[-1] != '\n'	/* not just after a \n */
	&& p2[-1] != '\r'	/* not just after a \r */
	&& (!isspace((uint8_t) *p2) /* not just before whitespace */
	    || isspace((uint8_t) p2[-1]))) { /* or just after whitespace */
      p2--;
      while ((isspace((uint8_t) p2[-1]) || !isspace((uint8_t) *p2))
	     && p2 > p)		/* seek a better place to break... */
	p2--;			/* ... earlier in the line */
    }
    if (p2 == p			/* did not find a better one */
	&& !column)		/* and we're already at the beginning of
				   a line */
      p2 = p0;			/* just use old one */
    if (!keepws && !column) 	/* we're not interested in whitespace;
				   if we have any at the start of the line
				   then we skip it. */
      while (isspace((uint8_t) *p) && *p != '\n') {
	p++;
	n--;
      }
    if (p2[-1] == '\n' || p2[-1] == '\r') {
      printf("%.*s", p2 - p, p); /* no extra \n at end */
      column = 0;
      printwLines++;
    } else if (toolong) {
      printf("%.*s\n", p2 - p, p); /* extra \n at end */
      column = 0;
      printwLines++;
    } else {
      p2 = p + n;
      fputs(p, stdout);
      column += n;
    }
    n -= (p2 - p);
    p = p2;
    if (!column && printwLines % (page - 2) == pager) {
      printf(":: Press RET for next line, any other key for next page...");
      fflush(stdout);
      a = getSingleStdinChar();
      printf("\r%s\r", cl_eos);	/* clear the line */
      if (a == '\n')
	setPager(1);
    }
  }
}
/*------------------------------------------------------------------------- */
void fprintw(FILE *fp, char const* string)
/* output to screen or file; avoiding line breaks inside words if outputting
   to the screen (when <fp> == stdout or <fp> == NULL).  LS 15oct98 */
/* Headers:
   <stdio.h>: FILE, stdout
 */
{
  if (!fp)
    fp = stdout;
  if (fp == stdout)
    printw(string);
  else
    fputs(string, fp);
}
/*------------------------------------------------------------------------- */
void vfprintfw(FILE *fp, char const* format, va_list ap)
/* send output to file fp, or to stdout (if fp is zero or equal to stdout).
   if outputting to the screen, then tries to avoid breaking lines in the
   middle of words.  LS 15oct98 */
/* Headers:
   <stdarg.h>: va_list, vsprintf(), vfprintf()
   <stdio.h>: FILE, stdout, vsprintf(), vfprintf()
 */
{
  if (!fp)
    fp = stdout;
  if (fp == stdout) {
    vsprintf(curScrat, format, ap); /* put formatted sentence in curScrat */
    printw(curScrat);
  } else
    vfprintf(fp, format, ap);
}
/*------------------------------------------------------------------------- */
void printfw(char const* format, ...)
/* print to standard output, avoiding line breaks inside words.  LS 15oct98 */
/* Headers:
   <stdarg.h>: va_list, va_start(), va_end()
 */
{
  va_list	ap;

  va_start(ap, format);
  vfprintfw(stdout, format, ap);
  va_end(ap);
}
/*------------------------------------------------------------------------- */
int32_t lux_hex(int32_t narg, int32_t ps[])
/* a temporary routine that types arg in hex notation, until we have
   a fully formatted print */
/* Headers:
   <stdio.h>: FILE, stdout, fprintf()
 */
{
  FILE	*fp;
  register int32_t	nelem;
  register union types_ptr p1;
  register int32_t	j;
  int32_t i,k,iq,jq,flag=0;
  char	*ptr;

  fp = stdout;
  for (i=0;i<narg;i++) {
    iq = ps[i];
    if (iq <=0 ) return LUX_ERROR;
    switch (symbol_class(iq))	{		
      case LUX_UNDEFINED: return cerror(ILL_CLASS, iq);
      case LUX_SCALAR:		/*scalar case */
	switch (sym[iq].type) {
	  case 0: fprintf(fp, "      %#4x",sym[iq].spec.scalar.b); break;
	  case 1: fprintf(fp, "    %#6x",sym[iq].spec.scalar.w); break;
	  case 2: fprintf(fp, "%#10x",sym[iq].spec.scalar.l); break;
	  case 3: fprintf(fp, "%#10x",sym[iq].spec.scalar.f); break;
	  case 4: fprintf(fp, "%#20x",sym[iq].spec.scalar.d); break;
	} flag=1; break;			/*end of scalar case */
      case LUX_SCAL_PTR:	/*scalar ptr case */
	switch (sym[iq].type) {
	  case 0: fprintf(fp, "      %#4x",*(uint8_t *)sym[iq].spec.array.ptr); break;
	  case 1: fprintf(fp, "    %#6x",*(short *)sym[iq].spec.array.ptr); break;
	  case 2: fprintf(fp, "%#10x",*(int32_t *)sym[iq].spec.array.ptr); break;
	  case 3: fprintf(fp, "%#10x",*(float *)sym[iq].spec.array.ptr); break;
	  case 4: fprintf(fp, "%#20x",*(double *)sym[iq].spec.array.ptr); break;
	} flag=1; break;			/*end of scalar case */
      case LUX_STRING:		/*string */
	ptr = (char *) sym[iq].spec.array.ptr;
	if ( sym[iq].spec.array.bstore < 3) fprintf(fp, "%.1s",ptr); else
	  fprintf(fp, "%s",ptr); flag=1; break;
      case LUX_ARRAY:		/*array case */
    nelem = array_size(iq);
    ptr = (char*) array_data(iq);
    jq = sym[iq].type;
    if (flag) fprintf(fp, "\n");	flag=0;
    /*print entire array, number per line depends on type */
	    switch (jq) {
	      case 2:  p1.l = (int32_t *)ptr; k=8; for (j=0;j<nelem;j++)
	      { fprintf(fp, "%#10x",*p1.l++); if (j%8 == 7) fprintf(fp, "\n");}  break;
	      case 3:  p1.f = (float *)ptr; k=6; for (j=0;j<nelem;j++)
	      { fprintf(fp, "%#10x",*p1.f++); if (j%6 == 5) fprintf(fp, "\n");}  break;
	      case 0:  p1.b = (uint8_t *)ptr; k=8; for (j=0;j<nelem;j++)
	      { fprintf(fp, "      %#4x",*p1.b++); if (j%8 == 7) fprintf(fp, "\n");}  break;
	      case 1:  p1.w = (short *)ptr; k=8; for (j=0;j<nelem;j++)
	      { fprintf(fp, "    %#6x",*p1.w++); if (j%8 == 7) fprintf(fp, "\n");}  break;
	      case 4:  p1.d = (double *)ptr; k=4; for (j=0;j<nelem;j++)
	      { fprintf(fp, "%#20x",*p1.d++); if (j%4 == 3) fprintf(fp, "\n");}  break;
	    } if ( nelem%k != 0 ) fprintf(fp, "\n"); break;	/*end of array class */
    }						/*end of class switch */
  }						/*end of loop over args */
  if (flag) fprintf(fp, "\n");
  return 1;
}							/*end of lux_type */
/*------------------------------------------------------------------------- */
#define	HEAD_LIMIT	100
int32_t fits_problems(int32_t i)
/* Headers:
   <stdio.h>: printf();
 */
{
  printf("There was a problem with your FITS file\n");
  switch (i) {
    case 1: printf("found > 1 BITPIX keys, using first one\n"); break;
    case 2: printf("illegal BITPIX key\n"); break;
    case 3: printf("found > 1 NAXIS keys, using first one\n"); break;
    case 4: printf("could not decode BITPIX value\n"); break;
    case 5: printf("could not decode NAXIS value\n"); break;
    case 6: printf("header block limit of %d exceeded\n", HEAD_LIMIT); break;
    case 7: printf("could not malloc for extended header\n"); break;
    case 8: printf("could not decode NAXISn key\n"); break;
    case 9: printf("could not create header\n"); break;
    case 10: printf("could not create data array\n"); break;
    case 11: printf("error reading fits header\n"); break;
    case 12: printf("error reading fits data array\n"); break;
    case 13: printf("error positioning to extension header\n"); break;
    case 14: printf("extension header block limit of %d exceeded\n", HEAD_LIMIT); break;
    case 15: printf("could not malloc for large extension header\n"); break;
    case 16: printf("error reading fits extension header\n"); break;
    case 17: printf("XTENSION keyword not found\n"); break;
    case 18: printf("could not create extension header\n"); break;
    case 19: printf("could not create extenesion data array\n"); break;
    case 20: printf("error reading fits extension data array\n"); break;
    case 21: printf("could not decode TFIELDS value\n"); break;
    case 22: printf("could not decode GCOUNT value\n"); break;
    case 23: printf("could not decode TFORM key value in extension\n"); break;
    case 24: printf("could not decode TTYPE key value in extension\n"); break;
    case 25: printf("extension column number > tfields\n"); break;
    default: printf("undocumented FITS error\n"); break;
  }
  return 1;
}
/*------------------------------------------------------------------------- */
char *bigger_header(int32_t n, char *head)
/* Headers:
   <stdlib.h>: malloc(), free()
   <string.h>: memmove()
 */
{
  /* we want 2880*n bytes */
  int32_t	nb;
  char	*p;
  
  nb = n*2880; 
  if (nb <= scratSize())
    p = curScrat;
  else {
    /* need to malloc a larger space and copy over */
    p = (char *) malloc(nb);
    if (p == NULL)
      return p;
    memmove(p, head, nb);
    /* if previous was a malloc, free it now */
    if (fits_head_malloc_flag) free(head);
    fits_head_malloc_flag = 1;
  }
  return p;
}
/*------------------------------------------------------------------------- */
int32_t fits_fatality(FILE *fin)
/* Headers:
   <stdio.h>: printf(), fclose()
   <stdlib.h>: free()
 */
{
  printf("FITS problem was fatal, sorry\n");
  if (fits_head_malloc_flag)
    free(fitshead);
  fclose(fin);
  return LUX_ZERO;	/* this is the zero symbol */
}
/*------------------------------------------------------------------------- */
void apply_bscale_bzero_blank(uint8_t *ptr, int32_t nelem, float bscale, float bzero,
			      float blank, float targetblank, int32_t type0,
			      int32_t type)
{
  Pointer	p, q;

  if ((bscale && bscale != 1.0) || bzero) {
    /* we must adjust the values for BSCALE, BZERO, and BLANK */
    if (!bscale)
      bscale = 1.0;
    q.b = ptr + lux_type_size[type]*(nelem - 1); /* final data */
    p.b = ptr + lux_type_size[type0]*(nelem - 1); /* raw data */
    /* we must start at the end so that we don't overwrite data that */
    /* we still need */
    switch (type) {
      case LUX_INT8:
	switch (type0) {
	  case LUX_INT8:
	    while (nelem--) {
	      *q.b = (*p.b == blank)? targetblank: *p.b * bscale + bzero;
	      q.b--;
	      p.b--;
	    }
	    break;
	  case LUX_INT16:
	    while (nelem--) {
	      *q.b = (*p.w == blank)? targetblank: *p.w * bscale + bzero;
	      q.b--;
	      p.w--;
	    }
	    break;
	  case LUX_INT32:
	    while (nelem--) {
	      *q.b = (*p.l == blank)? targetblank: *p.l * bscale + bzero;
	      q.b--;
	      p.l--;
	    }
	    break;
	  case LUX_INT64:
	    while (nelem--) {
	      *q.b = (*p.q == blank)? targetblank: *p.q * bscale + bzero;
	      q.b--;
	      p.q--;
	    }
	    break;
	}
	break;
      case LUX_INT16:
	switch (type0) {
	  case LUX_INT8:
	    while (nelem--) {
	      *q.w = (*p.b == blank)? targetblank: *p.b * bscale + bzero;
	      q.w--;
	      p.b--;
	    }
	    break;
	  case LUX_INT16:
	    while (nelem--) {
	      *q.w = (*p.w == blank)? targetblank: *p.w * bscale + bzero;
	      q.w--;
	      p.w--;
	    }
	    break;
	  case LUX_INT32:
	    while (nelem--) {
	      *q.w = (*p.l == blank)? targetblank: *p.l * bscale + bzero;
	      q.w--;
	      p.l--;
	    }
	    break;
	  case LUX_INT64:
	    while (nelem--) {
	      *q.w = (*p.q == blank)? targetblank: *p.q * bscale + bzero;
	      q.w--;
	      p.q--;
	    }
	    break;
	}
	break;
      case LUX_INT32:
	switch (type0) {
	  case LUX_INT8:
	    while (nelem--) {
	      *q.l = (*p.b == blank)? targetblank: *p.b * bscale + bzero;
	      q.l--;
	      p.b--;
	    }
	    break;
	  case LUX_INT16:
	    while (nelem--) {
	      *q.l = (*p.w == blank)? targetblank: *p.w * bscale + bzero;
	      q.l--;
	      p.w--;
	    }
	    break;
	  case LUX_INT32:
	    while (nelem--) {
	      *q.l = (*p.l == blank)? targetblank: *p.l * bscale + bzero;
	      q.l--;
	      p.l--;
	    }
	    break;
	  case LUX_INT64:
	    while (nelem--) {
	      *q.l = (*p.q == blank)? targetblank: *p.q * bscale + bzero;
	      q.l--;
	      p.q--;
	    }
	    break;
	}
	break;
      case LUX_FLOAT:
	switch (type0) {
	  case LUX_INT8:
	    while (nelem--) {
	      *q.f = (*p.b == blank)? targetblank: *p.b * bscale + bzero;
	      q.f--;
	      p.b--;
	    }
	    break;
	  case LUX_INT16:
	    while (nelem--) {
	      *q.f = (*p.w == blank)? targetblank: *p.w * bscale + bzero;
	      q.f--;
	      p.w--;
	    }
	    break;
	  case LUX_INT32:
	    while (nelem--) {
	      *q.f = (*p.l == blank)? targetblank: *p.l * bscale + bzero;
	      q.f--;
	      p.l--;
	    }
	    break;
	  case LUX_INT64:
	    while (nelem--) {
	      *q.f = (*p.q == blank)? targetblank: *p.q * bscale + bzero;
	      q.f--;
	      p.q--;
	    }
	    break;
	  case LUX_FLOAT:
	    while (nelem--) {
	      *q.f = *p.f * bscale + bzero;
	      q.f--;
	      p.f--;
	    }
	    break;
	}
	break;
      case LUX_DOUBLE:
	while (nelem--) {
	  *q.d = *p.d * bscale + bzero;
	  q.d--;
	  p.d--;
	}
	break;
    }
  }
}
/*------------------------------------------------------------------------- */
int32_t fits_read(int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, float);
int32_t lux_fits_read_general(int32_t narg, int32_t ps[], int32_t func)/* read fits files */
 /* status = fits_read(x, name, [h], [x2], [h2], [extvar_preamble]) */
{
  int32_t	hsym = 0, mode, xhsym=0, xdsym =0;
  float	targetblank = 0.0;
 /* uses fits_read, mode depends on arguments */
 /* mode mask uses bits to specify products to return
 1 bit: main header, 2 bit: main array, 4 bit: offset value
 8 bit: ext offset value, 16 bit: ext header, 32 bit: ext array
 64 bit: generate extension variables */

  mode = 2;
  if (narg >= 3) {
    mode = 3;
    hsym = ps[2];
  }
  if (narg >= 4) {
    mode = mode + 32;
    xdsym = ps[3];
  }
  if (narg >= 5) {
    mode = mode + 16;
    xhsym = ps[3];
  }
  if (narg >= 6) {
    mode = mode + 64; 
    if (!symbolIsStringScalar(ps[5]))
      return func? LUX_ZERO: cerror(NEED_STR, ps[5]);
    preamble = string_value(ps[5]);
  }
  if (narg >= 7)
    targetblank = float_arg(ps[6]);
  mode = fits_read(mode, ps[0], ps[1], hsym, 0, 0, xhsym, xdsym, targetblank);
  if (func)
    return (mode == LUX_ERROR)? LUX_ZERO: LUX_ONE;
  else
    return mode;
}
/*------------------------------------------------------------------------- */
int32_t lux_fits_read(int32_t narg, int32_t ps[])
{
  return lux_fits_read_general(narg, ps, 0);
}
/*------------------------------------------------------------------------- */
int32_t lux_fits_read_f(int32_t narg, int32_t ps[])
{
  return lux_fits_read_general(narg, ps, 1);
}
/*------------------------------------------------------------------------- */
int32_t lux_fits_header_f(int32_t narg, int32_t ps[])/* read fits header */
/* status = fits_header(name, [h], [h2], [extvar_preamble]) */
{
  int32_t	hsym = 0, mode, xhsym = 0;
 /* uses fits_read, mode depends on arguments */
 /* mode mask described in lux_fits_read */

  mode = 0;
  if (narg >= 2) {
    mode = 1;
    hsym = ps[1];
  }
  if (narg >= 3) {
    mode = mode + 16;
    xhsym = ps[2];
  }
  if (narg >= 4) {
    mode = mode + 64; 
    if (!symbolIsStringScalar(ps[3]))
      return cerror(NEED_STR, ps[3]);
    preamble = string_value(ps[3]);
  }
  return fits_read(mode, 0, ps[0], hsym, 0, 0, xhsym, 0, 0);
}
/*------------------------------------------------------------------------- */
int32_t lux_fits_xread_f(int32_t narg, int32_t ps[])/* read fits extension and headers */
 /* status = fits_xread(x2, name, [h], [h2], [offset], [extvar_preamble]) */
{
 int32_t	hsym = 0, mode, xhsym = 0, offsetsym = 0;

 /* uses fits_read, mode depends on arguments */
 /* mode mask described in lux_fits_read */
 mode = 32;
 if (narg >= 3) {
   mode = mode + 1;
   hsym = ps[2];
 }
 if (narg >= 4) {
   mode = mode + 16;
   xhsym = ps[3];
 }
 if (narg >= 5) {
   mode = mode + 4;
   offsetsym = ps[4];
 }
 if (narg >= 6) {
   mode = mode + 64;
   if (!symbolIsStringScalar(ps[5]))
     return cerror(NEED_STR, ps[5]);
   preamble = string_value(ps[5]);
 }
 return fits_read(mode, 0, ps[1], hsym, offsetsym, 0, xhsym, ps[0], 0);
}
/*------------------------------------------------------------------------- */
int32_t	anadecrunchrun8(uint8_t [], uint8_t [], int32_t, int32_t, int32_t),
  anadecrunch8(uint8_t *, uint8_t [], int32_t, int32_t, int32_t),
  anadecrunchrun(uint8_t *, int16_t [], int32_t, int32_t, int32_t),
  anadecrunch(uint8_t *, int16_t [], int32_t, int32_t, int32_t),
  anadecrunch32(uint8_t *, int32_t [], int32_t, int32_t, int32_t);
int32_t fits_read_compressed(int32_t mode, int32_t datasym, FILE *fp, int32_t headersym,
			 float targetblank)
/* reads data from an LUX Rice-compressed FITS file open on <fp>. */
/* <mode> determines which data to return: &1 -> header in <headersym>; */
/* &2 -> data in <datasym>. */
/* LS 18nov99 */
/* Headers:
   <stdlib.h>: malloc(), free(), atol(), realloc()
   <stdio.h>: fseek(), fread(), sscanf()
   <string.h>: strncmp(), memcpy()
 */
{
  int32_t	ncbytes, ndim, dims[MAX_DIMS], i, nblock, ok, slice, nx, ny,
    type0;
  Symboltype type;
  float	bscale = 0.0, bzero = 0.0, blank = FLT_MAX, min, max;
  char	*block, usescrat, *curblock, runlength;
  Pointer	p;

  /* header structure:

     SIMPLE  =                    T / LUX Rice compressed
     BITPIX  =                    8
     NAXIS   =                    1
     NAXIS1  =               217750
     COMPRESS= 'RICE    ' / used type of compression
     UBITPIX =                   16 / BITPIX of uncompressed data
     UNAXIS  =                    2 / NAXIS of uncompressed data
     UNAXIS1 =                  464 / dimension of uncompressed data
     UNAXIS2 =                  438 / dimension of uncompressed data

     COMMENT = header text

     We assume that BITPIX = 8 and NAXIS = 1, and that the keywords up
     to and including the last UNAXIS.. occur in the order indicated
     above -- as the FITS standard demands.
  */

  if (scratSize() < 2880) {
    block = (char *) malloc(2880);
    if (!block) {
      cerror(ALLOC_ERR, 0);
      return 0;
    }
    usescrat = 0;
  } else {
    block = curScrat;
    usescrat = 1;
  }

  fseek(fp, 0, SEEK_SET);	/* back to start of file */
  /* read first FITS block */
  if (!fread(block, 1, 2880, fp)) {
    if (!usescrat)
      free(block);
    return 0;
  }
  nblock = 1;

  /* We read what number of bytes of compressed data there are, and
     also the type and number of dimensions of the uncompressed data */
  ncbytes = atol(block + 80*3 + 9);
  {
    int dtype = atol(block + 80*5 + 9);
    ndim = atol(block + 80*6 + 9);

    switch (dtype) {
    case 8:
      type = LUX_INT8;
      break;
    case 16:
      type = LUX_INT16;
      break;
    case 32:
      type = LUX_INT32;
      break;
    case -32:
      type = LUX_FLOAT;
      break;
    case -64:
      type = LUX_DOUBLE;
      break;
    default:
      return 0;
    }
  }

  if (internalMode & 1) {	/* uncompress, too */
    /* get the dimensions of the uncompressed data */
    for (i = 0; i < ndim; i++) {
      dims[i] = atol(block + 80*(7 + i) + 9);
      if (dims[i] < 1) {
	if (!usescrat)
	  free(block);
	return 0;
      }
    }
    /* check for sane results */
    if (!ncbytes || ndim < 1 || ndim > MAX_DIMS) {
      if (!usescrat)
	free(block);
      return 0;
    }

    if (!strncmp(block + 80*4, "COMPRESS= 'RICE RLE'", 20))
      runlength = 1;		/* Rice compression with run-length encoding */
    else if (!strncmp(block + 80*4, "COMPRESS= 'RICE    '", 20))
      runlength = 0;		/* Ordinary Rice compression */
    else {
      if (!usescrat)
	free(block);
      return 0;
    }

  } else {			/* don't uncompress */
    dims[0] = ncbytes;
    type = LUX_INT8;
    ndim = 1;
  }

  curblock = block;

  /* check if the end of the FITS header is in the current block */
  /* also check for BZERO and BSCALE */
  for (i = 7 + ndim; i < 36; i++)
    if (!strncmp(curblock + 80*i, "END      ", 9))
      break;
    else (void) (sscanf(block + i*80, "BSCALE  =%f", &bscale)
                 || sscanf(block + i*80, "BZERO   =%f", &bzero)
                 || sscanf(block + i*80, "BLANK   =%f", &blank));

  while (i == 36) {
    nblock++;
    if (mode & 1) {		/* want the header */
      if (usescrat) {
	if (scratSize() < 2880*nblock) { /* not enough scratch size anymore */
	  block = (char*) malloc(2880*nblock);
	  if (!block) {
	    cerror(ALLOC_ERR, 0);
	    return 0;
	  }
	  memcpy(block, curScrat, 2880*(nblock - 1));
	  usescrat = 0;
	}
      } else {
	block = (char*) realloc(block, 2880*nblock);
	if (!block) {
	  cerror(ALLOC_ERR, 0);
	  return 0;
	}
      }
      curblock += 2880;
    } /* end of if (mode & 1) */
    if (!fread(curblock, 1, 2880, fp)) {
      if (!usescrat)
	free(block);
      return 0;
    }
    for (i = 0; i < 36; i++)
      if (!strncmp(curblock + 80*i, "END      ", 9))
	break;
      else (void) (sscanf(block + i*80, "BSCALE  =%f", &bscale)
                   || sscanf(block + i*80, "BZERO   =%f", &bzero)
                   || sscanf(block + i*80, "BLANK   =%f", &blank));
  } /* end of while (i == 36) */

  if (!usescrat)
    free(block);

  i += 36*(nblock - 1);
  /* we found the end of the FITS header. */
  if (mode & 1) {		/* want the header */
    nx = i;			/* number of lines */
    if (redef_array(headersym, LUX_STRING_ARRAY, 1, &nx) != LUX_OK)
      return 0;
    p.sp = (char**) array_data(headersym);
    curblock = block;
    while (nx--) {
      *p.sp = (char*) malloc(81);
      memcpy(*p.sp, curblock, 80);
      (*p.sp)[80] = '\0';
      p.sp++;
      curblock += 80;
    }
  }
  type0 = type;
  if (mode & 2) {		/* want the data */
    if (internalMode & 1) {	/* want to decompress, too */
      /* if /RAWVALUES wasn't selected, then */
      /* we must take the values of BZERO and BSCALE into account. */
      /* If the data in the file (before application of BSCALE and BZERO */
      /* but after decompression) are of an integer type but BSCALE or */
      /* BZERO are not integer values, then the final result will be */
      /* LUX_FLOAT.  If BSCALE and BZERO are integers, then the final result */
      /* will be an integer of a sufficiently large type that any possible */
      /* value from the file, corrected for BZERO and BSCALE, can fit in it. */
      if (!(internalMode & 2)
	  && (bscale || bzero)) {
	if (isIntegerType(type)) {
	  if ((bscale && bscale != (int32_t) bscale)
	      || (bzero != (int32_t) bzero))
	    /* we must upgrade the data type from integer to float */
	    type = LUX_FLOAT;
	  else {
	    if (!bscale)
	      bscale = 1.0;
	    switch (type) {
	      case LUX_INT8:
		min = bzero;
		max = 255*bscale + bzero;
		break;
	      case LUX_INT16:
		min = INT16_MIN*bscale + bzero;
		max = INT16_MAX*bscale + bzero;
		break;
	      case LUX_INT32:
		min = INT32_MIN*bscale + bzero;
		max = INT32_MAX*bscale + bzero;
		break;
	    }
	    if (min >= 0 && max <= 255)
	      type = LUX_INT8;
	    else if (min >= INT16_MIN && max <= INT16_MAX)
	      type = LUX_INT16;
	    else if (min >= INT32_MIN && max <= INT32_MAX)
	      type = LUX_INT32;
	    else
	      type = LUX_FLOAT;
	  }
	}
      }
    }
    if (redef_array(datasym, type, ndim, dims) != LUX_OK)
      return 0;
    if (internalMode & 1) {	/* want to decompress */
      if (scratSize() >= ncbytes) {
	block = curScrat;
	usescrat = 1;
      }	/* end of if (scratSize() >= ncbytes) */
      else {
	block = (char*) malloc(ncbytes);
	if (!block)
	  return 0;
	usescrat = 0;
      }	/* end of if (scratSize() >= ncbytes) else */
    } /* end of if (internalMode & 1) */
    else
      block = (char*) array_data(datasym);

    if (!fread(block, 1, ncbytes, fp)) { /* read the compressed data */
      if ((mode & 2) && !usescrat)
	free(block);
      return 0;
    } /* end of if (!fread) */

    if (internalMode & 1) {	/* decompress */
      p.b = (uint8_t *) block;
      slice = p.b[12];
      ny = p.l[1];
      nx = p.l[2];
      p.l += 3;
      p.b += 2;
#if !WORDS_BIGENDIAN
#else
      swapl(&nx, 1);
      swapl(&ny, 1);
#endif
      switch (type0) {
	case LUX_INT8:
	  ok = runlength?
	    anadecrunchrun8(p.b, (uint8_t*) array_data(datasym), slice, nx, ny):
	      anadecrunch8(p.b, (uint8_t*) array_data(datasym), slice, nx, ny);
	  break;
	case LUX_INT16:
	  ok = runlength?
	    anadecrunchrun(p.b, (int16_t*) array_data(datasym), slice, nx, ny):
            anadecrunch(p.b, (int16_t*) array_data(datasym), slice, nx, ny);
	  break;
	case LUX_INT32:
#if SIZEOF_LONG_LONG_INT == 8	/* 64-bit integers */
	  if (runlength) {
	    if (!usescrat)
	      free(block);
	    puts("32-bit run-length decompression was not compiled into this version of LUX.");
	    return 0;
	  } else
	    ok = anadecrunch32(p.b, array_data(datasym), slice, nx, ny);
#else
	  puts("32-bit decompression was not compiled into this version of LUX.");
	  if (!usescrat)
	    free(block);
	  return 0;
#endif
	  break;
      }	/* end of switch (type) */
      if (!usescrat)
	free(block);
#if !WORDS_BIGENDIAN
      if (ok)
	endian(array_data(datasym), array_size(datasym), type0);
#endif
    } /* end of if (internalMode & 1) */
    else ok = 1;
  } /* end of if (mode & 2) */
  else ok = 1;

  if (!(internalMode & 2) && (bscale || bzero))
    apply_bscale_bzero_blank((uint8_t*) array_data(datasym), nx*ny, bscale, bzero, blank,
			     targetblank, type0, type);

  return ok;
}
/*------------------------------------------------------------------------- */
int32_t	lux_replace(int32_t, int32_t), swapb(char [], int32_t);
void	swapd(char [], int32_t);
int32_t fits_read(int32_t mode, int32_t dsym, int32_t namsym, int32_t hsym, int32_t offsetsym,
	      int32_t xoffsetsym, int32_t xhsym, int32_t xdsym, float targetblank)
 /* internal, read fits files */
 /* returns status as sym # for 0, 1, or 2 */
/* Headers:
   <stdio.h>: FILE, fread(), sscanf(), printf(), fclose(), perror(), fseek()
   <stdlib.h>: malloc(), free(), atof()
   <string.h>: memcpy(), strstr(), strncmp(), strchr(), strdup(), strlen(),
               strcat()
 */
{
  static char	tforms[] = "IJAEDB";
  static int32_t	tform_sizes[] = { 2, 4, 1, 4, 8, 1};
  FILE	*fin;
  char	*fitshead, line[80], *lptr, c, **p, *q, *ext_ptr, *sq, *p2, *qb;
  int32_t	n, simple_flag, bitpix_flag = 0, naxis_flag = 0, naxis_count;
  int32_t	bitpix, nlines, end_flag = 0, nhblks, lc, iq, id, new_sym;
  Symboltype type;
  int32_t	maxdim, i, rsym = 1, nbsize, ext_flag=0, data_offset, npreamble;
  int32_t	fits_type, ndim_var, *dim_var, n_ext_rows, nrow_bytes, m;
  int32_t	xtension_found = 0, tfields, gcount, row_bytes,
    dim[MAX_DIMS], type0;
  int32_t	ext_stuff_malloc_flag = 0;
  float	bscale = 0.0, bzero = 0.0, blank = FLT_MAX, min, max;
  struct ext_params {
    int32_t	repeat;
    int32_t	type;
    double	scale, zero;
    char	*lab;
  };
  struct ext_params *ext_stuff;
#if LITTLEENDIAN
  int32_t	nb;
#endif

  /* first arg is the variable to load, second is name of file */
  if ((fin = fopenr_sym(namsym)) == NULL)
    return LUX_ZERO;
  /* use scrat to read in header block of 2880 bytes, we implicitly assume
     scrat is large enough for at least one, beyond that we check */
  fitshead = curScrat;		/* changed to curScrat - LS 19may99 */
  fits_head_malloc_flag = 0;
  if (fread(fitshead, 1, 2880, fin) != 2880) {
    perror("fits_read in header");
    return fits_fatality(fin);
  }
  /* a fits header is a series of card images, 80 columns by 36 cards */
  /* must have "SIMPLE" at start */

  /* first line is done separately, we look for SIMPLE */
  n = sscanf(fitshead, "SIMPLE  = %1s", &c);
  simple_flag = (c == 'T');
  if (!simple_flag) {
    /* check if one of my special fitz files */
    q = (char *) malloc(81);
    memcpy(q, fitshead, 80);
    *(q+80) = '\0';
    if (strstr(q, "lux rice compressed"))
      simple_flag = 2;
    else {
      printf("Not a simple FITS file\n");
      free(q);
      return fits_fatality(fin);
    }
    free(q);
  }
  nlines = 35;	/* lines left in this block */
  lptr = fitshead;

  if (!strncmp(fitshead + 320, "COMPRESS= ", 10)) {
    rsym = fits_read_compressed(mode, dsym, fin, hsym, targetblank);
    fclose(fin);
    return rsym;
  }

  nhblks = 1;
  lc = 1;
  zerobytes( dim, 8*sizeof(int32_t));
  maxdim = 0;
  while (1) {		/* outer loop to handle multiple header blocks */
    while (nlines--) {	/* 36 for a full header block, 35 for first */
      lptr += 80;	/* bump here so we can use continue's in each check */
      lc++;
      /* printf("%.80s\n", lptr); */

      /* look for keywords on each line, decode ones we need */
      if (strncmp(lptr, "BITPIX  ",8) == 0) {
	/* got a BITPIX, get value */
	if (bitpix_flag) { /* whoops, already have this */
	  fits_problems(1);
	} else {
	  n = sscanf(lptr+9,"%d", &bitpix);
	  if (n >= 1) {
	    bitpix_flag = 1;
	    /*printf("found bitpix = %d\n", bitpix); */
	    /* translate to lux type */
	    switch (bitpix) {
	      case 8:   type = LUX_INT8;  break;
	      case 16:  type = LUX_INT16;  break;
	      case 32:  type = LUX_INT32;  break;
                // NO LUX_INT64
	      case -32: type = LUX_FLOAT;  break;
	      case -64: type = LUX_DOUBLE;  break;
	      default: fits_problems(2); printf("%d\n", bitpix); return fits_fatality(fin);
	    }
	  } else {
	    /* couldn't decode the bitpix, sorry */
	    fits_problems(4); return fits_fatality(fin);
	  }
	}
	/* don't try to match other keys since this one matched */
      } else if (strncmp(lptr, "NAXIS   ",8) == 0) {
	/* got a NAXIS, get value */
	if (naxis_flag) { /* whoops, already have this */
	  fits_problems(3);
	} else {
	  n = sscanf(lptr+9,"%d", &naxis_count);
	  if (n >= 1) {
	    naxis_flag = 1;
	  } else {
	    /* couldn't decode the naxis, sorry */
	    fits_problems(5); return fits_fatality(fin);
	  }
	  /* check if dimension too high */
	  if (naxis_count > 8 || naxis_count < 1)
	  { printf("dimension count %d (NAXIS) out of range\n", naxis_count);
	    return fits_fatality(fin); }
	}
      } else if (strncmp(lptr, "NAXIS",5) == 0) {
	/* got a NAXISn, get value and dimension */
	n = sscanf(lptr+5,"%d = %d", &iq, &id);
	if (n != 2)
	  /* couldn't decode the naxis, sorry */
	{ fits_problems(8); return fits_fatality(fin); }
	/* printf("dimension = %d, value = %d\n", iq, id);*/
	if (iq < 1 || iq > 8)
	{ fits_problems(9); return fits_fatality(fin); }
	dim[iq-1] = id;
	maxdim = MAX(maxdim, iq);
	/* check if dimension too high */
	if (maxdim > 8 || maxdim < 1)
	{ printf("dimension count %d (NAXIS) out of range\n", maxdim);
	  return fits_fatality(fin); }
      } else if (strncmp(lptr, "EXTEND  ",8) == 0) {
	/* got an extension flag */
	ext_flag = 1;
      } else if (!bscale && !strncmp(lptr, "BSCALE  ",8))
	bscale = atof(lptr + 9);
      else if (!bzero && !strncmp(lptr, "BZERO   ", 8))
	bzero = atof(lptr + 9);
      else if (blank != FLT_MAX && !strncmp(lptr, "BLANK   ", 8))
	blank = atof(lptr + 9);
      else if (strncmp(lptr, "END     ",8) == 0) {
	end_flag = 1;
	/* printf("total lines in header = %d\n", lc);*/
	break;		/* this ends the header read */
      }

    }
    if (end_flag) break; else {
      /* haven't hit an END yet, so we read another header block */
      if (nhblks > HEAD_LIMIT)
      { fits_problems(6); return fits_fatality(fin);}
      fitshead = bigger_header(nhblks+1, fitshead);
      if (fitshead == NULL) { fits_problems(7); return fits_fatality(fin);}
      if (fread(fitshead+nhblks*2880, 1, 2880, fin) != 2880)
      { perror("fits_read in header");
	fits_problems(11); return fits_fatality(fin);}
      nlines = 36;  nhblks++;
    }

  }
  /* check out */
  /* printf("nhblks = %d, maxdim = %d\n", nhblks, maxdim);*/
  /* the data offset will be useful for TRACE hourlies */
  data_offset = nhblks * 2880;
  /*for (i=0;i<maxdim;i++) printf("dim%d = %d\n", i, dim[i]); */
  /* compute size of data array */
  nbsize = lux_type_size[type];
  for (i=0;i<maxdim;i++) nbsize = nbsize*dim[i];
  /* printf("nbsize = %d\n", nbsize);*/
  /* if an extension, figure out where */
  if (ext_flag) {
    ext_flag = nhblks*2880 + 2880*( (nbsize-1)/2880 + 1);
    /* printf("extension offset = %d\n", ext_flag); */
  }
  /* return extension offset ? */
  if (mode & 0x8) {
    redef_scalar(xoffsetsym, LUX_INT32, &ext_flag);
  }

  /* check if we want the offset to data */
  if (mode & 0x4) {
    redef_scalar(offsetsym, LUX_INT32, &data_offset);
  }

  /* done with reading the header, for a mode zero we are finished */
  if (mode == 0) { fclose(fin); return rsym; }

  /* check if header is wanted */
  if (mode & 0x1) {
    lc--;			/* exclude the END record.  LS 14apr00 */
    /* generate an lux string array for the header */
    if (to_scratch_array(hsym, LUX_STRING_ARRAY, 1, &lc) == LUX_ERROR) {
      fits_problems(9);
      return fits_fatality(fin);
    }
    p = (char**) array_data(hsym);
    n = lc;
    lptr = fitshead;
    while (n--) {
      *p = (char *) malloc(81);
      memcpy(*p, lptr, 80);
      i = 80;
      /* replace any NULL's with blanks */
      q = *p;
      while (i--) { if (*q == '\0') *q = 32;  q++; }
      *q = '\0';		/* changed NULLs to '\0'.  LS 24may99 */
      p++;	lptr += 80;
    }
  }
  /* should be done with any malloc for a long header */
  if (fits_head_malloc_flag) free(fitshead);

  /* now check if we want the main data array */
  type0 = type;
  if (mode & 0x2) {
    /* all set up for the most part */
      /* if we haven't selected /RAWVALUES, then */
      /* we must take the values of BZERO and BSCALE into account. */
      /* If the data in the file (before application of BSCALE and BZERO */
      /* but after decompression) are of an integer type but BSCALE or */
      /* BZERO are not integer values, then the final result will be */
      /* LUX_FLOAT.  If BSCALE and BZERO are integers, then the final result */
      /* will be an integer of a sufficiently large type that any possible */
      /* value from the file, corrected for BZERO and BSCALE, can fit in it. */
      /* LS 28jan00 */
    if (!(internalMode & 2)	/* no /RAWVALUES */
	&& (bscale || bzero)) {	/* and have BSCALE or BZERO */
      if (isIntegerType(type)) {
	if ((bscale && bscale != (int32_t) bscale)
	    || (bzero != (int32_t) bzero))
	  /* we must upgrade the data type from integer to float */
	  type = LUX_FLOAT;
	else {
	  if (!bscale)
	    bscale = 1.0;
	  switch (type) {
	    case LUX_INT8:
	      min = bzero;
	      max = 255*bscale + bzero;
	      break;
	    case LUX_INT16:
	      min = INT16_MIN*bscale + bzero;
	      max = INT16_MAX*bscale + bzero;
	      break;
	    case LUX_INT32:
	      min = INT32_MIN*bscale + bzero;
	      max = INT32_MAX*bscale + bzero;
	      break;
	  }
	  if (min >= 0 && max <= 255)
	    type = LUX_INT8;
	  else if (min >= INT16_MIN && max <= INT16_MAX)
	    type = LUX_INT16;
	  else if (min >= INT32_MIN && max <= INT32_MAX)
	    type = LUX_INT32;
	  else
	  type = LUX_FLOAT;
	}
      }
    }
    iq = array_scratch(type, maxdim, dim);
    q = (char*) array_data(iq);

    if (simple_flag == 1) {
      /* should be in position in file to just read in */
      if (fread(q, 1, nbsize, fin) != nbsize)
      { perror("fits_read in data array");
	fits_problems(12); return fits_fatality(fin);}

      /* fits data is supposed to always be big endian, so fix up data on
	 little endian machines (like alpha's and pc's) */
#if LITTLEENDIAN
      switch (type) {
      case LUX_INT16:           // 16 bits
        swapb((char *) q, nbsize);
        break;
      case LUX_INT32: case LUX_FLOAT: // 32 bits
        swapl((int32_t *) q, nbsize/4);
        break;
      case LUX_INT64: case LUX_DOUBLE: // 64 bits
        swapd((char *) q, nbsize/8);
        break;
      }
#endif
    }
    /* now correct for BZERO, BSCALE, BLANK */
    if (!(internalMode & 2) && (bscale || bzero))
      apply_bscale_bzero_blank((uint8_t *) q, nbsize/lux_type_size[type0], bscale, bzero,
			       blank, targetblank, type0, type);
    /* lux_replace ordinarily yields some output when STEPping or TRACEing, */
    /* but here we don't want that: ensure that noTrace is non-zero */
    noTrace++;
    if ( lux_replace(dsym, iq) != 1 )
    { fits_problems(10); noTrace--; return fits_fatality(fin);}
    noTrace--;
  }

  /* if there is an extension, process the header */
  if (ext_flag) {
    /* printf("processing the extension header\n"); */
    if (fseek(fin, ext_flag, SEEK_SET))
    { fits_problems(13); return fits_fatality(fin);}
    fits_head_malloc_flag = 0;
    nhblks = 0;
    fitshead = curScrat;		/* changed to curScrat -- LS 19may99 */
    lc = 0;
    zerobytes( dim, 8*sizeof(int32_t));
    maxdim = bitpix_flag = naxis_flag = end_flag = 0;

    while (1) {
      nhblks++;
      if (nhblks > HEAD_LIMIT)
      { fits_problems(14); return fits_fatality(fin);}
      fitshead = bigger_header(nhblks, fitshead);
      if (fitshead == NULL) { fits_problems(15); return fits_fatality(fin);}
      if (fread(fitshead+(nhblks-1)*2880, 1, 2880, fin) != 2880)
      { perror("fits_read in extension header"); fits_problems(16);
 	return fits_fatality(fin);}
      nlines = 36;
      lptr = fitshead+(nhblks-1)*2880;
      while (nlines--) {
	lc++;
	/* printf("%.80s\n", lptr); */

	/* look for keywords on each line, decode ones we need */

	if (strncmp(lptr, "XTENSION",8) == 0) {
	  xtension_found = 1;
	} else

	  if (strncmp(lptr, "BITPIX  ",8) == 0) {
	    /* got a BITPIX, get value */
	    if (bitpix_flag) { /* whoops, already have this */
	      fits_problems(1);
	    } else {
	      n = sscanf(lptr+9,"%d", &bitpix);
	      if (n >= 1) { bitpix_flag = 1;
	      /* printf("found bitpix = %d\n", bitpix); */
	      /* translate to lux type */
	      switch (bitpix) {
		case 8:   type = LUX_INT8;  break;
		case 16:  type = LUX_INT16;  break;
		case 32:  type = LUX_INT32;  break;
                  // NO LUX_INT64
		case -32: type = LUX_FLOAT;  break;
		case -64: type = LUX_DOUBLE;  break;
		default: fits_problems(2); printf("%d\n", bitpix); return fits_fatality(fin);
	      }
	      } else {
		/* couldn't decode the bitpix, sorry */
		fits_problems(4); return fits_fatality(fin);
	      }
	    }
	  } else

	    if (strncmp(lptr, "NAXIS   ",8) == 0) {
	      /* got a NAXIS, get value */
	      if (naxis_flag) { /* whoops, already have this */
		fits_problems(3);
	      } else {
		n = sscanf(lptr+9,"%d", &naxis_count);
		if (n >= 1) { naxis_flag = 1;
		} else {
		  /* couldn't decode the naxis, sorry */
		  fits_problems(5); return fits_fatality(fin);
		}
		/* check if dimension too high */
		if (naxis_count > 8 || naxis_count < 1)
		{ printf("dimension count %d (NAXIS) out of range\n", naxis_count);
		  return fits_fatality(fin); }
	      }
	    } else

	      if (strncmp(lptr, "NAXIS",5) == 0) {
		/* got a NAXISn, get value and dimension */
		n = sscanf(lptr+5,"%d = %d", &iq, &id);
		if (n != 2)
		  /* couldn't decode the naxis, sorry */
		{ fits_problems(8); return fits_fatality(fin); }
		/* printf("dimension = %d, value = %d\n", iq, id); */
		if (iq < 1 || iq > 8)
		{ fits_problems(9); return fits_fatality(fin); }
		dim[iq-1] = id;
		maxdim = MAX(maxdim, iq);
		/* check if dimension too high */
		if (maxdim > 8 || maxdim < 1)
		{ printf("dimension count %d (NAXIS) out of range\n", maxdim);
		  return fits_fatality(fin); }
	      } else

		if (strncmp(lptr, "TFIELDS ",8) == 0) {
		  n = sscanf(lptr+9,"%d", &tfields);
		  if (n !=1) { fits_problems(21); }
		} else

		  if (strncmp(lptr, "GCOUNT  ",8) == 0) {
		    n = sscanf(lptr+9,"%d", &gcount);
		    if (n !=1) { fits_problems(22); }
		  } else

		    if (strncmp(lptr, "END     ",8) == 0) {
		      end_flag = 1;
		      /* printf("total lines in header = %d\n", lc); */
		      break;		/* this ends the header read */
		    }
	lptr += 80;
      }
      if (!xtension_found) { fits_problems(17); return fits_fatality(fin); }
      if (end_flag) break;
    }

    /* check out */
    /* printf("nhblks = %d, maxdim = %d\n", nhblks, maxdim); */
    /* for (i=0;i<maxdim;i++) printf("extension: dim%d = %d\n", i, dim[i]); */
    /* compute size of data array */
    nbsize = lux_type_size[type];
    for (i=0;i<maxdim;i++) nbsize = nbsize*dim[i];
    /* printf("extension: nbsize = %d\n", nbsize); */
    n_ext_rows = nbsize/dim[0]/lux_type_size[type];
  }

  /* check if we want the extension header */
  if (mode & 0x10) {
    /* if no extension, return a scalar of value 0 */
    if (ext_flag == 0) {
      redef_scalar(xhsym, LUX_INT32, &ext_flag);

    } else {
      /* printf("creating extension header array\n"); */
      /* generate an lux string array for the header */
      if (to_scratch_array(xhsym, LUX_STRING_ARRAY, 1, &lc) == LUX_ERROR)
      { fits_problems(18); return fits_fatality(fin);}
      p = (char**) array_data(xhsym);
      n = lc;
      lptr = fitshead;
      while (n--) {
	*p = (char *) malloc(81);
	memcpy(*p, lptr, 80);
	i = 80;
	/* replace any NULL's with blanks */
	q = *p;
	while (i--) { if (*q == '\0') *q = 32;  q++; }
	*q = '\0';
	p++;	lptr += 80;
      }
    }
  }
  /* don't throw the extension header away yet, may need for decoding tables */

  /* the extension data array */
  if (mode & 0x20) {
    /* if no extension, return a scalar of value 0 */
    if (ext_flag == 0) {
      redef_scalar(xdsym, LUX_INT32, &ext_flag);

    } else {
      /* all set up for the most part */
      iq = array_scratch(type, maxdim, dim);
      q = ext_ptr = (char*) array_data(iq);

      /* should be in position in file to just read in */
      if (fread(q, 1, nbsize, fin) != nbsize)
      { perror("fits_read in data array");
	fits_problems(20); return fits_fatality(fin);}

      /* fits data is supposed to always be big endian, so fix up data on
	 little endian machines (like alpha's and pc's) */
      /* note, however, that these are normally I*1 (type 0) for binary tables */
#if LITTLEENDIAN
      switch (type) {
      case LUX_INT16:           // 16 bits
        swapb(q, nbsize);
        break;
      case LUX_INT32: case LUX_FLOAT: // 32 bits
        swapl((int32_t *) q, nbsize/4);
        break;
      case LUX_INT64: case LUX_DOUBLE: // 64 bits
        swapd((char *) q, nbsize/8);
        break;
      }
#endif
      noTrace++;
      if ( lux_replace(xdsym, iq) != 1 )
      { fits_problems(19); noTrace--; return fits_fatality(fin);}
      noTrace--;

    }
  }

  /* decode the extension (for binary tables) */
  if (mode & 0x40 && ext_flag) {
    /* can't do if no extension */
    int32_t	index, itype, ext_ptr_malloc_flag = 0;
    char	*loc;
    /* printf("lc = %d\n", lc); */
    lptr = fitshead;
    /* since the keywords for the columns are allowed in any order, we
       run through and store everything in a structure first, hopefully we can
       assumed that TFIELDS resembles the # of columns */
    ext_stuff = (struct ext_params*) malloc( tfields * sizeof(struct ext_params));
    ext_stuff_malloc_flag = 1;
    /* pre-load the zero and scale items */
    for (index=0;index<tfields;index++) {
      ext_stuff[index].scale = 1.0;
      ext_stuff[index].zero = 0.0;
      ext_stuff[index].lab = NULL;
    }
    i = lc;
    while (i--) {
      /* printf("i = %d, lptr = %#x\n", i, lptr); */
      /* look for a form */
      if (strncmp(lptr, "TFORM",5) == 0) {
	/* got a TFORM, get "column" */
	n = sscanf(lptr+5,"%d = '%d%1s", &iq, &id, line);
	if (n != 3)
	  /* couldn't decode the form, sorry */
	{ fits_problems(23); goto fits_read_1; }
	/* printf("TFORM column = %d, nnn = %d, format %s\n", iq, id, line); */
	/* check if within range */
	if (iq > tfields || iq < 1) { fits_problems(25); }
	else {
	  index = iq - 1;
	  ext_stuff[index].repeat = id;
	  loc = strchr(tforms, (int32_t) line[0]);
	  if (loc == NULL)
	  { printf("unsupported extension column format %s\n", line);
	    itype = -1;}	else	itype = loc - tforms;
	  ext_stuff[index].type = itype;
	}

      } else

	/* look for the label */
	if (strncmp(lptr, "TTYPE",5) == 0) {
	  /* got a TTYPE, get "column" */
	  n = sscanf(lptr+5,"%d = '%s", &iq, line);
	  if (n != 2)
	    /* couldn't decode the type, sorry */
	  { fits_problems(24); goto fits_read_1; }
	  /* check if within range */
	  if (iq > tfields || iq < 1) { fits_problems(25); }
	  else {
	    index = iq - 1;
	    q = line;
	    /* need to remove any "attached" single quotes on the end of the symbol */
	    while (*q) { if (*q == '\'') {*q = '\0'; break; } q++; }
	    /* printf("TTYPE column = %d, label %s\n", iq, line); */
	    ext_stuff[index].lab = strdup(line);
	  }
	}

      lptr += 80;
    }
    /* for binary tables, the first dim is the # of bytes in each "row" */
    row_bytes = 0;
    /* examine what we got */
    /* printf("ext_stuff listing\n"); */
    for (index=0;index<=tfields-1;index++) {
      row_bytes += tform_sizes[ext_stuff[index].type]*ext_stuff[index].repeat;
    }
    if (row_bytes > dim[0]) {
      printf("mismatch: accumulated row width = %d, first dim = %d\n", row_bytes,dim[0]);
      goto fits_read_1; }
      /* set nrow_bytes to dim[0] which we destroy below */
    nrow_bytes = dim[0];
      /* make the lux variables, the last dim will be the number of rows. If the
	 repeat in the tform is 1, then the array or strarr will be 1-D, otherwise
	 it will be 2-D with repeat as the first dim */
      /* want to use dim for the constructed arrays, so save the binary table row
	 count elsewhere */
      /* if we wanted the extension in a variable, we already read it in, otherwise
	 we read it now */
    if (!(mode & 0x20)) {
      q = ext_ptr = (char *) malloc(nbsize);
      ext_ptr_malloc_flag = 1;
      /* should be in position in file to just read in */
      if (fread(q, 1, nbsize, fin) != nbsize)
      { perror("fits_read in data array");
	fits_problems(20);
	if (ext_ptr_malloc_flag)
	  free(ext_ptr);
	goto fits_read_1;
      }
    }
      /* one way or another, the entire table is now at ext_ptr */
 
    npreamble = strlen(preamble);
    row_bytes = 0;
    lptr = ext_ptr;
    for (index=0;index<=tfields-1;index++) {
      int32_t	sl;
      char *ppq;

      /* construct the variable name */
      sl = npreamble + strlen(ext_stuff[index].lab);
      sq = (char *) malloc(sl + 1);
      strcpy(sq, preamble);  strcat(sq, ext_stuff[index].lab);
      ppq = sq;
      while (sl--) {
	*ppq = toupper(*ppq);
	ppq++;
      }
      new_sym = find_sym(sq);
      /* printf("variable name = %s, symbol # %d\n", sq, new_sym); */
      free(sq);
      /* how we define depends on type */
      id = ext_stuff[index].repeat;
      /* dimension count depends on "repeat" */
      fits_type = ext_stuff[index].type;
      if (id <= 1 || fits_type == 2) {
	ndim_var = maxdim-1;
	dim_var = dim+1;
      } else {
	ndim_var = maxdim;
	dim_var = dim;
	/* but we have to load the repeat count as the first dimension,
	   note that this destroys the original contents which we should no
	   longer need */
	dim[0] = id;
      }

      switch (fits_type) {
	case 0:  /* I*2 array */
	  type = LUX_INT16;
	  break;
	case 1:  /* I*4 array */
	  type = LUX_INT32;
	  break;
	case 3:  /* F*4 array */
	  type = LUX_FLOAT;
	  break;
	case 4:  /* F*8 array */
	  type = LUX_DOUBLE;
	  break;
	case 5:  /* I*1 array */
	  type = LUX_INT8;
	  break;
      }
      if (fits_type == 2) {
	char **psa;

	if (redef_strarr(new_sym, ndim_var, dim_var) != 1) {
	  if (ext_ptr_malloc_flag)
	    free(ext_ptr);
	  goto fits_read_1;
	}
	psa = (char**) array_data(new_sym);
	nbsize = tform_sizes[fits_type] * id;
	/* need to get a series of strings */
	p2 = lptr;
	m = n_ext_rows;
	while (m--) {
	  n = nbsize;  q = (char*) malloc(nbsize + 1);
	  *psa++ = q;
	  while (n--) *q++ = *p2++;  *q = '\0';
	  p2 = p2 + nrow_bytes - nbsize;  }
      } else {
	if (redef_array(new_sym, type, ndim_var, dim_var) != 1) {
	  if (ext_ptr_malloc_flag)
	    free(ext_ptr);
	  goto fits_read_1;
	}
	/* and load all the columns in */
	q = qb = (char*) array_data(new_sym);
	nbsize = tform_sizes[fits_type] * id;
	p2 = lptr;
	m = n_ext_rows;
	while (m--) { n = nbsize;  while (n--) *q++ = *p2++;
	p2 = p2 + nrow_bytes - nbsize;  }
#if LITTLEENDIAN
	nb = nbsize * n_ext_rows;
	if (fits_type == 0)  swapb(qb, nb);
	if (fits_type == 1 || fits_type ==3)  swapl((int32_t *) qb, nb/4);
	if (fits_type == 4)  swapd((char *) qb, nb/8);
#endif
      }
	
      row_bytes += nbsize;
      lptr += nbsize;
    }
    if (ext_ptr_malloc_flag)
      free(ext_ptr);
  } 
    /* should be done with any malloc for a long header */
  if (fits_head_malloc_flag) free(fitshead);
  if (ext_stuff_malloc_flag) free(ext_stuff);
  fclose(fin);
  return rsym;  /* this is the one symbol */

  fits_read_1:
  if (ext_stuff_malloc_flag)
    free(ext_stuff);
  return fits_fatality(fin);
}
/*------------------------------------------------------------------------- */
int32_t lux_fits_write_general(int32_t narg, int32_t ps[], int32_t func)
/* FITS_WRITE,data,file [,header,slice] [,/VOCAL] */
/* LS 18nov99 */
/* Headers:
   <stdio.h>: FILE, printf(), puts(), fopen(), fprintf(), putc(), fwrite(),
              fclose()
   <stdlib.h>: malloc(), free()
   <string.h>: strlen(), strncmp()
 */
{
  char	*file, runlength, *p;
  void	*data, *out;
  int32_t	*dims, ndim, headertype, nheader, slice, nlines, n, type,
    nx, ny, limit, bitpix[] = {8,16,32,-32,-64}, i, size;
  Pointer	header;
  FILE	*fp;

  if (!symbolIsNumericalArray(ps[0]) || symbolIsComplexArray(ps[0]))
    return func? LUX_ZERO: cerror(ILL_CLASS, ps[0]);
  if (!symbolIsStringScalar(ps[1]))
    return func? LUX_ZERO: cerror(ILL_CLASS, ps[1]);

  /* the data */
  data = array_data(ps[0]);
  dims = array_dims(ps[0]);
  ndim = array_num_dims(ps[0]);
  type = array_type(ps[0]);

  if (narg > 2 && ps[2]) {	/* header */
    if (symbolIsStringArray(ps[2])) {
      header.sp = (char**) array_data(ps[2]);
      headertype = LUX_STRING_ARRAY;
      nheader = array_size(ps[2]);
    } else if (symbolIsStringScalar(ps[2])) {
      header.s = string_value(ps[2]);
      headertype = LUX_TEMP_STRING;
    } else return func? LUX_ZERO: cerror(ILL_CLASS, ps[2]);
  } else
    headertype = 0;		/* no header specified */

  if (narg > 3 && ps[3]) {	/* slice */
    slice = int_arg(ps[3]);
    if (slice < 0) {
      runlength = 1;
      slice = -slice;
    } else
      runlength = 0;
  } else
    slice = 0;

  if (slice > 8*lux_type_size[type]) {
    printf("Compression parameter %1d is too large for current data type.\n",
	   slice);
    puts("Writing to disk without compression.");
    slice = 0;			/* default: no compression */
  }

  /* the output file */
  file = string_value(ps[1]);
  fp = fopen(file, "w");
  if (!fp)
    return func? LUX_ZERO: cerror(ERR_OPEN, ps[1]);

  nx = dims[0];
  ny = array_size(ps[0])/nx;

  if (slice) {
    /* prepare room for the compressed data */
    limit = array_size(ps[0])*lux_type_size[type]*2;
    if (limit < 25)
      limit = 25;			/* limit imposed by crunch */
    out = malloc(limit);
    if (!out)
      return func? LUX_ZERO: cerror(ALLOC_ERR, 0);
  }

  /* FITS data is always bigendian, so we should swap bytes on littleendian */
  /* machines (Dec alpha, PC).  The original data must be unchanged */
  /* upon exiting this routine, so we have two choices: (1) make a copy */
  /* of the data and swap that, or (2) swap the original data, write it to */
  /* the FITS file, and then swap the original data again.  We choose to */
  /* be frugal with memory, so we go for the double swap.  LS 18nov99 */
#if !WORDS_BIGENDIAN
  endian(data, array_size(ps[0])*lux_type_size[type], type);
#endif

  if (slice) {
    switch (type) {
      case LUX_INT8:
	size = runlength? anacrunchrun8((uint8_t*) out, (uint8_t*) data, slice, nx, ny, limit):
	  anacrunch8((uint8_t*) out, (uint8_t*) data, slice, nx, ny, limit);
	break;
      case LUX_INT16:
	size = runlength? anacrunchrun((uint8_t*) out, (int16_t*) data, slice, nx, ny, limit):
	  anacrunch((uint8_t*) out, (int16_t*) data, slice, nx, ny, limit);
	break;
#if SIZEOF_LONG_LONG_INT == 8	/* 64-bit integers */
      case LUX_INT32:
	if (runlength) {
	  puts("WARNING - no compression with run-length encoding is currently\navailable for 32-bit data.  Using compression without RLE instead.");
	  runlength = 0;
	}
	size = anacrunch32(out, data, slice, nx, ny, limit);
	break;
#else
	puts("WARNING - no 32-bit compression was compiled into this version of LUX.\nWriting uncompressed data instead.");
#endif
      default:			/* no compression */
	free(out);
	out = data;
	slice = 0;		/* flag no compression */
	size = -1;
	break;
    } /* end of switch (type) */
    if (size == -1		/* could not compress */
	&& (internalMode & 1))	/* /VOCAL */
      printf("Data compression failed -- storing uncompressed %s.\n", file);
  } else {
    out = data;
    slice = 0;			/* flag no compression */
    size = -1;
  }

  if (size == -1)
    size = array_size(ps[0])*lux_type_size[type];

  /* write the FITS header */
  nlines = 0;
  if (slice) {			/* we did compress */
    fprintf(fp, "%-8.8s= %20s / %-47s", "SIMPLE", "T",
	    "LUX Rice compressed");
    nlines++;
    fprintf(fp, "%-8.8s= %20d%50s", "BITPIX", 8, "");
    nlines++;
    fprintf(fp, "%-8.8s= %20d%50s", "NAXIS", 1, "");
    nlines++;
    fprintf(fp, "%-8.8s= %20d%50s", "NAXIS1", size, "");
    nlines++;
    fprintf(fp, "%-8.8s= '%-8s'%n", "COMPRESS",
	    runlength? "RICE RLE": "RICE", &n);
    fprintf(fp, " / %-*.*s", 77 - n, 77 - n, "used type of compression");
    nlines++;
    /* add some info about the uncompressed data */
    fprintf(fp, "%-8.8s= %20d / %-47s", "UBITPIX", bitpix[type],
	    "BITPIX of uncompressed data");
    nlines++;
    fprintf(fp, "%-8.8s= %20d / %-47s", "UNAXIS", ndim,
	    "NAXIS of uncompressed data");
    nlines++;
    for (i = 0; i < ndim; i++) {
      fprintf(fp, "UNAXIS%-2d= %20d / %-47s", i + 1, dims[i],
	      "dimension of uncompressed data");
      nlines++;
    } /* end of for (i = 0) */
  } /* end of if (size != -1) */
  else {			/* we did not compress */
    fprintf(fp, "%-8.8s= %20s / %-47s", "SIMPLE", "T",
	    "LUX-generated file");
    nlines++;
    fprintf(fp, "%-8.8s= %20d%50s", "BITPIX", bitpix[type], "");
    nlines++;
    fprintf(fp, "%-8.8s= %20d%50s", "NAXIS", ndim, "");
    nlines++;
    for (i = 0; i < ndim; i++) {
      fprintf(fp, "NAXIS%-3d= %20d%50s", i + 1, dims[i], "");
      nlines++;
    } /* end of for (i = 0) */
  } /* end of if (size != -1) else */

  /* add the user-specified header, if any.  If the header starts with */
  /* the FITS identification string (SIMPLE = T), then we take it to be */
  /* a FITS header and remove the SIMPLE, BITPIX, and NAXIS.. lines */
  /* before adding it to the output FITS file.  Otherwise, we chop the */
  /* header into 70-character parts and write each one to the FITS file */
  /* after a COMMENT = key. */
  switch (headertype) {
    case LUX_TEMP_STRING:
      if (strlen(header.s) > 9 && header.s[8] == '=') {
	/* assume a FITS header */
	p = header.s;
	n = strlen(header.s);
	/* we assume that the header is in strict FITS format, with 80-char */
	/* lines and everything.  We must, however, check if the last line */
	/* of the header contains END already, or if we must still add that */
	/* ourselves. */
	/* also, if we're writing compressed data, then we must rename
	   the BSCALE, BZERO, and BLANK keywords to */
	/* UBSCALE, UBZERO, and UBLANK to prevent modification of the */
	/* compressed data when it is read by FITS readers that don't */
	/* recognize the compression */
	while (n > 0) {		/* still have some to write */
	  fprintf(fp, "%-80s", header.s);
	  header.s += 80;
	  n -= 80;
	  nlines++;
	} /* end of if (n) */
	if (n || strncmp(header.s + (n - 80), "END      ", 9)) {
	  fprintf(fp, "%-80s", "END");
	  nlines++;
	} /* end of if (!size || !strncmp(...)) */
      }	/* end of if (!strncmp(header.s,...)) */
      else {			/* no FITS header, turn into comments */
	n = strlen(header.s);
	while (n > 0) {
	  fprintf(fp, "COMMENT = %-70.70s", header.s);
	  header.s += 70;
	  n -= 70;
	  nlines++;
	}
	fprintf(fp, "%-80s", "END");
	nlines++;
      }
      break;
    case LUX_STRING_ARRAY:
      if (strlen(header.sp[0]) > 9 && header.sp[0][8] == '=') {
	/* we assume a FITS header */
	n = nheader;
	nlines += n;
	while (n-- > 0) {
	  fprintf(fp, "%-80s", *header.sp);
	  header.sp++;
	}
	if (strncmp(header.sp[-1], "END      ", 9)) {
	  fprintf(fp, "%-80s", "END");
	  nlines++;
	} /* end of if (!size || !strncmp(...)) */
      }	/* end of if (!strncmp(header.s,...)) */
      else {			/* no FITS header, turn into comments */
	while (nheader--) {
	  p = *header.sp++;
	  n = strlen(p);
	  while (n > 0) {
	    fprintf(fp, "COMMENT = %-70.70s", p);
	    p += 70;
	    n -= 70;
	    nlines++;
	  }
	}
	fprintf(fp, "%-80s", "END");
	nlines++;
      }
      break;
    default:    
      fprintf(fp, "%-80s", "END");
      nlines++;
  }

  /* the header must have a length that is a multiple of 2880 bytes = */
  /* 36 lines of 80 characters each */
  if (nlines % 36) {
    n = (36 - (nlines % 36))*80;
    while (n--)
      putc(' ', fp);
  }

  /* write the data */
  fwrite(out, 1, size, fp);

  /* the data must also have a length that is a multiple of 2880 bytes */
  if (size % 2880) {
    n = 2880 - (size % 2880);
    while (n--)
      putc(' ', fp);
  }

  /* must swap the data back into its original order on littleendian */
  /* machines */
#if !WORDS_BIGENDIAN
  endian(out, size, type);
#endif

  /* done */
  fclose(fp);
  if (out != data)
    free(out);
  return LUX_OK;
}
/*------------------------------------------------------------------------- */
int32_t lux_fits_write(int32_t narg, int32_t ps[])
{
  return lux_fits_write_general(narg, ps, 0);
}
/*------------------------------------------------------------------------- */
int32_t lux_fits_write_f(int32_t narg, int32_t ps[])
{
  return lux_fits_write_general(narg, ps, 1);
}
/*------------------------------------------------------------------------- */
int32_t lux_fileread(int32_t narg, int32_t ps[])
 /* raw file read routine: fileread,lun,array,start,num,type */
 /* the file must be opened with a lun, the start position and num are in units
 of the Byte size for the data type (types 0 to 4 for I*1,I*2,I*4,F*4,F*8); i.e.,
 if the type is 2 (I*4) then the file is read starting at address start*4 and
 num*4 bytes are read */
 /* if start is <0, then we just read from current position */
/* RAS 1999 */
/* Headers:
   <stdio.h>: FILE, fseek(), perror(), fread(), printf()
 */
{
  int32_t	iq, lun, j, n, start, num, nd;
  Symboltype type;
  extern void	wait_sec(float);
  extern int32_t	byte_count;
  char	*p;
  FILE	*fp;

  if (int_arg_stat(ps[0], &lun) != LUX_OK)
    return LUX_ERROR;
  if (lun < 0 || lun >= MAXFILES)
    return cerror(ILL_LUN, ps[0]);
  if (lux_file_open[lun] == 0)
    return cerror(LUN_CLOSED, ps[0]);
  fp = lux_file[lun];
  if (int_arg_stat(ps[2], &start) != LUX_OK
      || int_arg_stat(ps[3], &num) != LUX_OK
      || int_arg_stat(ps[4], (int32_t*) &type) != LUX_OK)
    return LUX_ERROR;
  if (type < LUX_INT8 || type > LUX_DOUBLE)
    return cerror(ILL_TYPE, ps[4]);
  iq = ps[1];
  nd = 1;
  if (redef_array(iq, type, nd, &num) != LUX_OK)
    return LUX_ERROR;

  p = (char *) array_data(iq);

  n = lux_type_size[type];
  /* the starting offset is */
  if (start >=0) {
    j = n * start;
 /*printf("j,n,recn = %d %d %d\n", j,n,recn);*/
    if (fseek(fp, j, 0) == -1) {
      perror("fileread");
      return LUX_ERROR;
    }
  }
  /* now read the file */
  n = n * num;
  if ((j = fread(p, 1, n, fp)) != n) {
    if ( j <= 0 )
      return cerror(READ_ERR, 0);
    printf("only got %d bytes in readfile, expected %d\n",j,n);
    /* sometimes (much too often!) a network problem, so try again */
    wait_sec(10.0);
    printf("trying again for a complete read\n");
    j = fread(p, 1, n, fp);
    if (j !=n)  /* still a problem */
      printf("still short after 2 tries, got %d bytes\n", j);
  }
  byte_count = j;
  return LUX_OK;
}
 /*------------------------------------------------------------------------- */
int32_t lux_filewrite(int32_t narg, int32_t ps[])
 /* raw file write routine: filewrite,lun,array,[start] */
 /* the file must be opened with a lun, the start position and num are in units
 of the Byte size for the data type (types 0 to 4 for I*1,I*2,I*4,F*4,F*8);
 i.e., if the type is 2 (I*4) then we write starting at address start*4 and
 num*4 bytes are read */
 /* if there is no start argument, we just write from wherever the file
    pointer is, there is no fseek */
/* Headers:
   <stdio.h>: FILE, fseek(), perror(), fwrite(), printf()
 */
{
  int32_t	iq, lun, type, j, start, num, typesize;
  char	*p;
  FILE	*fp;

  if (int_arg_stat(ps[0], &lun) != LUX_OK)
    return LUX_ERROR;
  if (lun < 0 || lun >= MAXFILES)
    return cerror(ILL_LUN, ps[0]);
  if (lux_file_open[lun] == 0)
    return cerror(LUN_CLOSED, ps[0]);
  fp = lux_file[lun];
  iq = ps[1]; 
  type = symbol_type(iq);
  switch (symbol_class(iq)) {
    case LUX_SCAL_PTR:
      iq = class8_to_1(iq);	/*scalar ptr case */
    case LUX_SCALAR:
      p = (char *) &scalar_value(iq).l;	/*scalar case */
      typesize = num = lux_type_size[type];
      break;
    case LUX_STRING:		/*string */
      p = (char *) string_value(iq);
      typesize = 1;
      num = string_size(iq);	/*don't include the null */
      break;
    case LUX_ARRAY:		/*array case */
      p = (char *) array_data(iq);
      typesize = num = lux_type_size[type];
      num = array_size(iq);
      break;
    default:
      return cerror(ILL_CLASS, iq);
  }
  if (num <= 0)
    return LUX_ERROR;
  start = -1;
  if (narg > 2 && int_arg_stat(ps[2], &start) == LUX_ERROR)
    return LUX_ERROR;
  /* the starting offset is */
  if (start >= 0) {
    j = typesize * start;
    /*printf("j,n,start = %d %d %d\n", j,n, start);*/
    if (fseek(fp, j, 0) == -1) {
      perror("filewrite"); 
      return LUX_ERROR;
    }
  } 
  /* now write the file */
  if ((j = fwrite(p, 1, num, fp)) != num) {
    if (j <= 0)
      return cerror(WRITE_ERR, 0);
    printf("only got %d bytes in writefile, expected %d\n",j,num);
  }
  return LUX_OK;
}
/*------------------------------------------------------------------------- */

