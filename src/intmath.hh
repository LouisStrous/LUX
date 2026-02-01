/* This is file intmath.hh.

Copyright 2013-2014 Louis Strous

This file is part of LUX.

LUX is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your
option) any later version.

LUX is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with LUX.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef INTMATH_H
#define INTMATH_H

#include <cmath>
#include <concepts>
#include <cstdlib>
#include "action.hh"

// iasmod → rremainder
// i64asmod → rremainder
// fasmod → rremainder

/// Floored quotient, from floored division, which rounds the result
/// down to the nearest integer toward minus infinity.
///
/// The standard / operator does truncated division instead, which
/// rounds the result toward zero.
///
/// \param numerator is the numerator of the fraction.
///
/// \param denominator is the denominator of the fraction.
///
/// \returns the floored quotient.
constexpr auto
fquotient(std::floating_point auto numerator,
          std::floating_point auto denominator) noexcept
{
  return std::floor(numerator/denominator);
}

/// Floored quotient, from floored division, which rounds the result
/// down to the nearest integer toward minus infinity.
///
/// The standard / operator does truncated division instead, which
/// rounds the result toward zero.
///
/// \param numerator is the numerator of the fraction.
///
/// \param denominator is the denominator of the fraction.
///
/// \returns the floored quotient.
constexpr auto
fquotient(std::integral auto numerator, std::integral auto denominator) noexcept
{
  auto qr = std::div((int) numerator, (int) denominator);
  return (numerator/denominator)
    - (sgn(numerator)*sgn(denominator) < 0);
}

constexpr auto
fquotient(std::floating_point auto numerator,
          std::integral auto denominator) noexcept
{
  return std::floor(numerator/denominator);
}

constexpr auto
fquotient(std::integral auto numerator,
          std::floating_point auto denominator) noexcept
{
  return std::floor(numerator/denominator);
}

/// Floored remainder, from floored division, which rounds the
/// quotient down to the nearest integer toward minus infinity.
///
/// std::fmod() does truncated division instead, which rounds the
/// quotient toward zero.
///
/// \param numerator is the numerator of the fraction.
///
/// \param denominator is the denominator of the fraction.
///
/// \returns the floored remainder.
constexpr auto
fremainder(std::floating_point auto numerator,
           std::floating_point auto denominator) noexcept
{
  return std::fmod(numerator, denominator)
    + ((sgn(numerator)*sgn(denominator) < 0)? denominator: 0);
}

constexpr auto
fremainder(std::floating_point auto numerator,
           std::integral auto denominator) noexcept
{
  return std::fmod(numerator, denominator)
    + ((sgn(numerator)*sgn(denominator) < 0)? denominator: 0);
}

constexpr auto
fremainder(std::integral auto numerator,
           std::floating_point auto denominator) noexcept
{
  return std::fmod(numerator, denominator)
    + ((sgn(numerator)*sgn(denominator) < 0)? denominator: 0);
}

/// Floored remainder, from floored division, which rounds the
/// quotient down to the nearest integer toward minus infinity.
///
/// std::fmod() does truncated division instead, which rounds the
/// quotient toward zero.
///
/// \param numerator is the numerator of the fraction.
///
/// \param denominator is the denominator of the fraction.
///
/// \returns the floored remainder.
constexpr auto
fremainder(std::integral auto numerator,
           std::integral auto denominator) noexcept
{
  return (numerator%denominator)
    + ((sgn(numerator)*sgn(denominator) < 0)? denominator: 0);
}

/// A template type that represents the quotient and remainder of a
/// division of arbitrary type.
///
/// \tparam T is the data type of the quotient and remainder.
template<typename T>
struct Div {
  T quot;                       /// The quotient
  T rem;                        /// The remainder
};

/// Floored division, which rounds the quotient down to the nearest
/// integer toward minus infinity.
///
/// std::div() does truncated division instead, which rounds the
/// quotient toward zero.
///
/// \tparam T is the data type of numerator, denominator, quotient,
/// and remainder.
///
/// \param numerator is the numerator.
///
/// \param denominator is the denominator.
///
/// \returns the quotient and remainder wrapped in a #Div.
template<typename T>
constexpr auto
fdivide(T numerator, T denominator) noexcept
{
  return Div{fquotient(numerator, denominator),
             fremainder(numerator, denominator)};
}

Div<int32_t>
flinediv(int32_t numerator, int32_t factor, int32_t addend,
         int32_t denominator);

int32_t
flinequot(int32_t numerator, int32_t factor, int32_t addend,
          int32_t denominator);

int32_t
flinemod(int32_t numerator, int32_t factor, int32_t addend,
         int32_t denominator);

/// Rounded quotient, from rouded division, which rounds the result to
/// the nearest integer.
///
/// The standard / operator does truncated division instead, which
/// rounds the result toward zero.
///
/// \param numerator is the numerator of the fraction.
///
/// \param denominator is the denominator of the fraction.
///
/// \returns the floored quotient.
constexpr auto
rquotient(std::integral auto numerator,
          std::integral auto denominator) noexcept
{
  /*
    | / |     |    |    |    |    |   |            <r> |
    |   |   n |  d |  s |  q |  r |   |                |
    |---+-----+----+----+----+----+---+----------------|
    | # |  12 |  7 |  1 |  1 |  5 |   |   12 = 2×7 − 2 |
    | # | −12 |  7 | -1 | -1 | -5 |   | −12 = −2×7 + 2 |
    | # | −12 | −7 |  1 |  1 | -5 |   | −12 = 2×−7 + 2 |
    | # |  12 | −7 | -1 | -1 |  5 |   | 12 = −2×−7 − 2 |
    | # |   8 |  7 |  1 |  1 |  1 | ✓ |    8 = 1×7 + 1 |
    | # |  −8 |  7 | -1 | -1 | -1 | ✓ |  −8 = −1×7 − 1 |
    | # |  −8 | −7 |  1 |  1 | -1 | ✓ |  −8 = 1×−7 − 1 |
    | # |   8 | −7 | -1 | -1 |  1 | ✓ |  8 = −1×−7 + 1 |
    | # |   5 |  7 |  1 |  0 |  5 |   |    5 = 1×7 − 2 |
    | # |  −5 |  7 | -1 |  0 | -5 |   |  −5 = −1×7 + 2 |
    | # |  −5 | −7 |  1 |  0 | -5 |   |  −5 = 1×−7 + 2 |
    | # |   5 | −7 | -1 |  0 |  5 |   |  5 = −1×−7 − 2 |
    | # |   3 |  7 |  1 |  0 |  3 | ✓ |    3 = 0×7 + 3 |
    | # |  −3 |  7 | -1 |  0 | -3 | ✓ |   −3 = 0×7 − 3 |
    | # |  −3 | −7 |  1 |  0 | -3 | ✓ |  −3 = 0×−7 − 3 |
    | # |   3 | −7 | -1 |  0 |  3 | ✓ |   3 = 0×−7 − 3 |
    | # |   0 |  7 |  0 |  0 |  0 | ✓ |    0 = 0×7 + 0 |
    | # |   0 | -7 |  0 |  0 |  0 | ✓ |    0 = 0×7 − 0 |
    #+TBLFM: $4=sign($2*$3)::$5=trunc($2/$3)::$6=$2-$3*$5::

    if 2*|r| < |d| then q (from integer n/d) is good
    otherwise increase q by 1 with the sign of n*d
  */
  return numerator/denominator
    + ((2*abs(numerator%denominator) >= abs(denominator))?
       sgn(numerator)*sgn(denominator): 0);
}

