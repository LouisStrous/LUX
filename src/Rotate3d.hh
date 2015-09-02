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

#ifndef ROTATE3D_HH_
#define ROTATE3D_HH_

/// \file
/// The header file declaring the Rotate3d class.

/// A class providing functionality to rotate around the principal
/// axes of a three-dimensional cartesian coordinate system.
class Rotate3d
{
public:
  static void rotate_x(double* xyz, double angle_rad);
  static void rotate_y(double* xyz, double angle_rad);
  static void rotate_z(double* xyz, double angle_rad);
};

#endif
