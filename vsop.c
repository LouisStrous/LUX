#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "vsopdata.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "anaparser.h"

static struct planetIndex usedPlanetIndices[6*3*8];
static double planetIndexTolerance = -1;

struct planetIndex *planetIndicesForTolerance(double tolerance)
/* returns a pointer to an array of planet indices into the VSOP model
   values, suitable for an error tolerance of nonnegative <tolerance>.
   None of the amplitudes of the VSOP planet terms represented by the
   returned indices is greater than the indicated tolerance.  If
   <tolerance> is equal to 0, then the returned indices represent the
   full set of VSOP model values. The returned pointer is to a
   statically allocated array, which gets overwritten by subsequent
   calls to the same routine. */
{
  if (tolerance < 0)
    tolerance = 0;
  if (tolerance == planetIndexTolerance)
    return usedPlanetIndices;
  memcpy(usedPlanetIndices, planetIndices, sizeof(planetIndices));
  if (tolerance > 0) {
    int planet;
    for (planet = 0; planet < 8; planet++) {
      int coordinate;
      for (coordinate = 0; coordinate < 3; coordinate++) {
        int poweroft;
        for (poweroft = 0; poweroft < 6; poweroft++) {
          struct planetIndex *pi = &usedPlanetIndices[poweroft + 6*coordinate + 6*3*planet];
          int i, j;
          for (i = pi->index, j = 1; i < pi->index + pi->nTerms; i++, j++) {
            if (planetTerms[3*i]*sqrt(j)*2 < tolerance) {
              pi->nTerms = i - pi->index;
              break;
            }
          }
        }
      }
    }
  }
  return usedPlanetIndices;
}
/*--------------------------------------------------------------------------*/
static void gatherVSOP(double T, struct planetIndex *index, double *terms, 
                       double *value, double *error)
/* calculates one coordinate of one object, as indicated by <index>,
   at time <T> in Julian centuries since J2000.0, using the VSOP87
   theory of Bretagnon & Francou (1988).  The heliocentric longitude,
   latitude, (in radians) or radius (in AU) is returned in <*value>.
   The associated estimated variance is returned in <*error>, if
   <*error> is not NULL. */
{
  double	*ptr;
  int	nTerm, i;

  *value = 0.0;			/* initialize */
  if (error)
    *error = 0.0;
  for (i = 5; i >= 0; i--) {	/* powers of T */
    *value *= T;		/* move previous stuff to next power of T */
    if (error)
      *error *= T;
    nTerm = index[i].nTerms;	/* number of terms to add */
    if (nTerm) {
      ptr = terms + 3*(index[i].index); /* points at first term */
      while (nTerm--) {
	*value += ptr[0]*cos(ptr[1] + ptr[2]*T); /* add term */
	ptr += 3;		/* go to next term for this coordinate */
      }
      if (error)		/* variance estimate: number of terms
				 times square of amplitude of smallest
				 (last) term times a number smaller than 4 */
	*error += index[i].nTerms*ptr[-3]*ptr[-3];
    }
  }
  if (error)                  /* apply factor of 4 mentioned before */
    *error = 4* *error;
}
/*--------------------------------------------------------------------------*/
void XYZfromVSOPA(double T, int object, double *pos, double tolerance,
                  int getErrors)
/* returns the heliocentric cartesian coordinates referred to the mean
 dynamical ecliptic and equinox of J2000.0 using the VSOP87A theory as
 described in Bretagnon & Francou: "Planetary theories in rectangular
 and spherical variables.  VSOP 87 solutions", Astronomy and
 Astrophysics, 202, 309-315 (1988).  <T> represents the desired
 instant of time, measured in Julian centuries since J2000.0.
 <object> indicates the object for which coordinates are to be
 returned (0 = the Sun, 1 = Mercury, ..., 8 = Neptune).  <pos> points
 at an array of at least 3 (if <getErrors> is zero) or 9 (if
 <getErrors> is nonzero) elements, in which the results are returned.
 pos[0] -> X, pos[1] -> Y, pos[2] -> Z; if <getErrors> is unequal to
 zero, then also pos[3] -> var[X], pos[4] -> var[Y], pos[5] -> var[Z],
 pos[6] -> cov[X,Y], pos[7] -> cov[X,Z], pos[8] -> cov[Y,Z].  The
 covariances are assumed equal to zero (for want of a better
 estimate). <tolerance> indicates the maximum error allowed in the
 results due to truncation of the VSOP model series.  Specify 0 to get
 the highest accuracy. */
{
  struct planetIndex *indices;

  indices = planetIndicesForTolerance(tolerance);
  switch (object) {
    case 0:			/* Sun */
      pos[0] = pos[1] = pos[2] = 0.0;
      if (getErrors)
	pos[3] = pos[4] = pos[5] = pos[6] = pos[7] = pos[8] = 0.0;
      break;
    default:			/* other planets */
				/* heliocentric ecliptic X (AU) */
      gatherVSOP(T, &indices[6*3*(object - 1)], planetTerms, &pos[0],
		      getErrors? &pos[3]: NULL);
				/* heliocentric ecliptic Y (AU) */
      gatherVSOP(T, &indices[6*3*(object - 1) + 6], planetTerms, 
                      &pos[1], getErrors? &pos[4]: NULL);
				/* heliocentric ecliptic Z (AU) */
      gatherVSOP(T, &indices[6*3*(object - 1) + 12], planetTerms, 
                 &pos[2], getErrors? &pos[5]: NULL);
      if (getErrors)
	pos[6] = pos[7] = pos[8] = 0.0;
      break;
  }
}
