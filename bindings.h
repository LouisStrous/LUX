#ifndef HAVE_BINDINGS_H
#define HAVE_BINDINGS_H

/* A number of binding functions that bind a particular C function
   template to ANA and allow corresponding C functions to be quickly
   mapped to corresponding ANA function/subroutines

   The naming scheme is as follows:

   ana_<spec>_(s|f)_[<special>_]
   spec: ((i|o|r)<type>[<type>]*)+
   type: <typespec>
   typespec:
     l[<count1>] = int value(s)
     d[<count1>] = double value(s)
     b[<count2>] = double vector
     c[<count2>] = double 2-dimensional matrix
     0 = double zero

   these lowercase typespec letters indicate that the C function gets
   called multiple times until all input data is exhausted.  The
   corresponding uppercase typespec letters indicate that only a
   single value (or set of values) is used from this argument.

   count1 indicates how many data values are handled during a single
   call to the C function, one value per C function parameter.  For
   example, d3 means that for a single call to the C function three
   double values are read from a single ANA argument and are applied
   to three consecutive C function parameters.
   
   count2 indicates how many data values are handled during a single
   call to the C function in the single parameter.  For a matrix, both
   dimensions are concatenated. For example, b3 means that for a
   single call to the C function 3 double values are read from a
   single ANA argument and are passed to the C function in a single
   pointer parameter.  c33 means that for a single call to the C
   function 3 by 3 values (9 values in total) are read from a single
   ANA argument and are passed to the C function in a single pointer
   parameter.

   Some examples, for a C function f() corresponding to ANA function
   or subroutine F:

   ana_idod_s  - void f(double, double *) - "i>D*;o>D*" - F,x,y
   ana_idod_f  - void f(double, double *) - "i>D*;rD*" - y = F(x) ; x is scalar or array
   ana_iDoD_f  - void f(double, double *) - "i>D1;rD1" - y = F(x) ; x is scalar or 1-dimensional 1-element array
   ana_idrd_f  - double f(double)         - "i>D*;rD*" - y = F(x) ; x is scalar or array
   ana_id3rd_f - double f(double, double, double) - "i>D*;i>D*;i>D*;rD*" - q = F(x,y,z) ; x, y, z are scalars or arrays with the same dimensions
   ana_ib3rd_f - double f(double *)       - "i>D3*;rD-3*" - y = F(x) ; 1st dimension of x has 3 elements
   ana_ic33od_s - void f(double (*)[3], double *) - "i>D3,3*;oD-3-3*" - F,x,y ; x is array with 1st and 2nd dimension equal to 3
*/

