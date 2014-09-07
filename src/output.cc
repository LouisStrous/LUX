/* This is file output.cc.

Copyright 2013-2014 Louis Strous

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

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

extern char	recording;
extern FILE	*recordFile;

/*----------------------------------------------------------------------*/
int32_t Printf(const char *format, ...)
{
  va_list	ap;
  int32_t	result;

  va_start(ap, format);
  if (recording & 2)		/* record output */
    vfprintf(recordFile, format, ap);
  result = vprintf(format, ap);
  va_end(ap);
  return result;
}
/*----------------------------------------------------------------------*/
int32_t Fprintf(FILE *stream, const char *format, ...)
{
  va_list	ap;
  int32_t	result;

  va_start(ap, format);
  if ((stream == stdout || stream == stderr) && (recording & 2))
						/* record output */
    vfprintf(recordFile, format, ap);
  result = vfprintf(stream, format, ap);
  va_end(ap);
  return result;
}
/*----------------------------------------------------------------------*/
int32_t Vprintf(const char *format, va_list arg)
{
  int32_t	result;

  if (recording & 2) /* record output */
    vfprintf(recordFile, format, arg);
  result = vprintf(format, arg);
  return result;
}  
/*----------------------------------------------------------------------*/
int32_t Vfprintf(FILE *stream, const char *format, va_list arg)
{
  int32_t	result;

  if ((stream == stdout || stream == stderr) && (recording & 2))
						/* record output */
    vfprintf(recordFile, format, arg);
  result = vfprintf(stream, format, arg);
  return result;
}
/*----------------------------------------------------------------------*/
int32_t Puts(const char *s)
{
  if (recording & 2)		/* record output */
  { fputs(s, recordFile);
    fputc('\n', recordFile); }
  return puts(s);
}  
/*----------------------------------------------------------------------*/
int32_t Fputs(const char *s, FILE *stream)
{
  if ((stream == stdout || stream == stderr)
      && (recording & 2)
      && (stream != recordFile))
    fputs(s, recordFile);
  return fputs(s, stream);
}
/*----------------------------------------------------------------------*/
int32_t Fputc(int32_t c, FILE *stream)
{
  if ((stream == stdout || stream == stderr)
      && (recording & 2)
      && (stream != recordFile))
    fputc(c, recordFile);
  return fputc(c, stream);
}
/*----------------------------------------------------------------------*/
