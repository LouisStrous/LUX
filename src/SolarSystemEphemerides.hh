#ifndef INCLUDED_SOLARSYSTEMEPHEMERIDES_HH
#define INCLUDED_SOLARSYSTEMEPHEMERIDES_HH

// standard includes

#include <array>

// our features include
#include "config.hh"

// our includes

#include "AstronomicalConstants.hh"

/// An enumeration of Solar System objects and (planetary) systems.  Names
/// without a suffix represent the barycenter of the named object.  Names with
/// an `_S` suffix represent the barycenter of the system of which the named
/// object is the main component.
enum class SolarSystemObject {
  Sun       = 0,
  Mercury   = 1,
  Venus     = 2,
  Earth     = 3,
  Mars_S    = 4,
  Jupiter_S = 5,
  Saturn_S  = 6,
  Uranus_S  = 7,
  Neptune_S = 8,
  Pluto_S   = 9,
  Moon      = 10,
  // For technical reasons, both 0 and 11 represents the Sun.
  Sun_S     = 12,
  Earth_S   = 13
};

/// A class that provides access to ephemerides (location) of Solar System
/// objects such as planets.
class SolarSystemEphemerides {
public:
  // typedefs, enums

  /// A convenience type for 3-dimensional coordinates.
  typedef std::array<double, 3> Coords3;

  /// Length units.
  enum class LengthUnit {
    AU,                         //!< Astronomical Unit
    meter,                      //!< Meter
    Custom,                     //!< Custom unit
  };

  /// Angle units.
  enum class AngleUnit {
    radian,                     //!< Radian
    degree,                     //!< Degree
    Custom,                     //!< Custom unit
  };

  /// Coordinate systems
  enum class CoordinateSystem {
    Bare,                   //!< the coordinate system of the ephemerides source
    Ecliptic,               //!< ecliptic coordinates
    Equatorial,             //!< equatorial coordinates
  };

  /// Equinox
  enum class Equinox {
    Bare,                    //!< the ecliptic/equinox of the ephemerides source
    J2000,                   //!< the ecliptic/equinox J2000.0
    Date,                    //!< the ecliptic/equinox of the date
    Custom                   //!< the ecliptic/equinox of a fixed date
  };

  // constructor

  SolarSystemEphemerides();

  // non-const members

  SolarSystemEphemerides& set_angle_unit(AngleUnit unit);
  SolarSystemEphemerides& set_angle_unit(double unit_rad);
  SolarSystemEphemerides& set_coordinate_system(CoordinateSystem system);
  SolarSystemEphemerides& set_equinox(Equinox equinox);
  SolarSystemEphemerides& set_equinox(double JD);
  SolarSystemEphemerides& set_length_unit(LengthUnit unit);
  SolarSystemEphemerides& set_length_unit(double unit_m);

  // const members

  Coords3 cartesian(double JD, SolarSystemObject target,
                    SolarSystemObject observer) const;
  Coords3 cartesian_bare_units(double JD, SolarSystemObject target,
                               SolarSystemObject observer) const;
  Coords3 cartesian_geocentric(double JD, SolarSystemObject target) const;
  Coords3 cartesian_heliocentric(double JD, SolarSystemObject target) const;
  Coords3 polar(double JD, SolarSystemObject target, SolarSystemObject observer)
    const;
  Coords3 polar_geocentric(double JD, SolarSystemObject target) const;
  Coords3 polar_heliocentric(double JD, SolarSystemObject target) const;

private:
  AngleUnit        m_angle_unit;
  CoordinateSystem m_coordinate_system;
  mutable double   m_coordinate_transform[3][3];
  double           m_custom_angle_unit_rad;
  double           m_custom_length_unit_m;
  Equinox          m_equinox;
  double           m_equinox_JD;
  LengthUnit       m_length_unit;
};

#endif
