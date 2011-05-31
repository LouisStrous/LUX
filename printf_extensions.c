#include <stdio.h>
#include <printf.h>
#include <obstack.h>
#include <math.h>
#include <malloc.h>
#include "calendar.h"

#define obstack_chunk_alloc malloc
#define obstack_chunk_free free

static const char *const dfmts[] =
  { "%d", "%+d", "% d", "%+ d", "%-d", "%+-d", "%- d", "%+ -d" };
static const char *const ffmts[] = 
  { "%#*.*f", "%+#*.*f", "%# *.*f", "%+# *.*f", "%-#*.*f", "%+-#*.*f", "%-# *.*f", "%+# -*.*f" };

static int obstack_printf_sexagesimal_int_left(struct obstack *o,
                                               const struct printf_info *info,
                                               double value, int prefix_zero,
                                               double *last)
{
  int ivalue, sign, size, precision;

  size = obstack_object_size(o);
  precision = info->prec;
  if (precision < 1)
    precision = 1;
  sign = (value < 0)? -1: 1;
  value = fabs(value);
  ivalue = (int) value;
  if (sign < 0)
    obstack_1grow(o, '-');
  else if (info->showsign)
    obstack_1grow(o, '+');
  else if (info->space)
    obstack_1grow(o, ' ');
  if (prefix_zero)
    obstack_printf(o, "%02d", ivalue);
  else
    obstack_printf(o, "%d", ivalue);
  while (precision-- > 0) {	/* do the rest of them */
    value = 60*(value - ivalue);
    ivalue = (int) value;
    obstack_printf(o, ":%02d", ivalue);
  }
  if (last)
    *last = value;
  return obstack_object_size(o) - size;
}

int printf_sexagesimal(FILE *stream, const struct printf_info *info,
                       const void *const *args)
{
  int sign, width;
  double last;
  double value = *((const double *)(args[0]));
  struct obstack o;
  
  if (info->alt)                /* %#T */
    value /= 15;                /* transform from degrees to hours */

  /* we must take the specified width and justification into account.
     we do this by first printing the number without any regards to
     padding, and then checking if we need to move it any. */
  obstack_init(&o);
  width = obstack_printf_sexagesimal_int_left(&o, info, value, 0, &last);
  if (width < info->width) {    /* room left over */
    if (info->left) {
      if (info->prec == 0) {
        int fmttype;

        fmttype = 0;
        if (info->showsign)
          fmttype++;
        if (info->space)
          fmttype += 2;
        if (info->left)
          fmttype += 4;
        obstack_blank(&o, -width);
        obstack_printf(&o, ffmts[fmttype], info->width,
                       info->width - width - 1, value);
      } else if (info->width - width - 1 > 0 || fabs(value) >= 100) {
        obstack_blank(&o, -2);
        obstack_printf(&o, "%0#*.*f", info->width - width + 2,
                       info->width - width - 1, last);
      } else {
        obstack_blank(&o, -width);
        obstack_printf_sexagesimal_int_left(&o, info, value, 1, NULL);
      }
    } else {                    /* !info->left */
      obstack_blank(&o, -width);
      obstack_printf(&o, "%*s", info->width - width, "");
      obstack_printf_sexagesimal_int_left(&o, info, value, 0, NULL);
    }
  }
  width = obstack_object_size(&o);
  obstack_grow0(&o, 0, 0);      /* ensure terminating \0 */
  char *p = obstack_finish(&o);
  fputs(p, stream);
  obstack_free(&o, 0);
  return width;
}

int printf_double1_arginfo(const struct printf_info *p,
                           size_t n, int *argtypes, int *size)
{
  if (n > 0) {
    argtypes[0] = PA_DOUBLE;
    *size = sizeof(double);
  }
  return 1;
}

static int obstack_printf_date_int_left(struct obstack *o,
                                        const struct printf_info *info,
                                        double value,
                                        double *last)
{
  int year, month, ivalue, prec, size;
  double day, lastvalue;
  int fmttype = 0;
  
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
  JDtoCommonDate(value, &year, &month, &day);
  obstack_printf(o, dfmts[fmttype], year);
  if (prec == 1) {
    double JD1 = CommonDateToJD(year, 1, 1);
    double JD2 = CommonDateToJD(year + 1, 1, 1);
    lastvalue = (value - JD1)/(JD2 - JD1) + year;
  } else if (prec >= 2) {       /* month */
    obstack_printf(o, "-%02d", month);
    if (prec == 2) {
      double JD1 = CommonDateToJD(year, month, 1);
      double JD2 = CommonDateToJD(year, month + 1, 1);
      lastvalue = (value - JD1)/(JD2 - JD1) + month;
    } else if (prec >= 3) {     /* day */
      ivalue = (int) day;
      obstack_printf(o, "-%02d", ivalue);
      if (prec == 3)
        lastvalue = day;
      else if (prec >= 4) {     /* hour &c */
        lastvalue = 24*(day - ivalue);
        prec -= 3;
        obstack_1grow(o, 'T');
        while (prec > 0) {
          ivalue = (int) lastvalue;
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
int printf_date(FILE *stream, const struct printf_info *info,
                const void *const *args)
{
  double value = *((const double *)(args[0]));
  double value2;
  int year, month, iday;
  double day;
  void JDtoDate(double, int *, int *, double *, int);
  char *months[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
    "Sep", "Oct", "Nov", "Dec"
  };
  struct obstack o;
  size_t width, n;
  int fmttype = 0;
  
  if (info->showsign)
    fmttype++;
  if (info->space)
    fmttype += 2;
  if (info->left)
    fmttype += 4;
  obstack_init(&o);
  width = obstack_printf_date_int_left(&o, info, value, &value2);
  if (width && width < info->width) { /* still room left over */
    if (info->left) {
      if (info->prec == 1) {    /* year only */
        obstack_blank(&o, -width);
        obstack_printf(&o, ffmts[fmttype], info->width,
                       info->width - width - 1, value2);
      } else {
        obstack_blank(&o, -2);
        obstack_printf(&o, "%0#*.*f", info->width - width + 2,
                       info->width - width - 1, value2);
      }
    } else {
      obstack_blank(&o, -width);
      obstack_printf(&o, "%*s", info->width - width, "");
      obstack_printf_date_int_left(&o, info, value, NULL);
    }      
  }
  width = obstack_object_size(&o);
  obstack_grow0(&o, 0, 0);      /* ensure terminating \0 */
  char *p = obstack_finish(&o);
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
