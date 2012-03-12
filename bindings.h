#ifndef HAVE_BINDINGS_H
#define HAVE_BINDINGS_H

/* A number of binding functions that bind a particular C function
   template to ANA and allow corresponding C functions to be quickly
   mapped to corresponding ANA function/subroutines
*/

int ana_d_dT4_iDaDarDq_0011_2_f_(int narg, int ps[], double (*f)(double, double, double, double));
int ana_d_dT4_iDaT4rDq_0T3_4_f_(int narg, int ps[], double (*f)(double, double, double, double));
int ana_d_dT4_iLaDaDarDq_0z12_f_(int narg, int ps[], double (*f)(double, double, double, double));
int ana_d_dT4dp3_iDaDaDp3p3arDq_00112_3_f_(int narg, int ps[], double (*f)(double, double, double, double, double (*)[3]));
int ana_d_dT6_iLaDaD1T3rDq_0z1T4_5_f_(int narg, int ps[], double (*f)(double, double, double, double, double, double));
int ana_d_d_iDarDq_0_1_f_(int narg, int ps[], double (*f)(double));
int ana_d_dd_iDarDq_0z_1_f_(int narg, int ps[], double (*f)(double, double));
int ana_d_dd_iLarDq_0z_1_f_(int narg, int ps[], double (*f)(double, double));
int ana_d_dp3d_iD33aDmmarDmmq_01_2_f_(int narg, int ps[], double (*f)(double (*)[3], double));
int ana_d_dp_iD3arDm3q_0_1_f_(int narg, int ps[], double (*f)(double *));
int ana_d_dpdp_iD3aD3qrDm3q_01_2_f_(int narg, int ps[], double (*f)(double *, double *));
int ana_d_sd_iDaLarDxq_000_2_f_(int narg, int ps[], double (*f)(double *, size_t count, size_t stride));
int ana_i_dT10dpT6_iDaT8oDqT6_0T6z7z8T13_s_(int narg, int ps[], int (*f)(double, double, double, double, double, double, double, double, double, double, double *, double *, double *, double *, double *, double *));
int ana_i_dT3dpdp_iDaDarDq_0z122_f_(int narg, int ps[], int (*f)(double, double, double, double *, double *));
int ana_i_dT5dp_iD3aD1D1rD3q_120003_f_(int narg, int ps[], int (*f)(double, double, double, double, double, double *));
int ana_i_dT6dp3_iDaT6oDp2p3q_0T6_f_(int narg, int ps[], int (*f)(double, double, double, double, double, double, double [2][3]));
int ana_i_dddp3dp3_iLaoDp2p3qT2_0z12_s_(int narg, int ps[], int (*f)(double, double, double (*)[3], double (*)[3]));
int ana_i_dddpdp_iDarDq_0z11_f_(int narg, int ps[], int (*f)(double, double, double *, double *));
int ana_i_ddidp3_iDaL1rDp3p2q_0z12_f_(int narg, int ps[], int (*f)(double, double, int, double (*)[3]));
int ana_i_dp3dpT6_iD23aoDm2m3aDcT5_0T6_s_(int narg, int ps[], int (*f)(double [2][3], double *, double *, double *, double *, double *, double *));
int ana_i_dpiT3dp_iDaiLiLrDq_00T3_f_(int narg, int ps[], int (*f)(double *, int, int, int, double *));
int ana_i_dpiidp_iDaLrDq_00T2_f_(int narg, int ps[], int (*f)(double *, int, int, double *));
int ana_i_idT3dp_iL1D3arDcq_10002_f_(int narg, int ps[], int (*f)(int, double, double, double, double *));
int ana_i_idpT4_iL1D3arDcq_0T222_f_(int narg, int ps[], int (*f)(int, double *, double *, double *, double *));
int ana_i_idpdp_iLarDp2q_011_f_(int narg, int ps[], int (*f)(int, double *, double *));
int ana_i_sd_iDaLa_000_s_(int narg, int ps[], int (*f)(double *, size_t, size_t));
int ana_i_sd_iDaLarDq_000_f_(int narg, int ps[], int (*f)(double *, size_t, size_t));
int ana_i_sdddsd_iDaLDDrDq_000T333_f_(int narg, int ps[], int (*f)(double *, size_t, size_t, double, double, double *, size_t, size_t));
int ana_id0ddd_s_(int narg, int ps[], void (*f)(double, double, double *, double *, double *));
int ana_iddipT3dp_iLarDp3q_0z1111_f_(int narg, int ps[], int (*f)(double, double, int *, int *, int *, double *));
int ana_v_dT3d3_iD3DcqrDcq_0T2_f_(int narg, int ps[], void (*f)(double *, double *, double *));
int ana_v_dT3d3_iD3aD1D1rD3q_120333_f_(int narg, int ps[], int (*f)(double, double, double *, double *, double *, double *));
int ana_v_dT3dp3_iDT3rDp3p3_0T3_f_(int narg, int ps[], void (*f)(double, double, double, double (*)[3]));
int ana_v_dT3dp3_iDaT3rDp3p3q_0T3_f_(int narg, int ps[], void (*f)(double, double, double, double (*)[3]));
int ana_v_dT3dp_iDaT3rDp3q_0T3_f_(int narg, int ps[], void (*f)(double, double, double, double [3]));
int ana_v_dT4dp3_iDaT3rDp3p3q_0z1T3_f_(int narg, int ps[], void (*f)(double, double, double, double, double (*)[3]));
int ana_v_dT4dp3_iDaT4rDp3p3q_0T4_f_(int narg, int ps[], void (*f)(double, double, double, double, double (*)[3]));
int ana_v_dT4dpT3_iDaDaoDqT3_0z1z2T4_s_(int narg, int ps[], void (*f)(double, double, double, double, double *, double *, double *));
int ana_v_dT4dpT4_iDaDaDoDqT4_0T2z3T6_s_(int narg, int ps[], void (*f)(double, double, double, double, double *, double *, double *, double *));
int ana_v_dT4dpdp3T5_iDaT3oDqDp3p3qT5_0z1T8_s_(int narg, int ps[], void (*f)(double, double, double, double, double *, double (*)[3], double (*)[3], double (*)[3], double (*)[3], double (*)[3]));
int ana_v_dT4dpdp_iDaT3oDqDq_0T2z34_s_(int narg, int ps[], void (*f)(double, double, double, double, double *, double *));
int ana_v_dT4dpdp_iDaT4oDqDq_0T5_s_(int narg, int ps[], void (*f)(double, double, double, double, double *, double *));
int ana_v_dT6dp3_iDaT4rDp3p3q_0z11T4_f_(int narg, int ps[], void (*f)(double, double, double, double, double, double, double (*)[3]));
int ana_v_dT6dp3_iDaT6oDp2p3q_0T6_f_(int narg, int ps[], void (*f)(double, double, double, double, double, double, double [2][3]));
int ana_v_dT6dpT6_iDaT6oDqT6_0T11_s_(int narg, int ps[], void (*f)(double, double, double, double, double, double, double *, double *, double *, double *, double *, double *));
int ana_v_dT8dp3_iDaT5rDp3p3q_0zzz1T5_f_(int narg, int ps[], void (*f)(double, double, double, double, double, double, double, double, double (*)[3]));
int ana_v_dddp3T3_iLaoD33oDp3p3qDcq_0z1T3_s_(int narg, int ps[], void (*f)(double, double, double (*)[3], double (*)[3], double (*)[3]));
int ana_v_dddp3_iDarDp3p3q_0z1_f_(int narg, int ps[], void (*f)(double, double, double (*)[3]));
int ana_v_dddp3dp3_iDaiDp3p3arDcq_0z12_f_(int narg, int ps[], void (*f)(double, double, double (*)[3], double (*)[3]));
int ana_v_dddpT16_iDaoDqT16_0z1T16_s_(int narg, int ps[], void (*f)(double, double, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *, double *));
int ana_v_dddpT3_iDaoDqT3_0z1T3_s_(int narg, int ps[], void (*f)(double, double, double *, double *, double *));
int ana_v_dddpT3dp3T5_iDaoDqT3Dp3p3qT5_0z1T8_s_(int narg, int ps[], void (*f)(double, double, double *, double *, double *, double (*)[3], double (*)[3], double (*)[3], double (*)[3], double (*)[3]));
int ana_v_dddpT4_iDaoDqT4_0z1T4_s_(int narg, int ps[], void (*f)(double, double, double *, double *, double *, double *));
int ana_v_dddp_iDaDarDp3q_0T2_f_(int narg, int ps[], void (*f)(double, double, double [3]));
int ana_v_dddpdp_iDaoDqDq_0z12_s_(int narg, int ps[], void (*f)(double, double, double *, double *));
int ana_v_ddp3_iD1D33_01_s_(int narg, int ps[], void (*f)(double, double [3][3]));
int ana_v_dp3T3_iD23aD23qoDcq_0T2_f_(int narg, int ps[], void (*f)(double [2][3], double [2][3], double [2][3]));
int ana_v_dp3T3_iD33aDaoDc_0T2_f_(int narg, int ps[], void (*f)(double [3][3], double [2][3], double [2][3]));
int ana_v_dp3T3_iD33aDarDq_0T2_f_(int narg, int ps[], void (*f)(double (*)[3], double (*)[3], double (*)[3]));
int ana_v_dp3_rD33_0_f_(int narg, int ps[], void (*f)(double (*)[3]));
int ana_v_dp3ddp3dp3_iD33aDmmaDarDq_0T3_f_(int narg, int ps[], void (*f)(double (*)[3], double, double (*)[3], double (*)[3]));
int ana_v_dp3dp3_iD33arDq_01_f_(int narg, int ps[], void (*f)(double [3][3], double [3][3]));
int ana_v_dp3dp3dp_iD23aD23qoDcm3q_0T2_f_(int narg, int ps[], void (*f)(double [2][3], double [2][3], double [2]));
int ana_v_dp3dpT6_iD23aoDm2m3aDcT5_0T6_s_(int narg, int ps[], void (*f)(double [2][3], double *, double *, double *, double *, double *, double *));
int ana_v_dp3dp_iD33arDm3q_01_f_(int narg, int ps[], void (*f)(double [3][3], double [3]));
int ana_v_dp3dp_oD33D3_01_s_(int narg, int ps[], void (*f)(double (*)[3], double *));
int ana_v_dp3dpdp_iD23aoDm2m3qDcq_0T2_s_(int narg, int ps[], void (*f)(double [2][3], double *, double *));
int ana_v_dp3dpdp_iD33aDm3arDcq_0T2_f_(int narg, int ps[], void (*f)(double [3][3], double [3], double [3]));
int ana_v_dp3dpdp_iD33aoDm3m3qT2_0T2_s_(int narg, int ps[], void (*f)(double (*)[3], double *, double *));
int ana_v_dpT3_iD3aoDm3qoDq_0T2_s_(int narg, int ps[], void (*f)(double *, double *, double *));
int ana_v_dpT3_iD3arDm3p2q_011_f_(int narg, int ps[], void (*f)(double *, double *, double *));
int ana_v_dpT3_rD3_000_f_(int narg, int ps[], void (*f)(double *, double *, double *));
int ana_v_dpT4_iD3arD3q_0111_f_(int narg, int ps[], void (*f)(double *, double *, double *, double *));
int ana_v_dpdp3_iD3arDp3q_01_f_(int narg, int ps[], void (*f)(double [3], double [3][3]));

void register_ana_f(int (*f)(int, int []), char *name, int min_arg,
                   int max_arg, char *spec);
void register_ana_s(int (*f)(int, int []), char *name, int min_arg,
                   int max_arg, char *spec);

#define REGISTER(func, fs, name, minarg, maxarg, fsspec)        \
void register_ana_ ## func ## _ ## fs (void) __attribute__((constructor)); \
void register_ana_ ## func ## _ ## fs (void) { \
  register_ana_ ## fs( ana_ ## func, #name, minarg, maxarg, fsspec );    \
}

#define BIND(func, type, fs, name, minarg, maxarg, fsspec)      \
int ana_ ## func ## _ ## fs(int narg, int ps[]) { \
 int result = ana_ ## type ## _ ## fs ## _(narg, ps, func); \
 if (result < 0) anaerror("Error in " #name, 0); \
 return result; \
} \
REGISTER(func ## _ ## fs, fs, name, minarg, maxarg, fsspec)

#endif
