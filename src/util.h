#ifndef HAVE_UTIL_H
#define HAVE_UTIL_H

/**
   This interface provides a number of utilities of various kinds.
 */

/**
   Floating-point numbers can only be represented in a computer with
   finite accuracy, so calculations with floating-point numbers
   commonly involve round-off errors.  It is therefore likely that two
   different calculations that yield the exact same answer if infinite
   accuracy is available yield slightly different answers if only
   finite accuracy is available, with the difference being close to
   the limit of accuracy of the representation of floating-point
   numbers on the system.  It is therefore useful to define the
   concept of *floating-point comparison*, which can determine the
   relative order of two floating-point numbers, taking into account
   that accuracy is finite.

   Following Knuth 4.2.2, we define the relations "approximately equal
   to" (embodied in the function <approxeq>), "essentially equal to"
   (<essenteq>), "definitely less than" (<definitelt>), and
   "definitely greater than" (<definitegt>).  For any two
   floating-point numbers, exactly one of the relations "definitely
   less than", "approximately equal to", and "definitely greater than"
   holds.  The relation "essentially equal to" is at least as strong
   as "approximately equal to".

   If we envisage each floating-point number surrounded by a small
   segment of the number line, the size of which represents the
   inaccuracy, then "essentially equal to" means that each number is
   within the segment surrounding the other number, "approximately
   equal to" means that one or both of the numbers is in the segment
   surrounding the other number, and "definitely less than" and
   "definitely greater than" means that neither of the numbers is in
   the segment surrounding the other number.

   The function mentioned above compare <Double> values.  The
   corresponding functions that deal with <Float> values have <_f>
   appended to their name, and the corresponding functions that deal
   with <long Double> values have <_l> appended to their name.
 */

Int approxeq(const Double v1, const Double v2) __attribute__ ((const));
Int approxeq_l(const long Double v1, const long Double v2) __attribute__ ((const));
Int approxeq_f(const Float v1, const Float v2) __attribute__ ((const));
/**
   <approxeq> returns non-zero if the two arguments are approximately
   equal, and 0 if they are not, taking the possibility of round-off
   errors into account.  <approxeq> is weaker than <essenteq>.

   <approxeq_l> and <approxeq_f> are similar to <approxeq>, but deal
   with <long Double> and <Float> arguments rather than <Double>
   arguments.
 */

Int essenteq(const Double v1, const Double v2) __attribute__ ((const));
Int essenteq_l(const long Double v1, const long Double v2) __attribute__ ((const));
Int essenteq_f(const Float v1, const Float v2) __attribute__ ((const));
/**
   <essenteq> returns non-zero if the two arguments are essentially
   equal, and 0 if they are not, taking the possibility of round-off
   errors into account.  <essenteq> is stronger than <approxeq>.

   <essenteq_l> and <essenteq_f> are similar to <essenteq>, but deal
   with <long Double> and <Float> arguments rather than <Double>
   arguments.
 */

Int definitelt(const Double v1, const Double v2) __attribute__ ((const));
Int definitelt_l(const long Double v1, const long Double v2) __attribute__ ((const));
Int definitelt_f(const Float v1, const Float v2) __attribute__ ((const));
/**
   <definitelt> returns non-zero if the first argument is definitely
   less than the second, and 0 if it is not, taking the possibility of
   round-off errors into account.

   <definitelt_l> and <definitelt_f> are similar to <definitelt>, but
   deal with <long Double> and <Float> arguments rather than <Double>
   arguments.
 */

Int definitegt(const Double v1, const Double v2) __attribute__ ((const));
Int definitegt_l(const long Double v1, const long Double v2) __attribute__ ((const));
Int definitegt_f(const Float v1, const Float v2) __attribute__ ((const));
/**
   <definitegt> returns non-zero if the first argument is definitely
   greater than the second, and 0 if it is not, taking the possibility
   of round-off errors into account.

   <definitegt_l> and <definitegt_f> are similar to <definitegt>, but
   deal with <long Double> and <Float> arguments rather than <Double>
   arguments.
 */

#endif
