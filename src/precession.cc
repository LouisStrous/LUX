/* This is file precession.cc.

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
// HEADERS
#include <float.h> // for DBL_MAX
#include <math.h> // for sin cos
#include <string.h> // for memcpy
// END HEADERS
#include "action.hh"
#include <gsl/gsl_poly.h>

// time in millennia since epoch 2000.0
#define TM2000(JD) ((JD - 2451545.0)/365250)

//--------------------------------------------------------------------------
void matmul3(double *front, double *back, double *result)
/* matrix multiplication of 3-by-3 matrices <front> and <back> into
   <result> */
{
  /* R = F B

     0 1 2
     3 4 5
     6 7 8
  */
  result[0]
    = front[0]*back[0]
    + front[1]*back[3]
    + front[2]*back[6];
  result[1]
    = front[0]*back[1]
    + front[1]*back[4]
    + front[2]*back[7];
  result[2]
    = front[0]*back[2]
    + front[1]*back[5]
    + front[2]*back[8];
  result[3]
    = front[3]*back[0]
    + front[4]*back[3]
    + front[5]*back[6];
  result[4]
    = front[3]*back[1]
    + front[4]*back[4]
    + front[5]*back[7];
  result[5]
    = front[3]*back[2]
    + front[4]*back[5]
    + front[5]*back[8];
  result[6]
    = front[6]*back[0]
    + front[7]*back[3]
    + front[8]*back[6];
  result[7]
    = front[6]*back[1]
    + front[7]*back[4]
    + front[8]*back[7];
  result[8]
    = front[6]*back[2]
    + front[7]*back[5]
    + front[8]*back[8];
}
//--------------------------------------------------------------------------
#define F(x) (x*1e-12)
static double s11c[] = {  0,               0, F(-538867722),  F(-270670), F(1138205),  F(8604), F(-813) };
static double c11c[] = {  1,               0,     F(-20728),   F(-19147), F(-149390),   F(-34),  F(617) };
static double s12c[] = { -1,               0,    F(2575043),   F(-56157),  F(140001),   F(383), F(-613) };
static double c12c[] = {  0,               0, F(-539329786),  F(-479046), F(1144883),  F(8884), F(-830) }; 
static double s13c[] = {  0, F(2269380040LL),  F(-24745348), F(-2422542),   F(78247),  F(-468), F(-134) };
static double c13c[] = {  0,   F(-203607820),  F(-94040878),  F(2307025),   F(37729), F(-4862),   F(25) };
static double a31c[] = {  0,    F(203607820),   F(94040878), F(-1083606),  F(-50218),   F(929),   F(11) };
static double a32c[] = {  0, F(2269380040LL),  F(-24745348), F(-2532307),   F(27473),   F(643),   F(-1) };
static double a33c[] = {  1,               0,   F(-2595771),    F(37009),    F(1236),   F(-13),       0 };
static double a_fromJ2000[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, -DBL_MAX };
static double a_toJ2000[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, -DBL_MAX };
static double a_from_to[9];
void init_XYZ_eclipticPrecession(double fromequinox, double toequinox)
/* intializes some auxiliary data for ecliptic precession calculations
   for cartesian coordinates, for precession from <fromequinox> to
   <toequinox> (both measured in JDE) */
{
  int32_t is_new = (toequinox != a_toJ2000[9] || fromequinox != a_fromJ2000[9]);
  if (!is_new)
    return;
  if (toequinox != a_toJ2000[9]) {
    a_toJ2000[9] = toequinox;
    double T = TM2000(toequinox);
    double s11 = gsl_poly_eval(s11c, sizeof(s11c)/sizeof(s11c[0]), T);
    double c11 = gsl_poly_eval(c11c, sizeof(c11c)/sizeof(c11c[0]), T);
    double s12 = gsl_poly_eval(s12c, sizeof(s12c)/sizeof(s12c[0]), T);
    double c12 = gsl_poly_eval(c12c, sizeof(c12c)/sizeof(c12c[0]), T);
    double s13 = gsl_poly_eval(s13c, sizeof(s13c)/sizeof(s13c[0]), T);
    double c13 = gsl_poly_eval(c13c, sizeof(c13c)/sizeof(c13c[0]), T);
    // s2j = c1j
    double s21 = c11;
    double s22 = c12;
    double s23 = c13;
    // c2j = -s1j
    double c21 = -s11;
    double c22 = -s12;
    double c23 = -s13;

    double xi = 0.243817483530*T;
    double cxi = cos(xi);
    double sxi = sin(xi);

    a_toJ2000[0] = s11*sxi + c11*cxi;
    a_toJ2000[1] = s12*sxi + c12*cxi;
    a_toJ2000[2] = s13*sxi + c13*cxi;
    a_toJ2000[3] = s21*sxi + c21*cxi;
    a_toJ2000[4] = s22*sxi + c22*cxi;
    a_toJ2000[5] = s23*sxi + c23*cxi;
    a_toJ2000[6] = gsl_poly_eval(a31c, sizeof(a31c)/sizeof(a31c[0]), T);
    a_toJ2000[7] = gsl_poly_eval(a32c, sizeof(a32c)/sizeof(a32c[0]), T);
    a_toJ2000[8] = gsl_poly_eval(a33c, sizeof(a33c)/sizeof(a33c[0]), T);
  }
  if (fromequinox != a_fromJ2000[9]) {
    a_fromJ2000[9] = fromequinox;
    double T = TM2000(fromequinox);
    double s11 = gsl_poly_eval(s11c, sizeof(s11c)/sizeof(s11c[0]), T);
    double c11 = gsl_poly_eval(c11c, sizeof(c11c)/sizeof(c11c[0]), T);
    double s12 = gsl_poly_eval(s12c, sizeof(s12c)/sizeof(s12c[0]), T);
    double c12 = gsl_poly_eval(c12c, sizeof(c12c)/sizeof(c12c[0]), T);
    double s13 = gsl_poly_eval(s13c, sizeof(s13c)/sizeof(s13c[0]), T);
    double c13 = gsl_poly_eval(c13c, sizeof(c13c)/sizeof(c13c[0]), T);
    // s2j = c1j
    double s21 = c11;
    double s22 = c12;
    double s23 = c13;
    // c2j = -s1j
    double c21 = -s11;
    double c22 = -s12;
    double c23 = -s13;

    double xi = 0.243817483530*T;
    double cxi = cos(xi);
    double sxi = sin(xi);

    /* because we go in the opposite direction from the "to" case,
       we need the transposed matrix
       
       a[0] a[1] a[2]     a[0] a[3] a[6]
       a[3] a[4] a[5]  => a[1] a[4] a[7]
       a[6] a[7] a[8]     a[2] a[5] a[8]

    */

    a_fromJ2000[0] = s11*sxi + c11*cxi;
    a_fromJ2000[3] = s12*sxi + c12*cxi;
    a_fromJ2000[6] = s13*sxi + c13*cxi;
    a_fromJ2000[1] = s21*sxi + c21*cxi;
    a_fromJ2000[4] = s22*sxi + c22*cxi;
    a_fromJ2000[7] = s23*sxi + c23*cxi;
    a_fromJ2000[2] = gsl_poly_eval(a31c, sizeof(a31c)/sizeof(a31c[0]), T);
    a_fromJ2000[5] = gsl_poly_eval(a32c, sizeof(a32c)/sizeof(a32c[0]), T);
    a_fromJ2000[8] = gsl_poly_eval(a33c, sizeof(a33c)/sizeof(a33c[0]), T);
  }
  if (is_new)
    matmul3(a_fromJ2000, a_toJ2000, a_from_to);
}
//--------------------------------------------------------------------------
double *XYZ_eclipticPrecessionMatrix(void)
{
  return a_from_to;
}
//--------------------------------------------------------------------------
void XYZ_eclipticPrecession(double *pos, double equinox1, double equinox2)
/* precess the ecliptical cartesian coordinates <pos> from <equinox1>
   to <equinox2>, both measured in JDE. */
// From 1988A&A...202..309B
{
  init_XYZ_eclipticPrecession(equinox1, equinox2);

  double xyz[3];
  xyz[0] = a_from_to[0]*pos[0] + a_from_to[1]*pos[1] + a_from_to[2]*pos[2];
  xyz[1] = a_from_to[3]*pos[0] + a_from_to[4]*pos[1] + a_from_to[5]*pos[2];
  xyz[2] = a_from_to[6]*pos[0] + a_from_to[7]*pos[1] + a_from_to[8]*pos[2];
  memcpy(pos, xyz, 3*sizeof(double));
}
