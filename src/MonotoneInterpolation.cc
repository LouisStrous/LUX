/// \file
///
/// This file defines class methods for monotonic cubic interpolation.

#include "MonotoneInterpolation.hh"

#include <algorithm>            // for std::min, std::transform
#include <cassert>
#include <cmath>                // for sqrt
#include <functional>           // for std::divides
#include <vector>

/// Construct a Point.
///
/// \param x is the x coordinate of the point.
///
/// \param y is the y coordinate of the point.
MonotoneInterpolation::Point::Point(double x, double y)
  : m_x(x), m_y(y)
{ }

/// Construct a piecewise monontonic interpolator.
///
/// \param x is a vector of x coordinates of the zero or more data
/// points to interpolate between.  It is assumed (but not checked)
/// that every value of x is unique.  It is not required for the x
/// coordinates to be in ascending order.
///
/// \param y is a vector of y coordinates of the zero or more data
/// points to interpolate between.
///
/// \param method selects which method to use to enforce monotonicity.
///
/// If x and y do not have the same number of elements, then the
/// excess elements from the longer of the two are ignored.
MonotoneInterpolation::MonotoneInterpolation(const std::vector<double>& x,
                                             const std::vector<double>& y,
                                             MonotoneMethodSelection method)
{
  // Expect the x and y coordinates to have the same number of
  // elements...

  assert(x.size() == y.size());

  // ... but if they don't then ignore the excess elements from the
  // longer one.

  size_t n = std::min(x.size(), y.size());

  initialize(n, &x[0], &y[0], method);
}

MonotoneInterpolation::MonotoneInterpolation(size_t n,
                                             const double* x,
                                             const double* y,
                                             MonotoneMethodSelection method)
{
  initialize(n, x, y, method);
}

