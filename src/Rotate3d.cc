/*
  This is file Rotate3d.cc.

  Copyright 2015 Louis Strous

  This file is part of LUX.

  LUX is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  LUX is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  You should have received a copy of the GNU General Public License
  along with LUX.  If not, see <http://www.gnu.org/licenses/>.
*/

/// \file
/// The source file implementing the Rotate3d class.

#include <cassert>
#include <cmath>

#ifndef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "Rotate3d.hh"
#include "replacements.h"       // if sincos is missing

/// Rotate cartesian coordinates counterclockwise around the X axis
/// over a specific angle.
///
/// \param[in,out] xyz is a pointer to an array of at least three
/// elements containing cartesian coordinates X, Y, Z.
///
/// \param[in] angle_rad is the angle to rotate over, measured in
/// radians.
void
Rotate3d::rotate_x(double* xyz, double angle_rad)
{
  assert(xyz);
  double s, c;
  sincos(angle_rad, &s, &c);
  double y2 = xyz[1]*c - xyz[2]*s;
  double z2 = xyz[1]*s + xyz[2]*c;
  xyz[1] = y2;
  xyz[2] = z2;
}

/// Rotate cartesian coordinates counterclockwise around the Y axis
/// over a specific angle.
///
/// \param[in,out] xyz is a pointer to an array of at least three
/// elements containing cartesian coordinates X, Y, Z.
///
/// \param[in] angle_rad is the angle to rotate over, measured in
/// radians.
void
Rotate3d::rotate_y(double* xyz, double angle_rad)
{
  assert(xyz);
  double s, c;
  sincos(angle_rad, &s, &c);
  double x2 = xyz[0]*c + xyz[2]*s;
  double z2 = -xyz[0]*s + xyz[2]*c;
  xyz[0] = x2;
  xyz[2] = z2;
}

/// Rotate cartesian coordinates counterclockwise around the Z axis
/// over a specific angle.
///
/// \param[in,out] xyz is a pointer to an array of at least three
/// elements containing cartesian coordinates X, Y, Z.
///
/// \param[in] angle_rad is the angle to rotate over, measured in
/// radians.
void
Rotate3d::rotate_z(double* xyz, double angle_rad)
{
  assert(xyz);
  double s, c;
  sincos(angle_rad, &s, &c);
  double x2 = xyz[0]*c - xyz[1]*s;
  double y2 = xyz[0]*s + xyz[1]*c;
  xyz[0] = x2;
  xyz[1] = y2;
}

