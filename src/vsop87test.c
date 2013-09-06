#include <stdio.h>
#include <math.h>
#include "vsop.h"

#define NREC (80)
#define J2000 (2451545.0)

void main(void) {
  FILE *fin;
  Int planet[NREC];
  Double JD[NREC];
  Double l[NREC];
  Double b[NREC];
  Double r[NREC];
  Double pos[3];
  Int i;

  fin = fopen("vsop87test.txt", "r");
  if (!fin) {
    puts("Cannot open vsop87test.txt for reading");
    return;
  }
  for (i = 0; i < NREC; i++) {
    fscanf(fin, "%d %lg %*d %*d %*d %lg %lg %lg", &planet[i], &JD[i],
           &l[i], &b[i], &r[i]);
  }
  fclose(fin);

  Double maxerr = 0;
  for (i = 0; i < NREC; i++) {
    Double T;
    Double pos[3];
    Double x;

    T = (JD[i] - J2000)/365250;
    XYZfromVSOPA(T, planet[i], pos, 0, 0);
    x = fabs(l[i] - pos[0]);
    if (x > maxerr)
      maxerr = x;
    x = fabs(b[i] - pos[1]);
    if (x > maxerr)
      maxerr = x;
    x = fabs(r[i] - pos[2]);
    if (x > maxerr)
      maxerr = x;
    printf("%d %d %lg %lg: %lg %lg %lg %lg %lg %lg\n",
           i, planet[i], JD[i], maxerr,
           l[i], pos[0], b[i], pos[1], r[i], pos[2]);
  }
  printf("maxerr = %g\n", maxerr);

  /* now test tolerance truncation */
  Int nmax = sizeof(planetTerms)/(3*sizeof(Double));
  Double tol;
  Int bad = 0;
  for (tol = 0.1; tol > 1e-20; tol /= 10) {
    struct planetIndex *pi = planetIndicesForTolerance(tol);
    for (i = 0; i < 6*3*8; i++) {
      Int index = pi[i].index;
      Int n = pi[i].nTerms;
      if (index < 0 || index > nmax
          || (index == nmax && n > 0)) {
        printf("index at %d = %d out of bounds\n", i, index);
        bad++;
      } else if (i < 6*3*8 - 1) {
        if (index + n > pi[i + 1].index) {
          printf("index at %d = %d points outside its range\n", i, index);
          bad++;
        } else if (index + n < pi[i + 1].index
                   && planetTerms[3*(index + n)] > tol) {
          printf("tolerance error at %d = %d\n", i, index);
          bad++;
        }
      } else {                  /* i = 6*3*8 - 1, last element */
        if (index + n < nmax - 1 && planetTerms[3*(index + n)] > tol) {
          printf("tolerance error at %d = %d\n", i, index);
          bad++;
        }
      }
    }
  }
  printf("Found %d tolerance problems.\n", bad);
}
