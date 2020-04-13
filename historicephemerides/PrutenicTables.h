#ifndef PRUTENICTABLES_H_
#define PRUTENICTABLES_H_

/** \file */

#ifdef __cplusplus
extern "C" {
#endif

# include <math.h>               /* for fabs */

  double s2d(int count, double* values);
# define s2dm(x) s2d(sizeof(x)/sizeof((x)[0]), (x))

  /** A struct to represent the quotient and remainder of a division of
      double values. */
  struct fdiv_t {
    double quot;                /*!< the quotient */
    double rem;                 /*!< the remainde */
  };

  /** Return the quotient and nonnegative remainder of dividing two double
      values.

      \param[in] x is the value to divide (dividend, numerator).

      \param[in] y is the value to divide by (divisor, denominator).

      \returns the quotient and remainder.  If \a y is 0, then both are
      NaN.  Otherwise, the remainder is nonnegative and less than the
      magnitude of \a y.  If `result = pfdiv(x,y)` then `x =
      result.quot*y + result.rem`. */
  struct fdiv_t pfdiv(double x, double y);

  /** Returns the ecliptic longitude of the Sun based on the Prutenic
      Tables of Erasmus Rheinhold.

      \param[in] days_since_epoch is the number of days since the epoch
      of the Prutenic Tables, measured relative to the meridian of the
      Prutenic Tables.

      \returns the ecliptic longitude, in degrees.
  */
  double prutenic_longitude_sol(double days_since_epoch);

  double prutenic_longitude_luna(double days_since_epoch);

  double prutenic_nearest_new_moon(double days_since_epoch);

#ifdef __cplusplus
}
#endif
#endif
