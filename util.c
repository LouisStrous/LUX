#include <float.h> /* for DBL_EPSILON(9) FLT_EPSILON(8) */
#include <math.h> /* for fabsf(10) fabs(10) fabsl(10) */
#include "util.h"

#define EPS (2)

int approxeq(const double v1, const double v2)
{
  const double d = fabs(v1 - v2);
  return d <= EPS*DBL_EPSILON*fabs(v1) || d <= EPS*DBL_EPSILON*fabs(v2);
}

int essenteq(const double v1, const double v2)
{
  const double d = fabs(v1 - v2);
  return d <= EPS*DBL_EPSILON*fabs(v1) && d <= EPS*DBL_EPSILON*fabs(v2);
}

int definitelt(const double v1, const double v2)
{
  const double d = v1 - v2;
  return d < -EPS*DBL_EPSILON*fabs(v1) && d < -EPS*DBL_EPSILON*fabs(v2);
}

int definitegt(const double v1, const double v2)
{
  const double d = v1 - v2;
  return d > EPS*DBL_EPSILON*fabs(v1) && d > EPS*DBL_EPSILON*fabs(v2);
}

int approxeq_f(const float v1, const float v2)
{
  const double d = fabsf(v1 - v2);
  return d <= EPS*FLT_EPSILON*fabsf(v1) || d <= EPS*FLT_EPSILON*fabsf(v2);
}

int essenteq_f(const float v1, const float v2)
{
  const double d = fabsf(v1 - v2);
  return d <= EPS*FLT_EPSILON*fabsf(v1) && d <= EPS*FLT_EPSILON*fabsf(v2);
}

int definitelt_f(const float v1, const float v2)
{
  const double d = v1 - v2;
  return d < -EPS*FLT_EPSILON*fabsf(v1) && d < -EPS*FLT_EPSILON*fabsf(v2);
}

int definitegt_f(const float v1, const float v2)
{
  const double d = v1 - v2;
  return d > EPS*FLT_EPSILON*fabsf(v1) && d > EPS*FLT_EPSILON*fabsf(v2);
}

int approxeq_l(const long double v1, const long double v2)
{
  const double d = fabsl(v1 - v2);
  return d <= EPS*LDBL_EPSILON*fabsl(v1) || d <= EPS*LDBL_EPSILON*fabsl(v2);
}

int essenteq_l(const long double v1, const long double v2)
{
  const double d = fabsl(v1 - v2);
  return d <= EPS*LDBL_EPSILON*fabsl(v1) && d <= EPS*LDBL_EPSILON*fabsl(v2);
}

int definitelt_l(const long double v1, const long double v2)
{
  const double d = v1 - v2;
  return d < -EPS*LDBL_EPSILON*fabsl(v1) && d < -EPS*LDBL_EPSILON*fabsl(v2);
}

int definitegt_l(const long double v1, const long double v2)
{
  const double d = v1 - v2;
  return d > EPS*LDBL_EPSILON*fabsl(v1) && d > EPS*LDBL_EPSILON*fabsl(v2);
}

#include "unittest.h"
int test_ft_comparison(void)
{
  struct test {
    double v1;
    double v2;
    int lt;
    int eq;
    int aeq;
    int eeq;
    int gt;
  } tests[] = {
    { 1, 2, 1, 0, 0, 0, 0 },
    { 2, 1, 0, 0, 0, 0, 1 },
    { 1, 1, 0, 1, 1, 1, 0 },
    { 1, 1 + DBL_EPSILON, 0, 0, 1, 1, 0 },
  };
  int i;
  int bad = 0;

  for (i = 0; i < sizeof(tests)/sizeof(struct test); i++) {
    int result;

    result = definitelt(tests[i].v1, tests[i].v2);
    bad += assertEqualInts(tests[i].lt, result);
    result = tests[i].v1 == tests[i].v2;
    bad += assertEqualInts(tests[i].eq, result);
    result = approxeq(tests[i].v1, tests[i].v2);
    bad += assertEqualInts(tests[i].aeq, result);
    result = essenteq(tests[i].v1, tests[i].v2);
    bad += assertEqualInts(tests[i].eeq, result);
    result = definitegt(tests[i].v1, tests[i].v2);
    bad += assertEqualInts(tests[i].gt, result);
  }

  return bad;
}