constexpr auto
rquotient(std::floating_point auto numerator,
          std::floating_point auto denominator) noexcept
{
  return std::round(numerator/denominator);
}

constexpr auto
rquotient(std::floating_point auto numerator,
          std::integral auto denominator) noexcept
{
  return std::round(numerator/denominator);
}
constexpr auto
rquotient(std::integral auto numerator,
          std::floating_point auto denominator) noexcept
{
  return std::round(numerator/denominator);
}

/// Rounded remainder, from rounded division, which rounds the result
/// to the nearest integer.
///
/// The standard % operator does truncated division instead, which
/// rounds the quotient toward zero so the remainder, if nonzero, has
/// the same sign as the denominator.
///
/// \param numerator is the numerator of the fraction.
///
/// \param denominator is the denominator of the fraction.
///
/// \returns the rounded remainder.
constexpr auto
rremainder(std::integral auto numerator,
           std::integral auto  denominator) noexcept
{
  return numerator - denominator*rquotient(numerator, denominator);
}

constexpr auto
rremainder(std::floating_point auto numerator,
          std::floating_point auto denominator) noexcept
{
  return numerator - denominator*rquotient(numerator, denominator);
}

constexpr auto
rremainder(std::floating_point auto numerator,
          std::integral auto denominator) noexcept
{
  return numerator - denominator*rquotient(numerator, denominator);
}
constexpr auto
rremainder(std::integral auto numerator,
          std::floating_point auto denominator) noexcept
{
  return numerator - denominator*rquotient(numerator, denominator);
}

/// Rounded division, which rounds the quotient to the nearest
/// integer.
///
/// std::div() does truncated division instead, which rounds the
/// quotient toward zero.
///
/// \tparam T is the data type of numerator, denominator, quotient,
/// and remainder.
///
/// \param numerator is the numerator.
///
/// \param denominator is the denominator.
///
/// \returns the quotient and remainder wrapped in a #Div.
template<typename T>
constexpr auto
rdivide(T numerator, T denominator) noexcept
{
  return Div{rquotient(numerator, denominator),
             rremainder(numerator, denominator)};
}

#endif
