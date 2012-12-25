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
static char rcsid[] __attribute__ ((unused)) =
"$Id: output.c,v 4.0 2001/02/07 20:37:04 strous Exp $";

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
