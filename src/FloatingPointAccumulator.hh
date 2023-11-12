#ifndef INCLUDED_KAHAN_HH
#define INCLUDED_KAHAN_HH

/// This class provides a numerical value with reduced round-off error.  Any
/// number of values of its base type can be added to or subtracted from it,
/// with round-off error that grows much more slowly than for the base type.
/// This class is useful for example when thousands or millions of values are
/// summed, especially when some of those values can be much greater than
/// others.  The calculations are done using the <a
/// href="https://en.wikipedia.org/wiki/Kahan_summation_algorithm">Kahan
/// summation algorithm</a>.
///
/// Generally, using a wider base type produces less round-off error than using
/// the current class for the original base type, so the current class is most
/// useful when there is no wider base type.  If `double` is the widest
/// floating-point type, then use #FloatingPointAccumulator<double> if you need
/// to accumulate thousands or millions of `double` values.  If you need to
/// accumulate thousands or millions of `float` values, then using a `double`
/// value in which to accumulate the result provides more accurate results than
/// using a #FloatingPointAccumulator<float>.
///
/// Example:
///
/// \code
/// FloatingPointAccumulator<double> fpa;
/// for (auto it = collection.begin(); it != collection.end; ++it)
/// {
///    fpa += *it;
/// }
/// double sum = fpa; // 'sum' gets final sum with greater accuracy
/// \endcode
///
/// \tparam T is the base numerical type of the class.  It is expected to be a
/// floating-point type.  This class is not useful with integer base types.
template<typename T>
class FloatingPointAccumulator {
public:
  // constructor

  /// Constructor.  Initializes the value to 0.
  FloatingPointAccumulator()
    : m_sum(), m_compensation()
  { }

  // const members

  /// Cast to the base type.  This is the method for getting the final value
  /// when the accumulation is done.
  operator T() const
  {
    return m_sum;
  }

  // non-const members

  /// Add a value to the current instance.
  ///
  /// \param value is the value to add to the current instance.
  ///
  /// \returns a reference to the current instance.
  FloatingPointAccumulator&
  operator+=(const T value)
  {
    auto y = value - m_compensation;
    auto t = m_sum + y;
    m_compensation = (t - m_sum) - y;
    m_sum = t;
    return *this;
  }

  /// Subtract a value from the current instance.
  ///
  /// \param value is the value to subtract from the current instance.
  ///
  /// \returns a reference to the current instance.
  FloatingPointAccumulator&
  operator-=(const T value)
  {
    return operator+=(-value);
  }

private:
  T m_sum;
  T m_compensation;
};

#endif
