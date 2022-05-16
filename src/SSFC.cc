#include <cassert>
#include <cstdint>
#include <valarray>
#include <numeric>
#include "SSFC.hh"

typedef std::valarray<double> Vd;

SSFC::SSFC()
  : m_bits(0), m_level(3), m_have_polar(false), m_have_bits(true)
{
}

SSFC::SSFC(double latitude_rad, double longitude_rad, double precision_rad)
{
  set_polar(latitude_rad, longitude_rad, precision_rad);
}

SSFC::SSFC(double latitude_rad, double longitude_rad, int32_t level)
{
  set_polar(latitude_rad, longitude_rad, level);
}

SSFC::SSFC(double ssfc, uint32_t level)
{
  set_ssfc(ssfc, level);
}

void
SSFC::set_polar(double latitude_rad, double longitude_rad, double precision_rad)
{
  m_latitude_rad = latitude_rad;
  m_longitude_rad = longitude_rad;
  m_precision_rad = precision_rad;
  m_level = 0;
  m_have_polar = true;
  m_have_bits = false;
}

void
SSFC::set_polar(double latitude_rad, double longitude_rad, int32_t level)
{
  m_latitude_rad = latitude_rad;
  m_longitude_rad = longitude_rad;
  m_precision_rad = 0;
  m_level = level;
  m_have_polar = true;
  m_have_bits = false;
}

void
SSFC::set_ssfc(int32_t ssfc, uint32_t level)
{
  set_ssfc(static_cast<uint64_t>(ssfc), level);
}

void
SSFC::set_ssfc(int64_t ssfc, uint32_t level)
{
  set_ssfc(static_cast<uint64_t>(ssfc), level);
}

void
SSFC::set_ssfc(uint64_t ssfc, uint32_t level)
{
  if (level < 3) {
    level = 3;
  }
  else if (level > 64) {
    level = 64;
  }
  set_aligned_ssfc(ssfc << (8*sizeof(ssfc) - level), level);
}

void
SSFC::set_aligned_ssfc(uint64_t bits, uint32_t level)
{
  if (level < 3) {
    level = 3;
  }
  else if (level > 64) {
    level = 64;
  }
  m_level = level;
  m_have_polar = false;
  m_have_bits = true;
  m_bits = bits;
}

void
SSFC::set_ssfc(double ssfc, uint32_t level)
{
  double s = fmod(ssfc, 1);
  s *= pow(2, 64);
  set_aligned_ssfc(static_cast<uint64_t>(s), level);
}

uint64_t
SSFC::get_bits() const
{
  if (!m_have_bits) {
    calculate_bits();
  }
  return m_bits;
}

uint32_t
SSFC::get_level() const
{
  if (!m_have_bits) {
    calculate_bits();
  }
  return m_level;
}

double
SSFC::get_latitude_rad() const
{
  if (!m_have_polar) {
    calculate_polar();
  }
  return m_latitude_rad;
}

double
SSFC::get_longitude_rad() const
{
  if (!m_have_polar) {
    calculate_polar();
  }
  return m_longitude_rad;
}

void
SSFC::get_polar_rad(double& latitude_rad, double& longitude_rad) const
{
  if (!m_have_polar) {
    calculate_polar();
  }
  latitude_rad = m_latitude_rad;
  longitude_rad = m_longitude_rad;
}

void
SSFC::get_polar_rad(double& latitude_rad, double& longitude_rad,
                    double& precision_rad) const
{
  get_polar_rad(latitude_rad, longitude_rad);
  precision_rad = m_precision_rad;
}

double
SSFC::get_precision_rad() const
{
  if (!m_have_polar) {
    calculate_polar();
  }
  return m_precision_rad;
}

void
SSFC::octantPoints(int8_t bits, Vd& A, Vd& B, Vd& C)
{
  static double AOctants[][3] = {
    {  0, 0, +1 },
    { +1, 0,  0 },
    {  0, 0, -1 },
    { -1, 0,  0 },
    {  0, 0, +1 },
    { -1, 0,  0 },
    {  0, 0, -1 },
    {  1, 0,  0 }
  };
  static double BOctants[][3] = {
    { 0, +1, 0 },
    { 0, +1, 0 },
    { 0, +1, 0 },
    { 0, +1, 0 },
    { 0, -1, 0 },
    { 0, -1, 0 },
    { 0, -1, 0 },
    { 0, -1, 0 }
  };
  static double COctants[][3] = {
    { +1, 0, 0 },
    { 0, 0, -1 },
    { -1, 0, 0 },
    { 0, 0, +1 },
    { -1, 0, 0 },
    { 0, 0, -1 },
    { +1, 0, 0 },
    { 0, 0, +1 }
  };

  A = Vd(AOctants[bits], 3);
  B = Vd(BOctants[bits], 3);
  C = Vd(COctants[bits], 3);
}

