#ifndef HAVE_UNITTEST_H
#define HAVE_UNITTEST_H
#include <stdarg.h>		/* for ... */
#include <stddef.h> /* for NULL(2) size_t(1) */
#include <string.h> /* for strcmp(1) */
#include "util.h"		/* for approxeq */

/**
   This interface provides macros and functions that are useful in
   unit tests.  Unit tests are tests that check the correct working of
   one or more functions.
 */

#define assertNotNULL(x) \
  assertion(__FILE__, __LINE__, __func__, ((x) != NULL), \
  "Unexpected NULL for " #x)
/**
   <assertNotNULL> asserts that <<x>> is not equal to the <NULL>
   pointer, and prints a suitable error message otherwise.  The error
   message reports the file, line number, and function name where the
   error occurred.
 */

#define assertNULL(x) \
  assertionEquals(__FILE__, __LINE__, __func__, ((x) == NULL), \
  "Expected " #x " = %s but found %p for " #x, "NULL", (x))
/**
   <assertNULL> asserts that <<x>> is equal to the <NULL> pointer, and
   prints a suitable error message otherwise.  The error message
   reports the file, line number, and function name where the error
   occurred.
*/

#define assertTrue(x) \
  assertion(__FILE__, __LINE__, __func__, ((x) != 0), \
  "Expected " #x " to be true")
/**
   <assertTrue> asserts that <<x>> is not equal to 0, and prints a
   suitable error message otherwise.  The error message reports the
   file, line number, and function name where the error occurred.
*/

#define assertEqualTexts(expected, found) \
  assertionEquals(__FILE__, __LINE__, __func__, !strcmp((expected), (found)), \
  "Expected " #expected " = %s but found %s for " #found, (expected), (found))
/**
   <assertEqualTexts> asserts that the texts at <<expected>> and
   <<found>> are equal, and prints a suitable error message otherwise.
   The error message reports the file, line number, and function name
   where the error occurred.
 */

#define assertUnequalTexts(unexpected, found) \
  assertionEquals(__FILE__, __LINE__, __func__, strcmp((unexpected), (found)), \
  "Expected text different from " #unexpected)
/**
   <assertUnequalTexts> asserts that the texts at <<unexpected>> and
   <<found>> are unequal, and prints a suitable error message otherwise.
   The error message reports the file, line number, and function name
   where the error occurred.
 */

#define assertEqualInts(expected, found) \
  assertionEquals(__FILE__, __LINE__, __func__, (expected) == (found), \
  "Expected " #expected " = %d but found %d for " #found, (expected), (found))
/**
   <assertEqualInts> asserts that the integer values <<expected>> and
   <<found>> are equal, and prints a suitable error message otherwise.
   The error message reports the file, line number, and function name
   where the error occurred.
 */

#define assertEqualULongs(expected, found) \
  assertionEquals(__FILE__, __LINE__, __func__, (expected) == (found), \
  "Expected " #expected " = %lu but found %lu for " #found, (expected), (found))
/**
   <assertEqualULongs> asserts that the <unsigned long int> values
   <<expected>> and <<found>> are equal, and prints a suitable error
   message otherwise.  The error message reports the file, line
   number, and function name where the error occurred.
 */

#define assertEqualLlongs(expected, found) \
  assertionEquals(__FILE__, __LINE__, __func__, (expected) == (found), \
  "Expected " #expected " = %Ld but found %Ld for " #found, (expected), (found))
/**
   <assertEqualLlongs> asserts that the <long long int> values
   <<expected>> and <<found>> are equal, and prints a suitable error
   message otherwise.  The error message reports the file, line
   number, and function name where the error occurred.
 */

#define assertEqualFloats(expected, found) \
  assertionEquals(__FILE__, __LINE__, __func__, approxeq_f((expected), (found)), \
  "Expected " #expected " = %g but found %g for " #found, (expected), (found))
/**
   <assertEqualFloats> asserts that the <float> values <<expected>>
   and <<found>> are approximately equal (taking reasonable round-off
   errors into account), and prints a suitable error message
   otherwise.  The error message reports the file, line number, and
   function name where the error occurred.
 */

#define assertEqualDoubles(expected, found) \
  assertionEquals(__FILE__, __LINE__, __func__, approxeq((expected), (found)), \
  "Expected " #expected " = %g but found %g for " #found, (expected), (found))
/**
   <assertEqualDoubles> asserts that the <double> values <<expected>>
   and <<found>> are approximately equal (taking reasonable round-off
   errors into account), and prints a suitable error message
   otherwise.  The error message reports the file, line number, and
   function name where the error occurred.
 */

#define assertEqualLdoubles(expected, found) \
  assertionEquals(__FILE__, __LINE__, __func__, approxeq_l((expected), (found)), \
  "Expected " #expected " = %Lg but found %Lg for " #found, (expected), (found))
/**
   <assertEqualLdoubles> asserts that the <long double> values
   <<expected>> and <<found>> are approximately equal (taking
   reasonable round-off errors into account), and prints a suitable
   error message otherwise.  The error message reports the file, line
   number, and function name where the error occurred.
 */

#define assertUnequalInts(unexpected, found) \
  assertionEquals(__FILE__, __LINE__, __func__, (unexpected) != (found), \
  "Did not expect " #unexpected " = %d for " #found, (unexpected))
/**
   <assertUnequalInts> asserts that the integer values <<unexpected>>
   and <<found>> are not equal, and prints a suitable error message
   otherwise.  The error message reports the file, line number, and
   function name where the error occurred.
 */

#define assertEqualPtrs(expected, found) \
  assertionEquals(__FILE__, __LINE__, __func__, ((void *) (expected) == (void *) (found)), \
  "Expected " #expected " = %p but found %p for " #found, (expected), (found))
/**
   <assertEqualPtrs> asserts that the pointers <<expected>> and
   <<found>> are equal, and prints a suitable error message otherwise.
   The error message reports the file, line number, and function name
   where the error occurred.
 */

int assertion(const char *file, const int line, const char *func,
	      const int condition, const char *text);
/**
   <assertion> asserts that the <<condition>> is true, and prints a
   suitable error message otherwise.  <<file>> is supposed to point to
   the file name, <<line>> is supposed to be the line number, <<func>>
   is taken to point to the name of the function where the error
   occurred.  <<text>> is the descriptive text to print.

   It is better to use one of the predefined macros that automatically
   fill in the file name, line number, function name, and descriptive
   text.
 */

int assertionEquals(const char *file, const int line, const char *func,
		    const int equals, const char *format, ...)
     __attribute__ ((format (printf, 5, 6)));
/**
   <assertionEquals> asserts that <<equals>> is non-zero, and prints a
   suitable error message otherwise.  <<file>> is supposed to point to
   the file name, <<line>> is supposed to be the line number, <<func>>
   is taken to point to the name of the function where the error
   occurred.  <<format>> and following arguments are passed to
   <vprintf> to print the descriptive text.

   It is better to use one of the predefined macros that automatically
   fill in the file name, line number, function name, and descriptive
   text.
 */

int num_assertions;
/**
   <num_assertions> counts the number of assertions that have been
   made so far.
 */

int assertion_count;
/**
   <assertion_count> counts the number of assertions that have been
   made so far.  The value of <assertion_count> is printed in error
   messages generated by the assertion functions and macros defined in
   this interface.  The application may want to set this variable to
   zero at the beginning of a sequence of tests.
 */

#endif
