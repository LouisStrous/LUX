#ifndef HAVE_CDIV_HH
#define HAVE_CDIV_HH

/// A template type to store the results of the cdiv template function.
template<typename T>
struct QuotRem {
  T quot;                       //!< the quotient
  T rem;                        //!< the remainder
};

/// A template function for floating-point division that produces the quotient
/// and remainder of dividing the numerator by the denominator.  The quotient is
/// always rounded down (toward minus infinity), unlike with std::div.  The "c"
/// in "cdiv" stands for "continuous" to indicate that the rounding direction is
/// the same in the whole domain.
///
/// \tparam Float is the underlying data type, which is expected to be a
/// floating-point type.
///
/// \param numerator is the numerator of the division, the value being divided.
///
/// \param denominator is the denominator of the division, the value being
/// divided by.
///
/// \returns the quotient and remainder of the division, wrapped in a
/// QuotRem<Float>.
template<typename Float,
         std::enable_if_t<std::is_floating_point<Float>::value, bool> = true>
QuotRem<Float>
cdiv(Float numerator, Float denominator) {
  QuotRem<Float> result;
  result.quot = numerator/denominator;
  result.rem = numerator - result.quot*denominator;
  // ensure that remainder is nonnegative
  if (result.rem < 0) {
    auto s = std::abs(denominator);
    --result.quot;
    result.rem += s;
  }
  return result;
}

/// A template function for integer division that produces the quotient and
/// remainder of dividing the numerator by the denominator.  The quotient is
/// always rounded down (toward minus infinity), unlike with std::div.  The "c"
/// in "cdiv" stands for "continuous" to indicate that the rounding direction is
/// the same in the whole domain.
///
/// \tparam Int is the underlying data type, which is expected to be an integer
/// type.
///
/// \param numerator is the numerator of the division, the value being divided.
///
/// \param denominator is the denominator of the division, the value being
/// divided by.
///
/// \returns the quotient and remainder of the division, wrapped in a
/// QuotRem<Int>.
template<typename Int,
         std::enable_if_t<std::is_integral<Int>::value, bool> = true>
QuotRem<Int>
cdiv(Int numerator, Int denominator) {
  auto qr = std::div(numerator, denominator);
  QuotRem<Int> result;
  result.quot = qr.quot;
  result.rem = qr.rem;
  // ensure that remainder is nonnegative
  if (result.rem < 0) {
    auto s = std::abs(denominator);
    --result.quot;
    result.rem += s;
  }
  return result;
}

#endif
