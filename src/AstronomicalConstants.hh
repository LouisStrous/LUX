/*
  This is file AstronomicalConstants.hh.

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

#ifndef ASTRONOMICALCONSTANTS_HH_
#define ASTRONOMICALCONSTANTS_HH_

/// \file
/// A namespace containing astronomical constants.

/// A namespace containing astronomical constants.
namespace AstronomicalConstants {
  /// The Astronomical Unit measured in meters.
  const double AU_m = 149597870700.;

  /// The equatorial radius of the Earth measured in meters.
  const double Earth_equatorial_radius_m = 6378137;

  /// The dimensionless flattening of the Earth.
  const double Earth_flattening = 1/298.257223563;
};

#endif
