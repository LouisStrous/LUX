#include "config.h"

#ifdef HAVE_LIBSOFA_C
# include <algorithm>
# include <cmath>
# include <calceph.h>
# include <sofa.h>
# include "SolarSystemEphemerides.hh"

void astropos(double JD, int target_object, int observer_object, double xyz[3]);

/// Default constructor.  The default angle unit is AngleUnit::degree (degrees).
/// The default length unit is LengthUnit::AU (Astronomical Unit).
SolarSystemEphemerides::SolarSystemEphemerides()
  : m_angle_unit(),
    m_coordinate_system(),
    m_custom_angle_unit_rad(1),
    m_custom_length_unit_m(1),
    m_equinox(),
    m_equinox_JD(),
    m_length_unit()
{
  iauIr(m_coordinate_transform); // identity transform
}

/// Sets the angle unit for subsequent ephemerides.
///
/// \param unit is the unit to select.  If you want to specify a custom angle
/// unit, then use set_angle_unit() instead.
///
/// \returns a reference to the current object.
SolarSystemEphemerides&
SolarSystemEphemerides::set_angle_unit(AngleUnit unit)
{
  m_angle_unit = unit;
  return *this;
}

/// Sets the length unit for subsequent ephemerides.
///
/// \param unit is the unit to select.  If you want to specify a custom length
/// unit, then use set_length_unit() instead.
///
/// \returns a reference to the current object.
SolarSystemEphemerides&
SolarSystemEphemerides::set_length_unit(LengthUnit unit)
{
  m_length_unit = unit;
  return *this;
}

/// Sets a custom angle unit for subsequent ephemerides.
///
/// \param unit_rad is the custom unit to use, measured in radians.  If it is 0,
/// then 1 is used instead.  If it is negative, then the corresponding positive
/// value is used instead.  If you want to use a custom unit that is equivalent
/// to 0.123 radians, then specify 0.123.
///
/// \returns a reference to the current object.
SolarSystemEphemerides&
SolarSystemEphemerides::set_angle_unit(double unit_rad)
{
  if (unit_rad == 0)
    unit_rad = 1;
  m_custom_angle_unit_rad = std::abs(unit_rad);
  m_angle_unit = AngleUnit::Custom;
  return *this;
}

/// Sets a custom length unit for subsequent ephemerides.
///
/// \param unit_rad is the custom unit to use, measured in meters.  If it is 0,
/// then 1 is used instead.  If it is negative, then the corresponding positive
/// value is used instead.  If you want to use a custom unit that is equivalent
/// to 0.123 m, then specify 0.123.
///
/// \returns a reference to the current object.
SolarSystemEphemerides&
SolarSystemEphemerides::set_length_unit(double unit_m)
{
  if (unit_m == 0)
    unit_m = 1;
  m_custom_length_unit_m = std::abs(unit_m);
  m_length_unit = LengthUnit::Custom;
  return *this;
}

/// Sets the coordinate system for subsequent ephemerides.
///
/// \param system is the coordinate system to select.
///
/// \returns a reference to the current object.
SolarSystemEphemerides&
SolarSystemEphemerides::set_coordinate_system(CoordinateSystem system)
{
  m_coordinate_system = system;
  return *this;
}

/// Sets the ecliptic/equinox for subsequent ephemerides.
///
/// \param equinox is the coordinate system to select.  If it is
/// Equinox::Custom, then the ecliptic/equinox of JD 0 is selected.  Use
/// set_equinox(double) to select a different Julian Day.
///
/// \returns a reference to the current object.
SolarSystemEphemerides&
SolarSystemEphemerides::set_equinox(Equinox equinox)
{
  m_equinox = equinox;
  switch (m_equinox) {
  case Equinox::Bare:
  case Equinox::J2000:
    m_equinox_JD = AstronomicalConstants::J2000;
    break;
  case Equinox::Custom:         // should use set_equinox(double) instead
  case Equinox::Date:           // not a fixed date
    m_equinox_JD = 0;           // for consistency
    break;
  }
  return *this;
}

/// Sets the equinox for subsequent ephemerides.
///
/// \param JD is the Julian Day of the equinox.
///
/// \returns a reference to the current object.
SolarSystemEphemerides&
SolarSystemEphemerides::set_equinox(double JD)
{
  m_equinox = Equinox::Custom;
  m_equinox_JD = JD;
  return *this;
}

/// Returns the 3-dimensional cartesian (x, y, z) geocentric coordinates of a
/// Solar System object.
///
/// \param JD is the Julian Day number for which the position is desired.
///
/// \param target identifies the target object.
///
/// \returns the cartesian geocentric coordinates coordinates of the target
/// object.
SolarSystemEphemerides::Coords3
SolarSystemEphemerides::cartesian_geocentric(double JD,
                                             SolarSystemObject target) const {
  return cartesian(JD, target, SolarSystemObject::Earth);
}

/// Returns the 3-dimensional cartesian (x, y, z) heliocentric coordinates of a
/// Solar System object.
///
/// \param JD is the Julian Day number for which the position is desired.
///
/// \param target identifies the target object.
///
/// \returns the cartesian heliocentric coordinates coordinates of the target
/// object.
SolarSystemEphemerides::Coords3
SolarSystemEphemerides::cartesian_heliocentric(double JD,
                                               SolarSystemObject target) const {
  return cartesian(JD, target, SolarSystemObject::Sun);
}

