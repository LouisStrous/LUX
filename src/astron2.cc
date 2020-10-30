// Copyright 2020 Louis Strous
//
// This file is part of LUX.
//
// LUX is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your
// option) any later version.
//
// LUX is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License
// along with LUX.  If not, see <http://www.gnu.org/licenses/>.

/// \file
/// LUX routines for calculating various astronomical ephemerides and for
/// transforming dates between various calendars.
///
/// The calculations are based on the NASA JPL DE431 ephemeris, and use the
/// IMCCE CALCEPH and IAU SOFA libraries.

#include <calceph.h>
#include <cmath>                // for std::nan
#include <vector>

#include "config.h"

#include "action.hh"
#if HAVE_LIBSOFA_C
# include "sofa.h"
#endif

const double DEGREE_PER_RAD = 180/M_PI;
const double RAD_PER_DEGREE = M_PI/180;
const double J2000 = 2451545.0; // Julian Day Number of J2000.0

/// Translate a LUX celestial object number to a CALCEPH/NAIF celestial object
/// number.
int lux_to_calceph_object_id(int lux_object_id)
{
  // LUX recognizes 0 for the Sun, but CALCEPH/NAIF do not use 0.
  if (lux_object_id == 0)
    return 11;                  // Sun
  return lux_object_id;
}

//     XYZ = ASTRON2(JD, OBJECTS, OBSOBJECT)
//
// Returns the cartesian x, y, z coordinates of one or more celestial target
// objects relative to an observer object for one or more instant of time.  The
// returned coordinates are measured in Astronomical Units relative to the
// International Celestial Reference System (ICRS).
//
// Arguments:
//
// JD = Julian Day numbers of the instants of time, in Barycentric Dynamical
// Time (TDB).
//
// OBJECTS = The identifying numbers of the target objects.
//
// OBSOBJECT = The identifying number of the observer object.
//
// The identifying numbers of the celestial objects include:
//
//  0 Sun
//  1 Mercury barycenter
//  2 Venus barycenter
//  3 Earth
//  4 Mars system barycenter
//  5 Jupiter system barycenter
//  6 Saturn system barycenter
//  7 Uranus system barycenter
//  8 Neptune system barycenter
//  9 Pluto system barycenter
// 10 Moon
// 11 Sun
// 12 Solar System barycenter
// 13 Earth-Moon barycenter
// TODO: Asteroids

void
astropos(double JD, int target_object, int observer_object, double xyz[3])
{
  // got de431t.bsp from ftp://ssd.jpl.nasa.gov/pub/eph/planets/bsp
#if HAVE_LIBCALCEPH
  const double AU_m = 149'597'870'700; // 1 AU in meters
  static t_calcephbin* de431 = NULL;
  if (!de431)                   // TODO: make filename configurable
    de431 = calceph_open(expand_name("~/data/de431t.bsp", NULL));

  bool ok = false;
  if (de431) {
    const int units = (CALCEPH_UNIT_KM + CALCEPH_UNIT_SEC);
    double posvel[6];

    target_object = lux_to_calceph_object_id(target_object);
    observer_object = lux_to_calceph_object_id(observer_object);

    // If we use calceph_compute() then it complains that the DE431 ephemeris
    // file doesn't provide a definition of the "AU" constant (i.e., the size of
    // an Astronomical Unit).  So we must use calceph_compute_unit() instead and
    // ask for kilometers, and convert the results to AU ourselves.
    if (calceph_compute_unit(de431, JD, 0, target_object, observer_object,
                             CALCEPH_UNIT_KM + CALCEPH_UNIT_SEC, posvel)) {
      // got km, convert to AU
      for (int i = 0; i < 3; ++i)
        xyz[i] = posvel[i]*1e3/AU_m;
    }
  } else {
    // retrieval failed.  Return NaNs.
    double nan = std::nan("DE431");
    std::fill(xyz, xyz + 3, nan);
  }
#else
  cerror(NOSUPPORT, 0, "DE431", "libcalceph");
#endif
}

