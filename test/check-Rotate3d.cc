/* This is file check-Rotate3d.cc.

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
/// A file providing CppUTest unit tests for the Rotate3d class.

#ifdef HAVE_CONFIG_H
# include "config.h"            // for HAVE_LIBCPPUTEST
#endif

#if HAVE_LIBCPPUTEST

# include <cmath>               // for M_PI

# include "Rotate3d.hh"
# include "TestArray.hh"

# include "CppUTest/TestHarness.h"

# ifndef CHECK_UNEQUAL
#  define CHECK_UNEQUAL(expected, actual)\
  CHECK(!((expected) == (actual)))
# endif
# ifndef CHECK_UNEQUAL_TEXT
#  define CHECK_UNEQUAL_TEXT(expected, actual, text)\
  CHECK_TEXT(!((expected) == (actual)), text)
# endif

typedef TestArray<double, 3> A3d;

// Needed by the test framework for proper reporting of problems.
SimpleString StringFrom(A3d const& value)
{
  return SimpleString(StringFromFormat("(%g, %g, %g)",
                                       value.data[0],
                                       value.data[1],
                                       value.data[2]));
}

// Explicitly cast the values to the correct type, because the
// constructor uses variadic arguments which don't provide value type
// promotion.
static A3d unitx((double) 1, (double) 0, (double) 0);
static A3d unity((double) 0, (double) 1, (double) 0);
static A3d unitz((double) 0, (double) 0, (double) 1);
static A3d munitx((double) -1, (double) 0, (double) 0);
static A3d munity((double) 0, (double) -1, (double) 0);
static A3d munitz((double) 0, (double) 0, (double) -1);

TEST_GROUP(Rotate3dTestGroup)
{
  //void setup() { /* init stuff */ }
  //void teardown() { /* clean up */ }
};

TEST(Rotate3dTestGroup, EqualityTest)
{
  CHECK_EQUAL_TEXT(unitx, unitx, "exact same expression");
  A3d result;
  result = A3d(1, 0, 0);
  CHECK_EQUAL_TEXT(unitx, result, "same value, different variables");
  CHECK_UNEQUAL_TEXT(unitx, unity, "different values");
  CHECK_UNEQUAL_TEXT(unitx, unitz, "other different values");
}

TEST(Rotate3dTestGroup, RotateTest)
{
  struct {
    A3d input;
    A3d expected_x[3];
    A3d expected_y[3];
    A3d expected_z[3];
  } tests[] = {
    unitx,
    {  unitx,  unitx,  unitx },
    { munitz, munitx,  unitz },
    {  unity, munitx, munity },
  };

  A3d result;
  for (int itest = 0; itest < sizeof(tests)/sizeof(*tests); ++itest) {
    // no rotation
    result = tests[itest].input;
    Rotate3d::rotate_x(result.data, 0);
    CHECK_EQUAL_TEXT(tests[itest].input, result, "for no rotation");

    for (int k = 0; k < 3; ++k) {
      double angle = M_PI*(k + 1)/2;

      // rotation around x axis over 90*(k + 1) degrees
      result = tests[itest].input;
      Rotate3d::rotate_x(result.data, angle);
      CHECK_EQUAL(tests[itest].expected_x[k], result);

      // rotation around y axis over 90*(k + 1) degrees
      result = tests[itest].input;
      Rotate3d::rotate_y(result.data, angle);
      CHECK_EQUAL(tests[itest].expected_y[k], result);

      // rotation around z axis over 90*(k + 1) degrees
      result = tests[itest].input;
      Rotate3d::rotate_z(result.data, angle);
      CHECK_EQUAL(tests[itest].expected_z[k], result);
    }
  }
}

#endif
