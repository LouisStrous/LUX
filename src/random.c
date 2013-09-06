/* File random.c */
/* Various functions related to pseudo-random numbers */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <math.h> /* for cos(2) sqrt(2) log(2) sin(1) isnan(1) */
#include <string.h> /* for memcpy(5) */
#include "action.h"
#include "install.h"

#include <gsl/gsl_rng.h>

static uint32_t	currentBitSeed = 123459876;

static gsl_rng *rng;

/*------------------------------------------------------------------------- */
void random_init(Int seed)
{
  uint64_t s;

  s = (uint64_t) seed;
  if (!rng)
    rng = gsl_rng_alloc(gsl_rng_mt19937);
  gsl_rng_set(rng, s);
}
/*------------------------------------------------------------------------- */
Double random_one(void)
/* Returns a single uniformly distributed pseudo-random number */
/* between 0 and 1 (exclusive). */
{
  if (!rng)
    rng = gsl_rng_alloc(gsl_rng_mt19937);
  return gsl_rng_uniform(rng);
}
/*------------------------------------------------------------------------- */
Int locate_value(Double value, Double *values, Int nElem)
{
  Int	ilo, ihi, imid;

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

Int random_distributed(Int modulus, Double *distr)
/* returns a random number between 0 and <modulus - 1> drawn according */
/* to the distribution function <distr>, which must have at least */
/* <modulus> elements, with distr[0] == 0 and distr[modulus - 1] <= 1, */
/* and no element smaller than the previous one.  LS 25aug2000 */
{
  Double r;

  r = random_one();
  return locate_value(r, distr, modulus);
}
/*------------------------------------------------------------------------- */
Byte random_bit(void)
/* Returns a random bit (either zero or one), using primitive polynomials */
/* modulo 2. */
{
  static uint32_t	mask = 0x9, mask1 = 0x80000000;

  if (currentBitSeed & mask1) {
    currentBitSeed = ((currentBitSeed ^ mask) << 1) | 1;
    return (Byte) 1;
  } else {
    currentBitSeed = (currentBitSeed << 1) & ~1;
    return (Byte) 0;
  }
}
/*------------------------------------------------------------------------- */
uint32_t random_bits(void)
/* Returns a Int-full of random bits, using primitive polynomials modulo 2. */
{
  static uint32_t	mask = 0x9, mask1 = 0x80000000;
  Int	n = 32;
  uint32_t	result = 0;
  
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
/*------------------------------------------------------------------------- */
void randomu(Int seed, void *output, Int number, Int modulo)
/* generates <number> uniformly distributed pseudo-random numbers */
/* and stores them in <output>, for which */
/* memory must have been allocated by the user.  <seed> is an optional seed. */
/* a negative value indicates complete reinitialization of the random */
/* sequence.  a positive value indicates a mere change of seed.  a zero */
/* value indicates that the current random sequence is continued. */
/* if <modulo> is zero, then <output> is considered (Float *) and the */
/* generated random sequence is ANA_DOUBLE.  if <modulo> is positive, then */
/* <output> is considered (Int *) and the generated random sequence */
/* runs between 0 and <modulo> - 1 (inclusive) */
{
 Int	j;
 Double	*fp;
 Int	*ip;

 /* check if we are initializing */
 if (seed)
   random_init(seed);
 if (modulo) {			/* integers */
   if (modulo < 0)
     modulo = -modulo;
   ip = (Int *) output;
   for (j = 0; j < number; j++)
     *ip++ = (Int) (random_one()*modulo);
 } else { /* floating point */
   fp = (Double *) output;
   for (j = 0; j < number; j++)
     *fp++ = random_one();
 }
}
/*------------------------------------------------------------------------- */
void randome(void *output, Int number)
/* generates <number> exponentially distributed (with unit scale)
   pseudo-random numbers and stores them in <output>, for which memory
   must have been allocated by the user.  Both positive and negative
   numbers are returned */
{
 Int	j;
 Double	*fp;

 fp = (Double *) output;
 for (j = 0; j < number; j++) {
   Double value = 2*random_one() - 1;
   int negative = (value < 0);
   if (value) {
     *fp++ = log(fabs(value))*(negative? -1: 1);
   } else
     *fp++ = INFTY;
 }
}
/*----------------------------------------------------------------------*/
void random_unique(Int seed, Int *output, Int number, Int modulo)
/* generates <number> uniformly distributed integer pseudo-random numbers */
/* in the range 0 to <modulo> - 1 (inclusive) in which no particular */
/* number appears more than once, and stores them in <output>, for which */
/* memory must have been allocated by the user.  <seed> is an optional seed. */
/* a negative value indicates complete reinitialization of the random */
/* sequence.  a positive value indicates a mere change of seed.  a zero */
/* value indicates that the current random sequence is continued. */
/* Note that it is impossible to get such a unique-number sequence if */
/* <number> is more than <modulo>. */
/* LS 24nov95 */
{
  Int	m, t;

  if (number > modulo) {	/* both are assumed positive */
    anaerror("random_unique: asked %1d unique numbers modulo %1d: Impossible",
	  0, number, modulo);
    return;
  }
  /* initialize random number generator, if necessary */
  if (seed)
    random_init(seed);
  /* Use Knuth's Algorithm S (D. Knuth, Seminumerical Algorithms) */
  t = m = 0;
  while (m < number) {
    if ((Int) ((modulo - t)*random_one()) < number - m) {
      *output++ = t;
      m++;
    }
    t++;
  }
}
/*----------------------------------------------------------------------*/
void random_unique_shuffle(Int seed, Int *output, Int number, Int modulo)
/* generates <number> uniformly distributed integer pseudo-random numbers */
/* in the range 0 to <modulo> - 1 (inclusive) in which no particular */
/* number appears more than once, and stores them in <output>, for which */
/* memory must have been allocated by the user.  <seed> is an optional seed. */
/* a negative value indicates complete reinitialization of the random */
/* sequence.  a positive value indicates a mere change of seed.  a zero */
/* value indicates that the current random sequence is continued. */
/* Note that it is impossible to get such a unique-number sequence if */
/* <number> is less than <modulo>.  The random numbers are shuffled.  */
/* LS 5oct97 */
{
  Int	i, j, temp;

  random_unique(seed, output, number, modulo);
  if (number > modulo)
    return;			/* error */
  /* now shuffle.  I hope this algorithm is sufficient.  LS */
  for (i = 0; i < number; i++) {
    j = (Int) (random_one()*number);
    temp = output[i];
    output[i] = output[j];
    output[j] = temp;
  }
}
/*----------------------------------------------------------------------*/
void randomn(Int seed, Double *output, Int number, char hasUniform)
/* returns <number> pseudo-random numbers following the standard */
/* normal distribution (mean zero, standard deviation one), using the */
/* Box-Muller transformation.  Sufficient memory for the numbers */
/* must have been allocated by the user at <output>.  If <hasUniform> */
/* is non-zero, then it is assumed that uniformly distributed */
/* pseudo-random numbers are already present in <output>, */
/* otherwise they are generated in this routine.  LS 25oct95 */
{
  Int	n, i;
  Double	r, a, extra[2];

  n = number%2;
  if (!hasUniform)
    /* first get uniformly distributed numbers between 0 and 1. */
    randomu(seed, output, number, 0);
  for (i = 0; i < number/2 ; i++) {
    r = sqrt(-2.0*log(*output));
    a = output[1]*2*M_PI;
    *output++ = r*cos(a);
    *output++ = r*sin(a);	/* is r*sqrt(1-c*c) with c=cos(a) quicker? */
  }
  if (n) {			/* single number left */
    randomu(0, extra, 2, 0);
    r = sqrt(-2.0*log(extra[0]));
    a = extra[1]*2*M_PI;
    *output++ = r*cos(a);
  }
}  
/*----------------------------------------------------------------------*/
Int ana_randomu(Int narg, Int ps[])
 /*create an array of random elements in the [0,1.0] range (exclusive) */
{
 Double *p;
 Int	k, seed, cycle;
 Int	dims[8], *pd, j, result_sym, n;

 if (*ps) {
   seed = int_arg(*ps);
   if (seed > 0)
     seed = -seed;
 } else
   seed = 0;
 ps++;
 narg--;
 if (!narg) {			/* no more arguments */
   random_init(seed);		/* just reinitialize */
   return ANA_ONE;
 }
 if (*ps)
   cycle = int_arg(*ps);
 else
   cycle = 0;
 ps++;
 narg--;
 if (symbol_class(*ps) == ANA_ARRAY) {
   if (narg > 1)
     return anaerror("Dimension list must be either all scalars or one array",
		  *ps);
   k = ana_long(1, ps);
   narg = array_size(k);
   pd = array_data(k);
 } else {
   for (j = 0; j < narg; j++)
     dims[j] = int_arg(ps[j]); /*get the dimensions */
   pd = dims;
 }
 result_sym = array_scratch(cycle? ANA_LONG: ANA_DOUBLE, narg, pd);
 if (result_sym == ANA_ERROR)
   return ANA_ERROR;
 p = array_data(result_sym);
 n = array_size(result_sym);
 randomu(seed, p, n, cycle);
 return result_sym;
}
/*------------------------------------------------------------------------- */
Int ana_randomd(Int narg, Int ps[])
/* RANDOMD([seed,]distr,dimens) generates a LONG array with dimensions */
/* <dimens> with each number drawn at random from the range 0 through */
/* <NUM_ELEM(distr) - 1> (inclusive) according to the distribution */
/* specified by <distr>.  The elements of <distr> must be monotonically */
/* increasing, <distr(0)> must be greater than or equal to 0, and */
/* <distr(*-1)> must be equal to one.  LS 25aug2000 */
{
  Int	result, dims[MAX_DIMS], *pd, n, j, modulus, seed;
  Double	*distr;

  if (*ps) {			/* seed */
    seed = int_arg(ps[0]);
    random_init(seed);
  };

  if (!symbolIsNumericalArray(ps[1])) /* distr */
    return cerror(NEED_ARR, ps[1]);
  result = ana_double(1, ps + 1); /* ensure DOUBLE */
  distr = array_data(result);
  modulus = array_size(result);
  if (distr[0] < 0.0 || distr[modulus - 1] != 1.0)
    return anaerror("Illegal probability distribution specified", ps[1]);

  if (symbolIsNumericalArray(ps[2])) { /* dimensions */
    if (narg > 3)
      return anaerror("Dimension list must be either all scalars or one array",
		   ps[2]);
    result = ana_long(1, ps + 2);
    pd = array_data(result);
    n = array_size(result);
  } else {
    for (j = 2; j < narg; j++)
      dims[j - 2] = int_arg(ps[j]);
    n = narg - 2;
    pd = dims;
  }

  result = array_scratch(ANA_LONG, n, pd);
  if (result == ANA_ERROR)
    return ANA_ERROR;
  pd = array_data(result);
  n = array_size(result);
  while (n--)
    *pd++ = random_distributed(modulus, distr);
  return result;
}
/*------------------------------------------------------------------------- */
Int ana_randomn(Int narg, Int ps[])
 /*create a normal distribution of pseudo-random #'s, centered at 0 with */
 /* a width of 1.0
 uses Box-Muller transformation, given 2 uniformly random #'s (0 to 1 range)

 x1 and x2 then         n1 = sqrt(-2.*alog(x1))*cos(2*pi*x2)

                        n2 = sqrt(-2.*alog(x1))*sin(2*pi*x2)

 */
{
  Int	result_sym;

  /* first get a uniform distribution */
  result_sym = ana_randomu(narg,ps);
  if (result_sym == ANA_ERROR	/* an error occurred */
      || result_sym == ANA_ONE)	/* we just initialized with a specific seed */
    return result_sym;
  /* then apply Box-Muller transformation */
  randomn(0, (Double *) array_data(result_sym), array_size(result_sym), 1);
  return result_sym;
}
/*------------------------------------------------------------------------- */
Int ana_randome(Int narg, Int ps[])
 /* create an exponential distribution of pseudo-random #'s, centered
    at 0 with a given scale length */
{
  Double *p, scale;
  Int	k;
  Int	dims[8], *pd, j, result_sym, n;

  scale = double_arg(*ps++);
  --narg;
  if (symbol_class(*ps) == ANA_ARRAY) {
    if (narg > 1)
      return anaerror("Dimension list must be either all scalars or one array",
		      *ps);
    k = ana_long(1, ps);
    narg = array_size(k);
    pd = array_data(k);
  } else {
    for (j = 0; j < narg; j++)
      dims[j] = int_arg(ps[j]); /*get the dimensions */
    pd = dims;
  }
  result_sym = array_scratch(ANA_DOUBLE, narg, pd);
  if (result_sym == ANA_ERROR)
    return ANA_ERROR;
  p = array_data(result_sym);
  n = array_size(result_sym);
  randome(p, n);
  while (n--) {
    *p *= scale;
    ++p;
  }
  return result_sym;
}
REGISTER(randome, f, RANDOME, 2, MAX_DIMS, 0);
/*------------------------------------------------------------------------- */
Int ana_randomb(Int narg, Int ps[])
/* RANDOMB([SEED=seed,] dimens, [/LONG]) */
/* returns a BYTE array of the indicated dimensions where each value */
/* is either a 0 or a 1. LS 21jul98 */
{
  Int	dims[MAX_DIMS], ndim, iq, result, n;
  pointer p;

  if (*ps) 			/* seed */
    currentBitSeed = int_arg(*ps); /* install new seed */

  ps += 2;			/* move to first dimension symbol */
  narg -= 2;
  ndim = 0;			/* initialize total number of dimensions */
  while (narg--) {		/* treat all dimension symbols */
    switch (symbol_class(*ps)) {
      case ANA_SCALAR:
	dims[ndim++] = int_arg(*ps); /* add a single dimension */
	break;
      case ANA_ARRAY:
	if (ndim + array_size(*ps) > MAX_DIMS)
	  return anaerror("Too many dimensions specified", 0);
	iq = ana_long(1, ps);	/* ensure LONG dimensions */
	memcpy(dims + ndim, array_data(iq), array_size(iq)*sizeof(Int));
	ndim += array_size(iq);
	break;
      default:
	return cerror(ILL_CLASS, *ps);
    }
    ps++;			/* go to next argument */
  }

  result = array_scratch((internalMode & 1)? ANA_LONG: ANA_BYTE, ndim, dims);
  if (result == ANA_ERROR)	/* some error? */
    return ANA_ERROR;
  p.l = array_data(result);	/* pointer to result data */

  n = array_size(result);	/* number of bits to get */
  if (internalMode &1)          /* /LONG: get a LONG-full of bits */
    while (n--)
      *p.l++ = random_bits();
  else
    while (n--)
      *p.b++ = random_bit();	/* get one bit */

  return result;
}
/*------------------------------------------------------------------------- */
Int ana_randoml(Int narg, Int ps[])
/* RANDOML([SEED=seed,] dimens) */
/* returns a FLOAT or DOUBLE (if /DOUBLE is set) array of the indicated */
/* dimensions, filled with values drawn from a logarithmic distribution */
/* over all representable numbers.  LS 27aug2000 */
{
  Int	dims[MAX_DIMS], ndim, iq, result, n;
  pointer	p;
  Byte	type;

  if (*ps) 			/* seed */
    currentBitSeed = int_arg(*ps); /* install new seed */

  ps += 2;			/* move to first dimension symbol */
  narg -= 2;
  ndim = 0;			/* initialize total number of dimensions */
  while (narg--) {		/* treat all dimension symbols */
    switch (symbol_class(*ps)) {
      case ANA_SCALAR:
	dims[ndim++] = int_arg(*ps); /* add a single dimension */
	break;
      case ANA_ARRAY:
	if (ndim + array_size(*ps) > MAX_DIMS)
	  return anaerror("Too many dimensions specified", 0);
	iq = ana_long(1, ps);	/* ensure LONG dimensions */
	memcpy(dims + ndim, array_data(iq), array_size(iq)*sizeof(Int));
	ndim += array_size(iq);
	break;
      default:
	return cerror(ILL_CLASS, *ps);
    }
    ps++;			/* go to next argument */
  }

  type = (internalMode & 1)? ANA_DOUBLE: ANA_FLOAT;

  result = array_scratch(type, ndim, dims);
  if (result == ANA_ERROR)	/* some error? */
    return ANA_ERROR;
  p.v = array_data(result);	/* pointer to result data */

  /* we generate random bits by the Int-full, so we do a loop over */
  /* the number of Int-fulls in the array */
  n = array_size(result)*ana_type_size[type]/sizeof(Int);
  /* some of the generated bit patterns do not correspond to representable */
  /* numbers but rather to NaNs.  We must replace those with representable */
  /* numbers.  We use a single scheme for both FLOAT and DOUBLE numbers, */
  /* assuming that when the bit patterns of a DOUBLE NaN are interpreted */
  /* as the bit patterns of two FLOAT numbers instead, then at least one */
  /* of those FLOATs is a FLOAT NaN, and replacing that FLOAT NaN with */
  /* a representable FLOAT number generates a representable DOUBLE */
  /* number. */
  while (n--) {
    do 
      *p.l = random_bits();	/* get one Int-full of bits */
    while (isnan(*p.f));
    p.l++;
  }

  return result;
}
/*------------------------------------------------------------------------- */
Int ana_random(Int narg, Int ps[])
/* General pseudo-random-number generating routine.  Switches select the */
/* distribution of the numbers.  General Syntax: */
/*   Y = RANDOM([SEED=seed, PERIOD=period,] dimens */
/*              [,/UNIFORM,/NORMAL,/SAMPLE,/BITS]) */
/* Subcases: */
/*   Y = RANDOM([SEED=seed,] dimens [,/UNIFORM]) */
/*       generates a ANA_FLOAT array with dimensions <dimens>, containing */
/*       numbers uniformly distributed between 0 and 1 (exclusive) */
/*   Y = RANDOM([SEED=seed,] PERIOD=period, dimens [,/UNIFORM]) */
/*       generates a ANA_LONG array with dimensions <dimens>, containing */
/*       numbers uniformly distributed between 0 and <period> - 1 */
/*       (inclusive), where any number may occur more than once. */
/*   Y = RANDOM([SEED=seed,] PERIOD=period, dimens, /SAMPLE) */
/*       generates a ANA_LONG array with dimensions <dimens>, containing */
/*       numbers uniformly distributed between 0 and <period> - 1 */
/*       (inclusive), but any number occurs at most once.  The number */
/*       of elements must not be larger than <period>.  The numbers are */
/*       ordered in ascending order. */
/*   Y = RANDOM([SEED=seed,] PERIOD=period, dimens, /SHUFFLE) */
/*       generates a ANA_LONG array with dimensions <dimens>, containing */
/*       numbers uniformly distributed between 0 and <period> - 1 */
/*       (inclusive), but any number occurs at most once.  The number */
/*       of elements must not be larger than <period>.  The numbers are */
/*       in random order. */
/*   Y = RANDOM([SEED=seed,] dimens, /NORMAL) */
/*       generates a ANA_FLOAT array with dimensions <dimens>, containing */
/*       numbers normally distributed with zero mean and unit standard */
/*       deviation. */
/*   Y = RANDOM([SEED=seed,] dimens, /BITS) */
/*       generates an ANA_BYTE array with dimensions <dimens>, containing */
/*       either a 0 or a 1, drawn at random with equal probability.  */
{
  Int	result, ndim, dims[MAX_DIMS], iq, i, period, seed;
  Byte	*p;

  if (!internalMode)
    internalMode = 1;
  if (ps[0]) {			/* SEED */
    seed = int_arg(ps[0]);
    if (seed > 0)
      seed = -seed;
  } else
    seed = 0;
  if (internalMode == 5) 	/* /BITS */
    currentBitSeed = (uint32_t) seed;
  else {			/* other distributions */
    if (seed) /* need to (re)initialize */
      random_init(seed);
  }
  ndim = 0;
  if (narg > 2)			/* dimensions were specified */
    for (i = 2; i < narg; i++) {
      switch (symbol_class(ps[i])) {
	case ANA_ARRAY:
	  if (ndim + array_size(ps[i]) > MAX_DIMS)
	    return anaerror("Too many dimensions", 0);
	  iq = ana_long(1, &ps[i]); /* ensure LONG dimensions */
	  memcpy(dims + ndim, array_data(iq), ndim*sizeof(Int));
	  ndim += array_size(iq);
	  break;
	case ANA_SCALAR:
	  if (ndim == MAX_DIMS)
	    return anaerror("Too many dimensions", 0);
          if (!symbolIsScalar(ps[i]))
            return cerror(NEED_SCAL, ps[i]);
	  dims[ndim++] = int_arg(ps[i]);
          if (dims[ndim - 1] <= 0)
            return cerror(ILL_DIM, ps[i]);
	  break;
	default:
	  return cerror(ILL_CLASS, ps[i]);
      }	/* end of switch (symbol_class(ps[i])) */
    } /* end of for (i = 2; i < narg; i++) */
  else				/* no dimensions specified */
    return ANA_ONE;		/* we're done already */

  switch (internalMode) {
    case 1:			/* /UNIFORM */
      if (narg > 1 && ps[1]) {	/* PERIOD specified, so get LONGs */
        if (!symbolIsScalar(ps[1]))
          return cerror(NEED_SCAL, ps[1]);
	period = int_arg(ps[1]);
	result = array_scratch(ANA_LONG, ndim, dims);
	if (result == ANA_ERROR)
	  return ANA_ERROR;
	randomu(seed, (Int *) array_data(result), array_size(result),
		period);
      } else {			/* no PERIOD, so get FLOATs */
	result = array_scratch(ANA_DOUBLE, ndim, dims);
	randomu(seed, (Double *) array_data(result), array_size(result), 0);
      }
      break;
    case 2:			/* /NORMAL */
      if (narg > 1 && ps[1])	/* PERIOD specified - illegal */
	return anaerror("RANDOM - no PERIOD allowed with /NORMAL", ps[1]);
      result = array_scratch(ANA_DOUBLE, ndim, dims);
      if (result == ANA_ERROR)
	return ANA_ERROR;
      randomn(seed, (Double *) array_data(result), array_size(result), 0);
      break;
    case 3: case 4:			/* /SAMPLE, /SHUFFLE */
      if (narg < 1 || !ps[1])	/* PERIOD absent, but is required */
	return anaerror("RANDOM - need PERIOD with /SAMPLE", 0);
      period = int_arg(ps[1]);	/* PERIOD */
      result = array_scratch(ANA_LONG, ndim, dims);
      if (result == ANA_ERROR)
	return ANA_ERROR;
      if (internalMode == 3)
	random_unique(seed, (Int *) array_data(result), array_size(result),
		      period);
      else
	random_unique_shuffle(seed, (Int *) array_data(result),
			      array_size(result), period);
      break;
    case 5:			/* /BITS */
      if (narg > 1 && ps[1])	/* PERIOD specified - illegal */
	return anaerror("RANDOM - no PERIOD allowed with /BITS", ps[1]);
      result = array_scratch(ANA_BYTE, ndim, dims);
      if (result == ANA_ERROR)
	return ANA_ERROR;
      iq = array_size(result);
      p = array_data(result);
      while (iq--)
	*p++ = random_bit();
      break;
    default:			/* unknown or illegal combination */
      return anaerror("RANDOM - unknown distribution (%1d)", internalMode);
  }
  return result;
}