void
MonotoneInterpolation::initialize(size_t n,
                                  const double* x,
                                  const double* y,
                                  MonotoneMethodSelection method)
{
  // Combine the x and y into Points

  m_points.resize(n);
  std::transform(&x[0], &x[0] + n, &y[0], m_points.begin(),
                 constructPoint);

  if (n >= 2) // further preparation is only needed if there are at
              // least 2 data points
  {

    // Sort the points into ascending order of x

    std::sort(m_points.begin(), m_points.end());

    // Calculate slopes; m_slopeSecants[i] = slope of segment between
    // m_points[i] and m_points[i+1]

    m_slopeSecants.resize(n - 1); // n - 1 segements

    {
      PointCollection::iterator it1;
      PointCollection::iterator it2;
      std::vector<double>::iterator it3;
      for (it1 = m_points.begin(), it2 = m_points.begin() + 1,
             it3 = m_slopeSecants.begin();
           it2 != m_points.end(); ++it1, ++it2, ++it3)
      {
        *it3 = (it2->m_y - it1->m_y)/(it2->m_x - it1->m_x);
      }
    }

    // Estimate tangents; m_slopeTangents[i] = tangent of curve at
    // m_points[i].  For interior points, the estimate is the average
    // of the slopes of the adjacent segments.  For the endpoints, the
    // estimate is the slope of the adjacent segment.

    m_slopeTangents.resize(n);
    m_slopeTangents[0] = m_slopeSecants.front();

    {
      std::vector<double>::iterator it1;
      std::vector<double>::iterator it2;
      std::vector<double>::iterator it3;
      for (it1 = m_slopeSecants.begin(),
             it2 = m_slopeSecants.begin() + 1,
             it3 = m_slopeTangents.begin() + 1;
           it2 != m_slopeSecants.end(); ++it1, ++it2, ++it3)
      {
        *it3 = (*it1 + *it2)/2;
      }
      *it3 = m_slopeSecants.back();
    }

    if (method != NONE)
    {
      // Now adjust the tangents to ensure piecewise monotonicity

      for (size_t i = 0; i < n - 2; ++i) // n - 2 pairs of segments
      {
        if (m_slopeSecants[i]*m_slopeSecants[i + 1] < 0)
        {
          // The slope of the secants have opposite sign on either
          // side, so there is a local maximum or local minimum.  Set
          // the tangent slope of the locally extreme point to zero.
          m_slopeTangents[i + 1] = 0.0;
        }
        else if (m_slopeSecants[i] == 0)
        {
          // The slope of the segment is zero, so the two adjacent
          // data points have the same y coordinate.  Set the tangent
          // slope to zero on both sides of the segment.
          m_slopeTangents[i] = m_slopeTangents[i + 1] = 0.0;
        }
      }

      std::vector<double> alpha(n - 1); // n - 1 segments
      std::vector<double> beta(n - 1);
      std::transform(m_slopeTangents.begin(),
                     m_slopeTangents.end() - 1,
                     m_slopeSecants.begin(),
                     alpha.begin(),
                     std::divides<double>());
      std::transform(m_slopeTangents.begin() + 1,
                     m_slopeTangents.end(),
                     m_slopeSecants.begin(),
                     beta.begin(),
                     std::divides<double>());

      // alpha and beta are the two nonnegative coordinates of points
      // in a parameter space that indicate whether or not
      // interpolating the associated segment is monotonic.  If the
      // distance of (alpha, beta) from the origin does not exceed 3,
      // then interpolation is certainly monotonic.  If the distance
      // exceeds 4, then interpolation is certainly not monotonic.
      //
      // Enforcing monotonicity requires that points that fall outside
      // of the region of parameter space that indicates monotonicity
      // be moved into that region (called the "region of
      // monotonicity" below).
      //
      // The wikipedia article mentions two ways of doing that, which
      // I call the "circle" and "square" methods.
      //
      // The "circle" method checks if the point (alpha, beta) is more
      // than 3 units away from the origin.  If that is the case, then
      // the tangent slopes are adjusted so that the point (alpha,
      // beta) is moved along the line to the origin, to the point
      // that is at distance 3 from the origin.  Then all points fall
      // within the first quadrant of the circle of radius 3 centered
      // on the origin.  I calculate that that quandrant covers about
      // 53% of the region of monotonicity.
      //
      // The "square" method checks if alpha > 3 or beta > 3.  If
      // either is the case, then the tangent slopes are adjusted so
      // that whichever one of alpha and beta exceeds 3 becomes equal
      // to 3.  Then all points fall within the square defined by
      // alpha and beta being between 0 and 3.  I calculate that that
      // square covers about 68% of the region of monotonicity.
      //
      // I have found another method, which I call the "wide" method,
      // which follows the edge of the region of monotonicity more
      // closely, leading to smoother interpolation.  I calculate that
      // using that method covers about 98% of the region of
      // monotonicity.

      for (size_t i = 0; i < n - 1; ++i) // n - 1 segments
      {
        switch (method)
        {
          case SQUARE:
          {
            if (alpha[i] > 3)
            {
              m_slopeTangents[i] = 3*m_slopeSecants[i];
            }
            if (beta[i] > 3)
            {
              m_slopeTangents[i + 1] = 3*m_slopeSecants[i];
            }
          }
          break;
          case CIRCLE:
          case WIDE:
          case FULL:
          {
            double r2 = alpha[i]*alpha[i] + beta[i]*beta[i];
            if (r2 > 9)
            {
              // r2 is large enough that interpolation results might not be
              // monotone.

              double rNew = 3;

              if (method == WIDE)
              {
                // This method was invented by Louis Strous.

                double t = (alpha[i] - beta[i])/(alpha[i] + beta[i]);

                // We want the fourth power of t, but pow(t, 4) may
                // not allow negative t.

                t *= t;         // square
                t *= t;         // square again

                t = 0.912 - t;

                if (t > 0.0)
                {
                  rNew = 3 + 3*sqrt(3 - 2*sqrt(2))*sqrt(t);
                }
              }
              else if (method == FULL)
              {
                // This method also invented by Louis Strous.

                // t is a measure for the angle with respect to the
                // alpha axis, as seen from the origin of the
                // (alpha,beta) plane.  In the first quadrant of the
                // (alpha, beta) plane, it varies between -1 on the
                // alpha axis and +1 on the beta axis.
                double t = (alpha[i] - beta[i])/(alpha[i] + beta[i]);

                // The square of the radius (distance from origin)
                // of the furthest (from the origin) edge of the
                // allowed region is then given by
                // r^2 = 18 (−t^4 + 4(sqrt(1-t^2) (1+t^2) + t^2) + 5)/(t^2 + 3)^2
                // (derived by LS)

                double tsq = t*t; // square it

                // We need to calculate sqrt(1-t^2).  |t| <= 1 so
                // 1-t^2 >= 0, but due to round-off errors it might be
                // negative (though very small).
                double s = 1 - tsq;
                if (s < 0)
                {               // neutralize round-off error
                  s = 0;
                }
                double num = 18*(-tsq*tsq + 4*(sqrt(s)*(1 + tsq) + tsq) + 5);
                double denom = (tsq + 3);
                denom *= denom; // >= 4

                rNew = sqrt(num/denom);
              }

              double f = rNew/sqrt(r2);
              m_slopeTangents[i] = f*alpha[i]*m_slopeSecants[i];
              m_slopeTangents[i + 1] = f*beta[i]*m_slopeSecants[i];
            }
          }
          break;
        }
      }
    }
  }
}

