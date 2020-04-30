/* This is file random.cc.

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
// File random.c
// Various functions related to pseudo-random numbers
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <math.h> // for cos(2) sqrt(2) log(2) sin(1) isnan(1)
#include <string.h> // for memcpy(5)
#include "action.hh"
#include "install.hh"

#include <gsl/gsl_rng.h>

static uint32_t        currentBitSeed = 123459876;

static gsl_rng *rng;

//-------------------------------------------------------------------------
void random_init(int32_t seed)
{
  uint64_t s;

  s = (uint64_t) seed;
  if (!rng)
    rng = gsl_rng_alloc(gsl_rng_mt19937);
  gsl_rng_set(rng, s);
}
//-------------------------------------------------------------------------
double random_one(void)
// Returns a single uniformly distributed pseudo-random number
// between 0 and 1 (exclusive).
{
  if (!rng)
    rng = gsl_rng_alloc(gsl_rng_mt19937);
  return gsl_rng_uniform(rng);
}
//-------------------------------------------------------------------------
int32_t locate_value(double value, double *values, int32_t nElem)
{
  int32_t        ilo, ihi, imid;

  ilo = 0;
  ihi = nElem - 1;
  while (ilo <= ihi) {
    imid = (ilo + ihi)/2;
    if (value < values[imid])
      ihi = imid - 1;
    else if (value > values[imid])
      ilo = imid + 1;
    else {
      ilo = imid;
      ihi = imid - 1;
      break;
    }
  }
  return ihi;
}

int32_t random_distributed(int32_t modulus, double *distr)
// returns a random number between 0 and <modulus - 1> drawn according
// to the distribution function <distr>, which must have at least
// <modulus> elements, with distr[0] == 0 and distr[modulus - 1] <= 1,
// and no element smaller than the previous one.  LS 25aug2000
{
  double r;

  r = random_one();
  return locate_value(r, distr, modulus);
}
//-------------------------------------------------------------------------
uint8_t random_bit(void)
// Returns a random bit (either zero or one), using primitive polynomials
// modulo 2.
{
  static uint32_t        mask = 0x9, mask1 = 0x80000000;

  if (currentBitSeed & mask1) {
    currentBitSeed = ((currentBitSeed ^ mask) << 1) | 1;
    return (uint8_t) 1;
  } else {
    currentBitSeed = (currentBitSeed << 1) & ~1;
    return (uint8_t) 0;
  }
}
//-------------------------------------------------------------------------
uint32_t random_bits(void)
// Returns a int32_t-full of random bits, using primitive polynomials modulo 2.
{
  static uint32_t        mask = 0x9, mask1 = 0x80000000;
  int32_t        n = 32;
  uint32_t        result = 0;

  while (n--) {
    if (currentBitSeed & mask1) {
      currentBitSeed = ((currentBitSeed ^ mask) << 1) | 1;
      result = (result << 1) | 1;
    } else {
      currentBitSeed = (currentBitSeed << 1) & ~1;
      result = result << 1;
    }
  }
  return result;
}
//-------------------------------------------------------------------------
void randomu(int32_t seed, void *output, int32_t number, int32_t modulo)
// generates <number> uniformly distributed pseudo-random numbers
// and stores them in <output>, for which
// memory must have been allocated by the user.  <seed> is an optional seed.
// a negative value indicates complete reinitialization of the random
// sequence.  a positive value indicates a mere change of seed.  a zero
// value indicates that the current random sequence is continued.
// if <modulo> is zero, then <output> is considered (float *) and the
// generated random sequence is LUX_DOUBLE.  if <modulo> is positive, then
// <output> is considered (int32_t *) and the generated random sequence
// runs between 0 and <modulo> - 1 (inclusive)
{
 int32_t        j;
 double        *fp;
 int32_t        *ip;

 // check if we are initializing
 if (seed)
   random_init(seed);
 if (modulo) {                        // integers
   if (modulo < 0)
     modulo = -modulo;
   ip = (int32_t *) output;
   for (j = 0; j < number; j++)
     *ip++ = (int32_t) (random_one()*modulo);
 } else { // floating point
   fp = (double *) output;
   for (j = 0; j < number; j++)
     *fp++ = random_one();
 }
}
//-------------------------------------------------------------------------
void randome(void *output, int32_t number, double limit)
/* generates <number> exponentially distributed (with unit scale)
   pseudo-random numbers and stores them in <output>, for which memory
   must have been allocated by the user.  Both positive and negative
   numbers are returned.  If <limit> is greater than 0, then only
   numbers whose magnitude is at least <limit> are returned */
{
 int32_t        j;
 double        *fp, value;

 if (limit < 0)
   limit = 0;
 fp = (double *) output;
 /*
    We want only numbers whose magnitude is at least |λ| = <limit>.
    random_one() returns uniformly distributed pseudorandom numbers
    ρ between 0 and 1.  -log(ρ) is exponentially distributed from 0
    with scale 1.

    Target distribution: f(x) = exp(λ-x) for x ≥ λ
    ∫f(x)dx = -exp(λ-x)
    ∫_λ^∞ f(x)dx = -exp(λ-∞) + exp(0) = 1
    F(x) = ∫_-∞^x f(y)dy
    x ≤ λ ⇒ F(x) = 0
    x ≥ λ ⇒ F(x) = 1 - exp(λ-x)
        y = 1 - exp(λ-x)
        log(1 - y) = λ - x
        x = λ - log(1 - y)
    F(x) = y ⇒ F⁻¹(y) = λ - log(1 - y)
    Transform T(u) = F⁻¹(u) = λ - log(1 - u)
 */
 for (j = 0; j < number; j++) {
   value = 2*random_one() - 1;        // uniform between -1 and +1
   int negative = (value < 0);
   if (negative)
     value = -value;                // uniform between 0 and +1
   if (value)
     value = limit - log(value); // exponential from <limit>
   else
     value = INFTY;
   if (negative)
     value = - value;
   *fp++ = value;
 }
}
//----------------------------------------------------------------------
void random_unique(int32_t seed, int32_t *output, int32_t number, int32_t modulo)
// generates <number> uniformly distributed integer pseudo-random numbers
// in the range 0 to <modulo> - 1 (inclusive) in which no particular
// number appears more than once, and stores them in <output>, for which
// memory must have been allocated by the user.  <seed> is an optional seed.
// a negative value indicates complete reinitialization of the random
// sequence.  a positive value indicates a mere change of seed.  a zero
// value indicates that the current random sequence is continued.
// Note that it is impossible to get such a unique-number sequence if
// <number> is more than <modulo>.
// LS 24nov95
{
  int32_t        m, t;

  if (number > modulo) {        // both are assumed positive
    luxerror("random_unique: asked %1d unique numbers modulo %1d: Impossible",
          0, number, modulo);
    return;
  }
  // initialize random number generator, if necessary
  if (seed)
    random_init(seed);
  // Use Knuth's Algorithm S (D. Knuth, Seminumerical Algorithms)
  t = m = 0;
  while (m < number) {
    if ((int32_t) ((modulo - t)*random_one()) < number - m) {
      *output++ = t;
      m++;
    }
    t++;
  }
}
//----------------------------------------------------------------------
void random_unique_shuffle(int32_t seed, int32_t *output, int32_t number, int32_t modulo)
// generates <number> uniformly distributed integer pseudo-random numbers
// in the range 0 to <modulo> - 1 (inclusive) in which no particular
// number appears more than once, and stores them in <output>, for which
// memory must have been allocated by the user.  <seed> is an optional seed.
// a negative value indicates complete reinitialization of the random
// sequence.  a positive value indicates a mere change of seed.  a zero
// value indicates that the current random sequence is continued.
// Note that it is impossible to get such a unique-number sequence if
// <number> is less than <modulo>.  The random numbers are shuffled.
// LS 5oct97
{
  int32_t        i, j, temp;

  random_unique(seed, output, number, modulo);
  if (number > modulo)
    return;                        // error
  // now shuffle.  I hope this algorithm is sufficient.  LS
  for (i = 0; i < number; i++) {
    j = (int32_t) (random_one()*number);
    temp = output[i];
    output[i] = output[j];
    output[j] = temp;
  }
}
//----------------------------------------------------------------------
void randomn(int32_t seed, double *output, int32_t number, char hasUniform)
// returns <number> pseudo-random numbers following the standard
// normal distribution (mean zero, standard deviation one), using the
// Box-Muller transformation.  Sufficient memory for the numbers
// must have been allocated by the user at <output>.  If <hasUniform>
// is non-zero, then it is assumed that uniformly distributed
// pseudo-random numbers are already present in <output>,
// otherwise they are generated in this routine.  LS 25oct95
{
  int32_t        n, i;
  double        r, a, extra[2];

  n = number%2;
  if (!hasUniform)
    // first get uniformly distributed numbers between 0 and 1.
    randomu(seed, output, number, 0);
  for (i = 0; i < number/2 ; i++) {
    r = sqrt(-2.0*log(*output));
    a = output[1]*2*M_PI;
    *output++ = r*cos(a);
    *output++ = r*sin(a);        // is r*sqrt(1-c*c) with c=cos(a) quicker?
  }
  if (n) {                        // single number left
    randomu(0, extra, 2, 0);
    r = sqrt(-2.0*log(extra[0]));
    a = extra[1]*2*M_PI;
    *output++ = r*cos(a);
  }
}
//----------------------------------------------------------------------
int32_t lux_randomu(int32_t narg, int32_t ps[])
 //create an array of random elements in the [0,1.0] range (exclusive)
{
 double *p;
 int32_t        k, seed, cycle;
 int32_t        dims[8], *pd, j, result_sym, n;

 if (*ps) {
   seed = int_arg(*ps);
   if (seed > 0)
     seed = -seed;
 } else
   seed = 0;
 ps++;
 narg--;
 if (!narg) {                        // no more arguments
   random_init(seed);                // just reinitialize
   return LUX_ONE;
 }
 if (*ps)
   cycle = int_arg(*ps);
 else
   cycle = 0;
 ps++;
 narg--;
 if (symbol_class(*ps) == LUX_ARRAY) {
   if (narg > 1)
     return luxerror("Dimension list must be either all scalars or one array",
                  *ps);
   k = lux_long(1, ps);
   narg = array_size(k);
   pd = (int32_t*) array_data(k);
 } else {
   for (j = 0; j < narg; j++)
     dims[j] = int_arg(ps[j]); //get the dimensions
   pd = dims;
 }
 result_sym = array_scratch(cycle? LUX_INT32: LUX_DOUBLE, narg, pd);
 if (result_sym == LUX_ERROR)
   return LUX_ERROR;
 p = (double*) array_data(result_sym);
 n = array_size(result_sym);
 randomu(seed, p, n, cycle);
 return result_sym;
}
//-------------------------------------------------------------------------
int32_t lux_randomd(int32_t narg, int32_t ps[])
// RANDOMD([seed,]distr,dimens) generates a LONG array with dimensions
// <dimens> with each number drawn at random from the range 0 through
// <NUM_ELEM(distr) - 1> (inclusive) according to the distribution
// specified by <distr>.  The elements of <distr> must be monotonically
// increasing, <distr(0)> must be greater than or equal to 0, and
// <distr(*-1)> must be equal to one.  LS 25aug2000
{
  int32_t        result, dims[MAX_DIMS], *pd, n, j, modulus, seed;
  double        *distr;

  if (*ps) {                        // seed
    seed = int_arg(ps[0]);
    random_init(seed);
  };

  if (!symbolIsNumericalArray(ps[1])) // distr
    return cerror(NEED_ARR, ps[1]);
  result = lux_double(1, ps + 1); // ensure DOUBLE
  distr = (double*) array_data(result);
  modulus = array_size(result);
  if (distr[0] < 0.0 || distr[modulus - 1] != 1.0)
    return luxerror("Illegal probability distribution specified", ps[1]);

  if (symbolIsNumericalArray(ps[2])) { // dimensions
    if (narg > 3)
      return luxerror("Dimension list must be either all scalars or one array",
                   ps[2]);
    result = lux_long(1, ps + 2);
    pd = (int32_t*) array_data(result);
    n = array_size(result);
  } else {
    for (j = 2; j < narg; j++)
      dims[j - 2] = int_arg(ps[j]);
    n = narg - 2;
    pd = dims;
  }

  result = array_scratch(LUX_INT32, n, pd);
  if (result == LUX_ERROR)
    return LUX_ERROR;
  pd = (int32_t*) array_data(result);
  n = array_size(result);
  while (n--)
    *pd++ = random_distributed(modulus, distr);
  return result;
}
//-------------------------------------------------------------------------
int32_t lux_randomn(int32_t narg, int32_t ps[])
 //create a normal distribution of pseudo-random #'s, centered at 0 with
 /* a width of 1.0
 uses Box-Muller transformation, given 2 uniformly random #'s (0 to 1 range)

 x1 and x2 then         n1 = sqrt(-2.*alog(x1))*cos(2*pi*x2)

                        n2 = sqrt(-2.*alog(x1))*sin(2*pi*x2)

 */
{
  int32_t        result_sym;

  // first get a uniform distribution
  result_sym = lux_randomu(narg,ps);
  if (result_sym == LUX_ERROR        // an error occurred
      || result_sym == LUX_ONE)        // we just initialized with a specific seed
    return result_sym;
  // then apply Box-Muller transformation
  randomn(0, (double *) array_data(result_sym), array_size(result_sym), 1);
  return result_sym;
}
//-------------------------------------------------------------------------
int32_t lux_randome(int32_t narg, int32_t ps[])
 /* create an exponential distribution of pseudo-random #'s, centered
    at 0 with a given scale length */
{
  double *p, scale, limit;
  int32_t        k;
  int32_t        dims[8], *pd, j, result_sym, n;

  limit = ps[0]? double_arg(ps[0]): 0;
  scale = double_arg(ps[1]);
  ps += 2;
  narg -= 2;
  if (symbol_class(*ps) == LUX_ARRAY) {
    if (narg > 1)
      return luxerror("Dimension list must be either all scalars or one array",
                      *ps);
    k = lux_long(1, ps);
    narg = array_size(k);
    pd = (int32_t*) array_data(k);
  } else {
    for (j = 0; j < narg; j++)
      dims[j] = int_arg(ps[j]); //get the dimensions
    pd = dims;
  }
  result_sym = array_scratch(LUX_DOUBLE, narg, pd);
  if (result_sym == LUX_ERROR)
    return LUX_ERROR;
  p = (double*) array_data(result_sym);
  n = array_size(result_sym);
  randome(p, n, scale? limit/fabs(scale): 0);
  while (n--) {
    *p *= scale;
    ++p;
  }
  return result_sym;
}
REGISTER(randome, f, randome, 3, MAX_DIMS, "%1%limit:scale");
//-------------------------------------------------------------------------
int32_t lux_randomb(int32_t narg, int32_t ps[])
// RANDOMB([SEED=seed,] dimens, [/LONG])
// returns a BYTE array of the indicated dimensions where each value
// is either a 0 or a 1. LS 21jul98
{
  int32_t        dims[MAX_DIMS], ndim, iq, result, n;
  Pointer p;

  if (*ps)                         // seed
    currentBitSeed = int_arg(*ps); // install new seed

  ps += 2;                        // move to first dimension symbol
  narg -= 2;
  ndim = 0;                        // initialize total number of dimensions
  while (narg--) {                // treat all dimension symbols
    switch (symbol_class(*ps)) {
      case LUX_SCALAR:
        dims[ndim++] = int_arg(*ps); // add a single dimension
        break;
      case LUX_ARRAY:
        if (ndim + array_size(*ps) > MAX_DIMS)
          return luxerror("Too many dimensions specified", 0);
        iq = lux_long(1, ps);        // ensure LONG dimensions
        memcpy(dims + ndim, (int32_t*) array_data(iq),
               array_size(iq)*sizeof(int32_t));
        ndim += array_size(iq);
        break;
      default:
        return cerror(ILL_CLASS, *ps);
    }
    ps++;                        // go to next argument
  }

  result = array_scratch((internalMode & 1)? LUX_INT32: LUX_INT8, ndim, dims);
  if (result == LUX_ERROR)        // some error?
    return LUX_ERROR;
  p.i32 = (int32_t*) array_data(result);        // pointer to result data

  n = array_size(result);        // number of bits to get
  if (internalMode &1)          // /LONG: get a LONG-full of bits
    while (n--)
      *p.i32++ = random_bits();
  else
    while (n--)
      *p.ui8++ = random_bit();        // get one bit

  return result;
}
//-------------------------------------------------------------------------
int32_t lux_randoml(int32_t narg, int32_t ps[])
// RANDOML([SEED=seed,] dimens)
// returns a FLOAT or DOUBLE (if /DOUBLE is set) array of the indicated
// dimensions, filled with values drawn from a logarithmic distribution
// over all representable numbers.  LS 27aug2000
{
  int32_t        dims[MAX_DIMS], ndim, iq, result, n;
  Pointer        p;
  Symboltype        type;

  if (*ps)                         // seed
    currentBitSeed = int_arg(*ps); // install new seed

  ps += 2;                        // move to first dimension symbol
  narg -= 2;
  ndim = 0;                        // initialize total number of dimensions
  while (narg--) {                // treat all dimension symbols
    switch (symbol_class(*ps)) {
      case LUX_SCALAR:
        dims[ndim++] = int_arg(*ps); // add a single dimension
        break;
      case LUX_ARRAY:
        if (ndim + array_size(*ps) > MAX_DIMS)
          return luxerror("Too many dimensions specified", 0);
        iq = lux_long(1, ps);        // ensure LONG dimensions
        memcpy(dims + ndim, (int32_t*) array_data(iq),
               array_size(iq)*sizeof(int32_t));
        ndim += array_size(iq);
        break;
      default:
        return cerror(ILL_CLASS, *ps);
    }
    ps++;                        // go to next argument
  }

  type = (internalMode & 1)? LUX_DOUBLE: LUX_FLOAT;

  result = array_scratch(type, ndim, dims);
  if (result == LUX_ERROR)        // some error?
    return LUX_ERROR;
  p.v = array_data(result);        // pointer to result data

  // we generate random bits by the int32_t-full, so we do a loop over
  // the number of int32_t-fulls in the array
  n = array_size(result)*lux_type_size[type]/sizeof(int32_t);
  // some of the generated bit patterns do not correspond to representable
  // numbers but rather to NaNs.  We must replace those with representable
  // numbers.  We use a single scheme for both FLOAT and DOUBLE numbers,
  // assuming that when the bit patterns of a DOUBLE NaN are interpreted
  // as the bit patterns of two FLOAT numbers instead, then at least one
  // of those FLOATs is a FLOAT NaN, and replacing that FLOAT NaN with
  // a representable FLOAT number generates a representable DOUBLE
  // number.
  while (n--) {
    do
      *p.i32 = random_bits();        // get one int32_t-full of bits
    while (isnan(*p.f));
    p.i32++;
  }

  return result;
}
//-------------------------------------------------------------------------
int32_t lux_random(int32_t narg, int32_t ps[])
// General pseudo-random-number generating routine.  Switches select the
// distribution of the numbers.  General Syntax:
//   Y = RANDOM([SEED=seed, PERIOD=period,] dimens
//              [,/UNIFORM,/NORMAL,/SAMPLE,/BITS])
// Subcases:
//   Y = RANDOM([SEED=seed,] dimens [,/UNIFORM])
//       generates a LUX_FLOAT array with dimensions <dimens>, containing
//       numbers uniformly distributed between 0 and 1 (exclusive)
//   Y = RANDOM([SEED=seed,] PERIOD=period, dimens [,/UNIFORM])
//       generates a LUX_INT32 array with dimensions <dimens>, containing
//       numbers uniformly distributed between 0 and <period> - 1
//       (inclusive), where any number may occur more than once.
//   Y = RANDOM([SEED=seed,] PERIOD=period, dimens, /SAMPLE)
//       generates a LUX_INT32 array with dimensions <dimens>, containing
//       numbers uniformly distributed between 0 and <period> - 1
//       (inclusive), but any number occurs at most once.  The number
//       of elements must not be larger than <period>.  The numbers are
//       ordered in ascending order.
//   Y = RANDOM([SEED=seed,] PERIOD=period, dimens, /SHUFFLE)
//       generates a LUX_INT32 array with dimensions <dimens>, containing
//       numbers uniformly distributed between 0 and <period> - 1
//       (inclusive), but any number occurs at most once.  The number
//       of elements must not be larger than <period>.  The numbers are
//       in random order.
//   Y = RANDOM([SEED=seed,] dimens, /NORMAL)
//       generates a LUX_FLOAT array with dimensions <dimens>, containing
//       numbers normally distributed with zero mean and unit standard
//       deviation.
//   Y = RANDOM([SEED=seed,] dimens, /BITS)
//       generates an LUX_INT8 array with dimensions <dimens>, containing
//       either a 0 or a 1, drawn at random with equal probability.
{
  int32_t        result, ndim, dims[MAX_DIMS], iq, i, period, seed;
  uint8_t        *p;

  if (!internalMode)
    internalMode = 1;
  if (ps[0]) {                        // SEED
    seed = int_arg(ps[0]);
    if (seed > 0)
      seed = -seed;
  } else
    seed = 0;
  if (internalMode == 5)         // /BITS
    currentBitSeed = (uint32_t) seed;
  else {                        // other distributions
    if (seed) // need to (re)initialize
      random_init(seed);
  }
  ndim = 0;
  if (narg > 2)                        // dimensions were specified
    for (i = 2; i < narg; i++) {
      switch (symbol_class(ps[i])) {
        case LUX_ARRAY:
          if (ndim + array_size(ps[i]) > MAX_DIMS)
            return luxerror("Too many dimensions", 0);
          iq = lux_long(1, &ps[i]); // ensure LONG dimensions
          memcpy(dims + ndim, (int32_t*) array_data(iq), ndim*sizeof(int32_t));
          ndim += array_size(iq);
          break;
        case LUX_SCALAR:
          if (ndim == MAX_DIMS)
            return luxerror("Too many dimensions", 0);
          if (!symbolIsScalar(ps[i]))
            return cerror(NEED_SCAL, ps[i]);
          dims[ndim++] = int_arg(ps[i]);
          if (dims[ndim - 1] <= 0)
            return cerror(ILL_DIM, ps[i]);
          break;
        default:
          return cerror(ILL_CLASS, ps[i]);
      }        // end of switch (symbol_class(ps[i]))
    } // end of for (i = 2; i < narg; i++)
  else                                // no dimensions specified
    return LUX_ONE;                // we're done already

  switch (internalMode) {
    case 1:                        // /UNIFORM
      if (narg > 1 && ps[1]) {        // PERIOD specified, so get LONGs
        if (!symbolIsScalar(ps[1]))
          return cerror(NEED_SCAL, ps[1]);
        period = int_arg(ps[1]);
        result = array_scratch(LUX_INT32, ndim, dims);
        if (result == LUX_ERROR)
          return LUX_ERROR;
        randomu(seed, (int32_t *) array_data(result), array_size(result),
                period);
      } else {                        // no PERIOD, so get FLOATs
        result = array_scratch(LUX_DOUBLE, ndim, dims);
        randomu(seed, (double *) array_data(result), array_size(result), 0);
      }
      break;
    case 2:                        // /NORMAL
      if (narg > 1 && ps[1])        // PERIOD specified - illegal
        return luxerror("RANDOM - no PERIOD allowed with /NORMAL", ps[1]);
      result = array_scratch(LUX_DOUBLE, ndim, dims);
      if (result == LUX_ERROR)
        return LUX_ERROR;
      randomn(seed, (double *) array_data(result), array_size(result), 0);
      break;
    case 3: case 4:                        // /SAMPLE, /SHUFFLE
      if (narg < 1 || !ps[1])        // PERIOD absent, but is required
        return luxerror("RANDOM - need PERIOD with /SAMPLE", 0);
      period = int_arg(ps[1]);        // PERIOD
      result = array_scratch(LUX_INT32, ndim, dims);
      if (result == LUX_ERROR)
        return LUX_ERROR;
      if (internalMode == 3)
        random_unique(seed, (int32_t *) array_data(result), array_size(result),
                      period);
      else
        random_unique_shuffle(seed, (int32_t *) array_data(result),
                              array_size(result), period);
      break;
    case 5:                        // /BITS
      if (narg > 1 && ps[1])        // PERIOD specified - illegal
        return luxerror("RANDOM - no PERIOD allowed with /BITS", ps[1]);
      result = array_scratch(LUX_INT8, ndim, dims);
      if (result == LUX_ERROR)
        return LUX_ERROR;
      iq = array_size(result);
      p = (uint8_t*) array_data(result);
      while (iq--)
        *p++ = random_bit();
      break;
    default:                        // unknown or illegal combination
      return luxerror("RANDOM - unknown distribution (%1d)", internalMode);
  }
  return result;
}
