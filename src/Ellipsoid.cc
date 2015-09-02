/*
  This is file Ellipsoid.cc.

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
/// The source file implementing the Ellipsoid class.

#include <math.h>

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "replacements.h"       // for sincos replacement if needed

#include "Ellipsoid.hh"
#include "Rotate3d.hh"

/// A class implementing the private contents of class Ellipsoid.
/// The entire contents is private, but class Ellipsoid is a friend,
/// so Ellipsoid can access the members of EllipsoidPrivate.
class EllipsoidPrivate {
  friend class Ellipsoid;     // only Ellipsoid can access the members

private:
  /// Constructor for EllipsoidPrivate.
  ///
  /// \param[in] equatorial_radius is the equatorial radius of the
  /// ellipsoid.  All values that this class returns that have the
  /// dimension of length are expressed in the same units as the unit
  /// in which this equatorial radius is expressed.
  ///
  /// \param[in] flattening is the flattening ratio of the ellipsoid,
  /// which indicates the difference between the equatorial radius and
  /// the polar radius of the ellipsoid, in units of the equatorial
  /// radius.  A value equal to 0 indicates a perfect sphere.
  EllipsoidPrivate(double equatorial_radius, double flattening)
    : equatorial_radius_(equatorial_radius), flattening_(flattening)
  { }

  void xy_z(double geographic_latitude_rad, double height,
            double *xy, double *z) const;

  /// The equatorial radius of the ellipsoid.
  double equatorial_radius_;

  /// The flattening ratio of the ellipsoid, which indicates the
  /// difference between the equatorial radius and the polar radius of
  /// the ellipsoid, in units of the equatorial radius.  A value equal
  /// to 0 indicates a perfect sphere.
  double flattening_;
};

/// Calculates latitude-dependent cartesian coordinates of a location
/// of interest specified in geographic coordinates.  All input and
/// output values with the dimension of length are expressed in the
/// same unit that was used to specify the equatorial radius of the
/// ellipsoid.
///
/// \param[in] geographic_latitude_rad is the geographic latitude of
/// the location of interest, measured in radians.
///
/// \param[in] height is the height of the location of interest with
/// respect to the ellipsoid.
///
/// \param[out] xy is the length of the projection onto the equatorial
/// plane of the line from the center of the ellipsoid to the location
/// of interest.
///
/// \param[out] z is the height of the location of interest above the
/// equatorial plane of the ellipsoid.
void
EllipsoidPrivate::xy_z(double geographic_latitude_rad,
                       double height, double *xy, double *z) const
{
  // tan u = (1 − f) tan φ
  // ρ cos ψ = cos u + (h/r) cos φ
  // ρ sin ψ = (1 − f) sin u + (h/r) sin φ
  //
  // sin u = (1 − f) sin(φ)/q
  // cos u = cos(φ)/q
  // q² = 1 + (f sin² φ)(f − 2)
  //
  // ρ cos ψ = cos(φ)((1/q) + (h/r))
  // ρ sin ψ = sin(φ)((1 − f)²/q + (h/r))
  //
  // r_xy = r ρ cos ψ = cos(φ)((r/q) + h)
  // r_z  = r ρ sin ψ = sin(φ)((r/q)(1 − f)² + h)

  double sgl, cgl;
  sincos(geographic_latitude_rad, &sgl, &cgl);
  double q = 1 + flattening_*sgl*sgl*(flattening_ - 2);
  // Mathematically always q ≥ 0, but through round-off error the
  // calculated q might accidentally be barely negative for flattening
  // very close to 1.
  double roq = (q > 0? equatorial_radius_/sqrt(q): equatorial_radius_);
  *xy = cgl*(roq + height);
  double b = 1 - flattening_;
  *z = sgl*(roq*b*b + height);
}

/// Creates an object representing a default ellipsoid of revolution
/// with unit radius and zero flattening; i.e., a unit sphere.
Ellipsoid::Ellipsoid()
{
  private_ = new EllipsoidPrivate(1, 0);
}

/// Creates an object representing an ellipsoid of revolution.
///
/// \param[in] equatorial_radius is the equatorial radius of the
/// ellipsoid.  All values that this class returns that have the
/// dimension of length are expressed in the same units as the unit in
/// which this equatorial radius is expressed.
///
/// \param[in] flattening is the flattening ratio of the ellipsoid,
/// which indicates the difference between the equatorial radius and
/// the polar radius of the ellipsoid, in units of the equatorial
/// radius.  This should be a non-negative value less than 1.  A value
/// equal to 0 indicates a perfect sphere.
Ellipsoid::Ellipsoid(double equatorial_radius, double flattening)
{
  private_ = new EllipsoidPrivate(equatorial_radius, flattening);
}

/// Destroys the Ellipsoid.
Ellipsoid::~Ellipsoid()
{
  delete private_;
  private_ = 0;
}

/// Returns the equatorial radius of the ellipsoid, in the same units
/// with which the Ellipsoid was created.
///
/// \return the equatorial radius.
double
Ellipsoid::equatorial_radius() const
{
  return private_->equatorial_radius_;
}

/// Returns the flattening of the ellipsoid.
///
/// \return the flattening, as a dimensionless number.
double
Ellipsoid::flattening() const
{
  return private_->flattening_;
}

/// Returns the geocentric latitude of the location of interest.
///
/// \param[in] geographic_latitude_rad is the geographic latitude of
/// the location of interest, measured in radians.
///
/// \param[in] height is the height of the location of interest with
/// respect to the ellipsoid.
///
/// \return the geocentric latitude of the location of interest,
/// measured in radians.
double
Ellipsoid::geocentric_latitude_rad(double geographic_latitude_rad,
                                   double height) const
{
  double xy, z;
  private_->xy_z(geographic_latitude_rad, height, &xy, &z);
  double geocentric_latitude_rad = atan2(z, xy);
  return geocentric_latitude_rad;
}

/// Calculates the cartesian coordinates of a location of interest
/// specified in geographic coordinates.  All input and output values
/// with the dimension of length are expressed in the same unit that
/// was used to specify the equatorial radius of the ellipsoid.
///
/// \param[in] geographic_latitude_rad is the geographic latitude of
/// the location of interest, measured in radians.
///
/// \param[in] longitude_rad is the (geographic) longitude of the
/// location of interest, measured in radians.
///
/// \param[in] height is the height of the location of interest with
/// respect to the ellipsoid.
///
/// \param[out] xyz is a pointer to a buffer with room for at least 3
/// values, to which the cartesian coordinates (x, y, z) of the
/// location of interest are written.
void
Ellipsoid::xyz(double geographic_latitude_rad, double longitude_rad,
               double height, double* xyz) const
{
  double xy, z;
  private_->xy_z(geographic_latitude_rad, height, &xy, &z);
  xyz[0] = xy;
  xyz[1] = 0;
  xyz[2] = z;
  Rotate3d::rotate_z(xyz, longitude_rad);
}
