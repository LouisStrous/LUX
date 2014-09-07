/* This is file printf_extensions.cc.

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
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <printf.h>
#include <obstack.h>
#include <math.h>
#include <malloc.h>
#include "calendar.hh"

#define obstack_chunk_alloc malloc
#define obstack_chunk_free free

static const char *const dfmts[] =
  { "%d", "%+d", "% d", "%+ d", "%-d", "%+-d", "%- d", "%+ -d",
    "%0d", "%0+d", "%0 d", "%0+ d", "%0-d", "%0+-d", "%0- d", "%0+ -d" };
static const char *const ffmts[] =
  { "%#*.*f", "%+#*.*f", "%# *.*f", "%+# *.*f", "%-#*.*f", "%+-#*.*f", "%-# *.*f", "%+# -*.*f",
     "%0#*.*f", "%0+#*.*f", "%0# *.*f", "%0+# *.*f", "%0-#*.*f", "%0+-#*.*f", "%0-# *.*f", "%0+# -*.*f" };

static int32_t obstack_printf_sexagesimal_int_left(struct obstack *o,
                                               const struct printf_info *info,
                                               double value, double *lastvalue,
                                               int32_t firstwidth)
{
  int32_t ivalue, sign, size, precision;

  size = obstack_object_size(o);
  precision = info->prec;
  if (precision < 1)
    precision = 1;
  /* cannot just print first component value as-is with appropriate
     print format, because the first component value might be zero
     when the whole value is negative, and then we would not get the
     minus sign on the fist component value */
  sign = (value < 0)? -1: 1;
  value = fabs(value);
  ivalue = (int32_t) value;
  if (info->pad == L'0') {      /* sign comes first */
    if (sign < 0)
      obstack_1grow(o, '-');
    else if (info->showsign)
      obstack_1grow(o, '+');
    else if (info->space)
      obstack_1grow(o, ' ');
    if (firstwidth > 0)
      obstack_printf(o, "%0*d", firstwidth, 0);
  } else {                      /* sign comes after padding */
    obstack_printf(o, "%*s", firstwidth, "");
    if (sign < 0)
      obstack_1grow(o, '-');
    else if (info->showsign)
      obstack_1grow(o, '+');
    else if (info->space)
      obstack_1grow(o, ' ');
  }
  obstack_printf(o, "%d", ivalue);
  while (precision-- > 0) {	/* do the rest of them */
    value = 60*(value - ivalue);
    ivalue = (int32_t) value;
    obstack_printf(o, ":%02d", ivalue);
  }
  if (lastvalue)
    *lastvalue = value;
  return obstack_object_size(o) - size;
}

int32_t printf_sexagesimal(FILE *stream, const struct printf_info *info,
                       const void *const *args)
{
  int32_t width;
  double lastvalue;
  double value = *((const double *)(args[0]));
  struct obstack o;

  if (info->alt)                /* %#T */
    value /= 15;                /* transform from degrees to hours */

  /* we must take the specified width and justification into account.
     we do this by first printing the number without any regards to
     padding, and then checking if we need to move it any. */
  obstack_init(&o);
  width = obstack_printf_sexagesimal_int_left(&o, info, value, &lastvalue, 0);
  if (width < info->width) {    /* room left over */
    if (info->left) {
      if (info->prec == 0) {
        int32_t fmttype;

        fmttype = 0;
        if (info->showsign)
          fmttype++;
        if (info->space)
          fmttype += 2;
        if (info->left)
          fmttype += 4;
        if (info->pad == L'0')
          fmttype += 8;
        obstack_blank(&o, -width);
        obstack_printf(&o, ffmts[fmttype], info->width,
                       info->width - width - 1, value);
      } else {
        obstack_blank(&o, -2);
        obstack_printf(&o, "%0#*.*f", info->width - width + 2,
                       info->width - width - 1, lastvalue);
      }
    } else {                    /* !info->left */
      obstack_blank(&o, -width);
      obstack_printf_sexagesimal_int_left(&o, info, value, NULL, info->width - width);
    }
  }
  width = obstack_object_size(&o);
  obstack_1grow(&o, '\0');      /* ensure terminating \0 */
  char *p = (char*) obstack_finish(&o);
  fputs(p, stream);
  obstack_free(&o, 0);
  return width;
}

int32_t printf_double1_arginfo(const struct printf_info *p,
                           size_t n, int32_t *argtypes, int32_t *size)
{
  if (n > 0) {
    argtypes[0] = PA_DOUBLE;
    *size = sizeof(double);
  }
  return 1;
}

