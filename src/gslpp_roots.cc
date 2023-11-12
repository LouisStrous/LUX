#include "config.h"
#if HAVE_LIBGSL
# include <functional>
# include "gslpp_roots.hh"
# include "gsl/gsl_errno.h"         // for GSL_CONTINUE

#include <iostream>

double
unwrap_for_gsl_function(double x, void* params)
{
  auto fp = static_cast<std::function<double(double)>*>(params);
  auto z = (*fp)(x);
  return z;
}

Gsl_root_fsolver::Gsl_root_fsolver(const gsl_root_fsolver_type* type,
                                   std::function<double(double)> f,
                                   double x_lo, double x_hi)
  : d_solver(gsl_root_fsolver_alloc(type)),
    d_functor(f)
{
  d_gslf.function = &unwrap_for_gsl_function;
  d_gslf.params = (void*) &d_functor;
  gsl_root_fsolver_set(d_solver, &d_gslf, x_lo, x_hi);
}

Gsl_root_fsolver::~Gsl_root_fsolver()
{
  gsl_root_fsolver_free(d_solver);
}

double
Gsl_root_fsolver::get_root()
{
  int status;
  size_t iterations = 0;

  do
  {
    ++iterations;
    status = gsl_root_fsolver_iterate(d_solver);
    if (!status) {              // no problem during iteration
      double x_lo = gsl_root_fsolver_x_lower(d_solver);
      double x_hi = gsl_root_fsolver_x_upper(d_solver);
      status = gsl_root_test_interval(x_lo, x_hi, 0, 0.001);
    }
  } while (status == GSL_CONTINUE && iterations < 100);

  return gsl_root_fsolver_root(d_solver);
}

#endif
