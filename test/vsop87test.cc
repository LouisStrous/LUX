#include <stdio.h>
#include <math.h>
#include "vsop.hh"

#define NREC (80)
#define J2000 (2451545.0)

int main(void) {
  FILE *fin;
  int planet[NREC];
  double JD[NREC];
  double X[NREC];
  double Y[NREC];
  double Z[NREC];
  double pos[3];
  int i;

  fin = fopen("vsop87test.txt", "r");
  if (!fin) {
    puts("Cannot open vsop87test.txt for reading");
    return 1;
  }
  for (i = 0; i < NREC; i++) {
    fscanf(fin, "%d %lg %lg %lg %lg", &planet[i], &JD[i],
           &X[i], &Y[i], &Z[i]);
  }
  fclose(fin);

  double maxerr = 0;
  for (i = 0; i < NREC; i++) {
    double T;
    double pos[3];
    double x;
    double thiserr;

    T = (JD[i] - J2000)/365250;
    XYZJ2000fromVSOPA(T, planet[i], pos, 0);
    thiserr = fabs(X[i] - pos[0]);
    x = fabs(Y[i] - pos[1]);
    if (x > thiserr)
      thiserr = x;
    x = fabs(Z[i] - pos[2]);
    if (x > thiserr)
      thiserr = x;
    if (thiserr > maxerr)
      maxerr = thiserr;
    printf("%d %d %lg %lg: %lg %lg %lg %lg %lg %lg\n",
           i, planet[i], JD[i], thiserr,
           X[i], pos[0], Y[i], pos[1], Z[i], pos[2]);
  }
  printf("maxerr = %g\n", maxerr);

#if VSOPTEST
  /* now test tolerance truncation */
  int nmax = 6*8;
  double tol;
  int bad = 0;
  for (tol = 0.1; tol > 1e-20; tol /= 10) {
    VSOPdata* vsopdata = planetIndicesForTolerance(&VSOP87Adata, tol);
    struct planetIndex *pi = vsopdata->indices;
    for (i = 0; i < 6*3*8; i++) {
      int index = pi[i].index;
      int n = pi[i].nTerms;
      if (index < 0 || index > nmax
          || (index == nmax && n > 0)) {
        printf("index at %d = %d out of bounds\n", i, index);
        bad++;
      } else if (i < 6*3*8 - 1) {
        if (index + n > pi[i + 1].index) {
          printf("index at %d = %d points outside its range\n", i, index);
          bad++;
        } else if (index + n < pi[i + 1].index
                   && vsopdata->terms[3*(index + n)] > tol) {
          printf("tolerance error at %d = %d\n", i, index);
          bad++;
        }
      } else {                  /* i = 6*3*8 - 1, last element */
        if (index + n < nmax - 1 && vsopdata->terms[3*(index + n)] > tol) {
          printf("tolerance error at %d = %d\n", i, index);
          bad++;
        }
      }
    }
  }
  printf("Found %d tolerance problems.\n", bad);
#endif
  return 0;
}
