// See gslpp_sort.hh for documentation

// configuration include

#include "config.hh"

// own includes

#include "gslpp_sort.hh"

#if HAVE_LIBGSL

// char

void
gsl_sort_index(size_t* p, const char* data, size_t stride, size_t n)
{
  gsl_sort_char_index(p, data, stride, n);
}

std::vector<size_t>
gsl_sort_index(const char* data, size_t stride, size_t n)
{
  std::vector<size_t> p(n);
  gsl_sort_char_index(p.data(), data, stride, n);
  return p;
}

std::vector<size_t>
gsl_sort_index(const std::vector<char> data)
{
  std::vector<size_t> p(data.size());
  gsl_sort_char_index(p.data(), data.data(), 1, p.size());
  return p;
}

// unsigned char

void
gsl_sort_index(size_t* p, const unsigned char* data, size_t stride, size_t n)
{
  gsl_sort_uchar_index(p, data, stride, n);
}

std::vector<size_t>
gsl_sort_index(const unsigned char* data, size_t stride, size_t n)
{
  std::vector<size_t> p(n);
  gsl_sort_uchar_index(p.data(), data, stride, p.size());
  return p;
}

std::vector<size_t>
gsl_sort_index(const std::vector<unsigned char> data)
{
  std::vector<size_t> p(data.size());
  gsl_sort_uchar_index(p.data(), data.data(), 1, p.size());
  return p;
}

// short

void
gsl_sort_index(size_t* p, const short* data, size_t stride, size_t n)
{
  gsl_sort_short_index(p, data, stride, n);
}

std::vector<size_t>
gsl_sort_index(const short* data, size_t stride, size_t n)
{
  std::vector<size_t> p(n);
  gsl_sort_short_index(p.data(), data, stride, p.size());
  return p;
}

std::vector<size_t>
gsl_sort_index(const std::vector<short> data)
{
  std::vector<size_t> p(data.size());
  gsl_sort_short_index(p.data(), data.data(), 1, p.size());
  return p;
}

// unsigned short

void
gsl_sort_index(size_t* p, const unsigned short* data, size_t stride, size_t n)
{
  gsl_sort_ushort_index(p, data, stride, n);
}

std::vector<size_t>
gsl_sort_index(const unsigned short* data, size_t stride, size_t n)
{
  std::vector<size_t> p(n);
  gsl_sort_ushort_index(p.data(), data, stride, p.size());
  return p;
}

std::vector<size_t>
gsl_sort_index(const std::vector<unsigned short> data)
{
  std::vector<size_t> p(data.size());
  gsl_sort_ushort_index(p.data(), data.data(), 1, p.size());
  return p;
}

// int

void
gsl_sort_index(size_t* p, const int* data, size_t stride, size_t n)
{
  gsl_sort_int_index(p, data, stride, n);
}

std::vector<size_t>
gsl_sort_index(const int* data, size_t stride, size_t n)
{
  std::vector<size_t> p(n);
  gsl_sort_int_index(p.data(), data, stride, p.size());
  return p;
}

std::vector<size_t>
gsl_sort_index(const std::vector<int> data)
{
  std::vector<size_t> p(data.size());
  gsl_sort_int_index(p.data(), data.data(), 1, p.size());
  return p;
}

// unsigned int

void
gsl_sort_index(size_t* p, const unsigned int* data, size_t stride, size_t n)
{
  gsl_sort_uint_index(p, data, stride, n);
}

std::vector<size_t>
gsl_sort_index(const unsigned int* data, size_t stride, size_t n)
{
  std::vector<size_t> p(n);
  gsl_sort_uint_index(p.data(), data, stride, p.size());
  return p;
}

std::vector<size_t>
gsl_sort_index(const std::vector<unsigned int> data)
{
  std::vector<size_t> p(data.size());
  gsl_sort_uint_index(p.data(), data.data(), 1, p.size());
  return p;
}

// long

void
gsl_sort_index(size_t* p, const long* data, size_t stride, size_t n)
{
  gsl_sort_long_index(p, data, stride, n);
}

std::vector<size_t>
gsl_sort_index(const long* data, size_t stride, size_t n)
{
  std::vector<size_t> p(n);
  gsl_sort_long_index(p.data(), data, stride, p.size());
  return p;
}

std::vector<size_t>
gsl_sort_index(const std::vector<long> data)
{
  std::vector<size_t> p(data.size());
  gsl_sort_long_index(p.data(), data.data(), 1, p.size());
  return p;
}

// unsigned long

void
gsl_sort_index(size_t* p, const unsigned long* data, size_t stride, size_t n)
{
  gsl_sort_ulong_index(p, data, stride, n);
}

std::vector<size_t>
gsl_sort_index(const unsigned long* data, size_t stride, size_t n)
{
  std::vector<size_t> p(n);
  gsl_sort_ulong_index(p.data(), data, stride, p.size());
  return p;
}

std::vector<size_t>
gsl_sort_index(const std::vector<unsigned long> data)
{
  std::vector<size_t> p(data.size());
  gsl_sort_ulong_index(p.data(), data.data(), 1, p.size());
  return p;
}

// float

void
gsl_sort_index(size_t* p, const float* data, size_t stride, size_t n)
{
  gsl_sort_float_index(p, data, stride, n);
}

std::vector<size_t>
gsl_sort_index(const float* data, size_t stride, size_t n)
{
  std::vector<size_t> p(n);
  gsl_sort_float_index(p.data(), data, stride, p.size());
  return p;
}

std::vector<size_t>
gsl_sort_index(const std::vector<float> data)
{
  std::vector<size_t> p(data.size());
  gsl_sort_float_index(p.data(), data.data(), 1, p.size());
  return p;
}

// double

// void gsl_sort_index(size_t* p, const double* data, size_t stride, size_t n)
// is already provided by GSL

std::vector<size_t>
gsl_sort_index(const double* data, size_t stride, size_t n)
{
  std::vector<size_t> p(n);
  gsl_sort_index(p.data(), data, stride, p.size());
  return p;
}

std::vector<size_t>
gsl_sort_index(const std::vector<double> data)
{
  std::vector<size_t> p(data.size());
  gsl_sort_index(p.data(), data.data(), 1, p.size());
  return p;
}

// long double

void
gsl_sort_index(size_t* p, const long double* data, size_t stride, size_t n)
{
  gsl_sort_long_double_index(p, data, stride, n);
}

std::vector<size_t>
gsl_sort_index(const long double* data, size_t stride, size_t n)
{
  std::vector<size_t> p(n);
  gsl_sort_long_double_index(p.data(), data, stride, p.size());
  return p;
}

std::vector<size_t>
gsl_sort_index(const std::vector<long double> data)
{
  std::vector<size_t> p(data.size());
  gsl_sort_long_double_index(p.data(), data.data(), 1, p.size());
  return p;
}

#endif
