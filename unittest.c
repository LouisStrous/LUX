/* HEADERS */
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
/* END HEADERS */
#include <stdarg.h> /* for va_end(1) va_start(1) va_list(1) */
#include <stddef.h> /* for size_t(1) */
#include <stdio.h> /* for printf(2) putchar(2) vprintf(1) */
#include <stdio.h> /* for printf(1) putchar(1) */
#include "unittest.h"

Int num_assertions = 0;
Int assertion_count = 0;
char *assertion_file = NULL;
Int assertion_line = 0;
char *assertion_func = NULL;

void assertion_new(const char *file, const char *func) {
  if (!assertion_file || strcmp(assertion_file, file)
      || !func || strcmp(assertion_func, func))
    printf("\n%s(%s) [%d] ", file, func, assertion_count);
  assertion_file = (char *) file;
  assertion_func = (char *) func;
}

Int assertion(const char *file, const Int line, const char *func,
	      const Int condition, const char *text)
{
  num_assertions++;
  assertion_count++;
  if (!assertion_file || strcmp(assertion_file, file))
    printf("\n***No testinit() used! %s(%s) [%d] ", file, func, assertion_count);
  assertion_file = (char *) file;
  assertion_line = line;
  assertion_func = (char *) func;
  if (condition) {
    putchar('.');
    return 0;
  } else {
    printf("\n%s:%d (%s) [%d] {errno=%d}: %s\n", file, line, func,
	   assertion_count, errno, text);
    if (errno)
      perror(">>");
    errno = 0;
    return 1;
  }
}

Int assertionEquals(const char *file, const Int line, const char *func,
		    const Int equal, const char *format, ...)
{
  va_list ap;

  num_assertions++;
  assertion_count++;
  if (!assertion_file || strcmp(assertion_file, file))
    printf("\n***No testinit() used! %s(%s) [%d] ", file, func, assertion_count);
  assertion_file = (char *) file;
  assertion_line = line;
  assertion_func = (char *) func;
  if (equal) {
    putchar('.');
    return 0;
  } else {
    va_start(ap, format);
    printf("\n%s:%d (%s) [%d] {errno=%d}: ", file, line, func,
	   assertion_count, errno);
    vprintf(format, ap);
    va_end(ap);
    putchar('\n');
    if (errno)
      perror(">>");
    errno = 0;
    return 1;
  }
}
