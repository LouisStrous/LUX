/* This is file check-Ellipsoid.cc.

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
/// A file providing CppUTest unit tests for the Ellipsoid class.

#ifdef HAVE_CONFIG_H
# include "config.h"            // for HAVE_LIBCPPUTEST
#endif

#if HAVE_LIBCPPUTEST

# include "Ellipsoid.hh"
# include "TestArray.hh"
//# include "replacements.h"      // drop-in replacement for sincos if needed

# include "CppUTest/TestHarness.h"

typedef TestArray<double, 3> A3d;

// Needed by the test framework for proper reporting of problems.
SimpleString StringFrom(A3d const& value);

static A3d unitx((double) 1, (double) 0, (double) 0);
static A3d unity((double) 0, (double) 1, (double) 0);
static A3d unitz((double) 0, (double) 0, (double) 1);
static A3d munitx((double) -1, (double) 0, (double) 0);
static A3d munity((double) 0, (double) -1, (double) 0);
static A3d munitz((double) 0, (double) 0, (double) -1);

#define doubles_equal_text(expected, actual, text) \
  DOUBLES_EQUAL_TEXT(expected, actual, 1e-10, text)

TEST_GROUP(EllipsoidTestGroup)
{
  A3d xyz;

  //void setup() { /* set up */}
  //void teardown() { /* clean up */ }
};

TEST(EllipsoidTestGroup, Sphere)
{
  Ellipsoid e0;                 // default

  doubles_equal_text(1, e0.equatorial_radius(), "eq radius default");
  doubles_equal_text(0, e0.flattening(), "flattening default");
  doubles_equal_text(0.1, e0.geocentric_latitude_rad(0.1, 0),
                     "geoc lat 0.1 default");
  doubles_equal_text(0.1, e0.geocentric_latitude_rad(0.1, 0.1),
                     "geoc lat 0.1 0.1 default");

  // latitude, longitude, height
  e0.xyz(0, 0, 0, xyz.data);
  CHECK_EQUAL_TEXT(unitx, xyz, "xyz 0 0 0");

  e0.xyz(0, 0, 0.1, xyz.data);
  CHECK_EQUAL_TEXT(unitx*1.1, xyz, "xyz 0 0 0.1");

  e0.xyz(0, M_PI/2, 0, xyz.data);
  CHECK_EQUAL_TEXT(unity, xyz, "xyz 0 90 0");

  e0.xyz(0, M_PI, 0, xyz.data);
  CHECK_EQUAL_TEXT(munitx, xyz, "xyz 0 180 0");

  e0.xyz(M_PI/2, 0, 0, xyz.data);
  CHECK_EQUAL_TEXT(unitz, xyz, "xyz 90 0 0");

  e0.xyz(M_PI/2, 0.3, 0, xyz.data);
  CHECK_EQUAL_TEXT(unitz, xyz, "xyz 90 0.3 0");

  e0.xyz(M_PI/2, 0, -0.1, xyz.data);
  CHECK_EQUAL_TEXT(0.9*unitz, xyz, "xyz 90 0 -0.1");

  e0.xyz(-M_PI/2, 0, 0, xyz.data);
  CHECK_EQUAL_TEXT(munitz, xyz, "xyz -90 0 0");

  Ellipsoid e1(1, 0);           // sphere, unit radius

  doubles_equal_text(1, e1.equatorial_radius(), "eq radius unit");
  doubles_equal_text(0, e1.flattening(), "flattening unit");
  doubles_equal_text(-1, e1.geocentric_latitude_rad(-1, 0),
                     "geoc lat -1 unit");
  doubles_equal_text(-1, e1.geocentric_latitude_rad(-1, 0.1),
                     "geoc lat -1 0.1 unit");

  // latitude, longitude, height
  e1.xyz(0, 0, 0, xyz.data);
  CHECK_EQUAL_TEXT(unitx, xyz, "xyz 0 0 0 unit");

  e1.xyz(0, 0, 0.1, xyz.data);
  CHECK_EQUAL_TEXT(unitx*1.1, xyz, "xyz 0 0 0.1 unit");

  e1.xyz(0, M_PI/2, 0, xyz.data);
  CHECK_EQUAL_TEXT(unity, xyz, "xyz 0 90 0 unit");

  e1.xyz(0, M_PI, 0, xyz.data);
  CHECK_EQUAL_TEXT(munitx, xyz, "xyz 0 180 0 unit");

  e1.xyz(M_PI/2, 0, 0, xyz.data);
  CHECK_EQUAL_TEXT(unitz, xyz, "xyz 90 0 0 unit");

  e1.xyz(M_PI/2, 0.3, 0, xyz.data);
  CHECK_EQUAL_TEXT(unitz, xyz, "xyz 90 0.3 0 unit");

  e1.xyz(M_PI/2, 0, -0.1, xyz.data);
  CHECK_EQUAL_TEXT(0.9*unitz, xyz, "xyz 90 0 -0.1 unit");

  e1.xyz(-M_PI/2, 0, 0, xyz.data);
  CHECK_EQUAL_TEXT(munitz, xyz, "xyz -90 0 0 unit");

  Ellipsoid e2(3, 0);           // sphere, radius 3

  doubles_equal_text(3, e2.equatorial_radius(), "eq rad (3)");
  doubles_equal_text(0, e2.flattening(), "flattening (3)");
  doubles_equal_text(0.1, e2.geocentric_latitude_rad(0.1, 0),
                     "geoc lat 0.1 (3)");
  doubles_equal_text(0.1, e2.geocentric_latitude_rad(0.1, 0.1),
                     "geoc lat 0.1 0.1 (3)");

  // latitude, longitude, height
  e2.xyz(0, 0, 0, xyz.data);
  CHECK_EQUAL_TEXT(3*unitx, xyz, "xyz 0 0 0 (3)");

  e2.xyz(0, 0, 0.1, xyz.data);
  CHECK_EQUAL_TEXT(3.1*unitx, xyz, "xyz 0 0 0.1 (3)");

  e2.xyz(0, M_PI/2, 0, xyz.data);
  CHECK_EQUAL_TEXT(3*unity, xyz, "xyz 0 90 0 (3)");

  e2.xyz(0, M_PI, 0, xyz.data);
  CHECK_EQUAL_TEXT(3*munitx, xyz, "xyz 0 180 0 (3)");

  e2.xyz(M_PI/2, 0, 0, xyz.data);
  CHECK_EQUAL_TEXT(3*unitz, xyz, "xyz 90 0 0 (3)");

  e2.xyz(M_PI/2, 0.3, 0, xyz.data);
  CHECK_EQUAL_TEXT(3*unitz, xyz, "xyz 90 0.3 0 (3)");

  e2.xyz(M_PI/2, 0, -0.1, xyz.data);
  CHECK_EQUAL_TEXT(2.9*unitz, xyz, "xyz 90 0 -0.1 (3)");

  e2.xyz(-M_PI/2, 0, 0, xyz.data);
  CHECK_EQUAL_TEXT(3*munitz, xyz, "xyz -90 0 0 (3)");
}