int ana_iLb3od3rl_f_(int narg, int ps[], int (*f)(int, double *, double *, double *, double *));
int ana_iLd3ob3rl_f_(int narg, int ps[], int (*f)(int, double, double, double, double *));
int ana_ib3DDod3rl_f_(int narg, int ps[], int (*f)(double, double, double *, double *, double *, double *));
int ana_ib3rb2_f_(int narg, int ps[], void (*f)(double *, double *, double *));
int ana_ib3oc33_f_(int narg, int ps[], void (*f)(double [3], double [3][3]));
int ana_ib3od3_f_(int narg, int ps[], void (*f)(double *, double *, double *, double *));
int ana_ib3odb3_s_(int narg, int ps[], void (*f)(double *, double *, double *));
int ana_ic23c23ob2_f_(int narg, int ps[], void (*f)(double [2][3], double [2][3], double [2]));
int ana_ic23c23oc23_f_(int narg, int ps[], void (*f)(double [2][3], double [2][3], double [2][3]));
int ana_ic23odd_s_(int narg, int ps[], void (*f)(double [2][3], double *, double *));
int ana_ic23odddddd_s_(int narg, int ps[], void (*f)(double [2][3], double *, double *, double *, double *, double *, double *));
int ana_ic23oddddddrl_s_(int narg, int ps[], int (*f)(double [2][3], double *, double *, double *, double *, double *, double *));
int ana_ic33b3ob3_f_(int narg, int ps[], void (*f)(double [3][3], double [3], double [3]));
int ana_ic33dc33oc33_f_(int narg, int ps[], void (*f)(double (*)[3], double, double (*)[3], double (*)[3]));
int ana_ic33ob3_f_(int narg, int ps[], void (*f)(double [3][3], double [3]));
int ana_ic33oc33_f_(int narg, int ps[], void (*f)(double [3][3], double [3][3]));
int ana_ic33odd_s_(int narg, int ps[], void (*f)(double (*)[3], double *, double *));
int ana_id000ddddoc33_f_(int narg, int ps[], void (*f)(double, double, double, double, double, double, double, double, double (*)[3]));
int ana_id0d0ddoc33_mod_f_(int narg, int ps[], void (*f)(double, double, double, double, double, double, double (*)[3]));
int ana_id00DDDrd_f_(int narg, int ps[], double (*f)(double, double, double, double, double, double));
int ana_id0Loc23rl_f_(int narg, int ps[], int (*f)(double, double, int, double (*)[3]));
int ana_id0c23c23rl_s_(int narg, int ps[], int (*f)(double, double, double (*)[3], double (*)[3]));
int ana_id0c33oc33_f_(int narg, int ps[], void (*f)(double, double, double (*)[3], double (*)[3]));
int ana_id0d0oddd_s_(int narg, int ps[], void (*f)(double, double, double, double, double *, double *, double *));
int ana_id0ddd_s_(int narg, int ps[], void (*f)(double, double, double *, double *, double *));
int ana_id0ddrd_f_(int narg, int ps[], double (*f)(double, double, double, double));
int ana_id0doddrl_jd_f_(int narg, int ps[], int (*f)(double, double, double, double *, double *));
int ana_id0oc33_f_(int narg, int ps[], void (*f)(double, double, double (*)[3]));
int ana_id0oC33c33c33_s_(int narg, int ps[], void (*f)(double, double, double (*)[3], double (*)[3], double (*)[3]));
int ana_id0odddd_s_(int narg, int ps[], void (*f)(double, double, double *, double *, double *, double *));
int ana_id0odddddddddddddddd_s_(int narg, int ps[], void (*f)(double, double, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *));
int ana_id0oddrl_jd_f_(int narg, int ps[], int (*f)(double, double, double *, double *));
int ana_id0odd_s_(int narg, int ps[], void (*f)(double, double, double *, double *));
int ana_id0ollldrl_cal_f_(int narg, int ps[], int (*f)(double, double, int *, int *, int *, double *));
int ana_id0rd_f_(int narg, int ps[], double (*f)(double, double));
int ana_id3DDob3rl_f_(int narg, int ps[], int (*f)(double, double, double, double, double, double *));
int ana_id3rd_f_(int narg, int ps[], double (*f)(double *));
int ana_iddd0odd_s_(int narg, int ps[], void (*f)(double, double, double, double, double *, double *));
int ana_iddd0odddd_s_(int narg, int ps[], void (*f)(double, double, double, double, double *, double *, double *, double *));
int ana_iddddc33rd_mod_f_(int narg, int ps[], double (*f)(double, double, double, double, double (*)[3]));
int ana_iddddddoc23_f_(int narg, int ps[], void (*f)(double, double, double, double, double, double, double [2][3]));
int ana_iddddddoc23rl_f_(int narg, int ps[], int (*f)(double, double, double, double, double, double, double [2][3]));
int ana_iddddddd0d0oddddddrl_s_(int narg, int ps[], int (*f)(double, double, double, double, double, double, double, double, double, double, double *, double *, double *, double *, double *, double *));
int ana_iddddddodddddd_s_(int narg, int ps[], void (*f)(double, double, double, double, double, double, double *, double *, double *, double *, double *, double *));
int ana_iddddoc33_f_(int narg, int ps[], void (*f)(double, double, double, double, double (*)[3]));
int ana_iddddodd_s_(int narg, int ps[], void (*f)(double, double, double, double, double *, double *));
int ana_iddddrd_f_(int narg, int ps[], double (*f)(double, double, double, double));
int ana_iddddrd_mod_f_(int narg, int ps[], double (*f)(double, double, double, double));
int ana_idddob3_f_(int narg, int ps[], void (*f)(double, double, double, double [3]));
int ana_idddoc33_f_(int narg, int ps[], void (*f)(double, double, double, double (*)[3]));
int ana_iddob3_f_(int narg, int ps[], void (*f)(double, double, double [3]));
int ana_iddrd_f_(int narg, int ps[], double (*f)(double, double));
int ana_idrd_f_(int narg, int ps[], double (*f)(double));
int ana_ilob2rl_s_(int narg, int ps[], int (*f)(int, double *, double *));
int ana_oC33_f_(int narg, int ps[], void (*f)(double (*)[3]));
int ana_oC33B3_s_(int narg, int ps[], void (*f)(double (*)[3], double *));
int ana_oddd_combine_f_(int narg, int ps[], void (*f)(double *, double *, double *));

int register_ana_f(int (*f)(int, int []), char *name, int min_arg,
                   int max_arg, char *spec);
int register_ana_s(int (*f)(int, int []), char *name, int min_arg,
                   int max_arg, char *spec);

#define REGISTER(func, fs, name, minarg, maxarg, fsspec)        \
void register_ana_ ## func ## _ ## fs (void) __attribute__((constructor)); \
void register_ana_ ## func ## _ ## fs (void) { \
  register_ana_ ## fs( ana_ ## func, #name, minarg, maxarg, fsspec );    \
}

#define BIND(func, type, fs, name, minarg, maxarg, fsspec)      \
int ana_ ## func(int narg, int ps[]) { \
 int result = ana_ ## type ## _ ## fs ## _(narg, ps, func); \
 if (result < 0) anaerror("Error in " #name, 0); \
 return result; \
} \
REGISTER(func, fs, name, minarg, maxarg, fsspec)

#endif
