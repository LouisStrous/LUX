/* This is file output.h.

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
/* File output.h */
/*  */
/* $Id: output.h,v 4.0 2001/02/07 20:37:04 strous Exp $" */
/* Replaces output routines with similar routines that also record the */
/* output to a record file if so instructed. */
#if STDC_HEADERS
#include <stdio.h>
#include <stdarg.h>

/* we want to have the old output macros available */
#define oldputc		putc
#define oldputchar	putchar
#define oldfputc	fputc

/* redefine the output routine names to point at the enhanced ones */
#define printf		Printf
#define fprintf		Fprintf
#define vprintf		Vprintf
#define vfprintf	Vfprintf
#define puts		Puts
#define fputs		Fputs
#define fputc		Fputc

extern char recording;
extern FILE *recordFile;

Int Printf(const char *format, ...);
Int Fprintf(FILE *stream, const char *format, ...);
Int Vprintf(const char *format, va_list arg);
Int Vfprintf(FILE *stream, const char *format, va_list arg);
Int Puts(const char *s);
Int Fputs(const char *s, FILE *stream);
Int Fputc(Int c, FILE *stream);

/* also redefine the output macros: first undefine the old ones so no */
/* warnings ensue during compilation */
#undef putc
#undef putchar

#define putc(c, stream)\
  ((stream == stdout && recording & 2)? Fputc(c, stdout): oldputc(c, stream))
#define putchar(c)\
  ((recording && 2)? Fputc(c, stdout): oldputchar(c))
#endif
