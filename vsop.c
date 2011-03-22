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
void LBRfromVSOPD(double T, int object, double *pos)
/* returns the heliocentric longitude, latitude, and distance referred
 to the mean dynamical ecliptic and equinox of the date using the
 VSOP87D theory (as described in Meeus: Astronomical Algorithms).
 pos[0] -> L, pos[1] -> B, pos[2] -> R, pos[3] -> var[L]; if
 getAstronError is unequal to zero, then also pos[4] -> var[B], pos[5]
 -> var[R], pos[6] -> cov[L,B], pos[7] -> cov[L,R], pos[8] ->
 cov[B,R].  The covariances are assumed equal to zero (for want of a
 regular estimate). */
{
  struct planetIndex *indices;

  indices = fullVSOP? planetIndices: planetIndicesTrunc;
  switch (object) {
    case 0:			/* Sun */
      pos[0] = pos[1] = pos[2] = 0.0;
      if (getAstronError)
	pos[3] = pos[4] = pos[5] = pos[6] = pos[7] = pos[8] = 0.0;
      break;
    default:			/* other planets */
				/* heliocentric ecliptic longitude (rad) */
      gatherVSOP(T, &indices[6*3*(object - 1)], planetTermsD, &pos[0],
		      getAstronError? &pos[3]: NULL);
      pos[0] = fmod(pos[0], TWOPI);
      if (pos[0] < 0)
	pos[0] += TWOPI;
				/* heliocentric ecliptic latitude (rad) */
      gatherVSOP(T, &indices[6*3*(object - 1) + 6], planetTermsD, 
                      &pos[1], getAstronError? &pos[4]: NULL);
				/* heliocentric distance (AU) */
      gatherVSOP(T, &indices[6*3*(object - 1) + 12], planetTermsD, 
                 &pos[2], getAstronError? &pos[5]: NULL);
      if (getAstronError)
	pos[6] = pos[7] = pos[8] = 0.0;
      break;
  }
}
