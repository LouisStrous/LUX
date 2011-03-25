#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "vsop.h"
#include <math.h>
#include <stdio.h>
#include "anaparser.h"

/*--------------------------------------------------------------------------*/
int	getAstronError = 0;
int	fullVSOP = 1;

void gatherVSOP(double T, struct planetIndex *index, double *terms, 
                     double *value, double *error2)
/* calculates one coordinate of one object, as indicated by <index>,
   at time <T> in Julian centuries since J2000.0, using the VSOP87
   theory of Bretagnon & Francou (1988).  The heliocentric longitude,
   latitude, (in radians) or radius (in AU) is returned in <*value>.
   The associated estimated variance is returned in <*error2>. */
{
  double	*ptr;
  int	nTerm, i;

  *value = 0.0;			/* initialize */
  if (getAstronError)
    *error2 = 0.0;
  for (i = 5; i >= 0; i--) {	/* powers of T */
    *value *= T;		/* move previous stuff to next power of T */
    if (getAstronError)
      *error2 *= T;
    nTerm = index[i].nTerms;	/* number of terms to add */
    if (nTerm) {
      ptr = terms + 3*(index[i].index); /* points at first term */
      while (nTerm--) {
	*value += ptr[0]*cos(ptr[1] + ptr[2]*T); /* add term */
	ptr += 3;		/* go to next term for this coordinate */
      }
      if (getAstronError)		/* variance estimate: number of terms
				 times square of amplitude of smallest
				 (last) term times a number smaller than 4 */
	*error2 += index[i].nTerms*ptr[-3]*ptr[-3];
    }
  }
  if (getAstronError)			/* apply factor of 4 mentioned before */
    *error2 = 4* *error2;
}
/*--------------------------------------------------------------------------*/
void XYZfromVSOPA(double T, int object, double *pos)
/* returns the heliocentric cartesian coordinates referred to the mean
 dynamical ecliptic and equinox of J2000.0 using the VSOP87A theory
 (as described in Meeus: Astronomical Algorithms).  pos[0] -> X,
 pos[1] -> Y, pos[2] -> Z; if getAstronError is unequal to zero, then
 also pos[3] -> var[X], pos[4] -> var[Y], pos[5] -> var[Z], pos[6] ->
 cov[X,Y], pos[7] -> cov[X,Z], pos[8] -> cov[Y,Z].  The covariances
 are assumed equal to zero (for want of a regular estimate). */
{
  struct planetIndex *indices;

  /* TODO: implement "Trunc" based on user-selected accuracy
    indices = fullVSOP? planetIndices: planetIndicesTrunc; */
  indices = planetIndices;
  switch (object) {
    case 0:			/* Sun */
      pos[0] = pos[1] = pos[2] = 0.0;
      if (getAstronError)
	pos[3] = pos[4] = pos[5] = pos[6] = pos[7] = pos[8] = 0.0;
      break;
    default:			/* other planets */
				/* heliocentric ecliptic X (AU) */
      gatherVSOP(T, &indices[6*3*(object - 1)], planetTerms, &pos[0],
		      getAstronError? &pos[3]: NULL);
				/* heliocentric ecliptic Y (AU) */
      gatherVSOP(T, &indices[6*3*(object - 1) + 6], planetTerms, 
                      &pos[1], getAstronError? &pos[4]: NULL);
				/* heliocentric ecliptic Z (AU) */
      gatherVSOP(T, &indices[6*3*(object - 1) + 12], planetTerms, 
                 &pos[2], getAstronError? &pos[5]: NULL);
      if (getAstronError)
	pos[6] = pos[7] = pos[8] = 0.0;
      break;
  }
}