TEST(EllipsoidTestGroup, Ellipsoid)
{
  Ellipsoid e1(2, 0.1);

  doubles_equal_text(2, e1.equatorial_radius(), "eq radius unit");
  doubles_equal_text(0.1, e1.flattening(), "flattening unit");

  // latitude, longitude, height
  e1.xyz(0, 0, 0, xyz.data);
  CHECK_EQUAL_TEXT(2*unitx, xyz, "xyz 0 0 0 unit");

  e1.xyz(0, 0, 0.1, xyz.data);
  CHECK_EQUAL_TEXT(unitx*2.1, xyz, "xyz 0 0 0.1 unit");

  e1.xyz(0, M_PI/2, 0, xyz.data);
  CHECK_EQUAL_TEXT(2*unity, xyz, "xyz 0 90 0 unit");

  e1.xyz(0, M_PI, 0, xyz.data);
  CHECK_EQUAL_TEXT(2*munitx, xyz, "xyz 0 180 0 unit");

  e1.xyz(M_PI/2, 0, 0, xyz.data);
  // north pole; radius = 2, flattening = 0.1, height = 0, so z = 2*(1
  // - 0.1) + 0 = 1.8
  CHECK_EQUAL_TEXT(1.8*unitz, xyz, "xyz 90 0 0 unit");

  e1.xyz(M_PI/2, 0.3, 0, xyz.data);
  // north pole; radius = 2, flattening = 0.1, height = 0, so z = 2*(1
  // - 0.1) + 0 = 1.8
  CHECK_EQUAL_TEXT(1.8*unitz, xyz, "xyz 90 0.3 0 unit");

  e1.xyz(M_PI/2, 0, -0.1, xyz.data);
  // north pole; radius = 2, flattening = 0.1, height = -0.1, so z =
  // 2*(1 - 0.1) - 0.1 = 1.7
  CHECK_EQUAL_TEXT(1.7*unitz, xyz, "xyz 90 0 -0.1 unit");

  e1.xyz(-M_PI/2, 0, 0, xyz.data);
  // south pole; radius = 2, flattening = 0.1, height = 0, so z = -1.8
  CHECK_EQUAL_TEXT(1.8*munitz, xyz, "xyz -90 0 0 unit");

  doubles_equal_text(-0.90051822394, e1.geocentric_latitude_rad(-1, 0),
                     "geoc lat -1 unit");
  doubles_equal_text(-0.90556133289, e1.geocentric_latitude_rad(-1, 0.1),
                     "geoc lat -1 0.1 unit");
}

#endif

/*
  CHECK[_TEXT](condition [, text])
  CHECK_TRUE[_TEXT](condition [, text])
  CHECK_FALSE[_TEXT](condition [, text])
  CHECK_EQUAL[_TEXT](expected, actual [, text])
  STRCMP[_NOCASE]_EQUAL[_TEXT](expected, actual [, text])
  STRNCMP_EQUAL[_TEXT](expected, actual, length [, text])
  STRCMP[_NOCASE]_CONTAINS[_TEXT](expected, actual [, text])
  [UNSIGNED_]LONGS_EQUAL[_TEXT](expected, actual [, text])
  BYTES_EQUAL[_TEXT](expected, actual [, text])
  POINTERS_EQUAL[_TEXT](expected, actual [, text])
  FUNCTIONPOINTERS_EQUAL[_TEXT](expected, actual [, text])
  DOUBLES_EQUAL[_TEXT](expected, actual [, text])
  MEMCMP_EQUAL[_TEXT](expected, actual, size [, text])
  BITS_EQUAL[_TEXT](expected, actual [, text])
  FAIL(text)
  UT_PRINT(text)
  CHECK_THROWS(expected, expression)
  UT_CRASH()
 */

