#ifndef SSFC_HH_
#define SSFC_HH_

#include <valarray>
#include <cstdint>

/// A class for Sierpiński Surface Filling Coordinates.  See
/// https://www.ssfc.quae.nl/en/index.html.
class SSFC
{
public:
  SSFC();
  SSFC(double latitude_rad, double longitude_rad, double precision_rad);
  SSFC(double latitude_rad, double longitude_rad, int32_t level);
  SSFC(double ssfc, uint32_t level);
  void set_polar(double latitude_rad, double longitude_rad, double precision_rad);
  void set_polar(double latitude_rad, double longitude_rad, int32_t level);
  void set_ssfc(double ssfc, uint32_t level);
  void set_ssfc(int32_t ssfc, uint32_t level);
  void set_ssfc(int64_t ssfc, uint32_t level);
  void set_ssfc(uint64_t ssfc, uint32_t level);
  void set_aligned_ssfc(uint64_t ssfc, uint32_t level);
  uint64_t get_bits() const;
  uint32_t get_level() const;
  double get_latitude_rad() const;
  double get_longitude_rad() const;
  double get_precision_rad() const;
  void get_polar_rad(double& latitude_rad, double& longitude_rad) const;
  void get_polar_rad(double& latitude_rad, double& longitude_rad,
                     double& precision_rad) const;
private:
  void calculate_bits() const;
  void calculate_polar() const;

  typedef std::valarray<double> Vd;

  static void octantPoints(int8_t bits, Vd& A, Vd& B, Vd& C);
  static double sum2(const double& accumulator, const double& argument);
  static void normalizePoint(Vd& X);
  static double determinant(const Vd& A, const Vd& B, const Vd& C);
  static int32_t sgn(double value);
  static double distance_measure(const Vd& A, const Vd& B);

  mutable uint64_t m_bits;
  mutable uint8_t m_level;
  mutable double m_latitude_rad;
  mutable double m_longitude_rad;
  mutable double m_precision_rad;
  mutable bool m_have_polar;
  mutable bool m_have_bits;
};

#endif