void
SSFC::normalizePoint(Vd& X)
{
  // TODO: make this work with std::accumulate
  double length2 = 0.0;
  double* data = &X[0];
  size_t n = X.size();
  for (size_t i = 0; i < n; ++i) {
    length2 += data[i]*data[i];
  }
  if (length2) {
    X /= sqrt(length2);
  }
}

double
SSFC::determinant(const Vd& A, const Vd& B, const Vd& C)
{
  return
      A[0]*(B[1]*C[2] - B[2]*C[1])
    + A[1]*(B[2]*C[0] - B[0]*C[2])
    + A[2]*(B[0]*C[1] - B[1]*C[0]);
}

int32_t
SSFC::sgn(double value)
{
  return (value > 0) - (value < 0);
}

// calculate tan(d/2)^2 where d is the angular distance between A and B
double
SSFC::distance_measure(const Vd& A, const Vd& B)
{
  Vd S = A + B;
  Vd D = A - B;
  // TODO: make this work with std::accumulate
  double numerator = 0.0;
  double denominator = 0.0;
  for (size_t i = 0; i < D.size(); ++i) {
    numerator += D[i]*D[i];
    denominator += S[i]*S[i];
  }
  return denominator? numerator/denominator: 0;
}

void
SSFC::calculate_bits() const
{
  if (!m_have_bits) {
    assert(m_have_polar);

    // calculate the cartesian coordinates
    double slat, clat, slon, clon;
    sincos(m_latitude_rad, &slat, &clat);
    sincos(m_longitude_rad, &slon, &clon);

    double c[] = {clon*clat, slon*clat, slat};
    Vd P(c, 3);

    int desired_level;
    double desired_precision;
    double precision;
    if (m_precision_rad) {
      // we express the desired precision in terms of tan²(½d) of the
      // desired angular distance d.  tan²(½d) = ∑(a-b)²/∑(a+b)²
      desired_precision = tan(0.5*m_precision_rad);
      desired_precision *= desired_precision;
      desired_level = -1;
    } else {
      desired_precision = 0;
      precision = 1;
      desired_level = m_level;
    }

    // determine the first three bits: the octant
    int index = ((P[0] >= 0) << 2) | ((P[1] >= 0) << 1) | (P[2] >= 0);
    static int8_t octants[] = {
      5, 4, 2, 3, 6, 7, 1, 0
    };
    int8_t octant = octants[index];
    m_bits = octant;
    m_level = 3;

    Vd A(3), B(3), C(3);
    octantPoints(octant, A, B, C);
    if (m_precision_rad) {
      precision = distance_measure(A, C);
    }

    while (precision > desired_precision
           && (desired_level == -1 || m_level < desired_level)) {
      Vd D = A + C;
      normalizePoint(D);
      int s = sgn(determinant(P, A, B))
        + sgn(determinant(P, B, D))
        + sgn(determinant(P, D, A));
      m_bits <<= 1;
      ++m_level;
      if (abs(s) == 3) {
        // all three determinants have the same sign, so P is inside
        // triangle defined by A, B, and C.  Next bit is a zero
        m_bits |= 0;
        C = B;
      } else {
        // P is outside of triangle ABC.  Next bit is a one
        m_bits |= 1;
        A = B;
      }
      B = D;
      if (m_precision_rad) {
        precision = distance_measure(A, C);
      }
    }

    if (!m_precision_rad) {
      precision = distance_measure(A, C);
    }
    m_precision_rad = 2*atan(sqrt(precision));
    m_have_bits = true;
  }
}

void
SSFC::calculate_polar() const
{
  if (!m_have_polar) {
    assert(m_have_bits);
    assert(m_level >= 3);
    assert(sizeof(m_bits) == 8);

    const uint64_t highest_bit = ((uint64_t) 1) << (8*sizeof(m_bits) - 1);

    uint64_t bits = m_bits;

    // extract the first three bits: the octant
    uint8_t octant = (bits >> (8*sizeof(m_bits) - 3));
    // shift the first three bits out
    bits <<= 3;

    // calculate the vertices of the corresponding spherical triangle
    Vd A(3), B(3), C(3);
    octantPoints(octant, A, B, C);

    Vd D(3);
    for (int level = 3; level < m_level; ++level) {
      D = A + C;
      normalizePoint(D);
      if (bits & highest_bit) {
        A = B;
      } else {
        C = B;
      }
      B = D;
      bits <<= 1;               // shift out the highest bit
    }
    D = A + B + C;
    normalizePoint(D);

    m_precision_rad = atan(sqrt(distance_measure(A, C)));
    m_latitude_rad = asin(D[2]);
    m_longitude_rad = atan2(D[1], D[0]);

    m_have_polar = true;
  }
}