/// Get positions of astronomical objects from NASA JPL DE431.
int32_t
lux_astron2(int32_t narg, int32_t ps[])
{
#if HAVE_LIBCALCEPH
  std::vector<int32_t> output_dimensions;
  Pointer JDs, target_object_ids;
  int JD_count, target_object_id_count;
  int observer_object_id;
  double JD_equinox;

  const int S_KEEPDIMS   = 1;

  const int S_ECLIPTIC   = 0;   // ecliptic coordinates (default)
  const int S_EQUATORIAL = 2;   // equatorial coordinates
  const int S_BARE       = 4;   // non-rotated coordinates
  const int S_COORDAXES  = (S_BARE | S_ECLIPTIC | S_EQUATORIAL);

  const int S_POLAR      = 0;   // polar coordinates (default)
  const int S_XYZ        = 8;   // cartesian coordinates
  const int S_COORDTYPE  = (S_POLAR | S_XYZ);

  const int S_EQUINOX_DATE = 16;

  const int S_CONJUNCTION_SPREAD = 32;

  {
    // process arguments, and determine the dimensions of the return symbol

    // 3 data values (x, y, z) per result
    output_dimensions.push_back(3);

    int *dims;                  // pointer to list of dimensions
    int dims_count;             // dimensions count
    int elem_count;             // element count
    Pointer ptr;
    int32_t iq;

    // ps[1] specifies the target objects
    iq = lux_long(1, &ps[1]);
    if (numerical(iq, &dims, &dims_count, &target_object_id_count,
                  &target_object_ids) == LUX_ERROR)
      return LUX_ERROR;
    if ((internalMode & S_CONJUNCTION_SPREAD) == 0
        && (dims_count > 1
            || dims[0] != 1
            || (internalMode & S_KEEPDIMS))) {
      output_dimensions.insert(output_dimensions.end(), dims,
                               dims + dims_count);
    }

    // ps[0] specifies the instants of time
    iq = lux_double(1, &ps[0]);
    if (numerical(iq, &dims, &dims_count, &JD_count, &JDs) == LUX_ERROR)
      return LUX_ERROR;
    if (dims_count > 1 || dims[0] != 1 || (internalMode & S_KEEPDIMS)) {
      output_dimensions.insert(output_dimensions.end(), dims,
                               dims + dims_count);
    }

    if (narg >= 3 && ps[2]) {
      iq = lux_long(1, &ps[2]);   // observer object ID
      if (numerical(iq, NULL, NULL, &elem_count, &ptr) == LUX_ERROR)
        return LUX_ERROR;
      if (elem_count != 1)
        return cerror(NEED_SCAL, ps[2]);
      observer_object_id = *ptr.i32;
    } else {
      observer_object_id = 3;   // Earth
    }

    if (narg >= 4 && ps[3]) {
      if (internalMode & S_EQUINOX_DATE)
        return luxerror("Cannot specify both /date and equinox=... on astron2",
                        ps[3]);
      iq = lux_double(1, &ps[3]); // equinox
      if (numerical(iq, NULL, NULL, &elem_count, &ptr) == LUX_ERROR)
        return LUX_ERROR;
      if (elem_count != 1)
        return cerror(NEED_SCAL, ps[3]);
      JD_equinox = *ptr.d;
    } else {
      JD_equinox = J2000;
    }
  }

  int32_t result = array_scratch(LUX_DOUBLE, output_dimensions.size(),
                                 &output_dimensions[0]);
  Pointer output;
  numerical(result, NULL, NULL, NULL, &output);

  int coordinate_axes = (internalMode & S_COORDAXES);
  int coordinate_type = (internalMode & S_COORDTYPE);

  double M[3][3];
  if ((internalMode & S_EQUINOX_DATE) == 0) {
    // precess to fixed date

    switch (coordinate_axes) {
    case S_ECLIPTIC:
      // get matrix for converting cartesian equatorial (ICRS) to ecliptic
      // coordinates for a particular equinox
      iauEcm06(JD_equinox, 0, M);
      break;
    case S_EQUATORIAL:
      // get matrix for precessing cartesian equatorial (ICRS) to a particular
      // date.  TODO: SOFA documentation says "from GCRS to specified date".
      iauPmat06(JD_equinox, 0, M);
      break;
    case S_BARE:
      iauIr(M);
      break;
    }
  }

  for (int ijd = 0; ijd < JD_count; ++ijd) {
    if (internalMode & S_EQUINOX_DATE) { // precess to date
      switch (coordinate_axes) {
      case S_ECLIPTIC:
        // get matrix for converting cartesian equatorial (ICRS) to ecliptic
        // coordinates for target date
        iauEcm06(*JDs.d, 0, M);
        break;
      case S_EQUATORIAL:
        // get matrix for precessing cartesian equatorial (ICRS) to target date.
        // TODO: SOFA documentation says "from GCRS to specified date".
        iauPmat06(*JDs.d, 0, M);
        break;
      case S_BARE:
        iauIr(M);
        break;
      }
    }
    double average[3];
    if (internalMode & S_CONJUNCTION_SPREAD)
      std::fill(average, average + 3, 0.0);
    for (int ito = 0; ito < target_object_id_count; ++ito) {
      astropos(*JDs.d, target_object_ids.i32[ito], observer_object_id,
               output.d);
      // now output.d[0..2] contain the X, Y, and Z coordinates (in AU) relative
      // to ICRS -- which is close to (equatorial) J2000.0

      // precess and convert to ecliptic if needed
      iauRxp(M, output.d, output.d);

      if (internalMode & S_CONJUNCTION_SPREAD) {
        double dummy;
        iauPn(output.d, &dummy, output.d); // normalize
        for (int i = 0; i < 3; ++i)
          average[i] += output.d[i];
      } else {
        if (coordinate_type == S_POLAR) {
          // TODO: handle absence of SOFA
          double lon, lat, r;
          // convert from cartesian to polar coordinates
          iauP2s(output.d, &lon, &lat, &r);
          // force RA in range 0 .. 2π
          output.d[0] = iauAnp(lon)*DEGREE_PER_RAD;
          output.d[1] = lat*DEGREE_PER_RAD;
          output.d[2] = r;
        }
        output.d += 3;
      }
    }
    if (internalMode & S_CONJUNCTION_SPREAD) {
      double w = hypota(3, average)/target_object_id_count;
      w = sqrt(-26262.45*log(w));
      double lon, lat, dummy;
      // convert from cartesian to polar coordinates
      iauP2s(average, &lon, &lat, &dummy);
      output.d[0] = iauAnp(lon)*DEGREE_PER_RAD;;
      output.d[1] = lat*DEGREE_PER_RAD;
      output.d[2] = w;
      output.d += 3;
    }
    ++JDs.d;
  }

  return result;
#else
  return cerror(NOSUPPORT, 0, "ASTRON2", "libcalceph")
#endif
}
REGISTER(astron2, f, astron2, 2, 4, ":::equinox:1keepdimensions:~6ecliptic:2equatorial:4bare:~8polar:8xyz:16date:32conjspread", HAVE_LIBCALCEPH)