/// Construct a Point from its coordinates.
///
/// \param x is the x coordinate of the point.
///
/// \param y is the y coordinate of the point.
///
/// \returns a Point based on the coordinates.
MonotoneInterpolation::Point
MonotoneInterpolation::constructPoint(double x, double y)
{
  return Point(x, y);
}

/// Lesser-than operator for Points.
///
/// \param lhs is the left-hand Point to compare.
///
/// \param rhs is the right-hand Point to compare.
///
/// \returns true if the x coordinate of the left-hand point is less
/// than that of the right-hand point, and false otherwise.
bool
operator<(const MonotoneInterpolation::Point& lhs,
          const MonotoneInterpolation::Point& rhs)
{
  return lhs.m_x < rhs.m_x;
}

/// Return the interpolated y coordinate.
///
/// \param xTarget is the x coordinate for which the interpolated y
/// coordinate is desired.
///
/// \returns the interpolated y coordinate.
double
MonotoneInterpolation::interpolate(double xTarget) const
{
  // Take care of the trivial cases

  if (m_points.size() == 0)
    return 0.0;

  if (m_points.size() == 1)
    return m_points.front().m_y;

  // OK, we have at least two data points, so we need to do some
  // actual work

  // Locate the segment containing the requested x coordinate

  struct FindPointToTheRightOf {
    FindPointToTheRightOf(double value)
      : m_value(value)
    { }

    bool operator()(const Point& point) const
    {
      return point.m_x > m_value;
    }

    double m_value;
  };

  FindPointToTheRightOf findit(xTarget);
  auto it_leftPoint = std::find_if(m_points.begin(), m_points.end(), findit);

  // Now it_leftPoint (if not equal to m_points.end()) indicates the first
  // point that has an x coordinate greater than xTarget.

  if (it_leftPoint == m_points.begin())
  {
    // xTarget is to the left of (has lesser x coordinate than) the
    // first data point.  We treat it as if it is part of the first
    // segment.  No action needed here.
  }
  else if (it_leftPoint == m_points.end())
  {
    // xTarget is to the right of the last data point.  We treat it as
    // if it is part of the last segment.  Subtract 1 to get the last
    // point, and subtract 1 again to get the left-hand point of the
    // last segment.
    it_leftPoint -= 2;
  }
  else
  {
    // We actually want the last point that has an x coordinate less
    // than or equal to xTarget, so we move back by 1.
    --it_leftPoint;
  }

  // Now it_leftPoint indicates the left-hand point of the segment of
  // interest.  Find it_rightPoint for the right-hand point.

  auto it_rightPoint = it_leftPoint + 1;

  // And likewise for the left-hand and right-hand tangents.

  auto it_leftTangent = m_slopeTangents.begin() + (it_leftPoint - m_points.begin());
  auto it_rightTangent = it_leftTangent + 1;

  double h = it_rightPoint->m_x - it_leftPoint->m_x; // width of segment, assumed > 0
  double t = (xTarget - it_leftPoint->m_x)/h; // fraction of segment to the left of target

  // Calculate coefficients for cubic interpolation

  double h00 = (1 + 2*t)*(1 - t)*(1 - t);
  double h10 = t*(1 - t)*(1 - t);
  double h01 = t*t*(3 - 2*t);
  double h11 = t*t*(t - 1);

  // Now calculate the interpolated y value

  double yTarget
    = it_leftPoint->m_y*h00
    + h*(*it_leftTangent)*h10
    + it_rightPoint->m_y*h01
    + h*(*it_rightTangent)*h11;

  return yTarget;
}

/// A static function for monotonic interpolation.  This is convenient
/// when an interpolated value is desired for only a single value of
/// the x coordinate.
///
/// \param x is a vector of x coordinates of the zero or more data
/// points to interpolate between.
///
/// \param y is a vector of y coordinates of the zero or more data
/// points to interpolate between.
//
/// If x and y do not have the same number of elements, then the
/// excess elements from the longer of the two are ignored.
///
/// \param xTarget is the x coordinate for which the interpolated y
/// coordinate is desired.
///
/// \param method selects which method to use to enforce monotonicity.
///
/// \returns the interpolated y coordinate.
double
MonotoneInterpolation::interpolate(std::vector<double> x, std::vector<double> y,
                                   double xTarget,
                                   MonotoneMethodSelection method)
{
  MonotoneInterpolation mi(x, y, method);
  return mi.interpolate(xTarget);
}
