/* file output.c */
/* The following routines are like their counterparts with all small */
/* letters, but also record the output to a record file, if */
/* so instructed. */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdarg.h>
static char rcsid[] __attribute__ ((unused)) =
"$Id: output.c,v 4.0 2001/02/07 20:37:04 strous Exp $";

extern char	recording;
extern FILE	*recordFile;

/*----------------------------------------------------------------------*/
int Printf(const char *format, ...)
{
  va_list	ap;
  int	result;

  va_start(ap, format);
  if (recording & 2)		/* record output */
    vfprintf(recordFile, format, ap);
  result = vprintf(format, ap);
  va_end(ap);
  return result;
}
/*----------------------------------------------------------------------*/
int Fprintf(FILE *stream, const char *format, ...)
{
  va_list	ap;
  int	result;

  va_start(ap, format);
  if ((stream == stdout || stream == stderr) && (recording & 2))
						/* record output */
    vfprintf(recordFile, format, ap);
  result = vfprintf(stream, format, ap);
  va_end(ap);
  return result;
}
/*----------------------------------------------------------------------*/
int Vprintf(const char *format, va_list arg)
{
  int	result;

  if (recording & 2) /* record output */
    vfprintf(recordFile, format, arg);
  result = vprintf(format, arg);
  return result;
}  
/*----------------------------------------------------------------------*/
int Vfprintf(FILE *stream, const char *format, va_list arg)
{
  int	result;

  if ((stream == stdout || stream == stderr) && (recording & 2))
						/* record output */
    vfprintf(recordFile, format, arg);
  result = vfprintf(stream, format, arg);
  return result;
}
/*----------------------------------------------------------------------*/
int Puts(const char *s)
{
  if (recording & 2)		/* record output */
  { fputs(s, recordFile);
    fputc('\n', recordFile); }
  return puts(s);
}  
/*----------------------------------------------------------------------*/
int Fputs(const char *s, FILE *stream)
{
  if ((stream == stdout || stream == stderr)
      && (recording & 2)
      && (stream != recordFile))
    fputs(s, recordFile);
  return fputs(s, stream);
}
/*----------------------------------------------------------------------*/
int Fputc(int c, FILE *stream)
{
  if ((stream == stdout || stream == stderr)
      && (recording & 2)
      && (stream != recordFile))
    fputc(c, recordFile);
  return fputc(c, stream);
}
/*----------------------------------------------------------------------*/
