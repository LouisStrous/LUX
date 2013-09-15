/* This is file output.c.

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
/* file output.c */
/* The following routines are like their counterparts with all small */
/* letters, but also record the output to a record file, if */
/* so instructed. */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdarg.h>
#include "types.h"

extern char	recording;
extern FILE	*recordFile;

/*----------------------------------------------------------------------*/
Int Printf(const char *format, ...)
{
  va_list	ap;
  Int	result;

  va_start(ap, format);
  if (recording & 2)		/* record output */
    vfprintf(recordFile, format, ap);
  result = vprintf(format, ap);
  va_end(ap);
  return result;
}
/*----------------------------------------------------------------------*/
Int Fprintf(FILE *stream, const char *format, ...)
{
  va_list	ap;
  Int	result;

  va_start(ap, format);
  if ((stream == stdout || stream == stderr) && (recording & 2))
						/* record output */
    vfprintf(recordFile, format, ap);
  result = vfprintf(stream, format, ap);
  va_end(ap);
  return result;
}
/*----------------------------------------------------------------------*/
Int Vprintf(const char *format, va_list arg)
{
  Int	result;

  if (recording & 2) /* record output */
    vfprintf(recordFile, format, arg);
  result = vprintf(format, arg);
  return result;
}  
/*----------------------------------------------------------------------*/
Int Vfprintf(FILE *stream, const char *format, va_list arg)
{
  Int	result;

  if ((stream == stdout || stream == stderr) && (recording & 2))
						/* record output */
    vfprintf(recordFile, format, arg);
  result = vfprintf(stream, format, arg);
  return result;
}
/*----------------------------------------------------------------------*/
Int Puts(const char *s)
{
  if (recording & 2)		/* record output */
  { fputs(s, recordFile);
    fputc('\n', recordFile); }
  return puts(s);
}  
/*----------------------------------------------------------------------*/
Int Fputs(const char *s, FILE *stream)
{
  if ((stream == stdout || stream == stderr)
      && (recording & 2)
      && (stream != recordFile))
    fputs(s, recordFile);
  return fputs(s, stream);
}
/*----------------------------------------------------------------------*/
Int Fputc(Int c, FILE *stream)
{
  if ((stream == stdout || stream == stderr)
      && (recording & 2)
      && (stream != recordFile))
    fputc(c, recordFile);
  return fputc(c, stream);
}
/*----------------------------------------------------------------------*/
