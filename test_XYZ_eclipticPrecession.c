#include <stdio.h>
#include <math.h>

Double diff(Double a[9], Double b[9])
{
  Int i;
  Double r = 0;
  for (i = 0; i < 9; i++) {
    Double d = fabs(a[i] - b[i]);
    if (d > r)
      r = d;
  }
  return r;
}

void main(void) {
  Double *XYZ_eclipticPrecessionMatrix();
  Double equinox0 = 2451545.0;
  Double equinox1 = 1234567.0;
  Double equinox2 = 3668523.0;  /* as far beyond 2451545 as 1234567 is before it */
  
  Double expect01[9] = {
    .6920004376905933, .7218775990918209, .005310784601890324,
    -.7218970219208204, .6919775407984774, .005643681205594669,
    3.991274892506633e-4, -.007739280356186162, .9999699715544785
  };
  Double expect10[9] = {
    .6920003857029381, -.7218969674258517, 3.9910333899661043e-4,
    .7218775405704975, .6919774841980717, -.007739268847388554,
    .005310775767143926, .005643706146206156, .9999699718793482
  };
  Double expect02[9] = {
    .6835161253580114, -.7299238490092927, .004107139771958359,
    .7299334799015585, .6834905765468121, -.006142753541474567,
    0.00167653003930379, .007196624260687031, .9999726987606123
  };
  Double expect20[9] = {
    .6835161321340808, 0.729933487949836, .001676550999001878,
    -.7299238595481562, .6834905874150758, .007196610016832918,
    .004107164558889876, -.006142748067219738, .9999726984076661
  };
  Double expect12[9] = {
    -.05394267407287565, -.9985144873247225, .007675669902546775,
    .9984994974512363, -.05401123990457391, -.009024852740074857,
    .009426010638103005, .007177366238137705, .9999298156622219
  };
  Double expect21[9] = {
    -.05394267938830753, 0.998499589045248, .009426019511739504,
    -.9985145774890996, -.05401124399460198, .007177328502375709,
    .007675707800989555, -.009024842671961795, .9999298153364482
  };

  init_XYZ_eclipticPrecession(equinox0, equinox1);
  printf("01: %g\n", diff(XYZ_eclipticPrecessionMatrix(), expect01));

  init_XYZ_eclipticPrecession(equinox0, equinox2);
  printf("02: %g\n", diff(XYZ_eclipticPrecessionMatrix(), expect02));

  init_XYZ_eclipticPrecession(equinox1, equinox2);
  printf("12: %g\n", diff(XYZ_eclipticPrecessionMatrix(), expect12));

  init_XYZ_eclipticPrecession(equinox1, equinox0);
  printf("10: %g\n", diff(XYZ_eclipticPrecessionMatrix(), expect10));

  init_XYZ_eclipticPrecession(equinox2, equinox0);
  printf("20: %g\n", diff(XYZ_eclipticPrecessionMatrix(), expect20));

  init_XYZ_eclipticPrecession(equinox2, equinox1);
  printf("21: %g\n", diff(XYZ_eclipticPrecessionMatrix(), expect21));
}
