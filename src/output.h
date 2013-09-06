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