/// Returns the 3-dimensional cartesian (x, y, z) coordinates of a Solar System
/// object relative to another one, without transforming them to the desired
/// units.
///
/// \param JD is the Julian Day number for which the position is desired.
///
/// \param target identifies the target object.
///
/// \param observer identifies the object where the observer resides (in the
/// center).
///
/// \returns the cartesian coordinates coordinates of the target object relative
/// to the observer.
SolarSystemEphemerides::Coords3
SolarSystemEphemerides::cartesian_bare_units(double JD,
                                             SolarSystemObject target,
                                             SolarSystemObject observer) const {
  Coords3 result;

  astropos(JD, static_cast<int>(target), static_cast<int>(observer),
           result.data());
  // now result contains cartesian equatorial (ICRS) coordinates in AU

  double equinox_JD;

  switch (m_equinox) {
  case Equinox::Date:
    equinox_JD = JD;
    break;
  case Equinox::Bare:
    equinox_JD = AstronomicalConstants::J2000;
    break;
  }

  // TODO: avoid adjusting m_coordinate_system when it is already good
  switch (m_coordinate_system) {
  case CoordinateSystem::Ecliptic:
    // get matrix for converting cartesian equatorial (ICRS) to ecliptic
    // coordinates for a particular equinox
    iauEcm06(equinox_JD, 0, m_coordinate_transform);
    break;
  case CoordinateSystem::Equatorial:
    // get matrix for precessing cartesian equatorial (ICRS) to a particular
    // date.  TODO: SOFA documentation says "from GCRS to specified date".
    iauPmat06(equinox_JD, 0, m_coordinate_transform);
    break;
  case CoordinateSystem::Bare:
    iauIr(m_coordinate_transform);
    break;
  }

  // precess and convert to target coordinate system
  iauRxp(m_coordinate_transform, result.data(), result.data());

  return result;
}

/// Returns the 3-dimensional cartesian (x, y, z) coordinates of a Solar System
/// object relative to another one.
///
/// \param JD is the Julian Day number for which the position is desired.
///
/// \param target identifies the target object.
///
/// \param observer identifies the object where the observer resides (in the
/// center).
///
/// \returns the cartesian coordinates coordinates of the target object relative
/// to the observer.
SolarSystemEphemerides::Coords3
SolarSystemEphemerides::cartesian(double JD, SolarSystemObject target,
                                  SolarSystemObject observer) const {
  Coords3 result = cartesian_bare_units(JD, target, observer);

  switch (m_length_unit) {
  case LengthUnit::meter:
    std::transform(result.begin(), result.end(), result.begin(),
                   [](double x){ return x*AstronomicalConstants::AU_m; });
    break;
  case LengthUnit::AU:
    break;
  case LengthUnit::Custom:
    std::transform(result.begin(), result.end(), result.begin(),
                   [&](double x){ return x*AstronomicalConstants::AU_m
                       / m_custom_length_unit_m; });
    break;
  }
  return result;
}

/// Returns the 3-dimensional polar (longitude, latitude, distance) geocentric
/// coordinates of a Solar System object.
///
/// \param JD is the Julian Day number for which the position is desired.
///
/// \param target identifies the target object.
///
/// \returns the polar geocentric coordinates coordinates of the target object.
SolarSystemEphemerides::Coords3
SolarSystemEphemerides::polar_geocentric(double JD, SolarSystemObject target)
  const
{
  return polar(JD, target, SolarSystemObject::Earth);
}

/// Returns the 3-dimensional polar (longitude, latitude, distance) heliocentric
/// coordinates of a Solar System object.
///
/// \param JD is the Julian Day number for which the position is desired.
///
/// \param target identifies the target object.
///
/// \returns the polar heliocentric coordinates coordinates of the target
/// object.
SolarSystemEphemerides::Coords3
SolarSystemEphemerides::polar_heliocentric(double JD,
                                           SolarSystemObject target) const {
  return polar(JD, target, SolarSystemObject::Sun);
}

/// Returns the combined hypotenuse of 3 cartesian coordinates.
///
/// \param x is the first value.
///
/// \param y is the second value.
///
/// \param z is the third value.
///
/// \returns the hypotenuse.
double
hypot3(double x, double y, double z)
{
  return hypot(hypot(x, y), z);
}

/// Returns the 3-dimensional polar (longitude, latitude, distance) coordinates
/// of a Solar System object relative to another one.
///
/// \param JD is the Julian Day number for which the position is desired.
///
/// \param target identifies the target object.
///
/// \param observer identifies the object where the observer resides (in the
/// center).
///
/// \returns the polar heliocentric coordinates coordinates of the target
/// object relative to the observer.
SolarSystemEphemerides::Coords3
SolarSystemEphemerides::polar(double JD, SolarSystemObject target,
                              SolarSystemObject observer) const {
  Coords3 result = cartesian_bare_units(JD, target, observer);

  // convert from cartesian (AU) to polar coordinates (radians/AU)
  double lon, lat, r;
  iauP2s(result.data(), &lon, &lat, &r);
  result[0] = iauAnp(lon);      // force into range 0 .. 2π
  result[1] = lat;
  result[2] = r;

  switch (m_angle_unit) {
  case AngleUnit::degree:
    std::transform(result.begin(), &result[2], result.begin(),
                   [](double x){ return x*180/M_PI; });
    break;
  case AngleUnit::radian:
    break;
  case AngleUnit::Custom:
    std::transform(result.begin(), &result[2], result.begin(),
                   [&](double x){ return x/m_custom_angle_unit_rad; });
    break;
  }
  switch (m_length_unit) {
  case LengthUnit::meter:
    result[2] *= AstronomicalConstants::AU_m;
    break;
  case LengthUnit::AU:
    break;
  case LengthUnit::Custom:
    result[2] *= AstronomicalConstants::AU_m/m_custom_length_unit_m;
    break;
  }
  return result;
}
#endif
