/*
  This is file Ellipsoid.hh.

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

#ifndef ELLIPSOID_HH_
#define ELLIPSOID_HH_

/// \file
/// The header file declaring the Ellipsoid class.

class EllipsoidPrivate;         // forward declaration

/// A class for doing calculations related to ellipsoids of
/// revolution.
class Ellipsoid
{
public:
  Ellipsoid();
  Ellipsoid(double equatorial_radius, double flattening);
  ~Ellipsoid();
  double equatorial_radius() const;
  double flattening() const;
  double geocentric_latitude_rad(double geographic_latitude_rad,
                                 double height) const;
  void xyz(double geographic_latitude_rad, double longitude_rad, double height,
           double* xyz) const;
private:
  /// The private contents of class Ellipsoid.
  EllipsoidPrivate* private_;
};

#endif
