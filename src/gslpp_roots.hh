#ifndef INCLUDED_GSLPP_ROOTS_HH
# define INCLUDED_GSLPP_ROOTS_HH

# include "config.hh"
# if GSL_INCLUDE

#  include <gsl/gsl_roots.h>

double unwrap_for_gsl_function(double x, void* params);

/// A C++ wrapper around gsl_root_fsolver.
class Gsl_root_fsolver
{
public:
  // constructor, destructor

  /// Constructor.
  ///
  /// \param type is the type of the GSL root solver.
  ///
  /// \param f is the function-like thing (functor) to find the root for.
  ///
  /// \param x_lo is the lower bound of the range of the independent variable to
  /// search.
  ///
  /// \param x_hi is the upper bound of the range of the independent variable to
  /// search.
  Gsl_root_fsolver(const gsl_root_fsolver_type* type,
                   std::function<double(double)> f, double x_lo,
                   double x_hi);

  /// Destructor.
  virtual ~Gsl_root_fsolver();

  // const method

  /// Get the status of the GSL root solver.  0 means OK, non-0 means a problem.
  ///
  /// \returns the status.
  int get_status() const;

  // non-const methods

  /// Get the estimate for the root, the value of the independent variable for
  /// which the function returns the value 0.  If no root could be found within
  /// the specified range then a Not-a-Number value is returned.
  double get_root();

private:
  gsl_root_fsolver* d_solver;
  std::function<double(double)> d_functor;
  gsl_function d_gslf;
  int d_status = 0;
};

# endif
#endif
