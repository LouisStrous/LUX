/* This is file check-astron.cc.

   Copyright 2015 Louis Strous

   This file is part of LUX.

   LUX is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   LUX is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with LUX.  If not, see <http://www.gnu.org/licenses/>.
*/

/// \file
/// A file providing CppUTest unit tests for functionality from file
/// astron.cc.

#ifdef HAVE_CONFIG_H
# include "config.h"            // for HAVE_LIBCPPUTEST
#endif

#if HAVE_LIBCPPUTEST

# include "AstronomicalConstants.hh"
# include "Ellipsoid.hh"
# include "TestArray.hh"

# include "CppUTest/TestHarness.h"

typedef TestArray<double, 3> A3d;

// Needed by the test framework for proper reporting of problems.
SimpleString StringFrom(A3d const& value);

static A3d unitx((double) 1, (double) 0, (double) 0);
static A3d unity((double) 0, (double) 1, (double) 0);
static A3d unitz((double) 0, (double) 0, (double) 1);

void get_geocentric_ecliptic_xyz_AU(double geographic_latitude_rad,
                                    double sidereal_rad,
                                    double height_m,
                                    double obliquity_rad,
                                    double* xyz_AU);

TEST_GROUP(AstronTestGroup)
{
  double r_eq_AU;
  double f;

  void setup()
  {
    r_eq_AU = AstronomicalConstants::Earth_equatorial_radius_m
      / AstronomicalConstants::AU_m;
    f = AstronomicalConstants::Earth_flattening;
  }
};

TEST(AstronTestGroup, geocentric_ecliptic_xyz)
{
  A3d xyz;
  double obliquity = 23.45*M_PI/180; // 23.45 degrees

  // equator x
  get_geocentric_ecliptic_xyz_AU(0, 0, 0, 0, xyz.data);
  CHECK_EQUAL_TEXT(unitx*r_eq_AU, xyz, "xyz 0 0 0 0");

  // equator y
  get_geocentric_ecliptic_xyz_AU(0, M_PI, 0, 0, xyz.data);
  CHECK_EQUAL_TEXT(A3d(-0.00004263521, 0, 0), xyz, "xyz 0 90 0 0");

  // north pole
  get_geocentric_ecliptic_xyz_AU(M_PI/2, 0, 0, 0, xyz.data);
  CHECK_EQUAL_TEXT(unitz*r_eq_AU*(1 - f), xyz, "xyz 90 0 0 0");

  // equator x
  get_geocentric_ecliptic_xyz_AU(0, 0, 0, obliquity, xyz.data);
  CHECK_EQUAL_TEXT(unitx*r_eq_AU, xyz, "xyz 0 0 0 0.4");

  // north pole
  get_geocentric_ecliptic_xyz_AU(M_PI/2, 0, 0, obliquity, xyz.data);
  CHECK_EQUAL_TEXT(A3d(0, 0.00001690974, 0.00003898273), xyz, "xyz 90 0 0 0.4");
}

#endif
