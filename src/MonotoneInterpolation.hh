#ifndef MONOTONEINTERPOLATION_HPP_
#define MONOTONEINTERPOLATION_HPP_

#include <cstddef>              // for size_t
#include <vector>

/// \file
///
/// This file declares a class for performing monotonic cubic
/// interpolation in the graph of a function of one parameter.


/// This class performs monotonic cubic interpolation in the graph of
/// a function of one parameter, using the Fritsch-Carlson method. See
/// https://en.wikipedia.org/wiki/Monotone_cubic_interpolation.
///
/// The interpolation is monotonic in the sense that the interpolated
/// curve between any two adjacent data points does not go outside of
/// the rectangle defined by those data points.
class MonotoneInterpolation {
public:

  /// An enumeration of the different supported methods for enforcing
  /// interpolation to be monotonic.  The efficiency is expressed in
  /// terms of the fraction of the acceptable area in the relevant
  /// parameter space that is used by the method.  Higher fractions
  /// mean smoother results.
  enum MonotoneMethodSelection {
    /// Cubic Hermite spline, monotonicity *not* enforced.
    NONE,

    /// Monotonicity enforced using a circle in parameter space.
    /// Covers 53% of the acceptable area in parameter space.
    CIRCLE,

    /// Monotonicity enforced using a square in parameter space.
    /// Covers 68% of the acceptable area in parameter space.
    SQUARE,

    /// Monotonicity enforced using a wider area in parameter space.
    /// Covers 98% of the acceptable area in parameter space.
    WIDE,
  };

  MonotoneInterpolation(const std::vector<double>& x, const std::vector<double>& y, MonotoneMethodSelection method = WIDE);
  MonotoneInterpolation(size_t n, const double* x, const double* y, MonotoneMethodSelection method = WIDE);
  double interpolate(double xTarget) const;
  static double interpolate(const std::vector<double> x, const std::vector<double> y, double xTarget, MonotoneMethodSelection = WIDE);

private:

  /// A struct that defines a point in two dimensions.
  struct Point {
    /// The default constructor, sets both coordinates to 0.
    Point() : m_x(0), m_y(0) { }

    Point(double x, double y);

    /// The x coordinate
    double m_x;

    /// The y coordinate
    double m_y;
  };

  /// A collection of instances of Point.
  typedef std::vector<Point> PointCollection;

  static inline Point constructPoint(double x, double y);

  void initialize(size_t n, const double* x, const double* y,
                  MonotoneMethodSelection method);

  /// The data points to interpolate between.
  PointCollection m_points;

  /// The slope of the secant lines connecting adjacent data points.
  std::vector<double> m_slopeSecants;

  /// The slope of the tangent line at each data point.
  std::vector<double> m_slopeTangents;

  friend bool operator<(const Point& lhs, const Point& rhs);
};

bool operator<(const MonotoneInterpolation::Point& lhs,
               const MonotoneInterpolation::Point& rhs);

#endif