static int32_t obstack_printf_date_int_left(struct obstack *o,
                                        const struct printf_info *info,
                                        double value, double *last,
                                        int32_t firstwidth)
{
  int32_t year, month, ivalue, prec, size;
  double day, lastvalue;
  int32_t fmttype = 0;

  size = obstack_object_size(o);
  if (info->showsign)
    fmttype++;
  if (info->space)
    fmttype += 2;
  if (info->left)
    fmttype += 4;
  prec = info->prec;
  if (prec < 0)
    prec = 3;
  CJDtoCommon(JDtoCJD(value), &year, &month, &day);
  if (info->pad == L'0') {      /* sign comes first */
    if (year < 0)
      obstack_1grow(o, '-');
    else if (info->showsign)
      obstack_1grow(o, '+');
    else if (info->space)
      obstack_1grow(o, ' ');
    if (firstwidth > 0)
      obstack_printf(o, "%0*d", firstwidth, 0);
  } else {                      /* sign comes after padding */
    obstack_printf(o, "%*s", firstwidth, "");
    if (year < 0)
      obstack_1grow(o, '-');
    else if (info->showsign)
      obstack_1grow(o, '+');
    else if (info->space)
      obstack_1grow(o, ' ');
  }
  obstack_printf(o, "%d", year < 0? -year: year);
  if (prec == 1) {
    double JD1 = CJDtoJD(CommontoCJD(year, 1, 1));
    double JD2 = CJDtoJD(CommontoCJD(year + 1, 1, 1));
    lastvalue = (value - JD1)/(JD2 - JD1) + year;
  } else if (prec >= 2) {       /* month */
    obstack_printf(o, "-%02d", month);
    if (prec == 2) {
      double JD1 = CJDtoJD(CommontoCJD(year, month, 1));
      double JD2 = CJDtoJD(CommontoCJD(year, month + 1, 1));
      lastvalue = (value - JD1)/(JD2 - JD1) + month;
    } else if (prec >= 3) {     /* day */
      ivalue = (int32_t) day;
      obstack_printf(o, "-%02d", ivalue);
      if (prec == 3)
        lastvalue = day;
      else if (prec >= 4) {     /* hour &c */
        lastvalue = 24*(day - ivalue);
        prec -= 3;
        obstack_1grow(o, 'T');
        while (prec > 0) {
          ivalue = (int32_t) lastvalue;
          obstack_printf(o, "%02d", ivalue);
          if (--prec) {
            obstack_1grow(o, ':');
            lastvalue = 60*(lastvalue - ivalue);
          }
        }
      }
    }
  }
  if (last)
    *last = lastvalue;
  return obstack_object_size(o) - size;
}

/* .1 -> year
   .2 -> year-month
   .3 -> year-month-day
   .4 -> year-month-dayThour
   .5 -> year-month-dayThour:minute
   .6 -> year-month-dayThour:minute:second */
int32_t printf_date(FILE *stream, const struct printf_info *info,
                const void *const *args)
{
  double value = *((const double *)(args[0]));
  double value2;
  void JDtoDate(double, int32_t *, int32_t *, double *, int32_t);
  struct obstack o;
  size_t width;
  int32_t fmttype = 0;

  if (info->showsign)
    fmttype++;
  if (info->space)
    fmttype += 2;
  if (info->left)
    fmttype += 4;
  obstack_init(&o);
  width = obstack_printf_date_int_left(&o, info, value, &value2, 0);
  if (width && width < info->width) { /* still room left over */
    if (info->left) {
      if (info->prec == 1) {    /* year only */
        obstack_blank(&o, -width);
        obstack_printf(&o, ffmts[fmttype], info->width,
                       info->width - width - 1, value2);
      } else {
        obstack_blank(&o, -2);
        obstack_printf(&o, "%0#*.*f", (int32_t) (info->width - width + 2),
                       (int32_t) (info->width - width - 1), value2);
      }
    } else {
      obstack_blank(&o, -width);
      obstack_printf_date_int_left(&o, info, value, NULL, info->width - width);
    }
  }
  width = obstack_object_size(&o);
  obstack_1grow(&o, '\0');      /* ensure terminating \0 */
  char *p = (char*) obstack_finish(&o);
  fputs(p, stream);
  obstack_free(&o, 0);
  return width;
}

/*
  Experiments show that the following format specifier letters cannot
  be registered: I L Z h j l q t z

  The following letters are already in use: A C E G L S X Z a c d e f
  g h i j l m n o p q s t u x z

  This leaves the following format specifier letters available to use
  for extensions: B D F H J K M N O P Q R T U V W Y b k v w y
 */

void constructor(void) __attribute__ ((constructor));

void constructor(void)
{
  register_printf_specifier('T', printf_sexagesimal,
                            printf_double1_arginfo);
  register_printf_specifier('J', printf_date,
                            printf_double1_arginfo);
}

void destructor(void) __attribute__ ((destructor));
void destructor(void)
{
  register_printf_specifier('T', 0, 0);
  register_printf_specifier('J', 0, 0);
}
