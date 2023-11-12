#ifndef INCLUDED_GSLPP_ROOTS_HH
# define INCLUDED_GSLPP_ROOTS_HH

# include "config.h"
# if HAVE_LIBGSL

#  include <gsl/gsl_roots.h>

double unwrap_for_gsl_function(double x, void* params);

class Gsl_root_fsolver
{
public:
  // constructor, destructor

  Gsl_root_fsolver(const gsl_root_fsolver_type* type,
                   std::function<double(double)> f, double x_lo,
                   double x_hi);

  virtual ~Gsl_root_fsolver();

  // non-const methods

  double get_root();

private:
  gsl_root_fsolver* d_solver;
  std::function<double(double)> d_functor;
  gsl_function d_gslf;
  int d_status = 0;
};

# endif
#endif
