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

/// Signum function.
///
/// \tparam T is the data type.
///
/// \param x is the data value.
///
/// \returns `int` +1 if the argument is positive, 0 if the argument
/// is 0, -1 if the argument is negative.
template<typename T>
constexpr int
sgn(T x) noexcept
{
  return (x > 0)? 1: (x < 0)? -1: 0;
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
  return (numerator/denominator)
    - (sgn(numerator)*sgn(denominator) < 0);
}

constexpr auto
fquotient(std::floating_point auto numerator,
          std::integral auto denominator) noexcept
{
  return (numerator/denominator)
    - (sgn(numerator)*sgn(denominator) < 0);
}

constexpr auto
fquotient(std::integral auto numerator,
          std::floating_point auto denominator) noexcept
{
  return (numerator/denominator)
    - (sgn(numerator)*sgn(denominator) < 0);
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
    + (sgn(numerator)*sgn(denominator) < 0)? denominator: 0;
}

constexpr auto
fremainder(std::floating_point auto numerator,
           std::integral auto denominator) noexcept
{
  return std::fmod(numerator, denominator)
    + (sgn(numerator)*sgn(denominator) < 0)? denominator: 0;
}

constexpr auto
fremainder(std::integral auto numerator,
           std::floating_point auto denominator) noexcept
{
  return std::fmod(numerator, denominator)
    + (sgn(numerator)*sgn(denominator) < 0)? denominator: 0;
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
    + (sgn(numerator)*sgn(denominator) < 0)? denominator: 0;
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
alinediv(int32_t numerator, int32_t factor, int32_t addend,
         int32_t denominator);

int32_t
alinequot(int32_t numerator, int32_t factor, int32_t addend,
          int32_t denominator);

int32_t
alinemod(int32_t numerator, int32_t factor, int32_t addend,
         int32_t denominator);

#endif
